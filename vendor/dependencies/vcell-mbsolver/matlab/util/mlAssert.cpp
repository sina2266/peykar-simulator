#include <sstream>
#include <mex.h>
#include "matlabAssert.h"
void matlabLink::mlAssert(bool condition,const char *message, const char * file, unsigned int line) {
	if (!condition) {
		std::ostringstream msg;
		msg << "assert failure (" << message << ") " << file << ':' << line << std::ends;
		mexErrMsgTxt(msg.str( ).c_str( ));
	}
}