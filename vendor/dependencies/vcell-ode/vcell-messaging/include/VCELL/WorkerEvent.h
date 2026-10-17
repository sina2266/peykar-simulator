//
// Created by Logan Drescher on 11/25/25.
//

#ifndef VCELL_ODE_NUMERICS_WORKEREVENT_H
#define VCELL_ODE_NUMERICS_WORKEREVENT_H
#include <string>

#include "JobEventStatus.h"

struct WorkerEvent {
	JobEvent::Status status;
	double progress;
	double timepoint;
	std::string eventMessage;

	WorkerEvent(const WorkerEvent* aWorkerEvent) {
		status = aWorkerEvent->status;
		progress = aWorkerEvent->progress;
		timepoint = aWorkerEvent->timepoint;
		eventMessage = aWorkerEvent->eventMessage;
	}

	WorkerEvent(JobEvent::Status status, double progress, double timepoint, const char *eventMessage)
		:status(status),
		progress(progress),
		timepoint(timepoint),
		eventMessage(eventMessage) {}

	WorkerEvent(JobEvent::Status status, double progress, double timepoint)
		:status(status),
		progress(progress),
		timepoint(timepoint){}


	WorkerEvent(JobEvent::Status arg_status, const char* eventMessage)
		:status(arg_status),
		progress(0),
		timepoint(0),
		eventMessage(eventMessage) {}

	bool equals(const WorkerEvent* aWorkerEvent) const {
		return nullptr != aWorkerEvent
			&& this->status == aWorkerEvent->status
			&& this->progress == aWorkerEvent->progress
			&& this->timepoint == aWorkerEvent->timepoint
			&& this->eventMessage == aWorkerEvent->eventMessage;
	}
};

#endif //VCELL_ODE_NUMERICS_WORKEREVENT_H