$Revision: 431 $
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;  PCI16.INC                                                   ;;
;;                                                              ;;
;;  16 bit PCI driver code                                      ;;
;;                                                              ;;
;;  Version 0.2  December 21st, 2002                            ;;
;;                                                              ;;
;;  Author: Victor Prodan, victorprodan@yahoo.com               ;;
;;                                                              ;;
;;  See file COPYING for details                                ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


init_pci_16:

        pushad

        xor ax,ax
        mov es,ax
        mov byte [es:0x9020],1 ;default mechanism:1
        mov ax,0xb101
        int 0x1a
        or ah,ah
        jnz pci16skip

        mov [es:0x9021],cl ;last PCI bus in system
        mov [es:0x9022],bx
        mov [es:0x9024],edi

; we have a PCI BIOS, so check which configuration mechanism(s)
; it supports
; AL = PCI hardware characteristics (bit0 => mechanism1, bit1 => mechanism2)
        test al,1
        jnz pci16skip
        test al,2
        jz pci16skip
        mov byte [es:0x9020],2 ; if (al&3)==2 => mechanism 2

pci16skip:

        mov ax,0x1000
        mov es,ax

        popad
