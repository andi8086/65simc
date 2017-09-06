#ifndef __SIM_H__
#define __SIM_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool running;
    bool sync_clock;
    unsigned long clock_loop_times;
    unsigned long clock_loop;
    uint8_t inst_cycles;
    double timerstat;
    uint64_t cyclecount;
} sim65_t;

#endif //__SIM_H__
