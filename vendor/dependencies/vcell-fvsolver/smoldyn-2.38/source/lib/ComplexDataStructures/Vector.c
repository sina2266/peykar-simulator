//
// Created by Logan Drescher on 3/7/25.
// Licensed under the MIT LICENSE, copyright (https://opensource.org/license/mit).
//

#include "Vector.h"
#include <stdlib.h>
#include <stdio.h>


static void performOperationsOnAllEntriesInVector(Vector* vector, void (*operation)(void*)){
    for (int i = 0; i < vector->_utilized_size; i++) {
        operation(vector->_array[i]);
    }
}

static int resizeInternalArray(Vector* vector, int new_allocated_size) {
    void** new_array = (void**)realloc(vector->_array, new_allocated_size * sizeof(void*));
    if (!new_array) return 0;
    vector->_array = new_array;
    vector->_allocated_size = new_allocated_size;
    return 1;
}


static void checkForResize(Vector* vector) {
    double max_load_threshold = vector->_load_factor * vector->_allocated_size;
    double min_load_threshold = (1.0 - vector->_load_factor) * vector->_allocated_size;
    double current_load = vector->_utilized_size;
    if (current_load < max_load_threshold && current_load > min_load_threshold) return;
    int new_allocated_size = current_load < max_load_threshold ? vector->_allocated_size / 2 : vector->_allocated_size * 2;
    if (new_allocated_size < vector->_START_SIZE) return;
    resizeInternalArray(vector, new_allocated_size);
    checkForResize(vector);
}

//=================================================================================================================//
// String Implementation
//=================================================================================================================//
static bool insertStringDataIntoVector(Vector* vector, int index, void* data) {
    if (index < 0 || index > vector->_utilized_size) return false;
    for (int i = vector->_utilized_size; i > index; i--) {
        vector->_array[i] = vector->_array[i - 1];
    }
    vector->_array[index] = data;
    vector->_utilized_size++;
    checkForResize(vector);
    return true;
}

static void appendStringDataToVector(Vector* vector, void* data) {
    insertStringDataIntoVector(vector, vector->_utilized_size, data);
}

static void* getStringDataFromVector(Vector* vector, int index) {
    if (index < 0 || index >= vector->_utilized_size) return NULL;
    return vector->_array[index];
}

static void* replaceStringInVector(Vector* vector, int index, void* data) {
    if (index < 0 || index >= vector->_utilized_size) return NULL;
    void* oldData = vector->_array[index];
    vector->_array[index] = data;
    return oldData;
}

static bool removeStringFromVector_withoutFree(Vector* vector, int index) {
    if (index < 0 || index >= vector->_utilized_size) return false;
    for (int i = index; i < vector->_utilized_size-1; i++) {
        vector->_array[i] = vector->_array[i+1];
    }
    vector->_utilized_size--;
    checkForResize(vector);
    return true;
}

static bool removeStringFromVector_withFree(Vector* vector, int index) {
    if (index < 0 || index >= vector->_utilized_size) return false;
    free(vector->_array[index]);
    return removeStringFromVector_withoutFree(vector, index);
}


static bool clearStringVector_withFree(Vector* vector) {
    if (vector->_utilized_size == 0) return false;
    for (int i = 0; i < vector->_utilized_size; i++) {
        free(vector->_array[i]);
    }
    vector->_utilized_size = 0;
    checkForResize(vector);
    return true;
}

static bool clearStringVector_withoutFree(Vector* vector) {
    if (vector->_utilized_size == 0) return false;
    vector->_utilized_size = 0;
    checkForResize(vector);
    return false;
}

static void freeStringVector(Vector* vector) {
    vector->clear(vector);
    free(vector->_array);
    free(vector);
}

Vector* create_String_Vector(int initialSize, bool shouldFreeOnRemoval) {
    if (initialSize <= 0) return NULL;
    Vector* stringVector = (Vector*) malloc( sizeof(Vector));
    if (stringVector == NULL) return NULL;
    stringVector->_START_SIZE = initialSize;
    stringVector->_array = (void**)malloc(initialSize * sizeof(void*));
    stringVector->_utilized_size = 0;
    stringVector->_allocated_size = initialSize;
    stringVector->_load_factor = 0.75;
    stringVector->append = appendStringDataToVector;
    stringVector->insert = insertStringDataIntoVector;
    stringVector->get = getStringDataFromVector;
    stringVector->replace = replaceStringInVector;
    stringVector->remove = shouldFreeOnRemoval ? removeStringFromVector_withFree : removeStringFromVector_withoutFree;
    stringVector->applyToAll = performOperationsOnAllEntriesInVector;
    stringVector->clear = shouldFreeOnRemoval ? clearStringVector_withFree : clearStringVector_withoutFree;
    stringVector->freeThis = freeStringVector;
    return stringVector;
}

void display_String_Vector(Vector* vector) {
    printf("[ ");
    for (int i = 0; i < vector->_utilized_size; i++) {
        printf("(%p)\"%s\"", vector->_array[i], (char*)vector->_array[i]);
        if (i != vector->_utilized_size-1) printf(", ");
    }
    printf(" ]\n");
}
//=================================================================================================================//