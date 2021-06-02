/* Copyright (C) 2021 Logaev Maxim (turbocat2001), GPLv2 */

#include "ksys_fs.h"

int rmdir(const char* dir){
    return _ksys_file_delete(dir);
}

int mkdir(const char* dir, unsigned fake_mode){
    return _ksys_mkdir(dir);
}

int chdir(char* dir){
    _ksys_setcwd(dir);
    return 0;
}