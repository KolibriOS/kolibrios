format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _InitSound@4
public _CreateBuffer@12
public _DestroyBuffer@4

align 4
_InitSound@4:       ;p_ver:dword

           push ebx
           push ecx

           mov eax, 68
           mov ebx, 16
           mov ecx, szInfinity
           int 0x40
           mov [hSound], eax
           test eax, eax
           jz .fail

           mov eax, 68
           mov ebx, 16
           mov ecx, szSound
           int 0x40
           mov [hrdwSound], eax

           lea eax, [esp+12]   ;p_ver
           xor ebx, ebx

           push 4              ;.out_size
           push eax            ;.output
           push ebx            ;.inp_size
           push ebx            ;.input
           push SRV_GETVERSION ;.code
           push [hSound]       ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;[handle]
           int 0x40
           add esp, 24
           pop ecx  
           pop ebx
           ret 4
.fail:
           or eax, -1 
           pop ecx  
           pop ebx
           ret 4


align 4
_CreateBuffer@12:   ;format:dword,size:dword,p_str:dword

           push ebx
           push ecx
           lea eax, [esp+20]   ;p_str
           lea ebx, [esp+12]   ;format

           push 4              ;.out_size
           push eax            ;.output
           push 8              ;.inp_size
           push ebx            ;.input
           push SND_CREATE_BUFF;.code
           push [hSound]       ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp
           int 0x40
           add esp, 24       ;io_cintrol 
           pop ecx
           pop ebx
           ret 12

align 4
_DestroyBuffer@4:     ;str:dword

           push ebx
           push ecx

           xor eax, eax
           lea ebx, [esp+12]   ;[stream]

           push eax            ;.out_size
           push eax            ;.output
           push 4              ;.inp_size
           push ebx            ;.input
           push SND_DESTROY_BUFF;.code
           push [hSound]       ;.handle

           mov eax, 68
           mov ebx, 17
           mov ecx, esp        ;[handle]
           int 0x40
           add esp, 24
           pop ecx 
           pop ebx
           ret 4


section '.data' align 16 data readable writable

public hSound
public hrdwSound

hSound      dd ?
hrdwSound   dd ? 

szInfinity  db 'INFINITY',0
szSound     db 'SOUND',0


