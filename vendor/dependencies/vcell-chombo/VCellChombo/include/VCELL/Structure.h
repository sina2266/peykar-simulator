#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <vector>
#include <string>
using std::string;
using std::vector;

class Region;
class FastSystemExpression;
class Variable;
class SimulationExpression;

enum BoundaryType {
	BOUNDARY_UNKNOWN,
	BOUNDARY_VALUE, 
	BOUNDARY_FLUX, 
	BOUNDARY_PERIODIC
};

class Structure
{
public:
	Structure(string& Aname);
	virtual ~Structure(void);

	virtual BoundaryType getXmBoundaryType() { return boundaryType[0]; }
	virtual BoundaryType getXpBoundaryType() { return boundaryType[1]; }
	virtual BoundaryType getYmBoundaryType() { return boundaryType[2]; }
	virtual BoundaryType getYpBoundaryType() { return boundaryType[3]; }
	virtual BoundaryType getZmBoundaryType() { return boundaryType[4]; }
	virtual BoundaryType getZpBoundaryType() { return boundaryType[5]; }
	     
	virtual void setXmBoundaryType(BoundaryType bt) { boundaryType[0] = bt; }
	virtual void setXpBoundaryType(BoundaryType bt) { boundaryType[1] = bt; }
	virtual void setYmBoundaryType(BoundaryType bt) { boundaryType[2] = bt; }
	virtual void setYpBoundaryType(BoundaryType bt) { boundaryType[3] = bt; }
	virtual void setZmBoundaryType(BoundaryType bt) { boundaryType[4] = bt; }
	virtual void setZpBoundaryType(BoundaryType bt) { boundaryType[5] = bt; }

	virtual void resolveReferences(SimulationExpression *sim);

	const string& getName() { return name; }

	void addDefinedVariable(Variable* var);
	bool isVariableDefined(Variable* var);
	int getNumDefinedVariables() {
		return definedVariableList.size();
	}
	Variable* getDefinedVariable(int i) {
		return definedVariableList[i];
	}

	const BoundaryType* getBoundaryTypes() {
		return boundaryType;
	}

	double getSize()
	{
		return size;
	}
	void setSize(double s)
	{
		size = s;
	}
	void addSize(double s)
	{
		size += s;
	}
	void addNumPoints(int np)
	{
		numPoints += np;
	}
	void setNumPoints(int np)
	{
		numPoints = np;
	}
	int getNumPoints()
	{
		return numPoints;
	}

	void setFastSystem(FastSystemExpression* arg_fastSystem)	{fastSystem = arg_fastSystem; }
	FastSystemExpression *getFastSystem(){ return fastSystem; }

protected:
	string  name;
	vector<Variable*> definedVariableList;
	BoundaryType boundaryType[6];
	double size;
	double sizeFrac;
	int numPoints;

	FastSystemExpression *fastSystem;
};

#endif
