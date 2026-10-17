#include "ParserException.h"

ParserException::ParserException(string msg) : ExpressionException("ParserException", msg)
{
}

ParserException::~ParserException(void) throw( )
{
}
