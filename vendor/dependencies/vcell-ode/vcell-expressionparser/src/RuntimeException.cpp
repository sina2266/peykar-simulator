#include "RuntimeException.h"

RuntimeException::RuntimeException(string msg) : Exception("RuntimeException", msg)
{
	
}

RuntimeException::~RuntimeException(void) throw( )
{
}
