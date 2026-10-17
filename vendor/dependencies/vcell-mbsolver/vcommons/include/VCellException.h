#ifndef VCellException_h
#define VCellException_h
#include <sstream>
#include <stdexcept>

/**
* based on some ideas in apache Log4jcxxx (BSD licensed)
* @param EXC std:: exception to throw 
* @param x code fragment to stream 
*/
#define VCELL_RUNTIME_EXCEPTION(x) { std::ostringstream oss; oss << x << " at " << __FILE__ << ':' << __LINE__ << std::ends;  throw std::runtime_error(oss.str( )); }
#define VCELL_EXCEPTION(EXC,x) { std::ostringstream oss; oss << x << " at " << __FILE__ << ':' << __LINE__ << std::ends;  throw std::EXC(oss.str( )); } 
#define VCELL_EXCEPTION_NOLOCATION(EXC,x) { std::ostringstream oss; oss << x << std::ends;  throw std::EXC(oss.str( )); } 
#endif
