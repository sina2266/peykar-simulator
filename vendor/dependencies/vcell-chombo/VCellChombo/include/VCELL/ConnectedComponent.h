/* 
 * File:   ConnectedComponent.h
 * Author: developer
 *
 * Created on April 7, 2013, 4:08 PM
 */

#ifndef CONNECTEDCOMPONENT_H
#define	CONNECTEDCOMPONENT_H

class Feature;
template<class> class Vector;
template<class> class RefCountedPtr;

struct ConnectedComponent {
	int phase;
	int volumeIndexInPhase;
	Feature* feature;
	RefCountedPtr<EBIndexSpace> volume;
	Vector<ConnectedComponent*> adjacentVolumes;

	ConnectedComponent()
	{
		feature = NULL;
	}
};

#endif	/* CONNECTEDCOMPONENT_H */

