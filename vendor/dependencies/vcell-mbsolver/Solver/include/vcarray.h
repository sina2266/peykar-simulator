#ifndef vcarray_h
#define vcarray_h
#include <array>

namespace vcell_util {
	/**
	* hack around MSVC's 2012 lack of size for const expr
	*/
	template <typename T, int N>
	struct vcarray : public std::array<T,N> {
		static const size_t ArraySize= N;
	};
};	

#endif
