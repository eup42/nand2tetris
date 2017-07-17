/*
 *  parser.c 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include "parser.h"

#define BUFF_BLOCK_SIZE 1024
#define LINE_BLOCK_SIZE 1024
#define SIZE_OF_ARRAY(a) ((sizeof(a)) / (sizeof(a[0])))

static void trimStartSpaces(char *str);
static void trimEndSpaces(char *str);
static void uniteBlanks(char *str);
static void trimComments(char *str);

static bool hasCommandsInList(char *command, const char **list, size_t size);

void _parser_init(Parser *pThis, char *name)
{
    FILE *fp;
    char *buff;
    int ch;
    int indx = 0;
    int line_num = 0;
    int buff_block_num = 1;
    int line_block_num = 1;

    fp = fopen(name, "r");

    if (fp == NULL) {
        perror("Error");
        exit errno;
    }

    buff = (char *)malloc(sizeof(unsigned char) * BUFF_BLOCK_SIZE * buff_block_num);
    pThis->lines = (char **)malloc(sizeof(char *)
                                    * LINE_BLOCK_SIZE * line_block_num);

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n' || ch == '\r') {
            buff[indx] = '\0';

            trimStartSpaces(buff);
            trimEndSpaces(buff);
            uniteBlanks(buff);
            trimComments(buff);

            if (strlen(buff) != 0) {
                pThis->lines[line_num]
                    = (char *)malloc(sizeof(unsigned char) * strlen(buff) + 1);
                strncpy(pThis->lines[line_num], buff, strlen(buff) + 1);

                line_num++;
                if (line_num == LINE_BLOCK_SIZE * line_block_num) {
                    line_block_num++;
                    pThis->lines
                        = (char **)realloc(pThis->lines,
                            sizeof(char *)
                            * LINE_BLOCK_SIZE * line_block_num);
                }
            }
            continue;
        }

        buff[indx++] = (unsigned char)ch;

        if (indx == BUFF_BLOCK_SIZE * buff_block_num) {
            buff_block_num++;
            buff = (char *)realloc(buff,
                                    sizeof(ch) * BUFF_BLOCK_SIZE * buff_block_num);
        }
    }

    pThis->lines[line_num] = NULL;
    pThis->max_line = LINE_BLOCK_SIZE * line_block_num;

    free(buff);
    fclose(fp);
}

bool _parser_hasMoreCommands(Parser *pThis)
{
    return pThis->lines[pThis->current_line + 1] == NULL ? false : true;
}

void _parser_advance(Parser *pThis)
{
    pThis->current_line++;
}

void _parser_reset(Parser *pThis)
{
    pThis->current_line = -1;
}

int _parser_commandType(Parser *pThis)
{
    char *current_command = pThis->lines[pThis->current_line];
    static const char *list_arithmetric[] = {
        "add", "sub", "neg", "eq", "gt",
        "lt",  "and", "or",  "not",
    };
    static const char *list_pop[] = {"pop"};

    if (hasCommandsInList(current_command, list_arithmetric, SIZE_OF_ARRAY(list_arithmetric)))
        return C_ARITHMETRIC;
    else if (hasCommandsInList(current_command, list_pop, SIZE_OF_ARRAY(list_pop)))
        return C_POP;
    else
        return 0;
}

char *_parser_arg1(Parser *pThis) {
    char *current_command = pThis->lines[pThis->current_line];
    char *buf = (char *)malloc(sizeof(char) * strlen(current_command) + 1);
    char *arg1;

    for (; *buf != '\0'; buf++) {
        if (*buf == ' ') {
            arg1 = buf + 1;
            break;
        }
    }

    for (; *buf != '\0'; buf++) {
        if (*buf == ' ') {
            *buf = '\0';
            break;
        }
    }

    pThis->current_arg1 = (char *)malloc(sizeof(char) * strlen(buf) + 1);
    strcpy(pThis->current_arg1, buf);

    free(buf);
    return pThis->current_arg1;
}

int _parser_arg2(Parser *pThis) {
    char *current_command = pThis->lines[pThis->current_line];
    int cnt = 0;

    for (; *current_command != '\0'; current_command++) {
        if (*current_command == ' ') cnt++;
        if (cnt == 2) break;
    }

    return atoi(current_command);
}

void _parser_delete(Parser *pThis)
{
    int i;

    for (i = 0; i < pThis->max_line; i++) {
        free(pThis->lines[i]);
    }

    free(pThis->lines);
    free(pThis->current_arg1);
}


static bool hasCommandsInList(char *command, const char **list, size_t size)
{
    size_t i;
    char *buf = (char *)malloc(sizeof(char) * strlen(command) + 1);;
    bool ret = false;

    strcpy(buf, command);
    for (i = 0; buf[i] != '\0'; i++) {
        if (buf[i] == ' ') {
            buf[i] = '\0';
            break;
        }
    }

    for (i = 0; i < size; i++) {
        if (!strcmp(buf, list[i])) {
            ret = true;
            break;
        }
    }

    free(buf);
    return buf;
}

static void trimStartSpaces(char *str) {
    char *buf;
    char *orig = str;

    if (str == NULL || str[0] != ' ') return;

    buf = (char *)malloc(sizeof(char) * strlen(str) + 1);

    while (*str == ' ') str++;

    strcpy(buf, str);
    strcpy(orig, buf);

    free(buf);
    return;
}

static void trimEndSpaces(char *str) {
    int i;

    for (i = strlen(str); i != 0; i--) {
        if (str[i - 1] != ' ') {
            str[i] = '\0';
            return;
        }
    }

    str[0] = '\0';
    return;
}

static void uniteBlanks(char *str) {
    char *buf = (char *)malloc(sizeof(char) * strlen(str) + 1);
    char *orig = str;

    while ((*buf++ = *str++) != '\0') {
        if (*(str - 1) == ' ')
            while (*str == ' ') str++;
    }

    strcpy(orig, buf);
    free(buf);

    return;
}

static void trimComments(char *str) {
    while (*str != '\0') {
        if (*(str - 1) == '/' && *str == '/') {
            *(str - 1) = '\0';
            return;
        }
        str++;
    }
    return ;
}
