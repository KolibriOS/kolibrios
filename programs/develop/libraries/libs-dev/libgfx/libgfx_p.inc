;;================================================================================================;;
;;//// libgfx_p.inc //// (c) mike.dld, 2006-2008 /////////////////////////////////////////////////;;
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


struct APoint
  X dd ?
  Y dd ?
ends

struct ARect
  Left	 dd ?
  Top	 dd ?
  Right  dd ?
  Bottom dd ?
ends

struct AGfxContext
  NullPoint   APoint
  Caret       APoint
  Pen.Color   dd ?
  Brush.Color dd ?
  Font.Face   dd ?
  Flags       dd ?
ends

GFX_FLAG_DONT_CLOSE = 00000001b
