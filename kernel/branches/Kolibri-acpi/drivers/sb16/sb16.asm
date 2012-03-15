;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

include 'CONFIG.INC'

;structs----------------------------------------------------------
struc IOCTL
{  .handle      dd ?
   .io_code     dd ?
   .input       dd ?
   .inp_size    dd ?
   .output      dd ?
   .out_size    dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

;something--------------------------------------------------------
public START
public service_proc
public version

include '..\proc32.inc'
include '..\imports.inc'

section '.flat' code readable align 16

include 'SB16.INC'

;-------------------------------------------------------------------------------
proc START stdcall, state:dword
        cmp     [state], 1
        jne     .stop
.entry:

if DEBUG
        mov     esi, msgInit
        call    SysMsgBoardStr
end if

        call    detect           ;returns DSP version or zero if
        test    eax, eax         ;SB card not found
        jz      .exit

if DEBUG
        movzx   eax, al          ;major version
        mov     esi, sb_DSP_description
        dec     eax
        jz      .sb_say_about_found_dsp
        mov     dword[esi], '2.x '
        dec     eax
        jz      .sb_say_about_found_dsp
        mov     dword[esi], 'Pro '
        dec     eax
        jz      .sb_say_about_found_dsp
        mov     dword[esi], '16  '
.sb_say_about_found_dsp:
        mov     esi, msgDSPFound
        call    SysMsgBoardStr
end if
;          xor  eax,eax
;          mov  ebx,[sb_base_port]
;          lea  ecx,[ebx+0xF]
        xor     ebx, ebx
        mov     ecx, [sb_base_port]
        lea     edx, [ebx+0xF]

        call    ReservePortArea  ;these ports must be my!
if DEBUG
        dec     eax
        jnz     @f
        mov     esi, msgErrRsrvPorts
        call    SysMsgBoardStr
@@:
end if

        call    sb_setup         ;clock it, etc

        stdcall AttachIntHandler, sb_irq_num, sb_irq, 0

if DEBUG
        test    eax, eax
        jnz     @f

        mov     esi, msgErrAtchIRQ
        call    SysMsgBoardStr

        stdcall GetIntHandler, sb_irq_num
        call    SysMsgBoardNum

        jmp     .stop
@@:
        mov     esi, msgSucAtchIRQ
        call    SysMsgBoardStr
end if
        stdcall RegService, my_service, service_proc
        ret
.stop:
        call    sb_reset
.exit:

if DEBUG
        mov     esi, msgExit
        call    SysMsgBoardStr
end if

        xor     eax, eax
        ret
endp
;-------------------------------------------------------------------------------

handle     equ  IOCTL.handle
io_code    equ  IOCTL.io_code
input      equ  IOCTL.input
inp_size   equ  IOCTL.inp_size
output     equ  IOCTL.output
out_size   equ  IOCTL.out_size

align 4
proc service_proc stdcall, ioctl:dword
        mov     edi, [ioctl]
        mov     eax, [edi+io_code]
        cmp     eax, SRV_GETVERSION
        jne     @F

        mov     eax, [edi+output]
        cmp     [edi+out_size], 4
        jne     .fail
        mov     [eax], dword API_VERSION
        xor     eax, eax
        ret
@@:
        cmp     eax, DEV_PLAY
        jne     @f
if DEBUG
        mov     esi, msgPlay
        call    SysMsgBoardStr
end if
        call    sb_stop          ;to play smth new we must stop smth old

        call    pre_fill_data    ;fill first and second half of the buffer
        call    pre_fill_data    ;

        call    sb_set_dma       ;is it really needed here? Paranoia.
        call    sb_play
        xor     eax, eax          ;set maximum volume
        call    sb_set_master_vol
        xor     eax, eax
        ret
;@@:                             ;all this commented stuff in service proc
;           cmp  eax,DEV_STOP    ;is never used. Mixer do this virtually,
;           jne  @f              ;e.g. instead of stopping driver it
;if DEBUG                        ;outputs silence
;           mov  esi,msgStop
;           call SysMsgBoardStr
;end if
;           call sb_stop
;           xor  eax,eax
;           ret
@@:
        cmp     eax, DEV_CALLBACK
        jne     @f
if DEBUG
        mov     esi, msgCallback
        call    SysMsgBoardStr
end if
        mov     edi, [edi+input]
        mov     eax, [edi]
        mov     [callback], eax
if DEBUG
        call    SysMsgBoardNum
end if
        xor     eax, eax
        ret
@@:
        cmp     eax, DEV_SET_MASTERVOL;Serge asked me to unlock
        jne     @F ;DEV_SET(GET)_MASTERVOL, although mixer doesn't use it.
           ;It doesn't use it _in current version_ - but in the future...

if DEBUG
        mov     esi, msgSetVol
        call    SysMsgBoardStr
end if
        mov     eax, [edi+input]
        mov     eax, [eax]
        call    sb_set_master_vol
        xor     eax, eax
        ret
@@:
        cmp     eax, DEV_GET_MASTERVOL
        jne     @F
if DEBUG
        mov     esi, msgGetVol
        call    SysMsgBoardStr
end if
        mov     eax, [edi+output]
        mov     edx, [sb_master_vol]
        mov     [eax], edx
        xor     eax, eax
        ret

.fail:
        or      eax, -1
        ret
endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size

;-------------------------------------------------------------------------------
align 4
proc sb_irq
        mov     edx, [sb_base_port];tell the DSP that we have processed IRQ
        add     dl, 0xF            ;0xF for 16 bit sound, 0xE for 8 bit sound
        in      al, dx             ;for non-stop sound

pre_fill_data:
        mov     eax, int_flip_flop
        not     dword[eax]
        mov     eax, [eax]
        test    eax, eax
        jns     .fill_second_half

if sb_buffer_size eq small_buffer
        stdcall [callback], SB16Buffer0   ;for 32k buffer
else if sb_buffer_size eq full_buffer
        stdcall [callback], SB16Buffer0   ;for 64k buffer
        stdcall [callback], SB16Buffer1   ;for 64k buffer
end if
        xor     eax, eax
        not     eax
        ret

.fill_second_half:
if sb_buffer_size eq small_buffer
        stdcall [callback], SB16Buffer1   ;for 32k buffer
else if sb_buffer_size eq full_buffer
        stdcall [callback], SB16Buffer2   ;for 64k buffer
        stdcall [callback], SB16Buffer3   ;for 64k buffer
end if
        xor     eax, eax
        not     eax
        ret
endp
;-------------------------------------------------------------------------------
align 4
proc detect
.sb_detect_next_port:
if DEBUG
        inc     dword[port_second_digit_num]
end if
        mov     edx, sb_base_port
        add     byte[edx], 10h
        cmp     byte[edx], 80h
        jbe     .sb_try_to_detect_at_specified_port
;error - no SB card detected
.sb_not_found_err:
        xor     eax, eax
        ret

.sb_try_to_detect_at_specified_port:
        call    sb_reset
        add     dl, 8
        mov     ecx, 100
.sb_check_port:
        in      al, dx
        test    al, al           ;is DSP port ready to be read?
        jns     .sb_port_not_ready

        sub     dl, 4
        in      al, dx           ;check for AAh response
        add     dl, 4
        cmp     al, 0xAA
        jne     .sb_port_not_ready
.sb_card_found:
        and     dl, 0xF0
        add     dl, 0xC
           sb_out 0xE1           ;get DSP version
        add     dl, 2
@@:
        in      al, dx
        test    al, al           ;is DSP port ready to be read?
        jns     @b
        sub     dl, 4
        in      al, dx           ;get major version
        ror     eax, 16
        add     dl, 4
@@:
        in      al, dx
        test    al, al           ;is DSP port ready to be read?
        jns     @b
        sub     dl, 4
        in      al, dx           ;get minor version
        xor     edx, edx
        mov     dl, 10
        div     dl
        ror     eax, 16
        xor     ah, ah
        mov     [sb_DSP_version_int], eax;for internal usage
if DEBUG
        add     [sb_DSP_version], eax
end if
        ret

.sb_port_not_ready:
        loop    .sb_check_port   ;100 retries (~100 microsec.)
        jmp     .sb_detect_next_port
endp
;-------------------------------------------------------------------------------
if DEBUG
proc SysMsgBoardNum ;warning: destroys eax,ebx,ecx,esi
        mov     ebx, eax
        mov     ecx, 8
        mov     esi, (number_to_out+1)
.1:
        mov     eax, ebx
        and     eax, 0xF
        add     al, '0'
        cmp     al, (10+'0')
        jb      @f
        add     al, ('A'-'0'-10)
@@:
        mov     [esi+ecx], al
        shr     ebx, 4
        loop    .1
        dec     esi
        call    SysMsgBoardStr
        ret
endp
end if
;all initialized data place here
align 4
version       dd (5 shl 16) or (API_VERSION and 0xFFFF)

sb_base_port:
              dd 200h ;don't ask me why - see the code&docs

sound_dma     dd sb_dma_num

;note that 4th DMA channel doesn't exist, it is used for cascade
;plugging the first DMA controler to the second
dma_table     db 0x87,0x83,0x81,0x82,0xFF,0x8B,0x89,0x8A

my_service    db 'SOUND',0  ;max 16 chars include zero

if DEBUG
number_to_out db '0x00000000',13,10,0

msgInit       db 'detecting hardware...',13,10,0
msgExit       db 'exiting... May be some problems found?',13,10,0
msgPlay       db 'start play',13,10,0
;msgStop       db 'stop play',13,10,0
msgCallback   db 'set_callback received from the mixer!',13,10
              db 'callback handler is: ',0
msgErrAtchIRQ db 'failed to attach IRQ',(sb_irq_num+'0'),13,10
              db 'owner',39,'s handler: ',0
msgSucAtchIRQ db 'succesfully attached IRQ',(sb_irq_num+'0')
              db ' as hardcoded',13,10,0
msgErrRsrvPorts db 'failed to reserve needed ports.',13,10
              db 'Driver may work unstable',13,10,0
msgSetVol     db 'DEV_SET_MASTERVOL call came',13,10,0
msgGetVol     db 'DEV_GET_MASTERVOL call came',13,10,0
msgErrDMAsetup db 'failed to setup DMA - bad channel',13,10,0
;-------------------------------------------------------------------------------
msgDSPFound   db 'DSP found at port 2'
label port_second_digit_num dword at $
              db '00h',13,10,'DSP version '
sb_DSP_version:
                db '0.00 - SB'
sb_DSP_description:
                    db 32,32,32,32,13,10,0
;-------------------------------------------------------------------------------
end if

section '.data' data readable writable align 16
;all uninitialized data place here

;pTempBuf          rd 1

callback           rd 1

int_flip_flop      rd 1

sb_master_vol      rd 1

sb_DSP_version_int rd 1
