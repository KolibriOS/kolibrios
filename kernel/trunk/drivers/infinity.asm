;
;   This file is part of the Infinity sound library.
;   (C) copyright Serge 2006
;   email: infinity_sound@mail.ru
;
;   This program is free software; you can redistribute it and/or modify
;   it under the terms of the GNU General Public License as published by
;   the Free Software Foundation; either version 2 of the License, or
;   (at your option) any later version.
;
;   This program is distributed in the hope that it will be useful,
;   but WITHOUT ANY WARRANTY; without even the implied warranty of
;   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;   GNU General Public License for more details.

format MS COFF

include 'proc32.inc'
include 'main.inc'

DEBUG		    equ 1

EVENT_NOTIFY	    equ 0x00000200

OS_BASE               equ 0;  0x80400000
new_app_base          equ 0x60400000;   0x01000000
PROC_BASE             equ OS_BASE+0x0080000

public START
public service_proc
public version

extrn AttachIntHandler
extrn SysMsgBoardStr
extrn PciApi
extrn PciRead32
extrn PciRead8
extrn PciWrite8
extrn AllocKernelSpace
extrn MapPage
extrn RegService
extrn KernelAlloc
extrn KernelFree
extrn GetPgAddr
extrn GetCurrentTask
extrn GetService
extrn ServiceHandler
extrn FpuSave
extrn FpuRestore
extrn SendEvent

SND_CREATE_BUFF     equ 2
SND_PLAY            equ 3
SND_STOP            equ 4
SND_SETBUFF         equ 5
SND_DESTROY_BUFF    equ 6

DEV_PLAY            equ 1
DEV_STOP            equ 2
DEV_CALLBACK        equ 3

struc IOCTL
{  .handle           dd ?
   .io_code          dd ?
   .input            dd ?
   .inp_size         dd ?
   .output           dd ?
   .out_size         dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

section '.flat' code readable align 16

proc START stdcall, state:dword

           cmp [state], 1
           jne .exit

           stdcall GetService, szSound
           test eax, eax
           jz .fail
           mov [hSound], eax

           stdcall KernelAlloc, 16*512
           test eax, eax
           jz .out_of_mem
           mov [mix_buff], eax

           mov edi, stream_list
           mov ecx, 17
           xor eax, eax
           cld
           rep stosd

           mov edi, stream
           mov ecx, 4*STREAM_SIZE
           rep stosd
           mov [stream_count],0

           stdcall set_handler, [hSound], new_mix
           stdcall RegService, szInfinity, service_proc
           ret
.fail:
     if DEBUG
           mov esi, msgFail
           call SysMsgBoardStr
     end if
.exit:
           xor eax, eax
           ret

.out_of_mem:
     if DEBUG
           mov esi, msgMem
           call SysMsgBoardStr
     end if
           xor eax, eax
           ret
endp

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

align 4
proc service_proc stdcall, ioctl:dword

           mov edi, [ioctl]
           mov eax, [edi+io_code]

           cmp eax, SND_CREATE_BUFF
           jne @F
           mov ebx, [edi+input]
           stdcall CreateBuffer,[ebx]
           ret
@@:
           cmp eax, SND_PLAY
           jne @F

           mov ebx, [edi+input]
           stdcall play_buffer, [ebx]
           ret
@@:
           cmp eax, SND_STOP
           jne @F

;       if DEBUG
;	   mov esi, msgStop
;	   call   [SysMsgBoardStr]
;       end if

           mov ebx, [edi+input]
           stdcall stop_buffer, [ebx]
           ret
@@:
           cmp eax, SND_SETBUFF
           jne @F

           mov ebx, [edi+input]
           mov eax, [ebx+4]
           add eax, new_app_base
           stdcall set_buffer, [ebx],eax,[ebx+8],[ebx+12]
           ret
@@:
           cmp eax, SND_DESTROY_BUFF
           jne @F

           mov ebx, [edi+input]
           stdcall DestroyBuffer, [ebx]
           ret
@@:
           xor eax, eax
           ret
endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size

TASK_COUNT    equ 0x0003004
CURRENT_TASK  equ 0x0003000

align 8
proc CreateBuffer stdcall, format:dword
           locals
             str dd ?
           endl

           call alloc_stream
           and eax, eax
           jz .fail
           mov [str], eax
           mov edi, eax

           mov edx, [stream_count]
           mov [stream_list+edx*4], eax
           inc [stream_count]

           mov [edi+STREAM.magic], 'WAVE'
           mov [edi+STREAM.size], STREAM_SIZE

           stdcall KernelAlloc, 180*1024

           mov edi, [str]
           mov [edi+STREAM.base], eax
           mov [edi+STREAM.curr_seg], eax
           mov [edi+STREAM.notify_off1], eax
           add eax, 0x8000
           mov [edi+STREAM.notify_off2], eax
           add eax, 0x7FFF
           mov [edi+STREAM.limit], eax

           inc eax

           mov [edi+STREAM.work_buff], eax
           mov [edi+STREAM.work_read], eax
           mov [edi+STREAM.work_write], eax
           mov [edi+STREAM.work_count], 0
           add eax, 0x10000
           mov [edi+STREAM.work_top], eax
           add eax, 1024*32
           mov [edi+STREAM.r_buff], eax

           mov ebx, [CURRENT_TASK]
           shl ebx, 5
           mov eax, [0x3000+ebx+4]

           mov [edi+STREAM.notify_task], eax

           mov eax, [format]
           mov [edi+STREAM.format], eax
           mov [edi+STREAM.flags], SND_STOP

           xor ebx, ebx
           cmp eax, 19
           jb @f
           mov ebx, 0x80808080
@@:
           mov [edi+STREAM.r_silence], ebx

           shl eax, 4
           mov ebx, [resampler_params+eax]
           mov ecx, [resampler_params+eax+4]
           mov edx, [resampler_params+eax+8]

           mov [edi+STREAM.r_size],ebx
           mov [edi+STREAM.r_end], ecx
           mov [edi+STREAM.r_dt],  edx

           mov ebx, [resampler_params+eax+12]
           mov [edi+STREAM.resample], ebx

           mov edi, [edi+STREAM.base]
           mov ecx, 180*1024/4
           xor eax, eax
           rep stosd

           mov eax, [str]
           ret

.fail:
           xor eax, eax
           ret
endp

align 4
pid_to_slot:

           push   ebx
           push   ecx
           mov    ebx,[TASK_COUNT]
           shl    ebx,5
           mov    ecx,2*32
.loop:
           cmp    byte [CURRENT_TASK+ecx+0xa],9
           jz     .endloop              ;skip empty slots
           cmp    [CURRENT_TASK+ecx+0x4],eax ;check PID
           jz     .pid_found
.endloop:
           add    ecx,32
           cmp    ecx,ebx
           jle    .loop
           pop    ecx
           pop    ebx
           xor    eax,eax
           ret

.pid_found:
           shr    ecx,5
           mov    eax,ecx
           pop    ecx
           pop    ebx
           ret

align 4
proc DestroyBuffer stdcall, str:dword

           mov esi, [str]

           cmp [esi+STREAM.magic], 'WAVE'
           jne .fail

           cmp [esi+STREAM.size], STREAM_SIZE
           jne .fail

           stdcall KernelFree, [esi+STREAM.base]

           mov eax, [str]
           call free_stream

           mov edi, [str]
           mov ecx, STREAM_SIZE/4
           xor eax, eax
           cld
           rep stosd

           mov eax, [str]
           mov esi, stream_list
           mov ecx, 16
@@:
           cmp [esi], eax
           je .remove
           add esi, 4
           dec ecx
           jnz @B
           xor eax, eax
           inc eax
           ret
.remove:
           mov edi, esi
           add esi, 4
           cld
           rep movsd
           dec [stream_count]
           xor eax, eax
           inc eax
           ret
.fail:
           xor eax, eax
           ret
endp

align 4
proc play_buffer stdcall, str:dword

           mov ebx, [str]

           cmp [ebx+STREAM.magic], 'WAVE'
           jne .fail

           cmp [ebx+STREAM.size], STREAM_SIZE
           jne .fail

           mov [ebx+STREAM.flags], SND_PLAY

           mov eax,[ebx+STREAM.work_buff]
           mov [ebx+STREAM.work_read], eax
           mov [ebx+STREAM.work_write], eax
           mov [ebx+STREAM.work_count], 0

           mov eax, [ebx+STREAM.base]
           mov [ebx+STREAM.curr_seg], eax

           mov esi, [ebx+STREAM.curr_seg]
           mov edi, [ebx+STREAM.work_write]
           mov edx, [ebx+STREAM.r_buff]

           mov ecx, 32
           mov eax, [ebx+STREAM.r_silence]
@@:
           mov [edx], eax
           add edx, 4
           dec ecx
           jnz @B

           mov edx, [ebx+STREAM.r_buff]

           stdcall [ebx+STREAM.resample], edi, esi, edx,\
           [ebx+STREAM.r_dt],[ebx+STREAM.r_size],[ebx+STREAM.r_end]

           mov ebx, [str]

           add [ebx+STREAM.work_count], eax;
           add [ebx+STREAM.work_write], eax;

           mov eax, [ebx+STREAM.r_size]
           add [ebx+STREAM.curr_seg], eax

;       if DEBUG
;	   mov esi, msgPlay
;	   call   [SysMsgBoardStr]
;       end if

           stdcall  dev_play, [hSound]
           xor eax, eax
           inc eax
           ret
.fail:
           xor eax, eax
           ret
endp

align 4
proc stop_buffer stdcall, str:dword

           mov edi, [str]

           cmp [edi+STREAM.magic], 'WAVE'
           jne .fail

           cmp [edi+STREAM.size], STREAM_SIZE
           jne .fail

           mov [edi+STREAM.flags], SND_STOP

;           stdcall [ServiceHandler], [hSound], dword DEV_STOP, 0

           xor eax, eax
           inc eax
           ret
.fail:
           xor eax, eax
           ret
endp

align 4
proc set_buffer stdcall, str:dword,src:dword,offs:dword,size:dword

           mov edx, [str]
           test edx, edx
           jz .fail

           cmp [edx+STREAM.magic], 'WAVE'
           jne .fail

           cmp [edx+STREAM.size], STREAM_SIZE
           jne .fail

           mov esi,[src]
           test esi, esi
           jz .fail

           cmp esi, new_app_base
           jb .fail

           mov ecx, [size]
           test ecx, ecx
           jz .fail

           mov eax, [edx+STREAM.base]
           add eax, [offs]

           cmp eax, [edx+STREAM.base]
           jb .fail

           mov edi, eax
           add eax, ecx
           sub eax, 1

           cmp eax, [edx+STREAM.limit]
           ja .fail

           shr ecx, 2
           cld
           rep movsd

           xor eax, eax
           inc eax
           ret
.fail:
           xor eax, eax
           ret
endp

align 4
proc alloc_stream

           mov esi, stream_map

           pushf
           cli

           bsf eax, [esi]
           jnz .find
           popf
           xor eax, eax
           ret
.find:
           btr [esi], eax
           popf
           mov ebx, STREAM_SIZE
           mul ebx
           add eax, stream
           ret
endp

align 4
proc free_stream
           sub eax, stream
           mov ebx, STREAM_SIZE
           xor edx, edx
           div ebx

           and edx, edx
           jnz .err

           bts [stream_map], eax
           ret
.err:
           xor eax, eax
           ret
endp

align 4
proc check_stream

           xor edx, edx
           mov ecx, [play_count]
.l1:
           mov esi, [play_list+edx]

           mov eax, [esi+STR.curr_seg]
           cmp eax, [esi+STR.limit]
           jb .next

.m1:
           mov eax,[esi+STR.base]
           mov [esi+STR.curr_seg], eax
.next:
           add edx, 4
           loop .l1
           ret
endp


align 4
proc prepare_playlist

.restart:
           xor ebx, ebx
           xor edx, edx
           mov [play_count], 0
           mov ecx, [stream_count]
           jcxz .exit
.l1:
           mov esi, [stream_list+ebx]
           test esi, esi
           jz .next

           cmp [esi+STREAM.magic], 'WAVE'
           jne .next

           cmp [esi+STREAM.size], STREAM_SIZE
           jne .next

           mov eax,[esi+STREAM.notify_task]
           cmp eax, -1
           je .fail

           call pid_to_slot
           test eax, eax
           jz .fail

           cmp [esi+STREAM.flags], SND_PLAY;
           jne .next
           cmp [esi+STREAM.work_count], 16384
           jb .next

           mov [play_list+edx], esi
           inc [play_count]
           add edx, 4
.next:
           add ebx, 4
           loop .l1
.exit:
           ret
.fail:
           stdcall DestroyBuffer, esi
           jmp .restart
endp

align 4
proc prepare_updatelist

           xor ebx, ebx
           xor edx, edx
           mov [play_count], 0
           mov ecx, [stream_count]
           jcxz .exit
.l1:
           mov eax, [stream_list+ebx]
           test eax, eax
           jz .next
           cmp [eax+STREAM.flags], SND_PLAY
           jne .next

           mov [play_list+edx], eax
           inc [play_count]
           add edx, 4
.next:
           add ebx, 4
           loop .l1
.exit:
           ret
endp

align 4
proc set_handler stdcall, hsrv:dword, handler_proc:dword
           locals
             handler    dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
             val        dd ?
           endl

           mov eax, [hsrv]
           lea ecx, [handler_proc]
           xor ebx, ebx

           mov [handler], eax
           mov [io_code], DEV_CALLBACK
           mov [input], ecx
           mov [inp_size], 4
           mov [output], ebx
           mov [out_size], 0

           lea eax, [handler]
           stdcall ServiceHandler, eax
           ret
endp

align 4
proc dev_play stdcall, hsrv:dword
           locals
             handle     dd ?
             io_code    dd ?
             input      dd ?
             inp_size   dd ?
             output     dd ?
             out_size   dd ?
             val        dd ?
           endl

           mov eax, [hsrv]
           xor ebx, ebx

           mov [handle], eax
           mov [io_code], DEV_PLAY
           mov [input], ebx
           mov [inp_size], ebx
           mov [output], ebx
           mov [out_size], ebx

           lea eax, [handle]
           stdcall ServiceHandler, eax
           ret
endp

include 'mixer.asm'

align 16
resampler_params:
     ;r_size    r_end   r_dt   resampler_func
     dd 0,0,0,0                                  ; 0  PCM_ALL
     dd 16384,          0,     0, copy_stream    ; 1  PCM_2_16_48
     dd 16384,          0,     0, m16_stereo     ; 2  PCM_1_16_48

     dd 16384, 0x08000000, 30109, resample_2     ; 3  PCM_2_16_44
     dd  8192, 0x08000000, 30109, resample_1     ; 4  PCM_1_16_44

     dd 16384, 0x08000000, 21846, resample_2     ; 5  PCM_2_16_32
     dd  8192, 0x08000000, 21846, resample_1     ; 6  PCM_1_16_32

     dd 16384, 0x08000000, 16384, resample_2     ; 7  PCM_2_16_24
     dd  8192, 0x08000000, 16384, resample_1     ; 8  PCM_1_16_24

     dd  8192, 0x04000000, 15052, resample_2     ; 9  PCM_2_16_22
     dd  4096, 0x04000000, 15052, resample_1     ;10  PCM_1_16_22

     dd  8192, 0x04000000, 10923, resample_2     ;11  PCM_2_16_16
     dd  4096, 0x04000000, 10923, resample_1     ;12  PCM_1_16_16

     dd  8192, 0x04000000,  8192, resample_2     ;13  PCM_2_16_12
     dd  4096, 0x04000000,  8192, resample_1     ;14  PCM_1_16_12

     dd  4096, 0x02000000,  7527, resample_2     ;15  PCM_2_16_11
     dd  2048, 0x02000000,  7527, resample_1     ;16  PCM_1_16_11

     dd  4096, 0x02000000,  5462, resample_2     ;17  PCM_2_16_8
     dd  2048, 0x02000000,  5462, resample_1     ;18  PCM_1_16_8

     dd 16384,          0,     0, s8_stereo      ;19  PCM_2_8_48
     dd  8192,          0,     0, m8_stereo      ;20  PCM_1_8_48

     dd  8192, 0x08000000, 30109, resample_28    ;21  PCM_2_8_44
     dd  4096, 0x08000000, 30109, resample_18    ;22  PCM_1_8_44

     dd  8192, 0x08000000, 21846, resample_28    ;23  PCM_2_8_32
     dd  4096, 0x08000000, 21846, resample_18    ;24  PCM_1_8_32

     dd  8192, 0x08000000, 16384, resample_28    ;25  PCM_2_8_24
     dd  4096, 0x08000000, 16384, resample_18    ;26  PCM_1_8_24

     dd  4096, 0x04000000, 15052, resample_28    ;27  PCM_2_8_22
     dd  2048, 0x04000000, 15052, resample_18    ;28  PCM_1_8_22

     dd  4096, 0x04000000, 10923, resample_28    ;29  PCM_2_8_16
     dd  2048, 0x04000000, 10923, resample_18    ;30  PCM_1_8_16

     dd  4096, 0x04000000,  8192, resample_28    ;31  PCM_2_8_12
     dd  2048, 0x04000000,  8192, resample_18    ;32  PCM_1_8_12

     dd  2048, 0x02000000,  7527, resample_28    ;33  PCM_2_8_11
     dd  1024, 0x02000000,  7527, resample_18    ;34  PCM_1_8_11

     dd  2048, 0x02000000,  5462, resample_28    ;35  PCM_2_8_8
     dd  1024, 0x02000000,  5462, resample_18    ;36  PCM_1_8_8

m7	     dw 0x8000,0x8000,0x8000,0x8000
mm80	     dq 0x8080808080808080
mm_mask      dq 0xFF00FF00FF00FF00

stream_map   dd 0xFFFF	      ; 16
version      dd 0x00010001

szInfinity   db 'INFINITY',0
szSound      db 'SOUND',0

if DEBUG
msgFail      db 'Sound service not loaded',13,10,0
msgPlay      db 'Play buffer',13,10,0
msgStop      db 'Stop',13,10,0
msgUser      db 'User callback',13,10,0
msgMem       db 'Not enough memory',13,10,0
end if

section '.data' data readable writable align 16

stream       rb STREAM_SIZE*16

play_list    rd 16
mix_input    rd 16

stream_list  rd 17
play_count   rd 1
stream_count rd 1
hSound       rd 1
mix_buff     rd 1
mix_buff_map rd 1

