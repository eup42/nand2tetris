/* 
 * code_writer.h
 */

#ifndef _CODE_WRITER_H_
#define _CODE_WRITER_H_

#include "command_type.h"
#include <stdio.h>

typedef struct CodeWriter {
    FILE *fp;
    char *filename;
    char *funcname;
    void (*init)(struct CodeWriter*, char *);
    void (*setFileName)(struct CodeWriter *, char *);
    void (*writeArithmetric)(struct CodeWriter *, char *);
    void (*writePushPop)(struct CodeWriter *, enum commandType, char *, int);
    void (*writeLabel)(struct CodeWriter *, char *label);
    void (*writeGoto)(struct CodeWriter *, char *label);
    void (*writeIf)(struct CodeWriter *, char *label);
    void (*writeInit)(struct CodeWriter *pThis);
    void (*writeCall)(struct CodeWriter *pThis, char *functionName, int numArgs);
    void (*writeReturn)(struct CodeWriter *pThis);
    void (*writeFunction)(struct CodeWriter *pThis, char *functionName, int numArgs);
    void (*close)(struct CodeWriter *);
    void (*del)(struct CodeWriter *);
} CodeWriter;


extern void _code_writer_init(CodeWriter *pThis, char *filename);
extern void _code_writer_setFileName(CodeWriter *pThis, char *filename);
extern void _code_writer_writeArithmetric(CodeWriter *pThis, char *commnad);
extern void _code_writer_writePushPop(CodeWriter *pThis, enum commandType command, char *segment, int index);
extern void _code_writer_writeLabel(CodeWriter *pThis, char *label);
extern void _code_writer_writeGoto(CodeWriter *pThis, char *label);
extern void _code_writer_writeIf(CodeWriter *pThis, char *label);
extern void _code_writer_writeInit(CodeWriter *pThis);
extern void _code_writer_writeCall(CodeWriter *pThis, char *functionName, int numArgs);
extern void _code_writer_writeReturn(CodeWriter *pThis);
extern void _code_writer_writeFunction(CodeWriter *pThis, char *functionName, int numArgs);
extern void _code_writer_close(CodeWriter *pThis);
extern void _code_writer_del(CodeWriter *pThis);


#define newCodeWriter() {                               \
    .fp               = NULL,                           \
    .filename         = NULL,                           \
    .funcname         = NULL,                           \
    .init             = _code_writer_init,              \
    .setFileName      = _code_writer_setFileName,       \
    .writeArithmetric = _code_writer_writeArithmetric,  \
    .writePushPop     = _code_writer_writePushPop,      \
    .writeLabel       = _code_writer_writeLabel,        \
    .writeGoto        = _code_writer_writeGoto,         \
    .writeIf          = _code_writer_writeIf,           \
    .writeInit        = _code_writer_writeInit,         \
    .writeCall        = _code_writer_writeCall,         \
    .writeReturn      = _code_writer_writeReturn,       \
    .writeFunction    = _code_writer_writeFunction,     \
    .close            = _code_writer_close,             \
    .del              = _code_writer_del,               \
}

#endif

