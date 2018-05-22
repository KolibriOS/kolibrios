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

inline void debugi(dword d_int)
{
	char tmpch[12];
	itoa_(#tmpch, d_int);
	debugln(#tmpch);
}

:void debugval(dword text,number)
{
	debug(text);
	debug(": ");
	debugi(number);
}

#endif