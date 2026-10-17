#ifndef PARSEEXCEPTION_H
#define PARSEEXCEPTION_H

#include "Exception.h"
#include "Token.h"
using namespace VCell;

class ParseException : public Exception
{
public:
	ParseException();
	ParseException(string msg);
	~ParseException(void) throw( );
	Token* currentToken;
	int** expectedTokenSequences;
	int numETS;
	int* etsLengthArray;
	const string* tokenImage;
	ParseException(Token* currentTokenVal, int** expectedTokenSequencesVal, int numETSVal, int* etsLengthArrayVal, const string* tokenImageVal);
	string getExactMessage(void);

protected:
	bool specialConstructor;  
	static string eol;	
};
#endif
