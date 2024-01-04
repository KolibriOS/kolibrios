/*
 * Copyright (C) KolibriOS team 2004-2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <sys/ksys.h>

int chdir(char* dir){
    _ksys_setcwd(dir);
    return 0;
}
