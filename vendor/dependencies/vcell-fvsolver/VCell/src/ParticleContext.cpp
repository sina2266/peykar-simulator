/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <math.h>
#include <stdlib.h>
#include <VCELL/SimTypes.h>
#include <VCELL/VCellModel.h>
#include <VCELL/Element.h>
#include <VCELL/Solver.h>
#include <VCELL/VolumeVariable.h>
#include <VCELL/Feature.h>
#include <VCELL/EqnBuilder.h>
#include <VCELL/Simulation.h>
#include <VCELL/ParticleContext.h>
#include <VCELL/CartesianMesh.h>
typedef vector<ContourElement*> ContourElementList;

#ifndef M_PI
#define M_PI 3.14159265359
#endif

//-----------------------------------------------------------------
//
//  class Particle
//
//-----------------------------------------------------------------
char **Particle::names = NULL;
int Particle::numStates = 0;

Particle::Particle()
{
	index = 0;
	lc.u = 0;
	lc.v = 0;
	lc.w = 0;
	
	state = 0;
	pc = NULL;
}

void Particle::setLocation(LocalCoord coord, long Aindex, ParticleContext *Apc)
{
	lc = coord;
	index = Aindex;
	pc = Apc;
}

void Particle::setLocation(LocalCoord coord, long Aindex)
{
	lc = coord;
	index = Aindex;
}

LocalCoord Particle::getLocalCoord()
{
    return lc;
}

WorldCoord Particle::getWorldCoord()
{
    LocalCoord coord = lc;
    long ind = index;
    return pc->getWorldCoord(coord, ind);
}

void Particle::setState(int st)
{
	state = st;
}

void Particle::write(FILE *fp)
{
	WorldCoord wc = getWorldCoord();
	fprintf(fp,"%lg %lg %lg %d %d\n", 
				wc.x, wc.y, wc.z, getState(), (int)pc->getLocationContext());
}


long Particle::getIndex()
{
	return index;
}

int Particle::getState()
{
	return state;
}

void Particle::setParticleTypes(int numTypes, char **nameArray)
{
	numStates = numTypes;
	names = nameArray; 
}

//-----------------------------------------------------------------
//
//  class ParticleContext
//
//-----------------------------------------------------------------
ParticleContext::ParticleContext(Feature *Afeature)
{
	feature = Afeature;
	ASSERTION(feature);   
	   
	initialValue = NULL;

	sim = NULL;
	mesh = NULL;
	   
	particles.erase(particles.begin(), particles.end());    
	numParticles = 0;
}

void ParticleContext::resolveReferences(Simulation *Asim)
{
	sim = Asim;
	mesh = sim->getMesh();
	   
	if (!sim) {		
		throw "ParticleContext::resolveReferences(), simulation is null";
	}
	if (!mesh) {
		throw "ParticleContext::resolveReferences(), mesh is null";
	}
}

double ParticleContext::getInitialValue(long)
{
	if (initialValue){
		return *initialValue;
	}
	ASSERTION(0);
	return 0;
}

double ParticleContext::getRandomUniform()
{
#if ( !defined(WIN32) && !defined(WIN64) )
	long r = random();
	srand48(r);
   
	return drand48();
#else
	const long NUM_RAND = RAND_MAX+1;
	const double RAND_TOTAL_MAX = NUM_RAND*NUM_RAND - 1.0;
	long r = (NUM_RAND)*rand()+rand();
	return r / RAND_TOTAL_MAX;
#endif
}

double ParticleContext::getRandomNormal()
{
	// "inverse"

	double alpha;
	double beta;
	alpha = getRandomUniform();
	beta = getRandomUniform();
	
	return sqrt(-2*log(alpha))*cos(2*M_PI*beta);

	/*
	//  "limit"

	const int N = 10;
	const double coeff = 2*sqrt(3*N);
	double sum = 0.0;
	for(int i=0; i<N; i++){
			sum = sum + getRandomNormal();
	}

	return coeff * (sum/N-0.5);
	*/         
}

void ParticleContext::write(FILE *fp)
{
	for (int i=0;i<numParticles;i++){
		particles[i]->write(fp);
	}
}
/*
void ParticleContext::write(FILE *fp)
{
   for (int i=0;i<numParticles;i++){
       WorldCoord wc = particles[i]->getWorldCoord();
       fprintf(fp,"%lg %lg %lg %d %d\n", 
               wc.x, wc.y, wc.z, particles[i]->getState(), (int)getLocationContext());
   }
}
*/

void ParticleContext::addParticle(Particle *particle)
{ 
	particles.push_back(particle); 
}

Particle* ParticleContext::getParticle(long index)
{
	ASSERTION((index>=0)&&(index<particles.size()));
	return particles[index];
}

void ParticleContext::moveParticleTo(long index, ParticleContext *pc)
{
		pc->addParticle(getParticle(index));
		removeParticle(index);
}

bool ParticleContext::react()
{
	return true;
}

LocalCoord ParticleContext::setLocalCoord(double u, double v, double w)
{
	LocalCoord lc;
	lc.u = u;
	lc.v = v;
	lc.w = w;
	return lc;
}

//-----------------------------------------------------------------
//
//  class VolumeParticleContext
//
//-----------------------------------------------------------------
VolumeParticleContext::VolumeParticleContext(Feature *Afeature)
: ParticleContext(Afeature)
{
	diffusionRate = NULL;
}

double VolumeParticleContext::setDiffusionRate()
{
	if (diffusionRate){
		return *diffusionRate;
	}
	ASSERTION(0);
	return 0;
}

long VolumeParticleContext::getRandomIndex()
{
	long index;
	VolumeElement *element;
	long count =0;
	long numVolEl = mesh->getNumVolumeElements(); 
	do{
		count++; 
#if ( !defined(WIN32) && !defined(WIN64) )
		long randVar = random();
#else
		const long NUM_RAND = RAND_MAX+1;
		long randVar = ((NUM_RAND)*rand()+rand());
#endif
		index = randVar%numVolEl;
		element = mesh->getVolumeElements()+index;
		ASSERTION(count++<1e6);
	}while(element->feature != feature);
	return index;
}

WorldCoord VolumeParticleContext::getWorldCoord(LocalCoord lc, long index)
{
	WorldCoord wc;
	wc.x = lc.u;
	wc.y = lc.v;
	wc.z = lc.w;
	return wc;
}

bool VolumeParticleContext::move()
{
	double DOMAIN_X = ((CartesianMesh *)mesh)->getDomainSizeX();
	double DOMAIN_Y = ((CartesianMesh *)mesh)->getDomainSizeY();
	//      double DOMAIN_Z = mesh->getGeometry()->getDomainSizeZ();
	double DOMAIN_START_X = ((CartesianMesh *)mesh)->getDomainOriginX();
	double DOMAIN_START_Y = ((CartesianMesh *)mesh)->getDomainOriginY();
	//      double DOMAIN_START_Z = mesh->getGeometry()->getDomainOriginZ();
	double DELTATIME = sim->getDT_sec();
	double deviation = sqrt(*diffusionRate *2* DELTATIME);
	    
	double ldist_x;
	double rdist_x;
	double ldist_y;
	double rdist_y;
	//    double ldist_z;
	//    double rdist_z;
	int numX = ((CartesianMesh *)mesh)->getNumVolumeX();
	int numY = ((CartesianMesh *)mesh)->getNumVolumeY();
	//    int numZ = ((CartesianMesh *)mesh)->getNumVolumeZ();

	const double DELTAX = ((CartesianMesh *)mesh)->getXScale_um();
	const double DELTAY = ((CartesianMesh *)mesh)->getYScale_um();
	//const double DELTAZ = ((CartesianMesh *)mesh)->getZScale_um();

	if(getNumParticles()<=0){
		return true;
	}
    
	for(int i=0; i<getNumParticles(); i++){
		WorldCoord wc = particles[i]->getWorldCoord();
		long index = particles[i]->getIndex();
		WorldCoord vwc = mesh->getVolumeWorldCoord(index);
		    
		VolumeElement *element = mesh->getVolumeElements()+index;

		if (element->neighborMask & NEIGHBOR_XM_BOUNDARY){
			ldist_x = wc.x - vwc.x;   
			rdist_x = vwc.x + DELTAX/2 - wc.x;

		}else if(element->neighborMask & NEIGHBOR_XP_BOUNDARY){
			ldist_x = wc.x - vwc.x + DELTAX/2;   
			rdist_x = vwc.x - wc.x;   
		    
		}else{
			ldist_x = wc.x - vwc.x + DELTAX/2;   
			rdist_x = vwc.x + DELTAX/2 - wc.x;   
		}

		double increment = deviation * getRandomNormal();
		while((increment>rdist_x)||(increment<-ldist_x)){
			if(increment>rdist_x){
				if(!(element->neighborMask & NEIGHBOR_XP_MASK)){ 
					wc.x += rdist_x;
					increment -= rdist_x;
					element++;
					ldist_x = 0.0;
					if(element->neighborMask & NEIGHBOR_XP_BOUNDARY){
						rdist_x = DELTAX/2;
					}else{
						rdist_x = DELTAX;
					}
				}else if(element->neighborMask & NEIGHBOR_XP_BOUNDARY){
					wc.x += rdist_x;
					increment = -increment + rdist_x;
					rdist_x = 0.0;
					ldist_x = DELTAX/2;
				}else{
					wc.x += rdist_x;
					increment = -increment + rdist_x;
					rdist_x = 0.0;
					ldist_x = DELTAX;
				}
	         
			}
			if(increment<-ldist_x){
				if(!(element->neighborMask & NEIGHBOR_XM_MASK)){ 
					wc.x -= ldist_x;
					increment += ldist_x;
					element--;
					rdist_x = 0.0;
					if(element->neighborMask & NEIGHBOR_XM_BOUNDARY){
						ldist_x = DELTAX/2;
					}else{
						ldist_x = DELTAX;
					}
				}else if(element->neighborMask & NEIGHBOR_XM_BOUNDARY){
					wc.x -= ldist_x;
					increment = -increment - ldist_x;
					ldist_x = 0.0;
					rdist_x = DELTAX/2;
				}else{
					wc.x -= ldist_x;
					increment = -increment - ldist_x;
					ldist_x = 0.0;
					rdist_x = DELTAX;
				}
			         
			}
		}	  
		wc.x += increment;
		ASSERTION(wc.x>=DOMAIN_START_X);
		ASSERTION(wc.x<=DOMAIN_X);
		//printf("x=%lg, y=%lg, z=%lg, handle=%d, feature1=%p, feature2=%p\n", wc.x, wc.y, wc.z, 
		//                                           geo->getFeatureHandle(wc), feature, 
		//					   mesh->getVolumeElements()[mesh->getVolumeIndex(wc)].feature); 
		ASSERTION(mesh->getVolumeElements()[mesh->getVolumeIndex(wc)].feature == feature);
      
      
		if (element->neighborMask & NEIGHBOR_YM_BOUNDARY){
			ldist_y = wc.y - vwc.y;   
			rdist_y = vwc.y + DELTAY/2 - wc.y;   

		}else if(element->neighborMask & NEIGHBOR_YP_BOUNDARY){
			ldist_y = wc.y - vwc.y + DELTAY/2;   
			rdist_y = vwc.y - wc.y;   
	      
		}else{
			ldist_y = wc.y - vwc.y + DELTAY/2;   
			rdist_y = vwc.y + DELTAY/2 - wc.y;   
		}

		increment = deviation * getRandomNormal();
		while((increment>rdist_y)||(increment<-ldist_y)){
		if(increment>rdist_y){
			if(!(element->neighborMask & NEIGHBOR_YP_MASK)){ 
				wc.y += rdist_y;
				increment -= rdist_y;
				element += numX;
				ldist_y = 0.0;
				if(element->neighborMask & NEIGHBOR_YP_BOUNDARY){
					rdist_y = DELTAY/2;
				}else{
					rdist_y = DELTAY;
				}
			}else if(element->neighborMask & NEIGHBOR_YP_BOUNDARY){
				wc.y += rdist_y;
				increment = -increment + rdist_y;
				rdist_y = 0.0;
				ldist_y = DELTAY/2;
			}else{
				wc.y += rdist_y;
				increment = -increment + rdist_y;
				rdist_y = 0.0;
				ldist_y = DELTAY;
			}
		}
		if(increment<-ldist_y){
			if(!(element->neighborMask & NEIGHBOR_YM_MASK)){ 
				wc.y -= ldist_y;
				increment += ldist_y;
				element -= numX;
				rdist_y = 0.0;
				if(element->neighborMask & NEIGHBOR_YM_BOUNDARY){
					ldist_y = DELTAY/2;
				}else{
					ldist_y = DELTAY;
				}
			}else if(element->neighborMask & NEIGHBOR_YM_BOUNDARY){
				wc.y -= ldist_y;
				increment = -increment - ldist_y;
				ldist_y = 0.0;
				rdist_y = DELTAY/2;
			}else{
				wc.y -= ldist_y;
				increment = -increment - ldist_y;
				ldist_y = 0.0;
				rdist_y = DELTAY;
			}
		         
			}
		}	  
		wc.y += increment;
		ASSERTION(wc.y>=DOMAIN_START_Y);
		ASSERTION(wc.y<=DOMAIN_Y);
		ASSERTION(mesh->getVolumeElements()[mesh->getVolumeIndex(wc)].feature == feature);
            
		/*       
		if (element->neighborMask & NEIGHBOR_ZM_BOUNDARY){
		ldist_z = wc.z - vwc.z;   
		rdist_z = vwc.z + DELTAZ/2 - wc.z;   

		}else if(element->neighborMask & NEIGHBOR_ZP_BOUNDARY){
		ldist_z = wc.z - vwc.z + DELTAZ/2;   
		rdist_z = vwc.z - wc.z;   
	      
		}else{
		ldist_z = wc.z - vwc.z + DELTAZ/2;   
		rdist_z = vwc.z + DELTAZ/2 - wc.z;   
		}

		increment = deviation * getRandomNormal();
		while((increment>rdist_z)||(increment<-ldist_z)){
		if(increment>rdist_z){
			if(!(element->neighborMask & NEIGHBOR_ZP_MASK)){ 
				wc.z += rdist_z;
				increment -= rdist_z;
				element += numX*numY;
				ldist_z = 0.0;
				if(element->neighborMask & NEIGHBOR_ZP_BOUNDARY){
				rdist_z = DELTAZ/2;
				}else{
				rdist_z = DELTAZ;
				}
			}else if(element->neighborMask & NEIGHBOR_ZP_BOUNDARY){
			wc.z += rdist_z;
			increment = -increment + rdist_z;
			rdist_z = 0.0;
			ldist_z = DELTAZ/2;
			}else{
			wc.z += rdist_z;
			increment = -increment + rdist_z;
			rdist_z = 0.0;
			ldist_z = DELTAZ;
			}
		         
		}
		if(increment<-ldist_z){
			if(!(element->neighborMask & NEIGHBOR_ZM_MASK)){ 
				wc.z -= ldist_z;
				increment += ldist_z;
				element -= numX*numY;
				rdist_z = 0.0;
				if(element->neighborMask & NEIGHBOR_ZM_BOUNDARY){
				ldist_z = DELTAZ/2;
				}else{
				ldist_z = DELTAZ;
				}
			}else if(element->neighborMask & NEIGHBOR_ZM_BOUNDARY){
			wc.z -= ldist_z;
			increment = -increment - ldist_z;
			ldist_z = 0.0;
			rdist_z = DELTAZ/2;
			}else{
			wc.z -= ldist_z;
			increment = -increment - ldist_z;
			ldist_z = 0.0;
			rdist_z = DELTAZ;
			}
		         
		}
		}	  
		wc.z += increment;
		ASSERTION(wc.z>=DOMAIN_START_Z);
		ASSERTION(wc.z<=DOMAIN_Z);
		*/     
		index = mesh->getVolumeIndex(wc);
		LocalCoord lc;
		lc.u = wc.x;
		lc.v = wc.y;
		lc.w = wc.z;   
		particles[i]->setLocation(lc, index, this);
	}
	return true;
}

//-----------------------------------------------------------------
//
//  class MembraneParticleContext
//
//-----------------------------------------------------------------
MembraneParticleContext::MembraneParticleContext(Feature *Afeature)
: ParticleContext(Afeature)
{
	diffusionRate = NULL;
}

double MembraneParticleContext::setMembraneDiffusionRate()
{
	if (diffusionRate){
		return *diffusionRate;
	}
	ASSERTION(0);
	return 0;
}

//-----------------------------------------------------------------
//
//  class ContourParticleContext
//
//-----------------------------------------------------------------
ContourParticleContext::ContourParticleContext(Feature *Afeature, 
                                               int AnumTypes, 
                                               int AnumStates)
: ParticleContext(Afeature)
{
	numTypes = AnumTypes;
	numStates = AnumStates;
	releaseRate = new double*[numStates];
	int i=0;
	for(i=0; i<numStates; i++){
		releaseRate[i] = new double[numTypes];  
	}
	captureRate = new double*[numStates];
	for(i=0; i<numStates; i++){
		captureRate[i] = new double[numTypes];  
	}
	captureRadius = new double[numStates];
	 
	speed = new double*[numStates];
	for(i=0; i<numStates; i++){
		speed[i] = new double[numTypes];  
	}
}
 
WorldCoord ContourParticleContext::getWorldCoord(LocalCoord lc, long index)
{
	ContourElement *contourElement = mesh->getContourElement(index);
	double u = lc.u;
	WorldCoord wc = contourElement->getCoord(u);
	return wc;
}
 
double ContourParticleContext::getReleaseRate(int state, int type)
{
	return releaseRate[state][type];
}

double ContourParticleContext::getCaptureRate(int state, int type)
{
	return captureRate[state][type];
}

double ContourParticleContext::getCaptureRadius(int state)
{
	return captureRadius[state];
}

double ContourParticleContext::getSpeed(int state, int type)
{
	return speed[state][type];
}

bool ContourParticleContext::move()
{
	double DELTATIME = sim->getDT_sec();
	if(getNumParticles()==0){
		return true;
	}
	for(long i=0;i<getNumParticles();i++){
		Particle *particle = getParticle(i);
		if((particle->getParticleContext())!=this){
			printf("particle has wrong particleContext");
			return false;
		}
		int state = particle->getState();
		long contourIndex = particle->getIndex();
		long currContourIndex = contourIndex;
		ContourElement *contourElement = mesh->getContourElement(contourIndex);
		int type = (contourElement->getContour())->getType();
		double speed = getSpeed(state, type);
		double distance = speed*DELTATIME;
    
		if(speed>0.0){
			LocalCoord lc = particle->getLocalCoord();
			double u = distance;
			WorldCoord wc_end = contourElement->getEnd();
			double length = (contourElement->getElementLength())-lc.u;
			while(u>length){
				if(contourElement->getBorder()==CONTOUR_END){
					VolumeParticleContext *vpc = feature->getVolumeParticleContext();
					if(!vpc){
						printf("VolumeParticleContext is undefined");
						return false;
					}
					lc = vpc->setLocalCoord(wc_end.x, wc_end.y, wc_end.z);
					particle->setLocation(lc, contourElement->getVolumeIndex(), vpc);
					moveParticleTo(i, vpc);
					i--;
					u = -1;
				}else{
					u-=length;
					currContourIndex++;
					contourElement = mesh->getContourElement(currContourIndex);
					wc_end = contourElement->getEnd();
					length = contourElement->getElementLength();
				}
			}
			if(particle->getParticleContext()==this){
				if(currContourIndex==contourIndex){
					lc.u += distance;
					particle->setLocation(lc, contourIndex);
				}else{
					lc.u = u; 
					particle->setLocation(lc, currContourIndex);
				}
			}
		}else{
			LocalCoord lc = particle->getLocalCoord();
			double u = -distance;
			WorldCoord wc_begin = contourElement->getBegin();
			double length = lc.u;
			while(u>length){
				if(contourElement->getBorder()==CONTOUR_BEGIN){
					VolumeParticleContext *vpc = feature->getVolumeParticleContext();
					if(!vpc){
						printf("VolumeParticleContext is undefined");
						return false;
					}
					lc = vpc->setLocalCoord(wc_begin.x, wc_begin.y, wc_begin.z);
					particle->setLocation(lc, contourElement->getVolumeIndex(), vpc);
					moveParticleTo(i, vpc);
					i--;
					u = -1;
				}else{
					u-=length;
					currContourIndex--;
					contourElement = mesh->getContourElement(currContourIndex);
					wc_begin = contourElement->getBegin();
					length = contourElement->getElementLength();
				}
			}
			if(particle->getParticleContext()==this){
				if(currContourIndex==contourIndex){
					lc.u += distance;
					particle->setLocation(lc, contourIndex);
				}else{
					lc.u = length-u; 
					particle->setLocation(lc, currContourIndex);
				}
			}
		}
	}
	return true;
}

bool ContourParticleContext::releaseParticles()
{
	double DELTATIME = sim->getDT_sec();
	for(long i=0;i<getNumParticles();i++){
		Particle *particle = getParticle(i);
		int state = particle->getState();
		ContourElement *contourElement = mesh->getContourElement(particle->getIndex());
		int type = (contourElement->getContour())->getType();
		double releaseRate  = getReleaseRate(state, type);
	    
		if(releaseRate){
			double r = getRandomUniform();
			ASSERTION(releaseRate*DELTATIME<1);
			double probability = releaseRate*DELTATIME;
			if(r<probability){
				WorldCoord wc = particle->getWorldCoord();
				long volumeIndex = (mesh->getContourElement(particle->getIndex()))
								->getVolumeIndex();
				Feature *feature = getParent();
				VolumeParticleContext *vpc = feature->getVolumeParticleContext();
				if(!vpc){
					printf("VolumeParticleContext is undefined");
					return false;
				}
				LocalCoord lc = vpc->setLocalCoord(wc.x, wc.y, wc.z);
				particle->setLocation(lc, volumeIndex, vpc);
				moveParticleTo(i, vpc);
				i--;
			}
		}
	}
	return true; 
}
/*
bool ContourParticleContext::captureParticles()
//the closest element is chosen for interaction
{
  double DELTATIME = sim->getDT_sec();
  Feature *feature = getParent();
  VolumeParticleContext *vpc = feature->getVolumeParticleContext();
  if(!vpc){
     printf("VolumeParticleContext is undefined");
     return false;
  }
  for(long i=0; i<vpc->getNumParticles(); i++){
    Particle *particle = vpc->getParticle(i);
    if(particle->getParticleContext()==vpc){
      int state = particle->getState();
      double minDistance = getCaptureRadius(state);
      ContourElement *minContElement;
      double minCaptureRate = 0;
      ContourElementList contourElements = mesh->getContourElementList(particle->getIndex());
      for(int j=0; j<contourElements.size(); j++){
        int type = (contourElements[j]->getContour())->getType();
        double captureRate = getCaptureRate(state, type);
        if(captureRate){
          WorldCoord wc = particle->getWorldCoord();
	  double distance = contourElements[j]->getDistancetoPoint(wc);
          if(distance<minDistance){
	    minDistance = distance;
	    minContElement = contourElements[j];
	    minCaptureRate = captureRate;
	  }
        }
      }
      if(minCaptureRate){    
        double r = getRandomUniform();
        double probability = minCaptureRate*DELTATIME;
        ASSERTION(probability<1);
        if(r<probability){
	   LocalCoord lc = setLocalCoord(0, 0, 0);
	   long contourElementIndex = minContElement->getElementIndex();
	   particle->setLocation(lc, contourElementIndex, this);
	   vpc->moveParticleTo(i, this);
	   i--;
        }
      }
    }    
  }
  return true;
}


bool ContourParticleContext::captureParticles()
//the interacting element is chosen randomly among elements within
//the interaction range
{
  double DELTATIME = sim->getDT_sec();
  Feature *feature = getParent();
  VolumeParticleContext *vpc = feature->getVolumeParticleContext();
  if(!vpc){
     printf("VolumeParticleContext is undefined");
     return false;
  }
  for(long i=0; i<vpc->getNumParticles(); i++){
    Particle *particle = vpc->getParticle(i);
    if(particle->getParticleContext()==vpc){
      int state = particle->getState();
      double radius = getCaptureRadius(state);
      ContourElementList contourElements = mesh->getContourElementList(particle->getIndex());
      ContourElementList activElements;
      for(int j=0; j<contourElements.size(); j++){
        int type = (contourElements[j]->getContour())->getType();
        double rate = getCaptureRate(state, type);
        if(rate){
          WorldCoord wc = particle->getWorldCoord();
	  double distance = contourElements[j]->getDistancetoPoint(wc);
          if(distance<radius){
	     activElements.push_back(contourElements[j]);
	  }
        }
      }
      if(activElements.size()){
	int k = (int)(getRandomUniform()*activElements.size());
        int activeType = (activElements[k]->getContour())->getType();
        double activeRate = getCaptureRate(state, activeType);
        double r = getRandomUniform();
        double probability = activeRate*DELTATIME;
        ASSERTION(probability<1);
        ASSERTION((k<contourElements.size())&&(k>=0));
        if(r<probability){
	   LocalCoord lc = setLocalCoord(0, 0, 0);
	   long contourElementIndex = activElements[k]->getElementIndex();
	   particle->setLocation(lc, contourElementIndex, this);
	   vpc->moveParticleTo(i, this);
	   i--;
        }
      }
    }    
  }
  return true;
}
*/
bool ContourParticleContext::captureParticles()
//the interacting element is chosen due to the Bayes formula
{
	double DELTATIME = sim->getDT_sec();
	Feature *feature = getParent();
	VolumeParticleContext *vpc = feature->getVolumeParticleContext();
	if(!vpc){
		printf("VolumeParticleContext is undefined");
		return false;
	}
	for(long i=0; i<vpc->getNumParticles(); i++){
		Particle *particle = vpc->getParticle(i);
		ASSERTION(particle->getParticleContext()==vpc);
		int state = particle->getState();
		double radius = getCaptureRadius(state);
		ContourElementList contourElements = mesh->getContourElementList(particle->getIndex());
		ContourElementList activElements;
		double sumLength = 0;  // sum of lengths of active elements
		vector<double> rates;  // capture rates of active elements
		for(int j = 0; j < (int)contourElements.size(); j ++){
			int type = (contourElements[j]->getContour())->getType();
			double rate = getCaptureRate(state, type);
			if(rate>0.0){
				WorldCoord wc = particle->getWorldCoord();
				double distance = contourElements[j]->getDistancetoPoint(wc);
				if(distance<radius){
					activElements.push_back(contourElements[j]);
					rates.push_back(rate);
					sumLength += contourElements[j]->getElementLength();
				}
			}
		}
		if(activElements.size()){
			double probability = 0;
			for(int j = 0; j < (int)activElements.size(); j ++){
				probability += (activElements[j]->getElementLength())*rates[j];
			}
			ASSERTION(probability<sumLength/DELTATIME);
			double r = getRandomUniform()*sumLength/DELTATIME;
			if(r<probability){
				int k = 0;
				double q = getRandomUniform()*probability;
				double integralPr = (activElements[k]->getElementLength())*rates[k];
				while(q>integralPr){
					k++;
					integralPr += (activElements[k]->getElementLength())*rates[k];
				}
				ASSERTION((k<activElements.size())&&(k>=0));
				LocalCoord lc = setLocalCoord(0, 0, 0);
				long contourElementIndex = activElements[k]->getElementIndex();
				particle->setLocation(lc, contourElementIndex, this);
				vpc->moveParticleTo(i, this);
				i--;
			}
		}    
	}
	return true;
}

//-----------------------------------------------------------------
//
//  class Granule
//
//-----------------------------------------------------------------
Granule::Granule(Feature *Afeature)
: VolumeParticleContext(Afeature)
{
	RNA = NULL;

	onRate = 0.0;
	offRate = 0.0;
	stoichiometry = 0.0;
}

void Granule::resolveReferences(Simulation *Asim)
{
	VolumeParticleContext::resolveReferences(Asim);
	ASSERTION(sim);
	RNA = (VolumeVariable*)sim->getVariableFromName("RNA");
	if (RNA==NULL){
		throw "Granule::resolveReferences(), \"RNA\" is null";
	}
}

bool Granule::react()
{
	double DELTATIME = sim->getDT_sec();
	for(int i=0; i<numParticles; i++){
		long index = particles[i]->getIndex();
		double densityBound = stoichiometry * conversionFactor /
							mesh->getVolumeOfElement_cu(index);
		switch(particles[i]->getState()){ 
			case 0:{
				if(RNA->getOld(index) > densityBound){
					ASSERTION(onRate*RNA->getOld(index)*DELTATIME < 1.0);
					double r = getRandomUniform();
					if (r < onRate*RNA->getOld(index)*DELTATIME){
						particles[i]->setState(1);
						double newValue = RNA->getOld(index) - densityBound;
						ASSERTION(newValue>0);
						RNA->setOld(index, newValue);
					}
				}
				break;
			}
			case 1:{
				ASSERTION(offRate*DELTATIME < 1.0);
				double r = getRandomUniform();
				if (r < offRate*DELTATIME){
					particles[i]->setState(0);
					double newValue = RNA->getOld(index) + densityBound;
					ASSERTION(newValue>0);
					RNA->setOld(index, newValue);
				}
				break;
			}
		} // end switch
	}
	return true;
}
