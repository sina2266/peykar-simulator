#include <VCELL/Structure.h>
#include <VCELL/Region.h>

Structure::Structure(string& Aname) {
	name = Aname;
	numElements = 0;

	for (int i = 0; i < 6; i ++) {
		boundaryType[i] = BOUNDARY_VALUE;
	}
}

int Structure::getNumElements() {
	if (numElements == 0) {
		int num = 0;
		for (int i = 0; i < (int)regionList.size(); i ++) {
			num += regionList[i]->getNumElements();
		}
	}
	return numElements;
}

void Structure::addRegion(Region* r) {
	regionList.push_back(r);
	numElements = 0;
}
