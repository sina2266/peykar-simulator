/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SCALARVALUEPROXY_H
#define SCALARVALUEPROXY_H

#include "ValueProxy.h"

class ScalarValueProxy : public ValueProxy
{
public:
	ScalarValueProxy() : ValueProxy(0,  -1, 0) {
		value = 0;
	};

	double evaluate() {
		return value;
	};

	void setValue(double d) {
		value = d;
	}

private:
	double value;
};

#endif
