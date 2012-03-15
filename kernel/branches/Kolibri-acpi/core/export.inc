;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; Macroinstruction for making export section


macro export dllname,[label,string]
 { common
    local module,addresses,names,ordinal,count
    count = 0
   forward
    count = count+1
   common
    dd 0,0,0, (module-OS_BASE) , 1
    dd count,count,(addresses-OS_BASE),(names-OS_BASE),(ordinal-OS_BASE)
    addresses:
   forward
    dd (label-OS_BASE)
   common
    names:
   forward
    local name
    dd (name-OS_BASE)
   common
    ordinal:
             count = 0
   forward
    dw count
    count = count+1
   common
    module db dllname,0
   forward
    name db string,0
 }
