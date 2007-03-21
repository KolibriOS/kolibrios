;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;; KERNEL16.INC                                               ;;
;;                                                            ;;
;; Included 16 bit kernel files for MenuetOS                  ;;
;;                                                            ;;
;; This file is kept separate as it will be easier to         ;;
;; maintain and compile with an automated SETUP program       ;;
;; in the future.                                             ;;
;;                                                            ;;
;; Copyright Ville Turjanmaa, see file COPYING for details.   ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


;% +include

;!!!
if lang eq en
include "boot/booteng.inc"     ; english system boot messages
else if lang eq ru
include "boot/bootru.inc"      ; russian system boot messages
else if lang eq et
include "boot/bootet.inc"      ; estonian system boot messages
else
include "boot/bootge.inc"      ; german system boot messages
;!!!
end if
if lang eq et
include "boot/et.inc"          ; Estonian font
else
include "boot/ru.inc"          ; Russian font
end if

; сюда данные записываются в 16 разрядном режиме, читаются в 32 разрядном
bx_from_load:			; структура для хранения параметров- откуда гашрузились, берется ниже из bx ; {SPraid}[13.03.2007]
.from_disc	db 0		; a,b,c,d - винчестеры, r - рам диск 
.from_part	db 0,0		; # диска... символ, а не байт. '1', а не 1

org $-0x10000

include "boot/bootcode.inc"    ; 16 bit system boot code

include "bus/pci/pci16.inc"

;% -include