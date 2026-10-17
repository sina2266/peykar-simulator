/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#include <VCELL/ProjectionDataGenerator.h>
#include <VCELL/SimulationExpression.h>
#include <VCELL/CartesianMesh.h>
#include <VCELL/Element.h>
#include <string.h>
#include <Expression.h>
using VCell::Expression;

#include <algorithm>
using std::max;
using std::min;

const string ProjectionDataGenerator::Projection_OP_max = "max";
const string ProjectionDataGenerator::Projection_OP_min = "min";
const string ProjectionDataGenerator::Projection_OP_avg = "avg";
const string ProjectionDataGenerator::Projection_OP_sum = "sum";
	
const string ProjectionDataGenerator::Projection_Axis_x = "x";
const string ProjectionDataGenerator::Projection_Axis_y = "y";
const string ProjectionDataGenerator::Projection_Axis_z = "z";

ProjectionDataGenerator::ProjectionDataGenerator(string& name, Feature* feature, string& axis, string& op, Expression* func) 
	: DataGenerator(name, feature){
	this->axis = axis;
	this->op = op;
	this->function = func;
}

ProjectionDataGenerator::~ProjectionDataGenerator() {
	delete function;
}

void ProjectionDataGenerator::resolveReferences(SimulationExpression* sim) {
	function->bindExpression(sim->getSymbolTable());

	// compute data size
	CartesianMesh* mesh = (CartesianMesh*)sim->getMesh();
	dataSize = mesh->getNumVolumeX() * mesh->getNumVolumeY() * mesh->getNumVolumeZ();
	data = new double[dataSize];
	memset(data, 0, dataSize * sizeof(double));

	hdf5Rank = mesh->getDimension() - 1;
	if (hdf5Rank == 1) {
		hdf5Dims[0] = axis == Projection_Axis_x ? mesh->getNumVolumeY() : mesh->getNumVolumeX();
	} else if (hdf5Rank == 2) {
		if (axis == Projection_Axis_z) {
			hdf5Dims[1] = mesh->getNumVolumeX();
			hdf5Dims[0] = mesh->getNumVolumeY();
		} else if (axis == Projection_Axis_y) {
			hdf5Dims[1] = mesh->getNumVolumeX();
			hdf5Dims[0] = mesh->getNumVolumeZ();
		} else if (axis == Projection_Axis_x) {
			hdf5Dims[1] = mesh->getNumVolumeY();
			hdf5Dims[0] = mesh->getNumVolumeZ();
		}
	} else {
		throw "Projection is not supported in 1D simulation";
	}
}

void ProjectionDataGenerator::computePPData(SimulationExpression* sim) {
	CartesianMesh* mesh = (CartesianMesh*)sim->getMesh();
	int dimension = mesh->getDimension();
	if (dimension == 1) {
		throw "Projection is not supported in 1D simulation";
	}
	if (dimension == 2) {
		computePPData2D(sim);
	} else if (dimension == 3) {
		computePPData3D(sim);
	}
}

void ProjectionDataGenerator::computePPData2D(SimulationExpression* sim) {
	CartesianMesh* mesh = (CartesianMesh*)sim->getMesh();
	VolumeElement* volumeElements = mesh->getVolumeElements();
	int* indices = sim->getIndices();
	indices[VAR_MEMBRANE_INDEX] = -1;
	indices[VAR_MEMBRANE_REGION_INDEX] = -1;

	int numX = mesh->getNumVolumeX();
	int numY = mesh->getNumVolumeY();
	if (axis == Projection_Axis_y) {
		// fill all the data in the first row in x axis
		// for each point in X, loop through Y to do OP (min, max, sum)
		for (int i = 0; i < numX; ++i) {
			int dataIndex = i;

			double minValue = DataGenerator::double_max, maxValue = DataGenerator::double_min, sum = 0;
			for (int j = 0; j < numY; ++j) {
				long volIndex = j * numX + i;
				if (feature != NULL && feature != volumeElements[volIndex].getFeature()) {
					continue;
				}
				
				indices[VAR_VOLUME_INDEX] = volIndex;
				indices[VAR_VOLUME_REGION_INDEX] = volumeElements[volIndex].getRegionIndex();

				double value = function->evaluateProxy();
				minValue = min(value, minValue);
				maxValue = max(value, maxValue);
				sum += value;
			}
			if (op == Projection_OP_max) {
				data[dataIndex] = maxValue;
			} else if (op == Projection_OP_min) {
				data[dataIndex] = minValue;
			} else if (op == Projection_OP_sum) {
				data[dataIndex] = sum;
			} else if (op == Projection_OP_avg) {
				data[dataIndex] = sum/numY;
			}
		}

		// duplicate the first row to the rest of the rows
		// for displaying purpose only
		for (int j = 1; j < numY; ++ j) {
			memcpy(data + j * numX, data, numX * sizeof(double));
		}
	} else if (axis == Projection_Axis_x) {
		// fill all the data in the first colume in Y axis
		for (int j = 0; j < numY; ++j) {		
			int dataIndex = j * numX;

			double minValue = DataGenerator::double_max, maxValue = DataGenerator::double_min, sum = 0;
			for (int i = 0; i < numX; ++i) {	
				long volIndex = j * numX + i;				
				if (feature != NULL && feature != volumeElements[volIndex].getFeature()) {
					continue;
				}

				indices[VAR_VOLUME_INDEX] = volIndex;
				indices[VAR_VOLUME_REGION_INDEX] = volumeElements[volIndex].getRegionIndex();

				double value = function->evaluateProxy();
				minValue = min(value, minValue);
				maxValue = max(value, maxValue);
				sum += value;
			}
			if (op == Projection_OP_max) {
				data[dataIndex] = maxValue;
			} else if (op == Projection_OP_min) {
				data[dataIndex] = minValue;
			} else if (op == Projection_OP_sum) {
				data[dataIndex] = sum;
			} else if (op == Projection_OP_avg) {
				data[dataIndex] = sum/numX;
			}
		}

		// duplicate the first column to the rest of the columns
		// for displaying purpose only
		for (int j = 0; j < numY; ++ j) {
			for (int i = 1; i < numX; ++ i) {
				data[j* numX + i] = data[ j * numX];
			}
		}
	}
}

void ProjectionDataGenerator::computePPData3D(SimulationExpression* sim) {
	CartesianMesh* mesh = (CartesianMesh*)sim->getMesh();
	VolumeElement* volumeElements = mesh->getVolumeElements();
	int* indices = sim->getIndices();
	indices[VAR_MEMBRANE_INDEX] = -1;
	indices[VAR_MEMBRANE_REGION_INDEX] = -1;

	int numX = mesh->getNumVolumeX();
	int numY = mesh->getNumVolumeY();
	int numXY = numX * numY;
	int numZ = mesh->getNumVolumeZ();
	if (axis == Projection_Axis_z) {
		for (int j = 0; j < numY; ++j) {
			for (int i = 0; i < numX; ++i) {
				int dataIndex = j * numX + i;
				double minValue = DataGenerator::double_max, maxValue = DataGenerator::double_min, sum = 0;

				for (int k = 0; k < numZ; ++k) {
					long volIndex = k * numXY + j * numX + i;
					if (feature != NULL && feature != volumeElements[volIndex].getFeature()) {
						continue;
					}
				
					indices[VAR_VOLUME_INDEX] = volIndex;
					indices[VAR_VOLUME_REGION_INDEX] = volumeElements[volIndex].getRegionIndex();

					double value = function->evaluateProxy();
					minValue = min(value, minValue);
					maxValue = max(value, maxValue);
					sum += value;
				}

				if (op == Projection_OP_max) {
					data[dataIndex] = maxValue;
				} else if (op == Projection_OP_min) {
					data[dataIndex] = minValue;
				} else if (op == Projection_OP_sum) {
					data[dataIndex] = sum;
				} else if (op == Projection_OP_avg) {
					data[dataIndex] = sum/numZ;
				}
			}
		}
		// duplicate the first XY slice to the rest of the slices
		// for displaying purpose only
		for (int k = 1; k < numZ; ++ k) {
			memcpy(data + k * numXY, data, numXY * sizeof(double));
		}
	} else if (axis == Projection_Axis_y) {	
		for (int k = 0; k < numZ; ++k) {
			for (int i = 0; i < numX; ++i) {
				int dataIndex = k * numXY + i;
				double minValue = DataGenerator::double_max, maxValue = DataGenerator::double_min, sum = 0;

				for (int j = 0; j < numY; ++j) {
					long volIndex = k * numXY + j * numX + i;
					if (feature != NULL && feature != volumeElements[volIndex].getFeature()) {
						continue;
					}

					indices[VAR_VOLUME_INDEX] = volIndex;
					indices[VAR_VOLUME_REGION_INDEX] = volumeElements[volIndex].getRegionIndex();

					double value = function->evaluateProxy();
					minValue = min(value, minValue);
					maxValue = max(value, maxValue);
					sum += value;
				}

				if (op == Projection_OP_max) {
					data[dataIndex] = maxValue;
				} else if (op == Projection_OP_min) {
					data[dataIndex] = minValue;
				} else if (op == Projection_OP_sum) {
					data[dataIndex] = sum;
				} else if (op == Projection_OP_avg) {
					data[dataIndex] = sum/numY;
				}
			}
		}
		// duplicate the first XZ slice to the rest of the slices
		// for displaying purpose only
		for (int k = 0; k < numZ; ++k) {
			for (int j = 1; j < numY; ++j) {
				for (int i = 0; i < numX; ++i) {
					data[k * numXY + j* numX + i] = data[k * numXY + i];
				}
			}
		}
	} else if (axis == Projection_Axis_x) {	
		for (int k = 0; k < numZ; ++k) {
			for (int j = 0; j < numY; ++j) {			
				int dataIndex = k * numXY + j * numX;
				double minValue = DataGenerator::double_max, maxValue = DataGenerator::double_min, sum = 0;

				for (int i = 0; i < numX; ++i) {
					long volIndex = k * numXY + j * numX + i;
					if (feature != NULL && feature != volumeElements[volIndex].getFeature()) {
						continue;
					}

					indices[VAR_VOLUME_INDEX] = volIndex;
					indices[VAR_VOLUME_REGION_INDEX] = volumeElements[volIndex].getRegionIndex();

					double value = function->evaluateProxy();
					minValue = min(value, minValue);
					maxValue = max(value, maxValue);
					sum += value;
				}

				if (op == Projection_OP_max) {
					data[dataIndex] = maxValue;
				} else if (op == Projection_OP_min) {
					data[dataIndex] = minValue;
				} else if (op == Projection_OP_sum) {
					data[dataIndex] = sum;
				} else if (op == Projection_OP_avg) {
					data[dataIndex] = sum/numX;
				}
			}
		}
		// duplicate the first YZ slice to the rest of the slices
		// for displaying purpose only
		for (int k = 0; k < numZ; ++k) {
			for (int j = 0; j < numY; ++j) {
				for (int i = 1; i < numX; ++i) {
					data[k * numXY + j * numX + i] = data[k * numXY + j * numX];
				}
			}
		}
	}
}
