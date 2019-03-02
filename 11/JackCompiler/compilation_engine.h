/**
 * compilation_engine.h
 */

#ifndef _COMPILATION_ENGINE_H_
#define _COMPILATION_ENGINE_H_

#include "jack_tokenizer.h"
#include "symbol_table.h"
#include "vm_writer.h"
#include <stdio.h>

enum id_status {
    DEFINED,
    USED,
};


typedef struct compilation_engine {
    JackTokenizer tokenizer;
    SymbolTable   symbols;
    VMWriter      writer;
    FILE *fp;
    char *class_name;
    unsigned int nArgs;

    void (*init)(struct compilation_engine *, char *, char *);
    void (*compileClass)(struct compilation_engine *);
    void (*compileClassVarDec)(struct compilation_engine *);
    void (*compileSubroutine)(struct compilation_engine *);
    void (*compileParameterlist)(struct compilation_engine *);
    void (*compileVarDec)(struct compilation_engine *);
    void (*compileStatements)(struct compilation_engine *);
    void (*compileDo)(struct compilation_engine *);
    void (*compileLet)(struct compilation_engine *);
    void (*compileWhile)(struct compilation_engine *);
    void (*compileReturn)(struct compilation_engine *);
    void (*compileIf)(struct compilation_engine *);
    void (*compileExpression)(struct compilation_engine *);
    void (*compileTerm)(struct compilation_engine *);
    void (*compileExpressionList)(struct compilation_engine *);
    void (*del)(struct compilation_engine *);
} CompilationEngine;


extern void _compilation_engine_init(CompilationEngine *pThis, char *istream, char *ostream);
extern void _compilation_engine_compileClass(CompilationEngine *pThis);
extern void _compilation_engine_compileClassVarDec(CompilationEngine *pThis);
extern void _compilation_engine_compileSubroutine(CompilationEngine *pThis);
extern void _compilation_engine_compileParameterlist(CompilationEngine *pThis);
extern void _compilation_engine_compileVarDec(CompilationEngine *pThis);
extern void _compilation_engine_compileStatements(CompilationEngine *pThis);
extern void _compilation_engine_compileDo(CompilationEngine *pThis);
extern void _compilation_engine_compileLet(CompilationEngine *pThis);
extern void _compilation_engine_compileWhile(CompilationEngine *pThis);
extern void _compilation_engine_compileReturn(CompilationEngine *pThis);
extern void _compilation_engine_compileIf(CompilationEngine *pThis);
extern void _compilation_engine_compileExpression(CompilationEngine *pThis);
extern void _compilation_engine_compileTerm(CompilationEngine *pThis);
extern void _compilation_engine_compileExpressionList(CompilationEngine *pThis);
extern void _compilation_engine_del(CompilationEngine *pThis);


#define newCompilationEngine() {                                            \
    .tokenizer              = newJackTokenizer(),                           \
    .symbols                = newSymbolTable(),                             \
    .writer                 = newVMWriter(),                                \
    .fp                     = NULL,                                         \
    .init                   = _compilation_engine_init,                     \
    .compileClass           = _compilation_engine_compileClass,             \
    .compileClassVarDec     = _compilation_engine_compileClassVarDec,       \
    .compileSubroutine      = _compilation_engine_compileSubroutine,        \
    .compileParameterlist   = _compilation_engine_compileParameterlist,     \
    .compileVarDec          = _compilation_engine_compileVarDec,            \
    .compileStatements      = _compilation_engine_compileStatements,        \
    .compileDo              = _compilation_engine_compileDo,                \
    .compileLet             = _compilation_engine_compileLet,               \
    .compileWhile           = _compilation_engine_compileWhile,             \
    .compileReturn          = _compilation_engine_compileReturn,            \
    .compileIf              = _compilation_engine_compileIf,                \
    .compileExpression      = _compilation_engine_compileExpression,        \
    .compileTerm            = _compilation_engine_compileTerm,              \
    .compileExpressionList  = _compilation_engine_compileExpressionList,    \
    .del                    = _compilation_engine_del,                      \
}

#endif
