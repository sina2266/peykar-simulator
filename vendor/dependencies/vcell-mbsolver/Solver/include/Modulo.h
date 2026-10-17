#ifndef Modulo_h
#define Modulo_h

#include <limits>

namespace vcell_util {
	/**
	* modulo integer type
	* @tparam INTERFACE_TYPE is the type as it appears, must be an integer type
	* @tparam IMPL_TYPE is the type used for internal implementation, and must be a signed integer type
	* static casts are used to avoid warnings ; user is responsible for ensuring IMPL_TYPE is large
	* enough to handle expected range
	*/
	template <class INTERFACE_TYPE, class IMPL_TYPE = INTERFACE_TYPE>
	struct Modulo {
		typedef struct Modulo<INTERFACE_TYPE,IMPL_TYPE> OurType;
		static_assert(std::numeric_limits<INTERFACE_TYPE>::is_integer, "must use integer interface type");
		static_assert(std::numeric_limits<IMPL_TYPE>::is_signed, "must use unsigned implementation type");
		static_assert(std::numeric_limits<IMPL_TYPE>::is_integer, "must use integer implementation type");

		/**
		* v initial value
		* base_ modulo base
		*/
		Modulo(INTERFACE_TYPE v, INTERFACE_TYPE base_)
			:value(static_cast<IMPL_TYPE>(v)),
			base(static_cast<IMPL_TYPE>(base_)) { mod( );}

		Modulo & operator++() {
			value ++;
			mod( );
			return *this;
		}
		Modulo operator++(int) {
			OurType copy(*this);
			value ++;
			mod( );
			return copy; 
		}

		Modulo & operator--() {
			value --;
			mod( );
			return *this;
		}

		
		template <class OP>
		Modulo & operator+=(OP v) {
			value += v;
			mod( );
			return *this;
		}

		Modulo & operator=(INTERFACE_TYPE v) {
			value = static_cast<IMPL_TYPE>(v);
			mod( );
			return *this;
		}

		template <class OP>
		Modulo & operator-=(OP v) {
			value -= v;
			mod( );
			return *this;
		}

		operator INTERFACE_TYPE( ) const {
			return value;
		}


	private:
		void mod( ) {
			value %= base;
			if (value < 0) {
				value += base;
			}
		}
		IMPL_TYPE value;
		const IMPL_TYPE base;
	};
}
#endif
