
struc APP_HEADER_02
{ .banner      dq ?
  .version     dd ?    ;+8
  .start       dd ?    ;+12
  .i_end       dd ?    ;+16
  .mem_size    dd ?    ;+20
  .stack_top   dd ?    ;+24
  .cmdline     dd ?    ;+28
  .path        dd ?    ;+32
}

virtual at 0
  app_hdr APP_HEADER_02
end virtual

format MS COFF

public EXPORTS

section '.flat' code readable align 16

EXPORTS:
        dd szStart,    START
        dd szVersion,  0x00010001
        dd szExec,     exec
        dd 0

check   dd 0

szStart    db 'START',0
szVersion  db 'version',0
szExec     db 'exec',0

START:
        xor eax, eax
        cmp [app_hdr.path], 0
        je  .ret
        not eax
.ret:
        mov [check], eax
        ret 4

align 4
exec:
        cmp [check], 0
        lea ebp, [esp+4]
        je  .fail

        mov eax, [ebp+8]
        test eax, eax
        jz .fail

        mov ecx, [ebp]
        mov edx, [ebp+4]
        call validate_pe
        test eax, eax
        jz .fail

        mov eax, 68
        mov ebx, [ebp]
        mov ecx, [ebx+60]
        mov ecx, [ecx+96+ebx]       ; app stack size
        add ecx, 4095
        and ecx, -4096
        mov ebx, 12

        int 0x40
        test eax, eax
        jz .fail

        add ecx, eax
        mov [fs:4], eax         ;stack base
        mov [fs:8], ecx         ;stack top

        mov esp, ecx

        sub esp, 1024
        mov eax, 9
        mov ebx, esp
        mov ecx, -1
        int 0x40
        mov eax, [ebx+30]
        mov [fs:0], eax         ; save pid
        add esp, 1024

        mov ecx, my_libc
        call create_image
        test eax, eax
        jz .fail

        mov  ebx, [eax+60]
        mov  ebx, [ebx+40+eax]
        add  ebx, eax
        push ebp
        push EXPORTS
        push eax
        call ebx

        ret

.fail:
        ret 4

align 4

validate_pe:
        test    ecx, ecx
        je      .L2
        cmp     edx, 63
        jbe     .L2
        cmp     [ecx], word 23117
        je      .L10
.L2:
        xor     eax, eax
        ret

align 4
.L10:
        mov     eax, [ecx+60]
        test    eax, eax
        je      .L2
        add     ecx, eax
        jb      .L2
        cmp     [ecx], dword 17744
        jne     .L2
        cmp     [ecx+4], word 332
        jne     .L2
        test    [ecx+23], byte 32
        jne     .L2
        cmp     [ecx+24], word 267
        jne     .L2
        mov     eax, [ecx+56]
        cmp     eax, 4095
        ja      .L3
        cmp     eax, [ecx+60]
        jne     .L2
        test    eax, eax
        je      .L2
.L5:
        lea     edx, [eax-1]
        test    edx, eax
        jne     .L2
        mov     eax, [ecx+60]
        test    eax, eax
        je      .L2
        lea     edx, [eax-1]
        test    edx, eax
        jne     .L2
        xor     eax, eax
        cmp     [ecx+6], word 96
        setbe   al
        ret
.L3:
        cmp     eax, [ecx+60]
        jae     .L5
        jmp     .L2

align 4
create_image:
        push    ebp
        push    edi
        push    esi
        push    ebx
        sub     esp, 20
        mov     [esp+16], ecx
        mov     eax, [ecx+60]
        add     eax, ecx
        mov     [esp], eax
        mov     ecx, [eax+80]
        mov     ebx, 12
        mov     eax, 68
        int     0x40
        test    eax, eax
        je      .L16

        mov     edx, [esp]
        mov     ecx, [edx+84]
        mov     esi, [esp+16]
        mov     edi, eax
        shr     ecx, 2
        rep     movsd
        mov     cx, [edx+6]
        test    cx, cx
        je      .L17
        add     edx, 248
        movzx   ecx, cx
        lea     ebp, [ecx-1]
        xor     bl, bl
        jmp     .L19

align 4
.L31:
        add     edx, 40
        inc     ebx
.L19:
        mov     ecx, [edx+16]
        test    ecx, ecx
        je      .L18
        mov     esi, [edx+20]
        test    esi, esi
        je      .L18
        add     esi, [esp+16]
        mov     edi, [edx+12]
        add     edi, eax

        shr     ecx, 2
        rep     movsd

.L18:
        cmp     ebx, ebp
        jne     .L31
.L17:
        mov     edx, [esp]
        mov     ecx, [edx+164]
        test    ecx, ecx
        je      .L16

        mov     ebp, eax
        sub     ebp, [edx+52]
        mov     ebx, [edx+160]
        add     ebx, eax
        mov     esi, [ebx+4]
        test    esi, esi
        je      .L16

        mov     edi, ebp
        shr     edi, 16
        mov     [esp], di

align 4
.L26:
        lea     edi, [esi-8]
        shr     edi, 1
        je      .L20
        xor     ecx, ecx
        jmp     .L25

align 4
.L32:
        cmp     si, 3
        je      .L24
        dec     si
        jne     .L21
        mov     esi, [esp]
        add     [eax+edx], si
.L21:
        inc     ecx
        cmp     ecx, edi
        je      .L20
.L25:
        mov     si, [ebx+8+ecx*2]
        mov     edx, esi
        and     edx, 4095
        add     edx, [ebx]
        shr     si, 12
        cmp     si, 2
        jne     .L32
        add     [eax+edx], bp
        inc     ecx
        cmp     ecx, edi
        jne     .L25
.L20:
        add     ebx, [ebx+4]
        mov     esi, [ebx+4]
        test    esi, esi
        jne     .L26
.L16:
        add     esp, 20
        pop     ebx
        pop     esi
        pop     edi
        pop     ebp
        ret

align 4
.L24:
        add     [eax+edx], ebp
        jmp     .L21


align 16
my_libc:
        file '../libc.dll'
