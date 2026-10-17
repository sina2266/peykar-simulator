#include "Expression.h"
#include "Exception.h"

#include <iostream>
#include <vector>

int main() {
	std::vector<std::string> expStrs;
	expStrs.push_back(std::string("log(0)"));
	expStrs.push_back(std::string("(1<(2*(1<2)))*5+(1<(2*(2<1)))*5"));
	expStrs.push_back(std::string("(1<2)*3+5*log(0)*(0>0)+2"));
	expStrs.push_back(std::string("(1<2)*1*2+3*5+4+(1<1)"));
	expStrs.push_back(std::string("(1<3)*(1<log(0))*1*2*4"));
	expStrs.push_back(std::string("(0<1)*log(0)"));
	expStrs.push_back(std::string("(1<1)*log(0)"));
	double* values = new double[1];
	values[0] = 0.0;
	for (int i = 0; i < (int)expStrs.size(); ++i) {
		try {
			VCell::Expression expression(expStrs[i]);
			std::cout << "compiling " << expression.infix() << std::endl;
			std::cout << "instructions: " << std::endl;
			expression.showStackInstructions();
			std::cout << "stack machine (constant) ==> " << expression.evaluateConstant() << std::endl;
			std::cout << "tree eval     (constant) ==> " << expression.evaluateConstantTree() << std::endl;
			std::cout << "stack machine (vector)   ==> " << expression.evaluateVector(values) << std::endl;
			std::cout << "tree eval     (vector)   ==> " << expression.evaluateVectorTree(values) << std::endl;
		} catch (const char* ex) {
			std::cerr << "ExpressionParserTest failed : " << ex << std::endl;
		} catch (std::string& ex) {
			std::cerr << "ExpressionParserTest failed : " << ex << std::endl;
		} catch (VCell::Exception& ex) {
			std::cerr << "ExpressionParserTest failed : " << ex.getMessage() << std::endl;
		} catch (...) {
			std::cerr << "ExpressionParserTest failed : unknown error." << std::endl;
		}
	}
}
