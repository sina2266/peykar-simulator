#ifndef vcellutil_h
#define vcellutil_h
#include <array>
namespace vcell_util {

	/**
	* delete object and set pointer to null
	* @param &ptr reference to pointer to operate one
	*/
	template <class T>
	void deleteAndNull(T * &ptr) {
		delete ptr;
		ptr = nullptr;
	}

	/**
	* return std::array<T,2>
	*/
	template<class T>
	std::array<T,2> arrayInit(const T & a, const T & b) {
		std::array<T,2> ary;
		ary[0] = a;
		ary[1] = b;
		return ary;
	}

	/**
	* return std::array<T,3>
	*/
	template<class T>
	std::array<T,3> arrayInit(const T & a, const T & b, const T & c) {
		std::array<T,3> ary;
		ary[0] = a;
		ary[1] = b;
		ary[2] = c;
		return ary;
	} 
}
#endif
