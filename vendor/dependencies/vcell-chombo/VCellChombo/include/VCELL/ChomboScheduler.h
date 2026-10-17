#ifndef CHOMBOSCHEDULER_H
#define CHOMBOSCHEDULER_H

//#include <vector>
#include <iostream>
#include <map>
using namespace std;

#include <VCELL/VarContext.h>
#include <Vector.H>
#include <RealVect.H>
#include <IntVect.H>
#include <BaseIF.H>
#include <Expression.h>
#include <VCELL/ChomboSpec.h>

class SimulationExpression;
class Membrane;
class Variable;
class Feature;
class ChomboGeometry;
class ChomboSpec;
class SymbolTable;
class ProblemDomain;
class EBCellFAB;
class EBISLayout;
template <class> class RefCountedPtr;
template <class> class LevelData;
class Box;
class EBAMRPoissonOpFactory;
class DisjointBoxLayout;
template <class> class BaseIVFAB;
class VolIndex;

#define NUM_PHASES 2
struct ConnectedComponent;
struct MembraneElementMetrics;
struct Vertex;
#if CH_SPACEDIM == 2
struct Segment;
#else
struct Triangle;
struct SliceView;
#endif

enum MembraneInvalidIndex
{
	MEMBRANE_INDEX_IN_FINER_LEVEL = -1,
	MEMBRANE_INDEX_INVALID = -100,
};

class ChomboScheduler {

	friend class ChomboDomainBC;
	friend class ChomboEBBC;

public:
	ChomboScheduler(SimulationExpression* sim, ChomboSpec* chomboSpec);
	~ChomboScheduler();
	
	void initializeGrids();
	virtual void initValues()=0;

	void writeData(char* filename, bool convertChomboData);
	int getNumMembranePoints()
	{
		return numMembranePoints;
	}
	ChomboGeometry* getChomboGeometry()
	{
		return chomboGeometry;
	}
	virtual void iterate()=0;
	void writeMembraneFiles();

	const IntVect& getViewLevelMeshSize()
	{
		return vectNxes[chomboSpec->getViewLevel()];
	}
	
protected:
	SimulationExpression* simulation;

	int numLevels;
	int numUnknowns;
	Vector<int>  vectRefRatios;
	Vector<Vector<Box> > vectRefBoxes;
	
	ChomboSpec* chomboSpec;
	ChomboGeometry* chomboGeometry;
	BaseIF *geoIfs[NUM_PHASES];

	Vector<DisjointBoxLayout>  vectGrids;
	Vector< Vector< Vector<EBISLayout> > > vectEbis;
	Vector<ProblemDomain>  vectDomains;
	Vector<RealVect> vectDxes;
	Vector<IntVect> vectNxes;
	Vector< Vector< Vector< RefCountedPtr< LevelData< BaseIVFAB<int> > > > > > irregularPointMembraneIDs; // here it stores membrane ID
	Vector< Vector< Vector< RefCountedPtr< LevelData< BaseIVFAB<int> > > > > > irregularPointMembraneIndex; // here it stores membrane index
	int findLevel(const ProblemDomain& domain);

	Vector< Vector< Vector<LevelData<EBCellFAB>*> > > volSoln;
	Vector< Vector< RefCountedPtr< LevelData< BaseIVFAB<Real> > > > > memSoln;
	IntVect numGhostSoln;
	IntVect membraneIndexGhost;
	Vector< Vector< Vector< RefCountedPtr< LevelData<BaseIVFAB<Real> > > > > > extrapValues;

	Vector<EBAMRPoissonOpFactory *> opFactories;

	double* vectValues; // used to evaluate expressions
	int numSymbols;

	int hdf5FileCount;

	int getChomboBoxLocalIndex(const IntVect& size, int ivar, const IntVect& ijk);
	int getChomboBoxLocalIndex(const IntVect& size, int ivar, D_DECL(int i, int j, int k));
	int getVolumeIndex(const IntVect& size, const IntVect& ijk);

	double getExpressionConstantValue(Variable* var, ExpressionIndex expIndex, Feature* feature);
	
	bool isInNextFinerLevel(int level, const IntVect& gridIndex);

	Vector< Vector<ConnectedComponent*> > phaseVolumeList;
	int numConnectedComponents;
	int numMembranePoints;

	static const int phase0;
	static const int phase1;

	// phase, ivol, Level, Box,
	Vector< Vector< Vector< Vector< Vector<VolIndex> > > > > irregTinyVolNeighbors;
	bool bHasTinyVols;

private:
	bool computeOneFaceCross(int, int, int, RealVect&, RealVect&, RealVect&, RealVect&, RealVect&);

	static hid_t createRealVectDataType();
	static hid_t createIntVectDataType();

#if CH_SPACEDIM == 2
	int findNeighborMembraneIndex2D(int iphase, int ilev, const IntVect& gridIndex, int iedge,
	const RealVect& normalizedCrossPoint, const RealVect& crossPointRealCoords, int& neighborEdge);
#else
	bool assignEdgeVertArray(int ilev, IntVect& nGridIndex, bool isCorner, int otherEdge, int vertexIndex, int (*edgeVertArray)[21]);
	IntVect orientVertices(RealVect* vertices, RealVect& outNormal);
#endif
	
	void generatePhasesAndVolumes();
	void generateMesh();
	void computeFeatures();
	void generateMembraneIndexData();
	void computeStructureSizes();
	void generateVolumeMembraneIndexMap();
	
#ifdef CH_MPI
	void exchangeFeaturesAndMembraneIndexOffset();
	int totalNumMembranePoints;
#endif
	void updateSolutionFromLevelData();
	void updateSolutionFromChomboOutputFile();

#if CH_SPACEDIM == 2
	void writeMeshHdf5(MembraneElementMetrics* metricsData, int vertexCount, Vertex* vertexList, Segment* segmentList);
#else
	void writeMeshHdf5(MembraneElementMetrics* metricsData, int vertexCount, Vertex* vertexList, int triangleCount, Triangle* surfaceData, SliceView* sliceViewData);
#endif
	int memIndexOffset;
	Vector< map<int, int> > irregVolumeMembraneMap;

	void generateTinyVolumeNeighbors();

	void populateExtrapolatedValues();
	void populateMembraneSolution();

#ifndef CH_MPI
	void populateVolumeSolution();
	void populateVolumeSolution(int iphase, int ivol);
	void populateImplicitFunctions();
#endif
	
	void computeTotal();
	void computeTotal(int iphase, int ivol);
	Real computeSum(const Vector< LevelData<EBCellFAB>* >& a_src, const Vector< EBISLayout >& a_ebisl, const int& a_comp);

	bool tagROIs(Vector<IntVectSet>& tags);
};

#endif
