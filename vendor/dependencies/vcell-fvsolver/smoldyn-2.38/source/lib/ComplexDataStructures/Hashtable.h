//
// Created by Logan Drescher on 3/6/25.
// Licensed under the MIT LICENSE, copyright (https://opensource.org/license/mit).
// Adapted From Ben Hoyt et. all's work on GitHub (LICENSE = MIT) (https://github.com/benhoyt/ht/)
//

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stddef.h>
#include <stdbool.h>

// Hash table structure: create with ht_create, free with ht_destroy.
struct Struct_Hashtable; typedef struct Struct_Hashtable Hashtable;
struct Struct_HashtableIterator; typedef struct Struct_HashtableIterator HashtableIterator;
struct Struct_HashtableEntry; typedef struct Struct_HashtableEntry HashtableEntry;

// Free memory allocated for hash table, including allocated keys.
typedef void (*destroyHashtable)(Hashtable* table);

// Check if item with given key (NUL-terminated) is inside hash table. Return
// true if contained, else false.
typedef bool (*ifHashtableContains)(Hashtable* table, const char* key);

// Get item with given key (NUL-terminated) from hash table. Return
// value, or assertion failure if key not found.
typedef void* (*getFromHashtable)(Hashtable* table, const char* key);

// Set item with given key (NUL-terminated) to value.
// If not already present in table, key is augmented and copied to newly
// allocated memory (keys are freed automatically when `destroyHashtable` is
// called). Return address of copied key, or NULL if out of memory.
typedef const char* (*putInHashtable)(Hashtable* table, const char* key, void* value);

// Return number of items in hash table.
typedef size_t (*getHashtableLength)(Hashtable* table);

// Return new hash table iterator (for use with ht_next).
typedef HashtableIterator (*generateHashtableIterator)(Hashtable* table);

// Move iterator to next item in hash table, update iterator's key
// and value to current item, and return true. If there are no more
// items, return false. Don't call `putInHashtable` during iteration.
typedef bool (*iterateToNext)(HashtableIterator* it);

// Hash table entry (slot may be filled or empty).
struct Struct_HashtableEntry {
    const char* key;  // key is NULL if this slot is empty
    void* value;
};

// Hash table structure: create with createNewHashtable, free with destroyHashtable.
struct Struct_Hashtable {
    size_t _capacity;    // size of _entries array
    size_t _length;      // number of items in hash table
    HashtableEntry* entries;  // hash slots

    destroyHashtable freeThis;
    ifHashtableContains contains;
    getFromHashtable getFrom;
    putInHashtable putIn;
    getHashtableLength size;
    generateHashtableIterator getIterator;
};

Hashtable* createNewHashtable(void);

// Hash table iterator: create with ht_iterator, iterate with ht_next.
struct Struct_HashtableIterator {
    const char* key;  // current key
    void* value;      // current value

    // Don't use these fields directly.
    Hashtable* _table;       // reference to hash table being iterated
    size_t _index;    // current index into ht._entries

    // Method refs
    iterateToNext iter;
};

#endif //HASHTABLE_H
