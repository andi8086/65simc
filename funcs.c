/***************************************************************
 * MOS 6502 System Emulator
 * v 0.1
 *
 * (c) 2017 Andreas J. Reichel
 *
 * h o m e b a s e _ a r (a|t]> w e b . d e
 *
 * License: MIT (see LICENSE.txt) 
 ***************************************************************/

#include <stdio.h>
#include "6502.h"
#include "mem.h"

#define STACK_STORE16(x) \
    memory[0x100 + cpu.S--] = x >> 8; \
    memory[0x100 + cpu.S--] = x & 0xFF;

#define STACK_STORE8(x) \
    memory[0x100 + cpu.S--] = x;

#define STACK_LOAD16(x) \
    x = memory[0x100 + ++cpu.S]; \
    x |= memory[0x100 + ++cpu.S] << 8;

#define STACK_LOAD8(x) \
    x = memory[0x100 + ++cpu.S];

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
        mem_addr = idx_lo;
        return &memory[idx_lo];
    case aabs: /* $0000 */
        cpu.PC += 2;
        // Word after opcode is absolute address
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // Return address of this memory cell
        mem_addr = (uint16_t) idx_hi << 8 | idx_lo;
        return &memory[mem_addr];
    case arel: /* PC + $0000 */
        return 0;
    case azpx: /* $00, X */
        cpu.PC += 1;
        // Byte after opcode is zero page index
        idx_lo = memory[cpu.PC - 1];
        // X-Register is added to this index
        // This wraps around to stay in zero page
        mem_addr = (uint16_t) (cpu.X + idx_lo) & 0xFF;
        return &memory[mem_addr];
    case azpy: /* $00, Y */
        cpu.PC += 1;
        // Byte after opcode is zero page index
        idx_lo = memory[cpu.PC - 1];
        // Y-Register is added to this index
        // This wraps around to stay in zero page 
        mem_addr = (cpu.Y + idx_lo) & 0xFF;
        return &memory[mem_addr];
    case aabx: /* $0000, X */
        cpu.PC += 2;
        // Word after opcode is absolute address
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // X-Register is added to this absolute address
        mem_addr = (uint16_t) idx_hi << 8 | idx_lo + cpu.X;
        return &memory[mem_addr];
    case aaby: /* $0000, Y */
        cpu.PC += 2;
        // Word after opcode is absolute address
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // Y-Register is added to this absolute address
        mem_addr = (uint16_t) idx_hi << 8 | idx_lo + cpu.Y;
        return &memory[mem_addr];
    case aind: /* ($0000) */
        cpu.PC += 2;
        idx_lo = memory[cpu.PC - 2];
        idx_hi = memory[cpu.PC - 1];
        // Word after opcode is a pointer to the address
        addr = memory[(uint16_t) idx_hi << 8 | idx_lo];
        // Return address to where this address points
        mem_addr = addr;
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
        mem_addr = (uint16_t) idx_hi << 8 | idx_lo;
        return &memory[mem_addr];
    case aizy: /* ($00), Y */
        cpu.PC += 1;
        // Byte after offset is an index into zero page
        addr = memory[cpu.PC - 1];
        // Fetch word from this address to get new address
        idx_lo = memory[addr];
        idx_hi = memory[(addr + 1) & 0xFF];
        // To this indirect address add the Y-Register to get
        // the final address
        mem_addr = (uint16_t) idx_hi << 8 | idx_lo + cpu.Y;
        return &memory[mem_addr];
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
        printf("PHP = %02X", cpu.P);
        return;
    case 4:            // BPL
        if (!(cpu.P & F_N)) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BPL [true]");
        } else {
            printf("BPL [false]");
        }
        cpu.PC++;
        return;
    case 6:            // CLC
        cpu.P &= ~ (uint8_t) F_C;
        printf("CLC");
        return;
    }
}

void group1(int a, uint8_t idx)
{
    uint8_t *op;
    uint16_t ret_addr;

    cpu.PC++;
    switch(idx) {
    case 0:            // JSR abs
        ret_addr = cpu.PC + 2;
        STACK_STORE16(ret_addr);
        cpu.PC = *((uint16_t*) &memory[cpu.PC]);
        printf("JSR");
        return;
    case 1:            // BIT zp
    case 3:            // BIT abs
        op = addrDecode(a);
        printf("BIT %02X, %02X", cpu.A, op);
        if (!(*op & cpu.A)) {
            cpu.P |= F_Z;
        }
        // transfer topmost two bits to flags
        cpu.P = cpu.P & 0x3F;
        cpu.P |= *op & 0xC0;
        mem_rwb = 1;
        mem_sync = true;
        return;
    case 2:            // PLP
        STACK_LOAD8(cpu.P);
        return;
    case 4:            // BMI rel
        if (cpu.P & F_N) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BMI [true]");
        } else {
            printf("BMI [false]");
        }
        cpu.PC++;
        return;
    case 6:            // SEC
        cpu.P |= F_C;
        printf("SEC");
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
        printf("RTI");
        return;
    case 2:            // PHA
        STACK_STORE8(cpu.A);
        printf("PHA = %02X", cpu.A);
        return;
    case 3:            // JMP abs
        cpu.PC = *((uint16_t*) &memory[cpu.PC]);
        printf("JMP");
        return;
    case 4:            // BVC
        if (!(cpu.P & F_V)) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BVC [true]");
        } else {
            printf("BVC [false]");
        }
        cpu.PC++;
        return;
    case 6:            // CLI
        cpu.P &= ~ (uint8_t) F_I;
        printf("CLI");
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
        printf("RTS");
        return;
    case 2:            // PLA
        STACK_LOAD8(cpu.A);
        printf("PLA");
        return;
    case 3:            // JMP ind
        addr = *((uint16_t*) addrDecode(a));
        cpu.PC = addr;
        printf("JMP");
        return;
    case 4:            // BVS
        if (cpu.P & F_V) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BVS [true]");
        } else {
            printf("BVS [false]");
        }
        cpu.PC++;
        return;
    case 6:            // SEI
        cpu.P |= F_I; 
        printf("SEI");
        return;
    }
}

void group4(int a, uint8_t idx)
{
    uint8_t *addr;
    cpu.PC++;
    switch(idx) {
    case 2:            // DEY
        cpu.Y--;
        cpu.P &= F_MASK_NZ;
        cpu.P |= cpu.Y & 0x80;
        if (!cpu.Y) {
            cpu.P &= F_Z;
        }
        printf("DEY");
        return;
    case 4:            // BCC
        if (!(cpu.P & F_C)) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BCC [true]");
        } else {
            printf("BCC [false]");
        }
        cpu.PC++;
        return;
    default:           // STY, TYA
        addr = addrDecode(a);
        *addr = cpu.Y;
        if (addr != &cpu.A) {
            mem_rwb = 0;
            mem_sync = true;
        }
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.Y & 0x80;
        // set Z flag
        if (!cpu.Y) {
            cpu.P |= F_Z;
        }
        printf("STY / TYA");
        return;
    }
}

void group5(int a, uint8_t idx)
{
    uint8_t *addr;
    cpu.PC++;
    switch(idx) {
    case 4:            // BCS
        if (cpu.P & F_C) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BCS [true]");
        } else {
            printf("BCS [false]");
        }
        cpu.PC++;
        return;
    case 6:            // CLV
        cpu.P &= F_MASK_V;
        printf("CLV");
        return;
    default:           // LDY, TAY
        addr = addrDecode(a);
        if (addr != &cpu.A) {
            mem_rwb = 1;
            mem_sync = true;
        }
        cpu.Y = *addr;
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.Y & 0x80;
        // set Z flag
        if (!cpu.Y) {
            cpu.P |= F_Z;
        }
        
        printf("LDY / TAY");
        return;
    }
}

void group6(int a, uint8_t idx)
{
    int16_t erg;
    uint8_t op;
    cpu.PC++;
    switch(idx) {
    case 2:            // INY
        cpu.Y++;
        cpu.P &= F_MASK_NZ;
        cpu.P |= cpu.Y & 0x80;
        if (!cpu.Y) {
            cpu.P &= F_Z;
        }
        printf("INY");
        return;
    case 4:            // BNE
        if (!(cpu.P & F_Z)) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BNE [true]");
        } else {
            printf("BNE [false]");
        }
        cpu.PC++;
        return;
    case 6:            // CLD
        cpu.P &= ~ (uint8_t) F_D;
        printf("CLD");
        return;
    default:           // CPY
        op = *addrDecode(a);
        erg = (int16_t) cpu.Y - op;
        cpu.P &= F_MASK_NZC;
        // set N flag
        if (erg < 0)
            cpu.P |= F_N;
        // set Z flag
        if (!erg) {
            cpu.P |= F_Z;
        }
        // set C flag
        if (erg >= 0) {
            cpu.P |= F_C;
        }
        mem_rwb = 1;
        mem_sync = true;
        printf("CPY =%02X", op);
        return;
    }
}

void group7(int a, uint8_t idx)
{
    cpu.PC++;
    int16_t erg;
    uint8_t op;
    switch(idx) {
    case 2:            // INX
        cpu.X++;
        cpu.P &= F_MASK_NZ;
        cpu.P |= cpu.X & 0x80;
        if (!cpu.X) {
            cpu.P &= F_Z;
        }
        printf("INX");
        return;
    case 4:            // BEQ
        if (cpu.P & F_Z) {
            cpu.PC += (int8_t) memory[cpu.PC];
            printf("BEQ [true]");
        } else {
            printf("BEQ [false]");
        }
        cpu.PC++;
        return;
    case 6:            // SED
        cpu.P |= F_D;
        printf("SED");
        return;
    default:           // CPX
        op = *addrDecode(a);
        erg = (int16_t) cpu.X - op;
        cpu.P &= F_MASK_NZC;
        // set N flag
        if (erg < 0)
            cpu.P |= F_N;
        // set Z flag
        if (!erg) {
            cpu.P |= F_Z;
        }
        // set C flag
        if (erg >= 0) {
            cpu.P |= F_C;
        }
        mem_rwb = 1;
        mem_sync = true;
        printf("CPX =%02X", op);
        return;
    }
}

void groupORA(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    printf("ORA %02X, %02X", cpu.A, *op);
    cpu.A |= *op;
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
    mem_rwb = 1;
    mem_sync = true;
}

void groupAND(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    printf("AND %02X, %02X", cpu.A, *op);
    cpu.A &= *op;
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
    mem_rwb = 1;
    mem_sync = true;
}

void groupEOR(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *op = addrDecode(a);
    printf("EOR %02X, %02X", cpu.A, *op);
    cpu.A ^= *op;
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
    mem_rwb = 1;
    mem_sync = true;
}

void groupADC(int a, uint8_t idx)
{
    // This shall simulate all flags for bin and BCD correctly,
    // even for invalid arguments, for MOS 6502 only !
    
    cpu.PC = cpu.PC + 1;
    uint8_t *mem = addrDecode(a);
    uint8_t A = cpu.A;

    printf("ADC %02X, %02X", A, *mem);

    bool decimal_mode = cpu.P & F_D;
    uint8_t c = cpu.P & F_C;
    
    uint16_t erg = (A & 0x0F) + ((*mem) & 0x0F) + (cpu.P & F_C);
    if ((erg >= 0x0A) && decimal_mode) {
        erg = ((erg + 0x06) & 0x0F) + 0x10;
    }
    erg += (A & 0xF0) + ((*mem) & 0xF0);

    cpu.P &= F_MASK_NVZC;

    // Calculate N and V here for 6502
    // set negative flag
    if (erg & 0x80)
        cpu.P |= F_N;

    // set overflow flag
    //printf("\n%d %d %d\n", (int8_t) A, (int8_t) *mem, (uint8_t) erg);

    if (((int8_t) A >= 0) && ((int8_t) *mem >= 0) && (erg >= 0x80)) {
        cpu.P |= F_V;

    } else
    if (((int8_t) A < 0) && ((int8_t) *mem < 0) && ((erg & 0xFF) < 0x80)) {
        cpu.P |= F_V;
    }

    if ((erg >= 0xA0) && decimal_mode) {
        erg += 0x60;
    }
    // set carry flag
    if (erg >= 0x100) {
        cpu.P |= F_C;
    }
    if (!(A + (*mem) + c)) {
        cpu.P |= F_Z;
    }
    cpu.A = erg & 0xFF;
    mem_rwb = 1;
    mem_sync = true;
}

void groupSTA(int a, uint8_t idx)
{
    cpu.PC = cpu.PC + 1;
    uint8_t *op = addrDecode(a);
    *op = cpu.A;
    mem_rwb = 0;
    mem_sync = true;
    printf("STA =%02X", cpu.A);
}

void groupLDA(int a, uint8_t idx)
{
    cpu.PC = cpu.PC + 1;
    uint8_t *op = addrDecode(a);
    cpu.A = *op;
    printf("LDA =%02X", cpu.A);
    cpu.P &= F_MASK_NZ;
    cpu.P |= cpu.A & 0x80;
    if (!cpu.A) {
        cpu.P |= F_Z;
    }
    mem_rwb = 1;
    mem_sync = true;
}

void groupCMP(int a, uint8_t idx)
{
    cpu.PC++;
    int16_t erg;
    uint8_t m = *addrDecode(a);
    erg = (int16_t) cpu.A - m;
    cpu.P &= F_MASK_NZC;
    // set N flag
    if (erg < 0) 
       cpu.P |= F_N;
    
    // set Z flag
    if (!erg) {
        cpu.P |= F_Z;
    }
    // set C flag
    if (erg >= 0) {
        cpu.P |= F_C;
    }
    mem_rwb = 1;
    mem_sync = true;
    printf("CMP %02X, %02X", cpu.A, m);
}

void groupSBC(int a, uint8_t idx)
{
    // This shall simulate all flags for bin and BCD correctly,
    // even for invalid arguments, for MOS 6502 only !

    cpu.PC++;
    uint8_t *mem = addrDecode(a);
    uint8_t A = cpu.A;
    printf("SBC %02X, %02X", A, *mem);
    bool decimal_mode = cpu.P & F_D;
    int16_t erg = (A & 0x0F) - (*mem & 0x0F) + (cpu.P & F_C) - 1;
    if ((erg < 0) && decimal_mode) {
        erg = ((erg - 0x06) & 0x0F) - 0x10;
    }
    erg += (A & 0xF0) - (*mem & 0xF0);
    if ((erg < 0) && decimal_mode) {
        erg -= 0x60;
    }
    uint8_t c = cpu.P & F_C;
    int16_t x = (int8_t) cpu.A;
    int16_t y = (int8_t) *mem;
    int16_t e = x - y + (cpu.P & F_C) - 1;

    cpu.P &= F_MASK_NVZC;
    // Calculate N and V here for 6502

    // set overflow flag
    if (e < -128 || e > 127) {
        cpu.P |= F_V;
    }
    // set negative flag
    if (e < 0)
        cpu.P |= F_N;


    // calculate carry
    uint16_t cz = (uint16_t) A - *mem + c - 1;
    if (cz <= 0xFF) {
        cpu.P |= F_C;
    }
    if (cz == 0) {
        cpu.P |= F_Z;
    }
    cpu.A = erg & 0xFF;
    mem_rwb = 1;
    mem_sync = true;
}

void groupASL(int a, uint8_t idx)
{
    uint8_t *m;
    uint8_t val;
    cpu.PC++;
    switch(idx) {
    case 6:            // NOP
        printf("NOP"); 
        return;
    default:
        m = addrDecode(a);
        // get highest bit into carry
        cpu.P &= F_MASK_NZC;
        cpu.P |= (*m & 0x80) >> 7;
        val = *m;
        mem_rwb = 2;
        if (m != &cpu.A) mem_sync = true;
        while(mem_sync);
        val <<= 1;
        *m = val;
        if (m != &cpu.A) mem_sync = true;
        // set N flag
        cpu.P |= *m & 0x80;
        // set Z flag
        if (!*m) {
            cpu.P |= F_Z;
        }
        printf("ASL");
        return;
    }
}

void groupROL(int a, uint8_t idx)
{
    uint8_t *m;
    uint8_t c;
    uint8_t val;
    cpu.PC++;
    m = addrDecode(a);
    c = (*m & 0x80) >> 7;
    val = *m;
    mem_rwb = 2;
    if (m != &cpu.A) mem_sync = true;
    while(mem_sync); 
    val <<= 1;
    val |= cpu.P & F_C;
    *m = val;
    if (m != &cpu.A) mem_sync = true;
    cpu.P &= F_MASK_NZC;
    // set C flag
    cpu.P |= c;
    // set N flag
    cpu.P |= *m & 0x80;
    // set Z flag
    if (!*m) {
        cpu.P |= F_Z;
    }
    printf("ROL");
}

void groupLSR(int a, uint8_t idx)
{
    uint8_t val;
    cpu.PC++;
    uint8_t *m = addrDecode(a);
    cpu.P &= F_MASK_ZC;
    // set C flag
    val = *m;
    mem_rwb = 2;
    if (m != &cpu.A) mem_sync = true;
    while(mem_sync);
    cpu.P |= val & 1;
    val >>= 1;
    *m = val;
    if (m != &cpu.A) mem_sync = true;
    // set Z flag
    if (!*m) {
        cpu.P |= F_Z;
    }
    printf("LSR");
}

void groupROR(int a, uint8_t idx)
{
    uint8_t *m;
    uint8_t c, val;
    cpu.PC++;
    m = addrDecode(a);
    val = *m;
    mem_rwb = 2;
    if (m != &cpu.A) mem_sync = true;
    while(mem_sync); 
    c = val & 1;
    val >>= 1;
    val |= (cpu.P & F_C) << 7;
    *m = val;
    if (m != &cpu.A) mem_sync = true;

    cpu.P &= F_MASK_NZC;
     
    // N flag is C flag
    cpu.P |= *m & 0x80;
    cpu.P |= c;
    if (!*m) {
        cpu.P |= F_Z;
    }
    printf("ROR");
}

void groupSX(int a, uint8_t idx)
{
    cpu.PC++;
    *addrDecode(a) = cpu.X;
    // TXS (idx == 6) and TXA (id == 2) affect flags, STX (idx == 1,3,5) does not
    if (idx == 6 || idx == 2) {
        printf("TXS/TXA =%02X", cpu.X);
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.X & 0x80;
        if (!cpu.X) {
            // set Z flag
            cpu.P |= F_Z;
        }
    } else {
        mem_rwb = 0;
        mem_sync = true;
        printf("STX =%02X", cpu.X);
    }
}

void groupLX(int a, uint8_t idx)
{
    cpu.PC++;
    cpu.X = *addrDecode(a);
    // TSX (idx == 6) and TAX (id == 2) affect flags, LDX (idx = 0, 1, 3, 5, 7) does not
    if (idx == 6 || idx == 2) {
        printf("TSX/TAX =%02X", cpu.X);
        cpu.P &= F_MASK_NZ;
        // set N flag
        cpu.P |= cpu.X & 0x80;
        if (!cpu.X) {
            // set Z flag
            cpu.P |= F_Z;
        }
    } else {
        mem_rwb = 1;
        mem_sync = true;
        printf("LDX =%02X", cpu.X);
    }
}

void groupDEC(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t *m = addrDecode(a);
    uint8_t val;

    val = *m;
    mem_rwb = 2;
    if (m != &cpu.X) mem_sync = true;
    while(mem_sync);
    val--;
    *m = val;
    if (m != &cpu.X) mem_sync = true;
    cpu.P &= F_MASK_NZ;
    // set N flag
    cpu.P |= *m & 0x80;
    if (!*m) {
        // set Z flag
        cpu.P |= F_Z;
    }
    printf("DEC =%02X", *m);
}

void groupINC(int a, uint8_t idx)
{
    cpu.PC++;
    uint8_t val;
    if (idx == 2) {
                       // NOP
        printf("NOP");
        return;
    }
    uint8_t *m = addrDecode(a);
    printf("INC =%02X", *m);
    val = *m;
    mem_rwb = 2;
    mem_sync = true;
    while(mem_sync); 
    val++;
    *m = val;
    mem_sync = true;
    cpu.P &= F_MASK_NZ;
    // set N flag
    cpu.P |= *m & 0x80;
    if (!*m) {
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

