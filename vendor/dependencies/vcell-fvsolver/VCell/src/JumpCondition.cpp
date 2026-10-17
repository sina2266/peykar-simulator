#include <VCELL/JumpCondition.h>

#include <sstream>
using std::stringstream;

#include <VCELL/Element.h>
#include <VCELL/SimTypes.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/SimTool.h>
#include <Expression.h>
using VCell::Expression;

JumpCondition::JumpCondition(Membrane* m, Expression* e)
{
	membrane = m;
	expression = e;
	constantValue = 0;
	bNeedsXYZ = false;
}

JumpCondition::~JumpCondition(void)
{
	delete expression;
}

void JumpCondition::bindExpression(SymbolTable* symbolTable) {
	try {
			//cout << expression->infix() << endl;
			double d = expression->evaluateConstant();
			constantValue = new double[1];
			constantValue[0] = d;
		} catch (...) {		
			expression->bindExpression(symbolTable);
			if (expression->getSymbolBinding("x") != NULL ||
				expression->getSymbolBinding("y") != NULL ||
				expression->getSymbolBinding("z") != NULL) {
				bNeedsXYZ = true;
			}
		}
}

double JumpCondition::evaluateExpression(SimulationExpression* simulation, MembraneElement* element) {
	if (constantValue != 0) {
		return *constantValue;
	}	
	if (bNeedsXYZ) {
		CartesianMesh* mesh = (CartesianMesh*)simulation->getMesh();
		WorldCoord wc = mesh->getMembraneWorldCoord(element);
		simulation->setCurrentCoordinate(wc);
	}
	int* indices = simulation->getIndices();
	indices[VAR_VOLUME_INDEX] = -1;
	indices[VAR_VOLUME_REGION_INDEX] = -1;
	indices[VAR_MEMBRANE_INDEX] = element->index;
	indices[VAR_MEMBRANE_REGION_INDEX] = element->getRegionIndex();
	return expression->evaluateProxy();	
}

double JumpCondition::evaluateExpression(double* values) {
	if (constantValue != 0) {
		return *constantValue;
	}
	return expression->evaluateVector(values);	
}

bool JumpCondition::isConstantExpression(SimulationExpression *sim) {
	// pure constant
	if (constantValue != 0) {
		return true;
	}

	// not defined
	if (expression == 0) {
		stringstream ss;
		ss << "JumpCondition::isConstantExpression(), expression not defined";
		throw ss.str();
	}

	// has parameters only
	vector<string> symbols;
	expression->getSymbols(symbols);
	for (int i = 0; i < (int)symbols.size(); i ++) {
		if (!sim->isParameter(symbols[i])) {
			return false;
		}
	}
	return true;
}

void JumpCondition::reinitConstantValues(SimulationExpression *sim) {
	if (expression == nullptr || !isConstantExpression(sim)) {
		return;
	}
	double d = expression->evaluateProxy();
	if (constantValue == nullptr) {
		constantValue = new double[1];
	}
	constantValue[0] = d;
}
