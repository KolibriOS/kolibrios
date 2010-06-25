;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
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
    dd 0,0,0, (module+(0x100000000-OS_BASE)) , 1
    dd count,count,(addresses+(0x100000000-OS_BASE)),(names+(0x100000000-OS_BASE)),(ordinal+(0x100000000-OS_BASE))
    addresses:
   forward
    dd (label+(0x100000000-OS_BASE))
   common
    names:
   forward
    local name
    dd (name+(0x100000000-OS_BASE))
   common
    ordinal: count = 0
   forward
    dw count
    count = count+1
   common
    module db dllname,0
   forward
    name db string,0
 }
