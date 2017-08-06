/*
 * code_writer.c
 */


#include "code_writer.h"
#include <stdlib.h>
#include <string.h>

struct assemble_conv_list {
    char *command;
    char *assemble;
    void (*write_code_template)(CodeWriter *, const char *);
};

struct push_pop_conv_list {
    char *segment;
    void (*write_code_template)(CodeWriter *, int);
};

static void write_unary_function_code(CodeWriter *pThis, const char *assemble);
static void write_binary_function_code(CodeWriter *pThis, const char *assemble);
static void write_compare_function_code(CodeWriter *pThis, const char *assemble);
static void write_push_code(CodeWriter *pThis, const char *segment, int index);
static void write_push_constant(CodeWriter *pThis, int index);

void _code_writer_init(CodeWriter *pThis, char *filename)
{
    pThis->fp = fopen(filename, "w");
}

void _code_writer_setFileName(CodeWriter *pThis, char *filename)
{
    return;
}

void _code_writer_writeArithmetric(CodeWriter *pThis, char *command)
{
    static const struct assemble_conv_list conv_list[]= {
        {"add", "M=M+D", write_binary_function_code},
        {"sub", "M=M-D", write_binary_function_code},
        {"neg", "M=-M",  write_unary_function_code},
        {"eq",  "JEQ",   write_compare_function_code},
        {"gt",  "JGT",   write_compare_function_code},
        {"lt",  "JLT",   write_compare_function_code},
        {"and", "M=M&D", write_binary_function_code},
        {"or",  "M=M|D", write_binary_function_code},
        {"not", "M=!M" , write_unary_function_code},
        {NULL,  NULL,      NULL},
    };

    int i;

    for (i = 0; conv_list[i].command != NULL; i++) {
        if (!strcmp(command, conv_list[i].command)) {
            conv_list[i].write_code_template(pThis, conv_list[i].assemble);
        }
    }
}

void _code_writer_writePushPop(CodeWriter *pThis, enum commandType command, char *segment, int index)
{
    if (segment == NULL) return;

    switch (command) {
        case C_PUSH:
            write_push_code(pThis, segment, index);
            break;
        case C_POP:
            break;
        default:
            break;
    }

    return;
}

void _code_writer_close(CodeWriter *pThis)
{
    fclose(pThis->fp);
}

void _code_writer_del(CodeWriter *pThis)
{
    pThis->fp = NULL;
    return;
}


static void write_unary_function_code(CodeWriter *pThis, const char *assemble)
{
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "A=M-1\n");
    fprintf(pThis->fp, "%s\n", assemble);

    return;
}

static void write_binary_function_code(CodeWriter *pThis, const char *assemble)
{
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "AM=M-1\n");         // --SP
    fprintf(pThis->fp, "D=M\n");            // D = M[SP]
    fprintf(pThis->fp, "A=A-1\n");
    fprintf(pThis->fp, "%s\n", assemble);

    return;
}

static void write_compare_function_code(CodeWriter *pThis, const char *assemble)
{
    static unsigned long cnt = 0;

    fprintf(pThis->fp, "@SP\n");                    // --SP
    fprintf(pThis->fp, "AM=M-1\n");
    fprintf(pThis->fp, "D=M\n");                    // D=M[SP]
    fprintf(pThis->fp, "A=A-1\n");
    fprintf(pThis->fp, "D=M-D\n");                  // M[SP-1] - D
    fprintf(pThis->fp, "@TRUE%lu\n", cnt);
    fprintf(pThis->fp, "D;%s\n", assemble);
    fprintf(pThis->fp, "@SP\n");                    // if false
    fprintf(pThis->fp, "A=M-1\n");
    fprintf(pThis->fp, "M=0\n");                    // M[SP-1] = 0
    fprintf(pThis->fp, "@CONTINUE%lu\n", cnt);      // go to end if
    fprintf(pThis->fp, "0;JMP\n");
    fprintf(pThis->fp, "(TRUE%lu)\n", cnt);         // if true
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "A=M-1\n");
    fprintf(pThis->fp, "M=-1\n");                   // M[SP-1] = -1
    fprintf(pThis->fp, "(CONTINUE%lu)\n", cnt);     // end if

    cnt++;
    return;
}

static void write_push_code(CodeWriter *pThis, const char *segment, int index)
{
    static const struct push_pop_conv_list conv_list[] = {
        {"constant", write_push_constant},
        {NULL,       NULL},
    };

    int i;

    for (i = 0; conv_list[i].segment != NULL; i++) {
        if (!strcmp(segment, conv_list[i].segment)) {
            conv_list[i].write_code_template(pThis, index);
        }
    }

    return;
}

static void write_push_constant(CodeWriter *pThis, int index)
{
    fprintf(pThis->fp, "@%d\n", index);
    fprintf(pThis->fp, "D=A\n");            // D = index
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "A=M\n");
    fprintf(pThis->fp, "M=D\n");            // M[SP] = D
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "M=M+1\n");          // ++SP

    return;
}
