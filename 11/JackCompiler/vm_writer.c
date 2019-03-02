/*
 * vm_writer.c
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "vm_writer.h"



void _vm_writer_init(struct vm_writer *pThis, char *ostream)
{
    pThis->fp = fopen(ostream, "w");
    setbuf(pThis->fp, NULL);
    if (pThis->fp == NULL) {
        fprintf(stderr, "%s %s() : %s\n", __FILE__, __FUNCTION__, strerror(errno));
        exit errno;
    }
}

void _vm_writer_writePush(struct vm_writer *pThis, enum segment segment, unsigned int index)
{
    const char *seg_name[] = {
        "constant", "argument", "local",   "static",
        "this",     "that",     "pointer", "temp",
    };

    fprintf(pThis->fp, "push %s %d\n", seg_name[segment], index);
}


void _vm_writer_writePop(struct vm_writer *pThis, enum segment segment, unsigned int index)
{
    const char *seg_name[] = {
        "constant", "argument", "local",   "static",
        "this",     "that",     "pointer", "temp",
    };

    fprintf(pThis->fp, "pop %s %d\n", seg_name[segment], index);
}


void _vm_writer_writeArithmetic(struct vm_writer *pThis, enum command command)
{
    const char *command_name[] = {
        "add", "sub", "neg", "eq", "gt",
        "lt",  "and", "or",  "not",
    };

    fprintf(pThis->fp, "%s\n", command_name[command]);

}


void _vm_writer_writeLabel(struct vm_writer *pThis, char *label)
{
    fprintf(pThis->fp, "label %s\n", label);
}


void _vm_writer_writeGoto(struct vm_writer *pThis, char *label)
{
    fprintf(pThis->fp, "goto %s\n", label);
}


void _vm_writer_writeIf(struct vm_writer *pThis, char *label)
{
    fprintf(pThis->fp, "if-goto %s\n", label);
}


void _vm_writer_writeCall(struct vm_writer *pThis, char *name, unsigned int nArgs)
{
    fprintf(pThis->fp, "call %s %d\n", name, nArgs);
}


void _vm_writer_writeFunction(struct vm_writer *pThis, char *name, unsigned int nLocals)
{
    fprintf(pThis->fp, "function %s %d\n", name, nLocals);
}


void _vm_writer_writeReturn(struct vm_writer *pThis)
{
    fprintf(pThis->fp, "return\n");
}


void _vm_writer_close(struct vm_writer *pThis) {
    fclose(pThis->fp);
}

