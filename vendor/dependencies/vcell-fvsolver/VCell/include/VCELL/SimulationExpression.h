/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SIMULATIONEXPRESSION_H
#define SIMULATIONEXPRESSION_H

#include <VCELL/Simulation.h>
#include <VCELL/SimTypes.h>

enum VAR_INDEX {VAR_VOLUME_INDEX=0, VAR_MEMBRANE_INDEX, VAR_CONTOUR_INDEX, VAR_VOLUME_REGION_INDEX, VAR_MEMBRANE_REGION_INDEX,
    VAR_CONTOUR_REGION_INDEX};

#define NUM_VAR_INDEX (VAR_CONTOUR_REGION_INDEX + 1)

class FieldData;
class MembraneVariable;
class MembraneRegionVariable;
class VolumeRegionVariable;
class RandomVariable;
class RegionSizeVariable;
class SymbolTable;
class ScalarValueProxy;
class VolumeParticleVariable;
class MembraneParticleVariable;

class SimulationExpression : public Simulation
{
public:
	explicit SimulationExpression(CartesianMesh *mesh);
	~SimulationExpression() override;

	void initSimulation(SimTool* sim_tool) override;
	void resolveReferences(SimTool* sim_tool) override; // create symbol table
	void update(SimTool* sim_tool);           // copies new to old values
	void advanceTimeOn(SimTool* sim_tool);
	void advanceTimeOff(SimTool* sim_tool);

	void writeData(const char *filename, bool bCompress) override;

	void addFieldData(FieldData* fd) {
		fieldDataList.push_back(fd);
	}
	int getNumFields() const { return static_cast<int>(fieldDataList.size()); }
	string* getFieldSymbols();
	void populateFieldValues(double* darray, int index);

	void addRandomVariable(RandomVariable* rv) {
		randomVarList.push_back(rv);
	}
	RandomVariable* getRandomVariableFromName(char* rvname);
	int getNumRandomVariables() const
	{
		return static_cast<int>(randomVarList.size());
	}
	RandomVariable* getRandomVariable(int index) const
	{
		return randomVarList[index];
	}
	void populateRandomValues(double* darray, int index);

	int* getIndices() const { return indices; };
	SymbolTable* getSymbolTable() const
	{
		return symbolTable; 
	};
	void setCurrentCoordinate(WorldCoord& wc);

	bool isVolumeVariableDefinedInRegion(int volVarIndex, int regionIndex) const
	{
		if (volVariableRegionMap[volVarIndex] == nullptr) {
			return true;
		}
		return volVariableRegionMap[volVarIndex][regionIndex];
	}

	void addParameter(string& param);
	void setParameterValues(SimTool* sim_tool, double* paramValues);
	int getNumParameters() const
	{
		return static_cast<int>(paramList.size());
	}
	void populateParameterValues(double* darray);

	// right now bSolveRegion is only applicable for volume variables
	void addVariable(Variable *var, bool* bSolveRegions=nullptr);
	void addVolumeVariable(VolumeVariable *var, bool* bSolveRegions);
	void addVolumeParticleVariable(VolumeParticleVariable *var);
	void addMembraneVariable(MembraneVariable *var);
	void addMembraneParticleVariable(MembraneParticleVariable *var);
	void addVolumeRegionVariable(VolumeRegionVariable *var);
	void addMembraneRegionVariable(MembraneRegionVariable *var);

	double* getDiscontinuityTimes() const { return discontinuityTimes; }
	void setDiscontinuityTimes(double* stopTimes) { discontinuityTimes=stopTimes; }

	int getNumVolVariables() const { return volVarSize; }
	VolumeVariable* getVolVariable(int i) const { return volVarList[i]; }

	int getNumMemVariables() const { return memVarSize; }
	MembraneVariable* getMemVariable(int i) const { return memVarList[i]; }

	int getNumVolRegionVariables() const { return volRegionVarSize; }
	VolumeRegionVariable* getVolRegionVariable(int i) const { return volRegionVarList[i]; }

	int getNumMemRegionVariables() const { return memRegionVarSize; }
	MembraneRegionVariable* getMemRegionVariable(int i) const { return memRegionVarList[i]; }

	int getNumVolParticleVariables() const
	{
		return volParticleVarSize;
	}
	int getNumMemParticleVariables() const
	{
		return memParticleVarSize;
	}

	int getNumMemPde() const { return numMemPde; }
	int getNumVolPde() const { return numVolPde; }

	void setHasTimeDependentDiffusionAdvection() { bHasTimeDependentDiffusionAdvection = true; }
	bool hasTimeDependentDiffusionAdvection() const { return bHasTimeDependentDiffusionAdvection; }

	void setPSFFieldDataIndex(int idx) {
		psfFieldDataIndex = idx;
	}

	FieldData* getPSFFieldData() const
	{
		if (psfFieldDataIndex >= 0) {
			return fieldDataList[psfFieldDataIndex];
		}
		return nullptr;
	}
	bool isParameter(string& symbol); // can be serial scan parameter or opt parameter
	bool isVariable(string& symbol);

	int getNumRegionSizeVariables() const
	{
		return numRegionSizeVars;
	}
	RegionSizeVariable* getRegionSizeVariable(int index) const
	{
		return regionSizeVarList[index];
	}
	void populateRegionSizeVariableValues(double* darray, bool bVolumeRegion, int regionIndex);

	void createPostProcessingBlock() override;

	void populateFieldValuesNew(double* darray, int index);
	void populateRegionSizeVariableValuesNew(double* darray, bool bVolumeRegion, int regionIndex);
	void populateParameterValuesNew(double* darray);
	void populateRandomValuesNew(double* darray, int index);
	void populateParticleVariableValuesNew(double* array, bool bVolume, int index);

	int symbolIndexOfT() const { return symbolIndexOffset_T; }
	int symbolIndexOfXyz() const { return symbolIndexOffset_Xyz; }
	int symbolIndexOfVolVar() const { return symbolIndexOffset_VolVar; }
	int symbolIndexOfMemVar() const { return symbolIndexOffset_MemVar; }
	int symbolIndexOfVolRegionVar() const { return symbolIndexOffset_VolRegionVar; }
	int symbolIndexOfMemRegionVar() const { return symbolIndexOffset_MemRegionVar; }
	int symbolIndexOfVolParticleVar() const { return symbolIndexOffset_VolParticleVar; }
	int symbolIndexOfMemParticleVar() const { return symbolIndexOffset_MemParticleVar; }
	int symbolIndexOfRegionSizeVariable() const { return symbolIndexOffset_RegionSizeVariable; }
	int symbolIndexOfFieldData() const { return symbolIndexOffset_FieldData; }
	int symbolIndexOfRandomVar() const { return symbolIndexOffset_RandomVar; }
	int symbolIndexOfParameters() const { return symbolIndexOffset_Parameters; }
	int numOfSymbols() const { return numSymbols;}

private:
	SymbolTable* symbolTable;

	int* indices;
	void createSymbolTable(SimTool* sim_tool);

	ScalarValueProxy* valueProxyTime;
	ScalarValueProxy* valueProxyX;
	ScalarValueProxy* valueProxyY;
	ScalarValueProxy* valueProxyZ;
	vector<FieldData*> fieldDataList;

	vector<string> paramList;
	vector<ScalarValueProxy*> paramValueProxies;

	vector<bool*> volVariableRegionMap;
	double* discontinuityTimes;

	VolumeVariable** volVarList;
	int volVarSize;
	MembraneVariable** memVarList;
	int memVarSize;
	VolumeRegionVariable** volRegionVarList;
	int volRegionVarSize;
	MembraneRegionVariable** memRegionVarList;
	int memRegionVarSize;

	VolumeParticleVariable** volParticleVarList;
	int volParticleVarSize;
	MembraneParticleVariable** memParticleVarList;
	int memParticleVarSize;

	vector<RandomVariable*> randomVarList;

	int numRegionSizeVars;
	RegionSizeVariable** regionSizeVarList;

	int numVolPde;
	int numMemPde;
	bool bHasTimeDependentDiffusionAdvection;

	int psfFieldDataIndex;

	void reinitConstantValues(SimTool* sim_tool);

	int numSymbols;
	int symbolIndexOffset_T;
	int symbolIndexOffset_Xyz;
	int symbolIndexOffset_VolVar;
	int symbolIndexOffset_MemVar;
	int symbolIndexOffset_VolRegionVar;
	int symbolIndexOffset_MemRegionVar;
	int symbolIndexOffset_VolParticleVar;
	int symbolIndexOffset_MemParticleVar;
	int symbolIndexOffset_RegionSizeVariable;
	int symbolIndexOffset_FieldData;
	int symbolIndexOffset_RandomVar;
	int symbolIndexOffset_Parameters;
};

#endif
