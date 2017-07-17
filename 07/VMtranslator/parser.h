/*
 *  parser.h 
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>

enum commandType {
    C_ARITHMETRIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
};

typedef struct parser {
    char **lines;
    int  max_line;
    int  current_line;
    char *current_arg1;

    void (*init)(struct parser *, char *);
    bool (*hasMoreCommands)(struct parser *);
    void (*advance)(struct parser *);
    void (*reset)(struct parser *);
    int  (*commandType)(struct parser *);
    char *(*arg1)(struct parser *);
    int  (*arg2)(struct parser *);
    void (*del)(struct parser *);
} Parser;

extern void _parser_init(Parser *pThis, char *name);
extern bool _parser_hasMoreCommands(Parser *pThis);
extern void _parser_advance(Parser *pThis);
extern void _parser_reset(Parser *pThis);
extern int  _parser_commandType(Parser *pThis);
extern char *_parser_arg1(Parser *pThis);
extern int  _parser_arg2(Parser *pThis);
extern void _parser_delete(Parser *pThis);

#define newParser() {       \
    .lines = NULL,          \
    .max_line = 0,          \
    .current_line = -1,     \
    .current_arg1 = NULL,   \
    .init = _parser_init,   \
    .hasMoreCommands = _parser_hasMoreCommands, \
    .advance = _parser_advance,                 \
    .reset = _parser_reset,                     \
    .commandType = _parser_commandType,         \
    .arg1 = _parser_arg1,                       \
    .arg2 = _parser_arg2,                       \
    .del = _parser_delete,                      \
}

#endif
