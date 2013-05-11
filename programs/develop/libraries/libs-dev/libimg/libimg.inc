;;================================================================================================;;
;;//// libimg.inc //// (c) mike.dld, 2007-2008, (c) diamond, 2009, (c) dunkaist, 2011-2013 ///////;;
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

; list of format id's
LIBIMG_FORMAT_BMP	= 1
LIBIMG_FORMAT_ICO	= 2
LIBIMG_FORMAT_CUR	= 3
LIBIMG_FORMAT_GIF	= 4
LIBIMG_FORMAT_PNG	= 5
LIBIMG_FORMAT_JPEG	= 6
LIBIMG_FORMAT_TGA	= 7
LIBIMG_FORMAT_PCX	= 8
LIBIMG_FORMAT_XCF	= 9
LIBIMG_FORMAT_TIFF	= 10
LIBIMG_FORMAT_PNM	= 11
LIBIMG_FORMAT_WBMP	= 12
LIBIMG_FORMAT_XBM	= 13
LIBIMG_FORMAT_Z80	= 14

; scale type                    ; corresponding img.scale params
LIBIMG_SCALE_INTEGER	= 1     ; scale factor ; reserved 0
LIBIMG_SCALE_TILE	= 2     ; new width    ; new height
LIBIMG_SCALE_STRETCH	= 3     ; new width    ; new height
LIBIMG_SCALE_FIT_RECT	= 4     ; new width    ; new height
LIBIMG_SCALE_FIT_WIDTH	= 5     ; new width    ; new height
LIBIMG_SCALE_FIT_HEIGHT	= 6     ; new width    ; new height
LIBIMG_SCALE_FIT_MAX	= 7     ; new width    ; new height

; interpolation algorithm
LIBIMG_INTER_NONE	= 0     ; use it with LIBIMG_SCALE_INTEGER, LIBIMG_SCALE_TILE, etc
LIBIMG_INTER_BILINEAR	= 1
;LIBIMG_INTER_BICUBIC	= 2
;LIBIMG_INTER_LANCZOS	= 3
LIBIMG_INTER_DEFAULT	= LIBIMG_INTER_BILINEAR

; error codes
LIBIMG_ERROR_OUT_OF_MEMORY	= 1
LIBIMG_ERROR_FORMAT		= 2
LIBIMG_ERROR_CONDITIONS		= 3
LIBIMG_ERROR_BIT_DEPTH		= 4
LIBIMG_ERROR_ENCODER		= 5
LIBIMG_ERROR_SRC_TYPE		= 6
LIBIMG_ERROR_SCALE		= 7
LIBIMG_ERROR_INTER		= 8
LIBIMG_ERROR_NOT_INPLEMENTED	= 9
LIBIMG_ERROR_INVALID_INPUT      = 10

; encode flags (byte 0x02 of _common option)
LIBIMG_ENCODE_STRICT_SPECIFIC	= 0x01
LIBIMG_ENCODE_STRICT_BIT_DEPTH	= 0x02
LIBIMG_ENCODE_DELETE_ALPHA	= 0x08
LIBIMG_ENCODE_FLUSH_ALPHA	= 0x10

; convert flags
; TBD

struct FormatsTableEntry
  Format_id	dd ?
  Is		dd ?
  Decode	dd ?
  Encode	dd ?
  Capabilities	dd ?
ends

struct Image
  Checksum dd ? ; ((Width ROL 16) OR Height) XOR Data[0]	; ignored so far
  Width    dd ?
  Height   dd ?
  Next     dd ?
  Previous dd ?
  Type     dd ? ; one of Image.bppN
  Data     dd ?
  Palette  dd ? ; used iff Type eq Image.bpp1, Image.bpp2, Image.bpp4 or Image.bpp8i
  Extended dd ?
  Flags    dd ? ; bitfield
  Delay    dd ? ; used iff Image.IsAnimated is set in Flags
ends

; values for Image.Type
; must be consecutive to allow fast switch on Image.Type in support functions
Image.bpp8i = 1  ; indexed
Image.bpp24 = 2
Image.bpp32 = 3
Image.bpp15 = 4
Image.bpp16 = 5
Image.bpp1  = 6
Image.bpp8g = 7  ; grayscale
Image.bpp2i = 8
Image.bpp4i = 9
Image.bpp8a = 10  ; grayscale with alpha channel; application layer only!!! kernel doesn't handle this image type, libimg can only create and destroy such images

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
