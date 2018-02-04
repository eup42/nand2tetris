/*
 *  jack_tokenizer.h 
 */

#ifndef _JACK_TOKENIZER_H_
#define _JACK_TOKENIZER_H_

#include <stdbool.h>
#include <stdio.h>
#include "common.h"

union token_data {
    enum KeyWord keyword;
    char         symbol[2];
    unsigned int int_const;
    char         str_const[256];
    char         identifier[256];
};

struct token {
    enum TokenType type;
    union token_data data;
};

typedef struct jack_tokenizer {
    FILE *fp;
    char *current_token;
    struct token token;

    void (*init)(struct jack_tokenizer *, char *);
    bool (*hasMoreTokens)(struct jack_tokenizer *);
    void (*advance)(struct jack_tokenizer *);
    enum TokenType (*tokenType)(struct jack_tokenizer *);
    enum KeyWord (*keyWord)(struct jack_tokenizer *);
    char *(*symbol)(struct jack_tokenizer *);
    char *(*identifier)(struct jack_tokenizer *);
    int  (*intVal)(struct jack_tokenizer *);
    char *(*stringVal)(struct jack_tokenizer *);
    void (*printCurrentToken)(struct jack_tokenizer *, FILE *fp);
    void (*del)(struct jack_tokenizer *);
} JackTokenizer;

extern void _jack_tokenizer_init(JackTokenizer *pThis, char *name);
extern bool _jack_tokenizer_hasMoreTokens(JackTokenizer *pThis);
extern void _jack_tokenizer_advance(JackTokenizer *pThis);
extern enum TokenType _jack_tokenizer_tokenType(JackTokenizer *pThis);
extern enum KeyWord _jack_tokenizer_keyWord(JackTokenizer *pThis);
extern char *_jack_tokenizer_symbol(JackTokenizer *pThis);
extern char *_jack_tokenizer_identifier(JackTokenizer *pThis);
extern int  _jack_tokenizer_intVal(JackTokenizer *pThis);
extern char *_jack_tokenizer_stringVal(JackTokenizer *pThis);
extern void _jack_tokenizer_del(JackTokenizer *pThis);
extern void _jack_tokenizer_print_cur_token(JackTokenizer *pThis, FILE *fp);

#define newJackTokenizer() {       \
    .init              = _jack_tokenizer_init,              \
    .hasMoreTokens     = _jack_tokenizer_hasMoreTokens,     \
    .advance           = _jack_tokenizer_advance,           \
    .tokenType         = _jack_tokenizer_tokenType,         \
    .keyWord           = _jack_tokenizer_keyWord,           \
    .symbol            = _jack_tokenizer_symbol,            \
    .identifier        = _jack_tokenizer_identifier,        \
    .intVal            = _jack_tokenizer_intVal,            \
    .stringVal         = _jack_tokenizer_stringVal,         \
    .printCurrentToken = _jack_tokenizer_print_cur_token,   \
    .del               = _jack_tokenizer_del                \
}

#endif
