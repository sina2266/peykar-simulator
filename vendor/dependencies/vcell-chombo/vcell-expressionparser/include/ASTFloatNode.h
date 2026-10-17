#ifndef ASTFLOATNODE_H
#define ASTFLOATNODE_H

#include "Node.h"

class ASTFloatNode : public Node
{
public:
	ASTFloatNode(double i);
	ASTFloatNode(int i);
	~ASTFloatNode();
	double value;
	std::string infixString(int lang, NameScope* nameScope);
	void getStackElements(std::vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 

	Node* copyTree();
	bool equals(Node* node);
};

#endif
