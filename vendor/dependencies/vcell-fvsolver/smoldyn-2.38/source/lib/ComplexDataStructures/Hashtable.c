//
// Created by Logan Drescher on 3/6/25,
// Licensed under the MIT LICENSE, copyright (https://opensource.org/license/mit).
// Adapted From Ben Hoyt et. all's work on GitHub (LICENSE = MIT) (https://github.com/benhoyt/ht/)
//
#include "Hashtable.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#ifndef HASHTABLE_INITIAL_CAPACITY
#define HASHTABLE_INITIAL_CAPACITY 16
#endif

#ifndef HASHTABLE_FNV_OFFSET
#define HASHTABLE_FNV_OFFSET 14695981039346656037UL
#endif

#ifndef HASHTABLE_FNV_PRIME
#define HASHTABLE_FNV_PRIME 1099511628211UL
#endif

// to allow for null-hashing, we need to both
// a) handle null cases, and
// b) append a small prefix to non-null keys to prevent "spoofing"
static const char* createAugmentedKey(const char* key) {
    if (key == NULL) return strdup("NULL"); // notice: no prefix; can't be spoofed
    size_t len = strlen(key);
    char* augmentedKey = (char*)calloc(len + 4, sizeof(char)); // + 1 for null-term, +3 for prefix
    if (augmentedKey == NULL) printf("UH-OH"), exit(-1);
    strcpy(augmentedKey, "HT_"); // Apply prefix
    strcpy(augmentedKey + 3, key); // append rest of original key
    return augmentedKey;
}

// Return 64-bit FNV-1a hash for key (NUL-terminated). See description:
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
// Note that an *augmented* key should be passed here!
static uint64_t hash_augKey(const char* augKey) {
    uint64_t hash = HASHTABLE_FNV_OFFSET;
    for (const char* p = augKey; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= HASHTABLE_FNV_PRIME;
    }
    return hash;
}

// Free memory allocated for hash table, including allocated keys, but not values (they are not copied-in like keys, we're not the owner!)
static void destroyHashtableImpl(Hashtable* table){
    // First free allocated keys. DON'T USE HASHTABLE_ITERATOR!
    for (size_t i = 0; i < table->_capacity; i++) {
        free((void*)table->entries[i].key);
    }

    // Then free entries array and table itself.
    free(table->entries);
    free(table);
}

// Check if item with given key (NUL-terminated) is inside hash table. Return
// true if contained, else false.
static bool checkIfHashtableContains(Hashtable* table, const char* key){
    // AND hash with capacity-1 to ensure it's within entries array.
    const char* augKey = createAugmentedKey(key);
    uint64_t hash = hash_augKey(augKey);

    // Loop till we find an end condition; AND hash with capacity-1 to ensure it's within entries array.
    for (size_t i = (size_t)(hash & (uint64_t)(table->_capacity - 1)); true; i = (i + 1) % table->_capacity) {
        // If we get an empty cell, the entry was never in the table in the first place (load factor is super important here; without it, this could be infinite)!
        if (table->entries[i].key == NULL) break;
        // Check if we got a cache hit.
        if (strcmp(augKey, table->entries[i].key) != 0) continue;
        free((void*)augKey);
        return true;
    }
    free((void*)augKey);
    return false;
}

// Get item with given key (NUL-terminated) from hash table. Return
// value (which was set with `setEntryInHashtable`), or assertion failure if key not found.
static void* getFromHashtableImpl(Hashtable* table, const char* key){
    // AND hash with capacity-1 to ensure it's within entries array.
    const char* augKey = createAugmentedKey(key);
    uint64_t hash = hash_augKey(augKey);

    // Loop till we find an end condition; AND hash with capacity-1 to ensure it's within entries array.
    for (size_t i = (size_t)(hash & (uint64_t)(table->_capacity - 1)); true; i = (i + 1) % table->_capacity) {
        // If we get an empty cell, the entry was never in the table in the first place (load factor is super important here; without it, this could be infinite)!
        if (table->entries[i].key == NULL) { // to allow for null-value hashing, we're returning null AND setting errno on misses
            errno = EINVAL;
            free((void*)augKey);
            return NULL;
        }
        // Check if we got a cache hit.
        if (strcmp(augKey, table->entries[i].key) != 0) continue;
        free((void*)augKey);
        return table->entries[i].value; // Found key, return value.
    }
    free((void*)augKey);
    return NULL; // We should never actually get here, but in case of accidents we'll put this here.
}

// Internal function to set an entry (without expanding table).
static const char* setEntryInHashtable(HashtableEntry* entries, size_t capacity,
        const char* augKey, void* value, size_t* plength) {
    // AND hash with capacity-1 to ensure it's within entries array.
    uint64_t hash = hash_augKey(augKey);
    size_t i;
    // Loop till we find an end condition; AND hash with capacity-1 to ensure it's within entries array.
    for (i = (size_t)(hash & (uint64_t)(capacity - 1)); true; i = (i + 1) % capacity) {
        // If we get an empty cell, the entry was never in the table in the first place (load factor is super important here; without it, this could be infinite)!
        if (entries[i].key == NULL) break;
        // Check if we got a cache hit.
        if (strcmp(augKey, entries[i].key) != 0) continue; // Miss
        // Hit!
        entries[i].key = augKey;
        entries[i].value = value; // Update value
        return augKey;
    }

    // Didn't find key, allocate+copy if needed, then insert it.
    if (plength != NULL) (*plength)++;
    entries[i].key = augKey;
    entries[i].value = value;
    return augKey;
}

// Expand hash table to twice its current size. Return true on success,
// false if out of memory.
static bool expandHashtable(Hashtable* table) {
    // Allocate new entries array.
    size_t new_capacity = table->_capacity * 2;
    if (new_capacity < table->_capacity) return false;  // overflow (capacity would be too big)

    HashtableEntry* new_entries = (HashtableEntry*)calloc(new_capacity, sizeof(HashtableEntry));
    if (new_entries == NULL) return false;

    // Iterate entries, move all non-empty ones to new table's entries.
    for (size_t i = 0; i < table->_capacity; i++) {
        HashtableEntry entry = table->entries[i];
        if (entry.key == NULL) continue;
        setEntryInHashtable(new_entries, new_capacity, entry.key, entry.value, NULL); // Note: Entry keys are already augmented!
    }

    // Free old entries array and update this table's details.
    free(table->entries);
    table->entries = new_entries;
    table->_capacity = new_capacity;
    return true;
}


// Set item with given key (NUL-terminated) to value.
// If not already present in table, key is augmented and copied to newly
// allocated memory (keys are freed automatically when `destroyHashtable` is
// called). Return address of copied key, or NULL if out of memory.
static const char* putInHashtableImpl(Hashtable* table, const char* key, void* value){
    // If length will exceed half of current capacity, expand it.
    if (table->_length >= table->_capacity / 2 && !expandHashtable(table)) return NULL; // Don't worry, this short-circuits!
    const char* augKey = createAugmentedKey(key);
    setEntryInHashtable(table->entries, table->_capacity, augKey, value, &table->_length); // Set entry and update length.
    return key;
}

// Return number of items in hash table.
static size_t getHashtableLengthImpl(Hashtable* table){
    return table->_length;
}

///////////////// HashtableIterator /////////////////
// Move iterator to next item in hash table, update iterator's key
// and value to current item, and return true. If there are no more
// items, return false. Don't call ht_set during iteration.
static bool iterateToNextImpl(HashtableIterator* it){
    // Loop till we've hit end of entries array.
    Hashtable* table = it->_table;
    while (it->_index < table->_capacity) {
        size_t i = it->_index;
        it->_index++;
        if (table->entries[i].key != NULL) {
            // Found next non-empty item, update iterator key and value.
            HashtableEntry entry = table->entries[i];
            it->key = 0 == strcmp(entry.key, "NULL") ? NULL : entry.key + 3; // de-augment key
            it->value = entry.value;
            return true;
        }
    }
    return false;
}

// Return new hash table iterator (for use with ht_next).
static HashtableIterator generateHashtableIteratorImpl(Hashtable* table){
    HashtableIterator it;
    it._table = table;
    it._index = 0;
    it.iter = iterateToNextImpl;
    it.key = NULL;
    it.value = NULL;
    return it;
}
///////////////// ///////////////// /////////////////

// Create hash table and return pointer to it, or NULL if out of memory.
Hashtable* createNewHashtable(void){
    // Allocate space for hash table struct.
    Hashtable* table = (Hashtable*)calloc(1, sizeof(Hashtable));
    if (table == NULL) return NULL;

    table->_length = 0;
    table->_capacity = HASHTABLE_INITIAL_CAPACITY;

    // Allocate (zero'd) space for entry buckets.
    table->entries = (HashtableEntry*)calloc(table->_capacity, sizeof(HashtableEntry));
    if (table->entries == NULL) {
        free(table); // error, free table before we return!
        return NULL;
    }
    table->freeThis = destroyHashtableImpl;
    table->contains = checkIfHashtableContains;
    table->getFrom = getFromHashtableImpl;
    table->putIn = putInHashtableImpl;
    table->size = getHashtableLengthImpl;
    table->getIterator = generateHashtableIteratorImpl;
    return table;
}

