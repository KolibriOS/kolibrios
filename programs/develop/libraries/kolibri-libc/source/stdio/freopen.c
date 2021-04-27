#include "sys/ksys.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *freopen(const char *restrict _name, const char *restrict _mode, FILE *restrict out) {
    static ksys_bdfe_t info;
    info.size=0;
    if (!out) {
        errno = ENOMEM;
        return NULL;
    }

    _ksys_file_get_info(_name, &info);

    out->name = strdup(_name);
    out->position = 0;
    out->error = 0;
    out->eof = 0;
    out->kind = 0;
    out->orientation = 0;
    out->mode = 0;
    out->start_size = info.size;

    if (strchr(_mode, 'b')) { out->mode |= _STDIO_F_B; }
    if (strchr(_mode, 'x')) { out->mode |= _STDIO_F_X; }
    if (strchr(_mode, 'a')) { out->mode |= _STDIO_F_A; }
    if (strchr(_mode, 'r')) { out->mode |= _STDIO_F_R; }
    if (strchr(_mode, 'w')) { out->mode |= _STDIO_F_W; }
    if (strchr(_mode, '+')) { out->mode |= _STDIO_F_R | _STDIO_F_W; }

    if (out->mode & _STDIO_F_A) {
        out->position = info.size;
        out->append_offset = info.size;
    } else if(out->mode & _STDIO_F_W){
        if(_ksys_file_create(_name)){
            return NULL;
        }
    }
    return out;
}
