;;================================================================================================;;
;;//// libini_p.inc //// (c) mike.dld, 2007-2008 /////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; Lesser General Public License as published by the Free Software Foundation, either version 2.1 ;;
;; of the License, or (at your option) any later version.                                         ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; Lesser General Public License for more details.                                                ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU Lesser General Public License along with Libs-Dev.  ;;
;; If not, see <http://www.gnu.org/licenses/>.                                                    ;;
;;                                                                                                ;;
;;================================================================================================;;


ini.MAX_NAME_LEN  = 1024
ini.MAX_VALUE_LEN = 4096
ini.MAX_BOOL_LEN  = 9
ini.MEM_SIZE	  = 4096
ini.BLOCK_SIZE	  = ini.MEM_SIZE / 2

ini.COMMENT_CHAR  = ';'

struct IniFile
  fh	dd ?
  buf	dd ?
  cnt	dd ?
  pos	dd ?
  bsize dd ?
ends
