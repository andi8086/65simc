#include <stdio.h>
#include <stdlib.h>
#include "6522.h"


void *_6522_dtor(icircuit *self)
{

}

void *_6522_stepclock(icircuit *self)
{
    

}

void *_6522_reset(icircuit *self)
{
    fprintf(stdout, "6522 reset\n");
}

icircuit *_6522_ctor(void)
{
    icircuit *ic = (icircuit *)malloc(sizeof(icircuit));

    ic->dtor = (ChipFunc) _6522_dtor;
    ic->do_reset = (ChipFunc) _6522_reset;
    ic->pulse_clock = (ChipFunc) _6522_stepclock;
    ic->next = NULL;
    return ic;
}
