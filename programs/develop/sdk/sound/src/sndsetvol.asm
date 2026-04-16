format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _SetVolume@12

extrn hSound

align 4
_SetVolume@12:       ;str:dword, lvol:dword,rvol:dword
           
           push ebx
           push ecx
           
           xor eax, eax
           lea ebx, [esp+12]   ;[stream]

           push eax            ;.out_size
           push eax            ;.output
           push 12             ;.inp_size
           push ebx            ;.input
           push SND_SETVOLUME  ;.code
           push dword [hSound] ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx 
           pop ebx
           ret 12

