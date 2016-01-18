#include <_ansi.h>
#include <sys/unistd.h>
#include "io.h"

void load_libconsole();
void     __stdcall con_init(unsigned w_w, unsigned w_h, unsigned s_w, unsigned s_h, const char* t);
void     __stdcall con_exit(char bCloseWindow);
unsigned __stdcall con_get_flags(void);
unsigned __stdcall con_set_flags(unsigned new_flags);
void     __stdcall con_cls(void);
void     __stdcall con_write_string(const char* string, unsigned length);

int __gui_mode;

static int console_write(const char *path, const void *buff,
                 size_t offset, size_t count, size_t *writes)
{
    con_write_string(buff, count);

    *writes = count;
    return count;
};

void __init_conio()
{
    __io_handle *ioh;

    load_libconsole();
    con_init(80, 25, 80, 500, "Console application");

    ioh = &__io_tab[STDOUT_FILENO];
    ioh->mode  = _WRITE|_ISTTY;
    ioh->write = &console_write;
};

void __fini_conio()
{
    con_exit(0);
}




