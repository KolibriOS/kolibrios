#ifndef INCLUDE_MEM_H
#define INCLUDE_MEM_H

#ifndef INCLUDE_KOLIBRI_H
#include "../lib/kolibri.h"
#endif

inline fastcall void mem_init()
{
	$mov     eax, 68
	$mov     ebx, 11
	$int     0x40
}

:dword malloc(dword size)
{
	$push    ebx
	$push    ecx

	$mov     eax, 68
	$mov     ebx, 12
	$mov     ecx, size
	$int     0x40
	
	$pop     ecx
	$pop     ebx
	return  EAX;
}

:stdcall dword realloc(dword mptr, size)
{
	$push    ebx
	$push    ecx
	$push    edx

	$mov     eax, 68
	$mov     ebx, 20
	$mov     ecx, size
	$mov     edx, mptr
	$int     0x40

	$pop     edx
	$pop     ecx
	$pop     ebx
	return   EAX;
}

:dword free(dword mptr)
{
	$push    eax
	$push    ebx
	$push    ecx
	
	$mov     eax, 68
	$mov     ebx, 13
	$mov     ecx, mptr
	$test    ecx, ecx
	$jz      end0
	$int     0x40
   @end0:
	$pop     ecx
	$pop     ebx
	$pop     eax
	return 0;
}

inline fastcall memmov( EDI, ESI, ECX)
{
  asm {
    MOV EAX, ECX
    CMP EDI, ESI
    JG L1
    JE L2
    SAR ECX, 2
    JS L2
    REP MOVSD
    MOV ECX, EAX
    AND ECX, 3
    REP MOVSB
    JMP SHORT L2
L1: LEA ESI, DSDWORD[ ESI+ECX-4]
    LEA EDI, DSDWORD[ EDI+ECX-4]
    SAR ECX, 2
    JS L2
    STD
    REP MOVSD
    MOV ECX, EAX
    AND ECX, 3
    ADD ESI, 3
    ADD EDI, 3
    REP MOVSB
    CLD
L2:
  }
}

#define SHM_OPEN        0x00
#define SHM_OPEN_ALWAYS 0x04
#define SHM_CREATE      0x08
#define SHM_READ        0x00
#define SHM_WRITE       0x01
inline fastcall dword memopen(ECX, EDX, ESI)
{
	$mov     eax, 68
	$mov     ebx, 22
	// ecx = area name, 31 symbols max
	// edx = area size for SHM_CREATE SHM_OPEN_ALWAYS
	// esi = flags, see the list below:
	$int     0x40
	// eax, edx - please check system documentation
}

inline fastcall dword memclose(ECX)
{
	$mov     eax, 68
	$mov     ebx, 23
	$int     0x40
	// eax destroyed
}

#define mem_Alloc malloc
#define mem_ReAlloc realloc
#define mem_Free free
#define mem_Init mem_init

#endif