#include <stdio.h>
#include "cfg.h"
#include <string.h>

static int config_parse_cpu(jsmntok_t **jtok, char *jbuffer, int jref)
{
    char buffer[256];
    int res; 
    int children = (*jtok)->size;
    
    fprintf(stdout, "\nCPU:\n");
    (*jtok)++;
    jsmntype_t type = (*jtok)->type;
    if (type != JSMN_OBJECT) {
        fprintf(stderr, "Error, CPU definition must be json object\n");
        return EINVAL;
    }

    for (int i = 0; i < children; i++) {
        (*jtok)++;
        int start = (*jtok)->start;
        int end = (*jtok)->end;
        type = (*jtok)->type;
        snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
        buffer[end-start+1] = '\0';
        fprintf(stdout, "\n%s\n", buffer);

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

        res = config_parse(jtok, jbuffer, jref+1);
        if (res) {
            return res;
        }
    }
    return 0;
}

static int config_parse(jsmntok_t **jtok, char *jbuffer, int jref)
{
    char buffer[256];
    int res;

    int children = (*jtok)->size;
    printf("children: %d, level: %d\n", children, jref);
    for (int i = 0; i < children; i++) {
        (*jtok)++;
        int start = (*jtok)->start;
        int end = (*jtok)->end;
        jsmntype_t type = (*jtok)->type;
        snprintf(buffer, end-start+1, "%s", (char *)(jbuffer + start));
        buffer[end-start+1] = '\0';
        printf("child: %d, %d - %s\n", i, (int) type, buffer);

        switch(type) {
        case JSMN_STRING:
            if (strncmp("cpu", buffer, strlen("cpu")+1) == 0) {
                res = config_parse_cpu(jtok, jbuffer, jref+1);
                if (res) {
                    return res;
                }
                break;
            }
        default:
            res = config_parse(jtok, jbuffer, jref+1);
            if (res) {
                return res;
            }
            break;
        }
    }

    return 0;
}

int read_config_json(char *config_file) 
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
    return config_parse(&jtok, jbuffer, 0);
}
