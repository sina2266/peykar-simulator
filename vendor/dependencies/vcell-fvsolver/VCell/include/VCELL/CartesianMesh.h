/*
* (C) Copyright University of Connecticut Health Center 2001.
* All rights reserved.
*/
#ifndef CARTESIANMESH_H
#define CARTESIANMESH_H

#include <VCELL/Mesh.h>
#include <VCELL/DoubleVector3.h>
#include <vector>
#include <iostream>

using std::vector;
using std::istream;

class Geometry;
class VolumeRegion;
class MembraneRegion;
class VolumeVariable;
class VCellModel;
class Particle;
class SparseMatrixPCG;
template <class> class IncidenceMatrix;
struct VoronoiRidge;

enum NormalDirection {NORMAL_DIRECTION_ERROR = -1, NORMAL_DIRECTION_X = 111, NORMAL_DIRECTION_Y, NORMAL_DIRECTION_Z};
enum CurvePlane {CURVE_PLANE_XY = 222, CURVE_PLANE_XZ, CURVE_PLANE_YZ};
enum BoundaryLocation {BL_Xm = 0, BL_Xp, BL_Ym, BL_Yp, BL_Zm, BL_Zp};


class CartesianMesh : public Mesh
{
public:	   
	explicit CartesianMesh(double captureNeighborhood=0);
	void initialize(VCellModel* model, istream& ifs);

	WorldCoord getVolumeWorldCoord(long volumeIndex) override;
	WorldCoord getMembraneWorldCoord(long membraneIndex) override;
	WorldCoord getMembraneWorldCoord(MembraneElement *element) override;
	virtual long getVolumeIndex(WorldCoord coord);

	double getVolumeOfElement_cu(long volumeIndex) override;

	void showSummary(FILE *fp) override;
	void write(FILE *fp) override;
	void writeMeshMetrics(FILE* fp) override;

	double getDomainSizeX() const { return domainSizeX; }
	double getDomainSizeY() const { return domainSizeY; }
	double getDomainSizeZ() const { return domainSizeZ; }
	double getDomainOriginX() const { return domainOriginX; }
	double getDomainOriginY() const { return domainOriginY; }
	double getDomainOriginZ() const { return domainOriginZ; }

	int  getNumVolumeX() const { return numX; }
	int  getNumVolumeY() const { return numY; }
	int  getNumVolumeZ() const { return numZ; }

	double getXScale_um() const { return scaleX_um; }
	double getYScale_um() const { return scaleY_um; }
	double getZScale_um() const { return scaleZ_um; }

	double getXArea_squm() const { return areaX_squm; }
	double getYArea_squm() const { return areaY_squm; }
	double getZArea_squm() const { return areaZ_squm; }

	double getVolume_cu() const { return volume_cu; }

	VolumeRegion   *getVolumeRegion(int i); 
	MembraneRegion *getMembraneRegion(int i); 
	int getNumVolumeRegions() const { return static_cast<int>(pVolumeRegions.size()); }
	int getNumMembraneRegions() const { return static_cast<int>(pMembraneRegions.size()); }

	int getMembraneNeighborMask(long meindex) override;
	int getMembraneNeighborMask(MembraneElement* element) override;
	double* getMembraneFluxArea(long index) override;

	MeshCoord getMeshCoord(long index) const
	{
		MeshCoord mc;
		mc.x = index % numX;
		mc.y = (index / numX) % numY;
		mc.z = index/ numXY;
		return mc;        
	}

private:
	void readGeometryFile(VCellModel* model, istream& ifs);
	void setBoundaryConditions();

	void initScale();

	void findMembraneNeighbors();
	typedef StatusIndex<long,NeighborType::NeighborStatus> NeighborIndex;
	NeighborIndex orthoIndex(long memIndex, long insideIndex, long outsideIndex, long indexer, int boundMask);
	NeighborIndex getNeighbor(int n,  long index, int neighbor);

	void findMembranePointInCurve(int n,  long index, int neighborDir, int& leftOverN, int& returnNeighbor);

	double domainSizeX{};
	double domainSizeY{};
	double domainSizeZ{};
	double domainOriginX{};
	double domainOriginY{};
	double domainOriginZ{};

	int      numX{};
	int      numY{};
	int      numZ{};
	int      numXY{};

	double   scaleX_um{};
	double   scaleY_um{};
	double   scaleZ_um{};

	double   areaX_squm{};
	double   areaY_squm{};
	double   areaZ_squm{};

	double   volume_cu{};

	struct {
		int oppositeDirection;
		int nDirections;

	} membraneInfo{};
	/**
	* return index of direction opposite "d"; value depends on number of dimensions
	*/
	int oppositeMembraneDirection(int d) const {
		return (d + membraneInfo.oppositeDirection) % membraneInfo.nDirections;
	}

	void writeCartesianMeshHeader(FILE *fp);
	void writeVolumeRegionsMapSubvolume(FILE *fp);
	void writeVolumeElementsMapVolumeRegion(FILE *fp);
	void writeMembraneRegionMapVolumeRegion(FILE *fp);
	void writeMembraneElements_Connectivity_Region(FILE *fp);

	vector<VolumeRegion*>   pVolumeRegions;
	vector<MembraneRegion*> pMembraneRegions;	
	void computeMembraneCoupling() override;

	void computeExactNormals();
	WorldCoord computeExactNormal(long meIndex);

	/**
	* set membrane element unit normal as average, unless too small, then default to feature normal
	* @param meptr to set
	* @param tangentNormals input to average 
	* @param numberOfNormals how many 
	*/
	void computeNormal(MembraneElement& meptr, const UnitVector3* tangentNormals, int numberOfNormals);
	void computeNormalsFromNeighbors(); 
	/**
	* compute specific normals
	* @return false if can't find pair of suitable neighbor tangents
	*/
	bool computeNormalsFromNeighbors(long index); 
	void adjustMembraneAreaFromNormal();

	/**
	* formerly getN
	* return number of steps in membrane direction direction to use to approximate normal;
	* it is a function of the curvature of the membrane 
	*/
	//review
	ArrayHolder<int,4> getNormalApproximationHops(long index);

	int computeNormalApproximationHops(int startingIndex, CurvePlane curvePlane, vector<double> curvex, vector<double> curvey, int currentMeIndexInVector, bool bClose);
	bool findCurve(int startingIndex, CurvePlane curvePlane, vector<double>& curvex, vector<double>& curvey, int& currentMEInVector);

	IncidenceMatrix<VoronoiRidge>* symmetrize(IncidenceMatrix<VoronoiRidge>* im, long N);
	void getNeighborCandidates (vector<long>& neighborCandidates, DoubleVector3 centralNormal, long index, int hierarchy);

	/**
	* find normal direction between features of specified membrane element
	* @parm index of membrane element
	*/
	//review
	NormalDirection membraneElementFeatureDirection(int index) const ;

	UnitVector3 unitVectorBetween(long volumeIndexFrom, long volumeIndexTo);

};

#endif
