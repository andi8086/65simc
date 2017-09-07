#include <stdio.h>
#include <stdlib.h>
#include "6551.h"


void *_6551_dtor(icircuit *self)
{

}

void *_6551_stepclock(icircuit *self)
{
    

}

void *_6551_reset(icircuit *self)
{
    fprintf(stdout, "6551 reset\n");
}

icircuit *_6551_ctor(void)
{
    icircuit *ic = (icircuit *)malloc(sizeof(icircuit));

    ic->dtor = (ChipFunc) _6551_dtor;
    ic->do_reset = (ChipFunc) _6551_reset;
    ic->pulse_clock = (ChipFunc) _6551_stepclock;
    ic->next = NULL;
    return ic;
}
