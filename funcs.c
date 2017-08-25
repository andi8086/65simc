#include "sim.h"

uint8_t *addrDecode(enum amode a)
{
    uint8_t idx_lo, idx_hi;
    uint16_t addr;

    switch(a) {
    case aNULL:
        return NULL;
    case aimm:  /* #$00 */
        cpu.PC += 1;
        // Address points to immediate byte after opcode
        // which can be source only
        return &memory[cpu.PC - 1];
    case azp:  /* $00 */
        cpu.PC += 1;
        // Byte after opcode is zero page index
        idx_lo = memory[cpu.PC - 1];
        // Return address of this memory cell
        return &memory[idx_lo];
    case aabs: /* $0000 */
        cpu.PC += 2;
        // Word after opcode is absolute address
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // Return address of this memory cell
        return &memory[(uint16_t) idx_hi << 8 | idx_lo];
    case arel: /* PC + $0000 */
        return 0;
    case azpx: /* $00, X */
        cpu.PC += 1;
        // Byte after opcode is zero page index
        idx_lo = memory[cpu.PC - 1];
        // X-Register is added to this index
        // This wraps around to stay in zero page
        return &memory[(cpu.X + idx_lo) & 0xFF];
    case azpy: /* $00, Y */
        cpu.PC += 1;
        // Byte after opcode is zero page index
        idx_lo = memory[cpu.PC - 1];
        // Y-Register is added to this index
        // This wraps around to stay in zero page 
        return &memory[(cpu.Y + idx_lo) & 0xFF];
    case aabx: /* $0000, X */
        cpu.PC += 2;
        // Word after opcode is absolute address
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // X-Register is added to this absolute address
        return &memory[(uint16_t) idx_hi << 8 | idx_lo + cpu.X];
    case aaby: /* $0000, Y */
        cpu.PC += 2;
        // Word after opcode is absolute address
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // Y-Register is added to this absolute address
        return &memory[(uint16_t) idx_hi << 8 | idx_lo + cpu.Y];
    case aind: /* ($0000) */
        cpu.PC += 2;
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // Word after opcode is a pointer to the address
        addr = memory[(uint16_t) idx_hi << 8 | idx_lo];
        // Return address to where this address points
        return &memory[addr];
    case aizx: /* ($00, X) */
        cpu.PC += 1;
        // Byte after offset is an index into zero page
        addr = memory[cpu.PC - 1];
        // Fetch a word from there and add the X register
        // This word forms itself an address word
        idx_lo = memory[(addr + cpu.X) & 0xFF];
        idx_hi = memory[(addr + cpu.X + 1) & 0xFF];
        // Return address to where this address word points
        return &memory[(uint16_t) idx_hi << 8 | idx_lo];
    case aizy: /* ($00), Y */
        cpu.PC += 1;
        // Byte after offset is an index into zero page
        addr = memory[cpu.PC - 1];
        // Fetch word from this address to get new address
        idx_lo = memory[addr];
        idx_hi = memory[(addr + 1) & 0xFF];
        // To this indirect address add the Y-Register to get
        // the final address
        return &memory[(uint16_t) idx_hi << 8 | idx_lo + cpu.Y];
    case aA:
        return &cpu.A;
    case aS:
        return &cpu.S;
    }
}

void group0(int a, uint8_t idx)
{
    switch(idx) {
    case 0:
        // BRK;
        return;
    case 2:
        // PHP;
        return;
    case 4:
        // BPL;
        return;
    case 6:
        // CLC;
        return;
    }
}

void group1(int a, uint8_t idx)
{
    switch(idx) {
    case 0:
        // JSR abs
        return;
    case 1:
        // BIT zp
        return;
    case 2:
        // PLP
        return;
    case 3:
        // BIT abs
        return;
    case 4:
        // BMI rel
        return;
    case 6:
        // SEC
        return;
    }
}

void group2(int a, uint8_t idx)
{
    switch(idx) {
    case 0:
        // RTI
        return;
    case 2:
        // PHA
        return;
    case 3:
        // JMP abs
        return;
    case 4:
        // BVC
        return;
    case 6:
        // CLI
        return;
    }
}

void group3(int a, uint8_t idx)
{
    switch(idx) {
    case 0:
        // RTS
        return;
    case 2:
        // PLA
        return;
    case 3:
        // JMP ind
        return;
    case 4:
        // BVS
        return;
    case 6:
        // SEI
        return;
    }
}

void group4(int a, uint8_t idx)
{
    switch(idx) {
    case 2:
        // DEY
        return;
    case 4:
        // BCC
        return;
    default:
        // STY 
        return;
    }
}

void group5(int a, uint8_t idx)
{
    switch(idx) {
    case 4:
        // BCS
        return;
    case 6:
        // CLV
        return;
    default:
        // LDY 
        return;
    }
}

void group6(int a, uint8_t idx)
{
    switch(idx) {
    case 2:
        // INY
        return;
    case 4:
        // BNE
        return;
    case 6:
        // CLD
        return;
    default:
        // CPY
        return;
    }
}

void group7(int a, uint8_t idx)
{
    switch(idx) {
    case 2:
        // INX
        return;
    case 4:
        // BEQ
        return;
    case 6:
        // SED
        return;
    default:
        // CPX
        return;
    }
}

void groupORA(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    cpu.A |= *op;
}

void groupAND(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    cpu.A &= *op;
}

void groupEOR(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    cpu.A ^= *op;
}

void groupADC(int a, uint8_t idx)
{

}

void groupSTA(int a, uint8_t idx)
{
    cpu.PC = cpu.PC + 1;
    uint8_t *op = addrDecode(a);
    *op = cpu.A;
}

void groupLDA(int a, uint8_t idx)
{
    cpu.PC = cpu.PC + 1;
    uint8_t *op = addrDecode(a);
    cpu.A = *op;
}

void groupCMP(int a, uint8_t idx)
{

}

void groupSBC(int a, uint8_t idx)
{

}

void groupASL(int a, uint8_t idx)
{
    switch(idx) {
    case 6:
        // NOP
        return;
    default:
        // ASL
        return;
    }
}

void groupROL(int a, uint8_t idx)
{
}

void groupLSR(int a, uint8_t idx)
{

}

void groupROR(int a, uint8_t idx)
{

}

void groupSX(int a, uint8_t idx)
{

}

void groupLX(int a, uint8_t idx)
{

}

void groupDEC(int a, uint8_t idx)
{

}

void groupINC(int a, uint8_t idx)
{

}

void groupSLO(int a, uint8_t idx)
{

}

void groupRLA(int a, uint8_t idx)
{

}

void groupSRE(int a, uint8_t idx)
{

}

void groupRRA(int a, uint8_t idx)
{

}

void groupSAX(int a, uint8_t idx)
{

}

void groupLAX(int a, uint8_t idx)
{

}

void groupDCP(int a, uint8_t idx)
{

}

void groupISC(int a, uint8_t idx)
{

}

