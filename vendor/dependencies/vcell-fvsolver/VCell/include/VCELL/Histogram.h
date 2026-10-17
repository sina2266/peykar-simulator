#ifndef HISTOGRAM_H
#define HISTOGRAM_H

#include <VCELL/SimTypes.h>

class Simulation;
class VolumeVariable;
class VolumeParticleContext;
class MembraneParticleContext;
class ContourParticleContext;

class  Histogram {
public:
    Histogram(Simulation *sim, VolumeVariable *AvolVar, int AparticleState, LocationContext Acontext);
    Histogram(Simulation *sim, VolumeVariable *AvolVar, LocationContext Acontext);
    
    void load();

private:

    void clear();
    void loadVolumeParticles(VolumeParticleContext *pc);
    void loadMembraneParticles(MembraneParticleContext *pc);
    void loadContourParticles(ContourParticleContext *pc);
    
    VolumeVariable *volVar;
    int             particleState;
    LocationContext locationContext;
};

#endif
