#include <VCELL/Membrane.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Feature.h>
#include <VCELL/JumpCondition.h>
#include <SimpleSymbolTable.h>
#include <sstream>
using std::stringstream;

Membrane::Membrane(string& name, Feature* f1, Feature* f2) : Structure(name)
{
	feature1 = f1;
	feature2 = f2;
	ebbcType[0] = BOUNDARY_FLUX;
	ebbcType[1] = BOUNDARY_FLUX;
}

Membrane::~Membrane(void)
{
}

bool Membrane::inBetween(Feature* f1, Feature* f2) {
	return (f1 == feature1 && f2 == feature2 || f1 == feature2 && f2 == feature1);
}

void Membrane::resolveReferences(SimulationExpression *sim)
{
	Structure::resolveReferences(sim);
	
	for (int i = 0; i < sim->getNumMemVariables(); i ++) {
		Variable* var = (Variable*)sim->getMemVariable(i);
		if (isVariableDefined(var)) {
			feature1->addMemVarIndexInAdjacentMembrane(i);
			feature2->addMemVarIndexInAdjacentMembrane(i);
		}
	}
}

