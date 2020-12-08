;;================================================================================================;;
;;//// bmp.inc //// (c) mike.dld, 2007-2008, (c) diamond, 2009 ///////////////////////////////////;;
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


struct bmp.FileHeader
  Type	  dw ?	      ; File type, always 4D42h ("BM")
  Size	  dd ?	      ; Size of the file in bytes
	  dw 2 dup(?) ; Reserved; must be set to zero.
  OffBits dd ?	      ; Starting position of image data in bytes
ends

struct bmp.InfoHeader
; v2 (Windows 2.x)
  Size		 dd ? ; Size of this header in bytes
  union
    struct ; new format
      Width 	 dd ? ; Image width in pixels
      Height	 dd ? ; Image height in pixels
      Planes	 dw ? ; Number of color planes
      BitCount	 dw ? ; Number of bits per pixel
    ends
    struct ; old format
      OldWidth   dw ? ; Image width in pixels as word
      OldHeight  dw ? ; Image height in pixels as word
      OldPlanes  dw ? ; Number of color planes
      OldBitCount dw ? ; Number of bits per pixel
    ends
  ends
; v3 (Windows 3.x)
  Compression	 dd ? ; Compression method used
  SizeImage	 dd ? ; Size of bitmap in bytes
  XPelsPerMeter  dd ? ; Horizontal resolution in pixels per meter
  YPelsPerMeter  dd ? ; Vertical resolution in pixels per meter
  ClrUsed	 dd ? ; Number of colors in the image
  ClrImportant	 dd ? ; Minimum number of important colors
  union
    Palette	 dd ? ; Image palette if BitCount in [1,4,8]
; v4 (Windows 95)
    struct
      RedMask	 dd ? ; Mask identifying bits of red component
      GreenMask  dd ? ; Mask identifying bits of green component
      BlueMask	 dd ? ; Mask identifying bits of blue component
      AlphaMask  dd ? ; Mask identifying bits of alpha component
      CSType	 dd ? ; Color space type
      RedX	 dd ? ; X coordinate of red endpoint
      RedY	 dd ? ; Y coordinate of red endpoint
      RedZ	 dd ? ; Z coordinate of red endpoint
      GreenX	 dd ? ; X coordinate of green endpoint
      GreenY	 dd ? ; Y coordinate of green endpoint
      GreenZ	 dd ? ; Z coordinate of green endpoint
      BlueX	 dd ? ; X coordinate of blue endpoint
      BlueY	 dd ? ; Y coordinate of blue endpoint
      BlueZ	 dd ? ; Z coordinate of blue endpoint
      GammaRed	 dd ? ; Gamma red coordinate scale value
      GammaGreen dd ? ; Gamma green coordinate scale value
      GammaBlue  dd ? ; Gamma blue coordinate scale value
    ends
  ends
ends

define bmp.BI_RGB	0
define bmp.BI_RLE8	1
define bmp.BI_RLE4	2
define bmp.BI_BITFIELDS 3
define bmp.BI_JPEG	4
define bmp.BI_PNG	5

struct bmp.Header
  file bmp.FileHeader
  info bmp.InfoHeader
ends

struct bmp.RgbByteQuad
  Red	db ?
  Green db ?
  Blue	db ?
  Alpha db ?
ends

struct bmp.RgbQuad
  Red	dd ?
  Green dd ?
  Blue	dd ?
  Alpha dd ?
ends

;;------------------------------------------------------------------------------------------------;;

struct bmp.Image
  info bmp.InfoHeader
ends
