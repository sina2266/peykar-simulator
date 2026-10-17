#ifndef	SIMPLESYMBOLTABLEENTRY_H
#define SIMPLESYMBOLTABLEENTRY_H

#include "SymbolTableEntry.h"

class SimpleSymbolTableEntry : public SymbolTableEntry
{
public:
	SimpleSymbolTableEntry(const std::string& nameValue, int indexVal, NameScope* namescopeVal, ValueProxy* proxyVal);
	~SimpleSymbolTableEntry(void);
	double getConstantValue();
	VCell::Expression* getExpression();
	int getIndex();
	std::string& getName();
	NameScope* getNameScope();
	//VCUnitDefinition getUnitDefinition()=0;
	bool isConstant();	
	void setIndex(int symbolTableIndex);
	void setConstantValue(double v);
	ValueProxy* getValueProxy() { return valueProxy; };

private:
	std::string name;
	int index;	
	NameScope* namescope;
	bool bConstant;
	double value;
	ValueProxy* valueProxy;
};
#endif
