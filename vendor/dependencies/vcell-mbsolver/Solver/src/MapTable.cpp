#include <stdexcept>
#include <algorithm>
#include <MapTable.h>
#include <MTExpression.h>
using VCell::MapTable;
using VCell::MTExpression;

double MapTable::operator[](std::string name) const {
	auto iter = values.find(name);
	if (iter == values.end( )) {
		throw std::domain_error("no such name");
	}
	return iter->second;
}

double & MapTable::operator[](std::string name) {
	dirty = true;
	return values[name];
}

/**
* evaluate expression
* @throws VCell::ExpressionException if all symbols not set 
*/
void MapTable::buildTable( ) const {
	if (!dirty) {
		return;
	}
	for (auto iter = values.begin( ); iter != values.end( ); ++iter) {
		const std::string & name = iter->first;
		auto  sste = symbols.find(name);

		SimpleSymbolTableEntry *pTE;
		if (sste == symbols.end()) { 
			int n = static_cast<int>(symbols.size( ));
			symbols[name] = pTE = new SimpleSymbolTableEntry(name,n,nullptr,nullptr);
		}
		else {
			pTE = sste->second;
		}
		pTE->setConstantValue(iter->second);
	}
	for (auto client : clients) {
		client->bind(*this);
	}
}

SymbolTableEntry* MapTable::getEntry(const std::string & name) const {
	return symbols[name];
}

MapTable::~MapTable( ) {
	for (auto  s: symbols) {
		delete s.second;
	}
}
