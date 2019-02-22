;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native 0.05
entry START

DEBUG           equ 1
DEBUG_IRQ       equ 0

API_VERSION     equ 0x01000100


;irq 0,13 unavailable
;                   FEDCBA9876543210
VALID_IRQ       equ 1101111111111110b

CPU_FREQ        equ  2000d

BIT0  EQU 0x00000001
BIT1  EQU 0x00000002
BIT2  EQU 0x00000004
BIT3  EQU 0x00000008
BIT4  EQU 0x00000010
BIT5  EQU 0x00000020
BIT6  EQU 0x00000040
BIT7  EQU 0x00000080
BIT8  EQU 0x00000100
BIT9  EQU 0x00000200
BIT10 EQU 0x00000400
BIT11 EQU 0x00000800
BIT12 EQU 0x00001000
BIT13 EQU 0x00002000
BIT14 EQU 0x00004000
BIT15 EQU 0x00008000
BIT16 EQU 0x00010000
BIT17 EQU 0x00020000
BIT18 EQU 0x00040000
BIT19 EQU 0x00080000
BIT20 EQU 0x00100000
BIT21 EQU 0x00200000
BIT22 EQU 0x00400000
BIT23 EQU 0x00800000
BIT24 EQU 0x00100000
BIT25 EQU 0x02000000
BIT26 EQU 0x04000000
BIT27 EQU 0x08000000
BIT28 EQU 0x10000000
BIT29 EQU 0x20000000
BIT30 EQU 0x40000000
BIT31 EQU 0x80000000

VID_FM801         equ 0x1319
CTRL_FM801        equ 0x0801

FM_PCM_VOLUME         equ 0x00
FM_FM_VOLUME          equ 0x02
FM_I2S_VOLUME         equ 0x04
FM_RECORD_SOURCE      equ 0x06

FM_PLAY_CTL           equ 0x08
FM_PLAY_RATE_MASK     equ 0x0f00
FM_PLAY_BUF1_LAST     equ 0x0001
FM_PLAY_BUF2_LAST     equ 0x0002
FM_PLAY_START         equ 0x0020
FM_PLAY_PAUSE         equ 0x0040
FM_PLAY_STOPNOW       equ 0x0080
FM_PLAY_16BIT         equ 0x4000
FM_PLAY_STEREO        equ 0x8000

FM_PLAY_DMALEN        equ 0x0a
FM_PLAY_DMABUF1       equ 0x0c
FM_PLAY_DMABUF2       equ 0x10

FM_REC_CTL            equ 0x14
FM_REC_RATE_MASK      equ 0x0f00
FM_REC_BUF1_LAST      equ 0x0001
FM_REC_BUF2_LAST      equ 0x0002
FM_REC_START          equ 0x0020
FM_REC_PAUSE          equ 0x0040
FM_REC_STOPNOW        equ 0x0080
FM_REC_16BIT          equ 0x4000
FM_REC_STEREO         equ 0x8000

FM_REC_DMALEN         equ 0x16
FM_REC_DMABUF1        equ 0x18
FM_REC_DMABUF2        equ 0x1c

FM_CODEC_CTL          equ 0x22
FM_VOLUME             equ 0x26
FM_VOLUME_MUTE        equ 0x8000

FM_CODEC_CMD          equ 0x2a
FM_CODEC_CMD_READ     equ 0x0080
FM_CODEC_CMD_VALID    equ 0x0100
FM_CODEC_CMD_BUSY     equ 0x0200

FM_CODEC_DATA         equ 0x2c

FM_IO_CTL             equ 0x52
FM_CARD_CTL           equ 0x54

FM_INTMASK            equ 0x56
FM_INTMASK_PLAY       equ 0x0001
FM_INTMASK_REC        equ 0x0002
FM_INTMASK_VOL        equ 0x0040
FM_INTMASK_MPU        equ 0x0080

FM_INTSTATUS          equ 0x5a
FM_INTSTATUS_PLAY     equ 0x0100
FM_INTSTATUS_REC      equ 0x0200
FM_INTSTATUS_VOL      equ 0x4000
FM_INTSTATUS_MPU      equ 0x8000

CODEC_MASTER_VOL_REG         equ 0x02 ;
CODEC_AUX_VOL                equ 0x04 ;
CODEC_PCM_OUT_REG            equ 0x18 ; PCM output volume
CODEC_EXT_AUDIO_REG          equ 0x28 ; extended audio
CODEC_EXT_AUDIO_CTRL_REG     equ 0x2a ; extended audio control
CODEC_PCM_FRONT_DACRATE_REG  equ 0x2c ; PCM out sample rate
CODEC_PCM_SURND_DACRATE_REG  equ 0x2e ; surround sound sample rate
CODEC_PCM_LFE_DACRATE_REG    equ 0x30 ; LFE sample rate

SRV_GETVERSION        equ  0
DEV_PLAY              equ  1
DEV_STOP              equ  2
DEV_CALLBACK          equ  3
DEV_SET_BUFF          equ  4
DEV_NOTIFY            equ  5
DEV_SET_MASTERVOL     equ  6
DEV_GET_MASTERVOL     equ  7
DEV_GET_INFO          equ  8

struc AC_CNTRL    ;AC controller base class
{ .bus                dd ?
  .devfn              dd ?

  .vendor             dd ?
  .dev_id             dd ?
  .pci_cmd            dd ?
  .pci_stat           dd ?

  .codec_io_base      dd ?
  .codec_mem_base     dd ?

  .ctrl_io_base       dd ?
  .ctrl_mem_base      dd ?
  .cfg_reg            dd ?
  .int_line           dd ?

  .vendor_ids         dd ?    ;vendor id string
  .ctrl_ids           dd ?    ;hub id string

  .buffer             dd ?

  .notify_pos         dd ?
  .notify_task        dd ?

  .lvi_reg            dd ?
  .ctrl_setup         dd ?
  .user_callback      dd ?
  .codec_read16       dd ?
  .codec_write16      dd ?

  .ctrl_read8         dd ?
  .ctrl_read16        dd ?
  .ctrl_read32        dd ?

  .ctrl_write8        dd ?
  .ctrl_write16       dd ?
  .ctrl_write32       dd ?
}

struc CODEC   ;Audio Chip base class
{
  .chip_id            dd ?
  .flags              dd ?
  .status             dd ?

  .ac_vendor_ids      dd ?    ;ac vendor id string
  .chip_ids           dd ?    ;chip model string

  .shadow_flag        dd ?
                      dd ?

  .regs               dw ?     ; codec registers
  .reg_master_vol     dw ?     ;0x02
  .reg_aux_out_vol    dw ?     ;0x04
  .reg_mone_vol       dw ?     ;0x06
  .reg_master_tone    dw ?     ;0x08
  .reg_beep_vol       dw ?     ;0x0A
  .reg_phone_vol      dw ?     ;0x0C
  .reg_mic_vol        dw ?     ;0x0E
  .reg_line_in_vol    dw ?     ;0x10
  .reg_cd_vol         dw ?     ;0x12
  .reg_video_vol      dw ?     ;0x14
  .reg_aux_in_vol     dw ?     ;0x16
  .reg_pcm_out_vol    dw ?     ;0x18
  .reg_rec_select     dw ?     ;0x1A
  .reg_rec_gain       dw ?     ;0x1C
  .reg_rec_gain_mic   dw ?     ;0x1E
  .reg_gen            dw ?     ;0x20
  .reg_3d_ctrl        dw ?     ;0X22
  .reg_page           dw ?     ;0X24
  .reg_powerdown      dw ?     ;0x26
  .reg_ext_audio      dw ?     ;0x28
  .reg_ext_st         dw ?     ;0x2a
  .reg_pcm_front_rate dw ?     ;0x2c
  .reg_pcm_surr_rate  dw ?     ;0x2e
  .reg_lfe_rate       dw ?     ;0x30
  .reg_pcm_in_rate    dw ?     ;0x32
                      dw ?     ;0x34
  .reg_cent_lfe_vol   dw ?     ;0x36
  .reg_surr_vol       dw ?     ;0x38
  .reg_spdif_ctrl     dw ?     ;0x3A
                      dw ?     ;0x3C
                      dw ?     ;0x3E
                      dw ?     ;0x40
                      dw ?     ;0x42
                      dw ?     ;0x44
                      dw ?     ;0x46
                      dw ?     ;0x48
                      dw ?     ;0x4A
                      dw ?     ;0x4C
                      dw ?     ;0x4E
                      dw ?     ;0x50
                      dw ?     ;0x52
                      dw ?     ;0x54
                      dw ?     ;0x56
                      dw ?     ;0x58
                      dw ?     ;0x5A
                      dw ?     ;0x5C
                      dw ?     ;0x5E
  .reg_page_0         dw ?     ;0x60
  .reg_page_1         dw ?     ;0x62
  .reg_page_2         dw ?     ;0x64
  .reg_page_3         dw ?     ;0x66
  .reg_page_4         dw ?     ;0x68
  .reg_page_5         dw ?     ;0x6A
  .reg_page_6         dw ?     ;0x6C
  .reg_page_7         dw ?     ;0x6E
                      dw ?     ;0x70
                      dw ?     ;0x72
                      dw ?     ;0x74
                      dw ?     ;0x76
                      dw ?     ;0x78
                      dw ?     ;0x7A
  .reg_vendor_id_1    dw ?     ;0x7C
  .reg_vendor_id_2    dw ?     ;0x7E


  .reset              dd ?    ;virual
  .set_master_vol     dd ?
}

struc CTRL_INFO
{   .pci_cmd          dd ?
    .irq              dd ?
    .glob_cntrl       dd ?
    .glob_sta         dd ?
    .codec_io_base    dd ?
    .ctrl_io_base     dd ?
    .codec_mem_base   dd ?
    .ctrl_mem_base    dd ?
    .codec_id         dd ?
}

EVENT_NOTIFY      equ 0x00000200

section '.flat' code readable writable executable
include '../struct.inc'
include '../macros.inc'
include '../proc32.inc'
include '../peimport.inc'

proc START c uses ebx esi edi, state:dword, cmdline:dword

        cmp     [state], 1
        jne     .stop

     if DEBUG
        mov     eax, START
        call    dword2str
        invoke  SysMsgBoardStr
        mov     esi, msgInit
        invoke  SysMsgBoardStr
     end if

        call    detect_controller
        test    eax, eax
        jz      .fail

     if DEBUG
        mov     esi, [ctrl.vendor_ids]
        invoke  SysMsgBoardStr
        mov     esi, [ctrl.ctrl_ids]
        invoke  SysMsgBoardStr

     end if

        call    init_controller
        test    eax, eax
        jz      .fail

        call    init_codec
        test    eax, eax
        jz      .fail

        call    reset_controller
        call    setup_codec

        mov     esi, msgPrimBuff
        invoke  SysMsgBoardStr
        call    create_primary_buff

        mov     esi, msgDone
        invoke  SysMsgBoardStr

        mov     eax, VALID_IRQ
        mov     ebx, [ctrl.int_line]
        mov     esi, msgInvIRQ
        bt      eax, ebx
        jnc     .fail_msg

        invoke  AttachIntHandler, ebx, ac97_irq, dword 0
.reg:

        invoke  RegService, sz_sound_srv, service_proc
        ret
.fail:
   if DEBUG
        mov     esi, msgFail
        invoke  SysMsgBoardStr
   end if
        xor     eax, eax
        ret
.fail_msg:
        invoke  SysMsgBoardStr
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
        call    set_master_vol      ;eax= vol
        ret
@@:
        cmp     eax, DEV_GET_MASTERVOL
        jne     @F
        mov     ebx, [edi+output]
        stdcall get_master_vol, ebx
        ret
;@@:
;           cmp eax, DEV_GET_INFO
;           jne @F
;           mov ebx, [edi+output]
;           stdcall get_dev_info, ebx
;           ret
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
proc fill_buffer

        cmp     [ctrl.user_callback], 0
        je      .exit

        mov     esi, [ctrl.buffer]
        mov     eax, int_flip_flop
        inc     dword [eax]
        test    dword [eax], 1
        je      @f
        add     esi, 0x4000
@@:
        stdcall [ctrl.user_callback], esi

        mov     edx, FM_PLAY_DMABUF1
        mov     eax, [buffer_pgaddr]
        mov     esi, int_flip_flop
        test    dword [esi], 1
        je      @f
        mov     edx, FM_PLAY_DMABUF2
        add     eax, 0x4000
@@:
        call    [ctrl.ctrl_write32]

.exit:
        ret
endp

align 4
proc ac97_irq

     if DEBUG_IRQ
        mov     esi, msgIRQ
        invoke  SysMsgBoardStr
     end if

        mov     edx, FM_INTSTATUS
        call    [ctrl.ctrl_read16]

        test    eax, FM_INTSTATUS_PLAY
        je      .exit

        push    eax
        call    fill_buffer
        pop     eax

.exit:
        mov     edx, FM_INTSTATUS
        call    [ctrl.ctrl_write16]

        ret
endp

align 4
proc create_primary_buff

        invoke  KernelAlloc, 0x10000
        mov     [ctrl.buffer], eax

        mov     edi, eax
        mov     ecx, 0x10000/4
        xor     eax, eax
        cld
        rep stosd

        mov     eax, [ctrl.buffer]
        invoke  GetPgAddr
        mov     [buffer_pgaddr], eax

        ret
endp

align 4
proc detect_controller
           locals
             last_bus dd ?
             bus      dd ?
             devfn    dd ?
           endl

        xor     eax, eax
        mov     [bus], eax
        inc     eax
        invoke  PciApi
        cmp     eax, -1
        je      .err

        mov     [last_bus], eax

.next_bus:
        and     [devfn], 0
.next_dev:
        invoke  PciRead32, [bus], [devfn], dword 0
        test    eax, eax
        jz      .next
        cmp     eax, -1
        je      .next

        push    eax
        invoke  PciRead32, [bus], [devfn], dword 0x09
        and     eax, 0xffffff
        cmp     eax, 0x060100 ;pci-isa
        jne     .no_bridge

        mov     eax, [bus]
        mov     [brg_bus], eax
        mov     eax, [devfn]
        mov     [brg_devfn], eax
.no_bridge:
        pop     eax

        mov     edi, devices
@@:
        mov     ebx, [edi]
        test    ebx, ebx
        jz      .next

        cmp     eax, ebx
        je      .found
        add     edi, 12
        jmp     @B
.next:
        inc     [devfn]
        cmp     [devfn], 256
        jb      .next_dev
        mov     eax, [bus]
        inc     eax
        mov     [bus], eax
        cmp     eax, [last_bus]
        jna     .next_bus
        xor     eax, eax
        ret
.found:
        mov     ebx, [bus]
        mov     [ctrl.bus], ebx

        mov     ecx, [devfn]
        mov     [ctrl.devfn], ecx

        mov     edx, eax
        and     edx, 0xFFFF
        mov     [ctrl.vendor], edx
        shr     eax, 16
        mov     [ctrl.dev_id], eax

        mov     ebx, [edi+4]
        mov     [ctrl.ctrl_ids], ebx
        mov     [ctrl.vendor_ids], msg_FM

        mov     esi, [edi+8]
        mov     [ctrl.ctrl_setup], esi
        ret
.err:
        xor     eax, eax
        ret
endp

align 4
proc init_controller

        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 4
        mov     ebx, eax
        and     eax, 0xFFFF
        mov     [ctrl.pci_cmd], eax
        shr     ebx, 16
        mov     [ctrl.pci_stat], ebx

        mov     esi, msgPciCmd
        invoke  SysMsgBoardStr
        call    dword2str
        invoke  SysMsgBoardStr

        mov     esi, msgPciStat
        invoke  SysMsgBoardStr
        mov     eax, [ctrl.pci_stat]
        call    dword2str
        invoke  SysMsgBoardStr

        mov     esi, msgCtrlIsaIo
        invoke  SysMsgBoardStr

        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 0x10

        call    dword2str
        invoke  SysMsgBoardStr

        and     eax, 0xFFFE
        mov     [ctrl.ctrl_io_base], eax

        mov     esi, msgIrqNum
        invoke  SysMsgBoardStr

        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], 0x3C
        and     eax, 0xFF
        mov     [ctrl.int_line], eax

        call    dword2str
        invoke  SysMsgBoardStr

        call    [ctrl.ctrl_setup]
        xor     eax, eax
        inc     eax
        ret
endp

align 4
proc set_FM
        mov     [ctrl.codec_read16], codec_io_r16    ;virtual
        mov     [ctrl.codec_write16], codec_io_w16   ;virtual

        mov     [ctrl.ctrl_read8 ], ctrl_io_r8      ;virtual
        mov     [ctrl.ctrl_read16], ctrl_io_r16      ;virtual
        mov     [ctrl.ctrl_read32], ctrl_io_r32      ;virtual

        mov     [ctrl.ctrl_write8 ], ctrl_io_w8     ;virtual
        mov     [ctrl.ctrl_write16], ctrl_io_w16     ;virtual
        mov     [ctrl.ctrl_write32], ctrl_io_w32     ;virtual
        ret
endp

align 4
proc reset_controller

        mov     esi, msgInitCtrl
        invoke  SysMsgBoardStr

        mov     edx, FM_CARD_CTL
        call    [ctrl.ctrl_read8]
        push    eax
        or      al, 1
        mov     edx, FM_CARD_CTL
        call    [ctrl.ctrl_write8]
        mov     eax, 10
        call    StallExec
        pop     eax
        and     al, 0xFE
        mov     edx, FM_CARD_CTL
        call    [ctrl.ctrl_write8]
        mov     eax, 10
        call    StallExec

        mov     eax, 0x0404
        mov     edx, FM_PCM_VOLUME
        call    [ctrl.ctrl_write16]
        mov     edx, FM_FM_VOLUME
        call    [ctrl.ctrl_write16]
        mov     edx, FM_I2S_VOLUME
        call    [ctrl.ctrl_write16]

        mov     edx, FM_INTMASK
        call    [ctrl.ctrl_read16]
        and     eax, not FM_INTMASK_PLAY
        or      eax, FM_INTMASK_REC or FM_INTMASK_MPU or FM_INTMASK_VOL
        mov     edx, FM_INTMASK
        call    [ctrl.ctrl_write16]

        mov     eax, FM_INTMASK_PLAY or FM_INTMASK_REC or FM_INTMASK_MPU or FM_INTMASK_VOL
        mov     edx, FM_INTSTATUS
        call    [ctrl.ctrl_write16]

        ret
endp

align 4
proc init_codec

        mov     esi, msgInitCodec
        invoke  SysMsgBoardStr

        mov     al, FM_CODEC_CMD_READ
        mov     edx, FM_CODEC_CMD
        call    [ctrl.ctrl_write8]

        call    reset_codec

        call    detect_codec

        xor     eax, eax
        inc     eax
        ret
endp

align 4
proc reset_codec

        mov     ecx, 255
.L1:
        mov     edx, FM_CODEC_CMD
        call    [ctrl.ctrl_read16]
        test    ah, FM_CODEC_CMD_VALID shr 8
        jne     .L2
        loop    .L1
.L2:
        mov     edx, FM_CODEC_CTL
        call    [ctrl.ctrl_read8]
        push    eax
        or      al, 0x20
        mov     edx, FM_CODEC_CTL
        call    [ctrl.ctrl_write8]
        pop     eax
        and     al, 0xDF
        mov     edx, FM_CODEC_CTL
        call    [ctrl.ctrl_write8]

        xor     eax, eax
        inc     eax
        ret
endp

align 4
play:
        mov     eax, 0x4000-1
        mov     edx, FM_PLAY_DMALEN
        call    [ctrl.ctrl_write16]

        call    fill_buffer

        mov     eax, FM_PLAY_START or FM_PLAY_STOPNOW or FM_PLAY_STEREO or FM_PLAY_16BIT or 0xA00
        mov     edx, FM_PLAY_CTL
        call    [ctrl.ctrl_write16]

        xor     eax, eax
        ret

align 4
stop:
        mov     edx, FM_PLAY_CTL
        call    [ctrl.ctrl_read16]
        and     eax, not (FM_PLAY_START or FM_PLAY_STOPNOW)
        or      eax, FM_PLAY_BUF1_LAST or FM_PLAY_BUF2_LAST
        mov     edx, FM_PLAY_CTL
        call    [ctrl.ctrl_write16]

        xor     eax, eax
        ret

align 4
proc get_dev_info stdcall, p_info:dword
           virtual at esi
             CTRL_INFO CTRL_INFO
           end virtual

        mov     esi, [p_info]
        mov     eax, [ctrl.int_line]
        mov     ebx, [ctrl.codec_io_base]
        mov     ecx, [ctrl.ctrl_io_base]
        mov     edx, [ctrl.codec_mem_base]
        mov     edi, [ctrl.ctrl_mem_base]

        mov     [CTRL_INFO.irq], eax
        mov     [CTRL_INFO.codec_io_base], ebx
        mov     [CTRL_INFO.ctrl_io_base], ecx
        mov     [CTRL_INFO.codec_mem_base], edx
        mov     [CTRL_INFO.ctrl_mem_base], edi

        mov     eax, [codec.chip_id]
        mov     [CTRL_INFO.codec_id], eax

        mov     ebx, [ctrl.pci_cmd]
        mov     [CTRL_INFO.pci_cmd], ebx
        ret
endp

align 4
proc set_callback stdcall, handler:dword
        mov     eax, [handler]
        mov     [ctrl.user_callback], eax
        ret
endp

align 4
proc codec_read stdcall, ac_reg:dword   ; reg = edx, reval = eax

        mov     edx, [ac_reg]

        mov     ebx, edx
        shr     ebx, 1
        bt      [codec.shadow_flag], ebx
        jc      .use_shadow

        call    [ctrl.codec_read16]  ;change edx !!!
        mov     ecx, eax

.read_ok:
        mov     edx, [ac_reg]
        mov     [codec.regs+edx], cx
        bts     [codec.shadow_flag], ebx
        mov     eax, ecx
        ret
.use_shadow:
        movzx   eax, word [codec.regs+edx]
        ret

endp

align 4
proc codec_write stdcall, ac_reg:dword

        mov     esi, [ac_reg]

        mov     edx, esi

        call    [ctrl.codec_write16]

        mov     [codec.regs+esi], ax
        shr     esi, 1
        bts     [codec.shadow_flag], esi

        ret
endp

align 4
proc check_semafore
align 4
.ok:
        xor     eax, eax
        inc     eax
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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;          CONTROLLER IO functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc codec_io_r16

        push    edx
        mov     ecx, 255
.L1:
        mov     edx, FM_CODEC_CMD
        call    [ctrl.ctrl_read16]
        test    ah, FM_CODEC_CMD_BUSY shr 8
        je      .L2
        loop    .L1
.L2:
        pop     eax
        or      al, FM_CODEC_CMD_READ
        mov     edx, FM_CODEC_CMD
        call    [ctrl.ctrl_write8]

        mov     ecx, 255
.L3:
        mov     edx, FM_CODEC_CMD
        call    [ctrl.ctrl_read16]
        test    ah, FM_CODEC_CMD_VALID shr 8
        jne     .L4
        loop    .L3
.L4:
        mov     edx, FM_CODEC_DATA
        call    [ctrl.ctrl_read16]

        ret
endp

align 4
proc codec_io_w16

        push    edx
        push    eax
        mov     ecx, 255
.L1:
        mov     edx, FM_CODEC_CMD
        call    [ctrl.ctrl_read16]
        test    ah, FM_CODEC_CMD_BUSY shr 8
        je      .L2
        loop    .L1
.L2:
        pop     eax
        mov     edx, FM_CODEC_DATA
        call    [ctrl.ctrl_write16]

        pop     eax
        mov     edx, FM_CODEC_CMD
        call    [ctrl.ctrl_write16]

        ret
endp

align 4
proc ctrl_io_r8
        add     edx, [ctrl.ctrl_io_base]
        in      al, dx
        ret
endp

align 4
proc ctrl_io_r16
        add     edx, [ctrl.ctrl_io_base]
        in      ax, dx
        ret
endp

align 4
proc ctrl_io_r32
        add     edx, [ctrl.ctrl_io_base]
        in      eax, dx
        ret
endp

align 4
proc ctrl_io_w8
        add     edx, [ctrl.ctrl_io_base]
        out     dx, al
        ret
endp

align 4
proc ctrl_io_w16
        add     edx, [ctrl.ctrl_io_base]
        out     dx, ax
        ret
endp

align 4
proc ctrl_io_w32
        add     edx, [ctrl.ctrl_io_base]
        out     dx, eax
        ret
endp

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
brg_bus      dd ?
brg_devfn    dd ?
include "codec.inc"

align 4
devices dd (CTRL_FM801 shl 16)+VID_FM801, msg_FM801, set_FM
        dd 0

msg_FM801    db 'FM801 AC97 controller',13,10, 0
msg_FM       db 'Forte Media',13,10, 0

sz_sound_srv db 'SOUND',0

msgInit       db 'detect hardware...',13,10,0
msgFail       db 'device not found',13,10,0
msgInvIRQ     db 'IRQ line not assigned or invalid', 13,10, 0
msgPlay       db 'start play', 13,10,0
msgStop       db 'stop play',  13,10,0
;msgNotify    db 'call notify',13,10,0
msgIRQ        db 'AC97 IRQ', 13,10,0
msgInitCtrl  db 'init controller',13,10,0
msgInitCodec db 'init codec',13,10,0
msgPrimBuff   db 'create primary buffer ...',0
msgDone       db 'done',13,10,0
;msgReg       db 'set service handler',13,10,0
;msgOk        db 'service installed',13,10,0
;msgStatus    db 'global status   ',0
;msgControl   db 'global control  ',0
msgPciCmd     db 'PCI command     ',0
msgPciStat    db 'PCI status      ',0
msgCtrlIsaIo  db 'controller io base   ',0
msgIrqNum     db 'IRQ default          ',0
;msgIrqMap    db 'AC97 irq map as      ',0

align 4
data fixups
end data

codec CODEC
ctrl AC_CNTRL

int_flip_flop      rd 1
buffer_pgaddr      rd 1
