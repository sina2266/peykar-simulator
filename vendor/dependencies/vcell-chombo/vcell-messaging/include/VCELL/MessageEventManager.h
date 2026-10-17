//
// Created by Logan Drescher on 11/24/25.
//
#ifndef VCELL_ODE_NUMERICS_MESSAGEEVENTQUEUE_H
#define VCELL_ODE_NUMERICS_MESSAGEEVENTQUEUE_H

#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>
#include "WorkerEvent.h"
/*
 * We want to avoid threads being idle, while processing updates
 * Components:
 *	1) An "active" boolean (locked by a mutex) that indicates whether the queue is being processed.
 *		1a) We must ensure that all relevant information a thread would use to ensure it's done, is always locked by the mutex.
 *	2) A jthread dedicated to processing the queue when it has items, and sleeping when it does not
 *	3) A condition variable used to ensure the jthread only runs when it needs to.
 *
 *	LOCK ORDERING
 *	We are using two mutexes that could conflict with each other
 *	1) Mutex for the event queue (`queuetex`)
 *	2) Mutex for whether stop has been requested (`stopRequestedMutex`)
 *
 *	Both the worker (when it decides to "clock out") and `enqueue` have to consult the queue and the
 *	stop flag together, so the two mutexes do get held at the same time. That is only safe while every
 *	thread takes them in the same order.
 *	*****ALWAYS LOCK THE QUEUE MUTEX BEFORE THE STOP-REQUESTED MUTEX*******
 *
 *	SHUTDOWN
 *	`requestStopAndWaitForIt` joins the worker rather than waiting for the queue to drain. An empty
 *	queue is not the same as "all work finished": the worker pops an event under `queuetex` but sends
 *	it after releasing the lock, so a drain-based wait can hand control back to a caller that is about
 *	to destroy the very state the worker is still using.
 */
class MessageEventManager {
	public:
		explicit MessageEventManager(std::function<void(WorkerEvent*)> sendUpdateFunction);
		virtual ~MessageEventManager();
		void enqueue(JobEvent::Status status, double progress, double timepoint, const char *eventMessage);
		void enqueue(JobEvent::Status status, double progress, double timepoint);
		void enqueue(JobEvent::Status status, const char *eventMessage);
		void requestStopAndWaitForIt();
		bool stopWasCalled();

	private:
		void performQueueProcessing();
		void processQueue();
		void processEvent(WorkerEvent* event);
		void enqueue(WorkerEvent*);

		std::thread eventQueueProcessingWorkerThread; //TODO: make a `std::jthread` once compilers catch up with standard
		std::function<void(WorkerEvent*)> sendUpdateFunction;

		// Critical Resources / Regions
		// -> CR #1 - Whether stop has been requested or not
		bool stopRequested;
		std::mutex stopRequestedMutex;
		// -> CR #2 - The Event Queue
		std::queue<WorkerEvent*> eventQueue;
		std::condition_variable eventQueueForeman;
		std::mutex queuetex;
};

#endif //VCELL_ODE_NUMERICS_MESSAGEEVENTQUEUE_H