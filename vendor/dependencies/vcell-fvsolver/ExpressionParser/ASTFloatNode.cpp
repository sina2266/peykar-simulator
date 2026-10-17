#include <stdio.h>

#include "ASTFloatNode.h"
#include "RuntimeException.h"
#include "ExpressionParserTreeConstants.h"
#include "StackMachine.h"

ASTFloatNode::ASTFloatNode(double doubleValue) : Node(JJTFLOATNODE) {
	// is not a number
	if (doubleValue != doubleValue){
		throw RuntimeException("cannot set float node to NaN");
	}
	value = doubleValue;
}

ASTFloatNode::ASTFloatNode(int i) : Node(i) , value(0)
{
}

ASTFloatNode::~ASTFloatNode() {
}

string ASTFloatNode::infixString(int lang, NameScope* nameScope)
{
	//if (value == NULL) {
    //    return string("NULL");
    //} else 
	if (value == 0.0) {
        return string("0.0");
    } else {
		char s[256];		
		sprintf(s, "%.20lg\0", value);
        return string(s);
    }
}

void ASTFloatNode::getStackElements(vector<StackElement>& elements) {
	elements.push_back(StackElement(value));
}

double ASTFloatNode::evaluate(int evalType, double* values) {
	return value;
}

Node* ASTFloatNode::copyTree(){
	ASTFloatNode* node = new ASTFloatNode(value);
	return node;	
}

bool ASTFloatNode::equals(Node* node) {
	//
	// check to see if the types and children are the same
	//
	if (!Node::equals(node)){
		return false;
	}
	
	//
	// check this node for same state (value)
	//	
	ASTFloatNode* floatNode = (ASTFloatNode*)node;
	if (floatNode->value != value){
		return false;
	}	

	return true;
}
