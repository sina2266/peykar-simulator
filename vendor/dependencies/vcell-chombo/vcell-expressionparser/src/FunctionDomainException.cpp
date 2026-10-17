#include "FunctionDomainException.h"

FunctionDomainException::FunctionDomainException(string msg) : ExpressionException("FunctionDomainException", msg)
{
	
}

FunctionDomainException::~FunctionDomainException(void) throw( )
{
}
