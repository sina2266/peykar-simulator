#include <PointSubdomain.h>
#include <PointVariable.h>
#include <Physiology.h>
using namespace moving_boundary;

PointSubdomain::PointSubdomain(const string & name, Physiology* physiology, const string& posX, const string& posY)
				:Subdomain(name, physiology)
{
	positions[0] = new SExpression(posX);
	positions[1] = new SExpression(posY);
}

PointSubdomain::~PointSubdomain()
{
	for (int i = 0; i < DIM; ++ i)
	{
		delete positions[i];
	}
}

void PointSubdomain::bindExpressions(SymbolTable* symbolTable)
{
	for (int i = 0; i < DIM; ++ i)
	{
		positions[i]->bindExpression(symbolTable);
	}
}

void PointSubdomain::updatePosition(double* values)
{
	for (int i = 0; i < DIM; ++ i)
	{
		positionValues[i] = positions[i]->evaluate(values);
	}
}

