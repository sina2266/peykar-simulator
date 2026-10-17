#include <format>
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

std::string ASTFloatNode::infixString(int lang, NameScope* nameScope)
{
	//if (value == NULL) {
    //    return string("NULL");
    //} else 
	if (value == 0.0) return std::string{"0.0"};
	// The format spec has to sit inside a replacement field -- ":.20g" on its own
	// is a literal, so every float used to print as the string ":.20g" and any
	// infix() round-trip failed to re-parse.
	return std::format("{:.20g}", value);
}

void ASTFloatNode::getStackElements(std::vector<StackElement>& elements) {
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
