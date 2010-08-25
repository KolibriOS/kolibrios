;;================================================================================================;;
;;//// libimg.inc //// (c) mike.dld, 2007-2008, (c) diamond, 2009 ////////////////////////////////;;
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


struct FormatsTableEntry
  Is     dd ?
  Decode dd ?
  Encode dd ?
ends

struct Image
  Checksum dd ? ; ((Width ROL 16) OR Height) XOR Data[0]
  Width    dd ?
  Height   dd ?
  Next     dd ?
  Previous dd ?
  Type     dd ? ; one of Image.bppN
  Data     dd ?
  Palette  dd ? ; used iff Type eq Image.bpp8
  Extended dd ?
  Flags    dd ? ; bitfield
  Delay    dd ? ; used iff Image.IsAnimated is set in Flags
ends

; values for Image.Type
; must be consecutive to allow fast switch on Image.Type in support functions
Image.bpp8  = 1
Image.bpp24 = 2
Image.bpp32 = 3
Image.bpp15 = 4
Image.bpp16 = 5
Image.bpp1  = 6

; bits in Image.Flags
Image.IsAnimated = 1

struct ImageDecodeOptions
  UsedSize        dd ? ; if >=8, the field BackgroundColor is valid, and so on
  BackgroundColor dd ? ; used for transparent images as background
ends

FLIP_VERTICAL   = 0x01
FLIP_HORIZONTAL = 0x02
FLIP_BOTH   = FLIP_VERTICAL or FLIP_HORIZONTAL

ROTATE_90_CW   = 0x01
ROTATE_180     = 0x02
ROTATE_270_CW  = 0x03
ROTATE_90_CCW  = ROTATE_270_CW
ROTATE_270_CCW = ROTATE_90_CW
