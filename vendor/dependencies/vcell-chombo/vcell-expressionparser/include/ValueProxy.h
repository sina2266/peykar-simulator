/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef VALUEPROXY_H
#define VALUEPROXY_H

/*-----------------------------------------------------------
	vartype is the index to the indices of array
 ------------------------------------------------------------*/
class ValueProxy
{
public:
	ValueProxy(double* arg_values, int arg_vartype, int* arg_indices);  
	virtual double evaluate();

protected:
	double* values;
	int indexindex;	
	int* indices;
};

#endif
