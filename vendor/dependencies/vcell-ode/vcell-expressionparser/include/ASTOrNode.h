#ifndef ASTORNODE_H
#define ASTORNODE_H

#include "Node.h"

class ASTOrNode : public Node
{
public:
	ASTOrNode(int i);
	~ASTOrNode();
	std::string infixString(int lang, NameScope* nameScope);
	void getStackElements(std::vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 
	bool isBoolean();

	Node* copyTree();

private:
	ASTOrNode();
};

#endif
