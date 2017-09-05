#ifndef __CFG_H__
#define __CFG_H__

#include <error.h>
#include <errno.h>
#include "jsmn/jsmn.h"

static int config_parse_cpu(jsmntok_t **jtok, char *jbuffer, int jref);
static int config_parse(jsmntok_t **jtok, char *jbuffer, int jref);
extern int read_config_json(char *config_file);

#endif // __CFG_H__
