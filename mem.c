#include <pthread.h>
#include "sim.h"
#include "mem.h"

volatile uint16_t mem_addr;
volatile int mem_rwb;
volatile bool mem_sync;

bool in_mem_range(icircuit *chip, uint16_t mem_addr)
{
    return mem_addr >= chip->address && 
           mem_addr < chip->address + chip->regsize;
}

void *chip_sync_read(icircuit *chip)
{
    if (chip->sync_read) {
        if (in_mem_range(chip, mem_addr)) {
            chip->sync_read(chip); 
        }
    }
}

void *chip_sync_write(icircuit *chip)
{
    if (chip->sync_write) {
        if (in_mem_range(chip, mem_addr)) {
            chip->sync_write(chip);
        }
    }
}

void *sync_mem_access(void *s)
{
    sim65_t* sim = (sim65_t *)s;
    
    while(sim->running) {
        /* wait till sync triggered */
        while(!mem_sync && sim->running);
            
        switch(mem_rwb) {
        case 1:
        case 2:
            /* read sync is required, so that a chip may toggle internal
             * states if its register is read 
             */
            foreach_chip(sim, (ChipFunc) chip_sync_read);
            
            /* signal CPU operation, that sync has happened 
             */
            mem_sync = false;
            if (mem_rwb == 1) break;
        case 0: 
            /* this while is only active if not coming from
             * case 2 
             */
            while(!mem_sync && sim->running);
            
            /* write sync is required, so that a chip may toggle internal
             * states if its register is written
             */
            foreach_chip(sim, (ChipFunc) chip_sync_write);
            
            /* signal CPU operation, that sync has happened 
             */
            mem_sync = false;
            break;
        }

    }
    pthread_exit(NULL);
}
