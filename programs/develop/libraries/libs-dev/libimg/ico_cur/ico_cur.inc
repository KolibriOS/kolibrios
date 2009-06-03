;;================================================================================================;;
;;//// ico.inc //// (c) diamond, 2009 ////////////////////////////////////////////////////////////;;
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

struct ico.FileHeader
  Reserved   dw ? ; must be 0
  Type       dw ? ; must be 1
  Count      dw ?
ends

struct ico.DirEntry
  Width       db ?
  Height      db ?
  ColorCount  db ?
              db ? ; reseved
              dw ? ; reserved
              dw ? ; reserved
  ByteSize    dd ?
  ImageOffset dd ?
ends

struct cur.DirEntry
  Width       db ?
  Height      db ?
  ColorCount  db ?
              db ? ; reserved
  XHotSpot    dw ?
  YHotSpot    dw ?
  ByteSize    dd ?
  ImageOffset dd ?
ends
