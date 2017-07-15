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


void _parser_init(Parser *pThis, char *name)
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
    pThis->lines = (char **)malloc(sizeof(char *)
                                    * LINE_BLOCK_SIZE * line_block_num);

    while ((ch = fgetc(fp)) != EOF) {
        if (ch == '\n') {
            buff[indx] = '\0';

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

    if (current_command[0] == '@')
        return A_COMMAND;
    else if (current_command[0] == '(')
        return L_COMMAND;
    else 
        return C_COMMAND;
}

char *_parser_symbol(Parser *pThis)
{
    char *current_command = pThis->lines[pThis->current_line];
    size_t len = strlen(current_command);

    free(pThis->current_symbol);
    pThis->current_symbol = (char *)malloc(sizeof(unsigned char) * len + 1);

    if (pThis->symbol == NULL) return NULL;

    if (current_command[0] == '@') {
        strncpy(pThis->current_symbol, &current_command[1], len - 1);
        pThis->current_symbol[len - 1] = '\0';
        return pThis->current_symbol;

    } else if (current_command[0] == '(') {
        if (current_command[len - 1] != ')') {
            printf("Error: Command is not terminated as ')'. Command = %s\n",
                    current_command);
            return NULL;
        }
        strncpy(pThis->current_symbol, &current_command[1], len - 2);
        pThis->current_symbol[len - 2] = '\0';
        return pThis->current_symbol;

    } else {
        printf("Error: Command is invalid. Command = %s\n",  current_command);
        return NULL;
    }
}

char *_parser_dest(Parser *pThis)
{
    char *current_command;
    size_t len = strlen(pThis->lines[pThis->current_line]);
    char *dest;

    free(pThis->current_dest);
    pThis->current_dest = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, pThis->lines[pThis->current_line], len + 1);

    if (strstr(current_command, "="))
        dest = strtok(current_command, "=");
    else
        dest = NULL;

    if (dest != NULL) {
        strncpy(pThis->current_dest, dest, strlen(dest) + 1);
    } else {
        strncpy(pThis->current_dest, "null", strlen("null") + 1);
    }

    free(current_command);
    return pThis->current_dest;
}


char *_parser_comp(Parser *pThis)
{
    char *current_command = pThis->lines[pThis->current_line];
    size_t len = strlen(current_command);
    char *comp;

    free(pThis->current_comp);
    pThis->current_comp = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, pThis->lines[pThis->current_line], len + 1);


    if (strstr(current_command, "=")) {
        strtok(current_command, "=;");
        comp = strtok(NULL, "=;");
    } else if (strstr(current_command, ";")) {
        comp = strtok(current_command, ";");
    } else {
        comp = NULL;
    }

    if (comp) {
        strncpy(pThis->current_comp, comp, strlen(comp) + 1);
    } else {
        printf("Error: comp field is not existing.\n");
        pThis->current_comp[0] = '\0';
    }

    free(current_command);
    return pThis->current_comp;
}

char *_parser_jump(Parser *pThis)
{
    char *current_command = pThis->lines[pThis->current_line];
    size_t len = strlen(current_command);
    char *comp = NULL;
    char *jump = NULL;

    free(pThis->current_jump);
    pThis->current_jump = (char *)malloc(sizeof(unsigned char) * len + 1);

    //For using strtok
    current_command = (char *)malloc(len + 1);
    strncpy(current_command, pThis->lines[pThis->current_line], len + 1);

    comp = strtok(current_command, ";");

    if (comp != NULL) jump = strtok(NULL, ";");

    if (jump != NULL) {
        strncpy(pThis->current_jump, jump, strlen(jump) + 1);
    } else {
        strncpy(pThis->current_jump, "null", strlen("null") + 1);
    }

    free(current_command);
    return pThis->current_jump;
}

void _parser_delete(Parser *pThis)
{
    int i;

    for (i = 0; i < pThis->max_line; i++) {
        free(pThis->lines[i]);
    }

    free(pThis->lines);
    free(pThis->current_symbol);
    free(pThis->current_comp);
    free(pThis->current_dest);
    free(pThis->current_jump);
}

