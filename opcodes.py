#!/usr/bin/env python2

max_bits = 8

rol = lambda val, r_bits: \
    ((val << r_bits % max_bits)) & (2**max_bits - 1) | \
    ((val & (2**max_bits-1)) >> (max_bits-(r_bits % max_bits)))


ror = lambda val, r_bits: \
    ((val & (2**max_bits-1)) >> r_bits%max_bits) | \
    (val << (max_bits - (r_bits % max_bits)) & (2**max_bits - 1))

opcode_table = {
 0x00: "BRK, NULL, 7" 
, 0x01: "ORA, izx, 6"
, 0x02: "*KIL, NULL, 0"
, 0x03: "*SLO, izx, 8"
, 0x04: "*NOP, zp, 3"
, 0x05: "ORA, zp, 3"
, 0x06: "ASL, zp, 5"
, 0x07: "*SLO, zp, 5"
, 0x08: "PHP, NULL, 3"
, 0x09: "ORA, imm, 2"
, 0x0A: "ASL, NULL, 2"
, 0x0B: "*ANC, imm, 2"
, 0x0C: "*NOP, abs, 4"
, 0x0D: "ORA, abs, 4"
, 0x0E: "ASL, abs, 6"
, 0x0F: "*SLO, abs, 6"
 
, 0x10: "BPL, rel, 2*"
, 0x11: "ORA, izy, 5*"
, 0x12: "*KIL, NULL, 0"
, 0x13: "*SLO, izy, 8"
, 0x14: "*NOP, zpx, 4"
, 0x15: "ORA, zpx, 4"
, 0x16: "ASL, zpx, 6"
, 0x17: "*SLO, zpx, 6"
, 0x18: "CLC, NULL,  2"
, 0x19: "ORA, aby, 4*"
, 0x1A: "NOP, NULL, 2"
, 0x1B: "*SLO, aby, 7"
, 0x1C: "*NOP, abx, 4*"
, 0x1D: "ORA, abx, 4*"
, 0x1E: "ASL, abx, 7"
, 0x1F: "*SLO, abx, 7"
 
, 0x20: "JSR, abs, 6"
, 0x21: "AND, izx, 6"
, 0x22: "*KIL, NULL, 0"
, 0x23: "*RLA, izx, 8"
, 0x24: "BIT, zp, 3"
, 0x25: "AND, zp, 3"
, 0x26: "ROL, zp, 5"
, 0x27: "*RLA, zp, 5"
, 0x28: "PLP, NULL, 4"
, 0x29: "AND, imm, 2"
, 0x2A: "ROL, NULL, 2"
, 0x2B: "*ANC, imm, 2"
, 0x2C: "BIT, abs, 4"
, 0x2D: "AND, abs, 4"
, 0x2E: "ROL, abs, 6"
, 0x2F: "*RLA, abs, 6"

, 0x30: "BMI, rel, 2*"
, 0x31: "AND, izy, 5*"
, 0x32: "*KIL, NULL, 0"
, 0x33: "*RLA, izy, 8"
, 0x34: "*NOP, zpx, 4"
, 0x35: "AND, zpx, 4"
, 0x36: "ROL, zpx, 6"
, 0x37: "*RLA, zpx, 6"
, 0x38: "SEC, NULL, 2"
, 0x39: "AND, aby, 4*"
, 0x3A: "*NOP, NULL, 2"
, 0x3B: "*RLA, aby, 7"
, 0x3C: "*NOP, abx, 4*"
, 0x3D: "AND, abx, 4*"
, 0x3E: "ROL, abx, 7"
, 0x3F: "*RLA, abx, 7"

, 0x40: "RTI, NULL, 6"
, 0x41: "EOR, izx, 6"
, 0x42: "*KIL, NULL, 0"
, 0x43: "*SRE, izx, 8"
, 0x44: "*NOP, zp, 3"
, 0x45: "EOR, zp, 3"
, 0x46: "LSR, zp, 5"
, 0x47: "*SRE, zp, 5"
, 0x48: "PHA, NULL, 3"
, 0x49: "EOR, imm, 2"
, 0x4A: "LSR, NULL, 2"
, 0x4B: "*ALR, imm, 2"
, 0x4C: "JMP, abs, 3"
, 0x4D: "EOR, abs, 4"
, 0x4E: "LSR, abs, 6"
, 0x4F: "*SRE, abs, 6"

, 0x50: "BVC, rel, 2*"
, 0x51: "EOR, izy, 5*"
, 0x52: "*KIL, NULL, 0"
, 0x53: "*SRE, izy, 8"
, 0x54: "*NOP, zpx, 4"
, 0x55: "EOR, zpx, 4"
, 0x56: "LSR, zpx, 6"
, 0x57: "*SRE, zpx, 6"
, 0x58: "CLI, NULL, 2"
, 0x59: "EOR, aby, 4*"
, 0x5A: "*NOP, NULL, 2"
, 0x5B: "*SRE, aby, 7"
, 0x5C: "*NOP, abx, 4*"
, 0x5D: "EOR, abx, 4*"
, 0x5E: "LSR, abx, 7"
, 0x5F: "*SRE, abx, 7"

, 0x60: "RTS, NULL, 6"
, 0x61: "ADC, izx, 6"
, 0x62: "*KIL, NULL, 0"
, 0x63: "*RRA, izx, 8"
, 0x64: "*NOP, zp, 3"
, 0x65: "ADC, zp, 3"
, 0x66: "ROR, zp, 5"
, 0x67: "*RRA, zp, 5"
, 0x68: "PLA, NULL, 4"
, 0x69: "ADC, imm, 2"
, 0x6A: "ROR, NULL, 2"
, 0x6B: "*ARR, imm, 2"
, 0x6C: "JMP, ind, 5"
, 0x6D: "ADC, abs, 4"
, 0x6E: "ROR, abs, 6"
, 0x6F: "*RRA, abs, 6"

, 0x70: "BVS, rel, 2*"
, 0x71: "ADC, izy, 5*"
, 0x72: "*KIL, NULL, 0"
, 0x73: "*RRA, izy, 8"
, 0x74: "*NOP, zpx, 4"
, 0x75: "ADC, zpx, 4"
, 0x76: "ROR, zpx, 6"
, 0x77: "*RRA, zpx, 6"
, 0x78: "SEI, NULL, 2"
, 0x79: "ADC, aby, 4*"
, 0x7A: "*NOP, NULL, 2"
, 0x7B: "*RRA, aby, 7"
, 0x7C: "*NOP, abx, 4*"
, 0x7D: "ADC, abx, 4*"
, 0x7E: "ROR, abx, 7"
, 0x7F: "*RRA, abx, 7"

, 0x80: "*NOP, imm, 2"
, 0x81: "STA, izx, 6"
, 0x82: "*NOP, imm, 2"
, 0x83: "*SAX, izx, 6"
, 0x84: "STY, zp, 3"
, 0x85: "STA, zp, 3"
, 0x86: "STX, zp, 3"
, 0x87: "*SAX, zp, 3"
, 0x88: "DEY, NULL, 2"
, 0x89: "*NOP, imm, 2"
, 0x8A: "TXA, NULL, 2"
, 0x8B: "***XAA, imm, 2"
, 0x8C: "STY, abs, 4"
, 0x8D: "STA, abs, 4"
, 0x8E: "STX, abs, 4"
, 0x8F: "*SAX, abs, 4"

, 0x90: "BCC, rel, 2*"
, 0x91: "STA, izy, 6"
, 0x92: "*KIL, NULL, "
, 0x93: "**AHX, izy, 6"
, 0x94: "STY, zpx, 4"
, 0x95: "STA, zpx, 4"
, 0x96: "STX, zpy, 4"
, 0x97: "*SAX, zpy, 4"
, 0x98: "TYA, NULL, 2"
, 0x99: "STA, aby, 5"
, 0x9A: "TXS, NULL, 2"
, 0x9B: "**TAS, aby, 5"
, 0x9C: "**SHY, abx, 5"
, 0x9D: "STA, abx, 5"
, 0x9E: "**SHX, aby, 5"
, 0x9F: "**AHX, aby, 5"

, 0xA0: "LDY, imm, 2"
, 0xA1: "LDA, izx, 6"
, 0xA2: "LDX, imm, 2"
, 0xA3: "*LAX, izx, 6"
, 0xA4: "LDY, zp, 3"
, 0xA5: "LDA, zp, 3"
, 0xA6: "LDX, zp, 3"
, 0xA7: "*LAX, zp, 3"
, 0xA8: "TAY, NULL, 2"
, 0xA9: "LDA, imm, 2"
, 0xAA: "TAX, NULL, 2"
, 0xAB: "***LAX, imm, 2"
, 0xAC: "LDY, abs, 4"
, 0xAD: "LDA, abs, 4"
, 0xAE: "LDX, abs, 4"
, 0xAF: "*LAX, abs, 4"

, 0xB0: "BCS, rel, 2*"
, 0xB1: "LDA, izy, 5*"
, 0xB2: "*KIL, NULL, 0"
, 0xB3: "*LAX, izy, 5*"
, 0xB4: "LDY, zpx, 4"
, 0xB5: "LDA, zpx, 4"
, 0xB6: "LDX, zpy, 4"
, 0xB7: "*LAX, zpy, 4"
, 0xB8: "CLV, NULL, 2"
, 0xB9: "LDA, aby, 4*"
, 0xBA: "TSX, NULL, 2"
, 0xBB: "*LAS, aby, 4*"
, 0xBC: "LDY, abx, 4*"
, 0xBD: "LDA, abx, 4*"
, 0xBE: "LDX, aby, 4*"
, 0xBF: "*LAX, aby, 4*"

, 0xC0: "CPY, imm, 2"
, 0xC1: "CMP, izx, 6"
, 0xC2: "*NOP, imm, 2"
, 0xC3: "*DCP, izx, 8"
, 0xC4: "CPY, zp, 3"
, 0xC5: "CMP, zp, 3"
, 0xC6: "DEC, zp, 5"
, 0xC7: "*DCP, zp, 5"
, 0xC8: "INY, NULL, 2"
, 0xC9: "CMP, imm, 2"
, 0xCA: "DEX, NULL, 2"
, 0xCB: "*AXS, imm, 2"
, 0xCC: "CPY, abs, 4"
, 0xCD: "CMP, abs, 4"
, 0xCE: "DEC, abs, 6"
, 0xCF: "*DCP, abs, 6"

, 0xD0: "BNE, rel, 2*"
, 0xD1: "CMP, izy, 5*"
, 0xD2: "*KIL, NULL, 0"
, 0xD3: "*DCP, izy, 8"
, 0xD4: "*NOP, zpx, 4"
, 0xD5: "CMP, zpx, 4"
, 0xD6: "DEC, zpx, 6"
, 0xD7: "*DCP, zpx, 6"
, 0xD8: "CLD, NULL, 2"
, 0xD9: "CMP, aby, 4*"
, 0xDA: "*NOP, NULL, 0"
, 0xDB: "*DCP, aby, 7"
, 0xDC: "*NOP, abx, 4*"
, 0xDD: "CMP, abx, 4*"
, 0xDE: "DEC, abx, 7"
, 0xDF: "*DCP, abx, 7"

, 0xE0: "CPX, imm, 2"
, 0xE1: "SBC, izx, 6"
, 0xE2: "*NOP, imm, 2"
, 0xE3: "*ISC, izx, 8"
, 0xE4: "CPX, zp, 3"
, 0xE5: "SBC, zp, 3"
, 0xE6: "INC, zp, 5"
, 0xE7: "*ISC, zp, 5"
, 0xE8: "INX, NULL, 2"
, 0xE9: "SBC, imm, 2"
, 0xEA: "NOP, NULL, 2"
, 0xEB: "*SBC, imm, 2"
, 0xEC: "CPX, abx, 4"
, 0xED: "SBC, abs, 4"
, 0xEE: "INC, abs, 6"
, 0xEF: "*ISC, abs, 6"

, 0xF0: "BEQ, rel, 2*"
, 0xF1: "SBC, izy, 5*"
, 0xF2: "*KIL, NULL, 0"
, 0xF3: "*ISC, izy, 8"
, 0xF4: "*NOP, zpx, 4"
, 0xF5: "SBC, zpx, 4"
, 0xF6: "INC, zpx, 6"
, 0xF7: "*ISC, zpx, 6"
, 0xF8: "SED, NULL, 2"
, 0xF9: "SBC, aby, 4*"
, 0xFA: "*NOP, NULL, 2"
, 0xFB: "*ISC, aby, 7"
, 0xFC: "*NOP, abx, 4*"
, 0xFD: "SBC, abx, 4*"
, 0xFE: "INC, abx, 7"
, 0xFF: "*ISC, abx, 7"
}


print("imm = #$00, zp = $00, zpx = $00,X, zpy = $00,Y, izx = ($00,X), izy= ($00),Y")
print("abs = $0000, abx = $0000,X, aby = $0000,Y, ind=($0000), rel = $0000 + PC")
print("Opcode (aaabbbcc)    (cc)   (aaa)  (bbb)")
for i in range(0, 256):
    ri = rol(i, 2)
    print("{:02X}    ".format(ri), ror(i,6) & 3, ror(i,3) & 7, i & 7, opcode_table[ri])
