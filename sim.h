#ifndef __SIM_H__
#define __SIM_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

struct CPU {
    uint8_t A;
    uint8_t X;
    uint8_t Y;
    uint8_t S;
    uint16_t PC;
    uint8_t F;
};

extern struct CPU cpu;
extern uint8_t memory[65536];

typedef void (*INSTR_FUNC)(int);

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
    aizy
};

struct op_ccaaa {
    uint8_t c;
    uint8_t aaa;  
    enum amode addr_modes[8];
    uint8_t cycles[8];
    INSTR_FUNC func;    
};

#endif

