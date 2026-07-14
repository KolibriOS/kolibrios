;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2026. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Ensoniq ES1371 / ES1373 / Creative CT5880 / CT2518 AudioPCI driver.
; Also Ectiva EV1938 (1102:8938), register-compatible (untested - no magic).
; Playback only: 16-bit / 48000 Hz / stereo, AC97 codec + on-chip SRC.
;
; The driver framework (SOUND service contract, ping-pong buffer, PCI/IRQ
; bring-up, port-I/O helpers, StallExec) is derived from the KolibriOS
; cmi8738 / fm801 drivers (C) KolibriOS team.
;
; The ES1371 register map, AC97-codec access, SRC init and DAC2 programming
; were referenced and cross-checked against:
;   - Linux ALSA snd-ens1371 (sound/pci/ens1370.c), GPL, (C) Jaroslav Kysela
;   - FreeBSD/OpenBSD es137x / eap drivers
;   - 86Box ES1371 emulation (src/sound/snd_audiopci.c), GPLv2
;     -- used to verify the register-level programming.

format PE DLL native 0.05
entry START

        DEBUG   = 1     ; milestones and errors
        VERBOSE = 0     ; bring-up trace + DAC2/status probes on DEV_PLAY
                        ; (the probe alone adds ~30 ms to every play start)

API_VERSION     equ 0x01000100

;        FEDCBA9876543210
VALID_IRQ       equ 1101111111111110b
CPU_FREQ        equ 2000d

VID_ENSONIQ     equ 0x1274
VID_CREATIVE    equ 0x1102
CTRL_ES1371     equ 0x1371      ; ES1371 / ES1373 / CT2518
CTRL_ES5880     equ 0x5880      ; CT5880
CTRL_ECTIVA     equ 0x8938      ; Ectiva EV1938

; --- I/O register map (offsets from BAR0) ---
ES_CONTROL      equ 0x00        ; chip enables (dword)
ES_STATUS       equ 0x04        ; interrupt status (dword)
ES_MEM_PAGE     equ 0x0C        ; DMA page select
ES_SRC          equ 0x10        ; sample-rate-converter interface (dword)
ES_CODEC        equ 0x14        ; AC97 codec access (dword)
ES_LEGACY       equ 0x18        ; legacy control (dword)
ES_SERIAL       equ 0x20        ; serial interface control (dword)
ES_DAC2_COUNT   equ 0x28        ; DAC2 sample count / IRQ period (dword)
ES_DAC2_FRAME   equ 0x38        ; DAC2 DMA address  (page 0x0C)
ES_DAC2_SIZE    equ 0x3C        ; DAC2 buffer size  (page 0x0C)

; CONTROL bits
ES_ADC_EN       equ 0x00000010
ES_DAC2_EN      equ 0x00000020  ; enable DAC2 engine (0->1 edge starts DMA)
ES_DAC1_EN      equ 0x00000040
ES_BREQ         equ 0x00000080
ES_SYNC_RES     equ 0x00004000  ; AC97 warm reset

; STATUS bits
ES_INTR         equ 0x80000000  ; any interrupt pending
ES_STAT_DAC2    equ 0x00000002  ; DAC2 interrupt (our playback)
ES_ST_AC97_RST  equ 0x20000000  ; releases AC97 reset (CT5880/ES1373); a LEVEL

; bound for the SRC/codec busy-wait spins (real silicon can hold BUSY/WIP a
; while; 86Box never sets them). Larger than fm801/cmi8738 as ES1371 is strict.
POLL_COUNT      equ 0xA000

; MEM_PAGE values
ES_PAGE_DAC     equ 0x0000000C  ; selects DAC frame registers (0x30..0x3F)

; SRC interface bits
ES_SRC_DIS_R1   equ 0x00080000
ES_SRC_DIS_P2   equ 0x00100000
ES_SRC_DIS_P1   equ 0x00200000
ES_SRC_DISABLE  equ 0x00400000
ES_SRC_RAM_BUSY equ 0x00800000  ; poll: busy while set
ES_SRC_RAM_WE   equ 0x01000000  ; write enable
SRC_DIS_MASK    equ 0x00780000  ; all four DIS bits (preserve on every write)
SRC_STATE_MASK  equ 0x00870000  ; state field for codec interlock
SRC_SAFE        equ 0x00010000  ; safe/idle state value
; SRC RAM register addresses
SRC_DAC1_TRUNC  equ 0x70
SRC_DAC1_INT    equ 0x71
SRC_DAC1_VFREQ  equ 0x73
SRC_DAC2_TRUNC  equ 0x74
SRC_DAC2_INT    equ 0x75
SRC_DAC2_VFREQ  equ 0x77
SRC_ADC_TRUNC   equ 0x78
SRC_ADC_INT     equ 0x79
SRC_ADC_VFREQ   equ 0x7B
SRC_VOL_ADC_L   equ 0x6C
SRC_VOL_ADC_R   equ 0x6D
SRC_VOL_DAC1_L  equ 0x7C
SRC_VOL_DAC1_R  equ 0x7D
SRC_VOL_DAC2_L  equ 0x7E
SRC_VOL_DAC2_R  equ 0x7F

; CODEC access bits
CODEC_RDY       equ 0x80000000  ; read data valid
CODEC_WIP       equ 0x40000000  ; write in progress
CODEC_PIRD      equ 0x00800000  ; read direction

; SERIAL (serial interface control) bits
ES_P2_INT_EN    equ 0x00000200  ; DAC2 interrupt enable
ES_P2_PAUSE     equ 0x00001000
; DAC2 format field is bits [3:2]: 3 = 16-bit stereo
ES_FMT_16ST     equ 3
; full DAC2 sctrl value: 16-bit stereo + IRQ enable + END_INC=2
ES_SCTRL_PLAY   equ (ES_FMT_16ST shl 2) or ES_P2_INT_EN or (2 shl 19)

; AC97 codec registers
AC97_RESET      equ 0x00
AC97_MASTER     equ 0x02
AC97_PCM_OUT    equ 0x18
; AC97 volume register layout
AC97_MUTE       equ 0x8000      ; bit 15: channel mute
AC97_ATT_MAX    equ 63          ; 6-bit attenuation field maximum (-94.5 dB)

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

; KolibriOS volume applet range: 0 (loudest) .. -VOL_MIN (lowest/mute)
VOL_MIN         equ 4790

struc ES_CTRL
{ .bus            dd ?
  .devfn          dd ?
  .vendor         dd ?
  .dev_id         dd ?
  .revision       dd ?           ; PCI revision (gates CT5880/ES1373 pre-init)
  .pci_cmd        dd ?
  .io_base        dd ?
  .int_line       dd ?
  .buffer         dd ?           ; linear address of primary buffer
  .buffer_pg      dd ?           ; physical address of primary buffer
  .user_callback  dd ?
  .master_vol     dd ?           ; cached master volume (-VOL_MIN..0)
  .cssr           dd ?           ; persistent STATUS bits (ES_ST_AC97_RST on
}                                ; CT5880/ES1373): OR-ed into every STATUS write

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
        call    es1371_setup_codec
        call    create_primary_buff
     if VERBOSE
        mov     esi, msgBuff
        invoke  SysMsgBoardStr
     end if

        mov     eax, VALID_IRQ
        mov     ebx, [ctrl.int_line]
        bt      eax, ebx
        jnc     .fail_irq

        invoke  AttachIntHandler, ebx, es_irq, dword 0

     if DEBUG
        mov     esi, msgDone
        invoke  SysMsgBoardStr
     end if

        invoke  RegService, sz_sound_srv, service_proc
     if VERBOSE
        mov     esi, msgReg
        invoke  SysMsgBoardStr
     end if
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
        cmp     cx, VID_ENSONIQ
        jne     .try_ectiva
        shr     ecx, 16
        cmp     cx, CTRL_ES1371
        je      .found
        cmp     cx, CTRL_ES5880
        je      .found
        jmp     .next_dev
.try_ectiva:
        cmp     cx, VID_CREATIVE
        jne     .next_dev
        shr     ecx, 16
        cmp     cx, CTRL_ECTIVA
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

        ; PCI revision (class-code dword at 0x08, low byte)
        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 0x08
        and     eax, 0xFF
        mov     [ctrl.revision], eax
     if DEBUG
        mov     esi, msgRev             ; the CT5880/ES1373 AC97_RST gate keys
        invoke  SysMsgBoardStr          ; off this value - keep it in the log
        call    dbg_hex32
     end if

        ; enable I/O space + bus mastering
        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 4
        mov     [ctrl.pci_cmd], eax
        or      al, 5                                   ; bit0 IO, bit2 busmaster
        invoke  PciWrite16, [ctrl.bus], [ctrl.devfn], 4, eax

        xor     eax, eax
        inc     eax
        ret
endp

align 4
proc reset_controller
     if VERBOSE
        mov     esi, msgInitCtrl
        invoke  SysMsgBoardStr
     end if

        ; all engines off, clean serial/legacy state (matters on warm restart:
        ; a still-running DAC2 during the resets below corrupts the bring-up)
        xor     eax, eax
        mov     edx, ES_CONTROL
        call    io_w32
        xor     eax, eax
        mov     edx, ES_SERIAL
        call    io_w32
        xor     eax, eax
        mov     edx, ES_LEGACY
        call    io_w32

        ; CT5880 / ES1373(rev 0x07,0x08) hold the AC97 RESET# line until bit 29
        ; (ES_ST_AC97_RST) of STATUS is set. The bit is a LEVEL, not a strobe:
        ; it must stay set in EVERY subsequent STATUS write, or the codec is
        ; thrown back into reset (dead AC-link -> codec id reads 0, BITCLK
        ; stops, DAC2 stalls at FIFO depth). Linux keeps it in a shadow reg
        ; ("cssr") and so do we: ctrl.cssr is OR-ed into all STATUS writes.
        xor     eax, eax
        mov     [ctrl.cssr], eax
        mov     eax, [ctrl.dev_id]
        cmp     eax, CTRL_ES5880
        je      .ac97rst                ; all CT5880 revisions
        cmp     eax, CTRL_ES1371
        jne     .no_ac97rst
        mov     eax, [ctrl.revision]
        cmp     eax, 0x07
        jb      .no_ac97rst
        cmp     eax, 0x08
        ja      .no_ac97rst
.ac97rst:
        mov     eax, ES_ST_AC97_RST
        mov     [ctrl.cssr], eax
        mov     edx, ES_STATUS
        call    io_w32
        mov     edx, ES_STATUS
        call    io_r32                  ; posted-write flush
        mov     eax, 40000              ; codec wake-up settle (~20-40 ms)
        call    StallExec
.no_ac97rst:

        ; AC97 warm reset: start the AC97 bit-clock before any codec access
        mov     eax, ES_SYNC_RES
        mov     edx, ES_CONTROL
        call    io_w32
        mov     edx, ES_CONTROL
        call    io_r32                  ; readback flush
        mov     eax, 2000               ; ~2 ms settle
        call    StallExec
        xor     eax, eax
        mov     edx, ES_CONTROL
        call    io_w32

        ; bring up the sample-rate converter (mandatory or DAC2 is silent)
        call    es1371_src_init
     if DEBUG
        call    wait_src_ready          ; RAM_BUSY still set after a full poll
        test    eax, ES_SRC_RAM_BUSY    ; window = the chip has locked up:
        jz      @F                      ; a genuine fault, always report it
        mov     esi, msgSrcStuck
        invoke  SysMsgBoardStr
@@:
     end if
     if VERBOSE
        mov     esi, msgSrc
        invoke  SysMsgBoardStr
     end if

        ; clear codec + status (drain so nothing is latched before AttachIntHandler)
        xor     eax, eax
        mov     edx, ES_CODEC
        call    io_w32
        mov     eax, [ctrl.cssr]        ; keep AC97 RESET# released on CT5880
        mov     edx, ES_STATUS
        call    io_w32
        mov     edx, ES_STATUS
        call    io_r32                  ; flush
        ret
endp

align 4
proc es1371_setup_codec
     if VERBOSE
        mov     esi, msgInitCodec
        invoke  SysMsgBoardStr
     end if
        ; AC97 soft reset
        stdcall es1371_codec_write16, AC97_RESET, 0x0000
     if VERBOSE
        mov     esi, msgCd1
        invoke  SysMsgBoardStr
     end if
        ; PCM-out at 0 dB, unmuted. AC97 5-bit volume fields: 8 = 0 dB and
        ; LOWER values are gain (0 = +12 dB, clips and blasts) - never 0x0000.
        stdcall es1371_codec_write16, AC97_PCM_OUT, 0x0808
        ; default master level: match the stock intelac97 driver (att 11 =
        ; 0x0B0B = -16.5 dB) so a freshly loaded driver is not deafening;
        ; @VOLUME (its own default is 5/10) overrides this as soon as it runs.
        mov     eax, -1650
        call    set_master_vol
     if DEBUG
        ; codec liveness probe: AC97 vendor ID (regs 0x7C:0x7E).
        ; 00000000 or FFFFFFFF here = dead AC-link (codec never came up),
        ; which means total silence even with a running DAC2.
        stdcall es1371_codec_read16, 0x7C
        mov     ebx, eax
        stdcall es1371_codec_read16, 0x7E
        shl     ebx, 16
        or      eax, ebx
        mov     esi, msgCodecId
        invoke  SysMsgBoardStr
        call    dbg_hex32
      if VERBOSE
        mov     esi, msgCodecOk
        invoke  SysMsgBoardStr
      end if
     end if
        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        Sample-Rate Converter
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; wait until the SRC RAM is not busy; returns the last value read in eax
align 4
proc wait_src_ready
        mov     ecx, POLL_COUNT
.l:
        mov     edx, ES_SRC
        call    io_r32
        test    eax, ES_SRC_RAM_BUSY
        jz      .done
        dec     ecx
        jnz     .l
.done:
        ret
endp

align 4
proc src_write stdcall, sreg:dword, sdata:dword
        call    wait_src_ready
        and     eax, SRC_DIS_MASK       ; preserve DIS bits
        mov     ecx, [sreg]
        and     ecx, 0x7F
        shl     ecx, 25
        or      eax, ecx
        mov     ecx, [sdata]
        and     ecx, 0xFFFF
        or      eax, ecx
        or      eax, ES_SRC_RAM_WE
        mov     edx, ES_SRC
        call    io_w32
        ret
endp

; ES1371 SRC-RAM read is a two-step handshake (matches Linux snd_es1371_src_read):
; write addr WITH the 0x10000 state bit, wait for the SRC state field to reach
; SRC_SAFE, capture the data, then write addr again WITHOUT 0x10000 to release.
align 4
proc src_read stdcall, sreg:dword
        call    wait_src_ready
        mov     [src_orig], eax                 ; settled value for the 2nd write
        and     eax, SRC_DIS_MASK
        mov     ecx, [sreg]
        and     ecx, 0x7F
        shl     ecx, 25
        or      eax, ecx
        or      eax, SRC_SAFE                   ; first write sets 0x00010000
        mov     edx, ES_SRC
        call    io_w32

        mov     ecx, POLL_COUNT
.wait:
        mov     edx, ES_SRC
        call    io_r32
        mov     ebx, eax
        and     ebx, SRC_STATE_MASK
        cmp     ebx, SRC_SAFE
        je      .got
        dec     ecx
        jnz     .wait
.got:
        push    eax                             ; eax low16 = RAM data
        mov     eax, [src_orig]
        and     eax, SRC_DIS_MASK
        mov     ecx, [sreg]
        and     ecx, 0x7F
        shl     ecx, 25
        or      eax, ecx                        ; second write, without 0x10000
        mov     edx, ES_SRC
        call    io_w32
        pop     eax
        and     eax, 0xFFFF
        ret
endp

; program DAC2 sample rate converter for exactly 48000 Hz
align 4
proc es1371_dac2_48k
        ; freeze the P2 accumulator while reprogramming
        call    wait_src_ready
        and     eax, ES_SRC_DISABLE or ES_SRC_DIS_P1 or ES_SRC_DIS_R1
        or      eax, ES_SRC_DIS_P2
        mov     edx, ES_SRC
        call    io_w32

        ; INT_REGS: keep low byte, set integer step for 48k (0x4000)
        stdcall src_read, SRC_DAC2_INT
        and     eax, 0x00FF
        or      eax, 0x4000
        stdcall src_write, SRC_DAC2_INT, eax
        ; fractional step = 0 for exact 48000
        stdcall src_write, SRC_DAC2_VFREQ, 0

        ; unfreeze P2
        call    wait_src_ready
        and     eax, ES_SRC_DISABLE or ES_SRC_DIS_P1 or ES_SRC_DIS_R1
        mov     edx, ES_SRC
        call    io_w32
        ret
endp

align 4
proc es1371_src_init
        call    wait_src_ready
        ; disable SRC while loading its RAM
        mov     eax, ES_SRC_DISABLE
        mov     edx, ES_SRC
        call    io_w32

        ; zero the entire 128-entry SRC RAM
        xor     ebx, ebx
.zloop:
        stdcall src_write, ebx, 0
        inc     ebx
        cmp     ebx, 0x80
        jb      .zloop

        ; truncation / integer step for DAC1 and DAC2
        stdcall src_write, SRC_DAC1_TRUNC, 16 shl 4    ; 0x0100
        stdcall src_write, SRC_DAC1_INT,   16 shl 10   ; 0x4000
        stdcall src_write, SRC_DAC2_TRUNC, 16 shl 4
        stdcall src_write, SRC_DAC2_INT,   16 shl 10

        ; unity gains (0x1000) -- DAC2 volume of 0 here = total silence
        stdcall src_write, SRC_VOL_ADC_L,  1 shl 12
        stdcall src_write, SRC_VOL_ADC_R,  1 shl 12
        stdcall src_write, SRC_VOL_DAC1_L, 1 shl 12
        stdcall src_write, SRC_VOL_DAC1_R, 1 shl 12
        stdcall src_write, SRC_VOL_DAC2_L, 1 shl 12
        stdcall src_write, SRC_VOL_DAC2_R, 1 shl 12

        ; program the ADC converter too. Linux programs ALL THREE converters
        ; (ADC/DAC1/DAC2) before releasing SRC_DISABLE and warns that enabling
        ; the SRC with unprogrammed parameters locks the chip (RAM_BUSY stays
        ; asserted forever -> no DAC2 clock -> no DMA, no IRQ). 86Box does not
        ; model this. Values = snd_es1371_adc_rate(48000).
        stdcall src_write, SRC_ADC_TRUNC,  16 shl 4
        stdcall src_write, SRC_ADC_INT,    16 shl 10
        stdcall src_write, SRC_ADC_VFREQ,  0
        stdcall src_write, SRC_DAC1_VFREQ, 0

        call    es1371_dac2_48k

        ; enable the SRC (only after it is fully programmed)
        call    wait_src_ready
        xor     eax, eax
        mov     edx, ES_SRC
        call    io_w32
        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        AC97 codec access
;   (must serialize against the SRC)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc codec_wait_wip
        mov     ecx, POLL_COUNT
.l:
        mov     edx, ES_CODEC
        call    io_r32
        test    eax, CODEC_WIP
        jz      .done
        dec     ecx
        jnz     .l
.done:
        ret
endp

; drive the SRC to its safe window so the codec line is free
align 4
proc codec_src_acquire
        call    wait_src_ready          ; settled value (RAM_BUSY guaranteed clear)
        mov     [src_save], eax
        and     eax, SRC_DIS_MASK
        or      eax, SRC_SAFE
        mov     edx, ES_SRC
        call    io_w32
        ; wait state == 0
        mov     ecx, POLL_COUNT
.s0:
        mov     edx, ES_SRC
        call    io_r32
        and     eax, SRC_STATE_MASK
        jz      .s0done
        dec     ecx
        jnz     .s0
.s0done:
        ; wait state == SRC_SAFE
        mov     ecx, POLL_COUNT
.ss:
        mov     edx, ES_SRC
        call    io_r32
        and     eax, SRC_STATE_MASK
        cmp     eax, SRC_SAFE
        je      .ssdone
        dec     ecx
        jnz     .ss
.ssdone:
        ret
endp

align 4
proc codec_src_release
        call    wait_src_ready
        mov     eax, [src_save]
        mov     edx, ES_SRC
        call    io_w32
        ret
endp

align 4
proc es1371_codec_write16 stdcall, creg:dword, cval:dword
        call    codec_wait_wip
        call    codec_src_acquire
        mov     eax, [creg]
        and     eax, 0x7F
        shl     eax, 16
        mov     ecx, [cval]
        and     ecx, 0xFFFF
        or      eax, ecx                ; bit23 clear = write
        mov     edx, ES_CODEC
        call    io_w32
        call    codec_src_release
        ret
endp

; returns codec register low word in eax (garbage on timeout - debug use)
align 4
proc es1371_codec_read16 stdcall, creg:dword
        call    codec_wait_wip
        call    codec_src_acquire
        mov     eax, [creg]
        and     eax, 0x7F
        shl     eax, 16
        or      eax, CODEC_PIRD         ; bit23 set = read request
        mov     edx, ES_CODEC
        call    io_w32
        call    codec_src_release
        call    codec_wait_wip          ; command clocked out to the codec
        mov     ecx, POLL_COUNT
.rdy:
        mov     edx, ES_CODEC
        call    io_r32
        test    eax, CODEC_RDY
        jnz     .got
        dec     ecx
        jnz     .rdy
.got:
        and     eax, 0xFFFF
        ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;        Playback engine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc create_primary_buff
        invoke  KernelAlloc, BUFFER_SIZE
        mov     [ctrl.buffer], eax

        mov     edi, eax
        mov     ecx, BUFFER_SIZE/4
        xor     eax, eax
        cld
        rep stosd

        mov     eax, [ctrl.buffer]
        invoke  GetPgAddr
        mov     [ctrl.buffer_pg], eax
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

; Refill the half the hardware is NOT currently playing (= the one just
; finished). We read the live DAC2 position instead of a software flip-flop,
; because the interrupt does not fire exactly on the half boundary (86Box does
; not reload the sample counter on enable, so the IRQ phase is offset); a
; position-based choice self-corrects and never overwrites the playing half.
align 4
proc fill_buffer
        cmp     [ctrl.user_callback], 0
        je      .exit
        ; DAC2 current position is DAC2_SIZE(0x3C) high word, in DWORDs;
        ; the frame registers are paged, so select the DAC page first.
        mov     eax, ES_PAGE_DAC
        mov     edx, ES_MEM_PAGE
        call    io_w32
        mov     edx, ES_DAC2_SIZE
        call    io_r32
        shr     eax, 16
        and     eax, (DMA_TOTAL/4)-1     ; position in DWORDs (0..0x1FFF)
        mov     esi, [ctrl.buffer]
        cmp     eax, (DMA_TOTAL/4)/2     ; half boundary = 0x1000 DWORDs
        jae     @f                        ; chip in half1 -> refill half0
        add     esi, DMA_HALF             ; chip in half0 -> refill half1
@@:
        stdcall [ctrl.user_callback], esi
.exit:
        ret
endp

align 4
proc es_irq
     if VERBOSE
        cmp     [irq_seen], 0
        jne     .no_dbg
        mov     [irq_seen], 1
        pushad
        mov     esi, msgIrq
        invoke  SysMsgBoardStr
        popad
.no_dbg:
     end if
        mov     edx, ES_STATUS
        call    io_r32
        test    eax, ES_INTR
        jz      .not_ours

        test    eax, ES_STAT_DAC2
        jz      .spurious               ; INTR set by a source that is not DAC2
        call    fill_buffer

        ; dismiss the DAC2 IRQ: toggle P2 interrupt-enable off (flush) then on,
        ; using the known sctrl value (never read SERIAL back: on real ES1371 its
        ; low bits carry live count/status). Then clear the STATUS latch so INTA#
        ; de-asserts -- the kernel routes legacy IRQs edge-triggered, so an
        ; undismissed line would block every future DAC2 edge (= output hang).
        mov     eax, ES_SCTRL_PLAY and (not ES_P2_INT_EN)
        mov     edx, ES_SERIAL
        call    io_w32
        mov     edx, ES_SERIAL
        call    io_r32                  ; flush -> the 1->0 edge lands
        mov     eax, ES_SCTRL_PLAY
        mov     edx, ES_SERIAL
        call    io_w32
        mov     eax, [ctrl.cssr]        ; cssr: never drop AC97_RST on CT5880
        mov     edx, ES_STATUS
        call    io_w32                  ; clear latched status
        mov     edx, ES_STATUS
        call    io_r32                  ; flush -> INTA# low

        mov     eax, 1                  ; interrupt handled
        ret
.spurious:
        ; INTR asserted by an engine we never use (or a co-latched status). On an
        ; edge-triggered line a still-asserted level kills all future edges, so we
        ; must dismiss it: mask every interrupt-enable, clear STATUS, restore our
        ; DAC2 config, and CLAIM the IRQ (return 1) -- never leave it latched.
        xor     eax, eax
        mov     edx, ES_SERIAL
        call    io_w32                  ; all P1/P2/R1 int-enables off
        mov     edx, ES_SERIAL
        call    io_r32
        mov     eax, [ctrl.cssr]        ; cssr: never drop AC97_RST on CT5880
        mov     edx, ES_STATUS
        call    io_w32                  ; clear latched status
        mov     edx, ES_STATUS
        call    io_r32                  ; flush
        mov     eax, ES_SCTRL_PLAY      ; restore DAC2 format + P2 interrupt enable
        mov     edx, ES_SERIAL
        call    io_w32
        mov     eax, 1
        ret
.not_ours:
        xor     eax, eax
        ret
endp

align 4
play:
        ; prime both halves of the ring buffer
        call    prime_buffer

        ; stop DAC2 first so the enable below is a clean 0->1 edge
        mov     edx, ES_CONTROL
        call    io_r32
        and     eax, not ES_DAC2_EN
        mov     edx, ES_CONTROL
        call    io_w32

        ; make sure DAC2 runs at 48 kHz
        call    es1371_dac2_48k

        ; select the DAC frame-register page
        mov     eax, ES_PAGE_DAC
        mov     edx, ES_MEM_PAGE
        call    io_w32

        ; DMA buffer physical address
        mov     eax, [ctrl.buffer_pg]
        mov     edx, ES_DAC2_FRAME
        call    io_w32

        ; whole ring length in DWORDs - 1
        mov     eax, (DMA_TOTAL/4)-1
        mov     edx, ES_DAC2_SIZE
        call    io_w32

        ; DAC2 format = 16-bit stereo, enable its interrupt
        mov     eax, ES_SCTRL_PLAY
        mov     edx, ES_SERIAL
        call    io_w32

        ; interrupt period = half buffer, in samples - 1
        mov     eax, (DMA_HALF/4)-1
        mov     edx, ES_DAC2_COUNT
        call    io_w32

        ; drain stale interrupt status so the first DAC2 edge is a clean event
        mov     eax, [ctrl.cssr]        ; cssr: never drop AC97_RST on CT5880
        mov     edx, ES_STATUS
        call    io_w32
        mov     edx, ES_STATUS
        call    io_r32

        ; start: DAC2 enable (0->1 edge launches DMA + IRQs)
        mov     edx, ES_CONTROL
        call    io_r32
        or      eax, ES_DAC2_EN
        mov     edx, ES_CONTROL
        call    io_w32

     if VERBOSE
        mov     esi, msgPlayOk
        invoke  SysMsgBoardStr

        ; liveness probe: sample the DAC2 position twice ~10 ms apart.
        ; printed dword = pos1(hi16):pos2(lo16), both in DWORD units.
        ; equal halves  -> engine frozen (SRC gives no clock / DMA dead);
        ; advancing     -> engine runs: silence then = codec path,
        ;                  playback hang then = IRQ never delivered.
        mov     eax, 10000
        call    StallExec
        mov     eax, ES_PAGE_DAC
        mov     edx, ES_MEM_PAGE
        call    io_w32
        mov     edx, ES_DAC2_SIZE
        call    io_r32
        mov     ebx, eax                ; pos1 in hi16
        mov     eax, 20000
        call    StallExec
        mov     edx, ES_DAC2_SIZE
        call    io_r32
        and     ebx, 0xFFFF0000
        shr     eax, 16
        or      eax, ebx                ; pos1:pos2
        mov     esi, msgPos
        invoke  SysMsgBoardStr
        call    dbg_hex32

        mov     esi, msgStat            ; raw STATUS: bit31=INTR, bit1=DAC2
        invoke  SysMsgBoardStr
        mov     edx, ES_STATUS
        call    io_r32
        call    dbg_hex32
     end if
        xor     eax, eax
        ret

align 4
stop:
        ; halt the DAC2 DMA engine
        mov     edx, ES_CONTROL
        call    io_r32
        and     eax, not ES_DAC2_EN
        mov     edx, ES_CONTROL
        call    io_w32

        ; fully quiesce the chip so no interrupt stays asserted after unload
        ; (a latched/asserted level IRQ into a detached handler wedges shutdown).
        ; Leave P2_INT_EN cleared -- play() reprograms CONTROL/SERIAL on restart.
        xor     eax, eax
        mov     edx, ES_CONTROL
        call    io_w32                  ; all engines + IRQ path off
        xor     eax, eax
        mov     edx, ES_SERIAL
        call    io_w32                  ; P1/P2/R1 interrupt-enables cleared
        mov     eax, [ctrl.cssr]        ; cssr: never drop AC97_RST on CT5880
        mov     edx, ES_STATUS
        call    io_w32                  ; clear any latched interrupt status
        mov     edx, ES_STATUS
        call    io_r32                  ; flush

        xor     eax, eax
        ret

align 4
proc set_callback stdcall, handler:dword
        mov     eax, [handler]
        mov     [ctrl.user_callback], eax
        ret
endp

; eax = volume in hundredths of dB, 0 = loudest.
; The KolibriOS volume applet drives master volume over 0 .. -VOL_MIN and sends
; the SAME -VOL_MIN for both "volume 0" and "mute" (no separate mute ioctl), so
; we map that onto the AC97 master register and set the AC97 mute bit at the
; bottom -- giving true silence on mute instead of merely quiet.
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

        ; scale matches intelac97: one 1.5 dB step per 150 hundredths, so the
        ; @VOLUME mid-slider lands at -22.5 dB (mapping the full slider onto
        ; the whole 94.5 dB range made the middle inaudibly quiet). The very
        ; bottom (-VOL_MIN, sent both for level 0 and mute) still hard-mutes
        ; the codec for true silence instead of intelac97's "quiet but audible"
        cmp     eax, -VOL_MIN
        jne     .scale
        mov     ebx, AC97_MUTE or (AC97_ATT_MAX shl 8) or AC97_ATT_MAX
        jmp     .write
.scale:
        neg     eax
        xor     edx, edx
        mov     ebx, 150
        div     ebx                     ; eax = attenuation 0..31 on this scale
        mov     ebx, eax
        shl     ebx, 8
        or      ebx, eax                ; both channels
.write:
        stdcall es1371_codec_write16, AC97_MASTER, ebx
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

msgInit       db 'ES1371: detecting hardware...',13,10,0
msgFail       db 'ES1371: device not found',13,10,0
msgInvIRQ     db 'ES1371: no legacy IRQ routed for this PCI slot (check BIOS: PnP OS=No / assign IRQ / try another slot)',13,10,0
msgPlay       db 'ES1371: start play',13,10,0
msgStop       db 'ES1371: stop play',13,10,0
msgDone       db 'ES1371: ready',13,10,0

if VERBOSE
msgInitCtrl   db 'ES1371: init controller',13,10,0
msgInitCodec  db 'ES1371: init codec',13,10,0
msgSrc        db 'ES1371: src init ok',13,10,0
msgCd1        db 'ES1371: codec reset ok',13,10,0
msgCodecOk    db 'ES1371: codec setup ok',13,10,0
msgBuff       db 'ES1371: buffer ok',13,10,0
msgReg        db 'ES1371: service registered',13,10,0
msgPlayOk     db 'ES1371: DAC2 started',13,10,0
msgIrq        db 'ES1371: first IRQ',13,10,0
msgPos        db 'ES1371: DAC2 pos1:pos2 ',0
msgStat       db 'ES1371: status ',0
end if

if DEBUG
msgRev        db 'ES1371: pci rev ',0
msgSrcStuck   db 'ES1371: !! SRC RAM busy stuck - chip locked',13,10,0
msgCodecId    db 'ES1371: codec id ',0

; print eax as 8 hex digits + CRLF on the debug board; preserves registers
align 4
proc dbg_hex32
        pushad
        mov     edi, hex_buf
        mov     ecx, 8
.dig:
        rol     eax, 4
        mov     ebx, eax
        and     ebx, 15
        mov     bl, [hex_chars+ebx]
        mov     [edi], bl
        inc     edi
        dec     ecx
        jnz     .dig
        mov     esi, hex_buf
        invoke  SysMsgBoardStr
        popad
        ret
endp
hex_chars     db '0123456789ABCDEF'
hex_buf       db '00000000',13,10,0
end if

align 4
data fixups
end data

ctrl ES_CTRL

src_save       rd 1
src_orig       rd 1
if VERBOSE
irq_seen       rd 1
end if
