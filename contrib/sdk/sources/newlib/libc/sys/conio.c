/*
 * Copyright (C) KolibriOS team 2004-2024. All rights reserved.
 * Distributed under terms of the GNU General Public License
*/

#include <_ansi.h>
#include <stdio.h>
#include <sys/unistd.h>
#include "io.h"
#include <string.h>

extern void load_libconsole();
extern void  __stdcall con_init(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
extern void  __stdcall con_exit(char bCloseWindow);
extern void  __stdcall con_write_string(const char* string, unsigned length);
extern char* __stdcall con_gets(char*, unsigned);

int __gui_mode;

static int console_read(const char *path, void *buff,
           size_t offset, size_t count, size_t *done)
{
    char *p = buff;
    con_gets(p, count+1);
    *done = strlen(p);
    return 0;
}

static int console_write(const char *path, const void *buff,
                 size_t offset, size_t count, size_t *writes)
{
    con_write_string(buff, count);

    *writes = count;
    return 0;
}

void __init_conio()
{
    __io_handle *ioh;

    load_libconsole();
    con_init(80, 25, 80, 500, "Console application");

    ioh = &__io_tab[STDIN_FILENO];
    ioh->mode  = _READ|_ISTTY;
    ioh->read  = &console_read;

    ioh = &__io_tab[STDOUT_FILENO];
    ioh->mode  = _WRITE|_ISTTY;
    ioh->write = &console_write;
};

void __fini_conio()
{
    con_exit(0);
}
