#include <stdexcept>
#include <iostream>
#include <cstdio>
#include <sstream>
#include <map>
#include <fstream>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include "Gibson.h"
#include <hdf5.h>
#include <vector>

#define TEST_ASSERT(condition, message) \
    if (!(condition)) { \
        std::cerr << "FAIL: " << (message) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define TEST_LT(actual, expected, message) \
    if (!((actual) < (expected))) { \
        std::cerr << "FAIL: " << (message) << " - expected < " << (expected) << " got " << (actual) \
                  << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

const char* input_file_contents = R"INPUT_FILE(
<control>
STARTING_TIME	0.0
ENDING_TIME 	20.0
TOLERANCE 	1.0E-9
SAVE_PERIOD	0.02
MAX_SAVE_POINTS	1001.0
NUM_TRIAL	50000
SEED	1634997497
BMULTIBUTNOTHISTO	1
</control>

<model>
<discreteVariables>
TotalVars	3
s0_Count	1
s1_Count	0
s2_Count	1
</discreteVariables>

<jumpProcesses>
TotalProcesses	2
r0
r0_reverse
</jumpProcesses>

<processDesc>
TotalDescriptions	2
JumpProcess	r0
	Propensity	(0.9996443474639611 * s0_Count * s2_Count)
	Effect	3
		s0_Count	inc	-1.0

		s2_Count	inc	-1.0

		s1_Count	inc	1.0

	DependentProcesses	0

JumpProcess	r0_reverse
	Propensity	0.0
	Effect	3
		s0_Count	inc	1.0

		s2_Count	inc	1.0

		s1_Count	inc	-1.0

	DependentProcesses	1
		r0

</processDesc>
</model>
)INPUT_FILE";

const std::map<int, double> expected_S0 = {
    {0, 1.0},       // t=0.0
    {50, 0.36868},  // t=1.0
    {100, 0.1349},  // t=2.0
    {150, 0.04972}, // t=3.0
    {200, 0.0187},  // t=4.0
    {250, 0.00678}, // t=5.0
    {300, 0.00234}, // t=6.0
    {350, 7.6E-4},  // t=7.0
    {400, 2.8E-4},  // t=8.0
    {450, 8.0E-5},  // t=9.0
    {500, 2.0E-5},  // t=10.0
    {550, 2.0E-5},  // t=11.0
    {600, 0.0},     // t=12.0
    {650, 0.0},     // t=13.0
    {700, 0.0},     // t=14.0
    {750, 0.0},     // t=15.0
    {800, 0.0},     // t=16.0
    {850, 0.0},     // t=17.0
    {900, 0.0},     // t=18.0
    {950, 0.0},     // t=19.0
    {1000, 0.0},    // t=20.0
};

bool test_statstest_test1() {
    std::cout << "Running statstest::test1..." << std::endl;

    std::string inputFileName = std::tmpnam(nullptr);
    std::string outputFileName = std::tmpnam(nullptr);
    std::map<int, double> results;
    std::fstream inputFileStream;

    inputFileStream.open(inputFileName, std::fstream::out);
    TEST_ASSERT(!inputFileStream.fail(), "Input file creation");

    std::fstream outputFileStream;
    outputFileStream.open(outputFileName, std::fstream::out);
    TEST_ASSERT(!outputFileStream.fail(), "Output file creation");

    if (outputFileStream.is_open()) outputFileStream.close();
    inputFileStream << input_file_contents;
    inputFileStream.close();

    auto *gb = new Gibson(inputFileName.c_str(), outputFileName.c_str());
    gb->march();

    outputFileStream.open(outputFileName, std::fstream::in);
    std::string line;
    std::getline(outputFileStream, line);
    for (int i = 0; !outputFileStream.eof(); i++) {
        std::getline(outputFileStream, line);
        if (expected_S0.find(i) != expected_S0.end()) {
            float t, s0, s1, s2;
            std::stringstream line_stream(line);
            line_stream >> t >> s0 >> s1 >> s2;
            results[i] = s0;
        }
    }
    outputFileStream.close();

    double accumulatedError = 0.0, maxIndividualError = 0.0;
    for (auto const& expected : expected_S0) {
        double absoluteError = std::abs(expected.second - results[expected.first]);
        accumulatedError += absoluteError;
        maxIndividualError = std::max(maxIndividualError, absoluteError);
    }

    TEST_LT(accumulatedError, 0.015, "Accumulated error");
    TEST_LT(maxIndividualError, 0.005, "Max individual error");

    // The multi-trial statistics file VCell reads (<output>_hdf5): the layout, and its means
    // must be the ones in the text output.
    {
        std::string h5name = outputFileName + "_hdf5";
        hid_t file = H5Fopen(h5name.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
        TEST_ASSERT(file >= 0, "open " + h5name);
        const char* names[5] = { "SimTimes", "StatMean", "StatMin", "StatMax", "StatStdDev" };
        for (const char* name : names) {
            hid_t dset = H5Dopen2(file, name, H5P_DEFAULT);
            TEST_ASSERT(dset >= 0, std::string("dataset ") + name);
            hid_t space = H5Dget_space(dset);
            hsize_t dims[2] = { 0, 0 };
            int rank = H5Sget_simple_extent_dims(space, dims, NULL);
            TEST_ASSERT(dims[0] == 1001, std::string(name) + " has 1001 time points");
            TEST_ASSERT(rank == (std::string(name) == "SimTimes" ? 1 : 2), std::string(name) + " rank");
            if (rank == 2) TEST_ASSERT(dims[1] == 3, std::string(name) + " has 3 variables");
            if (std::string(name) == "StatMean") {
                std::vector<double> mean(1001 * 3);
                TEST_ASSERT(H5Dread(dset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, mean.data()) >= 0, "read StatMean");
                for (auto const& r : results) {
                    TEST_LT(std::abs(mean[r.first * 3] - r.second), 1e-6, "StatMean s0 = text output");
                }
            }
            H5Sclose(space);
            H5Dclose(dset);
        }
        hid_t vn = H5Dopen2(file, "VarNames", H5P_DEFAULT);
        TEST_ASSERT(vn >= 0, "dataset VarNames");
        H5Dclose(vn);
        H5Fclose(file);
        std::remove(h5name.c_str());
    }

    delete gb;
    if (inputFileStream.is_open()) inputFileStream.close();
    if (outputFileStream.is_open()) outputFileStream.close();

    std::cout << "PASS: statstest::test1" << std::endl;
    return true;
}