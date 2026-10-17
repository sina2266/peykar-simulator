#include <math.h>
#include <cstdio>

#include "ASTPowerNode.h"

#include <format>

#include "RuntimeException.h"
#include "DivideByZeroException.h"
#include "FunctionDomainException.h"
#include "ExpressionParserTreeConstants.h"
#include "StackMachine.h"
#include "MathUtil.h"

ASTPowerNode::ASTPowerNode() : Node(JJTPOWERNODE) {
}

ASTPowerNode::ASTPowerNode(int i) : Node(i) {
}

ASTPowerNode::~ASTPowerNode() {
}

std::string ASTPowerNode::infixString(int lang, NameScope* nameScope)
{
    if (jjtGetNumChildren() != 2) {
    	std::string errMsg{std::format("There are {} arguments for the power operator, expecting 2", jjtGetNumChildren())};
        throw RuntimeException(errMsg);
    }

    std::string buffer;
    if (lang == LANGUAGE_DEFAULT || lang == LANGUAGE_MATLAB) {
        buffer += "(";
        buffer += jjtGetChild(0)->infixString(lang, nameScope);
        buffer += " ^ ";
        buffer += jjtGetChild(1)->infixString(lang, nameScope);
        buffer += ")";
    } else if (lang == LANGUAGE_C) {
        buffer += "pow(";
        buffer += jjtGetChild(0)->infixString(lang, nameScope);
        buffer += ",";
        buffer += jjtGetChild(1)->infixString(lang, nameScope);
        buffer += ")";
    }

    return buffer;
}

void ASTPowerNode::getStackElements(std::vector<StackElement>& elements) {
	jjtGetChild(0)->getStackElements(elements);;
	jjtGetChild(1)->getStackElements(elements);;
	elements.push_back(StackElement(TYPE_POW));
}

double ASTPowerNode::evaluate(int evalType, double* values) {
	if (jjtGetNumChildren() != 2) {
		std::string errMsg{std::format("ASTPowerNode: wrong number of arguments for '^' ({}), expected 2", jjtGetNumChildren())};
		throw ExpressionException(errMsg);
	}
	//
	// see if there are any constant 0.0's, if there are simplify to 0.0
	//
	ExpressionException* exponentException = NULL;
	ExpressionException* baseException = NULL;
	Node* exponentChild = jjtGetChild(1);
	Node* baseChild = jjtGetChild(0);

	double exponentValue = 0.0;
	double baseValue = 0.0;
	try {
		exponentValue = exponentChild->evaluate(evalType, values);
	} catch (ExpressionException& e) {
		if (evalType == EVALUATE_VECTOR)
			throw e;
		exponentException = new ExpressionException(e.getMessage());
	}	
	try {
		baseValue = baseChild->evaluate(evalType, values);
	} catch (ExpressionException& e) {
		if (evalType == EVALUATE_VECTOR)
			throw e;
		baseException = new ExpressionException(e.getMessage());
	}

	if (exponentException == NULL && baseException == NULL) {
		if (baseValue == 0.0 && exponentValue < 0.0) {
			std::string childString = infixString(LANGUAGE_DEFAULT,0);
			std::string problem{std::format("u^v and u=0 and v={}<0", exponentValue)};
			std::string errorMsg = getFunctionDomainError(problem, values, "u", baseChild, "v", exponentChild);
			throw DivideByZeroException(errorMsg);
		}
		if (baseValue < 0.0 && exponentValue != MathUtil::round(exponentValue)) {
			std::string problem{std::format("u^v and u={}<0 and v={} not an integer: undefined", baseValue, exponentValue)};
			std::string errorMsg = getFunctionDomainError(problem, values, "u", baseChild, "v", exponentChild);
			throw FunctionDomainException(errorMsg);
		}
		double result = pow(baseValue, exponentValue);
		if (MathUtil::double_infinity == -result || MathUtil::double_infinity == result || result != result) {
			std::string problem{std::format("u^v evaluated to {}, u={}, v={}", result, baseValue, exponentValue)};
			std::string errorMsg = getFunctionDomainError(problem, values, "u", baseChild, "v", exponentChild);
			throw FunctionDomainException(errorMsg);
		}
		return result;
	} else if (exponentException == 0 && exponentValue == 0.0) {
		return 1.0;
	} else if (baseException == 0 && baseValue == 1.0) {
		return 1.0;
	} else {
		if (baseException != NULL) {
			throw (*baseException);
		} else if (exponentException != NULL) {
			throw (*exponentException);
		} 
	}
}

Node* ASTPowerNode::copyTree() {
	ASTPowerNode* node = new ASTPowerNode();
	for (int i=0;i<jjtGetNumChildren();i++){
		node->jjtAddChild(jjtGetChild(i)->copyTree());
	}
	return node;	
}
