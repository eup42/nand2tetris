/*
 * symbol_table.c
 */

#include <stdio.h>
#include <string.h>
#include "symbol_table.h"

static struct sym_table *search_identification(struct symbol_table *pThis, char *name);

void _symbol_table_init(struct symbol_table *pThis)
{
    memset(pThis->class, 0, sizeof(pThis->class));
    memset(pThis->sub, 0, sizeof(pThis->sub));
    memset(pThis->index, 0, sizeof(pThis->index));

    return;
}

void _symbol_table_startSubroutine(struct symbol_table *pThis)
{
    pThis->index[ID_ARG] = 0;
    pThis->index[ID_VAR] = 0;
    memset(pThis->sub, 0, sizeof(pThis->sub));

    return;
}

void _symbol_table_define(struct symbol_table *pThis, char *name, char *type, enum kind kind)
{
    struct sym_table *row = NULL;

    switch (kind) {
        case ID_STATIC:
        case ID_FIELD:
            row = &pThis->class[pThis->index[kind]];
            break;

        case ID_ARG:
        case ID_VAR:
            row = &pThis->sub[pThis->index[kind]];
            break;

        default:
            break;
    }

    strncpy(row->name, name, sizeof(row->name) - 1);
    row->name[sizeof(row->name) - 1] = '\0';
    strncpy(row->type, type, sizeof(row->type) - 1);
    row->type[sizeof(row->type) - 1] = '\0';
    row->kind = kind;
    row->index = pThis->index[kind];

    pThis->index[kind]++;

}

int  _symbol_table_varCount(struct symbol_table *pThis, enum kind kind)
{
    return pThis->index[kind];
}

enum kind _symbol_table_kindOf(struct symbol_table *pThis, char *name)
{
    struct sym_table *row;

    row = search_identification(pThis, name);

    if (row)
        return row->kind; 
    else
        return ID_NONE;
}

char *_symbol_table_typeOf(struct symbol_table *pThis, char *name)
{
    struct sym_table *row;

    row = search_identification(pThis, name);

    if (row)
        return row->type; 
    else
        return NULL;
}

int  _symbol_table_indexOf(struct symbol_table *pThis, char *name)
{
    struct sym_table *row;

    row = search_identification(pThis, name);

    if (row)
        return row->index; 
    else
        return 0;
}

void _symbol_table_del(struct symbol_table *pThis)
{
    memset(pThis->class, 0, sizeof(pThis->class));
    memset(pThis->sub, 0, sizeof(pThis->sub));
    memset(pThis->index, 0, sizeof(pThis->index));

    return;
}

static struct sym_table *search_identification(struct symbol_table *pThis, char *name)
{
    size_t i;
    struct sym_table *table;

    table = pThis->sub;

    for (i = 0; i < sizeof(table) || strlen(table[i].name) != 0; i++)
        if (!strcmp(table[i].name, name))
            return &(table[i]);

    table = pThis->class;

    for (i = 0; i < sizeof(table) || strlen(table[i].name) != 0; i++)
        if (!strcmp(table[i].name, name))
            return &(table[i]);

    return NULL;

}
