#ifndef MEMBRANE
#define MEMBRANE

#include <VCELL/Structure.h>
#include <vector>
using std::vector;

class Feature;
class SimulationExpression;
struct MembraneElement;
class MembraneVarContextExpression;
class MembraneRegionVarContextExpression;
class MembraneVariable;
class MembraneRegionVariable;

class Membrane : public Structure
{
public:
	Membrane(string& name, Feature* f1, Feature* f2);
	~Membrane() override;

	virtual void initMembraneValues(MembraneElement *membraneElement);
	virtual void initMembraneRegionValues(int membraneRegionIndex);

	MembraneVarContextExpression *getMembraneVarContext(MembraneVariable *var);
	void addMembraneVarContext(MembraneVarContextExpression *vc);

	MembraneRegionVarContextExpression *getMembraneRegionVarContext(MembraneRegionVariable *var);	
	void addMembraneRegionVarContext(MembraneRegionVarContextExpression *vc);

	void resolveReferences(SimulationExpression *sim);

	Feature* getFeature1() const { return feature1; }
	Feature* getFeature2() const { return feature2; }
	bool inBetween(Feature* f1, Feature* f2);
	void reinitConstantValues(SimulationExpression* sim);

private:
	Feature* feature1;
	Feature* feature2;

	vector<MembraneVarContextExpression*> membraneVarContextList;
	vector<MembraneRegionVarContextExpression*> membraneRegionVarContextList;
};

#endif
