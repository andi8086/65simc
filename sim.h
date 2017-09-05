#ifndef __SIM_H__
#define __SIM_H__

#include <stdbool.h>


typedef struct {
    bool running;
    bool sync_clock;
    unsigned long clock_loop_times;
    unsigned long clock_loop;
} sim65_t;

#endif //__SIM_H__
