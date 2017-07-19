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

void _code_writer_writeArithmetric(CodeWriter *pThis, char *command)
{
    static const struct assemble_conv_list conv_list[]= {
        {"add", "M=M+D\n", write_binary_function_code},
        {"sub", "M=M-D\n", write_binary_function_code},
        {"neg", "M=-M\n",  write_unary_function_code},
        {"eq",  "JEQ\n",   write_compare_function_code},
        {"gt",  "JGT\n",   write_compare_function_code},
        {"lt",  "JLT\n",   write_compare_function_code},
        {"and", "M=M&D\n", write_binary_function_code},
        {"or",  "M=M|D\n", write_binary_function_code},
        {"not", "M=!M\n" , write_unary_function_code},
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
    char *template = "@SP\nA=M-1\n";

    fprintf(pThis->fp, "%s%s", template, assemble); 

    return;
}

static void write_binary_function_code(CodeWriter *pThis, const char *assemble)
{
    char *template_start = "@SP\nA=M-1\nD=M\nA=A-1\n";
    char *template_end   = "@SP\nM=M-1\n";

    fprintf(pThis->fp, "%s%s%s", template_start, assemble, template_end); 

    return;
}

static void write_compare_function_code(CodeWriter *pThis, const char *assemble)
{
    static unsigned long cnt = 0;

    char *template_start = "@SP\nA=M-1\nD=M\nA=A-1\nM=M-D\n";
    char *template_end   = "@SP\nM=M-1\n";

    fprintf(pThis->fp, "%s@TRUE%lu\nM;%s\nM=0\n@CONTINUE%lu\n0;JMP(TRUE%lu)\nM=-1\n(CONTINUE%lu)\n%s", template_start, cnt, assemble, cnt, cnt, cnt, template_end);

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
    char *template_end   = "D=A\n@SP\nA=M\nM=D\n@SP\nM=M+1\n";

    fprintf(pThis->fp, "@%d\n%s", index, template_end);

    return;
}
