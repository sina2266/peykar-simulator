#ifndef VCELL_EXPRESSION_H
#define VCELL_EXPRESSION_H

#include "Node.h"

class SymbolTable;
class SymbolTableEntry;
class Node;
class StackMachine;

namespace VCell {

class Expression
{
public:
	Expression(void);
	Expression(string expString);
	/**
	* symbolTable must remain valid memory
	*/
	Expression(string expString, SymbolTable & symbolTable);
	Expression(Expression* expression);
	Expression(const Expression &);
	~Expression(void);
	Expression & operator=(const Expression &rhs);
	double evaluateConstant(void);
	// exercise the old way of evaluating constant and vector by traversing abstract syntax tree
	double evaluateConstantTree();
	double evaluateVectorTree(double* values);
	// exercise the new way of evaluating vector by using stack machine
	double evaluateVector(double* values);

	string infix(void);
	/**
	* symbolTable must remain valid memory
	*/
	void bindExpression(SymbolTable* symbolTable);
	static string trim(string str);
	void getSymbols(vector<string>& symbols); 

	string getEvaluationSummary(double* values);

	SymbolTableEntry* getSymbolBinding(string symbol);
	double evaluateProxy();
	
	void showStackInstructions();
	void substituteInPlace(Expression* origExp, Expression* newExp);
	string infix_Visit(void);
	bool isConstant( ) const;

private:
	Node  *rootNode;

	//static long flattenCount;
	//static long diffCount;
	//static long parseCount;
	//static long derivativeCount;
	//static long substituteCount;
	//static long bindCount;
	void parseExpression(string exp);
	StackMachine* stackMachine;
	inline StackMachine* getStackMachine();
	/**
	* common ctor code
	*/ 
	void init(const string & expString);
};
}
#endif
