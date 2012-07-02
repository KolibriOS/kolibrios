//mem.Init
dword mem_Init()
{
        $push    ebx
        $mov     eax, 68
        $mov     ebx, 11
        $int     0x40
        
        $pop     ebx
        return  EAX;
}

//mem.Alloc
dword mem_Alloc(dword size)
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

//mem.ReAlloc
inline fastcall dword mem_ReAllocR( ECX, EDX)
{
        $mov     eax, 68
        $mov     ebx, 20
        $int     0x40
}

stdcall dword mem_ReAlloc(dword mptr, size)
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

//mem.Free
void mem_Free(dword mptr)
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
}

//stdcall dword ??
inline fastcall mem_Move( EDI, ESI, ECX)
{
  asm {
    mov eax, ecx
    cmp edi, esi
    jg l1
    je l2
    sar ecx, 2
    js l2
    rep movsd
    mov ecx, eax
    and ecx, 3
    rep movsb
    jmp short l2
l1: lea esi, dsdword[ esi+ecx-4]
    lea edi, dsdword[ edi+ecx-4]
    sar ecx, 2
    js l2
    std
    rep movsd
    mov ecx, eax
    and ecx, 3
    add esi, 3
    add edi, 3
    rep movsb
    cld
l2:
  }
}
