#ifndef __CFG_H__
#define __CFG_H__

#include <error.h>
#include <errno.h>
#include "jsmn/jsmn.h"
#include "sim.h"

static int config_parse_cpu(sim65_t *sim, jsmntok_t **jtok, char *jbuffer, int jref);
static int config_parse_chip(sim65_t *sim, jsmntok_t **jtok, char *jbuffer, int jref);
static int config_parse(sim65_t *sim, jsmntok_t **jtok, char *jbuffer, int jref);
extern int read_config_json(sim65_t *sim, char *config_file);

#endif // __CFG_H__
