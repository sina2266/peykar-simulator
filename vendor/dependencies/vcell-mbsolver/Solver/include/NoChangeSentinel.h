#ifndef NoChangeSentinel_h
#define NoChangeSentinel_h
#include <iostream>
#include <string>
namespace vcell_util {
#ifndef NDEBUG

	/**
	* stack based debugging class to monitor values for change and display message if they do
	* @tparam T type to monitor
	* @tparam ID optional information type; must support copy by value and << operator
	*/
	template<typename T, typename ID = void>
	struct NoChangeSentinel {
		NoChangeSentinel(const std::string & label,const ID & id_, const T & v) 
			:name(label),
			id(id_),
			variable(v),
			value(v) {}
		~NoChangeSentinel( ) {
			if (variable != value) {
				const T diff = value - variable; 
				std::cout << name << ' ' << id << " changed from " << value << " to " << variable << ", difference of " << diff << std::endl;
			}
		}
	private:
		const std::string name;
		const ID id;
		const T & variable; 
		const T  value; 
	};

	/**
	* specialization for no ID type
	*/
	template<typename T>
	struct NoChangeSentinel<T,void> {
		NoChangeSentinel(const std::string & label,const T & v) 
			:name(label),
			variable(v),
			value(v) {}
		~NoChangeSentinel( ) {
			if (variable != value) {
				const T diff = value - variable; 
				std::cout << name << " changed from " << value << " to " << variable << ", difference of " << diff << std::endl;
			}
		}
	private:
		const std::string name;
		const T & variable; 
		const T  value; 
	};

	/**
	* create NoChangeSentinel with ID. Use function because compiler infers function template arguments but not class ones
	* @param label displayed if change
	* @param id streamd if change 
	* @param v value to monitor
	*/
	template<typename T, typename ID>
	NoChangeSentinel<T,ID> makeSentinel(const std::string & label,const ID & id, const T & v) {
		return NoChangeSentinel<T,ID>(label,id,v);
	}

	/**
	* create NoChangeSentinel without ID. Use function because compiler infers function template arguments but not class ones
	* @param label displayed if change
	* @param v value to monitor
	*/
	template<typename T>
	NoChangeSentinel<T> makeSentinel(const std::string & label,const T & v) {
		return NoChangeSentinel<T>(label,v);
	}
#else
	template<typename T, typename ID>
	char makeSentinel(const std::string & label,const ID & id, const T & v) {
		return 0; 
	}
	template<typename T>
	char makeSentinel(const std::string & label,const T & v) {
		return 0; 
	}
#endif
}
#endif
