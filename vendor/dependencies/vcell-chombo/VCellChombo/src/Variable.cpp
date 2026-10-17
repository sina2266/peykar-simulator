/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */

#include <VCELL/Variable.h>
#include <parstream.H>
#include <string.h>
#include <algorithm>
#include <VCELL/Structure.h>
#include <VCELL/VarContext.h>
#include <REAL.H>
#include <cmath>
#include <iostream>
#include <limits>

const double Variable::double_max = std::numeric_limits<double>::max();
const double Variable::double_min = std::numeric_limits<double>::min();

Variable::Variable(string& nameStr, Structure* s, long Asize)
{
	name = nameStr;
	structure = s;
	size = Asize;
	bDiffusing = false;
	bElliptic = false;
	varContext = 0;
	curr = NULL;
//	bVariableDiffusing = false;
#ifndef CH_MPI
	exactErrorVar = 0;
	relativeErrorVar = 0;
#endif
}

Variable::~Variable()
{
	if (curr != NULL)
	{
		delete[] curr;
	}
	delete varContext;
}

double* Variable::getCurr()
{
	if (curr == NULL)
	{
		curr = new double[size];
	}
	return curr;
}

string Variable::getQualifiedName(){
	if (structure != 0){
		return structure->getName() + "::" + name;
	}
	return name;
}

void Variable::reset(bool bSaveVCellOutput)
{
	const char* methodName = "(Variable::reset)";
	pout() << "Entry " << methodName << ": " << getQualifiedName() << std::endl;

	mean = 0;
	total = 0;

#ifndef CH_MPI
	if (bSaveVCellOutput)
	{
		for (int i = 0; i < size; ++ i)
		{
			getCurr()[i] = BASEFAB_REAL_SETVAL;
		}
		if (exactErrorVar != NULL)
		{
			memset(exactErrorVar->getCurr(), 0, exactErrorVar->getSize() * sizeof(double));
			memset(relativeErrorVar->getCurr(), 0, relativeErrorVar->getSize() * sizeof(double));
		}
	}
	min = double_max;
	max = double_min;
	maxError = 0;
	l2Error = 0;
	l2Exact = 0;
	meanVCell = 0;
	totalVCell = 0;
#endif
	
	pout() << "Exit " << methodName << std::endl;
}

#ifndef CH_MPI
void Variable::addL2Error(double d)
{
	l2Error += d;
}
void Variable::addL2Exact(double d)
{
	l2Exact += d;
}
void Variable::addTotalVCell(double d)
{
	totalVCell += d;
}
void Variable::updateMaxError(double d)
{
	maxError = std::max<double>(maxError, d);
}
#endif

void Variable::computeFinalStatistics()
{
	mean = total / structure->getSize();
	if (getVarType() == VAR_VOLUME || getVarType() == VAR_VOLUME_REGION)
	{
		total *= 602;
	}
	pout() << " !!!!!!!!!!!!Variable " << name << ": total=" << total << "!!!!!!!!!!!!!!!!" << std::endl;

#ifndef CH_MPI
	if (curr != NULL)
	{
		meanVCell = totalVCell / structure->getSize();
		if (getVarType() == VAR_VOLUME || getVarType() == VAR_VOLUME_REGION)
		{
			totalVCell *= 602;
		}
		if (l2Exact != 0)
		{
			l2Error /= l2Exact;
		}
		l2Error = std::sqrt(l2Error);

		// curr is populated, so totalVCell would be computed.
		if (fabs(total - totalVCell) > 1e-5)
		{
			pout() << " !!!!!!!!!!!!Variable " << name << ": diff=" << fabs(total - totalVCell) << ", totalChombo(" << total << ") is different from totalVCell(" << totalVCell << ")!!!!!!!!!!!!! " << std::endl;
		}
	}
#endif
}

void Variable::addTotal(double d)
{
	total += d;
}
