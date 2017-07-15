/*
 * assembler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libgen.h>
#include <ctype.h>

#include "parser.h"
#include "code.h"
#include "symbol_table.h"

#define SIZE_OF_ARRAY(s)    (sizeof(s) / sizeof(s[0]))

const static struct {
    char *symbol;
    uint16_t address;
} defined_symbol[23] = {
    {"SP",     0x0000},
    {"LCL",    0x0001},
    {"ARG",    0x0002},
    {"THIS",   0x0003},
    {"THAT",   0x0004},
    {"R0",     0x0000},
    {"R1",     0x0001},
    {"R2",     0x0002},
    {"R3",     0x0003},
    {"R4",     0x0004},
    {"R5",     0x0005},
    {"R6",     0x0006},
    {"R7",     0x0007},
    {"R8",     0x0008},
    {"R9",     0x0009},
    {"R10",    0x000a},
    {"R11",    0x000b},
    {"R12",    0x000c},
    {"R13",    0x000d},
    {"R14",    0x000e},
    {"R15",    0x000f},
    {"SCREEN", 0x4000},
    {"KBD",    0x6000},
};



int main(int argc, char *argv[])
{
    Parser parser = newParser();
    enum commandType type;
    uint16_t binary;
    char binstr[] = "0000""0000""0000""0000";
    int i;
    char *path;
    char *filename;
    FILE *fp;
    uint16_t address = 0;

    if (argc != 2) {
        printf("Error: argument is invalid\n");
        return 1;
    }

    /*
     * Initialize Symbol Table
     */

    hash.initSymbolTable();
    for (i = 0; i < SIZE_OF_ARRAY(defined_symbol); i++)
        hash.addEntry(defined_symbol[i].symbol, defined_symbol[i].address);

    /*
     * First Path
     */
    parser.parserInit(&parser, argv[1]);

    while (parser.hasMoreCommands(&parser) == true) {
        parser.advance(&parser);
        type = parser.commandType(&parser);

        switch (type) {
            case A_COMMAND:
            case C_COMMAND:
                address++;
                break;

            case L_COMMAND:
                if (!hash.contains(parser.symbol(&parser)))
                    hash.addEntry(parser.symbol(&parser), address);
                break;
        }
    }

    /*
     * Create output file
     */

    // Create output file
    // output filename length = original length + 1: .asm -> .hack
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

    /*
     * Second Path
     */

    parser.reset(&parser);
    address = 16;

    while (parser.hasMoreCommands(&parser) == true) {
        parser.advance(&parser);

        type = parser.commandType(&parser);

        switch (type) {
            case A_COMMAND:
                if (isdigit(parser.symbol(&parser)[0])) {
                    binary = (uint16_t)atoi(parser.symbol(&parser));
                } else if (hash.contains(parser.symbol(&parser))) {
                    binary = hash.getAddress(parser.symbol(&parser));
                } else {
                    hash.addEntry(parser.symbol(&parser), address);
                    address++;
                    binary = hash.getAddress(parser.symbol(&parser));
                }
                break;
            case C_COMMAND:
                binary = code.dest(parser.dest(&parser));
                binary |= code.comp(parser.comp(&parser));
                binary |= code.jump(parser.jump(&parser));
                break;
            default:
                continue;
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
