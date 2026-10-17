#ifndef SIMPLENODE_H
#define SIMPLENODE_H
#include <string>
#include <vector>
using std::string;
using std::vector;

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
	virtual void getStackElements(vector<StackElement>& elements)=0;
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
	void dump(string prefix);
	virtual string infixString(int lang, NameScope* nameScope)=0;
	string toString(string prefix);
	virtual void getSymbols(vector<string>& symbols, int language, NameScope* nameScope);
	virtual SymbolTableEntry* getBinding(string symbol);
	virtual void bind(SymbolTable* symbolTable);
	static string getFunctionDomainError(string problem, double* values, string argumentName1, Node* node1, string argumentName2="", Node* node2=0);
	static string getNodeSummary(double* values, Node* node);
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
