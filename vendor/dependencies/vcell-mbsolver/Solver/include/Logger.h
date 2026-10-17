#ifndef Logger_h
#define Logger_h
#include <sstream>
#include <fstream>
#include <array>
#include <iostream>
//std::ostream & EndLog(std::ostream &in); 
namespace vcell_util {
	struct KeyInfoSetter;

	class Logger {

	public:
		struct Destination {
			virtual void report(const char *msg, bool newline) = 0;
		};

		static Logger & get( )  {
			return instance;
		}


		~Logger( );

		enum Level{
			/**
			* zero reserved for internal use
			*/
			verbose = 1,
			trace,
			debug,
			info,
			warn,
			fatal
		};

		//update readKey if adding value to this enum
		enum struct Key : char {
			formBoundaryPolygon,
			setPos,
			notCollecting,
			concentrationExpression,
			generationTime,
			progressEstimate,
			nodeScaling,
			reactionTerm,
			/** utility key for development purpose, should be replaced by named enum before committing to source code control */
			extra1, 
			extra2,
			/** used for allocating storage, ensure last enum */
			last
		};

		static Level readLevel(const char *);

		static Key readKey(const char *);

		void setDestination(Destination &d) {
			dest = &d;
		}

		bool enabled(Level lvl) const {
			return lvl >= level;
		}

		/**
		* if not set explicitly,  return true if lvl > current level
		* @param lvl
		* @param key specific scope for logging
		*/
		bool enabled(Level lvl, Key key) const {
			int idx = static_cast<int>(key);
			return (lvl >= keys[idx].level);
		}

		/**
		* set master level
		*/
		void set(Level lvl);

		/**
		* get current level
		*/
		Level currentLevel( ) const;

		/**
		* activate level for specific scope
		* @param key specific scope for logging
		* @param  on turn on or off
		*/
		void set(Key key, bool on);

		void report(const char *msg, bool newline=true) {
			dest->report(msg,newline);
		}

		static std::ostream & stream( ) {
			return instance.logStream;
		}

		static void debugEntry(std::string method);
		static void debugExit(std::string method);
		static void Debug(std::string method, std::string message);

	private:
		struct KeyInfo {
			/**
			* is this Key set explicitly
			*/
			bool isSet;
			/**
			* level set to
			*/
			Level level;
		};

#pragma warning (disable : 4250) 
		struct LogStream: public std::stringstream {
			Logger &owner;
			LogStream(Logger &o)
				:owner(o) {}
		};

		Logger( );
		LogStream logStream;
		Destination * dest;
		Level level;
		std::array<KeyInfo,static_cast<int>(Key::last)> keys;
		static Logger instance;
		friend KeyInfoSetter;
	};


	struct FileDest: public Logger::Destination {
		FileDest(const char *name)
			:fstream(name) {}
		FileDest(const std::string &name)
			:fstream(name) {}
		void report(const char * msg, bool newline) {
			fstream  << msg; 
			if (newline) {
				fstream << std::endl;
			}
		}

	private:
		std::ofstream fstream;
	};

	struct StdoutDest: public Logger::Destination {
		StdoutDest() {}
		void report(const char * msg, bool newline) {
			std::cout << msg;
			if (newline) {
				std::cout << std::endl;
			}
		}
	};

	std::ostream &operator<<(std::ostream &, const Logger::Level &);


}

/**
* based on some ideas in apache Log4jcxxx (BSD licensed)
* @param level desired Logger level
* @param x code fragment to stream 
*/
#define VCELL_DEBUG(x) { \
	if (vcell_util::Logger::get( ).enabled(vcell_util::Logger::debug) ) \
	{ std::ostringstream oss; oss << x ; vcell_util::Logger::get( ).report(oss.str( ).c_str( )); } \
}

/**
* based on some ideas in apache Log4jcxxx (BSD licensed)
* @param level desired Logger level
* @param x code fragment to stream
*/
#define VCELL_LOG(level,x) { \
	if (vcell_util::Logger::get( ).enabled(vcell_util::Logger::level) ) \
	{ std::ostringstream oss; oss << x ; vcell_util::Logger::get( ).report(oss.str( ).c_str( )); } \
}

/**
* based on some ideas in apache Log4jcxxx (BSD licensed)
* unconditionally log 
* @param x code fragment to stream 
*/
#define VCELL_LOG_ALWAYS(x) { \
	{ std::ostringstream oss; oss << x ; vcell_util::Logger::get( ).report(oss.str( ).c_str( )); } \
}

/**
* log without newline (echo -n)
* @param level desired Logger level
* @param x code fragment to stream 
*/
#define VCELL_LOG_N(level,x) { \
if (vcell_util::Logger::get( ).enabled(vcell_util::Logger::level) ) \
	{ std::ostringstream oss; oss << x ; vcell_util::Logger::get( ).report(oss.str( ).c_str( ),false); } \
}

/**
* based on some ideas in apache Log4jcxxx (BSD licensed)
* @param level desired Logger level
* @param key arbitrary key to lookup level for 
* @param x code fragment to stream 
*/
#define VCELL_KEY_LOG(level,key,x) { \
	if (vcell_util::Logger::get( ).enabled(vcell_util::Logger::level,vcell_util::Logger::key) ) \
	{ std::ostringstream oss; oss << x ; vcell_util::Logger::get( ).report(oss.str( ).c_str( )); } \
}
/**
* based on some ideas in apache Log4jcxxx (BSD licensed)
* @param level desired Logger level
* @param predicate expression which evaluates to a boolean; if level >= set and predicate evaluates to true message streams
* @param x code fragment to stream 
*/
#define VCELL_COND_LOG(level,predicate, x) { \
	if (vcell_util::Logger::get( ).enabled(vcell_util::Logger::level) && ( predicate) ) \
	{ std::ostringstream oss; oss << x ; vcell_util::Logger::get( ).report(oss.str( ).c_str( )); } \
}

#endif
