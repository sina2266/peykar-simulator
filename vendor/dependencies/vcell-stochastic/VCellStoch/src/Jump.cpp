#include "Jump.h"

#include "SimpleSymbolTable.h"
#include "Exception.h"
#include "Expression.h"
using VCell::Exception;
using VCell::Expression;

#include <iostream>
using std::cerr;
using std::endl;

//---------------Definition of StochVarContext Class-----------------
/*
 *Constructor of StochVarContext
 *Input para: StochVar*, refenece of the variable.
 *            string, the operation to apply on the variable during a specific jump process
 *                    the operations can be "inc", "dec", "equ", etc.
 *            int, the amendment of the variable in a specfic process.
 */
StochVarContext::StochVarContext(StochVar *stochVar, string oper, int v)
{
	sv = stochVar;
	operation = oper;
	val = v;
}//end of constructor StochVarContext(StochVar *, int)

//destructor
StochVarContext::~StochVarContext()
{ 
}//end of destructor

/*
 *The update of the current variable value based on it's operation to be taken.
 */
void StochVarContext::updateCurr()
{
	if (operation.compare("inc")==0)
		*(sv->getCurr()) += val;
	else if (operation.compare("equ")==0)
		*(sv->getCurr()) = val;
}//end of method updateCurr()


//---------------------Definition of Jump Class----------------------
/*
 *The constructor of the Jump class which sets the initial tau and propensity
 *to input values.
 *Input para: double, time for the process to take place 
 *            int, position in the indexed tree
 */
Jump::Jump(double time, int nodeIdx, int nameIdx)
{
    tau = time;
    propensity = 0;
	nodeIndex = nodeIdx;
	nameIndex = nameIdx;
	log_rand = 1.0;

	varContextList.erase(varContextList.begin(), varContextList.end());
	jumpDependents.erase(jumpDependents.begin(), jumpDependents.end());
}//end of constructor Jump(double,int)

/*
 *The constructor of Jump which set the initial tau and propensity
 *by default values (0s).
 */
Jump::Jump()
{
    tau = 0;
    propensity = 0;
	nodeIndex = 0;
	nameIndex = 0;
	log_rand = 1.0;
	varContextList.erase(varContextList.begin(), varContextList.end());
	jumpDependents.erase(jumpDependents.begin(), jumpDependents.end());
}//end of constructor Jump()

//Destructor
Jump::~Jump()
{
	delete probExpression;
	for(int i=0;i<varContextList.size();i++)
		delete varContextList[i];
	varContextList.clear();
	delete table;
}//end of destructor ~Jump()

/*
 *The method defines operator < for Jump class 
 *Input para: const, the reference of another jump process
 */
boolean Jump::operator<(const Jump& x)
{
    if (tau<x.tau){
       return TRUE;
    }else{
		FALSE ;
	}
	return FALSE;
}//end of method operator<()

/*
 *Set tau to the input value.
 *Input para: double, new value for tau
 */
void Jump::setTime(double time)
{
	tau = time;
}//end of method setTime()

/*
 *Set the jump process's index in an indexed tree.
 *Input para: int, the index in the indexed tree.
 */
void Jump::setNode(int idx)
{
	nodeIndex = idx;
}//end of method setNode()

/*
 *Set the jump process's index in listOfProcessNames.
 *Input para: int, the index in the listOfProcessNames
 */
void Jump::setNameIndex(int idx)
{
	nameIndex = idx;
}//end of method setNameIndex()

/*
 *Set random number to calculate tau.
 *Input para: double, random number
 */
void Jump::setLogRand(double r)
{
	log_rand = r;
}//end of method setLogRand()

/*
 *Add a new StochVarContext to the jump process
 *Input para: StochVarContext*, address for the new StochVarContext
 */
void Jump::addVarContext(StochVarContext *svc)
{
	varContextList.push_back(svc);
}//end of method addVarContext()

/*
 *Add a new dependent process to the jump process
 *Input para: Jump*, address for the dependent process
 */
void Jump::addDependentJump(Jump *jump)
{
	jumpDependents.push_back(jump);
}//end of method addDependentJump()

/*
 *Setup the probability expression for the Jump process and bind
 *it to a symboltable for evaluating it's value.
 *Input para: string, the expression in string format
 *            string*, the possible symbols to be appear in the expression
 *            int, number of possible symbols
 */
void Jump::setProbabilityExpression(string exp, string* names, int numOfNames)
{
	probExpression=new Expression(exp);
	table=new SimpleSymbolTable(names,numOfNames);
	probExpression->bindExpression(table);
}//end of method setProbabilityExpression()

/*
 *Get the probability for the process to take place by
 *evaluating the expression.
 *Input para: double*, list of the values corresponding to the symbols
 *Output para: double, the probability
 */
double Jump::getProbabilityRate(double* expVal) 
{
	if(probExpression!=NULL)
	{
		propensity=probExpression->evaluateVector(expVal);
		return propensity;
	}
	return 0;
}//end of method getProbabilityRate()

string Jump::getProbabilityRateEvaluationSummary(double* values)
{
	if (probExpression!=NULL)
	{
		return probExpression->getEvaluationSummary(values);
	}
	return NULL;
}