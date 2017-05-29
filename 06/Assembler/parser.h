/*
 *  parser.h 
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>

extern void _parser_parserInit(char *name);
extern bool _parser_hasMoreCommands(void);
extern void _parser_advance(void);
extern void _parser_reset(void);
extern int  _parser_commandType(void);
extern char *_parser_symbol(void);
extern char *_parser_dest(void);
extern char *_parser_comp(void);
extern char *_parser_jump(void);

enum commandType {
    A_COMMAND,
    C_COMMAND,
    L_COMMAND,
};

const static struct parser {
    void (*parserInit)(char *);
    bool (*hasMoreCommands)(void);
    void (*advance)(void);
    void (*reset)(void);
    int  (*commandType)(void);
    char *(*symbol)(void);
    char *(*dest)(void);
    char *(*comp)(void);
    char *(*jump)(void);
} parser = {
    .parserInit      = _parser_parserInit,
    .hasMoreCommands = _parser_hasMoreCommands,
    .advance         = _parser_advance,
    .reset           = _parser_reset,
    .commandType     = _parser_commandType,
    .symbol          = _parser_symbol,
    .dest            = _parser_dest,
    .comp            = _parser_comp,
    .jump            = _parser_jump,
};

#endif
