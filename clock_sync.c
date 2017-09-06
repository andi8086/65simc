#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>
#include <sys/timerfd.h>
#include <string.h>
#include <errno.h>

#include "clock_sync.h"
#include "sim.h"

static int timer_fd;

void delay_sync_cycle_loop(sim65_t *sim)
{
    for (long i = 0; i < sim->clock_loop_times; i++) {
        asm volatile("nop;");
    }
}

void delay_sync_cycle_timer(sim65_t *sim)
{
    uint64_t tot_exp = 0, exp;
    size_t s;
    struct itimerspec tspec;
    tspec.it_interval.tv_sec = 0;
    tspec.it_interval.tv_nsec = F_CYC_NSEC* (sim->inst_cycles) - F_SYNC_CORRECT_TIME;
    
    tspec.it_value.tv_sec = 0;
    tspec.it_value.tv_nsec = F_CYC_NSEC* (sim->inst_cycles) - F_SYNC_CORRECT_TIME;
   
    if (timerfd_settime(timer_fd, 0, &tspec, NULL) == -1) {
        fprintf(stderr, "Error in delay_sync_cycle_timer:set %s\n", strerror(errno));
        return;
    }

    while(tot_exp < 1) {
retry:        
        s = read(timer_fd, &exp, sizeof(uint64_t));
        if (s != sizeof(uint64_t)) {
            if (errno == EAGAIN) goto retry; else {
                fprintf(stderr, "Error in delay_sync_cycle_timer %s\n", strerror(errno));
                tot_exp = sim->inst_cycles;
            }
        } else 
            tot_exp += exp;
    }
}

int delay_sync_cycle_init(sim65_t *sim)
{
    /* Get timer resolution */
    struct timespec time;
    clock_getres(CLOCK_REALTIME, &time);
    fprintf(stdout, "Realtime clock resolution: %lu ns\n", time.tv_nsec);

    /* Callibrate time delay */
    struct timespec oldtime;
    clock_gettime(CLOCK_REALTIME, &oldtime);

    for (long j = 0; j < F_CYC_CALLIB_LOOP; j++) {
        asm volatile("nop;");
    }
    clock_gettime(CLOCK_REALTIME, &time);

    long delta_ns = time.tv_nsec - oldtime.tv_nsec;
    long delta_s = time.tv_sec - oldtime.tv_sec;
    double delta = (double) delta_s + 1.0E-9 * delta_ns;
    delta *= 1.0E9;
    printf("delta = %f ns\n", delta);

    double factor = F_CYC_NSEC / delta;
    printf("factor = %.9f\n", factor);

    /* initial value */
    sim->inst_cycles = 0;
    
    sim->clock_loop = (long) round(F_CYC_CALLIB_LOOP * factor);
    printf("clock_loop = %lu\n", sim->clock_loop);
    sim->clock_loop_times = sim->clock_loop * sim->inst_cycles;

    timer_fd = timerfd_create(CLOCK_REALTIME, 0);
    fcntl(timer_fd, F_SETFL, O_NONBLOCK);

    struct itimerspec tspec;
    tspec.it_interval.tv_sec = 0;
    tspec.it_interval.tv_nsec = F_CYC_NSEC;
    
    tspec.it_value.tv_sec = 0;
    tspec.it_value.tv_nsec = F_CYC_NSEC;
   
    if (timerfd_settime(timer_fd, 0, &tspec, NULL) == -1) {
        return -1;
    }

    return 0;
}
