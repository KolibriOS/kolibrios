;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

DEBUG		equ 1

include 'proc32.inc'
include 'imports.inc'

API_VERSION	equ 0x01000100

USE_COM_IRQ	equ 0	 ;make irq 3 and irq 4 available for PCI devices
IRQ_REMAP	equ 0
IRQ_LINE	equ 0


;irq 0,1,2,8,12,13 недоступны
;                   FEDCBA9876543210
VALID_IRQ	equ 1100111011111000b
ATTCH_IRQ	equ 0000111010100000b

if USE_COM_IRQ
ATTCH_IRQ	equ 0000111010111000b
end if

CPU_FREQ	equ  2600d

BIT0  EQU 0x00000001
BIT1  EQU 0x00000002
BIT5  EQU 0x00000020
BIT10 EQU 0x00000400

VID_Creative	  equ 0x1102

CTRL_CT0200	  equ 0x0006  ; Dell OEM version (EMU10K1X)


CODEC_MASTER_VOL_REG	     equ 0x02
CODEC_AUX_VOL		     equ 0x04 ;
CODEC_PCM_OUT_REG	     equ 0x18 ; PCM output volume
CODEC_EXT_AUDIO_REG	     equ 0x28 ; extended audio
CODEC_EXT_AUDIO_CTRL_REG     equ 0x2a ; extended audio control
CODEC_PCM_FRONT_DACRATE_REG  equ 0x2c ; PCM out sample rate
CODEC_PCM_SURND_DACRATE_REG  equ 0x2e ; surround sound sample rate
CODEC_PCM_LFE_DACRATE_REG    equ 0x30 ; LFE sample rate


;EMU10K1(X) host controller registers set
;; common offsets
;; some definitions were borrowed from emu10k1 driver as they seem to be the same
;;**********************************************************************************************;;
;; PCI function 0 registers, address = <val> + PCIBASE0                                         ;;
;;**********************************************************************************************;;

PTR			equ  0x00	     ;; Indexed register set pointer register        ;;
						;; NOTE: The CHANNELNUM and ADDRESS words can   ;;
						;; be modified independently of each other.     ;;

DATA			equ  0x04	     ;; Indexed register set data register           ;;

IPR			equ  0x08	     ;; Global interrupt pending register            ;;
						;; Clear pending interrupts by writing a 1 to   ;;
						;; the relevant bits and zero to the other bits ;;
IPR_MIDITRANSBUFEMPTY	equ  0x00000001      ;; MIDI UART transmit buffer empty              ;;
IPR_MIDIRECVBUFEMPTY	equ  0x00000002      ;; MIDI UART receive buffer empty               ;;
IPR_CH_0_LOOP		equ  0x00000800      ;; Channel 0 loop                               ;;
IPR_CH_0_HALF_LOOP	equ  0x00000100      ;; Channel 0 half loop                          ;;
IPR_CAP_0_LOOP		equ  0x00080000      ;; Channel capture loop                         ;;
IPR_CAP_0_HALF_LOOP	equ  0x00010000      ;; Channel capture half loop                    ;;

INTE			equ  0x0c	     ;; Interrupt enable register                    ;;
INTE_MIDITXENABLE	equ  0x00000001      ;; Enable MIDI transmit-buffer-empty interrupts ;;
INTE_MIDIRXENABLE	equ  0x00000002      ;; Enable MIDI receive-buffer-empty interrupts  ;;
INTE_CH_0_LOOP		equ  0x00000800      ;; Channel 0 loop                               ;;
INTE_CH_0_HALF_LOOP	equ  0x00000100      ;; Channel 0 half loop                          ;;
INTE_CAP_0_LOOP 	equ  0x00080000      ;; Channel capture loop                         ;;
INTE_CAP_0_HALF_LOOP	equ  0x00010000      ;; Channel capture half loop                    ;;

HCFG			equ  0x14	     ;; Hardware config register                     ;;

HCFG_LOCKSOUNDCACHE	equ  0x00000008      ;; 1 = Cancel bustmaster accesses to soundcache ;;
						;; NOTE: This should generally never be used.   ;;
HCFG_AUDIOENABLE	equ  0x00000001      ;; 0 = CODECs transmit zero-valued samples      ;;
						;; Should be set to 1 when the EMU10K1 is       ;;
						;; completely initialized.                      ;;
GPIO			equ  0x18	     ;; Defaults: 00001080-Analog, 00001000-SPDIF.   ;;


AC97DATA		equ  0x1c	     ;; AC97 register set data register (16 bit)     ;;

AC97ADDRESS		equ  0x1e	     ;; AC97 register set address register (8 bit)   ;;

;;******************************************************************************************************;;
;; Emu10k1x pointer-offset register set, accessed through the PTR and DATA registers                    ;;
;;******************************************************************************************************;;
PLAYBACK_LIST_ADDR	equ  0x00	     ;; Base DMA address of a list of pointers to each period/size ;;
						;; One list entry: 4 bytes for DMA address,
						 ;; 4 bytes for period_size << 16.
						 ;; One list entry is 8 bytes long.
						 ;; One list entry for each period in the buffer.
						 ;;
PLAYBACK_LIST_SIZE	equ  0x01	     ;; Size of list in bytes << 19. E.g. 8 periods -> 0x00380000  ;;
PLAYBACK_LIST_PTR	equ  0x02	     ;; Pointer to the current period being played ;;
PLAYBACK_DMA_ADDR	equ  0x04	     ;; Playback DMA addresss ;;
PLAYBACK_PERIOD_SIZE	equ  0x05	     ;; Playback period size ;;
PLAYBACK_POINTER	equ  0x06	     ;; Playback period pointer. Sample currently in DAC ;;
PLAYBACK_UNKNOWN1	equ  0x07
PLAYBACK_UNKNOWN2	equ  0x08

;; Only one capture channel supported ;;
CAPTURE_DMA_ADDR	equ  0x10	     ;; Capture DMA address ;;
CAPTURE_BUFFER_SIZE	equ  0x11	     ;; Capture buffer size ;;
CAPTURE_POINTER 	equ  0x12	     ;; Capture buffer pointer. Sample currently in ADC ;;
CAPTURE_UNKNOWN 	equ  0x13

;; From 0x20 - 0x3f, last samples played on each channel ;;

TRIGGER_CHANNEL 	equ  0x40	     ;; Trigger channel playback                     ;;
TRIGGER_CHANNEL_0	equ  0x00000001      ;; Trigger channel 0                            ;;
TRIGGER_CHANNEL_1	equ  0x00000002      ;; Trigger channel 1                            ;;
TRIGGER_CHANNEL_2	equ  0x00000004      ;; Trigger channel 2                            ;;
TRIGGER_CAPTURE 	equ  0x00000100      ;; Trigger capture channel                      ;;

ROUTING 		equ  0x41	     ;; Setup sound routing ?                        ;;
ROUTING_FRONT_LEFT	equ  0x00000001
ROUTING_FRONT_RIGHT	equ  0x00000002
ROUTING_REAR_LEFT	equ  0x00000004
ROUTING_REAR_RIGHT	equ  0x00000008
ROUTING_CENTER_LFE	equ  0x00010000

SPCS0			equ  0x42	     ;; SPDIF output Channel Status 0 register       ;;
SPCS1			equ  0x43	     ;; SPDIF output Channel Status 1 register       ;;
SPCS2			equ  0x44	     ;; SPDIF output Channel Status 2 register       ;;

SPCS_CLKACCYMASK	equ  0x30000000      ;; Clock accuracy                               ;;
SPCS_CLKACCY_1000PPM	equ  0x00000000      ;; 1000 parts per million                       ;;
SPCS_CLKACCY_50PPM	equ  0x10000000      ;; 50 parts per million                         ;;
SPCS_CLKACCY_VARIABLE	equ  0x20000000      ;; Variable accuracy                            ;;
SPCS_SAMPLERATEMASK	equ  0x0f000000      ;; Sample rate                                  ;;
SPCS_SAMPLERATE_44	equ  0x00000000      ;; 44.1kHz sample rate                          ;;
SPCS_SAMPLERATE_48	equ  0x02000000      ;; 48kHz sample rate                            ;;
SPCS_SAMPLERATE_32	equ  0x03000000      ;; 32kHz sample rate                            ;;
SPCS_CHANNELNUMMASK	equ  0x00f00000      ;; Channel number                               ;;
SPCS_CHANNELNUM_UNSPEC	equ  0x00000000      ;; Unspecified channel number                   ;;
SPCS_CHANNELNUM_LEFT	equ  0x00100000      ;; Left channel                                 ;;
SPCS_CHANNELNUM_RIGHT	equ  0x00200000      ;; Right channel                                ;;
SPCS_SOURCENUMMASK	equ  0x000f0000      ;; Source number                                ;;
SPCS_SOURCENUM_UNSPEC	equ  0x00000000      ;; Unspecified source number                    ;;
SPCS_GENERATIONSTATUS	equ  0x00008000      ;; Originality flag (see IEC-958 spec)          ;;
SPCS_CATEGORYCODEMASK	equ  0x00007f00      ;; Category code (see IEC-958 spec)             ;;
SPCS_MODEMASK		equ  0x000000c0      ;; Mode (see IEC-958 spec)                      ;;
SPCS_EMPHASISMASK	equ  0x00000038      ;; Emphasis                                     ;;
SPCS_EMPHASIS_NONE	equ  0x00000000      ;; No emphasis                                  ;;
SPCS_EMPHASIS_50_15	equ  0x00000008      ;; 50/15 usec 2 channel                         ;;
SPCS_COPYRIGHT		equ  0x00000004      ;; Copyright asserted flag -- do not modify     ;;
SPCS_NOTAUDIODATA	equ  0x00000002      ;; 0 = Digital audio, 1 = not audio             ;;
SPCS_PROFESSIONAL	equ  0x00000001      ;; 0 = Consumer (IEC-958), 1 = pro (AES3-1992)  ;;

SPDIF_SELECT		equ  0x45	     ;; Enables SPDIF or Analogue outputs 0-Analogue, 0x700-SPDIF ;;

;; This is the MPU port on the card                                                             ;;
MUDATA		equ  0x47
MUCMD		equ  0x48
MUSTAT		equ  MUCMD

;; From 0x50 - 0x5f, last samples captured ;;


SRV_GETVERSION	      equ  0
DEV_PLAY	      equ  1
DEV_STOP	      equ  2
DEV_CALLBACK	      equ  3
DEV_SET_BUFF	      equ  4
DEV_NOTIFY	      equ  5
DEV_SET_MASTERVOL     equ  6
DEV_GET_MASTERVOL     equ  7
DEV_GET_INFO	      equ  8

struc AC_CNTRL		    ;AC controller base class
{ .bus		      dd ?
  .devfn	      dd ?

  .vendor	      dd ?
  .dev_id	      dd ?
  .pci_cmd	      dd ?
  .pci_stat	      dd ?

  .codec_io_base      dd ?
  .codec_mem_base     dd ?

  .ctrl_io_base       dd ?
  .ctrl_mem_base      dd ?
  .cfg_reg	      dd ?
  .int_line	      dd ?

  .vendor_ids	      dd ?    ;vendor id string
  .ctrl_ids	      dd ?    ;hub id string

  .buffer	      dd ?

  .notify_pos	      dd ?
  .notify_task	      dd ?

  .lvi_reg	      dd ?
  .ctrl_setup	      dd ?
  .user_callback      dd ?
  .codec_read16       dd ?
  .codec_write16      dd ?

  .ctrl_read8	      dd ?
  .ctrl_read16	      dd ?
  .ctrl_read32	      dd ?

  .ctrl_write8	      dd ?
  .ctrl_write16       dd ?
  .ctrl_write32       dd ?
}

struc CODEC		   ;Audio Chip base class
{
  .chip_id	      dd ?
  .flags	      dd ?
  .status	      dd ?

  .ac_vendor_ids      dd ?    ;ac vendor id string
  .chip_ids	      dd ?    ;chip model string

  .shadow_flag	      dd ?
		      dd ?

  .regs 	      dw ?     ; codec registers
  .reg_master_vol     dw ?     ;0x02
  .reg_aux_out_vol    dw ?     ;0x04
  .reg_mone_vol       dw ?     ;0x06
  .reg_master_tone    dw ?     ;0x08
  .reg_beep_vol       dw ?     ;0x0A
  .reg_phone_vol      dw ?     ;0x0C
  .reg_mic_vol	      dw ?     ;0x0E
  .reg_line_in_vol    dw ?     ;0x10
  .reg_cd_vol	      dw ?     ;0x12
  .reg_video_vol      dw ?     ;0x14
  .reg_aux_in_vol     dw ?     ;0x16
  .reg_pcm_out_vol    dw ?     ;0x18
  .reg_rec_select     dw ?     ;0x1A
  .reg_rec_gain       dw ?     ;0x1C
  .reg_rec_gain_mic   dw ?     ;0x1E
  .reg_gen	      dw ?     ;0x20
  .reg_3d_ctrl	      dw ?     ;0X22
  .reg_page	      dw ?     ;0X24
  .reg_powerdown      dw ?     ;0x26
  .reg_ext_audio      dw ?     ;0x28
  .reg_ext_st	      dw ?     ;0x2a
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
  .reg_page_0	      dw ?     ;0x60
  .reg_page_1	      dw ?     ;0x62
  .reg_page_2	      dw ?     ;0x64
  .reg_page_3	      dw ?     ;0x66
  .reg_page_4	      dw ?     ;0x68
  .reg_page_5	      dw ?     ;0x6A
  .reg_page_6	      dw ?     ;0x6C
  .reg_page_7	      dw ?     ;0x6E
		      dw ?     ;0x70
		      dw ?     ;0x72
		      dw ?     ;0x74
		      dw ?     ;0x76
		      dw ?     ;0x78
		      dw ?     ;0x7A
  .reg_vendor_id_1    dw ?     ;0x7C
  .reg_vendor_id_2    dw ?     ;0x7E


  .reset	      dd ?    ;virual
  .set_master_vol     dd ?
}

struc CTRL_INFO
{   .pci_cmd	      dd ?
    .irq	      dd ?
    .glob_cntrl       dd ?
    .glob_sta	      dd ?
    .codec_io_base    dd ?
    .ctrl_io_base     dd ?
    .codec_mem_base   dd ?
    .ctrl_mem_base    dd ?
    .codec_id	      dd ?
}

struc IOCTL
{  .handle	      dd ?
   .io_code	      dd ?
   .input	      dd ?
   .inp_size	      dd ?
   .output	      dd ?
   .out_size	      dd ?
}

virtual at 0
  IOCTL IOCTL
end virtual

EVENT_NOTIFY	equ 0x00000200

public START
public service_proc
public version

section '.flat' code readable align 16

proc START stdcall, state:dword

	   cmp	 [state], 1
	   jne	 .stop

     if DEBUG
	   mov	 esi, msgInit
	   call  SysMsgBoardStr
     end if

	   call     detect_controller
	   test     eax, eax
	   jz	    .fail

     if DEBUG
	   mov	 esi,[ctrl.vendor_ids]
	   call  SysMsgBoardStr
	   mov	 esi, [ctrl.ctrl_ids]
	   call  SysMsgBoardStr
     end if

	   call     init_controller
	   test     eax, eax
	   jz	    .fail

	   call     init_codec
	   test     eax, eax
	   jz	    .fail

	   call     setup_codec

	   mov	    esi, msgPrimBuff
	   call     SysMsgBoardStr
	   call     create_primary_buff
	   mov	    esi, msgDone
	   call     SysMsgBoardStr

  if IRQ_REMAP
	   pushf
	   cli

	   mov	    ebx, [ctrl.int_line]
	   in	    al, 0xA1
	   mov	    ah, al
	   in	    al, 0x21
	   test     ebx, ebx
	   jz	    .skip
	   bts	    ax, bx			;mask old line
.skip:
	   bts	    ax, IRQ_LINE		;mask new ine
	   out	    0x21, al
	   mov	    al, ah
	   out	    0xA1, al

	   stdcall  PciWrite8, 0, 0xF8, 0x61, IRQ_LINE	;remap IRQ

	   mov	    dx, 0x4d0			;8259 ELCR1
	   in	    al, dx
	   bts	    ax, IRQ_LINE
	   out	    dx, al			;set level-triggered mode
	   mov	    [ctrl.int_line], IRQ_LINE
	   popf
	   mov	    esi, msgRemap
	   call     SysMsgBoardStr
  end if

	   mov	    eax, VALID_IRQ
	   mov	    ebx, [ctrl.int_line]
	   mov	    esi, msgInvIRQ
	   bt	    eax, ebx
	   jnc	    .fail_msg
	   mov	    eax, ATTCH_IRQ
	   mov	    esi, msgAttchIRQ
	   bt	    eax, ebx
	   jnc	    .fail_msg

	   stdcall  AttachIntHandler, ebx, ac97_irq, dword 0
	   stdcall  create

.reg:
	   stdcall  RegService, sz_sound_srv, service_proc
	   ret
.fail:
     if DEBUG
	   mov	 esi, msgFail
	   call  SysMsgBoardStr
     end if
	   xor	    eax, eax
	   ret
.fail_msg:
	   call     SysMsgBoardStr
	   xor	    eax, eax
	   ret
.stop:
	   call     stop
	   xor	    eax, eax
	   ret
endp

handle	   equ	IOCTL.handle
io_code    equ	IOCTL.io_code
input	   equ	IOCTL.input
inp_size   equ	IOCTL.inp_size
output	   equ	IOCTL.output
out_size   equ	IOCTL.out_size

align 4
proc service_proc stdcall, ioctl:dword

	   mov	    edi, [ioctl]
	   mov	    eax, [edi+io_code]

	   cmp	    eax, SRV_GETVERSION
	   jne	    @F
	   mov	    eax, [edi+output]
	   cmp	    [edi+out_size], 4
	   jne	    .fail

	   mov	    [eax], dword API_VERSION
	   xor	    eax, eax
	   ret
@@:
	   cmp	    eax, DEV_PLAY
	   jne	    @F
     if DEBUG
	   mov	esi, msgPlay
	   call SysMsgBoardStr
     end if
	   call     play
	   ret
@@:
	   cmp	    eax, DEV_STOP
	   jne	    @F
     if DEBUG
	   mov	esi, msgStop
	   call SysMsgBoardStr
     end if
	   call     stop
	   ret
@@:
	   cmp	    eax, DEV_CALLBACK
	   jne	    @F
	   mov	    ebx, [edi+input]
	   stdcall  set_callback, [ebx]
	   ret
@@:
	   cmp	    eax, DEV_SET_MASTERVOL
	   jne	    @F
	   mov	    eax, [edi+input]
	   mov	    eax, [eax]
	   call     set_master_vol	;eax= vol
	   ret
@@:
	   cmp	    eax, DEV_GET_MASTERVOL
	   jne	    @F
	   mov	    ebx, [edi+output]
	   stdcall  get_master_vol, ebx
	   ret
@@:
	   cmp	    eax, DEV_GET_INFO
	   jne	    @F
	   mov	    ebx, [edi+output]
	   stdcall  get_dev_info, ebx
	   ret
@@:
.fail:
	   or	    eax, -1
	   ret
endp

restore   handle
restore   io_code
restore   input
restore   inp_size
restore   output
restore   out_size


align 4
proc ac97_irq
	   locals
	     status dd 0
	   endl

;        status = inl(chip->port + IPR);
	   mov	    edx, IPR
	   call     [ctrl.ctrl_read32]
	   test     eax, eax
	   jz	    @f

	   mov	    dword [status], eax

	   mov	    ebx, dword [buff_list]
	   cmp	    [ctrl.user_callback], 0
	   je	    @f
	   stdcall  [ctrl.user_callback], ebx
       @@:
	   mov	    eax, dword [status] 	  ;; ack ;;
	   mov	    edx, IPR
	   call     [ctrl.ctrl_write32]
	   ret
endp


align 4
proc create_primary_buff

	   stdcall  KernelAlloc, 0x10000
	   mov	    [ctrl.buffer], eax

	   mov	    edi, eax
	   mov	    ecx, 0x10000/4
	   xor	    eax, eax
	   cld
	   rep	    stosd

	   mov	    eax, [ctrl.buffer]
	   call     GetPgAddr

	   mov edi, pcmout_bdl
	   stosd
	   mov	    eax, 0x4000000
	   stosd

	   mov	    edi, buff_list
	   mov	    eax, [ctrl.buffer]
	   stosd		     ;1.]

	   mov	    eax, [ctrl.buffer]
	   call     GetPgAddr

	   stdcall  ptr_write, PLAYBACK_POINTER,   0, 0
	   stdcall  ptr_write, PLAYBACK_UNKNOWN1,  0, 0
	   stdcall  ptr_write, PLAYBACK_UNKNOWN2,  0, 0
	   stdcall  ptr_write, PLAYBACK_DMA_ADDR,  0, eax

	   mov	    eax, pcmout_bdl
	   mov	    ebx, eax
	   call     GetPgAddr
	   and	    ebx, 0xFFF
	   add	    eax, ebx

	   stdcall  ptr_write, PLAYBACK_LIST_ADDR, 0, eax
	   stdcall  ptr_write, PLAYBACK_LIST_SIZE, 0, 0
	   stdcall  ptr_write, PLAYBACK_LIST_PTR,  0, 0

	   ;mov     eax, 0x00004000
	   ;shl     eax, 16
	   stdcall  ptr_write, PLAYBACK_PERIOD_SIZE, 0, 0x40000000;eax

	   ret
endp


align 4
proc detect_controller
	   locals
	     last_bus dd ?
	     bus      dd ?
	     devfn    dd ?
	   endl

	   xor eax, eax
	   mov [bus], eax
	   inc eax
	   call PciApi
	   cmp eax, -1
	   je .err

	   mov [last_bus], eax

.next_bus:
	   and [devfn], 0
.next_dev:
	   stdcall PciRead32, [bus], [devfn], dword 0
	   test eax, eax
	   jz .next
	   cmp eax, -1
	   je .next

	   mov edi, devices
@@:
	   mov ebx, [edi]
	   test ebx, ebx
	   jz .next

	   cmp eax, ebx
	   je .found
	   add edi, 12
	   jmp @B
.next:
	   inc [devfn]
	   cmp [devfn], 256
	   jb .next_dev
	   mov eax, [bus]
	   inc eax
	   mov [bus], eax
	   cmp eax, [last_bus]
	   jna .next_bus
	   xor eax, eax
	   ret
.found:
	   mov ebx, [bus]
	   mov [ctrl.bus], ebx

	   mov ecx, [devfn]
	   mov [ctrl.devfn], ecx

	   mov edx, eax
	   and edx, 0xFFFF
	   mov [ctrl.vendor], edx
	   shr eax, 16
	   mov [ctrl.dev_id], eax

	   mov ebx, [edi+4]
	   mov [ctrl.ctrl_ids], ebx
	   mov esi, [edi+8]
	   mov [ctrl.ctrl_setup], esi

	   cmp edx, VID_Creative
	   jne @F
	   mov [ctrl.vendor_ids], msg_Creative
	   ret
@@:

.err:
	   xor eax, eax
	   mov [ctrl.vendor_ids], eax	  ;something  wrong ?
	   ret
endp

align 4
proc init_controller

	   stdcall  PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x2C
	   mov	    esi, msgPciSubsys
	   call     SysMsgBoardStr
	   call     dword2str
	   call     SysMsgBoardStr

	   stdcall  PciRead32, [ctrl.bus], [ctrl.devfn], dword 4
	   mov	    ebx, eax
	   and	    eax, 0xFFFF
	   mov	    [ctrl.pci_cmd], eax
	   shr	    ebx, 16
	   mov	    [ctrl.pci_stat], ebx

	   mov	    esi, msgPciCmd
	   call     SysMsgBoardStr
	   call     dword2str
	   call     SysMsgBoardStr

	   mov	    esi, msgPciStat
	   call     SysMsgBoardStr
	   mov	    eax, [ctrl.pci_stat]
	   call     dword2str
	   call     SysMsgBoardStr

	   mov	    esi, msgCtrlIsaIo
	   call     SysMsgBoardStr
	   stdcall  PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x10
	   call     dword2str
	   call     SysMsgBoardStr

	   and	    eax, 0xFFC0
	   mov	    [ctrl.ctrl_io_base], eax

.default:
	   stdcall  PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x3C
	   and	    eax, 0xFF
@@:
	   mov	    [ctrl.int_line], eax

	   call     [ctrl.ctrl_setup]
	   xor	    eax, eax
	   inc	    eax
	   ret
endp

align 4
proc set_Creative
	   mov	    [ctrl.codec_read16],  codec_io_r16	  ;virtual
	   mov	    [ctrl.codec_write16], codec_io_w16	  ;virtual

	   mov	    [ctrl.ctrl_read8 ],  ctrl_io_r8	  ;virtual
	   mov	    [ctrl.ctrl_read16],  ctrl_io_r16	  ;virtual
	   mov	    [ctrl.ctrl_read32],  ctrl_io_r32	  ;virtual

	   mov	    [ctrl.ctrl_write8 ], ctrl_io_w8	  ;virtual
	   mov	    [ctrl.ctrl_write16], ctrl_io_w16	  ;virtual
	   mov	    [ctrl.ctrl_write32], ctrl_io_w32	  ;virtual
	   ret
endp


align 4
proc init_codec
	   call     reset_codec
	   test     eax, eax
	   jz	    .err
	   call     detect_codec
	   xor	    eax, eax
	   inc	    eax
	   ret
     .err:
	   xor	    eax, eax
	   ret
endp

align 4
proc reset_codec
	   locals
	     counter dd ?
	   endl

     if DEBUG
	   mov	 esi, msgCold
	   call  SysMsgBoardStr
     end if

	   mov	    eax, 100000     ; wait 100 ms ;400000     ; wait 400 ms
	   call     StallExec

	   stdcall  ptr_read, TRIGGER_CHANNEL, 0

	   mov	    [counter], 16    ; total 20*100 ms = 2s
.wait:
	   stdcall  codec_read, dword 0x26
	   test     eax, 1
	   jnz	    .ok

	   mov	    eax, 100000    ; wait 100 ms
	   call     StallExec

	   dec	    [counter]
	   jnz	    .wait

     if DEBUG
	   mov	 esi, msgCRFail
	   call  SysMsgBoardStr
     end if

.fail:
	   stc
	   ret
.ok:


	   xor	    eax, eax
	   inc	    eax
	   ret
endp


align 4
play:
	   mov	    eax, INTE_CH_0_LOOP
	   stdcall  intr_enable, eax

	   stdcall  ptr_read, TRIGGER_CHANNEL, 0
	   mov	    ebx, TRIGGER_CHANNEL_0
	   or	    eax, ebx
	   stdcall  ptr_write, TRIGGER_CHANNEL, 0, eax

	   xor	    eax, eax
	   ret

align 4
stop:
	   mov	    eax, INTE_CH_0_LOOP or INTE_CH_0_HALF_LOOP
	   stdcall  intr_disable, eax

	   stdcall  ptr_read, TRIGGER_CHANNEL, 0
	   mov	    ebx, TRIGGER_CHANNEL_0
	   xor	    ebx, -1
	   or	    eax, ebx
	   stdcall  ptr_write, TRIGGER_CHANNEL, 0, eax
	   xor	    eax, eax
	   ret

align 4
proc get_dev_info stdcall, p_info:dword
	   virtual at esi
	     CTRL_INFO CTRL_INFO
	   end virtual

	   mov	    esi, [p_info]
	   mov	    eax, [ctrl.int_line]
	   mov	    ecx, [ctrl.ctrl_io_base]
	   mov	    [CTRL_INFO.irq], eax
	   mov	    [CTRL_INFO.ctrl_io_base], ecx
	   mov	    eax, [codec.chip_id]
	   mov	    [CTRL_INFO.codec_id], eax
	   mov	    ebx, [ctrl.pci_cmd]
	   mov	    [CTRL_INFO.pci_cmd], ebx

	   xor	    eax, eax
	   mov	    [CTRL_INFO.codec_io_base], eax
	   mov	    [CTRL_INFO.codec_mem_base], eax
	   mov	    [CTRL_INFO.ctrl_mem_base], eax
	   mov	    [CTRL_INFO.glob_cntrl], eax
	   mov	    [CTRL_INFO.glob_sta], eax
	   ret
endp

align 4
proc set_callback stdcall, handler:dword
	   mov	    eax, [handler]
	   mov	    [ctrl.user_callback], eax
	   ret
endp


align 4
proc create stdcall
	   stdcall  PciRead16, [ctrl.bus], [ctrl.devfn], dword 4
	   test     eax, 4 ; test master bit
	   jnz	    @f
	   or	    eax, 4
	   stdcall  PciWrite16, [ctrl.bus], [ctrl.devfn], dword 4, eax ; set master bit
	 @@:

	   xor	    eax, eax
	   mov	    edx, INTE
	   call     [ctrl.ctrl_write32]

	   stdcall  ptr_write, SPCS0, 0, \
			       SPCS_CLKACCY_1000PPM or SPCS_SAMPLERATE_48 or \
			       SPCS_CHANNELNUM_LEFT or SPCS_SOURCENUM_UNSPEC or \
			       SPCS_GENERATIONSTATUS or 0x00001200 or \
			       0x00000000 or SPCS_EMPHASIS_NONE or SPCS_COPYRIGHT
	   stdcall  ptr_write, SPCS1, 0, \
			       SPCS_CLKACCY_1000PPM or SPCS_SAMPLERATE_48 or \
			       SPCS_CHANNELNUM_LEFT or SPCS_SOURCENUM_UNSPEC or \
			       SPCS_GENERATIONSTATUS or 0x00001200 or \
			       0x00000000 or SPCS_EMPHASIS_NONE or SPCS_COPYRIGHT
	   stdcall  ptr_write, SPCS2, 0, \
			       SPCS_CLKACCY_1000PPM or SPCS_SAMPLERATE_48 or \
			       SPCS_CHANNELNUM_LEFT or SPCS_SOURCENUM_UNSPEC or \
			       SPCS_GENERATIONSTATUS or 0x00001200 or \
			       0x00000000 or SPCS_EMPHASIS_NONE or SPCS_COPYRIGHT

	   stdcall  ptr_write, SPDIF_SELECT, 0, 0x700  ; disable SPDIF
	   stdcall  ptr_write, ROUTING, 0, 0x1003F     ; routing
	   stdcall  gpio_write, 0x1080		       ; analog mode

	   mov	    eax, dword HCFG_LOCKSOUNDCACHE or HCFG_AUDIOENABLE
	   mov	    edx, HCFG
	   call     [ctrl.ctrl_write32]
	   ret
endp

align 4
proc codec_read stdcall, reg:dword
	   stdcall  ac97_read, dword [reg]
	   ret
endp


align 4
proc codec_write stdcall, reg:dword
	   stdcall  ac97_write, dword [reg], eax
	   ret
endp


align 4
proc ac97_read stdcall, reg:dword
	   push     edx
	   mov	    eax, dword [reg]
	   mov	    edx, AC97ADDRESS
	   call     [ctrl.ctrl_write8]

	   mov	    edx, AC97DATA
	   call     [ctrl.ctrl_read16]
	   and	    eax, 0xFFFF
	   pop	    edx
	   ret
endp

align 4
proc ac97_write stdcall, reg:dword, val:dword
	   push     eax edx
	   mov	    eax, dword [reg]
	   mov	    edx, AC97ADDRESS
	   call     [ctrl.ctrl_write8]

	   mov	    eax, dword [val]
	   mov	    edx, AC97DATA
	   call     [ctrl.ctrl_write16]
	   pop	    edx eax
	   ret
endp

align 4
proc ptr_read stdcall, reg:dword, chn:dword
	   push     edx
	   mov	    eax, dword [reg]
	   shl	    eax, 16
	   or	    eax, dword [chn]

	   mov	    edx, PTR
	   call     [ctrl.ctrl_write32]

	   mov	    edx, DATA
	   call     [ctrl.ctrl_read32]
	   pop	    edx
	ret
endp

align 4
proc ptr_write stdcall, reg:dword, chn:dword, data:dword
	   push     eax edx
	   mov	    eax, dword [reg]
	   shl	    eax, 16
	   or	    eax, dword [chn]

	   mov	    edx, PTR
	   call     [ctrl.ctrl_write32]

	   mov	    eax, dword [data]
	   mov	    edx, DATA
	   call     [ctrl.ctrl_write32]
	   pop	    edx eax
	ret
endp

align 4
proc intr_enable stdcall, intrenb:dword
	   push     edx
	   mov	    edx, INTE
	   call     [ctrl.ctrl_read32]

	   or	    eax, dword [intrenb]
	   mov	    edx, INTE
	   call     [ctrl.ctrl_write32]
	   pop	    edx
	   ret
endp

align 4
proc intr_disable stdcall, intrenb:dword
	   push     eax ebx edx
	   mov	    edx, INTE
	   call     [ctrl.ctrl_read32]

	   mov	    ebx, dword [intrenb]
	   xor	    ebx, -1
	   and	    eax, ebx
	   mov	    edx, INTE
	   call     [ctrl.ctrl_write32]
	   pop	    edx ebx eax
	   ret
endp

align 4
proc gpio_write stdcall, value:dword
	   push     eax edx
	   mov	    eax, dword [value]
	   mov	    edx, GPIO
	   call     [ctrl.ctrl_write32]
	   pop	    edx eax
	   ret
endp


align 4
proc StallExec
	   push     ecx
	   push     edx
	   push     ebx
	   push     eax

	   mov	    ecx, CPU_FREQ
	   mul	    ecx
	   mov	    ebx, eax	   ;low
	   mov	    ecx, edx	   ;high
	   rdtsc
	   add	    ebx, eax
	   adc	    ecx, edx
@@:
	   rdtsc
	   sub	    eax, ebx
	   sbb	    edx, ecx
	   js	    @B

	   pop	    eax
	   pop	    ebx
	   pop	    edx
	   pop	    ecx
	   ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;          CONTROLLER IO functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc codec_io_r16 ;Not used.
	   ;mov      edx, [ctrl.ctrl_io_base]
	   ;in       eax, dx
	   ret
endp

align 4
proc codec_io_w16 ;Not used.
	   ;mov      edx, [ctrl.ctrl_io_base]
	   ;out      dx,  eax
	   ret
endp

align 4
proc ctrl_io_r8
	   add	    edx, [ctrl.ctrl_io_base]
	   in	    al, dx
	   ret
endp

align 4
proc ctrl_io_r16
	   add	    edx, [ctrl.ctrl_io_base]
	   in	    ax, dx
	   ret
endp

align 4
proc ctrl_io_r32
	   add	    edx, [ctrl.ctrl_io_base]
	   in	    eax, dx
	   ret
endp

align 4
proc ctrl_io_w8
	   add	    edx, [ctrl.ctrl_io_base]
	   out	    dx, al
	   ret
endp

align 4
proc ctrl_io_w16
	   add	    edx, [ctrl.ctrl_io_base]
	   out	    dx, ax
	   ret
endp

align 4
proc ctrl_io_w32
	   add	    edx, [ctrl.ctrl_io_base]
	   out	    dx, eax
	   ret
endp


align 4
dword2str:
	   push     eax ebx ecx
	   mov	    esi, hex_buff
	   mov	    ecx, -8
   @@:
	   rol	    eax, 4
	   mov	    ebx, eax
	   and	    ebx, 0x0F
	   mov	    bl, [ebx+hexletters]
	   mov	    [8+esi+ecx], bl
	   inc	    ecx
	   jnz	    @B
	   pop	    ecx ebx eax
	   ret

hexletters   db '0123456789ABCDEF'
hex_buff     db 8 dup(0),13,10,0


include "codec.inc"

align 4
devices dd (CTRL_CT0200 shl 16)+VID_Creative,msg_CT_EMU10K1X,set_Creative
	dd 0	;terminator


version      dd (5 shl 16) or (API_VERSION and 0xFFFF)

msg_CT_EMU10K1X  db 'SB Live! Dell OEM', 13,10, 0
msg_Creative	 db 'Creative ', 0

szKernel	    db 'KERNEL', 0
sz_sound_srv	    db 'SOUND',0

msgInit      db 'detect hardware...',13,10,0
msgFail      db 'device not found',13,10,0
msgAttchIRQ  db 'IRQ line not supported', 13,10, 0
msgInvIRQ    db 'IRQ line not assigned or invalid', 13,10, 0
msgPlay      db 'start play', 13,10,0
msgStop      db 'stop play',  13,10,0
msgIRQ	     db 'AC97 IRQ', 13,10,0
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
;msgCFail     db 'codec not ready',13,10,0
;msgCInvalid  db 'codec is not valid',13,10,0 ;Asper
;msgResetOk   db 'reset complete',13,10,0
;msgStatus    db 'global status   ',0
;msgControl   db 'global control  ',0
msgPciCmd    db 'PCI command     ',0
msgPciStat   db 'PCI status      ',0
msgPciSubsys db 'PCI subsystem   ',0
msgCtrlIsaIo db 'controller io base   ',0
;msgMixIsaIo  db 'codec io base        ',0
;msgCtrlMMIo  db 'controller mmio base ',0
;msgMixMMIo   db 'codec mmio base      ',0
;msgIrqMap    db 'AC97 irq map as      ',0

section '.data' data readable writable align 16

pcmout_bdl	 rq 32
buff_list	 rd 32

codec CODEC
ctrl AC_CNTRL
