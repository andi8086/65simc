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

#include <pthread.h>
#include <stdio.h>
#include <math.h>
#include <argp.h>
#include "6502.h"

#define F_CYC_NSEC 1000

uint8_t memory[65536];

void dumpRegs()
{
    printf("S = %02X  A = %02X   X = %02X   Y = %02X  ", cpu.S, cpu.A, cpu.X, cpu.Y);
    printf("FLAGS = %c%c%c%c%c%c%c   ", 
        cpu.P & F_N ? 'N' : ' ',
        cpu.P & F_V ? 'V' : ' ',
        cpu.P & F_B ? 'B' : ' ',
        cpu.P & F_D ? 'D' : ' ',
        cpu.P & F_I ? 'I' : ' ',
        cpu.P & F_Z ? 'Z' : ' ',
        cpu.P & F_C ? 'C' : ' ');
}


const char *argp_program_version = "sim6502c 0.01 - Alpha";
const char doc[] = "sim6502c - an extendable 6502 simulator in C";

static struct argp_option options[] = {
    {"rom", 'r', "FILE", 0, "Specify ROM file"},
    {"raddr", 'a', "HEX", 0, "Specify ROM address in HEX, default is C000"},
    {0}
};

struct arguments
{
    char *romfile;
    uint16_t romaddr;
    bool debug_regs;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = state->input;

    switch(key)
    {
    case 'r':
        arguments->romfile = arg;
        fprintf(stdout, "ROM is loaded from %s\n", arguments->romfile);
        break;
    case 'a':
        arguments->romaddr = strtol(arg, NULL, 16);
        fprintf(stdout, "ROM starts at %u\n", arguments->romaddr);
        break;
    case 'd':
        arguments->debug_regs = true;
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

volatile bool sim_running = true;
volatile bool wait_clock = false;
volatile long clock_loop, clock_loop_times;

void *timer_func(void *threadid)
{
    struct timespec wait_time;
            
    while(sim_running) {

        for (long i = 0; i < clock_loop_times; i++) {
            asm("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop");
        }

        wait_clock = false;
        while(!wait_clock & sim_running);
    }
}

int main(int argc, char **argv)
{
    pthread_t timer_thread;

    struct arguments arguments;

    arguments.romfile = NULL;
    arguments.romaddr = 0xC000;
    arguments.debug_regs = false;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    bool done;
    uint8_t cc, idx;
    enum amode a;
    uint8_t op, cycles;
    struct timespec time;

    /* Get timer resolution */
    clock_getres(CLOCK_REALTIME, &time);
    fprintf(stdout, "Realtime clock resolution: %lu ns\n", time.tv_nsec);

    /* Callibrate time delay */
    
    // 
    struct timespec oldtime;
    clock_gettime(CLOCK_REALTIME, &oldtime);
    for (long j = 0; j < 5E8; j++) {
        asm("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop");
    }
    clock_gettime(CLOCK_REALTIME, &time);
    
    long delta_ns = time.tv_nsec - oldtime.tv_nsec;
    long delta_s = time.tv_sec - oldtime.tv_sec;
    double delta = (double) delta_s + 1.0E-9 * delta_ns;
    printf("delta = %f\n", delta);

    double factor = F_CYC_NSEC * 1E-9 / delta;
    printf("factor = %.9f\n", factor);

    clock_loop = (long) round(5.0E8 * factor);
    printf("clock_loop = %lu\n", clock_loop);
    clock_loop_times = clock_loop * 10;

    cpu.S = 0xFF;
    cpu.P = 0x20;   // undefined bit is always set

    FILE *romfile = fopen(arguments.romfile, "r");
    if (!romfile) {
        fprintf(stderr, "Could not load ROM from file.\n");
        exit(1);
    }  

    size_t res = fread(&memory[arguments.romaddr], 1, 0xFFFF - arguments.romaddr + 1, romfile);
    
    fclose(romfile);
    
    fprintf(stdout, "ROM: %u bytes read.\n", res);

    if (res != 0xFFFF - arguments.romaddr + 1) {
        fprintf(stderr, "Your ROM file is too small, no data at vector table.\n");
        exit(1);
    }
    /* Reset Vector */
    uint16_t start_addr = *((uint16_t *) &memory[0xFFFC]);
    cpu.PC = start_addr;

    /* Create timer thread */

    int rc = pthread_create(&timer_thread, NULL, timer_func, NULL);
    if (rc) {
        fprintf(stderr, "Cannot create timer thread.\n");
        exit(-1);
    }

    long oldns;
    long cdelta;
    while(cpu.PC != 0xFFFF) {
        clock_gettime(CLOCK_REALTIME, &time);
        
        /* fine tuning */
        cdelta = time.tv_nsec - oldns;
        if (cdelta > 0) {
            if (cdelta > F_CYC_NSEC * cycles) {
                clock_loop -= clock_loop / 4;
            }
            if (cdelta < F_CYC_NSEC * cycles) {
                clock_loop += clock_loop / 4;
            }
        }
        oldns = time.tv_nsec;
        printf("\n%lu,%09lu - ", time.tv_sec, time.tv_nsec);
        op = memory[cpu.PC];
        printf("%04X : %02X ", cpu.PC, op);
        cc = op & 3;
        op >>= 2;
        op |= cc << 6;
        idx = op >> 3;
        a = opcodes[idx].addr_modes[op & 7];
        cycles = opcodes[idx].cycles[op & 7];
        printf(" CYC = %u ", cycles);
        clock_loop_times = clock_loop * cycles;
        if (arguments.debug_regs) {
            dumpRegs();
        }
        wait_clock = true;
        opcodes[idx].func(a, op & 7);
        // wait cycles
        while(wait_clock);
    }
    sim_running = false;

    pthread_exit(NULL);
}
