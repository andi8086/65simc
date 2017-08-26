#include "stdio.h"
#include "sim.h"

#define STACK_STORE16(x) \
    memory[0x100 + cpu.S--] = x & 0xFF; \
    memory[0x100 + cpu.S--] = x >> 8;

#define STACK_STORE8(x) \
    memory[0x100 + cpu.S--] = x;

#define STACK_LOAD16(x) \
    x = memory[0x100 + cpu.S++]; \
    x |= memory[0x100 + cpu.S++] << 8;

#define STACK_LOAD8(x) \
    x = memory[0x100 + cpu.S++];


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
    case aX:
        return &cpu.X;
    }
}

void group0(int a, uint8_t idx)
{
    uint16_t ret_addr;

    cpu.PC++;
    switch(idx) {
    case 0:            // BRK;
        // calculate return address = PC+2
        // Store PC(hi)
        // Store PC(lo)
        // Store P
        ret_addr = cpu.PC + 2;
        STACK_STORE16(ret_addr);
        STACK_STORE8(cpu.P);
        // Fetch PC(lo) from $FFFE
        // Fetch PC(hi) from $FFFF
        cpu.PC = *((uint16_t *) &memory[0xFFFE]);
        return;
    case 2:            // PHP
        STACK_STORE8(cpu.P); 
        return;
    case 4:            // BPL
        if (!(cpu.P & F_N)) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    case 6:            // CLC
        cpu.P &= ~ (uint8_t) F_C;
        return;
    }
}

void group1(int a, uint8_t idx)
{
    uint8_t *op;

    cpu.PC++;
    switch(idx) {
    case 0:            // JSR abs
        op = addrDecode(a);
        cpu.PC = *op;
        return;
    case 1:            // BIT zp
    case 3:            // BIT abs
        op = addrDecode(a);
        if (!(*op & cpu.A)) {
            cpu.P |= F_Z;
        }
        // transfer topmost two bits to flags
        cpu.P = cpu.P & 0x3F;
        cpu.P |= *op & 0xC0;
        return;
    case 2:            // PLP
        STACK_LOAD8(cpu.P);
        return;
    case 4:            // BMI rel
        if (cpu.P & F_N) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    case 6:            // SEC
        cpu.P |= F_C;
        return;
    }
}

void group2(int a, uint8_t idx)
{
    cpu.PC++;
    switch(idx) {
    case 0:            // RTI
        STACK_LOAD8(cpu.P);
        STACK_LOAD16(cpu.PC);
        return;
    case 2:            // PHA
        STACK_STORE8(cpu.A);
        return;
    case 3:            // JMP abs
        cpu.PC = *((uint16_t*) &memory[cpu.PC]);
        return;
    case 4:            // BVC
        if (!(cpu.P & F_V)) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    case 6:            // CLI
        cpu.PC &= ~ (uint8_t) F_I;
        return;
    }
}

void group3(int a, uint8_t idx)
{
    uint16_t addr;
    cpu.PC++;
    switch(idx) {
    case 0:            // RTS
        STACK_LOAD16(cpu.PC);
        return;
    case 2:            // PLA
        STACK_LOAD8(cpu.A);
        return;
    case 3:            // JMP ind
        addr = *((uint16_t*) addrDecode(a));
        cpu.PC = addr;
        return;
    case 4:            // BVS
        if (cpu.P & F_V) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    case 6:            // SEI
        cpu.P |= F_I; 
        return;
    }
}

void group4(int a, uint8_t idx)
{
    cpu.PC++;
    switch(idx) {
    case 2:            // DEY
        cpu.Y--;
        return;
    case 4:            // BCC
        if (!(cpu.P & F_C)) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    default:           // STY, TYA
        *addrDecode(a) = cpu.Y;
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.Y & 0x80;
        // set Z flag
        if (!cpu.Y) {
            cpu.P |= F_Z;
        }
        return;
    }
}

void group5(int a, uint8_t idx)
{
    cpu.PC++;
    switch(idx) {
    case 4:            // BCS
        if (cpu.P & F_C) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    case 6:            // CLV
        cpu.P &= F_MASK_V;
        return;
    default:           // LDY, TAY
        cpu.Y = *addrDecode(a);
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.Y & 0x80;
        // set Z flag
        if (!cpu.Y) {
            cpu.P |= F_Z;
        }
        return;
    }
}

void group6(int a, uint8_t idx)
{
    int16_t erg;

    cpu.PC++;
    switch(idx) {
    case 2:            // INY
        cpu.Y++;
        return;
    case 4:            // BNE
        if (!(cpu.P & F_Z)) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    case 6:            // CLD
        cpu.P &= ~ (uint8_t) F_D;
        return;
    default:           // CPY
        erg = (int16_t) cpu.Y - *addrDecode(a);
        cpu.P &= F_MASK_NZC;
        // set N flag
        cpu.P |= erg & 0x80;
        // set Z flag
        if (!erg) {
            cpu.P |= F_Z;
        }
        // set C flag
        if (erg < 0) {
            cpu.P |= F_C;
        }
        return;
    }
}

void group7(int a, uint8_t idx)
{
    cpu.PC++;
    int16_t erg;
    switch(idx) {
    case 2:            // INX
        cpu.X++;
        return;
    case 4:            // BEQ
        if (cpu.P & F_Z) {
            cpu.PC += (int8_t) memory[cpu.PC];
        }
        cpu.PC++;
        return;
    case 6:            // SED
        cpu.P |= F_D;
        return;
    default:           // CPX
        erg = (int16_t) cpu.X - *addrDecode(a);
        cpu.P &= F_MASK_NZC;
        // set N flag
        cpu.P |= erg & 0x80;
        // set Z flag
        if (!erg) {
            cpu.P |= F_Z;
        }
        // set C flag
        if (erg < 0) {
            cpu.P |= F_C;
        }
        return;
    }
}

void groupORA(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    cpu.A |= *op;
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
}

void groupAND(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    cpu.A &= *op;
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
}

void groupEOR(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    cpu.A ^= *op;
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
}

static void doADC(uint8_t b) {
    uint16_t erg = (uint16_t) cpu.A + b;
    cpu.A = (uint8_t) erg & 0xFF;
    cpu.P &= F_MASK_NVZC;
    if (erg > 255) {
       cpu.P |= F_C; 
    }
    uint8_t a7 = cpu.A & 0x80;
    uint8_t b7 = b & 0x80;
    uint8_t c6 = (b & cpu.A & 0x40) << 1;
    /* On 6502 Silicon, overflow flag is
     *
     * NOT (((A7 NOR B7) AND C6) NOR ((A7 NAND B7) NOR C6))
     *
     * I write it as
     * (NOT(A7 OR B7) AND C6) OR NOT((A7 NAND B7) OR C6)
     * which is
     * (NOT(A7 OR B7) AND C6) OR NOT(NOT(A7 AND B7) OR C6)
     * which is
     * (NOT(A7 OR B7) AND C6) OR ((A7 AND B7) AND NOT(C6))
    
     * now it gets clear:
     * wether A7 nor B7 is set (both number positive), and
     * C6 is set, which would produce bit 7 set (negative)
     * this is an overflow

     * or, A7 and B7 ar both set, which means two negative
     * numbers and C6 is not set, which causes bit 7 clear
     * in the result, which is an overflow
     */

    uint8_t v = (!(a7 | b7) & c6) | ((a7 & b7) | !c6);
    // set overflow flag
    cpu.P |= v >> 1;
        
    if (!cpu.A) {
        cpu.P |= F_Z;
    }    
    cpu.P |= cpu.A & 0x80;
}

void groupADC(int a, uint8_t idx)
{
    cpu.PC = cpu.PC + 1;
    uint8_t *mem = addrDecode(a);
    doADC(*mem);
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
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
}

void groupCMP(int a, uint8_t idx)
{
    cpu.PC++;
    int16_t erg;
    erg = (int16_t) cpu.A - *addrDecode(a);
    cpu.P &= F_MASK_NZC;
    // set N flag
    cpu.P |= erg & 0x80;
    // set Z flag
    if (!erg) {
        cpu.P |= F_Z;
    }
    // set C flag
    if (erg < 0) {
        cpu.P |= F_C;
    }
}

void groupSBC(int a, uint8_t idx)
{
    cpu.PC++;
    // This is an ADC with negated operand
    uint8_t *mem = addrDecode(a);
    doADC((uint8_t) -*mem);
}

void groupASL(int a, uint8_t idx)
{
    uint8_t *m;
    cpu.PC++;
    switch(idx) {
    case 6:            // NOP
        return;
    default:
        m = addrDecode(a);
        // get highest bit into carry
        cpu.P &= F_MASK_NZC;
        cpu.P |= (*m & 0x80) >> 7;
        *m <<= 1;
        // set N flag
        cpu.P |= *m & 0x80;
        // set Z flag
        if (!*m) {
            cpu.P |= F_Z;
        }
        return;
    }
}

void groupROL(int a, uint8_t idx)
{
    uint8_t *m;
    uint8_t c;
    cpu.PC++;
    m = addrDecode(a);
    c = (*m & 0x80) >> 7;
    *m <<= 1;
    *m |= c;
    cpu.P &= F_MASK_NZC;
    // set C flag
    cpu.P |= c;
    // set N flag
    cpu.P |= *m & 0x80;
    // set Z flag
    if (!*m) {
        cpu.P |= F_Z;
    }
}

void groupLSR(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *m = addrDecode(a);
    cpu.P &= F_MASK_ZC;
    // set C flag
    cpu.P |= *m & 1;
    *m >>= 1;
    // set Z flag
    if (!*m) {
        cpu.P |= F_Z;
    } 
}

void groupROR(int a, uint8_t idx)
{
    uint8_t *m;
    uint8_t c;
    cpu.PC++;
    m = addrDecode(a);
    c = *m & 1;
    *m >>= 1;
    *m |= c << 7;
    cpu.P &= F_MASK_NZC;
     
    // N flag is C flag
    cpu.P |= c << 7;
    cpu.P |= c;
    if (!*m) {
        cpu.P |= F_Z;
    } 
}

void groupSX(int a, uint8_t idx)
{
    cpu.PC++;
    *addrDecode(a) = cpu.X;
    // TXS (idx == 6) and TXA (id == 2) affect flags, STX (idx == 1,3,5) does not
    if (idx == 6 || idx == 2) {
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.X & 0x80;
        if (cpu.X) {
            // set Z flag
            cpu.P |= F_Z;
        }
    }
}

void groupLX(int a, uint8_t idx)
{
    cpu.PC++;
    cpu.X = *addrDecode(a);
    // TSX (idx == 6) and TAX (id == 2) affect flags, LDX (idx = 0, 1, 3, 5, 7) does not
    if (idx == 6 || idx == 2) {
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.X & 0x80;
        if (cpu.X) {
            // set Z flag
            cpu.P |= F_Z;
        }
    }
}

void groupDEC(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *m = addrDecode(a);
    *m--;
    cpu.P &= F_MASK_NZ;
    // set N flag
    cpu.P |= cpu.X & 0x80;
    if (cpu.X) {
        // set Z flag
        cpu.P |= F_Z;
    }
}

void groupINC(int a, uint8_t idx)
{
    cpu.PC++;
    if (idx == 2) {
                       // NOP
        return;
    }
    uint8_t *m = addrDecode(a);
    *m++;
    cpu.P &= F_MASK_NZ;
    // set N flag
    cpu.P |= cpu.X & 0x80;
    if (cpu.X) {
        // set Z flag
        cpu.P |= F_Z;
    }
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

