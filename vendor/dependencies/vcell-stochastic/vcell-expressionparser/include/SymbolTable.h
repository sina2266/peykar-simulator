#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>


class SymbolTableEntry;

class SymbolTable {
public:
	virtual SymbolTableEntry* getEntry(const std::string & identifier) const = 0; 
	virtual ~SymbolTable() {}
};

#endif
