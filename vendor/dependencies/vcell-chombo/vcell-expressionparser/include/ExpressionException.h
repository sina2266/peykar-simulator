#ifndef EXPRESSIONEXCEPTION_H
#define EXPRESSIONEXCEPTION_H

#include "Exception.h"
using namespace VCell;

class ExpressionException : public VCell::Exception
{
public:
	~ExpressionException(void) throw( );
	ExpressionException(string msg);
	ExpressionException(string title, string msg);
};

#endif
