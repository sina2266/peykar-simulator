#ifndef Distance_h
#define Distance_h
#include <limits>
#include <cmath>
#include <TPoint.h>
#include <cassert>
#include <stdexcept>
#include <cstdint>
#include <VCellException.h>

namespace spatial {

	/**
	* default distance policy; use same types for all
	* works well with floating point types
	*/
	template <typename T>
	struct DefaultDistancePolicy{
		typedef T DistanceSquaredType;
		typedef T DistanceType;
		/**
		* allows range checking if subset of allowed types desired
		*/
		static void check(T) {}
		template <typename U>
		static DistanceType convert(U u) {
			return u;
		}
	};
	/**
	* a int64_t policy
	*/
	template <> 
	struct DefaultDistancePolicy<int64_t>{
		typedef double DistanceSquaredType;
		typedef int64_t DistanceType;
		static void check(int64_t) {}
		template <typename U>
		static DistanceType convert(U u) {
			// Use int64_t, not long: `long` is 32-bit on Windows (LLP64),
			// which would spuriously reject valid 64-bit values.
			if (u > std::numeric_limits<int64_t>::max( )) {
				VCELL_EXCEPTION(domain_error, u << " too big to be int64_t" );
			}
			return static_cast<int64_t>(u);
		}
	};
	/**
	* int sizes vary (more) from system to system; don't
	* provide default policy
	*/
	template <> 
	struct DefaultDistancePolicy<short>{
	};
	template <> 
	struct DefaultDistancePolicy<int>{
	};
#ifdef _MSC_VER
// long == int64_t on 64 bit linux
	template <> 
	struct DefaultDistancePolicy<long>{
	};
#endif


	/**
	* class to allow different policies to be used to calculate distances 
	* @tparam T TPoint<T,n> supported (for all positive n)
	* @tparam Policy policy for types, checking, and conversion 
	*/
	template <typename T, class Policy = DefaultDistancePolicy<T> >
	struct Distance {
		typedef typename Policy::DistanceType DistanceType;
		typedef typename Policy::DistanceSquaredType DistanceSquaredType;
		/**
		* distance squared; runtime cheaper than distance
		*/
		template<int N>
		static typename Policy::DistanceSquaredType squared(const TPoint<T,N> & lhs, const TPoint<T,N> & rhs ) {
			DistanceSquaredType dSquared = 0;
			for (int i = 0; i < N ;i++) {
				const Axis a = static_cast<Axis>(i);
				Policy::check(lhs(a));
				Policy::check(rhs(a));
				const DistanceSquaredType delta = lhs(a) - rhs(a);
				dSquared += delta * delta;
			}
			return dSquared;
		}

		/**
		* distance between
		* if supported by policy
		*/
		template<int N>
		static typename Policy::DistanceType calc(const TPoint<T,N> & lhs, const TPoint<T,N> & rhs ) {
			return Policy::convert(std::sqrt(squared(lhs,rhs)));
		}

		/**
		* distance between, cast (possibly truncated) to input type
		*/
		template<int N>
		static T approximate(const TPoint<T,N> & lhs, const TPoint<T,N> & rhs ) {
			return static_cast<T>(std::sqrt(squared(lhs,rhs)));
		}
	};
}
#endif
