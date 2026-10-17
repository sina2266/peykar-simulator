#ifndef STRUCTURE_H
#define STRUCTURE_H

#include <VCELL/SimTypes.h>
#include <vector>
#include <string>
using std::string;
using std::vector;

class Region;
class FastSystem;

class Structure
{
public:
	explicit Structure(string& Aname);
	virtual ~Structure() = default;

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

	const string& getName() { return name; }
	int getNumRegions() const
	{
		return static_cast<int>(regionList.size());
	}
	void addRegion(Region* r);
	Region* getRegion(int i) const
	{
		return regionList.at(i);
	}
	int getNumElements();

	void setFastSystem(FastSystem* arg_fastSystem)	{fastSystem = arg_fastSystem; }
	FastSystem *getFastSystem() const { return fastSystem; }

protected:
	string  name;
	vector<Region*> regionList;
	int numElements;

	FastSystem *fastSystem;	

private:
	BoundaryType boundaryType[6];
};

#endif
