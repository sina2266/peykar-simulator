#include <math.h>
#include <cstring>

#include "MathUtil.h"
#include "Exception.h"
#include "ExpressionTest.h"
#include "Expression.h"
#include "SimpleSymbolTable.h"
#include "RuntimeException.h"
#include "IOException.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>


ExpressionTest::ExpressionTest(void)
{
}

ExpressionTest::~ExpressionTest(void)
{
}

void ExpressionTest::testEvaluateVector(void)
{
	try {
		double d = 0.0;
		std::cout << "Parser:  evaluating vector" << std::endl;
		Expression* exp = new Expression("a+b/c");
		std::string ss[] = { "a", "b", "c" };
		SimpleSymbolTable* simpleSymbolTable = new SimpleSymbolTable(ss, 3);

		const int n = 3;
		double v[][n] = {{ 0,1,2 },{ 3,4,5 }, {6,7,8}};

		exp->bindExpression(simpleSymbolTable);

		for (int i = 0; i < n; i ++) {
			d = exp->evaluateVector(v[i]);
			std::cout << i << "." << exp->infix() << " = " << d << std::endl;
		}
		delete exp;
		delete simpleSymbolTable;
	} catch (Exception& ex) {
		Exception::rethrowException(ex);
	} catch (...) {
		throw RuntimeException("Unknown excpetion.");
	}
}

void ExpressionTest::testEvaluateConstant(void)
{
	try {
		double d = 0.0;
		std::cout << "Parser:  evaluating constant" << std::endl;
		Expression* exp = new Expression("(3/5");
		d = exp->evaluateConstant();
		std::cout << exp->infix() << " = " << d << std::endl;
		delete exp;
	} catch (Exception& ex) {
		Exception::rethrowException(ex);
	} catch (...) {
		throw RuntimeException("Unknown excpetion.");
	}
}

void ExpressionTest::testParser(int count, char* javaresult, double cvalue, char* expStr, SymbolTable* symbolTable, double* values) {
	Expression* exp = 0;
	std::string badmsg;
	try {
		double javavalue = -0.0;
		double d = 0.0;
		double dtree = 0.0;
		int n = -1;
		if (strcmp(javaresult, "NaN") == 0) {
			javavalue = log(-1.0);
		} else if (strcmp(javaresult, "Infinity") == 0) {
			javavalue = MathUtil::double_infinity;
		} else if (strcmp(javaresult, "-Infinity") == 0) {
			javavalue = -MathUtil::double_infinity;
		} else {
			n = sscanf(javaresult, "%lg", &javavalue);
			if (n != 1) {
				std::cout << " Not a Number:: " << javaresult ;
				return;
			}				
		}
		bool bException = false;
		std::string before = Expression::trim(std::string(expStr));
		exp = new Expression(expStr);	
		std::string exceptionMsg = "";
		try {
			exp->bindExpression(symbolTable);
			d = exp->evaluateVector(values);
			dtree = exp->evaluateVectorTree(values);
		} catch (Exception& ex) {
			bException = true;
			exceptionMsg = ex.getMessage();
		}
		if (d != d && javavalue != javavalue && cvalue != cvalue) {
			//cout << count << " EVAL_YES :: all NaN" << endl;
		} else if (javavalue == d && d == cvalue && d == dtree 
			|| fabs(javavalue - d) < 1E-14 && fabs(cvalue - d) < 1E-14 
			|| fabs(javavalue - d)/std::max(fabs(d),fabs(javavalue)) < 1E-14 && fabs(cvalue - d)/std::max(fabs(d),fabs(cvalue)) < 1E-14 ) {
			//cout << count << " EVAL_YES " << endl;
		} else {			
			if (bException) {
				//cout << endl << "-------------------------------------------" << endl;
				//char chrs[2560];					
				//sprintf(chrs, "%d. C++ throws exception: %s", count, exceptionMsg.data());
				//cout << chrs << " EVAL_NO "  ;
				//printf("Java/Infix_C/C++: %.20g/%.20g/Exception\n", count, cvalue, javavalue);
				//cout << expStr << endl;
			} else {
				std::cout << std::endl << "-------------------------------------------" << std::endl;
				printf("%d.  EVAL_NO Java/Infix_C/C++ Tree/C++: %.20lg/%.20lg/%.20lg/%.20lg\n", count, javavalue, cvalue, dtree, d);	
				printf("Java~Infix_C/Java~C++/Infix_C~C++: %.20lg/%.20lg/%.20lg/%.20lg\n", fabs((javavalue-cvalue)/javavalue), fabs((javavalue-d)/javavalue), fabs((dtree-cvalue)/cvalue), fabs((d-cvalue)/cvalue));	
				std::cout << expStr << std::endl;
			}	
		}
		
		delete exp;
	} catch (Exception& ex) {
		std::cout << ex.getMessage() << std::endl;
	}
}

void ExpressionTest::testParser(char* filename)
{
	Expression* exp = 0;
	/*	
	exp = new Expression("sqrt(( - (1 * 0.9097624582911913 * 5) * ((0.1156280095277954 * 0.9004846229454024 * 0.5705921628352123) || (0.2422790712041183 + 0.94879464626319 + 0.749322637829414)) * (0.8524685675688051 / 1 * (0.21046361766770427 * 0.1362470304410489 * 0.044029610742542746))))");
	double d = exp->evaluateConstant();
	printf("%s=%.20lg\n", exp->infix().c_str(), d);	
	*/

	std::string ids[] = {"id_0", "id_1", "id_2", "id_3",
		"id_4", "id_5", "id_6", "id_7", "id_8", "id_9"};
	SimpleSymbolTable* symbolTable = new SimpleSymbolTable(ids, 10); 

	const int m = 2;
	const int n = 10;
	double v[m][n] = {{0,1,2,3,4,5,6,7,8,9 },{ 1,2,3,4,5,6,7,8,9,10}};

	std::ifstream ifs(filename);
	if (!ifs.is_open()) {
		throw IOException(std::string("") + "Can't open file '" + filename);
	}
	int count = 0;
	std::vector<std::string> badmsg;
	bool bInfinity = false;
	bool bNAN = false;
	bool bException = false;

	char line[1000];
	while(!ifs.eof()) {
		try {
			double value = -0.0;
			double d = 0.0;
			count ++;
			bInfinity = false;
			bNAN = false;
			std::cout << count << "....";
			memset(line, 0, 1000*sizeof(char));
			ifs >> line;
			int n = -1;
			if (strcmp(line, "NaN") == 0) {
				bNAN = true;
			} else if (strcmp(line, "Infinity") == 0 || strcmp(line, "-Infinity") == 0) {
				bInfinity = true;
			} else {
				n = sscanf(line, "%lg", &value);
				if (n != 1) {
					std::cout << " Not a Number:: " << line ;
				}				
			}
			memset(line, 0, 1000*sizeof(char));
			ifs.getline(line, 1000);		
			if (n != 1 && !bInfinity && !bNAN) {
				std::cout << line;
				goto label_1;
			}
			bException = false;
			std::string before = Expression::trim(std::string(line));
			exp = new Expression(line);	
			std::string exceptionMsg = "";
			try {
				exp->bindExpression(symbolTable);
				d = exp->evaluateVector(v[0]);			
			} catch (Exception& ex) {
				bException = true;
				exceptionMsg = ex.getMessage();
			}
			if ((MathUtil::double_infinity == d || MathUtil::double_infinity == -d) && bInfinity) {
				std::cout << " INFINITY:::::::::::::Before/After: Infinity/" << d;
			} else if (d != d && bNAN) {
				std::cout << " NaN:::::::::::::Before/After: NaN/" << d;
			} else if (value == d || fabs(value - d) < 1E-14 || fabs(value - d)/std::max(fabs(d),fabs(value)) < 1E-14) {
				std::cout << " EVAL_YES ";
			} else {
				std::string afterparsing = exp->infix();
				if (before == afterparsing) {
					std::cout << "INFIX_YES " << std::endl;
				} else {
					std::cout << "INFIX_NO " << std::endl;
				}							
				std::cout << "Before: " << before << std::endl;
				std::cout << "After : " << exp->infix() << std::endl;
				
				if (bException) {
					char chrs[2560];					
					sprintf(chrs, "%d. C++ throws exception: %s", count, exceptionMsg.c_str());
					badmsg.push_back(chrs);
					std::cout << chrs << " EVAL_NO ";
					printf("Before/After: %.20g/Exception ", value);	
				} else if (bInfinity) {
					char chrs[256];					
					sprintf(chrs, "%d. Java evaluation is Infinity", count);
					badmsg.push_back(chrs);
					std::cout << chrs << " EVAL_NO ";
					printf("Before/After: Infinity/%.20lg ", d);	
				} else if (bNAN) {
					char chrs[256];					
					sprintf(chrs, "%d. Java evaluation is NAN", count);
					badmsg.push_back(chrs);
					std::cout << chrs << " EVAL_NO ";
					printf("Before/After: NAN/%.20lg ", d);	
				} else {
					double m = std::max(fabs(d),fabs(value));
					if (m < 1e-100) {
						char chrs[256];					
						sprintf(chrs, "%d. abs(error)=%lg", count, fabs(d-value));
						badmsg.push_back(chrs);
						std::cout << chrs << " EVAL_NO ";
					} else {
						char chrs[256];					
						sprintf(chrs, "%d. relative(error)=%lg", count, fabs((d-value)/m));
						badmsg.push_back(chrs);
						std::cout << chrs << " EVAL_NO ";
					}				
					printf("Before/After: %.20lg/%.20lg ", value, d);	
				}	
			}
			
			delete exp;
		} catch (Exception& ex) {
			std::cout << ex.getMessage() << std::endl;
		}
label_1:
		std::cout << std::endl << "-------------------------------------------" << std::endl;
	}
	std::cout << "BAD " << badmsg.size() << std::endl;
	for (int i = 0; i < badmsg.size(); i ++) {
		std::cout << badmsg[i] << std::endl;
	}
}
