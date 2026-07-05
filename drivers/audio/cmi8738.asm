;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2026. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; C-Media CMI8738 / CMI8338 PCI audio driver.
; Playback only: 16-bit / 48000 Hz / stereo.
; Native linear-DMA engine (DMA channel 0) + built-in SB16-compatible mixer.
;
; The driver framework (SOUND service contract, ping-pong buffer, PCI/IRQ
; bring-up, port-I/O helpers, StallExec) is derived from the KolibriOS
; fm801 driver (C) KolibriOS team.
;
; The CMI8738 register map and the playback / mixer programming sequence were
; referenced and cross-checked against:
;   - Linux ALSA "snd-cmipci" (sound/pci/cmipci.c), GPL, (C) Takashi Iwai et al.
;
; Developed with AI assistance (Claude); Tested on real hardware by Leency.

format PE DLL native 0.05
entry START

        DEBUG = 1

API_VERSION     equ 0x01000100

;irq 0,13 unavailable
;                   FEDCBA9876543210
VALID_IRQ       equ 1101111111111110b
CPU_FREQ        equ 2000d

VID_CMEDIA      equ 0x13F6
CTRL_CMI8338A   equ 0x0100
CTRL_CMI8338B   equ 0x0101
CTRL_CMI8738    equ 0x0111
CTRL_CMI8738B   equ 0x0112

; --- I/O register map (offsets from BAR0) ---
CM_FUNCTRL0     equ 0x00        ; channel enable / reset / direction
CM_FUNCTRL1     equ 0x04        ; sample rate fields, bus-master enable
CM_CHFORMAT     equ 0x08        ; channel data format
CM_INT_HLDCLR   equ 0x0C        ; interrupt enable / hold-clear
CM_INT_STATUS   equ 0x10        ; interrupt status
CM_LEGACY_CTRL  equ 0x14
CM_MISC_CTRL    equ 0x18
CM_SB16_DATA    equ 0x22        ; SB16 mixer data port
CM_SB16_ADDR    equ 0x23        ; SB16 mixer index port
CM_MIXER1       equ 0x24        ; C-Media mixer1 (wave/FM mute, speaker mode)
CM_WSMUTE       equ 0x40        ; mixer1 bit: mute wave/PCM
CM_CH0_FRAME1   equ 0x80        ; channel 0 DMA base address
CM_CH0_FRAME2   equ 0x84        ; channel 0 DMA counts (dwords-1)

; FUNCTRL0 bits
CM_CHADC0       equ 0x00000001  ; 0 = playback, 1 = record (channel 0)
CM_PAUSE0       equ 0x00000004
CM_CHEN0        equ 0x00010000  ; channel 0 enable
CM_RST_CH0      equ 0x00040000  ; channel 0 DMA reset
CM_RST_CH1      equ 0x00080000

; FUNCTRL1 bits
CM_BREQ         equ 0x00000010  ; bus-master request enable
; FUNCTRL1 holds two sample-rate fields. Sources disagree on whether the
; [15:13] / [12:10] fields are named per channel (ch0/ch1) or per converter
; (DAC/ADC), so we program BOTH to 48 kHz; the unused channel does not care.
CM_DSFC_MASK    equ 0x0000E000  ; rate field [15:13]
CM_DSFC_SHIFT   equ 13
CM_ASFC_MASK    equ 0x00001C00  ; rate field [12:10]
CM_ASFC_SHIFT   equ 10

; CHFORMAT bits  (format value: bit0 = stereo, bit1 = 16-bit)
; Channel 0 format is bits [1:0] of reg 0x08 (verified vs Linux cmipci.c,
; 86Box, OSS/BSD). Channel 1 is bits [3:2]. (Earlier [13:12] was wrong and
; left the format at 8-bit mono -> silence.)
CM_CH0FMT_MASK  equ 0x00000003  ; channel 0 format field [1:0]
CM_CH0FMT_SHIFT equ 0
CM_FMT_16ST     equ 3           ; 16-bit stereo (bit1=16-bit, bit0=stereo)

; INT_HLDCLR bits
CM_CH0_INT_EN   equ 0x00010000
CM_CH1_INT_EN   equ 0x00020000

; INT_STATUS bits
CM_INTR         equ 0x80000000  ; any interrupt pending (read only)
CM_CHINT0       equ 0x00000001  ; channel 0 interrupt

; MISC_CTRL bits
CM_PWD          equ 0x80000000  ; power down
CM_RESET        equ 0x40000000

; SB16 mixer register indices
SB16_RESET      equ 0x00
SB16_MASTER_L   equ 0x30
SB16_MASTER_R   equ 0x31
SB16_VOICE_L    equ 0x32
SB16_VOICE_R    equ 0x33

; sample-rate field value for 48000 Hz
; hw table: 5512,11025,22050,44100,8000,16000,32000,48000 -> index 7
CM_RATE_48000   equ 7

; service ioctl codes
SRV_GETVERSION    equ  0
DEV_PLAY          equ  1
DEV_STOP          equ  2
DEV_CALLBACK      equ  3
DEV_SET_BUFF      equ  4
DEV_NOTIFY        equ  5
DEV_SET_MASTERVOL equ  6
DEV_GET_MASTERVOL equ  7
DEV_GET_INFO      equ  8

; primary buffer geometry: two halves of DMA_HALF bytes each
BUFFER_SIZE     equ 0x10000     ; allocation size
DMA_TOTAL       equ 0x8000      ; bytes used by the DMA ring (two halves)
DMA_HALF        equ 0x4000      ; bytes per half = one callback chunk

struc CMI_CTRL
{ .bus            dd ?
  .devfn          dd ?
  .vendor         dd ?
  .dev_id         dd ?
  .pci_cmd        dd ?
  .io_base        dd ?
  .int_line       dd ?
  .buffer         dd ?           ; linear address of primary buffer
  .buffer_pg      dd ?           ; physical address of primary buffer
  .user_callback  dd ?
  .master_vol     dd ?           ; cached master volume (-10000..0)
}

section '.flat' code readable writable executable
include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../peimport.inc'

proc START c uses ebx esi edi, state:dword, cmdline:dword

        cmp     [state], 1
        jne     .stop

     if DEBUG
        mov     esi, msgInit
        invoke  SysMsgBoardStr
     end if

        call    detect_controller
        test    eax, eax
        jz      .fail

        call    init_controller
        test    eax, eax
        jz      .fail

        call    reset_controller
        call    init_mixer
        call    create_primary_buff
        test    eax, eax
        jz      .fail

        mov     eax, VALID_IRQ
        mov     ebx, [ctrl.int_line]
        bt      eax, ebx
        jnc     .fail_irq

        invoke  AttachIntHandler, ebx, cmi_irq, dword 0

     if DEBUG
        mov     esi, msgDone
        invoke  SysMsgBoardStr
     end if

        invoke  RegService, sz_sound_srv, service_proc
        ret
.fail_irq:
     if DEBUG
        mov     esi, msgInvIRQ
        invoke  SysMsgBoardStr
     end if
        xor     eax, eax
        ret
.fail:
     if DEBUG
        mov     esi, msgFail
        invoke  SysMsgBoardStr
     end if
        xor     eax, eax
        ret
.stop:
        call    stop
        xor     eax, eax
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
        jne     @F
     if DEBUG
        mov     esi, msgPlay
        invoke  SysMsgBoardStr
     end if
        call    play
        ret
@@:
        cmp     eax, DEV_STOP
        jne     @F
     if DEBUG
        mov     esi, msgStop
        invoke  SysMsgBoardStr
     end if
        call    stop
        ret
@@:
        cmp     eax, DEV_CALLBACK
        jne     @F
        mov     ebx, [edi+input]
        stdcall set_callback, [ebx]
        ret
@@:
        cmp     eax, DEV_SET_MASTERVOL
        jne     @F
        mov     eax, [edi+input]
        mov     eax, [eax]
        call    set_master_vol      ; eax = vol
        ret
@@:
        cmp     eax, DEV_GET_MASTERVOL
        jne     @F
        mov     ebx, [edi+output]
        stdcall get_master_vol, ebx
        ret
@@:
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

align 4
proc detect_controller
        push    ebx
        invoke  GetPCIList
        mov     ebx, eax                ; list head (sentinel)
.next_dev:
        mov     eax, [eax+PCIDEV.fd]
        cmp     eax, ebx
        je      .err

        mov     ecx, [eax+PCIDEV.vendor_device_id]
        cmp     cx, VID_CMEDIA
        jne     .next_dev

        shr     ecx, 16                 ; device id
        cmp     cx, CTRL_CMI8738
        je      .found
        cmp     cx, CTRL_CMI8738B
        je      .found
        cmp     cx, CTRL_CMI8338A
        je      .found
        cmp     cx, CTRL_CMI8338B
        je      .found
        jmp     .next_dev
.err:
        xor     eax, eax
        pop     ebx
        ret
.found:
        movzx   edx, [eax+PCIDEV.bus]
        mov     [ctrl.bus], edx
        movzx   edx, [eax+PCIDEV.devfn]
        mov     [ctrl.devfn], edx

        mov     edx, [eax+PCIDEV.vendor_device_id]
        movzx   ecx, dx
        mov     [ctrl.vendor], ecx
        shr     edx, 16
        mov     [ctrl.dev_id], edx

        xor     eax, eax
        inc     eax
        pop     ebx
        ret
endp

align 4
proc init_controller
        ; I/O base from BAR0
        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 0x10
        and     eax, 0xFFFFFFFC
        mov     [ctrl.io_base], eax

        ; interrupt line
        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 0x3C
        and     eax, 0xFF
        mov     [ctrl.int_line], eax

        ; enable I/O space + bus mastering
        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 4
        mov     [ctrl.pci_cmd], eax
        or      al, 101b                                ; bit0 IO, bit2 busmaster
        invoke  PciWrite16, [ctrl.bus], [ctrl.devfn], 4, eax

        xor     eax, eax
        inc     eax
        ret
endp

align 4
proc reset_controller
     if DEBUG
        mov     esi, msgInitCtrl
        invoke  SysMsgBoardStr
     end if

        ; stop all channels
        xor     eax, eax
        mov     edx, CM_FUNCTRL0
        call    io_w32

        ; pulse reset on both DMA channels
        mov     eax, CM_RST_CH0 or CM_RST_CH1
        mov     edx, CM_FUNCTRL0
        call    io_w32
        mov     eax, 10
        call    StallExec
        xor     eax, eax
        mov     edx, CM_FUNCTRL0
        call    io_w32

        ; FUNCTRL1: clear legacy/SPDIF, keep bus-master request enabled
        mov     eax, CM_BREQ
        mov     edx, CM_FUNCTRL1
        call    io_w32

        ; CHFORMAT: cleared (set per playback)
        xor     eax, eax
        mov     edx, CM_CHFORMAT
        call    io_w32

        ; power up the analog section (clear power-down and reset bits)
        mov     edx, CM_MISC_CTRL
        call    io_r32
        and     eax, not (CM_PWD or CM_RESET)
        mov     edx, CM_MISC_CTRL
        call    io_w32

        ; disable every interrupt source for now
        xor     eax, eax
        mov     edx, CM_INT_HLDCLR
        call    io_w32

        ; clear legacy control (SPDIF loopback, legacy emulation, ...)
        xor     eax, eax
        mov     edx, CM_LEGACY_CTRL
        call    io_w32
        ret
endp

align 4
proc init_mixer
     if DEBUG
        mov     esi, msgInitMix
        invoke  SysMsgBoardStr
     end if

        ; reset the SB16 mixer
        stdcall mixer_write, SB16_RESET, 0
        mov     eax, 10
        call    StallExec

        ; voice (PCM/DAC) volume to maximum
        stdcall mixer_write, SB16_VOICE_L, 0xF8
        stdcall mixer_write, SB16_VOICE_R, 0xF8

        ; master volume to maximum (0 dB attenuation)
        xor     eax, eax
        call    set_master_vol

        ; unmute the C-Media wave/FM path (clear WSMUTE/FMMUTE in mixer1 0x24)
        xor     eax, eax
        mov     edx, CM_MIXER1
        call    io_w8
        ret
endp

align 4
proc create_primary_buff
        invoke  KernelAlloc, BUFFER_SIZE
        mov     [ctrl.buffer], eax
        test    eax, eax
        jz      .fail

        mov     edi, eax
        mov     ecx, BUFFER_SIZE/4
        xor     eax, eax
        cld
        rep stosd

        mov     eax, [ctrl.buffer]
        invoke  GetPgAddr
        mov     [ctrl.buffer_pg], eax
        xor     eax, eax
        inc     eax
        ret
.fail:
        xor     eax, eax
        ret
endp

; Pre-load both buffer halves before the DMA engine starts.
align 4
proc prime_buffer
        cmp     [ctrl.user_callback], 0
        je      .exit
        mov     esi, [ctrl.buffer]
        stdcall [ctrl.user_callback], esi          ; half 0
        mov     esi, [ctrl.buffer]
        add     esi, DMA_HALF
        stdcall [ctrl.user_callback], esi          ; half 1
.exit:
        ret
endp

; Refill the half that the hardware has just finished playing.
align 4
proc fill_buffer
        cmp     [ctrl.user_callback], 0
        je      .exit
        mov     esi, [ctrl.buffer]
        test    [int_flip_flop], 1
        jz      @f
        add     esi, DMA_HALF
@@:
        stdcall [ctrl.user_callback], esi
        inc     [int_flip_flop]
.exit:
        ret
endp

align 4
proc cmi_irq
        mov     edx, CM_INT_STATUS
        call    io_r32
        test    eax, CM_INTR
        jz      .not_ours
        mov     [irq_status], eax

        ; acknowledge: drop channel interrupt enables, then restore them
        mov     edx, CM_INT_HLDCLR
        call    io_r32
        and     eax, not (CM_CH0_INT_EN or CM_CH1_INT_EN)
        mov     edx, CM_INT_HLDCLR
        call    io_w32

        test    [irq_status], CM_CHINT0
        jz      @f
        call    fill_buffer
@@:
        ; do not re-arm the channel interrupt after DEV_STOP has disabled it
        mov     edx, CM_FUNCTRL0
        call    io_r32
        test    eax, CM_CHEN0
        jz      .no_rearm
        mov     edx, CM_INT_HLDCLR
        call    io_r32
        or      eax, CM_CH0_INT_EN
        mov     edx, CM_INT_HLDCLR
        call    io_w32
.no_rearm:
        mov     eax, 1                  ; interrupt handled
        ret
.not_ours:
        xor     eax, eax
        ret
endp

align 4
play:
        ; prime both halves of the ring buffer
        xor     eax, eax
        mov     [int_flip_flop], eax
        call    prime_buffer

        ; reset channel 0 first (canonical order: reset -> program -> enable)
        mov     edx, CM_FUNCTRL0
        call    io_r32
        or      eax, CM_RST_CH0
        mov     edx, CM_FUNCTRL0
        call    io_w32
        mov     edx, CM_FUNCTRL0
        call    io_r32
        and     eax, not CM_RST_CH0
        mov     edx, CM_FUNCTRL0
        call    io_w32

        ; data format: channel 0 = 16-bit stereo
        mov     edx, CM_CHFORMAT
        call    io_r32
        and     eax, not CM_CH0FMT_MASK
        or      eax, CM_FMT_16ST shl CM_CH0FMT_SHIFT
        mov     edx, CM_CHFORMAT
        call    io_w32

        ; sample rate: both rate fields = 48000 Hz, keep bus-master request
        mov     edx, CM_FUNCTRL1
        call    io_r32
        and     eax, not (CM_DSFC_MASK or CM_ASFC_MASK)
        or      eax, (CM_RATE_48000 shl CM_DSFC_SHIFT) or (CM_RATE_48000 shl CM_ASFC_SHIFT) or CM_BREQ
        mov     edx, CM_FUNCTRL1
        call    io_w32

        ; DMA base address
        mov     eax, [ctrl.buffer_pg]
        mov     edx, CM_CH0_FRAME1
        call    io_w32

        ; DMA counts (in dwords - 1): total in low word, period in high word
        mov     eax, (DMA_TOTAL/4)-1
        mov     edx, CM_CH0_FRAME2
        call    io_w16
        mov     eax, (DMA_HALF/4)-1
        mov     edx, CM_CH0_FRAME2+2
        call    io_w16

        ; direction = playback, not paused
        mov     edx, CM_FUNCTRL0
        call    io_r32
        and     eax, not (CM_CHADC0 or CM_PAUSE0)
        mov     edx, CM_FUNCTRL0
        call    io_w32

        ; enable channel 0 interrupt
        mov     edx, CM_INT_HLDCLR
        call    io_r32
        or      eax, CM_CH0_INT_EN
        mov     edx, CM_INT_HLDCLR
        call    io_w32

        ; enable channel 0
        mov     edx, CM_FUNCTRL0
        call    io_r32
        or      eax, CM_CHEN0
        mov     edx, CM_FUNCTRL0
        call    io_w32

        xor     eax, eax
        ret

align 4
stop:
        ; disable channel 0
        mov     edx, CM_FUNCTRL0
        call    io_r32
        and     eax, not CM_CHEN0
        mov     edx, CM_FUNCTRL0
        call    io_w32

        ; disable channel 0 interrupt
        mov     edx, CM_INT_HLDCLR
        call    io_r32
        and     eax, not CM_CH0_INT_EN
        mov     edx, CM_INT_HLDCLR
        call    io_w32

        ; reset channel 0 DMA
        mov     edx, CM_FUNCTRL0
        call    io_r32
        or      eax, CM_RST_CH0
        mov     edx, CM_FUNCTRL0
        call    io_w32
        mov     eax, 10
        call    StallExec
        mov     edx, CM_FUNCTRL0
        call    io_r32
        and     eax, not CM_RST_CH0
        mov     edx, CM_FUNCTRL0
        call    io_w32

        xor     eax, eax
        ret

align 4
proc set_callback stdcall, handler:dword
        mov     eax, [handler]
        mov     [ctrl.user_callback], eax
        ret
endp

; eax = volume in hundredths of dB, 0 = loudest.
; The KolibriOS volume applet (programs/media/volume) drives master volume over
; the range 0 (loudest) .. -VOL_MIN (its lowest step), and sends the SAME -VOL_MIN
; for both "volume 0" and "mute" -- there is no separate mute ioctl. So we map
; that range onto the SB16 5-bit master register and gate the wave with WSMUTE at
; the bottom, making mute / lowest step true silence instead of merely quiet.
VOL_MIN equ 4790                ; applet range: 10 steps * 479
align 4
set_master_vol:
        cmp     eax, 0
        jle     @f
        xor     eax, eax
@@:
        cmp     eax, -VOL_MIN
        jge     @f
        mov     eax, -VOL_MIN
@@:
        mov     [ctrl.master_vol], eax

        ; level = 31 - (-vol)*31/VOL_MIN   (5-bit attenuation)
        neg     eax
        imul    eax, 31
        xor     edx, edx
        mov     ebx, VOL_MIN
        div     ebx
        mov     ebx, 31
        sub     ebx, eax                ; ebx = level 0..31

        ; True mute at the lowest level: the SB16 master register bottoms out at
        ; a still-audible attenuation, so gate the native PCM with WSMUTE (0x24).
        xor     eax, eax
        test    ebx, ebx
        jnz     @f
        mov     al, CM_WSMUTE           ; level 0 -> mute wave/PCM entirely
@@:
        mov     edx, CM_MIXER1
        call    io_w8

        shl     ebx, 3                  ; level into bits [7:3]
        stdcall mixer_write, SB16_MASTER_L, ebx
        stdcall mixer_write, SB16_MASTER_R, ebx
        xor     eax, eax
        ret

align 4
proc get_master_vol stdcall, pvol:dword
        mov     eax, [ctrl.master_vol]
        mov     ebx, [pvol]
        mov     [ebx], eax
        xor     eax, eax
        ret
endp

align 4
proc mixer_write stdcall uses eax edx, idx:dword, val:dword
        mov     eax, [idx]
        mov     edx, CM_SB16_ADDR
        call    io_w8
        mov     eax, [val]
        mov     edx, CM_SB16_DATA
        call    io_w8
        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;          I/O port helpers
;          edx = register offset
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 4
proc io_r8
        add     edx, [ctrl.io_base]
        in      al, dx
        ret
endp
align 4
proc io_r16
        add     edx, [ctrl.io_base]
        in      ax, dx
        ret
endp
align 4
proc io_r32
        add     edx, [ctrl.io_base]
        in      eax, dx
        ret
endp
align 4
proc io_w8
        add     edx, [ctrl.io_base]
        out     dx, al
        ret
endp
align 4
proc io_w16
        add     edx, [ctrl.io_base]
        out     dx, ax
        ret
endp
align 4
proc io_w32
        add     edx, [ctrl.io_base]
        out     dx, eax
        ret
endp

align 4
proc StallExec
        push    ecx
        push    edx
        push    ebx
        push    eax

        mov     ecx, CPU_FREQ
        mul     ecx
        mov     ebx, eax      ;low
        mov     ecx, edx      ;high
        rdtsc
        add     ebx, eax
        adc     ecx, edx
@@:
        rdtsc
        sub     eax, ebx
        sbb     edx, ecx
        js      @B

        pop     eax
        pop     ebx
        pop     edx
        pop     ecx
        ret
endp

align 4
sz_sound_srv  db 'SOUND', 0

msgInit       db 'CMI8738: detecting hardware...',13,10,0
msgFail       db 'CMI8738: device not found',13,10,0
msgInvIRQ     db 'CMI8738: IRQ line not assigned or invalid',13,10,0
msgInitCtrl   db 'CMI8738: init controller',13,10,0
msgInitMix    db 'CMI8738: init mixer',13,10,0
msgPlay       db 'CMI8738: start play',13,10,0
msgStop       db 'CMI8738: stop play',13,10,0
msgDone       db 'CMI8738: ready',13,10,0

align 4
data fixups
end data

ctrl CMI_CTRL

int_flip_flop  rd 1
irq_status     rd 1
