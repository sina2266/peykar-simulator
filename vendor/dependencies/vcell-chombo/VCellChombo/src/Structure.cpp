#include <VCELL/Structure.h>
#include <VCELL/Variable.h>
#include <VCELL/VarContext.h>
#include <VCELL/FastSystemExpression.h>

Structure::Structure(string& Aname)
:name(Aname),
 definedVariableList( ),
 boundaryType( ),
 size(0),
 sizeFrac(0),
 numPoints(0),
 fastSystem(0)
{
	for (int i = 0; i < 6; i ++)
	{
		boundaryType[i] = BOUNDARY_UNKNOWN;
	}
}

Structure::~Structure(void)
{
	//assuming this will be allocated as a single object (not array)
	delete fastSystem;
}

void Structure::addDefinedVariable(Variable* var) {
	definedVariableList.push_back(var);
}

bool Structure::isVariableDefined(Variable* var){
	for (int i = 0; i < definedVariableList.size(); i ++) {
		if (var == definedVariableList[i]) {
			return true;
		}
	}
	return false;
}

void Structure::resolveReferences(SimulationExpression *sim)
{
	for (int i = 0; i < definedVariableList.size(); i ++) {
		definedVariableList[i]->getVarContext()->resolveReferences(sim);
	}

	if (fastSystem)
	{
		fastSystem->resolveReferences();
	}
}
