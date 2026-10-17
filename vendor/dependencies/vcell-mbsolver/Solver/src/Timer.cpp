#include <Timer.h>
#include <iomanip>
#include <ctime>
#include <Logger.h>

using vcell_util::Timer;
Timer::Timer( ) {}
Timer::~Timer( )  {
}

void Timer::start( ) {
	startTime = system_clock::now();
}

void Timer::stop( ) {
	stopTime = system_clock::now();
}

double Timer::elapsedInS( ) const {
	 return duration_cast<seconds>(stopTime - startTime).count();
}

double Timer::elapsed( ) const {
	 return elapsedInS();
}
