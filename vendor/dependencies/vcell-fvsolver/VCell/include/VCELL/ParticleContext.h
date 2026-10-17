/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef PARTICLECONTEXT_H
#define PARTICLECONTEXT_H

#include <VCELL/SimTypes.h>
#include <VCELL/Mesh.h>
#include <vector>
using namespace std;

class Particle;

const double Avagadros_num = 6.02e23;
const double micro = 1e-6;
const double cubic_um_per_liter = 1e15;
const double conversionFactor = cubic_um_per_liter / (micro * Avagadros_num);

class VolumeParticle;
class Feature;
class Simulation;
class Mesh;
class Geometry;
struct MembraneElement;

class ParticleContext;

class Particle
{
public:
    Particle();
    
    WorldCoord getWorldCoord();
    LocalCoord getLocalCoord();
    long getIndex();
    int getState();
    char *getName() { return names[state]; }
    ParticleContext *getParticleContext(){return pc;}
    void setLocation(LocalCoord lc, long index, ParticleContext *pc);
    void setLocation(LocalCoord lc, long index);
    void setState(int state);
    virtual void write(FILE *fp);

	static void setParticleTypes(int numTypes, char **nameArray);
    static inline int getNumStates() { return numStates; }
    static inline char *getNameFromState(int Astate) { return names[Astate]; }
protected:
    ParticleContext *pc;
private:     
    LocalCoord   lc;
    long         index;  
    int          state;
    static  char **names;
    static int   numStates;
};


class ParticleContext
{
public:
    Feature *getParent() { return feature; }

    //
    // ALWAYS CALL ParentClass::resolveReferences() first
    //
    virtual void resolveReferences(Simulation *sim);
    virtual double getInitialValue(long index);
        
    virtual bool move()=0;
    virtual bool react();
    
    long getNumParticles() {return (int)particles.size();}
    void addParticle(Particle *particle);
    void removeParticle(long index) {particles.erase(particles.begin()+index);}
    Particle *getParticle(long index);
    void moveParticleTo(long index, ParticleContext *pc);
    LocalCoord setLocalCoord(double u, double v, double w);
    virtual WorldCoord getWorldCoord(LocalCoord lc, long index)=0;
    void load();
    void write(FILE *fp);
    virtual LocationContext getLocationContext()=0;     
     
protected:
    ParticleContext(Feature *feature);
    Feature       *feature;
    double        *initialValue;
    Mesh          *mesh;
    Simulation    *sim;
    int numParticles;
    vector<Particle*>  particles;
    double getRandomUniform();
    double getRandomNormal();
private:
    friend class Feature;
    friend class Contour;
};


class VolumeParticleContext : public ParticleContext
{
public:
    double setDiffusionRate();
    bool move();
    long getRandomIndex();
    WorldCoord getWorldCoord(LocalCoord lc, long index);
    virtual LocationContext getLocationContext() { return LOCATION_VOLUME; }
protected:
    VolumeParticleContext(Feature *feature);
    double         *diffusionRate;
};


class MembraneParticleContext : public ParticleContext
{
public:
    double setMembraneDiffusionRate();
    void setLocalCoord(double u, double v);
    virtual WorldCoord getWorldCoord(LocalCoord lc, long index)=0;
    virtual LocationContext getLocationContext() { return LOCATION_MEMBRANE; }
    
protected:
    MembraneParticleContext(Feature *feature);
    double         *diffusionRate;
};

class ContourParticleContext : public ParticleContext
{
public:
    WorldCoord getWorldCoord(LocalCoord lc, long index);
    virtual void resolveReferences(Simulation *sim);
    virtual bool move();
    virtual bool captureParticles();
    virtual bool releaseParticles();
    virtual LocationContext getLocationContext() { return LOCATION_CONTOUR; }
    
protected:
    ContourParticleContext(Feature *feature, int AnumTypes, int AnumStates);

    double getReleaseRate(int state, int type);
    double getCaptureRate(int state, int type);
    double getCaptureRadius(int state);
    double getSpeed(int state, int type);

    void setReleaseRate(int state, int type, double value){releaseRate[state][type]=value;}
    void setCaptureRate(int state, int type, double value){captureRate[state][type]=value;}
    void setCaptureRadius(int state, double value){captureRadius[state]=value;}
    void setSpeed(int state, int type, double value){speed[state][type]=value;}

    int numStates;
    int numTypes;
private:  
    double         **releaseRate;
    double         **captureRate;
    double         *captureRadius;
    double         **speed;
};

class Granule : public VolumeParticleContext
{
public:
    void resolveReferences(Simulation *sim);
    bool react();
        
protected:
    Granule(Feature *feature);
    Variable *RNA;
    
    double onRate;
    double offRate;
    double stoichiometry;
};

#endif
