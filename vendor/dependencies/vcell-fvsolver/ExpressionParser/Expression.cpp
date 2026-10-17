#include <typeinfo>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
using std::cout;
using std::endl;
using std::istringstream;

#include "Expression.h"
#include "ExpressionParser.h"
#include "ASTFloatNode.h"
#include "ParseException.h"
#include "ParserException.h"
#include "StackMachine.h"

//long Expression::flattenCount = 0;
//long Expression::diffCount = 0;
//long Expression::parseCount = 0;
//long Expression::derivativeCount = 0;
//long Expression::substituteCount = 0;
//long Expression::bindCount = 0;
using VCell::Expression;

Expression::Expression(void)
	:rootNode(NULL),
	stackMachine(NULL) {}

Expression::Expression(Expression* expression)
	:rootNode(expression->rootNode->copyTree()),
	stackMachine(NULL) {}

Expression::Expression(const Expression &rhs)
	:rootNode(rhs.rootNode->copyTree()),
	stackMachine(NULL) {}

/**
* create and bind in single ctor
* equivalent to default constructor if #expString is empty
*/
Expression::Expression(string expString, SymbolTable & symbolTable)
	:rootNode(NULL),
	stackMachine(NULL) 
{
	if (!expString.empty( )) {
		init(expString);
		bindExpression(&symbolTable);
	}
}

Expression::Expression(string expString)
	:rootNode(NULL),
	stackMachine(NULL) 
{
	init(expString);
}

void Expression::init(const string & expString) {

	if (expString.length() == 0) {
		throw ParserException("Empty expression");
	}

	bool bNumber = true;
	for (unsigned int i = 0; i < expString.length(); i ++) {
		if (!isdigit(expString[i]) && expString[i] != '.' && expString[i] != ';') {
			bNumber = false;
			break;
		}
	}
	if (bNumber) {
		double value = 0.0;
		int n = sscanf(expString.c_str(), "%lf", &value); 
		if (n == 1) {
			rootNode = new ASTFloatNode(value);
			return;
		}				
	} 

	string trimstr = trim(expString);
	if (trimstr[trimstr.length() - 1] != ';'){
		trimstr += ";";
	}
	parseExpression(trimstr);
}

Expression::~Expression(void)
{
	delete rootNode;
	delete stackMachine;
}

Expression & Expression::operator=(const Expression &rhs) {
	rootNode = rhs.rootNode->copyTree();
	delete stackMachine;
	stackMachine = NULL;
	return *this;
}

void Expression::showStackInstructions(void)
{
	getStackMachine()->showInstructions();
	cout.flush();
}

double Expression::evaluateConstant(void)
{
	return getStackMachine()->evaluate(NULL);
}

double Expression::evaluateConstantTree()
{
	return rootNode->evaluate(EVALUATE_CONSTANT);
}

double Expression::evaluateVectorTree(double* values)
{
	try {
		return rootNode->evaluate(EVALUATE_VECTOR, values);
	} catch (Exception& ex) {
		Exception::rethrowException(ex, ex.getMessage() + " in " + getEvaluationSummary(values));
	}
}

string Expression::getEvaluationSummary(double* values)
{
	return rootNode->getNodeSummary(values, rootNode);
}

double Expression::evaluateVector(double* values)
{
	try {
		return getStackMachine()->evaluate(values);
	} catch (Exception& ex) {
		Exception::rethrowException(ex, ex.getMessage()+ " in " + getEvaluationSummary(values));
	}
}

void Expression::parseExpression(string exp)
{
	//parseCount++;
	try {
		istringstream iss(exp);
		ExpressionParser parser(&iss);
		
		delete rootNode;
		rootNode = parser.Expression();

		if (typeid(*rootNode) == typeid(ASTExpression)){
			if (rootNode->jjtGetNumChildren() == 1){ // we abandon the real root node here, so there is tiny memory leak;
				Node * old = rootNode;
				rootNode = old->abandonChild(0);
				delete old;
			}
		}
	} catch (Exception& e) {
		throw ParserException("Parse Error while parsing expression " + e.getMessage());
	}
}

string Expression::infix(void)
{
	return rootNode->infixString(LANGUAGE_DEFAULT, 0);
}

string Expression::infix_Visit(void)
{
	return rootNode->infixString(LANGUAGE_VISIT, 0);
}

void Expression::bindExpression(SymbolTable* symbolTable)
{	
	//bindCount++;
	rootNode->bind(symbolTable);
}

string Expression::trim(string str)
{
	int len = (int)str.length();
	int st = 0;
	const char* val = str.c_str();

	while ((st < len) && (val[st] <= ' ')) {
	    st++;
	}
	while ((st < len) && (val[len - 1] <= ' ')) {
	    len--;
	}
	return ((st > 0) || (len < (int)str.length())) ?  str.substr(st, len-st) : str;
}

inline StackMachine* Expression::getStackMachine() {
	if (stackMachine == NULL) {
		vector<StackElement> elements_vector;
		rootNode->getStackElements(elements_vector);
		StackElement* elements = new StackElement[elements_vector.size()];
		int i = 0;
		for (vector<StackElement>::iterator iter = elements_vector.begin(); iter != elements_vector.end(); iter ++) {
			elements[i ++] = *iter;
		}
		stackMachine = new StackMachine(elements, (int)elements_vector.size());
		elements_vector.clear();
	}
	return stackMachine;
}

void Expression::getSymbols(vector<string>& symbols) {
	rootNode->getSymbols(symbols, LANGUAGE_DEFAULT, 0);
}

SymbolTableEntry* Expression::getSymbolBinding(string symbol){
	return rootNode->getBinding(symbol);
}

double Expression::evaluateProxy() {
	return evaluateVector(0);
}

void Expression::substituteInPlace(Expression* origExp, Expression* newExp) {
	Node* origNode = origExp->rootNode;
	Node* newNode = newExp->rootNode->copyTree();
	//
	// first check if must replace entire tree, if not then leaves can deal with it
	//
	if (origNode->equals(rootNode)){
		rootNode = newNode;
	} else {
		rootNode->substitute(origNode, newNode);
	}
}

bool Expression::isConstant( ) const {
	if (rootNode != NULL) {
		return rootNode->isConstant( );
	}
	return false;
}
