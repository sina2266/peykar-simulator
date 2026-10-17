#include "DivideByZeroException.h"

DivideByZeroException::DivideByZeroException(string msg) : ExpressionException("DivideByZeroException", msg)
{
}

DivideByZeroException::~DivideByZeroException(void) throw( )
{
}
