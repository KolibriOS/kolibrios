;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


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
        pushd   [Screen_Max_X] [Screen_Max_Y]
	popd	[old_screen_height] [old_screen_width]
     or   eax,-1                 ; If driver is absent then eax does not change
     call (VMODE_BASE+0x100)     ; Entry point of video driver
     mov  [esp+36],eax
     mov  [esp+24],ebx
     mov  [esp+32],ecx
;    mov  [esp+28],edx
	mov	eax,[old_screen_width]
	mov	ebx,[old_screen_height]
        sub     eax,[Screen_Max_X]
	jnz	@f
        sub     ebx,[Screen_Max_Y]
	jz	.resolution_wasnt_changed
	jmp	.lp1
    @@: sub     ebx,[Screen_Max_Y]
  .lp1: sub	[screen_workarea.right],eax
	sub	[screen_workarea.bottom],ebx

	call	repos_windows
        mov     eax, 0
        mov     ebx, 0
        mov     ecx, [Screen_Max_X]
        mov     edx, [Screen_Max_Y]
	call	calculatescreen

  .resolution_wasnt_changed:
     ret
   .no_vmode_drv_access:
