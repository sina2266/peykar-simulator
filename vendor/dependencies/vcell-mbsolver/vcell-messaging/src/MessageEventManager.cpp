//
// Created by Logan Drescher on 11/24/25.
//
#include <VCELL/MessageEventManager.h>
#include <iostream>
#include <utility>
#include <format>
#include <exception>
#include <system_error>

MessageEventManager::MessageEventManager(std::function<void(WorkerEvent*)> sendUpdateFunction):
		stopRequested{false},
		sendUpdateFunction{std::move(sendUpdateFunction)}
{
	this->eventQueueProcessingWorkerThread = std::thread([this]() {this->performQueueProcessing();}); // Should auto-start thread
}

MessageEventManager::~MessageEventManager() {
	this->requestStopAndWaitForIt(); // joins the worker; a no-op if stop was already requested
}

void MessageEventManager::enqueue(const JobEvent::Status status, const double progress, const double timepoint, const char *eventMessage) {
	this->enqueue(new WorkerEvent(status, progress, timepoint, eventMessage));
}

void MessageEventManager::enqueue(const JobEvent::Status status, const double progress, const double timepoint) {
	this->enqueue(status, progress, timepoint, "");
}

void MessageEventManager::enqueue(const JobEvent::Status status, const char *eventMessage) {
	this->enqueue(status, 0, 0, eventMessage);
}

void MessageEventManager::requestStopAndWaitForIt() {
	{
		// Queue mutex first, then the stop flag -- the same order `enqueue` and `processQueue` use.
		// Taking them the other way round deadlocks against a worker that is holding `queuetex`
		// while it checks whether it has been asked to stop.
		std::lock_guard queueLock{this->queuetex};
		std::lock_guard stopRequestedLock{this->stopRequestedMutex};
		this->stopRequested = true;
	}
	this->eventQueueForeman.notify_all(); // We want any sleeping workers to wake up, so they check if a stop was requested.

	// The worker only leaves `processQueue` once the queue is drained *and* stop was requested, so
	// joining is what makes this a real barrier: on return no event is in flight and nothing else
	// will touch `sendUpdateFunction`. `joinable()` keeps a second call harmless.
	if (this->eventQueueProcessingWorkerThread.joinable()) {
		this->eventQueueProcessingWorkerThread.join();
	}
}

bool MessageEventManager::stopWasCalled() {
	std::unique_lock stopRequestedLock{this->stopRequestedMutex};
	return this->stopRequested;
}

///////////////////////////////////////////////////
///				Private Methods					///
///////////////////////////////////////////////////

void MessageEventManager::performQueueProcessing() {
	try {
		processQueue();
	} catch (std::system_error e) {
		std::cerr << std::format("System Error (Code {} - {}): {}", std::to_string(e.code().value()), e.code().category().name(), e.what()) << std::endl;
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	} catch (...) {
		std::cerr << "An unknown exception occurred while processing the event queue!" << std::endl;
	}
}

void MessageEventManager::processQueue() {
	while (true) {
		WorkerEvent* event;
		{// START Queuetex Scope //
			std::unique_lock shouldBeActiveLock(this->queuetex);
			if (this->eventQueue.empty()) {
				{ // START stop-requested Scope
					std::unique_lock stopRequestedLock{this->stopRequestedMutex};
					if (this->stopRequested) {
						return; // We're all done; the stop-requester is waiting on our join()
					}
				} // END stop-requested Scope
				// If the code gets to here, the worker can stop working, and "get some sleep"
				this->eventQueueForeman.wait(shouldBeActiveLock); // Note that this `wait()` call, by design, unlocks the queue mutex, until it is "prodded".
				if (this->eventQueue.empty()) continue; // Probably means we need to check if stop was requested again
			}
			event = this->eventQueue.front();
			this->eventQueue.pop();
		}// END Queuetex Scope  //

		// Process Event
		this->processEvent(event);
		delete event; // Remember to delete the event! We need the memory back!
	}
}

void MessageEventManager::processEvent(WorkerEvent* event) {
	this->sendUpdateFunction(event);
}

void MessageEventManager::enqueue(WorkerEvent* event) {
	std::lock_guard workerIsActiveLock{this->queuetex}; // Need to lock to prevent worker from checking if it has more work while we make the request.
	std::lock_guard stopRequestedLock{this->stopRequestedMutex}; // We scope this lock to the whole function; "last in the door" policy
	if (this->stopRequested) {
		std::cerr << "A new event was added to the messaging queue, but this queue has had `stop` requested!" << std::endl;
		delete event;
		return; // note: on this return function scope ends, and thus so too does out function-scoped locks
	}
	this->eventQueue.emplace(event);
	this->eventQueueForeman.notify_one(); // Nudges one sleeping worker awake. If the worker is awake...this does nothing; as it should.
}