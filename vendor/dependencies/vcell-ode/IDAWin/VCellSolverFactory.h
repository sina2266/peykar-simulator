//
// Created by Logan Drescher on 6/26/25.
//

#ifndef VCELLSOLVERFACTORY_H
#define VCELLSOLVERFACTORY_H
#include <fstream>
#include <string>
#include "VCellSolver.h"
#include "VCellSolverInput.h"




class VCellSolverFactory {
	public:
		static VCellSolver* produceVCellSolver(std::ifstream& inputFileStream, int taskID);
		static VCellSolverInputBreakdown parseInputFile(std::ifstream& inputFileStream, int taskID);
	private:
		static VCellSolverTypes determineSolverType(const std::string& solverName);
		static void processEquations(std::ifstream& inputFileStream, VCellSolverInputBreakdown& inputBreakdown);
};



#endif //VCELLSOLVERFACTORY_H
