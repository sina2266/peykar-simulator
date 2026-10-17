#ifndef JJEXPRESSIONPARSERSTATE_H
#define JJEXPRESSIONPARSERSTATE_H

#include <vector>
using std::vector;

#include "Node.h"

class JJTExpressionParserState
{
public:
	JJTExpressionParserState(void);
	~JJTExpressionParserState(void);
	bool nodeCreated(void);
	void reset(void);
	Node* rootNode(void);
	void pushNode(Node* n);
	Node* popNode(void);
	Node* peekNode(void);
	int nodeArity(void);
	void clearNodeScope(Node* n);
	void openNodeScope(Node* n);
	void closeNodeScope(Node* n, int num);
	void closeNodeScope(Node* n, bool condition);

private:
	void popMark(void);
	vector<Node*> nodes;
	vector<int> marks;
	int sp;
	int mk;
	bool node_created;

};

#endif
