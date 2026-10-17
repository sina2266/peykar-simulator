#ifndef EXPRESSIONTEST_H
#define EXPRESSIONTEST_H

class SymbolTable;

class ExpressionTest
{
public:
	ExpressionTest(void);
	~ExpressionTest(void);
	static void testEvaluateVector(void);
	static void testEvaluateConstant(void);
	static void testParser(char* filename);
	static void testParser(int count, char* javaresult, double cvalue, char* expStr, SymbolTable* symbolTable, double* values);
};

#endif
