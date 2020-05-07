#ifndef _STDIO_H
#define _STDIO_H

int putchar(int ch);
void puts(const char *str);
char* gets(char* str);
void putuint(int i);
void putint(int i);
void puthex(uint32_t i);
void print(char *format, va_list args);
void printf(char *text, ... );


#endif