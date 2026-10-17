#include <map>
#include <string>
#include <stdint.h>
#include "mex.h"
namespace matlabLink {
	template <class T>
	struct MData {
		MData( )
			:size(0),
			data(0) {}
		MData(size_t size_, T *array)
			:size(size_),
			data(array) {}
		size_t size;
		T *data;
		T & operator[](size_t i) {
			return data[i];
		}
	};

	struct MatlabStruct {
		MatlabStruct(const mxArray *source);
		MData<double> doubles(const char * const name, bool required=true) const;

		std::string str(const char *const name, bool required = true) const;
		std::string str(const std::string &n, bool required = true) const {
			return str(n.c_str( ), required);
		}
		/**
		* fetch string, return default value if not present in structure
		*/
		std::string str(const char *const name, const char * const defaultValue) const;
		std::string str(const std::string & n, const char * const defaultValue) const {
			return str(n.c_str( ), defaultValue);
		}
		bool boolean(const char *const name, bool required = true) const;
		MData<int64_t> int64s(const char * const name, bool required=true) const;

		template <class T>
		T & fetch(const char * const name) const {
			void * vptr = getRequired(name);
			T *ptr = static_cast<T *>(vptr);
			return dynamic_cast<T &>(*ptr);
		}

		struct ArrayData {
			size_t size;
			const mxArray *marray;
			ArrayData( )
				:size(0),
				marray(0) {}
		};
	protected:
		typedef bool (*validateFunction)(const mxArray *);

		ArrayData get(const char * const name, bool required, const char *type, validateFunction) const; 

		void *getRequired(const char * const name) const;

		const mxArray &source;

		typedef std::map<std::string,MData<double> > DoubleMap; 
		mutable DoubleMap dblStore;

		typedef std::map<std::string,std::string> StringMap; 
		mutable StringMap strStore;

		typedef std::map<std::string,MData<int64_t> > Int64Map; 
		mutable Int64Map int64Store; 

	};
}