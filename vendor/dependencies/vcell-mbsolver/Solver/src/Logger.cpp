#include <iostream>
#include <cstring>
#include <map>
#include <Logger.h>
#include <VCellException.h>
#include <stdexcept>
#include <algorithm>
using vcell_util::Logger;
namespace {
	class DefaultDest: public Logger::Destination {
		void report(const char * msg, bool newline) {
			std::cout << msg;
			if (newline) {
				std::cout << std::endl;
			}
		}
	} defDest;

}
/**
* function object; set KeyInfos to lvl unless explicitly set
*/

struct vcell_util::KeyInfoSetter {
	const Logger::Level level;
	KeyInfoSetter(Logger::Level lvl)
		:level(lvl) {}

	void operator( )(Logger::KeyInfo & ki) {
		if (!ki.isSet) {
			ki.level = level;
		}
	}
};

Logger Logger::instance; 

#pragma warning ( disable: 4355 )
Logger::Logger( )
	:logStream(*this),
	dest(&defDest),
	level(fatal),
	keys( )
{ 
	set(level);
}

Logger::~Logger( ) {
}

//static 
/**
* convert string to level
* @param in lower case string
* @return corresponding level
* @throws std::domain_error if no match for string
*/
Logger::Level Logger::readLevel(const char *in) {
#	define MATCH(x) if (strcmp(in,#x) ==0) return x; 
	MATCH(verbose);
	MATCH(trace);
	MATCH(debug);
	MATCH(info);
	MATCH(warn);
	MATCH(fatal);
# undef MATCH
	std::string s("No match for logging level ");
	s += in;
	throw new std::domain_error(s);
}
Logger::Key Logger::readKey(const char *in) {
#	define MATCH(x) if (strcmp(in,#x) ==0) return Key::x; 
	MATCH(formBoundaryPolygon);
	MATCH(setPos);
	MATCH(notCollecting);
	MATCH(concentrationExpression);
	MATCH(generationTime);
	MATCH(progressEstimate);
	MATCH(reactionTerm);
	MATCH(extra1);
	MATCH(extra2);
# undef MATCH
	VCELL_EXCEPTION(domain_error,"no match for logging level '" << in << "'");
}



void Logger::set(Level lvl) {
	level = lvl;
	std::for_each(keys.begin( ),keys.end( ), KeyInfoSetter(lvl));
}

Logger::Level Logger::currentLevel( ) const {
	return level;
}
void Logger::set(Key key, bool on) {
	int idx = static_cast<int>(key);
	KeyInfo & ki = keys[idx];
	ki.isSet = true;
	ki.level = static_cast<Level>(on ? verbose - 1 : fatal + 1);
}

void Logger::Debug(std::string method, std::string message)
{
	if (instance.enabled(Logger::Level::debug) )
	{
		std::ostringstream oss;
		oss << "(" << method << ")" << message << std::endl;
		instance.report(oss.str( ).c_str( ));
	}
}

void Logger::debugEntry(std::string method)
{
	if (instance.enabled(Logger::Level::debug) )
	{
		std::ostringstream oss;
		oss << "Entry (" << method << ")" << std::endl;
		instance.report(oss.str( ).c_str( ));
	}
}

void Logger::debugExit(std::string method)
{
	if (instance.enabled(Logger::Level::debug) )
	{
		std::ostringstream oss;
		oss << "Exit (" << method << ")" << std::endl;
		instance.report(oss.str( ).c_str( ));
	}
}

namespace vcell_util {
	std::ostream &operator<<(std::ostream & os, const Logger::Level & lvl) {
#	define CASE(x) case Logger::x: os << #x; break;
		switch (lvl) {
			CASE(verbose);
			CASE(trace);
			CASE(debug);
			CASE(info);
			CASE(warn);
			CASE(fatal);
		}
#	undef CASE
		return os;
	}
}


/*
std::ostream & EndLog(std::ostream &in) {
Logger::LogStream & ls = static_cast<Logger::LogStream &>(in);
ls.owner.dest->report(ls.str( ).c_str( ),true);
ls.str("");
ls.clear( );
return in;
}
*/
