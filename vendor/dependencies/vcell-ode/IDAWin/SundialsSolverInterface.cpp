//
// Created by Logan Drescher on 11/12/25.
//
// Messaging
#include "SundialsSolverInterface.h"
#include "VCellSundialsSolver.h"
#include "VCellSolverFactory.h"

// included referenced in commented out code:
#include <iomanip>
#include <sstream>
#include <VCELL/GitDescribe.h>
#include <VCELL/SimulationMessaging.h>
#include "StoppedByUserException.h"

#define CVODE_SOLVER "CVODE"
#define IDA_SOLVER "IDA"

void errExit(int returnCode, const std::string &errorMsg);

std::string version() {
	return "VCell ODE solver (CVODE/IDA) v" + std::string(g_GIT_DESCRIBE);
}

int solve(const std::string& inputFilePath, const std::string& outputFilePath, const int taskID) {
	int returnCode = 0;
	FILE *outputFile = NULL;
	std::ifstream inputFileStream{inputFilePath};
	try {
		if (!inputFileStream.is_open()) { throw std::runtime_error("input file [" + inputFilePath + "] doesn't exit!"); }

		// Open the output file...
		if ((outputFile = fopen(outputFilePath.c_str(), "w")) == NULL) {
			throw std::runtime_error("Could not open output file[" + outputFilePath + "] for writing.");
		}
		returnCode = activateSolver(inputFileStream, outputFile, taskID);
	} catch (const std::runtime_error& err) {
		std::cerr << err.what() << std::endl;
		returnCode = 5;
	} catch (...) {
		std::cerr << "Unknown exception thrown." << std::endl;
		returnCode = 255;
	}

	if (outputFile != NULL) { fclose(outputFile); }
	if (inputFileStream.is_open()) { inputFileStream.close(); }
	return returnCode;
}


int activateSolver(std::ifstream& inputFileStream, FILE* outputFile, int taskID) {
	int returnCode = 0;
	std::string errorMsg;
	VCellSolver* targetSolver;
	// First try block - create the solver; failure => no need to delete targetSolver!
	try {
		targetSolver = VCellSolverFactory::produceVCellSolver(inputFileStream, taskID);
	} catch (const char *ex) {
		returnCode = -1;
		errorMsg = ex;
		targetSolver = nullptr;
	} catch (std::string &ex) {
		returnCode = -2;
		errorMsg = ex;
		targetSolver = nullptr;
	} catch (StoppedByUserException&) {
		returnCode = 0;
		errorMsg = "Execution Stopped By User";
		targetSolver = nullptr;
	} catch (VCell::Exception &ex) {
		returnCode = -3;
		errorMsg = ex.getMessage();
		targetSolver = nullptr;
	} catch (const std::exception& err) {
		returnCode = -4;
		errorMsg = err.what();
		targetSolver = nullptr;
	} catch (...) {
		returnCode = -5;
		errorMsg = "Unknown Error Detected";
		targetSolver = nullptr;
	}
	// second try block - solver is created; must delete it!
	if (nullptr != targetSolver) {
		try {
			targetSolver->solve(nullptr, true, outputFile, VCellSundialsSolver::checkStopRequested);
		} catch (const char *ex) {
			returnCode = -1;
			errorMsg = ex;
			// errExit(-1, ex);
		} catch (std::string &ex) {
			returnCode = -2;
			errorMsg = ex;
		} catch (StoppedByUserException&) {
			returnCode = 0;
			errorMsg = "Execution Stopped By User";
		} catch (VCell::Exception &ex) {
			returnCode = -3;
			errorMsg = ex.getMessage();
		} catch (const std::exception& err) {
			returnCode = -4;
			errorMsg = err.what();
		} catch (...) {
			returnCode = -5;
			errorMsg = "Unknown Error Detected";
		}
		// !!! DELETE THE SOLVER !!!
		delete targetSolver;
	}

	// Report a failure while the messaging instance (and its broker connection) still exists, then
	// clean up; the other order sent JOB_FAILURE to a fresh stdout-only instance, never the broker.
	if (!errorMsg.empty()) errExit(returnCode, errorMsg);
	SimulationMessaging::cleanupInstanceVar();
	return returnCode; // non-zero on failure, as the legacy vcell-solvers binary returned from main()
}

void errExit(int returnCode, const std::string &errorMsg) {
	#ifdef USE_MESSAGING
	if (returnCode && SimulationMessaging::getInstVar() && !SimulationMessaging::getInstVar()->isStopRequested()) {
		SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_FAILURE, errorMsg.c_str());
	}
	#endif
	std::cerr << errorMsg << std::endl;
}

// C-bound functions
extern "C" const char* version_ctypes() {
	static std::string originalVersion = version();
	return originalVersion.c_str();
}

extern "C" int solve_ctypes(const char* inputFilePath, const char* outputFilePath, const int taskID) {
	const std::string ifp{inputFilePath};
	const std::string ofp{outputFilePath};
	const int returnCode = solve(ifp, ofp, taskID);
	return returnCode;
}

