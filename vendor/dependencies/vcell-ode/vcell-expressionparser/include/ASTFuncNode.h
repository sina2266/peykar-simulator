#ifndef ASTFUNCNODE_H
#define ASTFUNCNODE_H

#include "Node.h"

class ASTFuncNode : public Node
{
public:
	ASTFuncNode(int i);
	~ASTFuncNode();
	void setFunctionFromParserToken(std::string parserToken);
	std::string infixString(int lang, NameScope* nameScope);
	void getStackElements(std::vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 

	Node* copyTree();
	bool equals(Node* node);

private:
	int funcType;
	std::string funcName;

	ASTFuncNode();
};

#endif
