#ifndef ASTANDNODE_H
#define ASTANDNODE_H

#include "Node.h"

class ASTAndNode : public Node
{
public:
	ASTAndNode(int i);
	~ASTAndNode();
	string infixString(int lang, NameScope* nameScope);
	void getStackElements(vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 
	bool isBoolean();

	Node* copyTree();

private:
	ASTAndNode();
};

#endif
