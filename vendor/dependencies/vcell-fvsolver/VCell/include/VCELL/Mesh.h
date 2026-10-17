/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef MESH_H
#define MESH_H

#include <stdio.h>
#include <VCELL/SimTypes.h>
//#include <VCELL/Contour.h>
//#include <VCELL/ContourSubdomain.h>
#include <map>
using std::pair;
using std::map;

class Geometry;
class VolumeRegion;
class MembraneRegion;
class VolumeVariable;
class Particle;
class SparseMatrixPCG;
struct MembraneElement;
struct VolumeElement;

class Mesh {
public:
    virtual double getVolumeOfElement_cu(long volumeIndex) = 0;

    virtual WorldCoord getVolumeWorldCoord(long volumeIndex) = 0;
    virtual WorldCoord getMembraneWorldCoord(long membraneIndex) = 0;
    virtual WorldCoord getMembraneWorldCoord(MembraneElement *element) = 0;

    virtual void showSummary(FILE *fp) {
        fprintf(fp, "Mesh::showSummary()...\n");
    }
    virtual void write(FILE *fp) = 0;
    virtual void writeMeshMetrics(FILE* fp) = 0;

    VolumeElement *getVolumeElements();
    long getNumVolumeElements();
    MembraneElement *getMembraneElements();
    long getNumMembraneElements();

    int getDimension() const { return dimension; }

    SparseMatrixPCG* getMembraneCoupling() {
        if (membraneElementCoupling) {
            return membraneElementCoupling;
        }
        if (dimension == 1) {
            throw "Membrane diffusion is not supported in 1D applications!";
        }
        computeMembraneCoupling();
        return membraneElementCoupling;
    }

    virtual int getMembraneNeighborMask(long meindex) = 0;
    virtual int getMembraneNeighborMask(MembraneElement* element) = 0;
    virtual double* getMembraneFluxArea(long index) = 0;

protected:
    Mesh(double captureNeighborhood);

    virtual ~Mesh();

    VolumeElement *pVolumeElement;
    MembraneElement *pMembraneElement;
    long numMembrane;
    long numVolume;
    int dimension;
    double captureNeighborhood;
    SparseMatrixPCG* membraneElementCoupling;
    virtual void computeMembraneCoupling(void) = 0;

    map<long, double*> membrane_boundary_flux;
};

#endif
