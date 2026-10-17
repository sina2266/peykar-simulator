/*
* (C) Copyright University of Connecticut Health Center 2001.
* All rights reserved.
*/
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <set>
#include <limits>
using std::stringstream;
using std::cout;
using std::endl;
using std::setprecision;
using std::max;
using std::min;
using std::set;

#include <MathUtil.h>
#include <cassert>
#include <cstring>
#include <VCELL/ManagedArrayPtr.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Element.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/SimTool.h>
#include <VCELL/SimTypes.h>
#include <VCELL/VCellModel.h>
#include <VCELL/SparseMatrixPCG.h>
#include <VCELL/IncidenceMatrix.h>
#include <VCELL/VoronoiRidge.h>

const int MAXNEIGHBOR_2D   = 3;
const int MAXNEIGHBOR_3D   = 30;
const int GOING_OUT_LAYERS = 3;

const double NORMAL_MINLENGTH_THRESHOLD = 1E-2;

const double PI = 3.1415926535897;
//#define COMPUTE_EXACT_NORMALS
//#define SPECIAL_SHAPE

//#define CIRCLE
//#define HALFCIRCLE
//#define QUARTERCIRCLE
//#define ROSE
//#define ELLIPSOID
//#define SPHERE
//#define CYLINDER
//#define HALFCUBE
//#define CUBE_PLANE
//#define QUARTERSPHERE
//#define CYLINDER_IN_CUBE
//#define SQUARE
//#define ELLIPSE
//#define APPLE
//#define PERPENDICULAR_CYLINDER

#define COMPUTE_IMPROVED_NORMALS
#define WITH_PROJECTION_TO_WALL

#ifdef ELLIPSOID
//double a = 2.0, b = sqrt(2.0), c = 1;
double a = 3.0, b = 2.0, c = 1.0;
//double a = 3.0, b = 1.0, c = 1.0/3;

//double a2 = 4.0, b2 = 2.0, c2 = 1.0;
double a2 = 9, b2 = 4, c2 = 1.0;
//double a2 = 9, b2 = 1, c2 = 1.0/9;
#endif

extern "C"
{
#include "zlib.h"
}

//static const int NumNaturalNeighbors_Membrane[] = {0, 0, 2, 4};

CartesianMesh::CartesianMesh(double AcaptureNeighborhood) : Mesh(AcaptureNeighborhood), domainSizeX(0), domainSizeY(0),
                                                            domainSizeZ(0),
                                                            domainOriginX(0),
                                                            domainOriginY(0),
                                                            domainOriginZ(0), numX(0),
                                                            numY(0), numZ(0),
                                                            numXY(0),
                                                            scaleX_um(0),
                                                            scaleY_um(0),
                                                            scaleZ_um(0),
                                                            areaX_squm(0),
                                                            areaY_squm(0),
                                                            areaZ_squm(0),
                                                            volume_cu(0),
                                                            membraneInfo()
{
}

//void CartesianMesh::setVolumeLists()
//{
//	if (dimension == 1){
//		return;
//	}
//	if(!getNumContourElements()){
//		cout << "no contour elements exist" << endl;
//		return;
//	}
//    
//	double distance =captureNeighborhood;
//	volumeLists = new vector<ContourElement*>[numVolume];
//
//	for(long volumeIndex=0;volumeIndex<numVolume;volumeIndex++){
//		volumeLists[volumeIndex].clear();
//	}
//	for(long index=0;index<getNumContourElements(); index++){
//		ContourElement *contourElement = getContourElement(index);
//		long volumeIndex = contourElement->getVolumeIndex();
//		WorldCoord wc_center = getVolumeWorldCoord(volumeIndex);
//		FeatureHandle handle = pVolumeElement[volumeIndex].feature->getHandle();
//		WorldCoord wc = wc_center;
//		FeatureHandle currHandle;
//		if(getDimension()==2){
//			double deltaX = getXScale_um();
//			double deltaY = getYScale_um();
//			int sizeX = (int)ceil(distance/deltaX);
//			int sizeY = (int)ceil(distance/deltaY);
//			for(int i=-sizeX;i<sizeX+1;i++){
//				wc.x = wc_center.x + deltaX*i;
//				if((wc.x>=domainOriginX)&&(wc.x<=domainSizeX)){
//					for(int j=-sizeY;j<sizeY+1;j++){
//						wc.y = wc_center.y + deltaY*j;
//						if((wc.y>=domainOriginY)&&(wc.y<=domainSizeY)){
//							long currVolumeIndex = getVolumeIndex(wc);
//							currHandle = pVolumeElement[currVolumeIndex].feature->getHandle();
//							if(currHandle==handle){
//								addElementToVolumeList(currVolumeIndex, contourElement);
//							}  
//						}
//					}
//				}  
//			}
//		} else if(getDimension()==3){
//			double deltaX = getXScale_um();
//			double deltaY = getYScale_um();
//			double deltaZ = getZScale_um();
//			int sizeX = (int)ceil(distance/deltaX);
//			int sizeY = (int)ceil(distance/deltaY);
//			int sizeZ = (int)ceil(distance/deltaZ);
//
//			for(int i=-sizeX;i<sizeX+1;i++){
//				wc.x = wc_center.x + deltaX*i;
//				if((wc.x>=domainOriginX)&&(wc.x<=domainSizeX)){
//					for(int j=-sizeY;j<sizeY+1;j++){
//						wc.y = wc_center.y + deltaY*j;
//						if((wc.y>=domainOriginY)&&(wc.y<=domainSizeY)){
//							for(int k=-sizeZ;k<sizeZ+1;k++){
//								wc.z = wc_center.z + deltaZ*k;
//								if((wc.z>=domainOriginZ)&&(wc.z<=domainSizeZ)){
//									long currVolumeIndex = getVolumeIndex(wc);
//									currHandle = pVolumeElement[currVolumeIndex].feature->getHandle();
//									if(currHandle==handle){
//										addElementToVolumeList(currVolumeIndex, contourElement);
//									}  
//								}  
//							}
//						}
//					}  
//				}
//			}	
//		}
//	}
//}

void CartesianMesh::initialize(VCellModel* model, istream& ifs)
{
	if (pVolumeElement!=nullptr) {
		return;
	}

	readGeometryFile(model, ifs);
	initScale();

	//sampleContours();
	//setVolumeLists();

	printf("numVolume=%ld\n",numVolume);

	setBoundaryConditions();
	findMembraneNeighbors();
	adjustMembraneAreaFromNormal();
}


inline unsigned char decimalValue(unsigned char hex) {
	if (hex == 0) {
		return 0;
	}
	assert( (hex >= '0' && hex <= '9')
		|| (hex >= 'a' && hex <= 'f')
		|| (hex >= 'A' && hex <= 'F') );
	if (hex <= '9') {
		return hex - '0';
	}
	if (hex >= 'a') {
		return hex - 'a' + 10;
	}
	assert (hex >= 'A');
	return hex - 'A' + 10;
}

inline unsigned char fromHex(unsigned char* src) {
	const unsigned char v = 16 *decimalValue(src[0]) + decimalValue(src[1]);
	return  v; 
}

/*
unsigned char fromHex(unsigned char* src) {
char chs[5];
chs[0] = '0';
chs[1] = 'x';
chs[2] = src[0];
chs[3] = src[1];
chs[4] = 0;
int v;
sscanf(chs, "%x", &v);
unsigned char alt = fromHex2(src);
assert (alt == v);
return (unsigned char)v;
}
*/
void CartesianMesh::readGeometryFile(VCellModel *model, istream& ifs) {
	stringstream ss;

	string line;
	string name;
	// name
	getline(ifs, line);
	// dimension
	getline(ifs, line);
	ss.clear();
	ss.str(line);
	ss >> name >> dimension;
	//size
	getline(ifs, line);
	ss.clear();
	ss.str(line);
	switch (dimension) {
	case 1:			
		ss >> name >> domainSizeX;
		domainSizeY = 1;
		domainSizeZ = 1;
		break;
	case 2:
		ss >> name >>  domainSizeX >> domainSizeY;
		domainSizeZ = 1;
		break;
	case 3:
		ss >> name >> domainSizeX >> domainSizeY >> domainSizeZ;
		break;
	default:
		throw std::runtime_error("CartesianMesh::readGeometryFile() : invalid dimension");
	}
	//origin
	getline(ifs, line);
	ss.clear();
	ss.str(line);
	switch (dimension) {
	case 1:
		ss >> name >> domainOriginX;
		domainOriginY = 0;
		domainOriginZ = 0;
		membraneInfo.nDirections = 0;
		membraneInfo.oppositeDirection= 0;
		break;
	case 2:
		ss >> name >> domainOriginX >> domainOriginY;
		domainOriginZ = 0;
		membraneInfo.nDirections = 2;
		membraneInfo.oppositeDirection= 1;
		break;
	case 3:
		ss >> name >> domainOriginX >> domainOriginY >> domainOriginZ;
		membraneInfo.nDirections = 4;
		membraneInfo.oppositeDirection= 2;
		break;
	default:
		throw std::runtime_error("CartesianMesh::readGeometryFile() : invalid dimension");
	}

	//volumeRegions
	getline(ifs, line);
	int numVolumeRegions;
	ss.clear();
	ss.str(line);
	ss >> name >> numVolumeRegions;

	for (int i = 0; i < numVolumeRegions; i ++) {
		int fhi;
		double volume;
		getline(ifs, line);
		ss.clear();
		ss.str(line);
		ss >> name >> volume >> fhi;
		auto fh = (FeatureHandle)fhi;
		Feature *feature = model->getFeatureFromHandle(fh);
		auto* vr = new VolumeRegion(i, name, this, feature);
		vr->setSize(volume);

		feature->addRegion(vr);
		pVolumeRegions.push_back(vr);
	}
	//membraneRegions
	getline(ifs, line);
	int numMembraneRegions;
	ss.clear();
	ss.str(line);
	ss >> name >> numMembraneRegions;

	for (int i = 0; i < numMembraneRegions; i ++) {
		int inside, outside;
		double area;
		getline(ifs, line);
		ss.clear();
		ss.str(line);
		ss >> name >> area >> inside >> outside;
		VolumeRegion *volRegion1 = pVolumeRegions[inside];
		VolumeRegion *volRegion2 = pVolumeRegions[outside];
		Membrane* membrane = model->getMembrane(volRegion1->getFeature(), volRegion2->getFeature());
		MembraneRegion* mr = new MembraneRegion(i, name, this, membrane, volRegion1, volRegion2);
		mr->setSize(area);

		membrane->addRegion(mr);
		volRegion1->addMembraneRegion(mr);
		volRegion2->addMembraneRegion(mr);
		pMembraneRegions.push_back(mr);
	}

	//volumeSamples
	getline(ifs, line);
	ss.clear();
	ss.str(line);
	switch (dimension) {
	case 1:
		ss >> name >> numX;
		numY = 1;
		numZ = 1;
		break;
	case 2:
		ss >> name >>  numX >> numY;
		numZ = 1;
		break;
	case 3:
		ss >> name >> numX >> numY >> numZ;
		break;
	}

	numXY = numX * numY;
	numVolume = numXY * numZ;

	//volumeSamples compressed, changed from byte to short
	int twiceNumVolume = 2 * numVolume;
	auto* bytes_from_compressed = new unsigned char[twiceNumVolume + 1000];
	memset(bytes_from_compressed, 0, (twiceNumVolume + 1000) * sizeof(unsigned char));

	auto* compressed_hex = new unsigned char[twiceNumVolume + 1000];
	memset(compressed_hex, 0, (twiceNumVolume + 1000) * sizeof(unsigned char));
	ifs.getline((char*)compressed_hex, twiceNumVolume + 1000);	
	const std::streamsize line_len = ifs.gcount()  - 1; //don't count null termination character

	//convert from std type to zlib compress
//	assert( line_len < std::numeric_limits<uLong>::max( ));
	const uLong compressed_len = static_cast<uLong>(line_len); 

	if (compressed_len <= 1) {
		throw "CartesianMesh::readGeometryFile() : invalid compressed volume";
	}

	for (uLong i = 0, j = 0; i < compressed_len; i += 2, j ++) {
		bytes_from_compressed[j] = fromHex(compressed_hex + i);
	}

	auto* inflated_bytes = new unsigned char[twiceNumVolume + 1];
	memset(inflated_bytes, 0, (twiceNumVolume + 1) * sizeof(unsigned char));

	unsigned long inflated_len = twiceNumVolume;
	int retVal = uncompress(inflated_bytes, &inflated_len, bytes_from_compressed, compressed_len/2);

	auto* volsamples = new unsigned short[numVolume];
	if (inflated_len == numVolume) {
		for (unsigned long i = 0; i < inflated_len; i ++) {		
			volsamples[i] = inflated_bytes[i];
		}
	} else if (inflated_len == twiceNumVolume) {
		// convert two bytes to short
		for (unsigned long i = 0, j = 0; i < inflated_len; i += 2, j ++) {
			volsamples[j] = inflated_bytes[i] | (inflated_bytes[i + 1] << 8);
		}
	} else {
		throw "CartesianMesh : unexpected number of volume samples";
	}

	pVolumeElement = new VolumeElement[numVolume];
	for(int i = 0; i < numVolume; i ++){
		int regionIndex = volsamples[i];
		VolumeRegion* vr = pVolumeRegions[regionIndex];
		vr->addElementIndex(i);
		pVolumeElement[i].neighborMask = 0;
		pVolumeElement[i].region = vr;
	}

	delete[] compressed_hex;
	delete[] bytes_from_compressed;
	delete[] inflated_bytes;
	delete[] volsamples;

	// cells
	getline(ifs, line);
	ss.clear();
	ss.str(line);
	ss >> name >> numMembrane;
	pMembraneElement = new MembraneElement[numMembrane];

	// cell data
	WorldCoord smoothedCoord;
	for (int i = 0; i < numMembrane; i ++) {
		getline(ifs, line);
		int cellID;

		MembraneElement& memElement = pMembraneElement[i];
		ss.clear();
		ss.str(line);
		ss >> cellID >> memElement.vindexFeatureLo >> memElement.vindexFeatureHi >> memElement.area 
			>> smoothedCoord.x >> smoothedCoord.y >> smoothedCoord.z 
			>> memElement.unitNormal.x >> memElement.unitNormal.y >> memElement.unitNormal.z;

		// sort
		if (pVolumeElement[memElement.vindexFeatureLo].getFeatureIndex() > pVolumeElement[memElement.vindexFeatureHi].getFeatureIndex()) {
			int ti = memElement.vindexFeatureLo;
			memElement.vindexFeatureLo = memElement.vindexFeatureHi;
			memElement.vindexFeatureHi = ti;
		}

		memElement.index = i;

		// for the purpose of finding neighbors of membrane elements later 
		pVolumeElement[memElement.vindexFeatureLo].adjacentMembraneIndexes.push_back(memElement.index);
		pVolumeElement[memElement.vindexFeatureHi].adjacentMembraneIndexes.push_back(memElement.index);

		// add membrane element to membrane region
		for (int j = 0; j < (int)pMembraneRegions.size(); j ++) {
			if (pMembraneRegions[j]->inBetween(pVolumeElement[memElement.vindexFeatureLo].region, 
				pVolumeElement[memElement.vindexFeatureHi].region)) {
					memElement.region = pMembraneRegions.at(j);
					pMembraneRegions[j]->addElementIndex(memElement.index);
					break;
			}
		}

		// try to figure out far for lo and hi and neighbor mask of volume elements
		int offsets[3];
		offsets[0] = 1;
		offsets[1] = numX;
		offsets[2] = numXY;
		for (int j = 0; j < dimension; j ++) {
			int diff = memElement.vindexFeatureHi - memElement.vindexFeatureLo;
			if (diff == offsets[j]) {  // hi is bigger, hi far is hi + offset, lo far is lo - offset
				int lofar = memElement.vindexFeatureLo - offsets[j];
				int hifar = memElement.vindexFeatureHi + offsets[j];
				memElement.vindexFeatureLoFar = lofar >= 0 && pVolumeElement[lofar].getFeature() == pVolumeElement[memElement.vindexFeatureLo].getFeature() ? lofar : -1;
				memElement.vindexFeatureHiFar = hifar < numVolume && pVolumeElement[hifar].getFeature() == pVolumeElement[memElement.vindexFeatureHi].getFeature() ? hifar : -1;
				switch (j) {
				case 0:
					pVolumeElement[memElement.vindexFeatureHi].neighborMask |= NEIGHBOR_XM_MEMBRANE;
					pVolumeElement[memElement.vindexFeatureLo].neighborMask |= NEIGHBOR_XP_MEMBRANE;
					break;
				case 1:
					pVolumeElement[memElement.vindexFeatureHi].neighborMask |= NEIGHBOR_YM_MEMBRANE;
					pVolumeElement[memElement.vindexFeatureLo].neighborMask |= NEIGHBOR_YP_MEMBRANE;
					break;
				case 2:
					pVolumeElement[memElement.vindexFeatureHi].neighborMask |= NEIGHBOR_ZM_MEMBRANE;
					pVolumeElement[memElement.vindexFeatureLo].neighborMask |= NEIGHBOR_ZP_MEMBRANE;
					break;
				}
				break;
			} else if (diff == -offsets[j]) {  // lo is bigger, lo far is lo + offset, hi far is hi - offset
				int lofar = memElement.vindexFeatureLo + offsets[j];
				int hifar = memElement.vindexFeatureHi - offsets[j];
				memElement.vindexFeatureLoFar = lofar < numVolume && pVolumeElement[lofar].getFeature() == pVolumeElement[memElement.vindexFeatureLo].getFeature() ? lofar : -1;
				memElement.vindexFeatureHiFar = hifar >= 0 && pVolumeElement[hifar].getFeature() == pVolumeElement[memElement.vindexFeatureHi].getFeature() ? hifar : -1;
				switch (j) {
				case 0:
					pVolumeElement[memElement.vindexFeatureLo].neighborMask |= NEIGHBOR_XM_MEMBRANE;
					pVolumeElement[memElement.vindexFeatureHi].neighborMask |= NEIGHBOR_XP_MEMBRANE;
					break;
				case 1:
					pVolumeElement[memElement.vindexFeatureLo].neighborMask |= NEIGHBOR_YM_MEMBRANE;
					pVolumeElement[memElement.vindexFeatureHi].neighborMask |= NEIGHBOR_YP_MEMBRANE;
					break;
				case 2:
					pVolumeElement[memElement.vindexFeatureLo].neighborMask |= NEIGHBOR_ZM_MEMBRANE;
					pVolumeElement[memElement.vindexFeatureHi].neighborMask |= NEIGHBOR_ZP_MEMBRANE;
					break;
				}
				break;
			}
		} 
	}
}

void CartesianMesh::setBoundaryConditions()
{
	switch (dimension){
	case 1:{			
		long index;
		int boundaryType;

		index = 0;
		pVolumeElement[index].region->setAdjacentToBoundary();
		pVolumeElement[index].neighborMask |= NEIGHBOR_XM_BOUNDARY;
		boundaryType = pVolumeElement[index].getFeature()->getXmBoundaryType();
		if (boundaryType == BOUNDARY_VALUE){
			pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_DIRICHLET;
			pVolumeElement[index].region->setBoundaryDirichlet();
		} else if (boundaryType == BOUNDARY_PERIODIC) {
			pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_PERIODIC;
		} else if (boundaryType == BOUNDARY_FLUX) {
			pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_NEUMANN;
		}
		pVolumeElement[index].neighborMask |= VOLUME_HALF;

		index = numX - 1;
		pVolumeElement[index].region->setAdjacentToBoundary();
		pVolumeElement[index].neighborMask |= NEIGHBOR_XP_BOUNDARY;
		boundaryType = pVolumeElement[index].getFeature()->getXpBoundaryType();
		if (boundaryType == BOUNDARY_VALUE){
			pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_DIRICHLET;
			pVolumeElement[index].region->setBoundaryDirichlet();
		} else if (boundaryType == BOUNDARY_PERIODIC) {
			pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_PERIODIC;
		} else if (boundaryType == BOUNDARY_FLUX) {
			pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_NEUMANN;
		}
		pVolumeElement[index].neighborMask |= VOLUME_HALF;

		break;
		   }
	case 2: {
		int boundaryType;
		long index;
		// set volume mask and boundary type for X boundaries.
		int xBoundary[2];
		xBoundary[0] = 0;
		xBoundary[1] = numX - 1;
		int j = 0;
		for (j = 0; j < numY; j ++){
			for (int i = 0; i < 2; i ++ ){
				index = j * numX + xBoundary[i];
				pVolumeElement[index].region->setAdjacentToBoundary();
				if (i == 0) {
					pVolumeElement[index].neighborMask |= NEIGHBOR_XM_BOUNDARY;
					boundaryType = pVolumeElement[index].getFeature()->getXmBoundaryType();
				} else {
					pVolumeElement[index].neighborMask |= NEIGHBOR_XP_BOUNDARY;
					boundaryType = pVolumeElement[index].getFeature()->getXpBoundaryType();
				}
				if (boundaryType == BOUNDARY_VALUE){
					pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_DIRICHLET;
					pVolumeElement[index].region->setBoundaryDirichlet();
				} else if (boundaryType == BOUNDARY_PERIODIC) {
					pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_PERIODIC;
				} else if (boundaryType == BOUNDARY_FLUX) {
					pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_NEUMANN;
				}	
				if (j == 0 || j == numY - 1) {
					pVolumeElement[index].neighborMask |= VOLUME_QUARTER;
				} else {
					pVolumeElement[index].neighborMask |= VOLUME_HALF;
				}
			} // end i
		} // end j

		// set volume mask and boundary type for Y boundaries.
		int yBoundary[2];
		yBoundary[0] = 0;
		yBoundary[1] = numY - 1;
		for (j = 0; j < 2; j ++){				
			for (int i = 0 ; i < numX; i ++){
				index = yBoundary[j] * numX + i;
				pVolumeElement[index].region->setAdjacentToBoundary();
				if (j == 0) {						
					pVolumeElement[index].neighborMask |= NEIGHBOR_YM_BOUNDARY;
					boundaryType = pVolumeElement[index].getFeature()->getYmBoundaryType();
				} else {
					pVolumeElement[index].neighborMask |= NEIGHBOR_YP_BOUNDARY;
					boundaryType = pVolumeElement[index].getFeature()->getYpBoundaryType();
				}
				if (boundaryType == BOUNDARY_VALUE){
					pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_DIRICHLET;
					pVolumeElement[index].region->setBoundaryDirichlet();
				} else if (boundaryType == BOUNDARY_PERIODIC) {
					pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_PERIODIC;
				} else if (boundaryType == BOUNDARY_FLUX) {
					pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_NEUMANN;
				}
				if (i == 0 || i == numX - 1) {
					pVolumeElement[index].neighborMask |= VOLUME_QUARTER;
				} else {
					pVolumeElement[index].neighborMask |= VOLUME_HALF;
				}
			}// end i
		}// end j
		break;
			}
	case 3:{
		long index = 0;
		int boundaryCount;
		int boundaryType;

		// X direction
		int xBoundary[2];
		xBoundary[0] = 0;
		xBoundary[1] = numX - 1;
		int k = 0;
		for (k = 0; k < numZ; k ++){
			for (int j = 0; j < numY; j ++){
				for (int i = 0; i < 2; i ++){						
					boundaryCount = 1;
					if (k == 0 || k == numZ - 1) {
						boundaryCount ++;
					}
					if (j == 0 || j == numY - 1) {
						boundaryCount ++;
					}
					index = k * numXY + j * numX + xBoundary[i];
					pVolumeElement[index].region->setAdjacentToBoundary();
					if (i == 0) {
						pVolumeElement[index].neighborMask |= NEIGHBOR_XM_BOUNDARY;
						boundaryType = pVolumeElement[index].getFeature()->getXmBoundaryType();
					} else {
						pVolumeElement[index].neighborMask |= NEIGHBOR_XP_BOUNDARY;
						boundaryType = pVolumeElement[index].getFeature()->getXpBoundaryType();
					}

					if (boundaryType == BOUNDARY_VALUE){
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_DIRICHLET;
						pVolumeElement[index].region->setBoundaryDirichlet();
					} else if (boundaryType == BOUNDARY_PERIODIC) {
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_PERIODIC;
					} else if (boundaryType == BOUNDARY_FLUX) {
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_NEUMANN;
					}
					switch (boundaryCount){
					case 1:
						pVolumeElement[index].neighborMask |= VOLUME_HALF;
						break;
					case 2:
						pVolumeElement[index].neighborMask |= VOLUME_QUARTER;
						break;
					case 3:
						pVolumeElement[index].neighborMask |= VOLUME_EIGHTH;
						break;
					}
				}
			}
		}
		// Y direction
		int yBoundary[2];
		yBoundary[0] = 0;
		yBoundary[1] = numY - 1;
		for (k = 0; k < numZ; k ++){
			for (int j = 0; j < 2; j ++){
				for (int i = 0; i < numX; i ++){						
					boundaryCount = 1;
					if (k == 0 || k == numZ - 1) {
						boundaryCount ++;
					}
					if (i == 0 || i == numX - 1) {
						boundaryCount ++;
					}
					index = k * numXY + yBoundary[j] * numX + i;
					pVolumeElement[index].region->setAdjacentToBoundary();
					if (j == 0) {
						pVolumeElement[index].neighborMask |= NEIGHBOR_YM_BOUNDARY;
						boundaryType = pVolumeElement[index].getFeature()->getYmBoundaryType();
					} else {
						pVolumeElement[index].neighborMask |= NEIGHBOR_YP_BOUNDARY;
						boundaryType = pVolumeElement[index].getFeature()->getYpBoundaryType();
					}

					if (boundaryType == BOUNDARY_VALUE){
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_DIRICHLET;
						pVolumeElement[index].region->setBoundaryDirichlet();
					} else if (boundaryType == BOUNDARY_PERIODIC) {
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_PERIODIC;
					} else if (boundaryType == BOUNDARY_FLUX) {
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_NEUMANN;
					}
					switch (boundaryCount){
					case 1:
						pVolumeElement[index].neighborMask |= VOLUME_HALF;
						break;
					case 2:
						pVolumeElement[index].neighborMask |= VOLUME_QUARTER;
						break;
					case 3:
						pVolumeElement[index].neighborMask |= VOLUME_EIGHTH;
						break;
					}
				}
			}
		}

		// Z direction
		int zBoundary[2];
		zBoundary[0] = 0;
		zBoundary[1] = numZ - 1;
		for (k = 0; k < 2; k ++){
			for (int j = 0; j < numY; j ++){
				for (int i = 0; i < numX; i ++){						
					boundaryCount = 1;
					if (j == 0 || j == numY - 1) {
						boundaryCount ++;
					}
					if (i == 0 || i == numX - 1) {
						boundaryCount ++;
					}
					index = zBoundary[k] * numXY + j * numX + i;
					pVolumeElement[index].region->setAdjacentToBoundary();
					if (k == 0) {
						pVolumeElement[index].neighborMask |= NEIGHBOR_ZM_BOUNDARY;
						boundaryType = pVolumeElement[index].getFeature()->getZmBoundaryType();
					} else {
						pVolumeElement[index].neighborMask |= NEIGHBOR_ZP_BOUNDARY;
						boundaryType = pVolumeElement[index].getFeature()->getZpBoundaryType();
					}

					if (boundaryType == BOUNDARY_VALUE){
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_DIRICHLET;
						pVolumeElement[index].region->setBoundaryDirichlet();
					} else if (boundaryType == BOUNDARY_PERIODIC) {
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_PERIODIC;
					} else if (boundaryType == BOUNDARY_FLUX) {
						pVolumeElement[index].neighborMask |= BOUNDARY_TYPE_NEUMANN;
					}
					switch (boundaryCount){
					case 1:
						pVolumeElement[index].neighborMask |= VOLUME_HALF;
						break;
					case 2:
						pVolumeElement[index].neighborMask |= VOLUME_QUARTER;
						break;
					case 3:
						pVolumeElement[index].neighborMask |= VOLUME_EIGHTH;
						break;
					}
				}// end i
			}// end j
		}// end k
		break;
		   }
	}// end switch (getDimension())

	cout << setprecision(10) << endl;
	for (int i = 0; i < getNumVolumeRegions(); i ++) {
		if (!pVolumeRegions[i]->isBoundaryDirichlet()) {
			int volIndex = pVolumeRegions[i]->getElementIndex(0);
			WorldCoord wc = getVolumeWorldCoord(volIndex);
			pVolumeElement[volIndex].neighborMask |= ELLIPTIC_PINNED;			
		}
	}
}

VolumeRegion* CartesianMesh::getVolumeRegion(int i){
	if ((i >= (int)pVolumeRegions.size()) || (i < 0)) {
		throw "CartesianMesh::getVolumeRegion(), index out of bound";
	}
	return pVolumeRegions[i]; 
}

MembraneRegion* CartesianMesh::getMembraneRegion(int i){
	if ((i >= (int)pMembraneRegions.size()) || (i <0)) {
		throw "CartesianMesh::getMembraneRegion(), index out of bound";
	}
	return pMembraneRegions[i]; 
}

double CartesianMesh::getVolumeOfElement_cu(long volumeIndex)
{
	int mask = pVolumeElement[volumeIndex].neighborMask;
	double VOLUME = getVolume_cu();
	if(!(mask&VOLUME_MASK)){
		return VOLUME;
	}else{
		return VOLUME/(mask&VOLUME_MASK);
	}
}

void CartesianMesh::initScale()
{
	const double decimeter_per_um=1.0/100000.0;            

	scaleX_um = (numX>1) ? (domainSizeX/(numX-1)) : domainSizeX;
	scaleY_um = (numY>1) ? (domainSizeY/(numY-1)) : domainSizeY;
	scaleZ_um = (numZ>1) ? (domainSizeZ/(numZ-1)) : domainSizeZ;

	areaX_squm = scaleY_um * scaleZ_um;
	areaY_squm = scaleX_um * scaleZ_um;
	areaZ_squm = scaleX_um * scaleY_um;

	volume_cu = (scaleX_um*scaleY_um*scaleZ_um);
}

void CartesianMesh::showSummary(FILE *fp)
{
	fprintf(fp,"Cartesian Mesh: Domain    x:[%lg,%lg] y:[%lg,%lg] z:[%lg,%lg]\n",
		domainOriginX, domainOriginX+ domainSizeX, 
		domainOriginY, domainOriginY+ domainSizeY, 
		domainOriginZ, domainOriginZ+ domainSizeZ);
	fprintf(fp,"                Elements  numX=%d numY=%d numZ=%d\n\n",
		numX,numY,numZ);
}

void CartesianMesh::write(FILE *fp)
{
	//
	// 'Version 1.1' added membrane connectivity
	// 'Version 1.2' added regions
	//
	fprintf(fp,"Version 1.2\n");
	fprintf(fp,"CartesianMesh {\n");//Begin CartesianMesh
	//
	writeCartesianMeshHeader(fp);
	fprintf(fp,"\n");
	writeVolumeRegionsMapSubvolume(fp);
	fprintf(fp,"\n");
	writeMembraneRegionMapVolumeRegion(fp);
	fprintf(fp,"\n");
	writeVolumeElementsMapVolumeRegion(fp);
	fprintf(fp,"\n");
	writeMembraneElements_Connectivity_Region(fp);
	//fprintf(fp,"\n");
	//writeContourElements(fp);
	//
	fprintf(fp,"}\n");//End CartesianMesh
}

void CartesianMesh::writeVolumeRegionsMapSubvolume(FILE *fp)
{
	int numVolumeRegions = (int)pVolumeRegions.size();
	fprintf(fp,"\tVolumeRegionsMapSubvolume {\n");
	fprintf(fp,"\t%d\n",numVolumeRegions);
	fprintf(fp,"\t//%8s %10s %10s\n","VolRegID","SubvolID","Volume");
	for(int c = 0;c < numVolumeRegions;c+= 1){
		VolumeRegion *volumeRegion = pVolumeRegions[c];
		fprintf(fp,"\t%10d %10d %10.17lg //%s\n",
			volumeRegion->getIndex(),
			volumeRegion->getFeature()->getHandle(),
			volumeRegion->getSize(),
			volumeRegion->getFeature()->getName().c_str());
	}
	fprintf(fp,"\t}\n");
}
void CartesianMesh::writeVolumeElementsMapVolumeRegion(FILE *fp)
{
	int numVolumeElements = getNumVolumeElements();
	VolumeElement *volumeElements = getVolumeElements();
	fprintf(fp,"\tVolumeElementsMapVolumeRegion {\n");
	if(numVolumeElements < 0){
		fprintf(fp,"\t%d UnCompressed",numVolumeElements);
		for(int c = 0;c < numVolumeElements;c+= 1){
			int volumeRegionId = volumeElements[c].getRegionIndex();
			if(c%numX == 0){
				fprintf(fp,"\n\t");
			}
			fprintf(fp,"%3ld ",volumeRegionId);
		}
	}else{
		fprintf(fp,"\t%d Compressed",numVolumeElements);
		unsigned char *src = new unsigned char[2 * numVolumeElements];
		for(int c = 0, b = 0; c < numVolumeElements * 2; b ++, c += 2){
			int volumeRegionId = volumeElements[b].getRegionIndex();
			src[c] = (unsigned char)(volumeRegionId & 0x000000ff);
			src[c + 1] = (unsigned char)((volumeRegionId & 0x0000ff00) >> 8);
		}
		unsigned long destLen = numVolumeElements * 2 * 2;
		if (destLen < 1000){
			destLen = 1000;  // minimum size compressed data buffer
		}
		unsigned char *dest = new unsigned char[destLen];
		int retVal = compress(dest, &destLen, src, numVolumeElements * 2);
		for(int c = 0; c < (int)destLen; c ++){
			if(c % 40 == 0){
				fprintf(fp,"\n\t");
			}
			fprintf(fp,"%2.2X",dest[c]);
		}
		delete[] dest;
		delete[] src;
	}
	fprintf(fp,"\n\t}\n");
}

void CartesianMesh::writeMembraneRegionMapVolumeRegion(FILE *fp)
{
	int numMembraneRegions = (int)pMembraneRegions.size();
	fprintf(fp,"\tMembraneRegionsMapVolumeRegion {\n");
	fprintf(fp,"\t%d\n",numMembraneRegions);
	fprintf(fp,"\t//%8s %10s %10s %10s\n","MemRegID","VolReg1","VolReg2","Surface");
	for(int c = 0;c < numMembraneRegions;c+= 1){
		MembraneRegion *membraneRegion = pMembraneRegions[c];
		VolumeRegion *vr1 = membraneRegion->getVolumeRegion1();
		VolumeRegion *vr2 = membraneRegion->getVolumeRegion2();
		fprintf(fp,"\t%10d %10d %10d %10.17lg\n",
			membraneRegion->getIndex(),
			vr1->getIndex(),
			vr2->getIndex(),
			membraneRegion->getSize());
	}
	fprintf(fp,"\t}\n");
}

void CartesianMesh::writeCartesianMeshHeader(FILE *fp)
{
	fprintf(fp,"\t%s     %10s %10s %10s\n","//","X","Y","Z");
	fprintf(fp,"\tSize   %10d %10d %10d\n",numX,numY,numZ);
	fprintf(fp,"\tExtent %10.17lg %10.17lg %10.17lg\n",domainSizeX,domainSizeY,domainSizeZ);
	fprintf(fp,"\tOrigin %10.17lg %10.17lg %10.17lg\n",domainOriginX,domainOriginY,domainOriginZ);
}

void CartesianMesh::writeMeshMetrics(FILE *fp)
{
	fprintf(fp,"MembraneElements {\n");
	fprintf(fp,"%d\n",(int)getNumMembraneElements());
	fprintf(fp,"%5s %11s %17s %25s %25s %25s %25s %25s %25s\n","Index","RegionIndex","X","Y","Z","Area","Nx","Ny","Nz");
	for (int i=0;i<getNumMembraneElements();i++){
		MembraneElement *memEl = pMembraneElement + i;
		WorldCoord wc = getMembraneWorldCoord(i);
		fprintf(fp,"%-5ld %11ld %25.17lg %25.17lg %25.17lg %25.17lg %25.17lg %25.17lg %25.17lg\n",
			memEl->index,
			memEl->getRegionIndex(),
			wc.x,
			wc.y,
			wc.z,
			memEl->area,
			memEl->unitNormal.x,
			memEl->unitNormal.y,
			memEl->unitNormal.z);
	}
	fprintf(fp,"}");	
}

namespace {
	/**
	 * output NeighborStatus in format expected by Java client ; index value or -1 if invalid
	 */
	inline signed long clientAnnotatedOutput(const StatusIndex<long,NeighborType::NeighborStatus> &si) {
		if (si.valid( )) {
			return si.asSignedLong( );
		}
		return -1;
	}
}

void CartesianMesh::writeMembraneElements_Connectivity_Region(FILE *fp)
{
	fprintf(fp,"\tMembraneElements {\n");
	fprintf(fp,"\t%d\n",(int)getNumMembraneElements());
	fprintf(fp,"\t//%4s %4s %4s %4s %4s %4s %4s %4s\n","Indx","Vol1","Vol2","Conn0","Conn1","Conn2","Conn3","MemRegID");
	for (int i=0;i<getNumMembraneElements();i++){
		MembraneElement *memEl = getMembraneElements()+i;
		fprintf(fp,"\t%6ld %4ld %4ld %5ld %5ld %5ld %5ld %8d\n",
			memEl->index,
			memEl->vindexFeatureLo,
			memEl->vindexFeatureHi,
			clientAnnotatedOutput(memEl->neighborMEIndex[0]),
			clientAnnotatedOutput(memEl->neighborMEIndex[1]),
			clientAnnotatedOutput(memEl->neighborMEIndex[2]),
			clientAnnotatedOutput(memEl->neighborMEIndex[3]),
			memEl->getRegionIndex());
	}
	fprintf(fp,"\t}\n");
}

//void CartesianMesh::writeContourElements(FILE *fp)
//{
//	//
//	// write out contour elements (if present)
//	//
//	if (getNumContourElements()>0){
//		fprintf(fp,"\tContourElements {\n");
//		fprintf(fp,"\t\t%d\n",(int)getNumContourElements());
//		//
//		// index volumeIndex begin.x begin.y begin.z  end.x, end.y end.z neighbor(prev) neighbor(next)
//		//
//		for (int i = 0; i < getNumContourElements(); i++){
//			ContourElement *cEl = getContourElement(i);
//			int neighborPrev = -1;
//			int neighborNext = -1;
//			if (cEl->getBorder() == CONTOUR_BEGIN){
//				neighborPrev = -1;
//				neighborNext = i+1;
//			}else if (cEl->getBorder() == CONTOUR_END){
//				neighborPrev = i-1;
//				neighborNext = -1;
//			}else if (cEl->getBorder() == CONTOUR_INTERIOR){
//				neighborPrev = i-1;
//				neighborNext = i+1;
//			}else{
//				char errMsg[512];
//				sprintf(errMsg, "Error writing contour mesh, contour element(%ld) has an illegal ContourBorder type = %d\n",i,cEl->getBorder());
//				throw errMsg;
//			}
//			fprintf(fp,"\t\t%ld %ld %lg %lg %lg %lg %lg %lg %ld %ld\n",cEl->getElementIndex(),cEl->getVolumeIndex(),
//						cEl->getBegin().x,cEl->getBegin().y,cEl->getBegin().z,
//						cEl->getEnd().x,  cEl->getEnd().y,  cEl->getEnd().z,
//						neighborPrev, neighborNext);
//		}
//		fprintf(fp,"\t}\n");  // end ContourElements
//	}
//}


UnitVector3 CartesianMesh::unitVectorBetween(long volumeIndexFrom, long volumeIndexTo) {
	MeshCoord from = getMeshCoord(volumeIndexFrom);
	MeshCoord to = getMeshCoord(volumeIndexTo);
	MeshCoord diff = to - from; 
	double x = diff.x * scaleX_um;
	double y = diff.y * scaleY_um;
	double z = diff.z * scaleZ_um;
	return UnitVector3(x,y,z);
}

WorldCoord CartesianMesh::getVolumeWorldCoord(long volumeIndex)
{
	MeshCoord mc = getMeshCoord(volumeIndex);

	//unit.x = (numX>1)?(((double)(mc.x))/(numX-1)):0.5;
	//double unity = (numY>1)?(((double)(mc.y))/(numY-1)):0.5;
	//double unitz = (numZ>1)?(((double)(mc.z))/(numZ-1)):0.5;

	double wcx = domainOriginX + mc.x * scaleX_um;
	double wcy = domainOriginY + (numY > 1 ? mc.y * scaleY_um : domainSizeY * 0.5);
	double wcz = domainOriginZ + (numZ > 1 ? mc.z * scaleZ_um : domainSizeZ * 0.5);

	return WorldCoord(wcx, wcy, wcz);
}

WorldCoord CartesianMesh::getMembraneWorldCoord(long membraneIndex)
{
	return getMembraneWorldCoord(pMembraneElement + membraneIndex);
}

WorldCoord CartesianMesh::getMembraneWorldCoord(MembraneElement *element)
{
	WorldCoord wc_lo = getVolumeWorldCoord(element->vindexFeatureLo);
	WorldCoord wc_hi = getVolumeWorldCoord(element->vindexFeatureHi);

	return WorldCoord(
		(wc_lo.x + wc_hi.x)/2.0, 
		(wc_lo.y + wc_hi.y)/2.0, 
		(wc_lo.z + wc_hi.z)/2.0
		);
}

long CartesianMesh::getVolumeIndex(WorldCoord coord)
{
	MeshCoord meshCoord;

	ASSERTION(coord.x>=domainOriginX);
	ASSERTION(coord.x<=(domainOriginX + domainSizeX));
	ASSERTION(coord.y>=domainOriginY);
	ASSERTION(coord.y<=(domainOriginY + domainSizeY));
	ASSERTION(coord.z>=domainOriginZ);
	ASSERTION(coord.z<=(domainOriginZ + domainSizeZ));

	meshCoord.x = (int)((coord.x-domainOriginX) * (numX-1)/domainSizeX + 0.5);
	meshCoord.y = (int)((coord.y-domainOriginY) * (numY-1)/domainSizeY + 0.5);
	meshCoord.z = (int)((coord.z-domainOriginZ) * (numZ-1)/domainSizeZ + 0.5);

	return meshCoord.x + numX*(meshCoord.y + numY*meshCoord.z);
}


//long CartesianMesh::getVolumeIndex(MeshCoord coord)
//{
//	return coord.x + numX*(coord.y + numY*coord.z);
//}


double getExactVolume() {
#if (defined(SPHERE))
	return 4 * PI;

#elif (defined(QUARTERSPHERE))
	return PI/2;

#elif (defined(SQUARE))
	return 4;

#elif (defined(ELLIPSE)) 
	return 0;

#elif (defined(ELLIPSOID))
	double m = a2 * (b2 - c2)/(b2 * (a2 - c2)) ;
	double theta = asin(sqrt(1-c2/a2));

	int N = 10000000;

	double sum1 = 0;
	double sum2 = 0;
	for (int i = 0; i <= N; i ++) {
		double phi = (theta * i)/N;
		double sinphi =  sin(phi);
		double d = sqrt(1 - m * sinphi * sinphi);
		sum1 += d;
		sum2 += 1/d;
	}
	double EllipticE = sum1 * theta /(N+1);
	double EllipticF = sum2 * theta /(N+1);	

	double d = sqrt(a2 - c2);
	return 2 * PI * (c2 + b * c2 * EllipticF / d + b * d * EllipticE);

#elif (defined(HALFCUBE)) 
	return 1.0 * 1.0;
#elif (defined(CIRCLE)) 
	return 2 * PI;
#elif (defined(HALFCIRCLE)) 
	return PI;
#elif (defined(QUARTERCIRCLE))
	return PI/2;
#elif (defined(ROSE)) 
	return 19.2680242763864;
#elif (defined(CUBE_PLANE))
	return 0.62360956446232352;
#elif (defined(CYLINDER_IN_CUBE))
	return 8 * sqrt(6.0) * PI;
#elif (defined(PERPENDICULAR_CYLINDER)) 
	return 2 * PI * PI;
#endif
	return 0.0;
}

WorldCoord CartesianMesh::computeExactNormal(long meIndex) {
	MembraneElement& meptr = pMembraneElement[meIndex];
	WorldCoord wc = getMembraneWorldCoord(&meptr);
#if (defined(SPHERE) || defined(QUARTERSPHERE))
	wc.normalize();
	return wc;

#elif (defined(ROSE)) 
	return wc;
#elif (defined(APPLE))
	double R = sqrt(wc.x*wc.x + wc.y*wc.y) + 1E-12;
	double R3  = R*R*R;
	WorldCoord normal(wc.x/R -wc.y*wc.y/R3, wc.y/R + wc.x * wc.y/R3, 0);
	normal.normalize();
	return normal;

#elif ( defined(CIRCLE) || defined(HALFCIRCLE) || defined(QUARTERCIRCLE)) 
	wc.z = 0;
	wc.normalize();
	return wc;
#elif (defined(SQUARE)) 
	WorldCoord normal = getVolumeWorldCoord(meptr.outsideIndexNear) - getVolumeWorldCoord(meptr.insideIndexNear);
	normal.normalize();
	return normal;

#elif (defined(CYLINDER))
	if (meptr.outsideIndexNear - meptr.insideIndexNear == numXY) {
		return WorldCoord(0, 0, 1)
	} else if (meptr.outsideIndexNear - meptr.insideIndexNear == -numXY){
		return WorldCoord(0, 0, -1)
	} else {
		wc.z = 0;
		wc.normalize();
		return wc;
		}
#elif (defined(ELLIPSOID))
	WorldCoord normal(2 * wc.x/a2, 2 * wc.y/b2, 2 * wc.z/c2);
	normal.normalize();
	return normal;
#elif (defined(ELLIPSE)) 
	WorldCoord normal(2 * wc.x/4, 2 * wc.y/1, 0);
	normal.normalize();
	return normal;

#elif (defined(HALFCUBE))
	return WorldCoord(0,0,1);
#elif (defined(CUBE_PLANE))
	double ok = sqrt(14.0);
	return WorldCoord(-2/ok, -1/ok, 3/ok);
#elif (defined(CYLINDER_IN_CUBE))
	double xp = wc.x, yp = wc.y, zp = wc.z;
	double v1 = -2/sqrt(6.0), v2 = 1/sqrt(6.0), v3 = 1/sqrt(6.0);
	double lambdap= (-2*xp+yp+zp)/sqrt(6.0);
	double rp = sqrt(xp*xp+yp*yp+zp*zp - lambdap*lambdap);
	return WorldCoord( (xp-lambdap*v1)/rp , (yp-lambdap*v2)/rp , (zp-lambdap*v3)/rp );
#elif (defined(PERPENDICULAR_CYLINDER)) 
	double d = sqrt(wc.y * wc.y + wc.z * wc.z);
	return WorldCoord(0, wc.y/d, wc.z/d);
#else
	throw "Computing exact normals is not supported for arbitrary geometry!";
#endif	
}

void CartesianMesh::computeExactNormals() {
	cout << "CartesianMesh::computeExactNormals(), compute exact normals for special shape" << endl;
#if (defined(COMPUTE_EXACT_NORMALS) && defined(SPECIAL_SHAPE))
	for (int i = 0; i < numMembrane; i ++) {
		MembraneElement& meptr = pMembraneElement[i];
		meptr.unitNormal = computeExactNormal(i);
	}
#else
	throw "Computing exact normals is not supported for arbitrary geometry!";
#endif
}

CartesianMesh::NeighborIndex CartesianMesh::getNeighbor(int n,  long index, int neighbor)
{
	//end of recursion
	if ( n == 0) {
		return index;
	}
	assert(n > 0);
	/*
	if(getDimension() == 2){k=2;}
	if(getDimension() == 3){k=4;}
	*/
	n = n - 1;
	MembraneElement *pElement = getMembraneElements()+index;
	NeighborIndex neighborIndex = pElement->neighborMEIndex[neighbor];
	if(!neighborIndex.valid( )) {
		return neighborIndex;
	}
	int new_neighbor = -1;
	MembraneElement *pNeighbor = getMembraneElements()+neighborIndex;
	for(int direction=0; direction <membraneInfo.nDirections; direction++){
		//find neighbor's reference to us
		if((pNeighbor->neighborMEIndex[direction])==index){
			new_neighbor = (direction + membraneInfo.oppositeDirection) % membraneInfo.nDirections;
		}
	}
	if (new_neighbor < 0){
		return NeighborIndex(NeighborType::unknown);
	}
	return getNeighbor(n, neighborIndex, new_neighbor);
}

// modified from getNeighbor()
// starting form membrane point index, find the N-th membrane point 
// in the direction of a natural neighbor, following the curve.
// if it reaches N, leftOverN returns 0 and returnNeighbor is the N-th point
// if the curve ends before reaching N points, leftOverN returns a positive value <= N,
// and returnNeighbor is the last point which is (N-leftOverN)-th point
void CartesianMesh::findMembranePointInCurve(int N,  long index, int neighbor_dir, int& leftOverN, int& returnNeighbor)
{	
	//int numNeighbors = NumNaturalNeighbors_Membrane[dimension];
	leftOverN = N;
	N = N - 1;
	if (N < 0){		
		returnNeighbor = index;
		return;
	}

	NeighborIndex neighborIndex = pMembraneElement[index].neighborMEIndex[neighbor_dir];
	if (!neighborIndex.valid( )) {
		returnNeighbor = index;
		return;
	}
	int new_neighbor_dir;
	bool found  = false;
	for (int i = 0; !found && i < membraneInfo.nDirections; i ++){
		if (pMembraneElement[neighborIndex].neighborMEIndex[i] == index){
			new_neighbor_dir = oppositeMembraneDirection(i); 
			found = true;
		}
	}
	if (!found) {
		throw std::runtime_error("CartesianMesh::findMembranePointInCurve(), new_neighbor can never be < 0");
	}
	return findMembranePointInCurve(N, neighborIndex, new_neighbor_dir, leftOverN, returnNeighbor);
}

namespace {
	//review
	struct UnitHolder {
		bool valid; //do we want to use the corresponding unit vector?
		UnitVector3 unitVector;
		UnitHolder( )
			:valid(false),
			unitVector( ) {}
	};
}

inline bool CartesianMesh::computeNormalsFromNeighbors(long index) {
	//
	//averaging over neighbors of arbitrary number  
	//
	int tangentNeighbors[4];
	UnitHolder tangentWc[4];

	const int localN = 3;
	const double angleTol = PI/6;
	const double cosAngleTol = cos(angleTol);
	const double NFrac=0.5; // percent for wall , >50%
	const double NRelax = max<double>(fabs(NFrac),0.50);
	const double maxDotProductOfIndependentVectors = 0.99;

	WorldCoord wc = getMembraneWorldCoord(index);
	MembraneElement& meptr = pMembraneElement[index];

	//Hops or steps
	ArrayHolder<int,4> tangentN = getNormalApproximationHops(index);
	if (dimension == 3) {
		bool direction0Zero =  (tangentN[0] == 0) && (tangentN[0 + membraneInfo.oppositeDirection] == 0);
		bool direction1Zero =  (tangentN[1] == 0) && (tangentN[1 + membraneInfo.oppositeDirection] == 0);
		if (direction0Zero || direction1Zero) {
			return false;
		}
	}

	//check opposite pairs both zero && dim == 3
	// set  
	for (;;)  {
		int numOfValidTangentNeighbors = 0;

		int leftOverN[4];
		// first go through all the valid tangent neighbors
		for (int i = 0; i < membraneInfo.nDirections; i ++) {
			findMembranePointInCurve(tangentN[i], index, i, leftOverN[i], tangentNeighbors[i]);
			if (index == tangentNeighbors[i]) {
				continue;
			}

			// sanity check
			NeighborIndex oldGetNeighbor = getNeighbor(tangentN[i], index, i);
			assert((oldGetNeighbor.valid( )	&& tangentNeighbors[i] == oldGetNeighbor && leftOverN[i] == 0) 
				|| (!oldGetNeighbor.valid( ) && leftOverN[i] > 0));


			DoubleVector3 diff = wc - getMembraneWorldCoord(tangentNeighbors[i]);
			int newN = tangentN[i] - leftOverN[i];
			if (newN >= NRelax*tangentN[i] ) {
				numOfValidTangentNeighbors ++;
				tangentWc[i].valid = true;
				tangentWc[i].unitVector = diff;

				if (cosAngleTol > 0 && newN > localN) {
					int localTangentNeighbor = getNeighbor(localN, index, i);
					UnitVector3 localTangentWc = wc - getMembraneWorldCoord(localTangentNeighbor);
					double dotp = tangentWc[i].unitVector.dotProduct(localTangentWc); 
					if (fabs(dotp) <= cosAngleTol) {
						tangentWc[i].unitVector = localTangentWc;
					}
				}
			} // if (leftOverN[i]
		} // for (int i

		if (numOfValidTangentNeighbors == 0) {  // if there are no neighbors at all, use face normal
			meptr.unitNormal = unitVectorBetween( meptr.vindexFeatureLo, meptr.vindexFeatureHi);
			assert(meptr.unitNormal.lengthSquared( ) == 1);
			return true;
		} 

		// second go through all the invalid neighbors 
		// which is either point itself or < 0			
		for (int i = 0; i < membraneInfo.nDirections; i ++) {
			if (tangentWc[i].valid) {
				continue;
			}

			if (leftOverN[i] == tangentN[i]) {
				// point itself, cannot do more
				continue;
			} 
			// not enough points
			int opposite_i = oppositeMembraneDirection(i);

			// must have enough in opposite direction
			assert(tangentWc[opposite_i].valid);
			UnitVector3 tempWc = -tangentWc[opposite_i].unitVector;

			double dotp = tangentWc[i].unitVector.dotProduct(tempWc);
			if (fabs(dotp) > cosAngleTol) {
				tangentNeighbors[i] = -1;
				continue;
			}

			numOfValidTangentNeighbors ++;
			if (cosAngleTol > 0 && tangentN[i] - leftOverN[i] > localN) {
				int localTangentNeighbor = getNeighbor(localN, index, i);
				UnitVector3 localTangentWc = wc - getMembraneWorldCoord(localTangentNeighbor);
				double dotp = tangentWc[i].unitVector.dotProduct(localTangentWc); 
				if (fabs(dotp) <= cosAngleTol) {
					tangentWc[i].unitVector = localTangentWc;
				}
			} // if (cosAngleTol > 0
		} // for (int i

		//int tangentNormalCount = 0;
		UnitVector3 tangentNormals[4];
		int tangentNormalIndex = 0;

		// compute tangent normals
		if (dimension == 2) {
			const int lastOfTwo = 1;
			UnitHolder &uh1 =  tangentWc[0];
			if (uh1.valid) {
				UnitVector3 & first = uh1.unitVector;
				DoubleVector3 dv(first.yvalue( ), -first.xvalue( ),0);
				if (!dv.isAbsolutelyZero( )) {
					tangentNormals[tangentNormalIndex++] = dv;
				}
			}
			UnitHolder &uh2 =  tangentWc[lastOfTwo];
			if (uh2.valid) {
				UnitVector3 & last = uh2.unitVector;
				//sign flipped from first
				DoubleVector3 dv2(-last.yvalue( ), last.xvalue( ),0);
				if (!dv2.isAbsolutelyZero( )) {
					tangentNormals[tangentNormalIndex++] = dv2;
				}
			}

		} else if (dimension == 3) {
			for (int i = 0; i < membraneInfo.nDirections; i ++) {
				int nextIndex = (i+1) % membraneInfo.nDirections;
				if ( tangentWc[i].valid &&  tangentWc[nextIndex].valid ) { 
					double dotp = tangentWc[i].unitVector.dotProduct(tangentWc[nextIndex].unitVector);
					if (fabs(dotp) <= maxDotProductOfIndependentVectors) { //ensure independent vectors
						DoubleVector3 crossProd = tangentWc[i].unitVector.crossProduct(tangentWc[nextIndex].unitVector);
						assert(!crossProd.isAbsolutelyZero( )); 
						tangentNormals[tangentNormalIndex++] = crossProd;
					}
				}
			}
		}

		// average tangent normals
		if (tangentNormalIndex > 0) {
			computeNormal(meptr, tangentNormals, tangentNormalIndex);
			return true;
		} 
		bool changed = false; 
		for (int i = 0; i < membraneInfo.nDirections; i ++) {
			if (tangentN[i] > 1) {
				tangentN[i] --;
				changed = true;
			}
		}
		if (!changed) {
			throw "Mesh is too coarse, found isolated membrane element, please try to refine mesh";
		}
	}		
}

#ifndef _NDEBUG
#define DEBUG_OUT(x) cout << x << endl;
#else
#define DEBUG_OUT(x) 
#endif

void CartesianMesh::computeNormalsFromNeighbors() {
	//
	//averaging over neighbors of arbitrary number  
	//
	if (dimension < 2) {
		return;
	}
	cout << "CartesianMesh::computeNormalsFromNeighbors(), compute normals from neighbors" << endl;

	std::set<long> insufficientNeighbors;
	for (long index = 0; index < numMembrane; index ++){
		if (!computeNormalsFromNeighbors(index)) {
			insufficientNeighbors.insert(index);
		}
	}

	// use average of neighbor normals, if not too far off of face normal
	const double angleTooFar = PI/6; //bigger than this is "too far"
	const double cosAngleTooFar = cos(angleTooFar);
	for (set<long>::const_iterator iter = insufficientNeighbors.begin( ); iter != insufficientNeighbors.end( ); ++iter) {
		DEBUG_OUT( "insufficient_neighbors index " << *iter)
		MembraneElement & mE = pMembraneElement[*iter];
		DoubleVector3 dv;
		for (int i = 0; i < membraneInfo.nDirections;i++) {
			if (mE.neighborMEIndex[i].valid( )) {
				long nIndex = mE.neighborMEIndex[i]; 
				if (insufficientNeighbors.count(nIndex) == 1) {
					continue; //don't use normal we already averaged
				}
				dv += pMembraneElement[nIndex].unitNormal;
			}
		}
		UnitVector3 faceNormal = unitVectorBetween( mE.vindexFeatureLo, mE.vindexFeatureHi);
		if (!dv.isAbsolutelyZero( )) {
			dv.normalize( );
			const double dp = faceNormal.dotProduct(dv);
			if (dp >= cosAngleTooFar) {
				mE.unitNormal = dv;
				DEBUG_OUT( "\t using average " << dv);
				continue;
			}
		}
		//dv was zero, or average too far from face normal
		mE.unitNormal = faceNormal; 
		DEBUG_OUT( "\t using face normal " << faceNormal);
	}
}

#undef DEBUG_OUT

void CartesianMesh::computeNormal(MembraneElement& meptr, const UnitVector3* normal, int numberOfNormals) {
	DoubleVector3 average;
	for (int i = 0; i < numberOfNormals; i ++) {
		average += normal[i];
	}
	average /= numberOfNormals;

	switch (dimension) {
	case 2: 
		average.z = 0;
		break;
	case 3: 
		break;
	default: 
		throw std::runtime_error("CartesianMesh::computeNormal(), dimension should be 2 or 3.");
	}

	// overwriting the normal computed by neighborhood if unnormalized length is much smaller than 1.
	// under this condition, the direction is meaningless and we use staircase normal.
	double normalLengthSquare = average.lengthSquared( );
	if (normalLengthSquare <= NORMAL_MINLENGTH_THRESHOLD * NORMAL_MINLENGTH_THRESHOLD) {	
		average = unitVectorBetween(meptr.vindexFeatureLo,meptr.vindexFeatureHi);
	} 
	meptr.unitNormal = average.normalize( );
}

void CartesianMesh::adjustMembraneAreaFromNormal(){

#if (defined(COMPUTE_EXACT_NORMALS) && defined(SPECIAL_SHAPE))
	computeExactNormals(); 
#elif (defined(COMPUTE_IMPROVED_NORMALS))
	computeNormalsFromNeighbors();
#endif

	for (int memIndex = 0; memIndex < numMembrane; memIndex ++) {	
		MembraneElement& element = pMembraneElement[memIndex];
		long diff = abs(element.vindexFeatureHi - element.vindexFeatureLo);
		int mask = getMembraneNeighborMask(&element);

		switch (dimension) {
		case 2:
			if (diff==1){
				// membrane element is aligned with the Y axis
				element.area = scaleY_um * fabs(element.unitNormal.x);
			} else if (diff==numX){
				// membrane element is aligned with the X axis
				element.area = scaleX_um * fabs(element.unitNormal.y);
			} else {
				throw "CartesianMesh::adjustMembraneAreaFromNormal(), diff is neither 1 nor numX";
			}	
			// correcting for egde or corner			
			if (mask & NEIGHBOR_X_BOUNDARY_MASK) {
				element.area *= 0.5;
			}
			if (mask & NEIGHBOR_Y_BOUNDARY_MASK) {
				element.area *= 0.5;
			}
			break;
		case 3:
			if (diff==1){
				// membrane element is in YZ plane
				element.area = scaleZ_um * scaleY_um * fabs(element.unitNormal.x);
			} else if (diff==numX){
				// membrane element is in XZ plane
				element.area = scaleX_um * scaleZ_um * fabs(element.unitNormal.y);
			} else if (diff == numXY) {
				// membrane element is in XY plane
				element.area =  scaleX_um * scaleY_um * fabs(element.unitNormal.z);
			} else {
				throw "CartesianMesh::adjustMembraneAreaFromNormal(), diff is neither 1 nor numX nor numXY";
			}	
			if (mask & NEIGHBOR_X_BOUNDARY_MASK) {
				element.area *= 0.5;
			}
			if (mask & NEIGHBOR_Y_BOUNDARY_MASK) {
				element.area *= 0.5;
			}
			if (mask & NEIGHBOR_Z_BOUNDARY_MASK) {
				element.area *= 0.5;
			}
			break;
		}
	}
	if (dimension == 2 || dimension == 3) {
		computeMembraneCoupling();
	}
}

void CartesianMesh::findMembraneNeighbors()
{
	long mecount = getNumMembraneElements();
	long mex = getNumVolumeX();
	long mey = getNumVolumeY();
	long mez = getNumVolumeZ();
	MembraneElement* meptr = getMembraneElements();
	long meloop;

	for(meloop = 0;meloop < mecount;meloop+= 1)
	{
		meptr[meloop].neighborMEIndex[0] = NeighborType::unset;
		meptr[meloop].neighborMEIndex[1] = NeighborType::unset;
		meptr[meloop].neighborMEIndex[2] = NeighborType::unset;
		meptr[meloop].neighborMEIndex[3] = NeighborType::unset;
	}

	if (dimension == 1) {
		return;
	}

	for(meloop = 0;meloop < mecount;meloop+= 1)
	{
		long idxlo,idxhi;

		idxlo = meptr[meloop].vindexFeatureLo;
		idxhi = meptr[meloop].vindexFeatureHi;

		long idxDiff = labs(idxhi - idxlo);

		if(getDimension() == 2)
		{
			if(idxDiff == 1)
			{
				meptr[meloop].neighborMEIndex[0] = orthoIndex(meloop, idxlo,idxhi,mex,NEIGHBOR_YP_BOUNDARY);
				meptr[meloop].neighborMEIndex[1] = orthoIndex(meloop, idxlo,idxhi,-mex,NEIGHBOR_YM_BOUNDARY);
			}
			else if (idxDiff == mex)
			{
				meptr[meloop].neighborMEIndex[0] = orthoIndex(meloop, idxlo,idxhi,1,NEIGHBOR_XP_BOUNDARY);
				meptr[meloop].neighborMEIndex[1] = orthoIndex(meloop, idxlo,idxhi,-1,NEIGHBOR_XM_BOUNDARY);
			}
		}
		else if (getDimension() == 3)
		{
			if(idxDiff == 1)
			{
				meptr[meloop].neighborMEIndex[0] = orthoIndex(meloop, idxlo,idxhi,mex,NEIGHBOR_YP_BOUNDARY);
				meptr[meloop].neighborMEIndex[1] = orthoIndex(meloop, idxlo,idxhi,-mex*mey,NEIGHBOR_ZM_BOUNDARY);
				meptr[meloop].neighborMEIndex[2] = orthoIndex(meloop, idxlo,idxhi,-mex,NEIGHBOR_YM_BOUNDARY);
				meptr[meloop].neighborMEIndex[3] = orthoIndex(meloop, idxlo,idxhi,mex*mey,NEIGHBOR_ZP_BOUNDARY);
			}
			else if (idxDiff == mex)
			{
				meptr[meloop].neighborMEIndex[0] = orthoIndex(meloop, idxlo,idxhi,1,NEIGHBOR_XP_BOUNDARY);
				meptr[meloop].neighborMEIndex[1] = orthoIndex(meloop, idxlo,idxhi,-mex*mey,NEIGHBOR_ZM_BOUNDARY);
				meptr[meloop].neighborMEIndex[2] = orthoIndex(meloop, idxlo,idxhi,-1,NEIGHBOR_XM_BOUNDARY);
				meptr[meloop].neighborMEIndex[3] = orthoIndex(meloop, idxlo,idxhi,mex*mey,NEIGHBOR_ZP_BOUNDARY);
			}
			else if (idxDiff == (mex*mey))
			{
				meptr[meloop].neighborMEIndex[0] = orthoIndex(meloop, idxlo,idxhi,mex,NEIGHBOR_YP_BOUNDARY);
				meptr[meloop].neighborMEIndex[1] = orthoIndex(meloop, idxlo,idxhi,-1,NEIGHBOR_XM_BOUNDARY);
				meptr[meloop].neighborMEIndex[2] = orthoIndex(meloop, idxlo,idxhi,-mex,NEIGHBOR_YM_BOUNDARY);
				meptr[meloop].neighborMEIndex[3] = orthoIndex(meloop, idxlo,idxhi,1,NEIGHBOR_XP_BOUNDARY);
			}
		}
	}
}

int CartesianMesh::getMembraneNeighborMask(long meindex) {
	return getMembraneNeighborMask(pMembraneElement + meindex);
}

int CartesianMesh::getMembraneNeighborMask(MembraneElement* element) {	
	Membrane* membrane = element->getMembrane();
	int A = pVolumeElement[element->vindexFeatureLo].neighborMask;
	int B = pVolumeElement[element->vindexFeatureHi].neighborMask;
	int tentativemask =  (A & B & NEIGHBOR_BOUNDARY_MASK);	
	if (tentativemask & NEIGHBOR_BOUNDARY_MASK) {
		if (((tentativemask & NEIGHBOR_XM_BOUNDARY) && (membrane->getXmBoundaryType() == BOUNDARY_VALUE)) 
			|| ((tentativemask & NEIGHBOR_XP_BOUNDARY) && (membrane->getXpBoundaryType() == BOUNDARY_VALUE))
			|| ((tentativemask & NEIGHBOR_YM_BOUNDARY) && (membrane->getYmBoundaryType() == BOUNDARY_VALUE))
			|| ((tentativemask & NEIGHBOR_YP_BOUNDARY) && (membrane->getYpBoundaryType() == BOUNDARY_VALUE))
			|| ((tentativemask & NEIGHBOR_ZM_BOUNDARY) && (membrane->getZmBoundaryType() == BOUNDARY_VALUE)) 
			|| ((tentativemask & NEIGHBOR_ZP_BOUNDARY) && (membrane->getZpBoundaryType() == BOUNDARY_VALUE))) {
				tentativemask |= BOUNDARY_TYPE_DIRICHLET;
		} else if (((tentativemask & NEIGHBOR_XM_BOUNDARY) && (membrane->getXmBoundaryType() == BOUNDARY_PERIODIC)) 
			|| ((tentativemask & NEIGHBOR_XP_BOUNDARY) && (membrane->getXpBoundaryType() == BOUNDARY_PERIODIC))
			|| ((tentativemask & NEIGHBOR_YM_BOUNDARY) && (membrane->getYmBoundaryType() == BOUNDARY_PERIODIC))
			|| ((tentativemask & NEIGHBOR_YP_BOUNDARY) && (membrane->getYpBoundaryType() == BOUNDARY_PERIODIC))
			|| ((tentativemask & NEIGHBOR_ZM_BOUNDARY) && (membrane->getZmBoundaryType() == BOUNDARY_PERIODIC)) 
			|| ((tentativemask & NEIGHBOR_ZP_BOUNDARY) && (membrane->getZpBoundaryType() == BOUNDARY_PERIODIC))) {
				tentativemask |= BOUNDARY_TYPE_PERIODIC;
		} else {
			tentativemask |= BOUNDARY_TYPE_NEUMANN;
		}
	}

	return tentativemask;
}

CartesianMesh::NeighborIndex CartesianMesh::orthoIndex(long memIndex, long idxLo,long idxHi,long indexer,int bmask)
{
	MembraneElement* meptr = getMembraneElements();
	//const int NO_CONNECTING_NEIGHBOR = -1;
	VolumeElement* veptr = getVolumeElements();

	//If we are at the world boundary, there are no neighbors
	if((getVolumeElements()[idxLo].neighborMask & bmask) != 0){
		return NeighborIndex(NeighborType::wall);
	}

	//See if we're at a place where membranes may overlap (not allowed).  More than two feature types abut
	Feature* featureMasterInside = veptr[idxLo].getFeature();
	Feature* featureMasterOutside = veptr[idxHi].getFeature();
	Feature* featureCompareInside = veptr[idxLo+indexer].getFeature();
	Feature* featureCompareOutside = veptr[idxHi+indexer].getFeature();
	if(((featureCompareInside != featureMasterInside) && (featureCompareInside != featureMasterOutside)) ||
		((featureCompareOutside != featureMasterInside) && (featureCompareOutside != featureMasterOutside))){
			//More than 2 feature types abut here, we do not connect even if there are potential neighbors
			return NeighborIndex(NeighborType::boundary);
	}

	//      ......
	//      |    |
	//      |    |
	//      ......
	//	f2  x f1 |
	//		x    |
	// -----------
	// :    |    :
	// :    |    :
	// ----- .....
	//
	// in above 2d case, membrane element x has 4 neighbors. But we only want 2, so we consider feature1 
	// is more important for volume connectivity.
	// Previously inside feature took priority but we eliminated inside and outside for general topology.
	// As before We are now left with an arbitrary decision of which membrane neighbor to choose first. 
	// So for backward compatibility we choose feature1 as if it were "inside".
	//
	// so if vindexFeatureLo's feature is feature1 of the membrane, we search vindexFeatureLo first. 
	// Otherwise vindexFeatureHi first.
	// 
	if (meptr[memIndex].getMembrane()->getFeature1() == pVolumeElement[idxLo].getFeature()) {
		// search for the neighbor membrane element which shares the same idxLo
		vector<long>& v1 = pVolumeElement[idxLo].adjacentMembraneIndexes;
		for (vector<long>::iterator iter1 = v1.begin(); iter1 != v1.end(); iter1 ++) {
			long mesearch = *iter1;

			if (mesearch == memIndex || meptr[mesearch].vindexFeatureLo != idxLo) {
				continue;
			}

			if(meptr[mesearch].vindexFeatureHi == (idxLo+indexer)){
				return mesearch;
			}
		}

		// search for the neighbor membrane element which shares the same idxHi
		vector<long>& v2 = pVolumeElement[idxHi].adjacentMembraneIndexes;
		for (vector<long>::iterator iter2 = v2.begin(); iter2 != v2.end(); iter2 ++) {
			long mesearch = *iter2;

			if (mesearch == memIndex || meptr[mesearch].vindexFeatureHi != idxHi) {
				continue;
			}

			if(meptr[mesearch].vindexFeatureLo == (idxHi+indexer)){
				return mesearch;
			}
		}	
	} else {
		// search for the neighbor membrane element which shares the same idxHi
		vector<long>& v2 = pVolumeElement[idxHi].adjacentMembraneIndexes;
		for (vector<long>::iterator iter2 = v2.begin(); iter2 != v2.end(); iter2 ++) {
			long mesearch = *iter2;

			if (mesearch == memIndex || meptr[mesearch].vindexFeatureHi != idxHi) {
				continue;
			}

			if(meptr[mesearch].vindexFeatureLo == (idxHi+indexer)){
				return mesearch;
			}
		}	

		// search for the neighbor membrane element which shares the same idxLo
		vector<long>& v1 = pVolumeElement[idxLo].adjacentMembraneIndexes;
		for (vector<long>::iterator iter1 = v1.begin(); iter1 != v1.end(); iter1 ++) {
			long mesearch = *iter1;

			if (mesearch == memIndex || meptr[mesearch].vindexFeatureLo != idxLo) {
				continue;
			}

			if(meptr[mesearch].vindexFeatureHi == (idxLo+indexer)){
				return mesearch;
			}
		}
	}

	// two membrane elements are connected as a straight line
	if (pVolumeElement[idxLo+indexer].getFeature() != pVolumeElement[idxHi+indexer].getFeature()) {
		vector<long>& v3 = pVolumeElement[idxLo + indexer].adjacentMembraneIndexes;
		vector<long>& v4 = pVolumeElement[idxHi + indexer].adjacentMembraneIndexes;
		for (vector<long>::iterator iter3 = v3.begin(); iter3 != v3.end(); iter3 ++) {
			long mi = *iter3;			
			for (vector<long>::iterator iter4 = v4.begin(); iter4 != v4.end(); iter4 ++) {
				long mj = *iter4;
				if (mi == mj) {
					return mi;
				}
			}
		}
	}
	return NeighborIndex(NeighborType::unknown);
}


long* qvoronoi(int dim, int numpoints, double (*points)[2], int focus, vector<VoronoiRidge>& ridges, int artificialOffset);

bool addElementToVector(vector<long>& v, long a) {
	if (v.empty()) {
		v.push_back(a);
		return true;
	}
	for (vector<long>::iterator iter = v.begin(); iter != v.end(); iter ++) {
		if (a == (*iter))  {
			return false;
		}
	}
	v.push_back(a);
	return true;
}

void CartesianMesh::getNeighborCandidates (vector<long>& neighborCandidates, DoubleVector3 centralNormal, long index, int hierarchy) {
	hierarchy = hierarchy - 1;
	if (hierarchy < 0) {
		return;
	}
	NeighborIndex* neighbors = pMembraneElement[index].neighborMEIndex;	
	for (int i = 0; i < 4; i ++) {
		if (neighbors[i].valid( )) {
			DoubleVector3 wc1 = getVolumeWorldCoord(pMembraneElement[neighbors[i]].vindexFeatureHi);
			DoubleVector3 wc2 = getVolumeWorldCoord(pMembraneElement[neighbors[i]].vindexFeatureLo);
			DoubleVector3 v2 = wc1 - wc2;
			double d = centralNormal.dotProduct(v2);

			// only continue if pointing in same direction 
			if (d >= 0) {
				addElementToVector(neighborCandidates, neighbors[i]);			
				getNeighborCandidates(neighborCandidates, centralNormal, neighbors[i], hierarchy);
			}		
		}		
	}	
}

bool project(WorldCoord pointWc, WorldCoord e1, WorldCoord e2, double* points) {
	double c1 = pointWc.dotProduct(e1); 
	double c2 = pointWc.dotProduct(e2);
	if ((c1*c1 + c2*c2) < 1e-30){
		return false;
	}else{
		points[0] = c1;
		points[1] = c2;
		return true;
	}
}

void CartesianMesh::computeMembraneCoupling(){	
	if (dimension == 1) {
		throw "No membrane diffusion allowed in 1D applications!";
	}

	cout << setprecision(10);	
	long N = getNumMembraneElements();

	cout << "Membrane Elements -> N=" << N << endl;

	switch (getDimension()) {
	case 2: {
		double totalLenghth = 0;
		StatusIndex<long,NeighborType::NeighborStatus> arr[2];

		SparseMatrixPCG* smat = new SparseMatrixPCG(N, MAXNEIGHBOR_2D * N, MATRIX_GENERAL);	
		for (int i = 0; i < N; i ++) {				
			long currentIndex = pMembraneElement[i].index;				
			ASSERTION(currentIndex==i);
			bool currentOnBoundary = false;
			if (getMembraneNeighborMask(currentIndex) & NEIGHBOR_BOUNDARY_MASK) {
				currentOnBoundary = true;
			}

			 
			int numNeighbors = 0;
			for (int m = 0; m < 2; m++) {
			  const StatusIndex<long,NeighborType::NeighborStatus> & ns =  pMembraneElement[currentIndex].neighborMEIndex[m];
			  if (ns.valid( )) {
			    arr[numNeighbors++] = ns;
			  }
			}
			if (numNeighbors == 2 && arr[0] > arr[1]) {
			  //sort the neighbor index as needed by sparse matrix format
			  std::swap(arr[0],arr[1]);
			}
			smat->addNewRow();
			double sum_arclength = 0;
			for (int j = 0; j < numNeighbors; j ++) {
				long neighborIndex = arr[j];

				double arc_length = 0;
				if (getMembraneNeighborMask(neighborIndex) & NEIGHBOR_BOUNDARY_MASK) {
					arc_length += pMembraneElement[neighborIndex].area;
				} else {
					arc_length += pMembraneElement[neighborIndex].area/2;
				}
				if (currentOnBoundary) {
					arc_length += pMembraneElement[currentIndex].area;
				} else {
					arc_length += pMembraneElement[currentIndex].area/2;
				}

				double cross_sectional_area = 1;
				smat->setCol(neighborIndex, cross_sectional_area/arc_length); // off-diagnal element and its column
				sum_arclength += arc_length;
			}
			double volLength = 0;
			if (numNeighbors == 0) {
				volLength = pMembraneElement[currentIndex].area;
			} else {
				if (sum_arclength != 0) {
					volLength = sum_arclength/2;
				} else {
					stringstream ss;
					ss << "Unexpected zero area membrane element [" << currentIndex << + "]" << getMembraneWorldCoord(currentIndex)
					<< " with neighbors " << arr[0] << " and " << arr[1]; 
					throw ss.str();
				}
			}
			smat->setDiag(i, volLength); // set diagonal elements		
			totalLenghth += volLength;
		}
		smat->close();
		cout << "Total volume= " << totalLenghth << endl;
#ifdef SPECIAL_SHAPE
		double exactVol = getExactVolume();
		cout << " Exact Volume= " <<  exactVol << endl;
		cout << " Volume absolute error with Exact= " << (exactVol - totalLenghth) << endl;
#endif
		membraneElementCoupling = smat;
		break;
			}

			/*==========================================================
			3D case
			1. find neighbor candidate
			2. projection
			3. vonoroi
			4. symmetrize (averaging)
			5. set value off diagonal si/di and diagonal vi
			==========================================================*/
	case 3: {	
		cout << "==============================================" << endl;

		long numNeighborsBeforeSymmetrize = 0;
		long numNeighborsAfterSymmetrize = 0;
		long numZeroNeighbor = 0;
		//max scale -- used for sanity / special cases inside following loop
		const double scaleMax_um =  max<double>(scaleX_um, max<double>(scaleY_um, scaleZ_um));

		SparseMatrixPCG* smat = new SparseMatrixPCG(N, N * MAXNEIGHBOR_3D, MATRIX_GENERAL);	
		if (N == 0) {
			membraneElementCoupling = smat;
			return;
		}
		IncidenceMatrix<VoronoiRidge>* vrIM = new IncidenceMatrix<VoronoiRidge>(N, (MAXNEIGHBOR_3D + 1) * N, MATRIX_GENERAL, MAXNEIGHBOR_3D);

		vector<long> neighborCandidates;
		neighborCandidates.reserve(30);			

		vector<VoronoiRidge> vrRidges;
		vrRidges.reserve(MAXNEIGHBOR_3D);

		for (long index = 0; index < N; index ++) {	

			neighborCandidates.clear();
			vrRidges.clear();

			long currentIndex = pMembraneElement[index].index;				
			ASSERTION(currentIndex==index);

			WorldCoord currentWC = getMembraneWorldCoord(currentIndex);
			DoubleVector3 currentNormal = pMembraneElement[currentIndex].unitNormal;
			int currentMask = getMembraneNeighborMask(currentIndex);

			// get neigbor candidates
			neighborCandidates.push_back(currentIndex);  // including itself

			DoubleVector3 wc1 = getVolumeWorldCoord(pMembraneElement[index].vindexFeatureHi);
			DoubleVector3 wc2 = getVolumeWorldCoord(pMembraneElement[index].vindexFeatureLo);
			DoubleVector3 centralNormal = wc1 - wc2;

			getNeighborCandidates(neighborCandidates, centralNormal, currentIndex, GOING_OUT_LAYERS);
			if (neighborCandidates.size( ) <= 1) {
				MembraneElement & me = pMembraneElement[index];	
				stringstream ss;
				ss << "index " << index << " at " << currentWC <<  " between " << wc1  << getMeshCoord(me.vindexFeatureHi)
					<< " and " << wc2 << getMeshCoord(me.vindexFeatureLo) << " has no neighborCandidates: ";
				NeighborIndex* neighbors = me.neighborMEIndex;	
				for (int i = 0; i < 4; i ++) {
					ss << neighbors[i] << ' ';
				}
				std::cout << ss.str( ) << std::endl; 
			}
			
			//assert(neighborCandidates.size() > 1);							

			/*==========================================================
			start projection
			==========================================================*/

			//
			// create an ortho-normal basis (e1,e2) for tangent plane of "current" membrane element
			//
			ArrayHolder<UnitVector3,2> tangents = perpendicular(currentNormal);
			const WorldCoord e1 = tangents[0];
			const WorldCoord e2 = tangents[1];

			const size_t pointsSize = neighborCandidates.size() + 8; // room for up to 4 artificial points at boundary.
			const size_t MAX_POINTS = 89;  //8 + 3^4
			StackPtr<double[2], MAX_POINTS> points(pointsSize);
			if (!points.onStack( )) {
				cout << "point projection storage specification too small, allocated on heap" << endl;
			}


			// current point projection
			points[0][0] = 0.0;
			points[0][1] = 0.0;

			int numPoints = 1;				
			for (vector<long>::iterator iter = neighborCandidates.begin() + 1; iter != neighborCandidates.end(); ) {
				// point X
				int neighborIndex = *iter;
				assert (neighborIndex != currentIndex);
				WorldCoord candidateX_WC = getMembraneWorldCoord(neighborIndex);						
				// move X to new coordiate system where the current point is the origin
				WorldCoord candidateX_LC = candidateX_WC - currentWC;

				// project X to tangent plane
				// convert to 2D coordinates
				if (!project(candidateX_LC, e1, e2, points[numPoints])){
					iter = neighborCandidates.erase(iter);
				} else {
					iter ++;
					numPoints ++;
				}
			}	/*============= end of projection ========================*/				

			//
			// add artifical points if the "current" membrane element is on the boundary
			//								

			if (currentMask & NEIGHBOR_BOUNDARY_MASK) {
				for (int j = 0; j < 4; j ++) {
					NeighborIndex nindex = pMembraneElement[currentIndex].neighborMEIndex[j];
					if (nindex.valid( )) {
						//
						// for each neighbor that is also on the boundary:
						//    create two artificial points to force boundary to be within Voronoi diagram
						//
						//    1) add "artificial" point near "current point" - outside domain along "boundary normal" <<<NOT MEMBRANE NORMAL>>>
						//    2) project "artificial" point onto tagent plane (transformed into e1,e2 coordinates).
						//    (note: projection can skew orthogonal vectors...so have to re-orthogonalize)
						//    3) subtract portion of projected "artificial" point in the direction of transformed boundary.
						//       a) form unit vector in direction of transformed boundary boundary_new_unit = (neighbor-current)/|neighbor-current|
						//       artificial_new = artificial_old -  (boundary_new_unit .dot. artificial_old)*boundary_new_unit
						//    4) repeat for neighbor point ....
						//
						int mask = getMembraneNeighborMask(nindex);
						int commonMask = mask & currentMask;
						if ( commonMask & NEIGHBOR_BOUNDARY_MASK) {
							WorldCoord candidateX_WC = getMembraneWorldCoord(nindex);	
							WorldCoord lc_neighbor = candidateX_WC - currentWC;

							WorldCoord lc_artificial_offset(0,0,0);
							double artificial_scale = 10000;
							if (commonMask & NEIGHBOR_XM_BOUNDARY) {									
								lc_artificial_offset.x = -scaleX_um /artificial_scale;									
							} else if (commonMask & NEIGHBOR_XP_BOUNDARY) {
								lc_artificial_offset.x = scaleX_um /artificial_scale;		
							} else if (commonMask & NEIGHBOR_YM_BOUNDARY) {
								lc_artificial_offset.y = -scaleY_um /artificial_scale;		
							} else if (commonMask & NEIGHBOR_YP_BOUNDARY) {
								lc_artificial_offset.y = scaleY_um /artificial_scale;		
							} else if (commonMask & NEIGHBOR_ZM_BOUNDARY) {
								lc_artificial_offset.z = -scaleZ_um /artificial_scale;		
							} else if (commonMask & NEIGHBOR_ZP_BOUNDARY) {
								lc_artificial_offset.z = scaleZ_um /artificial_scale;		
							}
							double prj_neighbor[2];
							double prj_artificial_offset[2];
							// current is the origin 
							if (project(lc_artificial_offset, e1, e2, prj_artificial_offset) && project(lc_neighbor, e1, e2, prj_neighbor)) {
								WorldCoord boundary_new_unit(prj_neighbor[0], prj_neighbor[1], 0);
								boundary_new_unit.normalize(); // should we protect against zero vector?

								WorldCoord artificial_old(prj_artificial_offset[0], prj_artificial_offset[1], 0);
								WorldCoord artificial_new = artificial_old - boundary_new_unit.scale(boundary_new_unit.dotProduct(artificial_old));

								points[numPoints][0] = artificial_new.x; // + 0; (current.x)
								points[numPoints][1] = artificial_new.y; // + 0; (current.y)
								numPoints ++;

								points[numPoints][0] = artificial_new.x + prj_neighbor[0];
								points[numPoints][1] = artificial_new.y + prj_neighbor[1];
								numPoints ++;
							}								
						}
					}
				}
			}				


			long *temp =  0;
			int numRealNeighbors =  0;	
			if (neighborCandidates.size( ) > 1 ) {
				temp = qvoronoi(2, numPoints, &points[0], 0, vrRidges, (int)neighborCandidates.size());
				numRealNeighbors = static_cast<int>(vrRidges.size());				
			}
			else {
				numPoints = 0;
			}
			ManagedArrayPtr<long> vertices(numPoints, temp);

			//Bug http://code.vcell.uchc.edu/bugzilla/show_bug.cgi?id=3518 temporary fix
			if (numRealNeighbors == 2) {
				VoronoiRidge & r0 =  vrRidges[0]; 
				VoronoiRidge & r1 =  vrRidges[1]; 
				const double totalSi = r0.si + r1.si; 
				if (totalSi == 0) {
					r0.si = r1.si = scaleMax_um; 
					r0.di = r1.di = scaleMax_um; 
				}
			}

#ifdef WITH_PROJECTION_TO_WALL
			if (currentMask & NEIGHBOR_BOUNDARY_MASK) {
				for (int j = 0; j < numRealNeighbors; j ++) {
					int nindex = neighborCandidates[vertices[j]];
					int mask = getMembraneNeighborMask(nindex);
					int commonMask = mask & currentMask;	
					WorldCoord wall_normal(0,0,0);

					if (commonMask & NEIGHBOR_BOUNDARY_MASK) {
						if (commonMask & NEIGHBOR_XM_BOUNDARY || commonMask & NEIGHBOR_XP_BOUNDARY) {							
							wall_normal.x = 1;
						} else if (commonMask & NEIGHBOR_YM_BOUNDARY || commonMask & NEIGHBOR_YP_BOUNDARY) {
							wall_normal.y = 1;						
						} else if (commonMask & NEIGHBOR_ZM_BOUNDARY || commonMask & NEIGHBOR_ZP_BOUNDARY) {
							wall_normal.z = 1;						
						}							

						if (wall_normal.length() != 0) {
							WorldCoord cp = currentNormal.crossProduct(wall_normal);	

							double pr[2];
							project(cp, e1, e2, pr);
							cp.x = pr[0];
							cp.y = pr[1];
							cp.z = 0;																				
							WorldCoord neighbor(points[vertices[j]][0], points[vertices[j]][1], 0);							

							cp.normalize();							
							vrRidges[j].di = fabs(neighbor.dotProduct(cp));	
						}
					}
				}
			}								
#endif				
			if (numRealNeighbors > MAXNEIGHBOR_3D) {
				throw "mesh is too coarse, try with finer mesh"; 
			}
			int indexes[MAXNEIGHBOR_3D];
			vrIM->addNewRow();

			if (numRealNeighbors == 0) {
				cout << currentIndex << " has 0 neighbors !" << endl;
				numZeroNeighbor ++;
			} else {
				//sort
				for (int j = 0; j < numRealNeighbors; j ++) {
					indexes[j] = j;
					vertices[j] = neighborCandidates[vertices[j]]; // change to membrane index
				}

				for (int j = 0; j < numRealNeighbors - 1; j ++) {
					for (int k = j + 1; k < numRealNeighbors; k ++) {
						if (vertices[indexes[j]] > vertices[indexes[k]]) {
							int a = indexes[j];
							indexes[j] = indexes[k];
							indexes[k] = a;
						}
					}
				}				

				numNeighborsBeforeSymmetrize += numRealNeighbors;	
				for (int j = 0; j < numRealNeighbors; j ++) {
					long neighborIndex = vertices[indexes[j]];
					assert(neighborIndex != currentIndex);
					VoronoiRidge& neighborVoronoi = vrRidges[indexes[j]];
					//
					// si might be very big when one of the voronoi points is close to infinity;
					// it usually happens at the corner. Then we set si to be zero, it is ok because when we 
					// symmtrize, it will get si back.
					//
					if (neighborVoronoi.si > 10 * max<double>(scaleX_um, max<double>(scaleY_um, scaleZ_um))) {
						neighborVoronoi.si = 0;
					}
					vrIM->setCol(neighborIndex, neighborVoronoi); // off-diagnal element and its column					
				}					
			}
		}
		vrIM->close();
		cout << "--------Num of points that have zero neighbors " << numZeroNeighbor << endl;		

		// symmetrize returns the pointer to the matrix that's passed in so symmIM and vrIM point to the same matrix.
		IncidenceMatrix<VoronoiRidge>* symmIM = symmetrize(vrIM, N);

		cout << setprecision(10);
		double volTotal = 0.0;
		double totalFluxArea = 0.0;
		double boundaryFluxArea[6];
		memset(boundaryFluxArea, 0, 6 * sizeof(double));
		for (int index = 0; index < N; index ++) {
			bool bFlux = false;
			double* fluxArea = new double[6];
			memset(fluxArea, 0, 6 * sizeof (double));				

			int currentmask = getMembraneNeighborMask(index);

			if ((currentmask & BOUNDARY_TYPE_MASK) == BOUNDARY_TYPE_NEUMANN){ // boundary and only neumann
				bFlux = true;		
			}

			smat->addNewRow();
			int32* columns;
			VoronoiRidge* vrs;				
			int numColumns = symmIM->getColumns(index, columns, vrs);				
			assert(numColumns > 0);

			double vol = 0.0;									
			int numNeighbors = 0;
			for (int j = 0; j < numColumns; j ++) {							
				int32 neighborIndex = columns[j];
				if (neighborIndex <= -1) {
					break;
				}

				numNeighbors ++;
				assert(neighborIndex < N && neighborIndex != index);
				assert(vrs[j].bSymmetrized);

				numNeighborsAfterSymmetrize ++;
				smat->setCol(neighborIndex, vrs[j].si/vrs[j].di);
				vol += vrs[j].si * vrs[j].di/4;

				double boundary_arclength = vrs[j].di/2;
				int neighbormask = getMembraneNeighborMask(neighborIndex);					
				if (bFlux && neighbormask & NEIGHBOR_BOUNDARY_MASK){ 
					int commonmask = currentmask & neighbormask; 
					// boundary and neumann
					if (commonmask & NEIGHBOR_XM_BOUNDARY) {
						fluxArea[BL_Xm] += boundary_arclength;
						totalFluxArea += boundary_arclength;
						boundaryFluxArea[BL_Xm] += boundary_arclength;
					} else if (commonmask & NEIGHBOR_XP_BOUNDARY) {
						fluxArea[BL_Xp] += boundary_arclength;
						totalFluxArea += boundary_arclength;
						boundaryFluxArea[BL_Xp] += boundary_arclength;
					} else if (commonmask & NEIGHBOR_YM_BOUNDARY) {
						fluxArea[BL_Ym] += boundary_arclength;
						totalFluxArea += boundary_arclength;
						boundaryFluxArea[BL_Ym] += boundary_arclength;
					} else if (commonmask & NEIGHBOR_YP_BOUNDARY) {
						fluxArea[BL_Yp] += boundary_arclength;
						totalFluxArea += boundary_arclength;
						boundaryFluxArea[BL_Yp] += boundary_arclength;
					} else if (commonmask & NEIGHBOR_ZM_BOUNDARY) {
						fluxArea[BL_Zm] += boundary_arclength;
						totalFluxArea += boundary_arclength;
						boundaryFluxArea[BL_Zm] += boundary_arclength;
					} else if (commonmask & NEIGHBOR_ZP_BOUNDARY) {
						fluxArea[BL_Zp] += boundary_arclength;
						totalFluxArea += boundary_arclength;
						boundaryFluxArea[BL_Zp] += boundary_arclength;
					} 
				}
			}
			if (numNeighbors == 0) {				
				vol = pMembraneElement[index].area;
			} else {
				if (vol == 0) {
					stringstream ss;
					ss << "Unexpected zero (0)  area membrane element [" << index << + "]" << getMembraneWorldCoord(index) << std::endl;
					MembraneElement & me = pMembraneElement[index];
					ss << " Feature hi at " << getMeshCoord(me.vindexFeatureHi) << " and feature low at " << getMeshCoord(me.vindexFeatureLo) << std::endl;
					ss << " with  candidate neighbors with index/status ";
					for (int j = 0; j < numColumns; j ++) {							
						int32 neighborIndex = columns[j];
						if (neighborIndex >= 0) {
							ss << neighborIndex << ' ' << getMembraneWorldCoord(neighborIndex) << ' ';
						}
						else {
							const NeighborType::NeighborStatus ns = static_cast<NeighborType::NeighborStatus>(neighborIndex);
							ss << ns << ' ';
						}
					}
					std::cout << ss.str( ) << std::endl;
					vol = pMembraneElement[index].area;
				}
			}
			smat->setDiag(index, vol);
			if (bFlux) {
				membrane_boundary_flux.insert(pair<long, double*>(index, fluxArea));
			} 
			volTotal += vol;
		}
		smat->close();
		delete vrIM;

		cout << "--------Num Neighbors before symmetrize " << numNeighborsBeforeSymmetrize << endl;
		cout << "--------Num Neighbors after symmetrize " << numNeighborsAfterSymmetrize << endl;			
		cout << "Total volume=" << volTotal << endl;
		cout << "Total FluxArea =" << totalFluxArea << endl;
		cout << "Total FluxAreaXM =" << boundaryFluxArea[BL_Xm] << endl;
		cout << "Total FluxAreaXP =" << boundaryFluxArea[BL_Xp] << endl;
		cout << "Total FluxAreaYM =" << boundaryFluxArea[BL_Ym] << endl;
		cout << "Total FluxAreaYP =" << boundaryFluxArea[BL_Yp] << endl;
		cout << "Total FluxAreaZM =" << boundaryFluxArea[BL_Zm] << endl;
		cout << "Total FluxAreaZP =" << boundaryFluxArea[BL_Zp] << endl;
#ifdef SPECIAL_SHAPE
		double exactVol = getExactVolume();
		cout << " Exact Volume= " <<  exactVol << endl;
		cout << " Volume absolute error with Exact= " << (exactVol - volTotal) << endl;
#endif
		membraneElementCoupling = smat;
		break;
			} // end of 3D case
	} // end of switch (dimension)

	for (int index = 0; index < N; index ++) {				
		pMembraneElement[index].area = membraneElementCoupling->getValue(index, index); // diagonal element is the new area
	}	
}

bool is_next_point_on_curve(CurvePlane curvePlane, WorldCoord centerWc, WorldCoord nextWc) {
	if (curvePlane == CURVE_PLANE_XY && nextWc.z == centerWc.z 
		|| curvePlane == CURVE_PLANE_XZ && nextWc.y == centerWc.y 
		|| curvePlane == CURVE_PLANE_YZ && nextWc.x == centerWc.x) {
			return true;
	}
	return false;
}

/**
* move values from curve to curvex, curvey, depending on which plane set by curvePlane
*/
void getXYCurve(CurvePlane curvePlane, const vector<WorldCoord> & curve, vector<double>& curvex, vector<double>& curvey) {
	assert(curvex.empty( ));
	assert(curvey.empty( ));
	for (vector<WorldCoord>::const_iterator iter = curve.begin(); iter != curve.end(); iter ++) {
		switch (curvePlane) {
		case CURVE_PLANE_XY:
			curvex.push_back(iter->x);
			curvey.push_back(iter->y);
			break;

		case CURVE_PLANE_XZ:
			curvex.push_back(iter->x);
			curvey.push_back(iter->z);
			break;

		case CURVE_PLANE_YZ:
			curvex.push_back(iter->y);
			curvey.push_back(iter->z);
			break;
		default:
			assert(false);
		}
	}
}

bool CartesianMesh::findCurve(int startingIndex, CurvePlane curvePlane, vector<double>& curvex, vector<double>& curvey, int& currentMeIndexInVector){	
	vector<long> indexCurve;
	vector<WorldCoord> curve;

	WorldCoord currentWc, neighborWc;
	currentWc = getMembraneWorldCoord(startingIndex);

	// push back the staring index;
	curve.push_back(currentWc);	
	indexCurve.push_back(startingIndex);
	currentMeIndexInVector = 0;

	bool bOpenCurve = false;
	NeighborIndex neighborIndex(NeighborType::unset);
	long index = startingIndex;

	while (true) {
		currentWc = getMembraneWorldCoord(index);
		bool bFound = false;
		for (int i = 0; i < 4; i ++) {
			neighborIndex = pMembraneElement[index].neighborMEIndex[i];
			if (!neighborIndex.valid( )) {
				switch (neighborIndex.validity( )) {
				case NeighborType::unset:
				  
				case NeighborType::boundary:
					//specific actions to be determined
				case NeighborType::wall:
				case NeighborType::unknown:
					continue;
				}
			}
			if (!bOpenCurve) {
				if (index != startingIndex && neighborIndex == *(indexCurve.end() - 2)) {// if closed, make sure the next point hasn't been added to curve already 
					continue;
				}
			} else {
				if (neighborIndex == indexCurve.at(1)) { // if open, make sure the next point hasn't been added to curve already
					continue;
				}
			}			
			neighborWc = getMembraneWorldCoord(neighborIndex);
			if (is_next_point_on_curve(curvePlane, currentWc, neighborWc)) {
				bFound = true;
				break;
			}
		}

		if (bFound) { // found the next one
			if (neighborIndex == startingIndex) {
				break;
			}  else {
				if (bOpenCurve) {
					curve.insert(curve.begin(), neighborWc); // push back this new point
					indexCurve.insert(indexCurve.begin(), neighborIndex); 
					currentMeIndexInVector ++;
				} else {
					curve.push_back(neighborWc); // push back this new pooint
					indexCurve.push_back(neighborIndex);
				}
				index = neighborIndex;
			}
		} else {
			if (bOpenCurve) {
				break;
			} else {
				bOpenCurve = true;
				if (indexCurve.size() == 1) {
					break;
				}
				index = startingIndex;
			}		
		}
	}	

	getXYCurve(curvePlane, curve, curvex, curvey);

	return !bOpenCurve;
}

//review
int CartesianMesh::computeNormalApproximationHops(int startingIndex, CurvePlane curvePlane, 
				vector<double> curvex, vector<double> curvey, int currentMeIndexInVector, bool bClose) {
	assert(curvex.size() == curvey.size());
	int numPoints = static_cast<int>(curvex.size());
	if (numPoints == 1) {
		return 0;
	}
	double maxX = 1E-10, maxY = 1E-10;
	double minX = 1E10, minY = 1E10;
	for (int i = 0; i < numPoints; i ++) {
		if (maxX < curvex[i]) {
			maxX = curvex[i];
		}
		if (minX > curvex[i]) {
			minX = curvex[i];
		}
		if (maxY < curvey[i]) {
			maxY = curvey[i];
		}
		if (minY > curvey[i]) {
			minY = curvey[i];
		}
	}
	double Nx = 0, Ny = 0, h = 0;
	switch (curvePlane) {
	case CURVE_PLANE_XY:
		Nx = (maxX - minX)/scaleX_um;
		Ny = (maxY - minY)/scaleY_um;
		h = (scaleX_um + scaleY_um) / 2;
		break;
	case CURVE_PLANE_XZ:
		Nx = (maxX - minX)/scaleX_um;
		Ny = (maxY - minY)/scaleZ_um;
		h = (scaleX_um + scaleZ_um) / 2;
		break;
	case CURVE_PLANE_YZ:
		Nx = (maxX - minX)/scaleY_um;
		Ny = (maxY - minY)/scaleZ_um;
		h = (scaleY_um + scaleZ_um) / 2;
		break;
	}
	int coeff = 2;
	int nwave = (int)min<double>(floor(coeff * sqrt((Nx + Ny)/2)), floor(sqrt((double)numPoints)));
	//
	// find coordinates of nearby points (left,right that are nwave neighbors away) to estimate curvature 
	// from the secants in each direction and measuring the angles between the secants.  These secants are
	// sampled at an appropriate characteristic length to ensure convergence as deltaX --> 0
	//
	WorldCoord left(0,0,0), right(0,0,0);
	if (bClose) {
		if (numPoints <= 6) {
			return 1;
		}
		left.x = curvex[(currentMeIndexInVector - nwave + numPoints) % numPoints] - curvex[currentMeIndexInVector];
		left.y = curvey[(currentMeIndexInVector - nwave + numPoints) % numPoints] - curvey[currentMeIndexInVector];
		right.x = curvex[(currentMeIndexInVector + nwave) % numPoints] - curvex[currentMeIndexInVector];
		right.y = curvey[(currentMeIndexInVector + nwave) % numPoints] - curvey[currentMeIndexInVector];
	} else {
		if (currentMeIndexInVector - nwave < 0 && currentMeIndexInVector + nwave >= numPoints) {
			throw "Mesh size is too coarse, please refine mesh!";
		} else if (currentMeIndexInVector - nwave < 0) { // left side doesn't have enough neighbors
			if (currentMeIndexInVector + nwave/2 > currentMeIndexInVector &&
				currentMeIndexInVector + nwave/2 < currentMeIndexInVector + nwave) {
					left.x = curvex[currentMeIndexInVector] - curvex[currentMeIndexInVector + nwave/2];
					left.y = curvey[currentMeIndexInVector] - curvey[currentMeIndexInVector + nwave/2] ;
					right.x = curvex[currentMeIndexInVector + nwave] - curvex[currentMeIndexInVector + nwave/2];
					right.y = curvey[currentMeIndexInVector + nwave] - curvey[currentMeIndexInVector + nwave/2];
			} else if (currentMeIndexInVector + 2 * nwave < numPoints) {				
				left.x = curvex[currentMeIndexInVector] - curvex[currentMeIndexInVector + nwave];
				left.y = curvey[currentMeIndexInVector] - curvey[currentMeIndexInVector + nwave];
				right.x = curvex[currentMeIndexInVector + 2 * nwave] - curvex[currentMeIndexInVector + nwave];
				right.y = curvey[currentMeIndexInVector + 2 * nwave] - curvey[currentMeIndexInVector + nwave];
			} else {
				return 1;
			}
		} else if (currentMeIndexInVector + nwave >= numPoints) { // right side doesn't have enough neighbors
			if (currentMeIndexInVector - nwave/2 < currentMeIndexInVector &&
				currentMeIndexInVector - nwave/2 > currentMeIndexInVector - nwave) {
					left.x = curvex[currentMeIndexInVector - nwave] - curvex[currentMeIndexInVector - nwave/2];
					left.y = curvey[currentMeIndexInVector - nwave ] - curvey[currentMeIndexInVector - nwave/2] ;
					right.x = curvex[currentMeIndexInVector] - curvex[currentMeIndexInVector - nwave/2];
					right.y = curvey[currentMeIndexInVector] - curvey[currentMeIndexInVector - nwave/2];
			} else if (currentMeIndexInVector - 2 * nwave >= 0) {				
				left.x = curvex[currentMeIndexInVector - 2 * nwave] - curvex[currentMeIndexInVector - nwave];
				left.y = curvey[currentMeIndexInVector - 2 * nwave] - curvey[currentMeIndexInVector - nwave];
				right.x = curvex[currentMeIndexInVector] - curvex[currentMeIndexInVector - nwave];
				right.y = curvey[currentMeIndexInVector] - curvey[currentMeIndexInVector - nwave];
			} else {
				return 1;
			}
		} else { // both sides have enough neighbors
			left.x = curvex[currentMeIndexInVector - nwave] - curvex[currentMeIndexInVector];
			left.y = curvey[currentMeIndexInVector - nwave] - curvey[currentMeIndexInVector];
			right.x = curvex[currentMeIndexInVector + nwave] - curvex[currentMeIndexInVector];
			right.y = curvey[currentMeIndexInVector + nwave] - curvey[currentMeIndexInVector];
		}
	}
	double r = (left - right).length()/2;
	left.normalize();
	right.normalize();
	double dotp = max<double>(-1.0, min(1.0, left.dotProduct(right)));
	double phi = acos(dotp);
	if (phi < 0.01) {
		phi = 0.01;
	}
	if (phi > PI - 0.01) {
		phi = PI - 0.01;
	}
	double R = r / fabs(sin(phi));

	if (MathUtil::double_infinity == -R || MathUtil::double_infinity == R) {
		return nwave;
	}

	if (R != R) {
		throw "CartesianMesh::computeN(), Radius of curvature is NAN";
	}	

	int N = min<int>((int)floor(coeff * sqrt(R/h)), nwave);
	return N;
}


inline NormalDirection CartesianMesh::membraneElementFeatureDirection(int index) const {
	int difference = abs(pMembraneElement[index].vindexFeatureHi - pMembraneElement[index].vindexFeatureLo);
	if (difference == 1) {
		return  NORMAL_DIRECTION_X;
	} 
	if (difference == numX) {
		return NORMAL_DIRECTION_Y;
	} 
	if (difference == numXY) {
		return NORMAL_DIRECTION_Z;
	}
	throw std::logic_error("can't find normal direction");
}

ArrayHolder<int,4> CartesianMesh::getNormalApproximationHops(long index) {
	ArrayHolder<int,4> rval(0);
	switch (dimension) {
	case 2: {
#ifdef N_EQUALS_2
		N[0] = 2;
		N[1] = N[0];			
#else
		vector<double> curvex;
		vector<double> curvey;

		int currentMeInVector = 0;
		bool bClose = findCurve(index, CURVE_PLANE_XY, curvex, curvey, currentMeInVector);
		rval[0] = computeNormalApproximationHops(index, CURVE_PLANE_XY, curvex, curvey, currentMeInVector, bClose);
		rval[1] = rval[0];
#endif
		break;
			}
	case 3: 
#ifdef N_EQUALS_2
		for (int i = 0; i < 4; i ++) {
			N[i] = 2;				
		}
#else
		{
			vector<double> curve1x, curve1y;
			vector<double> curve2x, curve2y;

			WorldCoord currentWc = getMembraneWorldCoord(index);
			NormalDirection normalDir = membraneElementFeatureDirection(index);

			CurvePlane aNormal; 
			CurvePlane otherNormal; 
			switch (normalDir) {
			case NORMAL_DIRECTION_X:
				aNormal = CURVE_PLANE_XY;
				otherNormal = CURVE_PLANE_XZ;
				break;
			case NORMAL_DIRECTION_Y:
				aNormal = CURVE_PLANE_XY;
				otherNormal = CURVE_PLANE_YZ;
				break;
			case NORMAL_DIRECTION_Z:
				aNormal = CURVE_PLANE_XZ;
				otherNormal = CURVE_PLANE_YZ;
				break;
			default:
				assert(false);
			}

			int currentMeInVector1 = 0;
			int currentMeInVector2 = 0;
			const bool bClose1 = findCurve(index, aNormal, curve1x, curve1y, currentMeInVector1);
			const int resultN1 = computeNormalApproximationHops(index, aNormal, curve1x, curve1y, currentMeInVector1, bClose1);
			const bool bClose2 = findCurve(index, otherNormal, curve2x, curve2y, currentMeInVector2);
			const int resultN2 = computeNormalApproximationHops(index, otherNormal, curve2x, curve2y, currentMeInVector2, bClose2);
			for (int i = 0; i < 4; i ++) {					
				NeighborIndex neighborIndex = pMembraneElement[index].neighborMEIndex[i];
				if (neighborIndex.valid( )) {
					WorldCoord wc = getMembraneWorldCoord(neighborIndex);
					if (is_next_point_on_curve(aNormal, wc, currentWc)) {
						rval[i] = resultN1;
					} else if (is_next_point_on_curve(otherNormal, wc, currentWc)) {
						rval[i] = resultN2;
					}
				}
				else {
					if (neighborIndex.validity( ) == NeighborType::boundary) {
						///		N[i] =  normalDir;
					}
				}
			}
#endif
			break;
		}
	}	
	return rval;
}

IncidenceMatrix<VoronoiRidge>* CartesianMesh::symmetrize(IncidenceMatrix<VoronoiRidge>* im, long N) {
	IncidenceMatrix<VoronoiRidge>* symmIM = im;
	for (int index = 0; index < N; index ++) {
		int32* columnIndices = 0;
		VoronoiRidge* columnValues = 0;
		int numColumns = symmIM->getColumns(index, columnIndices, columnValues);

		for (int j = 0; j < numColumns; j ++) {	
			int32 neighborIndex = columnIndices[j];
			if (neighborIndex <= -1) {
				break;
			}

			VoronoiRidge& vrij = columnValues[j];	
			if (vrij.bSymmetrized) {
				continue;
			}
			assert(neighborIndex < N && neighborIndex != index);

			bool bJIExists = false;
			if (index < neighborIndex) {
				VoronoiRidge* vrji = symmIM->getValue(neighborIndex, index);
				if (vrji != 0) {
					vrij = vrji->average(vrij);
					vrij.bSymmetrized = true;
					(*vrji) = vrij;
					bJIExists = true;					
				}
			}

			if (!bJIExists || index > neighborIndex) {
				vrij.si = vrij.si/2;				

				// recover di of vrji whose dji = 0
				WorldCoord wc_center = getMembraneWorldCoord(neighborIndex);
				WorldCoord wc_neighbor = getMembraneWorldCoord(index);

				WorldCoord r = wc_neighbor - wc_center;
				WorldCoord unitNormal_center = pMembraneElement[neighborIndex].unitNormal;
				double dot_rn = r.dotProduct(unitNormal_center);
				double di = (r - unitNormal_center.scale(dot_rn)).length();

#ifdef WITH_PROJECTION_TO_WALL				
				// project the boundary to the wall
				int commonMask = getMembraneNeighborMask(neighborIndex) & getMembraneNeighborMask(index);
				if (commonMask & NEIGHBOR_BOUNDARY_MASK) {
					if (commonMask & NEIGHBOR_XM_BOUNDARY || commonMask & NEIGHBOR_XP_BOUNDARY) {
						di = fabs(r.y * unitNormal_center.z - r.z*unitNormal_center.y)/sqrt(unitNormal_center.y * unitNormal_center.y + unitNormal_center.z * unitNormal_center.z);
					} else if (commonMask & NEIGHBOR_YM_BOUNDARY || commonMask & NEIGHBOR_YP_BOUNDARY) {						
						di = fabs(r.x * unitNormal_center.z - r.z*unitNormal_center.x)/sqrt(unitNormal_center.x * unitNormal_center.x + unitNormal_center.z * unitNormal_center.z);
					} else if (commonMask & NEIGHBOR_ZM_BOUNDARY || commonMask & NEIGHBOR_ZP_BOUNDARY) {
						di = fabs(r.y * unitNormal_center.x - r.x*unitNormal_center.y)/sqrt(unitNormal_center.x * unitNormal_center.x + unitNormal_center.y * unitNormal_center.y);
					}
				}
#endif							
				vrij.di = (di + vrij.di)/2;	

				vrij.bSymmetrized = true;
				symmIM->setValue(neighborIndex, index, vrij);
			}
		}
	}
	return symmIM;
}

double* CartesianMesh::getMembraneFluxArea(long index) {	
	map <long, double*> :: const_iterator iter;
	iter = membrane_boundary_flux.find(index);
	if (iter == membrane_boundary_flux.end()) {
		char errmsg[1000];
		sprintf(errmsg, "Membrane boundary flux information exception: no boundary condition found for membrane element %d", index);
		throw errmsg;
	}
	return iter->second;
}

std::ostream& operator<<(std::ostream &os ,const IntVector3 & v) {
	os << '[' << v.x << ',' << v.y << ',' << v.z << ']'; 
	return os;
}

namespace NeighborType { 
	std::ostream& operator<<(std::ostream & os,NeighborStatus ns) {
		switch (ns) {
			case good: 
				os << "good";
				break;
			case unset: 
				os << "unset";
				break;
			case wall: 
				os << "wall";
				break;
			case boundary: 
				os << "boundary";
				break;
			case unknown: 
				os << "unknown";
				break;
		}
		return os;
	}
}
