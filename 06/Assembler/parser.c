/*
 *  parser.c 
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "parser.h"

#define BUFF_BLOCK_SIZE 1024
#define LINE_BLOCK_SIZE 1024

struct data {
    char **lines;
    int  current_line;
    char *symbol;
    char *dest;
    char *comp;
    char *jump;
};

static struct data data = {
    .lines = NULL,
    .current_line = -1,
    .symbol = NULL,
    .dest = NULL,
    .comp = NULL,
    .jump = NULL,
};

void _parser_parserInit(char *name)
{
    FILE *fp;
    char *buff;
    int ch;
    int indx = 0;
    int line_num = 0;
    int slash_cnt = 0;
    int buff_block_num = 1;
    int line_block_num = 1;

    fp = fopen(name, "r");

    if (fp == NULL) {
        perror("Error");
        exit errno;
    }

    buff = (char *)malloc(sizeof(unsigned char) * BUFF_BLOCK_SIZE * buff_block_num);
    data.lines = (char **)malloc(sizeof(char *)
                                    * LINE_BLOCK_SIZE * line_block_num);

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            buff[indx] = '\0';

            if (strlen(buff) != 0) {
                data.lines[line_num]
                    = (char *)malloc(sizeof(unsigned char) * strlen(buff) + 1);
                strncpy(data.lines[line_num], buff, strlen(buff) + 1);

                line_num++;
                if (line_num == LINE_BLOCK_SIZE * line_block_num) {
                    line_block_num++;
                    data.lines
                        = (char **)realloc(data.lines,
                            sizeof(char *)
                            * LINE_BLOCK_SIZE * line_block_num);
                }
            }

            // Line initialize
            indx = 0;
            slash_cnt = 0;
            continue;
        }

        if (ch == '/') {
            slash_cnt++;
            continue;
        }

        if (ch == ' ' || ch == '\r' || slash_cnt > 1) continue;

        buff[indx] = (unsigned char)ch;

        indx++;
        if (indx == BUFF_BLOCK_SIZE * buff_block_num) {
            buff_block_num++;
            buff = (char *)realloc(buff,
                                    sizeof(ch) * BUFF_BLOCK_SIZE * buff_block_num);
        }
    }

    data.lines[line_num] = NULL;

    free(buff);
    fclose(fp);
}

bool _parser_hasMoreCommands(void)
{
    return data.lines[data.current_line + 1] == NULL ? false : true;
}

void _parser_advance(void)
{
    data.current_line++;
}

void _parser_reset(void)
{
    data.current_line = -1;
}

int _parser_commandType(void)
{
    char *current_command = data.lines[data.current_line];

    if (current_command[0] == '@')
        return A_COMMAND;
    else if (current_command[0] == '(')
        return L_COMMAND;
    else 
        return C_COMMAND;
}

char *_parser_symbol(void)
{
    char *current_command = data.lines[data.current_line];
    size_t len = strlen(current_command);

    free(data.symbol);
    data.symbol = (char *)malloc(sizeof(unsigned char) * len + 1);

    if (data.symbol == NULL) return NULL;

    if (current_command[0] == '@') {
        strncpy(data.symbol, &current_command[1], len - 1 + 1);
        return data.symbol;

    } else if (current_command[0] == '(') {
        if (current_command[len] != ')') {
            printf("Error: Command is not terminated as ')'. Command = %s\n",
                    current_command);
            return NULL;
        }
        strncpy(data.symbol, &current_command[1], len - 2 + 1);
        return data.symbol;

    } else {
        printf("Error: Command is invalid. Command = %s\n",  current_command);
        return NULL;
    }
}

char *_parser_dest(void)
{
    char *current_command;
    size_t len = strlen(data.lines[data.current_line]);
    char *dest;

    free(data.dest);
    data.dest = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, data.lines[data.current_line], len + 1);

    dest = strtok(current_command, "=");

    if (dest != NULL) {
        strncpy(data.dest, dest, strlen(dest) + 1);
    } else {
        strncpy(data.dest, "null", strlen("null") + 1);
    }

    free(current_command);
    return data.dest;
}


char *_parser_comp(void)
{
    char *current_command = data.lines[data.current_line];
    size_t len = strlen(current_command);
    char *dest;
    char *comp;

    free(data.comp);
    data.comp = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, data.lines[data.current_line], len + 1);

    dest = strtok(current_command, "=");

    if (dest != NULL)
        comp = strtok(NULL, "=");
    else
        comp = strtok(current_command, ";");

    if (comp != NULL) {
        strncpy(data.comp, comp, strlen(comp) + 1);
    } else {
        printf("Error: comp field is not existing.\n");
        data.comp[0] = '\0';
    }

    free(current_command);
    return data.comp;
}

char *_parser_jump(void)
{
    char *current_command = data.lines[data.current_line];
    size_t len = strlen(current_command);
    char *comp = NULL;
    char *jump = NULL;

    free(data.jump);
    data.jump = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, data.lines[data.current_line], len + 1);

    comp = strtok(current_command, ";");

    if (comp != NULL) jump = strtok(NULL, ";");

    if (jump != NULL) {
        strncpy(data.jump, jump, strlen(jump) + 1);
    } else {
        strncpy(data.jump, "null", strlen("null") + 1);
    }

    free(current_command);
    return data.jump;
}
