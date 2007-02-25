format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _SetVolume@12

extrn hSound

align 4
proc _SetVolume@12 stdcall, str:dword, lvol:dword,rvol:dword
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
           mov [io_code], SND_SETVOLUME
           mov [input], ebx
           mov [inp_size], 12
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

