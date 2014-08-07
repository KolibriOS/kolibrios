format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _GetBufferSize@8
public _GetBufferFree@8

extrn hSound

align 4
_GetBufferSize@8:      ;str:dword, p_size:dword
           
           push ebx
           push ecx
           
           lea eax, [esp+16]
           lea ebx, [esp+12]   ;[stream]

           push 4              ;.out_size
           push eax            ;.output
           push 4              ;.inp_size
           push ebx            ;.input
           push SND_GETBUFFSIZE;.code
           push dword [hSound] ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx 
           pop ebx
           ret 8
           
align 4
_GetBufferFree@8:      ;str:dword, p_free:dword
           push ebx
           push ecx
           
           lea ebx, [esp+12]      ;[stream]
           push 0                 ;storage for free_space    
           mov eax, esp

           push 4                 ;.out_size
           push eax               ;.output
           push 4                 ;.inp_size
           push ebx               ;.input
           push SND_GETFREESPACE  ;.code
           push dword [hSound]    ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           mov ecx, [esp+20]      ; p_free  
           pop dword [ecx]
           pop ecx 
           pop ebx
           ret 8
