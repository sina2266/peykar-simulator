#ifndef vhd5_types_h
#define vhd5_types_h
#include <vhdf5/vH5cpp.h>
namespace vcellH5 {
	/**
	* templates to convert C++ types 
	* to corresponding H5:PredTypes
	*/
	template <class T>
	struct TPredType {
	};
	template <>
	struct TPredType<const double> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_DOUBLE;
		}
	};
	template <>
	struct TPredType<double> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_DOUBLE;
		}
	};
	template <>
	struct TPredType<const float> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_FLOAT;
		}
	};
	template <>
	struct TPredType<float> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_FLOAT;
		}
	};
	template <>
	struct TPredType<const short> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_SHORT;
		}
	};
	template <>
	struct TPredType<short> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_SHORT;
		}
	};
	template <>
	struct TPredType<const int> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_INT;
		}
	};
	template <>
	struct TPredType<int> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_INT;
		}
	};
	template <>
	struct TPredType<const unsigned int> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_UINT;
		}
	};
	template <>
	struct TPredType<unsigned int> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_UINT;
		}
	};
	template <>
	struct TPredType<const long> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_LONG;
		}
	};
	template <>
	struct TPredType<long> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_LONG;
		}
	};
	template <>
	struct TPredType<const unsigned long> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_ULONG;
		}
	};
	template <>
	struct TPredType<unsigned long> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_ULONG;
		}
	};
	template <>
	struct TPredType<const unsigned long long> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_ULLONG;
		}
	};
	template <>
	struct TPredType<unsigned long long> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_ULLONG;
		}
	};
	template <>
	struct TPredType<unsigned char> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_UCHAR;
		}
	};
	template <>
	struct TPredType<char> {
		static H5::PredType predType( ) {
			return H5::PredType::NATIVE_CHAR;
		}
	};
}
#endif
