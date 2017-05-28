/*
 * assembler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>

#include "parser.h"
#include "code.h"

#define SIZE_OF_ARRAY(s)    (sizeof(s) / sizeof(s[0]))


int main(int argc, char *argv[])
{
    enum commandType type;
    uint16_t binary;
    char binstr[] = "0000""0000""0000""0000";
    int i;
    char *path;
    char *filename;
    FILE *fp;

    if (argc != 2) {
        printf("Error: argument is invalid\n");
        return 1;
    }

    // output filename (original length + 1: .asm -> .hack)
    path = (char *)malloc(strlen(argv[1]) + 1 + 1);
    strncpy(path, argv[1], strlen(argv[1] + 1 + 1));

    filename = basename(path);
    filename = strtok(filename, ".");
    if (filename == NULL) {
        printf("Error: file extension is not invalid.\n");
        return 1;
    } 
    strcat(filename, ".hack");

    fp = fopen(filename, "w");

    parser.parserInit(argv[1]);

    while (parser.hasMoreCommands() == true) {
        parser.advance();

        type = parser.commandType();

        switch (type) {
            case A_COMMAND:
            case L_COMMAND:
                binary = (uint16_t)atoi(parser.symbol());
                break;
            case C_COMMAND:
                binary = code.dest(parser.dest());
                binary |= code.comp(parser.comp());
                binary |= code.jump(parser.jump());
                break;
            default:
                break;
        }

        for (i = 0; i < SIZE_OF_ARRAY(binstr) - 1; i++) {
            binstr[SIZE_OF_ARRAY(binstr) - 2 - i]
                = (char)((binary >> i) & 1) + '0';
        }
        fprintf(fp, "%s\n", binstr);
    }

    free(path);
    fclose(fp);

    return 0;
}
