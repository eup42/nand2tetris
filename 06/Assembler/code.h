/*
 * code.h
 */

#ifndef _CODE_H_
#define _CODE_H_

#include <stdint.h>

extern uint16_t _code_dest(char *mnemonic);
extern uint16_t _code_comp(char *mnemonic);
extern uint16_t _code_jump(char *mnemonic);

const static struct code {
    uint16_t (*dest)(char *);
    uint16_t (*comp)(char *);
    uint16_t (*jump)(char *);
} code = {
    .dest = _code_dest,
    .comp = _code_comp,
    .jump = _code_jump,
};

#endif
