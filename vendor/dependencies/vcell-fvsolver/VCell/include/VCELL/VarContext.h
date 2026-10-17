/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VARCONTEXT_H
#define VARCONTEXT_H

#include <vector>
#include <string>

#include "Expression.h"
using std::string;
using std::vector;

#define DEPENDENCY_MASK_UNDEFINED	0x00		// '0000 0000'b
#define DEPENDENCY_MASK_CONSTANT	0x01		// '0000 0001'b
#define DEPENDENCY_MASK_XYZ			0x02		// '0000 0010'b
#define DEPENDENCY_MASK_TIME		0x04		// '0000 0100'b
#define DEPENDENCY_MASK_VARIABLE	0x08		// '0000 1000'b
#define DEPENDENCY_MASK_TOTAL		0x15		// '0000 1111'b

enum EXPRESSION_INDEX {INITIAL_VALUE_EXP=0, DIFF_RATE_EXP, REACT_RATE_EXP, 
	BOUNDARY_XM_EXP, BOUNDARY_XP_EXP, BOUNDARY_YM_EXP, BOUNDARY_YP_EXP, 
	BOUNDARY_ZM_EXP, BOUNDARY_ZP_EXP, VELOCITY_X_EXP, VELOCITY_Y_EXP, VELOCITY_Z_EXP,
	GRADIENT_X_EXP, GRADIENT_Y_EXP, GRADIENT_Z_EXP, UNIFORM_RATE_EXP};
static string String_Expression_Index[] = {"INITIAL_VALUE_EXP", "DIFF_RATE_EXP", "REACT_RATE_EXP", 
	"BOUNDARY_XM_EXP", "BOUNDARY_XP_EXP", "BOUNDARY_YM_EXP", "BOUNDARY_YP_EXP", 
	"BOUNDARY_ZM_EXP", "BOUNDARY_ZP_EXP", "VELOCITY_X_EXP", "VELOCITY_Y_EXP", "VELOCITY_Z_EXP",
	"GRADIENT_X_EXP", "GRADIENT_Y_EXP", "GRADIENT_Z_EXP", "UNIFORM_RATE_EXP"};
#define TOTAL_NUM_EXPRESSIONS (UNIFORM_RATE_EXP + 1)

class Variable;
class VolumeVariable;
class Structure;
class SimulationExpression;
class Mesh;
class EqnBuilder;
class SimulationExpression;
class Membrane;
class JumpCondition;
struct MembraneElement;
namespace VCell {
	class Expression;
}

class VarContext {

public:
	virtual ~VarContext();

	Variable *getVar() const { return species; }
	
	//
	// ALWAYS CALL ParentClass::resolveReferences() first
	//
	virtual void resolveReferences(SimulationExpression *sim);

	virtual double getInitialValue(long index);
	
	virtual bool hasExact() { return false; }
	virtual bool hasRemainders() { return false; }
	
	Structure  *getStructure() const { return structure; }

	void setExpression(VCell::Expression* newexp, int expIndex);

	// exclusively for sundials pde
	double evaluateJumpCondition(MembraneElement*, double* values) const;
	double evaluateExpression(long expIndex, double* values) const;
	double evaluateConstantExpression(long expIndex) const;

	void addJumpCondition(Membrane* membrane, VCell::Expression* exp);
	void reinitConstantValues(SimulationExpression* sim);

protected:
    VarContext(Structure *s, Variable* var);

    Variable *species;
    Structure *structure;
    SimulationExpression    *sim;

	void bindAll(SimulationExpression* simulation);
	double evaluateJumpCondition(MembraneElement*) const;
	double evaluateExpression(long volIndex, long expIndex) const; // for volume
	double evaluateMembraneRegionExpression(long memRegionIdex, long expIndex) const; // for membrane region
	double evaluateVolumeRegionExpression(long volRegionIdex, long expIndex) const; // for volume regin
	double evaluateExpression(MembraneElement* element, long expIndex) const; // for membrane

	virtual bool isNullExpressionOK(int expIndex) const = 0;
	bool isConstantExpression(long expIndex) const;
	bool isXYZOnlyExpression(long expIndex) const;
	bool isNullExpression(int expIndex) const
	{
		return expressions[expIndex] == nullptr;
	}

private:
	vector<VCell::Expression*> expressions;
	vector<double*> constantValues;
	vector<unsigned char> dependencyMask;
	vector<JumpCondition*> jumpConditionList;

	void computeDependencyMask(SimulationExpression* sim, int expIndex);
};

#endif
