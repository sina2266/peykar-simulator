#ifndef Separator_h
#define Separator_h
#include <iostream>

namespace vcell_util {

	/**
	* streamable object which skips output first time. Used for internal separation (e.g. commas) in list
	* @tparam S object type to to stream
	*/
	template <typename S = char>
	struct Separator {
		/**
		* create
		* @param s value to stream
		*/
		Separator(S s = ',')
			:sep(s),
			first(true) {}

		/**
		* output. Provided for << operator, not direct use
		* @param os stream to write to
		*/
		void write(std::ostream &os) {
			if (!first) {
				os << sep;
			}
			else {
				first = false;
			}
		}
		/**
		* reset to intial state
		*/
		void reset( ) {
			first = true;
		}

	private:
		S sep;
		bool first;

	};

	template <typename S>
	std::ostream &operator<< (std::ostream &os, Separator<S> &sep) {
		sep.write(os);
		return os;
	}

}
#endif
