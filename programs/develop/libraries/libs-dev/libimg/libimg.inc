;;================================================================================================;;
;;//// libimg.inc //// (c) mike.dld, 2007-2008 ///////////////////////////////////////////////////;;
;;================================================================================================;;
;;                                                                                                ;;
;; This file is part of Common development libraries (Libs-Dev).                                  ;;
;;                                                                                                ;;
;; Libs-Dev is free software: you can redistribute it and/or modify it under the terms of the GNU ;;
;; General Public License as published by the Free Software Foundation, either version 3 of the   ;;
;; License, or (at your option) any later version.                                                ;;
;;                                                                                                ;;
;; Libs-Dev is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without  ;;
;; even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU  ;;
;; General Public License for more details.                                                       ;;
;;                                                                                                ;;
;; You should have received a copy of the GNU General Public License along with Libs-Dev. If not, ;;
;; see <http://www.gnu.org/licenses/>.                                                            ;;
;;                                                                                                ;;
;;================================================================================================;;


struct FormatsTableEntry
  Is	 dd ?
  Decode dd ?
  Encode dd ?
ends

struct Image
  Checksum dd ? ; ((Width ROL 16) OR Height) XOR Data[0]
  Width    dd ?
  Height   dd ?
  Next	   dd ?
  Previous dd ?
  Data	   dd ?
  Extended dd ?
ends

FLIP_VERTICAL	= 0x01
FLIP_HORIZONTAL = 0x02
FLIP_BOTH	= FLIP_VERTICAL or FLIP_HORIZONTAL
