/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <stdlib.h>
#include <string.h>
#include <VCELL/FVDataSet.h>
#include <VCELL/SimTypes.h>
#include <VCELL/Variable.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/Mesh.h>
#include <VCELL/Element.h>
#include <VCELL/Region.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/FieldData.h>
#include <VCELL/MembraneRegion.h>
#include <VCELL/RandomVariable.h>
#include <VCELL/RegionSizeVariable.h>
#include <VCELL/PostProcessingBlock.h>
#include <VCELL/DataGenerator.h>
#include <VCELL/VolumeParticleVariable.h>
#include <VCELL/MembraneParticleVariable.h>
using std::endl;

#define CONVOLVE_SUFFIX "_Convolved"

FieldData* getPSFFieldData(const SimulationExpression* sim);

void FVDataSet::readRandomVariables(char *filename, SimulationExpression *sim)
{
	FILE *fp=NULL;
	FileHeader fileHeader;
	DataBlock *dataBlock;

	if ((fp=fopen(filename, "rb"))==NULL){
		char errmsg[512];
		sprintf(errmsg, "DataSet::read() - could not open file '%s'.", filename); 
		throw errmsg;
	}
	DataSet::readHeader(fp,&fileHeader);

	if (strcmp(fileHeader.magicString, MAGIC_STRING)){
		throw "DataSet::read() - file is not a VCellDump file";
	}

	if (fileHeader.numBlocks <= 0){
		char errmsg[512];
		sprintf(errmsg, "DataSet::read() - number of blocks ( %d ) less than 1.", fileHeader.numBlocks); 
		throw errmsg;
	}
	   
	dataBlock = new DataBlock[fileHeader.numBlocks];
	   
	if (fseek(fp, fileHeader.firstBlockOffset, SEEK_SET)){
		char errmsg[512];
		sprintf(errmsg, "DataSet::read() - could not find first block at offset %d.", fileHeader.firstBlockOffset); 
		throw errmsg;
	}
	for (int i=0;i<fileHeader.numBlocks;i++){
		DataSet::readDataBlock(fp,dataBlock+i);
	}

	for (int i=0;i<fileHeader.numBlocks;i++){
		RandomVariable *rv = sim->getRandomVariableFromName(dataBlock[i].varName);
		if (rv==NULL){
			cout << "DataSet::read() - variable '" << dataBlock[i].varName << "' not found in Simulation" << endl;
			continue;
		}
		if (rv->getSize()!=dataBlock[i].size){
			char errmsg[512];
			sprintf(errmsg, "DataSet::read() - size mismatch for var '%s', file=%d, var=%d.", dataBlock[i].varName, dataBlock[i].size, rv->getSize()); 
			throw errmsg;
		}
	      
		if (fseek(fp, dataBlock[i].dataOffset, SEEK_SET)){
			char errmsg[512];
			sprintf(errmsg, "DataSet::read() - could not find data offset ( %d ).", dataBlock[i].dataOffset); 
			throw errmsg;
		}

		DataSet::readDoubles(fp, rv->getRandomNumbers(), rv->getSize());
		cout << "read data for random variable '" << rv->getName() << "'" << endl;
	}
	delete[] dataBlock;
	   
	fclose(fp);
}

/**
  * the variabe name in the data set can be Cell::Dex,
  * we need to extract Dex
**/
static string extractVarNameFromQualifiedName(char* varName) {
	string str(varName);
	string::size_type pos = str.find("::");
	if (pos != string::npos) {
		str = str.substr(pos + 2);
	}
	return str;
}

void FVDataSet::read(const char *filename, Simulation *sim)
{
	FILE *fp=NULL;
	FileHeader fileHeader;
	DataBlock *dataBlock;

	if ((fp=fopen(filename, "rb"))==NULL){
		char errmsg[512];
		sprintf(errmsg, "DataSet::read() - could not open file '%s'.", filename); 
		throw errmsg;
	}
	DataSet::readHeader(fp,&fileHeader);

	if (strcmp(fileHeader.magicString, MAGIC_STRING)){
		throw "DataSet::read() - file is not a VCellDump file";
	}

	if (fileHeader.numBlocks <= 0){
		char errmsg[512];
		sprintf(errmsg, "DataSet::read() - number of blocks ( %d ) less than 1.", fileHeader.numBlocks); 
		throw errmsg;
	}
	   
	dataBlock = new DataBlock[fileHeader.numBlocks];
	   
	if (fseek(fp, fileHeader.firstBlockOffset, SEEK_SET)){
		char errmsg[512];
		sprintf(errmsg, "DataSet::read() - could not find first block at offset %d.", fileHeader.firstBlockOffset); 
		throw errmsg;
	}
	for (int i=0;i<fileHeader.numBlocks;i++){
		DataSet::readDataBlock(fp,dataBlock+i);
	}

	for (int i=0;i<fileHeader.numBlocks;i++){
		string varName = extractVarNameFromQualifiedName(dataBlock[i].varName);
		Variable *var = sim->getVariableFromName(varName);
		if (var==NULL){
			cout << "DataSet::read() - variable '" << dataBlock[i].varName << "' not found in Simulation" << endl;
			continue;
		}
		if (var->getSize()!=dataBlock[i].size){
			char errmsg[512];
			sprintf(errmsg, "DataSet::read() - size mismatch for var '%s', file=%d, var=%ld.", dataBlock[i].varName, dataBlock[i].size, var->getSize());
			throw errmsg;
		}
	      
		if (fseek(fp, dataBlock[i].dataOffset, SEEK_SET)){
			char errmsg[512];
			sprintf(errmsg, "DataSet::read() - could not find data offset ( %d ).", dataBlock[i].dataOffset); 
			throw errmsg;
		}
		DataSet::readDoubles(fp, var->getCurr(), var->getSize());
		var->update();   
		cout << "read data for variable '" << var->getName() << "'" << endl;
	}
	delete[] dataBlock;
	   
	fclose(fp);
}

void FVDataSet::convolve(SimulationExpression* sim, Variable* var, double* values) {

	FieldData* psfFieldData = getPSFFieldData(sim);
	if (psfFieldData == 0) {
		throw "psf field data is not defined";
	}

	CartesianMesh* mesh = (CartesianMesh *)sim->getMesh(); 

	int meshX = mesh->getNumVolumeX();
	int meshY = mesh->getNumVolumeY();
	int meshZ = mesh->getNumVolumeZ();
	int meshXY = meshX * meshY;

	int psfX = psfFieldData->getSizeX();
	int psfY = psfFieldData->getSizeY();
	int psfZ = psfFieldData->getSizeZ();
	int psfZOffset = -psfZ/2;
	int psfYOffset = -psfY/2;
	int psfXOffset = -psfX/2;

	double* psfData = psfFieldData->getData();
	memset(values, 0, meshX * meshY * meshZ * sizeof(double));

	if (var->getVarType() == VAR_VOLUME || var->getVarType() == VAR_VOLUME_REGION) {
		for (int z = 0; z < meshZ; z ++) {
			for (int y = 0; y < meshY; y ++) {
				for (int x = 0; x < meshX; x ++) {
					int volIndex = z * meshXY + y * meshX + x;
					int psfindex = 0;
					for (int zz = 0; zz < psfZ; zz ++) {						
						for (int yy = 0; yy < psfY; yy ++) {
							for (int xx = 0; xx < psfX; xx ++) {
								int volIndex2X = x + psfXOffset + xx;
								int volIndex2Y = y + psfYOffset + yy;
								int volIndex2Z = z + psfZOffset + zz;
								double psf_val = psfData[psfindex ++];

								if (volIndex2X >= 0 && volIndex2Y >= 0 
										&& volIndex2Z >= 0 && volIndex2X < meshX && volIndex2Y < meshY && volIndex2Z < meshZ) {
									int volIndex2 = volIndex2Z * meshXY + volIndex2Y * meshX + volIndex2X;	
									if (var->getVarType() == VAR_VOLUME_REGION) {
										values[volIndex] += var->getCurr()[mesh->getVolumeElements()[volIndex2].getRegionIndex()] * psf_val;
									} else {
										values[volIndex] += var->getCurr()[volIndex2] * psf_val;
									}
								}
							}
						}
					}
				}
			}
		}
	} else if (var->getVarType() == VAR_MEMBRANE || var->getVarType() == VAR_MEMBRANE_REGION) {				
		for (int m = 0; m < mesh->getNumMembraneElements(); m++) {
			int insideVolIndex = mesh->getMembraneElements()[m].vindexFeatureLo;
			int outsideVolIndex = mesh->getMembraneElements()[m].vindexFeatureHi;
			MeshCoord insideMC = mesh->getMeshCoord(insideVolIndex);
			MeshCoord outsideMC = mesh->getMeshCoord(outsideVolIndex);
			double fullArea = mesh->getXArea_squm();
			int diffVolIndex = abs(outsideVolIndex - insideVolIndex);
			if (diffVolIndex  == meshX) {
				fullArea = mesh->getYArea_squm();
			} else if (diffVolIndex == meshXY) {
				fullArea = mesh->getZArea_squm();
			}

			double memareaRatio = mesh->getMembraneElements()[m].area/fullArea;
			int psfindex = 0;
			for (int zz = 0; zz < psfZ; zz ++) {								
				for (int yy = 0; yy < psfY; yy ++) {									
					for (int xx = 0; xx < psfX; xx ++) {
						double psf_val = psfData[psfindex ++];

						// inside
						int volIndex2X = insideMC.x + psfXOffset + xx;
						int volIndex2Y = insideMC.y + psfYOffset + yy;
						int volIndex2Z = insideMC.z + psfZOffset + zz;
						if (volIndex2X >= 0 && volIndex2Y >= 0 && volIndex2Z >= 0 && volIndex2X < meshX && volIndex2Y < meshY && volIndex2Z < meshZ) {
							int volIndex2 = volIndex2Z * meshXY + volIndex2Y * meshX + volIndex2X;
							if (var->getVarType() == VAR_MEMBRANE_REGION) {
								values[volIndex2] += var->getCurr()[mesh->getMembraneElements()[m].getRegionIndex()] * psf_val/2 * memareaRatio;
							} else {
								values[volIndex2] += var->getCurr()[m] * psf_val/2 * memareaRatio;
							}
						}

						// outside
						volIndex2X = outsideMC.x + psfXOffset + xx;
						volIndex2Y = outsideMC.y + psfYOffset + yy;
						volIndex2Z = outsideMC.z + psfZOffset + zz;
						if (volIndex2X >= 0 && volIndex2Y >= 0 && volIndex2Z >= 0 && volIndex2X < meshX && volIndex2Y < meshY && volIndex2Z < meshZ) {
							int volIndex2 = volIndex2Z * meshXY + volIndex2Y * meshX + volIndex2X;
							if (var->getVarType() == VAR_MEMBRANE_REGION) {
								values[volIndex2] += var->getCurr()[mesh->getMembraneElements()[m].getRegionIndex()] * psf_val/2 * memareaRatio;
							} else {
								values[volIndex2] += var->getCurr()[m] * psf_val/2 * memareaRatio;
							}
						}
					}						
				}
			}
		}
	}			
}

void FVDataSet::write(const char *filename, SimulationExpression *sim, bool bCompress)
{
	FILE *fp=NULL;
	FileHeader fileHeader;
	DataBlock *dataBlock;
	static double *writeBuffer = NULL;

	if (writeBuffer==NULL){
		writeBuffer = new double[sim->getMesh()->getNumVolumeElements()];
	}

	if ((fp=fopen(filename, "wb"))==NULL){
		char errmsg[512];
		sprintf(errmsg, "DataSet::write() - could not open file '%s'.", filename); 
		throw errmsg;
	}

	rewind(fp);

	strcpy(fileHeader.magicString, MAGIC_STRING);
	strcpy(fileHeader.versionString, VERSION_STRING);
	int numVars = sim->getNumVariables();
	if (numVars <= 0){
		cout << "DataSet::write() - no variables defined" << endl;
	}
   
	FieldData* psfFieldData = getPSFFieldData(sim);
	int numBlocks = psfFieldData == 0 ? numVars : numVars*2;

	// region size variable
	int numRegionSizeVars = sim->getNumRegionSizeVariables();
	numBlocks += numRegionSizeVars;

	// random variables
	int numRandVars = sim->getNumRandomVariables();
	numBlocks += numRandVars;

	fileHeader.sizeX = ((CartesianMesh *)sim->getMesh())->getNumVolumeX();
	fileHeader.sizeY = ((CartesianMesh *)sim->getMesh())->getNumVolumeY();
	fileHeader.sizeZ = ((CartesianMesh *)sim->getMesh())->getNumVolumeZ();
	int volVarSize = fileHeader.sizeX * fileHeader.sizeY * fileHeader.sizeZ;
	fileHeader.numBlocks = numBlocks;
	fileHeader.firstBlockOffset = sizeof(FileHeader);

	//
	// write file header
	//   
	DataSet::writeHeader(fp, &fileHeader);
	long ftell_pos = ftell(fp);
	if (ftell_pos != fileHeader.firstBlockOffset){
		char errmsg[512];
		sprintf(errmsg, "DataSet::write() - file offset for first block is incorrect, ftell() says %ld, should be %d", ftell_pos, fileHeader.firstBlockOffset);
		throw errmsg;
	}
   
	dataBlock = new DataBlock[numBlocks];
	   
	//
	// write data blocks (describing data)
	//   
	int blockIndex = 0;
	int32 dataOffset = fileHeader.firstBlockOffset + numBlocks * sizeof(DataBlock);
	for (int i = 0; i < numVars; i ++) {
		Variable* var = sim->getVariable(i);
		memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
		strcpy(dataBlock[blockIndex].varName, var->getQualifiedName().c_str());
		
		dataBlock[blockIndex].varType = var->getVarType();
		dataBlock[blockIndex].size = var->getSize();
		dataBlock[blockIndex].dataOffset = dataOffset;
		DataSet::writeDataBlock(fp,dataBlock+blockIndex);
		dataOffset += dataBlock[blockIndex].size*sizeof(double);
		blockIndex ++;
	}

	// write data blocks for _Convolved variables
	if (psfFieldData != 0) {
		for (int i = 0; i < numVars; i ++) {
			Variable* var = sim->getVariable(i);			
			string varz_name = var->getQualifiedName() + CONVOLVE_SUFFIX;
			memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
			strcpy(dataBlock[blockIndex].varName, varz_name.c_str());
	       
			dataBlock[blockIndex].varType = VAR_VOLUME;
			dataBlock[blockIndex].size = volVarSize;
			dataBlock[blockIndex].dataOffset = dataOffset;
			DataSet::writeDataBlock(fp,dataBlock + blockIndex);
			dataOffset += dataBlock[blockIndex].size*sizeof(double);
			blockIndex ++;
		}
	}

	// region size variable
	for (int i = 0; i < numRegionSizeVars; i ++) {
		RegionSizeVariable* rsv = sim->getRegionSizeVariable(i);			
		memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
		strcpy(dataBlock[blockIndex].varName, rsv->getQualifiedName().c_str());
	
		dataBlock[blockIndex].varType = rsv->getVarType();
		dataBlock[blockIndex].size = rsv->getSize();
		dataBlock[blockIndex].dataOffset = dataOffset;
		DataSet::writeDataBlock(fp,dataBlock + blockIndex);
		dataOffset += dataBlock[blockIndex].size*sizeof(double);
		blockIndex ++;
	}

	
	for (int i = 0; i < numRandVars; i ++) {
		RandomVariable* rv = sim->getRandomVariable(i);
		memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
		strcpy(dataBlock[blockIndex].varName, rv->getName().c_str());
	
		dataBlock[blockIndex].varType = rv->getVariableType();
		dataBlock[blockIndex].size = rv->getSize();
		dataBlock[blockIndex].dataOffset = dataOffset;
		DataSet::writeDataBlock(fp,dataBlock + blockIndex);
		dataOffset += dataBlock[blockIndex].size*sizeof(double);
		blockIndex ++;
	}

	//
	// write data
	//
	blockIndex = 0;
	for (int i = 0; i < numVars; i ++) {
		Variable* var = sim->getVariable(i);
		if (!var){
			char errmsg[512];
			sprintf(errmsg, "DataSet::write() - variable '%s' not found during write", dataBlock[blockIndex].varName);
			throw errmsg;
		}
		ftell_pos = ftell(fp);
		if (ftell_pos != dataBlock[blockIndex].dataOffset){
			char errmsg[512];
			sprintf(errmsg, "DataSet::write() - offset for data is incorrect (block %d, var=%s), ftell() says %ld, should be %d", blockIndex, dataBlock[blockIndex].varName, ftell_pos, dataBlock[blockIndex].dataOffset);
			throw errmsg;
		}

		if (var->getSize() != dataBlock[blockIndex].size) {
			throw "DataSet::write() : inconsistent number of data blocks for variable";
		}

		double* dataVal = 0;
		if(var->getVarType() == VAR_VOLUME_PARTICLE){
			VolumeParticleVariable* volParticleVar = (VolumeParticleVariable*)var;
			dataVal = volParticleVar->getMoleculeCounts();
		}else if(var->getVarType() == VAR_MEMBRANE_PARTICLE){
			MembraneParticleVariable* memParticleVar = (MembraneParticleVariable*)var;
			dataVal = memParticleVar->getMoleculeCounts();
		}else{
			dataVal = var->getCurr();
		}
		
		DataSet::writeDoubles(fp, dataVal, var->getSize());
		blockIndex ++;
	}
	
	//
	// write data for _Convolved variables
	//
	if (psfFieldData != 0) {
		double* values = new double[volVarSize];

		for (int i = 0; i < numVars; i ++) {
			Variable* var = sim->getVariable(i);
			convolve(sim, var, values);

			ftell_pos = ftell(fp);
			if (ftell_pos != dataBlock[blockIndex].dataOffset){
				char errmsg[512];
				sprintf(errmsg, "DataSet::write() - offset for data is "
					"incorrect (block %d, var=%s), ftell() says %ld, should be %d",
					blockIndex, dataBlock[blockIndex].varName, ftell_pos, dataBlock[blockIndex].dataOffset);
				throw errmsg;
			}
			if (volVarSize != dataBlock[blockIndex].size) {
				throw "DataSet::write() : inconsistent number of data blocks for variable";
			}

			DataSet::writeDoubles(fp, values, volVarSize);
			blockIndex ++;
		}	
	}

	//
	// write data for region size variables
	//	
	for (int i = 0; i < numRegionSizeVars; i ++) {
		RegionSizeVariable* rsv = sim->getRegionSizeVariable(i);
		ftell_pos = ftell(fp);
		if (ftell_pos != dataBlock[blockIndex].dataOffset){
			char errmsg[512];
			sprintf(errmsg, "DataSet::write() - offset for data is "
				"incorrect (block %d, var=%s), ftell() says %ld, should be %d",
				blockIndex, dataBlock[blockIndex].varName, ftell_pos, dataBlock[blockIndex].dataOffset);
			throw errmsg;
		}

		if (rsv->getSize() != dataBlock[blockIndex].size) {
			throw "DataSet::write() : inconsistent number of data blocks for variable";
		}
		DataSet::writeDoubles(fp, rsv->getCurr(), rsv->getSize());
		blockIndex ++;
	}

	//
	// write data for random variables
	//	
	for (int i = 0; i < numRandVars; i ++) {
		RandomVariable* rv = sim->getRandomVariable(i);						
		ftell_pos = ftell(fp);
		if (ftell_pos != dataBlock[blockIndex].dataOffset){
			char errmsg[512];
			sprintf(errmsg, "DataSet::write() - offset for data is "
				"incorrect (block %d, var=%s), ftell() says %ld, should be %d",
				blockIndex, dataBlock[blockIndex].varName, ftell_pos, dataBlock[blockIndex].dataOffset);
			throw errmsg;
		}

		if (rv->getSize() != dataBlock[blockIndex].size) {
			throw "DataSet::write() : inconsistent number of data blocks for variable";
		}
		DataSet::writeDoubles(fp, rv->getRandomNumbers(), rv->getSize());
		blockIndex ++;
	}	

	fclose(fp);
	if (bCompress){
		char commandBuffer[200];
		sprintf(commandBuffer,"compress %s",filename);
		system(commandBuffer);
	}

	delete[] dataBlock;
}
