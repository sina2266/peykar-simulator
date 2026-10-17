#include <assert.h>
#include <math.h>
#include "IndexedTree.h"

/*
 *The constructor of IndexedTree.
 *Initilization of the vector storing tree nodes. 
 */
IndexedTree::IndexedTree()
{
	index.erase(index.begin(), index.end());
	size = 0;
}//end of constructor IndexedTree()

//Destructor
IndexedTree::~IndexedTree()
{
	index.clear();
	//index.~vector(); the original sentence, is it useful?
}//end of destructor ~IndexedTree()

/*
 *Add a new jump process in the tree.
 *Input para: Jump*, jump process to be updated. 
 */
void IndexedTree::addProcess(Jump *jmp)
{
	index.push_back(jmp);
	size = index.size();
	return;
}//end of method addProcess()

/*
 *Get the jump process from tree by it's index.
 *Input para: int, position of the process in tree. 
 */
Jump* IndexedTree::getProcess(int i)
{
	return index[i];
}//end of method getProcess()

/*
 *Set the jump process's position in the tree.
 *Input para: int, position to put the process.
 *            Jump*, the process.
 */
void IndexedTree::setProcess(int i, Jump *jump)
{
	index[i] = jump;
	return;
}//end of method setProcess()

/*
 *Exchange the positions in the tree between two tree nodes(processes).
 *Input para: int, position to be changed with another.
 *            int, position to be changed with another.
 */
void IndexedTree::swap(int i, int j)
{
	assert(i<size);
	assert(j<size);
	Jump *pTemp = index[i];
	index[i] = index[j];
	index[i]->setNode(i);
	index[j] = pTemp;
	index[j]->setNode(j);
	return;
}//end of method swap()

/*
 *Bubble-up the smallest treenodes (in terms of tau value)in
 *a subtree. Node "i" is the root of the subtree.
 *Input para: int, root of the subtree. 
 */
void IndexedTree::heapify(int i)
{
	int l, r; 
	int smallest = i;
	l = leftChild(i);
	r = rightChild(i);
	if(l<size){
		if(*index[l]<*index[i]){
			smallest = l;
		}
	}
	if(r<size){
		if(*index[r]<*index[smallest]){
			smallest = r;
		}
	}
	if(smallest!=i){
		swap(i, smallest);
		heapify(smallest);
	}
	return;
}//end of the method heapify()

/*
 *Build up the tree in order to make each parent node
 *is smaller than its children.
 *
 */
void IndexedTree::build()
{
	for(int i=int(size/2)-1; i>=0; i--){
		heapify(i);
	}
	return;
}//end of method build()

/*
 *Update one node by adjusting it's position with it's parents and
 *childern.
 *Input para: int, the node whose position is to be adjusted. 
 */
void IndexedTree::update(int i)
{
	int p = parent(i);

	if((p>=0)&&(*index[i]<*index[p])){
			swap(i, p);
			update(p);
			return;
	}else{
		int smallest = i;
		int l = leftChild(i);
		int r = rightChild(i);

		if(l<size){
			smallest = l;
		}
		if(r<size){
			if(*index[r]<*index[smallest]){
				smallest = r;
			}
		}
		if(smallest!=i){
			if(*index[smallest]<*index[i]){
				swap(i, smallest);
				update(smallest);
			}
		}
		return;
	}
}//end of method update()

/*
 *Update a process by a new tau and adjust it's position in the tree.
 *Input para: jump*, the process to be adjusted.
 *            double, the new value of tau to be set to the process.
 */
void IndexedTree::updateTree(Jump *jump, double newTime)
{
	int nd = jump->getNode();
	index[nd]->setTime(newTime);
	update(nd);
	return;
}//end of method updateTree()
