;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) 2010-13 KolibriOS team.  All rights reserved.  ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  HT.inc                                                      ;;                                                    ;;
;;                                                              ;;
;;  AMD HyperTransport bus control                              ;;
;;                                                              ;;
;;                  art_zh  <kolibri@jerdev.co.uk>              ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 1554 $

NB_MISC_INDEX	equ	0xF0000060	; NB Misc indirect access
NB_MISC_DATA	equ	0xF0000064
PCIEIND_INDEX	equ	0xF00000E0	; PCIe Core indirect config space access
HTIU_NB_INDEX	equ	0xF0000094	; HyperTransport indirect config space access

;=============================================================================
;
; This code is a part of Kolibri-A and will only work with AMD RS760+ chipsets
;
;=============================================================================

org $-OS_BASE	; physical addresses needed at initial stage

align 4

;------------------------------------------
;       params:   al = nbconfig register#
;       returns: eax = register content
;
rs7xx_nbconfig_read_pci:
	and	eax, 0x0FC		 ; leave register# only
	or	eax, 0x80000000 	 ; bdf = 0:0.0
	mov	dx,  0x0CF8		 ; write to index reg
	out	dx, eax
	add	dl, 4
	in	eax, dx
	ret
align 4

rs7xx_nbconfig_flush_pci:
	mov	eax, 0x0B0		; a scratch reg
	mov	dx,  0xCF8
	out	dx,  eax
	ret

align 4

;------------------------------------------
;       params:   al = nbconfig register#
;                ebx = register content
;
rs7xx_nbconfig_write_pci:
	and	eax, 0x0FC		 ; leave register# only
	or	eax, 0x80000000 	 ; bdf = 0:0.0
	mov	dx,  0x0CF8		 ; write to index reg
	out	dx, eax
	add	dl, 4
	mov	eax, ebx
	out	dx, eax
	ret

;***************************************************************************
;   Function
;      rs7xx_unlock_bar3:     unlocks the BAR3 register of nbconfig that
;                             makes pcie config address space visible
;   -----------------------
;   in: nothing      out: nothing      destroys:   eax ebx edx
;
;***************************************************************************
align 4
rs7xx_unlock_bar3:
	mov	eax, NB_MISC_INDEX
	mov	ebx, 0x080			; NBMISCIND:0x0; write-enable
	call	rs7xx_nbconfig_write_pci	; set index
	mov	eax, NB_MISC_DATA
	call	rs7xx_nbconfig_read_pci 	; read data
	mov	ebx, eax
	and	ebx, 0xFFFFFFF7 		; clear bit3
	mov	eax, NB_MISC_DATA
	call	rs7xx_nbconfig_write_pci	; write it back
	mov	eax, NB_MISC_INDEX
	xor	ebx, ebx			; reg#0; write-locked
	call	rs7xx_nbconfig_write_pci	; set index
	ret



;***************************************************************************
;   Function
;      fusion_pcie_init:
;
;   Description
;       PCIe extended  config space detection and mapping
;
;***************************************************************************

align 4



; ---- stepping 10h CPUs and Fusion APUs: the configspace is stored in MSR_C001_0058 ----
align 4
fusion_pcie_init:
	mov	ecx, 0xC0010058
	rdmsr
	or	edx, edx
	jnz	$				; PCIe is in the upper memory. Stop.
	xchg	dl, al
	mov	dword[mmio_pcie_cfg_addr-OS_BASE], eax	; store the physical address
	mov	ecx, edx

	shr	cl, 2
	mov	word[PCIe_bus_range-OS_BASE], cx
	sub	cl, 2
	jae	@f
	xor	cl, cl
@@:
	shl	edx, cl 			; edx = number of 4M pages to map
	mov	word[mmio_pcie_cfg_pdes-OS_BASE], dx
	shl	edx, 22
	dec	edx
	add	edx, eax			; the upper configspace limit
	mov	dword[mmio_pcie_cfg_lim-OS_BASE], edx

; ---- large pages mapping  ----
; (eax = phys. address of PCIe conf.space)
;
.map_pcie_pages:
	or	eax, (PG_NOCACHE + PG_SHARED + PG_LARGE + PG_UW)  ;  UW is unsafe!
	mov	ecx, PCIe_CONFIG_SPACE			; linear address
	mov	ebx, ecx
	shr	ebx, 20
	add	ebx, (sys_pgdir - OS_BASE)		; PgDir entry @
	mov	dl, byte[mmio_pcie_cfg_pdes-OS_BASE]	; 1 page = 4M in address space
	cmp	dl, 0x34	; =(USER_DMA_BUFFER - PCIe_CONFIG_SPACE) / 4M
	jb	@f
	mov	dl, 0x33
	mov	byte[mmio_pcie_cfg_pdes-OS_BASE], dl
@@:
	xor	dx,  dx 			; PDEs counter
.write_pde:
	mov	dword[ebx], eax 		; map 4 buses
	add	bx,  4				; new PDE
	add	eax, 0x400000			; +4M phys.
	add	ecx, 0x400000			; +4M lin.
	cmp	dl, byte[mmio_pcie_cfg_pdes-OS_BASE]
	jae	.pcie_cfg_mapped
	inc	dl
	jmp	.write_pde

.pcie_cfg_mapped:

create_mmio_pte:
	mov	ecx, mmio_pte				; physical address
	or	ecx, (PG_NOCACHE + PG_SHARED + PG_UW)
	mov	ebx, FUSION_MMIO			; linear address
	shr	ebx, 20
	add	ebx, (sys_pgdir - OS_BASE)		; PgDir entry @
	mov	dword[ebx], ecx 			; Fusion MMIO tables

; ---- map APIC regs  ----
.map_apic_mmio:
	mov	ecx, 0x01B	; APIC BAR
	rdmsr
	and	eax, 0xFFFFF000 			; physical address
	or	eax, (PG_NOCACHE + PG_SHARED + PG_UW)
	mov	ebx, mmio_pte
	mov	[ebx], eax

; ---- map GPU MMRegs  ----
.map_gpu_mmr:
	mov	eax, [mmio_pcie_cfg_addr-OS_BASE]	; PCIe space
	add	eax, 0x08018				; b:0, d:1, f:0, reg=18
	mov	eax, [eax]

	xor	al, al					; physical address
	or	eax, (PG_NOCACHE + PG_SHARED + PG_UW)
@@:
	add   bl, 4
	mov	[ebx], eax
	add   eax, 0x01000
	cmp   bl, 16*4	  ; map 15 pages
	jb    @b

	ret	; <<< OK >>>

; ================================================================================

org OS_BASE+$	; back to the linear address space

;--------------------------------------------------------------
align 4
rs780_read_misc:
;  in: eax(al) - reg#           out: eax = NBMISCIND data
	push	edx
	mov	edx, NB_MISC_INDEX
	and	eax, 0x07F
	mov	[edx], eax
	add	dl, 4
	mov	eax, [edx]
	pop	edx
	ret

;-------------------------------------------
align 4
rs780_write_misc:
;  in: eax(al) - reg#     ebx = NBMISCIND data
	push	edx
	mov	edx, NB_MISC_INDEX
	and	eax, 0x07F
	or	eax, 0x080		; set WE
	mov	[edx], eax
	add	dl, 4
	mov	[edx], ebx
	sub	dl, 4
	xor	eax,   eax
	mov	[edx], eax		; safety last
	pop	edx
	ret

;-------------------------------------------------------------
align 4
rs780_read_pcieind:
;  in: ah = bridge#, al = reg#           out: eax = PCIEIND data
	push	edx
	xor	edx, edx
	mov	ah,  dl 		; bridge# :     0 = Core+GFX;   0x10 = Core+SB
	and	dl,  15 		;            0x20 = Core+GPP;  2..12 = a PortBridge
	shl	edx, 15 		; device#
	add	edx, PCIEIND_INDEX	; full bdf-address
	and	eax, 0x30FF
	or	al,  al
	jnz	@f
	shl	eax, 4			; set bits 17..16 for a Core bridge
@@:
	mov	[edx], eax
	add	dl,  4
	mov	eax, [edx]
	pop	edx
	ret

;-------------------------------------------
align 4
rs780_write_pcieind:
;  in: ah = bridge#, al = reg#,  ebx = PCIEIND data
	push	edx
	xor	edx, edx
	mov	ah,  dl 		; bridge# :     0 = Core+GFX;   0x10 = Core+SB
	and	dl,  15 		;            0x20 = Core+GPP;  2..12 = a PortBridge
	shl	edx, 15 		; device#
	add	edx, PCIEIND_INDEX	; full bdf-address
	and	eax, 0x30FF
	or	al,  al
	jnz	@f
	shl	eax, 4			; set bits 17..16 for a Core bridge
@@:
	mov	[edx], eax
	add	dl,  4
	mov	[edx], ebx
	sub	dl,  4
	xor	eax,   eax
	mov	[edx], eax		; safety last
	pop	edx
	ret

;------------------------------------------------
align 4
rs780_read_htiu:
; in:  al = reg#  |  out: eax = HTIU data
;------------------------------------------------
	push	edx
	mov	edx,  HTIU_NB_INDEX
	and	eax, 0x07F
	mov	[edx], eax
	add	dl,  4
	mov	eax, [edx]
	pop	edx
	ret
;------------------------------------------------
align 4
rs780_write_htiu:
; in:  al = reg#; ebx = data
;------------------------------------------------
	push	edx
	mov	edx,  HTIU_NB_INDEX
	and	eax, 0x07F
	or	eax, 0x100
	mov	[edx], eax
	add	dl,  4
	mov	[edx], ebx
	sub	dl,  4
	xor	eax,   eax
	mov	[edx], eax
	pop	edx
	ret

;------------------------------------------------
align 4
sys_rdmsr:
;  in: [esp+8] = MSR#
; out: [esp+8] = MSR[63:32]
;        [eax] = MSR[31: 0]
;------------------------------------------------
	push	ecx edx
	mov	ecx, [esp+16]
	rdmsr
	mov	[esp+16], edx
	pop	edx ecx
	ret

;------------------------------------------------
uglobal

align 4
diff16 "apic_data : ", 0, $
apic_data:

    .counter dd ?
    .ticks   dd ?
    .t_freq  dd ?
    .gpu_r6998 dd ?
endg

apic_timer_reset:
	mov	eax, [pll_frequency.osc]
	shr	eax, 1					; default prescaler - fix it !!
	mov	[apic_data.t_freq], eax
	shr	eax, 4					; 16 per second
	mov	[apic_data.ticks], eax

	mov	ebx, LAPIC_BAR+ 0x320
	mov	edx, [ebx]
	and	edx, 0xFFFEFF00
	or	edx, 0x0002003F 			; int vector + restart
;--     mov     [ebx], edx
	mov	dword [LAPIC_BAR + 0x380], eax		; load APICTIC

; ret

init_hw_cursor:
	call	alloc_page			 ; eax = phys. addr
	push	eax
	or	eax, (PG_NOCACHE + PG_SHARED + PG_UW)	 ; i like dirty hacks
	mov	[mmio_pte + OS_BASE + 15*4], eax ; mapped to the end of GPU MMRegs
	mov	edi, GPU_CURSOR 		 ; lin. addr
	invlpg	[edi]
	xor	ecx, ecx
.fill64pix:
	xor	ebx, ebx
	mov	eax, 0x80000000 		; black, non-transparent
.check_pix:
	cmp	ebx, ecx
	jbe	@f
	xor	eax, eax			; transparent
@@:
	mov	[edi + ebx*4], eax
	inc	ebx
	cmp	bl, 64
	jb	.check_pix
	inc	ecx
	cmp	ecx, 16
	je	@f
	add	edi, 64*4			; new line
	jmp	.fill64pix
@@:
	pop	eax
	mov	dword[GPU_MMR + 0x0699C], eax	     ; cur_surface_addr
	mov	dword[GPU_MMR + 0x069A0], 0x000F000F ; cur_size    = 16x16
	mov	dword[GPU_MMR + 0x069A4], 0	     ; cur_adr_hi
	mov	dword[GPU_MMR + 0x069A8], 0x02000100 ; cur_pos     = 512,256
	mov	dword[GPU_MMR + 0x069AC], 0	     ; cur_hotspot = 0,0

	mov	dword[GPU_MMR + 0x06998], 0x00000301 ; set it!



	ret


apic_timer_int:
	push	eax
	inc	dword [apic_data.counter]
;        mov     eax,  [apic_data.ticks]
;        mov     dword [LAPIC_BAR + 0x380], eax          ; reload APICTIC
	mov	dword [LAPIC_BAR + 0x0B0], 0		; end of interrupt
;        mov     dword [LAPIC_BAR + 0x420], 0x3F            ; end of interrupt
	pop	eax
	iretd


