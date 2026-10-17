/*
 * (C) Copyright University of Connecticut Health Center 2001.
 * All rights reserved.
 */
#ifndef SIMTYPES_H
#define SIMTYPES_H
#include <sstream>
#include <stdexcept>

struct DoubleVector3;

#if (defined(IRIX) || defined(CXX) || defined(LINUX))
#define UNIX_TIMER
#endif

typedef int int32;
typedef unsigned int uint32;

#ifdef VCELL_DEBUG
#if ( defined(WIN32) || defined(WIN64) )
#define ASSERTION(x) \
	if (!(x)) { \
		char str[300]; \
		sprintf(str, "File '%s', Line %d : Assertion Failed...\n",__FILE__, __LINE__); \
		throw str; \
	} 
#else
#define ASSERTION(x)  assert(x)
#endif
#else 
#define ASSERTION(x) ((void *)0)
#endif

typedef enum {
	VAR_UNKNOWN =			0,
	VAR_VOLUME =			1,
	VAR_MEMBRANE =			2,
	VAR_CONTOUR =			3,
	VAR_VOLUME_REGION =		4,
	VAR_MEMBRANE_REGION=	5,
	VAR_CONTOUR_REGION =	6,
	VAR_NONSPATIAL = 7,
	VAR_VOLUME_PARTICLE = 8,
	VAR_MEMBRANE_PARTICLE = 9
} VariableType;

typedef enum {
    LOCATION_VOLUME, 
    LOCATION_MEMBRANE, 
    LOCATION_CONTOUR, 
    LOCATION_ALL
} LocationContext;

typedef enum {
	BOUNDARY_VALUE, 
	BOUNDARY_FLUX, 
	BOUNDARY_PERIODIC
} BoundaryType;

typedef struct {
	double u;
	double v;
	double w;
} LocalCoord;

 struct IntVector3 {
	int x;
	int y;
	int z;
	IntVector3 & operator-=(const IntVector3 &rhs) {
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}
};

inline IntVector3 operator-(const IntVector3 &lhs,const IntVector3 &rhs) { 
	IntVector3 copy(lhs);
	copy -= rhs;
	return copy;
}

typedef DoubleVector3 WorldCoord;
typedef IntVector3    MeshCoord;

typedef long          MeshIndex;
typedef unsigned char FeatureHandle;
//typedef unsigned char ContourHandle;
	std::ostream& operator<<(std::ostream &,const IntVector3 & );

typedef enum {
	MATRIX_GENERAL=0,
	MATRIX_SYMMETRIC=1,
} MatrixSymmFlag;

namespace NeighborType { 
	enum NeighborStatus {good = 0,unset= -1,wall = -2, boundary = -3, unknown = -4};
	std::ostream& operator<<(std::ostream &,NeighborStatus);
}

template <class T, class E>
struct StatusIndex {

	/**
	* construct with value
	*/
	StatusIndex(T index = 0) 
		:value(index) {
			checkNonNegative(index);
		}
	/**
	* construct with status code
	*/
	StatusIndex(E e) 
		:value(e)
	{ verifyErrorValue(e); } 

	StatusIndex & operator=(T index) {
		checkNonNegative(index);
		value = index;
		return *this;
	}
	StatusIndex & operator=(E e) {
		verifyErrorValue(e);
		value = e;
		return *this;
	}

	/**
	* explicit get value
	*/
	T get( ) const {
		assert( valid( )); 
		return value;
	}

	/**
	* implicit get 
	*/
	operator T( ) const {
		return get( );
	}
	
	bool valid( ) const {
		return value >= 0;
	}

	E validity( ) const {
		if (value >= 0) {
			return static_cast<E>(0);
		}
		return static_cast<E>(value);
	}
	/**
	 * @return as signed int for writing to file
	 */
	signed long int asSignedLong( ) const {
	  return static_cast<signed long int>(value);
	}

private:
	T value; 
#ifndef NDEBUG
	void checkNonNegative(T index) {
		if (index >= static_cast<T>(0)) {
			return;
		}
		std::stringstream ss;
		ss << "StatusIndex " << index << " must be non-negative ";
		throw std::invalid_argument(ss.str( ));
	}
	void verifyErrorValue(E e) {
		if (e < 0) {
			return;
		}
		std::stringstream ss;
		ss << "StatusIndex enum " << e <<  " must be negative";
		throw std::invalid_argument(ss.str( ));
	}
#else
	void checkNonNegative(T index) { }
	void verifyErrorValue(E e) { }
#endif
//friend operator==(const StatusIndex<T,E> &lhs,const StatusIndex<T,E> &rhs); 
};

template <class T, class E>
inline bool operator==(const StatusIndex<T,E> &lhs,const StatusIndex<T,E> &rhs) {
	return lhs.value == rhs.value;
}
template <class T, class E>
inline bool operator==(const StatusIndex<T,E> &lhs,const T &rhs) {
	return lhs.valid() && lhs.get( ) == rhs;
}
template <class T, class E>
inline bool operator==(const T &lhs,const StatusIndex<T,E> &rhs) {
	return rhs.valid() && lhs == rhs.get( );
}
template <class T, class E>
inline bool operator==(const E &lhs,const StatusIndex<T,E> &rhs) {
	return lhs == rhs.validity( );
}
template <class T, class E>
inline bool operator==(const StatusIndex<T,E> &lhs,const E &rhs) {
	return lhs.validity( ) == rhs;
}
template <class T, class E>
inline std::ostream & operator<<(std::ostream & os, const StatusIndex<T,E> &e) {
	if (e.valid( )) {
		os << e.get( );
	}
	else {
		os << e.validity( );
	}
	return os;
}
#endif
