#include "SimpleSymbolTable.h"

SimpleSymbolTable::SimpleSymbolTable(string* symbols, int symbolCount, ValueProxy** valueProxies)
{
	for (int i = 0; i < symbolCount; i ++){
		steArray.push_back(new SimpleSymbolTableEntry(symbols[i],i,0, valueProxies == 0 ? 0 : valueProxies[i]));
	}
}

SimpleSymbolTable::~SimpleSymbolTable(void)
{
	for (unsigned int i = 0; i < steArray.size(); i ++) {
		delete steArray[i];
	}
	steArray.clear();
}


SymbolTableEntry* SimpleSymbolTable::getLocalEntry(const string & identifier) const
{
	for (unsigned int i = 0; i < steArray.size(); i++){
		if (steArray[i]->getName() == identifier){
			return steArray[i];
		}
	}
	return NULL;
}
