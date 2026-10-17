//
// Created by Logan Drescher on 2/12/25.
// Licensed under the MIT LICENSE, copyright (https://opensource.org/license/mit).
//

#ifndef LINKEDLISTNODE_H
#define LINKEDLISTNODE_H

struct Struct_LinkedList_Node;
typedef struct Struct_LinkedList_Node LinkedList_Node;

// Define node "methods" here
// Base Methods
typedef void* (*copyData_LLN)(const void*);
typedef void* (*freeData_LLN)(void*);

typedef void* (*replaceDataOperation_LLN)(LinkedList_Node*, const void*);

// Node Definition
struct Struct_LinkedList_Node {
    void* data;
    LinkedList_Node* next;
    //-----------
    // Because you can have dangling pointers, we must *always* copy incoming data
    // Please maintain the paradigm that there is one owner to the data in a Node of a Linked List!
    copyData_LLN _copy;
    freeData_LLN free;
    replaceDataOperation_LLN replace;
};

// Utility functions
LinkedList_Node* createStringLinkedList_Node(int autoFreeOnReplace);

#endif //LINKEDLISTNODE_H
