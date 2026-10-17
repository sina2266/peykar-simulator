#ifndef DIVIDEBYZEROEXCEPTION_H
#define DIVIDEBYZEROEXCEPTION_H

#include "ExpressionException.h"

class DivideByZeroException : public ExpressionException
{
public:
	DivideByZeroException(string msg);
	~DivideByZeroException(void) throw( );
};
#endif
