#ifndef explore_h
#define explore_h
#include "mex.h"
/* Pass analyze_structure a pointer to a structure mxArray.  Each element
   in a structure mxArray holds one or more fields; each field holds zero
   or one mxArray.  analyze_structure accesses every field of every
   element and displays information about it. */ 
void analyze_structure(const mxArray *structure_array_ptr);
#endif
