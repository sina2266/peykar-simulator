#include <algorithm>
#include <MTExpression.h>
#include <MapTable.h>
using VCell::MTExpression;
using VCell::MapTable;
MTExpression::MTExpression(const char * const exp, MapTable & mt) 
	:expression(exp),
	mapTable(mt) {
		mapTable.clients.push_back(this);
}

MTExpression::MTExpression(const std::string & exp, MapTable & mt) 
	:expression(exp),
	mapTable(mt) {
		mapTable.clients.push_back(this);
}

void MTExpression::bind(const MapTable &mt) {
	expression.bindExpression(const_cast<MapTable *>(&mt));
}

/**
* evaluate expression
* @throws VCell::ExpressionException if all symbols not set 
*/
double MTExpression::evaluate( ) const {
	mapTable.buildTable( );
	return expression.evaluateConstantTree( );
}
