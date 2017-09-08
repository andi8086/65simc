#ifndef __6522_H__
#define __6522_H__

#include "sim.h"

typedef struct _6522_registerfile {
    uint8_t ORB_IRB;
    uint8_t ORA_IRA;
    uint8_t DDRB;
    uint8_t DDRA;
    uint8_t T1C_L;
    uint8_t T1C_H;
    uint8_t T1L_L;
    uint8_t T1L_H;
    uint8_t T2C_L;
    uint8_t T2C_H;
    uint8_t SR;
    uint8_t ACR;
    uint8_t PCR;
    uint8_t IFR;
    uint8_t IER;
    uint8_t ORA_IRA_NO_HS;
} _6522_registerfile;

void *_6522_stepclock(icircuit *self);
icircuit *_6522_ctor(void);
void *_6522_dtor(icircuit *self);


#endif // __6522_H__
