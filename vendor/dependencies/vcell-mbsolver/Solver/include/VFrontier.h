#ifndef VFrontier_h
#define VFrontier_h
/*
* compatability include for Frontier.h -- undef defines which can interfere with other code
*/

#ifdef _WIN32 //Visual Studio defines this by default
#ifndef WIN32
#define WIN32 //FronTier expects this for windows
#endif
#endif

#include <FronTier.h>
#undef Coords
#undef REAL 
#undef radians
#undef degrees 
#undef Error 
#undef isnan
#undef free 

#endif
