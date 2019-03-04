/*
 * jack_tokenizer.c
 */

#include "jack_tokenizer.h"
#include "common.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define SIZE_OF_ARRAY(a) ((sizeof(a)) / (sizeof(a[0])))

const char *keyword[] = {"class", "method", "function", "constructor", "int",    "boolean",
                         "char",  "void",   "var",      "static",      "field",  "let",
                         "do",    "if",     "else",     "while",       "return", "true",
                         "false", "null",   "this",
                        };

static bool isKeyword(char *str);
static bool isSymbol(char *str);
static bool isConstantInt(char *str);
static bool isConstantStr(char *str);
static bool isIdentifier(char *str);
static struct token getTokenInfo(char *str);
static void printToken(struct token token, FILE *fp);
static char *formatXML(const char *in, char *out);

void _jack_tokenizer_init(JackTokenizer *pThis, char *name)
{
    pThis->fp = fopen(name, "r");

    if (pThis->fp == NULL) {
        fprintf(stderr, "%s %s() : %s\n", __FILE__, __FUNCTION__, strerror(errno));
        exit errno;
    }

    memset(&pThis->token, 0, sizeof(pThis->token));
    pThis->current_token = NULL;

    return;
}

bool _jack_tokenizer_hasMoreTokens(JackTokenizer *pThis)
{
    int c;
    bool oneline_comment = false;
    bool multiline_commnent = false;
    char buf[4] = {0};
    unsigned long int i = 0;

    while ((c = fgetc(pThis->fp)) != EOF) {
        buf[i % 4] = (char)c;
        i++;

        if (oneline_comment) {
            if (c == '\n') {
                oneline_comment = false;
                continue;
            } else {
                continue;
            }
        }

        if (multiline_commnent) {
            if (buf[(i - 2) % 4] == '*' && c == '/') {
                multiline_commnent = false;
                continue;
            } else {
                continue;
            }
        }

        if (buf[(i - 2) % 4] == '/') {
            if (c == '*') {
                multiline_commnent = true;
                continue;
            } else if (c == '/') {
                oneline_comment = true;
                continue;
            } else if (buf[(i - 3) % 4] != '*'){
                fseek(pThis->fp, -2, SEEK_CUR);
                return true;
            }
        }

        if ((c == ' ') || (c == '\n') || (c == '\r') || (c == '/') || (c == '\t')) continue;

        fseek(pThis->fp, -1, SEEK_CUR);
        return true;
    }

    if (buf[(i - 2) % 4] == '/') {
        fseek(pThis->fp, -2, SEEK_CUR);
        return true;
    } else {
        return false;
    }
}

void _jack_tokenizer_advance(JackTokenizer *pThis)
{
    char c;
    char *buf;
    unsigned int i;
    static const int buf_block_size = 256;
    bool in_quote = false;;

    buf = (char *)malloc(sizeof(char *) * buf_block_size);

    for (i = 0; (c = fgetc(pThis->fp)) != EOF; i++) {
        if (c == '\n') break;

        if (!in_quote) {
            if (c == ' ') break;

            if (i == 0) {
                if (c == '"' ) in_quote = true;
            } else {
                if (buf[i - 1] == '/' && (c == '/' || c == '*')) {
                    fseek(pThis->fp, -1, SEEK_CUR);
                    break;
                }

                if (!(isalpha(buf[0]) || isdigit(buf[0]) || buf[0] == '_')) {
                    break;
                }

                if (!(isalpha(c) || isdigit(c) || c == '_' || c == '"')) {
                    break;
                }
            }
        } else {
            if (i != 1 && buf[i - 1] == '"') break;
        }


        buf[i] = c;

        if ((i % buf_block_size) == (buf_block_size - 1)) {
            buf = (char *)realloc(buf, sizeof(char *) * (i / (buf_block_size + 1)));
        }
    }
    if (c != EOF) fseek(pThis->fp, -1, SEEK_CUR);
    buf[i] = '\0';

    free(pThis->current_token);
    pThis->current_token = (char *)malloc(sizeof(char *) * (strlen(buf) + 1));
    strcpy(pThis->current_token, buf);

    free(buf);

    pThis->token = getTokenInfo(pThis->current_token);

    return;
}

enum TokenType _jack_tokenizer_tokenType(JackTokenizer *pThis)
{
    return pThis->token.type;
}

enum KeyWord _jack_tokenizer_keyWord(JackTokenizer *pThis)
{
    return pThis->token.data.keyword;
}

char *_jack_tokenizer_symbol(JackTokenizer *pThis)
{
    return pThis->token.data.symbol;
}

char *_jack_tokenizer_identifier(JackTokenizer *pThis)
{
    return pThis->token.data.identifier;
}

int  _jack_tokenizer_intVal(JackTokenizer *pThis)
{
    return pThis->token.data.int_const;
}

char *_jack_tokenizer_stringVal(JackTokenizer *pThis)
{
    return pThis->token.data.str_const;
}

void _jack_tokenizer_print_cur_token(JackTokenizer *pThis, FILE *fp)
{
    printToken(pThis->token, fp);
}

void _jack_tokenizer_del(JackTokenizer *pThis)
{
    fclose(pThis->fp);
    memset(&pThis->token, 0, sizeof(pThis->token));
    free(pThis->current_token);

    return;
}

static bool isKeyword(char *str)
{
    int i;

    for (i = 0; i < (int)SIZE_OF_ARRAY(keyword); i++) {
        if (!strcmp(str, keyword[i])) return true;
    }

    return false;
}

static bool isSymbol(char *str)
{
    const char *symbol[] = {"{", "}", "(", ")", "[", "]", ".", ",", ";", "+", "-",
                            "*", "/", "&", "|", "<", ">", "=", "~",
                           };
    int i;

    for (i = 0; i < (int)SIZE_OF_ARRAY(symbol); i++) {
        if (!strcmp(str, symbol[i])) return true;
    }

    return false;
}

static bool isConstantInt(char *str)
{
    int i;
    for (i = 0; i < (int)strlen(str); i++) {
        if (!isdigit(str[i])) return false;
    }

    return true;
}

static bool isConstantStr(char *str)
{
    int i = 0;

    if (str[0] != '"' || str[strlen(str) - 1] != '"') return false;

    for (i = 1; i < (int)strlen(str) - 1; i++) {
        if (str[i] == '\n') return false;
    }
    return true;
}

static bool isIdentifier(char *str)
{
    int i;
    for (i = 0; i < (int)strlen(str); i++) {
        if (!(isdigit(str[i]) || isalpha(str[i]) || str[i] == '_')) return false;
    }

    return true;
}

static struct token getTokenInfo(char *str)
{
    struct token token;
    int i;

    if (isKeyword(str)) {
        token.type = KEYWORD;
        token.data.keyword = MAX_KEYWORD;
        for (i = 0; i < MAX_KEYWORD; i++) {
            if (!strcmp(str, keyword[i]))
                token.data.keyword = i;
        }

    } else if (isSymbol(str)) {
        token.type = SYMBOL;
        strncpy(token.data.symbol, str, sizeof(token.data.symbol) - 1);
        token.data.symbol[sizeof(token.data.symbol) - 1] = '\0';

    } else if (isConstantInt(str)) {
        token.type = INT_CONST;
        token.data.int_const = atoi(str);

    } else if (isIdentifier(str)) {
        token.type = INDENTIFIER;
        strncpy(token.data.identifier, str, sizeof(token.data.identifier) - 1);
        token.data.identifier[sizeof(token.data.identifier) - 1] = '\0';

    } else if (isConstantStr(str)) {
        token.type = STRING_CONST;

        strncpy(token.data.str_const, &str[1], sizeof(token.data.identifier) - 1);
        token.data.identifier[sizeof(token.data.identifier) - 1] = '\0';

        for (i = 0; token.data.identifier[i] != '"'; i++);
        if (token.data.identifier[i] == '"' ) token.data.identifier[i] = '\0';

    } else {
        token.type = MAX_TOKEN_TYPE;
    }

    return token;
}

static void printToken(struct token token, FILE *fp)
{
    char buf[256] = {0};

    switch (token.type) {
        case KEYWORD:
            fprintf(fp, "<keyword> %s </keyword>\n", keyword[token.data.keyword]);
            break;
        case SYMBOL:
            fprintf(fp, "<symbol> %s </symbol>\n", formatXML(token.data.symbol, buf));
            break;
        case INT_CONST:
            fprintf(fp, "<integerConstant> %d </integerConstant>\n", token.data.int_const);
            break;
        case STRING_CONST:
            fprintf(fp, "<stringConstant> %s </stringConstant>\n", token.data.str_const);
            break;
        case INDENTIFIER:
            fprintf(fp, "<identifier> %s </identifier>\n", token.data.identifier);
            break;
        default:
            break;
    }
}


static char *formatXML(const char *in, char *out)
{
    if (!strcmp(in, "<")) {
        strcpy(out, "&lt;");
    } else if (!strcmp(in, ">")) {
        strcpy(out, "&gt;");
    } else if (!strcmp(in, "&")) {
        strcpy(out, "&amp;");
    } else {
        strcpy(out, in);
    }

    return out;

}
