#ifndef vcellstring_h
#define vcellstring_h
#include <string>
/**
* vcell string utilities
*/
namespace vcell_util {
	std::string convertNonPrintable(const std::string &source);
	inline bool endsWith(std::string const & value, std::string const & ending);
	template <typename T>
	std::string to_string(T value)
	{
			std::stringstream ss ;

			//throw the value into the string stream
			ss << value ;

			//convert the string stream into a string and return
			return ss.str() ;
	}
}
#endif
