#ifndef ASTMINUSTERMNODE_H
#define ASTMINUSTERMNODE_H

#include "Node.h"

class ASTMinusTermNode : public Node
{
public:
	ASTMinusTermNode(int i);
	~ASTMinusTermNode();
	string infixString(int lang, NameScope* nameScope);
	void getStackElements(vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 

	Node* copyTree();

private:
	ASTMinusTermNode();
};

#endif
