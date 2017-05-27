/*
 *  parser.h 
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include <stdbool.h>

void parserInit(char *name);
bool hasMoreCommands(void);
void advance(void);
int  commandType(void);
char *symbol(void);
char *dest(void);
char *comp(void);
char *jump(void);

enum commandType {
    A_COMMAND,
    C_COMMAND,
    L_COMMAND,
};

#endif _PARSER_H_
