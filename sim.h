#ifndef __SIM_H__
#define __SIM_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct icircuit icircuit;

typedef void (*ChipFunc)(struct icircuit *);
typedef icircuit (*ChipCtor)(uint16_t addr);

typedef struct icircuit {
    void *registerfile;
    uint16_t address;
    ChipFunc pulse_clock;
    ChipFunc do_reset;
    icircuit *next;
    ChipFunc dtor;
} icircuit;

typedef struct {
    bool running;
    bool sync_clock;
    unsigned long clock_loop_times;
    unsigned long clock_loop;
    uint8_t inst_cycles;
    double timerstat;
    uint64_t cyclecount;
    icircuit *circuit;
} sim65_t;

void add_chip(sim65_t *sim, icircuit *chip);
void foreach_chip(sim65_t *sim, ChipFunc func);

#endif //__SIM_H__
