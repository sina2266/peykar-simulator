#ifndef ASTIDNODE_H
#define ASTIDNODE_H

#include "Node.h"

class SymbolTableEntry;

class ASTIdNode : public Node
{
public:
	ASTIdNode(int i);
	~ASTIdNode();
	std::string name;
	std::string infixString(int lang, NameScope* nameScope);
	SymbolTableEntry* symbolTableEntry;
	SymbolTableEntry* getBinding(std::string symbol);
	void bind(SymbolTable* symbolTable);
	void getStackElements(std::vector<StackElement>& elements);
	double evaluate(int evalType, double* values=0); 
	void getSymbols(std::vector<std::string>& symbols, int language, NameScope* nameScope);

	Node* copyTree();
	bool equals(Node* node);
	/**
	* @return false
	*/
	virtual bool isConstant( ) const;

private:
	ASTIdNode(ASTIdNode*);
};

#endif
