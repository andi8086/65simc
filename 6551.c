#include <stdio.h>
#include <stdlib.h>
#include "6502.h"
#include "6551.h"


void *_6551_dtor(icircuit *self)
{
    free(self->registerfile);
}

void *_6551_stepclock(icircuit *self)
{
    _6551_registerfile *regs = (_6551_registerfile *) &memory[self->address];    

    // Check if data has been written... but how???? :D

}

void *_6551_sync_read(icircuit *self)
{
    fprintf(stdout, "\n6551: reg read access\n");
}

void *_6551_sync_write(icircuit *self)
{
    fprintf(stdout, "\n6551: reg write access\n");
}

void *_6551_reset(icircuit *self)
{
    fprintf(stdout, "6551 reset\n");
}

icircuit *_6551_ctor(void)
{
    icircuit *ic = (icircuit *)malloc(sizeof(icircuit));

    ic->registerfile = malloc(sizeof(_6551_registerfile));
    ic->regsize = sizeof(_6551_registerfile);
    ic->dtor = (ChipFunc) _6551_dtor;
    ic->do_reset = (ChipFunc) _6551_reset;
    ic->pulse_clock = (ChipFunc) _6551_stepclock;
    ic->next = NULL;
    ic->sync_read = (ChipFunc) _6551_sync_read;
    ic->sync_write = (ChipFunc) _6551_sync_write;
    return ic;
}
