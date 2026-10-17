/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VARCONTEXT_H
#define VARCONTEXT_H

#include <vector>
#include <string>
using std::string;
using std::vector;

enum ExpressionIndex {INITIAL_VALUE_EXP=0, EXACT_EXP, DIFF_RATE_EXP, REACT_RATE_EXP,
	BOUNDARY_XM_EXP, BOUNDARY_XP_EXP, BOUNDARY_YM_EXP, BOUNDARY_YP_EXP, 
	BOUNDARY_ZM_EXP, BOUNDARY_ZP_EXP, VELOCITY_X_EXP, VELOCITY_Y_EXP, VELOCITY_Z_EXP,
	UNIFORM_RATE_EXP};
static string String_Expression_Index[] = {"INITIAL_VALUE_EXP", "EXACT_EXP", "DIFF_RATE_EXP", "REACT_RATE_EXP",
	"BOUNDARY_XM_EXP", "BOUNDARY_XP_EXP", "BOUNDARY_YM_EXP", "BOUNDARY_YP_EXP", 
	"BOUNDARY_ZM_EXP", "BOUNDARY_ZP_EXP", "VELOCITY_X_EXP", "VELOCITY_Y_EXP", "VELOCITY_Z_EXP",
	"UNIFORM_RATE_EXP"};
#define TOTAL_NUM_EXPRESSIONS (UNIFORM_RATE_EXP + 1)

namespace VCell {
	class Expression;
}
class Variable;
class VolumeVariable;
class Structure;
class EqnBuilder;
class SimulationExpression;
class Membrane;
class JumpCondition;
struct MembraneElement;

class VarContext {

public:
	~VarContext();

	Variable *getVar() { return variable; }
	
	//
	// ALWAYS CALL ParentClass::resolveReferences() first
	//
	virtual void resolveReferences(SimulationExpression *sim);

	Structure  *getStructure() { return structure; }

	void setExpression(VCell::Expression* newexp, ExpressionIndex expIndex);

	// exclusively for sundials pde
	double evaluateJumpCondition(Membrane* membrane, double* values);
	double evaluateExpression(ExpressionIndex expIndex, double* values);
	double evaluateConstantExpression(ExpressionIndex expIndex);

	void addJumpCondition(Membrane* membrane, VCell::Expression* exp);
	JumpCondition* getJumpCondition();

protected:
	VarContext(Structure *s, Variable* var);

	Variable *variable;
	Structure *structure;
	SimulationExpression    *sim;

	void bindAll(SimulationExpression* simulation);

	virtual bool isNullExpressionOK(ExpressionIndex expIndex) { return false; }
	bool isConstantExpression(ExpressionIndex expIndex);

private:
	VCell::Expression** expressions;
	double** constantValues;
	bool* needsXYZ;

	vector<JumpCondition*> jumpConditionList;
};

#endif
