#include <VCELL/JumpCondition.h>

#include <sstream>
using std::stringstream;

#include <VCELL/SimulationExpression.h>
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

double JumpCondition::evaluateExpression(double* values) {
	if (constantValue != 0) {
		return *constantValue;
	}
	return expression->evaluateVector(values);	
}

bool JumpCondition::isConstantExpression() {
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
	return true;
}
