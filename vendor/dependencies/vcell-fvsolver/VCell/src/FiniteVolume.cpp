#if (defined(WIN32) || defined(WIN64) )
#define _HAS_STD_BYTE 0
#include <windows.h>
#endif

#include <iostream>
#include <fstream>
#include <string>

#include <VCELL/FVSolver.h>
#include <sys/stat.h>
#include <VCELL/SimTool.h>
#include <VCELL/SimulationMessaging.h>
#include <VCELL/GitDescribe.h>
#include <Exception.h>
#include <vcellhybrid.h>

void vcellExit(int returnCode, std::string& errorMsg) {
	if (SimulationMessaging::getInstVar() == 0) {
		if (returnCode != 0) {
			std::cerr << errorMsg << std::endl;
		}
	} else if (!SimulationMessaging::getInstVar()->isStopRequested()) {
		if (returnCode != 0) {
			SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_FAILURE, errorMsg.c_str()));
		}
#ifdef USE_MESSAGING
		SimulationMessaging::getInstVar()->waitUntilFinished();
#endif
	}
}

void printUsage() {
#ifdef USE_MESSAGING
	cout << "Arguments : [-d output] [-nz] [-tid taskID] fvInputFile" <<  endl;
#else
	std::cout << "Arguments : [-d output] [-nz] fvInputFile vcgInputFile" <<  std::endl;
#endif
}

int main(int argc, char *argv[])
{
	std::cout
	    << "Finite Volume version " << g_GIT_DESCRIBE << " with smoldyn version " << VERSION
		<< std::endl;

	vcellhybrid::setHybrid(); //get smoldyn library in correct state
  	int returnCode = 0;
	std::string errorMsg = "Exception : ";

	std::string outputPathStr{};
	std::string fvInputFileStr{};
	std::string vcgInputFileStr{};

	std::ifstream ifsInput;
	std::ifstream vcgInput;
	FVSolver* fvSolver = nullptr;

	bool bSimZip = true;
	try {
		int taskID = -1;
		if (argc < 2) {
			std::cout << "Missing arguments!" << std::endl;
			printUsage();
			exit(1);
		}
		for (int i = 1; i < argc; i ++) {
			if (!strcmp(argv[i], "-nz")) {
				bSimZip = false;
			} else if (!strcmp(argv[i], "-d")) {
				i++;
				if (i >= argc) {
					std::cout << "Missing output directory!" << std::endl;
					printUsage();
					exit(1);
				}
				outputPathStr = argv[i];
			} else if (!strcmp(argv[i], "-tid")) {
#ifdef USE_MESSAGING
				i ++;
				if (i >= argc) {
					cout << "Missing taskID!" << endl;
					printUsage();
					exit(1);
				}
				for (int j = 0; j < (int)strlen(argv[i]); j ++) {
					if (argv[i][j] < '0' || argv[i][j] > '9') {
						cout << "Wrong argument : " << argv[i] << ", taskID must be an integer!" << endl;
						printUsage();
						exit(1);
					}
				}
				taskID = atoi(argv[i]);
#else
				std::cout << "Wrong argument : " << argv[i] << std::endl;
				printUsage();
				exit(1);
#endif
			} else if (fvInputFileStr.empty()) {
				fvInputFileStr = argv[i];
			} else {
				vcgInputFileStr = argv[i];
			}
		}

		std::filesystem::path fvInputFilePath = outputPathStr;
		if (!outputPathStr.empty() && !std::filesystem::exists(fvInputFilePath)) {
			std::cerr << "Output directory [" << outputPathStr <<"] doesn't exist" << std::endl;
			exit(1);
		}

		// strip " in case that file name has " around
		size_t fl = fvInputFileStr.length();
		if (fvInputFileStr[0] == '"' && fvInputFileStr[fl-1] == '"') {
			fvInputFileStr = fvInputFileStr.substr(1, fl-2);
		}
		ifsInput.open(fvInputFileStr);
		if (!ifsInput.is_open()) {
			std::cout << "FV Input File doesn't exist: " << fvInputFileStr << std::endl;
			exit(102);
		}

		// If we're not provided the location of the vcg, let's try and deduce it.
		if (vcgInputFileStr.empty())
			vcgInputFileStr = fvInputFileStr.substr(0, fvInputFileStr.find_last_of('.')) + ".vcg";

		// strip " in case that file name has " around
		fl = vcgInputFileStr.length();
		if (vcgInputFileStr[0] == '"' && vcgInputFileStr[fl-1] == '"') {
			vcgInputFileStr = vcgInputFileStr.substr(1, fl-2);
		}
		vcgInput.open(vcgInputFileStr);
		if (!vcgInput.is_open()) {
			std::cout << "VCG Input File doesn't exist: " << vcgInputFileStr << std::endl;
			exit(102);
		}

		fvSolver = new FVSolver(outputPathStr.empty() ? nullptr : outputPathStr.c_str());
		SimTool* sim_tool = fvSolver->createSimTool(ifsInput, vcgInput, taskID, bSimZip);
		ifsInput.close();
		vcgInput.close();

		if(FVSolver::getNumVariables(sim_tool) == 0){
			//sims with no reactions and no diffusing species cause exit logic to 'wait' forever
			//never sending a job failed or job finished message and never cleaning up threads
			throw invalid_argument("FiniteVolume error: Must have at least 1 variable or reaction to solve");
		}
		FVSolver::solve(sim_tool);

	} catch (const char *exStr){
		errorMsg += exStr;
		returnCode = 1;
	} catch (string& exStr){
		errorMsg += exStr;
		returnCode = 1;
	} catch (VCell::Exception& ex){
		errorMsg += ex.getMessage();
		returnCode = 1;
	} catch (std::exception & e) {
		errorMsg += e.what(); 
		returnCode = 1;
	} catch (...){
		errorMsg += "unknown error";
		returnCode = 1;
	}

	if (ifsInput.is_open()) {
		ifsInput.close();
	}
	vcellExit(returnCode, errorMsg);
	delete fvSolver;
	return returnCode;
}
