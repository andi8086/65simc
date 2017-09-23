#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "6502.h"
#include "6551.h"

void *_6551_dtor(icircuit *self, void *data)
{
    free(self->registerfile);
    _6551_internal *i = (_6551_internal *) self->internal;
    close(i->pseudo_term_fd);
    free(self->internal);
}

void *_6551_stepclock(icircuit *self, void *data)
{
    _6551_registerfile *regs = (_6551_registerfile *) &memory[self->address];    
    _6551_internal *internal = (_6551_internal *) self->internal;
    
    /* read byte from pseudo terminal into data register */
    read(internal->pseudo_term_fd, &(regs->data), 1);
}

void *_6551_sync_read(icircuit *self, void *data)
{
    fprintf(stdout, "\n6551: reg read access\n");
}

void *_6551_sync_write(icircuit *self, void *data)
{
    // get register which is written to
    uint16_t reg = *((uint16_t *) data);
    _6551_internal *i = (_6551_internal *) self->internal;
    if (reg == 0) {
        write(i->pseudo_term_fd, &memory[self->address], 1);
    }
}

void *_6551_reset(icircuit *self, void *data)
{
    fprintf(stdout, "6551 reset\n");
}

icircuit *_6551_ctor(void)
{
    /* Create pseudo terminal to have a real interface to
     * the outer world
     */

    int fdm = posix_openpt(O_RDWR);
    if (fdm < 0) {
        fprintf(stderr, "Error opening pseudo terminal: %s\n", strerror(errno));
        exit(-1);
    } 
    
    int rc = grantpt(fdm);
    if (rc) {
        fprintf(stderr, "Error on grantpt(): %s\n", strerror(errno));
        exit(-1);
    }

    rc = unlockpt(fdm);
    if (rc) {
        fprintf(stderr, "Error on unlockpt(): %s\n", strerror(errno));
        exit(-1);
    }

    fprintf(stdout, "6551: slave is on %s", ptsname(fdm));

    icircuit *ic = (icircuit *)malloc(sizeof(icircuit));

    ic->registerfile = malloc(sizeof(_6551_registerfile));
    ic->regsize = sizeof(_6551_registerfile);
    ic->dtor = (ChipFunc) _6551_dtor;
    ic->do_reset = (ChipFunc) _6551_reset;
    ic->pulse_clock = (ChipFunc) _6551_stepclock;
    ic->next = NULL;
    ic->sync_read = (ChipFunc) _6551_sync_read;
    ic->sync_write = (ChipFunc) _6551_sync_write;

    ic->internal = malloc(sizeof(_6551_internal));
    _6551_internal *i = (_6551_internal *)ic->internal;
    i->pseudo_term_fd = fdm;
    return ic;
}
