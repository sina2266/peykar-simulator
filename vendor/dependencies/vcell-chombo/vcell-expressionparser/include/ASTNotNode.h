#ifndef ASTNOTNODE_H
#define ASTNOTNODE_H

#include "Node.h"

class ASTNotNode : public Node
{
public:
	ASTNotNode(int i);
	~ASTNotNode();
	std::string infixString(int lang, NameScope* nameScope);
	void getStackElements(std::vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 
	bool isBoolean();

	Node* copyTree();

private:
	ASTNotNode();
};

#endif
