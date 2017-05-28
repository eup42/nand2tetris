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

struct parser {
    char **lines;
    int  current_line;
    char *symbol;
    char *dest;
    char *comp;
    char *jump;
};

static struct parser parser = {
    .lines = NULL,
    .current_line = -1,
    .symbol = NULL,
    .dest = NULL,
    .comp = NULL,
    .jump = NULL,
};

void parserInit(char *name)
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
    parser.lines = (char **)malloc(sizeof(char *)
                                    * LINE_BLOCK_SIZE * line_block_num);

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            buff[indx] = '\0';

            if (strlen(buff) != 0) {
                parser.lines[line_num]
                    = (char *)malloc(sizeof(unsigned char) * strlen(buff) + 1);
                strncpy(parser.lines[line_num], buff, strlen(buff) + 1);

                line_num++;
                if (line_num == LINE_BLOCK_SIZE * line_block_num) {
                    line_block_num++;
                    parser.lines
                        = (char **)realloc(parser.lines,
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

    parser.lines[line_num] = NULL;

    free(buff);
    for (line_num = 0; parser.lines[line_num] != NULL; line_num++)
        printf("%s\n", parser.lines[line_num]);

    fclose(fp);
}

bool hasMoreCommands(void)
{
    return parser.lines[parser.current_line + 1] == NULL ? false : true;
}

void advance(void)
{
    parser.current_line++;
}

int commandType(void)
{
    char *current_command = parser.lines[parser.current_line];

    if (current_command[0] == '@')
        return A_COMMAND;
    else if (current_command[0] == '(')
        return L_COMMAND;
    else 
        return C_COMMAND;
}

char *symbol(void)
{
    char *current_command = parser.lines[parser.current_line];
    size_t len = strlen(current_command);

    free(parser.symbol);
    parser.symbol = (char *)malloc(sizeof(unsigned char) * len + 1);

    if (parser.symbol == NULL) return NULL;

    if (current_command[0] == '@') {
        strncpy(parser.symbol, &current_command[1], len - 1 + 1);
        return parser.symbol;

    } else if (current_command[0] == '(') {
        if (current_command[len] != ')') {
            printf("Error: Command is not terminated as ')'. Command = %s\n",
                    current_command);
            return NULL;
        }
        strncpy(parser.symbol, &current_command[1], len - 2 + 1);
        return parser.symbol;

    } else {
        printf("Error: Command is invalid. Command = %s\n",  current_command);
        return NULL;
    }
}

char *dest(void)
{
    char *current_command;
    size_t len = strlen(parser.lines[parser.current_line]);
    char *dest;

    free(parser.dest);
    parser.dest = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, parser.lines[parser.current_line], len + 1);

    dest = strtok(current_command, "=");

    if (dest == NULL) {
        strncpy(parser.dest, "null", strlen("null") + 1);
    } else {
        strncpy(parser.dest, dest, strlen(dest) + 1);
    }

    free(current_command);
    return parser.dest;
}


char *comp(void)
{
    char *current_command = parser.lines[parser.current_line];
    size_t len = strlen(current_command);
    char *dest;
    char *comp;

    free(parser.comp);
    parser.comp = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, parser.lines[parser.current_line], len + 1);

    dest = strtok(current_command, "=");

    if (dest != NULL)
        comp = strtok(NULL, "=");
    else
        comp = strtok(current_command, ";");

    if (comp != NULL) {
        strncpy(parser.comp, comp, strlen(comp));
    } else {
        printf("Error: comp field is not existing.\n");
        parser.comp[0] = '\0';
    }

    free(current_command);
    return parser.comp;
}

char *jump(void)
{
    char *current_command = parser.lines[parser.current_line];
    size_t len = strlen(current_command);
    char *comp = NULL;
    char *jump = NULL;

    free(parser.jump);
    parser.jump = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, parser.lines[parser.current_line], len + 1);

    comp = strtok(current_command, ";");

    if (comp != NULL) jump = strtok(NULL, ";");

    if (jump != NULL) {
        strncpy(parser.jump, jump, strlen(jump));
    } else {
        printf("Error: jump field is not existing.\n");
        parser.jump[0] = '\0';
    }

    free(current_command);
    return parser.jump;
}
