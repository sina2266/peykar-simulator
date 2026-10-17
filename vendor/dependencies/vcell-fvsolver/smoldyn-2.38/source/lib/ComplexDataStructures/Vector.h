//
// Created by Logan Drescher on 3/7/25.
// Licensed under the MIT LICENSE, copyright (https://opensource.org/license/mit).
//

#ifndef VECTOR_H
#define VECTOR_H
#include <stdbool.h>

struct Struct_Vector;
typedef struct Struct_Vector Vector;

// Define linked list "methods" here
// Because you can have dangling pointers, we must *always* copy incoming data
// Please maintain the paradigm that there is one owner to the data in a Node of a Linked List!
typedef void (*appendDataToVector)(Vector*, void* data);

typedef bool (*insertDataIntoVector)(Vector*, int index, void* data);

typedef void* (*getDataFromVector)(Vector*, int index);

typedef void* (*replaceDataInVector)(Vector*, int index, void* data);

typedef bool (*removeDataFromVector)(Vector*, int index);

/*
 * This function is meant to serve as an analog to the modern "for each" loop.
 * The function pointer parameter must perform all operations as side effects.
 * The programmer must ensure that the function pointed to by the function-pointer can safely handle the data
 */
typedef void (*performOpOnAllDataInVector)(Vector*, void (*)(void*)); // Note the function pointer parameter!

typedef bool (*clearDataFromVector)(Vector*);

typedef void (*freeVector)(Vector*);

// Vector Definition
struct Struct_Vector {
    int _START_SIZE;
    void** _array;
    int _utilized_size;
    int _allocated_size;
    double _load_factor;
    //-----------
    appendDataToVector append;
    insertDataIntoVector insert;
    getDataFromVector get;
    replaceDataInVector replace;
    removeDataFromVector remove;
    performOpOnAllDataInVector applyToAll;
    clearDataFromVector clear;

    freeVector freeThis;
};

// Creation functions
Vector* create_String_Vector(int initialSize, bool shouldFreeOnRemoval);

// Utility functions
void display_String_Vector(Vector*);

#endif //VECTOR_H
