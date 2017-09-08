#include <stdio.h>
#include <stdlib.h>
#include "AY-3-8910.h"


void *_AY_3_8910_dtor(icircuit *self)
{

}

void *_AY_3_8910_stepclock(icircuit *self)
{
    

}

void *_AY_3_8910_sync_read(icircuit *self)
{

}

void *_AY_3_8910_sync_write(icircuit *self)
{

}

void *_AY_3_8910_reset(icircuit *self)
{
    fprintf(stdout, "AY-3-8910 reset\n");
}

icircuit *_AY_3_8910_ctor(void)
{
    icircuit *ic = (icircuit *)malloc(sizeof(icircuit));

    ic->registerfile = NULL;
    ic->regsize = 0;
    ic->dtor = (ChipFunc) _AY_3_8910_dtor;
    ic->do_reset = (ChipFunc) _AY_3_8910_reset;
    ic->pulse_clock = (ChipFunc) _AY_3_8910_stepclock;
    ic->next = NULL;
    ic->sync_read = (ChipFunc) _AY_3_8910_sync_read;
    ic->sync_write = (ChipFunc) _AY_3_8910_sync_write;
    return ic;
}
