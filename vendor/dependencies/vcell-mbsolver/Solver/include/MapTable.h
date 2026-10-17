#ifndef MapTable_h
#define MapTable_h
#include <typeinfo>
#include <map>
#include <list>
#include <SymbolTable.h>
#include <SimpleSymbolTableEntry.h>
namespace VCell {
	struct MTExpression;

	/**
	* map backed symbol table 
	*/
	struct MapTable  : private SymbolTable {
		/**
		* @param exp string representation of function
		*/
		MapTable() 
			:dirty(true),
			symbols( ),
			values( ),
			clients( )
		{ 
		}
		virtual ~MapTable( );

		/**
		* access previous set value
		* @throws std::domain_error if value not set
		*/
		double operator[](std::string name) const;

		/**
		* access / sets symbol value
		*/
		double & operator[](std::string name);


	private:
		
		/**
		* @Override SymbolTable function
		*/
		virtual SymbolTableEntry* getEntry(const std::string & name) const;
		void buildTable( ) const;
		 

		friend MTExpression;
		bool dirty;
		mutable std::map<std::string,SimpleSymbolTableEntry *> symbols;
		std::map<std::string,double> values;
		std::list<MTExpression *> clients;
	};

}
#endif
