/*
 *  vm_writer.h 
 */

#ifndef _VM_WRITER_H_
#define _VM_WRITER_H_

#include <stdio.h>

enum segment {
    SEG_CONST,
    SEG_ARG,
    SEG_LOCAL,
    SEG_STATIC,
    SEG_THIS,
    SEG_THAT,
    SEG_POINTER,
    SEG_TEMP,
    SEG_MAX,
};

enum command {
    COM_ADD,
    COM_SUB,
    COM_NEG,
    COM_EQ,
    COM_GT,
    COM_LT,
    COM_AND,
    COM_OR,
    COM_NOT,
    COM_MAX,
};


typedef struct vm_writer {
    FILE *fp;

    void (*init)(struct vm_writer *, char *);
    void (*writePush)(struct vm_writer *, enum segment, unsigned int);
    void (*writePop)(struct vm_writer *, enum segment, unsigned int);
    void (*writeArithmetic)(struct vm_writer *, enum command);
    void (*writeLabel)(struct vm_writer *, char *);
    void (*writeGoto)(struct vm_writer *, char *);
    void (*writeIf)(struct vm_writer *, char *);
    void (*writeCall)(struct vm_writer *, char *, unsigned int);
    void (*writeFunction)(struct vm_writer *, char *, unsigned int);
    void (*writeReturn)(struct vm_writer *);
    void (*close)(struct vm_writer *);
} VMWriter;

extern void _vm_writer_init(struct vm_writer *pThis, char *ostream);
extern void _vm_writer_writePush(struct vm_writer *pThis, enum segment segment, unsigned int index);
extern void _vm_writer_writePop(struct vm_writer *pThis, enum segment segment, unsigned int index);
extern void _vm_writer_writeArithmetic(struct vm_writer *pThis, enum command command);
extern void _vm_writer_writeLabel(struct vm_writer *pThis, char *label);
extern void _vm_writer_writeGoto(struct vm_writer *pThis, char *label);
extern void _vm_writer_writeIf(struct vm_writer *pThis, char *label);
extern void _vm_writer_writeCall(struct vm_writer *pThis, char *name, unsigned int nArgs);
extern void _vm_writer_writeFunction(struct vm_writer *pThis, char *name, unsigned int nLocals);
extern void _vm_writer_writeReturn(struct vm_writer *pThis);
extern void _vm_writer_close(struct vm_writer *pThis);

#define newVMWriter() {                            \
    .init            = _vm_writer_init,            \
    .writePush       = _vm_writer_writePush,       \
    .writePop        = _vm_writer_writePop,        \
    .writeArithmetic = _vm_writer_writeArithmetic, \
    .writeLabel      = _vm_writer_writeLabel,      \
    .writeGoto       = _vm_writer_writeGoto,       \
    .writeIf         = _vm_writer_writeIf,         \
    .writeCall       = _vm_writer_writeCall,       \
    .writeFunction   = _vm_writer_writeFunction,   \
    .writeReturn     = _vm_writer_writeReturn,     \
    .close           = _vm_writer_close,           \
}

#endif
