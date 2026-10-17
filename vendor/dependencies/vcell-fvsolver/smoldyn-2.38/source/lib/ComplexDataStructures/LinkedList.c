//
// Created by Logan Drescher on 2/12/25.
// Licensed under the MIT LICENSE, copyright (https://opensource.org/license/mit).
//

#include "LinkedList.h"
#include <stdlib.h>
#include <stdio.h>

static LinkedList_Node* getNodeAtIndexFromLL(LinkedList* ll, int index) {
    if (index < 0 || index >= ll->_size) return NULL;
    if (index == ll->_size - 1) return ll->_tail;
    LinkedList_Node* currentNode = ll->_head;
    for (int i = 0; i < index; i++) currentNode = currentNode->next;
    return currentNode;
}

static void performOperationsOnAllEntriesInLL(LinkedList* ll, void (*operation)(void*)){
    if (ll->_size == 0) return;
    LinkedList_Node* currentNode = ll->_head;
    while (currentNode != NULL) {
        operation(currentNode->data);
        currentNode = currentNode->next;
    }
}

//=================================================================================================================//
// String Implementation
//=================================================================================================================//
static bool insertStringDataIntoLL(LinkedList* ll, int index, void* data) {
    if (index < 0 || index > ll->_size) return false;
    LinkedList_Node* newNode = createStringLinkedList_Node(0); // if this is NULL, we have BIG problems!
    newNode->data = data;
    ll->_size++; // remember: we increased size! all calculations below must account for that!
    if (index == 0 && ll->_head == NULL) { ll->_head = newNode; ll->_tail = ll->_head; return true; } // Only a value of NULL for `newNode` would cause a "false" value
    if (index == ll->_size - 1) { ll->_tail->next = newNode; ll->_tail = newNode; return true;} // Only a value of NULL for `newNode` would cause a "false" value
    if (index == 0) { newNode->next = ll->_head; ll->_head = newNode; return true; } // Only a value of NULL for `newNode` would cause a "false" value

    LinkedList_Node* targetNode = getNodeAtIndexFromLL(ll, index - 1);
    newNode->next = targetNode->next;
    targetNode->next = newNode; // Only a value of NULL for `newNode` would cause a "false" value
    return true;
}

static void appendStringDataToLL(LinkedList* ll, void* data) {
    insertStringDataIntoLL(ll, ll->_size, data);
}

static void* getStringDataFromLL(LinkedList* ll, int index) {
    if (index < 0 || index >= ll->_size) return NULL;
    return getNodeAtIndexFromLL(ll, index)->data;
}

static void* replaceStringInLL(LinkedList* ll, int index, void* data) {
    if (index < 0 || index >= ll->_size) return NULL;
    LinkedList_Node* currentNode = getNodeAtIndexFromLL(ll, index);
    return currentNode->replace(currentNode, data);
}

static bool removeStringFromLL_withFree(LinkedList* ll, int index) {
    if (index < 0 || index >= ll->_size) return false;
    LinkedList_Node* nodeToBeRemoved;
    if (index == 0) {
        nodeToBeRemoved = ll->_head;
        ll->_head = ll->_head->next;
    } else {
        LinkedList_Node* oneNodeBefore = getNodeAtIndexFromLL(ll, index - 1);
        if (index == ll->_size - 1) ll->_tail = oneNodeBefore;
        nodeToBeRemoved = oneNodeBefore->next;
        oneNodeBefore->next = nodeToBeRemoved->next;
    }
    char* oldData = (char*)nodeToBeRemoved->replace(nodeToBeRemoved, NULL);
    if (oldData != NULL) free(oldData);
    free(nodeToBeRemoved);
    ll->_size--;
    if (ll->_size != 0) return true;
    ll->_head = NULL;
    ll->_tail = NULL;
    return true;
}

static bool removeStringFromLL_withoutFree(LinkedList* ll, int index) {
    if (index < 0 || index >= ll->_size) return false;
    LinkedList_Node* nodeToBeRemoved;
    if (index == 0) {
        nodeToBeRemoved = ll->_head;
        ll->_head = ll->_head->next;
    } else {
        LinkedList_Node* oneNodeBefore = getNodeAtIndexFromLL(ll, index - 1);
        if (index == ll->_size - 1) ll->_tail = oneNodeBefore;
        nodeToBeRemoved = oneNodeBefore->next;
        oneNodeBefore->next = nodeToBeRemoved->next;
    }
    free(nodeToBeRemoved);
    ll->_size--;
    if (ll->_size != 0) return true;
    ll->_head = NULL;
    ll->_tail = NULL;
    return true;
}

static bool clearStringLinkedList(LinkedList* ll) {
    bool thingsHaveBeenRemoved = false;
    while (ll->_head != NULL) {
        int result = ll->remove(ll, 0);
        //int result = removeStringFromLL(ll, 0);
        thingsHaveBeenRemoved |= result;
    }

    return thingsHaveBeenRemoved;
}

static void freeStringLinkedList(LinkedList* ll) {
    clearStringLinkedList(ll);
    free(ll);
}

LinkedList* create_String_LinkedList(bool shouldFreeOnRemoval) {
    LinkedList* stringLinkedList = (LinkedList*) malloc(sizeof(LinkedList));
    if (stringLinkedList == NULL) return NULL;
    stringLinkedList->_head = NULL;
    stringLinkedList->_tail = stringLinkedList->_head;
    stringLinkedList->_size = 0;
    stringLinkedList->append = appendStringDataToLL;
    stringLinkedList->insert = insertStringDataIntoLL;
    stringLinkedList->get = getStringDataFromLL;
    stringLinkedList->replace = replaceStringInLL;
    stringLinkedList->remove = shouldFreeOnRemoval ? removeStringFromLL_withFree : removeStringFromLL_withoutFree;
    stringLinkedList->applyToAll = performOperationsOnAllEntriesInLL;
    stringLinkedList->clear = clearStringLinkedList;
    stringLinkedList->freeThis = freeStringLinkedList;
    return stringLinkedList;
}

void display_String_Vector(LinkedList* ll) {
    LinkedList_Node* currentNode = ll->_head;
    while (currentNode != NULL) {
        printf("(%p)\"%s\"->", currentNode, (char*)currentNode->data);
        currentNode = currentNode->next;
    }
    printf("0\n");
}
//=================================================================================================================//