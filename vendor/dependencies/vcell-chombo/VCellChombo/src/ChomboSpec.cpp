#include <VCELL/ChomboSpec.h>
#include <iostream>
using std::endl;

int ChomboSpec::defaultTagsGrow = 2;

ChomboSpec::ChomboSpec()
{
	numLevels = 1;
	viewLevel = 0;
	refRatios = new int[1];
	refRatios[0] = 2;	
	maxBoxSize = 32;
	fillRatio = 0.9;
	relTol = 1e-9;
	bActivateFeatureUnderDevelopment = false;
	smallVolfracThreshold = 0;
	blockFactor = 4;

	chomboGeometry = new ChomboGeometry();
	
#ifdef CH_MPI
	bSaveVCellOutput = false;
	bSaveChomboOutput = true;
#else
	bSaveVCellOutput = true;
	bSaveChomboOutput = false;
#endif
}

ChomboSpec::~ChomboSpec()
{
	delete chomboGeometry;
	delete[] refRatios;
	for (vector<ChomboRefinementRoi*>::iterator it = membraneRefinementRois.begin(); it != membraneRefinementRois.end(); ++ it)
	{
		delete *it;
	}
	for (vector<ChomboRefinementRoi*>::iterator it = volumeRefinementRois.begin(); it != volumeRefinementRois.end(); ++ it)
	{
		delete *it;
	}
}

static void printRoi(ChomboRefinementRoi* roi)
{
	pout() << "\tlevel:" << roi->getLevel() << "; expression:" << roi->getRoi()->infix() << ";";
	const double* c = roi->getConstantValue();
  if (c != NULL)
	{
		pout() << "constant=" << *c << ";";
	}
	pout() << endl;
}

void ChomboSpec::printSummary()
{
	pout() << "numLevels=" << numLevels << endl;
	pout() << "refRatios";
	for (int i = 0; i < numLevels; ++ i)
	{
		pout() << " " << refRatios[i];
	}
	pout() << endl;
	pout() << "relTol=" << relTol << endl;
	pout() << "maxBoxSize=" << maxBoxSize << endl;
	pout() << "fillRatio=" << fillRatio << endl;
	pout() << "viewLevel=" << viewLevel << endl;
	pout() << "bSaveVCellOutput=" << bSaveVCellOutput << endl;
	pout() << "bSaveChomboOutput=" << bSaveChomboOutput << endl;
	pout() << "bActivateFeatureUnderDevelopment=" << bActivateFeatureUnderDevelopment << endl;
	pout() << "smallVolfracThreshold=" << smallVolfracThreshold << endl;
	pout() << "blockFactor=" << blockFactor << endl;
	pout() << "tagsGrow=" << tagsGrow << endl;
	pout() << "Membrane ROIs: " << membraneRefinementRois.size() << endl;
	for (vector<ChomboRefinementRoi*>::iterator it = membraneRefinementRois.begin(); it != membraneRefinementRois.end(); ++ it)
	{
		printRoi(*it);
	}
	pout() << "Volume ROIs: " << volumeRefinementRois.size() << endl;
	for (vector<ChomboRefinementRoi*>::iterator it = volumeRefinementRois.begin(); it != volumeRefinementRois.end(); ++ it)
	{
		printRoi(*it);
	}
}
