;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2006-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Serge 2006-2008
; email: infinity_sound@mail.ru

format PE DLL native 0.05
entry START

DEBUG             equ 1


CURRENT_API     equ   0x0101      ;1.01
COMPATIBLE_API  equ   0x0100      ;1.00

API_VERSION     equ   (COMPATIBLE_API shl 16) or CURRENT_API
SOUND_VERSION   equ   CURRENT_API


FORCE_MMX         equ 0  ;set to 1 to force use mmx or
FORCE_MMX_128     equ 0  ;integer sse2 extensions
                         ;and reduce driver size

;USE_SSE          equ 0

USE_SSE2_MIXER    equ 0  ;floating point mixer. Disabled by default

OS_BASE           equ 0x80000000

CAPS_SSE2         equ 26
PG_SW             equ 0x003

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
DEV_GET_POS       equ 9

section '.flat' code readable executable
include '../../struct.inc'
include '../../macros.inc'
include '../../proc32.inc'
include 'main.inc'
include '../../peimport.inc'



proc START c uses ebx esi edi, state:dword, cmdline:dword

        cmp     [state], 1
        jne     .exit

        invoke  GetService, szSound
        test    eax, eax
        jz      .fail
        mov     [hSound], eax

        invoke  KernelAlloc, 16*512
        test    eax, eax
        jz      .out_of_mem
        mov     [mix_buff], eax

        mov     eax, str.fd-FD_OFFSET
        mov     [str.fd], eax
        mov     [str.bk], eax

if FORCE_MMX
 if FORCE_MMX_128
  display 'Use only FORCE_MMX or FORCE_MMX_128 not both together',13,10
  stop
 end if
        mov     [mix_2_core], mmx_mix_2
        mov     [mix_3_core], mmx_mix_3
        mov     [mix_4_core], mmx_mix_4
end if

if FORCE_MMX_128
 if FORCE_MMX
  display 'Use only FORCE_MMX or FORCE_MMX_128 not both together',13,10
  stop
 end if
        mov     [mix_2_core], mmx128_mix_2
        mov     [mix_3_core], mmx128_mix_3
        mov     [mix_4_core], mmx128_mix_4
end if

if 0

if ~(FORCE_MMX or FORCE_MMX_128)  ;autodetect
        mov     eax, 1
        cpuid
        bt      edx, CAPS_SSE2
        jc      .mmx128
                                           ;old 64-bit mmx
        mov     [mix_2_core], mmx_mix_2
        mov     [mix_3_core], mmx_mix_3
        mov     [mix_4_core], mmx_mix_4
        jmp     @F
.mmx128:                                   ;128-bit integer sse2 extensions
        mov     [mix_2_core], mmx128_mix_2
        mov     [mix_3_core], mmx128_mix_3
        mov     [mix_4_core], mmx128_mix_4
@@:
end if

end if
        stdcall set_handler, [hSound], new_mix
        mov     [eng_state], SND_STOP
        invoke  RegService, szInfinity, service_proc
        ret
.fail:
     if DEBUG
        mov     esi, msgFail
        invoke  SysMsgBoardStr
     end if
.exit:
        xor     eax, eax
        ret

.out_of_mem:
     if DEBUG
        mov     esi, msgMem
        invoke  SysMsgBoardStr
     end if
        xor     eax, eax
        ret
endp

align 4

srv_calls  dd service_proc.srv_getversion       ; 0
           dd service_proc.snd_create_buff      ; 1
           dd service_proc.snd_destroy_buff     ; 2
           dd service_proc.snd_setformat        ; 3
           dd service_proc.snd_getformat        ; 4
           dd service_proc.snd_reset            ; 5
           dd service_proc.snd_setpos           ; 6
           dd service_proc.snd_getpos           ; 7
           dd service_proc.snd_setbuff          ; 8
           dd service_proc.snd_out              ; 9
           dd service_proc.snd_play             ; 10
           dd service_proc.snd_stop             ; 11
           dd service_proc.snd_setvolume        ; 12
           dd service_proc.snd_getvolume        ; 13
           dd service_proc.snd_setpan           ; 14
           dd service_proc.snd_getpan           ; 15
           dd service_proc.snd_getbuffsize      ; 16
           dd service_proc.snd_getfreespace     ; 17
           dd service_proc.snd_settimebase      ; 18
           dd service_proc.snd_gettimestamp     ; 19
srv_calls_end:

proc service_proc stdcall, ioctl:dword

        mov     edi, [ioctl]
        mov     eax, [edi+IOCTL.io_code]

        cmp     eax, (srv_calls_end-srv_calls)/4
        ja      .fail

        cmp     eax, SND_DESTROY_BUFF
        jb      @F

;           cmp [edi+inp_size], 4
;           jb .fali

        mov     ebx, [edi+IOCTL.input]
        mov     edx, [ebx]

        cmp     [edx+STREAM.magic], 'WAVE'
        jne     .fail

        cmp     [edx+STREAM.size], STREAM.sizeof
        jne     .fail

@@:
        jmp     [srv_calls+eax*4]


.fail:
        mov     eax, -1
        ret

align 4
.srv_getversion:
        mov     eax, [edi+IOCTL.output]
        cmp     [edi+IOCTL.out_size], 4
        jne     .fail
        mov     eax, [eax]
        mov     [eax], dword API_VERSION
        xor     eax, eax
        ret

align 4
.snd_create_buff:
        mov     ebx, [edi+IOCTL.input]
        stdcall CreateBuffer, [ebx], [ebx+4]
        mov     edi, [ioctl]
        mov     ecx, [edi+IOCTL.output]
        mov     ecx, [ecx]
        mov     [ecx], ebx
        ret

align 4
.snd_destroy_buff:
        mov     eax, edx
        call    DestroyBuffer
        ret

align 4
.snd_setformat:
        stdcall SetFormat, edx, [ebx+4]
        ret

align 4
.snd_getformat:
        movzx   eax, word [edx+STREAM.format]
        mov     ecx, [edi+IOCTL.output]
        mov     ecx, [ecx]
        mov     [ecx], eax
        xor     eax, eax
        ret

align 4
.snd_reset:
        stdcall ResetBuffer, edx, [ebx+4]
        ret

align 4
.snd_setpos:
        stdcall SetBufferPos, edx, [ebx+4]
        ret

align 4
.snd_getpos:
        stdcall GetBufferPos, edx
        mov     edi, [ioctl]
        mov     ecx, [edi+IOCTL.output]
        mov     ecx, [ecx]
        mov     [ecx], ebx
        ret

align 4
.snd_setbuff:
        mov     eax, [ebx+4]
        stdcall set_buffer, edx, eax, [ebx+8], [ebx+12]
        ret

align 4
.snd_out:
        mov     eax, [ebx+4]
        stdcall wave_out, edx, eax, [ebx+8]
        ret

align 4
.snd_play:
        stdcall play_buffer, edx, [ebx+4]
        ret

align 4
.snd_stop:
        stdcall stop_buffer, edx
        ret

align 4
.snd_setvolume:
        stdcall SetBufferVol, edx, [ebx+4], [ebx+8]
        ret

align 4
.snd_getvolume:
        mov     eax, [edi+IOCTL.output]
        mov     ecx, [eax]
        mov     eax, [eax+4]
        stdcall GetBufferVol, edx, ecx, eax
        ret
align 4
.snd_setpan:
        stdcall SetBufferPan, edx, [ebx+4]
        ret

align 4
.snd_getpan:
        mov     eax, [edx+STREAM.pan]
        mov     ebx, [edi+IOCTL.output]
        mov     ebx, [ebx]
        mov     [ebx], eax
        xor     eax, eax
        ret

align 4
.snd_getbuffsize:
        test    [edx+STREAM.format], PCM_RING
        mov     eax, [edx+STREAM.in_size]
        jz      @F

        mov     eax, [edx+STREAM.r_size]
        add     eax, eax
@@:
        mov     ecx, [edi+IOCTL.output]
        mov     ecx, [ecx]
        mov     [ecx], eax
        xor     eax, eax
        ret

align 4
.snd_getfreespace:
        test    [edx+STREAM.format], PCM_OUT
        jz      .fail

        mov     ebx, [edx+STREAM.in_free]
        mov     ecx, [edi+IOCTL.output]
        mov     [ecx], ebx
        xor     eax, eax
        ret
align 4
.snd_settimebase:
        cmp     [edi+IOCTL.inp_size], 12
        jne     .fail

        mov     eax, [ebx+4]
        mov     ebx, [ebx+8]

        pushfd
        cli
        mov     dword [edx+STREAM.time_base], eax
        mov     dword [edx+STREAM.time_base+4], ebx
        xor     eax, eax
        mov     dword [edx+STREAM.time_stamp], eax
        mov     dword [edx+STREAM.time_stamp+4], eax
        popfd

        ret

align 4
.snd_gettimestamp:
        cmp     [edi+IOCTL.out_size], 8
        jne     .fail

        pushfd
        cli

        xor     ebx, ebx
        push    48
        push    ebx            ; local storage

        cmp     [edx+STREAM.flags], SND_STOP
        je      @F

        mov     eax, esp

        push    ebx
        push    ecx
        push    edx
        push    esi
        push    edi

        push    4              ;.out_size
        push    eax            ;.output
        push    ebx            ;.inp_size
        push    ebx            ;.input
        push    DEV_GET_POS    ;.code
        push    dword [hSound] ;.handle
        mov     eax, esp

        invoke  ServiceHandler, eax
        add     esp, 6*4

        pop     edi
        pop     esi
        pop     edx
        pop     ecx
        pop     ebx

        test    eax, eax
        jz      @F

        mov     dword [esp], 0  ; clear offset
@@:
        mov     edi, [edi+IOCTL.output]

        emms
        fild    qword [edx+STREAM.time_stamp]
        fiadd   dword [esp]     ; primary buffer offset
        fidiv   dword [esp+4]   ; total_samples / frequency
        fadd    qword [edx+STREAM.time_base]
        fstp    qword [edi]
        add     esp, 8

        popfd

        xor     eax, eax
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
        str     dd ?
             ring_size   dd ?
             ring_pages  dd ?
           endl

        mov     eax, [format]
        cmp     ax, PCM_1_8_8
        ja      .fail

        test    eax, PCM_OUT
        jnz     .test_out
        test    eax, PCM_RING
        jnz     .test_ring
;staic
        test    eax, PCM_STATIC
        jz      .test_out                 ;use PCM_OUT as default format
        jmp     .test_ok
.test_out:
        test    eax, PCM_RING+PCM_STATIC
        jnz     .fail
        or      [format], PCM_OUT         ;force set
        jmp     .test_ok
.test_ring:
        test    eax, PCM_OUT+PCM_STATIC
        jnz     .fail
.test_ok:

        invoke  GetPid
        mov     ebx, eax
        mov     eax, STREAM.sizeof

        invoke  CreateObject
        test    eax, eax
        jz      .fail
        mov     [str], eax

        mov     ebx, [format]
        mov     [eax+STREAM.format], ebx

        xor     ecx, ecx
        movzx   ebx, bx
        cmp     ebx, 19
        jb      @f
        mov     ecx, 0x80808080
@@:
        mov     [eax+STREAM.r_silence], ecx

        shl     ebx, 2
        lea     ebx, [ebx+ebx*2]    ;ebx*=12

        mov     ecx, [resampler_params+ebx]
        mov     edx, [resampler_params+ebx+4]
        mov     esi, [resampler_params+ebx+8]

        mov     [eax+STREAM.r_size], ecx
        mov     [eax+STREAM.r_dt], edx
        mov     [eax+STREAM.resample], esi
        xor     ecx, ecx
        mov     [eax+STREAM.l_vol], ecx
        mov     [eax+STREAM.r_vol], ecx
        mov     dword [eax+STREAM.l_amp], 0x7FFF7FFF
        mov     [eax+STREAM.pan], ecx

        test    [format], PCM_STATIC
        jnz     .static

; ring and waveout

        mov     ebx, 0x10000
        test    [format], PCM_RING
        jz      .waveout

        mov     ebx, [eax+STREAM.r_size]
        add     ebx, 4095
        and     ebx, -4096
        add     ebx, ebx
.waveout:
        mov     [ring_size], ebx
        mov     eax, ebx
        shr     ebx, 12
        mov     [ring_pages], ebx

        invoke  CreateRingBuffer, eax, PG_SW

        mov     edi, [str]
        mov     ecx, [ring_size]
        mov     [edi+STREAM.in_base], eax
        mov     [edi+STREAM.in_size], ecx
        add     eax, 128
        mov     [edi+STREAM.in_wp], eax
        mov     [edi+STREAM.in_rp], eax
        mov     [edi+STREAM.in_count], 0

        mov     [edi+STREAM.in_free], ecx
        add     eax, ecx
        mov     [edi+STREAM.in_top], eax

        jmp     .out_buff
.static:
        mov     ecx, [size]
        add     ecx, 128         ;resampler required
        mov     [eax+STREAM.in_size], ecx
        invoke  KernelAlloc, ecx

        mov     edi, [str]
        mov     [edi+STREAM.in_base], eax
        add     eax, 128
        mov     [edi+STREAM.in_wp], eax
        mov     [edi+STREAM.in_rp], eax
        mov     ebx, [size]
        mov     [edi+STREAM.in_count], ebx
        mov     [edi+STREAM.in_free], ebx
        add     eax, ebx
        mov     [edi+STREAM.in_top], eax

.out_buff:
        invoke  AllocKernelSpace, dword 128*1024

        mov     edi, [str]
        xor     ebx, ebx

        mov     [edi+STREAM.out_base], eax
        mov     [edi+STREAM.out_wp], eax
        mov     [edi+STREAM.out_rp], eax
        mov     [edi+STREAM.out_count], ebx
        add     eax, 64*1024
        mov     [edi+STREAM.out_top], eax

        mov     dword [edi+STREAM.time_base], ebx
        mov     dword [edi+STREAM.time_base+4], ebx

        mov     dword [edi+STREAM.time_stamp], ebx
        mov     dword [edi+STREAM.time_stamp+4], ebx
        mov     dword [edi+STREAM.last_ts], ebx

        invoke  AllocPages, dword 64/4
        mov     edi, [str]
        mov     ebx, [edi+STREAM.out_base]
        mov     ecx, 16
        or      eax, PG_SW
        push    eax
        push    ebx
        invoke  CommitPages ;eax, ebx, ecx
        mov     ecx, 16
        pop     ebx
        pop     eax
        add     ebx, 64*1024
        invoke  CommitPages    ;double mapped

        mov     edi, [str]
        mov     ecx, [edi+STREAM.in_top]
        mov     edi, [edi+STREAM.in_base]
        sub     ecx, edi
        xor     eax, eax
        shr     ecx, 2
        cld
        rep stosd

        mov     edi, [str]
        mov     edi, [edi+STREAM.out_base]
        mov     ecx, (64*1024)/4
        rep stosd

        xor     esi, esi
        mov     ecx, MANUAL_DESTROY
        invoke  CreateEvent

        mov     ebx, [str]
        mov     [ebx+STREAM.notify_event], eax
        mov     [ebx+STREAM.notify_id], edx

        mov     [ebx+STREAM.magic], 'WAVE'
        mov     [ebx+STREAM.destroy], DestroyBuffer.destroy
        mov     [ebx+STREAM.size], STREAM.sizeof
        mov     [ebx+STREAM.flags], SND_STOP

        pushf
        cli
        mov     eax, str.fd-FD_OFFSET
        mov     edx, [eax+STREAM.str_fd]
        mov     [ebx+STREAM.str_fd], edx
        mov     [ebx+STREAM.str_bk], eax
        mov     [eax+STREAM.str_fd], ebx
        mov     [edx+STREAM.str_bk], ebx
        popf

        xor     eax, eax
        ret
.fail:
        xor     ebx, ebx
        or      eax, -1
        ret
endp

;param
; eax= buffer handle

align 4
DestroyBuffer:
           .handle  equ esp       ;local

        mov     [eax+STREAM.flags], SND_STOP
.destroy:
        push    eax

        pushfd
        cli
        mov     ebx, [eax+STREAM.str_fd]
        mov     ecx, [eax+STREAM.str_bk]
        mov     [ebx+STREAM.str_bk], ecx
        mov     [ecx+STREAM.str_fd], ebx
        popf

        invoke  KernelFree, [eax+STREAM.in_base]
        mov     eax, [.handle]
        invoke  KernelFree, [eax+STREAM.out_base]

        pop     eax              ;restore stack
        invoke  DestroyObject    ;eax= stream
        xor     eax, eax
        ret
.fail:
        or      eax, -1
        ret
restore .handle

align 4
proc SetFormat stdcall, str:dword, format:dword

        cmp     word [format], PCM_1_8_8
        ja      .fail

        mov     edx, [str]
        mov     [edx+STREAM.flags], SND_STOP

        test    [edx+STREAM.format], PCM_RING
        jnz     .fail

;           mov eax,[edx+STREAM.out_base]
;           mov [edx+STREAM.out_wp], eax
;           mov [edx+STREAM.out_rp], eax
;           mov [edx+STREAM.out_count], 0

        movzx   eax, word [format]
        mov     word [edx+STREAM.format], ax

        xor     ebx, ebx
        cmp     eax, 19
        jb      @f
        mov     ebx, 0x80808080
@@:
        mov     [edx+STREAM.r_silence], ebx

        shl     eax, 2
        lea     eax, [eax+eax*2]    ;eax*=12

        mov     edi, [resampler_params+eax]
        mov     ecx, [resampler_params+eax+4]
        mov     ebx, [resampler_params+eax+8]

        mov     [edx+STREAM.r_size], edi
        mov     [edx+STREAM.r_dt], ecx
        mov     [edx+STREAM.resample], ebx

        mov     edi, [edx+STREAM.in_base]
        mov     ecx, 128/4
        mov     eax, [edx+STREAM.r_silence]
        cld
        rep stosd
        xor     eax, eax
        ret
.fail:
        or      eax, -1
        ret
endp

; for static buffers only
; use waveout for streams

align 4
proc set_buffer stdcall, str:dword,src:dword,offs:dword,size:dword

        mov     edx, [str]
        test    [edx+STREAM.format], PCM_OUT
        jnz     .fail

        mov     esi, [src]
        mov     edi, [offs]
        add     edi, [edx+STREAM.in_base]
        add     edi, 128

        cmp     edi, [edx+STREAM.in_top]
        jae     .fail

        mov     ecx, [size]
        lea     ebx, [ecx+edi]
        sub     ebx, [edx+STREAM.in_top]
        jb      @F
        sub     ecx, ebx
@@:
        shr     ecx, 2
        cld
        rep movsd
        xor     eax, eax
        ret
.fail:
        or      eax, -1
        ret
endp

; for stream buffers only

align 4
proc wave_out stdcall, str:dword,src:dword,size:dword
           locals
             state_saved  dd ?
             fpu_state    rb 528
           endl

        mov     edx, [str]
        mov     eax, [edx+STREAM.format]
        test    eax, PCM_OUT
        jz      .fail

        cmp     ax, PCM_ALL
        je      .fail

        mov     esi, [src]
        test    esi, esi
        jz      .fail

        cmp     esi, OS_BASE
        jae     .fail

        mov     [state_saved], 0

.main_loop:
        mov     edx, [str]

        mov     ebx, [size]
        test    ebx, ebx
        jz      .done

        cmp     [edx+STREAM.flags], SND_STOP
        jne     .fill

        mov     edi, [edx+STREAM.in_base]
        mov     ecx, 128/4
        mov     eax, [edx+STREAM.r_silence]
        cld
        rep stosd

        mov     ecx, [edx+STREAM.in_size]
        sub     ecx, 128
        mov     [edx+STREAM.in_wp], edi
        mov     [edx+STREAM.in_rp], edi
        mov     [edx+STREAM.in_count], 0
        mov     [edx+STREAM.in_free], ecx

        mov     eax, [edx+STREAM.out_base]
        mov     [edx+STREAM.out_wp], eax
        mov     [edx+STREAM.out_rp], eax
        mov     [edx+STREAM.out_count], 0
.fill:
        cli

        mov     ecx, [edx+STREAM.in_free]
        test    ecx, ecx
        jz      .wait

        cmp     ecx, ebx
        jbe     @F

        mov     ecx, ebx
@@:
        sub     [size], ecx
        add     [edx+STREAM.in_count], ecx
        sub     [edx+STREAM.in_free], ecx

        shr     ecx, 2
        mov     edi, [edx+STREAM.in_wp]
        mov     esi, [src]
        cld
        rep movsd

        mov     [src], esi
        cmp     edi, [edx+STREAM.in_top]
        jb      @F
        sub     edi, [edx+STREAM.in_size]
@@:
        mov     [edx+STREAM.in_wp], edi

        cmp     [edx+STREAM.out_count], 32768
        jae     .skip

        cmp     [state_saved], 0
        jne     @F
        lea     eax, [fpu_state+15]
        and     eax, -16
        invoke  FpuSave
        mov     [state_saved], 1
@@:
        stdcall refill, edx

.skip:
        sti
        mov     edx, [str]
        mov     [edx+STREAM.flags], SND_PLAY
        cmp     [eng_state], SND_PLAY
        je      .main_loop

        stdcall dev_play, [hSound]
        mov     [eng_state], SND_PLAY
        jmp     .main_loop
.wait:
        sti
        mov     edx, [str]
        mov     eax, [edx+STREAM.notify_event]
        mov     ebx, [edx+STREAM.notify_id]
        invoke  WaitEvent   ;eax ebx
        jmp     .main_loop
.done:
        cmp     [state_saved], 1
        jne     @F

        lea     eax, [fpu_state+15]
        and     eax, -16
        invoke  FpuRestore
@@:
        xor     eax, eax
        ret
.fail:
        or      eax, -1
        ret
endp

; both static and stream
; reset all but not clear buffers


; flags reserved
;  RESET_INPUT  equ 1   ;reset and clear input buffer
;  RESET_OUTPUT equ 2   ;reset and clear output buffer
;  RESET_ALL    equ 3


align 4
proc ResetBuffer stdcall, str:dword, flags:dword

        mov     edx, [str]
        mov     [edx+STREAM.flags], SND_STOP

        mov     edi, [edx+STREAM.in_base]
        mov     ecx, 128/4
        mov     eax, [edx+STREAM.r_silence]
        cld
        rep stosd

        mov     [edx+STREAM.in_wp], edi
        mov     [edx+STREAM.in_rp], edi

        test    [edx+STREAM.flags], PCM_STATIC
        jnz     .static
        mov     [edx+STREAM.in_count], 0
        jmp     @F
.static:
        mov     eax, [edx+STREAM.in_size]
        mov     [edx+STREAM.in_count], eax
@@:

        mov     eax, [edx+STREAM.in_size]
        sub     eax, 128
        mov     [edx+STREAM.in_free], eax

        xor     eax, eax
        mov     ebx, [edx+STREAM.out_base]
        mov     [edx+STREAM.out_wp], ebx
        mov     [edx+STREAM.out_rp], ebx
        mov     [edx+STREAM.out_count], eax

        mov     dword [edx+STREAM.time_base], eax
        mov     dword [edx+STREAM.time_base+4], eax

        mov     dword [edx+STREAM.time_stamp], eax
        mov     dword [edx+STREAM.time_stamp+4], eax
        mov     dword [edx+STREAM.last_ts], eax


        mov     eax, [edx+STREAM.r_silence]
        test    [flags], 1
        jz      @F

        mov     ecx, [edx+STREAM.in_top]
        mov     edi, [edx+STREAM.in_base]
        sub     ecx, edi
        shr     ecx, 2
        cld
        rep stosd
@@:
        test    [flags], 2
        jz      @F

        mov     edi, [edx+STREAM.out_base]
        mov     ecx, (64*1024)/4
        rep stosd
@@:
        ret
.fail:
        or      eax, -1
        ret
endp

; for static buffers only

align 4
proc SetBufferPos stdcall, str:dword, pos:dword

        mov     edx, [str]
        test    [edx+STREAM.format], PCM_STATIC
        jz      .fail

        mov     [edx+STREAM.flags], SND_STOP

        mov     eax, [pos]
        add     eax, [edx+STREAM.in_base]
        mov     ebx, [edx+STREAM.in_top]
        add     eax, 128

        cmp     eax, ebx
        jae     .fail

        mov     [edx+STREAM.in_rp], eax
        sub     ebx, eax
        mov     [edx+STREAM.in_count], ebx
        xor     eax, eax
        ret
.fail:
        or      eax, -1
        ret
endp

align 4
proc GetBufferPos stdcall, str:dword

        mov     edx, [str]
        test    [edx+STREAM.format], PCM_STATIC
        jz      .fail

        mov     ebx, [edx+STREAM.in_rp]
        sub     ebx, [edx+STREAM.in_base]
        sub     ebx, 128
        xor     eax, eax
        ret
.fail:
        xor     ebx, ebx
        or      eax, -1
        ret
endp

; both

align 4
proc SetBufferVol stdcall, str:dword,l_vol:dword,r_vol:dword

        mov     edx, [str]
        stdcall set_vol_param, [l_vol], [r_vol], [edx+STREAM.pan]
        ret
endp


proc minw stdcall, arg1:dword, arg2:dword
        mov     ax, word [arg1]
        cmp     ax, word [arg2]
        jle     @f
        mov     eax, [arg2]
@@:
        ret
endp

proc maxw stdcall, arg1:dword, arg2:dword
        mov     ax, word [arg1]
        cmp     ax, word [arg2]
        jge     @f
        mov     eax, [arg2]
@@:
        ret
endp


proc set_vol_param stdcall, l_vol:dword,r_vol:dword,pan:dword
           locals
             _600    dd ?
             _32767  dd ?
             state   rb 108
           endl

        mov     [_600], 0x44160000  ;600.0
        mov     [_32767], 32767

        lea     ebx, [state]
        fnsave  [ebx]

        stdcall minw, [l_vol], [vol_max]
        stdcall maxw, eax, [vol_min]
        mov     [l_vol], eax
        mov     [edx+STREAM.l_vol], eax
        stdcall minw, [r_vol], [vol_max+4]
        stdcall maxw, eax, [vol_min+4]
        mov     [r_vol], eax
        mov     [edx+STREAM.r_vol], eax

        stdcall minw, [pan], [pan_max]
        stdcall maxw, eax, [vol_min]
        mov     [edx+STREAM.pan], eax

        cmp     word [edx+STREAM.pan], 0
        jl      @f

        mov     ebx, [l_vol]
        sub     ebx, eax
        stdcall minw, ebx, [vol_max]
        stdcall maxw, eax, [vol_min]
        mov     [l_vol], eax
        jmp     .calc_amp
@@:
        mov     ebx, [r_vol]
        add     ebx, [pan]
        stdcall minw, ebx, [vol_max+4]
        stdcall maxw, eax, [vol_min+4]
        mov     [r_vol], eax
.calc_amp:
        emms
        fild    word [l_vol]

        call    .calc

        fistp   word [edx+STREAM.l_amp]
        fstp    dword [edx+STREAM.l_amp_f]
        fstp    st0

        fild    word [r_vol]

        call    .calc

        fistp   word [edx+STREAM.r_amp]
        fstp    dword [edx+STREAM.r_amp_f]
        fstp    st0

        fnclex
        lea     ebx, [state]
        frstor  [ebx]

        xor     eax, eax
        inc     eax
        ret
.calc:
        fdiv    dword [_600]
        fld     st0
        frndint
        fxch    st1
        fsub    st, st1
        f2xm1
        fld1
        faddp   st1, st0
        fscale
        fld     st0
        fimul   dword [_32767]
        ret     0
endp


align 4
proc GetBufferVol stdcall, str:dword,p_lvol:dword,p_rvol:dword

        mov     edx, [str]
        mov     eax, [p_lvol]
        movsx   ecx, word [edx+STREAM.l_vol]
        mov     [eax], ecx

        mov     eax, [p_rvol]
        movsx   ecx, word [edx+STREAM.r_vol]
        mov     [eax], ecx
        xor     eax, eax
        ret
endp

align 4
proc SetBufferPan stdcall, str:dword,pan:dword

        mov     edx, [str]
        stdcall set_vol_param, [edx+STREAM.l_vol], \
                [edx+STREAM.r_vol],[pan]
        ret
endp

; for static and ring buffers only

align 4
proc play_buffer stdcall, str:dword, flags:dword

        mov     ebx, [str]
        mov     eax, [ebx+STREAM.format]
        test    eax, PCM_OUT
        jnz     .fail

        cmp     ax, PCM_ALL
        je      .fail

        mov     [ebx+STREAM.flags], SND_PLAY
        cmp     [eng_state], SND_PLAY
        je      .done

        stdcall dev_play, [hSound]
        mov     [eng_state], SND_PLAY
.done:
        test    [flags], PLAY_SYNC
        jz      @F

        mov     edx, [str]
.wait:
        mov     eax, [edx+STREAM.notify_event]
        mov     ebx, [edx+STREAM.notify_id]
        invoke  WaitEvent   ;eax ebx

        mov     edx, [str]
        cmp     [edx+STREAM.flags], SND_STOP
        jne     .wait
@@:
        xor     eax, eax
        ret
.fail:
        or      eax, -1
        ret
endp

; for static and ring buffers only

align 4
proc stop_buffer stdcall, str:dword

        mov     edx, [str]
        test    [edx+STREAM.format], PCM_STATIC+PCM_RING
        jz      .fail

        mov     [edx+STREAM.flags], SND_STOP

        mov     eax, [edx+STREAM.notify_event]
        mov     ebx, [edx+STREAM.notify_id]
        invoke  ClearEvent   ;eax ebx

        xor     eax, eax
        ret
.fail:
        or      eax, -1
        ret
endp

; param
;  eax= mix_list

align 4
do_mix_list:

        xor     edx, edx
        mov     esi, str.fd-FD_OFFSET
        mov     ebx, [esi+STREAM.str_fd]
@@:
        cmp     ebx, esi
        je      .done

        cmp     [ebx+STREAM.magic], 'WAVE'
        jne     .next

        cmp     [ebx+STREAM.size], STREAM.sizeof
        jne     .next

        cmp     [ebx+STREAM.flags], SND_PLAY;
        jne     .next

        mov     ecx, [ebx+STREAM.out_count]
        test    ecx, ecx
        jnz     .l1

        test    [ebx+STREAM.format], PCM_RING
        jnz     .next
        mov     [ebx+STREAM.flags], SND_STOP
        jmp     .next
.l1:
        cmp     ecx, 512
        jae     .add_buff

        mov     edi, [ebx+STREAM.out_rp]
        add     edi, ecx
        sub     ecx, 512
        neg     ecx
        push    eax
        xor     eax, eax
        cld
        rep stosb
        pop     eax

        mov     [ebx+STREAM.out_count], 512

.add_buff:
        mov     ecx, [ebx+STREAM.out_rp]
        mov     [eax], ecx

if USE_SSE2_MIXER
        mov     edi, dword [ebx+STREAM.l_amp_f]
        mov     [eax+4], edi
        mov     edi, dword [ebx+STREAM.r_amp_f]
        mov     [eax+8], edi
else
        mov     edi, dword [ebx+STREAM.l_amp]
        mov     [eax+4], edi
end if
        add     [ebx+STREAM.out_rp], 512
        sub     [ebx+STREAM.out_count], 512

        add     eax, 12
        inc     edx
.next:
        mov     ebx, [ebx+STREAM.str_fd]
        jmp     @B
.done:
        mov     eax, edx
        ret

align 4
prepare_playlist:

        xor     edx, edx
        mov     [play_count], edx
        mov     esi, str.fd-FD_OFFSET
        mov     edi, [esi+STREAM.str_fd]
@@:
        cmp     edi, esi
        je      .done

        cmp     [edi+STREAM.magic], 'WAVE'
        jne     .next

        cmp     [edi+STREAM.size], STREAM.sizeof
        jne     .next

        cmp     [edi+STREAM.flags], SND_PLAY;
        jne     .next

        mov     [play_list+edx], edi
        inc     [play_count]
        add     edx, 4
.next:
        mov     edi, [edi+STREAM.str_fd]
        jmp     @B
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

        mov     eax, [hsrv]
        lea     ecx, [handler_proc]
        xor     ebx, ebx

        mov     [handler], eax
        mov     [io_code], DEV_CALLBACK
        mov     [input], ecx
        mov     [inp_size], 4
        mov     [output], ebx
        mov     [out_size], 0

        lea     eax, [handler]
        invoke  ServiceHandler, eax
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

        mov     eax, [hsrv]
        xor     ebx, ebx

        mov     [handle], eax
        mov     [io_code], DEV_PLAY
        mov     [input], ebx
        mov     [inp_size], ebx
        mov     [output], ebx
        mov     [out_size], ebx

        lea     eax, [handle]
        invoke  ServiceHandler, eax
        ret
endp

if 0
align 4
dword2str:
        mov     esi, hex_buff
        mov     ecx, -8
@@:
        rol     eax, 4
        mov     ebx, eax
        and     ebx, 0x0F
        mov     bl, [ebx+hexletters]
        mov     [8+esi+ecx], bl
        inc     ecx
        jnz     @B
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
     dd  8192,      0, m16_stereo     ; 2  PCM_1_16_48

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

align 4
data fixups
end data

section '.data' data readable writable

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
