/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <math.h>
#include <VCELL/SimTypes.h>
#include <VCELL/ContourSubdomain.h>
#include <assert.h> 

//----------------------------------------------------------------------------
//
// class ContourSubdomain
//
//----------------------------------------------------------------------------
ContourSubdomain::ContourSubdomain(ContourHandle AcontourHandle, 
                                   FeatureHandle AfeatureHandle,
                                   double AcharacteristicLength)
{
	characteristicLength = AcharacteristicLength;
	contourHandle = AcontourHandle;
	featureHandle = AfeatureHandle;
}

//----------------------------------------------------------------------------
//
// class ListContour
//
//----------------------------------------------------------------------------
ListContour::ListContour(ContourHandle AcontourHandle, 
                         FeatureHandle AfeatureHandle,
                         double AcharacteristicLength)
:ContourSubdomain(AcontourHandle, AfeatureHandle, AcharacteristicLength)
{
	contourPoints.erase(contourPoints.begin(), contourPoints.end());
	length.erase(length.begin(), length.end());
}

void ListContour::addPoint(WorldCoord wc)
{
	double length_increment;
	int n = (int)contourPoints.size();
	double lngth = getLength();
	contourPoints.push_back(wc);

	if(n>0){
		length_increment 
			= sqrt((contourPoints[n-1].x - wc.x)*(contourPoints[n-1].x - wc.x)
					+(contourPoints[n-1].y - wc.y)*(contourPoints[n-1].y - wc.y)
					+(contourPoints[n-1].z - wc.z)*(contourPoints[n-1].z - wc.z));
			
		length.push_back(lngth+length_increment);
	}
}

double ListContour::getLength()
{
	if(length.size()>0){
		return length[length.size()-1];
	}else{
		return 0;
	}
}

WorldCoord ListContour::getCoord(double u)
{
	ASSERTION((u>=0)&&(u<=getLength()));

	WorldCoord wc;
	double t;

	if( u == 0){
		wc.x = contourPoints[0].x;
		wc.y = contourPoints[0].y;
		wc.z = contourPoints[0].z;
	} else if(u == getLength()){
		wc.x = contourPoints[contourPoints.size()-1].x;
		wc.y = contourPoints[contourPoints.size()-1].y;
		wc.z = contourPoints[contourPoints.size()-1].z;

	}else{
		for(int i = 0; i < (int)length.size(); i ++){
			if(length[i] > u){
				if(i == 0){
					t = u /length[i];
				}else{
					t = (u - length[i-1])/(length[i] - length[i-1]);
				}
				wc.x = contourPoints[i].x 
						+ (contourPoints[i+1].x - contourPoints[i].x)*t;
				wc.y = contourPoints[i].y
						+ (contourPoints[i+1].y - contourPoints[i].y)*t;  
				wc.z = contourPoints[i].z
						+ (contourPoints[i+1].z - contourPoints[i].z)*t;
				break;	     
			}
		}
	}
	return wc;
}
