#ifndef vhd5_suppressor_h
#define vhd5_suppressor_h
#include <hdf5.h>

namespace vcellH5 {
	/**
	* stack based suppression of HDF error stack
	*/
	struct Suppressor {
		/**
		* suppress message from specified stack
		*/
		Suppressor(hid_t stack = 0) {
			H5Eget_auto(0,&currentFunction,&currentData);
			H5Eset_auto(0,0,0);
		}

		~Suppressor() {
			H5Eset_auto(0,currentFunction,currentData);
		}
	private:
		H5E_auto_t currentFunction;
		void *currentData;
	};
}
#endif
