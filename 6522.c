#include <stdio.h>
#include <stdlib.h>
#include "6522.h"


void *_6522_dtor(icircuit *self)
{

}

void *_6522_stepclock(icircuit *self)
{
    

}

void *_6522_sync_read(icircuit *self)
{
    fprintf(stdout, "\n6522: reg read access\n");
}

void *_6522_sync_write(icircuit *self)
{
    fprintf(stdout, "\n6522: reg write access\n");
}

void *_6522_reset(icircuit *self)
{
    fprintf(stdout, "6522 reset\n");
}

icircuit *_6522_ctor(void)
{
    icircuit *ic = (icircuit *)malloc(sizeof(icircuit));

    ic->registerfile = malloc(sizeof(_6522_registerfile));
    ic->regsize = sizeof(_6522_registerfile);
    ic->dtor = (ChipFunc) _6522_dtor;
    ic->do_reset = (ChipFunc) _6522_reset;
    ic->pulse_clock = (ChipFunc) _6522_stepclock;
    ic->next = NULL;
    ic->sync_read = (ChipFunc) _6522_sync_read;
    ic->sync_write = (ChipFunc) _6522_sync_write;
    return ic;
}
