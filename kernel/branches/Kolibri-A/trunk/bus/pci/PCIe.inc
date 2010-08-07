;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) 2010 KolibriOS team.     All rights reserved.  ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;                                                              ;;
;;  PCIe.INC                                                    ;;
;;                                                              ;;
;;  Extended PCI express services                               ;;
;;                                                              ;;
;;                  art_zh  <artem@jerdev.co.uk>                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision: 1554 $

;***************************************************************************
;   Function
;      pci_ext_config:
;
;   Description
;       PCIe extended (memory-mapped) config space detection
;
;   WARNINGs:
;       1) Very Experimental!
;       2) direct HT-detection (no ACPI or BIOS service used)
;       3) Only AMD/HT processors currently supported
;
;***************************************************************************

align 4
mmio_pcie_cfg_addr	dd	0x00000000	; pcie space may be defined here
mmio_pcie_cfg_lim	dd	0x000FFFFF	; upper pcie space address
mmio_pcie_cfg_pdes	dw	1		; number of PDEs to map the space
PCIe_bus_range		dw	2		; the Bus range: power-of-2 Megabytes


align 4
pci_ext_config:
	mov	eax, [mmio_pcie_cfg_addr]
	mov	ebx, eax
	or	ebx, ebx
	jz	@f
	or	ebx, 0x7FFFFFFF 	; required by PCI-SIG standards
	jnz	.pcie_failed
	add	ebx, 0x0FFFFC
	cmp	ebx, [mmio_pcie_cfg_lim]; is the space limit correct?
	ja	.pcie_failed
	ret				; return the address forced
@@:
	mov	ebx, [cpu_vendor]
	cmp	ebx, dword [AMD_str]
	jne	.pcie_failed
	mov	bx, 0xC184		; dev = 24, fn = 01, reg = 84h

.check_HT_mmio:
	mov	cx, bx
	mov	ax, 0x0002		; bus = 0, 1dword to read
	call	pci_read_reg
	mov	bx, cx
	sub	bl, 4
	and	al, 0x80		; check the NP bit
	jz	.no_pcie_cfg
	shl	eax, 8			; bus:[27..20], dev:[19:15]
	or	eax, 0x00007FFC 	; fun:[14..12], reg:[11:2]
;       mov     [mmio_pcie_cfg_lim], eax
	mov	cl, bl
	mov	ax, 0x0002		; bus = 0, 1dword to read
	call	pci_read_reg
	mov	bx, cx
	test	al, 0x03		; MMIO Base RW enabled?
	jz	.no_pcie_cfg
	test	al, 0x0C		; MMIO Base locked?
	jnz	.no_pcie_cfg
	xor	al, al
	shl	eax, 8
	test	eax, 0x000F0000 	; MMIO Base must be bus0-aligned
	jnz	.no_pcie_cfg

;       -- it looks like a true PCIe config space;

	ret	; <<<<<<<<<<< OK >>>>>>>>>>>

.no_pcie_cfg:
	xor	eax, eax
	mov	[mmio_pcie_cfg_addr], eax
	mov	[mmio_pcie_cfg_lim],  eax
	add	bl, 12
	cmp	bl, 0xC0		; MMIO regs lay below this offset
	jb	.check_HT_mmio

.pcie_failed:
	mov	esi, boot_pcie_fail
	call	boot_log
	ret	; <<<<<<<<< FAILURE >>>>>>>>>

