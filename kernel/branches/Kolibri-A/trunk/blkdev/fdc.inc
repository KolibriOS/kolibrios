;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Copyright (C) MenuetOS 2000-2004 Ville Mikael Turjanmaa      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$



uglobal
  dmasize	  db 0x0
  dmamode	  db 0x0
endg

fdc_init:				;start with clean tracks.
	mov edi,OS_BASE+0xD201
	mov al,0
	mov ecx,160
	rep stosb
ret

;fdc_irq:
fdc_null:
ret



