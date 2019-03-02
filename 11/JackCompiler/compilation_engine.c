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


static bool write_keyword(CompilationEngine *pThis, enum KeyWord keyword);
static bool write_symbol(CompilationEngine *pThis, char *symbol);
static bool write_stringConstant(CompilationEngine *pThis);
static bool write_identifier(CompilationEngine *pThis, enum id_status status, char *type, enum kind kind);
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
        (strcmp(p_tokenizer->symbol(p_tokenizer), ","))) {
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
    if (p_tokenizer->tokenType(p_tokenizer) == KEYWORD) {
        switch (p_tokenizer->keyWord(p_tokenizer)) {
            case VOID:
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

    fprintf(pThis->fp, "<varDec>\n");

    // 'var'
    if (!write_keyword(pThis, VAR)) return;

    // type
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case INT:
            strcpy(type, "int");
            if (!write_keyword(pThis, INT)) return;
            break;
        case CHAR:
            strcpy(type, "char");
            if (!write_keyword(pThis, CHAR)) return;
            break;
        case BOOLEAN:
            strcpy(type, "boolean");
            if (!write_keyword(pThis, BOOLEAN)) return;
            break;
        default:
            if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER) {
                strcpy(type, p_tokenizer->current_token);
                write_identifier(pThis, USED, NULL, ID_CLASS);
            } else {
                TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
            }
    }

    // varName
    if (!write_identifier(pThis, DEFINED, type, ID_VAR)) return;

    // (',' , vafName)*
    while (p_tokenizer->tokenType(p_tokenizer) == SYMBOL
            && !strcmp(",", p_tokenizer->symbol(p_tokenizer))) {
        if (!write_symbol(pThis, ",")) return;
        if (!write_identifier(pThis, DEFINED, type, ID_VAR)) return;
    }

    // ';'
    if (!write_symbol(pThis, ";")) return;

    fprintf(pThis->fp, "</varDec>\n");

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

        // expressionnList
        pThis->nArgs = 0;
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
        sprintf(vm_func_name, "%s.%s", identifier, p_tokenizer->current_token);
        advance(p_tokenizer);

        // '('
        if ((p_tokenizer->tokenType(p_tokenizer) != SYMBOL) ||
                (strcmp(p_tokenizer->symbol(p_tokenizer), "("))) {
            TOKEN_ERR("(");
            exit (0);
        }
        advance(p_tokenizer);

        // expressionnList
        pThis->nArgs = 0;
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
    int i;

    fprintf(pThis->fp, "<letStatement>\n");

    // 'let'
    if (!write_keyword(pThis, LET)) return;

    // varName
    if (!write_identifier(pThis, USED, NULL, ID_NONE)) return;

    // ('[' expression ']')?
    for (i = 0; (i < 2) && (!strcmp(p_tokenizer->current_token, "[")); i++) {
        if (!write_symbol(pThis, "[")) return;
        pThis->compileExpression(pThis);
        if (!write_symbol(pThis, "]")) return;
    }

    // '='
    if (!write_symbol(pThis, "=")) return;

    // expression
    pThis->compileExpression(pThis);

    // ';'
    if (!write_symbol(pThis, ";")) return;

    fprintf(pThis->fp, "</letStatement>\n");
}


void _compilation_engine_compileWhile(CompilationEngine *pThis)
{
    fprintf(pThis->fp, "<whileStatement>\n");

    // 'while'
    if (!write_keyword(pThis, WHILE)) return;

    // '('
    if (!write_symbol(pThis, "(")) return;

    // expression
    pThis->compileExpression(pThis);

    // ')'
    if (!write_symbol(pThis, ")")) return;

    // '{'
    if (!write_symbol(pThis, "{")) return;

    // statements
    pThis->compileStatements(pThis);

    // '}'
    if (!write_symbol(pThis, "}")) return;
    fprintf(pThis->fp, "</whileStatement>\n");
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
    pThis->writer.writeReturn(&(pThis->writer));
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
}

void _compilation_engine_compileIf(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;

    fprintf(pThis->fp, "<ifStatement>\n");

    // 'if'
    if (!write_keyword(pThis, IF)) return;

    // '('
    if (!write_symbol(pThis, "(")) return;

    // expression
    pThis->compileExpression(pThis);

    // ')'
    if (!write_symbol(pThis, ")")) return;

    // '{'
    if (!write_symbol(pThis, "{")) return;

    // statements
    pThis->compileStatements(pThis);

    // '}'
    if (!write_symbol(pThis, "}")) return;

    for (i = 0; (i < 2) && (p_tokenizer->keyWord(p_tokenizer) == ELSE); i++) {
        // 'else'
        if (!write_keyword(pThis, ELSE)) return;

        // '{'
        if (!write_symbol(pThis, "{")) return;

        // statements
        pThis->compileStatements(pThis);

        // '}'
        if (!write_symbol(pThis, "}")) return;
    }

    fprintf(pThis->fp, "</ifStatement>\n");
}

void _compilation_engine_compileExpression(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    char op[2];
    struct {char op[2]; enum command com;} op_tbl[7] =
        {{"+", COM_ADD}, {"-", COM_SUB}, {"=", COM_EQ},
         {">", COM_GT},  {"<", COM_LT},  {"&", COM_ADD}, {"|", COM_OR},
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

    switch (p_tokenizer->tokenType(p_tokenizer)) {
        case INT_CONST:
            // integerConstant
            pThis->writer.writePush(&(pThis->writer), SEG_CONST, atoi(p_tokenizer->current_token));
            advance(p_tokenizer);
            break;

        case STRING_CONST:
            // stringConstant
            if (!write_stringConstant(pThis)) return;
            break;

        case KEYWORD:
            // keywordConstant
            if (p_tokenizer->keyWord(p_tokenizer) == TRUE
                    || p_tokenizer->keyWord(p_tokenizer) == FALSE
                    || p_tokenizer->keyWord(p_tokenizer) == NUL
                    || p_tokenizer->keyWord(p_tokenizer) == THIS
               ) { 
                if (!write_keyword(pThis, p_tokenizer->keyWord(p_tokenizer))) return;
            } else {
                TOKEN_ERR("true, false, null or this");
                return;
            }
            break;

        case SYMBOL:
            // unaryOP term
            if (!strcmp(p_tokenizer->symbol(p_tokenizer), "-")
                    || !strcmp(p_tokenizer->symbol(p_tokenizer), "~")
               ) {
                if (!write_symbol(pThis, p_tokenizer->symbol(p_tokenizer))) return;
                pThis->compileTerm(pThis);

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
            if (!write_identifier(pThis, USED, NULL, ID_NONE)) return;

            if (!strcmp(p_tokenizer->identifier(p_tokenizer), "[")) {
                // '[' expression ']'
                if (!write_symbol(pThis, "[")) return;
                pThis->compileExpression(pThis);
                if (!write_symbol(pThis, "]")) return;

            } else if (!strcmp(p_tokenizer->identifier(p_tokenizer), "(")) {
                if (write_symbol(pThis, "(")) {
                    // '('
                    // expressionnList
                    pThis->compileExpressionList(pThis);

                    // ')'
                    if (!write_symbol(pThis, ")")) return;
                }
            } else if (!strcmp(p_tokenizer->symbol(p_tokenizer), ".")) {
                if (write_symbol(pThis, ".")) {
                    // '.'
                    // subroutineName
                    if (!write_identifier(pThis, USED, NULL, ID_SUBROUTINE)) return;

                    // '('
                    if (!write_symbol(pThis, "(")) return;

                    // expressionnList
                    pThis->compileExpressionList(pThis);

                    // ')'
                    if (!write_symbol(pThis, ")")) return;
                }
            }
            break;

        default:
            TOKEN_ERR("integerConstant, stringConstant, keywordConnstant or identifier");
            return;
            break;
    }

}

void _compilation_engine_compileExpressionList(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;

    for (i = 0; (i < 2) && (is_begin_of_expression(p_tokenizer)); i++) {
        // expression
        pThis->compileExpression(pThis);
        pThis->nArgs++;

        while (p_tokenizer->tokenType(p_tokenizer) == SYMBOL &&
                (!strcmp(p_tokenizer->symbol(p_tokenizer), ","))) {
            // ','
            advance(p_tokenizer);

            // expression
            pThis->compileExpression(pThis);
            pThis->nArgs++;
        }
    };

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

static bool write_keyword(CompilationEngine *pThis, enum KeyWord keyword)
{
    const char *str[] = {"class", "method", "function", "constructor", "int",    "boolean",
                         "char",  "void",   "var",      "static",      "field",  "let",
                         "do",    "if",     "else",     "while",       "return", "true",
                         "false", "null",   "this",
                            };
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    if (!((p_tokenizer->tokenType(p_tokenizer) == KEYWORD) 
            && (p_tokenizer->keyWord(p_tokenizer) == keyword))) {
        TOKEN_ERR(str[keyword]);
        return false;
    }
    p_tokenizer->printCurrentToken(p_tokenizer, pThis->fp);

    if (p_tokenizer->hasMoreTokens(p_tokenizer))
        p_tokenizer->advance(p_tokenizer);

    return true;
}

static bool write_symbol(CompilationEngine *pThis, char *symbol)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    if (!(p_tokenizer->tokenType(p_tokenizer) == SYMBOL && !strcmp(p_tokenizer->symbol(p_tokenizer), symbol))) {
        printf("token = %s\n", p_tokenizer->current_token);
        TOKEN_ERR(symbol);
        return false;
    }
    // fprintf(pThis->fp, "<symbol> %s </symbol>\n", p_tokenizer->symbol(p_tokenizer));
    p_tokenizer->printCurrentToken(p_tokenizer, pThis->fp);

    if (p_tokenizer->hasMoreTokens(p_tokenizer))
        p_tokenizer->advance(p_tokenizer);

    return true;
}

static bool write_stringConstant(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    if (!(p_tokenizer->tokenType(p_tokenizer) == STRING_CONST)) {
        TOKEN_ERR("stringConstant");
        return false;
    }
    // fprintf(pThis->fp, "<stringConstant> %s </stringConstant>\n", p_tokenizer->stringVal(p_tokenizer));
    p_tokenizer->printCurrentToken(p_tokenizer, pThis->fp);

    if (p_tokenizer->hasMoreTokens(p_tokenizer))
        p_tokenizer->advance(p_tokenizer);

    return true;
}

static bool write_identifier(CompilationEngine *pThis, enum id_status status, char *type, enum kind kind)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    SymbolTable   *p_sym_table = &(pThis->symbols);
    char *name;

    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        return false;
    }

    name = p_tokenizer->current_token;

    if (status == DEFINED) {
        switch (kind) {
            case ID_ARG:
            case ID_VAR:
            case ID_FIELD:
            case ID_STATIC:
                p_sym_table->define(p_sym_table, name, type, kind);
                fprintf(pThis->fp, "defined name = %s, type = %s, kind = %d\n", name, type, kind);
                break;
            case ID_CLASS:
            case ID_SUBROUTINE:
                fprintf(pThis->fp, "defined name = %s, type = %s, kind = %d\n", name, "none", kind);
                break;
            default:
                break;
        }
    } else if (status == USED) {
        if (p_sym_table->kindOf(p_sym_table, name) == ID_NONE) {
            fprintf(pThis->fp, "used name = %s, type = %s, kind = %d\n", name, "none", kind);
        } else {
            fprintf(pThis->fp, "used name = %s, type = %s, kind = %d idx = %d\n", name, p_sym_table->typeOf(p_sym_table, name), p_sym_table->kindOf(p_sym_table, name), p_sym_table->indexOf(p_sym_table, name));
        }
    }

    // fprintf(pThis->fp, "<identifier> %s </identifier>\n", p_tokenizer->identifier(p_tokenizer));
    p_tokenizer->printCurrentToken(p_tokenizer, pThis->fp);

    if (p_tokenizer->hasMoreTokens(p_tokenizer))
        p_tokenizer->advance(p_tokenizer);

    return true;
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
