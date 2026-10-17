#ifndef SYMBOLTABLEENTRY_H
#define SYMBOLTABLEENTRY_H

#include <string>
using std::string;

class ValueProxy;
namespace VCell {
class Expression;
}
class NameScope;

class SymbolTableEntry {
public:
	virtual double getConstantValue()=0;
	virtual VCell::Expression* getExpression()=0;
	virtual int getIndex()=0;
	virtual string& getName()=0;    
	virtual NameScope* getNameScope()=0;
	//VCUnitDefinition getUnitDefinition()=0;
	virtual bool isConstant()=0;
	virtual ValueProxy* getValueProxy()=0;
};

#endif
