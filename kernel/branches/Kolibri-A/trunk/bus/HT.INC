;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) 2010 KolibriOS team.     All rights reserved.  ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  HT.inc                                                      ;;                                                    ;;
;;                                                              ;;
;;  AMD HyperTransport bus control                              ;;
;;                                                              ;;
;;                  art_zh  <artem@jerdev.co.uk>                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;



align 4

;=============================================================================
;
; This code is a part of Kolibri-A and will only work with AMD RS760+ chipsets
;
;=============================================================================

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

rs7xx_nbconfig_flush_pci:
	mov	eax, 0x0B0		; a scratch reg
	mov	dx,  0xCF8
	out	dx,  eax
	ret


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
;      rs7xx_pcie_init:
;
;   Description
;       PCIe extended (memory-mapped) config space detection
;
;***************************************************************************

rs7xx_pcie_init:
;        mov     al,  0x7C                       ; NB_IOC_CFG_CNTL
;        mov     ebx, 0x20000000
;        call    rs7xx_nbconfig_write_pci
	mov	al, 0x7C		       ; NB_IOC_CFG_CNTL
	call	rs7xx_nbconfig_read_pci
	mov	ebx, eax
	call	rs7xx_nbconfig_flush_pci
	test	ebx, 0x20000000 		; BAR3 locked?
	jz	.rs7xx_pcie_blocked
	mov	al, 0x84			; NB_PCI_ARB
	call	rs7xx_nbconfig_read_pci
	shr	eax,16
	and	ax, 7				; the Bus range lays here:
	jnz	@f
	mov	ax, 8				; 1=2Mb,  2=4MB,  3=8MB,  4=16MB
@@:
	mov	[PCIe_bus_range], ax		; 5=32Mb, 6=64MB, 7=128Mb, 8=256Mb
	mov	cl, al
	call	rs7xx_nbconfig_flush_pci
	dec	cl				; <4M ?
	jnz	@f
	inc	cl				; one PDE needed anyway
@@:
	dec	cl
	mov	ebx, 1
	shl	ebx, cl
	mov	[mmio_pcie_cfg_pdes], bx	; 1..64 PDE(s) needed,
	shl	ebx, 22
	mov	[mmio_pcie_cfg_lim], ebx	; or 4..256Mb space to map
	dec	[mmio_pcie_cfg_lim]

	mov	al, 0x1C			; NB_BAR3_PCIEXP_MMCFG
	call	rs7xx_nbconfig_read_pci
	mov	ebx, eax
	call	rs7xx_nbconfig_flush_pci
	mov	eax, ebx
	and	eax, 0xFFE00000 		; valid bits [31..21]
	jnz	@f				; NB BAR3 may be invisible!
	call  pci_ext_config		; try to get pcie ecfg address indirectly
@@:
	or 	eax, eax
	jz	.rs7xx_pcie_fail
	mov	[mmio_pcie_cfg_addr], eax	; physical address (lower 32 bits)
	add	[mmio_pcie_cfg_lim],  eax

;       -- map the whole PCIe config space;
	or	eax, (PG_SHARED + PG_LARGE + PG_UW)  ;  by the way, UW is unsafe!
	mov	ecx, PCIe_CONFIG_SPACE		; linear address
	mov	ebx, ecx
	shr	ebx, 20
	add	ebx, sys_pgdir			; PgDir entry @
	xor	dx,  dx 				; PDEs counter
@@:
	mov	dword[ebx], eax 			; map 4 buses
	invlpg	[ecx]				; next PgDir entry
	add	bx,  4				; new PDE
	add	eax, 0x400000			; +4M phys.
	add	ecx, 0x400000			; +4M lin.
	inc	dx
	cmp	dx, [mmio_pcie_cfg_pdes]	; all mapped yet?
	jnz	@b

.pcie_cfg_mapped:
	mov	esi, boot_pcie_ok
	call	boot_log
	ret	; <<<<<<<<<<< OK >>>>>>>>>>>

.rs7xx_pcie_fail:
	mov	esi, boot_rs7xx_fail
	call	boot_log
	ret

.rs7xx_pcie_blocked:
	mov	esi, boot_rs7xx_blkd
	call	boot_log


	ret



