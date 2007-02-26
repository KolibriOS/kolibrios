format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

extrn hSound      

public _GetFormat@8

align 4
_GetFormat@8:       ;str:dword, p_fmt:dword
           push ebx
           push ecx
           
           lea eax, [esp+16]   ;p_fmt
           lea ebx, [esp+12]   ;[stream]

           push 4            ;.out_size
           push eax            ;.output
           push 4              ;.inp_size
           push ebx            ;.input
           push SND_GETFORMAT  ;.code
           push dword [hSound] ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx 
           pop ebx
           ret 8
           
