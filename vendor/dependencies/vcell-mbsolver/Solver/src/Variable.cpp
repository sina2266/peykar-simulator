#include <Variable.h>
#include <VCellException.h>
#include <cstring>
#include <sstream>
using moving_boundary::Variable;
using std::stringstream;
using std::endl;

Variable::Variable(const string & name)
				:name_(name),
				 bAdvecting(false),
				 size(0),
				 currSol(nullptr)
{
	initExpressions();
}

Variable::Variable(const string & name, int a_size)
				:name_(name),
				 bAdvecting(false),
				 size(a_size)
{
	currSol = new double[size];
	initExpressions();
}

Variable::~Variable()
{
	for (int i = 0; i < expr_size; ++ i)
	{
		delete expressions[i];
	}
	delete[] expressions;
	delete[] currSol;
}

void Variable::initExpressions()
{
	expressions = new SExpression*[expr_size];
	for (int i = 0; i < expr_size; ++ i)
	{
		expressions[i] = nullptr;
	}
}

void Variable::bindExpressions(const SimpleSymbolTable* symTable) {
	forbidNullExpression(expr_initial);
	forbidNullExpression(expr_source);
	if (getType() == vartype_volume)
	{
		forbidNullExpression(expr_diffusion);
	}

	for (int i = 0; i < expr_size; ++ i)
	{
		if (expressions[i] != nullptr)
		{
			expressions[i]->bindExpression(symTable);

			if (!bAdvecting)
			{
				// if advection X or Y is not constant or 0, set bAdvecting true
				if ((i == expr_advection_x || i == expr_advection_y) && isExpressionNonZero((ExpressionIndex)i))
				{
					bAdvecting = true;
				}
			}
		}
	}
}

void Variable::setExpression(ExpressionIndex exprIndex, const string& expr)
{
	expressions[exprIndex] = new SExpression(expr);
}

void Variable::forbidNullExpression(ExpressionIndex exprIndex) const
{
	if (expressions[exprIndex] == nullptr)
	{
		stringstream ss;
		VCELL_RUNTIME_EXCEPTION("Expression for " << ExpressionDescription[exprIndex] << " is null ");
		throw ss.str();
	}
}

double Variable::evaluateExpression(ExpressionIndex exprIndex, double* inputValues) const
{
	forbidNullExpression(exprIndex);
	return expressions[exprIndex]->evaluate(inputValues);
}


double Variable::getExpressionConstantValue(ExpressionIndex exprIndex) const
{
	forbidNullExpression(exprIndex);
	return expressions[exprIndex]->constantValue();
}

bool Variable::isExpressionConstant(ExpressionIndex exprIndex) const
{
	return expressions[exprIndex] != nullptr && expressions[exprIndex]->isConstant();
}

bool Variable::isExpressionNonZero(ExpressionIndex exprIndex) const
{
	return expressions[exprIndex] != nullptr && (!expressions[exprIndex]->isConstant() || expressions[exprIndex]->constantValue() != 0);
}

void Variable::setCurrSol(int index, double v)
{
	if (currSol == nullptr)
	{
		VCELL_RUNTIME_EXCEPTION("currSol for Variable " << name_ << " is not allocated. ");
	}
	currSol[index] = v;
}

double Variable::getCurrSol(int index)
{
	if (currSol == nullptr)
	{
		VCELL_RUNTIME_EXCEPTION("currSol for Variable " << name_ << " is not allocated. ");
	}
	return currSol[index];
}

double* Variable::getCurrSol()
{
	if (currSol == nullptr)
	{
		VCELL_RUNTIME_EXCEPTION("currSol for Variable " << name_ << " is not allocated. ");
	}
	return currSol;
}
