#include <stdio.h>
#include <stdlib.h>
#include "cfg.h"
#include <string.h>
#include "6522.h"
#include "6551.h"
#include "AY-3-8910.h"

static int config_parse_cpu(sim65_t *sim, jsmntok_t **jtok, char *jbuffer, int jref)
{
    char buffer[256];
    int res; 
    int children;     

    (*jtok)++;
    jsmntype_t type = (*jtok)->type;
    if (type != JSMN_OBJECT) {
        fprintf(stderr, "Error, CPU definition must be json object\n");
        return EINVAL;
    }
    children = (*jtok)->size;

    for (int i = 0; i < children; i++) {
        (*jtok)++;
        int start = (*jtok)->start;
        int end = (*jtok)->end;
        type = (*jtok)->type;
        snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
        buffer[end-start+1] = '\0';

        switch(type) {
        case JSMN_STRING:
            if (strncmp(buffer, "type", strlen("type")+1) == 0) {
                if ((*jtok)->size != 1) {
                    fprintf(stderr, "Invalid cpu type spec\n");
                    return EINVAL;
                }
                (*jtok)++;
                start = (*jtok)->start;
                end = (*jtok)->end;
                type = (*jtok)->type;
                if (type != JSMN_STRING) {
                    fprintf(stderr, "Invalid cpu type spec\n");
                    return EINVAL;
                }
                snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
                buffer[end-start+1] = '\0';

                if (strncmp(buffer, "6502", strlen("6502")+1) == 0) {
                    fprintf(stdout, "System CPU: MOS 6502\n");
                } else {
                    fprintf(stderr, "Invalid System CPU selected.\n");
                    return EINVAL;
                }

            }
            break;        
        }

        res = config_parse(sim, jtok, jbuffer, jref+1);
        if (res) {
            return res;
        }
    }
    return 0;
}

static int config_parse_chip(sim65_t *sim, jsmntok_t **jtok, char *jbuffer, int jref)
{
    char buffer[256];
    int res; 
    int children;
    icircuit *chip = NULL;
    
    (*jtok)++;
    jsmntype_t type = (*jtok)->type;
    if (type != JSMN_OBJECT) {
        fprintf(stderr, "Error, chip definition must be json object\n");
        return EINVAL;
    }
    children = (*jtok)->size;

    for (int i = 0; i < children; i++) {
        (*jtok)++;
        int start = (*jtok)->start;
        int end = (*jtok)->end;
        type = (*jtok)->type;
        snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
        buffer[end-start+1] = '\0';
        
        

        switch(type) {
        case JSMN_STRING:
            if (strncmp(buffer, "type", strlen("type")+1) == 0) {
                if (chip) {
                    fprintf(stderr, "Error, chip type already set.\n");
                    return EINVAL;
                }
                
                if ((*jtok)->size != 1) {
                    fprintf(stderr, "Invalid cpu type spec\n");
                    return EINVAL;
                }
                (*jtok)++;
                start = (*jtok)->start;
                end = (*jtok)->end;
                type = (*jtok)->type;
                if (type != JSMN_STRING) {
                    fprintf(stderr, "Invalid chip type spec\n");
                    return EINVAL;
                }
                snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
                buffer[end-start+1] = '\0';

                if (strncmp(buffer, "6522", strlen("6522")+1) == 0) {
                    fprintf(stdout, "Adding chip: 6522\n");
                    
                    chip = _6522_ctor();
                    add_chip(sim, chip);
                } else
                if (strncmp(buffer, "6551", strlen("6551")+1) == 0) {
                    fprintf(stdout, "Adding chip: 6551\n");
                
                    chip = _6551_ctor();
                    add_chip(sim, chip);
                } else 
                if (strncmp(buffer, "AY-3-8910", strlen("AY-3-8910")+1) == 0) {
                    fprintf(stdout, "Adding chip: AY-3-8910\n");
                    
                    chip = _AY_3_8910_ctor();
                    add_chip(sim, chip);
                } else
                {
                    fprintf(stderr, "Invalid chip selected: %s.\n", buffer);
                    return EINVAL;
                }

            } else
            if (strncmp(buffer, "addr", strlen("addr")+1) == 0) {
                if ((*jtok)->size != 1) {
                    fprintf(stderr, "Invalid cpu type spec\n");
                    return EINVAL;
                }
                (*jtok)++;
                start = (*jtok)->start;
                end = (*jtok)->end;
                type = (*jtok)->type;
                if (type != JSMN_STRING) {
                    fprintf(stderr, "Invalid chip type spec\n");
                    return EINVAL;
                }
                if (!chip) {
                    fprintf(stderr, "Invalid chip spec. Type must be set first.\n");
                    return EINVAL;
                }

                snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
                buffer[end-start+1] = '\0';
                unsigned long address = strtol(buffer, NULL, 16);
                chip->address = (uint16_t) address & 0xFFFF;
                printf("    Address: %04X\n", chip->address);
            } else {
                (*jtok)++;
            }
        }
    }
    return 0;
}

static int config_parse(sim65_t *sim, jsmntok_t **jtok, char *jbuffer, int jref)
{
    char buffer[256];
    int res;

    int children = (*jtok)->size;

    for (int i = 0; i < children; i++) {
        (*jtok)++;
        int start = (*jtok)->start;
        int end = (*jtok)->end;
        jsmntype_t type = (*jtok)->type;
        snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
        buffer[end-start+1] = '\0';

        switch(type) {
        case JSMN_STRING:
            if (strncmp("cpu", buffer, strlen("cpu")+1) == 0) {
                res = config_parse_cpu(sim, jtok, jbuffer, jref+1);
                if (res) {
                    return res;
                }
                break;
            } else
            if (strncmp("chip", buffer, strlen("chip")+1) == 0) {
                res = config_parse_chip(sim, jtok, jbuffer, jref+1);
                if (res) {
                    return res;
                }
            }
        default:
            res = config_parse(sim, jtok, jbuffer, jref+1);
            if (res) {
                return res;
            }
            break;
        }
    }

    return 0;
}

int read_config_json(sim65_t *sim, char *config_file) 
{
    char jbuffer[16384];
    FILE *configfile = fopen(config_file, "r");
    fread(jbuffer, 1, 16383, configfile);
    fclose(configfile);

    jsmn_parser parser;
    jsmntok_t tokens[128];
    
    jsmn_init(&parser);
    int jtk = jsmn_parse(&parser, jbuffer, strlen(jbuffer), tokens, 128);
    if (jtk < 0) {
        switch(jtk) {
        case JSMN_ERROR_INVAL:
            fprintf(stderr, "Error, config json corrupted.\n");
            return EINVAL; 
        case JSMN_ERROR_NOMEM:
            fprintf(stderr, "Out of memory while parsing config json.\n");
            return ENOMEM;
        case JSMN_ERROR_PART:
            fprintf(stderr, "Unexpected end of json config data.\n");
            return EIO;
        default:
            fprintf(stderr, "Unknown json parser error.\n");
            return EINVAL;
        }
    }

    jsmntok_t *jtok = tokens;
    return config_parse(sim, &jtok, jbuffer, 0);
}
