#include <stdio.h>
#include <stdlib.h>
#include "AY-3-8910.h"


void *_AY_3_8910_dtor(icircuit *self)
{

}

void *_AY_3_8910_stepclock(icircuit *self)
{
    

}

void *_AY_3_8910_reset(icircuit *self)
{
    fprintf(stdout, "AY-3-8910 reset\n");
}

icircuit *_AY_3_8910_ctor(void)
{
    icircuit *ic = (icircuit *)malloc(sizeof(icircuit));

    ic->dtor = (ChipFunc) _AY_3_8910_dtor;
    ic->do_reset = (ChipFunc) _AY_3_8910_reset;
    ic->pulse_clock = (ChipFunc) _AY_3_8910_stepclock;
    ic->next = NULL;
    return ic;
}
