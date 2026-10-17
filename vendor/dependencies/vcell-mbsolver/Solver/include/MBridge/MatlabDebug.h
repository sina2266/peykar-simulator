#ifndef MatlabDebug_h
#define MatlabDebug_h
#include <iostream>
#include <map>
#include <set>
#include <string>

namespace matlabBridge {

	struct MatLabDebug {
		/**
		* stream to write MATLAB script commands to
		*/
		static void setDebug(std::ofstream & dStream) {
			instance.debugStream = &dStream;
		}

		static void endDebug() {
			instance.debugStream = nullptr;
		}

		/**
		* return true if debugging turned on via #setDebug
		*/
		static bool on( ) {
			return instance.debugStream != nullptr;
		}

		/**
		* return true if debugging turned on via #activate with specified key has been made
		*/
		static bool on(const std::string &key) {
			return  instance.keys.find(key) != instance.keys.end( ) ;
		}

		static bool on(const char * const key) { 
			return on(std::string(key));
		}


		static void activate(const std::string &key, bool turnOn = true) {
			if (turnOn) {
				instance.keys.insert(key);
			}
			else {
				instance.keys.erase(key);
			}
		}

		/**
		* turn logging for key off or on
		@ param key named key to set
		@ param turnOn 
		*/
		static void activate(const char *const key, bool turnOn = true) { 
			activate(std::string(key), turnOn);
		}

		static std::ostream & stream( ) {
			assert(instance.debugStream);
			return *instance.debugStream;
		}
#if 0
		/**
		* access stream by name -- if it doesn't currently exist,
		* create it
		*/
		static std::ostream & stream(const char * const name) {
			return stream(std::string(name));
		}

		/**
		* access stream by name -- if it doesn't currently exist,
		* create it
		*/
		static std::ostream & stream(const std::string &name) {
			std::ofstream & f= instance.files[name];
			if (!f.is_open( )) {
				f.open(name);
			}
			return f;
		}
#endif	

	private:
		std::map<std::string,std::ofstream> files;
		std::ofstream *debugStream;
		std::set<std::string> keys;

		static MatLabDebug instance;
	};

	struct DebugStream {
		DebugStream(std::ofstream &dStream) {
			MatLabDebug::setDebug(dStream);
		}
		~DebugStream( ) {
			MatLabDebug::endDebug( );
		}
	};
}
#endif
