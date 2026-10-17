#ifndef SIMPLENODE_H
#define SIMPLENODE_H
#include <string>
#include <vector>

#define LANGUAGE_DEFAULT  0
#define LANGUAGE_C 1
#define LANGUAGE_MATLAB  2
#define LANGUAGE_JSCL 3
#define LANGUAGE_VISIT 5

#define EVALUATE_CONSTANT  111
#define EVALUATE_VECTOR 112
#define EVALUATE_PROXY  113

class NameScope;
class SymbolTableEntry;
struct StackElement;
class SymbolTable;


class ExpressionParser;
class NameScope;

class Node
{
public:
	Node(int unused);
	virtual ~Node(void);
	virtual Node* copyTree()=0;
	virtual void getStackElements(std::vector<StackElement>& elements)=0;
	virtual double evaluate(int type, double* values=0)=0;	

	void jjtOpen();
	void jjtClose();
	void jjtSetParent(Node* n);
	Node* jjtGetParent();
	void jjtAddChild(Node* n, int i);
	/**
	* remove self as child's parent
	* remove reference to specified child
	* @return removed child
	*/
	Node* abandonChild(int i);
	Node* jjtGetChild(int i);
	int jjtGetNumChildren();
	void dump(std::string prefix);
	virtual std::string infixString(int lang, NameScope* nameScope)=0;
	std::string toString(std::string prefix);
	virtual void getSymbols(std::vector<std::string>& symbols, int language, NameScope* nameScope);
	virtual SymbolTableEntry* getBinding(std::string symbol);
	virtual void bind(SymbolTable* symbolTable);
	static std::string getFunctionDomainError(std::string problem, double* values, std::string argumentName1, Node* node1, std::string argumentName2="", Node* node2=0);
	static std::string getNodeSummary(double* values, Node* node);
	virtual bool isBoolean();

	void jjtAddChild(Node* n);
	void substitute(Node* origNode, Node* newNode);
	virtual bool equals(Node* node);
	virtual bool isConstant( ) const;

protected:
	Node* parent;
	Node** children;
	int numChildren;
};
#endif
