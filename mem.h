#ifndef __MEM_H__
#define __MEM_H__

#include <stdint.h>
#include <stdbool.h>

extern volatile uint16_t mem_addr;
/* 0 = sync write, 1 = sync read, 2 = sync read, then write */
extern volatile int mem_rwb;
extern volatile bool mem_sync;

extern void *sync_mem_access(void *s);

#endif // __MEM_H__
