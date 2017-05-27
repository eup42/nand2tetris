/*
 * assembler.c
 */

#include <stdio.h>

#include "parser.h"

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Error: argument is invalid\n");
        return 1;
    }

    parserInit(argv[1]);

    return 0;
}
