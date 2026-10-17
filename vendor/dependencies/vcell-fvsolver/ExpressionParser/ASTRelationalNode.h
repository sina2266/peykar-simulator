#ifndef ASTRELATIONALNODE_H
#define ASTRELATIONALNODE_H

#include "Node.h"

class ASTRelationalNode : public Node
{
public:
	ASTRelationalNode(int i);
	~ASTRelationalNode();
	void setOperationFromToken(string op);
	string infixString(int lang, NameScope* nameScope);
	void getStackElements(vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 
	bool isBoolean();

	Node* copyTree();
	bool equals(Node* node);

private:
	int operation;
	string opString;
	ASTRelationalNode();
};

#endif
