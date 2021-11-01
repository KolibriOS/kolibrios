#include <stddef.h>
#include <sys/ksys.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dirent.h>


static FILE* _set_errno(FILE *out, int err){
    errno = err;
    if(out){
        free(out->name);
        free(out);
    }
    out = NULL;
    return out;
}

static void _create_file(char *name, FILE *out){
    if(_ksys_file_create(name)){
       _set_errno(out, EIO);
    }
}

FILE *freopen(const char *restrict _name, const char *restrict _mode, FILE *restrict out) {
    if(!_name || !_mode || !out){
        return _set_errno(out, EINVAL);
    }

    ksys_bdfe_t info;
    int no_file = _ksys_file_get_info(_name, &info);
    if(!no_file && info.attributes & IS_FOLDER){
        return _set_errno(out, EISDIR);
    }
    
    out->eof=0;
    out->error=0;
    out->position=0;
    out->name = strdup(_name);
    
    if (strchr(_mode, 'r')) { out->mode = _FILEMODE_R; }
    if (strchr(_mode, 'w')) { out->mode = _FILEMODE_W; }
    if (strchr(_mode, 'a')) { out->mode = _FILEMODE_A; }
    if (strchr(_mode, '+')) { out->mode |= _FILEMODE_PLUS; }

    if(out->mode & _FILEMODE_A){
        if(no_file){
            _create_file(out->name, out);
        }
        out->position = info.size;
    }else if(out->mode & _FILEMODE_W){
        _create_file(out->name, out);
    }else if((out->mode & _FILEMODE_R)){
        if(no_file){
            _set_errno(out, ENOENT);
        }
    }else{
        _set_errno(out, EINVAL);
    }
    return out;
}