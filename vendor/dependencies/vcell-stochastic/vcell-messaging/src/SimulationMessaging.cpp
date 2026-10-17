#include <VCELL/SimulationMessaging.h>
#include <iostream>
#include <functional>
#include <string>
#include <format>

static constexpr double WORKEREVENT_MESSAGE_MIN_TIME_SECONDS = 15.0;
bool SimulationMessaging::isInitialized = false;


SimulationMessaging *SimulationMessaging::m_inst = nullptr;

SimulationMessaging::SimulationMessaging():
	eventHandler(std::bind(&SimulationMessaging::sendStatus, this, std::placeholders::_1)),
	workerEventOutputMode{WORKEREVENT_OUTPUT_MODE_STDOUT},
	taskID{-1},
	m_jobIndex{-1},
	bNewWorkerEvent{false}
{
	this->curlHandler = new NullCurlProxy();
	time(&this->lastSentEventTime);
}

SimulationMessaging::~SimulationMessaging() noexcept{
	// The worker thread reaches curlHandler through sendStatus(), and `eventHandler` -- being a
	// member -- is not torn down until after this body runs. Stop it first, or a destroy that did
	// not go through cleanupInstanceVar() frees the handler out from under an in-flight event.
	this->eventHandler.requestStopAndWaitForIt();
	delete this->curlHandler;
}

SimulationMessaging* SimulationMessaging::getInstVar() {
	if (nullptr == SimulationMessaging::m_inst) SimulationMessaging::m_inst = new SimulationMessaging();
	return SimulationMessaging::m_inst;
}

void SimulationMessaging::cleanupInstanceVar() {
	if (nullptr == SimulationMessaging::m_inst) return;
	SimulationMessaging::m_inst->waitUntilFinished();
	delete SimulationMessaging::m_inst;
	SimulationMessaging::m_inst = nullptr;
}

void SimulationMessaging::sendStatus(WorkerEvent* event) {
	if (WORKEREVENT_OUTPUT_MODE_STDOUT & workerEventOutputMode) SimulationMessaging::sendStdOutStatus(event);
	if (WORKEREVENT_OUTPUT_MODE_MESSAGING & workerEventOutputMode) this->curlHandler->sendStatus(event);

	time(&lastSentEventTime); // Update the time var
}

void SimulationMessaging::keepAlive() {
	if (workerEventOutputMode & WORKEREVENT_OUTPUT_MODE_MESSAGING) this->curlHandler->keepAlive();

	time(&lastSentEventTime); // Update the time var
}

void SimulationMessaging::sendStdOutStatus(WorkerEvent* workerEvent) {
	switch (workerEvent->status) {
		case JobEvent::JOB_DATA:
			std::cout << std::format("[[[data:{}]]])", workerEvent->timepoint) << std::endl;
			// printf("[[[data:%lg]]]", workerEvent->timepoint);
			// fflush(stdout);
			break;
		case JobEvent::JOB_PROGRESS:
			std::cout << std::format("[[[progress:{}%]]]", workerEvent->progress * 100.0) << std::endl;
			// printf("[[[progress:%lg%%]]]", workerEvent->progress * 100.0);
			// fflush(stdout);
			break;
		case JobEvent::JOB_STARTING:
			std::cout<< workerEvent->eventMessage << std::endl;
			break;
		case JobEvent::JOB_COMPLETED:
			std::cerr << "Simulation Complete in Main() ... " << std::endl;
			break;
		case JobEvent::JOB_FAILURE:
			std::cerr << workerEvent->eventMessage << std::endl;
			break;
		default:
			throw std::runtime_error(std::format("SimulationMessaging::sendStatus: `{}` is not recognized",JobEvent::toString(workerEvent->status)));
	}
}

void SimulationMessaging::setWorkerEvent(JobEvent::Status status, const char *eventMessage) {
	this->eventHandler.enqueue(status, eventMessage);
}

void SimulationMessaging::setWorkerEvent(const JobEvent::Status status, const double progress, const double timepoint) {
	this->eventHandler.enqueue(status, progress, timepoint);
	// bool critical = criticalDelivery(*arg_workerEvent);
	// if (!critical) {
	// 	iftry = true;
	// 	//
	// 	// for portability, if not POSIX, time_t not guaranteed to be in seconds
	// 	// add WORKEREVENT_MESSAGE_MIN_TIME_SECONDS to last event time
	// 	//
	// 	tm nextMessageTime_tm = *localtime( &lastSentEventTime);
	// 	nextMessageTime_tm.tm_sec += WORKEREVENT_MESSAGE_MIN_TIME_SECONDS; // add MIN_TIME seconds to the time
	// 	time_t nextMessageTime = mktime( &nextMessageTime_tm);             // normalize it
	// 	time_t currTime;
	// 	time(&currTime);
	// 	if (currTime <= nextMessageTime){
	// 		isTimeForNextMessage = false;
	// 	}
	// }
	//
	// if (isTimeForNextMessage) {
	// 	{ //scope for lock
	// 		WorkerEventLocker locker(*this,iftry);
	// 		if (locker.locked) {
	// 			events.push_back(arg_workerEvent);
	// 		}
	// 	} //unlock worker event
	// 	pthread_mutex_lock(&mutex_cond_workerEvent);
	// 	bNewWorkerEvent = true;
	// 	pthread_cond_signal(&cond_workerEvent);
	// 	pthread_mutex_unlock(&mutex_cond_workerEvent);
	// }
}

void SimulationMessaging::setWorkerEvent(JobEvent::Status status, const double progress, const double timepoint, const char *eventMessage) {
	this->eventHandler.enqueue(status, progress, timepoint, eventMessage);
}

void SimulationMessaging::waitUntilFinished() {
	this->eventHandler.requestStopAndWaitForIt();
}

#ifdef USE_MESSAGING

void SimulationMessaging::initialize_curl_messaging(bool alsoPrintToStdOut, const char* broker, const char* vcusername, int simKey, int jobIndex, int givenTaskID, int ttl_low, int ttl_high){
	this->workerEventOutputMode = alsoPrintToStdOut ? WORKEREVENT_OUTPUT_MODE_ALL : WORKEREVENT_OUTPUT_MODE_MESSAGING;
	this->taskID = givenTaskID;
	this->m_jobIndex = jobIndex;
	delete this->curlHandler; // get rid of the null one we make by default
	this->curlHandler = new CurlProxy(simKey, taskID, jobIndex, vcusername, broker, ttl_low, ttl_high);
}

// void SimulationMessaging::start() {
// 	if (workerEventOutputMode == WORKEREVENT_OUTPUT_MODE_STDOUT) {
// 		return;
// 	}
// 	if (bStarted) {
// 		return;
// 	}
// 	bStarted = true;
// 	int retv = pthread_create(&newWorkerEventThread, NULL, startMessagingThread, this);
// }

// void* startMessagingThread(void* lpParam){
// 	int waitReturn = 0;
// 	struct timespec timeout;
// 	struct timeval now;
// 	struct timeval start;
// 	SimulationMessaging* simMessaging = (SimulationMessaging*)lpParam;
//
// 	while (true) {
// 		waitReturn = 1;
// 		gettimeofday(&start, NULL);
// 		timeout.tv_sec = start.tv_sec;
// 		// condition might be signalled before this thread gets blocked
// 		// so this thread might miss signal and doesn't get wakened
// 		// if this happens don't want to wait too long to check if there
// 		// is new worker event. This is the reason for the loop and 2 sec timeout
// 		pthread_mutex_lock(&simMessaging->mutex_cond_workerEvent);
// 		while (timeout.tv_sec - start.tv_sec < 5 * 60) {
// 			// at this point, there are two possibilities
// 			// 1. this thread has been awakened, waited < 2 secs, waitReturn is 0
// 			// 2. this thread has waited full 2 sec timout, waitReturn is non zero
// 			// in either case, check if there is new worker event.
// 			if (simMessaging->bNewWorkerEvent) {
// 				simMessaging->bNewWorkerEvent = false;
// 				waitReturn = 0;
// 				break;
// 			}
// 			gettimeofday(&now, NULL);
// 			timeout.tv_sec = now.tv_sec + 2;
// 			timeout.tv_nsec = now.tv_usec * 1000;
// 			waitReturn = pthread_cond_timedwait(&simMessaging->cond_workerEvent, &simMessaging->mutex_cond_workerEvent, &timeout);
// 		}
// 		pthread_mutex_unlock(&simMessaging->mutex_cond_workerEvent);
//
// 		switch (waitReturn) {
// 			case 0:  { // new event
// 				simMessaging->sendStatus();
// 				break;
// 			}
//
// 			case ETIMEDOUT: // time out
// 				simMessaging->keepAlive();
// 				break;
//
// 			default:
// 				std::cout<< "Wait error: " << waitReturn << std::endl;
// 				break;
// 		}
// 	}
// }
#endif
