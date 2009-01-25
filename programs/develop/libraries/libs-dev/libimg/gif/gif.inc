;;================================================================================================;;
;;//// gif.inc //// (c) mike.dld, 2007-2008 //////////////////////////////////////////////////////;;
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


struct gif.FileHeader ; GIF87a
  Signature	  db 3 dup(?) ; Header Signature (always "GIF")
  Version	  db 3 dup(?) ; GIF format version("87a" or "89a")
ends

struct gif.LogicalScreenDescriptor ; GIF87a
  ScreenWidth	  dw ?	      ; Width of Display Screen in Pixels
  ScreenHeight	  dw ?	      ; Height of Display Screen in Pixels
  Packed	  db ?	      ; Screen and Color Map Information
  BackgroundColor db ?	      ; Background Color Index
  AspectRatio	  db ?	      ; Pixel Aspect Ratio
ends

gif.LSD.Packed.SizeOfGlobalColorTableMask  = 000000111b
gif.LSD.Packed.SizeOfGlobalColorTableShift = 0
gif.LSD.Packed.ColorTableSortFlag	   = 000001000b
gif.LSD.Packed.ColorTableSortShift	   = 3
gif.LSD.Packed.ColorResolutionMask	   = 001110000b
gif.LSD.Packed.ColorResolutionShift	   = 4
gif.LSD.Packed.GlobalColorTableFlag	   = 010000000b
gif.LSD.Packed.GlobalColorTableShift	   = 7

struct gif.Header
  file gif.FileHeader
  lsd  gif.LogicalScreenDescriptor
ends

struct gif.RgbTriplet ; GIF87a
  Red	db ? ; Red Color Element
  Green db ? ; Green Color Element
  Blue	db ? ; Blue Color Element
ends

struct gif.Block
  Introducer db ?
ends

gif.Block.Introducer.EndOfData	     = 0x00
gif.Block.Introducer.Extension	     = 0x21
gif.Block.Introducer.ImageDescriptor = 0x2C
gif.Block.Introducer.EndOfFile	     = 0x3B

struct gif.ImageDescriptor ; GIF87a
  b	     gif.Block ; Introducer = 2Ch (',')
  Left	     dw ?      ; X position of image on the display
  Top	     dw ?      ; Y position of image on the display
  Width      dw ?      ; Width of the image in pixels
  Height     dw ?      ; Height of the image in pixels
  Packed     db ?      ; Image and Color Table Data Information
ends

gif.ID.Packed.SizeOfLocalColorTableMask  = 000000111b
gif.ID.Packed.SizeOfLocalColorTableShift = 0
gif.ID.Packed.SortFlag			 = 000100000b
gif.ID.Packed.SortShift 		 = 5
gif.ID.Packed.InterleaceFlag		 = 001000000b
gif.ID.Packed.InterleaceShift		 = 6
gif.ID.Packed.LocalColorTableFlag	 = 010000000b
gif.ID.Packed.LocalColorTableShift	 = 7

struct gif.Extension
  b	     gif.Block ; Introducer = 21h ('|')
  Label      db ?      ; Extension label
ends

gif.Extension.Label.PlainText	    = 0x01
gif.Extension.Label.GraphicsControl = 0xF9
gif.Extension.Label.Comment	    = 0xFE
gif.Extension.Label.Application     = 0xFF

struct gif.PlainTextExtension ; GIF89a
  e		   gif.Extension ; Label = 01h
  BlockSize	   db ? 	 ; Size of Extension Block (always 0Ch)
  TextGridLeft	   dw ? 	 ; X position of text grid in pixels
  TextGridTop	   dw ? 	 ; Y position of text grid in pixels
  TextGridWidth    dw ? 	 ; Width of the text grid in pixels
  TextGridHeight   dw ? 	 ; Height of the text grid in pixels
  CellWidth	   db ? 	 ; Width of a grid cell in pixels
  CellHeight	   db ? 	 ; Height of a grid cell in pixels
  TextFgColorIndex db ? 	 ; Text foreground color index value
  TextBgColorIndex db ? 	 ; Text background color index value
  PlainTextData    db ? 	 ; The Plain Text data (*)
; Terminator       db ?          ; Block Terminator (always 0)
ends

struct gif.GraphicsControlExtension ; GIF89a
  e	     gif.Extension ; Label = F9h
  BlockSize  db ?	   ; Size of remaining fields (always 04h)
  Packed     db ?	   ; Method of graphics disposal to use
  DelayTime  dw ?	   ; Hundredths of seconds to wait
  ColorIndex db ?	   ; Transparent Color Index
; Terminator db ?          ; Block Terminator (always 0)
ends

struct gif.CommentExtension ; GIF89a
  e	      gif.Extension ; Label = FEh
  CommentData db ?	    ; Pointer to Comment Data sub-blocks (*)
; Terminator  db ?          ; Block Terminator (always 0)
ends

struct gif.ApplicationExtension ; GIF89a
  e		  gif.Extension ; Label = FFh
  BlockSize	  db ?		; Size of Extension Block (always 0Bh)
  Identifier	  db 8 dup(?)	; Application Identifier
  AuthentCode	  db 3 dup(?)	; Application Authentication Code
  ApplicationData db ?		; Point to Application Data sub-blocks (*)
; Terminator      db ?          ; Block Terminator (always 0)
ends

;;------------------------------------------------------------------------------------------------;;

struct gif.Image
  gce  gif.GraphicsControlExtension
  info gif.ImageDescriptor
ends

gif.Null equ 0x1000
