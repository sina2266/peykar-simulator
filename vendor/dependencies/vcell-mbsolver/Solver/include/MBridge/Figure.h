#ifndef Figure_h
#define Figure_h
#include <vector>
#include <iostream>
#include <ctime>
namespace matlabBridge {


	inline std::ostream & writeDateTime(std::ostream & in) {
		time_t t = time(0);
		struct tm  now;
#if defined(_MSC_VER)
		localtime_s(&now,&t);
#elif defined(__MINGW32__) || defined(__MINGW64__)
		struct tm* now0 = localtime(&t);
		now = *now0;
#else
		localtime_r(&t,&now);
#endif
		in << '%' << (now.tm_year + 1900) << '-' 
			<< (now.tm_mon + 1) << '-'
			<<  now.tm_mday << ' ' 
			<< now.tm_hour << ':'
			<< now.tm_min
			<< std::endl;
		return in;
	}
	inline std::ostream & clearFigure(std::ostream & in) {
		in << "cla;" << std::endl;
		return in;
	}

	inline std::ostream & pauseOn(std::ostream & in) {
		in << "pause on;" << std::endl;
		return in;
	}

	inline std::ostream & pauseOff(std::ostream & in) {
		in << "pause off;" << std::endl;
		return in;
	}

	inline std::ostream & pause(std::ostream & in) {
		in << "pause;" << std::endl;
		return in;
	}


	//ostream manipulator to insert pause command
	struct PauseSeconds {
		/**
		* pause for specifed seconds -- specify 0 for infinite (until key pressed) pauseo
		*/
		PauseSeconds(double seconds = 0)
			:s(seconds) {}
		void write(std::ostream &os) const {
			if (s == 0) {
				os << "pause" << std::endl;
			}
			else {
				os << "pause(" << s << ");" << std::endl;
			}
		}

	private:
		const double s; 
	};

	inline std::ostream & operator<<(std::ostream & in, const PauseSeconds & p) {
		p.write(in);
		return in;
	}

	struct FigureName {
		/**
		* @param n name
		*/
		FigureName(const char *n)
			:name(n) {}
		/**
		* @param n name 
		*/
		FigureName(std::string n)
			:name(n) {}

		void write(std::ostream &os) const {
			os << "figure('Name','" <<name <<"');" <<std::endl;
		}

	private:
		std::string name;
	};

	inline std::ostream & operator<<(std::ostream & in, const FigureName & fn) {
		fn.write(in);
		return in;
	}

	struct ConsoleMessage {
		ConsoleMessage(const char *m)
			:msg(m) {}

		ConsoleMessage(const std::string & m)
			:msg(m) {}

		void write(std::ostream &os) const {
			os << "fprintf(1,'" <<msg <<"\\n');" <<std::endl;
		}

	private:
		std::string msg;
	};

	inline std::ostream & operator<<(std::ostream & in, const ConsoleMessage & fn) {
		fn.write(in);
		return in;
	}

	struct Text {
		Text(double x_, double y_, const char * str_)
			:x(x_),
			y(y_),
			str(str_) {}
		void write(std::ostream &os) const {
			os << "text(" << x << ',' << y << ",'" << str << "');" << std::endl;
		}
		double x;
		double y;
		const char * const str;
	};

	inline std::ostream & operator<<(std::ostream & in, const Text & t) {
		t.write(in);
		return in;
	}

}
#endif
