format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable


public _InitSound@0
public _CreateBuffer@8
public _DestroyBuffer@4
public _SetBuffer@16
public _WaveOut@12
public _PlayBuffer@4
public _StopBuffer@4

public _GetMasterVol@4
public _SetMasterVol@4

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
proc _WaveOut@12 stdcall,str:dword, src:dword, size:dword
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
           mov [io_code], SND_OUT
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

align 4
proc _StopBuffer@4 stdcall, str:dword
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
           mov [io_code], SND_STOP
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
proc _GetMasterVol@4 stdcall, pvol:dword
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
           mov eax, [hrdwSound]
           mov ecx, [pvol]
           xor ebx, ebx
           mov [handle], eax
           mov [io_code], DEV_GET_MASTERVOL
           mov [input], ebx
           mov [inp_size], ebx
           mov [output], ecx
           mov [out_size], 4

           mov eax, 68
           mov ebx, 17
           lea ecx, [handle]
           int 0x40
           pop ecx 
           pop ebx
           ret
endp

align 4
proc _SetMasterVol@4 stdcall,vol:dword
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
           mov eax, [hrdwSound]
           lea ebx, [vol]
           xor ecx, ecx

           mov [handle], eax
           mov [io_code], DEV_SET_MASTERVOL
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

if 0
align 4
proc _GetDevInfo@8 stdcall, hSrv:dword, p_info:dword
           locals
             handle     dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
           endl
           
           push ebx
           mov eax, [hSrv]
           xor ebx, ebx
           mov ecx, [p_info]

           mov [handle], eax
           mov [io_code], DEV_GET_INFO
           mov [input], ebx
           mov [inp_size], ebx
           mov [output], ecx
           mov [out_size], CTRL_INFO_SIZE

           lea eax, [handle]
           stdcall CallServiceEx, eax
           pop ebx
               ret
endp

end if

section '.data' align 16 data readable writable

szInfinity  db 'INFINITY',0
szSound     db 'SOUND',0

align 4
hSound      dd ?
hrdwSound   dd ? 