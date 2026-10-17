#ifndef NumericConvert_h
#define NumericConvert_h
#include <cmath>
#include <limits>
#include <cstdint>
#include <type_traits>
#include <VCellException.h>
/* The Windows SDK (<minwindef.h>, reached transitively via Boost/HDF5/FronTier)
 * defines IN and OUT as empty SAL annotation macros, which collide with the
 * template parameter names below.  They expand to nothing, so dropping them is
 * harmless. */
#ifdef IN
#  undef IN
#endif
#ifdef OUT
#  undef OUT
#endif
namespace vcell_util {
	/**
	* @tparam IN input type
	* @tparam OUT output type
	* return lowest valid OUT which is >= in, taking sign into consideration. 
	* e.g. ConvertUp<int>(-4.5) is -4 
	*/
	template <class OUT, class IN>
	OUT ConvertUp(IN in) {
		const IN x =in;
		x++;
	}

	/**
	* @tparam IN input type
	* @tparam OUT output type
	* return lowest valid OUT which is <= in, taking sign into consideration. 
	* e.g. ConverDown<int>(-4.5) is -5 
	*/
	template <class OUT, class IN>
	OUT ConvertDown(IN in) {
		const IN x =in;
		x++;
	}

	template <>
	inline int32_t ConvertDown(double in) {
		if (in >= 0) {
			return static_cast<int32_t>(in);
		}
		//implicit else 
		return static_cast<int32_t>(floor(in)); 
	}

	template <>
	inline int32_t ConvertUp(double in) {
		const double c = ceil(in);
		return static_cast<int32_t>(c);
	}

	template <>
	inline int64_t ConvertDown(double in) {
		if (in >= 0) {
			return static_cast<int64_t>(in);
		}
		return static_cast<int64_t>(floor(in)); 
	}

	template <>
	inline int64_t ConvertUp(double in) {
			return static_cast<int64_t>(ceil(in));
	}
#ifdef __MACH__

	template <>
	inline long int ConvertDown(double in) {
		if (in >= 0) {
			return static_cast<long int>(in);
		}
		return static_cast<long int>(floor(in)); 
	}

	template <>
	inline long int ConvertUp(double in) {
			return static_cast<long int>(ceil(in));
	}
#endif


	template <>
	inline int16_t ConvertDown(double in) {
		if (in >= 0) {
			return static_cast<int16_t>(in);
		}
		return static_cast<int16_t>(floor(in)); 
	}

	template <>
	inline int16_t ConvertUp(double in) {
			return static_cast<int16_t>(ceil(in));
	}

	template <>
	inline double ConvertDown(double in) {
		return in;
	}

	template <>
	inline double ConvertUp(double in) {
		return in;
	}


	template <>
	inline int64_t ConvertDown(long double in) {
		if (in >= 0) {
			return static_cast<int64_t>(in);
		}
		return static_cast<int64_t>(floor(in)); 
	}

	template <>
	inline int64_t ConvertUp(long double in) {
		return static_cast<int64_t>(in);
	}

	//template long ConvertUp<long, double>(double); 
	//template long ConvertDown<long, double>(double); 

	template <class T>
	bool validMultiply(T a, T b) {
		return std::numeric_limits<T>::max( ) / std::abs(a) >= std::abs(b);
	}
#ifdef _MSC_VER
//MSVC doesn't define long as one of the cstdint types
	template <>
	inline long ConvertDown(double in) {
		if (in >= 0) {
			return static_cast<long>(in);
		}
		return static_cast<long>(floor(in)); 
	}

	template <>
	inline long ConvertUp(double in) {
			return static_cast<long>(ceil(in));
	}
#endif

	template <typename T>
	typename std::enable_if<std::numeric_limits<T>::is_integer, unsigned int>::type
	numberDigits(T value) {
		unsigned int digits = 0;
		if (value < 0) digits = 1;
		while (value) {
			value /= 10;
			++digits;
		}
		return digits;
	}
}
#endif
