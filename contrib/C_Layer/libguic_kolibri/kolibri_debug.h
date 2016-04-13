#ifndef KOLIBRI_DEBUG_H
#define KOLIBRI_DEBUG_H

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdarg.h>

/* Write a printf() like function (variable argument list) for
   writing to debug board */

inline void debug_board_write_byte(const char ch){
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

void debug_board_printf(const char *format,...)
{
	va_list ap;
	char log_board[300];
	
	va_start (ap, format);
	vsprintf(log_board, format, ap);
	va_end(ap);
	debug_board_write_str(log_board);
}

#endif /* KOLIBRI_DEBUG_H */
