/*
 * code_writer.c
 */


#include "code_writer.h"
#include <stdlib.h>
#include <string.h>

#define SIZE_OF_ARRAY(a) ((sizeof(a)) / (sizeof(a[0])))

struct assemble_conv_list {
    char *command;
    char *assemble;
    void (*write_code_template)(CodeWriter *, const char *);
};

struct push_pop_conv_list {
    char *segment;
    char *label;
    void (*write_code_template)(CodeWriter *, const char *, int);
};

static void write_unary_function_code(CodeWriter *pThis, const char *assemble);
static void write_binary_function_code(CodeWriter *pThis, const char *assemble);
static void write_compare_function_code(CodeWriter *pThis, const char *assemble);

static void write_push_code(CodeWriter *pThis, const char *segment, int index);
static void write_push_constant(CodeWriter *pThis, const char *regs, int index);
static void write_push_with_base_regs(CodeWriter *pThis, const char *regs, int index);
static void write_push_with_base_addr(CodeWriter *pThis, const char *addr, int index);
static void write_push_static(CodeWriter *pThis, const char *regs, int index);

static void write_pop_code(CodeWriter *pThis, const char *segment, int index);
static void write_pop_with_base_regs(CodeWriter *pThis, const char *regs, int index);
static void write_pop_with_base_addr(CodeWriter *pThis, const char *addr, int index);
static void write_pop_static(CodeWriter *pThis, const char *addr, int index);

void _code_writer_init(CodeWriter *pThis, char *filename)
{
    pThis->fp = fopen(filename, "w");
}

void _code_writer_setFileName(CodeWriter *pThis, char *filename)
{
    free(pThis->filename);

    pThis->filename = (char *)malloc(sizeof(char) * strlen(filename) + 1);

    strcpy(pThis->filename, filename);
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
            write_pop_code(pThis, segment, index);
            break;
        default:
            break;
    }

    return;
}


void _code_writer_writeLabel(CodeWriter *pThis, char *label)
{
    char *func = "null";

    if (pThis->funcname)
        func = pThis->funcname;

    fprintf(pThis->fp, "(%s$%s)\n", func, label);

    return;
}

void _code_writer_writeGoto(CodeWriter *pThis, char *label)
{
    char *func = "null";

    if (pThis->funcname)
        func = pThis->funcname;

    fprintf(pThis->fp, "@%s$%s\n", func, label);
    fprintf(pThis->fp, "0;JMP\n");

    return;
}

void _code_writer_writeIf(CodeWriter *pThis, char *label)
{
    char *func = "null";

    if (pThis->funcname)
        func = pThis->funcname;

    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "AM=M-1\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@%s$%s\n", func, label);
    fprintf(pThis->fp, "D;JNE\n");

    return;
}

void _code_writer_writeInit(CodeWriter *pThis)
{
    fprintf(pThis->fp, "@256\n");                   // SP = 256
    fprintf(pThis->fp, "D=A\n");
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "M=D\n");

    _code_writer_writeCall(pThis, "Sys.init", 0);   //call Sys.init
}

void _code_writer_writeCall(CodeWriter *pThis, char *functionName, int numArgs)
{
    char *push_list[] = {"return-address", "LCL", "ARG", "THIS", "THAT"};
    static int ret_num = 0;
    int i;

    for (i = 0; i < SIZE_OF_ARRAY(push_list); i++) {                // each label in push_list push to stack
        if (!strcmp(push_list[i], "return-address")) {
            fprintf(pThis->fp, "@%s%d\n", push_list[i], ret_num);
            fprintf(pThis->fp, "D=A\n");
        } else {
            fprintf(pThis->fp, "@%s\n", push_list[i]);
            fprintf(pThis->fp, "D=M\n");
        }

        fprintf(pThis->fp, "@SP\n");
        fprintf(pThis->fp, "A=M\n");
        fprintf(pThis->fp, "M=D\n");
        fprintf(pThis->fp, "@SP\n");
        fprintf(pThis->fp, "M=M+1\n");
    }

    fprintf(pThis->fp, "@SP\n");                                    // ARG = SP - n - 5
    fprintf(pThis->fp, "D=M\n");
    for (i = 0; i < numArgs + 5; i++) {
        fprintf(pThis->fp, "D=D-1\n");
    }
    fprintf(pThis->fp, "@ARG\n");
    fprintf(pThis->fp, "M=D\n");

    fprintf(pThis->fp, "@SP\n");                                    // LCL = SP
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@LCL\n");
    fprintf(pThis->fp, "M=D\n");

    fprintf(pThis->fp, "@%s\n", functionName);                      // goto f
    fprintf(pThis->fp, "0;JMP\n");

    fprintf(pThis->fp, "(return-address%d)\n", ret_num);            // (return-addressXX)

    ret_num++;

    return;
}

void _code_writer_writeReturn(CodeWriter *pThis)
{
    int i;

    // R13: FRAME
    // R14: RET

    fprintf(pThis->fp, "@LCL\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@R13\n");
    fprintf(pThis->fp, "M=D\n");        // FRAME = LCL

    for (i = 5; i > 0; i--) {
        fprintf(pThis->fp, "D=D-1\n");
    }
    fprintf(pThis->fp, "A=D\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@R14\n");
    fprintf(pThis->fp, "M=D\n");        // RET = *(FRAME - 5)

    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "AM=M-1\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@ARG\n");
    fprintf(pThis->fp, "A=M\n");
    fprintf(pThis->fp, "M=D\n");        // *ARG = pop()

    fprintf(pThis->fp, "@ARG\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "M=D+1\n");      // SP = ARG + 1

    fprintf(pThis->fp, "@R13\n");
    fprintf(pThis->fp, "AM=M-1\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@THAT\n");
    fprintf(pThis->fp, "M=D\n");        // THAT = *(FRAME - 1)

    fprintf(pThis->fp, "@R13\n");
    fprintf(pThis->fp, "AM=M-1\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@THIS\n");
    fprintf(pThis->fp, "M=D\n");        // THIS = *(FRAME - 2)

    fprintf(pThis->fp, "@R13\n");
    fprintf(pThis->fp, "AM=M-1\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@ARG\n");
    fprintf(pThis->fp, "M=D\n");        // ARG = *(FRAME - 3)

    fprintf(pThis->fp, "@R13\n");
    fprintf(pThis->fp, "AM=M-1\n");
    fprintf(pThis->fp, "D=M\n");
    fprintf(pThis->fp, "@LCL\n");
    fprintf(pThis->fp, "M=D\n");        // LCL = *(FRAME - 4)

    fprintf(pThis->fp, "@R14\n");
    fprintf(pThis->fp, "A=M\n");
    fprintf(pThis->fp, "0;JMP\n");      // goto RET
}

void _code_writer_writeFunction(CodeWriter *pThis, char *functionName, int numArgs)
{
    int i;

    pThis->funcname = functionName;

    fprintf(pThis->fp, "(%s)\n", functionName);     // (f)
    for (i = 0; i < numArgs; i++) {
        fprintf(pThis->fp, "@SP\n");
        fprintf(pThis->fp, "A=M\n");
        fprintf(pThis->fp, "M=0\n");                // M[SP] = 0
        fprintf(pThis->fp, "@SP\n");
        fprintf(pThis->fp, "M=M+1\n");              // ++SP
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
    free(pThis->filename);
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
        {"constant", NULL,   write_push_constant},
        {"local",    "LCL",  write_push_with_base_regs},
        {"argument", "ARG",  write_push_with_base_regs},
        {"this",     "THIS", write_push_with_base_regs},
        {"that",     "THAT", write_push_with_base_regs},
        {"pointer",  "3",    write_push_with_base_addr},
        {"temp",     "5",    write_push_with_base_addr},
        {"static",   NULL,   write_push_static},
        {NULL,       NULL,   NULL},
    };

    int i;

    for (i = 0; conv_list[i].segment != NULL; i++) {
        if (!strcmp(segment, conv_list[i].segment)) {
            conv_list[i].write_code_template(pThis, conv_list[i].label, index);
        }
    }

    return;
}

static void write_push_constant(CodeWriter *pThis, const char *regs, int index)
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

static void write_push_with_base_regs(CodeWriter *pThis, const char *regs, int index)
{
    fprintf(pThis->fp, "@%s\n", regs);
    fprintf(pThis->fp, "D=M\n");            // D = base
    fprintf(pThis->fp, "@%d\n", index);
    fprintf(pThis->fp, "A=D+A\n");          // A = base + index
    fprintf(pThis->fp, "D=M\n");            // D = M[base + index]
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "A=M\n");
    fprintf(pThis->fp, "M=D\n");            // M[SP] = D
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "M=M+1\n");          // ++SP

    return;
}

static void write_push_with_base_addr(CodeWriter *pThis, const char *addr, int index)
{
    fprintf(pThis->fp, "@%d\n", atoi(addr) + index);
    fprintf(pThis->fp, "D=M\n");            // D = M[3 + index]
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "A=M\n");
    fprintf(pThis->fp, "M=D\n");            // M[SP] = D
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "M=M+1\n");          // ++SP

    return;
}

static void write_push_static(CodeWriter *pThis, const char *addr, int index)
{
    fprintf(pThis->fp, "@%s.%d\n", pThis->filename, index);
    fprintf(pThis->fp, "D=M\n");            // D = M[3 + index]
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "A=M\n");
    fprintf(pThis->fp, "M=D\n");            // M[SP] = D
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "M=M+1\n");          // ++SP

    return;
}

static void write_pop_code(CodeWriter *pThis, const char *segment, int index)
{
    static const struct push_pop_conv_list conv_list[] = {
        {"local",    "LCL",  write_pop_with_base_regs},
        {"argument", "ARG",  write_pop_with_base_regs},
        {"this",     "THIS", write_pop_with_base_regs},
        {"that",     "THAT", write_pop_with_base_regs},
        {"pointer",  "3",    write_pop_with_base_addr},
        {"temp",     "5",    write_pop_with_base_addr},
        {"static",   NULL,   write_pop_static},
        {NULL,       NULL,   NULL},
    };

    int i;

    for (i = 0; conv_list[i].segment != NULL; i++) {
        if (!strcmp(segment, conv_list[i].segment)) {
            conv_list[i].write_code_template(pThis, conv_list[i].label, index);
        }
    }

    return;
}

static void write_pop_with_base_regs(CodeWriter *pThis, const char *regs, int index)
{
    fprintf(pThis->fp, "@%s\n", regs);
    fprintf(pThis->fp, "D=M\n");            // D = base
    fprintf(pThis->fp, "@%d\n", index);
    fprintf(pThis->fp, "D=D+A\n");          // D = base + index
    fprintf(pThis->fp, "@R13\n");
    fprintf(pThis->fp, "M=D\n");            // M[R13] = D

    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "AM=M-1\n");         // --SP
    fprintf(pThis->fp, "D=M\n");            // D = M[SP]

    fprintf(pThis->fp, "@R13\n");
    fprintf(pThis->fp, "A=M\n");            // A = base + index
    fprintf(pThis->fp, "M=D\n");            // M[base + index] = D

    return;
}

static void write_pop_with_base_addr(CodeWriter *pThis, const char *addr, int index)
{
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "AM=M-1\n");         // --SP
    fprintf(pThis->fp, "D=M\n");            // D = M[SP]

    fprintf(pThis->fp, "@%d\n", atoi(addr) + index);
    fprintf(pThis->fp, "M=D\n");            // M[base + index] = D

    return;
}

static void write_pop_static(CodeWriter *pThis, const char *addr, int index)
{
    fprintf(pThis->fp, "@SP\n");
    fprintf(pThis->fp, "AM=M-1\n");         // --SP
    fprintf(pThis->fp, "D=M\n");            // D = M[SP]

    fprintf(pThis->fp, "@%s.%d\n", pThis->filename, index);
    fprintf(pThis->fp, "M=D\n");            // M[base + index] = D

    return;
}
