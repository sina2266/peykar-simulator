#ifndef EXPRESSIONPARSERTOKENMANAGER_H
#define EXPRESSIONPARSERTOKENMANAGER_H

#include <iostream>
#include <string>
using std::string;
using std::ostream;

#include "Token.h"
#include "SimpleCharStream.h"

#ifndef int64
typedef long long int64;
#endif

class ExpressionParserTokenManager
{
public:
	ExpressionParserTokenManager(SimpleCharStream* stream);
	ExpressionParserTokenManager(SimpleCharStream* stream, int lexState);
	~ExpressionParserTokenManager(void);
	Token* getNextToken(void);

private: 
	int curLexState;
	int defaultLexState;
	int jjnewStateCnt;
	int jjround;
	int jjmatchedPos;
	int jjmatchedKind;
	static int jjnextStates[];
	static int64 jjtoToken[];
	static int64 jjtoSkip[];
	char curChar;

	static string* jjstrLiteralImages[];
	static string lexStateNames[];
	ostream* debugStream;
	void setDebugStream(ostream* os);
	void ReInit(SimpleCharStream* stream);
	void ReInit(SimpleCharStream* stream, int lexState);
	void SwitchTo(int lexState);

	int jjStopStringLiteralDfa_0(int pos, long active0);
	int jjStartNfa_0(int pos, long active0);
	int jjStopAtPos(int pos, int kind);
	int jjStartNfaWithStates_0(int pos, int kind, int state);	
	SimpleCharStream* input_stream;
	int* jjrounds;
	int* jjstateSet;

	int jjMoveStringLiteralDfa0_0(void);
	void jjCheckNAdd(int state);
	void jjAddStates(int start, int end);
	void jjCheckNAddTwoStates(int state1, int state2);
	void jjCheckNAddStates(int start);
	void jjCheckNAddStates(int start, int end);
	int jjMoveNfa_0(int startState, int curPos);	
	void ReInitRounds(void);
	void init(SimpleCharStream* stream);
	Token* jjFillToken(void);	
};

#endif
