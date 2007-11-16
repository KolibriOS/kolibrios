
align 4

proc load_PE stdcall, file_name:dword
           locals
             image  dd ?
           endl

           stdcall load_file, [file_name]
           test eax, eax
           jz .fail

           mov [image], eax

           mov edx, [eax+60]

           stdcall kernel_alloc, [eax+80+edx]
           test eax, eax
           jz .cleanup

           stdcall map_PE, eax, [image]
           ret
.cleanup:
           stdcall kernel_free,[image]
.fail:
           xor eax, eax
           ret
endp


align 4
map_PE:                    ;stdcall base:dword, image:dword
           cld
           push    ebp
           push    edi
           push    esi
           push    ebx
           sub     esp, 44

           mov     ebp, [esp+68]
           mov     ebx, [esp+64]
           mov     edx, ebp
           mov     esi, ebp
           add     edx, [ebp+60]
           mov     edi, ebx
           mov     [esp+32], edx

           mov     ecx, [edx+84]
           shr     ecx, 2
           rep     movsd

           movzx   eax, word [edx+6]
           mov     dword [esp+36], 0
           mov     [esp+28], eax
           jmp     .L6
.L7:
           mov     eax, [edx+264]
           test    eax, eax
           je      .L8

           mov     esi, ebp
           mov     edi, ebx
           add     esi, [edx+268]
           mov     ecx, eax
           add     edi, [edx+260]

           shr     ecx, 2
           rep     movsd
.L8:
           mov     ecx, [edx+256]
           add     ecx, 4095
           and     ecx, -4096
           cmp     ecx, eax
           jbe     .L10

           sub     ecx, eax
           add     eax, [edx+260]
           lea     edi, [eax+ebx]

           xor     eax, eax
           rep     stosb
.L10:
           inc     dword [esp+36]
           add     edx, 40
.L6:
           mov     esi, [esp+28]
           cmp     [esp+36], esi
           jne     .L7

           mov     edi, [esp+32]
           cmp     dword [edi+164], 0
           je      .L13

           mov     eax, [esp+32]
           mov     edi, ebx
           mov     ecx, ebx
           sub     edi, [eax+52]
           add     ecx, [eax+160]

           mov     edx, edi
           shr     edx, 16
           mov     [esp+20], edx
           jmp     .L15
.L16:
           lea     esi, [eax-8]
           xor     ebp, ebp
           shr     esi, 1
           jmp     .L17
.L18:
           movzx   eax, word [ecx+8+ebp*2]
           mov     edx, eax
           shr     eax, 12
           and     edx, 4095
           add     edx, [ecx]
           cmp     ax, 2
           je      .L21

           cmp     ax, 3
           je      .L22

           dec     ax
           jne     .L19

           mov     eax, [esp+20]
           add     [edx+ebx], ax
.L21:
           add     [edx+ebx], di
.L22:
           add     [edx+ebx], edi
.L19:
           inc     ebp
.L17:
           cmp     ebp, esi
           jne     .L18

           add     ecx, [ecx+4]
.L15:
           mov     eax, [ecx+4]
           test    eax, eax
           jne     .L16
.L13:
           mov     edx, [esp+32]
           cmp     dword [edx+132], 0
           je      .L24

           mov     eax, ebx
           add     eax, [edx+128]
           lea     esi, [eax+20]
.L26:
           cmp     dword [esi-16], 0
           jne     .L27

           cmp     dword [esi-8], 0
           je      .L24
.L27:
           mov     ecx, [esi-20]
           mov     ebp, ebx
           add     ebp,  [esi-4]
           add     ecx, ebx
           mov     [esp+40], ecx
.L29:
           mov     edi, [esp+40]
           mov     eax, [edi]
           test    eax, eax
           je      .L30

           test    eax, eax
           js      .L30

           lea     eax, [eax+2+ebx]
           mov     edi, kernel_export
           mov     [ebp], dword -1
           mov     [esp+24], eax
.L33:
           push    ecx
           push    16
           push    dword [edi]
           push    dword [esp+36]
           call    strncmp
           pop     edx
           test    eax, eax
           jne     .L34
           mov     eax, [edi+4]
           mov     [ebp], eax
           jmp     .L36
.L34:
           add     edi, 8
           cmp     dword [edi], 0
           jne     .L33
.L36:
           add     dword [esp+40], 4
           add     ebp, 4
           jmp     .L29
.L30:
           add     esi, 20
           jmp     .L26
.L24:
           mov     eax, [esp+32]
           add     ebx, [eax+40]
           add     esp, 44
           mov     eax, ebx
           pop     ebx
           pop     esi
           pop     edi
           pop     ebp
           ret     8

