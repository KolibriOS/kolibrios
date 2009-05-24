;;================================================================================================;;
;;//// tga.inc //// (c) Nable, 2007-2008 /////////////////////////////////////////////////////////;;
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

struct tga.FileHeader
  CommentLength	db ?
  ColormapType	db ?
  SubType		db ?
  DontKnow1		dw ?
  ColormapSize	dw ?
  ColormapBpp	db ?
  DontKnow2		dd ?
  Width			dw ?
  Height		dw ?
  BitPerPixel	db ?
  DontKnow3		db ?
ends

tga.flags.top_down_row_order equ 32 ;bit5
tga.flags.interlace_type equ 192;bits6/7
