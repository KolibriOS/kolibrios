;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

struc VBE_VGAInfo {
  .VESASignature	  dd ?	  ; char
  .VESAVersion		  dw ?	  ; short
  .OemStringPtr 	  dd ?	  ; char *
  .Capabilities 	  dd ?	  ; ulong
  .VideoModePtr 	  dd ?	  ; ulong
  .TotalMemory		  dw ?	  ; short
  ; VBE 2.0+
  .OemSoftwareRev	  db ?	  ; short
  .OemVendorNamePtr	  dw ?	  ; char *
  .OemProductNamePtr	  dw ?	  ; char *
  .OemProductRevPtr	  dw ?	  ; char *
  .reserved		  rb 222  ; char
  .OemData		  rb 256  ; char
}

struc VBE_ModeInfo {
  .ModeAttributes	  dw ?	  ; short
  .WinAAttributes	  db ?	  ; char
  .WinBAttributes	  db ?	  ; char
  .WinGranularity	  dw ?	  ; short
  .WinSize		  dw ?	  ; short
  .WinASegment		  dw ?	  ; ushort
  .WinBSegment		  dw ?	  ; ushort
  .WinFuncPtr		  dd ?	  ; void *
  .BytesPerScanLine	  dw ?	  ; short
  .XRes 		  dw ?	  ; short
  .YRes 		  dw ?	  ; short
  .XCharSize		  db ?	  ; char
  .YCharSize		  db ?	  ; char
  .NumberOfPlanes	  db ?	  ; char
  .BitsPerPixel 	  db ?	  ; char
  .NumberOfBanks	  db ?	  ; char
  .MemoryModel		  db ?	  ; char
  .BankSize		  db ?	  ; char
  .NumberOfImagePages	  db ?	  ; char
  .res1 		  db ?	  ; char
  .RedMaskSize		  db ?	  ; char
  .RedFieldPosition	  db ?	  ; char
  .GreenMaskSize	  db ?	  ; char
  .GreenFieldPosition	  db ?	  ; char
  .BlueMaskSize 	  db ?	  ; char
  .BlueFieldPosition	  db ?	  ; char
  .RsvedMaskSize	  db ?	  ; char
  .RsvedFieldPosition	  db ?	  ; char
  .DirectColorModeInfo	  db ?	  ; char ; MISSED IN THIS TUTORIAL!! SEE ABOVE
  ; VBE 2.0+
  .PhysBasePtr		  dd ?	  ; ulong
  .OffScreenMemOffset	  dd ?	  ; ulong
  .OffScreenMemSize	  dw ?	  ; short
  ; VBE 3.0+
  .LinbytesPerScanLine	  dw ?	  ; short
  .BankNumberOfImagePages db ?	  ; char
  .LinNumberOfImagePages  db ?	  ; char
  .LinRedMaskSize	  db ?	  ; char
  .LinRedFieldPosition	  db ?	  ; char
  .LingreenMaskSize	  db ?	  ; char
  .LinGreenFieldPosition  db ?	  ; char
  .LinBlueMaskSize	  db ?	  ; char
  .LinBlueFieldPosition   db ?	  ; char
  .LinRsvdMaskSize	  db ?	  ; char
  .LinRsvdFieldPosition   db ?	  ; char
  .MaxPixelClock	  dd ?	  ; ulong
  .res2 		  rb 190  ; char
}

virtual at $A000
  vi VBE_VGAInfo
  mi VBE_ModeInfo
modes_table:
end virtual
cursor_pos  dw 0	 ;временное хранение курсора.
home_cursor dw 0    ;current shows rows a table
end_cursor  dw 0     ;end of position current shows rows a table
scroll_start dw 0    ;start position of scroll bar
scroll_end  dw 0     ;end position of scroll bar
long_v_table equ 9   ;long of visible video table
size_of_step equ 10
scroll_area_size equ (long_v_table-2)
int2str:
	dec	bl
	jz	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	int2str
	pop	eax
    @@: or	al,0x30
	mov	[ds:di],al
	inc	di
	ret

int2strnz:
	cmp	eax,ecx
	jb	@f
	xor	edx,edx
	div	ecx
	push	edx
	call	int2strnz
	pop	eax
    @@: or	al,0x30
	mov	[es:di],al
	inc	di
	ret


;-------------------------------------------------------
print_vesa_info:

	mov	[es:vi.VESASignature],'VBE2'
	mov	ax,0x4F00
	mov	di,vi	   ;0xa000
	int	0x10
	or	ah,ah
	jz	@f
	mov	[es:vi.VESASignature],'VESA'
	mov	ax,$4F00
	mov	di,vi
	int	0x10
	or	ah,ah
	jnz	$
  @@:
	cmp	[es:vi.VESASignature],'VESA'
	jne	$
	cmp	[es:vi.VESAVersion],0x0100
	jb	$

  .vesaok2:

	ret
;-----------------------------------------------------------------------------

calc_vmodes_table:
	pushad

;        push    0
;        pop     es

	lfs	si, [es:vi.VideoModePtr]

	mov	bx,modes_table

  .next_mode:
	mov	cx,word [fs:si] ; mode number
	cmp	cx,-1
	je	.modes_ok.2

	mov	ax,0x4F01
	mov	di,mi
	int	0x10

	or	ah,ah
	jnz	.modes_ok.2		;vesa_info.exit

	test	[es:mi.ModeAttributes],00000001b   ;videomode support ?
	jz	@f
	test	[es:mi.ModeAttributes],00010000b   ;picture ?
	jz	@f
	test	[es:mi.ModeAttributes],10000000b   ;LFB ?
	jz	@f

	cmp	[es:mi.BitsPerPixel], 32	;to show only 32 bpp videomodes
	jb	@f

.l0:
	cmp	[es:mi.XRes],800	; only 800x600 and higher
	jb	@f

	mov	ax,[es:mi.XRes]
	mov	[es:bx+0],ax		   ; +0[2] : resolution X
	mov	ax,[es:mi.YRes]
	mov	[es:bx+2],ax		   ; +2[2] : resolution Y
	mov	ax,[es:mi.ModeAttributes]
	mov	[es:bx+4],ax		   ; +4[2] : attributes

;<<        cmp     [s_vesa.ver],'2'
;<<        jb      .lp1

	or	cx,0x4000			; use LFB <<< ?
.lp1:	mov	[es:bx+6],cx		   ; +6 : mode number
	movzx	ax,byte [es:mi.BitsPerPixel]
	mov	word [es:bx+8],ax		; +8 : bits per pixel << ?
	add	bx,size_of_step 		; size of record

    @@:
	add	si,2
	jmp	.next_mode

  .modes_ok.2:

	mov	word[es:bx],-1	;end video table
	mov	word[end_cursor],bx	;save end cursor position
	popad
	ret

check_first_parm:
	mov	si,word [preboot_graph]
	test	si,si
	jnz	 .no_zero	 ;if no zero
.zerro:
	mov	word[preboot_graph], ax

	mov	ax,1024
	mov	bx,768
	mov	si,modes_table
	call	.loops
	test	ax,ax
	jz     .ok_found_mode

	mov	si,modes_table
	jmp	.ok_found_mode

.no_zero:
	mov	bp,word [number_vm]
	cmp	bp,word [es:si+6]
	jz	.ok_found_mode
	mov	ax,word [x_save]
	mov	bx,word [y_save]
	mov	si,modes_table
	call	.loops
	test	ax,ax
	jz     .ok_found_mode

	mov    si,modes_table

.ok_found_mode:
	mov	word [home_cursor],si
	mov	word [preboot_graph],si
	mov	ax,si

	mov	ecx,long_v_table

.loop:	add	ax,size_of_step
	cmp	ax,word [end_cursor]
	jae	.next_step
	loop	.loop
.next_step:
	sub	ax,size_of_step*long_v_table
	cmp	ax,modes_table
	jae	@f
	mov	ax,modes_table
@@:

	mov	word [home_cursor],ax
	mov	si,[preboot_graph]
	mov	word [cursor_pos],si

	push	word [es:si]
	pop	word [x_save]
	push	word [es:si+2]
	pop	word [y_save]
	push	word [es:si+6]
	pop	word [number_vm]

	ret
;;;;;;;;;;;;;;;;;;;;;;;;;;;
.loops:
	cmp	ax,word [es:si]
	jne	.next
	cmp	bx,word [es:si+2]
	jne	.next
	je	.ok
.next:	add	si,size_of_step
	cmp	word [es:si],-1
	je	.exit
	jmp	.loops
.ok:	xor	ax,ax
	ret
.exit:	or	ax,-1
	ret


;-----------------------------------------------------------------------------

set_vmode:
	push	0 ;0;x1000
	pop	es

	mov	si,word [preboot_graph] 	   ;[preboot_graph]
	mov	cx,word [es:si+6]	     ; number of mode


	mov	ax,word [es:si+0]	     ; resolution X
	mov	bx,word [es:si+2]	     ; resolution Y


	mov	word [es:0x900A],ax		 ; resolution X
	mov	word [es:0x900C],bx		 ; resolution Y
	mov	word [es:0x9008],cx		 ; number of mode


;  VESA 2 and Vesa 3 only

	mov	ax,0x4f01
	and	cx,0xfff
	mov	di,mi;0xa000
	int	0x10
	; LFB
	mov	eax,[es:mi.PhysBasePtr] 	;di+0x28]
	mov	[es:0x9018],eax
	; ---- vbe voodoo
	BytesPerLine equ 0x10
	mov	ax, [es:di+BytesPerLine]
	mov	[es:0x9001], ax
	; BPP
.l0:
	mov	al, byte [es:di+0x19]
	mov	[es:0x9000], al
	jmp	.exit

  .exit:
	ret


;=============================================================================

