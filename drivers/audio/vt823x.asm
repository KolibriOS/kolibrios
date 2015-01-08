;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format PE DLL native 0.05
entry START

DEBUG           equ 1

API_VERSION     equ 0x01000100

USE_COM_IRQ     equ 0    ;make irq 3 and irq 4 available for PCI devices
IRQ_REMAP       equ 0
IRQ_LINE        equ 0


;irq 0,1,2,8,12,13 unavailable
;                   FEDCBA9876543210
VALID_IRQ       equ 1100111011111000b
ATTCH_IRQ       equ 0000111010100000b

if USE_COM_IRQ
ATTCH_IRQ       equ 0000111010111000b
end if

CPU_FREQ        equ  2600d

BIT0  EQU 0x00000001
BIT1  EQU 0x00000002
BIT5  EQU 0x00000020
BIT10 EQU 0x00000400

VID_VIA           equ 0x1106

CTRL_VT82C686     equ 0x3058
CTRL_VT8233_5     equ 0x3059


CODEC_MASTER_VOL_REG         equ 0x02
CODEC_AUX_VOL                equ 0x04 ;
CODEC_PCM_OUT_REG            equ 0x18 ; PCM output volume
CODEC_EXT_AUDIO_REG          equ 0x28 ; extended audio
CODEC_EXT_AUDIO_CTRL_REG     equ 0x2a ; extended audio control
CODEC_PCM_FRONT_DACRATE_REG  equ 0x2c ; PCM out sample rate
CODEC_PCM_SURND_DACRATE_REG  equ 0x2e ; surround sound sample rate
CODEC_PCM_LFE_DACRATE_REG    equ 0x30 ; LFE sample rate


;VIA host controller registers set
;; common offsets
VIA_REG_OFFSET_STATUS        equ   0x00    ;; byte - channel status
  VIA_REG_STAT_ACTIVE              equ   0x80    ;; RO
  VIA_REG_STAT_PAUSED              equ   0x40    ;; RO
  VIA_REG_STAT_TRIGGER_QUEUED      equ   0x08    ;; RO
  VIA_REG_STAT_STOPPED             equ   0x04    ;; RWC
  VIA_REG_STAT_EOL                 equ   0x02    ;; RWC
  VIA_REG_STAT_FLAG                equ   0x01    ;; RWC
VIA_REG_OFFSET_CONTROL       equ   0x01    ;; byte - channel control
  VIA_REG_CTRL_START               equ   0x80    ;; WO
  VIA_REG_CTRL_TERMINATE           equ   0x40    ;; WO
  VIA_REG_CTRL_AUTOSTART           equ   0x20
  VIA_REG_CTRL_PAUSE               equ   0x08    ;; RW
  VIA_REG_CTRL_INT_STOP            equ   0x04
  VIA_REG_CTRL_INT_EOL             equ   0x02
  VIA_REG_CTRL_INT_FLAG            equ   0x01
  VIA_REG_CTRL_RESET               equ   0x01    ;; RW - probably reset? undocumented
  VIA_REG_CTRL_INT                 equ  (VIA_REG_CTRL_INT_FLAG or \
                                         VIA_REG_CTRL_INT_EOL or \
                                         VIA_REG_CTRL_AUTOSTART)
VIA_REG_OFFSET_TYPE          equ   0x02    ;; byte - channel type (686 only)
  VIA_REG_TYPE_AUTOSTART           equ   0x80    ;; RW - autostart at EOL
  VIA_REG_TYPE_16BIT               equ   0x20    ;; RW
  VIA_REG_TYPE_STEREO              equ   0x10    ;; RW
  VIA_REG_TYPE_INT_LLINE           equ   0x00
  VIA_REG_TYPE_INT_LSAMPLE         equ   0x04
  VIA_REG_TYPE_INT_LESSONE         equ   0x08
  VIA_REG_TYPE_INT_MASK            equ   0x0c
  VIA_REG_TYPE_INT_EOL             equ   0x02
  VIA_REG_TYPE_INT_FLAG            equ   0x01
VIA_REG_OFFSET_TABLE_PTR     equ   0x04    ;; dword - channel table pointer
VIA_REG_OFFSET_CURR_PTR      equ   0x04    ;; dword - channel current pointer
VIA_REG_OFFSET_STOP_IDX      equ   0x08    ;; dword - stop index, channel type, sample rate
  VIA8233_REG_TYPE_16BIT           equ   0x00200000      ;; RW
  VIA8233_REG_TYPE_STEREO          equ   0x00100000      ;; RW
VIA_REG_OFFSET_CURR_COUNT    equ   0x0c    ;; dword - channel current count (24 bit)
VIA_REG_OFFSET_CURR_INDEX    equ   0x0f    ;; byte - channel current index (for via8233 only)


VIADEV_PLAYBACK         equ   0x00
VIADEV_CAPTURE          equ   0x10
VIADEV_FM               equ   0x20

;; AC'97 ;;
VIA_REG_AC97             equ   0x80    ; dword
  VIA_REG_AC97_CODEC_ID_MASK       equ  0xC0000000 ;(3<<30)
  VIA_REG_AC97_CODEC_ID_SHIFT      equ  30
  VIA_REG_AC97_CODEC_ID_PRIMARY    equ  0x00
  VIA_REG_AC97_CODEC_ID_SECONDARY  equ  0x01
  VIA_REG_AC97_SECONDARY_VALID     equ  0x08000000 ;(1<<27)
  VIA_REG_AC97_PRIMARY_VALID       equ  0x02000000 ;(1<<25)
  VIA_REG_AC97_BUSY                equ  0x01000000 ;(1<<24)
  VIA_REG_AC97_READ                equ  0x00800000 ;(1<<23)
  VIA_REG_AC97_CMD_SHIFT           equ  16
  VIA_REG_AC97_CMD_MASK            equ  0x7E
  VIA_REG_AC97_DATA_SHIFT          equ  0
  VIA_REG_AC97_DATA_MASK           equ  0xFFFF

VIA_REG_SGD_SHADOW       equ   0x84    ; dword

;; via8233-specific registers ;;
VIA_REG_OFS_PLAYBACK_VOLUME_L   equ  0x02    ;; byte
VIA_REG_OFS_PLAYBACK_VOLUME_R   equ  0x03    ;; byte
VIA_REG_OFS_MULTPLAY_FORMAT     equ  0x02    ;; byte - format and channels
  VIA_REG_MULTPLAY_FMT_8BIT          equ  0x00
  VIA_REG_MULTPLAY_FMT_16BIT         equ  0x80
  VIA_REG_MULTPLAY_FMT_CH_MASK       equ  0x70    ;; # channels << 4 (valid = 1,2,4,6)
VIA_REG_OFS_CAPTURE_FIFO        equ  0x02    ;; byte - bit 6 = fifo  enable
  VIA_REG_CAPTURE_FIFO_ENABLE        equ  0x40

VIA_DXS_MAX_VOLUME              equ  31      ;; max. volume (attenuation) of reg 0x32/33

VIA_TBL_BIT_FLAG          equ   0x40000000
VIA_TBL_BIT_EOL           equ   0x80000000

;; pci space ;;
VIA_ACLINK_STAT           equ   0x40
  ;...
  VIA_ACLINK_C00_READY             equ   0x01 ; primary codec ready
VIA_ACLINK_CTRL           equ   0x41
  VIA_ACLINK_CTRL_ENABLE           equ   0x80 ; 0: disable, 1: enable
  VIA_ACLINK_CTRL_RESET            equ   0x40 ; 0: assert, 1: de-assert
  VIA_ACLINK_CTRL_SYNC             equ   0x20 ; 0: release SYNC, 1: force SYNC hi
  VIA_ACLINK_CTRL_SDO              equ   0x10 ; 0: release SDO, 1: force SDO hi
  VIA_ACLINK_CTRL_VRA              equ   0x08 ; 0: disable VRA, 1: enable VRA
  VIA_ACLINK_CTRL_PCM              equ   0x04 ; 0: disable PCM, 1: enable PCM
  VIA_ACLINK_CTRL_FM               equ   0x02 ; via686 only
  VIA_ACLINK_CTRL_SB               equ   0x01 ; via686 only
  VIA_ACLINK_CTRL_INIT             equ  (VIA_ACLINK_CTRL_ENABLE or \
                                         VIA_ACLINK_CTRL_RESET or \
                                         VIA_ACLINK_CTRL_PCM or \
                                         VIA_ACLINK_CTRL_VRA)
VIA_FUNC_ENABLE           equ   0x42
  VIA_FUNC_MIDI_PNP                equ   0x80 ; FIXME: it's 0x40 in the datasheet!
  VIA_FUNC_MIDI_IRQMASK            equ   0x40 ; FIXME: not documented!
  VIA_FUNC_RX2C_WRITE              equ   0x20
  VIA_FUNC_SB_FIFO_EMPTY           equ   0x10
  VIA_FUNC_ENABLE_GAME             equ   0x08
  VIA_FUNC_ENABLE_FM               equ   0x04
  VIA_FUNC_ENABLE_MIDI             equ   0x02
  VIA_FUNC_ENABLE_SB               equ   0x01
VIA_PNP_CONTROL           equ   0x43
VIA_FM_NMI_CTRL           equ   0x48
VIA8233_VOLCHG_CTRL       equ   0x48
VIA8233_SPDIF_CTRL        equ   0x49
  VIA8233_SPDIF_DX3                equ   0x08
  VIA8233_SPDIF_SLOT_MASK          equ   0x03
  VIA8233_SPDIF_SLOT_1011          equ   0x00
  VIA8233_SPDIF_SLOT_34            equ   0x01
  VIA8233_SPDIF_SLOT_78            equ   0x02
  VIA8233_SPDIF_SLOT_69            equ   0x03
;] Asper


SRV_GETVERSION        equ  0
DEV_PLAY              equ  1
DEV_STOP              equ  2
DEV_CALLBACK          equ  3
DEV_SET_BUFF          equ  4
DEV_NOTIFY            equ  5
DEV_SET_MASTERVOL     equ  6
DEV_GET_MASTERVOL     equ  7
DEV_GET_INFO          equ  8
DEV_GET_POS           equ  9

struc AC_CNTRL              ;AC controller base class
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

struc CODEC                ;Audio Chip base class
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


  .reset              dd ?    ;virtual
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

EVENT_NOTIFY    equ 0x00000200

section '.flat' code readable writable executable
include '../struct.inc'
include '../macros.inc'
include '../proc32.inc'
include '../peimport.inc'

proc START c, state:dword, cmdline:dword

        cmp     [state], 1
        jne     .stop

     if DEBUG
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

        call    setup_codec

        mov     esi, msgPrimBuff
        invoke  SysMsgBoardStr
        call    create_primary_buff
        mov     esi, msgDone
        invoke  SysMsgBoardStr

  if IRQ_REMAP
        pushf
        cli

        mov     ebx, [ctrl.int_line]
        in      al, 0xA1
        mov     ah, al
        in      al, 0x21
        test    ebx, ebx
        jz      .skip
        bts     ax, bx                          ;mask old line
.skip:
        bts     ax, IRQ_LINE                    ;mask new ine
        out     0x21, al
        mov     al, ah
        out     0xA1, al

        invoke  PciWrite8, 0, 0xF8, 0x61, IRQ_LINE      ;remap IRQ

        mov     dx, 0x4d0                       ;8259 ELCR1
        in      al, dx
        bts     ax, IRQ_LINE
        out     dx, al                          ;set level-triggered mode
        mov     [ctrl.int_line], IRQ_LINE
        popf
        mov     esi, msgRemap
        invoke  SysMsgBoardStr
  end if

        mov     eax, VALID_IRQ
        mov     ebx, [ctrl.int_line]
        mov     esi, msgInvIRQ
        bt      eax, ebx
        jnc     .fail_msg
        mov     eax, ATTCH_IRQ
        mov     esi, msgAttchIRQ
        bt      eax, ebx
        jnc     .fail_msg

        invoke  AttachIntHandler, ebx, ac97_irq_VIA, 0
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
        call    set_master_vol          ;eax= vol
        ret
@@:
        cmp     eax, DEV_GET_MASTERVOL
        jne     @F
        mov     ebx, [edi+output]
        stdcall get_master_vol, ebx
        ret
@@:
        cmp     eax, DEV_GET_INFO
        jne     @F
        mov     ebx, [edi+output]
        stdcall get_dev_info, ebx
        ret
@@:
        cmp     eax, DEV_GET_POS
        jne     @F
        push    ebx  edx
        mov     edx, VIADEV_PLAYBACK + VIA_REG_OFFSET_CURR_COUNT
        call    [ctrl.ctrl_read32]
        and     eax, 0x00FFFFFF
        mov     ebx, 4096
        sub     ebx, eax
        shr     ebx, 2
        mov     edx, [edi+output]
        mov     [edx], ebx
        pop     edx ebx
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
proc ac97_irq_VIA
           locals
             status db 0
           endl

        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_STATUS
        call    [ctrl.ctrl_read8]
        test    al, VIA_REG_STAT_ACTIVE
        jz      @f

        and     al, VIA_REG_STAT_EOL or VIA_REG_STAT_FLAG or VIA_REG_STAT_STOPPED
        mov     byte [status], al

        mov     ebx, dword [buff_list]
        cmp     [ctrl.user_callback], 0
        je      @f
        stdcall [ctrl.user_callback], ebx
       @@:
        mov     al, byte [status]               ;; ack ;;
        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_STATUS
        call    [ctrl.ctrl_write8]

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
        mov     edi, pcmout_bdl
        stosd
        mov     eax, 0x80004000
        stosd

        mov     edi, buff_list
        mov     eax, [ctrl.buffer]
        mov     ecx, 4
@@:
        mov     [edi], eax
        mov     [edi+16], eax
        mov     [edi+32], eax
        mov     [edi+48], eax
        mov     [edi+64], eax
        mov     [edi+80], eax
        mov     [edi+96], eax
        mov     [edi+112], eax

           ;add      eax, 0x4000
        add     edi, 4
        loop    @B

        stdcall channel_reset, VIADEV_PLAYBACK
        stdcall codec_check_ready

        mov     eax, pcmout_bdl
        mov     ebx, eax
        invoke  GetPgAddr
        and     ebx, 0xFFF
        add     eax, ebx

        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_TABLE_PTR
        call    [ctrl.ctrl_write32]

        stdcall codec_check_ready

        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFS_PLAYBACK_VOLUME_L
        mov     eax, 7    ;31
        call    [ctrl.ctrl_write8]

        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFS_PLAYBACK_VOLUME_R
        mov     eax, 7    ;31
        call    [ctrl.ctrl_write8]

        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_STOP_IDX
        mov     eax, VIA8233_REG_TYPE_16BIT or VIA8233_REG_TYPE_STEREO or 0xfffff or 0xff000000
        mov     [ctrl.lvi_reg], 16  ;0xF;eax
        call    [ctrl.ctrl_write32]

        stdcall codec_check_ready
        ret
endp


proc channel_reset channel:dword
        mov     esi, dword [channel]
        mov     edx, esi
        add     edx, VIA_REG_OFFSET_CONTROL
        mov     eax, VIA_REG_CTRL_PAUSE or VIA_REG_CTRL_TERMINATE or VIA_REG_CTRL_RESET
        call    [ctrl.ctrl_write8]

        mov     edx, esi
        add     edx, VIA_REG_OFFSET_CONTROL
        call    [ctrl.ctrl_read8]

        mov     eax, 50000       ; wait 50 ms
        call    StallExec
           ; disable interrupts
        mov     edx, esi
        add     edx, VIA_REG_OFFSET_CONTROL
        xor     eax, eax
        call    [ctrl.ctrl_write8]

           ; clear interrupts
        mov     edx, esi
        add     edx, VIA_REG_OFFSET_STATUS
        mov     eax, 0x03
        call    [ctrl.ctrl_write8]

        ;outb(0x00, VIADEV_REG(viadev, OFFSET_TYPE)); /* for via686 */
          ; mov      edx, esi                  ;; for via686
          ; add      edx, VIA_REG_OFFSET_TYPE
          ; mov      eax, 0x03
          ; call     [ctrl.ctrl_write8]

        ;; outl(0, VIADEV_REG(viadev, OFFSET_CURR_PTR));
           ;mov      edx, esi
           ;add      edx, VIA_REG_OFFSET_CURR_PTR
           ;xor      eax, eax
           ;call     [ctrl.ctrl_write8]

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
        mov     esi, [edi+8]
        mov     [ctrl.ctrl_setup], esi

        cmp     edx, VID_VIA
        jne     @F
        mov     [ctrl.vendor_ids], msg_VIA
        ret
@@:

.err:
        xor     eax, eax
        mov     [ctrl.vendor_ids], eax    ;something  wrong ?
        ret
endp

align 4
proc init_controller

        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], dword 4
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
        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x10
        call    dword2str
        invoke  SysMsgBoardStr

        and     eax, 0xFFC0
        mov     [ctrl.ctrl_io_base], eax

.default:
        invoke  PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x3C
        and     eax, 0xFF
@@:
        mov     [ctrl.int_line], eax

           ;stdcall  PciRead8, [ctrl.bus], [ctrl.devfn], dword VIA_FUNC_ENABLE ;0x42
           ;mov      byte [old_legacy], al

           ;stdcall  PciRead8, [ctrl.bus], [ctrl.devfn], dword VIA_PNP_CONTROL ;0x43
           ;mov      byte [old_legacy_cfg], al

           ;mov      al, VIA_FUNC_ENABLE_SB or VIA_FUNC_ENABLE_FM
           ;xor      al, 0xFF
           ;and      al, byte [old_legacy]
           ;and      eax, 0xFF
           ;stdcall  PciWrite8, [ctrl.bus], [ctrl.devfn], dword VIA_FUNC_ENABLE, eax ;0x42
           ;mov      byte [old_legacy], al

        call    [ctrl.ctrl_setup]
        xor     eax, eax
        inc     eax
        ret
endp

align 4
proc set_VIA
        mov     [ctrl.codec_read16], codec_io_r16         ;virtual
        mov     [ctrl.codec_write16], codec_io_w16        ;virtual

        mov     [ctrl.ctrl_read8 ], ctrl_io_r8            ;virtual
        mov     [ctrl.ctrl_read16], ctrl_io_r16           ;virtual
        mov     [ctrl.ctrl_read32], ctrl_io_r32           ;virtual

        mov     [ctrl.ctrl_write8 ], ctrl_io_w8           ;virtual
        mov     [ctrl.ctrl_write16], ctrl_io_w16          ;virtual
        mov     [ctrl.ctrl_write32], ctrl_io_w32          ;virtual
        ret
endp


align 4
proc init_codec
           locals
             counter dd ?
           endl

        mov     esi, msgControl
        invoke  SysMsgBoardStr
        invoke  PciRead8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_CTRL
        and     eax, 0xFF
        call    dword2str
        invoke  SysMsgBoardStr

        mov     esi, msgStatus
        invoke  SysMsgBoardStr
        invoke  PciRead8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_STAT
        and     eax, 0xFF
        push    eax
        call    dword2str
        invoke  SysMsgBoardStr
        pop     eax

        test    eax, VIA_ACLINK_C00_READY
        jnz     .ready

        call    reset_codec
        test    eax, eax
        jz      .err

.ready:
        xor     edx, edx         ; ac_reg_0
        call    [ctrl.codec_write16]
        jmp     .done

.err:
        xor     eax, eax              ; timeout error
        ret

.done:
        call    detect_codec

        xor     eax, eax
        inc     eax
        ret
endp

align 4
proc reset_codec
        invoke  PciWrite8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_CTRL, \
                VIA_ACLINK_CTRL_ENABLE or VIA_ACLINK_CTRL_RESET or VIA_ACLINK_CTRL_SYNC
        mov     eax, 100000        ; wait 100 ms
        call    StallExec
.cold:
        call    cold_reset
        jnc     .ok

     if DEBUG
        mov     esi, msgCFail
        invoke  SysMsgBoardStr
     end if
        xor     eax, eax         ; timeout error
        ret
.ok:
     if DEBUG
        mov     esi, msgResetOk
        invoke  SysMsgBoardStr
     end if
        xor     eax, eax
        inc     eax
        ret
endp


align 4
proc cold_reset
           locals
             counter dd ?
           endl

        invoke  PciWrite8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_CTRL, dword 0

     if DEBUG
        mov     esi, msgCold
        invoke  SysMsgBoardStr
     end if

        mov     eax, 100000         ; wait 100 ms ;400000     ; wait 400 ms
        call    StallExec

           ;; ACLink on, deassert ACLink reset, VSR, SGD data out
           ;; note - FM data out has trouble with non VRA codecs !!
        invoke  PciWrite8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_CTRL, dword VIA_ACLINK_CTRL_INIT

        mov     [counter], 16        ; total 20*100 ms = 2s
.wait:
        invoke  PciRead8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_STAT
        test    eax, VIA_ACLINK_C00_READY
        jnz     .ok

        mov     eax, 100000        ; wait 100 ms
        call    StallExec

        dec     [counter]
        jnz     .wait

     if DEBUG
        mov     esi, msgCRFail
        invoke  SysMsgBoardStr
     end if

.fail:
        stc
        ret
.ok:
        mov     esi, msgControl
        invoke  SysMsgBoardStr
        invoke  PciRead8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_CTRL
        call    dword2str
        invoke  SysMsgBoardStr

        mov     esi, msgStatus
        invoke  SysMsgBoardStr
        invoke  PciRead8, [ctrl.bus], [ctrl.devfn], dword VIA_ACLINK_STAT
        and     eax, 0xFF
        push    eax
        call    dword2str
        invoke  SysMsgBoardStr
        pop     eax

        test    eax, VIA_ACLINK_C00_READY     ;CTRL_ST_CREADY
        jz      .fail
        clc
        ret
endp

align 4
play:
        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_STOP_IDX
        mov     eax, VIA8233_REG_TYPE_16BIT or VIA8233_REG_TYPE_STEREO or 0xfffff or 0xff000000
        mov     [ctrl.lvi_reg], 16
        call    [ctrl.ctrl_write32]

        mov     eax, VIA_REG_CTRL_INT
        or      eax, VIA_REG_CTRL_START
        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_CONTROL
        call    [ctrl.ctrl_write8]

        xor     eax, eax
        ret

align 4
stop:
        mov     eax, VIA_REG_CTRL_INT
        or      eax, VIA_REG_CTRL_TERMINATE
        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_CONTROL
        call    [ctrl.ctrl_write8]

        stdcall channel_reset, VIADEV_PLAYBACK
        xor     eax, eax
        ret

align 4
proc get_dev_info stdcall, p_info:dword
           virtual at esi
             CTRL_INFO CTRL_INFO
           end virtual

        mov     esi, [p_info]
        mov     eax, [ctrl.int_line]
        mov     ecx, [ctrl.ctrl_io_base]
        mov     [CTRL_INFO.irq], eax
        mov     [CTRL_INFO.ctrl_io_base], ecx

        xor     eax, eax
           ;mov      edx, VIADEV_PLAYBACK   +VIA_REG_OFFSET_TABLE_PTR
           ;call     [ctrl.ctrl_read32]
        mov     [CTRL_INFO.codec_io_base], eax
           ;mov      edx, VIADEV_PLAYBACK   +VIA_REG_OFFSET_STOP_IDX
           ;call     [ctrl.ctrl_read32]
        mov     [CTRL_INFO.codec_mem_base], eax
           ;mov      edx, VIADEV_PLAYBACK   +VIA_REG_OFFSET_CURR_COUNT
           ;call     [ctrl.ctrl_read32]
        mov     [CTRL_INFO.ctrl_mem_base], eax

        mov     eax, [codec.chip_id]
        mov     [CTRL_INFO.codec_id], eax

        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_CONTROL
        call    [ctrl.ctrl_read8]
        and     eax, 0xFF
        mov     [CTRL_INFO.glob_cntrl], eax

        mov     edx, VIADEV_PLAYBACK +VIA_REG_OFFSET_STATUS
        call    [ctrl.ctrl_read8]
        and     eax, 0xFF
        mov     [CTRL_INFO.glob_sta], eax

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
proc codec_check_ready stdcall
           locals
             counter dd ?
           endl

        mov     [counter], 1000         ; total 1000*1 ms = 1s
.wait:
        call    [ctrl.codec_read16]
        test    eax, VIA_REG_AC97_BUSY
        jz      .ok

        mov     eax, 1000       ; wait 1 ms
        call    StallExec

        sub     [counter] , 1
        jnz     .wait
.err:
        mov     eax, -1
        ret
.ok:
        and     eax, 0xFFFF
        ret
endp


align 4
proc codec_valid stdcall
        stdcall codec_check_ready
        ret
endp

align 4
proc codec_read stdcall, ac_reg:dword      ; reg = edx, reval = eax
           locals
             counter dd ?
           endl

           ;Use only primary codec.
        mov     eax, [ac_reg]
        and     eax, 0x7F
        shl     eax, VIA_REG_AC97_CMD_SHIFT
        or      eax, VIA_REG_AC97_PRIMARY_VALID or VIA_REG_AC97_READ

        mov     [counter], 3         ; total 3*20 ms = 60ms
.wait:
        push    eax
        call    [ctrl.codec_write16]

        mov     eax, 20000       ; wait 20 ms
        call    StallExec

        stdcall codec_valid
        cmp     eax, 0
        pop     eax
        jge     .ok

        sub     [counter] , 1
        jnz     .wait
        jmp     .err

.ok:
        mov     eax, 25000       ; wait 25 ms
        call    StallExec

        call    [ctrl.codec_read16]      ;change edx !!!
        and     eax, 0xFFFF
        ret
.err:
     if DEBUG
        mov     esi, msgCInvalid
        invoke  SysMsgBoardStr
     end if
        mov     eax, -1            ; invalid codec error
        ret
endp

align 4
proc codec_write stdcall, ac_reg:dword
           ;Use only primary codec.
        mov     esi, [ac_reg]
        mov     edx, esi
        shl     edx, VIA_REG_AC97_CMD_SHIFT

        shl     eax, VIA_REG_AC97_DATA_SHIFT
        or      edx, eax

        mov     eax, VIA_REG_AC97_CODEC_ID_PRIMARY     ;not VIA_REG_AC97_CODEC_ID_PRIMARY
        shl     eax, VIA_REG_AC97_CODEC_ID_SHIFT
        or      edx, eax

        mov     eax, edx
        mov     edx, esi
        call    [ctrl.codec_write16]
        mov     [codec.regs+esi], ax

        stdcall codec_check_ready
        cmp     eax, 0
        jl      .err
.ok:
        ret
.err:
     if DEBUG
        mov     esi, msgCFail
        invoke  SysMsgBoardStr
     end if
           ;mov      eax, -1        ; codec not ready error
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
        mov     ebx, eax           ;low
        mov     ecx, edx           ;high
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
proc codec_io_r16 ;r32
        mov     edx, [ctrl.ctrl_io_base]
        add     edx, VIA_REG_AC97
        in      eax, dx
        ret
endp

align 4
proc codec_io_w16 ;w32
        mov     edx, [ctrl.ctrl_io_base]
        add     edx, VIA_REG_AC97
        out     dx, eax
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
        push    eax ebx ecx
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
        pop     ecx ebx eax
        ret

hexletters   db '0123456789ABCDEF'
hex_buff     db 8 dup(0),13,10,0


include "codec.inc"

align 4
devices dd (CTRL_VT82C686  shl 16)+VID_VIA,msg_VT82C686,set_VIA
        dd (CTRL_VT8233_5  shl 16)+VID_VIA,msg_VT8233,set_VIA
        dd 0    ;terminator


msg_VT82C686 db 'VT82C686', 13,10, 0
msg_VT8233   db 'VT8233',   13,10, 0
msg_VIA      db 'VIA'   ,   13,10, 0

szKernel            db 'KERNEL', 0
sz_sound_srv        db 'SOUND',0

msgInit      db 'detect hardware...',13,10,0
msgFail      db 'device not found',13,10,0
msgAttchIRQ  db 'IRQ line not supported', 13,10, 0
msgInvIRQ    db 'IRQ line not assigned or invalid', 13,10, 0
msgPlay      db 'start play', 13,10,0
msgStop      db 'stop play',  13,10,0
;msgIRQ       db 'AC97 IRQ', 13,10,0
;msgInitCtrl  db 'init controller',13,10,0
;msgInitCodec db 'init codec',13,10,0
msgPrimBuff  db 'create primary buffer ...',0
msgDone      db 'done',13,10,0
msgRemap     db 'Remap IRQ',13,10,0
;msgReg       db 'set service handler',13,10,0
;msgOk        db 'service installed',13,10,0
msgCold      db 'cold reset',13,10,0
;msgWarm      db 'warm reset',13,10,0
;msgWRFail    db 'warm reset failed',13,10,0
msgCRFail    db 'cold reset failed',13,10,0
msgCFail     db 'codec not ready',13,10,0
msgCInvalid  db 'codec is not valid',13,10,0 ;Asper
msgResetOk   db 'reset complete',13,10,0
msgStatus    db 'global status   ',0
msgControl   db 'global control  ',0
msgPciCmd    db 'PCI command     ',0
msgPciStat   db 'PCI status      ',0
msgCtrlIsaIo db 'controller io base   ',0
;msgMixIsaIo  db 'codec io base        ',0
;msgCtrlMMIo  db 'controller mmio base ',0
;msgMixMMIo   db 'codec mmio base      ',0
;msgIrqMap    db 'AC97 irq map as      ',0

align 4
data fixups
end data

align 8
pcmout_bdl       rq 32
buff_list        rd 32

codec CODEC
ctrl AC_CNTRL

chip_type        rb 1
