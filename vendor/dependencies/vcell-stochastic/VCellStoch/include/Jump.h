#ifndef JUMP_H
#define JUMP_H

#include <string>
#include <vector>
#include "SymbolTable.h"

using std::string;
using std::vector;

#include "StochVar.h"

namespace VCell {
	class Expression;
}

#ifndef boolean
typedef unsigned char boolean;
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif
/* This class defines the reference to an exsiting stochastic variable and it's
 * change during a specific jump process. The amendment can be increment, assignment
 * or decrement etc. 
 * Reference Jump.cpp for the method descriptions.
 *
 * @Author: Boris Slepchenko
 * @Amended: Tracy LI (operation added)
 * @version:1.0 Beta
 * @CCAM,UCHC. May 25,2006
 */
class StochVarContext {
public:
	StochVarContext(StochVar*, string, int);
	~StochVarContext();
	void updateCurr();
private:
	StochVar *sv;
	string operation;
	int val;
} ;

/* This class defines the jump process which is decribed by
 * time and possibility for the process to take place,
 * it's location in indexed tree, list of stochastic variables involved,
 * and list of processes being affected.
 * Reference Jump.cpp for the method descriptions.
 *
 * @Author: Boris Slepchenko
 * @Amended: Tracy LI
 * @version:1.0 Beta
 * @CCAM,UCHC. May 25,2006
 */
class Jump
{
    public:
       Jump(double, int, int); 
	   Jump();
	   ~Jump();
       boolean operator <(const Jump&);
       void setTime(double);
	   double getTime() {return tau;}
	   void setNode(int);
	   int getNode() {return nodeIndex;}
	   void setNameIndex(int);
	   int getNameIndex() {return nameIndex;}
       void setLogRand(double);
	   double getLogRand() {return log_rand;}
	   int getNumDependentJumps() {return jumpDependents.size();}
	   Jump *getDependent(int i) {return jumpDependents[i];}
	   int getNumVars() {return varContextList.size();}
	   StochVarContext *getVar(int i) {return varContextList[i];}
	   
	   void addVarContext(StochVarContext*);
	   void addDependentJump(Jump*);
	   void setProbabilityExpression(string, string*, int);
	   double getOldProbabilityRate() {return propensity;}
	   double getProbabilityRate(double*);
	   string getProbabilityRateEvaluationSummary(double* values);


    private:
       SymbolTable* table;
       double tau;  // absolute time (from starting time)
	   double propensity; //the propensity calculated by the probExpression, used to save old value
	   VCell::Expression* probExpression;  // probability expression
	   int nodeIndex; //the location in the indexed tree
	   int nameIndex; //the location in the StochModel.listOfProcessNames
	   double log_rand;  //product of propensity and putative time
	   vector <StochVarContext*> varContextList; //list of stochastic variable contexts
	   vector <Jump*> jumpDependents; //list of dependent jump processes
};


#endif

