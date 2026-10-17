#ifndef ASTEXPRESSION_H
#define ASTEXPRESSION_H

#include "Node.h"

class ASTExpression : public Node
{
public:
	ASTExpression(int i);
	~ASTExpression();
	string infixString(int lang, NameScope* nameScope);
	void getStackElements(vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 

	Node* copyTree();

private:
	ASTExpression();
};

#endif
