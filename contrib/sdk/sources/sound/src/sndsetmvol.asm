format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _SetMasterVol@4

extrn hrdwSound   

align 4
_SetMasterVol@4:         ;vol:dword
           
           push ebx
           push ecx

           xor eax, eax
           lea ebx, [esp+12]   ;[stream]

           push eax            ;.out_size
           push eax            ;.output
           push 4              ;.inp_size
           push ebx            ;.input
           push DEV_SET_MASTERVOL ;.code
           push dword [hrdwSound] ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx 
           pop ebx
           ret 4

