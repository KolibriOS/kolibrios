format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _SetTimeBase@12
public _GetTimeStamp@8
public _GetDevTime@4

extrn hSound
extrn hrdwSound

align 4
_SetTimeBase@12:       ;stdcall str:dword, timebase:qword

           push ebx
           push ecx

           xor eax, eax
           lea ebx, [esp+12]    ;[stream]

           push eax             ;.out_size
           push eax             ;.output
           push 12              ;.inp_size
           push ebx             ;.input
           push SND_SETTIMEBASE ;.code
           push dword [hSound]  ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx
           pop ebx
           ret 12

align 4
_GetTimeStamp@8: ; stdcall, str:dword, pstamp:dword

           push ebx
           push ecx

           lea ebx, [esp+12]      ;[stream]
           mov eax, [esp+16]      ;[pstamp]

           push 8                 ;.out_size
           push eax               ;.output
           push 4                 ;.inp_size
           push ebx               ;.input
           push SND_GETTIMESTAMP  ;.code
           push dword [hSound]    ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx
           pop ebx
           ret 8

align 4
_GetDevTime@4: ; pstamp:dword

           push ebx
           push ecx

           xor ebx, ebx
           mov eax, [esp+12]      ;[pstamp]

           push 4                 ;.out_size
           push eax               ;.output
           push ebx               ;.inp_size
           push ebx               ;.input
           push 9                 ;.code
           push dword [hrdwSound] ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24
           pop ecx
           pop ebx
           ret 4


