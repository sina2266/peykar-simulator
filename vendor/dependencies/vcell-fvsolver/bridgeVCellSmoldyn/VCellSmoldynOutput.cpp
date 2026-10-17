/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include "VCellSmoldynOutput.h"

#include <VCELL/SimulationMessaging.h>
#include <SimCommand.h>
#include <VCELL/SimTool.h>
#include <VCELL/Simulation.h>
#include <VCELL/VolumeParticleVariable.h>
#include <VCELL/MembraneParticleVariable.h>
#include <VCELL/ZipUtils.h>
#include "SmoldynVarStatDataGenerator.h"
#include "vcellhybrid.h"
//#include "SmoldynROIDataGenerator.h"
#include "SmoldynHdf5Writer.h"
#define SIM_FILE_EXT "sim"
#define LOG_FILE_EXT "log"
#define ZIP_FILE_EXT "zip"
#define HDF5_EXT "hdf5"
#define ZIP_FILE_LIMIT 1E9

typedef int int32;
typedef unsigned int uint32;

#include <sys/stat.h>
#include <math.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <sstream>
using namespace std;

static int getZipCount(char* zipFileName) {
	char* p = strstr(zipFileName, ZIP_FILE_EXT);
	if (p == NULL) {
		return -1;
	}

	char str[3];
	strncpy(str, p - 2, 2 * sizeof(char));
	str[2] = 0;
	return atoi(str);
}

VCellSmoldynOutput::VCellSmoldynOutput(simptr sim)
:smoldynSim(sim),
 simFileCount(0),
zipFileCount(0),
	baseFileName(),
 	baseSimName( ),
	 Nx(0),
	 Ny(0),
	 Nz(0),
	numVolumeElements(0),
	numMembraneElements(0),
	dimension(0),
	extent( ),
	origin(),
 	volVariables( ),
	 memVariables( ),
	fileHeader( ),
	dataBlock(0),
	volVarOutputData(0),
 	memVarOutputData(0),
	molIdentVarIndexMap(0),
	variables(0),
     hdf5DataWriter(0),
	dataGeneratorList( ) ,
	simTool(0) {}

VCellSmoldynOutput::~VCellSmoldynOutput() {
    for (int i = 0; i < volVariables.size(); i ++) {
        delete[] volVarOutputData[i];
    }
    for (int i = 0; i < memVariables.size(); i ++) {
        delete[] memVarOutputData[i];
    }
    delete[] volVarOutputData;
    delete[] memVarOutputData;
    delete[] molIdentVarIndexMap;
    delete[] dataBlock;

    delete hdf5DataWriter;
    for (int i = 0; i < smoldynSim->mols->nspecies - 1; i ++) {
        delete variables[i];
    }
    delete[] variables;
}

VCellSmoldynOutput* VCellSmoldynOutput::updateVCellSmoldynOutput(VCellSmoldynOutput* output, simptr sim) {
	if (output == NULL) return new VCellSmoldynOutput(sim);
	if (output->smoldynSim != sim) output->smoldynSim = sim;
	return output;
}

void VCellSmoldynOutput::parseDataProcessingInput(string& name, string& input) {
	//always add variable statistics data generator
	SmoldynDataGenerator* dataGenerator = new SmoldynVarStatDataGenerator();
	dataGeneratorList.push_back(dataGenerator);
	if (name == "RoiTimeSeries") {
		//SmoldynDataGenerator* roiDataProcessor = new SmoldynROIDataGenerator();
	} else {
		throw "unknown DataProcessor";
	}
}

const char * const  VCellSmoldynKeyword_dimension ="dimension";
const char * const  VCellSmoldynKeyword_sampleSize ="sampleSize";
const char * const  VCellSmoldynKeyword_numMembraneElements ="numMembraneElements";
const char * const  VCellSmoldynKeyword_variable = "variable";
const char * const  VCellSmoldynKeyword_membrane = "membrane";
const char * const  VCellSmoldynKeyword_volume = "volume";

void VCellSmoldynOutput::parseInput(string& input) {
	if (dimension > 0) {
		return;
	}

	cmdssptr cmds = ((cmdssptr)smoldynSim->cmds);
	char* rootdir = cmds->root;
	char* fname = cmds->fname[0];
	strcpy(baseFileName, rootdir);
	strcat(baseFileName, fname);
	char* p = strrchr(baseFileName, '.');
	*p = '\0';

	strcpy(baseSimName, fname);
	p = strrchr(baseSimName, '.');
	*p = '\0';

	clearLog();

	if (input.size() == 0) {
		throw "writeOutput : no dimension specified.";
	}
	molssptr mols = smoldynSim->mols;
	int numVars = mols->nspecies - 1;
	variables = new SmoldynVariable*[numVars];

	stringstream inputSS(input);
	string line, token;
	int varCount = 0;
	while (!inputSS.eof()) {
		getline(inputSS, line);

		stringstream liness(line);
		token = "";
		liness >> token;
		if (token == "") {
			continue;
		}
		if (token == VCellSmoldynKeyword_dimension) {
			liness >> dimension;
		} else if (token == VCellSmoldynKeyword_sampleSize) {
			liness >> Nx;
			Ny = Nz = 1;
			if (dimension > 1) {
				liness >> Ny;
				if (dimension > 2) {
					liness >> Nz;
				}
			}
			numVolumeElements = Nx * Ny * Nz;
		} else if (token == VCellSmoldynKeyword_numMembraneElements) {
			liness >> numMembraneElements;
		} else if (token == VCellSmoldynKeyword_variable) {
			string type;
			variables[varCount] = new SmoldynVariable;
			liness >> variables[varCount]->name >> type >> variables[varCount]->domain;
			if (type == VCellSmoldynKeyword_membrane) {
				variables[varCount]->type = VAR_MEMBRANE;
			} else if (type == VCellSmoldynKeyword_volume) {
				variables[varCount]->type = VAR_VOLUME;
			}
			if (strcmp(mols->spname[varCount + 1], variables[varCount]->name.c_str())) {
				throw "VCellSmoldynOutput::parseInput(), variables not in the same order";
			}
			const char* vardomain = variables[varCount]->domain.c_str();
			if (variables[varCount]->type == VAR_VOLUME) {
				compartssptr cmptss = smoldynSim->cmptss;
				for(int c=0; c<cmptss->ncmpt; c++) {
					if (!strcmp(cmptss->cnames[c], vardomain)) {
						variables[varCount]->cmpt = cmptss->cmptlist[c];
						break;
					}
				}
				if (variables[varCount]->cmpt == 0) {
					stringstream ss;
					ss << "found no compartment for variable '" << variables[varCount]->name << "'";
					throw ss.str();
				}
			} else if (variables[varCount]->type == VAR_MEMBRANE) {
				surfacessptr srfss = smoldynSim->srfss;
				Hashtable* nameToSurfaceptrMap = smoldynSim->srfss->snametosrf;
				if (!nameToSurfaceptrMap->contains(nameToSurfaceptrMap, vardomain)) {
					stringstream ss;
					ss << "found no surface for variable '" << variables[varCount]->name << "'";
					throw ss.str();
				}
				variables[varCount]->srf = (surfaceptr)nameToSurfaceptrMap->getFrom(nameToSurfaceptrMap, vardomain);
			}
			varCount ++;
		}
	}

	origin[0] = smoldynSim->wlist[0]->pos;
	extent[0] = smoldynSim->wlist[1]->pos - origin[0];
	if (dimension > 1) {
		origin[1] = smoldynSim->wlist[2]->pos;
		extent[1] = smoldynSim->wlist[3]->pos - origin[1];
		if (dimension > 2) {
			origin[2] = smoldynSim->wlist[4]->pos;
			extent[2] = smoldynSim->wlist[5]->pos - origin[2];
		}
	}

	int volCount = 0;
	int memCount = 0;
	molIdentVarIndexMap = new int[numVars];
	for (int i = 0; i < numVars; i ++) {
		char* varName = new char[128];
		int molIdent = i + 1;
		strcpy(varName, mols->spname[molIdent]);
		if(variables[i]->type == VAR_MEMBRANE) { // membrane variable
			memVariables.push_back(variables[i]);
			molIdentVarIndexMap[i] = memCount;
			memCount ++;
		} else {
			volVariables.push_back(variables[i]);
			molIdentVarIndexMap[i] = volCount;
			volCount ++;
		}
	}
	strcpy(fileHeader.magicString, MAGIC_STRING);
	strcpy(fileHeader.versionString, VERSION_STRING);

	fileHeader.sizeX = Nx;
	fileHeader.sizeY = Ny;
	fileHeader.sizeZ = Nz;
	fileHeader.numBlocks = numVars;
	fileHeader.firstBlockOffset = sizeof(FileHeader);

	dataBlock = new DataBlock[numVars];
	//
	// compute data blocks (describing data)
	//
	int dataOffset = fileHeader.firstBlockOffset + numVars * sizeof(DataBlock);
	int blockIndex = 0;
	for (unsigned int v = 0; v < volVariables.size(); v ++) {
		memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
		strcpy(dataBlock[blockIndex].varName, volVariables[v]->getFullyQualifiedName().c_str());

		dataBlock[blockIndex].varType = VAR_VOLUME;
		dataBlock[blockIndex].size = numVolumeElements;
		dataBlock[blockIndex].dataOffset = dataOffset;
		dataOffset += dataBlock[blockIndex].size * sizeof(double);
		blockIndex ++;
	}
	for (unsigned int v = 0; v < memVariables.size(); v ++) {
		memset(dataBlock[blockIndex].varName, 0, DATABLOCK_STRING_SIZE * sizeof(char));
		strcpy(dataBlock[blockIndex].varName, memVariables[v]->getFullyQualifiedName().c_str());

		dataBlock[blockIndex].varType = VAR_MEMBRANE;
		dataBlock[blockIndex].size = numMembraneElements;
		dataBlock[blockIndex].dataOffset = dataOffset;
		dataOffset += dataBlock[blockIndex].size * sizeof(double);
		blockIndex ++;
	}
	volVarOutputData = new double*[volVariables.size()];
	if (vcellhybrid::isHybrid( )) {
		Simulation* const sim = simTool->getSimulation();
		for (int i = 0; i < volVariables.size(); i ++) {
			VolumeParticleVariable* var = (VolumeParticleVariable*)sim->getVariableFromName(volVariables[i]->name);
			//reminder that we ask smoldyn to write to molecule counts and in simtool we convert the counts to counts/mesh element size for FV
			//volVarOutputData[i] = var->getCurr();
			volVarOutputData[i] = var->getMoleculeCounts();
		}
		memVarOutputData = new double*[memVariables.size()];
		for (int i = 0; i < memVariables.size(); i ++) {
			MembraneParticleVariable* var = (MembraneParticleVariable*)sim->getVariableFromName(memVariables[i]->name);
			//reminder that we ask smoldyn to write to molecule counts and in simtool we convert the counts to counts/mesh element size for FV
			//memVarOutputData[i] = var->getCurr();
			memVarOutputData[i] = var->getMoleculeCounts();
		}
	}
	else {

		for (int i = 0; i < volVariables.size(); i ++) {
			volVarOutputData[i] = new double[numVolumeElements];
		}
		memVarOutputData = new double*[memVariables.size()];
		for (int i = 0; i < memVariables.size(); i ++) {
			memVarOutputData[i] = new double[numMembraneElements];
		}
	}
	//initialize hdf5 writer
	if(hdf5DataWriter == 0)
	{
		char hdf5FileName[256];
		sprintf(hdf5FileName, "%s.%s", baseFileName, HDF5_EXT);
		hdf5DataWriter = new SmoldynHdf5Writer(hdf5FileName, this);
	}
	//initialize data generators
	for(int i=0; i < dataGeneratorList.size(); i++)
	{
		dataGeneratorList[i]->initialize(this);
	}
}

void VCellSmoldynOutput::write() {	//for each save time interval
	computeHistogram();
	if (vcellhybrid::isHybrid()) {
		return;
	}

	// write sim file
	char simFileName[256];
	char zipFileName[256];

	struct stat buf;
	static char* tempDir = "/tmp/";
	static bool bUseTempDir = false;
	static bool bFirstTimeWrite = true;

	if (bFirstTimeWrite) {
		if (stat(tempDir, &buf) == 0) {
			// use local temp directory for .sim files
			// to avoid network traffic
			if (buf.st_mode & S_IFDIR) {
				bUseTempDir = true;
			}
		}
		bFirstTimeWrite = false;
	}
	if (bUseTempDir) {
		sprintf(simFileName, "%s%s%.4d.%s", tempDir, baseSimName, simFileCount, SIM_FILE_EXT);
	} else {
		sprintf(simFileName, "%s%.4d.%s", baseFileName, simFileCount, SIM_FILE_EXT);
	}
	sprintf(zipFileName, "%s%.2d.%s", baseFileName, zipFileCount, ZIP_FILE_EXT);

	writeSim(simFileName, zipFileName);

	// write log file
	char logfilename[256];
	sprintf(logfilename, "%s.%s", baseFileName, LOG_FILE_EXT);
	FILE* logfp = NULL;
	if (simFileCount == 0) {
		logfp = fopen(logfilename, "w");
	} else {
		logfp = fopen(logfilename, "a");
	}
	if (logfp == NULL) {
		throw "can't open logfile for write";
	}
	int iteration = (int)(smoldynSim->time/smoldynSim->dt + 0.5);
	char zipFileNameWithoutPath[512];
	sprintf(zipFileNameWithoutPath,"%s%.2d.%s",baseSimName, zipFileCount, ZIP_FILE_EXT);
	char simFileNameWithoutPath[512];
	sprintf(simFileNameWithoutPath,"%s%.4d.%s", baseSimName, simFileCount, SIM_FILE_EXT);
	fprintf(logfp,"%4d %s %s %.15lg\n", iteration, simFileNameWithoutPath, zipFileNameWithoutPath, smoldynSim->time);
	fclose(logfp);

	// print message
	simFileCount ++;

	if (stat(zipFileName, &buf) == 0) { // if exists
		if (buf.st_size > ZIP_FILE_LIMIT) {
			zipFileCount ++;
		}
	}

	if (hdf5DataWriter != 0) {
		hdf5DataWriter->writeOutput();
	}

	double progress = (smoldynSim->time - smoldynSim->tmin) / (smoldynSim->tmax - smoldynSim->tmin);
	SimulationMessaging::getInstVar()->setWorkerEvent(new WorkerEvent(JOB_DATA, progress, smoldynSim->time));
}

void VCellSmoldynOutput::clearLog() {

	char logFileName[256];
	sprintf(logFileName,"%s.%s",baseFileName, LOG_FILE_EXT);
	ifstream logfs(logFileName);
	if (!logfs.is_open()){
		cout << "error opening log file <" << logFileName << ">" << endl;
		return;
	}

	char simFileName[128];
	char zipFileName[128];
	int iteration, oldCount=-1, count;
	double time;

	while (!logfs.eof()) {
		logfs >> iteration >> simFileName >> zipFileName >> time;
		count = getZipCount(zipFileName);
		if (oldCount != count && count >= 0) {
			cout << endl << "clearLog(), removing zip file " << zipFileName << endl;
			remove(zipFileName);
			oldCount = count;
		}
	}
	logfs.close();

	cout << "clearLog(), removing log file " << logFileName << endl;
	remove(logFileName);

	char hdf5FileName[128];
	sprintf(hdf5FileName,"%s.%s",baseFileName, HDF5_EXT);
	remove(hdf5FileName);
}

double VCellSmoldynOutput::distance2(double* pos1, double* pos2) {
	if (dimension == 1) {
		return (pos1[0] - pos2[0]) * (pos1[0] - pos2[0]);
	}
	if (dimension == 2) {
		return (pos1[0] - pos2[0]) * (pos1[0] - pos2[0])
			+ (pos1[1] - pos2[1]) * (pos1[1] - pos2[1]);
	}
	if (dimension == 3) {
		return (pos1[0] - pos2[0]) * (pos1[0] - pos2[0])
			+ (pos1[1] - pos2[1]) * (pos1[1] - pos2[1])
			+ (pos1[2] - pos2[2]) * (pos1[2] - pos2[2]);
	}
}

void VCellSmoldynOutput::computeHistogram() {
	molssptr mols = smoldynSim->mols;

	for (int i = 0; i < volVariables.size(); i ++) {
		memset(volVarOutputData[i], 0, numVolumeElements * sizeof(double));
	}
	for (int i = 0; i < memVariables.size(); i ++) {
		memset(memVarOutputData[i], 0, numMembraneElements * sizeof(double));
	}

	double dx = extent[0]/(Nx-1);
	double dy = (dimension > 1) ? extent[1]/(Ny-1) : 0;
	double dz = (dimension > 2) ? extent[2]/(Nz-1) : 0;
	double center[3];
	for(int ll=0;ll<mols->nlist;ll++) {
		for(int m=0;m<mols->nl[ll];m++) {
			moleculeptr mptr=mols->live[ll][m];
			int molIdent = mptr->ident - 1;
			if (molIdent < 0 ) {
				continue;
			}
			int varIndex = molIdentVarIndexMap[molIdent];
			if (variables[molIdent]->type == VAR_MEMBRANE) {
				char* panelName = mptr->pnl->pname;
				int memIndex = 0;
				char* p = strrchr(panelName, '_');
				sscanf(p+1, "%d", &memIndex);
				memVarOutputData[varIndex][memIndex] ++;
			} else {
				double* coord = mptr->pos;
				int i = 0, j = 0, k = 0;
				i = (int)((coord[0] - origin[0])/dx + 0.5);
				center[0] = i * dx;
				if (dimension > 1) {
					j = (int)((coord[1] - origin[1])/dy + 0.5);
					center[1] = j * dy;
					if (dimension > 2) {
						k = (int)((coord[2] - origin[2])/dz + 0.5);
						center[2] = k * dy;
					}
				}

				int volIndex = k * Nx * Ny + j * Nx + i;
				// not in the same compartment, try to find the nearest neighbor
				// in the same compartment, if not found, keep it in the wrong
				// compartment
				/*if (!isInSameCompartment(coord, center)) {
					bool bFound = false;
					double distance = 1e9;
					if (i > 0) {
						double center0[3] = {center[0]-dx, center[1], center[2]};
						if (isInSameCompartment(coord, center0)) {
							double dl = distance2(center0, coord);
							if (distance > dl) {
								distance = dl;
								volIndex = k * Nx * Ny + j * Nx + (i-1);
							}
						}
					}
					if (i < Nx - 1) {
						double center0[3] = {center[0]+dx, center[1], center[2]};
						if (isInSameCompartment(coord, center0)) {
							double dl = distance2(center0, coord);
							if (distance > dl) {
								distance = dl;
								volIndex = k * Nx * Ny + j * Nx + (i+1);
							}
						}
					}
					if (dimension > 1) {
						if (j > 0) {
							double center0[3] = {center[0], center[1]-dy, center[2]};
							if (isInSameCompartment(coord, center0)) {
								double dl = distance2(center0, coord);
								if (distance > dl) {
									distance = dl;
									volIndex = k * Nx * Ny + (j-1) * Nx + i;
								}
							}
						}
						if (j < Ny - 1) {
							double center0[3] = {center[0], center[1]+dy, center[2]};
							if (isInSameCompartment(coord, center0)) {
								double dl = distance2(center0, coord);
								if (distance > dl) {
									distance = dl;
									volIndex = k * Nx * Ny + (j+1) * Nx + i;
								}
							}
						}
						if (dimension > 2) {
							if (k > 0) {
								double center0[3] = {center[0], center[1], center[2]-dz};
								if (isInSameCompartment(coord, center0)) {
									double dl = distance2(center0, coord);
									if (distance > dl) {
										distance = dl;
										volIndex = (k-1) * Nx * Ny + j * Nx + i;
									}
								}
							}
							if (k < Nz - 1) {
								double center0[3] = {center[0], center[1], center[2]+dz};
								if (isInSameCompartment(coord, center0)) {
									double dl = distance2(center0, coord);
									if (distance > dl) {
										distance = dl;
										volIndex = (k+1) * Nx * Ny + j * Nx + i;
									}
								}
							}
						}
					}

				}*/
				volVarOutputData[varIndex][volIndex] ++;
			}
		}
	}
}

void VCellSmoldynOutput::writeSim(char* simFileName, char* zipFileName) {

	FILE* simfp = fopen(simFileName, "wb");
	if (simfp == NULL){
		throw "Cannot open .sim file to write";
	}

	DataSet::writeHeader(simfp, &fileHeader);
	long ftell_pos = ftell(simfp);
	if (ftell_pos != fileHeader.firstBlockOffset){
		char errMsg[256];
		sprintf(errMsg, "DataSet::write() - file offset for first block is incorrect, ftell() says %ld, should be %d", ftell_pos, fileHeader.firstBlockOffset);
		throw errMsg;
	}

	//
	// write data blocks (describing data)
	//
	int blockIndex = 0;
	for (unsigned int v = 0; v < volVariables.size(); v ++) {
		DataSet::writeDataBlock(simfp, dataBlock + blockIndex);
		blockIndex ++;
	}
	for (unsigned int v = 0; v < memVariables.size(); v ++) {
		DataSet::writeDataBlock(simfp, dataBlock + blockIndex);
		blockIndex ++;
	}

	//
	// write data
	//
	blockIndex = 0;
	int dataOffset = fileHeader.firstBlockOffset + (volVariables.size() + memVariables.size()) * sizeof(DataBlock);
	for (unsigned int v = 0; v < volVariables.size(); v ++) {
		ftell_pos = ftell(simfp);
		if (ftell_pos != dataBlock[blockIndex].dataOffset){
			char errMsg[256];
			sprintf(errMsg, "DataSet::write() - offset for data is "
				"incorrect (block %d, var=%s), ftell() says %ld, should be %d", blockIndex, dataBlock[blockIndex].varName,
				ftell_pos, dataBlock[blockIndex].dataOffset);
			throw errMsg;
		}
		DataSet::writeDoubles(simfp, volVarOutputData[v], numVolumeElements);
		blockIndex ++;
	}
	for (unsigned int v = 0; v < memVariables.size(); v ++) {
		ftell_pos = ftell(simfp);
		if (ftell_pos != dataBlock[blockIndex].dataOffset){
			char errMsg[256];
			sprintf(errMsg, "DataSet::write() - offset for data is "
				"incorrect (block %d, var=%s), ftell() says %ld, should be %d", blockIndex, dataBlock[blockIndex].varName,
				ftell_pos, dataBlock[blockIndex].dataOffset);
			throw errMsg;
		}
		DataSet::writeDoubles(simfp, memVarOutputData[v], numMembraneElements);
		blockIndex ++;
	}
	fclose(simfp);

	addFilesToZip(zipFileName, simFileName);

	remove(simFileName);
//	if (retcode != 0) {
//		char errMsg[256];
//		sprintf(errMsg, "Writing zip file <%s> failed, return code is %d", zipFileName, retcode);
//		throw errMsg;
//	}
}
