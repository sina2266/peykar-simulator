/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include "ValueProxy.h"

//-----------------------------------------------------------------
//
//  class ValueProxy
//
//-----------------------------------------------------------------
ValueProxy::ValueProxy(double* arg_values, int arg_indexindex, int* arg_indices)
{	
	values = arg_values;
	indexindex = arg_indexindex;
	indices = arg_indices;
}

double ValueProxy::evaluate(){
	return values[indices[indexindex]];
}
