/**
 * Wrapper functions called from the Fortran side.
 *
 * The names carry a trailing underscore because that is how gfortran mangles a
 * module-less external by default; the Windows spellings are the upper-case
 * ones Intel Fortran used.
 *
 * Ported to the current vcell-messaging API when Hy3S was split out of
 * vcell-solvers. Previously this drove SimulationMessaging::create() and an
 * explicit start(); the singleton is now lazy and MessageEventManager owns the
 * sending thread, so neither exists.
 */
#include <iostream>

#include <VCELL/SimulationMessaging.h>

extern "C"
#ifdef WIN32
void __cdecl LOAD_JMS_INFO(char* broker, int slen1, char* smqusername, int slen2, char* password, int slen3,
		char* qname, int slen4, char* tname, int slen5, char* vcname, int slen6, int* simKey, int* jobIndex, int* taskID) {
#else
void load_jms_info_(char* broker, char* smqusername, char* password, char* qname, char* tname, char* vcname, int* simKey, int* jobIndex, int* taskID) {
#endif
	// smqusername, password, qname and tname are still accepted so the Fortran
	// call site and the input file format are unchanged, but the JMS REST bridge
	// derives the queue and topic itself and they go unused.
	(void)smqusername; (void)password; (void)qname; (void)tname;
#ifdef USE_MESSAGING
	if (*taskID >= 0) {
		SimulationMessaging::getInstVar()->initialize_curl_messaging(
				false, broker, vcname, *simKey, *jobIndex, *taskID);
	}
#else
	(void)broker; (void)vcname; (void)simKey; (void)jobIndex; (void)taskID;
#endif
}

extern "C"
#ifdef WIN32
void __cdecl SEND_PROGRESS(double* progress, double* time) {
#else
void send_progress_(double* progress, double* time) {
#endif
	SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_PROGRESS, *progress, *time);
	SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_DATA, *progress, *time);
}

extern "C"
#ifdef WIN32
void __cdecl SEND_COMPLETE(double* progress, double* time) {
#else
void send_complete_(double* time) {
#endif
	SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_COMPLETED, 1, *time);
	SimulationMessaging::getInstVar()->waitUntilFinished();
}

extern "C"
#ifdef WIN32
void __cdecl SEND_FAILED(char* errmsg) {
#else
void send_failed_(char* errmsg) {
#endif
	SimulationMessaging::getInstVar()->setWorkerEvent(JobEvent::JOB_FAILURE, errmsg);
	SimulationMessaging::getInstVar()->waitUntilFinished();
}

extern "C"
#ifdef WIN32
bool __cdecl IS_STOP_REQUESTED() {
#else
bool is_stop_requested_() {
#endif
	return SimulationMessaging::getInstVar()->isStopRequested();
}
