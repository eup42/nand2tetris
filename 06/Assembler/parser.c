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
    int current_line;
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
                    = (char *)malloc(sizeof(unsigned char) * strlen(buff));
                strcpy(parser.lines[line_num], buff);

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
