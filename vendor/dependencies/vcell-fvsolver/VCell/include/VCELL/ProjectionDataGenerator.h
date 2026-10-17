/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef PROJECTION_DATA_GENERATOR_H
#define PROJECTION_DATA_GENERATOR_H

#include <VCELL/DataGenerator.h>

namespace VCell {
	class Expression;
}

class ProjectionDataGenerator : public DataGenerator
{
public:
	ProjectionDataGenerator(string& name, Feature* f, string& axis, string& op, VCell::Expression* func);
	virtual ~ProjectionDataGenerator();

	void resolveReferences(SimulationExpression* sim);
	void computePPData(SimulationExpression* sim);

private:
	const static string Projection_OP_max;
	const static string Projection_OP_min;
	const static string Projection_OP_avg;
	const static string Projection_OP_sum;
	
	const static string Projection_Axis_x;
	const static string Projection_Axis_y;
	const static string Projection_Axis_z;
	
	string op;
	string axis;
	VCell::Expression* function;

	void computePPData2D(SimulationExpression* sim);
	void computePPData3D(SimulationExpression* sim);
};

#endif
