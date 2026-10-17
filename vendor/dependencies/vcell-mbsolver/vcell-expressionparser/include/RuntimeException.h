#ifndef RUNTIMEEXCEPTION_H
#define RUNTIMEEXCEPTION_H

#include "Exception.h"
using namespace VCell;

class RuntimeException : public Exception
{
public:
	RuntimeException(string msg);
	~RuntimeException(void) throw( );
};
#endif
