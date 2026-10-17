#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cassert>
#include <VCELL/SimulationMessaging.h>
#include "NFMonitor.h"
using vcell_nfsim::NFMonitor;
namespace {
	const char newline = '\n';
	const char startToken[] = "simulating system for: "; //must use [] to use sizeof pattern below
	const size_t startTokenLen = sizeof(startToken) / sizeof(startToken[0]) - 1; // -1 to account for null
	const char timeToken[] = "Sim time: ";
	const size_t timeTokenLen = sizeof(timeToken) / sizeof(timeToken[0]) - 1;
	const double placeholderSimTime = 1000.7639;
	const char successToken[] = "You just simulated";
	const char failToken[] = { "fail"};
}

/**
 * deactivate monitor temporarily
 */
struct NFMonitor::Suppress {
	Suppress(NFMonitor &m)
			:monitor(m) {
		monitor.active = false;
	}
	~Suppress() {
		monitor.active = true;
	}
	NFMonitor & monitor;

};

/**
 * do not use std::cout in this class; when connected via vcell_util::OstreamSpy it would  recursively call itself
 */

NFMonitor::NFMonitor(bool networkMessaging_)
		:networkMessaging(networkMessaging_),
		buffer(),
		errBuffer(),
		cursor(0),
		errCursor(0),
		active(true),
		startOfTimeData(buffer + timeTokenLen),
		totalSimTime(placeholderSimTime),
		lastUpdate( 0),
		lastFraction(0),
		checkSimTime(true),
		endMessageReceived( false),
		failStream(),
		exitValue(999){
}

void NFMonitor::reportStart() {
	Suppress  suppressor(*this);
	SimulationMessaging & sm = *SimulationMessaging::getInstVar();
	WorkerEvent * const se = new WorkerEvent(JOB_STARTING, "NFsim preprocessing started");
	sm.setWorkerEvent(se);
	lastUpdate = clock();
}

void NFMonitor::reportComplete() {
	Suppress  suppressor(*this);
	SimulationMessaging & sm = *SimulationMessaging::getInstVar();
	if (endMessageReceived) {
		sm.setWorkerEvent(new WorkerEvent(JOB_PROGRESS, 1.0, totalSimTime));
		sm.setWorkerEvent(new WorkerEvent(JOB_COMPLETED, 1.0, totalSimTime));
		exitValue = 0;
		return;
	}
	if (networkMessaging) {
		failStream << std::ends; //null-terminate
		std::string failMessage = failStream.str();
		sm.setWorkerEvent(new WorkerEvent(JOB_FAILURE, failMessage.c_str()));
	}
	exitValue = -1;
}

inline NFMonitor::timeUnit NFMonitor::timeDiff(clock_t end, clock_t start) {
	//assert(end > start);
	return (end - start) / CLOCKS_PER_SEC;
}

inline void NFMonitor::parseStartToken() {
	const char * st = buffer + startTokenLen;
	totalSimTime = std::strtod(st, 0);
}

inline void NFMonitor::parseTimeToken() {
	double simTime = std::strtod(startOfTimeData, 0);
	if (checkSimTime) {
		if (totalSimTime == placeholderSimTime) {
			std::cerr << "WARNING, sim time message not received " << __FILE__
					<< " " << __LINE__ << std::endl;
		}
		checkSimTime = false;
	}
	clock_t now = clock();
	const timeUnit delta = timeDiff(now, lastUpdate);
	if (delta >= UPDATE_SECONDS) {
		double fraction = simTime / totalSimTime;
		if (fraction > lastFraction) {
			Suppress  suppressor(*this);
			//std::cerr  << std::endl <<  "elapsed " << delta << " prog " << simTime << " tot " << totalSimTime << " frac " << fraction << std::endl;
			SimulationMessaging::getInstVar()->setWorkerEvent(
					new WorkerEvent(JOB_PROGRESS, fraction, simTime));
			lastFraction = fraction;
			lastUpdate = now;
		}
	}
}

inline void NFMonitor::parseFailToken() {
	if (!endMessageReceived) {
		failStream << buffer;
		return;
	}
	std::cerr << "WARNING, fail after finish token" << __FILE__ << " "
			<< __LINE__ << std::endl;
}

inline void NFMonitor::endOfLine() {
	cursor = 0;
	active = true;
	if (strstr(buffer, timeToken) != 0) { //occurs more often, check first
		parseTimeToken();
	} else if (strstr(buffer, startToken) != 0) {
		parseStartToken();
	} else if (strstr(buffer, successToken) != 0) {
		endMessageReceived = true;
	} else {
		if (strstr(buffer, failToken) != 0) { //hopefully never received ...
			parseFailToken();
		}
	}
	std::fill(buffer, buffer + bufferSize, 0);
}
inline void NFMonitor::endOfErrLine() {
	failStream << errBuffer << " ";
	std::fill(errBuffer, errBuffer + bufferSize, 0);
	errCursor = 0;
}

inline void NFMonitor::process(char c) {
	buffer[cursor++] = c;
}
inline void NFMonitor::processErr(char c) {
	errBuffer[errCursor++] = c;
}
inline void NFMonitor::bufferOverflow(int line) {
		Suppress  suppressor(*this);
		std::cerr << "buffer overflow warning " << __FILE__ << ':' << line << std::endl;
}

void NFMonitor::interceptCout(char c) {
	if (active) {
		if (c == newline) {
			endOfLine();
		} else {
			if (cursor < bufferSize) {
				process(c);
			} else {
				bufferOverflow(__LINE__);
				endOfLine();
			}
		}
	}
}
void NFMonitor::interceptCerr(char c) {
	if (active) {
		if (c == newline) {
			endOfErrLine();
		} else {
			if (errCursor < bufferSize) {
				processErr(c);
			} else {
				bufferOverflow(__LINE__);
				endOfErrLine();
			}
		}
	}
}
