/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/Mesh.h>
#include <VCELL/Element.h>
#include <VCELL/SparseMatrixPCG.h>

Mesh::Mesh(double AcaptureNeighborhood)
{
	numMembrane = 0;
	numVolume = 0;
	pMembraneElement = NULL;
	pVolumeElement = NULL;
	//contourElements.erase(contourElements.begin(), contourElements.end());
	//volumeLists = NULL;
	captureNeighborhood = AcaptureNeighborhood;
	membraneElementCoupling= NULL;
}

Mesh::~Mesh()
{
	delete[] pVolumeElement;
	delete[] pMembraneElement;
	delete membraneElementCoupling;

	for (map<long, double*>::iterator iter = membrane_boundary_flux.begin(); iter != membrane_boundary_flux.end(); iter ++) {
		delete[] iter->second;
	}
	membrane_boundary_flux.clear();
}

VolumeElement *Mesh::getVolumeElements() 
{ 
	if (!pVolumeElement) {
		throw "mesh has not been initialized";
	}
	return pVolumeElement; 
}

long Mesh::getNumVolumeElements() 
{ 
	if (!pVolumeElement) {
		throw "mesh has not been initialized";
	}
	return numVolume;
}

MembraneElement *Mesh::getMembraneElements() 
{ 
	if (!pMembraneElement) {
		throw "mesh has not been initialized";
	}
	return pMembraneElement; 
}

long Mesh::getNumMembraneElements() 
{ 
	if (!pMembraneElement) {
		throw "mesh has not been initialized";
	}
	return numMembrane;
}

//void Mesh::addContourSubdomain(ContourSubdomain *cs)
//{
//	pContourSubdomains.push_back(cs);							
//}
//
//ContourSubdomain* Mesh::getContourSubdomain(int i)
//{
//	int numContourSubdomains = (int)pContourSubdomains.size();
//	ASSERTION((i<numContourSubdomains)&&(i>=0));
//	return pContourSubdomains[i]; 
//}
//
//bool Mesh::sampleContours()
//{
//	//contours are discretized into contourElements; each element belongs to a certain volume
//	//element; the element length equals characteristicLength of a contourDomain unless 
//	//the element intersects volume boundaries; in the latter case dichotomy is used 
//	//to determine within a given tolerance the end of the current element and the beginning 
//	//of the next one
// 
//	printf("started sampling contours...\n");
//
//	int numContourSubdomains = getNumContourSubdomains();
//	if(numContourSubdomains == 0){
//		printf("no contourSubdomains exist\n");
//		return true; 
//	}
//
//	long index = 0;
//	double u_tolerance = 1e-5;
//	long currVolumeIndex = 0;
//	ContourElement contourElement;
//	VCellModel *model = SimTool::getInstance()->getModel();
//	ASSERTION(model);
//
//	for(int i=0; i<numContourSubdomains; i++){
//		printf("sampling contour #%d \n", i);
//		double u = 0;
//		ContourSubdomain *contourSubdomain = getContourSubdomain(i);
//		double _length = contourSubdomain->getLength();
//		ASSERTION(_length>0);
//
//		if(getVolumeIndex(contourSubdomain->getCoord(_length))!=               //make sure
//				getVolumeIndex(contourSubdomain->getCoord(_length-2*u_tolerance))){ //that the last
//			_length -= 2*u_tolerance;                                       //element does 
//		}                                                          //not consist of one point
//
//		double charLength = contourSubdomain->getCharacteristicLength();
//		if(_length <= charLength){charLength = _length/2;}
//
//		ContourHandle handle = contourSubdomain->getContourHandle();
//
//		bool isFirst = true;
//		while(u < _length){
//		//
//		//setting contour, elementIndex
//		//
//			contourElement.contour = model->getContour(handle);
//			contourElement.elementIndex = index;
//		//
//		//setting wc_begin, volumeIndex
//		//
//			double du = charLength;
//			WorldCoord wc = contourSubdomain->getCoord(u);
//			WorldCoord wc1 = contourSubdomain->getCoord(u+2*u_tolerance);//make sure
//			long volIndex = getVolumeIndex(wc);                          //that the current 
//			long volIndex1 = getVolumeIndex(wc1);                        //element
//			if(volIndex!=volIndex1){                                     //does not 
//				u += 2*u_tolerance;                                      //consist
//				contourElement.wc_begin = wc1;                           //of one point  
//				contourElement.volumeIndex = currVolumeIndex = volIndex1;//
//			}else{                                                       //
//				contourElement.wc_begin = wc;
//				contourElement.volumeIndex = currVolumeIndex = volIndex;
//			}
//
//		//
//		//setting wc_end, contourBorder, elementLength
//		//
//			contourElement.border = CONTOUR_INTERIOR;
//			if(isFirst){
//				contourElement.border = CONTOUR_BEGIN;
//				isFirst = false;
//			}	  
//			if((u+du)<_length){
//				wc = contourSubdomain->getCoord(u+du);
//				if(getVolumeIndex(wc)==currVolumeIndex){
//					contourElement.wc_end = wc;
//					contourElement.elementLength = du;
//					u += du;
//					if(u+2*u_tolerance>_length){
//						u = _length;
//						contourElement.border = CONTOUR_END;
//					}
//				}else{
//					double u_minus = u;
//					double u_plus = u+du;
//					do{
//						du /= 2;
//						wc = contourSubdomain->getCoord(u_minus+du);
//						if(getVolumeIndex(wc)==currVolumeIndex){
//							u_minus += du;
//						}else{ 
//							u_plus -= du;
//						}   
//					}while(du>u_tolerance);
//					wc = contourSubdomain->getCoord(u_minus);
//					contourElement.wc_end = wc;
//					contourElement.elementLength = u_minus-u;
//					u = u_plus;
//				}
//			}else{
//				wc = contourSubdomain->getCoord(_length);
//				if(getVolumeIndex(wc)==currVolumeIndex){
//					contourElement.wc_end = wc;
//					contourElement.elementLength = _length-u;
//					u = _length;
//					contourElement.border = CONTOUR_END;
//				}else{
//					double u_minus = u;
//					double u_plus = _length;
//					du = _length-u;
//					do{
//						du /= 2;
//						wc = contourSubdomain->getCoord(u_minus+du);
//						if(getVolumeIndex(wc)==currVolumeIndex){
//							u_minus += du;
//						}else{ 
//							u_plus -= du;
//						}   
//					}while(du>u_tolerance);
//					wc = contourSubdomain->getCoord(u_minus);
//					contourElement.wc_end = wc;
//					contourElement.elementLength = u_minus-u;
//					u = u_plus;
//				} 
//			}
//
//			//
//			//add new contourElement
//			//
//			contourElements.push_back(contourElement);
//			index++;	
//		}
//	}
//	printf("contour sampling is done!\n");
//	return true;
//}
//
//ContourElement* Mesh::getContourElement(long index)
//{
//	return &contourElements[index];
//}
//
//void Mesh::addElementToVolumeList(long volumeIndex, ContourElement *element)
//{
//	volumeLists[volumeIndex].push_back(element);
//}
