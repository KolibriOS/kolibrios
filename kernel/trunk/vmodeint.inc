;
; Call of videomode driver's functions
;
; (Add in System function 21 (and/or 26) as a subfunction 13)
;
; Author: Trans
; Date:  19.07.2003
;
; Include in MeOS kernel and compile with FASM
;

uglobal
  old_screen_width  dd ?
  old_screen_height dd ?
endg

     cmp  eax,13                 ; CALL VIDEOMODE DRIVER FUNCTIONS
     jne  .no_vmode_drv_access
	pushd	[0x0000fe00] [0x0000fe04]
	popd	[old_screen_height] [old_screen_width]
     or   eax,-1                 ; If driver is absent then eax does not change
     call 0x760100               ; Entry point of video driver
     mov  [esp+36],eax
     mov  [esp+24],ebx
     mov  [esp+32],ecx
;    mov  [esp+28],edx
	mov	eax,[old_screen_width]
	mov	ebx,[old_screen_height]
	sub	eax,[0x0000fe00]
	jnz	@f
	sub	ebx,[0x0000fe04]
	jz	.resolution_wasnt_changed
	jmp	.lp1
    @@: sub	ebx,[0x0000fe04]
  .lp1: sub	[screen_workarea.right],eax
	sub	[screen_workarea.bottom],ebx

	call	repos_windows
	call	calculatescreen

  .resolution_wasnt_changed:
     ret
   .no_vmode_drv_access:
