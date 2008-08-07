;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$




align 4

init_BIOS32:
           mov edi, 0xE0000
.pcibios_nxt:
           cmp dword[edi], '_32_' ; "magic" word
           je .BIOS32_found
.pcibios_nxt2:
           add edi, 0x10
           cmp edi, 0xFFFF0
           je .BIOS32_not_found
           jmp .pcibios_nxt
.BIOS32_found:			; magic word found, check control summ

           movzx ecx, byte[edi + 9]
           shl ecx, 4
           mov esi, edi
           xor eax, eax
           cld   ; paranoia
@@:	lodsb
           add ah, al
           loop @b
           jnz .pcibios_nxt2 ; control summ must be zero
    ; BIOS32 service found !
           mov ebp, [edi + 4]
           mov [bios32_entry], ebp
    ; check PCI BIOS present
           mov eax, '$PCI'
           xor ebx, ebx
           push cs  ; special for 'ret far' from  BIOS
           call ebp
           test al, al
           jnz .PCI_BIOS32_not_found

 ; здесь создаются дискрипторы для PCI BIOS

           add ebx, OS_BASE
           dec ecx
           mov [(pci_code_32-OS_BASE)], cx    ;limit 0-15
           mov [(pci_data_32-OS_BASE)], cx    ;limit 0-15

           mov [(pci_code_32-OS_BASE)+2], bx  ;base  0-15
           mov [(pci_data_32-OS_BASE)+2], bx  ;base  0-15

           shr ebx, 16
           mov [(pci_code_32-OS_BASE)+4], bl  ;base  16-23
           mov [(pci_data_32-OS_BASE)+4], bl  ;base  16-23

           shr ecx, 16
           and cl, 0x0F
           mov ch, bh
           add cx, D32
           mov [(pci_code_32-OS_BASE)+6], cx  ;lim   16-19 &
           mov [(pci_data_32-OS_BASE)+6], cx  ;base  24-31

           mov [(pci_bios_entry-OS_BASE)], edx
         ; jmp .end
.PCI_BIOS32_not_found:
	; здесь должна заполнятся pci_emu_dat
.BIOS32_not_found:
.end:
           ret


