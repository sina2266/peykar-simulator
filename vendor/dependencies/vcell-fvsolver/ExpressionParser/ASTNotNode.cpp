#include "ASTNotNode.h"
#include "DivideByZeroException.h"
#include "ExpressionException.h"
#include "ExpressionParserTreeConstants.h"
#include "StackMachine.h"

ASTNotNode::ASTNotNode() : Node(JJTNOTNODE) {
}

ASTNotNode::ASTNotNode(int i) : Node(i) {
}

ASTNotNode::~ASTNotNode() {
}

bool ASTNotNode::isBoolean() {
	return true;
}

string ASTNotNode::infixString(int lang, NameScope* nameScope)
{
	string buffer;
	if (lang == LANGUAGE_VISIT){
		buffer.append("not(");
	}else{
		buffer.append("!(");
	}
	buffer += jjtGetChild(0)->infixString(lang,nameScope);
	buffer += ")";

	return buffer;
}

void ASTNotNode::getStackElements(vector<StackElement>& elements) {
	jjtGetChild(0)->getStackElements(elements);
	elements.push_back(StackElement(TYPE_NOT));
}


double ASTNotNode::evaluate(int evalType, double* values) {
	double childValue = jjtGetChild(0)->evaluate(evalType, values);
	if (childValue==0.0){
		return 1.0;
	}else{
		return 0.0;
	}
}

Node* ASTNotNode::copyTree() {
	ASTNotNode* node = new ASTNotNode();
	for (int i=0;i<jjtGetNumChildren();i++){
		node->jjtAddChild(jjtGetChild(i)->copyTree());
	}
	return node;	
}
