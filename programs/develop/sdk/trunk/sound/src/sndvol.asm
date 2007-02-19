format MS COFF

include "snd.inc"
include "proc32.inc"

section '.text' align 16 code readable executable

public _GetMasterVol@4
public _SetMasterVol@4

extrn hrdwSound   

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