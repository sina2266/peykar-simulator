#ifndef vhd5_vlen_h
#define vhd5_vlen_h
#include <vhdf5/types.h>
#include <vhdf5/vH5cpp.h>
namespace vcellH5 {
	template <class T>
	struct VarLen {
		/**
		* @param dt datatype (okay if the argument is invalidated after construction as the HDF5 implementation is reference counted)
		*/
		VarLen(H5::DataType dt) 
			:dataType(H5::VarLenType(&dt)) {}

		/**
		* @return hvl_t backed by vector passed at constructor; will most likely become invalidated if vector altered 
		*/
		hvl_t adapt(const std::vector<T> & vec) {
			hvl_t  hdf5Data;
			hdf5Data.p = const_cast<T *>(vec.data( ));
			hdf5Data.len = vec.size( );
			return hdf5Data;
		}

		/**
		* @return variable length type based on data type passed into constructor
		*/
		H5::DataType getType( ) const {
			return dataType;
		}
	private:
		const H5::DataType dataType;
	};
	template <class T>
	struct VarLenSimple : public VarLen<T> {
		/**
		* use #TPredType for #T to generate default
		*/
		VarLenSimple( )
		:VarLen<T>(supportedType( ) ) {}

		 static H5::VarLenType arrayType( ) {
			 return H5::VarLenType(supportedType( ));
		 }

	private:
		/**
		 * return default type; must be function (not static member) due to static member 
		 * initialization issues
		 */
		static H5::DataType &supportedType( ) {
			static H5::DataType st(TPredType<T>::predType( ));
			return st;
		}
	};
	
}
#endif
