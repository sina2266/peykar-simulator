#ifndef SIMPLESYMBOLTABLE_H
#define SIMPLESYMBOLTABLE_H

#include <vector>
using std::vector;

#include "SymbolTable.h"
#include "SimpleSymbolTableEntry.h"

class SimpleSymbolTable : public SymbolTable
{
public:
	SimpleSymbolTable(string* symbols, int symbolCount, ValueProxy** valueProxies=0);
	/**
	* non-standard copy constructor -- transfers ownership of
	* implementation to new object; rhs will be unusable
	* @param rhs object to move data from
	*/
	SimpleSymbolTable(const SimpleSymbolTable &rhs)
		:steArray(rhs.steArray) {
			SimpleSymbolTable &scr = const_cast<SimpleSymbolTable &>(rhs);
			scr.steArray.clear( );
	}

	//SimpleSymbolTable(string* symbols, NameScope* namescopeVal);
	~SimpleSymbolTable(void);
	/**
	* same as #getLocalEntry
	*/
	SymbolTableEntry* getEntry(const std::string & identifier) const {
		return getLocalEntry(identifier);
	}
	SymbolTableEntry* getLocalEntry(const std::string & identifier) const; 
	/**
	* return number of symbols in table
	*/
	size_t size( ) const {
		return steArray.size( );
	}

private:
	SimpleSymbolTable & operator=(const SimpleSymbolTable &);
	vector<SimpleSymbolTableEntry*> steArray;
};

#endif
