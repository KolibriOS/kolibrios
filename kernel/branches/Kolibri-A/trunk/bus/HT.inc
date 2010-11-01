;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) 2010 KolibriOS team.     All rights reserved.  ;;
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
;      rs7xx_pcie_init:
;
;   Description
;       PCIe extended (memory-mapped) config space detection
;
;***************************************************************************

align 4

rs7xx_pcie_init:
	call	rs7xx_unlock_bar3
	mov	al, 0x7C		       ; NB_IOC_CFG_CNTL
	call	rs7xx_nbconfig_read_pci
	mov	ebx, eax
;       call    rs7xx_nbconfig_flush_pci
	test	ebx, 0x20000000 		; BAR3 locked?
	jz	$
	mov	al, 0x84			; NB_PCI_ARB
	call	rs7xx_nbconfig_read_pci
	shr	eax,16
	and	ax, 7				; the Bus range lays here:
	jnz	@f
	mov	ax, 8					; 1=2Mb,  2=4MB,  3=8MB,  4=16MB
@@:
	mov	word[PCIe_bus_range-OS_BASE], ax	; 5=32Mb, 6=64MB, 7=128Mb, 8=256Mb
	mov	cl, al
	call	rs7xx_nbconfig_flush_pci
	dec	cl				; <4M ?
	jz	@f
	dec	cl				; one PDE needed anyway
@@:
	mov	ebx, 1
	shl	ebx, cl
	mov	word[mmio_pcie_cfg_pdes-OS_BASE], bx	; 1..64 PDE(s) needed,
	shl	ebx, 22
	mov	dword[mmio_pcie_cfg_lim-OS_BASE], ebx	; or 4..256Mb space to map
	dec	dword[mmio_pcie_cfg_lim-OS_BASE]

	mov	al, 0x1C			; NB_BAR3_PCIEXP_MMCFG
	call	rs7xx_nbconfig_read_pci
	mov	ebx, eax
	call	rs7xx_nbconfig_flush_pci
	mov	eax, ebx
	and	eax, 0xFFE00000 		; valid bits [31..21]
	jz	$				; NB BAR3 may be invisible!
.addr_found:
	mov	dword[mmio_pcie_cfg_addr-OS_BASE], eax	; physical address (lower 32 bits)
	add	dword[mmio_pcie_cfg_lim-OS_BASE],  eax

	or	eax, (PG_NOCACHE + PG_SHARED + PG_LARGE + PG_UW)  ;  by the way, UW is unsafe!
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
;        mov     eax, cr3
;        mov     cr3, eax                        ; flush TLB
.pcie_cfg_mapped:
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




