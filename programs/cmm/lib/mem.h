
dword mem_init()
{
        $push    ebx
        $mov     eax, 68
        $mov     ebx, 11
        $int     0x40
        
        $pop     ebx
        return  EAX;
}

dword malloc(dword size)
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

stdcall dword realloc(dword mptr, size)
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

dword free(dword mptr)
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

#define mem_Alloc malloc
#define mem_ReAlloc realloc
#define mem_Free free
#define mem_Init mem_init

/*:void fastcall memsetz( EDI, ECX)
{
  asm {
    XOR EAX, EAX
    MOV EDX, ECX
    SHR ECX, 2
    REP STOSD
    MOV ECX, EDX
    AND ECX, 3
    REP STOSB
  }
}

:void fastcall memset( EDI, ECX, AL) //copy AL to EDI of ECX num (void *dest, size_t, char c )
{
  asm {
    MOV AH, AL
    MOVZX EDX, AX
    SHL EAX, 16
    OR EAX, EDX
    MOV EDX, ECX
    SHR ECX, 2
    REP STOSD
    MOV ECX, EDX
    AND ECX, 3
    REP STOSB
  }
}

:void fastcall memcpy( EDI, ESI, ECX)
{
  asm {
    MOV EDX, ECX
    SHR ECX, 2
    REP MOVSD
    MOV ECX, EDX
    AND ECX, 3
    REP MOVSB
  }
}

:void fastcall memsetd( EDI, ECX, EAX)
{
  asm {
    REP STOSD
  }
}

:void fastcall memcpyd( EDI, ESI, ECX)
{
  asm {
    REP MOVSD
  }
}

:void fastcall memmov( EDI, ESI, ECX)
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

:long fastcall memcmp( ESI, EDI, ECX)
{
  asm {
    MOV EAX, ECX
    SHR ECX, 2
    REPE CMPSD
    MOV ECX, EAX
    AND ECX, 3
    REPE CMPSB
    XOR EAX, EAX
    XOR ECX, ECX
    MOV AL, DSBYTE[ ESI-1]
    MOV CL, DSBYTE[ EDI-1]
    SUB EAX, ECX
  }
}*/


/*#define memzset memsetz
#define memset0 memsetz
#define mem0set memsetz
#define memset32 memsetd
#define memcpy32 memcpyd*/
