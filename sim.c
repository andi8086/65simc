#include <stdio.h>
#include <argp.h>
#include "sim.h"

uint8_t memory[65536];

struct CPU cpu;

void group0(int a, uint8_t idx);
void group1(int a, uint8_t idx);
void group2(int a, uint8_t idx);
void group3(int a, uint8_t idx);
void group4(int a, uint8_t idx);
void group5(int a, uint8_t idx);
void group6(int a, uint8_t idx);
void group7(int a, uint8_t idx);
void groupORA(int a, uint8_t idx);
void groupAND(int a, uint8_t idx);
void groupEOR(int a, uint8_t idx);
void groupADC(int a, uint8_t idx);
void groupSTA(int a, uint8_t idx);
void groupLDA(int a, uint8_t idx);
void groupCMP(int a, uint8_t idx);
void groupSBC(int a, uint8_t idx);
void groupASL(int a, uint8_t idx);
void groupROL(int a, uint8_t idx);
void groupLSR(int a, uint8_t idx);
void groupROR(int a, uint8_t idx);
void groupSX(int a, uint8_t idx);
void groupLX(int a, uint8_t idx);
void groupDEC(int a, uint8_t idx);
void groupINC(int a, uint8_t idx);
void groupSLO(int a, uint8_t idx);
void groupRLA(int a, uint8_t idx);
void groupSRE(int a, uint8_t idx);
void groupRRA(int a, uint8_t idx);
void groupSAX(int a, uint8_t idx);
void groupLAX(int a, uint8_t idx);
void groupDCP(int a, uint8_t idx);
void groupISC(int a, uint8_t idx);

// This structure defines the PLA inside the CPU to decode the instructions

struct op_ccaaa opcodes[] = {
    {0, 0, {aNULL, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {7, 3, 3, 4, 2, 4, 2, 4}, group0},
    {0, 1, {aabs, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {6, 3, 4, 4, 2, 4, 2, 4}, group1},
    {0, 2, {aNULL, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {6, 3, 3, 3, 2, 4, 2, 4}, group2},
    {0, 3, {aNULL, azp, aNULL, aind, arel, azpx, aNULL, aabx}, {6, 3, 4, 5, 2, 4, 2, 4}, group3},
    {0, 4, {aimm, azp, aNULL, aabs, arel, azpx, aA, aabx}, {2, 3, 2, 4, 2, 4, 2, 5}, group4},
    {0, 5, {aimm, azp, aA, aabs, arel, azpx, aNULL, aabx}, {2, 3, 2, 4, 2, 4, 2, 4}, group5},
    {0, 6, {aimm, azp, aNULL, aabs, arel, azpx, aNULL, aabx}, {2, 3, 2, 4, 2, 4, 2, 4}, group6},
    {0, 7, {aimm, azp, aNULL, aabx, arel, azpx, aNULL, aabx}, {2, 3, 2, 4, 2, 4, 2, 4}, group7},
    {1, 0, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupORA},
    {1, 1, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupAND},
    {1, 2, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupEOR},
    {1, 3, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupADC},
    {1, 4, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 6, 4, 5, 5}, groupSTA},
    {1, 5, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupLDA},
    {1, 6, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupCMP},
    {1, 7, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {6, 3, 2, 4, 5, 4, 4, 4}, groupSBC},
    {2, 0, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupASL},
    {2, 1, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupROL},
    {2, 2, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupLSR},
    {2, 3, {aNULL, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {0, 5, 2, 6, 0, 6, 2, 7}, groupROR},
    {2, 4, {aimm, azp, aA, aabs, aNULL, azpy, aS, aaby}, {2, 3, 2, 4, 0, 4, 2, 5}, groupSX},
    {2, 5, {aimm, azp, aA, aabs, aNULL, azpy, aS, aaby}, {2, 3, 2, 4, 0, 4, 2, 4}, groupLX},
    {2, 6, {aimm, azp, aX, aabs, aNULL, azpx, aNULL, aabx}, {2, 5, 2, 6, 0, 6, 0, 7}, groupDEC},
    {2, 7, {aimm, azp, aNULL, aabs, aNULL, azpx, aNULL, aabx}, {2, 5, 2, 6, 0, 6, 2, 7}, groupINC},
    {3, 0, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupSLO},
    {3, 1, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupRLA},
    {3, 2, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupSRE},
    {3, 3, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6 , 8, 6, 7, 7}, groupRRA},
    {3, 4, {aizx, azp, aimm, aabs, aizy, azpy, aaby, aaby}, {6, 3, 2, 4, 6, 4, 5, 5}, groupSAX},
    {3, 5, {aizx, azp, aimm, aabs, aizy, azpy, aaby, aaby}, {6, 3, 2, 4, 5, 4, 4, 4}, groupLAX},
    {3, 6, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupDCP},
    {3, 7, {aizx, azp, aimm, aabs, aizy, azpx, aaby, aabx}, {8, 5, 2, 6, 8, 6, 7, 7}, groupISC}
};

void dumpRegs()
{
    printf("PC = %04X  S = %02X  A = %02X   X = %02X   Y = %02X\n", cpu.PC, cpu.S, cpu.A, cpu.X, cpu.Y);

}


const char *argp_program_version = "sim6502c 0.01 - Alpha";
const char doc[] = "sim6502c - an extendable 6502 simulator in C";

static struct argp_option options[] = {
    {"rom", 'r', "FILE", 0, "Specify ROM file"},
    {"raddr", 'a', "HEX", 0, "Specify ROM address in HEX, default is C000"}
};

struct arguments
{
    char *romfile;
    uint16_t romaddr;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch(key)
    {
    case 'r':
        arguments->romfile = arg;
        fprintf(stdout, "ROM is loaded from %s", arguments->romfile);
        break;
    case 'a':
        arguments->romaddr = strtol(arg, NULL, 16);
        fprintf(stdout, "ROM starts at %u", arguments->romaddr);
        break;
    case ARGP_KEY_ARG:
        if (state->arg_num > 0) {
            argp_usage(state);
        }
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = {
    options, parse_opt, 0, doc
};

int main(int argc, char **argv)
{
    struct arguments arguments;

    arguments.romfile = NULL;
    arguments.romaddr = 0xC000;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    bool done;
    uint8_t cc, idx;
    enum amode a;
    uint8_t op, cycles;
    struct timespec wait_time = {0};
    struct timespec time, time_old;
    
    /* Get timer resolution */
    clock_getres(CLOCK_REALTIME, &time);
    printf("Realtime clock resolution: %lu ns\n", time.tv_nsec);



    cpu.S = 0xFF;
    cpu.P = 0x20;   // undefined bit is always set

    FILE *romfile = fopen(arguments.romfile, "r");
    if (!romfile) {
        fprintf(stderr, "Could not load ROM from file.\n");
        exit(1);
    }  

    size_t res = fread(&memory[arguments.romaddr], 1, 0xFFFF - arguments.romaddr + 1, romfile);
    
    fclose(romfile);
    
    fprintf(stdout, "ROM: %u bytes read.", res);

    if (res != 0xFFFF - arguments.romaddr + 1) {
        fprintf(stderr, "Your ROM file is too small, no data at vector table.\n");
        exit(1);
    }
    /* Reset Vector */
    uint16_t start_addr = *((uint16_t *) &memory[0xFFFC]);
    cpu.PC = start_addr;

    clock_gettime(CLOCK_REALTIME, &time_old);
    clock_gettime(CLOCK_REALTIME, &time);
    printf("%lu - \n", time.tv_nsec - time_old.tv_nsec);

    while(cpu.PC != 0xFFFF) {
        clock_gettime(CLOCK_REALTIME, &time);
        printf("%lu,%lu - ", time.tv_sec, time.tv_nsec);

        op = memory[cpu.PC];
        printf("Opcode: %2X ", op);
        cc = op & 3;
        op >>= 2;
        op |= cc << 6;
        idx = op >> 3;
//        printf("%3o\n", op);
        a = opcodes[idx].addr_modes[op & 7];
//        printf("%d\n", idx);
        cycles = opcodes[idx].cycles[op & 7];
        dumpRegs();
        
        opcodes[idx].func(a, op & 7);
        // wait cycles
        wait_time.tv_nsec = cycles * 500;
//        nanosleep(&wait_time, NULL); 
    }

    return 0;

}
