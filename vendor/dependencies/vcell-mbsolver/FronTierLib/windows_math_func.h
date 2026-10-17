#ifdef WIN32

#ifndef WINDOWS_MATH_FUNC
#define WINDOWS_MATH_FUNC

#define isnan(x)    _isnan(x)
#define strcasecmp(x,y) _stricmp(x,y)
#define strncasecmp(x,y,n) _strnicmp(x,y,n)
#define isgreater(x,y) (x > y)

double erand48(unsigned short xsubi[3]);
double erf(double x);
double erfc(double x);
// double rint(double x);

#endif // WINDOWS_MATH_FUNC

#endif
