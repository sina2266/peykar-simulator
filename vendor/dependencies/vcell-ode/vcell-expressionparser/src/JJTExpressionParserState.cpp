#include "JJTExpressionParserState.h"

JJTExpressionParserState::JJTExpressionParserState(void)
: sp(0)
, mk(0)
, node_created(false)
{
}

JJTExpressionParserState::~JJTExpressionParserState(void)
{
	nodes.clear();
	marks.clear();
}

bool JJTExpressionParserState::nodeCreated(void)
{
	return node_created;
}

void JJTExpressionParserState::reset(void)
{
	nodes.clear();
	marks.clear();
	sp = 0;
	mk = 0;
}

Node* JJTExpressionParserState::rootNode(void)
{
	return nodes.front();
}

void JJTExpressionParserState::pushNode(Node* n)
{
	nodes.push_back(n);
	++sp;
}

Node* JJTExpressionParserState::popNode(void)
{
	if (--sp < mk) {
		popMark();
	}

	Node* n = nodes.back();
	nodes.pop_back();	
	return n;
}

Node* JJTExpressionParserState::peekNode(void) {
	return nodes.front();
}

int JJTExpressionParserState::nodeArity(void)
{
	return sp - mk;
}

void JJTExpressionParserState::clearNodeScope(Node* n)
{
	while (sp > mk) {
		popNode();
	}
	popMark();
}

void JJTExpressionParserState::openNodeScope(Node* n)
{
	marks.push_back(mk);
    mk = sp;
    n->jjtOpen();
}

void JJTExpressionParserState::closeNodeScope(Node* n, int num)
{
	popMark();
	while (num-- > 0) {
		Node* c = popNode();
		c->jjtSetParent(n);
		n->jjtAddChild(c, num);
	}
	n->jjtClose();
	pushNode(n);
	node_created = true;
}

void JJTExpressionParserState::closeNodeScope(Node* n, bool condition)
{
	if (condition) {
		int a = nodeArity();
		popMark();
		while (a-- > 0) {
			Node* c = popNode();
			c->jjtSetParent(n);
			n->jjtAddChild(c, a);
		}
		n->jjtClose();
		pushNode(n);
		node_created = true;
	} else {
		popMark();
		node_created = false;
		delete n;
	}
}

void JJTExpressionParserState::popMark(void)
{
	mk = 0;
	if (marks.size() > 0) {
		mk = marks.back();
		marks.pop_back();
	}
}
