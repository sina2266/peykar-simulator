/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>
#include <iterator>
#include <vector>
#include <filesystem>
using std::stringstream;
using std::cout;
using std::endl;

#include <VCELL/SimTypes.h>
#include <VCELL/SimTool.h>
#include <VCELL/Simulation.h>
#include <VCELL/FVDataSet.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Variable.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/FVUtils.h>
#include <VCELL/PostProcessingHdf5Writer.h>
#include <VCELL/VolumeParticleVariable.h>
#include <VCELL/MembraneParticleVariable.h>
#include <VCELL/Element.h>
#include <vcellhybrid.h>

#include <cfloat>
#include <cmath>
#include <ctime>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <VCELL/ZipUtils.h>

#include <algorithm>
using std::min;
using std::max;

#if ( !defined(WIN32) && !defined(WIN64) ) // UNIX
#include <unistd.h>
#endif

#define ZIP_FILE_LIMIT 1E9

#define MESH_FILE_EXT ".mesh"
#define MESHMETRICS_FILE_EXT ".meshmetrics"
#define SIM_FILE_EXT ".sim"
#define PARTICLE_FILE_EXT ".particle"
#define LOG_FILE_EXT ".log"
#define ZIP_FILE_EXT ".zip"
#define TID_FILE_EXT ".tid"
#define HDF5_FILE_EXT ".hdf5"

/*
#ifdef VCELL_HYBRID
	void smoldynOneStep(simptr sim);
	void vcellhybrid::smoldynOneStep(simptr sim) {
		::smoldynOneStep(sim);
	}
	simptr smoldynInit(SimTool* simTool, string& root);
	simptr vcellhybrid::smoldynInit(SimTool* simTool, string& root) {
		return ::smoldynInit(simTool, root);
	}
	void smoldynEnd(simptr sim);
	void vcellhybrid::smoldynEnd(simptr sim) {
		::smoldynEnd(sim);
	}
}
#endif
*/

static int NUM_TOKENS_PER_LINE = 4;
static constexpr int numRetries = 2;
static constexpr int retryWaitSeconds = 5;

SimTool::SimTool()
	:bSimZip(true ),
	 vcellModel(nullptr),
	simulation(nullptr),
	_timer(nullptr),
	bSimFileCompress(false),
	simEndTime(0),
	simStartTime(0),
	bCheckSpatiallyUniform(false),
	simDeltaTime(0),
	smoldynStepMultiplier(1),
	keepEvery(100),
	bStoreEnable(true),
	simFileCount(0),
	zipFileCount(0),
	solver(FV_SOLVER ),
    discontinuityTimes(nullptr),
	numDiscontinuityTimes(0),
	bLoadFinal(true ),
	sundialsSolverOptions( ),
	spatiallyUniformAbsTol(1e-6 ),
	spatiallyUniformRelTol(1e-3),
	pcgRelTol(1e-8),

	bSundialsOneStepOutput(false),
	keepAtMost(5000),

    serialScanParameterValues(nullptr),
	numSerialParameterScans(0),

	postProcessingHdf5Writer(nullptr),
	smoldynSim(nullptr) {}

SimTool::~SimTool()
{
	delete[] discontinuityTimes;

	for (int i = 0; i < numSerialParameterScans; i ++) {
		delete[] serialScanParameterValues[i];
	}
	delete[] serialScanParameterValues;

	delete postProcessingHdf5Writer;
}

void SimTool::setModel(VCellModel* model) {
	if (model == nullptr) {
		throw runtime_error("SimTool::setModel(), model can't be null");
	}
	vcellModel = model;
}

void SimTool::setSimulation(SimulationExpression* sim) {
	if (sim == nullptr) {
		throw runtime_error("SimTool::setSimulation(), simulation can't be null");
	}
	simulation = sim;
	simulation->setDT_sec(simDeltaTime);
}

void SimTool::setTimeStep(double period) {
	simDeltaTime = period;
}

void SimTool::setSmoldynStepMultiplier(int steps) {
    if(steps <= 0) {
        throw runtime_error("SimTool::setSmoldynStepMultiplier(), smoldynStepMultiplier must be a positive integer");
    }
	smoldynStepMultiplier = steps;
}

void SimTool::requestNoZip() {
	bSimZip = false;
}

bool SimTool::checkStopRequested() {
	 return SimulationMessaging::getInstVar()->isStopRequested();
}

TimerHandle SimTool::getTimerHandle(string& identifier)
{
	if (_timer==nullptr){
		_timer = new Timer();
	}
	return _timer->registerID(identifier);
}

void SimTool::startTimer(TimerHandle hnd) const
{
	_timer->start(hnd);
}

void SimTool::stopTimer(TimerHandle hnd) const
{
	_timer->stop(hnd);
}

double SimTool::getElapsedTimeSec(TimerHandle hnd) const
{
	double time=0.0;
	_timer->getElapsedTimeSec(hnd, time);
	return time;
}

void SimTool::showSummary(FILE *fp)
{
	fprintf(fp,"--------------- sim summary ----------------\n");
	simulation->getMesh()->showSummary(fp);
	fprintf(fp,"\ttime = %lg sec\n",simulation->getTime_sec(this));
	fprintf(fp,"\tdT   = %lg sec\n",simulation->getDT_sec());
	fprintf(fp,"--------------------------------------------\n");
	if (_timer){
		fprintf(fp,"--------------- benchmark times ------------\n");
		_timer->show();
		fprintf(fp,"--------------------------------------------\n\n\n");
	}
}

void SimTool::setBaseFilename(const std::filesystem::path& fname) {
	if (fname.string().empty()) {
		throw runtime_error("invalid base file name for data set");
	}
	std::cout <<  "Setting Base file name to: `" << fname << "`" << std::endl;
	baseFileName = fname;
	baseDirName = baseFileName.parent_path();
	baseSimName = baseFileName.filename();
}

static void retryWait(int seconds) {
#if ( defined(WIN32) || defined(WIN64) )
	Sleep(seconds * 1000);
#else
	sleep(seconds);
#endif
}

static FILE* openFileWithRetry(const char* fileName, const char* mode) {
	FILE *fp = nullptr;
	for (int retry = 0; retry < numRetries; retry ++) {
		fp = fopen(fileName, mode);

		if (fp != nullptr) {
			break;
		}
		if (retry < numRetries - 1) {
			cout << "SimTool, error opening log file <" << fileName << ">, trying again" << endl;
			retryWait(retryWaitSeconds);
		}
	}
	return fp;
}

static bool unzipWithRetry(const filesystem::path& zipFileName, const filesystem::path& simFileName, std::string* errmsg) {
	bool bSuccess = false;
	for (int attemptNum = 0; attemptNum <= numRetries; attemptNum++) {
		try {
			extractFileFromZip(zipFileName, simFileName);
			bSuccess = true;
			break;
		} catch (runtime_error &e) {
			errmsg->clear();
			errmsg->append(e.what());
		}
		if (attemptNum == numRetries) continue;
		retryWait(retryWaitSeconds);
		cout << "SimTool::updateLog(), adding .sim to .zip failed (attempt "
			<< (attemptNum + 1) << "), trying again" << endl;

	}
	if (!bSuccess) {
		errmsg->clear();
		errmsg->append("Writing zip file <").append(zipFileName.string()).append("> failed.");
	}
	return bSuccess;
}

static bool zipWithRetry(const filesystem::path& zipFileName, const filesystem::path& simFileName, std::string* errmsg) {
	bool bSuccess = false;
	for (int attemptNum = 0; attemptNum <= numRetries; attemptNum++) {
		try {
			addFilesToZip(zipFileName, simFileName);
			bSuccess = true;
			break;
		} catch (runtime_error& e) {
			errmsg->clear();
			errmsg->append(e.what());
		}
		if (attemptNum == numRetries) continue;
		retryWait(retryWaitSeconds);
		cout << "SimTool::updateLog(), adding .sim to .zip failed (attempt "
			<< (attemptNum + 1) << "), trying again" << endl;

	}
	if (!bSuccess) {
		errmsg->clear();
		errmsg->append("Writing zip file <").append(zipFileName.string()).append("> failed.");
	}
	return bSuccess;
}

void SimTool::loadFinal()
{
	if (!bLoadFinal) {
		clearLog();
		return;
	}

	if (smoldynSim != nullptr) {
		clearLog();
		return;
	}

	//
	// read '.log' file to determine simulation time and iteration
	//

	bool bStartOver = true;

	std::filesystem::path logFileName = baseFileName;
	logFileName.append(LOG_FILE_EXT);
	std::filesystem::path zipFileName = baseFileName;
	const std::filesystem::path dataFileName;


	FILE* tidFP = lockForReadWrite();
	FILE* logFP = fopen(logFileName.string().c_str(), "r");

	if (logFP != nullptr) {
		bStartOver = false; // log file exists, there is old data
		zipFileName.append("00").append(ZIP_FILE_EXT);

		if (exists(zipFileName)) {
			bSimZip = false;
			NUM_TOKENS_PER_LINE = 3;
		} else {
			bSimZip = true;
			NUM_TOKENS_PER_LINE = 4;
		}

		simStartTime = -1;
		int tempIteration = -1, tempFileCount = 0;
		int numTimes = 0;

		while (!feof(logFP)){
			char logBuffer[1024];
			if (!fgets(logBuffer, 1024, logFP)){
				break;
			}
			numTimes ++;
			//
			// look for line with a valid filename (includes basename)
			//
			if (strstr(logBuffer, baseSimName.string().c_str())){
				//
				// parse iteration number and time
				//
				int numTokens;
				if (bSimZip) {
					numTokens = sscanf(logBuffer, "%d %s %s %lg", &tempIteration, dataFileName.string().c_str(), zipFileName.string().c_str(), &simStartTime);
				} else {
					numTokens = sscanf(logBuffer, "%d %s %lg", &tempIteration, dataFileName.string().c_str(), &simStartTime);
				}
				if (numTokens != NUM_TOKENS_PER_LINE){
					printf("SimTool::load(), error reading log file %s, reading iteration\n", logFileName.string().c_str());
					printf("error in line %d = '%s'\n", tempFileCount, logBuffer);
					bStartOver = true;
					break;
				}
			}
			tempFileCount++;
		} // while (!feof(logFP))

		// close log file
		fclose(logFP);

		if (tempIteration <= 0  || simStartTime <= 0) {
			bStartOver = true;
		}

		if (!bStartOver) {
			if (bSimZip) {
				// check if zip file exists
				std::filesystem::path zipFileAbsoluteName;
				const bool needs_path_completion = zipFileName.is_absolute() && is_directory(baseDirName);
				if (needs_path_completion) {
					zipFileAbsoluteName = baseDirName / zipFileName;
				} else {
					zipFileAbsoluteName = zipFileName;
				}
				if (exists(zipFileAbsoluteName)) {
					cout << "SimTool::loadFinal(), unable to open zip file <" << zipFileAbsoluteName << ">" << endl;
					bStartOver = true;
				} else {
					// unzip the file (without directory) into exdir, currently we
					// unzip the file to the current working directory
					std::string errmsg;
					unzipWithRetry(zipFileAbsoluteName.string().c_str(), dataFileName.string().c_str(), &errmsg);
				}
			}

			if (!bStartOver) {
				// otherwise check if sim file exists
				if (exists(dataFileName)) {
					cout << "SimTool::loadFinal(), unable to open sim file <" << dataFileName << ">" << endl;
					bStartOver = true;
				} else {
					try {
						if (postProcessingHdf5Writer != nullptr) {
							bStartOver = !postProcessingHdf5Writer->loadFinal(numTimes);
						}

						if (!bStartOver) {
							FVDataSet::read(dataFileName.string().c_str(), simulation);
							simulation->setCurrIteration(tempIteration);
							// set start time on sundials
							if (isSundialsPdeSolver()) {
								simulation->setSimStartTime(this, simStartTime);
							}
							simFileCount = tempFileCount;

							if (bSimZip) {
								remove(dataFileName.string().c_str());
								zipFileCount = getZipCount(zipFileName);
								// wrong zip file Name
								if (zipFileCount < 0) { // should never happen
									bStartOver = true;
								}  else {
									// check if this zip file is already big enough
									if (exists(zipFileName)) {
										if (file_size(zipFileName) > ZIP_FILE_LIMIT) {
											zipFileCount ++;
										}
									}
								}
							}
						}
					} catch (const char* msg) {
						cout << "SimTool::loadFinal() : dataSet.read(" << dataFileName << " failed : " << msg << endl;
						bStartOver = true;
					} catch (...) {
						cout << "SimTool::loadFinal() : dataSet.read(" << dataFileName << " failed : unexpected error" << endl;
						bStartOver = true;
					}
				}
			} // if (!bStartOver)
		}  // if (!bStartOver)
	} // if (logFP != NULL)

	// close tid file
	if (tidFP != nullptr) {
		fclose(tidFP);
	}
	if (bStartOver) {
		clearLog();
	}
}
void SimTool::checkTaskIdLockFile() const
{
	FILE* fp = lockForReadWrite();
	if (fp != nullptr) {
		fclose(fp);
	}
}

FILE* SimTool::lockForReadWrite() const
{
	const int myTaskID = SimulationMessaging::getInstVar()->getTaskID();
	if (myTaskID < 0) return nullptr;


	std::string tidFileName = baseFileName.string();
	tidFileName.append(TID_FILE_EXT);

	bool bExist = false;

	struct stat buf{};
	if (stat(tidFileName.c_str(), &buf) == 0) { // if exists
		bExist = true;
	}

	FILE* fp = openFileWithRetry(tidFileName.c_str(), bExist ? "r+" : "w+");

	if (fp == nullptr){
		throw runtime_error("SimTool::lockForReadWrite() - error opening .tid file <" + tidFileName + ">");
	}

	if (bExist) {
		int taskIDInFile = 0;
		int numRead = fscanf(fp, "%d", &taskIDInFile);
		if (numRead == 1) {
			if (myTaskID < taskIDInFile) {
				ostringstream msg;
				msg <<  "Starting simulation with taskid=" << myTaskID <<" but lockfile has later taskid="
						<< taskIDInFile << std::ends;
				throw std::runtime_error(msg.str( ));
			}
			if (myTaskID == taskIDInFile) { // it's me
				return fp;
			}
		}
		rewind(fp);
	}
	fprintf(fp, "%5d", myTaskID);
	fflush(fp);

	return fp;
}

std::string getFileName(const std::string& path){
	return path.substr(1 + path.find_first_of("\\/"));
}

void SimTool::updateLog(double progress, double time, int iteration)
{
	if (bStoreEnable) {
		FILE *logFP;
		std::string simFileName;

		bool bSuccess = true;
		std::string errorMsg;
		FILE* tidFP = lockForReadWrite();

		struct stat buf{};
#if ( defined(WIN32) || defined(WIN64) ) // Windows
		wstring TempPath;
		wchar_t wcharPath[128]; // This needs to be fixed when on windows.
		if (GetTempPathW(128, wcharPath)){
			TempPath = wcharPath;
		}else{
			throw runtime_error("failed to obtain system temp directory");
		}
		std::string tempDir( TempPath.begin(), TempPath.end() );
#else
		std::string tempDir = "/tmp/";
		char const *folder = getenv("TMPDIR");
		if (folder != nullptr)
    		tempDir = folder;		
    	if (tempDir.at(tempDir.length()-1) != '/') 
    	    tempDir += '/';
#endif
		std::cout << "temporary directory used is " << tempDir << std::endl;
		static bool bUseTempDir = false;
		static bool bFirstTimeUpdateLog = true;

		if (bFirstTimeUpdateLog) {
			if (stat(tempDir.c_str(), &buf) == 0) {
				// use local temp directory for .sim files
				// to avoid network traffic
				if (buf.st_mode & S_IFDIR) {
					bUseTempDir = true;
				}
			}
			bFirstTimeUpdateLog = false;
		}

		std::stringstream simFileNameStream;
		// write sim files to local
		if (bSimZip && bUseTempDir) {
			simFileNameStream << tempDir << getFileName(baseSimName.string()) << std::setfill('0') << std::setw(4) << simFileCount << SIM_FILE_EXT;
		} else {
			simFileNameStream << baseFileName.string() << std::setfill('0') << std::setw(4) << simFileCount << SIM_FILE_EXT;
		}
		simFileName = simFileNameStream.str();
		std::cout << "sim file name is " << simFileName << std::endl;

		std::string particleFileName{simFileName};
		particleFileName.append(PARTICLE_FILE_EXT);
		
		simulation->writeData(simFileName.c_str(), bSimFileCompress);

		// hybrid runs: the molecules' positions at this output time, when VCell asked for particle files.
		// Named as VCell reads them (SimulationData.createCanonicalSmoldynOutputFileName): <base>_<NNN>.smoldynOutput,
		// NNN = this output's 1-based index in the log.
		if (smoldynSim != nullptr && vcellhybrid::savesParticlePositions()) {
			std::stringstream particleOutputName;
			particleOutputName << baseFileName.string() << "_" << std::setfill('0') << std::setw(3) << (simFileCount + 1)
				<< ".smoldynOutput";
			vcellhybrid::writeParticlePositions(smoldynSim, particleOutputName.str());
		}

		std::string logFileName = baseFileName.string();
		logFileName.append(LOG_FILE_EXT);

		logFP = openFileWithRetry(logFileName.c_str(), "a");

		if (logFP == nullptr) {
			errorMsg.append("SimTool::updateLog() - error opening log file <").append(logFileName).append(">");
			bSuccess = false;
		} else {
		   // write zip file first, then write log file, in case that
		   // zipping fails
			if (bSimZip) {
				//int retcode = 0;
				std::stringstream zipFileNameStream;
				zipFileNameStream << baseFileName.string() << std::setfill('0') << std::setw(2) << zipFileCount << ZIP_FILE_EXT;
				std::string zipFileName{zipFileNameStream.str()};
				if (stat(particleFileName.c_str(), &buf) == 0) {	// has particle
				//	retcode = zip32(2, zipFileName, simFileName, particleFileName);
					addFilesToZip(zipFileName.c_str(), simFileName.c_str(), particleFileName.c_str());
					remove(particleFileName.c_str());
				} else {
					bSuccess = zipWithRetry(zipFileName.c_str(), simFileName.c_str(), &errorMsg);
				}
				remove(simFileName.c_str());

				// write the log file
				if (bSuccess) {
					// write hdf5 post processing before writing log entry
					if (postProcessingHdf5Writer != nullptr) {
						postProcessingHdf5Writer->writeOutput(this);
					}



					std::stringstream zipFileNameWithoutPathStream;
					zipFileNameWithoutPathStream << baseSimName.string() << std::setfill('0') << std::setw(2) <<
						zipFileCount << ZIP_FILE_EXT;
					std::stringstream simFileNameWithoutPathStream;
					simFileNameWithoutPathStream << baseSimName.string() << std::setfill('0') << std::setw(4) <<
						simFileCount << SIM_FILE_EXT;
					fprintf(logFP,"%4d %s %s %.15lg\n", iteration, simFileNameWithoutPathStream.str().c_str(),
						zipFileNameWithoutPathStream.str().c_str(), time);

					if (stat(zipFileName.c_str(), &buf) == 0) { // if exists
						if (buf.st_size > ZIP_FILE_LIMIT) {
							zipFileCount ++;
						}
					}
				}
			} else { // old format, no zip
				std::stringstream simFileNameWithoutPathStream;
				simFileNameWithoutPathStream << baseSimName.string() << std::setfill('0') << std::setw(4) <<
					simFileCount << SIM_FILE_EXT;
				fprintf(logFP,"%4d %s %.15lg\n", iteration,
					simFileNameWithoutPathStream.str().c_str(), time);
			}
		}
		// close log file
		fclose(logFP);
		// close tid file
		if (tidFP != nullptr) {
			fclose(tidFP);
		}
		if (bSuccess) {
			simFileCount++;
		} else {
			throw runtime_error(errorMsg);
		}
	} else{
		// write hdf5 post processing before writing log entry
		if (postProcessingHdf5Writer != nullptr) {
			postProcessingHdf5Writer->writeOutput(this);
		}

	}

	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_DATA, progress, time));
}

int SimTool::getZipCount(const filesystem::path& zipFileName) {
	size_t firstIndex = zipFileName.string().find_first_of(ZIP_FILE_EXT);
	if (firstIndex == std::string::npos || firstIndex < 2) return -1;

	std::string strCount{zipFileName.string().substr(firstIndex - 2, 2)};
	return std::stoi(strCount);
}

void SimTool::clearLog(){
	this->simStartTime = 0;
	this->simFileCount = 0;
	this->zipFileCount = 0;

	if (this->isSundialsPdeSolver()) {
		this->simulation->setSimStartTime(this, 0);
	}

	std::string logFileName;
	std::string buffer;

	// remove mesh file
	buffer.append(this->baseFileName.string()).append(MESH_FILE_EXT);
	remove(buffer.c_str());
	buffer.clear();

	buffer.append(this->baseFileName.string()).append(MESHMETRICS_FILE_EXT);
	remove(buffer.c_str());
	buffer.clear();

	buffer.append(this->baseFileName.string()).append(ZIP_FILE_EXT);
	remove(buffer.c_str());
	buffer.clear();

	buffer.append(this->baseFileName.string()).append("00").append(ZIP_FILE_EXT);
	remove(buffer.c_str());
	buffer.clear();

	logFileName.append(this->baseFileName.string()).append(LOG_FILE_EXT);

	std::filesystem::path logFilePath{logFileName};

	if (!std::filesystem::exists(logFilePath)) {
		printf("No log-file found at constructed path `%s`.", logFilePath.string().c_str());
		return;
	}

	std::ifstream logStream{};
	logStream.open(logFilePath.string().c_str());

	if (!logStream.is_open()) {
		printf("error opening log file <%s>\n", logFilePath.string().c_str());
		return;
	}

	std::string simFileName;
	std::string zipFileName;
	int iteration, oldCount=-1, correctNumTokens = this->bSimZip ? 4 : 3;
	double time;

	while (logStream.good()) {
		std::string lineBuffer{};
		std::getline(logStream, lineBuffer);
		std::istringstream iss(lineBuffer);
		std::vector<std::string> tokens{std::istream_iterator<std::string>{iss}, std::istream_iterator<std::string>{}};
		if (tokens.empty()) break;
		if (tokens.size() != correctNumTokens) {
			std::cerr << "Unexpected number of tokens (found:" << tokens.size() << ", expected: " << correctNumTokens << ")" << std::endl;
			break;
		}
		iteration = std::stoi(tokens[0]);
		simFileName = tokens[1];
		zipFileName = this->bSimZip ? tokens[2] : "";
		time = std::stod(tokens[this->bSimZip ? 3 : 2]);

		char simFileNameCharArray[simFileName.size()];
		strcpy(simFileNameCharArray, simFileName.c_str());
		char *dotSim = strstr(simFileNameCharArray, SIM_FILE_EXT);
		if (!dotSim) continue;

		*dotSim = '\0';

		buffer.append(this->baseFileName.string()).append(SIM_FILE_EXT);
		remove(buffer.c_str());
		buffer.clear();

		buffer.append(this->baseFileName.string()).append(SIM_FILE_EXT).append(PARTICLE_FILE_EXT);
		remove(buffer.c_str());
		buffer.clear();

		if (!bSimZip) continue;
		const int count = getZipCount(zipFileName);
		if (oldCount != count && count >= 0) {
			remove(zipFileName.c_str());
			oldCount = count;
		}
	}

	logStream.close();
	printf("SimTool::clearLog(), removing log file %s\n",logFileName.c_str());
	std::filesystem::remove(logFilePath);
}

bool SimTool::isSundialsPdeSolver() const
{
	return solver == SUNDIALS_PDE_SOLVER;
}

void SimTool::setSolver(const string& s) {
	if (!s.empty() && s != FV_SOLVER && s != SUNDIALS_PDE_SOLVER) {
		stringstream ss;
		ss << "unknown solver : " << s;
		throw runtime_error(ss.str());
	}
	solver = s;
}

void SimTool::start() {
	if (simulation->getNumVariables() == 0) {
		return;
	}
	simulation->resolveReferences(this);
    if (numSerialParameterScans == 0 ) {
        // Is Not paramScan, the .fvinput had no SERIAL_SCAN_PARAMETER_... section
        start1();
    } else if (SimulationMessaging::getInstVar()->getTaskID() >= 0) {
        // Is paramScan, the .fvinput had 'SERIAL_SCAN_PARAMETER_...' section
        // Is also a remote server (green-button) run , the .fvinput has 'JMS_PARAM_BEGIN' section
        // Each paramScan is run as a separate job with serial jobIndex in the 'JMS_PARAM_BEGIN' section
        SimulationExpression* sim = simulation;
        sim->setParameterValues(this, serialScanParameterValues[SimulationMessaging::getInstVar()->getJobIndex()]);
		start1();
	} else {
        // Is paramScan, the .fvinput had SERIAL_SCAN_PARAMETER_... section
        // Is also a local (blue-button) run where each paramScan is run in the following loop
		SimulationExpression* sim = simulation;
		for (int scan = 0; scan < numSerialParameterScans; scan ++) {
			if (scan > 0) {
				string bfn(baseFileName.string());
				char oldIndex[10], newIndex[10];
				sprintf(oldIndex, "_%d_\0", scan - 1);
				sprintf(newIndex, "_%d_\0", scan);
				int p = (int)bfn.rfind(oldIndex);
				bfn.replace(p, strlen(oldIndex), newIndex);
				setBaseFilename(filesystem::path(bfn));
			}
			sim->setParameterValues(this, serialScanParameterValues[scan]);
			start1();
		}
	}
}

void SimTool::setSmoldynInputFile(const string& inputfile) {
	smoldynInputFile = inputfile;
}

void SimTool::start1() {

	// create post processing hdf5 writer
	if (simulation->getPostProcessingBlock() != nullptr) {
		if (postProcessingHdf5Writer == nullptr) {
			std::string h5PPFileName;
			h5PPFileName.append(baseFileName.string()).append(HDF5_FILE_EXT);
			postProcessingHdf5Writer = new PostProcessingHdf5Writer(h5PPFileName, simulation->getPostProcessingBlock());
		}
	}

	simulation->initSimulation(this);

	if (!smoldynInputFile.empty()) {
		smoldynSim = vcellhybrid::smoldynInit(this, smoldynInputFile);//smoldynInit will write output therefore computeHistogram is called by VCellSmoldynOutput.write().
		copyParticleCountsToConcentration();
		// since smoldyn only initializes variable current value,
		// we need to copy current to old.
		for (int i = 0; i < (int)simulation->getNumVariables(); i ++) {
			Variable* var = simulation->getVariable(i);
			var->update();
		}
	}

	loadFinal();   // initializes to the latest file if it exists

	if (checkStopRequested()) {
		return;
	}

	if (bStoreEnable && !baseFileName.has_filename()) {
		throw runtime_error("Invalid base file name "+baseFileName.string()+"for dataset");
	}

	std::string message;
	message.append("simulation [").append(baseSimName.string()).append("] started");
	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_STARTING, message.c_str()));

	//
    // destroy any partial results from unfinished iterations
    //
    double percentile = simStartTime/simEndTime;
    double increment = 0.01;
	double lastSentPercentile = percentile;

	if (simulation->getCurrIteration()==0) {
		// simulation starts from scratch, needs to
		// write .mesh and .meshmetrics file
		if (bStoreEnable){
			FILE *fp;
			std::string filename;
			filename.append(baseFileName.string()).append(MESH_FILE_EXT);
			if ((fp=fopen(filename.c_str(),"w")) == nullptr){
				std::string errorMessage;
				errorMessage.append("cannot open mesh file ").append(filename).append(" for writing");
				throw runtime_error(errorMessage);
			}
			simulation->getMesh()->write(fp);
			fclose(fp);

			filename.erase(strlen(baseFileName.string().c_str()), 5);
			filename.append(MESHMETRICS_FILE_EXT);
			if ((fp=fopen(filename.c_str(),"w")) == nullptr){
				std::string errorMessage;
				errorMessage.append("cannot open mesh metrics file ").append(filename).append(" for writing");
				throw runtime_error(errorMessage);
			}
			simulation->getMesh()->writeMeshMetrics(fp);
			fclose(fp);
		}
		updateLog(0.0, 0.0, 0);
	} else {
		// simulation continues from existing results, send data message
		SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_DATA, percentile, simStartTime));
	}

    //
    // iterate up to but not including end time
    //
	if (bCheckSpatiallyUniform) {
		increment = 0.001;
	}
	clock_t oldTime = clock(); // to control the output of progress, send progress every 2 seconds.
	int counter = 0;
	while (true) {
		double epsilon = 1e-12;
		if (simulation->getTime_sec(this) - simEndTime > epsilon // currentTime past endTime
				|| fabs(simEndTime - simulation->getTime_sec(this)) < epsilon) { // reached the end time
			break;
		}
		if (isSundialsPdeSolver() && bSundialsOneStepOutput) {
			if (simulation->getCurrIteration() / keepEvery > keepAtMost){ // reached keep at most
				break;
			}
		}

		if (checkStopRequested()) {
			return;
		}

		counter++;
		simulation->iterate(this);
		if (smoldynSim != nullptr && counter == smoldynStepMultiplier) {
			vcellhybrid::smoldynOneStep(smoldynSim);//smoldynOneStep includes computeHistogram after each time step.
			copyParticleCountsToConcentration();
			counter = 0;
		}

		if (checkStopRequested()) {
			return;
		}

		simulation->update(this);

		if (simulation->getCurrIteration() % keepEvery == 0 || fabs(simEndTime - simulation->getTime_sec(this)) < epsilon) {
			updateLog(percentile,simulation->getTime_sec(this), simulation->getCurrIteration());
        }

		clock_t currentTime = clock();
		double duration = (double)(currentTime - oldTime) / CLOCKS_PER_SEC;
		if (duration >= 2){
			percentile = (simulation->getTime_sec(this) - simStartTime)/(simEndTime - simStartTime);
			if (percentile - lastSentPercentile >= increment) {
				std::cout << "SimTool.start1() sending JOB_PROGRESS to SimulationMessaging: percentile=" << percentile << ", time=" << simulation->getTime_sec(this) << std::endl;
				SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_PROGRESS, percentile, simulation->getTime_sec(this)));
				lastSentPercentile = percentile;
				oldTime = currentTime;
			}
		}

		if (bCheckSpatiallyUniform) {
			bool uniform = true;
			for (int i = 0; i < simulation->getNumVariables(); i ++) {
				if (!checkSpatiallyUniform(simulation->getVariable(i))) {
					uniform = false;
					break;
				}
			}
			if (uniform) {
				cout << endl << "!!!Spatially Uniform reached at " << simulation->getTime_sec(this) << endl;
				if (simulation->getCurrIteration() % keepEvery != 0) {
					updateLog(percentile,simulation->getTime_sec(this), simulation->getCurrIteration());
				}
				break;
			}
		}
	}

	if (smoldynSim != nullptr) {
		vcellhybrid::smoldynEnd(smoldynSim);
	}
	if (checkStopRequested()) {
		return;
	}

	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_PROGRESS, 1.0, simulation->getTime_sec(this)));
	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_COMPLETED, percentile, simulation->getTime_sec(this)));

	showSummary(stdout);
}

void SimTool::copyParticleCountsToConcentration(){ //concentration in terms of particleCounts/mesh element size
	//translating the particle counts into concentration (molecules/mesh size) for each mesh element
	int numVars = simulation->getNumVariables();
	for (int i=0; i<numVars; i++){
		if(simulation->getVariable(i)->getVarType() == VAR_VOLUME_PARTICLE){
			auto* var = (VolumeParticleVariable*)simulation->getVariable(i);
			for(int j=0; j<simulation->getMesh()->getNumVolumeElements(); j++){
				double count = var->getMoleculeCounts()[j];
				if(count>0){
					var->getCurr()[j]=count/(simulation->getMesh()->getVolumeOfElement_cu(j));
				}else {
					var->getCurr()[j]=0;
				}
			}
		}
		else if(simulation->getVariable(i)->getVarType() == VAR_MEMBRANE_PARTICLE){
			auto* var = (MembraneParticleVariable*)simulation->getVariable(i);
			for(int j=0; j<simulation->getMesh()->getNumMembraneElements(); j++){
				double count = var->getMoleculeCounts()[j];
				if(count>0){
					var->getCurr()[j]=count/(simulation->getMesh()->getMembraneElements()[j].area);
				}else {
					var->getCurr()[j]=0;
				}
			}
		}
	}
}


bool SimTool::checkSpatiallyUniform(Variable* var) {
	double* currSol = var->getCurr();
	CartesianMesh* mesh = simulation->getMesh();
	switch (var->getVarType()) {
		case VAR_VOLUME:
			for (int r = 0; r < mesh->getNumVolumeRegions(); r ++) {
				VolumeRegion* volRegion = mesh->getVolumeRegion(r);
				int numElements = volRegion->getNumElements();
				double minSol = DBL_MAX;
				double maxSol = -DBL_MAX;
				for(int j = 0; j < numElements; j ++){
					int volIndex = volRegion->getElementIndex(j);
					maxSol = max<double>(maxSol, currSol[volIndex]);
					minSol = min<double>(minSol, currSol[volIndex]);
				}
				double den = max<double>(fabs(maxSol), fabs(minSol));
				if (den >= spatiallyUniformAbsTol && (maxSol - minSol)/den > spatiallyUniformRelTol) {
					return false;
				}
			}
			return true;
		case VAR_MEMBRANE:
			for (int r = 0; r < mesh->getNumMembraneRegions(); r ++) {
				MembraneRegion* memRegion = mesh->getMembraneRegion(r);
				int numElements = memRegion->getNumElements();
				double minSol = DBL_MAX;
				double maxSol = -DBL_MAX;
				for(int j = 0; j < numElements; j ++){
					int memIndex = memRegion->getElementIndex(j);
					maxSol = max<double>(maxSol, currSol[memIndex]);
					minSol = min<double>(minSol, currSol[memIndex]);
				}
				double den = max<double>(fabs(maxSol), fabs(minSol));
				if (den >= spatiallyUniformAbsTol && (maxSol - minSol)/den > spatiallyUniformRelTol) {
					return false;
				}
			}
			return true;
		case VAR_VOLUME_REGION:
			return true;
		case VAR_MEMBRANE_REGION:
			return true;
		default:
			return true;
	}
}

void SimTool::setCheckSpatiallyUniform() {
	bCheckSpatiallyUniform = true;
	cout << endl << "----Stop At Spatially Uniform----" << endl;
}

void SimTool::setSerialParameterScans(int numScans, double** values) {
	numSerialParameterScans = numScans;
	serialScanParameterValues = values;
	cout << endl << "----Serial Parameter Scans : " << numSerialParameterScans << endl;
}
