#include "Token.h"

Token::Token(void)
{
	next = 0;
	specialToken = 0;
	kind = 0;
	beginLine = 0;
	beginColumn = 0;
	endLine = 0;
	endColumn = 0;
}

Token::~Token(void)
{
}

Token* Token::newToken(int ofKind)
{
	switch (ofKind) {
		default:
			return new Token();
	}
}
