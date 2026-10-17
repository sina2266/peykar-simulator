#ifndef vhd5_file_h
#define vhd5_file_h
#include <vhdf5/vH5cpp.h>
#include <sys/stat.h>

namespace vcellH5 {
	/**
	* subclass H5::H5File to provide smarter class than creates or opens existing
	* depending on whether file already exists or not
	*/
	struct VH5File : public H5::H5File {
		/**
		* default
		*/
		VH5File( )
			:H5::H5File( ) {}
		/**
		* copy
		* take base class as argument as this has no data
		*/
		VH5File(const H5::H5File & rhs)
			:H5::H5File(rhs) {}

		/**
		* create new or open existing file
		*/
		VH5File( const char* name, unsigned int flags = H5F_ACC_TRUNC|H5F_ACC_RDWR,
			const H5::FileCreatPropList& create_plist = H5::FileCreatPropList::DEFAULT,
			const H5::FileAccPropList& access_plist = H5::FileAccPropList::DEFAULT )
			:H5::H5File(name,smartFlags(name,flags),create_plist,access_plist) {}
	private:
		/**
		* figure out which flags to use depending on whether file exists
		*/
		static int smartFlags(const char *name, int input) {
			if (name != nullptr) {
				const int readFlags = H5F_ACC_RDONLY|H5F_ACC_RDWR;
				const int creationFlags = H5F_ACC_EXCL|H5F_ACC_TRUNC|H5F_ACC_DEBUG;

				struct stat filestat;
				if (stat(name,&filestat) == 0) { //file exists already
					return input & readFlags; 
				}
				else {
					return input & creationFlags; 
				}
			}
			throw std::invalid_argument("null pointer to vcellH5::VHFile constructor");
		}

	};
}
#endif
