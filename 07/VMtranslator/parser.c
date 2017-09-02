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

            trimComments(buff);
            trimStartSpaces(buff);
            trimEndSpaces(buff);
            uniteBlanks(buff);

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
            indx = 0;
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

enum commandType _parser_commandType(Parser *pThis)
{
    char *current_command = pThis->lines[pThis->current_line];
    static const char *list_arithmetric[] = {
        "add", "sub", "neg", "eq", "gt",
        "lt",  "and", "or",  "not",
    };
    static const char *list_pop[] = {"pop"};
    static const char *list_push[] = {"push"};
    static const char *list_label[] = {"label"};
    static const char *list_goto[] = {"goto"};
    static const char *list_if_goto[] = {"if-goto"};

    if (hasCommandsInList(current_command, list_arithmetric, SIZE_OF_ARRAY(list_arithmetric)))
        return C_ARITHMETRIC;
    else if (hasCommandsInList(current_command, list_pop, SIZE_OF_ARRAY(list_pop)))
        return C_POP;
    else if (hasCommandsInList(current_command, list_push, SIZE_OF_ARRAY(list_pop)))
        return C_PUSH;
    else if (hasCommandsInList(current_command, list_label, SIZE_OF_ARRAY(list_label)))
        return C_LABEL;
    else if (hasCommandsInList(current_command, list_goto, SIZE_OF_ARRAY(list_goto)))
        return C_GOTO;
    else if (hasCommandsInList(current_command, list_if_goto, SIZE_OF_ARRAY(list_if_goto)))
        return C_IF;
    else
        return C_OTHER;
}

char *_parser_arg1(Parser *pThis)
{
    const char *current_command = pThis->lines[pThis->current_line];
    char *buf, *arg1;
    char *space;

    buf = (char *)malloc(sizeof(char) * strlen(current_command) + 1);
    strcpy(buf, current_command);

    // get arg1 position and strings
    if (pThis->commandType(pThis) == C_ARITHMETRIC) {
        arg1 = buf;
    } else {
        if ((arg1 = strchr(buf, ' ')) ==  NULL)
            return NULL;

        arg1 += 1;

        // if there is space after arg1
        if ((space = strchr(arg1, ' ')) != NULL) *space = '\0';
    }

    pThis->current_arg1 = (char *)malloc(sizeof(char) * strlen(arg1) + 1);
    strcpy(pThis->current_arg1, arg1);

    free(buf);
    return pThis->current_arg1;
}

int _parser_arg2(Parser *pThis)
{
    char *current_command = pThis->lines[pThis->current_line];
    char *index;

    index = strrchr(current_command, ' ');

    if (index == NULL) return 0;

    return atoi(index + 1);
}

void _parser_delete(Parser *pThis)
{
    int i;

    for (i = 0; pThis->lines[i] != NULL; i++) {
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
    return ret;
}

static void trimStartSpaces(char *str)
{
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

static void trimEndSpaces(char *str)
{
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

static void uniteBlanks(char *str)
{
    char *buf = (char *)malloc(sizeof(char) * strlen(str) + 1);
    char *orig = str;
    char *orig_buf = buf;

    while ((*buf++ = *str++) != '\0') {
        if (*(str - 1) == ' ')
            while (*str == ' ') str++;
    }

    strcpy(orig, orig_buf);
    free(orig_buf);

    return;
}

static void trimComments(char *str)
{
    while (*str != '\0') {
        if (*(str - 1) == '/' && *str == '/') {
            *(str - 1) = '\0';
            return;
        }
        str++;
    }
    return ;
}
