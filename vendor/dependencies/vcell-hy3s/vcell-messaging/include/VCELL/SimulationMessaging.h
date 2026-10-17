#ifndef _SIMULATIONMESSAGING_H_
#define _SIMULATIONMESSAGING_H_

#include <vector>


#include "CurlProxyClasses.h"
#include "MessageEventManager.h"
#include "WorkerEvent.h"
#ifdef USE_MESSAGING
#include <sys/time.h>

static constexpr int ONE_SECOND = 1000;
static constexpr int ONE_MINUTE = 60 * ONE_SECOND;
static constexpr int DEFAULT_TTL_HIGH = 10 * ONE_MINUTE;
static constexpr int DEFAULT_TTL_LOW = ONE_MINUTE;
#endif

enum WorkerEventOutputMode {
	// ***Align values to bits***
	WORKEREVENT_OUTPUT_MODE_STDOUT = 1,
	WORKEREVENT_OUTPUT_MODE_MESSAGING = 2,
	// = 4
	// = 8
	WORKEREVENT_OUTPUT_MODE_ALL = 3 // This should be 2^n -1, where n is number of bits

};

static constexpr int DEFAULT_PRIORITY = 0; //range 0-127, the bigger number the higher priority
static constexpr int Message_DEFAULT_PRIORITY = 0;


//TODO: Re-write so that there is a messaging handler abstract class with two children: stdOut and CUrl
class SimulationMessaging {
	public:
		static SimulationMessaging* getInstVar();
		static void cleanupInstanceVar();
		void setWorkerEvent(JobEvent::Status status, const char *eventMessage);
		void setWorkerEvent(JobEvent::Status status, double progress, double timepoint);
		void setWorkerEvent(JobEvent::Status status, double progress, double timepoint, const char *eventMessage);

		bool isStopRequested() {
			return this->eventHandler.stopWasCalled();
		}

		[[nodiscard]] int getTaskID() const { // default = -1
			return this->taskID;
		}

		[[nodiscard]] int getJobIndex() const { // default = 0
			return m_jobIndex;
		}
		void waitUntilFinished();
		#ifdef USE_MESSAGING
		void initialize_curl_messaging(bool alsoPrintToStdOut, const char* broker, const char* vcusername, int simKey, int jobIndex, int taskID, int ttl_low=DEFAULT_TTL_LOW, int ttl_high=DEFAULT_TTL_HIGH);
		#endif

	protected:
		virtual ~SimulationMessaging() noexcept;

	private:
		// Statics
		static void sendStdOutStatus(WorkerEvent*);
		static bool criticalDelivery(const WorkerEvent & event) {
			// is this a critical message type?
			switch (event.status) {
				case JobEvent::JOB_DATA:
				case JobEvent::JOB_PROGRESS:
					return false;
				default:
					return true;
			}
		}
		static SimulationMessaging *m_inst;

		// Members
		static bool isInitialized;

		SimulationMessaging();

		void sendStatus(WorkerEvent*);
		void keepAlive();
		std::vector<WorkerEvent *> events;
		MessageEventManager eventHandler;
		AbstractCurlProxy *curlHandler;
		WorkerEventOutputMode workerEventOutputMode;

		int taskID;
		int m_jobIndex;
		time_t lastSentEventTime;
		bool bNewWorkerEvent;

};

#endif // _SIMULATIONMESSAGING_H_
