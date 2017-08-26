#ifndef __6502_H__
#define __6502_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include "funcs.h"

#define F_N 0x80
#define F_V 0x40
#define F_B 0x10
#define F_D 0x08
#define F_I 0x04
#define F_Z 0x02
#define F_C 0x01

#define F_MASK_NZ  0x7D
#define F_MASK_NZC 0x7C
#define F_MASK_NVZC 0x3C
#define F_MASK_ZC  0xFC
#define F_MASK_V   0xBF

struct CPU {
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t S;
    uint16_t PC;
    uint8_t P;
};


typedef void (*INSTR_FUNC)(int, uint8_t);

enum amode {
    aNULL,
    aimm,
    azp,
    aabs,
    arel,
    azpx,
    azpy,
    aabx,
    aaby,
    aind,
    aizx,
    aizy,
    aS,
    aA,
    aX
};

struct op_ccaaa {
    uint8_t c;
    uint8_t aaa;
    enum amode addr_modes[8];
    uint8_t cycles[8];
    INSTR_FUNC func;
};

extern struct CPU cpu;
extern struct op_ccaaa opcodes[32];
extern uint8_t memory[65536];

#endif
