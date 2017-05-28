/*
 * code.h
 */

#include "code.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define SIZE_OF_ARRAY(s)    (sizeof(s) / sizeof(s[0]))

struct convert_table {
    char *mnemonic;
    uint16_t binary;
};

const static struct convert_table tbl_dest[8] = {
    {"null", 0b1110000000000000},
    {"M",    0b1110000000001000},
    {"D",    0b1110000000010000},
    {"MD",   0b1110000000011000},
    {"A",    0b1110000000100000},
    {"AM",   0b1110000000101000},
    {"AD",   0b1110000000110000},
    {"AMD",  0b1110000000111000},
};

const static struct convert_table tbl_comp[28] = {
    {"0",   0b1110101010000000},
    {"1",   0b1110111111000000},
    {"-1",  0b1110111010000000},
    {"D",   0b1110001100000000},
    {"A",   0b1110110000000000},
    {"!D",  0b1110001101000000},
    {"!A",  0b1110110001000000},
    {"-D",  0b1110001111000000},
    {"-A",  0b1110110011000000},
    {"D+1", 0b1110011111000000},
    {"A+1", 0b1110110111100000},
    {"D-1", 0b1110001110000000},
    {"A-1", 0b1110110010000000},
    {"D+A", 0b1110000010000000},
    {"D-A", 0b1110010011000000},
    {"A-D", 0b1110000111000000},
    {"D&A", 0b1110000000000000},
    {"D|A", 0b1110010101000000},
};

const static struct convert_table tbl_jump[8] = {
    {"null", 0b1110000000000000},
    {"JGT",  0b1110000000000001},
    {"JEQ",  0b1110000000000010},
    {"JGE",  0b1110000000000011},
    {"JLT",  0b1110000000000100},
    {"JNE",  0b1110000000000101},
    {"JLE",  0b1110000000000110},
    {"JMP",  0b1110000000000111},
};

uint16_t _code_dest(char *mnemonic)
{
    int i;

    for (i = 0; i < SIZE_OF_ARRAY(tbl_dest); i++)
        if (!strcmp(mnemonic, tbl_dest[i].mnemonic)) break;

    if (i == SIZE_OF_ARRAY(tbl_dest)) {
        printf("Error: invalid mnemonic. [mnemonic]%s\n", mnemonic);
        return 0;
    }

    return tbl_dest[i].binary;
}

uint16_t _code_comp(char *mnemonic)
{
    int i;

    for (i = 0; i < SIZE_OF_ARRAY(tbl_comp); i++)
        if (!strcmp(mnemonic, tbl_comp[i].mnemonic)) break;

    if (i == SIZE_OF_ARRAY(tbl_comp)) {
        printf("Error: invalid mnemonic. [mnemonic]%s\n", mnemonic);
        return 0;
    }

    return tbl_comp[i].binary;
}

uint16_t _code_jump(char *mnemonic)
{
    int i;

    for (i = 0; i < SIZE_OF_ARRAY(tbl_jump); i++)
        if (!strcmp(mnemonic, tbl_jump[i].mnemonic)) break;

    if (i == SIZE_OF_ARRAY(tbl_jump)) {
        printf("Error: invalid mnemonic. [mnemonic]%s\n", mnemonic);
        return 0;
    }

    return tbl_jump[i].binary;
}
