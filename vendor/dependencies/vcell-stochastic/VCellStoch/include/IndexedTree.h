#ifndef INDEXEDTREE_H
#define INDEXEDTREE_H

#include "Jump.h"

/* This class defines an indexed tree.  
 * In this package, it is used by Gibson method to select next reaction.
 * It is basically a binary tree and processes are stored in a vector as tree nodes.
 * Reference method descriptions in IndexedTree.cpp.
 *
 * @Author: Boris Slepchenko
 * @Amended: Tracy LI
 * @version:1.0 Beta
 * @CCAM,UCHC. May 25,2006
 */
class IndexedTree
{
public:
	IndexedTree();
	~IndexedTree();
	inline int parent(int i){return int((i+1)/2)-1;}
	inline int leftChild(int i){return 2*i+1;}
	inline int rightChild(int i){return 2*i+2;}
	void addProcess(Jump*);
	void swap (int, int);
	void heapify(int);
	void build();
	void update(int);
	void updateTree(Jump*, double);
	int getSize() {return size;}
	Jump *getProcess(int);
	void setProcess(int, Jump*);

private:
	int size;// the size of the tree
	vector<Jump*> index; //binary tree which saves jump processes and the root has the smallest value
};

#endif
