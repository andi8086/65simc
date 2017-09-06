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
#include <sched.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <argp.h>

#include <gtk/gtk.h>
#include "gui.h"
#include "cfg.h"
#include "sim.h"
#include "6502.h"
#include "clock_sync.h"

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

void *timer_func(void *s)
{
    struct timespec wait_time;
    sim65_t *state = (sim65_t *)s;

    while(state->running) {

        if (state->inst_cycles <= 3) {
            delay_sync_cycle_loop(state);
        } else {
            delay_sync_cycle_timer(state);
        }

        state->sync_clock = false;
        while(!state->sync_clock & state->running);
    }
}

void *update_func(void *threadid)
{
}

static sim65_t sim = { 0 };

int main(int argc, char **argv)
{
    pthread_t timer_thread;
    pthread_t gtk_thread;
    pthread_t update_thread;

    struct arguments arguments;

    arguments.romfile = NULL;
    arguments.romaddr = 0xC000;
    arguments.debug_regs = false;

    argp_parse(&argp, argc, argv, 0, 0, &arguments);

    bool done;
    uint8_t cc, idx;
    enum amode a;
    uint8_t op; 

    initMainWindow();

    if (delay_sync_cycle_init(&sim)) {
        fprintf(stderr, "Error initializing cycle timers\n");
        exit(-1);
    }

    /* Initialize stack pointer */
    cpu.S = 0xFF;

    /* Initialize processor status (bit 5 is always 1) */
    cpu.P = 0x20;

    /* Read ROM from file */
    FILE *romfile = fopen(arguments.romfile, "r");
    if (!romfile) {
        fprintf(stderr, "Could not load ROM from file.\n");
        exit(1);
    }
    size_t res = fread(&memory[arguments.romaddr], 1, 0xFFFF - arguments.romaddr + 1, romfile);
    fclose(romfile);
    fprintf(stdout, "ROM: %u bytes read.\n", res);

    /* Check if ROM has correct size and if vectors are present */
    if (res != 0xFFFF - arguments.romaddr + 1) {
        fprintf(stderr, "Your ROM file is too small, no data at vector table.\n");
        exit(1);
    }

    /* Fetch reset Vector */
    uint16_t start_addr = *((uint16_t *) &memory[0xFFFC]);
    cpu.PC = start_addr;

    if (read_config_json("system1.json")) {
        fprintf(stderr, "Error reading config spec\n");
        exit(-1);
    }

    sim.running = true;

    /*
    int priority = 99;
    pthread_attr_t tattr;
    struct sched_param spar;

    pthread_attr_init(&tattr);
    pthread_attr_getschedparam(&tattr, &spar);
    spar.sched_priority = priority;
    pthread_attr_setschedparam(&tattr, &spar);
    */

    /* Create timer thread */
    int rc = pthread_create(&timer_thread, NULL, timer_func, &sim);
    if (rc) {
        fprintf(stderr, "Cannot create timer thread.\n");
        exit(-1);
    }

    /* Create gtk thread */
    rc = pthread_create(&gtk_thread, NULL, gtk_main_func, &sim);
    if (rc) {
        fprintf(stderr, "Cannot create gtk thread.\n");
        exit(-1);
    }

    unsigned long oldns = 999999999;
    struct timespec time, oldtime;
    long cdelta;

    sim.timerstat = 0.0;
    sim.cyclecount = 0;

    while(cpu.PC != 0xFFFF) {

        clock_gettime(CLOCK_REALTIME, &time);

        /* fine tuning (for fastest instructions only) */
        cdelta = time.tv_nsec - oldns;
        if (cdelta < 0) {
            cdelta += 9999999999;
        }
        long expected_time = F_CYC_NSEC * sim.inst_cycles;
        /* correct clock sync loop */
        double factor = (double) expected_time / cdelta;
        sim.cyclecount++;
        if (sim.cyclecount == 1) {
            sim.timerstat = factor;
        } else {
            sim.timerstat += (factor - sim.timerstat) / (double) sim.cyclecount;
        }


        if (sim.inst_cycles <= 3) {
            if (factor < 2.0 && factor > 0.5) {
                sim.clock_loop = (unsigned long) round(sim.clock_loop * factor);
            }
        }
        oldns = time.tv_nsec;

        // fetch the opcode from memory
        op = memory[cpu.PC];

        // each opcode can be understood as aaabbbcc, where
        // bbb gives the addressing mode and ccaaa the instruction

        // extract cc
        cc = op & 3;

        // rol by 2 (opcode becomes ccaaabbb)
        op >>= 2;
        op |= cc << 6;

        // exctract instruction selector (ccaaa)
        idx = op >> 3;

        // idx specifies instruction, op & 7 is bbb, which gives
        // adressing mode
        a = opcodes[idx].addr_modes[op & 7];

        // bbb also gives number of cycles
        sim.inst_cycles = opcodes[idx].cycles[op & 7];

        // calculate needed timing loop counts
        sim.clock_loop_times = sim.clock_loop * sim.inst_cycles;
            
        printf("\n%lu,%09lu - ", time.tv_sec, time.tv_nsec);
        printf(" CYC = %u ", sim.inst_cycles);

        if (arguments.debug_regs) {
            printf("%04X : %02X ", cpu.PC, op);
            dumpRegs();
        }

        // signal the timing thread that it can start counting
        sim.sync_clock = true;

        // parallely, execute CPU instruction
        opcodes[idx].func(a, op & 7);

        // wait remaining counts done by timing thread
        while(sim.sync_clock);
    }

    // signal the timing thread that it must stop
    sim.running = false;
    
    pthread_join(timer_thread, NULL);
    pthread_join(gtk_thread, NULL);
    
    exit(0);
}
