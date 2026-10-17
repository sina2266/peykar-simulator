#ifndef Timer_h
#define Timer_h

#include <chrono>
using namespace std::chrono;

namespace vcell_util {
	struct TimerImpl;
	struct Timer {
		Timer( );
		~Timer( );
		void start( );
		void stop( );

		double elapsedInS() const;
		double elapsed() const;

	private:
		time_point<system_clock> startTime;
		time_point<system_clock> stopTime;
	};
}

#endif
