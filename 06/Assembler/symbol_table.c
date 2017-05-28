/*
 * hash.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"

#define HASH_TABLE_BLOCK_SIZE 1024

struct entry {
    char *key;
    uint16_t value;
};

static struct data {
    struct entry **hash_table;
    unsigned int entry_num;
    unsigned int block_num;
} data = {
    .hash_table = NULL,
    .entry_num = 0,
    .block_num = 1,
};

void _hash_initSymbolTable(void)
{
    data.hash_table
        = (struct entry **)malloc(sizeof(struct entry)
                                    * HASH_TABLE_BLOCK_SIZE * data.block_num);
}

void _hash_addEntry(char *key, uint16_t value)
{
    data.hash_table[data.entry_num] = (struct entry *)malloc(sizeof(struct entry));
    data.hash_table[data.entry_num]->key   = (char *)malloc(strlen(key) + 1);

    strncpy(data.hash_table[data.entry_num]->key, key, strlen(key) + 1);
    data.hash_table[data.entry_num]->value = value;

    data.entry_num++;

    if (data.entry_num == HASH_TABLE_BLOCK_SIZE * data.block_num) {
        data.block_num++;
        data.hash_table
            = (struct entry **)realloc(data.hash_table,
                                        sizeof(struct entry)
                                         * HASH_TABLE_BLOCK_SIZE * data.block_num);
    }
}

bool _hash_contains(char *key) {
    int i;

    for (i = 0; i < data.entry_num; i++)
        if(!strcmp(key, data.hash_table[i]->key)) return true;

    return false;
}

uint16_t _hash_getAddress(char *key) {
    int i;

    for (i = 0; i < data.entry_num; i++)
        if(!strcmp(key, data.hash_table[i]->key))
            return data.hash_table[i]->value;

    printf("Error: key is not found.\n");
    return 0;
}

void _hash_destroyHashTable(void) 
{
    int i;

    for (i = 0; i < data.entry_num; i++) {
        free(data.hash_table[i]->key);
        free(data.hash_table[i]);
    }

    free(data.hash_table);
}
