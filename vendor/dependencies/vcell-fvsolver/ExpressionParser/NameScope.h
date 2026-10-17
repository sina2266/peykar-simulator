#ifndef NAMESCOPE_H
#define NAMESCOPE_H

#include "stdinc.h"

class NameScope {
public:
	NameScope** getChildren()=0;
	SymbolTableEntry getExternalEntry(string identifier)=0;;
	String getName();
	NameScope* getNameScopeFromPrefix(string prefix)=0;;
	NameScope* getParent()=0;;
	string getRelativeScopePrefix(NameScope referenceNameScope)=0;;
	ScopedSymbolTable getScopedSymbolTable()=0;;
	string getSymbolName(SymbolTableEntry* symbolTableEntry)=0;;
	string getUnboundSymbolName(String unboundName)=0;;
	bool isAncestor(NameScope nameScope)=0;;
	bool isPeer(NameScope nameScope)=0;;
}

#endif
