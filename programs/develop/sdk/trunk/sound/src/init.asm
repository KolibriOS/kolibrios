format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _InitSound@0
public _CreateBuffer@8
public _DestroyBuffer@4

align 4
proc _InitSound@0
           locals
             handle     dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
           endl

           push ebx
           push ecx
           mov eax, 68
           mov ebx, 16
           mov ecx, szInfinity
           int 0x40
           test eax, eax
           jz .fail
           
           mov [hSound], eax
           mov eax, 68
           mov ebx, 16
           mov ecx, szSound
           int 0x40
           mov [hrdwSound], eax
           
           mov eax, [hSound]
           xor ebx, ebx

           mov [handle], eax
           mov [io_code], SRV_GETVERSION
           mov [input], ebx
           mov [inp_size], ebx
           mov [output], ebx
           mov [out_size], ebx
           
           mov eax, 68
           mov ebx, 17
           lea ecx, [handle]
           int 0x40
.fail:           
           pop ecx  
           pop ebx
           ret
endp


align 4
proc _CreateBuffer@8 stdcall, format:dword, size:dword
           locals
             handle     dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
           endl

           push ebx
           push ecx
           mov eax, [hSound]
           lea ebx, [format]
           xor ecx, ecx

           mov [handle], eax
           mov [io_code], SND_CREATE_BUFF
           mov [input], ebx
           mov [inp_size], 4
           mov [output], ecx
           mov [out_size], ecx
           
           mov eax, 68
           mov ebx, 17
           lea ecx, [handle]
           int 0x40
           pop ecx
           pop ebx
           ret
endp

align 4
proc _DestroyBuffer@4 stdcall, str:dword
           locals
             handle     dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
           endl

           push ebx
           push ecx
           
           mov eax, [hSound]
           lea ebx, [str]
           xor ecx, ecx

           mov [handle], eax
           mov [io_code], SND_DESTROY_BUFF
           mov [input], ebx
           mov [inp_size], 4
           mov [output], ecx
           mov [out_size], ecx

           mov eax, 68
           mov ebx, 17
           lea ecx, [handle]
           int 0x40
           pop ecx 
           pop ebx
           ret
endp

section '.data' align 16 data readable writable

szInfinity  db 'INFINITY',0
szSound     db 'SOUND',0

public hSound
public hrdwSound

align 4
hSound      dd ?
hrdwSound   dd ? 