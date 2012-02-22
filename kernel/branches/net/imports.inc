;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;============================================================================
;
;  External kernel dependencies
;
;============================================================================

$Revision$


align 4
@IMPORT:

library \
        libini,'libini.obj'

import  libini, \
        ini.lib_init,'lib_init',\
        ini.get_str,'ini.get_str',\
        ini.enum_keys,'ini.enum_keys',\
        ini.get_int,'ini.get_int'
