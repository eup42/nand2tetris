/*
 *  symbol_table.h 
 */

#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <stdio.h>

enum kind {
    ID_NONE,
    ID_STATIC,
    ID_FIELD,
    ID_ARG,
    ID_VAR,
    ID_CLASS,
    ID_SUBROUTINE,
    ID_MAX,
};

struct sym_table {
    char name[1024];
    char type[1024];
    enum kind kind;
    int  index;
};


typedef struct symbol_table {
    struct sym_table class[1024];
    struct sym_table sub[1024];
    int index[ID_MAX];

    void (*init)(struct symbol_table *);
    void (*startSubroutine)(struct symbol_table *);
    void (*define)(struct symbol_table *, char *, char *, enum kind);
    int  (*varCount)(struct symbol_table *, enum kind);
    enum kind (*kindOf)(struct symbol_table *, char *);
    char *(*typeOf)(struct symbol_table *, char *);
    int  (*indexOf)(struct symbol_table *, char *);
    void (*del)(struct symbol_table *);
} SymbolTable;

extern void _symbol_table_init(struct symbol_table *pThis);
extern void _symbol_table_startSubroutine(struct symbol_table *pThis);
extern void _symbol_table_define(struct symbol_table *pThis, char *name, char *type, enum kind kind);
extern int  _symbol_table_varCount(struct symbol_table *pThis, enum kind kind);
extern enum kind _symbol_table_kindOf(struct symbol_table *pThis, char *name);
extern char *_symbol_table_typeOf(struct symbol_table *pThis, char *name);
extern int  _symbol_table_indexOf(struct symbol_table *pThis, char *name);
extern void _symbol_table_del(struct symbol_table *pThis);

#define newSymbolTable() {                            \
    .init            = _symbol_table_init,            \
    .startSubroutine = _symbol_table_startSubroutine, \
    .define          = _symbol_table_define,          \
    .varCount        = _symbol_table_varCount,        \
    .kindOf          = _symbol_table_kindOf,          \
    .typeOf          = _symbol_table_typeOf,          \
    .indexOf         = _symbol_table_indexOf,         \
    .del             = _symbol_table_del,             \
}

#endif
