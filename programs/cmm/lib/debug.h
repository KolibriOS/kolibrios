#ifndef INCLUDE_DEBUG_H
#define INCLUDE_DEBUG_H

#ifndef INCLUDE_STRING_H
#include "../lib/strings.h"
#endif

inline fastcall void debugch( ECX)
{
	$push eax
	$push ebx
	$mov eax,63
	$mov ebx,1
	$int 0x40
	$pop ebx
	$pop eax
}

inline fastcall void debug( EDX)
{
	$push eax
	$push ebx
	$push ecx
	$mov eax, 63
	$mov ebx, 1
NEXT_CHAR:
	$mov ecx, DSDWORD[edx]
	$or	 cl, cl
	$jz  DONE
	$int 0x40
	$inc edx
	$jmp NEXT_CHAR
DONE:
	$pop ecx
	$pop ebx
	$pop eax
}

inline fastcall void debugln( EDX)
{
	debug( EDX);
	debugch(10);
}

inline fastcall void debugcls()
{
	char i;
	for (i=0;i<70;i++) debugch(10);
}

:void debugval(dword text,number)
{
	char tmpch[12];
	debug(text);
	debug(": ");
	itoa_(#tmpch, number);
	debugln(#tmpch);
}

:void debug_n(dword _text, _size)
{
	dword res_text = malloc(_size);
	strncpy(res_text, _text, _size-1);
	debugln(res_text);
	free(res_text);
}

#endif