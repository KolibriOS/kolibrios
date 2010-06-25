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
else
include "boot/bootru.inc"      ; russian system boot messages
;!!!
end if
include "boot/ru.inc"          ; Russian font

include "boot/bootcode.inc"    ; 16 bit system boot code

include "bus/pci/pci16.inc"

;% -include