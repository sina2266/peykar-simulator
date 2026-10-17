#include <assert.h>
#include <math.h>
#include <string>
#include "StochVar.h"

/*
 *Assign the initial value to the stochastic variable.
 *Input para: int, initial value.
 */
StochVar::StochVar(uint64_t initValue)
{
    currValue = new uint64_t();
	*currValue = initValue;
}//end of constructor StochVar(int)

//Destructor
StochVar::~StochVar()
{
	delete currValue;
}//end of destructor ~StochVar()

/*
 *Update the value of the stochastic variable.
 *Input para: int, value to be updated. 
 */
void StochVar::setCurr(uint64_t value)
{
	*currValue = value;
}//end of method setCurr

