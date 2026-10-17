#ifndef vhd5_attribute_h
#define vhd5_attribute_h
#include <vhdf5/types.h>
#include <vhdf5/suppressor.h>
namespace vcellH5 {
	/**
	* remove attribute if it exists
	*/
	inline void removeAttribute(H5::H5Location &parent, const char *attributeName) {
		Suppressor s; //disable error messages

		H5Adelete(parent.getId(), attributeName);
	}

	/**
	* Example: \n
		vcellH5::SeqFacade<std::vector<double> > gt(genTime); \n
		vcellH5::facadeWriteAttribute(dataset,"generationTimes",gt); \n
	* @param parent object to write to
	* @param attributeName name to use
	* @param facade Facade adapter class
	* @param overwrite remove existing existing attribute if present 
	*/
	template <class FACADE>
	void facadeWriteAttribute(H5::H5Object &parent, const char *attributeName, FACADE & facade, bool overwrite = false) {
		if (overwrite) {
			removeAttribute(parent,attributeName);
		}
		if (!facade.empty( )) {
			std::array<hsize_t,FACADE::N> dim;
			facade.fillDim(dim);
			H5::DataType dt = facade.elementType( );
			H5::DataSpace dataspace(FACADE::N,dim.data( ));
			H5::Attribute attr = parent.createAttribute(attributeName,dt,dataspace);
			static_assert(FACADE::N == 1, "only N =1 implemented thus far");
			if (FACADE::N == 1) {
				std::vector<typename FACADE::Container::value_type> buffer(facade.arraySize( ));
				facade.fill(buffer);
				attr.write(dt,buffer.data( ));
			}
		}
		else {
			const H5::PredType predType = TPredType<typename FACADE::Container::value_type>::predType( );
			H5::DataType opaqueType(H5T_OPAQUE,sizeof(H5::PredType));
			opaqueType.setTag("empty");
			hsize_t dim[1] = {1};
			H5::DataSpace dataspace(1,dim);
			H5::Attribute attr = parent.createAttribute(attributeName,opaqueType,dataspace);
			attr.write(opaqueType,&predType);
		}
	}

	template <class T>
	void writeAttribute(H5::H5Object &parent, const char *attributeName, T& value, bool overwrite = false) {
		if (overwrite) {
			removeAttribute(parent,attributeName);
		}
		const H5::PredType predType = TPredType<T>::predType( );
		hsize_t dim[1] = {1};
		H5::DataSpace dataspace(1,dim);
		H5::Attribute attr = parent.createAttribute(attributeName,predType,dataspace);
		attr.write(predType,&value);
	}
	template <>
	inline void writeAttribute<const std::string>(H5::H5Object &parent, const char *attributeName, const std::string& value, bool overwrite) {
		if (overwrite) {
			removeAttribute(parent,attributeName);
		}
		// HDF5 rejects a zero-size string type; store an empty string as one NUL byte.
		H5::StrType st(0,value.empty( ) ? 1 : value.length( ));
		H5::DataSpace dataspace(H5S_SCALAR);
		H5::Attribute attr = parent.createAttribute(attributeName,st,dataspace);
		attr.write(st,value);
	}
	/**
	* overload non-const string by explicity invoking const template specialization
	*/
	template <>
	inline void writeAttribute<std::string>(H5::H5Object &parent, const char *attributeName, std::string& value, bool overwrite)  {
		writeAttribute<const std::string>(parent,attributeName,value, overwrite);
	}
}
#endif
