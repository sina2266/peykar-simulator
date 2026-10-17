/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef ELEMENT_H
#define ELEMENT_H

#include <VCELL/DoubleVector3.h>
#include <VCELL/VolumeRegion.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/Feature.h>

#include <vector>
using std::vector;

#define BOUNDARY_TYPE_MASK		   0x70000   // '0111 0000 0000 0000 0000'b
#define BOUNDARY_TYPE_DIRICHLET    0x10000   // '0001 0000 0000 0000 0000'b
#define BOUNDARY_TYPE_NEUMANN	   0x20000   // '0010 0000 0000 0000 0000'b
#define BOUNDARY_TYPE_PERIODIC	   0x40000   // '0100 0000 0000 0000 0000'b

#define ELLIPTIC_PINNED			   0x80000   // '1000 0000 0000 0000 0000'b

#define VOLUME_MASK                0x0000E   // '0000 0000 0000 0000 1110'b
#define VOLUME_HALF                0x00002   // '0000 0000 0000 0000 0010'b
#define VOLUME_QUARTER             0x00004   // '0000 0000 0000 0000 0100'b
#define VOLUME_EIGHTH              0x00008   // '0000 0000 0000 0000 1000'b

#define NEIGHBOR_MASK              0x0FFF0   // '0000 1111 1111 1111 0000'b

#define NEIGHBOR_BOUNDARY_MASK     0x05550   // '0000 0101 0101 0101 0000'b
#define NEIGHBOR_X_BOUNDARY_MASK   0x00050   // '0000 0000 0000 0101 0000'b
#define NEIGHBOR_Y_BOUNDARY_MASK   0x00500   // '0000 0000 0101 0000 0000'b
#define NEIGHBOR_Z_BOUNDARY_MASK   0x05000   // '0000 0101 0000 0000 0000'b

#define NEIGHBOR_MEMBRANE_MASK     0x0AAA0   // '0000 1010 1010 1010 0000'b
#define NEIGHBOR_X_MEMBRANE_MASK   0x000A0   // '0000 0000 0000 1010 0000'b
#define NEIGHBOR_Y_MEMBRANE_MASK   0x00A00   // '0000 0000 1010 0000 0000'b
#define NEIGHBOR_Z_MEMBRANE_MASK   0x0A000   // '0000 1010 0000 0000 0000'b

#define NEIGHBOR_XM_MASK           0x00030   // '0000 0000 0000 0011 0000'b
#define NEIGHBOR_XM_BOUNDARY       0x00010   // '0000 0000 0000 0001 0000'b
#define NEIGHBOR_XM_MEMBRANE       0x00020   // '0000 0000 0000 0010 0000'b
#define NEIGHBOR_XP_MASK           0x000C0   // '0000 0000 0000 1100 0000'b
#define NEIGHBOR_XP_BOUNDARY       0x00040   // '0000 0000 0000 0100 0000'b
#define NEIGHBOR_XP_MEMBRANE       0x00080   // '0000 0000 0000 1000 0000'b
#define NEIGHBOR_YM_MASK           0x00300   // '0000 0000 0011 0000 0000'b
#define NEIGHBOR_YM_BOUNDARY       0x00100   // '0000 0000 0001 0000 0000'b
#define NEIGHBOR_YM_MEMBRANE       0x00200   // '0000 0000 0010 0000 0000'b
#define NEIGHBOR_YP_MASK           0x00C00   // '0000 0000 1100 0000 0000'b
#define NEIGHBOR_YP_BOUNDARY       0x00400   // '0000 0000 0100 0000 0000'b
#define NEIGHBOR_YP_MEMBRANE       0x00800   // '0000 0000 1000 0000 0000'b
#define NEIGHBOR_ZM_MASK           0x03000   // '0000 0011 0000 0000 0000'b
#define NEIGHBOR_ZM_BOUNDARY       0x01000   // '0000 0001 0000 0000 0000'b
#define NEIGHBOR_ZM_MEMBRANE       0x02000   // '0000 0010 0000 0000 0000'b
#define NEIGHBOR_ZP_MASK           0x0C000   // '0000 1100 0000 0000 0000'b
#define NEIGHBOR_ZP_BOUNDARY       0x04000   // '0000 0100 0000 0000 0000'b
#define NEIGHBOR_ZP_MEMBRANE       0x08000   // '0000 1000 0000 0000 0000'b

class Membrane;

struct VolumeElement
{
    int neighborMask;
    VolumeRegion  *region;
	vector<long>  adjacentMembraneIndexes;

	inline Feature* getFeature() {
		return region->getFeature();
	}
	inline unsigned char getFeatureHandle() {
		return getFeature()->getHandle();
	}
	inline unsigned char getFeatureIndex() {
		return getFeature()->getIndex();
	}
	inline int getRegionIndex() {
		return region->getIndex();
	}
};

class CartesianMesh;
struct MembraneElement
{
	MembraneElement( ) {
	}
	//orientation? x, y, z gcw
    double	area;
	DoubleVector3 unitNormal;
	//DoubleVector3 smoothedCoord;
	long vindexFeatureLo, vindexFeatureHi, vindexFeatureLoFar, vindexFeatureHiFar;
    long index;  
	StatusIndex<long,NeighborType::NeighborStatus> neighborMEIndex[4];
    MembraneRegion  *region;

	inline Membrane* getMembrane() {
		return region->getMembrane();
	}
	inline int getRegionIndex() {
		return region->getIndex();
	}
};

/*
void showMask(VolumeElement *element){
   int mask = element->neighborMask;
   printf("mask = ");
   if ((mask&NEIGHBOR_XM_BOUNDARY)==NEIGHBOR_XM_BOUNDARY){
      printf("XM_BOUNDARY ");
   }
   if ((mask&NEIGHBOR_XM_MEMBRANE)==NEIGHBOR_XM_MEMBRANE){
      printf("XM_MEMBRANE ");
   }
		   
   if ((mask&NEIGHBOR_XP_BOUNDARY)==NEIGHBOR_XP_BOUNDARY){
      printf("XP_BOUNDARY ");
   }
   if ((mask&NEIGHBOR_XP_MEMBRANE)==NEIGHBOR_XP_MEMBRANE){  
      printf("XP_MEMBRANE ");
   }
		   
   if ((mask&NEIGHBOR_YM_BOUNDARY)==NEIGHBOR_YM_BOUNDARY){
      printf("YM_BOUNDARY ");
   }
   if ((mask&NEIGHBOR_YM_MEMBRANE)==NEIGHBOR_YM_MEMBRANE){
      printf("YM_MEMBRANE ");
   }
		   
   if ((mask&NEIGHBOR_YP_BOUNDARY)==NEIGHBOR_YP_BOUNDARY){
      printf("YP_BOUNDARY ");
   }
   if ((mask&NEIGHBOR_YP_MEMBRANE)==NEIGHBOR_YP_MEMBRANE){
      printf("YP_MEMBRANE ");
   }
		   
   if ((mask&NEIGHBOR_ZM_BOUNDARY)==NEIGHBOR_ZM_BOUNDARY){
      printf("ZM_BOUNDARY ");
   }
   if ((mask&NEIGHBOR_ZM_MEMBRANE)==NEIGHBOR_ZM_MEMBRANE){
      printf("ZM_MEMBRANE ");
   }
		   
   if ((mask&NEIGHBOR_ZP_BOUNDARY)==NEIGHBOR_ZP_BOUNDARY){
      printf("ZP_BOUNDARY ");
   }
   if ((mask&NEIGHBOR_ZP_MEMBRANE)==NEIGHBOR_ZP_MEMBRANE){
      printf("ZP_MEMBRANE ");
   }
   printf("\n");
}
*/


#endif
