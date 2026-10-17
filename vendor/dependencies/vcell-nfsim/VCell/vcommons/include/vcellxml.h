#ifndef vcellxml_h
#define vcellxml_h
#include <iostream>
#include <stdexcept>
#include <VCellException.h>
#include <tinyxml2.h>
#include <vcellconvert.h>
namespace vcell_xml {
	using vcell_util::convertType;

	/**
	* convenience method to extract text from specified XML element and convert to specified type
	* @tparam T type to convert to
	* @param element  
	* @throw std::domain_error if element not present or not parseable as specified type
	*/
	template <class T>
	T convertElement(const tinyxml2::XMLElement &element) {
		try {
			const tinyxml2::XMLNode *n = element.FirstChild( );
			if (n) {
				const tinyxml2::XMLText * t = n->ToText( ); 
				if (t) {
					const char * v = t->Value( );
					T val = convertType<T>(v); 
					return val;
				}
			}
			VCELL_EXCEPTION(domain_error,"Can't convert XML element '" << element.Name( )  << "' to correct type, text not found");
		} catch (std::exception &e) {
			VCELL_EXCEPTION(domain_error,"Can't convert XML element '" << element.Name( ) << "' to correct type, exception " << e.what( ));
		}
	}
	/**
	* convenience method to extract text from specified XML element child node and convert to specified type
	* @tparam T type to convert to
	* @param node place to extract from
	* @param name element name to get
	* @throw st::domain_error if element not present or not parseable as specified type
	*/
	template <class T>
	T convertChildElement(const tinyxml2::XMLElement &node, const char * name) {
		const tinyxml2::XMLElement *e = node.FirstChildElement(name);
		if (!e) {
			VCELL_EXCEPTION(invalid_argument,"no element " <<name);
		}
		return convertElement<T>(*e);
	}
	/**
	* convenience method to extract optional text from given XML element and convert to specified type
	* @tparam T type to convert to
	* @param node place to extract from
	* @param name element name to get
	* @throw std::domain_error if element present but not parseable as specified type
	* @return std::pair<bool (success>, T> 
	*/
	template <class T>
	std::pair<bool,T> queryElement(const tinyxml2::XMLElement &node, const char * name) {
		try {
			const tinyxml2::XMLElement *e = node.FirstChildElement(name);
			if (!e) {
				return std::pair<bool,T>(false,T( ));
			}
			const tinyxml2::XMLNode *n = e->FirstChild( );
			if (n) {
				const tinyxml2::XMLText * t = n->ToText( ); 
				if (t) {
					const char * v = t->Value( );
					T val = convertType<T>(v); 
					return std::pair<bool,T>(true,val);
				}
			}
			VCELL_EXCEPTION(domain_error,"Can't convert XML element '" << name << "' to correct type, text not found");
		} catch (std::exception &e) {
			VCELL_EXCEPTION(domain_error,"Can't convert XML element '" << name << "' to correct type, exception " << e.what( ));
		}
	}
	/**
	* convenience method to extract text from given XML element and convert to specified type if present or return default value
	* @tparam T type to convert to
	* @param node place to extract from
	* @param name element name to get
	* @param defaultValue what you expect 
	* @throw std::domain_error if element present but not parseable as specified type
	* @return value from XML or default if not present in XML
	*/
	template <class T>
	const T convertChildElementWithDefault(const tinyxml2::XMLElement &node, const char * name, const T & defaultValue) {
		std::pair<bool,T> q = queryElement<T>(node,name);
		if (q.first) {
			return q.second;
		}
		return defaultValue;
	}

	/**
	* get element
	* @param node place to get name from
	* @param name tag name to get
	* @throws invalid_argument if element not present
	*/
	inline const tinyxml2::XMLElement & get(const tinyxml2::XMLElement &node,const char *name) {
		const tinyxml2::XMLElement *r = node.FirstChildElement(name);
		if (r) {
			return *r;
		}
		VCELL_EXCEPTION(invalid_argument,"missing XML element " << name);
	}

	/**
	* get element if present
	* (adds no functionality to tinyxml2 but is used for API consistency with #get 
	* @param node place to get name from
	* @param name tag name to get
	* @returns null if not present
	*/
	inline const tinyxml2::XMLElement * query(const tinyxml2::XMLElement &node,const char *name) {
		return node.FirstChildElement(name);
	}
}
namespace tinyxml2 {
	std::ostream & operator<<(std::ostream &os, XMLError);  
}
#endif
