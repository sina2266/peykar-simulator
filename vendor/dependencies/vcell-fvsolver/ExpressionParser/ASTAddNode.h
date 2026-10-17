#ifndef ASTADDNODE_H
#define ASTADDNODE_H

#include "Node.h"

class ASTAddNode : public Node
{
public:
	ASTAddNode(int i);
	~ASTAddNode();
	string infixString(int lang, NameScope* nameScope);
	void getStackElements(vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 

	Node* copyTree();

private:
	ASTAddNode();
};

#endif
