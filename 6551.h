#ifndef __6551_H__
#define __6551_H__

#include "sim.h"

typedef struct _6551_registerfile {
    uint8_t data;
    uint8_t status;
    uint8_t command;
    uint8_t control;    
} _6551_registerfile;

typedef struct _6551_internal {
    int pseudo_term_fd;
} _6551_internal;

void *_6551_stepclock(icircuit *self, void *data);
icircuit *_6551_ctor(void);
void *_6551_dtor(icircuit *self, void *data);


#endif // __6551_H__
