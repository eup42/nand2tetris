/**
 * compilation_engine.c
 */

#include "compilation_engine.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define TOKEN_ERR(token) do {fprintf(stderr, "%s %s() : error %s is expected\n", __FILE__, __FUNCTION__, token); } while (0)


static bool write_keyword(CompilationEngine *pThis, enum KeyWord keyword);
static bool write_symbol(CompilationEngine *pThis, char *symbol);
static bool write_integerConstant(CompilationEngine *pThis);
static bool write_stringConstant(CompilationEngine *pThis);
static bool write_identifier(CompilationEngine *pThis);
static bool is_type(JackTokenizer *pThis);
static bool is_begin_of_term(JackTokenizer *pThis);
static bool is_op(JackTokenizer *pThis);
static bool is_begin_of_expression(JackTokenizer *pThis);

void _compilation_engine_init(CompilationEngine *pThis, char *istream, char *ostream)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    p_tokenizer->init(p_tokenizer, istream);

    pThis->fp = fopen(ostream, "w");

    if (pThis->fp == NULL) {
        fprintf(stderr, "%s %s() : %s\n", __FILE__, __FUNCTION__, strerror(errno));
        exit errno;
    }
}



void _compilation_engine_compileClass(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    fprintf(pThis->fp, "<class>\n");

    // 'class'
    if (!write_keyword(pThis, CLASS)) return;

    // className
    if (!write_identifier(pThis)) return;

    // '{'
    if (!write_symbol(pThis, "{")) return;

    // classVarDec*
    while ((p_tokenizer->tokenType(p_tokenizer) == KEYWORD) &&
        ((p_tokenizer->keyWord(p_tokenizer) == STATIC) ||
        (p_tokenizer->keyWord(p_tokenizer) == FIELD)))
        pThis->compileClassVarDec(pThis);

    // subroutineDec*
    while ((p_tokenizer->tokenType(p_tokenizer) == KEYWORD) &&
        ((p_tokenizer->keyWord(p_tokenizer) == CONSTRUCTOR) ||
        (p_tokenizer->keyWord(p_tokenizer) == FUNCTION) ||
        (p_tokenizer->keyWord(p_tokenizer) == METHOD)))
        pThis->compileSubroutine(pThis);

    if (!write_symbol(pThis, "}")) return;

    fprintf(pThis->fp, "</class>\n");
}

void _compilation_engine_compileClassVarDec(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    fprintf(pThis->fp, "<classVarDec>\n");

    // ('static' | 'field')
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case STATIC:
            if (!write_keyword(pThis, STATIC)) return;
            break;
        case FIELD:
            if (!write_keyword(pThis, FIELD)) return;
            break;
        default:
            TOKEN_ERR("'static' or 'field'");
    }

    // type
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case INT:
            if (!write_keyword(pThis, INT)) return;
            break;
        case CHAR:
            if (!write_keyword(pThis, CHAR)) return;
            break;
        case BOOLEAN:
            if (!write_keyword(pThis, BOOLEAN)) return;
            break;
        default:
            if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)
                write_identifier(pThis);
            else
                TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
    }

    // varName
    if (!write_identifier(pThis)) return;

    // (',' varName)*
    while ((p_tokenizer->tokenType(p_tokenizer) == SYMBOL) &&
        (!strcmp(p_tokenizer->symbol(p_tokenizer), ","))) {
        if (!write_symbol(pThis, ",")) return;
        if (!write_identifier(pThis)) return;
    }

    // ';'
    if (!write_symbol(pThis, ";")) return;

    fprintf(pThis->fp, "</classVarDec>\n");
}

void _compilation_engine_compileSubroutine(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    fprintf(pThis->fp, "<subroutineDec>\n");

    // ('constructor' | 'function' | 'method')
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case CONSTRUCTOR:
            if (!write_keyword(pThis, CONSTRUCTOR)) return;
            break;
        case FUNCTION:
            if (!write_keyword(pThis, FUNCTION)) return;
            break;
        case METHOD:
            if (!write_keyword(pThis, METHOD)) return;
            break;
        default:
            TOKEN_ERR("'constructor' or 'function' or 'method'");
    }

    // ('void' or type)
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case VOID:
            if (!write_keyword(pThis, VOID)) return;
            break;
        case INT:
            if (!write_keyword(pThis, INT)) return;
            break;
        case CHAR:
            if (!write_keyword(pThis, CHAR)) return;
            break;
        case BOOLEAN:
            if (!write_keyword(pThis, BOOLEAN)) return;
            break;
        default:
            if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)
                write_identifier(pThis);
            else
                TOKEN_ERR("'void' or 'int' or 'char' or 'boolean' or 'identifier'");
    }

    // subroutineName
    if (!write_identifier(pThis)) return;

    // '('
    if (!write_symbol(pThis, "(")) return;

    // parameterList
    pThis->compileParameterlist(pThis);

    // ')'
    if (!write_symbol(pThis, ")")) return;

    // subroutineBody
    fprintf(pThis->fp, "<subroutineBody>\n");

    // "{"
    if (!write_symbol(pThis, "{")) return;

    // verDec*
    while (p_tokenizer->tokenType(p_tokenizer) == KEYWORD && p_tokenizer->keyWord(p_tokenizer) == VAR)
        pThis->compileVarDec(pThis);

    // statements
    pThis->compileStatements(pThis);

    // "}"
    if (!write_symbol(pThis, "}")) return;

    fprintf(pThis->fp, "</subroutineBody>\n");

    fprintf(pThis->fp, "</subroutineDec>\n");
}

void _compilation_engine_compileParameterlist(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;

    fprintf(pThis->fp, "<parameterList>\n");

    for (i = 0; (i < 2) && (is_type(p_tokenizer)); i++) {
        // type
        switch (p_tokenizer->keyWord(p_tokenizer)) {
            case INT:
                if (!write_keyword(pThis, INT)) return;
                break;
            case CHAR:
                if (!write_keyword(pThis, CHAR)) return;
                break;
            case BOOLEAN:
                if (!write_keyword(pThis, BOOLEAN)) return;
                break;
            default:
                if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)
                    write_identifier(pThis);
                else
                    TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
        }

        // varName
        if (!write_identifier(pThis)) return;

        while (p_tokenizer->tokenType(p_tokenizer) == SYMBOL
                && !strcmp(",", p_tokenizer->symbol(p_tokenizer))) {
            // ','
            if (!write_symbol(pThis, ",")) return;

            // type
            switch (p_tokenizer->keyWord(p_tokenizer)) {
                case INT:
                    if (!write_keyword(pThis, INT)) return;
                    break;
                case CHAR:
                    if (!write_keyword(pThis, CHAR)) return;
                    break;
                case BOOLEAN:
                    if (!write_keyword(pThis, BOOLEAN)) return;
                    break;
                default:
                    if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)
                        write_identifier(pThis);
                    else
                        TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
            }

            // varName
            if (!write_identifier(pThis)) return;
        }
    };

    fprintf(pThis->fp, "</parameterList>\n");

    return;

}

void _compilation_engine_compileVarDec(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    fprintf(pThis->fp, "<varDec>\n");

    // 'var'
    if (!write_keyword(pThis, VAR)) return;

    // type
    switch (p_tokenizer->keyWord(p_tokenizer)) {
        case INT:
            if (!write_keyword(pThis, INT)) return;
            break;
        case CHAR:
            if (!write_keyword(pThis, CHAR)) return;
            break;
        case BOOLEAN:
            if (!write_keyword(pThis, BOOLEAN)) return;
            break;
        default:
            if (p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)
                write_identifier(pThis);
            else
                TOKEN_ERR("'int' or 'char' or 'boolean' or 'identifier'");
    }

    // varName
    if (!write_identifier(pThis)) return;

    // (',' , vafName)*
    while (p_tokenizer->tokenType(p_tokenizer) == SYMBOL
            && !strcmp(",", p_tokenizer->symbol(p_tokenizer))) {
        if (!write_symbol(pThis, ",")) return;
        if (!write_identifier(pThis)) return;
    }

    // ';'
    if (!write_symbol(pThis, ";")) return;

    fprintf(pThis->fp, "</varDec>\n");
}

void _compilation_engine_compileStatements(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    fprintf(pThis->fp, "<statements>\n");

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

    fprintf(pThis->fp, "</statements>\n");
}

void _compilation_engine_compileDo(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    fprintf(pThis->fp, "<doStatement>\n");

    // 'do'
    if (!write_keyword(pThis, DO)) return;

    // subroutineCall
    // subroutineName or className or varName
    if (!write_identifier(pThis)) return;

    if (!strcmp(p_tokenizer->symbol(p_tokenizer), "(")) {
        // '('
        if (!write_symbol(pThis, "(")) return;

        // expressionnList
        pThis->compileExpressionList(pThis);

        // ')'
        if (!write_symbol(pThis, ")")) return;

    } else if (!strcmp(p_tokenizer->symbol(p_tokenizer), ".")) {
        // '.'
        if (!write_symbol(pThis, ".")) return;

        // subroutinenName
        if (!write_identifier(pThis)) return;

        // '('
        if (!write_symbol(pThis, "(")) return;

        // expressionnList
        pThis->compileExpressionList(pThis);

        // ')'
        if (!write_symbol(pThis, ")")) return;
    } else {
        return;
    }

    // ';'
    if (!write_symbol(pThis, ";")) return;

    fprintf(pThis->fp, "</doStatement>\n");
}

void _compilation_engine_compileLet(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;

    fprintf(pThis->fp, "<letStatement>\n");

    // 'let'
    if (!write_keyword(pThis, LET)) return;

    // varName
    if (!write_identifier(pThis)) return;

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

    fprintf(pThis->fp, "<returnStatement>\n");

    // 'return'
    if (!write_keyword(pThis, RETURN)) return;

    // expression?
    for (i = 0; (i < 2) && is_begin_of_term(p_tokenizer); i++)
        pThis->compileExpression(pThis);

    // ';'
    if (!write_symbol(pThis, ";")) return;

    fprintf(pThis->fp, "</returnStatement>\n");
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

    fprintf(pThis->fp, "<expression>\n");

    // term
    pThis->compileTerm(pThis);

    // (op term)*
    while (is_op(p_tokenizer)) {
        if(!write_symbol(pThis, p_tokenizer->current_token)) return;

        pThis->compileTerm(pThis);
    }

    fprintf(pThis->fp, "</expression>\n");
}

void _compilation_engine_compileTerm(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    fprintf(pThis->fp, "<term>\n");

    switch (p_tokenizer->tokenType(p_tokenizer)) {
        case INT_CONST:
            // integerConstant
            if (!write_integerConstant(pThis)) return;
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
                if (!write_symbol(pThis, "(")) return;
                pThis->compileExpression(pThis);
                if (!write_symbol(pThis, ")")) return;

            } else {
                TOKEN_ERR("-, ~ or '(' ");
                return;
            }
            break;

        case INDENTIFIER:
            // common element "identifier"
            if (!write_identifier(pThis)) return;

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
                    // subroutinenName
                    if (!write_identifier(pThis)) return;

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

    fprintf(pThis->fp, "</term>\n");
}

void _compilation_engine_compileExpressionList(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);
    int i;

    fprintf(pThis->fp, "<expressionList>\n");

    for (i = 0; (i < 2) && (is_begin_of_expression(p_tokenizer)); i++) {
        // expression
        pThis->compileExpression(pThis);

        while (!strcmp(p_tokenizer->symbol(p_tokenizer), ",")) {
            // ','
            if (!write_symbol(pThis, ",")) return;

            // expression
            pThis->compileExpression(pThis);
        }
    };

    fprintf(pThis->fp, "</expressionList>\n");

    return;
}

void _compilation_engine_del(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    p_tokenizer->del(p_tokenizer);

    fclose(pThis->fp); 
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
    // fprintf(pThis->fp, "<keyword> %s </keyword>\n", str[keyword]);
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

static bool write_integerConstant(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    if (!(p_tokenizer->tokenType(p_tokenizer) == INT_CONST)) {
        TOKEN_ERR("integerConstant");
        return false;
    }
    // fprintf(pThis->fp, "<integerConstant> %d </integerConstant>\n", p_tokenizer->intVal(p_tokenizer));
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

static bool write_identifier(CompilationEngine *pThis)
{
    JackTokenizer *p_tokenizer = &(pThis->tokenizer);

    if (!(p_tokenizer->tokenType(p_tokenizer) == INDENTIFIER)) {
        TOKEN_ERR("identifier");
        return false;
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
