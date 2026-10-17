//
// Created by Logan Drescher on 11/25/25.
//
#include <VCELL/JobEventStatus.h>
#include <stdexcept>

std::string JobEvent::toString(const Status status) {
	switch (status) {
		case JOB_ALIVE:
			return "JOB_ALIVE";
		case JOB_FAILURE:
			return "JOB_DEAD";
		case JOB_PROGRESS:
			return "JOB_PROGRESS";
		case JOB_COMPLETED:
			return "JOB_COMPLETED";
		case JOB_DATA:
			return "JOB_DATA";
		case JOB_STARTING:
			return "JOB_STARTING";
		default:
			throw std::runtime_error("Unknown status type");
	}
}
