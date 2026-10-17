#include "FunctionRangeException.h"

FunctionRangeException::FunctionRangeException(string msg) : ExpressionException("FunctionRangeException", msg)
{
	
}

FunctionRangeException::~FunctionRangeException(void) throw()
{
}
