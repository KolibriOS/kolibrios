// -------------------------------------------------------------
// KWINE is a fork of program PELoad written by 0CodErr
// author of fork - rgimad
//-------------------------------------------------------------
#ifndef MSVCRT_DLL_H
#define MSVCRT_DLL_H

#define NULL ((void*)0)

#define cdecl   __attribute__ ((cdecl))
#define stdcall __attribute__ ((stdcall))

//#define cdecl   __cdecl
//#define stdcall __stdcall


static inline void debug_board_write_byte(const char ch){
    __asm__ __volatile__(
    "int $0x40"
    :
    :"a"(63), "b"(1), "c"(ch));
}

static inline void debug_board_write_string(char *str){
	char ch;
	while (ch = *(str++))
	{
	    __asm__ __volatile__(
	    "int $0x40"
	    :
	    :"a"(63), "b"(1), "c"(ch));
	}
}
//

#endif