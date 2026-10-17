/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
using std::stringstream;
using std::cout;
using std::endl;

#include <VCELL/ChomboIF.h>
#include <VCELL/ChomboSpec.h>
#include <VCELL/SimTool.h>
#include <VCELL/DataSet.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Variable.h>
#include <VCELL/PostProcessingHdf5Writer.h>
#include <VCELL/ZipUtils.h>

#include <float.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>

#if ( !defined(WIN32) && !defined(WIN64) ) // UNIX
#include <unistd.h>

#include "VCELL/VCellModel.h"
#endif

#define DIRECTORY_SEPARATOR_WINDOWS '\\'
#define DIRECTORY_SEPARATOR_OTHER '/'

#ifndef DIRECTORY_SEPARATOR
#if ( defined(WIN32) || defined(WIN64) || defined(CH_CYGWIN) )
#define DIRECTORY_SEPARATOR DIRECTORY_SEPARATOR_WINDOWS
#else
#define DIRECTORY_SEPARATOR DIRECTORY_SEPARATOR_OTHER
#endif
#endif

#define ZIP_FILE_LIMIT 1E9

// Buffers that hold a path built from BASE_FILE_NAME. They were 128 bytes, and
// VCell passes an absolute path (a user's simdata directory on the desktop,
// /simdata/<user>/ on the cluster), so a long directory silently overran the
// stack and the follow-up open failed on a truncated name. PATH_MAX-sized now;
// the error-message buffers leave room for a path plus the message text.
#define FILE_NAME_BUFSIZE 4096
#define FILE_ERRMSG_BUFSIZE (FILE_NAME_BUFSIZE + 512)

#define SIM_FILE_EXT ".sim"
#define SIM_HDF5_FILE_EXT ".sim.hdf5"
#define MESH_HDF5_FILE_EXT ".mesh.hdf5"
#define LOG_FILE_EXT ".log"
#define ZIP_FILE_EXT ".zip"
#define ZIP_HDF5_FILE_EXT ".hdf5.zip"
#define TID_FILE_EXT ".tid"
#define PP_HDF5_FILE_EXT ".hdf5"

SimTool* SimTool::instance = NULL;

int SimTool::rootRank = 0;
extern bool bConsoleOutput;

static const int numRetries = 2;
static const int retryWaitSeconds = 5;

SimTool::SimTool()
{
	simEndTime = 0.0;
	bStoreEnable = true;
	simFileCount=0;
	zipFileCount = 0;

	vcellModel = new VCellModel(this);
	simulation = new SimulationExpression(this);
	solver = CHOMBO_SEMIIMPLICIT_SOLVER;
	simStartTime = 0;
	
	postProcessingHdf5Writer = NULL;
	
	chomboSpec = new ChomboSpec();
	
#ifdef CH_MPI
	MPI_Comm_rank(MPI_COMM_WORLD,&myRank);
	MPI_Comm_size(MPI_COMM_WORLD,&commSize);
#else
	myRank = rootRank;
	commSize = 1;
#endif
	
//	numDiscontinuityTimes = 0;
//	discontinuityTimes = 0;
//	sundialsRelTol = 1e-7;
//	sundialsAbsTol = 1e-9;
//	sundialsMaxStep = 0.1;
//	pcgRelTol = 1e-8;
//	bSundialsOneStepOutput = false;
//	keepAtMost = 5000;
}

SimTool::~SimTool()
{
	delete postProcessingHdf5Writer;
//	delete[] discontinuityTimes;
}

void SimTool::create() {
	if (instance == 0) {
		instance = new SimTool();
	} else {
		throw "SimTool (singleton) has been created";
	}
}

SimTool* SimTool::getInstance() {
	return instance;
}

bool SimTool::checkStopRequested() {
	 return SimulationMessaging::getInstVar()->isStopRequested();
}

void SimTool::setBaseFilename(string& fname) {
	if (fname.empty()) {
		throw "invalid base file name for data set";
	}
	baseFileName = fname;

	if (baseFileName.find(DIRECTORY_SEPARATOR_OTHER) != string::npos)
	{
		directorySeparator = DIRECTORY_SEPARATOR_OTHER;
	}
	else if (baseFileName.find(DIRECTORY_SEPARATOR_WINDOWS) != string::npos)
	{
		directorySeparator = DIRECTORY_SEPARATOR_WINDOWS;
	}
	else
	{
		directorySeparator = DIRECTORY_SEPARATOR;
	}
	std::size_t p = baseFileName.rfind(directorySeparator);
	if (p == string::npos)
	{
		baseDirName = "";
		baseSimName = baseFileName;
	}
	else
	{
		baseDirName = baseFileName.substr(0, p + 1);
		baseSimName = baseFileName.substr(p + 1);
	}

	pout() << "baseDirName=" << baseDirName << endl;
	pout() << "baseSimName=" << baseSimName << endl;
}

void SimTool::setPrimaryDataDir(string& pd)
{
	primaryDataDir = pd;
	if (!primaryDataDir.empty() && *primaryDataDir.rbegin() != directorySeparator)
	{
		primaryDataDir = primaryDataDir + directorySeparator;
	}
	pout() << "primaryDataDir=" << primaryDataDir << endl;
}

static void retryWait(int seconds) {
#if ( defined(WIN32) || defined(WIN64) )
	Sleep(seconds * 1000);
#else
	sleep(seconds);
#endif
}

static FILE* openFileWithRetry(const char* fileName, const char* mode) {
	static const char* METHOD = "(openFileWithRetry)";
	pout() << "Entry " << METHOD << endl;

	FILE *fp = NULL;
	for (int retry = 0; retry < numRetries; retry ++)
	{
		fp = fopen(fileName, mode);

		if (fp != NULL) {
			break;
		}
		if (retry < numRetries - 1) {
			cout << "SimTool, error opening log file <" << fileName << ">, trying again" << endl;
			retryWait(retryWaitSeconds);
		}
	}
	pout() << "Exit " << METHOD << endl;
	return fp;
}

static bool zipUnzipWithRetry(bool bZip, char* zipFileName, char* simFileName, char* errmsg) {
	const char* methodName = "(zipUnzipWithRetry)";
	pout() << "Entry " << methodName << endl;

	bool bSuccess = true;
	int retcode = 0;
	for (int retry = 0; retry < numRetries; retry ++) {
		try {
			if (bZip) {
				addFilesToZip(zipFileName, simFileName);
			} else {
				extractFileFromZip(zipFileName, simFileName);
			}
			bSuccess = true;
			break;
		} catch (const char* ziperr) {
			sprintf(errmsg, "%s", ziperr);
		} catch (...) {
			sprintf(errmsg, "%s, adding .sim to .zip failed.", methodName);
		}
		bSuccess = false;
		if (retry < numRetries - 1) {
			retryWait(retryWaitSeconds);
			pout() << methodName << ", adding .sim to .zip failed, trying again" << endl;
		}
	}
	if (bSuccess && retcode != 0) {
		sprintf(errmsg, "Writing zip file <%s> failed, return code is %d", zipFileName, retcode);
		bSuccess = false;
	}
	pout() << "Exit " << methodName << endl;
	return bSuccess;
}

FILE* SimTool::lockForReadWrite()
{
	const char* thisMethod = "(SimTool::lockForReadWrite)";
	pout() << "Entry " << thisMethod << endl;
	
	int myTaskID = SimulationMessaging::getInstVar()->getTaskID();
	if (myTaskID < 0) {
		return 0;
	}

	char tidFileName[FILE_NAME_BUFSIZE];
	sprintf(tidFileName,"%s%s", baseFileName.c_str(), TID_FILE_EXT);

	bool bExist = false;

	struct stat buf;
	if (stat(tidFileName, &buf) == 0) { // if exists
		bExist = true;
	}
	
	FILE* fp = openFileWithRetry(tidFileName, bExist ? "r+" : "w+");

	if (fp == 0){
		char errmsg[FILE_ERRMSG_BUFSIZE];
		sprintf(errmsg, "%s - error opening .tid file <%s>", thisMethod, tidFileName);
		// A copy: errmsg is a stack buffer, gone by the time main() catches this.
		throw std::string(errmsg);
	}
	if (bExist) {
		int taskIDInFile = 0;
		int numRead = fscanf(fp, "%d", &taskIDInFile);
		if (numRead == 1) {
			if (myTaskID < taskIDInFile) {
				pout() << thisMethod << "there is a new process running the simulation, exit..." << endl;
				exit(0);
			}
			if (myTaskID == taskIDInFile) { // it's me
				return fp;
			}
		}
		rewind(fp);
	}
	fprintf(fp, "%5d", myTaskID);
	fflush(fp);

	pout() << "Exit " << thisMethod << endl;
	return fp;
}

void SimTool::writeData(double progress, double time, int iteration, bool convertChomboData)
{
	const char* methodName = "(SimTool::writeData)";
	pout() << "Entry " << methodName << endl;	

#ifndef CH_MPI
	FILE *logFP;
	char logFileName[FILE_NAME_BUFSIZE];
	char zipHdf5FileName[FILE_NAME_BUFSIZE];

	FILE* tidFP = NULL;
	if (isRootRank())
	{
		tidFP = lockForReadWrite();
	}
#endif
	
	bool bSuccess = true;
	char errmsg[FILE_ERRMSG_BUFSIZE];
	// The .log and the zip entry name the .sim.hdf5 without a directory, but
	// the scratch copy is written next to the other results, not into the
	// current directory: that may not be writable (a read-only container, a
	// desktop launched from anywhere), and the results directory must be.
	char hdf5SimFileName[FILE_NAME_BUFSIZE];
	sprintf(hdf5SimFileName,"%s%.4d%s",baseSimName.c_str(), simFileCount, SIM_HDF5_FILE_EXT);
	char hdf5SimFilePath[FILE_NAME_BUFSIZE];
	sprintf(hdf5SimFilePath,"%s%s",baseDirName.c_str(), hdf5SimFileName);
	// write VCell and/or Chombo output
	simulation->getScheduler()->writeData(hdf5SimFilePath, convertChomboData);

#ifndef CH_MPI
	if (chomboSpec->isSaveVCellOutput())
	{
		sprintf(logFileName,"%s%s",baseFileName.c_str(), LOG_FILE_EXT);
		logFP = openFileWithRetry(logFileName, "a");

		if (logFP == 0) {
			sprintf(errmsg, "%s - error opening log file <%s>", methodName, logFileName);
			bSuccess = false;
		} else {
			sprintf(zipHdf5FileName,"%s%.2d%s",baseFileName.c_str(), zipFileCount, ZIP_HDF5_FILE_EXT);
			bSuccess = zipUnzipWithRetry(true, zipHdf5FileName, hdf5SimFilePath, errmsg);
			remove(hdf5SimFilePath);

			// write the log file
			if (bSuccess) {					
				char zipFileNameWithoutPath[FILE_NAME_BUFSIZE];
				sprintf(zipFileNameWithoutPath,"%s%.2d%s",baseSimName.c_str(), zipFileCount, ZIP_HDF5_FILE_EXT);
				fprintf(logFP,"%4d %s %s %.15lg\n", iteration, hdf5SimFileName, zipFileNameWithoutPath, time);

				struct stat buf;
				if (stat(zipHdf5FileName, &buf) == 0 || stat(zipHdf5FileName, &buf)) { // if exists
					if (buf.st_size > ZIP_FILE_LIMIT) {
						zipFileCount ++;
					}
				}
			}
		}
		// close log file
		fclose(logFP);
		// close tid file
		if (tidFP != 0) {
			fclose(tidFP);
		}
	}

	// write hdf5 post processing before writing log entry
	if (postProcessingHdf5Writer != NULL)
	{
		postProcessingHdf5Writer->writeOutput();
	}
#endif
	
	if (bSuccess) {
		if (bConsoleOutput || isRootRank())
		{
#ifndef CH_MPI
			SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_DATA, progress, time);
#endif
		}
		simFileCount++;
	} else {
		// A copy: errmsg is a stack buffer, gone by the time main() catches this.
		throw std::string(errmsg);
	}
	pout() << "Exit " << methodName << endl;
}

void SimTool::cleanupLastRun(bool convertChomboData)
{
	const char* thisMethod = "(cleanupLastRun)";
	pout() << "Entry " << thisMethod << ", removing log and zip files" << endl;

	simStartTime = 0;
	simFileCount = 0;
	zipFileCount = 0;

	if (isRootRank())
	{
		char buffer[FILE_NAME_BUFSIZE];
		if (!convertChomboData)
		{
			sprintf(buffer,"%s%s",baseFileName.c_str(), MESH_HDF5_FILE_EXT);
			remove(buffer);
		}
		for (int i = 0; i < 10; ++ i)
		{
			sprintf(buffer,"%s%02d%s", baseFileName.c_str(), i, ZIP_HDF5_FILE_EXT);
			remove(buffer);
		}
		sprintf(buffer,"%s%s",baseFileName.c_str(), LOG_FILE_EXT);
		remove(buffer);
		sprintf(buffer,"%s%s",baseFileName.c_str(), PP_HDF5_FILE_EXT);
		remove(buffer);
	}
	pout() << "Exit " << thisMethod << endl;
}

void SimTool::setSolver(string& s) {
	if (s.length() == 0 || s != CHOMBO_SEMIIMPLICIT_SOLVER) {
		stringstream ss;
		ss << "SimTool::setSolver(), unknown solver : " << s;
		throw ss.str();
	}
	solver = s;
}

void SimTool::start(bool convertChomboData)
{
	const char* thisMethod = "(start)";
	pout() << "Entry " << thisMethod << endl;

	if (convertChomboData)
	{
#ifdef CH_MPI
		throw "Chombo data conversion is not supported in MPI Parallel";
#else
		chomboSpec->setSaveChomboOutput(false);
		chomboSpec->setSaveVCellOutput(true);
#endif
	}

#ifndef CH_MPI
	if (simulation->getPostProcessingBlock() != NULL)
	{
		char h5PPFileName[FILE_NAME_BUFSIZE];
		sprintf(h5PPFileName, "%s%s", baseFileName.c_str(), PP_HDF5_FILE_EXT);
		postProcessingHdf5Writer = new PostProcessingHdf5Writer(h5PPFileName, simulation->getPostProcessingBlock());
	}
#endif
	
	simulation->initSimulation();
	// clean up last run results
	cleanupLastRun(convertChomboData);

	if (convertChomboData)
	{
		// we need to convert to primary data dir
		if (!primaryDataDir.empty() && baseDirName != primaryDataDir)
		{
		  baseDirName = primaryDataDir;
			baseFileName = primaryDataDir + baseSimName;
			pout() << thisMethod << "reset baseDirName=" << baseDirName << ", baseFileName=" << baseFileName << endl;
		}
	}

	if (checkStopRequested()) {
		return;
	}

	if (simulation == NULL) {
		throw "NULL simulation";
	}

	if (bStoreEnable && baseFileName.empty()) {
		throw "Invalid base file name for dataset";
	}

	char message[256];
	sprintf(message, "simulation [%s] started", baseSimName.c_str());
	if (bConsoleOutput || isRootRank())
	{
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_STARTING, message);
	}

	//
	// destroy any partial results from unfinished iterations
	//
	double percentile = simStartTime/simEndTime;
	double increment = 0.01;
	double lastSentPercentile = percentile;
	//
	// store initial log if enabled
	//
	if (simulation->getCurrIteration()==0)
	{
		// simulation starts from scratch
		if (bStoreEnable)
		{
			if (!convertChomboData)
			{
				simulation->getScheduler()->writeMembraneFiles();
			}
			writeData(0.0, 0.0, 0, convertChomboData);
		}
		else
		{
			if (bConsoleOutput || isRootRank())
			{
#ifndef CH_MPI
				SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_DATA, 0, 0);
#endif
			}
		}
	} else {
		// simulation continues from existing results, send data message
		if (bConsoleOutput || isRootRank())
		{
#ifndef CH_MPI
			SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_DATA, percentile, simStartTime);
#endif
		}
	}
	//
	// iterate up to but not including end time
	//
	double epsilon = 1e-12;

	while (true) {
		if (simulation->getTime_sec() + simulation->getDT_sec() > simEndTime + epsilon) {
			break;
		}

		if (checkStopRequested()) {
			return;
		}

#ifdef CH_MPI
		if (MPI_Barrier(MPI_COMM_WORLD) != MPI_SUCCESS) {
			throw std::invalid_argument("barrier didn't accept MPI_COMM_WORLD"); 
		}
#endif
		
		simulation->iterate(convertChomboData ? false : true);

		if (checkStopRequested()) {
			return;
		}

		if (simulation->shouldSave() || simulation->getTime_sec() > simEndTime - epsilon){
			if (bStoreEnable){
				writeData(percentile,simulation->getTime_sec(), simulation->getCurrIteration(), convertChomboData);
      }
    }
		percentile = (simulation->getTime_sec() - simStartTime)/(simEndTime - simStartTime);
		if (percentile - lastSentPercentile >= increment) {
			if (bConsoleOutput || isRootRank())
			{
				SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_PROGRESS, percentile, simulation->getTime_sec());
			}
			lastSentPercentile = percentile;
		}
	}

	if (checkStopRequested()) {
		return;
	} 
	if (bConsoleOutput || isRootRank())
	{
#ifdef CH_MPI
		copyToPrimaryDataDir();
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_DATA, 1.0, simulation->getTime_sec());
#endif
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_PROGRESS, 1.0, simulation->getTime_sec());
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_COMPLETED, percentile, simulation->getTime_sec());
	}
	pout() << "Exit " << thisMethod << endl;
}

void SimTool::copyToPrimaryDataDir()   // no directory information
{
	const char* thisMethod = "(copyToPrimaryDataDir)";
	pout() << "Entry " << thisMethod << endl;

	if (!primaryDataDir.empty() && baseDirName != primaryDataDir)
	{
		{
		stringstream cpcmd;
		cpcmd << "cp " << baseFileName << "* " << primaryDataDir;
		pout() << thisMethod << "Running copy command: " << cpcmd.str() << endl;
		system(cpcmd.str().c_str());
		}
		{
		// delete the local files
		stringstream rmcmd;
		rmcmd << "rm -f " << baseFileName << "*";
		pout() << thisMethod << "Running rm command: " << rmcmd.str() << endl;
		system(rmcmd.str().c_str());
		}
	}
	pout() << "Exit " << thisMethod << endl;
}

