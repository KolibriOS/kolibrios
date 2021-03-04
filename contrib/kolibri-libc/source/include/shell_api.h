#ifndef _SHELL_API_H_
#define _SHELL_API_H_

#include <stddef.h>

extern void _FUNC(shell_printf)(const char* format, ...); 
extern void _FUNC(shell_puts)(const char *s);
extern void _FUNC(shell_putc)(char c);
extern char _FUNC(shell_getc)();
extern void _FUNC(shell_gets)(char *str);
extern void _FUNC(shell_cls)();
extern void _FUNC(shell_exit)();
extern unsigned _FUNC(shell_get_pid)();
extern int  _FUNC(shell_ping)();
#endif