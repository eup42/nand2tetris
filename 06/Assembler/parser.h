/*
 *  parser.h 
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>

enum commandType {
    A_COMMAND,
    C_COMMAND,
    L_COMMAND,
};

typedef struct parser {
    char **lines;
    int  current_line;
    char *current_symbol;
    char *current_dest;
    char *current_comp;
    char *current_jump;

    void (*parserInit)(struct parser *, char *);
    bool (*hasMoreCommands)(struct parser *);
    void (*advance)(struct parser *);
    void (*reset)(struct parser *);
    int  (*commandType)(struct parser *);
    char *(*symbol)(struct parser *);
    char *(*dest)(struct parser *);
    char *(*comp)(struct parser *);
    char *(*jump)(struct parser *);
} Parser;

extern void _parser_parserInit(Parser *pThis, char *name);
extern bool _parser_hasMoreCommands(Parser *pThis);
extern void _parser_advance(Parser *pThis);
extern void _parser_reset(Parser *pThis);
extern int  _parser_commandType(Parser *pThis);
extern char *_parser_symbol(Parser *pThis);
extern char *_parser_dest(Parser *pThis);
extern char *_parser_comp(Parser *pThis);
extern char *_parser_jump(Parser *pThis);

#define newParser() {       \
    .lines = NULL,          \
    .current_line = -1,     \
    .current_symbol = NULL, \
    .current_dest = NULL,   \
    .current_comp = NULL,   \
    .current_jump = NULL,   \
    .parserInit = _parser_parserInit,           \
    .hasMoreCommands = _parser_hasMoreCommands, \
    .advance = _parser_advance,                 \
    .reset = _parser_reset,                     \
    .commandType = _parser_commandType,         \
    .symbol = _parser_symbol,                   \
    .dest = _parser_dest,                       \
    .comp = _parser_comp,                       \
    .jump = _parser_jump,                       \
}

#endif
