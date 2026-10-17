#ifndef EXPRESSIONBINDINGEXCEPTION_H
#define EXPRESSIONBINDINGEXCEPTION_H

#include "ExpressionException.h"

class ExpressionBindingException : public ExpressionException
{
public:
	ExpressionBindingException(string msg);
	~ExpressionBindingException(void) throw( );
	string identify();
};

#endif
