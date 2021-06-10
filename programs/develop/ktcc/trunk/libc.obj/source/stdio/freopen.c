#include "stddef.h"
#include "sys/ksys.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CREATE_FILE()   if(_ksys_file_create(_name)){ \
                            errno= EIO; \
                            free(out); \
                            out = NULL; \
                        }

FILE *freopen(const char *restrict _name, const char *restrict _mode, FILE *restrict out) {
    if(!_name || !_mode || !out){
        errno = EINVAL;
        return NULL;
    }

    if (strchr(_mode, 'r')) { out->mode = _FILEMODE_R; }
    if (strchr(_mode, 'a')) { out->mode = _FILEMODE_A; }
    if (strchr(_mode, 'w')) { out->mode = _FILEMODE_W; }

    ksys_bdfe_t info;
    int no_file = _ksys_file_get_info(_name, &info);
    out->eof=0;
    out->error=0;
    out->position=0;
    out->name = strdup(_name);
    
    switch (out->mode) {
    case _FILEMODE_A :
        if(no_file){
            CREATE_FILE();
        }
        out->position = info.size;
        break;
    case _FILEMODE_W :
        CREATE_FILE();
        break;
    case _FILEMODE_R :
        if(no_file){
            free(out);
            out = NULL;
        }
        break;
    default:
        free(out);
        out = NULL;
        break;
    }
    return out;
}