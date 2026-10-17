//
// Created by Logan Drescher on 11/12/25.
//
#include <algorithm>
#include <array>
#include <filesystem>
#include <format>
#include <fstream>
#include <cmath>
#include <sstream>
#include <string>
#include <vector>
#include <gtest/gtest.h>

#include "SundialsSolverInterface.h"

void compare(const std::filesystem::path& file1, const std::filesystem::path& file2, float tolerance);

void performSmokeTest(const int taskID, const int hashID) {
	const std::filesystem::path RESOURCE_DIRECTORY{RESOURCE_DIR};
	const std::filesystem::path OUTPUT_TARGET{RESOURCE_DIRECTORY /std::format("SimID_{}_0_.ida", hashID)};
	const std::array NECESSARY_FILES{
		RESOURCE_DIRECTORY /std::format("SimID_{}_0_.cvodeInput", hashID),
		RESOURCE_DIRECTORY /std::format("SimID_{}_0_.ida.expected", hashID)
	};
	for (const auto& file : NECESSARY_FILES) {
		assert(std::filesystem::exists(file));
	}
	FILE *outputFile = NULL;
	std::ifstream inputFileStream{NECESSARY_FILES[0]};
	if (!inputFileStream.is_open()) { throw std::runtime_error("input file [" + NECESSARY_FILES[0].string() + "] doesn't exit!"); }

	// Open the output file...
	if (NULL == (outputFile = fopen(OUTPUT_TARGET.string().c_str(), "w"))) {
		throw std::runtime_error("Could not open output file[" + OUTPUT_TARGET.string() + "] for writing.");
	}

	activateSolver(inputFileStream, outputFile, taskID);
	fclose(outputFile);

	compare(OUTPUT_TARGET, NECESSARY_FILES[1], 1e-7);
}

TEST(SmokeTest, UserProvidesFilesWithoutJMS) {
	constexpr int taskID = -1, hashID = 1489333437;
	performSmokeTest(taskID, hashID);
}

/**
 * To perform localhost smoke test, run the following docker command:
 * `docker run -p 61616:61616 -p 8161:8161 apache/activemq-classic`
 * Then connect to http://localhost:8161, and login as `admin` pass `admin`
 */
TEST(SmokeTest, UserProvidesFilesWithJMS) {
	constexpr int taskID = 2025;
	#ifdef TEST_WITH_LOCALHOST
	constexpr int hashID = 886118677;
	#else
	constexpr int hashID = 256118677;
	#endif

	performSmokeTest(taskID, hashID);
}

void compare(const std::filesystem::path& file1, const std::filesystem::path& file2, float tolerance) {
	std::ifstream fileStream1(file1);
	std::ifstream fileStream2(file2);
	if (!fileStream1.is_open()) throw std::runtime_error("Could not file: `" + file1.string() + "`");
	if (!fileStream2.is_open()) throw std::runtime_error("Could not file: `" + file2.string() + "`");

	std::string line1, line2;
	bool eof1, eof2;
	// we do **NOT** short circuit here, so we use `&` not `&&`
	while (
			!((eof1 = !std::getline(fileStream1, line1)))
			&
			!((eof2 = !std::getline(fileStream2, line2)))
	){
		if (line1 == line2) continue;
		double value;
		std::istringstream lineStream1{line1};
		std::vector<double> lineValues1;
		while (lineStream1 >> value) lineValues1.push_back(value);
		std::istringstream lineStream2{line2};
		std::vector<double> lineValues2;
		while (lineStream2 >> value) lineValues2.push_back(value);
		if (lineValues1.size() != lineValues2.size()) throw std::runtime_error("Length of data does not match");

		for (int i = 0; i < lineValues2.size(); i++) {
			if (lineValues1[i] == lineValues2[i]) continue;
			double adjustedTolerance = tolerance * std::max(std::abs(lineValues1[i]), std::abs(lineValues2[i]));
			if (std::abs(lineValues1[i] - lineValues2[i]) > adjustedTolerance)
				throw std::runtime_error("Values outside tolerance");
		}
	}

	// last check; make sure they are both eof!
	if (eof1 ^ eof2) {
		throw std::runtime_error("Files do not contain same number of lines.");
	}
}