#ifndef RANDOMVARIABLE_H
#define RANDOMVARIABLE_H

#include <VCELL/SimTypes.h>

#include <string>
using std::string;

class RandomVariable
{
public:
	RandomVariable(string& name, VariableType vt, int size);
	~RandomVariable(void);

	string getName() {
		return name;
	}
	int getSize() {
		return size;
	}

	double* getRandomNumbers() {
		return randomNumbers;
	}

	VariableType getVariableType() {
		return varType;
	}

private:
	string name;
	int size;
	VariableType varType;
	double* randomNumbers;
	
};

#endif
