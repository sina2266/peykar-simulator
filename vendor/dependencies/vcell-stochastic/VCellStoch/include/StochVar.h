#ifndef STOCHVAR_H
#define STOCHVAR_H

#include <stdint.h>

/* This class defines stochastic variable.
 * The class's only attribute is a pointer link to the value of the variable.
 * The variable can be accessed through two public functions.
 * Reference method descriptions in StochVar.cpp.
 *
 * @Author: Boris Slepchenko
 * @Amended: Tracy LI
 * @version:1.0 Beta
 * @CCAM,UCHC. May 25,2006
 */
class StochVar
{
    public:
       StochVar(uint64_t); 
	   ~StochVar();
	   uint64_t *getCurr() {return currValue;}
	   void setCurr(uint64_t);
    private:
       uint64_t *currValue; //the pointer to where the value is stored.
};
#endif

