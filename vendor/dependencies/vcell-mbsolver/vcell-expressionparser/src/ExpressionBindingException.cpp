#include "ExpressionBindingException.h"

ExpressionBindingException::ExpressionBindingException(string msg) : ExpressionException("ExpressionBindingException", msg)
{
}

ExpressionBindingException::~ExpressionBindingException(void) throw( )
{
}

std::string ExpressionBindingException::identify(){
	return "ExpressionBindingException";
}
