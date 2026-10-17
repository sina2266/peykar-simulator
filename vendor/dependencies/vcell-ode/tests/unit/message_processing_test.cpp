//
// Created by Logan Drescher on 12/1/25.
//
#include <string>
#include <vector>
#include <exception>
#include <gtest/gtest.h>
#include <VCELL/MessageEventManager.h>

#include "VCellSundialsSolver.h"

static std::vector<std::string> eventTracker;

void appendNextIndex(const WorkerEvent* event);
std::vector<std::string> generateFibonacci(int length);
long long sequenceComparison(const std::vector<std::string> &sequence1, const std::vector<std::string> &sequence2);


TEST(MessageProcessingTest, MessagesAreProcessed) {
	try {
		MessageEventManager eventManager{[](WorkerEvent* event)->void{appendNextIndex(event);}};
		const int NUM_ITERATIONS = 50;
		for (int i = 0; i < NUM_ITERATIONS; ++i) {
			JobEvent::Status status;
			switch (i + 1) {
				case 1: status = JobEvent::JOB_STARTING; break;
				case NUM_ITERATIONS: status = JobEvent::JOB_COMPLETED; break;
				default: status = JobEvent::JOB_PROGRESS; break;
			}
			eventManager.enqueue(status, (i + 1) / 100.0, i, std::to_string(i).c_str());
		}
		eventManager.requestStopAndWaitForIt();
		std::vector<std::string> expectedResults = generateFibonacci(NUM_ITERATIONS);
		// delete eventTrackerMutex;
		ASSERT_TRUE(!sequenceComparison(eventTracker, expectedResults));
	} catch (const std::exception& e) {
		std::cerr << "Caught exception in main test body: " << e.what() << std::endl;
	}
}

void appendNextIndex(const WorkerEvent* event) {
	try {
		if (eventTracker.empty()) {
			eventTracker.emplace_back("0");
		} else if (1 == eventTracker.size()) {
			eventTracker.emplace_back("1");
		} else {
			const long long firstValue = std::stoll(eventTracker[eventTracker.size() - 2]);
			const long long secondValue = std::stoll(eventTracker[eventTracker.size() - 1]);
			const std::string nextValue{std::to_string(firstValue + secondValue)};
			eventTracker.push_back(nextValue);
		}
	} catch (const std::exception& e) {
		std::cerr << "Caught exception in event loop: " << e.what() << std::endl;
		exit(2);
	}
}

std::vector<std::string> generateFibonacci(const int length) {
	std::vector<std::string> sequence;
	for (int i = 0; i < length; ++i) {
		if (i == 0) { sequence.emplace_back("0"); continue; }
		if (i == 1) { sequence.emplace_back("1"); continue; }
		const long long firstValue = std::stoll(sequence[sequence.size() - 2]);
		const long long secondValue = std::stoll(sequence[sequence.size() - 1]);
		const std::string nextValue{std::to_string(firstValue + secondValue)};
		sequence.push_back(nextValue);
	}
	return sequence;
}

long long sequenceComparison(const std::vector<std::string> &sequence1, const std::vector<std::string> &sequence2) {
	if (const size_t comp = sequence2.size() - sequence1.size(); comp) return static_cast<int>(comp);
	for (size_t i = 0; i < sequence1.size(); ++i) {
		const long long firstValue = std::stoll(sequence1[i]);
		const long long secondValue = std::stoll(sequence2[i]);
		if (const long long comp = secondValue - firstValue; comp) return comp;
	}
	return 0;
}