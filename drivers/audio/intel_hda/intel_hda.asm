;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2012. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format MS COFF

DEBUG		equ 1
FDEBUG		equ 0
DEBUG_IRQ	equ 0

USE_SINGLE_MODE   equ  0   ; 1 = Single mode; 0 = Normal mode.

TEST_VERSION_NUMBER  equ '018d'

;Asper+ [
SDO_TAG  equ 1	      ;Asper: Output stream tag id (any number except 0)
SDO_IDX  equ 4	      ;Asper: Output stream index
;According to "Intel® I/O Controller Hub 6 (ICH6) High Definition Audio / AC ’97 Programmer’s Reference Manual (PRM) May 2005 Document"
;and "Intel® I/O Controller Hub 6 (ICH6) Family Datasheet" SDO0=4,
;but according to "High Definition Audio Specification Revision 1.0a June 17, 2010" SDO0 depends on the number of SDIs.

SDO_INT  equ 1 shl SDO_IDX  ;Asper: Output stream interrupt (must be power of 2)
SDO_OFS  equ 0x80+(SDO_IDX*0x20) ;Asper: Output stream offset
;Asper+ ]

include 'PROC32.INC'
include 'IMPORTS.INC'
include 'CODEC_H.INC'


CURRENT_API	equ   0x0100	  ;1.00
COMPATIBLE_API	equ   0x0101	  ;1.01
API_VERSION	equ   (COMPATIBLE_API shl 16) or CURRENT_API

IRQ_REMAP	equ 0
IRQ_LINE	equ 0

CPU_FREQ	equ  2600d

; Vendors
VID_INTEL	  equ 0x8086
VID_NVIDIA	  equ 0x10DE
VID_ATI 	  equ 0x1002
VID_AMD 	  equ 0x1022
VID_VIA 	  equ 0x1106
VID_SIS 	  equ 0x1039
VID_ULI 	  equ 0x10B9
VID_CREATIVE	  equ 0x1102
VID_TERA	  equ 0x6549
VID_RDC 	  equ 0x17F3
VID_VMWARE	  equ 0x15AD

; Devices
; Intel
CTRL_INTEL_SCH2 	 equ  0x080a
CTRL_INTEL_HPT		 equ  0x0c0c
CTRL_INTEL_CPT		 equ  0x1c20
CTRL_INTEL_PGB		 equ  0x1d20
CTRL_INTEL_PPT1 	 equ  0x1e20
CTRL_INTEL_82801F	 equ  0x2668
CTRL_INTEL_63XXESB	 equ  0x269a
CTRL_INTEL_82801G	 equ  0x27d8
CTRL_INTEL_82801H	 equ  0x284b
CTRL_INTEL_82801_UNK1	 equ  0x2911
CTRL_INTEL_82801I	 equ  0x293e
CTRL_INTEL_82801_UNK2	 equ  0x293f
CTRL_INTEL_82801JI	 equ  0x3a3e
CTRL_INTEL_82801JD	 equ  0x3a6e
CTRL_INTEL_PCH		 equ  0x3b56
CTRL_INTEL_PCH2 	 equ  0x3b57
CTRL_INTEL_SCH		 equ  0x811b
CTRL_INTEL_LPT		 equ  0x8c20
; Nvidia
CTRL_NVIDIA_MCP51	 equ  0x026c
CTRL_NVIDIA_MCP55	 equ  0x0371
CTRL_NVIDIA_MCP61_1	 equ  0x03e4
CTRL_NVIDIA_MCP61_2	 equ  0x03f0
CTRL_NVIDIA_MCP65_1	 equ  0x044a
CTRL_NVIDIA_MCP65_2	 equ  0x044b
CTRL_NVIDIA_MCP67_1	 equ  0x055c
CTRL_NVIDIA_MCP67_2	 equ  0x055d
CTRL_NVIDIA_MCP78_1	 equ  0x0774
CTRL_NVIDIA_MCP78_2	 equ  0x0775
CTRL_NVIDIA_MCP78_3	 equ  0x0776
CTRL_NVIDIA_MCP78_4	 equ  0x0777
CTRL_NVIDIA_MCP73_1	 equ  0x07fc
CTRL_NVIDIA_MCP73_2	 equ  0x07fd
CTRL_NVIDIA_MCP79_1	 equ  0x0ac0
CTRL_NVIDIA_MCP79_2	 equ  0x0ac1
CTRL_NVIDIA_MCP79_3	 equ  0x0ac2
CTRL_NVIDIA_MCP79_4	 equ  0x0ac3
CTRL_NVIDIA_0BE2	 equ  0x0be2
CTRL_NVIDIA_0BE3	 equ  0x0be3
CTRL_NVIDIA_0BE4	 equ  0x0be4
CTRL_NVIDIA_GT100	 equ  0x0be5
CTRL_NVIDIA_GT106	 equ  0x0be9
CTRL_NVIDIA_GT108	 equ  0x0bea
CTRL_NVIDIA_GT104	 equ  0x0beb
CTRL_NVIDIA_GT116	 equ  0x0bee
CTRL_NVIDIA_MCP89_1	 equ  0x0d94
CTRL_NVIDIA_MCP89_2	 equ  0x0d95
CTRL_NVIDIA_MCP89_3	 equ  0x0d96
CTRL_NVIDIA_MCP89_4	 equ  0x0d97
CTRL_NVIDIA_GF119	 equ  0x0e08
CTRL_NVIDIA_GF110_1	 equ  0x0e09
CTRL_NVIDIA_GF110_2	 equ  0x0e0c
; ATI
CTRL_ATI_SB450		 equ  0x437b
CTRL_ATI_SB600		 equ  0x4383
; ATI HDMI
CTRL_ATI_RS600		 equ  0x793b
CTRL_ATI_RS690		 equ  0x7919
CTRL_ATI_RS780		 equ  0x960f
CTRL_ATI_RS_UNK1	 equ  0x970f
CTRL_ATI_R600		 equ  0xaa00
CTRL_ATI_RV630		 equ  0xaa08
CTRL_ATI_RV610		 equ  0xaa10
CTRL_ATI_RV670		 equ  0xaa18
CTRL_ATI_RV635		 equ  0xaa20
CTRL_ATI_RV620		 equ  0xaa28
CTRL_ATI_RV770		 equ  0xaa30
CTRL_ATI_RV730		 equ  0xaa38
CTRL_ATI_RV710		 equ  0xaa40
CTRL_ATI_RV740		 equ  0xaa48
; AMD
CTRL_AMD_HUDSON 	 equ  0x780d
; VIA
CTRL_VIA_VT82XX 	 equ  0x3288
CTRL_VIA_VT61XX 	 equ  0x9140
CTRL_VIA_VT71XX 	 equ  0x9170
; SiS
CTRL_SIS_966		 equ  0x7502
; ULI
CTRL_ULI_M5461		 equ  0x5461
; Creative
CTRL_CREATIVE_CA0110_IBG     equ  0x0009
CTRL_CREATIVE_SOUND_CORE3D_1 equ  0x0010
CTRL_CREATIVE_SOUND_CORE3D_2 equ  0x0012
; Teradici
CTRL_TERA_UNK1		 equ  0x1200
; RDC Semiconductor
CTRL_RDC_R3010		 equ  0x3010
;VMware
CTRL_VMWARE_UNK1	 equ  0x1977


; driver types
AZX_DRIVER_ICH		 equ  0
AZX_DRIVER_PCH		 equ  1
AZX_DRIVER_SCH		 equ  2
AZX_DRIVER_ATI		 equ  3
AZX_DRIVER_ATIHDMI	 equ  4
AZX_DRIVER_VIA		 equ  5
AZX_DRIVER_SIS		 equ  6
AZX_DRIVER_ULI		 equ  7
AZX_DRIVER_NVIDIA	 equ  8
AZX_DRIVER_TERA 	 equ  9
AZX_DRIVER_CTX		 equ  10
AZX_DRIVER_GENERIC	 equ  11
AZX_NUM_DRIVERS 	 equ  12


; registers

ICH6_REG_GCAP		 equ  0x00
ICH6_REG_VMIN		 equ  0x02
ICH6_REG_VMAJ		 equ  0x03
ICH6_REG_OUTPAY 	 equ  0x04
ICH6_REG_INPAY		 equ  0x06
ICH6_REG_GCTL		 equ  0x08
  ICH6_GCTL_RESET	   equ	(1 shl 0)  ; controller reset
  ICH6_GCTL_FCNTRL	   equ	(1 shl 1)  ; flush control
  ICH6_GCTL_UNSOL	   equ	(1 shl 8)  ; accept unsol. response enable
ICH6_REG_WAKEEN 	 equ  0x0c
ICH6_REG_STATESTS	 equ  0x0e
ICH6_REG_GSTS		 equ  0x10
  ICH6_GSTS_FSTS	   equ	(1 shl 1)  ; flush status
ICH6_REG_INTCTL 	 equ  0x20
ICH6_REG_INTSTS 	 equ  0x24
ICH6_REG_WALLCLK	 equ  0x30  ; 24Mhz source
ICH6_REG_OLD_SSYNC	 equ  0x34  ; SSYNC for old ICH
ICH6_REG_SSYNC		 equ  0x38
ICH6_REG_CORBLBASE	 equ  0x40
ICH6_REG_CORBUBASE	 equ  0x44
ICH6_REG_CORBWP 	 equ  0x48
ICH6_REG_CORBRP 	 equ  0x4A
  ICH6_CORBRP_RST	   equ	(1 shl 15)  ; read pointer reset
ICH6_REG_CORBCTL	 equ  0x4c
  ICH6_CORBCTL_RUN	   equ	(1 shl 1)   ; enable DMA
  ICH6_CORBCTL_CMEIE	   equ	(1 shl 0)   ; enable memory error irq
ICH6_REG_CORBSTS	 equ  0x4d
  ICH6_CORBSTS_CMEI	   equ	(1 shl 0)   ; memory error indication
ICH6_REG_CORBSIZE	 equ  0x4e

ICH6_REG_RIRBLBASE	 equ  0x50
ICH6_REG_RIRBUBASE	 equ  0x54
ICH6_REG_RIRBWP 	 equ  0x58
  ICH6_RIRBWP_RST	   equ	(1 shl 15)  ; write pointer reset
ICH6_REG_RINTCNT	 equ  0x5a
ICH6_REG_RIRBCTL	 equ  0x5c
  ICH6_RBCTL_IRQ_EN	   equ	(1 shl 0)   ; enable IRQ
  ICH6_RBCTL_DMA_EN	   equ	(1 shl 1)   ; enable DMA
  ICH6_RBCTL_OVERRUN_EN    equ	(1 shl 2)   ; enable overrun irq
ICH6_REG_RIRBSTS	 equ  0x5d
  ICH6_RBSTS_IRQ	   equ	(1 shl 0)   ; response irq
  ICH6_RBSTS_OVERRUN	   equ	(1 shl 2)   ; overrun irq
ICH6_REG_RIRBSIZE	 equ  0x5e

ICH6_REG_IC		 equ  0x60
ICH6_REG_IR		 equ  0x64
ICH6_REG_IRS		 equ  0x68
  ICH6_IRS_VALID	   equ	2
  ICH6_IRS_BUSY 	   equ	1

ICH6_REG_DPLBASE	 equ  0x70
ICH6_REG_DPUBASE	 equ  0x74
  ICH6_DPLBASE_ENABLE	   equ	1     ; Enable position buffer

; SD offset: SDI0=0x80, SDI1=0xa0, ... SDO3=0x160 */
SDI0_SD_OFFSET	  equ  0x80
SDI1_SD_OFFSET	  equ  0xA0
SDI2_SD_OFFSET	  equ  0xC0
SDI3_SD_OFFSET	  equ  0xE0
SDO0_SD_OFFSET	  equ  0x100
SDO1_SD_OFFSET	  equ  0x120
SDO2_SD_OFFSET	  equ  0X140
SDO3_SD_OFFSET	  equ  0x160

; stream register offsets from stream base
ICH6_REG_SD_CTL 	 equ  0x00
ICH6_REG_SD_STS 	 equ  0x03
ICH6_REG_SD_LPIB	 equ  0x04
ICH6_REG_SD_CBL 	 equ  0x08
ICH6_REG_SD_LVI 	 equ  0x0c
ICH6_REG_SD_FIFOW	 equ  0x0e
ICH6_REG_SD_FIFOSIZE	 equ  0x10
ICH6_REG_SD_FORMAT	 equ  0x12
ICH6_REG_SD_BDLPL	 equ  0x18
ICH6_REG_SD_BDLPU	 equ  0x1c

; PCI space
ICH6_PCIREG_TCSEL	 equ  0x44

; other constants
ICH6_RIRB_EX_UNSOL_EV	 equ   (1 shl 4)

; max number of SDs
MAX_ICH6_DEV		 equ  8
; max number of fragments - we may use more if allocating more pages for BDL
AZX_MAX_FRAG		 equ  (4096 / (MAX_ICH6_DEV * 16))
; max buffer size - no h/w limit, you can increase as you like
AZX_MAX_BUF_SIZE	 equ  (1024*1024*1024)
; max number of PCM devices per card
AZX_MAX_PCMS		 equ  8

; RIRB int mask: overrun[2], response[0]
RIRB_INT_RESPONSE	 equ  0x01
RIRB_INT_OVERRUN	 equ  0x04
RIRB_INT_MASK		 equ  0x05

; STATESTS int mask: SD2,SD1,SD0
STATESTS_INT_MASK	 equ  0x07
AZX_MAX_CODECS		 equ  4

; SD_CTL bits
SD_CTL_STREAM_RESET	 equ  0x01    ; stream reset bit
SD_CTL_DMA_START	 equ  0x02    ; stream DMA start bit
SD_CTL_STREAM_TAG_MASK	 equ  (0xf shl 20)
SD_CTL_STREAM_TAG_SHIFT  equ  20

; SD_CTL and SD_STS
SD_INT_DESC_ERR 	 equ  0x10    ; descriptor error interrupt
SD_INT_FIFO_ERR 	 equ  0x08    ; FIFO error interrupt
SD_INT_COMPLETE 	 equ  0x04    ; completion interrupt
SD_INT_MASK		 equ  (SD_INT_DESC_ERR or SD_INT_FIFO_ERR or SD_INT_COMPLETE)

; SD_STS
SD_STS_FIFO_READY	 equ  0x20    ; FIFO ready

; INTCTL and INTSTS
ICH6_INT_ALL_STREAM	 equ  0xff	      ; all stream interrupts
ICH6_INT_CTRL_EN	 equ  0x40000000      ; controller interrupt enable bit
ICH6_INT_GLOBAL_EN	 equ  0x80000000      ; global interrupt enable bit

; GCTL reset bit
ICH6_GCTL_RESET 	 equ  1

; CORB/RIRB control, read/write pointer
ICH6_RBCTL_DMA_EN	 equ  0x02    ; enable DMA
ICH6_RBCTL_IRQ_EN	 equ  0x01    ; enable IRQ
ICH6_RBRWP_CLR		 equ  0x8000  ; read/write pointer clear
; below are so far hardcoded - should read registers in future
ICH6_MAX_CORB_ENTRIES	 equ  256
ICH6_MAX_RIRB_ENTRIES	 equ  256

; position fix mode
	POS_FIX_AUTO	 equ  0
	POS_FIX_LPIB	 equ  1
	POS_FIX_POSBUF	 equ  2
	POS_FIX_VIACOMBO equ  4
	POS_FIX_COMBO	 equ  8

; Defines for ATI HD Audio support in SB450 south bridge
ATI_SB450_HDAUDIO_MISC_CNTR2_ADDR   equ  0x42
ATI_SB450_HDAUDIO_ENABLE_SNOOP	    equ  0x02

; Defines for Nvidia HDA support
NVIDIA_HDA_TRANSREG_ADDR	    equ  0x4e
NVIDIA_HDA_ENABLE_COHBITS	    equ  0x0f
NVIDIA_HDA_ISTRM_COH		    equ  0x4d
NVIDIA_HDA_OSTRM_COH		    equ  0x4c
NVIDIA_HDA_ENABLE_COHBIT	    equ  0x01

; Defines for Intel SCH HDA snoop control
INTEL_SCH_HDA_DEVC		    equ  0x78
INTEL_SCH_HDA_DEVC_NOSNOOP	    equ  (0x1 shl 11)

; Define IN stream 0 FIFO size offset in VIA controller
VIA_IN_STREAM0_FIFO_SIZE_OFFSET     equ  0x90
; Define VIA HD Audio Device ID
VIA_HDAC_DEVICE_ID		    equ  0x3288

; HD Audio class code
PCI_CLASS_MULTIMEDIA_HD_AUDIO	    equ  0x0403


SRV_GETVERSION		 equ  0
DEV_PLAY		 equ  1
DEV_STOP		 equ  2
DEV_CALLBACK		 equ  3
DEV_SET_BUFF		 equ  4
DEV_NOTIFY		 equ  5
DEV_SET_MASTERVOL	 equ  6
DEV_GET_MASTERVOL	 equ  7
DEV_GET_INFO		 equ  8
DEV_GET_POS		 equ  9
DEV_SET_CHANNEL_VOLUME	 equ  10
DEV_GET_CHANNEL_VOLUME	 equ  11
;Asper: Non standard system service. For the tests only! [
DEV_EXEC_CODEC_CMD	 equ  100
;Asper: Non standard system service. For the tests only! ]

struc AC_CNTRL		    ;AC controller base class
{ .bus		      dd ?
  .devfn	      dd ?

  .vendor	      dw ?
  .dev_id	      dw ?
  .pci_cmd	      dd ?
  .pci_stat	      dd ?

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
  .civ_val	      dd 1
  .user_callback      dd ?

  .ctrl_read8	      dd ?
  .ctrl_read16	      dd ?
  .ctrl_read32	      dd ?

  .ctrl_write8	      dd ?
  .ctrl_write16       dd ?
  .ctrl_write32       dd ?

;Asper+ [
    .codec_mask       dd ?
    .rb 	      dd ?
    .rirb_rp	      dw 0
    .rirb_wp	      dw 0
    .corb_rp	      dw 0
    .corb_wp	      dw 0
    .rirb_cmd	      dd 0
    .rirb_res	      dd 0
    .rirb_error       dd 0
    .response_reset   dd 0
    .polling_mode     db 0
    .poll_count       db 0
    .posbuf	      dd ?
    .start_wallclk    dd ? ; start + minimum wallclk
    .period_wallclk   dd ? ; wallclk for period
    .position_fix     db ?
;Asper+ ]
}

struc CODEC		   ;Audio Chip base class
{
;Asper+ [
  .addr 	      dd ?    ; codec slot index (codec address)
  .afg		      dd ?    ; AFG node id
  .mfg		      dd ?    ; MFG node id

  .function_id	      dd ?
  .subsystem_id       dd ?
  .revision_id	      dd ?
  .chip_id	      dw ?
  .vendor_id	      dw ?

  ; widget capabilities cache
  .num_nodes	      dw ?
  .start_nid	      dw ?
  .wcaps	      dd ?

  .init_pins	      dd ?    ; initial (BIOS) pin configurations
  .num_pins	      dd ?    ;Asper +  : word is enough, but for align...
  .beeper_nid	      dw ?
		.pad  dw ?
;Asper+ ]

  .ac_vendor_ids      dd ?    ;ac vendor id string
  .chip_ids	      dd ?    ;chip model string
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

; Macroses by CleverMouse
; The following macro assume that we are on uniprocessor machine.
; Serious work is needed for multiprocessor machines.
macro spin_lock_irqsave spinlock
{
	pushf
	cli
}
macro spin_unlock_irqrestore spinlock
{
	popf
}
macro spin_lock_irq spinlock
{
	cli
}
macro spin_unlock_irq spinlock
{
	sti
}

SPINLOCK_BUSY = 1
SPINLOCK_FREE = 0

macro spin_lock
{
	push	eax ebx
	mov	eax, aspinlock
	mov	ebx, SPINLOCK_BUSY
  @@:
	lock	xchg [eax], ebx
	cmp	ebx, SPINLOCK_FREE
	jnz	@b
	pop	ebx eax
}

macro spin_unlock
{
	push	eax ebx
	mov	eax, aspinlock
	mov	eax, aspinlock
	mov	ebx, SPINLOCK_FREE
lock	xchg	[eax], ebx
	pop	ebx eax
}

public START
public service_proc
public version

section '.flat' code readable align 16

proc START stdcall, state:dword

	   cmp [state], 1
	   jne .stop

     if DEBUG
	   mov esi, msgTV
	   call SysMsgBoardStr

	   mov esi, msgInit
	   call SysMsgBoardStr
     end if

	   call detect_controller
	   test eax, eax
	   jz .fail

	   mov esi,[ctrl.vendor_ids]
	   call SysMsgBoardStr
	   mov esi, [ctrl.ctrl_ids]
	   call SysMsgBoardStr

	   call init_controller
	   test eax, eax
	   jz .fail

;Asper This part is from "azx_create" proc. [
	   ;(...)
	   mov	 [ctrl.position_fix], POS_FIX_LPIB
	   cmp	 [driver_type], AZX_DRIVER_VIA
	   je	 .set_via_patch
	   cmp	 [driver_type], AZX_DRIVER_ATI
	   jne	 .no_via_patch
  .set_via_patch:
	   or	 [ctrl.position_fix], POS_FIX_VIACOMBO
  .no_via_patch:
	   ; codec detection
	   mov	 eax, [ctrl.codec_mask]
	   test  eax, eax
	   jnz	 @f
	if DEBUG
	   mov	 esi, msgNoCodecsFound
	   jmp	 .fail_msg
	else
	   jmp	 .fail
	end if
  @@:
;Asper ]

	   mov esi, msgPrimBuff
	   call SysMsgBoardStr
	   call create_primary_buff
	   mov esi, msgDone
	   call SysMsgBoardStr

  if IRQ_REMAP
	   pushf
	   cli

	   mov ebx, [ctrl.int_line]
	   in al, 0xA1
	   mov ah, al
	   in al, 0x21
	   test ebx, ebx
	   jz .skip
	   bts ax, bx			   ;mask old line
.skip
	   bts ax, IRQ_LINE		   ;mask new ine
	   out 0x21, al
	   mov al, ah
	   out 0xA1, al
					   ;remap IRQ
	   stdcall PciWrite8, 0, 0xF8, 0x61, IRQ_LINE

	   mov dx, 0x4d0		   ;8259 ELCR1
	   in al, dx
	   bts ax, IRQ_LINE
	   out dx, al			   ;set level-triggered mode
	   mov [ctrl.int_line], IRQ_LINE
	   popf
	   mov esi, msgRemap
	   call SysMsgBoardStr
  end if

	   mov ebx, [ctrl.int_line]
	   stdcall AttachIntHandler, ebx, hda_irq, dword 0

;Asper This part is from "azx_probe" proc. [
	   call  azx_codec_create
	   cmp	 eax, 0
	   jl	 .fail

	   call  azx_codec_configure
	   cmp	 eax, 0
	   jl	 .fail
;] Asper

	   ; create PCM streams
;Asper+ [
	   mov	   eax, [spec.dac_node]
   if DEBUG
       push  eax esi
       mov   esi, msgVal
       call  SysMsgBoardStr
       stdcall	fdword2str, 3
       call  SysMsgBoardStr
       pop   esi eax
   end if

	   test    eax, eax
	   jz	   .fail
	   mov	   ebx, [spec.dac_node+4]
   if DEBUG
       push  eax esi
       mov   esi, msgVal
       call  SysMsgBoardStr
       mov	eax, [spec.dac_node+4]
       stdcall	fdword2str, 3
       call  SysMsgBoardStr
       pop   esi eax
   end if

	   test    ebx, ebx
	   jz	   @f
	   cmp	   eax, ebx
	   je	   @f
	   stdcall hda_codec_setup_stream, ebx, SDO_TAG, 0, 0x11   ; Left & Right channels (Front panel)
	@@:
	   stdcall hda_codec_setup_stream, eax, SDO_TAG, 0, 0x11   ; Left & Right channels (Back panel)
;Asper+ ]

   if USE_SINGLE_MODE
       mov   esi, msgSingleMode
       call  SysMsgBoardStr
   else
       mov   esi, msgNormalMode
       call  SysMsgBoardStr
   end if


.reg:
	   stdcall RegService, sz_sound_srv, service_proc
	   ret
.fail:
	   mov esi, msgFail
.fail_msg:
	   call SysMsgBoardStr
	   xor eax, eax
	   ret
.stop:
	   call stop
	   xor eax, eax
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
	   mov edi, [ioctl]
	   mov eax, [edi+io_code]

	   cmp eax, SRV_GETVERSION
	   jne @F

	   mov eax, [edi+output]
	   cmp [edi+out_size], 4
	   jne .fail

	   mov [eax], dword API_VERSION
	   xor eax, eax
	   ret
@@:
	   cmp eax, DEV_PLAY
	   jne @F
     if DEBUG
	   mov esi, msgPlay
	   call SysMsgBoardStr
     end if
	   call    play
	   xor	   eax, eax
	   ret
@@:
	   cmp eax, DEV_STOP
	   jne @F
     if DEBUG
	   mov esi, msgStop
	   call SysMsgBoardStr
     end if
	   call    stop
	   xor	   eax, eax
	   ret
@@:
	   cmp	   eax, DEV_CALLBACK
	   jne	   @f
	   mov	   ebx, [edi+input]
	   stdcall set_callback, [ebx]
	   xor	   eax, eax
	   ret
@@:
	   cmp	   eax, DEV_SET_MASTERVOL
	   jne	   @f
	   mov	   eax, [edi+input]
	   mov	   eax, [eax]
	   call    set_master_vol
	   xor	   eax, eax
	   ret
@@:
	   cmp	   eax, DEV_GET_MASTERVOL
	   jne	   @f
	   mov	   ebx, [edi+output]
	   stdcall get_master_vol, ebx
	   xor	   eax, eax
	   ret
;@@:
;           cmp     eax, DEV_GET_INFO
;           jne     @f
;           mov     ebx, [edi+output]
;           stdcall get_dev_info, ebx
;           xor     eax, eax
;           ret
@@:
	   cmp	   eax, DEV_GET_POS
	   jne	   @f
	   stdcall azx_get_position
	   shr	   eax, 2
	   mov	   ebx, [edi+output]
	   mov	   [ebx], eax
	   xor	   eax, eax
	   ret
@@:
;           cmp eax, DEV_SET_CHANNEL_VOLUME
;           jne @f
;     if DEBUG
;           mov esi, msgSetChannelVolume
;           call SysMsgBoardStr
;     end if
;           mov      ebx, [edi+input]
;           mov      cl,  byte [ebx]      ; cl=channel
;           mov      eax, dword [ebx+1]   ; eax=volume in Db
;     if DEBUG
;           push    eax esi
;           mov     esi, msgYAHOO1
;           call    SysMsgBoardStr
;           stdcall fdword2str, 1
;           call    SysMsgBoardStr
;           mov     esi, strSemicolon
;           call    SysMsgBoardStr
;           movzx   eax, cl
;           stdcall fdword2str, 3
;           call    SysMsgBoardStr
;           pop     esi eax
;     end if
;
;           call    set_channel_volume
;           xor     eax, eax
;           ret
;@@:
;           cmp     eax, DEV_GET_CHANNEL_VOLUME
;           jne     @f
;           mov     cl,  byte [edi+input]  ; cl=channel
;           call    get_channel_volume
;           mov     ebx, [edi+output]
;           mov     [ebx], eax
;           xor     eax, eax
;           ret
;@@:

;Asper: Non standard system service. For the tests only! [
@@:
	   cmp	    eax, DEV_EXEC_CODEC_CMD
	   jne	    @f

	   mov	    eax, [edi+input]
	   mov	    eax, [eax]
	   stdcall  codec_exec_verb, eax
	   xor	    eax, eax
	   ret
@@:
;Asper: Non standard system service. For the tests only! ]


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
proc hda_irq   ;+
	   spin_lock
     if DEBUG_IRQ
	   push eax esi
	   ;mov esi, msgIRQ
	   ;call SysMsgBoardStr
	   call GetTimerTicks
	   stdcall  fdword2str, 2
	   call SysMsgBoardStr
	   pop	esi eax
     end if
	   mov	 edx, ICH6_REG_INTSTS
	   call  azx_readl
	   test  eax, eax
	   jnz	 @f
	   spin_unlock
	   ret
  @@:
	   mov	 ebx, eax ; status
	   mov	 eax, SDO_INT
	   test  ebx, eax
	   jz	 @f

	   mov	 edx, ICH6_REG_SD_STS + SDO_OFS
	   call  azx_readb
	   mov	 bl, al

	   mov	  al, SD_INT_MASK
	   mov	 edx, ICH6_REG_SD_STS + SDO_OFS
	   call  azx_writeb

	   test  bl, SD_INT_COMPLETE
	   jz	 @f

	   mov	 eax, [ctrl.civ_val]
	   inc	 eax
	   and	 eax, 4-1 ;2-1
	   mov	 [ctrl.civ_val], eax

	   mov	 ebx, dword [buff_list+eax*4]
	   cmp	 [ctrl.user_callback], 0
	   je	 @f
	   stdcall [ctrl.user_callback], ebx

  @@:

	   ; clear rirb int
	   mov	 edx, ICH6_REG_RIRBSTS
	   call  azx_readb

	   test  al, RIRB_INT_MASK
	   jz	 .l1
	   test  al, RIRB_INT_RESPONSE
	   jz	 .l2

	   cmp	 byte [driver_type], AZX_DRIVER_CTX
	   jne	 @f
	   mov	 eax, 80    ; wait 80 us
	   call  StallExec
  @@:

	   call  azx_update_rirb
  .l2:
	   mov	  al, RIRB_INT_MASK
	   mov	 edx, ICH6_REG_RIRBSTS
	   call  azx_writeb
  .l1:

;if 0
	; clear state status int
	   mov	 edx, ICH6_REG_STATESTS
	   call  azx_readb
	   test  al, 0x04
	   jz	 @f

	   mov	 al, 0x04
	   mov	 edx, ICH6_REG_STATESTS
	   call  azx_writeb
  @@:
;end if
	   or	 eax, 1
	   spin_unlock
	   ret
endp


align 4
proc create_primary_buff

	   stdcall KernelAlloc, 4096
	   mov [ctrl.posbuf], eax

	   stdcall KernelAlloc, 0x10000 ;0x8000
	   mov [ctrl.buffer], eax

	   mov edi, eax
	   mov ecx, 0x10000/4 ;0x8000/4
	   xor eax, eax
	   cld
	   rep stosd


	   stdcall KernelAlloc, 4096
	   mov [pcmout_bdl], eax

	   mov edi, eax
	   mov ecx, 4096/4
	   xor eax, eax
	   cld
	   rep stosd


	   ; reset BDL address
	   xor	 eax, eax
	   mov	 edx, ICH6_REG_SD_BDLPL + SDO_OFS
	   call  azx_writel
	   xor	 eax, eax
	   mov	 edx, ICH6_REG_SD_BDLPU + SDO_OFS
	   call  azx_writel

	   ; program the initial BDL entries
	   mov eax, [ctrl.buffer]
	   mov ebx, eax
	   call GetPgAddr
	   and	 ebx, 0xFFF
	   add	 eax, ebx


	   mov ebx, 0x4000 ;buffer size
	   mov ecx, 8	   ;number of periods
	   mov edi, [pcmout_bdl] ;pcmout_bdl
  .next_period:
	   push eax ecx
	   mov ecx, 4 ;2  ;number of bdl in a period
  .next_bdl:
	   ; program the address field of the BDL entry
	   mov dword [edi], eax
	   mov dword [edi+4], 0
	   ; program the size field of the BDL entry
	   mov dword [edi+8],  ebx
	   ; program the IOC to enable interrupt when buffer completes
	   mov dword [edi+12], 0x01


	   add	eax, ebx
	   add	edi, 16
	   dec	ecx
	   jnz	.next_bdl

	   pop	ecx eax
	   dec	ecx
	   jnz	.next_period


	   mov edi, buff_list
	   mov eax, [ctrl.buffer]
	   mov ecx, 4 ;2
@@:
	   mov [edi], eax
	   mov [edi+8], eax
	   mov [edi+16], eax
	   mov [edi+24], eax
	   mov [edi+32], eax
	   mov [edi+40], eax
	   mov [edi+48], eax
	   mov [edi+56], eax

	   add eax, ebx
	   add edi, 4
	   loop @B

	   ; wallclk has 24Mhz clock source
	   mov [ctrl.period_wallclk], ((0x4000 * 24000) / 48000) * 1000

	   call  azx_stream_reset
	   call  azx_setup_controller
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
	   mov [ctrl.vendor], dx
	   shr eax, 16
	   mov [ctrl.dev_id], ax

	   mov ebx, [edi+4]
	   mov [ctrl.ctrl_ids], ebx

	   cmp edx, VID_INTEL
	   jne @F
	   mov [ctrl.vendor_ids], msg_Intel
	   jmp .ok
  @@:
	   cmp edx, VID_NVIDIA
	   jne @F
	   mov [ctrl.vendor_ids], msg_NVidia
	   jmp .ok
  @@:
	   cmp edx, VID_ATI
	   jne @F
	   cmp eax, 0x4383
	   jg  .ati_hdmi
	   mov [ctrl.vendor_ids], msg_ATI
	   jmp .ok
  .ati_hdmi:
	   mov [ctrl.vendor_ids], msg_ATI_HDMI
	   jmp .ok
  @@:
	   cmp edx, VID_AMD
	   jne @F
	   mov [ctrl.vendor_ids], msg_AMD
	   jmp .ok
  @@:
	   cmp edx, VID_VIA
	   jne @F
	   mov [ctrl.vendor_ids], msg_VIA
	   jmp .ok
  @@:
	   cmp edx, VID_SIS
	   jne @F
	   mov [ctrl.vendor_ids], msg_SIS
	   jmp .ok
  @@:
	   cmp edx, VID_ULI
	   jne @F
	   mov [ctrl.vendor_ids], msg_ULI
	   jmp .ok
  @@:
	   cmp edx, VID_TERA
	   jne @F
	   mov [ctrl.vendor_ids], msg_TERA
	   jmp .ok
  @@:
	   cmp edx, VID_CREATIVE
	   jne @F
	   mov [ctrl.vendor_ids], msg_CREATIVE
	   jmp .ok
  @@:
	   cmp edx, VID_RDC
	   jne @F
	   mov [ctrl.vendor_ids], msg_RDC
	   jmp .ok
  @@:
	   cmp edx, VID_VMWARE
	   jne @F
	   mov [ctrl.vendor_ids], msg_VMWARE
	   jmp .ok
  @@:
  .err:
	   xor eax, eax
	   mov [ctrl.vendor_ids], eax	  ;something  wrong ?
	   mov [driver_type], -1
	   ret
  .ok:
	   mov ebx, [edi+8]
	   mov [driver_type], ebx
	   ret
endp

align 4
proc init_controller

	   stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 4
	   test eax, 0x4 ; Test Master bit
	   jnz	@f
	   or	eax, 0x4 ; Set Master bit
	   stdcall PciWrite32, [ctrl.bus], [ctrl.devfn], dword 4, eax
	   stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 4
  @@:

	   mov ebx, eax
	   and eax, 0xFFFF
	   mov [ctrl.pci_cmd], eax
	   shr ebx, 16
	   mov [ctrl.pci_stat], ebx

	   mov esi, msgPciCmd
	   call SysMsgBoardStr
	   stdcall  fdword2str, 2
	   call SysMsgBoardStr

	   mov esi, msgPciStat
	   call SysMsgBoardStr
	   mov eax, [ctrl.pci_stat]
	   stdcall  fdword2str, 2
	   call SysMsgBoardStr

	   mov esi, msgHDALowMMIo
	   call SysMsgBoardStr
	   stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x10
	   stdcall  fdword2str, 2
	   call SysMsgBoardStr

	   and eax, 0xFFFFC000
	   mov [ctrl.ctrl_mem_base], eax

	   mov esi, msgHDAUpMMIo
	   call SysMsgBoardStr
	   stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x14
	   ;-mov [ctrl.hda_upper_mem_base], eax
	   stdcall  fdword2str, 2
	   call SysMsgBoardStr

  .default:
	   stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x3C
	   and eax, 0xFF
  @@:
	   mov [ctrl.int_line], eax
	   mov [ctrl.user_callback], 0

	   call    set_HDA
;Asper This is from "azx_create" proc. [
	   xor	   eax, eax
	   mov	   edx, ICH6_REG_GCAP
	   call    azx_readw
	if DEBUG
	   mov	   esi, msgGCap
	   call    SysMsgBoardStr
	   stdcall fdword2str, 2
	   call    SysMsgBoardStr
	end if

	   ; allocate CORB/RIRB
	   call    azx_alloc_cmd_io

	   ; initialize chip
	   call    azx_init_pci

	   xor	   eax, eax
	   call    azx_init_chip
;] Asper

	   xor	 eax, eax
	   inc	 eax
	   ret
endp




PG_SW		 equ 0x003
PG_NOCACHE	 equ 0x018

align 4
proc set_HDA

	   stdcall MapIoMem,[ctrl.ctrl_mem_base],0x1000,PG_SW+PG_NOCACHE
	   mov	 [ctrl.ctrl_mem_base], eax
	   ret
endp


; in:  eax - fullreset_flag
;
; reset codec link
align 4
proc reset_controller
	   locals
	     counter dd ?
	   endl

	   test  eax, eax
	   jz	 .skip

	   ; clear STATESTS
	   mov	 eax, STATESTS_INT_MASK
	   mov	 edx, ICH6_REG_STATESTS
	   call  azx_writeb

	   ; reset controller
	   mov	 edx, ICH6_REG_GCTL
	   call  azx_readl
	   mov	 ebx, ICH6_GCTL_RESET
	   xor	 ebx, -1
	   and	 eax, ebx
	   mov	 edx, ICH6_REG_GCTL
	   call  azx_writel

	   mov	 [counter], 50	  ; total 50*100 us = 0.5s
  .wait0:

	   mov	 edx, ICH6_REG_GCTL
	   call  azx_readb
	   test  eax, eax
	   jz	 @f

	   mov	 eax, 100    ; wait 100 us
	   call  StallExec

	   dec	 [counter]
	   jnz	 .wait0
  @@:
	   ; delay for >= 100us for codec PLL to settle per spec
	   ; Rev 0.9 section 5.5.1
	   mov	 eax, 100    ; wait 100 us
	   call  StallExec

	   ; Bring controller out of reset
	   mov	 edx, ICH6_REG_GCTL
	   call  azx_readb
	   or	 eax, ICH6_GCTL_RESET
	   mov	 edx, ICH6_REG_GCTL
	   call  azx_writeb

	   mov	 [counter], 50	  ; total 50*100 us = 0.5s
  .wait1:

	   mov	 edx, ICH6_REG_GCTL
	   call  azx_readb
	   test  eax, eax
	   jnz	 @f

	   mov	 eax, 100    ; wait 100 us
	   call  StallExec

	   dec	 [counter]
	   jnz	 .wait1
  @@:

	   ; Brent Chartrand said to wait >= 540us for codecs to intialize
	   mov	 eax, 540    ; wait 540 us
	   call  StallExec

  .skip:
	   ; check to see if controller is ready
	   mov	 edx, ICH6_REG_GCTL
	   call  azx_readb
	   test  eax, eax
	   jz	 .fail

	   ; Accept unsolicited responses
       if USE_SINGLE_MODE
       else
;UNSUPPORTED YET! [
;           mov   edx, ICH6_REG_GCTL
;           call  azx_readl
;           or    eax, ICH6_GCTL_UNSOL
;           mov   edx, ICH6_REG_GCTL
;           call  azx_writel
;UNSUPPORTED YET! ]
       end if

	   ; detect codecs
	   mov	 eax, [ctrl.codec_mask]
	   test  ax, ax
	   jnz	 @f

	   mov	 edx, ICH6_REG_STATESTS
	   call  azx_readw
	   mov	 [ctrl.codec_mask], eax

     if DEBUG
	   mov	 esi, msgCodecMask
	   call  SysMsgBoardStr
	   stdcall  fdword2str, 2
	   call  SysMsgBoardStr
     end if

  @@:

  .ok:
	   clc
	   ret
  .fail:
     if DEBUG
	   mov	 esi, msgHDARFail
	   call  SysMsgBoardStr
     end if
	   stc
	   ret
endp


align 4
play:
	   spin_lock
	   mov	 edx, ICH6_REG_WALLCLK
	   call  azx_readl
	   mov	 [ctrl.start_wallclk], eax

	   call  azx_stream_start
	   xor	 eax, eax
	   spin_unlock
	   ret

align 4
stop:
	     spin_lock
;*           call  azx_stream_stop        ;Asper: Hangs system
;R           push  ebx ecx edx
;R           ; stop DMA
;R           mov   edx, ICH6_REG_SD_CTL
;R           call  azx_sd_readb
;R           mov    bl, SD_CTL_DMA_START or SD_INT_MASK
;R           xor    bl, -1
;R           and    al, bl
;R           mov   edx, ICH6_REG_SD_CTL
;R           call  azx_sd_writeb
;R           mov   edx, ICH6_REG_SD_STS
;R           mov    al, SD_INT_MASK
;R           call  azx_sd_writeb  ; to be sure
	   ; disable SIE
;N           mov   edx, ICH6_REG_INTCTL
;N           call  azx_readb
;N           mov    bl, SDO_INT ;shl azx_dev->index
;N           xor    bl, -1
;N           and    al, bl
;N           mov   edx, ICH6_REG_INTCTL
;N           call  azx_writeb

	   ;     int timeout = 5000;
	   ;     while (azx_sd_readb(azx_dev, SD_CTL) & SD_CTL_DMA_START && --timeout) ;
;Asper: Hangs system   [
;*           mov   ecx, 5000
;*  .l1:
;*           mov   edx, ICH6_REG_SD_CTL
;*           call  azx_sd_readb
;*           test    al, SD_CTL_DMA_START
;*           jz    @f
;*           dec   ecx
;*           jnz   .l1
;*  @@:
;*
;*           pop   edx ecx ebx
;Asper ]

	   xor	 eax, eax
	   spin_unlock
	   ret

;align 4
;proc get_dev_info stdcall, p_info:dword
;           virtual at esi
;             CTRL_INFO CTRL_INFO
;           end virtual
;
;           mov esi, [p_info]
;           mov eax, [ctrl.int_line]
;           mov bx,  [ctrl.dev_id]
;           shl ebx, 16
;           and bx,  [ctrl.vendor]
;           mov ecx, [ctrl.pci_cmd]
;           mov edx, [ctrl.codec_mem_base] ;[ctrl.hda_lower_mem_base]
;           mov edi, [ctrl.ctrl_mem_base] ;[ctrl.hda_upper_mem_base]
;
;           mov [CTRL_INFO.irq], eax
;           mov [CTRL_INFO.codec_id], ebx
;           mov [CTRL_INFO.pci_cmd], ecx
;           mov [CTRL_INFO.codec_mem_base], edx
;           mov [CTRL_INFO.ctrl_mem_base], edi
;
;           xor eax, eax
;           mov [CTRL_INFO.codec_io_base], eax
;           mov [CTRL_INFO.ctrl_io_base], eax
;           mov [CTRL_INFO.glob_cntrl], eax
;           mov [CTRL_INFO.glob_sta], eax
;           ret
;endp

align 4
proc set_callback stdcall, handler:dword
	   mov eax, [handler]
	   mov [ctrl.user_callback], eax
	   ret
endp



;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Interface for HD codec ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; CORB / RIRB interface ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc  azx_alloc_cmd_io
	   push  eax ecx edx
	   ; single page (at least 4096 bytes) must suffice for both ringbuffers
	   stdcall KernelAlloc, 4096
	   mov	 [ctrl.rb], eax

	   mov	 edi, eax
	   mov	 ecx, 4096/4
	   xor	 eax, eax
	   cld
	   rep	 stosd

	   pop	 edx ecx eax
	   ret
endp

proc  azx_init_cmd_io
	   spin_lock_irq
	   pusha
	   ; CORB set up
	   mov	 eax, [ctrl.rb]
	   mov	 ebx, eax
	   call  GetPgAddr
	   and	 ebx, 0xFFF
	   add	 eax, ebx
	   push  eax  ; save corb address
	   mov	 edx, ICH6_REG_CORBLBASE
	   call  azx_writel
	   xor	 eax, eax
	   mov	 edx, ICH6_REG_CORBUBASE
	   call  azx_writel

	   ; set the corb size to 256 entries (ULI requires explicitly)
	   mov	  al, 0x02
	   mov	 edx, ICH6_REG_CORBSIZE
	   call  azx_writeb
	   ; set the corb write pointer to 0
	   xor	  ax, ax
	   mov	 edx, ICH6_REG_CORBWP
	   call  azx_writew
	   ; reset the corb hw read pointer
	   mov	  ax, ICH6_CORBRP_RST
	   mov	 edx, ICH6_REG_CORBRP
	   call  azx_writew
	   ; enable corb dma
	   mov	  al, ICH6_CORBCTL_RUN
	   mov	 edx, ICH6_REG_CORBCTL
	   call  azx_writeb

	   ; RIRB set up
	   mov	 [ctrl.rirb_rp], 0
	   mov	 [ctrl.rirb_wp], 0
	   mov	 [ctrl.rirb_cmd], 0

	   pop	 eax  ; restore corb address
	   add	 eax, 2048
	   mov	 edx, ICH6_REG_RIRBLBASE
	   call  azx_writel
	   xor	 eax, eax
	   mov	 edx, ICH6_REG_RIRBUBASE
	   call  azx_writel

	   ; set the rirb size to 256 entries (ULI requires explicitly)
	   mov	  al, 0x02
	   mov	 edx, ICH6_REG_RIRBSIZE
	   call  azx_writeb
	   ; reset the rirb hw write pointer
	   mov	  ax, ICH6_RIRBWP_RST
	   mov	 edx, ICH6_REG_RIRBWP
	   call  azx_writew
	   ; set N=1, get RIRB response interrupt for new entry
	   xor	  ax, ax
	   cmp	 byte [driver_type], AZX_DRIVER_CTX
	   jne	 @f
	   mov	 ax, 0xC0-1
  @@:
	   inc	  ax
	   mov	 edx, ICH6_REG_RINTCNT
	   call  azx_writew
	   ; enable rirb dma and response irq
	   mov	  al, ICH6_RBCTL_DMA_EN or ICH6_RBCTL_IRQ_EN
	   mov	 edx, ICH6_REG_RIRBCTL
	   call  azx_writeb

	   popa
	   spin_unlock_irq
	   ret
endp

proc  azx_free_cmd_io
	   spin_lock_irq
	   push  eax edx
	   ; disable ringbuffer DMAs
	   xor	  al, al
	   mov	 edx, ICH6_REG_RIRBCTL
	   call  azx_writeb
	   mov	 edx, ICH6_REG_CORBCTL
	   call  azx_writeb
	   pop	 edx eax
	   spin_unlock_irq
	   ret
endp


; send a command
proc  azx_corb_send_cmd  stdcall, val:dword
	   spin_lock_irq
	   push  edx edi
	   xor	 eax, eax
	   ; add command to corb
	   mov	 edx, ICH6_REG_CORBWP
	   call  azx_readb
	   inc	 al
	   inc	 dword [ctrl.rirb_cmd]
	   mov	 edi, dword [ctrl.rb]

	   push  eax
	   shl	 eax, 2 ;wp=wp*sizeof(corb entry)=wp*4
	   add	 edi, eax
	   mov	 eax, dword [val]
	   stosd
	   pop	 eax
	   mov	 edx, ICH6_REG_CORBWP
	   call  azx_writel

	   pop	 edi edx
	   xor	 eax, eax ;Asper+
	   spin_unlock_irq
	   ret
endp


; retrieve RIRB entry - called from interrupt handler
proc  azx_update_rirb
	   pusha
	   xor	 eax, eax
	   mov	 edx, ICH6_REG_RIRBWP
	   call  azx_readb ;call  azx_readw

	   cmp	 ax, [ctrl.rirb_wp]
	   je	 .done
	   mov	 [ctrl.rirb_wp], ax
	   mov	 bx, [ctrl.rirb_rp]

  .l1:
	   cmp	 bx, [ctrl.rirb_wp]
	   je	 .l3

	   inc	 bl
  .l2:
	   cmp	 bx, ICH6_MAX_RIRB_ENTRIES
	   jl	 @f
	   sub	 bx, ICH6_MAX_RIRB_ENTRIES
	   jmp	 .l2
  @@:

	   movzx edx, bx
	   shl	 edx, 1 + 2 ; an RIRB entry is 8-bytes
	   mov	 esi, dword [ctrl.rb]
	   add	 esi, 2048
	   add	 esi, edx
	   lodsd   ; res
	   mov	 edx, eax
	   lodsd   ; res_ex

	   test  eax, ICH6_RIRB_EX_UNSOL_EV
	   jz	 @f
	   stdcall  snd_hda_queue_unsol_event, edx, eax
	   jmp	 .l1
  @@:
	   mov	 ecx, [ctrl.rirb_cmd]
	   test  ecx, ecx
	   jz	 @f
	   mov	 [ctrl.rirb_res], edx
	   dec	 dword [ctrl.rirb_cmd]
	   jmp	 .l1
  @@:
       if DEBUG
	   push  esi
	   mov	 esi, msgSpuriousResponce
	   call  SysMsgBoardStr
	   pop	 esi
       end if
	   jmp	 .l1
  .l3:
	   mov	  [ctrl.rirb_rp], bx
  .done:
	   popa
	   ret
endp

; receive a response
proc  azx_rirb_get_response
	 locals
	     do_poll  db 0
	 endl

      push    ebx ecx edx
  .again:
      mov     ecx, 1000;+1000
  .next_try:
      mov     al, [ctrl.polling_mode]
      test    al, al
      jnz     .poll
      mov     ah, [do_poll]
      test    ah, ah
      jz      @f
  .poll:
      spin_lock_irq
      call    azx_update_rirb
      spin_unlock_irq
  @@:
      mov     eax, [ctrl.rirb_cmd]
      test    eax, eax
      jnz     .l1
      mov     [ctrl.rirb_error], 0
      mov     al, [do_poll]
      test    al, al
      jnz     @f
      mov     [ctrl.poll_count], 0
  @@:
      mov     eax, [ctrl.rirb_res] ; the last value
      jmp     .out
  .l1:
      push    eax
      mov     eax, 2000  ; temporary workaround
      call    StallExec
      pop     eax
      dec     ecx
      jnz     .next_try
  .no_next_try:
      mov     al, [ctrl.polling_mode]
      test    al, al
      jnz     .no_poll

      mov     al, [ctrl.poll_count]
      cmp     al, 2
      jge     .poll_count_overflow
    if DEBUG
      push    eax esi
      mov     esi, msgGetResponceTimeout
      call    SysMsgBoardStr
      mov     esi, msgPollingCodecOnce
      call    SysMsgBoardStr
      pop     esi eax
    end if
      mov     [do_poll], 1
      inc     [ctrl.poll_count]
      jmp     .again

  .poll_count_overflow:
    if DEBUG
      push    eax esi
      mov     esi, msgGetResponceTimeout
      call    SysMsgBoardStr
      mov     esi, msgSwitchToPollMode
      call    SysMsgBoardStr
      pop     esi eax
    end if
      mov     [ctrl.polling_mode], 1
      jmp     .again

  .no_poll:

      mov     al, [ctrl.polling_mode]
      test    al, al
      jz      @f
      mov     eax, -1
      jmp     .out
  @@:


      ; a fatal communication error; need either to reset or to fallback
      ; to the single_cmd mode
      mov     [ctrl.rirb_error], 1
      ;Asper~ -?  [
      mov     [ctrl.response_reset], 1
      mov     eax, -1  ; give a chance to retry
      jmp     .out
      ;Asper~ -?  ]

      ;-? mov     [ctrl.single_cmd], 1
      mov     [ctrl.response_reset], 0

      ; release CORB/RIRB
      call  azx_free_cmd_io
      ; disable unsolicited responses
      mov   edx, ICH6_REG_GCTL
      call  azx_readl
      mov   ebx, ICH6_GCTL_UNSOL
      xor   ebx, -1
      and   eax, ebx
      mov   edx, ICH6_REG_GCTL
      call  azx_writel
      mov   eax, -1
  .out:
      pop    edx ecx ebx
      ret
endp

;
; Use the single immediate command instead of CORB/RIRB for simplicity
;
; Note: according to Intel, this is not preferred use.  The command was
;       intended for the BIOS only, and may get confused with unsolicited
;       responses.  So, we shouldn't use it for normal operation from the
;       driver.
;       I left the codes, however, for debugging/testing purposes.
;

; receive a response
proc  azx_single_wait_for_response
	push  ecx edx esi

	mov   ecx, 50
  .l1:
	test  ecx, ecx
	jz    .timeout

	; check IRV busy bit
	mov   edx, ICH6_REG_IRS
	call  azx_readw
	test   ax, ICH6_IRS_VALID
	jz    @f
	; reuse rirb.res as the response return value
	mov   edx, ICH6_REG_IR
	call  azx_readl
	mov   [ctrl.rirb_res], eax

	pop   esi edx ecx
	xor   eax, eax
	ret
     @@:
	xor   eax, eax
	inc   eax
	call  StallExec

	dec    ecx
	jmp    .l1
  .timeout:
      if DEBUG
	xor   eax, eax
	mov   edx, ICH6_REG_IRS
	call  azx_readw
	mov   esi, msgGetResponceTimeout
	call  SysMsgBoardStr
	mov   esi, msgIRS
	call  SysMsgBoardStr
	stdcall  fdword2str, 2
	call  SysMsgBoardStr
      end if

	pop   esi edx ecx
	mov   eax, -1
	mov   [ctrl.rirb_res], eax
	ret
endp

; send a command
proc  azx_single_send_cmd  stdcall, val:dword
	push	ecx edx esi

	mov	ecx, 50
  .l1:
	test	ecx, ecx
	jz	.timeout

	; check ICB busy bit
	mov    edx, ICH6_REG_IRS
	call   azx_readw
	test   ax, ICH6_IRS_BUSY
	jnz    @f
	; Clear IRV valid bit
	mov    edx, ICH6_REG_IRS
	call   azx_readw
	or     ax, ICH6_IRS_VALID
	mov    edx, ICH6_REG_IRS
	call   azx_writew

	mov    eax, dword [val]
	mov    edx, ICH6_REG_IC
	call   azx_writel

	mov    edx, ICH6_REG_IRS
	call   azx_readw
	or     ax, ICH6_IRS_BUSY
	mov    edx, ICH6_REG_IRS
	call   azx_writew

	stdcall azx_single_wait_for_response
	pop    esi edx ecx
	ret
     @@:
	dec	ecx
	jmp	.l1
  .timeout:
      if DEBUG
	xor    eax, eax
	mov    edx, ICH6_REG_IRS
	call   azx_readw
	mov    esi, msgSendCmdTimeout
	call   SysMsgBoardStr
	stdcall  fdword2str, 2
	call   SysMsgBoardStr
	mov    esi, msgVal
	call   SysMsgBoardStr
	mov    eax, dword [val]
	stdcall  fdword2str, 2
	call   SysMsgBoardStr
      end if

	pop    esi edx ecx
	mov    eax, -1
	ret
endp

; receive a response
proc  azx_single_get_response
	mov   eax, [ctrl.rirb_res]
	ret
endp

;
; The below are the main callbacks from hda_codec.
;
; They are just the skeleton to call sub-callbacks according to the
; current setting of chip->single_cmd.
;

; send a command
proc  azx_send_cmd  stdcall, val:dword
	   if USE_SINGLE_MODE
		 stdcall  azx_single_send_cmd, [val]
	   else
		 stdcall  azx_corb_send_cmd, [val]
	   end if
	   ret
endp

; get a response
proc  azx_get_response
	   if USE_SINGLE_MODE
		 call  azx_single_get_response
	   else
		 call  azx_rirb_get_response
	   end if
	   ret
endp


;;;;;;;;;;;;;;;;;;;;;;;;
;; Lowlevel interface ;;
;;;;;;;;;;;;;;;;;;;;;;;;

; enable interrupts
proc  azx_int_enable
	   push  eax edx
	   ; enable controller CIE and GIE
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_readl
	   or	 eax, ICH6_INT_CTRL_EN or ICH6_INT_GLOBAL_EN
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_writel
	   pop	 edx eax
	   ret
endp

; disable interrupts
proc  azx_int_disable
	   push  eax ebx edx

	   ; disable interrupts in stream descriptor
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readb
	   mov	 bl, SD_INT_MASK
	   xor	 bl, -1
	   and	 al, bl
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_writeb

	   ; disable SIE for all streams
	   xor	 al, al
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_writeb

	   ; disable controller CIE and GIE
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_readl
	   mov	 ebx, ICH6_INT_CTRL_EN or ICH6_INT_GLOBAL_EN
	   xor	 ebx, -1
	   and	 eax, ebx
	   call  azx_writel
	   pop	 edx ebx eax
	   ret
endp

; clear interrupts
proc  azx_int_clear
	   push  eax edx

	   ; clear stream status
	   mov	  al, SD_INT_MASK
	   mov	 edx, ICH6_REG_SD_STS + SDO_OFS
	   call  azx_writeb

	   ; clear STATESTS
	   mov	  al, STATESTS_INT_MASK
	   mov	 edx, ICH6_REG_STATESTS
	   call  azx_writeb

	   ; clear rirb status
	   mov	  al, RIRB_INT_MASK
	   mov	 edx, ICH6_REG_RIRBSTS
	   call  azx_writeb

	   ; clear int status
	   mov	 eax, ICH6_INT_CTRL_EN or ICH6_INT_ALL_STREAM
	   mov	 edx, ICH6_REG_INTSTS
	   call  azx_writel
	   pop	 edx eax
	   ret
endp


; start a stream
proc  azx_stream_start
	   push  eax edx

	   ; enable SIE
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_readl

	   or	 eax, 0xC0000000 ;Asper+
	   or	 eax, SDO_INT  ; Asper: output stream interrupt index
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_writel
	   ; set DMA start and interrupt mask
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readb

	   or	  al, SD_CTL_DMA_START or SD_INT_MASK

	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_writeb

	   pop	 edx eax
	   ret
endp

; stop DMA
proc  azx_stream_clear
	   push  eax ebx edx
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readb
	   mov	  bl, SD_CTL_DMA_START or SD_INT_MASK
	   xor	  bl, -1
	   and	  al, bl
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_writeb
	   mov	  al, SD_INT_MASK
	   mov	 edx, ICH6_REG_SD_STS + SDO_OFS
	   call  azx_writeb
	   pop	 edx ebx eax
	   ret
endp

; stop a stream
proc  azx_stream_stop
	   push  eax ebx edx
	   call  azx_stream_clear
	   ; disable SIE
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_readl
	   mov	 ebx, (SDO_INT)
	   xor	 ebx, -1
	   and	 eax, ebx
	   mov	 edx, ICH6_REG_INTCTL
	   call  azx_writel
	   pop	 edx ebx eax
	   ret
endp

;
;in: eax = full_reset
;
; initialize the chip
proc  azx_init_chip
	   push  eax

	   ; reset controller
	   mov	 eax, 1 ;full reset
	   call  reset_controller

	   ; initialize interrupts
	   call  azx_int_clear
	   call  azx_int_enable

	   ; initialize the codec command I/O
	if USE_SINGLE_MODE
	else
	   call  azx_init_cmd_io
	end if

	   ; program the position buffer
	   mov	 eax, dword [ctrl.posbuf]
	   mov	 ebx, eax
	   call  GetPgAddr
	   and	 ebx, 0xFFF
	   add	 eax, ebx
	   mov	 edx, ICH6_REG_DPLBASE
	   call  azx_writel
	   xor	 eax, eax
	   mov	 edx, ICH6_REG_DPUBASE
	   call  azx_writel

	   pop	 eax
	   ret
endp


; initialize the PCI registers

; update bits in a PCI register byte
proc   update_pci_byte	stdcall, reg:dword, mask:dword, val:dword
	   push  ax bx
	   stdcall  PciRead8,  [ctrl.bus], [ctrl.devfn], [reg]
	   mov	 bl, byte [mask]
	   mov	 bh, bl
	   xor	 bl, -1
	   and	 al, bl
	   shr	 bx, 8
	   and	 bl, byte [val]
	   or	 al, bl
	   stdcall  PciWrite8, [ctrl.bus], [ctrl.devfn], [reg], eax
	   pop	 bx ax
	   ret
endp


proc  azx_init_pci
	   ; Clear bits 0-2 of PCI register TCSEL (at offset 0x44)
	   ; TCSEL == Traffic Class Select Register, which sets PCI express QOS
	   ; Ensuring these bits are 0 clears playback static on some HD Audio
	   ; codecs
	   push  eax
	   stdcall  update_pci_byte, ICH6_PCIREG_TCSEL, 0x07, 0

	   mov	 eax, [driver_type]
	   cmp	 eax, AZX_DRIVER_ATI
	   jne	 @f
	   ; For ATI SB450 azalia HD audio, we need to enable snoop
	   stdcall update_pci_byte, ATI_SB450_HDAUDIO_MISC_CNTR2_ADDR, 0x07, ATI_SB450_HDAUDIO_ENABLE_SNOOP
	   jmp	 .done
  @@:
	   cmp	 eax, AZX_DRIVER_NVIDIA
	   jne	 @f
	   ; For NVIDIA HDA, enable snoop
	   stdcall update_pci_byte, NVIDIA_HDA_TRANSREG_ADDR, 0x0f, NVIDIA_HDA_ENABLE_COHBITS
	   stdcall update_pci_byte, NVIDIA_HDA_ISTRM_COH, 0x01, NVIDIA_HDA_ENABLE_COHBIT
	   stdcall update_pci_byte, NVIDIA_HDA_OSTRM_COH, 0x01, NVIDIA_HDA_ENABLE_COHBIT
	   jmp	 .done
  @@:
	   cmp	 eax, AZX_DRIVER_SCH
	   je	 .l1
	   cmp	 eax, AZX_DRIVER_PCH
	   jne	 @f
  .l1:
	   stdcall  PciRead16,	[ctrl.bus], [ctrl.devfn], dword INTEL_SCH_HDA_DEVC
	   test   ax, INTEL_SCH_HDA_DEVC_NOSNOOP
	   jz	  @f
	   push   ebx
	   mov	  ebx, INTEL_SCH_HDA_DEVC_NOSNOOP
	   xor	  ebx, -1
	   and	  eax, ebx
	   pop	  ebx
	   stdcall  PciWrite16,  [ctrl.bus], [ctrl.devfn], dword INTEL_SCH_HDA_DEVC, eax
	   stdcall  PciRead16,	[ctrl.bus], [ctrl.devfn], dword INTEL_SCH_HDA_DEVC

       if DEBUG
	   push  esi
	   mov	 esi, msgHDASnoopDisabled
	   call  SysMsgBoardStr
	   mov	 esi, msg_OK
	   test   ax, INTEL_SCH_HDA_DEVC_NOSNOOP
	   jz	 .snoop_ok
	   mov	 esi, msg_Fail
	 .snoop_ok:
	   call  SysMsgBoardStr
	   pop	 esi
       end if
  @@:
  .done:
	   pop	 eax
	   ret
endp


; reset stream
proc  azx_stream_reset
	   push  eax ebx ecx edx

	   call  azx_stream_clear

	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readb
	   or	  al, SD_CTL_STREAM_RESET
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_writeb

	   mov	 eax, 3
	   call  StallExec

	   mov	 ecx, 300
  .l1:
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readb
	   test  al, SD_CTL_STREAM_RESET
	   jnz	 @f
	   dec	 ecx
	   jnz	 .l1
  @@:
	   mov	 bl, SD_CTL_STREAM_RESET
	   xor	 bl, -1
	   and	 al, bl
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_writeb

	   mov	 eax, 3
	   call  StallExec

	   mov	 ecx, 300
	   ; waiting for hardware to report that the stream is out of reset
  .l2:
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readb
	   test  al, SD_CTL_STREAM_RESET
	   jnz	 @f
	   dec	 ecx
	   jnz	 .l2
  @@:
	   ; reset first position - may not be synced with hw at this time
	   mov	 edx, [ctrl.posbuf]
	   mov	 dword [edx], 0
	   pop	 edx ecx ebx eax
	   ret
endp


; set up the SD for streaming
proc azx_setup_controller
	   push  eax ebx ecx edx
	   ; make sure the run bit is zero for SD
	   call  azx_stream_clear

	   ; program the stream_tag
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readl
	   mov	 ecx, SD_CTL_STREAM_TAG_MASK
	   xor	 ecx, -1
	   and	 eax, ecx
	   mov	 ecx, SDO_TAG
	   shl	 ecx, SD_CTL_STREAM_TAG_SHIFT
	   or	 eax, ecx
	   ; Asper stream_tag = SDO_TAG
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_writel

	   ; program the length of samples in cyclic buffer
	   mov	 eax, 0x4000*32
	   mov	 edx, ICH6_REG_SD_CBL + SDO_OFS
	   call  azx_writel

	   ; program the stream format
	   ; this value needs to be the same as the one programmed
	   mov	 ax, 0x11
	   mov	 edx, ICH6_REG_SD_FORMAT + SDO_OFS
	   call  azx_writew

	   ; program the stream LVI (last valid index) of the BDL
	   mov	 eax, 32-1 ;4-1 ;2-1
	   mov	 [ctrl.lvi_reg], eax
	   mov	 edx, ICH6_REG_SD_LVI + SDO_OFS
	   call  azx_writew

	   ; program the BDL address
	   ; lower BDL address
	   mov	 eax, [pcmout_bdl]
	   mov	 ebx, eax
	   call  GetPgAddr
	   and	 ebx, 0xFFF
	   add	 eax, ebx
	   mov	 edx, ICH6_REG_SD_BDLPL + SDO_OFS
	   call  azx_writel
	   ; upper BDL address
	   xor	 eax, eax	;upper_32bit(azx_dev->bdl_addr)
	   mov	 edx, ICH6_REG_SD_BDLPU + SDO_OFS
	   call  azx_writel

	   ; enable the position buffer
	   cmp	 [ctrl.position_fix], POS_FIX_LPIB
	   jz	 @f
	   mov	 edx, ICH6_REG_DPLBASE
	   call  azx_readl
	   and	 eax, ICH6_DPLBASE_ENABLE
	   jnz	 @f
	   mov	 eax, dword [ctrl.posbuf]
	   mov	 ebx, eax
	   call  GetPgAddr
	   and	 ebx, 0xFFF
	   add	 eax, ebx
	   or	 eax, ICH6_DPLBASE_ENABLE
	   mov	 edx, ICH6_REG_DPLBASE
	   call  azx_writel
  @@:

	   ; set the interrupt enable bits in the descriptor control register
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_readl
	   or	 eax, SD_INT_MASK
	   mov	 edx, ICH6_REG_SD_CTL + SDO_OFS
	   call  azx_writel

	   pop	 edx ecx ebx eax
	   ret
endp


;(...)

; Probe the given codec address
proc probe_codec, addr:dword
	   push    edx
	   mov	   eax, [addr]
	   shl	   eax, 28
	   mov	   edx, (AC_NODE_ROOT shl 20) or (AC_VERB_PARAMETERS shl 8) or AC_PAR_VENDOR_ID
	   or	   eax, edx
	   stdcall azx_send_cmd, eax
	   stdcall azx_get_response

	   cmp	   eax, -1
	   je	   .out
	   mov	   eax, [addr]
	   mov	   [codec.addr], eax ;Asper+
	if DEBUG
	   push    esi
	   mov	   esi, msgCodecOK
	   call    SysMsgBoardStr
	   mov	   esi, msgCAd
	   call    SysMsgBoardStr
	   stdcall  fdword2str, 3
	   call    SysMsgBoardStr
	   pop	   esi
	end if
	   xor	   eax, eax
  .out:
	   pop	   edx
	   ret
endp


proc  azx_bus_reset
	   call azx_stop_chip
	   call azx_init_chip
endp


; Codec initialization
proc  azx_codec_create
	   push    ebx	ecx  edx
	   ;(...)
	; First try to probe all given codec slots
	; Asper: We asume for now that max slots for codecs = 4
	   xor	   ecx, ecx
	   xor	   edx, edx
	   inc	   edx
  .next_slot:
	   test    edx, [ctrl.codec_mask]
	   jz	   @f
	   stdcall probe_codec, ecx
	   test    eax, eax
	   jz	   .init ;@f
	   ; Some BIOSen give you wrong codec addresses that don't exist
	if DEBUG
	   mov	   esi, msgCodecError
	   call    SysMsgBoardStr
	end if
	   mov	    ebx, edx
	   xor	    ebx, -1
	   and	    [ctrl.codec_mask], ebx

	   ; More badly, accessing to a non-existing
	   ; codec often screws up the controller chip,
	   ; and disturbs the further communications.
	   ; Thus if an error occurs during probing,
	   ; better to reset the controller chip to
	   ; get back to the sanity state.
	   ;call azx_bus_reset
  @@:
	   shl	   edx, 1
	   inc	   ecx
;        if USE_FIRST_CODEC
;           cmp     ecx, 1
;        else
	   cmp	   ecx, 3
;        end if
	   jl	   .next_slot
	   mov	   eax, -1
	   jmp	   .out
  .init:
	   stdcall snd_hda_codec_init
	   xor	   eax, eax
  .out:
	   pop	   edx	ecx  ebx
	   ret
endp


proc  azx_codec_configure
	   ;(...)
	   call  snd_hda_codec_configure
	   ret
endp


proc  azx_get_position
	   test  [ctrl.position_fix], POS_FIX_LPIB
	   jz	 @f
	   ; read LPIB
	   mov	 edx, ICH6_REG_SD_LPIB + SDO_OFS
	   call  azx_readl
	   jmp	 .out
  @@:
	   test  [ctrl.position_fix], POS_FIX_VIACOMBO
	   jz	 @f
;           call  azx_get_via_position
;           jmp   .out
  @@:
	   ; use the position buffer
	   push  edx
	   mov	 edx, dword [ctrl.posbuf]
	   mov	 eax, dword [edx]
	   pop	 edx
  .out:
	   cmp	 eax, 0x4000 ; bufsize
	   jl	 @f
	   xor	 eax, eax
  @@:
	   ret
endp


proc  azx_stop_chip
	   push  eax edx

	   ; disable interrupts
	   call  azx_int_disable
	   call  azx_int_clear
	   ; disable CORB/RIRB
	   call  azx_free_cmd_io
	   ; disable position buffer
	   xor	 eax, eax
	   mov	 edx, ICH6_REG_DPLBASE
	   call  azx_writel
	   mov	 edx, ICH6_REG_DPUBASE
	   call  azx_writel

	   pop	 edx eax
	   ret
endp


; in:  eax = volume (-10000 - 0)
align 4
set_master_vol:
	   mov	 ecx, 3
	   call  set_channel_volume
	   ret


; out:  [pvol] = volume (-10000 - 0)
align 4
proc  get_master_vol stdcall, pvol:dword
	   xor	 ecx, ecx
	   call  get_channel_volume
	   mov	 ebx, [pvol]
	   mov	 [ebx], eax
	   xor	 eax, eax
	   ret
endp


; in:   ecx = channel mask (1 - Left; 2 - Right; 3-Both)
;       eax = volume (-10000 - 0)
align 4
set_channel_volume:
	   push  eax ebx ecx edx
	   mov	 ebx, [volume.maxDb]
	   neg	 eax
     if 0;DEBUG ;YAHOO
	   push eax esi
	   mov	esi, msgNewVolume
	   call SysMsgBoardStr
	   stdcall  fdword2str, 2
	   call SysMsgBoardStr

	   mov	esi, msgMaxVolume
	   call SysMsgBoardStr
	   mov	eax, ebx
	   stdcall  fdword2str, 2
	   call SysMsgBoardStr
	   pop	esi eax
     end if
	   test  ebx, ebx
	   jz	 .err_out

	   cmp	 eax, 0
	   jg	 @f
	   xor	 eax, eax
	   jmp	 .set
  @@:
	   cmp	 eax, ebx
	   jl	 .set
	   mov	 eax, ebx
  .set:
	   ;cdq
	   xor	 edx, edx
	   shl	 eax, 2
	   mov	 ebx, 100
	   div	 bx
	   mov	 bl, [volume.step_size]
	   div	 bl

	   mov	 edx, [volume.out_amp_node]
	   test  edx, edx
	   jz	 .out
	   movzx ebx, [edx+HDA_GNODE.nid]

	   test  ecx, 1   ; Left channel ?
	   jz	 @f
	   stdcall put_volume_mute, ebx, 0, HDA_OUTPUT, 0, eax
  @@:
	   test  ecx, 2   ; Right channel ?
	   jz	 .out
	   stdcall put_volume_mute, ebx, 1, HDA_OUTPUT, 0, eax
  .out:
	   pop	 edx ecx ebx eax
	   ret
  .err_out:
       if 0;DEBUG  ;YAHOO
	   push  esi
	   mov	 esi, emsgNoVolCtrl
	   call  SysMsgBoardStr
	   pop	 esi
       end if
	   jmp	 .out

; in:   ecx = channel (1 - Left; 2 - Right)
; out:  eax = volume (-10000 - 0)
align 4
get_channel_volume:
      push    ebx ecx edx
      cmp     ecx, 2
      jg      .out
      dec     cl
      xor     eax, eax
      mov     edx, [volume.out_amp_node]
      test    edx, edx
      jz      .out
      movzx   ebx, [edx+HDA_GNODE.nid]
      stdcall get_volume_mute, ebx, ecx, HDA_OUTPUT, 0
      mov     cl, [volume.step_size]
      mul     cl

      mov     cx, 100
      mul     cx
      shr     eax, 2 ; *0.25
      neg     eax
  .out:
	   pop	   edx ecx ebx
	   ret


; in:  ecx = delay
udelay:
	push	eax ecx edx
	test	ecx, ecx
	jnz	@f
	inc	ecx
  @@:
	mov	eax, ecx
	mov	cx, 500
	mul	cl
	mov	ecx, edx
	shl	ecx, 16
	or	ecx, eax
  @@:
	xor	eax, eax
	cpuid
	dec	ecx
	jz	@b
	pop	edx ecx eax
	ret

align 4
proc StallExec
	push	ecx edx ebx eax

	mov	ecx, CPU_FREQ
	mul	ecx
	mov	ebx, eax       ;low
	mov	ecx, edx       ;high
	rdtsc
	add	ebx, eax
	adc	ecx,edx
  @@:
	rdtsc
	sub	eax, ebx
	sbb	edx, ecx
	js	@B

	pop	eax ebx edx ecx
	ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         MEMORY MAPPED IO    (os depended) ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc azx_readb
	add	edx, [ctrl.ctrl_mem_base]
	mov	al, [edx]
	ret
endp

align 4
proc azx_readw
	add	edx, [ctrl.ctrl_mem_base]
	mov	ax, [edx]
	ret
endp

align 4
proc azx_readl
	add	edx, [ctrl.ctrl_mem_base]
	mov	eax, [edx]
	ret
endp

align 4
proc azx_writeb
	add	edx, [ctrl.ctrl_mem_base]
	mov	[edx], al
	ret
endp

align 4
proc azx_writew
	add	edx, [ctrl.ctrl_mem_base]
	mov	[edx], ax
	ret
endp

align 4
proc azx_writel
	add	edx, [ctrl.ctrl_mem_base]
	mov	[edx], eax
	ret
endp

;_______


;Asper remember to add this functions:
proc  snd_hda_queue_unsol_event stdcall, par1:dword, par2:dword
  if DEBUG
	push	esi
	mov	esi, msgUnsolEvent
	call	SysMsgBoardStr
	pop	esi
  end if
	ret
endp
;...


align 4
proc  fdword2str stdcall, flags:dword	; bit 0 - skipLeadZeroes; bit 1 - newLine; other bits undefined
	push	eax ebx ecx
	mov	esi, hex_buff
	mov	ecx, -8
	push	eax
  @@:
	rol	eax, 4
	mov	ebx, eax
	and	ebx, 0x0F
	mov	bl, [ebx+hexletters]
	mov	[8+esi+ecx], bl
	inc	ecx
	jnz	@b
	pop	eax

	mov	dword [esi+8], 0
	test	[flags], 0x2 ; new line ?
	jz	.no_newline
	mov	dword [esi+8], 0x00000A0D
  .no_newline:

	push	eax
	test	[flags], 0x1 ; skip zero bits ?
	jz	.no_skipz
	mov	ecx, 8
  @@:
	test	eax, 0xF0000000
	jnz	.skipz_done
	rol	eax, 4
	inc	esi
	dec	ecx
	jnz	@b
	dec	esi
  .skipz_done:
  .no_skipz:
	pop	eax

	pop	ecx ebx eax
	ret
endp

hexletters   db '0123456789ABCDEF'
hex_buff     db 8 dup(0),13,10,0,0


include "CODEC.INC"
include "hda_generic.inc"

align 4
devices:
; Intel
	 dd (CTRL_INTEL_SCH2	shl 16)+VID_INTEL,msg_INTEL_SCH2,	    AZX_DRIVER_SCH
	 dd (CTRL_INTEL_HPT	shl 16)+VID_INTEL,msg_INTEL_HPT,	    AZX_DRIVER_SCH
	 dd (CTRL_INTEL_CPT	shl 16)+VID_INTEL,msg_INTEL_CPT,	    AZX_DRIVER_PCH
	 dd (CTRL_INTEL_PGB	shl 16)+VID_INTEL,msg_INTEL_PGB,	    AZX_DRIVER_PCH
	 dd (CTRL_INTEL_PPT1	shl 16)+VID_INTEL,msg_INTEL_PPT1,	    AZX_DRIVER_PCH
	 dd (CTRL_INTEL_82801F	shl 16)+VID_INTEL,msg_INTEL_82801F,	    AZX_DRIVER_ICH
	 dd (CTRL_INTEL_63XXESB shl 16)+VID_INTEL,msg_INTEL_63XXESB,	    AZX_DRIVER_ICH
	 dd (CTRL_INTEL_82801G	shl 16)+VID_INTEL,msg_INTEL_82801G,	    AZX_DRIVER_ICH
	 dd (CTRL_INTEL_82801H	shl 16)+VID_INTEL,msg_INTEL_82801H,	    AZX_DRIVER_ICH
	 dd (CTRL_INTEL_82801_UNK1  shl 16)+VID_INTEL,msg_INTEL_82801_UNK1, AZX_DRIVER_ICH
	 dd (CTRL_INTEL_82801I	shl 16)+VID_INTEL,msg_INTEL_82801I,	    AZX_DRIVER_ICH
	 dd (CTRL_INTEL_82801_UNK2  shl 16)+VID_INTEL,msg_INTEL_82801_UNK2, AZX_DRIVER_ICH
	 dd (CTRL_INTEL_82801JI shl 16)+VID_INTEL,msg_INTEL_82801JI,	    AZX_DRIVER_ICH
	 dd (CTRL_INTEL_82801JD shl 16)+VID_INTEL,msg_INTEL_82801JD,	    AZX_DRIVER_ICH
	 dd (CTRL_INTEL_PCH	shl 16)+VID_INTEL,msg_INTEL_PCH,	    AZX_DRIVER_PCH
	 dd (CTRL_INTEL_PCH2	shl 16)+VID_INTEL,msg_INTEL_PCH2,	    AZX_DRIVER_PCH
	 dd (CTRL_INTEL_SCH	shl 16)+VID_INTEL,msg_INTEL_SCH,	    AZX_DRIVER_SCH
	 dd (CTRL_INTEL_LPT	shl 16)+VID_INTEL,msg_INTEL_LPT,	    AZX_DRIVER_PCH
; Nvidia
	 dd (CTRL_NVIDIA_MCP51	  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP51,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP55	  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP55,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP61_1  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP61,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP61_2  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP61,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP65_1  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP65,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP65_2  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP65,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP67_1  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP67,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP67_2  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP67,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP73_1  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP73,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP73_2  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP73,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP78_1  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP78,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP78_2  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP78,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP78_3  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP78,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP78_4  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP78,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP79_1  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP79,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP79_2  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP79,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP79_3  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP79,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP79_4  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP79,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_0BE2	  shl 16)+VID_NVIDIA,msg_NVIDIA_0BE2,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_0BE3	  shl 16)+VID_NVIDIA,msg_NVIDIA_0BE3,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_0BE4	  shl 16)+VID_NVIDIA,msg_NVIDIA_0BE4,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GT100	  shl 16)+VID_NVIDIA,msg_NVIDIA_GT100,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GT106	  shl 16)+VID_NVIDIA,msg_NVIDIA_GT106,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GT108	  shl 16)+VID_NVIDIA,msg_NVIDIA_GT108,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GT104	  shl 16)+VID_NVIDIA,msg_NVIDIA_GT104,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GT116	  shl 16)+VID_NVIDIA,msg_NVIDIA_GT116,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP89_1  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP89,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP89_2  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP89,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP89_3  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP89,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_MCP89_4  shl 16)+VID_NVIDIA,msg_NVIDIA_MCP89,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GF119	  shl 16)+VID_NVIDIA,msg_NVIDIA_GF119,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GF110_1  shl 16)+VID_NVIDIA,msg_NVIDIA_GF110,	    AZX_DRIVER_NVIDIA
	 dd (CTRL_NVIDIA_GF110_2  shl 16)+VID_NVIDIA,msg_NVIDIA_GF110,	    AZX_DRIVER_NVIDIA
; ATI
	 dd (CTRL_ATI_SB450  shl 16)+VID_ATI,msg_ATI_SB450,		    AZX_DRIVER_ATI
	 dd (CTRL_ATI_SB600  shl 16)+VID_ATI,msg_ATI_SB600,		    AZX_DRIVER_ATI
	 dd (CTRL_ATI_RS600  shl 16)+VID_ATI,msg_ATI_RS600,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RS690  shl 16)+VID_ATI,msg_ATI_RS690,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RS780  shl 16)+VID_ATI,msg_ATI_RS780,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RS_UNK1  shl 16)+VID_ATI,msg_ATI_RS_UNK1, 	    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_R600   shl 16)+VID_ATI,msg_ATI_R600,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV610  shl 16)+VID_ATI,msg_ATI_RV610,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV620  shl 16)+VID_ATI,msg_ATI_RV620,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV630  shl 16)+VID_ATI,msg_ATI_RV630,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV635  shl 16)+VID_ATI,msg_ATI_RV635,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV670  shl 16)+VID_ATI,msg_ATI_RV670,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV710  shl 16)+VID_ATI,msg_ATI_RV710,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV730  shl 16)+VID_ATI,msg_ATI_RV730,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV740  shl 16)+VID_ATI,msg_ATI_RV740,		    AZX_DRIVER_ATIHDMI
	 dd (CTRL_ATI_RV770  shl 16)+VID_ATI,msg_ATI_RV770,		    AZX_DRIVER_ATIHDMI
; AMD
	 dd (CTRL_AMD_HUDSON shl 16)+VID_AMD,msg_AMD_HUDSON,		    AZX_DRIVER_GENERIC
; VIA
	 dd (CTRL_VIA_VT82XX shl 16)+VID_VIA,msg_VIA_VT82XX,		    AZX_DRIVER_VIA
	 dd (CTRL_VIA_VT61XX shl 16)+VID_VIA,msg_VIA_VT61XX,		    AZX_DRIVER_GENERIC
	 dd (CTRL_VIA_VT71XX shl 16)+VID_VIA,msg_VIA_VT71XX,		    AZX_DRIVER_GENERIC
; SiS
	 dd (CTRL_SIS_966    shl 16)+VID_SIS,msg_SIS_966,		    AZX_DRIVER_SIS
; ULI
	 dd (CTRL_ULI_M5461  shl 16)+VID_ULI,msg_ULI_M5461,		    AZX_DRIVER_ULI
; Teradici
	 dd (CTRL_TERA_UNK1  shl 16)+VID_ULI,msg_TERA_UNK1,		    AZX_DRIVER_TERA
; Creative
	 dd (CTRL_CREATIVE_CA0110_IBG	  shl 16)+VID_CREATIVE,msg_CREATIVE_CA0110_IBG,   AZX_DRIVER_CTX
	 dd (CTRL_CREATIVE_SOUND_CORE3D_1 shl 16)+VID_CREATIVE,msg_CREATIVE_SOUND_CORE3D, AZX_DRIVER_GENERIC
	 dd (CTRL_CREATIVE_SOUND_CORE3D_2 shl 16)+VID_CREATIVE,msg_CREATIVE_SOUND_CORE3D, AZX_DRIVER_GENERIC
; RDC Semiconductor
	 dd (CTRL_RDC_R3010  shl 16)+VID_RDC,msg_RDC_R3010,		    AZX_DRIVER_GENERIC
; VMware
	 dd (CTRL_VMWARE_UNK1  shl 16)+VID_VMWARE,msg_VMWARE_UNK1,	    AZX_DRIVER_GENERIC

	 dd 0	 ;terminator


version      dd (5 shl 16) or (API_VERSION and 0xFFFF)

msg_Intel		db 'Intel ',0
msg_INTEL_CPT		db 'Cougar Point',13,10,0
msg_INTEL_PGB		db 'Patsburg',13,10,0
msg_INTEL_PPT1		db 'Panther Point',13,10,0
msg_INTEL_LPT		db 'Lynx Point',13,10,0
msg_INTEL_HPT		db 'Haswell',13,10,0
msg_INTEL_82801F	db '82801F',13,10,0
msg_INTEL_63XXESB	db '631x/632xESB',13,10,0
msg_INTEL_82801G	db '82801G', 13,10,0
msg_INTEL_82801H	db '82801H', 13,10,0
msg_INTEL_82801I	db '82801I', 13,10,0
msg_INTEL_82801JI	db '82801JI',13,10,0
msg_INTEL_82801JD	db '82801JD',13,10,0
msg_INTEL_PCH		db 'PCH',13,10,0
msg_INTEL_PCH2		db 'PCH2',13,10,0
msg_INTEL_SCH		db 'Poulsbo',13,10,0
msg_INTEL_SCH2		db 'Oaktrail',13,10,0
msg_INTEL_82801_UNK1	db '82801_UNK1',  13,10,0
msg_INTEL_82801_UNK2	db '82801_UNK2',  13,10,0

msg_NVidia		db 'NVidia ',0
msg_NVIDIA_MCP51	db 'MCP51',	 13,10,0
msg_NVIDIA_MCP55	db 'MCP55',	 13,10,0
msg_NVIDIA_MCP61	db 'MCP61',	 13,10,0
msg_NVIDIA_MCP65	db 'MCP65',	 13,10,0
msg_NVIDIA_MCP67	db 'MCP67',	 13,10,0
msg_NVIDIA_MCP73	db 'MCP73',	 13,10,0
msg_NVIDIA_MCP78	db 'MCP78',	 13,10,0
msg_NVIDIA_MCP79	db 'MCP79',	 13,10,0
msg_NVIDIA_MCP89	db 'MCP89',	 13,10,0
msg_NVIDIA_0BE2 	db '(0x0be2)',	 13,10,0
msg_NVIDIA_0BE3 	db '(0x0be3)',	 13,10,0
msg_NVIDIA_0BE4 	db '(0x0be4)',	 13,10,0
msg_NVIDIA_GT100	db 'GT100',	 13,10,0
msg_NVIDIA_GT104	db 'GT104',	 13,10,0
msg_NVIDIA_GT106	db 'GT106',	 13,10,0
msg_NVIDIA_GT108	db 'GT108',	 13,10,0
msg_NVIDIA_GT116	db 'GT116',	 13,10,0
msg_NVIDIA_GF119	db 'GF119',	 13,10,0
msg_NVIDIA_GF110	db 'GF110',	 13,10,0

msg_ATI 	     db 'ATI ',0
msg_ATI_SB450	     db 'SB450',      13,10,0
msg_ATI_SB600	     db 'SB600',      13,10,0

msg_ATI_HDMI	     db 'ATI HDMI ',0
msg_ATI_RS600	     db 'RS600',      13,10,0
msg_ATI_RS690	     db 'RS690',      13,10,0
msg_ATI_RS780	     db 'RS780',      13,10,0
msg_ATI_RS_UNK1      db 'RS_UNK1',    13,10,0
msg_ATI_R600	     db 'R600',       13,10,0
msg_ATI_RV610	     db 'RV610',      13,10,0
msg_ATI_RV620	     db 'RV620',      13,10,0
msg_ATI_RV630	     db 'RV630',      13,10,0
msg_ATI_RV635	     db 'RV635',      13,10,0
msg_ATI_RV670	     db 'RV670',      13,10,0
msg_ATI_RV710	     db 'RV710',      13,10,0
msg_ATI_RV730	     db 'RV730',      13,10,0
msg_ATI_RV740	     db 'RV740',      13,10,0
msg_ATI_RV770	     db 'RV770',      13,10,0

msg_AMD 	     db 'AMD ',0
msg_AMD_HUDSON	     db 'Hudson',     13,10,0

msg_VIA 	     db 'VIA ',0
msg_VIA_VT82XX	     db 'VT8251/8237A',     13,10,0
msg_VIA_VT61XX	     db 'GFX VT6122/VX11',  13,10,0
msg_VIA_VT71XX	     db 'GFX VT7122/VX900', 13,10,0

msg_SIS 	     db 'SIS ',0
msg_SIS_966	     db '966',	    13,10,0

msg_ULI 	     db 'ULI ',0
msg_ULI_M5461	     db 'M5461',      13,10,0

msg_TERA	     db 'Teradici ',0
msg_TERA_UNK1	     db 'UNK1',      13,10,0

msg_CREATIVE		      db 'Creative ',0
msg_CREATIVE_CA0110_IBG       db 'CA0110-IBG',13,10,0 ;SB X-Fi Xtreme Audio
msg_CREATIVE_SOUND_CORE3D     db 'Sound Core3D'

msg_RDC 	     db 'RDC ',0
msg_RDC_R3010	     db 'R3010', 13,10,0

msg_VMWARE	     db 'VMware ',0
msg_VMWARE_UNK1      db 'UNK1', 13,10,0

szKernel	     db 'KERNEL',0
sz_sound_srv	     db 'SOUND',0

msgInit      db 'detect hardware...',13,10,0
msgFail      db 'device not found',13,10,0
msgAttchIRQ  db 'IRQ line not supported', 13,10,0
msgInvIRQ    db 'IRQ line not assigned or invalid', 13,10,0
msgPlay      db 'start play', 13,10,0
msgStop      db 'stop play',  13,10,0
msgSetChannelVolume  db 'Set Channel Volume', 13,10,0
msgIRQ	     db 'HDA IRQ', 13,10,0
msgInitCtrl  db 'init controller',13,10,0
msgPrimBuff  db 'create primary buffer ...',0
msgDone      db 'done',13,10,0
msgRemap     db 'Remap IRQ',13,10,0
msgOk	     db 'service installed',13,10,0
msgCold      db 'cold reset',13,10,0
    msgHDARFail    db 'controller not ready',13,10,0
msgCFail     db 'codec not ready',13,10,0
msgResetOk   db 'reset complete',13,10,0
msgPciCmd    db 'PCI command     ',0
msgPciStat   db 'PCI status      ',0
    msgHDALowMMIo db 'lower mmio base ',0
    msgHDAUpMMIo  db 'upper mmio base ',0
msgIrqMap    db 'HDA irq map as      ',0

;Asper [
if DEBUG
    msgCodecMask	     db 'codec_mask = ',0
    msgNoCodecsFound	     db 'no codecs found!',13,10,0
    msgHDASnoopDisabled      db 'HDA snoop disabled, enabling ... ',0
    msg_OK		     db 'OK',13,10,0
    msg_Fail		     db 'Failed',13,10,0
    msgSpuriousResponce      db 'spurious responce ',0
    emsgInvalidAFGSubtree    db 'Invalid AFG subtree',13,10,0
    emsgConnListNotAvailable db 'connection list not available for ',0
    msgUnmuteOut	     db 'UNMUTE OUT: NID=',0
    msgUnmuteIn 	     db 'UNMUTE IN: NID=',0
    msgGetResponceTimeout    db 'get_response timeout: ',0
    msgVal		     db ' val=',0
    emsgBusResetFatalComm    db 'resetting BUS due to fatal communication error',13,10,0
    msgCodecOK		     db 'codec probed OK',13,10,0
    msgCodecError	     db 'codec probe error disabling it...',13,10,0
    emsgNoAFGorMFGFound      db 'no AFG or MFG node found',13,10,0
    emsgNoMem		     db 'hda_codec: cannot malloc',13,10,0
    msgConnect		     db 'CONNECT: NID=',0
    msgIdx		     db ' IDX=',0
    msgSkipDigitalOutNode    db 'Skip Digital OUT node ',0
    msgAudOutFound	     db 'AUD_OUT found ',0
    emsgNoParserAvailable    db 'No codec parser is available',13,10,0
    emsgNoProperOutputPathFound  db 'hda_generic: no proper output path found',13,10,0
    emsgInvConnList	     db 'hda_codec: invalid CONNECT_LIST verb ',0
    emsgInvDepRangeVal	     db 'hda_codec: invalid dep_range_val ',0
    emsgTooManyConns	     db 'Too many connections',13,10,0
	emsgNoVolCtrl	     db 'No volume control',13,10,0
    msgHDACodecSetupStream   db 'hda_codec_setup_stream: NID=',0
    msgStream		     db 'stream=',0
    msgChannel		     db 'channel=',0
    msgFormat		     db 'format=',0

    msgPollingCodecOnce      db 'polling the codec once',13,10,0 ;Asper~
    msgSwitchToPollMode      db 'switching to polling mode',13,10,0 ;Asper~

    msgUnsolEvent	     db 'Unsolicited event!',13,10,0
    strSemicolon	     db ':',0
    msgSETUP_FG_NODES	     db 'Setup FG nodes = start_nid:total_nodes = ',0
    msgFG_TYPE		     db 'FG type = ',0
    msgPinCfgs		     db 'Pin configurations:',13,10,0
    msgWCaps		     db 'Widget capabilities:',13,10,0
    msgCAd		     db 'CAd = ',0
    msgTCSEL		     db 'PCI TCSEL     ',0
    msgTV		     db 'HDA test version ',TEST_VERSION_NUMBER,13,10,0
    msgGCap		     db 'GCAP = ',0
end if

if USE_SINGLE_MODE
    msgSingleMode	     db 'Single mode !',13,10,0
    msgIRS		     db 'IRS=',0
    msgSendCmdTimeout	     db 'send_cmd timeout: IRS=',0
else
    msgNormalMode	     db 'Normal mode !',13,10,0
end if

if DEBUG
    msgYAHOO2		     db 'YAHOO2: ',0
    msgMaxVolume	     db 'MaxVolume: ',0
    msgNewVolume	     db 'NewVolume: ',0

    msgVerbQuery	     db 'Q: ',0
    msgVerbAnswer	     db 'A: ',0
    msgPin_Nid		     db 'Pin Nid = ',0
    msgPin_Ctl		     db 'Pin Control = ',0
    msgPin_Caps 	     db 'Pin Capabilities = ',0
    msgDef_Cfg		     db 'Pin def_cfg = ',0
    msgAmp_Out_Caps	     db 'Pin Amp Out caps = ',0
    msgAmpVal		     db 'Amp val = ',0
    msgEnableEAPD	     db 'Enable EAPD: NID=',0
    msgBeeperNid	     db 'Beeper found: NID=',0
    msgBeeperValue	     db 'Beeper initial value: ',0
    msgBeepNow		     db 'Beep!',13,10,0
end if

;] Asper


section '.data' data readable writable align 16

aspinlock	 dd SPINLOCK_FREE

codec CODEC
ctrl AC_CNTRL

;Asper: BDL must be aligned to 128 according to HDA specification.
pcmout_bdl	 rd 1
buff_list	 rd 32

driver_type	 rd 1
