//
// Created by Jim Schaff on 6/1/24.
//
#if (defined(WIN32) || defined(WIN64) )
#define _HAS_STD_BYTE 0
#include <windows.h> 
#endif

#include "../include/VCELL/SolverMain.h"

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
using namespace std;

#undef USE_MESSAGING

#include <../include/VCELL/FVSolver.h>
#include <sys/stat.h>
#include <../include/VCELL/SimTool.h>
#include <../../VCellMessaging/include/VCELL/SimulationMessaging.h>
#include <../../VCellMessaging/include/VCELL/GitDescribe.h>
#include <../../ExpressionParser/Exception.h>
#include <../../bridgeVCellSmoldyn/vcellhybrid.h>



void vcellExit(int returnCode, string& errorMsg) {
	if (returnCode != 0) {
		cerr << errorMsg << endl;
	}
}

std::string version()
{
	return "Finite Volume version " + std::string(g_GIT_DESCRIBE) + " with smoldyn version " + std::string(VERSION);
}

int solve(const std::string& inputFilename, const std::string& vcgFilename, const std::string& outputDir) {
	// Check if output directory exists, if not create it
	std::filesystem::path dirPath(outputDir);
	// converty dirPath to an absolute path and check to see if dirPath exists
	dirPath = std::filesystem::absolute(dirPath);
	if (!std::filesystem::exists(dirPath)) {
		// create the directory
		std::filesystem::create_directories(dirPath);
	}

	// Open the input file
	std::ifstream inputFile(inputFilename);
	if (!inputFile.is_open()) {
		throw std::runtime_error("Could not open input file: " + inputFilename);
	}

	// Open the vcg file
	std::ifstream vcgFile(vcgFilename);
	if (!vcgFile.is_open()) {
		throw std::runtime_error("Could not open vcg file: " + vcgFilename);
	}

	vcellhybrid::setHybrid(); //get smoldyn library in correct state
  	int returnCode = 0;
	string errorMsg = "Exception : ";

	bool bSimZip = true;
	int taskID = 0;

	if (SimulationMessaging::getInstVar() == nullptr)
	{
		SimulationMessaging::create();
	}

	FVSolver* fvSolver = nullptr;
	SimTool* sim_tool = nullptr;

	try {
		fvSolver = new FVSolver(outputDir.c_str());
		sim_tool = fvSolver->createSimTool(inputFile, vcgFile, taskID, bSimZip);

		inputFile.close();

		if (FVSolver::getNumVariables(sim_tool) == 0){
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

	if (inputFile.is_open()) {
		inputFile.close();
	}
	vcellExit(returnCode, errorMsg);
	if (sim_tool != nullptr) {
		delete sim_tool;
	}
	if (fvSolver != nullptr) {
		delete fvSolver;
	}
	return returnCode;
}
