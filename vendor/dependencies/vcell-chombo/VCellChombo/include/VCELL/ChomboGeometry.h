
#ifndef CHOMBOGEOMETRY_H
#define CHOMBOGEOMETRY_H

#include "BaseIF.H"
#include <assert.h>

#include <string>
#include <vector>
using std::string;
using std::vector;

#include <VCELL/ChomboIF.h>

namespace VCell {
class Expression;
}
class SymbolTable;
class Feature;

class ChomboGeometry
{
public: 
	ChomboGeometry();
	ChomboGeometry(const ChomboGeometry*);
	virtual ~ChomboGeometry();
	
	void addSubdomain(Feature* feature, int pi, VCell::Expression* ifExp, VCell::Expression* userExp);
	void addSubdomain(Feature* feature, int phaseIndex, string& distanceMapFile);
	Feature* getFeature(const RealVect& a_point, bool validate) const;

	void setDomainOrigin(RealVect& rv) {
		domainOrigin = rv;
	}
	void setDomainSize(RealVect& rv) {
		domainSize = rv;
	}
	void setMeshSize(IntVect& iv) {
		meshSize = iv;
	}
	void setDimension(int dim) {
		assert(dim == SpaceDim);
		geoDim = dim;
	}
	int getDimension() {
		return geoDim;
	}
	const RealVect& getDomainOrigin() {
		return domainOrigin;
	}
	const RealVect& getDomainSize() {
		return domainSize;
	}
	const IntVect& getMeshSize() {
		return meshSize;
	}
	const int getNumX()
	{
		return meshSize[0];
	}
	const int getNumY()
	{
		return meshSize[1];
	}
	const int getNumZ()
	{
		return geoDim == 2 ? 1 : meshSize[2];
	}

	const int getNumSubdomains() {
		return chomboIFList.size();
	}

	ChomboIF* getChomboIF(int index) {
		return chomboIFList[index];
	}

	Feature* getFeature(int index) {
		return chomboIFList[index]->getFeature();
	}

	static SymbolTable* getGeometrySymbolTable();

private:
	vector<ChomboIF*> chomboIFList;
	static SymbolTable* geometrySymbolTable;
	
	RealVect domainOrigin;
	RealVect domainSize;
	IntVect meshSize;
	int geoDim;
};

#endif
