#ifndef vhd5_facade_h
#define vhd5_facade_h
#include <vhdf5/types.h>
namespace vcellH5 {
	/**
	* general sequence container facade
	*/
	template <class T>
	struct SeqFacade {
		typedef T Container;
		typename Container::const_iterator IteratorType;
		/**
		* this is the number of dimensions supported by the facade
		*/
		const static size_t N = 1;
		SeqFacade(T & v)
			:container(v) {}
		static size_t numDimensions( ) {
			return N;
		}
		H5::DataType elementType( ) const {
			const H5::PredType predType = TPredType<typename Container::value_type>::predType( );
			hsize_t dim[1] = { container.size( )};
			H5::ArrayType arrayType(predType,N,dim);
			return arrayType;
		}
		void fillDim(std::array<hsize_t,1> &dim) const {
			dim[0] = N; 
		}
		size_t arraySize( ) const {
			return container.size( );
		}
		void fill(std::vector<typename Container::value_type> &buffer) {
			assert(buffer.size( ) == container.size( ));
			std::copy(container.begin( ),container.end( ), buffer.begin( )); 
		}

		bool empty( ) const {
			return container.empty( );
		}
	private:
		T & container;
	};

	/**
	* future specialization?
	*/
	template<class T>
	struct VectorFacade {};

	template <>
	struct VectorFacade<double> : public SeqFacade<std::vector<double> > {
		VectorFacade(std::vector<double> &vOfd)
			:SeqFacade<std::vector<double> >(vOfd) {} 
	};
}
#endif
