//
// Created by Logan Drescher on 11/12/25.
//
#ifndef VCELL_ODE_NUMERICS_SUNDIALSSOLVERINTERFACE_H
#define VCELL_ODE_NUMERICS_SUNDIALSSOLVERINTERFACE_H

#include <fstream>

// We need to explicitly declare functions as exposed for Windows...fun
#if defined(_WIN32)
  #if defined(IDAWIN_BUILDING_DLL)
	#define IDAWIN_API __declspec(dllexport)
  #else
	#define IDAWIN_API __declspec(dllimport)
  #endif
#else
  #define IDAWIN_API __attribute__((visibility("default")))
#endif

IDAWIN_API std::string version();

IDAWIN_API int solve(const std::string& inputFilePath, const std::string& outputFilePath, const int taskID);

IDAWIN_API int activateSolver(std::ifstream& inputFileStream, FILE* outputFile, int taskID);

// C-bound functions
extern "C" IDAWIN_API const char* version_ctypes();

extern "C" IDAWIN_API int solve_ctypes(const char* inputFilePath, const char* outputFilePath, const int taskID);

#endif //VCELL_ODE_NUMERICS_SUNDIALSSOLVERINTERFACE_H