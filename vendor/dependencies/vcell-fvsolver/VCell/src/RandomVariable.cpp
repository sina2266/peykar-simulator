#include <VCELL/RandomVariable.h>
#include <string.h>

RandomVariable::RandomVariable(string& arg_name, VariableType vt, int s)
{
	name = arg_name;
	varType = vt;
	size = s;
	randomNumbers = new double[size];
	memset(randomNumbers, 0, size * sizeof(double));
}

RandomVariable::~RandomVariable(void)
{
	delete[] randomNumbers;
}
