;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                                 ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved.    ;;
;; Distributed under terms of the GNU General Public License       ;;
;;                                                                 ;;
;;          GNU GENERAL PUBLIC LICENSE                             ;;
;;             Version 2, June 1991                                ;;
;;                                                                 ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

proc set_cdw0 stdcall, pci:dword, y:dword, opcode:byte

	stdcall get_new_cid, [pci], [y]
	shl 	eax, 16
	or 	al, [opcode]
	ret

endp

; See pages 161-205 of the NVMe 1.4 specification for reference
proc nvme_identify stdcall, pci:dword, nsid:dword, prp1:dword, cns:byte

	push 	esi
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.spinlock], 1
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4

	mov     eax, [nsid]
	mov     dword [esp + SQ_ENTRY.nsid], eax
	mov     eax, [prp1]
	mov     dword [esp + SQ_ENTRY.prp1], eax
	stdcall set_cdw0, esi, ADMIN_QUEUE, ADM_CMD_IDENTIFY
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	mov     al, [cns]
	mov     byte [esp + SQ_ENTRY.cdw10], al
	stdcall sqytdbl_write, esi, ADMIN_QUEUE, esp

	add 	esp, sizeof.SQ_ENTRY
	stdcall nvme_poll, esi
	pop 	esi
	ret

endp

; See page 101 of the NVMe 1.4 specification for reference
proc create_io_completion_queue stdcall, pci:dword, prp1:dword, qid:dword, ien:byte
	
	push 	esi
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.spinlock], 1
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, esi, ADMIN_QUEUE, ADM_CMD_CRE_IO_COMPLETION_QUEUE
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	mov 	eax, [prp1]
	mov 	dword [esp + SQ_ENTRY.prp1], eax
	mov 	eax, (CQ_ENTRIES - 1) shl 16 ; CDW10.QSIZE (0's based)
	or 	eax, [qid] ; CDW10.QID
	mov 	dword [esp + SQ_ENTRY.cdw10], eax
	movzx 	eax, [ien] ; CDW11.IEN 
	or 	eax, 0x1 ; CDW11.PC
	; Don't set CDW11.IV since we're not using MSI-X or MSI vector
	mov 	dword [esp + SQ_ENTRY.cdw11], eax
	stdcall sqytdbl_write, esi, ADMIN_QUEUE, esp
	add 	esp, sizeof.SQ_ENTRY
	stdcall nvme_poll, esi
	pop 	esi
	ret

endp

; See page 103-104 of the NVMe 1.4 specification for reference
proc create_io_submission_queue stdcall, pci:dword, prp1:dword, qid:dword, cqid:word

	push 	esi
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.spinlock], 1
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, esi, ADMIN_QUEUE, ADM_CMD_CRE_IO_SUBMISSION_QUEUE
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	mov 	eax, [prp1]
	mov 	dword [esp + SQ_ENTRY.prp1], eax
	mov 	eax, (SQ_ENTRIES - 1) shl 16 ; CDW10.QSIZE (0's based)
	or 	eax, [qid]
	mov 	dword [esp + SQ_ENTRY.cdw10], eax
	movzx 	eax, [cqid]
	shl 	eax, 16 ; CDW11.CQID
	or 	eax, 0x1 ; CDW11.PC (always set this to 1 as some devices may not support non-contiguous pages)
	; TODO: Set CDW10.QPRIO 
	mov 	dword [esp + SQ_ENTRY.cdw11], eax
	stdcall sqytdbl_write, esi, ADMIN_QUEUE, esp
	add 	esp, sizeof.SQ_ENTRY
	stdcall nvme_poll, esi
	pop 	esi
	ret

endp

; See page 95-96 of the NVMe 1.4 specification for reference
proc abort stdcall, pci:dword, cid:word, sqid:word

	push 	esi
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.spinlock], 1
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, esi, ADMIN_QUEUE, ADM_CMD_ABORT
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	movzx 	eax, [cid]
	shl 	eax, 16
	or 	eax, word [sqid]
	mov 	dword [esp + SQ_ENTRY.cdw10], eax
	stdcall sqytdbl_write, esi, ADMIN_QUEUE, esp
	add 	esp, sizeof.SQ_ENTRY
	stdcall nvme_poll, esi
	pop 	esi
	ret

endp


; See page 205 of the NVMe 1.4 specification for reference
proc set_features stdcall, pci:dword, prp1:dword, fid:byte, cdw11:dword
	
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, [pci], ADMIN_QUEUE, ADM_CMD_SET_FEATURES
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	mov 	eax, [prp1]
	mov 	dword [esp + SQ_ENTRY.prp1], eax
	movzx 	eax, [fid]
	mov 	dword [esp + SQ_ENTRY.cdw10], eax
	mov 	eax, [cdw11]
	mov 	dword [esp + SQ_ENTRY.cdw11], eax
	stdcall sqytdbl_write, [pci], ADMIN_QUEUE, esp
	add 	esp, sizeof.SQ_ENTRY
	ret

endp

; See page 105 of the NVMe 1.4 specification for reference
proc delete_io_completion_queue stdcall, pci:dword, qid:word

	push 	esi
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.spinlock], 1
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, esi, ADMIN_QUEUE, ADM_CMD_DEL_IO_COMPLETION_QUEUE
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	mov 	ax, [qid]
	mov 	word [esp + SQ_ENTRY.cdw10], ax
	stdcall sqytdbl_write, esi, ADMIN_QUEUE, esp
	add 	esp, sizeof.SQ_ENTRY
	stdcall nvme_poll, esi
	pop 	esi
	ret

endp

; See page 114-116 of the NVMe 1.4 specification for reference
proc get_features stdcall, pci:dword, prp1:dword, sel:byte, fid:byte
	
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, [pci], ADMIN_QUEUE, ADM_CMD_GET_FEATURES
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	movzx 	eax, [sel]
	and 	eax, 111b
	shl 	eax, 8 ; CDW10.SEL
	or 	eax, byte [fid] ; CDW10.FID
	mov 	dword [esp + SQ_ENTRY.cdw10], eax
	mov 	eax, [prp1]
	mov 	dword [esp + SQ_ENTRY.prp1], eax
	; TODO: Implement CDW14.UUID?
	stdcall sqytdbl_write, [pci], ADMIN_QUEUE, esp
	add 	esp, sizeof.SQ_ENTRY
	ret

endp

; See page 105-106 of the NVMe 1.4 specification for reference
proc delete_io_submission_queue stdcall, pci:dword, qid:word

	push 	esi
	mov 	esi, [pci]
	mov 	dword [esi + pcidev.spinlock], 1
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, esi, ADMIN_QUEUE, ADM_CMD_DEL_IO_SUBMISSION_QUEUE
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	mov 	ax, [qid]
	mov 	word [esp + SQ_ENTRY.cdw10], ax
	stdcall sqytdbl_write, esi, ADMIN_QUEUE, esp
	add 	esp, sizeof.SQ_ENTRY
	stdcall nvme_poll, esi
	pop 	esi
	ret

endp

; See page 117-118 of the NVMe 1.4 specification for reference
; INCOMPLETE
proc get_log_page stdcall, pci:dword, prp1:dword, lid:byte

	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	stdcall set_cdw0, [pci], ADMIN_QUEUE, ADM_CMD_GET_LOG_PAGE
	mov 	dword [esp + SQ_ENTRY.cdw0], eax
	mov 	eax, [prp1]
	mov 	dword [esp + SQ_ENTRY.prp1], eax
	add 	esp, sizeof.SQ_ENTRY
	ret

endp

; See pages 348-349 of the NVMe 1.4 specification for information on creating namespaces
proc create_namespace stdcall, pci:dword, cid:word
	
	push 	esi
	invoke  AllocPage
	test 	eax, eax
	jz 	.fail
	invoke  GetPhysAddr
	stdcall nvme_identify, [pci], 0xffffffff, eax, CNS_IDNS
	test 	eax, eax
	jz 	.fail

.fail:
	pop 	esi
	ret

endp

; See page 258-261 (read) and 269-271 (write) of the NVMe 1.4 specification for reference
proc nvme_io_rw stdcall, pci:dword, qid:word, nsid:dword, prps:qword, slba:qword, nlb:dword, opcode:dword
	
	; TODO: Use IDENTC.NOIOB to construct read/write commands that don't
	; cross the I/O boundary to achieve optimal performance
	; 
	; TODO: Read AWUN/NAWUN
	sub 	esp, sizeof.SQ_ENTRY
	mov 	eax, esp
	stdcall memsetdz, eax, sizeof.SQ_ENTRY / 4
	movzx 	ecx, [qid]
	stdcall set_cdw0, [pci], ecx, [opcode]
	mov 	dword [esp + SQ_ENTRY.cdw0], eax ; CDW0
	mov 	eax, dword [prps]
	mov 	dword [esp + SQ_ENTRY.prp1], eax
	mov 	eax, dword [prps + 4]
	mov 	dword [esp + SQ_ENTRY.prp2], eax
	mov 	eax, [nsid]
	mov 	dword [esp + SQ_ENTRY.nsid], eax
	mov 	eax, dword [slba] ; slba_lo
	mov 	dword [esp + SQ_ENTRY.cdw10], eax
	mov 	eax, dword [slba + 4] ; slba_hi
	mov 	dword [esp + SQ_ENTRY.cdw11], eax
	mov 	eax, [nlb]
	mov 	word [esp + SQ_ENTRY.cdw12], ax
	movzx 	ecx, [qid]
	stdcall sqytdbl_write, [pci], ecx, esp
	add 	esp, sizeof.SQ_ENTRY
	ret
	
endp

; vim: syntax=fasm
