format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

extrn hSound      

public _SetBuffer@16
public _PlayBuffer@4

align 4
proc _SetBuffer@16 stdcall,str:dword, src:dword, offs:dword, size:dword
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
           mov [io_code], SND_SETBUFF
           mov [input], ebx
           mov [inp_size], 16
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
proc _PlayBuffer@4 stdcall, str:dword
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
           mov [io_code], SND_PLAY
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
