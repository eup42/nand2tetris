/*
 *  common.h 
 */

#ifndef _COMMON_H_
#define _COMMON_H_

enum TokenType {
    KEYWORD,
    SYMBOL,
    INDENTIFIER,
    INT_CONST,
    STRING_CONST,
    MAX_TOKEN_TYPE,
};


enum KeyWord {
    CLASS,
    METHOD,
    FUNCTION,
    CONSTRUCTOR,
    INT,
    BOOLEAN,
    CHAR,
    VOID,
    VAR,
    STATIC,
    FIELD,
    LET,
    DO,
    IF,
    ELSE,
    WHILE,
    RETURN,
    TRUE,
    FALSE,
    NUL,
    THIS,
    MAX_KEYWORD,
};

#endif
