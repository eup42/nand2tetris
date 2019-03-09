/**
 * compilation_engine.c
 */

#include "compilation_engine.h"
#include "common.h"
#include "symbol_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define TOKEN_ERR(token) do {fprintf(stderr, "%s %s() : error %s is expected\n", __FILE__, __FUNCTION__, token); } while (0)

static bool is_type(JackTokenizer *pThis);
static bool is_begin_of_term(JackTokenizer *pThis);
static bool is_op(JackTokenizer *pThis);
static bool is_begin_of_expression(JackTokenizer *pThis);
static void advance(JackTokenizer *p_tokenizer);


void _compilation_engine_init(CompilationEngine *pThis, char *istream, char *ostream)
{
    pThis->tokenizer.init(&pThis->tokenizer, istream);
    pThis->symbols.init(&pThis->symbols);
    pThis->writer.init(&pThis->writer, ostream);

    pThis->class_name = malloc(sizeof(char) * 1024);
}


void _compilation_engine_compileClass(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    // 'class'
    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                && (p_tokenizer->keyWord(p_tokenizer) == CLASS))) {
        TOKEN_ERR("class");
        exit (0);
    }
    advance(p_tokenizer);


    // className
    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        exit (0);
    }
    strcpy(pThis->class_name, p_tokenizer->current_token);
    advance(p_tokenizer);


    // '{'
    if (!(p_tokenizer->tokenType(p_tokenizer) == SYMBOL && !strcmp(p_tokenizer->symbol(p_tokenizer), "{"))) {
        TOKEN_ERR("{");
        exit (0);
    }
    advance(p_tokenizer);


    // classVarDec*
    while ((p_tokenizer->tokenType(p_tokenizer) == KEYWORD) &&
        ((p_tokenizer->keyWord(p_tokenizer) == STATIC) ||
        (p_tokenizer->keyWord(p_tokenizer) == FIELD)))
        pThis->compileClassVarDec(pThis);

    // subroutineDec*
    while ((p_tokenizer->tokenType(p_tokenizer) == KEYWORD) &&
        ((p_tokenizer->keyWord(p_tokenizer) == CONSTRUCTOR) ||
        (p_tokenizer->keyWord(p_tokenizer) == FUNCTION) ||
        (p_tokenizer->keyWord(p_tokenizer) == METHOD))) {
        pThis->compileSubroutine(pThis);
    }

    // '}'
    if (!(p_tokenizer->tokenType(p_tokenizer) == SYMBOL && !strcmp(p_tokenizer->symbol(p_tokenizer), "}"))) {
        TOKEN_ERR("}");
        exit (0);
    }
}

void _compilation_engine_compileClassVarDec(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    enum kind kind;
    char *type;

    type = malloc(sizeof(char) * 1024);

    // ('static' | 'field')
    if (!(p_tokenizer->tokenType(p_tokenizer) == KEYWORD)) {
        TOKEN_ERR("keyword");
        exit (0);
    }
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case STATIC:
            kind = ID_STATIC;
            advance(p_tokenizer);
            break;
        case FIELD:
            kind = ID_FIELD;
            advance(p_tokenizer);
            break;
        default:
            TOKEN_ERR("'static' or 'field'");
    }

    // type
    if (p_tokenizer->tokenType(p_tokenizer) == KEYWORD) {
        switch (p_tokenizer->keyWord(p_tokenizer)) {
            case INT:
                strcpy(type, "int");
                break;
            case CHAR:
                strcpy(type, "char");
                break;
            case BOOLEAN:
                strcpy(type, "boolean");
                break;
            default:
                TOKEN_ERR("'int' or 'char' or 'boolean'");
                exit (0);
        }
    } else if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER) {
        strcpy(type, p_tokenizer->current_token);
    } else {
        TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
        exit (0);
    }
    advance(p_tokenizer);

    // varName
    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        exit (0);
    }
    pThis->symbols.define(&(pThis->symbols), p_tokenizer->current_token, type, kind);
    advance(p_tokenizer);

    // (',' varName)*
    while ((p_tokenizer->tokenType(p_tokenizer) == SYMBOL) &&
        (!strcmp(p_tokenizer->symbol(p_tokenizer), ","))) {
        advance(p_tokenizer);

        if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
            TOKEN_ERR("identifier");
            exit (0);
        }
        pThis->symbols.define(&(pThis->symbols), p_tokenizer->current_token, type, kind);
        advance(p_tokenizer);
    }

    // ';'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), ";"))) {
        TOKEN_ERR(";");
        exit (0);
    }
    advance(p_tokenizer);

    free(type);
}

void _compilation_engine_compileSubroutine(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    enum KeyWord subroutine;
    char *vm_func_name;

    vm_func_name = malloc(sizeof(char) * 1024);

    pThis->symbols.startSubroutine(&pThis->symbols);

    // ('constructor' | 'function' | 'method')
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case CONSTRUCTOR:
        case FUNCTION:
        case METHOD:
            subroutine = p_tokenizer->keyWord(p_tokenizer);
            break;
        default:
            TOKEN_ERR("'constructor' or 'function' or 'method'");
            exit(0);
    }
    advance(p_tokenizer);

    // ('void' or type)
    pThis->ret_type = MAX_KEYWORD;  // prevent from remaining past value
    if (p_tokenizer->tokenType(p_tokenizer) == KEYWORD) {
        switch (p_tokenizer->keyWord(p_tokenizer)) {
            case VOID:
                pThis->ret_type = VOID;
                break;
            case INT:
            case CHAR:
            case BOOLEAN:
                break;
            default:
                TOKEN_ERR("'int' or 'char' or 'boolean'");
                exit (0);
        }
    } else if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER) {
        // nop
    } else {
        TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
        exit (0);
    }
    advance(p_tokenizer);

    // subroutineName
    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        exit (0);
    }
    sprintf(vm_func_name, "%s.%s", pThis->class_name, p_tokenizer->current_token);
    advance(p_tokenizer);

    // '('
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
        TOKEN_ERR("(");
        exit (0);
    }
    advance(p_tokenizer);

    // parameterList
    if (subroutine == METHOD)
        pThis->symbols.define(&(pThis->symbols), "this", pThis->class_name, ID_ARG);
    pThis->compileParameterlist(pThis);

    // ')'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
        TOKEN_ERR(")");
        exit (0);
    }
    advance(p_tokenizer);

    // subroutineBody
    // "{"
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), "{"))) {
        TOKEN_ERR("{");
        exit (0);
    }
    advance(p_tokenizer);

    // verDec*
    while (p_tokenizer->tokenType(p_tokenizer) == KEYWORD && p_tokenizer->keyWord(p_tokenizer) == VAR)
        pThis->compileVarDec(pThis);

    // writeFunction
    pThis->writer.writeFunction(&(pThis->writer), vm_func_name, pThis->symbols.varCount(&(pThis->symbols), ID_VAR));
    switch (subroutine) {
        case CONSTRUCTOR:
            pThis->writer.writePush(&(pThis->writer), SEG_CONST, pThis->symbols.varCount(&(pThis->symbols), ID_FIELD));
            pThis->writer.writeCall(&(pThis->writer), "Memory.alloc", 1);
            pThis->writer.writePop(&(pThis->writer), SEG_POINTER, 0);
            break;

        case METHOD:
            pThis->writer.writePush(&(pThis->writer), SEG_ARG, 0);
            pThis->writer.writePop(&(pThis->writer), SEG_POINTER, 0);
        default:
            break;
    }

    // statements
    pThis->compileStatements(pThis);

    // "}"
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), "}"))) {
        TOKEN_ERR("}");
        exit (0);
    }
    advance(p_tokenizer);

    free(vm_func_name);

}

void _compilation_engine_compileParameterlist(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;
    char *type;

    type = malloc(sizeof(char) * 1024);

    for (i = 0; (i < 2) && (is_type(p_tokenizer)); i++) {
        // type
        if (p_tokenizer->tokenType(p_tokenizer) == KEYWORD) {
            switch (p_tokenizer->keyWord(p_tokenizer)) {
                case INT:
                    strcpy(type, "int");
                    break;
                case CHAR:
                    strcpy(type, "char");
                    break;
                case BOOLEAN:
                    strcpy(type, "boolean");
                    break;
                default:
                    TOKEN_ERR("'int' or 'char' or 'boolean'");
                    exit (0);
            }
        } else if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER) {
            strcpy(type, p_tokenizer->current_token);
        } else {
            TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
            exit (0);
        }
        advance(p_tokenizer);

        // varName
        if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
            TOKEN_ERR("identifier");
            exit (0);
        }
        pThis->symbols.define(&(pThis->symbols), p_tokenizer->current_token, type, ID_ARG);
        advance(p_tokenizer);

        while (p_tokenizer->tokenType(p_tokenizer) == SYMBOL
                && !strcmp(",", p_tokenizer->symbol(p_tokenizer))) {
            // ','
            advance(p_tokenizer);

            // type
            if (p_tokenizer->tokenType(p_tokenizer) == KEYWORD) {
                switch (p_tokenizer->keyWord(p_tokenizer)) {
                    case INT:
                        strcpy(type, "int");
                        break;
                    case CHAR:
                        strcpy(type, "char");
                        break;
                    case BOOLEAN:
                        strcpy(type, "boolean");
                        break;
                    default:
                        TOKEN_ERR("'int' or 'char' or 'boolean'");
                        exit (0);
                }
            } else if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER) {
                strcpy(type, p_tokenizer->current_token);
            } else {
                TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
                exit (0);
            }
            advance(p_tokenizer);

            // varName
            if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
                TOKEN_ERR("identifier");
                exit (0);
            }
            pThis->symbols.define(&(pThis->symbols), p_tokenizer->current_token, type, ID_ARG);
            advance(p_tokenizer);
        }
    };

    free(type);

    return;

}

void _compilation_engine_compileVarDec(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    char *type;

    type = malloc(sizeof(char) * 1024);

    // 'var'
    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                && (p_tokenizer->keyWord(p_tokenizer) == VAR))) {
        TOKEN_ERR("var");
        exit (0);
    }
    advance(p_tokenizer);

    // type
    if (p_tokenizer->tokenType(p_tokenizer) == KEYWORD) {
        switch (p_tokenizer->keyWord(p_tokenizer)) {
            case INT:
                strcpy(type, "int");
                break;
            case CHAR:
                strcpy(type, "char");
                break;
            case BOOLEAN:
                strcpy(type, "boolean");
                break;
            default:
                TOKEN_ERR("'int' or 'char' or 'boolean'");
                exit (0);
        }
    } else if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER) {
        strcpy(type, p_tokenizer->current_token);
    } else {
        TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
        exit (0);
    }
    advance(p_tokenizer);

    // varName
    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        exit (0);
    }
    pThis->symbols.define(&(pThis->symbols), p_tokenizer->current_token, type, ID_VAR);
    advance(p_tokenizer);

    // (',' varName)*
    while ((p_tokenizer->tokenType(p_tokenizer) == SYMBOL) &&
        (!strcmp(p_tokenizer->symbol(p_tokenizer), ","))) {
        advance(p_tokenizer);

        if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
            TOKEN_ERR("identifier");
            exit (0);
        }
        pThis->symbols.define(&(pThis->symbols), p_tokenizer->current_token, type, ID_VAR);
        advance(p_tokenizer);
    }

    // ';'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), ";"))) {
        TOKEN_ERR(";");
        exit (0);
    }
    advance(p_tokenizer);

    free(type);
}

void _compilation_engine_compileStatements(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    while (p_tokenizer->tokenType(p_tokenizer) == KEYWORD
            && (p_tokenizer->keyWord(p_tokenizer) == LET
            || p_tokenizer->keyWord(p_tokenizer) == IF
            || p_tokenizer->keyWord(p_tokenizer) == WHILE
            || p_tokenizer->keyWord(p_tokenizer) == DO
            || p_tokenizer->keyWord(p_tokenizer) == RETURN)
          ) {
        switch (p_tokenizer->keyWord(p_tokenizer)) {
            case LET:
                pThis->compileLet(pThis);
                break;
            case IF:
                pThis->compileIf(pThis);
                break;
            case WHILE:
                pThis->compileWhile(pThis);
                break;
            case DO:
                pThis->compileDo(pThis);
                break;
            case RETURN:
                pThis->compileReturn(pThis);
                break;
            default:
                break;
        }
    }
}

void _compilation_engine_compileDo(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    char *vm_func_name, *identifier;
    struct {enum kind kind; enum segment seg;} knd_seg_tbl[4] = {
        {ID_STATIC, SEG_STATIC}, {ID_ARG, SEG_ARG},
        {ID_VAR, SEG_LOCAL},     {ID_FIELD, SEG_THIS},
    };
    unsigned int i;
    enum kind kind;

    vm_func_name = malloc(sizeof(char) * 1024);
    identifier = malloc(sizeof(char) * 1024);

    // 'do'
    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                && (p_tokenizer->keyWord(p_tokenizer) == DO))) {
        TOKEN_ERR("do");
        exit (0);
    }
    advance(p_tokenizer);

    // subroutineCall
    // subroutineName or className or varName
    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        exit (0);
    }
    strcpy(identifier, p_tokenizer->current_token);
    advance(p_tokenizer);

    if (!strcmp(p_tokenizer->symbol(p_tokenizer), "(")) {
        sprintf(vm_func_name, "%s.%s", pThis->class_name, identifier);
        // '('
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
            TOKEN_ERR("(");
            exit (0);
        }
        advance(p_tokenizer);

        // write push 'this' pointer because call method
        pThis->writer.writePush(&(pThis->writer), SEG_POINTER, 0);

        // expressionnList
        pThis->nArgs = 1;
        pThis->compileExpressionList(pThis);

        // ')'
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
            TOKEN_ERR(")");
            exit (0);
        }
        advance(p_tokenizer);

    } else if (!strcmp(p_tokenizer->symbol(p_tokenizer), ".")) {
        // '.'
        advance(p_tokenizer);

        // subroutineName
        if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
            TOKEN_ERR("identifier");
            exit (0);
        }

        kind = pThis->symbols.kindOf(&(pThis->symbols), identifier);
        if (kind == ID_NONE)
            sprintf(vm_func_name, "%s.%s", identifier, pThis->tokenizer.identifier(&(pThis->tokenizer)));
        else
            sprintf(vm_func_name, "%s.%s", pThis->symbols.typeOf(&(pThis->symbols), identifier), p_tokenizer->current_token);

        advance(p_tokenizer);

        // '('
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
            TOKEN_ERR("(");
            exit (0);
        }
        advance(p_tokenizer);

        pThis->nArgs = 0;

        // write push 'this' pointer because call method
        if (kind != ID_NONE) {
            for (i = 0; knd_seg_tbl[i].kind != kind; i++);
            pThis->writer.writePush(&(pThis->writer), knd_seg_tbl[i].seg, pThis->symbols.indexOf((&pThis->symbols), identifier));
            pThis->nArgs++;
        }

        // expressionnList
        pThis->compileExpressionList(pThis);

        // ')'
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
            TOKEN_ERR(")");
            exit (0);
        }
        advance(p_tokenizer);
    } else {
        TOKEN_ERR("'(' or ','");
        exit (0);
    }

    // writeCall
    pThis->writer.writeCall(&(pThis->writer), vm_func_name, pThis->nArgs);
    pThis->writer.writePop(&(pThis->writer), SEG_TEMP, 0);

    // ';'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), ";"))) {
        TOKEN_ERR(";");
        exit (0);
    }
    advance(p_tokenizer);

    free(vm_func_name);
    free(identifier);
}

void _compilation_engine_compileLet(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    SymbolTable *p_symbols = &(pThis->symbols);
    int i;
    char *var_name;
    struct {enum kind kind; enum segment seg;} knd_seg_tbl[4] = {
        {ID_STATIC, SEG_STATIC}, {ID_ARG, SEG_ARG},
        {ID_VAR, SEG_LOCAL},     {ID_FIELD, SEG_THIS},
    };
    bool is_array_elem = false;
    static unsigned int tmp_index_base = 0;
    unsigned int tmp_index;

    var_name = malloc(sizeof(char) * 1024);

    // 'let'
    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                && (p_tokenizer->keyWord(p_tokenizer) == LET))) {
        TOKEN_ERR("let");
        exit (0);
    }
    advance(p_tokenizer);

    // varName
    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        exit (0);
    }
    strcpy(var_name, pThis->tokenizer.identifier(&(pThis->tokenizer)));
    advance(p_tokenizer);

    // ('[' expression ']')?
    for (i = 0; (i < 2) && (!strcmp(p_tokenizer->current_token, "[")); i++) {
        is_array_elem = true;
        tmp_index = tmp_index_base % 7 + 1;

        // '['
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), "["))) {
            TOKEN_ERR("[");
            exit (0);
        }
        advance(p_tokenizer);

        pThis->compileExpression(pThis);

        // write code
        for (i = 0; knd_seg_tbl[i].kind != p_symbols->kindOf(p_symbols, var_name); i++);
        pThis->writer.writePush(&(pThis->writer),
                knd_seg_tbl[i].seg, p_symbols->indexOf(p_symbols, var_name));
        pThis->writer.writeArithmetic(&(pThis->writer), COM_ADD);
        pThis->writer.writePop(&(pThis->writer), SEG_TEMP, tmp_index);

        // ']'
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), "]"))) {
            TOKEN_ERR("]");
            exit (0);
        }
        advance(p_tokenizer);
    }

    // '='
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), "="))) {
        TOKEN_ERR("=");
        exit (0);
    }
    advance(p_tokenizer);

    // expression
    pThis->compileExpression(pThis);

    // write code
    if (is_array_elem) {
        pThis->writer.writePush(&(pThis->writer), SEG_TEMP, tmp_index);
        pThis->writer.writePop(&(pThis->writer), SEG_POINTER, 1);
        pThis->writer.writePop(&(pThis->writer), SEG_THAT, 0);
        tmp_index_base++;
    } else {
        for (i = 0; knd_seg_tbl[i].kind != p_symbols->kindOf(p_symbols, var_name); i++);
        pThis->writer.writePop(&(pThis->writer),
                knd_seg_tbl[i].seg, p_symbols->indexOf(p_symbols, var_name));
    }

    // ';'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), ";"))) {
        TOKEN_ERR(";");
        exit (0);
    }
    advance(p_tokenizer);

    free(var_name);
}


void _compilation_engine_compileWhile(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);;
    static unsigned int n_label = 0;
    unsigned int tmp_n_label;
    char *label_name;

    label_name = malloc(sizeof(char) * 1024);

    // 'while'
    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                && (p_tokenizer->keyWord(p_tokenizer) == WHILE))) {
        TOKEN_ERR("while");
        exit (0);
    }
    advance(p_tokenizer);

    // write label in-position
    tmp_n_label = n_label;
    n_label++;
    sprintf(label_name, "while_%d_in", tmp_n_label);
    pThis->writer.writeLabel(&(pThis->writer), label_name);

    // '('
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
        TOKEN_ERR("(");
        exit (0);
    }
    advance(p_tokenizer);

    // expression
    pThis->compileExpression(pThis);

    // ')'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
        TOKEN_ERR(")");
        exit (0);
    }
    advance(p_tokenizer);

    // write if-go out
    sprintf(label_name, "while_%d_out", tmp_n_label);
    pThis->writer.writeArithmetic(&(pThis->writer), COM_NOT);
    pThis->writer.writeIf(&(pThis->writer), label_name);

    // '{'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), "{"))) {
        TOKEN_ERR("{");
        exit (0);
    }
    advance(p_tokenizer);

    // statements
    pThis->compileStatements(pThis);

    // '}'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), "}"))) {
        TOKEN_ERR("}");
        exit (0);
    }
    advance(p_tokenizer);

    // write goto in
    sprintf(label_name, "while_%d_in", tmp_n_label);
    pThis->writer.writeGoto(&(pThis->writer), label_name);

    // write label out-position
    sprintf(label_name, "while_%d_out", tmp_n_label);
    pThis->writer.writeLabel(&(pThis->writer), label_name);

    free(label_name);
}

void _compilation_engine_compileReturn(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;

    // 'return'
    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                && (p_tokenizer->keyWord(p_tokenizer) == RETURN))) {
        TOKEN_ERR("return");
        exit (0);
    }
    advance(p_tokenizer);

    // expression?
    for (i = 0; (i < 2) && is_begin_of_term(p_tokenizer); i++)
        pThis->compileExpression(pThis);

    // ';'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
        (strcmp(p_tokenizer->symbol(p_tokenizer), ";"))) {
        TOKEN_ERR(";");
        exit (0);
    }
    advance(p_tokenizer);

    // write return
    if (pThis->ret_type == VOID)
        pThis->writer.writePush(&(pThis->writer), SEG_CONST, 0);
    pThis->writer.writeReturn(&(pThis->writer));
}

void _compilation_engine_compileIf(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;
    static unsigned int n_label = 0;
    unsigned int tmp_n_label;
    char *label_name;

    label_name = malloc(sizeof(char) * 1024);

    // 'if'
    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                && (p_tokenizer->keyWord(p_tokenizer) == IF))) {
        TOKEN_ERR("if");
        exit (0);
    }
    advance(p_tokenizer);

    tmp_n_label = n_label;
    n_label++;

    // '('
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
        TOKEN_ERR("(");
        exit (0);
    }
    advance(p_tokenizer);

    // expression
    pThis->compileExpression(pThis);

    // ')'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
        TOKEN_ERR(")");
        exit (0);
    }
    advance(p_tokenizer);

    // write if-go else
    sprintf(label_name, "if_%d_else", tmp_n_label);
    pThis->writer.writeArithmetic(&(pThis->writer), COM_NOT);
    pThis->writer.writeIf(&(pThis->writer), label_name);

    // '{'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), "{"))) {
        TOKEN_ERR("{");
        exit (0);
    }
    advance(p_tokenizer);

    // statements
    pThis->compileStatements(pThis);

    // '}'
    if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
            (strcmp(p_tokenizer->symbol(p_tokenizer), "}"))) {
        TOKEN_ERR("}");
        exit (0);
    }
    advance(p_tokenizer);

    // write goto out 
    sprintf(label_name, "if_%d_out", tmp_n_label);
    pThis->writer.writeGoto(&(pThis->writer), label_name);

    // write label else-position
    sprintf(label_name, "if_%d_else", tmp_n_label);
    pThis->writer.writeLabel(&(pThis->writer), label_name);

    for (i = 0; (i < 2) && (p_tokenizer->keyWord(p_tokenizer) == ELSE); i++) {
        // 'else'
        if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD)
                    && (p_tokenizer->keyWord(p_tokenizer) == ELSE))) {
            TOKEN_ERR("else");
            exit (0);
        }
        advance(p_tokenizer);

        // '{'
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), "{"))) {
            TOKEN_ERR("{");
            exit (0);
        }
        advance(p_tokenizer);

        // statements
        pThis->compileStatements(pThis);

        // '}'
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), "}"))) {
            TOKEN_ERR("}");
            exit (0);
        }
        advance(p_tokenizer);
    }

    // write label out-position
    sprintf(label_name, "if_%d_out", tmp_n_label);
    pThis->writer.writeLabel(&(pThis->writer), label_name);
}

void _compilation_engine_compileExpression(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    char op[2];
    struct {char op[2]; enum command com;} op_tbl[7] =
        {{"+", COM_ADD}, {"-", COM_SUB}, {"=", COM_EQ},
         {">", COM_GT},  {"<", COM_LT},  {"&", COM_AND}, {"|", COM_OR},
    };
    int i;


    // term
    pThis->compileTerm(pThis);

    // (op term)*
    while (is_op(p_tokenizer)) {
        strcpy(op, p_tokenizer->current_token);
        advance(p_tokenizer);

        pThis->compileTerm(pThis);

        // write op
        for (i = 0; strcmp(op_tbl[i].op, op) && i < 7; i++);
        if (i < 7) {
            pThis->writer.writeArithmetic(&(pThis->writer), op_tbl[i].com);
        } else if (!strcmp(op, "/")) {
            pThis->writer.writeCall(&(pThis->writer), "Math.divide", 2);
        } else if (!strcmp(op, "*")) {
            pThis->writer.writeCall(&(pThis->writer), "Math.multiply", 2);
        }
    }
}

void _compilation_engine_compileTerm(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    SymbolTable *p_symbols = &(pThis->symbols);
    char op[2];
    struct {char op[2]; enum command com;} op_tbl[2] =
        {{"-", COM_NEG}, {"~", COM_NOT}, 
    };
    struct {enum kind kind; enum segment seg;} knd_seg_tbl[4] = {
        {ID_STATIC, SEG_STATIC}, {ID_ARG, SEG_ARG},
        {ID_VAR, SEG_LOCAL},     {ID_FIELD, SEG_THIS},
    };
    int i;
    char *identifier, *subroutine_name;
    enum kind kind;
    char *str;

    identifier = malloc(sizeof(char) * 1024);
    subroutine_name = malloc(sizeof(char) * 1024);

    switch (p_tokenizer->tokenType(p_tokenizer)) {
        case INT_CONST:
            // integerConstant
            pThis->writer.writePush(&(pThis->writer), SEG_CONST, atoi(p_tokenizer->current_token));
            advance(p_tokenizer);
            break;

        case STRING_CONST:
            // stringConstant
            str = p_tokenizer->stringVal(p_tokenizer);
            pThis->writer.writePush(&(pThis->writer), SEG_CONST, strlen(str));
            pThis->writer.writeCall(&(pThis->writer), "String.new", 1);
            for (i = 0; i < strlen(str); i++) {
                pThis->writer.writePush(&(pThis->writer), SEG_CONST, str[i]);
                pThis->writer.writeCall(&(pThis->writer), "String.appendChar", 2);
            }
            advance(p_tokenizer);
            break;

        case KEYWORD:
            // keywordConstant
            switch (p_tokenizer->keyWord(p_tokenizer)) {
                case TRUE:
                    pThis->writer.writePush(&(pThis->writer), SEG_CONST, 1);
                    pThis->writer.writeArithmetic(&(pThis->writer), COM_NEG);
                    break;
                case FALSE:
                    pThis->writer.writePush(&(pThis->writer), SEG_CONST, 0);
                    break;
                case NUL:
                    pThis->writer.writePush(&(pThis->writer), SEG_CONST, 0);
                    break;
                case THIS:
                    pThis->writer.writePush(&(pThis->writer), SEG_POINTER, 0);
                    break;
                default:
                    TOKEN_ERR("true, false, null or this");
                    exit (0);
                    break;
            }
            advance(p_tokenizer);
            break;

        case SYMBOL:
            // unaryOP term
            if (!strcmp(p_tokenizer->symbol(p_tokenizer), "-")
                    || !strcmp(p_tokenizer->symbol(p_tokenizer), "~")
               ) {
                strcpy(op, p_tokenizer->current_token);
                advance(p_tokenizer);

                pThis->compileTerm(pThis);

                // write code
                for (i = 0; strcmp(op_tbl[i].op, op) && i < 2; i++);
                pThis->writer.writeArithmetic(&(pThis->writer), op_tbl[i].com);

            } else if (!strcmp(p_tokenizer->symbol(p_tokenizer), "(")) {
                // '(' expression ')'

                // '('
                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
                    TOKEN_ERR("(");
                    exit (0);
                }
                advance(p_tokenizer);

                pThis->compileExpression(pThis);


                // ')'
                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
                    TOKEN_ERR(")");
                    exit (0);
                }
                advance(p_tokenizer);

            } else {
                TOKEN_ERR("-, ~ or '(' ");
                return;
            }
            break;

        case INDENTIFIER:
            // common element "identifier"
            if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
                TOKEN_ERR("identifier");
                exit (0);
            }
            strcpy(identifier, pThis->tokenizer.identifier(&(pThis->tokenizer)));
            advance(p_tokenizer);

            if (!strcmp(p_tokenizer->identifier(p_tokenizer), "[")) {
                // '[' expression ']'
                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), "["))) {
                    TOKEN_ERR("[");
                    exit (0);
                }
                advance(p_tokenizer);

                pThis->compileExpression(pThis);

                // write code
                for (i = 0; knd_seg_tbl[i].kind != p_symbols->kindOf(p_symbols, identifier); i++);
                pThis->writer.writePush(&(pThis->writer),
                        knd_seg_tbl[i].seg, p_symbols->indexOf(p_symbols, identifier));
                pThis->writer.writeArithmetic(&(pThis->writer), COM_ADD);
                pThis->writer.writePop(&(pThis->writer), SEG_POINTER, 1);
                pThis->writer.writePush(&(pThis->writer), SEG_THAT, 0);

                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), "]"))) {
                    TOKEN_ERR("]");
                    exit (0);
                }
                advance(p_tokenizer);


            } else if (!strcmp(p_tokenizer->identifier(p_tokenizer), "(")) {
                // '('
                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
                    TOKEN_ERR("(");
                    exit (0);
                }
                advance(p_tokenizer);

                // write push 'this' pointer because call method
                pThis->writer.writePush(&(pThis->writer), SEG_POINTER, 0);

                // expressionnList
                pThis->nArgs = 1;
                pThis->compileExpressionList(pThis);

                // ')'
                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
                    TOKEN_ERR(")");
                    exit (0);
                }
                advance(p_tokenizer);

                // write code
                sprintf(subroutine_name, "%s.%s", pThis->class_name, identifier);
                pThis->writer.writeCall(&(pThis->writer), subroutine_name, pThis->nArgs);

            } else if (!strcmp(p_tokenizer->symbol(p_tokenizer), ".")) {
                advance(p_tokenizer);

                // subroutineName
                if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
                    TOKEN_ERR("identifier");
                    exit (0);
                }
                kind = pThis->symbols.kindOf(&(pThis->symbols), identifier);
                if (kind == ID_NONE)
                    sprintf(subroutine_name, "%s.%s", identifier, pThis->tokenizer.identifier(&(pThis->tokenizer)));
                else
                    sprintf(subroutine_name, "%s.%s", pThis->symbols.typeOf(&(pThis->symbols), identifier), p_tokenizer->current_token);
                advance(p_tokenizer);

                // '('
                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
                    TOKEN_ERR("(");
                    exit (0);
                }
                advance(p_tokenizer);

                pThis->nArgs = 0;

                // write push 'this' pointer because call method
                if (kind != ID_NONE) {
                    for (i = 0; knd_seg_tbl[i].kind != kind; i++);
                    pThis->writer.writePush(&(pThis->writer), knd_seg_tbl[i].seg, pThis->symbols.indexOf((&pThis->symbols), identifier));
                    pThis->nArgs++;
                }

                // expressionnList
                pThis->compileExpressionList(pThis);

                // ')'
                if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                        (strcmp(p_tokenizer->symbol(p_tokenizer), ")"))) {
                    TOKEN_ERR(")");
                    exit (0);
                }
                advance(p_tokenizer);

                // write code
                pThis->writer.writeCall(&(pThis->writer), subroutine_name, pThis->nArgs);

            } else {
                // write code
                for (i = 0; knd_seg_tbl[i].kind != p_symbols->kindOf(p_symbols, identifier); i++);
                pThis->writer.writePush(&(pThis->writer),
                        knd_seg_tbl[i].seg, p_symbols->indexOf(p_symbols, identifier));
            }
            break;

        default:
            TOKEN_ERR("integerConstant, stringConstant, keywordConnstant or identifier");
            return;
            break;
    }

    free(identifier);
    free(subroutine_name);

}

void _compilation_engine_compileExpressionList(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i, nArgs;

    nArgs = pThis->nArgs;

    for (i = 0; (i < 2) && (is_begin_of_expression(p_tokenizer)); i++) {
        // expression
        pThis->compileExpression(pThis);
        nArgs++;

        while (p_tokenizer->tokenType(p_tokenizer) == SYMBOL &&
                (!strcmp(p_tokenizer->symbol(p_tokenizer), ","))) {
            // ','
            advance(p_tokenizer);

            // expression
            pThis->compileExpression(pThis);
            nArgs++;
        }
    };

    pThis->nArgs = nArgs;

    return;
}

void _compilation_engine_del(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    p_tokenizer->del(p_tokenizer);
    pThis->writer.close(&pThis->writer);

    free(pThis->class_name);
    return;
}

static bool is_type(JackTokenizer *pThis)
{
    if ((pThis->tokenType(pThis) == KEYWORD
            && ((pThis->keyWord(pThis) == INT)
            || (pThis->keyWord(pThis) == CHAR)
            || (pThis->keyWord(pThis) == BOOLEAN)))
            || (pThis->tokenType(pThis) == INDENTIFIER))
        return true;

    return false;
}

static bool is_begin_of_term(JackTokenizer *pThis)
{
    if ((pThis->tokenType(pThis) == INT_CONST)
            || (pThis->tokenType(pThis) == STRING_CONST)
            || (!strcmp(pThis->current_token, "true"))
            || (!strcmp(pThis->current_token, "false"))
            || (!strcmp(pThis->current_token, "null"))
            || (!strcmp(pThis->current_token, "this"))
            || (pThis->tokenType(pThis) == INDENTIFIER)
            || (!strcmp(pThis->current_token, "("))
            || (!strcmp(pThis->current_token, "-"))
            || (!strcmp(pThis->current_token, "~")))
        return true;

    return false;
}

static bool is_op(JackTokenizer *pThis)
{
    if ((pThis->tokenType(pThis) == SYMBOL)
            && (!strcmp(pThis->current_token, "+")
                || (!strcmp(pThis->current_token, "-"))
                || (!strcmp(pThis->current_token, "*"))
                || (!strcmp(pThis->current_token, "/"))
                || (!strcmp(pThis->current_token, "&"))
                || (!strcmp(pThis->current_token, "|"))
                || (!strcmp(pThis->current_token, "<"))
                || (!strcmp(pThis->current_token, ">"))
                || (!strcmp(pThis->current_token, "="))))
        return true;

    return false;
}

static bool is_begin_of_expression(JackTokenizer *pThis)
{
    if (pThis->tokenType(pThis) == INT_CONST ||
            pThis->tokenType(pThis) == STRING_CONST ||
            (pThis->tokenType(pThis) == KEYWORD &&
                (pThis->keyWord(pThis) == TRUE  ||
                 pThis->keyWord(pThis) == FALSE ||
                 pThis->keyWord(pThis) == NUL   ||
                 pThis->keyWord(pThis) == THIS)) ||
            (pThis->tokenType(pThis) == SYMBOL &&
                (!strcmp(pThis->symbol(pThis), "-")  ||
                !strcmp(pThis->symbol(pThis), "~")   ||
                !strcmp(pThis->symbol(pThis), "("))) ||
            pThis->tokenType(pThis) == INDENTIFIER) {
        return true;
    }

    return false;
}

static void advance(JackTokenizer *p_tokenizer)
{
    if (!p_tokenizer->hasMoreTokens(p_tokenizer)) {
        fprintf(stderr, "no more token\n");
        exit (0);
    }
    p_tokenizer->advance(p_tokenizer);
}
