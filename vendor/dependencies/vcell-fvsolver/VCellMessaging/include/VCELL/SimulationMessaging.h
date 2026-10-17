#ifndef _SIMULATIONMESSAGING_H_
#define _SIMULATIONMESSAGING_H_

#if (defined(WIN32) || defined(WIN64) )
#define _HAS_STD_BYTE 0
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#endif


#include <deque>
#ifdef USE_MESSAGING
#include <stdlib.h>
#include <stdio.h>
#include <memory>

#endif
#include <iostream>
using namespace std;


#ifdef USE_MESSAGING
#if (!defined(WIN32) && !defined(WIN64) )
#include <sys/time.h>
#else
#include <time.h>
#endif

static const int ONE_SECOND = 1000;
static const int ONE_MINUTE = 60 * ONE_SECOND;
static const int DEFAULT_TTL_HIGH = 10 * ONE_MINUTE;
static const int DEFAULT_TTL_LOW = ONE_MINUTE;
#endif

static const int WORKEREVENT_OUTPUT_MODE_STDOUT = 0;
static const int WORKEREVENT_OUTPUT_MODE_MESSAGING = 1;

static const int JOB_STARTING = 999;
static const int JOB_DATA = 1000;
static const int JOB_PROGRESS = 1001;
static const int JOB_FAILURE = 1002;
static const int JOB_COMPLETED = 1003;
static const int JOB_WORKER_ALIVE = 1004;

static const int DEFAULT_PRIORITY = 0; //range 0-127, the bigger number the higher priority
static const int Message_DEFAULT_PRIORITY = 0;

struct WorkerEvent {
	int status;
	double progress;
	double timepoint;
	char* eventMessage;

	WorkerEvent(const WorkerEvent* aWorkerEvent) {
		status = aWorkerEvent->status;
		progress = aWorkerEvent->progress;
		timepoint = aWorkerEvent->timepoint;
		eventMessage = copy(aWorkerEvent->eventMessage);
	}

	WorkerEvent(int arg_status, double arg_progress, double arg_timepoint, const char *arg_eventMessage)
		:status(arg_status),
		progress(arg_progress),
		timepoint(arg_timepoint),
		eventMessage(copy(arg_eventMessage)) {}

	WorkerEvent(int arg_status, double arg_progress, double arg_timepoint)
		:status(arg_status),
		progress(arg_progress),
		timepoint(arg_timepoint),
		eventMessage(0) {}


	WorkerEvent(int arg_status, const char* arg_eventMessage)
		:status(arg_status),
		progress(0),
		timepoint(0),
		eventMessage(copy(arg_eventMessage)) {}
private:
	static char *copy(const char *in) {
		if (in != 0) {
			size_t len = strlen(in) + 1;
			char * c = new char[len + 1];
			memset(c, 0, len);
			strcpy(c, in);
			return c;
		}
		return 0;
	}
public:

	bool equals(WorkerEvent* aWorkerEvent) {
		if (status != aWorkerEvent->status || progress != aWorkerEvent->progress || timepoint != aWorkerEvent->timepoint) {
			return false;
		}
		if (eventMessage != NULL && aWorkerEvent->eventMessage != NULL && strcmp(eventMessage, aWorkerEvent->eventMessage) != 0) {
			return false;
		}

		if (eventMessage != NULL && aWorkerEvent->eventMessage == NULL
			|| eventMessage == NULL && aWorkerEvent->eventMessage != NULL) {
			return false;
		}

		return true;
	}
};

class SimulationMessaging 
{
public:
    virtual ~SimulationMessaging() throw();
	static SimulationMessaging* create();
	static SimulationMessaging* getInstVar();
	void start();
	void setWorkerEvent(WorkerEvent* workerEvent);

	bool isStopRequested() {
		return bStopRequested;
	}

	int getTaskID() {
#ifdef USE_MESSAGING
		return m_taskID;
#else
		return -1;
#endif
	}

    int getJobIndex() {
#ifdef USE_MESSAGING
        return m_jobIndex;
#else
        return 0;
#endif
    }
#ifdef USE_MESSAGING
	static SimulationMessaging* create(const char* broker, const char* smqusername, const char* passwd, const char* qname, const char* tname,
			const char* vcusername, int simKey, int jobIndex, int taskID, int ttl_low=DEFAULT_TTL_LOW, int ttl_high=DEFAULT_TTL_HIGH);
	void waitUntilFinished();
	friend void* startMessagingThread(void* param);
#endif

private:
	SimulationMessaging();
	static SimulationMessaging *m_inst;
	std::deque<WorkerEvent *> events;
	int workerEventOutputMode;

	void sendStatus();
	bool bStopRequested;
	/**
	 * is this a critical message type?
	 */
	bool criticalDelivery(const WorkerEvent & event) {
		switch (event.status) {
		case JOB_DATA:
		case JOB_PROGRESS:
			return false;
		default:
			return true;
		}
	}

#ifdef USE_MESSAGING
	bool bStarted;

	SimulationMessaging(const char* broker, const char* smqusername, const char* passwd, const char* qname, const char*tname, const char* vcusername, int simKey, int jobIndex,  int taskID, int ttl_low=DEFAULT_TTL_LOW, int ttl_high=DEFAULT_TTL_HIGH);

	void keepAlive();
	static char* trim(char* str);
	void setupConnection ();    //synchronized
	void cleanup();
	
	char *m_broker;
	char *m_smqusername;
	char *m_password;
	char *m_qname;
	char* m_tname;
	char* m_vcusername;
	int m_simKey;
	int m_taskID;
	int m_jobIndex;

	char* m_tListener;

	char m_hostname[256];
	int  m_ttl_lowPriority;
	int  m_ttl_highPriority;
	time_t lastSentEventTime;

	const char* getStatusString(int status);

	bool lockMessaging();
	void unlockMessaging();
	void delay(int duration);

	bool lockWorkerEvent(bool bTry=false);
	void unlockWorkerEvent();
	struct WorkerEventLocker {
		 WorkerEventLocker(SimulationMessaging &sm_, bool bTry = false)
			 :sm(sm_),
			  locked( sm.lockWorkerEvent(false)) {
			 }
		 ~WorkerEventLocker( ) {
				 sm.unlockWorkerEvent();
		 }
		SimulationMessaging &sm;
		const bool locked;
	};

#ifdef WIN32
    CRITICAL_SECTION lockForMessaging;
	CRITICAL_SECTION lockForWorkerEvent;
	HANDLE hNewWorkerEvent;
	HANDLE hMessagingThreadEndEvent;
#else // UNIX
	pthread_t newWorkerEventThread;
	pthread_mutex_t mutex_messaging;
    pthread_mutex_t mutex_workerEvent;
    pthread_mutex_t mutex_cond_workerEvent;
	pthread_cond_t cond_workerEvent;
	bool bNewWorkerEvent;
#endif

#else
	//NO MESSAGING compile compatibility variant
	struct WorkerEventLocker {
		 WorkerEventLocker(SimulationMessaging &, bool bTry = false) {}
	};
#endif
};

#endif // _SIMULATIONMESSAGING_H_
