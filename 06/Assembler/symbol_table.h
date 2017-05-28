/*
 * hash.h
 */

#ifndef _HASH_H_
#define _HASH_H_

#include <stdbool.h>
#include <stdint.h>

extern void _hash_initSymbolTable(void);
extern void _hash_addEntry(char *key, uint16_t value);
extern bool _hash_contains(char *key);
extern uint16_t _hash_getAddress(char *key);
extern void _hash_destroyHashTable(void);

const static struct hash {
    void (*initSymbolTable)(void);
    void (*addEntry)(char *, uint16_t);
    bool (*contains)(char *);
    uint16_t (*getAddress)(char *);
    void (*destroyHashTable)(void);
} hash = {
    .initSymbolTable = _hash_initSymbolTable,
    .addEntry = _hash_addEntry,
    .contains = _hash_contains,
    .getAddress = _hash_getAddress,
    .destroyHashTable = _hash_destroyHashTable,
};

#endif

