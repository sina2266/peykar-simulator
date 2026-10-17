#ifndef VCDictionary_h
#define VCDictionary_h
#include <map>
#include <VCellException.h>
#include <stdexcept>
namespace vcell_util {
	/**
	* a specialization of string based dictionaries.
	*/
	template <typename T>
	struct Dictionary : public std::map<std::string,T> {
		typedef typename std::map<std::string,T> base;

		/**
		* @param n identifying name
		*/
		Dictionary(const char *n)
			:name_(n) {}

		/**
		* return set of available options
		* @param includeName include dictionary name in output?
		*/
		std::string options(bool includeName = false) const {
			std::ostringstream scratch;
			if (includeName) {
				scratch << name_ << ":  ";
			}
			bool first = true;
			for (auto e = this->begin(); e != this->end(); ++ e) {
				if (!first) {
					scratch << ", "; 
				}
				first = false;
				scratch << (*e).first;
			}
			return scratch.str( );
		}

		/**
		* return specified value
		* @param key to lookup
		* @throws std::domain_error if key not present
		*/
		T get(const std::string &key) const {
			Dictionary &self = const_cast<Dictionary &>(*this);
			if (self.find(key) == self.end( )) {
				VCELL_EXCEPTION_NOLOCATION(domain_error,"Invalid option " << key << ", value must be one of (" << options( ) << ')');
			}
			return base::at(key);
		}

		/**
		* @return name passed to constructor
		*/
		const std::string & name( ) const {
			return name_;
		}

	private:
		const std::string name_;
	};

}
#endif
