//
// Created by Jim Schaff on 6/1/24.
//

#ifndef SOLVERMAIN_H
#define SOLVERMAIN_H

#include <string>

std::string version();
int solve(const std::string& fvInputFilename, const std::string& vcgInputFilename, const std::string& outputDir);

#endif //SOLVERMAIN_H
