#ifndef KOLIBRI_DEBUG_H
#define KOLIBRI_DEBUG_H

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdarg.h>

/* Write a printf() like function (variable argument list) for
   writing to debug board */

static inline void debug_board_write_byte(const char ch){
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(63), "b"(1), "c"(ch));
}

//added noninline because incofortabre stepping in in debugger
void __attribute__ ((noinline)) debug_board_write_str(const char* str){
  while(*str)
    debug_board_write_byte(*str++);
}

static inline void debug_board_printf(const char *format,...)
{
	va_list ap;
	char log_board[300];

	va_start (ap, format);
	vsprintf(log_board, format, ap);
	va_end(ap);
	debug_board_write_str(log_board);
}

__attribute__ ((noinline)) void trap(int n)
{
    // nothing todo, just see n in debugger. use "bp trap" command
    __asm__ __volatile__(
    "nop"
    :
    :"a"(n));
}

#endif /* KOLIBRI_DEBUG_H */
