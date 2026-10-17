#ifndef	SIMPLESYMBOLTABLEENTRY_H
#define SIMPLESYMBOLTABLEENTRY_H

#include "SymbolTableEntry.h"

class SimpleSymbolTableEntry : public SymbolTableEntry
{
public:
	SimpleSymbolTableEntry(const string& nameValue, int indexVal, NameScope* namescopeVal, ValueProxy* proxyVal);
	~SimpleSymbolTableEntry(void);
	double getConstantValue();
	VCell::Expression* getExpression();
	int getIndex();
	string& getName();    
	NameScope* getNameScope();
	//VCUnitDefinition getUnitDefinition()=0;
	bool isConstant();	
	void setIndex(int symbolTableIndex);
	void setConstantValue(double v);
	ValueProxy* getValueProxy() { return valueProxy; };

private:
	string name;	
	int index;	
	NameScope* namescope;
	bool bConstant;
	double value;
	ValueProxy* valueProxy;
};
#endif
