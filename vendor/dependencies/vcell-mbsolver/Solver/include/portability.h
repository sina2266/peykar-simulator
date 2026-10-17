#ifndef portability_h
#define portability_h
namespace vcell_portable {
#if defined( __GNUG__)
#include <cmath>
#undef isnan
	inline bool isNotANumber(float f) {
		return std::isnan(f);
	}
	inline bool isNotANumber(double d) {
		return std::isnan(d);
	}
	inline bool isNotANumber(long double ld) {
		return std::isnan(ld);
	}
#elif defined(_MSC_VER)
#include <float.h>
	inline bool isNotANumber(float f) {
		return _isnan(f) != 0;
	}
	inline bool isNotANumber(double d) {
		return _isnan(d) != 0;
	}
	inline bool isNotANumber(long double ld) {
		return _isnan(ld) != 0;
	}
#endif
	/**
	* all ints are numbers
	*/
	inline bool isNotANumber(int ld) {
		return false;
	}
}

#endif
