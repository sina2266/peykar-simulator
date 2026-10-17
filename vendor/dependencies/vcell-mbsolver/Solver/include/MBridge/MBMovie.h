#ifndef MBMovie_h
#define MBMovie_h
#define Figure_h
#include <vector>
#include <iostream>
#include <string>
#include <ctime>
namespace matlabBridge {
	struct Movie {
		Movie(std::ostream &script_, const char *name_ = "movieStore")
			:script(script_),
			name(name_ ? name_ : "movieStore"),
			frame(1) {}
		void recordFrame( ) {
			script << name << '(' << frame++ << ") = getframe;" << std::endl;
		}
		void play(int numberTimes = 1, int framesPerSecond = 12) {
			script << "movie(" << name << ',' << numberTimes << ',' << framesPerSecond << ");" << std::endl;
		}
		void clear( ) {
			script << "clear " << name << ';' << std::endl;
		}

		std::ostream &script;
		std::string name;
		int frame;
	};
}
#endif
