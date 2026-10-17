#ifndef TOKEN_H
#define TOKEN_H

#include <string>
using std::string;

class Token
{
public:
	Token(void);
	~Token(void);

	int kind;
	int beginLine, beginColumn, endLine, endColumn;
	string image;

	Token* next;
	Token* specialToken;
	static Token* newToken(int ofKind);
};

#endif
