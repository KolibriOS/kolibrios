;;================================================================================================;;
;;//// z80.asm //// (c) Nable, 2007-2008 /////////////////////////////////////////////////////////;;
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
;;                                                                                                ;;
;; References:                                                                                    ;;
;;   1.                                                                                           ;;
;;                                                                                                ;;
;;================================================================================================;;

include 'z80.inc'

;;================================================================================================;;
proc img.is.z80 _data, _length ;//////////////////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Determine if raw data could be decoded (is in z80 screen format)                               ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = false / true                                                                             ;;
;;================================================================================================;;
	xor eax,eax
	cmp [_length],6929
	setz al
	je @f
	cmp [_length],6912
	setz al
@@:	
	ret
endp

;;================================================================================================;;
proc img.decode.z80 _data, _length, _options ;////////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Decode data into image if it contains correctly formed raw data in z80 screen format           ;;
;;------------------------------------------------------------------------------------------------;;
;> _data = raw data as read from file/stream                                                      ;;
;> _length = data length                                                                          ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to image                                                            ;;
;;================================================================================================;;
;---------------------------------------------------------------------------------------------------
;During the decoding:
;bl - PixelLeft (this means how much pixels left to put in current string)
;bh - CurrentString
;High half of ebx - use DualStos (two frames per one pixel_write)
;cl - PixelColorIndexInPalette
;ch - BackgroundColorIndexInPalette
;High half of ecx - blinking flag
;edx - address of current attribute byte
;---------------------------------------------------------------------------------------------------
locals
  frame1	   dd ?
  OffsetIn2ndFrame dd ?
endl
	xor	eax,eax
	pushad
	cld						;paranoia
	stdcall img.create,256,192,Image.bpp8i
	test eax,eax
	jz	img.decode.z80.locret			;test if allocation failed
	mov	[frame1],eax
	mov	esi,z80._._16color_palette
	mov	ecx,16
	mov	edi,[eax+Image.Palette]
	rep movsd				;write palette for the first frame
	mov	esi,[_data]
	cmp	[_length],6929
    jne  @f
    add  esi,17 ;in case of 6929 byte files we just skip the info in the begininning.
@@:
;---------------------------------------------------------------------------------------------------
;At first we'll determine if there are any blinking pixels
;if no - we'll produce statical single image
    mov  ecx,768
    lea  edx,[esi+6912-768];edx points to attribute area
    xor  ebx,ebx	  ;begin from <0,0> (for further decoding)
@@:
    test byte[edx+ecx-1],z80.BlinkFlag ;such addressing is a good optimisation
					;(as I hope), edx is unchanged
    jnz  .decode_z80_with_blinking
    loop @b
.decode_z80_without_blinking:
    jmp  .decode_z80_main_stage

.decode_z80_with_blinking:
    or	 ebx,0xFFFF0000 	;use DualStos
	mov	ecx,eax 			;eax still points to the first frame
	stdcall img.create,256,192,Image.bpp8i
	test eax,eax
	jz	img.decode.z80.failed
	mov	[eax+Image.Previous],ecx
	mov	[ecx+Image.Next],eax
	mov	esi,z80._._16color_palette
	mov	ecx,16
	mov	edi,[eax+Image.Palette]
	rep movsd				;write palette for the second frame
	mov eax,[eax+Image.Data]
	mov	[OffsetIn2ndFrame],eax
;-------------------------------------------------------------------------------
.decode_z80_main_stage:
;2nd stage - convert z80 screen to 8bpp image with palette
.decode_z80_main_stage_main_loop:
    test bl,7
    jnz  .decode_z80_main_stage_put_now

._z80_update_attributes:
    movsx ecx,byte[edx]   ;note that BlinkFlag is the highest bit in attribute
			  ;byte, so ecx's highest bit is set automatically
    shl  ecx,5
    shr  cl,5
    and  ch,0xF
    test ch,1000b
    jz	 @f
    or	 ecx,1000b	  ;it has the same size with 'or cl,1000b' but could be faster
@@:
    inc  edx

    lodsb
    mov  ah,al

.decode_z80_main_stage_put_now:
    shl  ah,1
;-------------------------------------------------------------------------------
._z80_put_pixel:
;In: CF - put pixel with color CL, !CF - pixel with color CH
;High parts of ebx and ecx - as described above
    mov  al,cl		  ;'mov' doesn't affect flags
    jc	 @f
    mov  al,ch
@@:
    stosb		  ;'stosb' doesn't affect flags

    test ebx,ebx
    jns  @f
    test ecx,ecx
    jns  .1
    mov  al,ch
.1:
	xchg [OffsetIn2ndFrame],edi
	stosb
	xchg [OffsetIn2ndFrame],edi
@@:
    inc  bl		  ;next pixel
    jz	 .decode_z80_main_stage_row_finished
    jmp  .decode_z80_main_stage_main_loop
;-------------------------------------------------------------------------------
.decode_z80_main_stage_row_finished:
    cmp  bh,191 	  ;is image finished?
    jb	 .decode_z80_main_stage_image_not_finished ;no.
.decode_z80_finish:
    jmp  .locret	  ;now really finished
;-------------------------------------------------------------------------------
;or not finished yet. Branch according to a row number (see documentation)
.decode_z80_main_stage_next_third:
    sub  bh,7
    sub  edi,256*(8-1)
    jmp  .decode_z80_main_stage_main_loop

.decode_z80_main_stage_image_not_finished:
;next row
    add  bh,8		  ;refer to documentation
    add  edi,256*(8-1)

;if finished row is 63 or 127 then we process next third of the image
    cmp  bh,63+8
    je	 .decode_z80_main_stage_next_third
    cmp  bh,127+8
    je	 .decode_z80_main_stage_next_third

    cmp  bh,56+8	  ;if finished row in [56;63) or [120;127) or [184;191)
    jb	 .decode_z80_main_stage_main_loop
    cmp  bh,63+8
    jb	 .4
    cmp  bh,120+8
    jb	 .decode_z80_main_stage_main_loop
    cmp  bh,127+8
    jb	 .4
    cmp  bh,184+8
    jb	 .decode_z80_main_stage_main_loop
;note that if we are here then bh is < 191 (see label .2) but >= 184+8
.4:
;and if we here then bh is in [56;63) or [120;127) or [184;191)
    sub  bh,(8+56-1)
    sub  edi,256*(8+56-1)
    sub  edx,z80.AttrString*8
    jmp  .decode_z80_main_stage_main_loop
img.decode.z80.locret:
	popad
	ret
img.decode.z80.failed:
	stdcall img.destroy,[frame1]
	jmp    img.decode.z80.locret
endp

;;================================================================================================;;
proc img.encode.z80 _img, _p_length, _options ;///////////////////////////////////////////////////;;
;;------------------------------------------------------------------------------------------------;;
;? Encode image into raw data in z80 screen format                                                ;;
;;------------------------------------------------------------------------------------------------;;
;> _img = pointer to image                                                                        ;;
;;------------------------------------------------------------------------------------------------;;
;< eax = 0 (error) or pointer to encoded data                                                     ;;
;< _p_length = encoded data length                                                                ;;
;;================================================================================================;;
	xor	eax, eax
	ret
endp


;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below are private procs you should never call directly from your code                          ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;

;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
;! Below is private data you should never use directly from your code                             ;;
;;================================================================================================;;
;;////////////////////////////////////////////////////////////////////////////////////////////////;;
;;================================================================================================;;
z80._._16color_palette:
dd	0			; black
dd	0x000000b0	; blue
dd	0x00b00000	; red
dd	0x00b000b0	; magenta
dd	0x0000b000	; green
dd	0x0000b0b0	; cyan
dd	0x00b0b000	; yellow
dd	0x00b0b0b0	; gray
dd	0			; black
dd	0x000000ff	; light blue
dd	0x00ff0000	; light red
dd	0x00ff00ff	; light magenta
dd	0x0000ff00	; light green
dd	0x0000ffff	; light cyan
dd	0x00ffff00	; light yellow
dd	0x00ffffff	; white
