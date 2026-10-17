#ifndef NFMonitor_h
#define NFMonitor_h
#include <array>
#include <ctime>
#include <sstream>
namespace vcell_nfsim {
	struct NFMonitor {
		typedef unsigned int timeUnit;
		/**
		 * don't send progress more than this time
		 */
		static const timeUnit UPDATE_SECONDS = 2;

		/**
		 * if true, use network messaging
		 */
		NFMonitor(bool networkMessaging);
		/**
		 * begin simulation
		 */
		void reportStart( );
		void reportComplete( );
		int exitCode( ) const {
			return exitValue;
		}
	private:
		/**
		 * receives each char written to stream (cout)
		 */
		void interceptCout(char);
		/**
		 * receives each char written to stream (cerr)
		 */
		void interceptCerr(char);
		void endOfLine();
		void endOfErrLine();
		void parseStartToken();
		void parseTimeToken();
		void parseFailToken();
		void process(char);
		void processErr(char);
		void bufferOverflow(int line);
		timeUnit timeDiff(clock_t end, clock_t start);

		const bool networkMessaging;
		static const size_t bufferSize = 1024;
		char buffer[bufferSize];
		char errBuffer[bufferSize];
		size_t cursor;
		size_t errCursor;
		bool active;
		const char *startOfTimeData;
		/**
		 * how long is sim going to run?
		 */
		double totalSimTime;
		/**
		 * last message time
		 */
		std::clock_t lastUpdate;
		/**
		 * last percent sent
		 */
		double lastFraction;
		/**
		 * flag to only check sim time set once
		 */
		bool checkSimTime;
		bool endMessageReceived;
		/**
		 * fail messages
		 */
		std::ostringstream failStream;
		/**
		 * what program should return (NFsim itself always returns 0)
		 */
		int exitValue;

		struct Suppress;
		friend struct Suppress;
		friend struct NFMonitorCout;
		friend struct NFMonitorCerr;
	};
	struct NFMonitorCout {
		NFMonitor &mon;
		NFMonitorCout(NFMonitor &m)
			:mon(m) {}
		void intercept(char c) {
			mon.interceptCout(c);
		}
	};

	struct NFMonitorCerr {
		NFMonitor &mon;
		NFMonitorCerr(NFMonitor &m)
			:mon(m) {}
		void intercept(char c) {
			mon.interceptCerr(c);
		}
	};
}
#endif
