#ifndef ASTINVERTTERMNODE_H
#define ASTINVERTTERMNODE_H

#include "Node.h"

class ASTInvertTermNode : public Node
{
public:
	ASTInvertTermNode(int i);
	~ASTInvertTermNode();
	string infixString(int lang, NameScope* nameScope);
	void getStackElements(vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 

	Node* copyTree();

private:
	ASTInvertTermNode();
};

#endif
