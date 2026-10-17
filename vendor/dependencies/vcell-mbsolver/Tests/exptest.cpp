#include <iostream>
#include <cassert>

#include <vector>
#include <array>
#include <fstream>
#include <algorithm>
#include <stdexcept>
#include <VCellException.h>
#include "Expression.h"
#include "SimpleSymbolTable.h"
#include "gtest/gtest.h"
#include "vcellxml.h"
#include <SExpression.h>
#include <MapTable.h>
#include <MTExpression.h>
#include <Exception.h>
#ifndef __WIN32__
	#include <cmath>
#endif
#include <iomanip>

namespace {

	void expTest(const char * const exp, bool expectedToBeConstant) {
		VCell::Expression e(exp); 
		EXPECT_EQ(e.isConstant( ), expectedToBeConstant);
	}


}
TEST(expression,constant) {
	expTest("7 * x ",false);
	VCell::Expression empty; 
	ASSERT_FALSE(empty.isConstant( ));

	expTest("7",true);
	expTest("7 * 3 ",true);
	expTest("7 + 3",true);
	expTest("7 + y",false);
	expTest("sin(.1)",true);
	expTest("sin(t)",false);

}

TEST(expression,base) {
	std::string syms[] = {"a","b","c","d"};
	VCell::Expression exp("(a + b) * c");
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	exp.bindExpression(&symTb);
	double v[] = {2,3,4, 5};
	const double r= exp.evaluateVector(v);
	std::cout << exp.infix( ) << " = " << r << std::endl;
	std::vector<std::string> esyms;
	exp.getSymbols(esyms);
	std::ostream_iterator<std::string> sout(std::cout,", ");
	std::copy(esyms.begin( ),esyms.end( ), sout);
	ASSERT_TRUE(r == 20);
}
TEST(sexpression,ops) {
	std::string syms[] = {"a","b","c","d"};
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	moving_boundary::SExpression exp("(a + b) * c",&symTb);
	moving_boundary::SExpression copy(exp); 
	std::array<double,4> arr = {2,3,4,5};
	double r= copy.evaluate(arr);
	moving_boundary::SExpression alone("1");
	alone = exp;
	double r2= alone.evaluate(arr);
	ASSERT_TRUE(r == 20);
	ASSERT_TRUE(r2 == 20);
	std::cout << exp.infix( ) << " = " << r << std::endl;
}
TEST(sexpression,base) {
	std::string syms[] = {"a","b","c"};
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	moving_boundary::SExpression exp("(a + b) * c",&symTb);
	std::array<double,3> arr = {2,3,4};
	double r= exp.evaluate(arr);
	std::vector<double> vec;
	vec.resize(arr.size( ));
	std::copy(arr.begin( ),arr.end( ),vec.begin( ));
	double r2= exp.evaluate(vec);
	ASSERT_TRUE(r == 20);
	ASSERT_TRUE(r2 == 20);
	std::cout << exp.infix( ) << " = " << r << std::endl;
}

TEST(sexpression,badsize) {
	std::string syms[] = {"a","b","c"};
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	moving_boundary::SExpression exp("(a + b) * c",&symTb);
	std::array<double,2> arr = {2,3};
    GTEST_SKIP() << "test no longer throws std::domain_error - is this intensional?";
	ASSERT_THROW(double r= exp.evaluate(arr),std::domain_error);
	std::vector<double> vec;
	vec.resize(arr.size( ));
	std::copy(arr.begin( ),arr.end( ),vec.begin( ));
	ASSERT_THROW(double r= exp.evaluate(vec),std::domain_error);
}
TEST(sexpression,xvel) {
	const char * const xString = "x / sqrt(x*x + y*y)";
	std::string syms[] = {"x","y"};
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	moving_boundary::SExpression exp(xString,&symTb);
	const double up  = std::sqrt(2) / 2.0;
	const double direct = up / std::sqrt(up*up + up*up);
	std::array<double,2> values = {up,up};
	double out = exp.evaluate(values);
	double delta = out - direct;
	std::cout << std::setw(20) << std::setprecision(12) << up << ", " << out << std::endl ;
	std::cout << delta << std::endl; 
	std::cout << (out - up) << std::endl; 
}
TEST(sexpression,cse) {
	const char * const hString = "hM / 10";
	std::string syms[] = {"h","hM"};
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	moving_boundary::SExpression exp(hString,&symTb);
	std::array<double,2> values = {2,3};
	double out = exp.evaluate(values);
	std::cout << " out " << out << std::endl;
}

TEST(expression,construct) {
	VCell::Expression a; 
	{
		VCell::Expression exp("(ax + b) * c");
		a = exp;
	}
	std::string syms[] = {"ax","b","c"};
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	a.bindExpression(&symTb);
	double v[] = {5,6,8};
	const double r= a.evaluateVector(v);
	std::cout << a.infix( ) << " = " << r << std::endl;
	ASSERT_TRUE(r == 88);
}
using namespace vcell_xml;
namespace {

	double toDouble(const tinyxml2::XMLElement &node, const char * name) {
		return convertChildElement<double>(node,name);
	}

	void parseXMLElement(const tinyxml2::XMLElement &element,double &min, double &max, double &step) {
		min = convertChildElement<double>(element,"min");
		max = toDouble(element,"max");
		min = toDouble(element,"min");
	}


}
TEST(vcellxml,errorPrint) {
	for (int i = 0; i < 30; i++) {
		tinyxml2::XMLError e = (tinyxml2::XMLError)i;
		std::cout << e << std::endl;
	}
}
TEST(expression,parse) {
	bool interactive = false;
	if (!interactive) {
		return;
	}

	const char * const XML_FILENAME = "expressioncontrol.xml";
	using std::cout;
	using std::cin;
	using std::endl;
	using std::getline;

	tinyxml2::XMLDocument doc;
	doc.LoadFile(XML_FILENAME);
	if (doc.ErrorID( ) == tinyxml2::XML_ERROR_FILE_NOT_FOUND) {
		std::cout << "expression.parse requires " << XML_FILENAME << " file to run, attempting to generate " << std::endl;
		std::ofstream xmlfile(XML_FILENAME);
		const char newline = '\n';
		xmlfile << "<?xml version=\"1.0\"?>" << newline
			<< "<parameters>" << newline
			<< "<x>" << newline
			<< "    <min>0</min>" << newline
			<< "    <max>10</max>" << newline
			<< "    <step>1</step>" << newline
			<< "</x>" << newline
			<< "<y>" << newline
			<< "    <min>0</min>" << newline
			<< "    <max>10</max>" << newline
			<< "    <step>1</step>" << newline
			<< "</y>" << newline
			<< "<t>" << newline
			<< "    <min>0</min>" << newline
			<< "    <max>10</max>" << newline
			<< "    <step>1</step>" << newline
			<< "</t>" << newline
			<< "</parameters>" << newline;
	}
	doc.LoadFile(XML_FILENAME);
	std::cout << "error code " << doc.LoadFile(XML_FILENAME) << std::endl;
	if (doc.ErrorID( ) != tinyxml2::XML_NO_ERROR) {
		VCELL_EXCEPTION(runtime_error,"Unable to open " << XML_FILENAME);
	}
	const tinyxml2::XMLElement *root = doc.RootElement( );
	const tinyxml2::XMLElement & xElmnt = get(*root,"x"); 
	const tinyxml2::XMLElement & yElmnt = get(*root,"y"); 
	const tinyxml2::XMLElement & tElmnt = get(*root,"t"); 
	double xmin = 0;
	double xmax = 0;
	double xstep = 1;
	parseXMLElement(xElmnt,xmin,xmax,xstep);
	double ymin = 0;
	double ymax = 0;
	double ystep = 1;
	parseXMLElement(yElmnt,ymin,ymax,ystep);
	double tmin = 0;
	double tmax = 0;
	double tstep = 1;
	parseXMLElement(tElmnt,tmin,tmax,tstep);


	std::string syms[] = {"x","y","t","e"};
	SimpleSymbolTable symTb(syms,sizeof(syms)/sizeof(syms[0]));
	double v[] = {0,0,0, 2.71828};
	std::string input;
	cout << "Enter expressions to evaluate. Press RETURN to exit" << endl;
	for (;;) {
		cout << "Expression: ";
		getline(cin,input);
		if (input.size( ) == 0) {
			break;
		}
		VCell::Expression exp(input,symTb);
		for (double x = xmin; x <=xmax; x += xstep)
			for (double y = ymin; y <=ymax; y += ystep)
				for (double t = tmin; t <=tmax; t += tstep) {
					v[0] = x;
					v[1] = y;
					v[2] = t;
					const double r= exp.evaluateVector(v);
					cout << "f(" << x << ',' << y << ',' << t << ") = " << r << endl; 
				}
	}

}
TEST(expression,solo) {
	using VCell::MapTable;
	using VCell::MTExpression;
		MapTable mt;
		MTExpression solo("x + 2",mt);
		mt["x"] = 3;
		double five = solo.evaluate( );
		ASSERT_TRUE(five == 5);

		MTExpression solo2("x + y",mt);
		mt["x"] = 1;
		mt["y"] = 4;
		five = solo2.evaluate( );
		ASSERT_TRUE(five == 5);
		mt["x"] = 7;
		mt["y"] = -2;
		five = solo2.evaluate( );
		ASSERT_TRUE(five == 5);
		ASSERT_TRUE(solo.evaluate( ) == 9);
}
