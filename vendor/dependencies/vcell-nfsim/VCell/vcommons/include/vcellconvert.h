#ifndef vcellconvert_h
#define vcellconvert_h
#include <limits>
#include <string>
#include <cassert>
namespace vcell_util {

	template <typename TARGET, typename BASE> 
	struct VCellConvertUHelper {
		TARGET operator( )(BASE b) {
			if (b < std::numeric_limits<TARGET>::max( )) {
				return static_cast<TARGET>(b);
			}
			throw std::out_of_range("unsigned convert");
		}
	};
	template <typename TARGET>
	struct VCellConvert {
		TARGET operator( )(const char *v) {
			 return v; //SFINAE! add unimplemented type
		}
	};

	template <>
	struct VCellConvert<unsigned long> {
		unsigned long operator( )(const char *v) {
			char *end;
			unsigned long ul = std::strtoul(v,&end,10);
			const size_t len = end - v;
			if (len != strlen(v) ) { 
				throw std::domain_error("invalid unsigned int type");
			}
			return ul;
		}
	};
	template <>
	struct VCellConvert<unsigned short> {
		unsigned short operator( )(const char *v) {
			return VCellConvertUHelper<unsigned short, unsigned long>( )( VCellConvert<unsigned long>( )(v) ); 
		}
	};
	template <>
	struct VCellConvert<unsigned int> {
		unsigned int operator( )(const char *v) {
			return VCellConvertUHelper<unsigned int, unsigned long>( )( VCellConvert<unsigned long>( )(v) ); 
		}
	};

	template <>
	struct VCellConvert<long> {
		long operator( )(const char *v) {
			char *end;
			long lng = std::strtol(v,&end,10);
			const size_t len = end - v;
			if (len != strlen(v) ) { 
				throw std::domain_error("invalid unsigned int type");
			}
			return lng;
		}
	};

	template <>
	struct VCellConvert<double> {
		double operator( )(const char *v) {
			char *end;
			double d = std::strtod(v,&end);
			const size_t len = end - v;
			if (len != strlen(v) ) { 
				throw std::domain_error("invalid unsigned double");
			}
			return d;
		}
	};
	/*
	* specialize template; boost does not handle std::string conversion
	*/
	template <>
	struct VCellConvert<std::string> {
		std::string operator( )(const char *v) {
			return std::string(v);
		}
	};
	template <>
	struct VCellConvert<const char *> {
		const char * operator( )(const char *v) {
			return v;
		} 
	};
	template <>
	struct VCellConvert<char *> {
		char * operator( )(const char *v) {
			assert(false);
			//do not use this specialization; use <const char *> instead
			return 0;
		}  
	};
	template <>
	struct VCellConvert<bool> {
		bool operator( )(const char *v) {
			return strcmp(v,"false")!=0;
		}
	};

	/**
	* convenience method to convert const char * to specified type
	* @tparam T type to convert to
	* @throws boost std::exception subclass if string not parseable / convertible
	*/
	template <class TARGET>
	TARGET convertType(const char *v) {
		return VCellConvert<TARGET>( )(v); 
	}

	/**
	* intentional convert types with possible loss of data / truncation
	* i.e. static_cast
	* @tparam A one type 
	* @tparam B other type 
	*/
	template <typename A, typename B>
	struct LossyConvert {
		B operator( )(A s) {
			return static_cast<B>(s);
		}
		A operator( )(B s) {
			return static_cast<A>(s);
		}
	};

}
#endif
