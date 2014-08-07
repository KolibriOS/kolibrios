format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

extrn hSound      

public _SetBufferPos@8

align 4
_SetBufferPos@8:        ;str:dword, offs:dword

           push ebx
           push ecx
           
           xor eax, eax
           lea ebx, [esp+12]   ;[stream]

           push eax            ;.out_size
           push eax            ;.output
           push 8              ;.inp_size
           push ebx            ;.input
           push SND_SETPOS     ;.code
           push dword [hSound] ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx 
           pop ebx
           ret 8
           
