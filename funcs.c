#include "sim.h"

uint8_t *addrDecode(enum amode a)
{
    switch(a) {
    case aNULL:
        return NULL;
    case aimm:  /* #$00 */
        cpu.PC += 2;
        return &memory[cpu.PC - 1];
    case azp:  /* $00 */
        cpu.PC += 2;
        uint8_t zp_idx = memory[cpu.PC - 1];
        return &memory[zp_idx];
    case aabs: /* $0000 */
        cpu.PC += 3;
        uint8_t idx_lo = memory[cpu.PC - 2];
        uint8_t idx_hi = memory[cpu.PC - 1];
        return &memory[(uint16_t) idx_hi << 8 | idx_lo];
    case arel: /* PC + $0000 */
        
        return 0;
    case azpx: /* $00, X */
        
        return 0;
    case azpy: /* $00, Y */
        return 0;
    case aabx: /* $0000, X */
        return 0;
    case aaby: /* $0000, Y */
        return 0;
    case aind: /* ($0000) */
        return 0;
    case aizx: /* ($00, X) */
        return 0;
    case aizy: /* ($00), Y */
        return 0;    
    }
}

void group0(int a, int c)
{

}

void group1(int a, int c)
{

}

void group2(int a, int c)
{

}

void group3(int a, int c)
{

}

void group4(int a, int c)
{

}

void group5(int a, int c)
{

}

void group6(int a, int c)
{

}

void group7(int a, int c)
{

}

void groupORA(int a, int c)
{

}

void groupAND(int a, int c)
{

}

void groupEOR(int a, int c)
{

}

void groupADC(int a, int c)
{

}

void groupSTA(int a, int c)
{

}

void groupLDA(int a, int c)
{
    uint8_t op = *addrDecode(a);
}

void groupCMP(int a, int c)
{

}

void groupSBC(int a, int c)
{

}

void groupASL(int a, int c)
{

}

void groupROL(int a, int c)
{

}

void groupLSR(int a, int c)
{

}

void groupROR(int a, int c)
{

}

void groupSX(int a, int c)
{

}

void groupLX(int a, int c)
{

}

void groupDEC(int a, int c)
{

}

void groupINC(int a, int c)
{

}

void groupSLO(int a, int c)
{

}

void groupRLA(int a, int c)
{

}

void groupSRE(int a, int c)
{

}

void groupRRA(int a, int c)
{

}

void groupSAX(int a, int c)
{

}

void groupLAX(int a, int c)
{

}

void groupDCP(int a, int c)
{

}

void groupISC(int a, int c)
{

}

