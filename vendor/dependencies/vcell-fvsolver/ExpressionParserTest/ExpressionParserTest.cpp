#include "Expression.h"
#include "Exception.h"
using namespace VCell;
#include <stdio.h>
#include <iostream>
using std::cout;
using std::endl;
using std::cerr;
int main(int argc, char *argv[]) {
	
	vector<string> expStrs;
	expStrs.push_back(string("log(0)"));
	expStrs.push_back(string("(1<(2*(1<2)))*5+(1<(2*(2<1)))*5"));
	expStrs.push_back(string("(1<2)*3+5*log(0)*(0>0)+2"));
	expStrs.push_back(string("(1<2)*1*2+3*5+4+(1<1)"));
	expStrs.push_back(string("(1<3)*(1<log(0))*1*2*4"));
	expStrs.push_back(string("(0<1)*log(0)"));
	expStrs.push_back(string("(1<1)*log(0)"));
	double* values = new double[1];
	values[0] = 0.0;
	for (int i = 0; i < (int)expStrs.size(); ++i) {
		try {
			Expression expression(expStrs[i]);
			cout << "compiling " << expression.infix() << endl;
			cout << "instructions: " << endl;
			expression.showStackInstructions();
			cout << "stack machine (constant) ==> " << expression.evaluateConstant() << endl;
			cout << "tree eval     (constant) ==> " << expression.evaluateConstantTree() << endl;
			cout << "stack machine (vector)   ==> " << expression.evaluateVector(values) << endl;
			cout << "tree eval     (vector)   ==> " << expression.evaluateVectorTree(values) << endl;
		} catch (const char* ex) {
			cerr << "ExpressionParserTest failed : " << ex << endl;
		} catch (string& ex) {
			cerr << "ExpressionParserTest failed : " << ex << endl;
		} catch (Exception& ex) {
			cerr << "ExpressionParserTest failed : " << ex.getMessage() << endl;
		} catch (...) {
			cerr << "ExpressionParserTest failed : unknown error." << endl;
		}
	}
}
