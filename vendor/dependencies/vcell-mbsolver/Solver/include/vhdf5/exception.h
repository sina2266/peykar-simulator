#ifndef vhd5_exception_h
#define vhd5_exception_h
#include <H5Exception.h>
#include <stdexcept>

namespace vcellH5 {
	struct Exception : public std::exception {
		Exception(H5::Exception &e)
			:h5e(e) {}

		virtual const char *what() const throw( ) {
			return h5e.getCDetailMsg( );
		}
		const H5::Exception &h5e;
	};
}
#endif
