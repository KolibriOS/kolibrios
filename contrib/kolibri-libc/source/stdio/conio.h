/*

This is adapded thunk for console.obj sys library
Only for internal use in stdio.h

Adapted for tcc by Siemargl, 2016

*/
#ifndef _CONIO_H_
#define _CONIO_H_

#define cdecl   __attribute__ ((cdecl))
#define stdcall __attribute__ ((stdcall))

extern void stdcall (*__con_write_asciiz)(const char* str);
/*	Display ASCIIZ-string to the console at the current position, shifting
the current position. */

extern void stdcall (*__con_write_string)(const char* str, unsigned length);
/* 	Similar to __con_write_asciiz, but length of the string must be given as a 
separate parameter */

extern int stdcall (*__con_getch)(void);
/*	Get one character from the keyboard. 

For normal characters function returns ASCII-code. For extended 
characters (eg, Fx, and arrows), first function call returns 0
and second call returns the extended code (similar to the DOS-function 
input). Starting from version 7, after closing the console window,
this function returns 0. */

extern short stdcall (*__con_getch2)(void);
/*	Reads a character from the keyboard. Low byte contains the ASCII-code 
(0 for extended characters), high byte - advanced code (like in BIOS 
input functions). Starting from version 7, after closing the console
window, this function returns 0. */

extern int stdcall (*__con_kbhit)(void);
/*	Returns 1 if a key was pressed, 0 otherwise. To read pressed keys use
__con_getch and __con_getch2. Starting from version 6, after closing 
the console window, this function returns 1. */

extern char* stdcall (*__con_gets)(char* str, int n);
/*	Reads a string from the keyboard. Reading is interrupted when got
"new line" character, or after reading the (n-1) characters (depending on 
what comes first). In the first case the newline is also recorded in the
str. The acquired line is complemented by a null character.
Starting from version 6, the function returns a pointer to the entered
line if reading was successful, and NULL if the console window was closed. */

typedef int (stdcall * __con_gets2_callback)(int keycode, char** pstr, int* pn, 
	int* ppos);

extern char* stdcall (*__con_gets2)(__con_gets2_callback callback, char* str, int n);
/*	Con_gets completely analogous, except that when the user
press unrecognized key, it calls the specified callback-procedure
(which may, for example, handle up / down for history and tab to enter
autocompletion). You should pass to the procedure: key code and three pointers 
- to the string, to the maximum length and to the current position. 
function may change the contents of string and may change the string 
itself (for example, to reallocate memory for increase the limit), 
maximum length, and position of the line - pointers are passed for it.
Return value: 0 = line wasn't changed 1 = line changed, you should
remove old string and display new, 2 = line changed, it is necessary 
to display it; 3 = immediately exit the function.
Starting from version 6, the function returns a pointer to the entered
line with the successful reading, and NULL if the console window was closed. */

extern int __con_is_load; 
extern unsigned *__con_dll_ver;

extern int __con_init(void);
extern int __con_init_opt(int wnd_width, int wnd_height, int scr_width, int scr_height, const char* title);
extern void stdcall (*__con_exit)(int status);

#endif
