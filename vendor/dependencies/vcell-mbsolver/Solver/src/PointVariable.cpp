#include <PointVariable.h>
#include <cstring>
using namespace moving_boundary;

PointVariable::PointVariable(const string & name)
				:Variable(name, 1)
{
}

PointVariable::~PointVariable()
{
}

