
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
map_PE:
           cld
           push    ebp
           push    edi
           push    esi
           push    ebx
           sub     esp, 28

           mov     edx, [esp+52]
           mov     ebx, [esp+48]
           mov     dword [esp+20], 0
           add     edx, [edx+60]
           movzx   eax, word [edx+6]
           mov     [esp+16], edx
           mov      [esp+12], eax
           jmp     .L6
.L7:
           mov     eax, [edx+264]
           mov     ebp, [edx+260]
           mov     esi, [esp+52]
           add     esi, [edx+268]
           mov     ecx, eax
           lea     edi, [ebx+ebp]

           shr ecx, 2
           rep movsd

           mov     ecx, [edx+256]
           cmp     ecx, eax
           jbe     .L8
           sub     ecx, eax

           xor eax, eax
           rep stosb

.L8:
           inc     dword [esp+20]
           add     edx, 40
.L6:
           mov     eax, [esp+12]
           cmp     [esp+20], eax
           jne     .L7

           mov     edx, [esp+16]
           cmp     dword [edx+164], 0
           je      .L11

           mov     edi, ebx
           mov     ecx, ebx
           sub     edi, [edx+52]
           add     ecx, [edx+160]
           mov     eax, edi
           shr     eax, 16
           mov     [esp+4], eax
           jmp     .L13
.L14:
           lea     esi, [eax-8]
           xor     ebp, ebp
           shr     esi,1
           jmp     .L15
.L16:
           movzx   eax, word [ecx+8+ebp*2]
           mov     edx, eax
           shr     eax, 12
           and     edx, 4095
           add     edx, [ecx]
           cmp     ax, 2
           je      .L19

           cmp     ax, 3
           je      .L20

           dec     ax
           jne     .L17

           mov     eax, [esp+4]
           add     word [edx+ebx], ax
.L19:
           add     word [edx+ebx], di
.L20:
           add     [edx+ebx], edi
.L17:
           inc     ebp
.L15:
           cmp     ebp, esi
           jne     .L16
           add     ecx, [ecx+4]
.L13:
           mov     eax, [ecx+4]
           test    eax, eax
           jne     .L14
.L11:
           mov     edx, [esp+16]
           cmp     dword [edx+132], 0
           je      .L22

           mov     eax, ebx
           add     eax, [edx+128]
           lea     esi, [eax+20]
.L24:
           cmp     dword [esi-16], 0
           jne     .L25

           cmp     dword [esi-8], 0
           je      .L22
.L25:
           mov     ecx, [esi-20]
           mov     ebp, ebx
           add     ebp, [esi-4]
           add     ecx, ebx
           mov     [esp+24], ecx
.L27:
           mov     edx, [esp+24]
           mov     eax, [edx]
           test    eax, eax
           je      .L28

           test    eax, eax
           js      .L28

           lea     eax, [eax+2+ebx]
           mov     edi, kernel_export
           mov     dword [ebp], -1
           mov     [esp+8], eax
.L31:
           push    ecx
           push    16
           push    dword [edi]
           push    dword [esp+20]
           call    strncmp
           pop     edx
           test    eax, eax
           jne     .L32

           mov     eax, [edi+4]
           mov     [ebp], eax
           jmp     .L34
.L32:
           add     edi, 8
           cmp     dword [edi], 0
           jne     .L31
.L34:
           add     dword [esp+24], 4
           add     ebp, 4
           jmp     .L27
.L28:
           add     esi, 20
           jmp     .L24
.L22:

     push eax
     push edx
     mov edx, 0x400   ;bochs
     mov al,0xff      ;bochs
     out dx, al       ;bochs
     pop edx
     pop eax

           mov     ecx, [esp+16]
           add     ebx, [ecx+40]

           add     esp, 28
           mov     eax, ebx

           pop     ebx
           pop     esi
           pop     edi
           pop     ebp

           ret     8
