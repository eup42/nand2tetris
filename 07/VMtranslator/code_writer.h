/* 
 * code_writer.h
 */

#ifndef _CODE_WRITER_H_
#define _CODE_WRITER_H_

#include "command_type.h"
#include <stdio.h>

typedef struct CodeWriter {
    FILE *fp;
    void (*init)(struct CodeWriter*, char *);
    void (*setFileName)(struct CodeWriter *, char *);
    void (*writeArithmetric)(struct CodeWriter *, char *);
    void (*writePushPop)(struct CodeWriter *, enum commandType);
    void (*close)(struct CodeWriter *);
    void (*del)(struct CodeWriter *);
} CodeWriter;


extern void _code_writer_init(CodeWriter *pThis, char *filename);
extern void _code_writer_setFileName(CodeWriter *pThis, char *filename);
extern void _code_writer_writeArithmetric(CodeWriter *pThis, char *commnad);
extern void _code_writer_writePushPop(CodeWriter *pThis, enum commandType command, char *segment, int index);
extern void _code_writer_close(CodeWriter *pThis);
extern void _code_writer_del(CodeWriter *pThis);


#define newCodeWriter() {                               \
    .init             = _code_writer_init,              \
    .setFileName      = _code_writer_setFileName,       \
    .writeArithmetric = _code_writer_writeArithmetric   \
    .writePushPop     = _code_writer_writePushPop       \
    .close            = _code_writer_close              \
    .del              = _code_writer_del                \
}

#endif

