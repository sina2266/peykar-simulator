#ifndef Subdomain_h
#define Subdomain_h
#include <string>
#include <vector>
#include <Variable.h>
using std::string;
using std::vector;

namespace moving_boundary
{
enum SubdomainType
{
	subdomain_volume=0,
	subdomain_membrane,
	subdomain_point,
};

struct Subdomain {

	Subdomain(const string & name, Physiology* a_physiology);
	virtual ~Subdomain();

	const string & name( ) const {
		return name_;
	}
	virtual SubdomainType getType()=0;
	void addVariable(Variable* v)
	{
		_variables.push_back(v);
	}
	int numVariables()
	{
		return _variables.size();
	}
	Variable* getVariable(int index)
	{
		return _variables.at(index);
	}
protected:
	string name_;
	vector<Variable*> _variables;
	Physiology* physiology;
};

}

#endif
