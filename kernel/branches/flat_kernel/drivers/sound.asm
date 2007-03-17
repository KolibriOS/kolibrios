
format MS COFF


include 'proc32.inc'
include 'imports.inc'

DEBUG	    equ 1

REMAP_IRQ   equ 0

;irq 0,1,2,8,12,13 недоступны
;               FEDCBA9876543210
VALID_IRQ   equ 1100111011111000b
ATTCH_IRQ   equ 0000111010101000b

IRQ_LINE    equ 0

CPU_FREQ    equ  2600d

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

VID_INTEL        equ 0x8086
VID_NVIDIA       equ 0x10DE

CTRL_ICH         equ 0x2415
CTRL_ICH0        equ 0x2425
CTRL_ICH2        equ 0x2435
CTRL_ICH3        equ 0x2445
CTRL_ICH4        equ 0x24C5
CTRL_ICH5        equ 0x24D5
CTRL_ICH6        equ 0x266E
CTRL_ICH7        equ 0x27DE

CTRL_NFORCE      equ 0x01B1
CTRL_NFORCE2     equ 0x006A
CTRL_NFORCE3     equ 0x00DA
CTRL_MCP04       equ 0x003A
CTRL_CK804       equ 0x0059
CTRL_CK8         equ 0x008A
CTRL_CK8S        equ 0x00EA
CTRL_MCP51       equ 0x026B


PCM_OUT_BDL      equ  0x10  ; PCM out buffer descriptors list
PCM_OUT_CR_REG	  equ  0x1b	 ; PCM out Control Register
PCM_OUT_LVI_REG  equ  0x15  ; PCM last valid index
PCM_OUT_SR_REG	  equ  0x16	 ; PCM out Status register
PCM_OUT_PIV_REG  equ  0x1a
PCM_OUT_CIV_REG  equ  0x14  ; PCM out current index

PCM_IN_CR_REG    equ  0x0b  ; PCM in Control Register
MC_IN_CR_REG     equ  0x2b  ; MIC in Control Register
RR equ  BIT1  ; reset registers.  Nukes all regs

CODEC_MASTER_VOL_REG		equ	0x02
CODEC_AUX_VOL			equ	0x04	;
CODEC_PCM_OUT_REG		equ	18h	; PCM output volume
CODEC_EXT_AUDIO_REG		equ	28h	; extended audio
CODEC_EXT_AUDIO_CTRL_REG	equ	2ah	; extended audio control
CODEC_PCM_FRONT_DACRATE_REG	equ	2ch	; PCM out sample rate
CODEC_PCM_SURND_DACRATE_REG	equ	2eh	; surround sound sample rate
CODEC_PCM_LFE_DACRATE_REG	equ	30h	; LFE sample rate

GLOB_CTRL        equ  0x2C ;   Global Control
CTRL_STAT        equ  0x30  ;   Global Status
CTRL_CAS         equ  0x34  ;   Codec Access Semiphore

CAS_FLAG         equ  0x01  ;   Codec Access Semiphore Bit

CTRL_ST_CREADY   equ  BIT8+BIT9+BIT28 ;   Primary Codec Ready

CTRL_ST_RCS      equ  0x00008000  ;   Read Completion Status

CTRL_CNT_CRIE    equ  BIT4+BIT5+BIT6  ;   Codecs Resume Interrupt Enable
CTRL_CNT_AC_OFF  equ  0x00000008  ;   ACLINK Off
CTRL_CNT_WARM    equ  0x00000004  ;   AC97 Warm Reset
CTRL_CNT_COLD    equ  0x00000002  ;   AC97 Cold Reset
CTRL_CNT_GIE     equ  0x00000001  ;   GPI Interrupt Enable

CODEC_REG_POWERDOWN   equ 0x26
CODEC_REG_ST          equ 0x26

DEV_PLAY              equ  1
DEV_STOP              equ  2
DEV_CALLBACK          equ  3
DEV_SET_BUFF          equ  4
DEV_NOTIFY            equ  5
DEV_SET_MASTERVOL     equ  6
DEV_GET_MASTERVOL     equ  7
DEV_GET_INFO          equ  8

struc AC_CNTRL		    ;AC controller base class
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

struc CODEC		   ;Audio Chip base class
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
{   .pci_cmd         dd ?
    .irq             dd ?
    .glob_cntrl      dd ?
    .glob_sta        dd ?
    .codec_io_base   dd ?
    .ctrl_io_base    dd ?
    .codec_mem_base  dd ?
    .ctrl_mem_base   dd ?
    .codec_id        dd ?
}

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

EVENT_NOTIFY    equ 0x00000200

OS_BASE         equ 0x80000000
SLOT_BASE       equ OS_BASE+0x0080000

public START
public service_proc
public version

section '.flat' code readable align 16

proc START stdcall, state:dword

           cmp [state], 1
           jne .stop

     if DEBUG
           mov esi, msgInit
           call SysMsgBoardStr
     end if

           call detect_controller
           test eax, eax
           jz .fail

     if DEBUG
           mov esi,[ctrl.vendor_ids]
           call SysMsgBoardStr
           mov esi, [ctrl.ctrl_ids]
           call SysMsgBoardStr

     end if

           call init_controller
           test eax, eax
           jz .fail

     if DEBUG
           mov esi, msgInitCodec
           call SysMsgBoardStr
     end if

           call init_codec
           test eax, eax
           jz .fail

     if DEBUG
           mov esi, [codec.ac_vendor_ids]
           call SysMsgBoardStr

           mov esi, [codec.chip_ids]
           call SysMsgBoardStr
     end if

           call reset_controller
           call setup_codec

           mov esi, msgPrimBuff
           call SysMsgBoardStr

           call create_primary_buff

;     if REMAP_IRQ

;           call get_LPC_bus
;           cmp eax, -1
;           jz .fail

;           mov [lpc_bus], 0  ;eax
;           call remap_irq
;     end if

           mov eax, VALID_IRQ
           mov ebx, [ctrl.int_line]
           mov esi, msgInvIRQ
           bt eax, ebx
           jnc .fail
           mov eax, ATTCH_IRQ
           mov esi, msgAttchIRQ
           bt eax, ebx
           jnc .fail

           stdcall AttachIntHandler, ebx, ac97_irq
.reg:
           stdcall RegService, sz_sound_srv, service_proc
           ret
.fail:
     if DEBUG
           mov esi, msgFail
           call SysMsgBoardStr
     end if
           xor eax, eax
           ret
.stop:
           call stop
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
           cmp eax, DEV_PLAY
           jne @F
     if DEBUG
           mov esi, msgPlay
           call SysMsgBoardStr
     end if
           call play
           ret
@@:
           cmp eax, DEV_STOP
           jne @F
     if DEBUG
           mov esi, msgStop
           call SysMsgBoardStr
     end if
           call stop
           ret
@@:
           cmp eax, DEV_CALLBACK
           jne @F
           mov ebx, [edi+input]
           stdcall set_callback, [ebx]
           ret
@@:
           cmp eax, DEV_SET_MASTERVOL
           jne @F
           mov eax, [edi+input]
           mov eax, [eax]
           call set_master_vol      ;eax= vol
           ret
@@:
           cmp eax, DEV_GET_MASTERVOL
           jne @F
           mov ebx, [edi+output]
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
proc remap_irq                         ;for Intel chipsets ONLY !!!
           mov eax, VALID_IRQ
           bt eax, IRQ_LINE
           jnc .exit

           mov edx, 0x4D0
           in ax,dx
           bts ax, IRQ_LINE
           out dx, aX

           stdcall PciWrite8, dword 0, dword 0xF8, dword 0x61, dword IRQ_LINE
           mov [ctrl.int_line], IRQ_LINE

.exit:
	   ret
endp

align 4
proc ac97_irq

;     if DEBUG
;           mov esi, msgIRQ
;           call SysMsgBoardStr
;     end if

           mov edx, PCM_OUT_CR_REG
           mov al, 0x10;               0x10
           call [ctrl.ctrl_write8]

           mov ax, 0x1c
           mov edx, PCM_OUT_SR_REG
           call [ctrl.ctrl_write16]

           mov edx, PCM_OUT_CIV_REG
           call [ctrl.ctrl_read8]

           and eax, 0x1F
           cmp eax, [civ_val]
           je .skip

           mov [civ_val], eax
           dec eax
           and eax, 0x1F
           mov [ctrl.lvi_reg], eax

           mov edx, PCM_OUT_LVI_REG
           call [ctrl.ctrl_write8]

           mov edx, PCM_OUT_CR_REG
           mov ax, 0x11              ;0x1D
           call [ctrl.ctrl_write8]

           mov eax, [civ_val]
           add eax, 1
           and eax, 31
           mov ebx, dword [buff_list+eax*4]

           cmp [ctrl.user_callback], 0
           je @f

           stdcall [ctrl.user_callback], ebx
@@:
           ret

.skip:
           mov edx, PCM_OUT_CR_REG
           mov ax, 0x11               ;0x1D
           call [ctrl.ctrl_write8]
           ret
endp

align 4
proc create_primary_buff

           stdcall KernelAlloc, 0x10000
           mov [ctrl.buffer], eax

           mov edi, eax
           mov ecx, 0x10000/4
           xor eax, eax
           cld
           rep stosd

           mov eax, [ctrl.buffer]
           call GetPgAddr

           mov ebx, 0xC0002000
           mov ecx, 4
           mov edi, pcmout_bdl
@@:
           mov [edi], eax
           mov [edi+4], ebx

           mov [edi+32], eax
           mov [edi+4+32], ebx

           mov [edi+64], eax
           mov [edi+4+64], ebx

           mov [edi+96], eax
           mov [edi+4+96], ebx

           mov [edi+128], eax
           mov [edi+4+128], ebx

           mov [edi+160], eax
           mov [edi+4+160], ebx

           mov [edi+192], eax
           mov [edi+4+192], ebx

           mov [edi+224], eax
           mov [edi+4+224], ebx

           add eax, 0x4000
           add edi, 8
           loop @B

           mov edi, buff_list
           mov eax, [ctrl.buffer]
           mov ecx, 4
@@:
           mov [edi], eax
           mov [edi+16], eax
           mov [edi+32], eax
           mov [edi+48], eax
           mov [edi+64], eax
           mov [edi+80], eax
           mov [edi+96], eax
           mov [edi+112], eax

           add eax, 0x4000
           add edi, 4
           loop @B

           mov eax, pcmout_bdl
           mov ebx, eax
           call GetPgAddr     ;eax
           and ebx, 0xFFF
           add eax, ebx

           mov edx, PCM_OUT_BDL
           call [ctrl.ctrl_write32]

           mov eax, 16
           mov [ctrl.lvi_reg], eax
           mov edx, PCM_OUT_LVI_REG
           call [ctrl.ctrl_write8]
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

           cmp ebx, VID_INTEL
           jne @F
           mov [ctrl.vendor_ids], msg_Intel
           ret
@@:
           cmp ebx, VID_NVIDIA
           jne @F
           mov [ctrl.vendor_ids], msg_NVidia
@@:
           mov [ctrl.vendor_ids], 0     ;something  wrong ?
           ret
.err:
           xor eax, eax
           ret
endp

align 4
proc get_LPC_bus                ;for Intel chipsets ONLY !!!
           locals
             last_bus dd ?
             bus      dd ?
           endl

           xor eax, eax
           mov [bus], eax
           inc eax
           call [PciApi]
           cmp eax, -1
           je .err

           mov [last_bus], eax
.next_bus:
           stdcall PciRead32, [bus], dword 0xF8, dword 0
           test eax, eax
           jz .next
           cmp eax, -1
           je .next

           cmp eax, 0x24D08086
           je .found
.next:
           mov eax, [bus]
           inc eax
           cmp eax, [last_bus]
           mov [bus], eax
           jna .next_bus
.err:
           xor eax, eax
           dec eax
           ret
.found:
           mov eax, [bus]
           ret
endp

align 4
proc init_controller

           stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 4
           mov ebx, eax
           and eax, 0xFFFF
           mov [ctrl.pci_cmd], eax
           shr ebx, 16
           mov [ctrl.pci_stat], ebx

           stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x10
           and eax,0xFFFE
           mov [ctrl.codec_io_base], eax

           stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x14
           and eax, 0xFFC0
           mov [ctrl.ctrl_io_base], eax

           stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x18
           mov [ctrl.codec_mem_base], eax

           stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x1C
           mov [ctrl.ctrl_mem_base], eax

           stdcall PciRead32, [ctrl.bus], [ctrl.devfn], dword 0x3C
           and eax, 0xFF
           mov [ctrl.int_line], eax

           stdcall PciRead8, [ctrl.bus], [ctrl.devfn], dword 0x41
           and eax, 0xFF
           mov [ctrl.cfg_reg], eax

           call [ctrl.ctrl_setup]
           xor eax, eax
           inc eax
           ret
endp

align 4
proc set_ICH
           mov [ctrl.codec_read16],  codec_io_r16    ;virtual
           mov [ctrl.codec_write16], codec_io_w16    ;virtual

           mov [ctrl.ctrl_read8 ],  ctrl_io_r8      ;virtual
           mov [ctrl.ctrl_read16],  ctrl_io_r16      ;virtual
           mov [ctrl.ctrl_read32],  ctrl_io_r32      ;virtual

           mov [ctrl.ctrl_write8 ], ctrl_io_w8      ;virtual
           mov [ctrl.ctrl_write16], ctrl_io_w16      ;virtual
           mov [ctrl.ctrl_write32], ctrl_io_w32      ;virtual
           ret
endp

PG_SW            equ 0x003
PG_NOCACHE       equ 0x018

align 4
proc set_ICH4
           stdcall AllocKernelSpace, dword 0x2000
           mov edi, eax
           stdcall MapPage, edi,[ctrl.codec_mem_base],PG_SW+PG_NOCACHE
           mov [ctrl.codec_mem_base], edi
           add edi, 0x1000
           stdcall MapPage, edi, [ctrl.ctrl_mem_base],PG_SW+PG_NOCACHE
           mov [ctrl.ctrl_mem_base], edi

           mov [ctrl.codec_read16],  codec_mem_r16    ;virtual
           mov [ctrl.codec_write16], codec_mem_w16    ;virtual

           mov [ctrl.ctrl_read8 ],  ctrl_mem_r8       ;virtual
           mov [ctrl.ctrl_read16],  ctrl_mem_r16      ;virtual
           mov [ctrl.ctrl_read32],  ctrl_mem_r32      ;virtual

           mov [ctrl.ctrl_write8 ], ctrl_mem_w8       ;virtual
           mov [ctrl.ctrl_write16], ctrl_mem_w16      ;virtual
           mov [ctrl.ctrl_write32], ctrl_mem_w32      ;virtual
           ret
endp

align 4
proc reset_controller

           xor eax, eax
           mov edx, PCM_IN_CR_REG
           call [ctrl.ctrl_write8]

           mov edx, PCM_OUT_CR_REG
           call [ctrl.ctrl_write8]

           mov edx, MC_IN_CR_REG
           call [ctrl.ctrl_write8]

           mov eax, RR
           mov edx, PCM_IN_CR_REG
           call [ctrl.ctrl_write8]

           mov edx, PCM_OUT_CR_REG
           call [ctrl.ctrl_write8]

           mov edx, MC_IN_CR_REG
           call [ctrl.ctrl_write8]
           ret
endp

align 4
proc init_codec
           locals
             counter dd ?
           endl

           mov esi, msgControl
           call SysMsgBoardStr

           mov edx, GLOB_CTRL
           call [ctrl.ctrl_read32]
           call dword2str
           call SysMsgBoardStr

           mov esi, msgStatus
           call SysMsgBoardStr

           mov edx, CTRL_STAT
           call [ctrl.ctrl_read32]

           call dword2str
           call SysMsgBoardStr

           test eax, CTRL_ST_CREADY
           jnz .ready

           call reset_codec
           and eax, eax
           jz .err

           xor edx, edx     ;ac_reg_0
           call [ctrl.codec_write16]

           xor eax, eax
           mov edx, CODEC_REG_POWERDOWN
           call [ctrl.codec_write16]

           mov [counter], 200     ; total 200*5 ms = 1s
.wait:
           mov edx, CODEC_REG_POWERDOWN
           call [ctrl.codec_read16]
           and eax, 0x0F
           cmp eax, 0x0F
           jz .ready

           mov eax, 5000   ; wait 5 ms
           call StallExec
           sub [counter] , 1
           jnz .wait
.err:
           xor eax, eax        ; timeout error
           ret
.ready:
           call detect_codec

           xor eax, eax
           inc eax
           ret
endp

align 4
proc reset_codec
           mov edx, GLOB_CTRL
           call [ctrl.ctrl_read32]

           test eax, 0x02
           jz .cold

           call warm_reset
           jnc .ok
.cold:
           call cold_reset
           jnc .ok

     if DEBUG
           mov esi, msgCFail
           call SysMsgBoardStr
           end if
           xor eax, eax     ; timeout error
           ret
.ok:
     if DEBUG
           mov esi, msgResetOk
           call SysMsgBoardStr
     end if

           xor eax, eax
           inc eax
           ret
endp

align 4
proc warm_reset
           locals
             counter dd ?
           endl

           mov eax, 0x06
           mov edx, GLOB_CTRL
           call [ctrl.ctrl_write32]

     if DEBUG
           mov esi, msgWarm
           call SysMsgBoardStr
     end if

           mov [counter], 10    ; total 10*100 ms = 1s
.wait:
           mov eax, 100000    ; wait 100 ms
           call StallExec

           mov edx, GLOB_CTRL
           call [ctrl.ctrl_read32]
           test eax, 4
           jz .ok
           sub [counter], 1
           jnz .wait

     if DEBUG
           mov esi, msgWRFail
           call SysMsgBoardStr
     end if

           stc
           ret
.ok:
           mov edx, CTRL_STAT
           call [ctrl.ctrl_read32]
           and eax, CTRL_ST_CREADY
           jz .fail
           clc
           ret
.fail:
           stc
	   ret
endp

align 4
proc cold_reset
           locals
             counter dd ?
           endl

           xor eax, eax
           mov edx, GLOB_CTRL
           call [ctrl.ctrl_write32]

     if DEBUG
           mov esi, msgCold
           call SysMsgBoardStr
     end if

           mov eax, 1000000    ; wait 1 s
           call StallExec

           mov eax, 2
           mov edx, GLOB_CTRL
           call [ctrl.ctrl_write32]

           mov [counter], 10    ; total 10*100 ms = 1s
.wait:
           mov eax, 100000    ; wait 100 ms
           call StallExec

           mov edx, GLOB_CTRL
           call [ctrl.ctrl_read32]
           test eax, 4
           jz .ok
           sub [counter], 1
           jnz .wait

     if DEBUG
           mov esi, msgCRFail
           call SysMsgBoardStr
     end if
           stc
           ret
.ok:
           mov edx, CTRL_STAT
           call [ctrl.ctrl_read32]
           and eax, CTRL_ST_CREADY
           jz .fail
           clc
           ret
.fail:
           stc
           ret
endp

align 4
play:
           mov eax, 16
           mov [ctrl.lvi_reg], eax
           mov edx, PCM_OUT_LVI_REG
           call [ctrl.ctrl_write8]

           mov edx, PCM_OUT_CR_REG
           mov ax, 0x1D
           call [ctrl.ctrl_write8]
           xor eax, eax
           ret

align 4
stop:
           mov edx, PCM_OUT_CR_REG
           mov ax, 0x0
           call [ctrl.ctrl_write8]

           mov ax, 0x1c
           mov edx, PCM_OUT_SR_REG
           call [ctrl.ctrl_write16]
           xor eax, eax
           ret

align 4
proc get_dev_info stdcall, p_info:dword
           virtual at esi
             CTRL_INFO CTRL_INFO
           end virtual

           mov esi, [p_info]
           mov eax, [ctrl.int_line]
           mov ebx, [ctrl.codec_io_base]
           mov ecx, [ctrl.ctrl_io_base]
           mov edx, [ctrl.codec_mem_base]
           mov edi, [ctrl.ctrl_mem_base]

           mov [CTRL_INFO.irq], eax
           mov [CTRL_INFO.codec_io_base], ebx
           mov [CTRL_INFO.ctrl_io_base], ecx
           mov [CTRL_INFO.codec_mem_base], edx
           mov [CTRL_INFO.ctrl_mem_base], edi

           mov eax, [codec.chip_id]
           mov [CTRL_INFO.codec_id], eax

           mov edx, GLOB_CTRL
           call [ctrl.ctrl_read32]
           mov [CTRL_INFO.glob_cntrl], eax

           mov edx, CTRL_STAT
           call [ctrl.ctrl_read32]
           mov [CTRL_INFO.glob_sta], eax

           mov ebx, [ctrl.pci_cmd]
           mov [CTRL_INFO.pci_cmd], ebx
           ret
endp

align 4
proc set_callback stdcall, handler:dword
           mov eax, [handler]
           mov [ctrl.user_callback], eax
           ret
endp

align 4
proc codec_read stdcall, ac_reg:dword	   ; reg = edx, reval = eax

           mov edx, [ac_reg]

           mov ebx, edx
           shr ebx, 1
           bt [codec.shadow_flag], ebx
           jc .use_shadow

           call [ctrl.codec_read16]  ;change edx !!!
           mov ecx, eax

           mov edx, CTRL_STAT
           call [ctrl.ctrl_read32]
           test eax, CTRL_ST_RCS
           jz .read_ok

           mov edx, CTRL_STAT
           call [ctrl.ctrl_write32]
           xor eax,eax
           not eax  ;timeout
           ret
.read_ok:
           mov edx, [ac_reg]
           mov [codec.regs+edx], cx
           bts [codec.shadow_flag], ebx
           mov eax, ecx
           ret
.use_shadow:
           movzx eax, word [codec.regs+edx]
           ret
endp

align 4
proc codec_write stdcall, ac_reg:dword
           push eax
           call check_semafore
           and eax, eax
           jz .err
           pop eax

           mov esi, [ac_reg]
           mov edx, esi
           call [ctrl.codec_write16]
           mov [codec.regs+esi], ax
           shr esi, 1
           bts [codec.shadow_flag], esi
           ret
.err:
           pop eax
           ret
endp

align 4
proc codec_check_ready

           mov edx, CTRL_ST
           call [ctrl.ctrl_read32]
           and eax, CTRL_ST_CREADY
           jz .not_ready

           xor eax, wax
           inc eax
           ret
.not_ready:
           xor eax, eax
           ret
endp

align 4
proc check_semafore
           local counter:DWORD

           mov [counter], 100
.l1:
           mov edx, CTRL_CAS
           call [ctrl.ctrl_read8]
           and eax, CAS_FLAG
           jz .ok

           mov eax, 1
           call StallExec
           sub [counter], 1
           jnz .l1
           xor eax, eax
           ret
align 4
.ok:
           xor eax,eax
           inc eax
           ret
endp

align 4
proc StallExec
           push ecx
           push edx
           push ebx
           push eax

           mov ecx, CPU_FREQ
           mul ecx
           mov ebx, eax       ;low
           mov ecx, edx       ;high
           rdtsc
           add ebx, eax
           adc ecx,edx
@@:
           rdtsc
           sub eax, ebx
           sbb edx, ecx
           js @B

           pop eax
           pop ebx
           pop edx
           pop ecx
           ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;          CONTROLLER IO functions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc codec_io_r16
           add edx, [ctrl.codec_io_base]
           in  ax, dx
           ret
endp

align 4
proc codec_io_w16
           add edx, [ctrl.codec_io_base]
           out dx, ax
           ret
endp

align 4
proc ctrl_io_r8
           add edx, [ctrl.ctrl_io_base]
           in  al, dx
           ret
endp

align 4
proc ctrl_io_r16
           add edx, [ctrl.ctrl_io_base]
           in  ax, dx
           ret
endp

align 4
proc ctrl_io_r32
           add edx, [ctrl.ctrl_io_base]
           in  eax, dx
           ret
endp

align 4
proc ctrl_io_w8
           add edx, [ctrl.ctrl_io_base]
           out dx, al
           ret
endp

align 4
proc ctrl_io_w16
           add edx, [ctrl.ctrl_io_base]
           out dx, ax
           ret
endp

align 4
proc ctrl_io_w32
           add edx, [ctrl.ctrl_io_base]
           out dx, eax
           ret
endp

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;         MEMORY MAPPED IO    (os depended)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

align 4
proc codec_mem_r16
           add edx, [ctrl.codec_mem_base]
           mov ax, word [edx]
           ret
endp

align 4
proc codec_mem_w16
           add edx, [ctrl.codec_mem_base]
           mov word [edx], ax
           ret
endp

align 4
proc ctrl_mem_r8
           add edx, [ctrl.ctrl_mem_base]
           mov al, [edx]
           ret
endp

align 4
proc ctrl_mem_r16
           add edx, [ctrl.ctrl_mem_base]
           mov ax, [edx]
           ret
endp

align 4
proc ctrl_mem_r32
           add edx, [ctrl.ctrl_mem_base]
           mov eax, [edx]
           ret
endp

align 4
proc ctrl_mem_w8
           add edx, [ctrl.ctrl_mem_base]
           mov [edx], al
           ret
endp

align 4
proc ctrl_mem_w16
           add edx, [ctrl.ctrl_mem_base]
           mov [edx], ax
           ret
endp

align 4
proc ctrl_mem_w32
           add edx, [ctrl.ctrl_mem_base]
           mov [edx], eax
           ret
endp

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


include "codec.inc"

align 4
devices dd (CTRL_ICH  shl 16)+VID_INTEL,msg_ICH, set_ICH
        dd (CTRL_ICH0 shl 16)+VID_INTEL,msg_ICH0,set_ICH
        dd (CTRL_ICH2 shl 16)+VID_INTEL,msg_ICH2,set_ICH
        dd (CTRL_ICH3 shl 16)+VID_INTEL,msg_ICH3,set_ICH
        dd (CTRL_ICH4 shl 16)+VID_INTEL,msg_ICH4,set_ICH4
        dd (CTRL_ICH5 shl 16)+VID_INTEL,msg_ICH5,set_ICH4
        dd (CTRL_ICH6 shl 16)+VID_INTEL,msg_ICH6,set_ICH4
        dd (CTRL_ICH7 shl 16)+VID_INTEL,msg_ICH7,set_ICH4

        dd (CTRL_NFORCE  shl 16)+VID_NVIDIA,msg_NForce, set_ICH
        dd (CTRL_NFORCE2 shl 16)+VID_NVIDIA,msg_NForce2,set_ICH
        dd (CTRL_NFORCE3 shl 16)+VID_NVIDIA,msg_NForce3,set_ICH
        dd (CTRL_MCP04   shl 16)+VID_NVIDIA,msg_MCP04,set_ICH
        dd (CTRL_CK804   shl 16)+VID_NVIDIA,msg_CK804,set_ICH
        dd (CTRL_CK8     shl 16)+VID_NVIDIA,msg_CK8,set_ICH
        dd (CTRL_CK8S    shl 16)+VID_NVIDIA,msg_CK8S,set_ICH
        dd (CTRL_MCP51   shl 16)+VID_NVIDIA,msg_MCP51,set_ICH

        dd 0    ;terminator

version      dd 0x00040004

msg_ICH      db 'Intel ICH',  13,10, 0
msg_ICH0     db 'Intel ICH0', 13,10, 0
msg_ICH2     db 'Intel ICH2', 13,10, 0
msg_ICH3     db 'Intel ICH3', 13,10, 0
msg_ICH4     db 'Intel ICH4', 13,10, 0
msg_ICH5     db 'Intel ICH5', 13,10, 0
msg_ICH6     db 'Intel ICH6', 13,10, 0
msg_ICH7     db 'Intel ICH7', 13,10, 0
msg_Intel    db 'Intel Corp. ', 0

msg_NForce   db 'NForce',      13,10, 0
msg_NForce2  db 'NForce 2',    13,10, 0
msg_NForce3  db 'NForce 3',    13,10, 0
msg_MCP04    db 'NForce MCP04',13,10, 0
msg_CK804    db 'NForce CK804',13,10, 0
msg_CK8      db 'NForce CK8',  13,10, 0
msg_CK8S     db 'NForce CK8S', 13,10, 0
msg_MCP51    db 'NForce MCP51',13,10, 0

msg_NVidia   db 'NVidia', 0

szKernel	    db 'KERNEL', 0
sz_sound_srv	    db 'SOUND',0

msgInit      db 'detect hardware...',13,10,0
msgFail      db 'device not found',13,10,0
msgAttchIRQ  db 'IRQ line not supported', 13,10, 0
msgInvIRQ    db 'IRQ line not assigned or invalid', 13,10, 0
msgPlay      db 'start play', 13,10,0
msgStop      db 'stop play',  13,10,0
msgNotify    db 'call notify',13,10,0
msgIRQ       db 'AC97 IRQ', 13,10,0
msgInitCtrl  db 'init controller',13,10,0
msgInitCodec db 'init codec',13,10,0
msgPrimBuff  db 'create primary buffer',13,10,0
msgReg       db 'set service handler',13,10,0
msgOk        db 'service installed',13,10,0
msgCold      db 'cold reset',13,10,0
msgWarm      db 'warm reset',13,10,0
msgWRFail    db 'warm reset failed',13,10,0
msgCRFail    db 'cold reset failed',13,10,0
msgCFail     db 'codec not ready',13,10,0
msgResetOk   db 'reset complete',13,10,0
msgStatus    db 'global status   ',0
msgControl   db 'global control  ',0

section '.data' data readable writable align 16

pcmout_bdl       rq 32
buff_list        rd 32

codec CODEC
ctrl AC_CNTRL

lpc_bus  rd 1
civ_val  rd 1
