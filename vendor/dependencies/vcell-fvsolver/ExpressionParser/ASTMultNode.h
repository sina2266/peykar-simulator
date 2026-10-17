#ifndef ASTMULTNODE_H
#define ASTMULTNODE_H

#include "Node.h"

class ASTMultNode : public Node
{
public:
	ASTMultNode(int i);
	~ASTMultNode();
	string infixString(int lang, NameScope* nameScope);
	void getStackElements(vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 
	bool isBoolean();

	Node* copyTree();

private:
	ASTMultNode();
};

#endif
