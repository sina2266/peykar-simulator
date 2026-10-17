#include <VCELL/Histogram.h>
#include <VCELL/Simulation.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/VCellModel.h>
#include <VCELL/SimTool.h>
#include <VCELL/Feature.h>
#include <VCELL/ParticleContext.h>

//=======================================================
//   class  Histogram
//=======================================================
Histogram::Histogram(Simulation *sim, VolumeVariable *AvolVar, int AparticleState, LocationContext Acontext)
{
	volVar = AvolVar;
	particleState = AparticleState;
	locationContext = Acontext;
}

Histogram::Histogram(Simulation *sim, VolumeVariable *AvolVar, LocationContext Acontext)
{
	volVar = AvolVar;
	particleState = PARTICLE_ALL;
	locationContext = Acontext;
}
    
void Histogram::clear() { 
	for (int i=0;i<volVar->getSize();i++){
		volVar->setCurr(i, 0.0);
	}
}
        
void Histogram::load()
{
	VCellModel *model = SimTool::getInstance()->getModel();

	clear();

	Feature *feature = NULL;
	while ((feature=model->getNextFeature(feature))!=NULL){
		if (locationContext==LOCATION_VOLUME || locationContext==LOCATION_ALL){
			loadVolumeParticles(feature->getVolumeParticleContext());
		}
		if (locationContext==LOCATION_MEMBRANE || locationContext==LOCATION_ALL){
			loadMembraneParticles(feature->getMembraneParticleContext());
		}
		if (locationContext==LOCATION_CONTOUR || locationContext==LOCATION_ALL){
			loadContourParticles(feature->getContourParticleContext());
		}
	}
}

void Histogram::loadVolumeParticles(VolumeParticleContext *pc)
{
	if(pc!=NULL){  
		for (int i=0;i<pc->getNumParticles();i++){
			Particle *particle = pc->getParticle(i);
			if (particle->getState()==particleState || particleState==PARTICLE_ALL){
				((volVar->getCurr())[particle->getIndex()])++;
			}
		}
	}   
}

void Histogram::loadContourParticles(ContourParticleContext *pc)
{
	Mesh *mesh = SimTool::getInstance()->getSimulation()->getMesh();
	for (int i=0;i<pc->getNumParticles();i++){
			Particle *particle = pc->getParticle(i);
		if (particle->getState()==particleState || particleState==PARTICLE_ALL){
			((volVar->getCurr())[mesh->getContourElement(particle->getIndex())->getVolumeIndex()])++;
		}
	}
}

void Histogram::loadMembraneParticles(MembraneParticleContext *pc)
{
	throw "Histogram::loadMembraneParticles(), not supported yet";
}
