#ifndef _SHELL_API_H_
#define _SHELL_API_H_

#include <sys/ksys.h>

#ifdef _BUILD_LIBC
#define __EXTERN
#else
#define __EXTERN extern
#endif

#define SHELL_OK   0
#define SHELL_EXIT 1
#define SHELL_PUTC 2
#define SHELL_PUTS 3
#define SHELL_GETC 4
#define SHELL_GETS 5
#define SHELL_CLS  6
#define SHELL_PID  7
#define SHELL_PING 8

#define SHELL_SHM_MAX 1024 * 16

enum __SHELL_INIT_STATE {
    __SHELL_NOT_LOADED = 0, // not try init shell before
    __SHELL_LOADING = 1,    // in progress
    __SHELL_INIT_OK = 2,    // ok
    __SHELL_INIT_FAILED = 3 // fail init shell
};

__EXTERN char __shell_shm_name[32];
__EXTERN char* __shell_shm;
__EXTERN enum __SHELL_INIT_STATE __shell_is_init;
__EXTERN void __shell_init();

#define __SHELL_WAIT()        \
    do {                      \
        _ksys_thread_yield(); \
    } while (*__shell_shm);

__EXTERN int shell_ping();
__EXTERN unsigned shell_get_pid();
__EXTERN void shell_exit();

__EXTERN char shell_getc();
__EXTERN void shell_gets(char* str, int n);

__EXTERN void shell_putc(char c);
__EXTERN void shell_puts(const char* str);
__EXTERN void shell_printf(const char* format, ...);

__EXTERN void shell_write_string(const char* s, size_t len);

__EXTERN void shell_cls();
#endif
