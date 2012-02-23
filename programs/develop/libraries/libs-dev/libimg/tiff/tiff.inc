;;================================================================================================;;
;;//// tiff.inc //// (c) dunkaist, 2011-2012 /////////////////////////////////////////////////////;;
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

struct tiff_header
	magic_1			rw	1
	magic_2			rw	1
	first_IFD		rd	1
ends

struct tiff_extra
	image_width			rd	1	; SHORT or LONG
	image_height			rd	1	; SHORT or LONG
	bits_per_sample			rd	1	; SHORT
	samples_per_pixel		rd	1	; SHORT
	compression			rd	1	; SHORT
	photometric			rd	1	; SHORT
	offsets_number			rd	1
	strip_offsets			rd	1	; SHORT or LONG
	strip_offsets_length		rd	1
	rows_per_strip			rd	1	; SHORT or LONG
	strip_byte_counts		rd	1	; LONG or SHORT
	strip_byte_counts_length	rd	1
	palette				rd	1	; SHORT
	palette_size			rd	1	; in colors, not samples
ends


TIFF.IFDE_TYPE.BYTE		=	1
TIFF.IFDE_TYPE.ASCII		=	2
TIFF.IFDE_TYPE.SHORT		=	3
TIFF.IFDE_TYPE.LONG		=	4
TIFF.IFDE_TYPE.RATIONAL		=	5
TIFF.IFDE_TYPE.SBYTE		=	6
TIFF.IFDE_TYPE.UNDEFINED	=	7
TIFF.IFDE_TYPE.SSHORT		=	8
TIFF.IFDE_TYPE.SLONG		=	9
TIFF.IFDE_TYPE.SRATIONAL	=	10
TIFF.IFDE_TYPE.FLOAT		=	11
TIFF.IFDE_TYPE.DOUBLE		=	12

TIFF.IFDE_TYPE_LENGTH.BYTE	=	1
TIFF.IFDE_TYPE_LENGTH.ASCII	=	1
TIFF.IFDE_TYPE_LENGTH.SHORT	=	2
TIFF.IFDE_TYPE_LENGTH.LONG	=	4
TIFF.IFDE_TYPE_LENGTH.RATIONAL	=	8
TIFF.IFDE_TYPE_LENGTH.SBYTE	=	1
TIFF.IFDE_TYPE_LENGTH.UNDEFINED	=	1
TIFF.IFDE_TYPE_LENGTH.SSHORT	=	2
TIFF.IFDE_TYPE_LENGTH.SLONG	=	4
TIFF.IFDE_TYPE_LENGTH.SRATIONAL	=	8
TIFF.IFDE_TYPE_LENGTH.FLOAT	=	4
TIFF.IFDE_TYPE_LENGTH.DOUBLE	=	8

TIFF.COMPRESSION.UNCOMPRESSED	=	1
TIFF.COMPRESSION.CCITT1D	=	2
TIFF.COMPRESSION.GROUP3FAX	=	3
TIFF.COMPRESSION.GROUP4FAX	=	4
TIFF.COMPRESSION.LZW		=	5
TIFF.COMPRESSION.JPEG		=	6
TIFF.COMPRESSION.PACKBITS	=	32773

TIFF.PHOTOMETRIC.WHITE_IS_ZERO	=	0
TIFF.PHOTOMETRIC.BLACK_IS_ZERO	=	1
TIFF.PHOTOMETRIC.RGB		=	2
TIFF.PHOTOMETRIC.RGB_PALETTE	=	3
TIFF.PHOTOMETRIC.MASK		=	4
TIFF.PHOTOMETRIC.CMYK		=	5
TIFF.PHOTOMETRIC.YCbCr		=	6
TIFF.PHOTOMETRIC.CIELAB		=	8


macro	lodsw_
{
	stdcall	tiff._.get_word, [_endianness]
}

macro	lodsd_
{
	stdcall	tiff._.get_dword, [_endianness]
}

