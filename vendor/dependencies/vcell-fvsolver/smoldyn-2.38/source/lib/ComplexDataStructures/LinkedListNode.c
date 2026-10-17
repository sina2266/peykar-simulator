//
// Created by Logan Drescher on 2/12/25.
// Licensed under the MIT LICENSE, copyright (https://opensource.org/license/mit).
//

#include "LinkedListNode.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//=================================================================================================================//
// String Implementation
//=================================================================================================================//
static void* copyStringData_LLN(const void* string) {
    const char* src = (const char*)string;
    int strLen = strlen(src) + 1;
    char* data = (char*)malloc(sizeof(char) * strLen);;
    strcpy(data, src);
    return data;
}

// always returns NULL
static void* freeStringData_LLN(void* string) {
    char* str = (char*)string;
    if (str != NULL) free(str);
    return NULL;
}

static void* replaceAndDoNotFreeStringOperation_LLN(LinkedList_Node* node, const void* data) {
    char* formerData = (char*)node->data;
    node->data = data == NULL ? NULL : node->_copy(data);
    return formerData;
}

static void* removeStringOperation_LLN(LinkedList_Node* node, const void* data) {
    node->data = node->free(node->data);
    node->data = node->_copy(data);
    return NULL;
}

LinkedList_Node* createStringLinkedList_Node(int autoFreeOnReplace) {
    LinkedList_Node* node = (LinkedList_Node*)malloc(sizeof(LinkedList_Node));
    if (node == NULL) return NULL;
    node->data = NULL;
    node->next = NULL;
    node->_copy = copyStringData_LLN;
    node->free = freeStringData_LLN;
    node->replace = autoFreeOnReplace ? removeStringOperation_LLN : replaceAndDoNotFreeStringOperation_LLN;
    return node;
}
//=================================================================================================================//