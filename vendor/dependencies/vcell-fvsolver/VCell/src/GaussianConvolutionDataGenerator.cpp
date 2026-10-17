/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/GaussianConvolutionDataGenerator.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Element.h>
#include <Expression.h>
using VCell::Expression;

#include <string.h>
#include <algorithm>
using std::max;
using std::min;

GaussianConvolutionDataGenerator::GaussianConvolutionDataGenerator(string& name, Feature* feature, double sigmaXY, double sigmaZ, Expression* volFunc, Expression* memFunc) 
	: DataGenerator(name, feature){
		this->sigmaXY = sigmaXY;
		this->sigmaZ = sigmaZ;
		sigmaRatio = sigmaZ/sigmaXY;
	this->volFunction = volFunc;
	this->memFunction = memFunc;
	gaussianPsfSamples = NULL;
	functionValues = NULL;
}

GaussianConvolutionDataGenerator::~GaussianConvolutionDataGenerator() {
	delete volFunction;
	delete memFunction;
	delete[] gaussianPsfSamples;
	delete[] functionValues;
}

void GaussianConvolutionDataGenerator::resolveReferences(SimulationExpression* sim) {
	volFunction->bindExpression(sim->getSymbolTable());
	memFunction->bindExpression(sim->getSymbolTable());
	// compute data size
	CartesianMesh* mesh = (CartesianMesh*)sim->getMesh();
	dataSize = mesh->getNumVolumeX() * mesh->getNumVolumeY() * mesh->getNumVolumeZ();
	data = new double[dataSize];
	memset(data, 0, dataSize * sizeof(double));
	
	funcValueSize = (dataSize > mesh->getNumMembraneElements())? dataSize:mesh->getNumMembraneElements();
	functionValues = new double[funcValueSize];
	memset(functionValues, 0, funcValueSize * sizeof(double));

	hdf5Rank = mesh->getDimension();
	hdf5Dims[1] = mesh->getNumVolumeX();
	hdf5Dims[0] = mesh->getNumVolumeY();
	hdf5Dims[2] = mesh->getNumVolumeZ();

	double dx = mesh->getXScale_um();
	double dy = mesh->getYScale_um();
	double dz = mesh->getZScale_um();

	int numSigmas = 8;
	gaussianPsfSampleNx = int(sigmaXY * numSigmas / dx);
	gaussianPsfSampleNy = mesh->getDimension() > 1 ? int(sigmaXY * numSigmas / dy) : 1;
	gaussianPsfSampleNz = mesh->getDimension() > 2 ? int(sigmaZ * numSigmas / dz) : 1;
	if (gaussianPsfSampleNx % 2 == 0) {
		++gaussianPsfSampleNx;
	}
	if (gaussianPsfSampleNy % 2 == 0) {
		++gaussianPsfSampleNy;
	}
	if (gaussianPsfSampleNz % 2 == 0) {
		++gaussianPsfSampleNz;
	}
	int gaussianPsfSampleNxy = gaussianPsfSampleNx * gaussianPsfSampleNy;
	gaussianPsfSamples = new double[gaussianPsfSampleNx * gaussianPsfSampleNy * gaussianPsfSampleNz];
	int midI = gaussianPsfSampleNx / 2;
	int midJ = gaussianPsfSampleNy / 2;
	int midK = gaussianPsfSampleNz / 2;
	double sum = 0;
	int volIndex = -1;
	for (int k = 0; k < gaussianPsfSampleNz; ++k) {
		double z = (k - midK) * dz;
		for (int j = 0; j < gaussianPsfSampleNy; ++j) {
			double y = (j - midJ) * dy;
			for (int i = 0; i < gaussianPsfSampleNx; ++i) {				
				double x = (i - midI) * dx;

				++ volIndex;
				double r2 = x*x + y*y + z*z/(sigmaRatio*sigmaRatio);
				double d = exp(-r2/(2*sigmaXY*sigmaXY));
				gaussianPsfSamples[volIndex] = d;
				sum += d;
			}
		}
	}
	for (int i = 0; i < gaussianPsfSampleNxy * gaussianPsfSampleNz; ++ i) {
		gaussianPsfSamples[i] /= sum;
	}
}

void GaussianConvolutionDataGenerator::computePPData(SimulationExpression* sim) {
	CartesianMesh* mesh = (CartesianMesh*)sim->getMesh();
	VolumeElement* volumeElements = mesh->getVolumeElements();
	int* indices = sim->getIndices();

	int numX = mesh->getNumVolumeX();
	int numY = mesh->getNumVolumeY();
	int numXY = numX * numY;
	int numZ = mesh->getNumVolumeZ();
	int numXYZ = numX*numY*numZ;
//	memset(data, 0, numXYZ * sizeof(double));

	// convolve volume function
	if(volFunction != 0){
		memset(functionValues, 0, funcValueSize * sizeof(double));
		// precompute function values
		indices[VAR_MEMBRANE_INDEX] = -1;
		indices[VAR_MEMBRANE_REGION_INDEX] = -1;
		int volIndex = -1;
		for (int volIndex = 0; volIndex < numXYZ; volIndex ++) {
			WorldCoord wc = mesh->getVolumeWorldCoord(volIndex);
			sim->setCurrentCoordinate(wc);

			indices[VAR_VOLUME_INDEX] = volIndex;
			indices[VAR_VOLUME_REGION_INDEX] = volumeElements[volIndex].getRegionIndex();
			double value = volFunction->evaluateProxy();								
			functionValues[volIndex] = value;
		}

		int midI = gaussianPsfSampleNx / 2;
		int midJ = gaussianPsfSampleNy / 2;
		int midK = gaussianPsfSampleNz / 2;
		int gaussianPsfSampleNxy = gaussianPsfSampleNx * gaussianPsfSampleNy;
		volIndex = -1;
		for (int k = 0; k < numZ; ++k) {
			for (int j = 0; j < numY; ++j) {
				for (int i = 0; i < numX; ++i) {
					++ volIndex;

					data[volIndex] = 0;
					int kkOffset = 0;
					for (int kk = 0; kk < gaussianPsfSampleNz; kk ++, kkOffset += gaussianPsfSampleNxy) {
						int k2 = k - midK + kk;
						if (k2 < 0 || k2 >= numZ) {
							continue;
						}
						int jjOffset = kkOffset;
						for (int jj = 0; jj < gaussianPsfSampleNy; jj ++, jjOffset += gaussianPsfSampleNx) {
							int j2 = j - midJ + jj;
							if (j2 < 0 || j2 >= numY) {
								continue;
							}
							int iiOffset = jjOffset;
							for (int ii = 0; ii < gaussianPsfSampleNx; ii ++, ++iiOffset) {
								int i2 = i - midI + ii;
								if (i2 < 0 || i2 >= numX) {
									continue;
								}
								int psfIndex = iiOffset; 
								int volIndex2 = k2 * numXY + j2 * numX + i2;
								data[volIndex] += functionValues[volIndex2] * gaussianPsfSamples[psfIndex];
							
							} // end ii
						} // end jj
					} // end kk

				} // end i
			} // end j
		} // end k
	}//end of convolve volume function
	if(memFunction != 0){
		memset(functionValues, 0, funcValueSize * sizeof(double));
		indices[VAR_VOLUME_INDEX] = -1;
		indices[VAR_VOLUME_REGION_INDEX] = -1;	
		
		// precompute function values
		for (int m = 0; m < mesh->getNumMembraneElements(); m++) {
			MembraneElement* memElement = mesh->getMembraneElements()+m;
			WorldCoord wc = mesh->getMembraneWorldCoord(memElement);
			sim->setCurrentCoordinate(wc);

			indices[VAR_MEMBRANE_INDEX] = memElement->index;
			indices[VAR_MEMBRANE_REGION_INDEX] = memElement->getRegionIndex();
			double value = memFunction->evaluateProxy();	
			functionValues[m] = value;
		}
		
		int psfZOffset = -gaussianPsfSampleNz/2;
		int psfYOffset = -gaussianPsfSampleNy/2;
		int psfXOffset = -gaussianPsfSampleNx/2;
		for (int m = 0; m < mesh->getNumMembraneElements(); m++) {
			int insideVolIndex = mesh->getMembraneElements()[m].vindexFeatureLo;
			int outsideVolIndex = mesh->getMembraneElements()[m].vindexFeatureHi;
			MeshCoord insideMC = mesh->getMeshCoord(insideVolIndex);
			MeshCoord outsideMC = mesh->getMeshCoord(outsideVolIndex);
			double fullArea = mesh->getXArea_squm();
			int diffVolIndex = abs(outsideVolIndex - insideVolIndex);
			if (diffVolIndex  == numX) {
				fullArea = mesh->getYArea_squm();
			} else if (diffVolIndex == numXY) {
				fullArea = mesh->getZArea_squm();
			}

			double memareaRatio = mesh->getMembraneElements()[m].area/fullArea;
			int psfindex = 0;
			for (int zz = 0; zz < gaussianPsfSampleNz; zz ++) {								
				for (int yy = 0; yy < gaussianPsfSampleNy; yy ++) {									
					for (int xx = 0; xx < gaussianPsfSampleNx; xx ++) {
						double psf_val = gaussianPsfSamples[psfindex ++];

						// inside
						int volIndex2X = insideMC.x + psfXOffset + xx;
						int volIndex2Y = insideMC.y + psfYOffset + yy;
						int volIndex2Z = insideMC.z + psfZOffset + zz;
						if (volIndex2X >= 0 && volIndex2Y >= 0 && volIndex2Z >= 0 && volIndex2X < numX && volIndex2Y < numY && volIndex2Z < numZ) {
							int volIndex2 = volIndex2Z * numXY + volIndex2Y * numX + volIndex2X;
							data[volIndex2] += functionValues[m] * psf_val/2 * memareaRatio;
						}

						// outside
						volIndex2X = outsideMC.x + psfXOffset + xx;
						volIndex2Y = outsideMC.y + psfYOffset + yy;
						volIndex2Z = outsideMC.z + psfZOffset + zz;
						if (volIndex2X >= 0 && volIndex2Y >= 0 && volIndex2Z >= 0 && volIndex2X < numX && volIndex2Y < numY && volIndex2Z < numZ) {
							int volIndex2 = volIndex2Z * numXY + volIndex2Y * numX + volIndex2X;
							data[volIndex2] += functionValues[m] * psf_val/2 * memareaRatio;
						}
					}						
				}
			}
		}
	}
}
