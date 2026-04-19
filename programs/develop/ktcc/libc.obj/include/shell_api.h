#ifndef _SHELL_API_H_
#define _SHELL_API_H_

#include <sys/ksys.h>

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

extern char __shell_shm_name[32];
extern char* __shell_shm;
extern int __shell_is_init;
extern void __shell_init();

#define __SHELL_WAIT()   \
    while (*__shell_shm) \
    _ksys_delay(5)

extern int shell_ping();
extern unsigned shell_get_pid();
extern void shell_exit();

extern char shell_getc();
extern void shell_gets(char* str, int n);

extern void shell_putc(char c);
extern void shell_puts(const char* str);
extern void shell_printf(const char* format, ...);

extern void shell_cls();
#endif
