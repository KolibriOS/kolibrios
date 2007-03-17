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
include 'imports.inc'

FORCE_MMX         equ 0  ;set to 1 to force use mmx or
FORCE_MMX_128     equ 0  ;integer sse2 extensions
                        ;and reduce driver size
;USE_SSE          equ 0

DEBUG             equ 1


OS_BASE           equ 0x80000000
SLOT_BASE         equ (OS_BASE+0x0080000)
TASK_COUNT        equ (OS_BASE+0x0003004)
CURRENT_TASK      equ (OS_BASE+0x0003000)


CAPS_SSE2         equ 26
PG_SW             equ 0x003

public START
public service_proc
public version

RT_INP_EMPTY      equ 0xFF000001
RT_OUT_EMPTY      equ 0xFF000002
RT_INP_FULL       equ 0xFF000003
RT_OUT_FULL       equ 0xFF000004

EVENT_WATCHED     equ 0x10000000
EVENT_SIGNALED    equ 0x20000000
MANUAL_RESET      equ 0x40000000
MANUAL_DESTROY    equ 0x80000000

DEV_PLAY          equ 1
DEV_STOP          equ 2
DEV_CALLBACK      equ 3

struc IOCTL
{  .handle        dd ?
   .io_code       dd ?
   .input         dd ?
   .inp_size      dd ?
   .output        dd ?
   .out_size      dd ?
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

           mov eax, str.fd-FD_OFFSET
           mov [str.fd], eax
           mov [str.bk], eax

if FORCE_MMX
 if FORCE_MMX_128
  display 'Use only FORCE_MMX or FORCE_MMX_128 not both together',13,10
  stop
 end if
           mov [mix_2_core], mmx_mix_2
           mov [mix_3_core], mmx_mix_3
           mov [mix_4_core], mmx_mix_4
end if

if FORCE_MMX_128
 if FORCE_MMX
  display 'Use only FORCE_MMX or FORCE_MMX_128 not both together',13,10
  stop
 end if
           mov [mix_2_core], mmx128_mix_2
           mov [mix_3_core], mmx128_mix_3
           mov [mix_4_core], mmx128_mix_4
end if

if 0

if ~(FORCE_MMX or FORCE_MMX_128)  ;autodetect
           mov eax, 1
           cpuid
           bt edx, CAPS_SSE2
           jc .mmx128
                                           ;old 64-bit mmx
           mov [mix_2_core], mmx_mix_2
           mov [mix_3_core], mmx_mix_3
           mov [mix_4_core], mmx_mix_4
           jmp @F
.mmx128:                                   ;128-bit integer sse2 extensions
           mov [mix_2_core], mmx128_mix_2
           mov [mix_3_core], mmx128_mix_3
           mov [mix_4_core], mmx128_mix_4
@@:
end if

end if
           stdcall set_handler, [hSound], new_mix
           mov [eng_state], SND_STOP
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

           cmp eax, SRV_GETVERSION
           jne @F
           mov eax, [edi+output]
           mov eax, [eax]
           mov [eax], dword SOUND_VERSION
           xor eax, eax
           ret
@@:
           cmp eax, SND_CREATE_BUFF
           jne @F
           mov ebx, [edi+input]
           push edi
           stdcall CreateBuffer,[ebx],[ebx+4]
           pop edi
           mov ecx, [edi+output]
           mov ecx, [ecx]
           mov [ecx], ebx
           ret
@@:
           mov ebx, [edi+input]
           mov edx, [ebx]

           cmp [edx+STREAM.magic], 'WAVE'
           jne .fail

           cmp [edx+STREAM.size], STREAM_SIZE
           jne .fail

           cmp eax, SND_DESTROY_BUFF
           jne @F
           mov eax, edx
           call DestroyBuffer    ;edx= stream
           ret
@@:
           cmp eax, SND_SETFORMAT
           jne @F
           stdcall SetFormat,[ebx],[ebx+4]
           ret
@@:
           cmp eax, SND_GETFORMAT
           jne @F

           movzx eax, word [edx+STREAM.format]
           mov ecx, [edi+output]
           mov ecx, [ecx]
           mov [ecx], eax
           xor eax, eax
           ret
@@:
           cmp eax, SND_RESET
           jne @F
           stdcall ResetBuffer,[ebx],[ebx+4]
           ret
@@:
           cmp eax, SND_SETPOS
           jne @F
           stdcall SetBufferPos,[ebx],[ebx+4]
           ret
@@:
           cmp eax, SND_GETPOS
           jne @F
           push edi
           stdcall GetBufferPos, [ebx]
           pop edi
           mov ecx, [edi+output]
           mov ecx, [ecx]
           mov [ecx], ebx
           ret
@@:
           cmp eax, SND_SETBUFF
           jne @F
           mov eax, [ebx+4]
           stdcall set_buffer, [ebx],eax,[ebx+8],[ebx+12]
           ret
@@:
           cmp eax, SND_SETVOLUME
           jne @F
           stdcall SetBufferVol,[ebx],[ebx+4],[ebx+8]
           ret
@@:
           cmp eax, SND_GETVOLUME
           jne @F

           mov eax, [edi+output]
           mov ecx, [eax]
           mov eax, [eax+4]
           stdcall GetBufferVol,[ebx],ecx,eax
           ret
@@:
           cmp eax, SND_SETPAN
           jne @F
           stdcall SetBufferPan,[ebx],[ebx+4]
           ret
@@:
           cmp eax, SND_GETPAN
           jne @F
           mov eax, [edx+STREAM.pan]
           mov ebx, [edi+output]
           mov ebx, [ebx]
           mov [ebx], eax
           xor eax, eax
           ret
@@:
           cmp eax, SND_OUT
           jne @F

           mov eax, [ebx+4]
           stdcall wave_out, [ebx],eax,[ebx+8]
           ret
@@:
           cmp eax, SND_PLAY
           jne @F

           stdcall play_buffer, [ebx],[ebx+4]
           ret
@@:
           cmp eax, SND_STOP
           jne @F

           stdcall stop_buffer, [ebx]
           ret
@@:
           cmp eax, SND_GETBUFFSIZE
           jne @F
           mov eax, [edx+STREAM.in_size]
           mov ecx, [edi+output]
           mov ecx, [ecx]
           mov [ecx], eax
           xor eax, eax
           ret
@@:
.fail:
           or eax, -1
           ret
endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size

align 4
proc CreateBuffer stdcall, format:dword, size:dword
           locals
             str         dd ?
             ring_size   dd ?
             ring_pages  dd ?
           endl

           mov eax, [format]
           cmp ax, PCM_1_8_8
           ja .fail

           test eax, PCM_OUT
           jnz .test_out
           test eax, PCM_RING
           jnz .test_ring
;staic
           test eax, PCM_OUT+PCM_RING
           jnz .fail
           jmp .test_ok
.test_out:
           test eax, PCM_RING+PCM_STATIC
           jnz .fail
           jmp .test_ok
.test_ring:
           test eax, PCM_OUT+PCM_STATIC
           jnz .fail
.test_ok:
           mov ebx, [CURRENT_TASK]      ;hack: direct accsess
           shl ebx, 5                   ;to kernel data
           mov ebx, [CURRENT_TASK+ebx+4]
           mov eax, STREAM_SIZE

           call CreateObject
           test eax, eax
           jz .fail
           mov [str], eax

           mov ebx, [format]
           mov [eax+STREAM.format], ebx

           xor ecx, ecx
           movzx ebx, bx
           cmp ebx, 19
           jb @f
           mov ecx, 0x80808080
@@:
           mov [eax+STREAM.r_silence], ecx

           shl ebx, 2
           lea ebx, [ebx+ebx*2]     ;ebx*=12

           mov ecx, [resampler_params+ebx]
           mov edx, [resampler_params+ebx+4]
           mov esi, [resampler_params+ebx+8]

           mov [eax+STREAM.r_size],ecx
           mov [eax+STREAM.r_dt],  edx
           mov [eax+STREAM.resample], esi
           xor ecx, ecx
           mov [eax+STREAM.l_vol], ecx
           mov [eax+STREAM.r_vol], ecx
           mov dword [eax+STREAM.l_amp], 0x7FFF7FFF
           mov [eax+STREAM.pan], ecx

           test [format], PCM_STATIC
           jnz .static

; ring and waveout

           mov ebx, 0x10000
           test [format], PCM_RING
           jz .waveout

           mov ebx, [eax+STREAM.r_size]
           add ebx, 4095
           and ebx, -4096
           add ebx, ebx
.waveout:
           mov [ring_size], ebx
           mov eax, ebx
           shr ebx, 12
           mov [ring_pages], ebx

           add eax, eax              ;double ring size
           stdcall AllocKernelSpace, eax

           mov edi, [str]
           mov ecx, [ring_size]
           mov [edi+STREAM.in_base], eax
           mov [edi+STREAM.in_size], ecx
           add eax, 128
           sub ecx, 128
           mov [edi+STREAM.in_wp], eax
           mov [edi+STREAM.in_rp], eax
           mov [edi+STREAM.in_count], 0

           mov [edi+STREAM.in_free], ecx
           add eax, ecx
           mov [edi+STREAM.in_top], eax

           mov ebx, [ring_pages]
           stdcall AllocPages, ebx
           mov edi, [str]
           mov ebx, [edi+STREAM.in_base]
           mov ecx, [ring_pages]
           or eax, PG_SW
           push eax
           push ebx
           call CommitPages ;eax, ebx, ecx
           mov ecx, [ring_pages]
           pop ebx
           pop eax
           add ebx, [ring_size]
           call CommitPages    ;double mapped

           jmp .out_buff
.static:
           mov ecx, [size]
           add ecx, 128          ;resampler required
           mov [eax+STREAM.in_size], ecx
           stdcall KernelAlloc, ecx

           mov edi, [str]
           mov [edi+STREAM.in_base], eax
           add eax, 128
           mov [edi+STREAM.in_wp], eax
           mov [edi+STREAM.in_rp], eax
           mov ebx, [size]
           mov [edi+STREAM.in_count], ebx
           mov [edi+STREAM.in_free], ebx
           add eax, ebx
           mov [edi+STREAM.in_top], eax

.out_buff:
           stdcall AllocKernelSpace, dword 128*1024

           mov edi, [str]
           mov [edi+STREAM.out_base], eax
           mov [edi+STREAM.out_wp], eax
           mov [edi+STREAM.out_rp], eax
           mov [edi+STREAM.out_count], 0
           add eax, 64*1024
           mov [edi+STREAM.out_top], eax

           stdcall AllocPages, dword 64/4
           mov edi, [str]
           mov ebx, [edi+STREAM.out_base]
           mov ecx, 16
           or eax, PG_SW
           push eax
           push ebx
           call CommitPages ;eax, ebx, ecx
           mov ecx, 16
           pop ebx
           pop eax
           add ebx, 64*1024
           call CommitPages    ;double mapped

           mov edi, [str]
           mov ecx, [edi+STREAM.in_top]
           mov edi, [edi+STREAM.in_base]
           sub ecx, edi
           xor eax, eax
           shr ecx, 2
           cld
           rep stosd

           mov edi, [str]
           mov edi, [edi+STREAM.out_base]
           mov ecx, (64*1024)/4
           rep stosd

           xor edx, edx
           mov ebx, MANUAL_DESTROY
           call CreateEvent

           mov ebx, [str]
           mov [ebx+STREAM.notify_event], eax
           mov [ebx+STREAM.notify_id], edx

           mov [ebx+STREAM.magic], 'WAVE'
           mov [ebx+STREAM.destroy], DestroyBuffer.destroy
           mov [ebx+STREAM.size], STREAM_SIZE
           mov [ebx+STREAM.flags], SND_STOP

           pushf
           cli
           mov eax, str.fd-FD_OFFSET
           mov edx, [eax+STREAM.str_fd]
           mov [ebx+STREAM.str_fd], edx
           mov [ebx+STREAM.str_bk], eax
           mov [eax+STREAM.str_fd], ebx
           mov [edx+STREAM.str_bk], ebx
           popf

           xor eax, eax
           ret
.fail:
           xor ebx, ebx
           or eax, -1
           ret
endp

;param
; eax= buffer handle

align 4
DestroyBuffer:
           .handle  equ esp       ;local

           mov [eax+STREAM.flags], SND_STOP
.destroy:
           push eax

           pushfd
           cli
           mov ebx, [eax+STREAM.str_fd]
           mov ecx, [eax+STREAM.str_bk]
           mov [ebx+STREAM.str_bk], ecx
           mov [ecx+STREAM.str_fd], ebx
           popf

           stdcall KernelFree, [eax+STREAM.in_base]
           mov eax, [.handle]
           stdcall KernelFree, [eax+STREAM.out_base]

           pop eax               ;restore stack
           call DestroyObject    ;eax= stream
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
restore .handle

align 4
proc SetFormat stdcall, str:dword, format:dword

           cmp word [format], PCM_1_8_8
           ja .fail

           mov edx, [str]
           mov [edx+STREAM.flags], SND_STOP

           test [edx+STREAM.format], PCM_RING
           jnz .fail

;           mov eax,[edx+STREAM.out_base]
;           mov [edx+STREAM.out_wp], eax
;           mov [edx+STREAM.out_rp], eax
;           mov [edx+STREAM.out_count], 0

           movzx eax, word [format]
           mov word [edx+STREAM.format], ax

           xor ebx, ebx
           cmp eax, 19
           jb @f
           mov ebx, 0x80808080
@@:
           mov [edx+STREAM.r_silence], ebx

           shl eax, 2
           lea eax, [eax+eax*2]     ;eax*=12

           mov edi, [resampler_params+eax]
           mov ecx, [resampler_params+eax+4]
           mov ebx, [resampler_params+eax+8]

           mov [edx+STREAM.r_size],edi
           mov [edx+STREAM.r_dt],  ecx
           mov [edx+STREAM.resample], ebx

           mov edi, [edx+STREAM.in_base]
           mov ecx, 128/4
           mov eax, [edx+STREAM.r_silence]
           cld
           rep stosd
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
endp

; for static buffers only
; use waveout for streams

align 4
proc set_buffer stdcall, str:dword,src:dword,offs:dword,size:dword

           mov edx, [str]
           test [edx+STREAM.format], PCM_OUT
           jnz .fail

           mov esi, [src]
           mov edi, [offs]
           add edi, [edx+STREAM.in_base]
           add edi, 128

           cmp edi, [edx+STREAM.in_top]
           jae .fail

           mov ecx, [size]
           lea ebx, [ecx+edi]
           sub ebx, [edx+STREAM.in_top]
           jb @F
           sub ecx, ebx
@@:
           shr ecx, 2
           cld
           rep movsd
           xor eax,eax
           ret
.fail:
           or eax, -1
           ret
endp

; for stream buffers only

align 4
proc wave_out stdcall, str:dword,src:dword,size:dword
           locals
             state_saved  dd ?
             fpu_state    rb 528
           endl

           mov edx, [str]
           mov eax, [edx+STREAM.format]
           test eax, PCM_STATIC+PCM_RING
           jnz .fail

           cmp ax, PCM_ALL
           je .fail

           mov esi,[src]
           test esi, esi
           jz .fail

           cmp esi, OS_BASE
           ja .fail

           mov [state_saved], 0

.main_loop:
           mov edx, [str]

           mov ebx, [size]
           test ebx, ebx
           jz .done

           cmp [edx+STREAM.flags], SND_STOP
           jne .fill

           mov edi, [edx+STREAM.in_base]
           mov ecx, 128/4
           mov eax, [edx+STREAM.r_silence]
           cld
           rep stosd

           mov ecx, [edx+STREAM.in_size]
           sub ecx, 128
           mov [edx+STREAM.in_wp], edi
           mov [edx+STREAM.in_rp], edi
           mov [edx+STREAM.in_count], 0
           mov [edx+STREAM.in_free], ecx

           mov eax,[edx+STREAM.out_base]
           mov [edx+STREAM.out_wp], eax
           mov [edx+STREAM.out_rp], eax
           mov [edx+STREAM.out_count], 0
.fill:
           mov ecx, [edx+STREAM.in_free]
           test ecx, ecx
           jz .wait

           cmp ecx, ebx
           jbe @F

           mov ecx, ebx
@@:
           sub [size], ecx
           add [edx+STREAM.in_count], ecx
           sub [edx+STREAM.in_free], ecx

           shr ecx, 2
           mov edi, [edx+STREAM.in_wp]
           mov esi, [src]
           cld
           rep movsd

           mov [src], esi
           cmp edi, [edx+STREAM.in_top]
           jb @F
           sub edi, [edx+STREAM.in_size]
@@:
           mov [edx+STREAM.in_wp], edi

           cmp [edx+STREAM.out_count], 32768
           jae .skip

           cmp [state_saved], 0
           jne @F
           lea eax, [fpu_state+15]
           and eax, -16
           call FpuSave
           mov [state_saved], 1
@@:
           stdcall refill, edx
.skip:
           mov ebx, [str]
           mov [ebx+STREAM.flags], SND_PLAY
           cmp [eng_state], SND_PLAY
           je .main_loop

           stdcall dev_play, [hSound]
           mov [eng_state], SND_PLAY
           jmp .main_loop
.wait:
           mov edx, [str]
           mov eax, [edx+STREAM.notify_event]
           mov ebx, [edx+STREAM.notify_id]
           call WaitEvent   ;eax ebx
           jmp .main_loop
.done:
           cmp [state_saved], 1
           jne @F

           lea eax, [fpu_state+15]
           and eax, -16
           call FpuRestore
@@:
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
endp

; both static and stream
; reset all but not clear buffers


; flags reserved
;  RESET_INPUT  equ 1   ;reserved reset and clear input buffer
;  RESET_OUTPUT equ 2   ;reserved reset and clear output buffer
;  RESET_ALL    equ 3


align 4
proc ResetBuffer stdcall, str:dword, flags:dword

           mov edx, [str]
           mov [edx+STREAM.flags], SND_STOP

           mov edi, [edx+STREAM.in_base]
           mov ecx, 128/4
           mov eax, [edx+STREAM.r_silence]
           cld
           rep stosd

           mov [edx+STREAM.in_wp], edi
           mov [edx+STREAM.in_rp], edi

           mov [edx+STREAM.in_count], 0
           mov eax, [edx+STREAM.in_size]
           sub eax, 128
           mov [edx+STREAM.in_free], eax

           xor eax, eax
           mov ebx,[edx+STREAM.out_base]
           mov [edx+STREAM.out_wp], ebx
           mov [edx+STREAM.out_rp], ebx
           mov [edx+STREAM.out_count], eax
           ret
.fail:
           or eax, -1
           ret
endp

; for static buffers only

align 4
proc SetBufferPos stdcall, str:dword, pos:dword

           mov edx, [str]
           test [edx+STREAM.format], PCM_OUT+PCM_RING
           jnz .fail

           mov [edx+STREAM.flags], SND_STOP

           mov eax, [pos]
           add eax, [edx+STREAM.in_base]
           mov ebx, [edx+STREAM.in_top]
           add eax, 128

           cmp eax, ebx
           jae .fail

           mov [edx+STREAM.in_rp], eax
           sub ebx, eax
           mov [edx+STREAM.in_count], ebx
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
endp

align 4
proc GetBufferPos stdcall, str:dword

           mov edx, [str]
           test [edx+STREAM.format], PCM_OUT+PCM_RING
           jnz .fail

           mov ebx, [edx+STREAM.in_rp]
           xor eax, eax
           ret
.fail:
           xor ebx,ebx
           or eax, -1
           ret
endp

; both

align 4
proc SetBufferVol stdcall, str:dword,l_vol:dword,r_vol:dword

           mov edx, [str]
           stdcall set_vol_param,[l_vol],[r_vol],[edx+STREAM.pan]
           ret
endp

proc set_vol_param stdcall, l_vol:dword,r_vol:dword,pan:dword
           locals
             _600    dd ?
             _32767  dd ?
             state   rb 108
           endl

           mov [_600], 0x44160000   ;600.0
           mov [_32767], 32767

           lea ebx, [state]
           fnsave [ebx]

           movq mm0, qword [l_vol]
           pminsw mm0, qword [vol_max]
           pmaxsw mm0, qword [vol_min]
           movq qword [l_vol], mm0
           movq qword [edx+STREAM.l_vol], mm0

           movd mm1,[pan]
           pminsw mm1, qword [pan_max]
           pmaxsw mm1, qword [vol_min]
           movd [edx+STREAM.pan], mm1

           cmp word [edx+STREAM.pan], 0
           jl @F

           psubsw mm0,mm1
           pminsw mm0, qword [vol_max]
           pmaxsw mm0, qword [vol_min]
           movd [l_vol],mm0
           jmp .calc_amp
@@:
           punpckhdq mm0,mm0
           paddsw mm0,mm1
           pminsw mm0, qword [vol_max]
           pmaxsw mm0, qword [vol_min]
           movd [r_vol], mm0
.calc_amp:
           emms
           fild word [l_vol]

           call .calc

           fistp word [edx+STREAM.l_amp]
           fstp st0

           fild word [r_vol]

           call .calc

           fistp word [edx+STREAM.r_amp]
           fstp st0

           fnclex
           lea ebx, [state]
           frstor [ebx]

           xor eax, eax
           inc eax
           ret
.calc:
           fdiv dword [_600]
           fld st0
           frndint
           fxch st1
           fsub st, st1
           f2xm1
           fld1
           faddp st1, st0
           fscale
           fimul dword [_32767]
           ret 0
endp

align 4
proc GetBufferVol stdcall, str:dword,p_lvol:dword,p_rvol:dword

           mov edx, [str]
           mov eax, [p_lvol]
           movsx ecx, word [edx+STREAM.l_vol]
           mov [eax], ecx

           mov eax, [p_rvol]
           movsx ecx, word [edx+STREAM.r_vol]
           mov [eax], ecx
           xor eax, eax
           ret
endp

align 4
proc SetBufferPan stdcall, str:dword,pan:dword

           mov edx, [str]
           stdcall set_vol_param,[edx+STREAM.l_vol],\
                                 [edx+STREAM.r_vol],[pan]
           ret
endp

; for static and ring buffers only

align 4
proc play_buffer stdcall, str:dword, flags:dword
           locals
             fpu_state   rb 528
           endl

           mov ebx, [str]
           mov eax, [ebx+STREAM.format]
           test eax, PCM_OUT
           jnz .fail

           cmp ax, PCM_ALL
           je .fail

           mov [ebx+STREAM.flags], SND_PLAY
           cmp [eng_state], SND_PLAY
           je .done

           stdcall  dev_play, [hSound]
           mov [eng_state], SND_PLAY
.done:
           test [flags], PLAY_SYNC
           jz @F

           mov edx, [str]
.wait:
           mov eax, [edx+STREAM.notify_event]
           mov ebx, [edx+STREAM.notify_id]
           call WaitEvent   ;eax ebx

           mov edx, [str]
           cmp [edx+STREAM.flags], SND_STOP
           jne .wait
@@:
           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
endp

; for static buffers only

align 4
proc stop_buffer stdcall, str:dword

           mov edx, [str]
           test [edx+STREAM.format], PCM_STATIC+PCM_RING
           jz .fail

           mov [edx+STREAM.flags], SND_STOP

;           stdcall [ServiceHandler], [hSound], dword DEV_STOP, 0

           mov eax, [edx+STREAM.notify_event]
           mov ebx, [edx+STREAM.notify_id]
           call ClearEvent   ;eax ebx

           xor eax, eax
           ret
.fail:
           or eax, -1
           ret
endp

; parm
;  eax= mix_list

align 4
do_mix_list:

           xor edx, edx
           mov esi, str.fd-FD_OFFSET
           mov ebx, [esi+STREAM.str_fd]
@@:
           cmp ebx, esi
           je .done

           cmp [ebx+STREAM.magic], 'WAVE'
           jne .next

           cmp [ebx+STREAM.size], STREAM_SIZE
           jne .next

           cmp [ebx+STREAM.flags], SND_PLAY;
           jne .next

           mov ecx, [ebx+STREAM.out_count]
           test ecx, ecx
           jnz .l1

           test [ebx+STREAM.format], PCM_RING
           jnz .next
           mov [ebx+STREAM.flags], SND_STOP
           jmp .next
.l1:
           cmp ecx, 512
           jae .add_buff

           mov edi, [ebx+STREAM.out_rp]
           add edi, ecx
           sub ecx, 512
           neg ecx
           push eax
           xor eax, eax
           cld
           rep stosb
           pop eax

           mov [ebx+STREAM.out_count], 512

.add_buff:
           mov ecx, [ebx+STREAM.out_rp]
           mov [eax],ecx
           mov edi, dword [ebx+STREAM.l_amp]
           mov [eax+4], edi
           add [ebx+STREAM.out_rp], 512
           sub [ebx+STREAM.out_count], 512

           add eax, 8
           inc edx
.next:
           mov ebx, [ebx+STREAM.str_fd]
           jmp @B
.done:
           mov eax, edx
           ret

align 4
prepare_playlist:

           xor edx, edx
           mov [play_count], edx
           mov esi, str.fd-FD_OFFSET
           mov edi, [esi+STREAM.str_fd]
@@:
           cmp edi, esi
           je .done

           cmp [edi+STREAM.magic], 'WAVE'
           jne .next

           cmp [edi+STREAM.size], STREAM_SIZE
           jne .next

           cmp [edi+STREAM.flags], SND_PLAY;
           jne .next

           mov [play_list+edx], edi
           inc [play_count]
           add edx, 4
.next:
           mov edi, [edi+STREAM.str_fd]
           jmp @B
.done:
           ret

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

if 0
align 4
dword2str:
      mov  esi, hex_buff
      mov ecx, -8
@@:
      rol eax, 4
      mov ebx, eax
      and ebx, 0x0F
      mov bl, [ebx+hexletters]
      mov [8+esi+ecx], bl
      inc ecx
      jnz @B
      ret

hexletters   db '0123456789ABCDEF'
hex_buff     db 8 dup(0),13,10,0

end if

include 'mixer.asm'
include 'mix_mmx.inc'
include 'mix_sse2.inc'

;if USE_SSE
; include 'mix_sse.inc'
;end if

align 16
resampler_params:
     ;r_size    r_dt   resampler_func
     dd 0,0,0                                  ; 0  PCM_ALL
     dd 16384,      0, copy_stream    ; 1  PCM_2_16_48
     dd 16384,      0, m16_stereo     ; 2  PCM_1_16_48

     dd 16384,  30109, resample_2     ; 3  PCM_2_16_44
     dd  8192,  30109, resample_1     ; 4  PCM_1_16_44

     dd 16384,  21846, resample_2     ; 5  PCM_2_16_32
     dd  8192,  21846, resample_1     ; 6  PCM_1_16_32

     dd 16384,  16384, resample_2     ; 7  PCM_2_16_24
     dd  8192,  16384, resample_1     ; 8  PCM_1_16_24

     dd  8192,  15052, resample_2     ; 9  PCM_2_16_22
     dd  4096,  15052, resample_1     ;10  PCM_1_16_22

     dd  8192,  10923, resample_2     ;11  PCM_2_16_16
     dd  4096,  10923, resample_1     ;12  PCM_1_16_16

     dd  8192,   8192, resample_2     ;13  PCM_2_16_12
     dd  4096,   8192, resample_1     ;14  PCM_1_16_12

     dd  4096,   7527, resample_2     ;15  PCM_2_16_11
     dd  2048,   7527, resample_1     ;16  PCM_1_16_11

     dd  4096,   5462, resample_2     ;17  PCM_2_16_8
     dd  2048,   5462, resample_1     ;18  PCM_1_16_8

     dd 16384,      0, s8_stereo      ;19  PCM_2_8_48
     dd  8192,      0, m8_stereo      ;20  PCM_1_8_48

     dd  8192,  30109, resample_28    ;21  PCM_2_8_44
     dd  4096,  30109, resample_18    ;22  PCM_1_8_44

     dd  8192,  21846, resample_28    ;23  PCM_2_8_32
     dd  4096,  21846, resample_18    ;24  PCM_1_8_32

     dd  8192,  16384, resample_28    ;25  PCM_2_8_24
     dd  4096,  16384, resample_18    ;26  PCM_1_8_24

     dd  4096,  15052, resample_28    ;27  PCM_2_8_22
     dd  2048,  15052, resample_18    ;28  PCM_1_8_22

     dd  4096,  10923, resample_28    ;29  PCM_2_8_16
     dd  2048,  10923, resample_18    ;30  PCM_1_8_16

     dd  4096,   8192, resample_28    ;31  PCM_2_8_12
     dd  2048,   8192, resample_18    ;32  PCM_1_8_12

     dd  2048,   7527, resample_28    ;33  PCM_2_8_11
     dd  1024,   7527, resample_18    ;34  PCM_1_8_11

     dd  2048,   5462, resample_28    ;35  PCM_2_8_8
     dd  1024,   5462, resample_18    ;36  PCM_1_8_8

m7            dw 0x8000,0x8000,0x8000,0x8000
mm80          dq 0x8080808080808080
mm_mask       dq 0xFF00FF00FF00FF00

vol_max       dd 0x00000000,0x00000000
vol_min       dd 0x0000D8F0,0x0000D8F0
pan_max       dd 0x00002710,0x00002710

;stream_map    dd 0xFFFF       ; 16
version       dd (4 shl 16) or (SOUND_VERSION and 0xFFFF)

szInfinity    db 'INFINITY',0
szSound       db 'SOUND',0

if DEBUG
msgFail       db 'Sound service not loaded',13,10,0
msgPlay       db 'Play buffer',13,10,0
msgStop       db 'Stop',13,10,0
msgUser       db 'User callback',13,10,0
msgMem        db 'Not enough memory',13,10,0
msgDestroy    db 'Destroy sound buffer', 13,10,0
msgWaveout    db 'Play waveout', 13,10,0
msgSetVolume  db 'Set volume',13,10,0
end if

section '.data' data readable writable align 16

play_list     rd 16
mix_input     rd 16
play_count    rd 1
hSound        rd 1
eng_state     rd 1
mix_buff      rd 1
mix_buff_map  rd 1
str.fd        rd 1
str.bk        rd 1

mix_2_core    rd 1
mix_3_core    rd 1
mix_4_core    rd 1

