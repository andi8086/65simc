#ifndef __6522_H__
#define __6522_H__

#include "sim.h"

void *_6522_stepclock(icircuit *self);
icircuit *_6522_ctor(void);
void *_6522_dtor(icircuit *self);


#endif // __6522_H__
