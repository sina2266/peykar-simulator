#ifndef PARSEREXCEPTION_H
#define PARSEREXCEPTION_H

#include "ExpressionException.h"

class ParserException : public ExpressionException
{
public:
	ParserException(string msg);
	~ParserException(void) throw( );
};

#endif
