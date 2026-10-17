#ifndef FUNCTIONRANGEEXCEPTION_H
#define FUNCTIONRANGEEXCEPTION_H

#include "ExpressionException.h"

class FunctionRangeException : public ExpressionException
{
public:
	FunctionRangeException(string msg);
	~FunctionRangeException(void) throw( );
};
#endif
