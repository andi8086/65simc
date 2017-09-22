#ifndef __AY_3_8910_H__
#define __AY_3_8910_H__

#include "sim.h"

void *_AY_3_8910_stepclock(icircuit *self, void *data);
icircuit *_AY_3_8910_ctor(void);
void *_AY_3_8910_dtor(icircuit *self, void *data);


#endif // __AY_3_8910_H__
