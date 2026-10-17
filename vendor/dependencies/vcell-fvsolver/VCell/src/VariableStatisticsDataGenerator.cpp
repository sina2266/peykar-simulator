/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/VariableStatisticsDataGenerator.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Element.h>
#include <VCELL/Variable.h>
#include <VCELL/Feature.h>
#include <VCELL/Membrane.h>
#include <string.h>
#include <Expression.h>
#include <VCELL/ParticleVariable.h>
using VCell::Expression;

#include <algorithm>

const string VariableStatisticsDataGenerator::VariableStatistics_Name = "VariableStatistics";

VariableStatisticsDataGenerator::VariableStatisticsDataGenerator() 
	: DataGenerator(VariableStatistics_Name, NULL){
	statData = NULL;
}

VariableStatisticsDataGenerator::~VariableStatisticsDataGenerator() {
}

void VariableStatisticsDataGenerator::resolveReferences(SimulationExpression* sim) {
	int numVar = sim->getNumVariables();

	// compute data size
	statData = new StatData[numVar];
	memset(statData, 0, numVar * sizeof(StatData));
	dataSize = numVar * sizeof(StatData)/sizeof(double); // total doubles
	data = (double*)statData;  // force the data to take values from StatData

	hdf5Rank = 1;
	hdf5Dims[0] = dataSize;
}

void VariableStatisticsDataGenerator::computePPData(SimulationExpression* sim) {
	int numVar = sim->getNumVariables();

	CartesianMesh* mesh = (CartesianMesh *)sim->getMesh(); 

	int meshX = mesh->getNumVolumeX();
	int meshY = mesh->getNumVolumeY();
	int meshZ = mesh->getNumVolumeZ();

	MembraneElement* membraneElements = mesh->getMembraneElements();
	VolumeElement* volumeElements = mesh->getVolumeElements();

	memset(statData, 0, numVar * sizeof(StatData));
	for (int i = 0; i < numVar; i ++) {
		Variable* var = sim->getVariable(i);
		double totalVolume = 0;

		double& average = statData[i].average;
		double& total = statData[i].total;
		double& min = statData[i].min;
		double& max = statData[i].max;
		min = DataGenerator::double_max;
		max = DataGenerator::double_min;
		if (var->getVarType() == VAR_VOLUME) {
			for (int j = 0; j < meshX * meshY * meshZ; j ++) {
				if (var->getStructure() == 0 || var->getStructure() == volumeElements[j].getFeature()) {					
					double volume = mesh->getVolumeOfElement_cu(j);
					totalVolume += volume;

					double c = var->getCurr()[j];
					double curr = c * volume;// num of (1e-21) moles (=uM*um3)
					average += curr; //num of (1e-21) moles (=uM*um3)
					total += curr * 602.0; //num of molecules (602 = (molecules/um3*uM)*6.02e23)

					min = std::min<double>(min, c);
					max = std::max<double>(max, c);
				}
			}
		} else if (var->getVarType() == VAR_VOLUME_PARTICLE) {
			ParticleVariable* particleVar = (ParticleVariable *)var;
			for (int j = 0; j < meshX * meshY * meshZ; j ++) {
				if (var->getStructure() == 0 || var->getStructure() == volumeElements[j].getFeature()) {					
					double volume = mesh->getVolumeOfElement_cu(j);
					totalVolume += volume;

					long count = particleVar->getMoleculeCounts()[j];//num of (1e-21) moles (=uM*um3)
					average += count/602.0;//num of (1e-21) moles (=uM*um3)
					total += count; //num of molecules (602 = (molecules/um3*uM)*6.02e23)

					min = std::min<double>(min, count);
					max = std::max<double>(max, count);
				}
			}
		} else if (var->getVarType() == VAR_VOLUME_REGION) {
			for (int j = 0; j < meshX * meshY * meshZ; j ++) {
				if (var->getStructure() == 0 || var->getStructure() == volumeElements[j].getFeature()) {
					int regionIndex = volumeElements[j].region->getIndex();					
					double volume = mesh->getVolumeOfElement_cu(j);
					totalVolume += volume;

					double c = var->getCurr()[regionIndex];
					double curr = c * volume;
					average += curr;
					total += curr * 602.0;

					min = std::min<double>(min, c);
					max = std::max<double>(max, c);
				}
			}		
		} else if (var->getVarType() == VAR_MEMBRANE) {
			for (int j = 0; j < mesh->getNumMembraneElements(); j ++) {
				if (var->getStructure() == 0 || var->getStructure() == membraneElements[j].getMembrane()) {
					double area = membraneElements[j].area;
					totalVolume += area;

					double c = var->getCurr()[j];
					double mols = c * area;
					average += mols;
					total += mols;
					min = std::min<double>(min, c);
					max = std::max<double>(max, c);
				}
			}
		} else if (var->getVarType() == VAR_MEMBRANE_PARTICLE) {
			ParticleVariable* particleVar = (ParticleVariable *)var;
			for (int j = 0; j < mesh->getNumMembraneElements(); j ++) {
				if (var->getStructure() == 0 || var->getStructure() == membraneElements[j].getMembrane()) {
					double area = membraneElements[j].area;
					totalVolume += area;

					long count = particleVar->getMoleculeCounts()[j]; //num of molecules
					average += count; //num of molecules
					total += count; //num of molecules

					min = std::min<double>(min, count);
					max = std::max<double>(max, count);
				}
			}
		} else if (var->getVarType() == VAR_MEMBRANE_REGION) {
			for (int j = 0; j < mesh->getNumMembraneElements(); j ++) {
				if (var->getStructure() == 0 || var->getStructure() == membraneElements[j].getMembrane()) {
					int regionIndex = membraneElements[j].region->getIndex();
					double area = membraneElements[j].area;
					totalVolume += area;

					double c = var->getCurr()[regionIndex];
					double mols = c * area;
					average += mols;
					total += mols;

					min = std::min<double>(min, c);
					max = std::max<double>(max, c);
				}
			}
		}

		// average
		average /= totalVolume;
	}
}

void VariableStatisticsDataGenerator::writeAttributeComponent(H5::Group& dataGeneratorGroup, const char* varName, 
	int componentIndex, const char* dataName, const string& unit)
{
	H5::DataSpace attributeDataSpace(H5S_SCALAR);
	H5::StrType attributeStrType(0, 64);

	char attrName[64];
	char attrValue[64];

	//write name and unit
	snprintf(attrName, 64, "comp_%d_name", componentIndex);
	H5::Attribute attribute = dataGeneratorGroup.createAttribute(attrName, attributeStrType, attributeDataSpace);
	snprintf(attrValue, 64, "%s_%s", varName, dataName);
	attribute.write(attributeStrType, attrValue);

	snprintf(attrName, 64, "comp_%d_unit", componentIndex);
	attribute = dataGeneratorGroup.createAttribute(attrName, attributeStrType, attributeDataSpace);
	attribute.write(attributeStrType, unit.c_str());
}

void VariableStatisticsDataGenerator::detailGroup(H5::H5File* h5PPFile, H5::Group& dataGeneratorGroup, SimulationExpression* sim)
{
	// attributes : all the components, e.g. comp_0_name = varName, comp_0_unit = varUnit, comp_1_name = ... comp_1_unit = ...
	int numVar = sim->getNumVariables();
	int componentIndex = 0;
	for (int i = 0; i < numVar; i ++) {
		Variable* var = sim->getVariable(i);
		const char* varName = var->getName().c_str();
		std::string unit;

		//write var average name and unit
		unit = (var->getVarType() == VAR_MEMBRANE || var->getVarType() == VAR_MEMBRANE_PARTICLE || var->getVarType() == VAR_MEMBRANE_REGION)
			? "molecules.um-2" : "uM";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "average", unit);

		//write var total name and unit
		unit = "molecules";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "total", unit);
		unit = "uM";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "min", unit);
		unit = "uM";
		writeAttributeComponent(dataGeneratorGroup, varName, componentIndex ++, "max", unit);
	}
}
