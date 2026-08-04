#include <stdio.h>
#include <shell_api.h>
#include <conio.h>
#include "_conio.h"
#include "mutex.h"

AUTO_INIT_MUTEX(__conio_mutex);


char* console_gets(char* buff, size_t len)
{
    __libc_mutex_lock(&__conio_mutex);

    char* ret = buff;

    if (__shell_is_init < __SHELL_INIT_FAILED) {
        shell_gets(buff, len);
    }
    if (__shell_is_init == __SHELL_INIT_FAILED) {
        con_init();
        ret = con_gets(buff, len);
    }

    __libc_mutex_unlock(&__conio_mutex);

    return ret;
}

void console_write(const char* ptr, size_t len)
{
    __libc_mutex_lock(&__conio_mutex);

    if (__shell_is_init < __SHELL_INIT_FAILED) {
        shell_write_string(ptr, len);
    }
    if (__shell_is_init == __SHELL_INIT_FAILED) {
        con_init();
        con_write_string((char*)ptr, len);
    }

    __libc_mutex_unlock(&__conio_mutex);
}

void console_exit()
{
    __libc_mutex_lock(&__conio_mutex);

    if (__shell_is_init < __SHELL_INIT_FAILED) {
        shell_exit();
    }
    if (__shell_is_init == __SHELL_INIT_FAILED) {

        if (__con_is_load) {
            con_exit(0);
        }
    }

    __libc_mutex_unlock(&__conio_mutex);
}
