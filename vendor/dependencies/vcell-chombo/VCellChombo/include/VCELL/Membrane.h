#ifndef MEMBRANE
#define MEMBRANE

#include <VCELL/Structure.h>
#include <vector>
using std::vector;

class Feature;
struct MembraneElement;
class MembraneVariable;
class MembraneVarContextExpression;
class MembraneRegionVariable;
class MembraneRegionVarContextExpression;
class Simulation;
class JumpCondition;
class SimulationExpression;
class Variable;
class Expression;

class Membrane : public Structure
{
public:
	Membrane(string& name, Feature* f1, Feature* f2);
	~Membrane();

	virtual void resolveReferences(SimulationExpression *sim);

	Feature* getFeature1() {
		return feature1;
	}
	Feature* getFeature2() {
		return feature2;
	}
	bool inBetween(Feature* f1, Feature* f2);

	void setPhaseEbBcType(int phase, BoundaryType bt)
	{
		ebbcType[phase] = bt;
	}
	
	BoundaryType getPhaseEbBcType(int phase)
	{
		return ebbcType[phase];
	}

private:
	Feature* feature1;
	Feature* feature2;
  BoundaryType ebbcType[2];
//	vector<MembraneVarContextExpression*> membraneVarContextList;
//	vector<MembraneRegionVarContextExpression*> membraneRegionVarContextList;
};

#endif
