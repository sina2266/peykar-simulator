#ifndef vhd5_dataset_h
#define vhd5_dataset_h
#include <vhdf5/types.h>
#include <vhdf5/vlen.h>
namespace vcellH5 {
	template <class FACADE>
	H5::DataSet facadeWrite(H5::H5Object &parent, const char *dataSetName, FACADE & facade) {
		if (!facade.empty( )) {
			std::array<hsize_t,FACADE::N> dim;
			facade.fillDim(dim);
			H5::DataType dt = facade.elementType( );
			H5::DataSpace dataspace(FACADE::N,dim.data( ));
			H5::DataSet dataset = parent.createDataSet(dataSetName,dt,dataspace);
			static_assert(FACADE::N == 1, "only N =1 implemented thus far");
			if (FACADE::N == 1) {
				std::vector<typename FACADE::Container::value_type> buffer(facade.arraySize( ));
				facade.fill(buffer);
				dataset.write(buffer.data( ),dt);
			}
			return dataset;
		}
		else {
			const H5::PredType predType = TPredType<typename FACADE::Container::value_type>::predType( );
			H5::DataType opaqueType(H5T_OPAQUE,sizeof(H5::PredType));
			opaqueType.setTag("empty");
			hsize_t dim[1] = {1};
			H5::DataSpace dataspace(1,dim);
			H5::DataSet dataset = parent.createDataSet(dataSetName,opaqueType,dataspace);
			dataset.write(&predType,opaqueType);
			return dataset;
		}
	}

	template <class T>
	H5::DataSet primitiveWrite(H5::H5Object &parent, const char *dataSetName, T& value) {
		const H5::PredType predType = TPredType<T>::predType( );
		hsize_t dim[1] = {1};
		H5::DataSpace dataspace(1,dim);
		H5::DataSet dataset = parent.createDataSet(dataSetName,predType,dataspace);
		dataset.write(&value,predType);
		return dataset;
	}
}
#endif
