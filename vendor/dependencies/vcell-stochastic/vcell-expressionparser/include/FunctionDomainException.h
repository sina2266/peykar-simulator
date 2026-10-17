#ifndef FUNCTIONDOMAINEXCEPTION_H
#define FUNCTIONDOMAINEXCEPTION_H

#include "ExpressionException.h"

class FunctionDomainException : public ExpressionException
{
public:
	FunctionDomainException(string msg);
	~FunctionDomainException(void) throw( );
};
#endif
