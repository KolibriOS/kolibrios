#include <sys/ksys.h>
#include <sys/dir.h>
#include <stdlib.h>

char *getcwd(char *buf, unsigned size){
    if(!buf){
       if((buf = malloc(size))==NULL){
           return NULL;
       }
    }
    _ksys_getcwd(buf, size);
    return(buf);
}

void setcwd(const char* cwd){
    _ksys_setcwd((char*)cwd);
}

int rmdir(const char* dir){
    return _ksys_file_delete(dir);
}

int mkdir(const char* dir){
    return _ksys_mkdir(dir);
}