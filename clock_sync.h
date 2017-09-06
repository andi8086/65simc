#ifndef __CLOCK_SYNC_H__
#define __CLOCK_SYNC_H__

#include <stdint.h>
#include "sim.h"

/* Time in ns to subtract from wait time for
 * timer function, since system calls also need
 * some time. This value can be used to callibarte
 * the timer function.
 */
#define F_SYNC_CORRECT_TIME 1200

/* Number of nano seconds per CPU cycle
 */
#define F_CYC_NSEC 1E3

/* Number of loops used to initially calculate
 * delay loop for 1 cpu cycle
 */
#define F_CYC_CALLIB_LOOP 2E8

int delay_sync_cycle_init(sim65_t *sim);
void delay_sync_cycle_loop(sim65_t *sim);
void delay_sync_cycle_timer(sim65_t *sim);

#endif // __CLOCK_SYNC_H__
