;******************************************************************************
;   MAIN MENU
;******************************************************************************
; last update:  17/04/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Support for boot parameters.
;------------------------------------------------------------------------------
; last update:  22/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Global optimization! The program uses
;               only 32 KB of memory instead of 128 kb is now.
;------------------------------------------------------------------------------
; last update:  19/09/2011
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      Checking for program exist to memory
;               Added processing of keys: left and right arrow
;------------------------------------------------------------------------------
;   MAIN MENU by lisovin@26.ru
;   Some parts of code rewritten by Ivan Poddubny <ivan-yar@bk.ru>
;
;   Compile with FASM for Kolibri
;******************************************************************************
  BTN_HEIGHT         = 26
  BTN_WIDTH          = 198 ; 178 for a small font
  TXT_Y              = (BTN_HEIGHT)/2-7
  FONT_TYPE          = 0x90000000

  PANEL_HEIGHT       = 20
  MENU_BOTTON_X_POS  = 10
  MENU_BOTTON_X_SIZE = 50
;------------------------------------------------------------------------------
	use32
	org 0x0

	db 'MENUET01'  ; 8 byte id
	dd 0x01        ; header version
	dd START       ; start of code
	dd IM_END      ; size of image
	dd mem_end     ; memory for app
	dd stack_area  ; esp
	dd bootparam   ; boot parameters
	dd 0x0         ; path
;------------------------------------------------------------------------------
include "..\..\..\macros.inc"
include "..\..\..\gui_patterns.inc"
; Formatted debug output:
include "..\..\..\debug-fdo.inc"
__DEBUG__       = 1             ; 0 - disable debug output / 1 - enable debug output
__DEBUG_LEVEL__ = DBG_ERR      ; set the debug level
DBG_ALL       = 0  ; all messages
DBG_INFO      = 1  ; info and errors
DBG_ERR       = 2  ; only errors
;------------------------------------------------------------------------------
align 4
conversion_ASCII_to_HEX:
	xor	ebx,ebx
	cld
	lodsd
	mov	ecx,4
;--------------------------------------
align 4
.loop:
	cmp	al,0x60	; check for ABCDEF
	ja	@f
	sub	al,0x30 ; 0-9
	jmp	.store
;--------------------------------------
align 4
@@:
	sub	al,0x57 ; A-F
;--------------------------------------
align 4
.store:
	and	al,0xf
	rol	ebx,4
	add	bl,al
	ror	eax,8
	dec	ecx
	jnz	.loop

	ret
;------------------------------------------------------------------------------
align 4
START:		       ; start of execution
	mcall	68,11
	
	mcall 30, 1, default_dir

	; DEBUGF DBG_INFO, "MENU START! sc.work = %x\n", [sc.work]

	mov	esi,bootparam	
	cmp	[esi],byte 0
	je	.no_boot_parameters
; boot params - hex
; db '9999'	; +0	Menu button X
; db '9999'	; +4	Menu button X size
; db '9999'	; +8	Menu button Y
; db '9999'	; +12	Menu button Y size
; db '9999'	; +16	Panel height
; db '1000'	; +20	Panel attachment

;	mov	edx,bootparam
;	call	debug_outstr
;	newline

	call	conversion_ASCII_to_HEX
	mov	[menu_button_x.start],ebx
	
;	dps	"menu_button_x.start: "
;	dpd	ebx
;	newline

	call	conversion_ASCII_to_HEX
	mov	[menu_button_x.size],ebx

;	dps	"menu_button_x.size: "
;	dpd	ebx
;	newline

	call	conversion_ASCII_to_HEX
	mov	[menu_button_y.start],ebx

;	dps	"menu_button_y.start: "
;	dpd	ebx
;	newline
	
	call	conversion_ASCII_to_HEX
	mov	[menu_button_y.size],ebx
	
;	dps	"menu_button_y.size: "
;	dpd	ebx
;	newline
	
	call	conversion_ASCII_to_HEX
	mov	[panel_height],ebx

;	dps	"panel_height: "
;	dpd	ebx
;	newline
	
	call	conversion_ASCII_to_HEX
	mov	[panel_attachment],ebx
	
;	dps	"panel_attachment: "
;	dpd	ebx
;	newline
;--------------------------------------
align 4	
.no_boot_parameters:
	call	program_exist
	mcall	14
	mov	[screen_size],eax
	
	mcall	48,3,sc,sizeof.system_colors	; load system colors

	; DEBUGF  DBG_INFO, "sc.work = %x\n", [sc.work]

	mov esi, [sc.work]
	add esi, 0x1a1a1a
	mov ebx, esi
	and ebx, 0xFF000000
	test ebx, ebx
	jz @f
	mov esi, 0x00FFFFFF
@@:
	mov [active_color], esi

	mov  esi, [sc.work]
	cmp  esi, 0xdfdfdf
	jb   @f
	sub  esi, 0x1b1b1b
@@:
	mov [work_color], esi

	mov eax, 68
	mov ebx, 22
	mov ecx, icons_resname
	mov esi, 0 ; SHM_READ
	mcall
	test eax, eax
	jnz @f
	mov [no_shared_resources], 1
	DEBUGF DBG_ERR, "Failed to get ICONS18W from @RESHARE.\nTry rerun @RESHARE.\n"
	jmp .no_res
@@:
	mov [shared_icons_size], edx
	; copy shared icons
	mov esi, eax
	mov ecx, edx
	mcall 68, 12, edx
	mov edi, eax
	mov [shared_icons_ptr], eax
	shr ecx, 2 ; /= 4; ecx = how many dwords in shared icons
	cld
	rep movsd

	; copy shared icons to active icons
	mov esi, [shared_icons_ptr]
	mov ecx, edx
	mcall 68, 12, edx
	mov edi, eax
	mov [shared_icons_active_ptr], eax
	shr ecx, 2 ; /= 4; ecx = how many dwords in shared icons
	cld
	rep movsd

	; fix colors:
	mov esi, [shared_icons_active_ptr]
	mov edi, [shared_icons_ptr]
	xor ecx, ecx
.for1:
	cmp ecx, [shared_icons_size]
	jae .end_for1

	mov eax, esi
	add eax, ecx
	mov edx, [eax]
	cmp edx, [sc.work]
	; DEBUGF DBG_INFO, "eax = %x, sc.work = %x\n", eax, [sc.work]
	jne @f
	mov ebx, [active_color]
	mov [eax], ebx
@@:

	mov eax, edi
	add eax, ecx
	mov edx, [eax]
	cmp edx, [sc.work]
	jne @f
	mov ebx, [work_color]
	mov [eax], ebx
@@:

	add ecx, 4
	jmp .for1
.end_for1:
.no_res:
	
; get size of file MENU.DAT
	mcall	70,fileinfo
	test	eax,eax
	jnz	close
; get memory for MENU.DAT
	mov	ecx,[procinfo+32]
	mov	[fileinfo.size],ecx
	mcall	68,12
	mov	[fileinfo.return],eax
	mcall	68
	mov	[menu_data],eax
; load MENU.DAT
	mov	[fileinfo],dword 0
	mcall	70,fileinfo
	test	eax,eax
	jnz	close

	test	ebx,ebx	   ; length = 0 ?
	jz	close
	mov	ecx,ebx
	mov	edi,[fileinfo.return]	;mem_end
;--------------------------------------
align 4
newsearch: ; search for next submenu in MENU.DAT
	mov	al,'#'
	cld
	repne	scasb
	test	ecx,ecx	   ; if not found
	jz	close
	call	get_number ; get submenu number from char at edi position to ebx
	test	ebx,ebx
	jnz	.number
	cmp	al,'#'
	je	search_end
;--------------------------------------
align 4
.number:
	shl	ebx,4 ; *= 16 . 16 is size of process table (see virtual at 0 ... stuff in the end of file)
	add	ebx,[menu_data]     ; pointer to process table
	mov	[ebx],edi ; process_table->pointer = edi
	inc	[processes]
	jmp	newsearch
;--------------------------------------
align 4
search_end:
	mov	[end_pointer],edi
	mov	ebx,[processes]
	dec	ebx
	shl	ebx,4
	add	ebx,[menu_data]
;--------------------------------------
align 4
newprocess:
	xor	edx,edx
	mov	ecx,edi
	sub	ecx,[ebx]
	mov	al,10
;--------------------------------------
align 4
newsearch1:
	std
	repne	scasb
	test	ecx,ecx
	je	endprocess
	cmp	[edi],byte 13
	jne	newsearch1
	inc	edx
	jmp	newsearch1
;--------------------------------------
align 4
endprocess:
	mov	esi,ebx
	add	esi,4
	dec	edx
	mov	[esi],dl
	cmp	ebx,[menu_data]
	jbe	search_end1
	sub	ebx,16
	jmp	newprocess
;--------------------------------------
align 4
search_end1:
	mcall	14
	cmp	[panel_attachment],byte 1
	je	@f
	xor	ax,ax
	jmp	.store
;--------------------------------------
align 4	
@@:
	sub	ax,[panel_height]	;20
.store:
	mov	ebx,[menu_data]
	mov	[ebx + y_end],ax
	mov	[ebx + x_start],5
	mov	al,[ebx + rows]
	mov	[ebx + cur_sel],al	 ; clear selection
	mov	[ebx + prev_sel],al
	mov	[buffer],0
;------------------------------------------------------------------------------
align 4
thread: ; starts new thread. called when opening each menu
	DEBUGF DBG_INFO, "start new THREAD\n"
	mov	ebp,esp
	sub	ebp,0x1000
	cmp	ebp,0x2000 ; if this is first started thread
	ja	@f
	xor	ebp,ebp ; not free area
;--------------------------------------
align 4
@@:
	mov	eax,[buffer]      ; identifier
	shl	eax,4
	add	eax,[menu_data]
	mov	edi,eax
	mcall	40,100111b	; mouse + button + key + redraw
;------------------------------------------------------------------------------
align 4
red:	
	call	draw_window	; redraw
;------------------------------------------------------------------------------
align 4
still: ; event loop
	call	free_area_if_set_mutex

	mcall	23,5	; wait here for event
	test	[close_now],1      ; is close flag set?
	jnz	close
	
	cmp	eax,1	; redraw request ?
	je	red
	cmp	eax,2	; key pressed ?
	je	key
	cmp	eax,3	; button in buffer ?
	je	button
	cmp	eax,6	; mouse event ?
	je	mouse
	cmp	edi,[menu_data]
	je	still	     ; if main process-ignored
	
	movzx	ebx,[edi + parent]	 ; parent id
	shl	ebx,4
	add	ebx,[menu_data]      ; ebx = base of parent info
	call	backconvert	     ; get my id in al
	cmp	al,[ebx + child]    ; if I'm not child of my parent, I shall die :)
	jne	close
	
	jmp	still
;------------------------------------------------------------------------------
align 4
key:
	mcall	2
	mov	[last_key],ah
	mov	al,[edi + rows]     ; number of buttons
	cmp	ah,178	  ; KEY_UP
	jne	.noup
	
	mov	ah,[edi+cur_sel]
	mov	[edi+prev_sel],ah
	dec	byte [edi+cur_sel]
	jnz	redrawbut
	mov	[edi+cur_sel],al
	jmp	redrawbut
;--------------------------------------
align 4
.noup:
	cmp	ah,177	 ; KEY_DOWN
	jne	.nodn
	
	mov	ah,[edi + cur_sel]
	mov	[edi + prev_sel],ah
	inc	[edi + cur_sel]
	cmp	[edi + cur_sel],al
	jna	redrawbut
	mov	[edi + cur_sel],1
	jmp	redrawbut
;--------------------------------------
align 4
.nodn:
	cmp	ah,179 	 ; KEY_LEFT
	je	@f
	cmp	ah,13 	 ; ENTER
	jne	.noenter
@@:
	mov	ah,[edi + cur_sel]
	jmp	button1
;--------------------------------------
align 4
.noenter:
	cmp	ah,176 	 ; KEY_RIGHT
	je	@f
	cmp	ah,27 	 ; ESC
	jne	still
	jmp	close
;--------------------------------------
align 4
@@:
	call	get_process_ID
	cmp	[main_process_ID],ecx
	jne	close
	jmp	still
;------------------------------------------------------------------------------
align 4
button:	; BUTTON HANDLER
	mcall	17	; get id
				; dunkaist[
	test	eax,0xfffffe00	; is it system close button? (close signal from @taskbar)
	setz	byte[close_now]	; set (or not set) close_recursive flag
	jz	close		; if so,close all menus
				; dunkaist]
;--------------------------------------
align 4
button1:
	mov	esi,edi
	push	edi
	mov	edi,[edi + pointer]
; print "hello"
	mov	al,[esi + cur_sel]
	mov	[esi + prev_sel],al
	mov	[esi + cur_sel],ah
	
	pushad
	mov	edi,esi
; dph eax
	call	draw_only_needed_buttons
	popad
; look (.next_string) for the next line in MENU.DAT <ah> times; <ah> = button_id
	push	eax
;--------------------------------------
align 4
.next_string:
	; DEBUGF DBG_INFO, ".next_string called\n"
	call	searchstartstring
	dec	ah
	jnz	.next_string
	pop	eax
	
	mov	ecx,40
	mov	al,'|'
	cld
	repne	scasb
	test	ecx,ecx	  ; if '|' not found
	je	searchexit
	
	cmp	[edi],byte '@'     ; check for submenu
	je	runthread
	
	cmp	[last_key],179
	je	searchexit
	
	;dec	edi
	push	edi			; pointer to start of filename
	call	searchstartstring	; search for next string
	sub	edi,2		; to last byte of string
	
	mov	ecx,edi
	pop	esi
	sub	ecx,esi
	inc	ecx		 ; length of filename
	mov	edi, fileinfo_start.name
	rep	movsb		   ; copy string
	mov	[edi],byte 0	       ; store terminator
	mcall	70,fileinfo_start	; start program
	pop	edi
	or	[close_now],1      ; set close flag
	mov	[mousemask],0
	; if program run failed then start /sys/@open with param
	test	eax,eax
	jns	close
	mov	eax, fileinfo_start.name
	mov [file_open.params], eax
	mcall	70,file_open	
	jmp	close
;--------------------------------------
align 4
searchexit:
	pop	edi
	jmp	still
;------------------------------------------------------------------------------
align 4
runthread:
	inc	edi
	
	push	eax
	call	get_number	     ; get number of this process
	pop	eax
	
	test	ebx,ebx	   ; returned zero - main menu or not number
	jz	searchexit
	
	mov	al,bl
	
	mov	ebx,[processes]
	dec	bl
	cmp	al,bl
	ja	searchexit	       ; such process doesnt exist
	cmp	al,[esi + child]
	je	searchexit	       ; such process already exists
	
	mov	[esi + child],al    ; this is my child
	mov	cx,[esi + x_start]
	add	cx,BTN_WIDTH+1	  ; new x_start in cx
	movzx	edx,al
	shl	edx,4
	add	edx,[menu_data]       ; edx points to child's base address
	mov	[edx + x_start],cx  ; xstart for new thread
	mov	cx,[esi + y_end]   ; y_end in cx
	mov	bl,[esi + rows]    ; number of buttons in bl
	sub	bl,ah	  ; number of btn from bottom

	; Leency: store vars for case when attachement=top 
	pusha
	mov [prior_thread_selected_y_end], bl
	mcall	9,procinfo,-1
	m2m     [prior_thread_y], dword[procinfo+38]
	m2m     [prior_thread_h], dword[procinfo+46]
	popa

	movzx	eax,al
	mov	[buffer],eax		; thread id in buffer
	movzx	ebx,bl
	push	edx
	mov	eax,BTN_HEIGHT
	mul	ebx
	sub	cx,ax	  ; new y_end for new thread
	pop	edx
	mov	[edx + y_end],cx    ; store y_end
	mov	edi,esi
	call	backconvert	      ; get number of this process (al)
	mov	[edx + parent],al   ; store number of parent process

	mov	al,[edx + rows]
	mov	[edx + cur_sel],al  ; clear current selected element
	mov	[edx + prev_sel],al ; clear previous selected element
	mov	[edx + child],0
	
	mcall	68,12,0x1000	; stack of each thread is allocated 4 KB
	add	eax,0x1000	; set the stack pointer to the desired position
	mov	edx,eax
	mcall	51,1,thread	; Go ahead!
	jmp	searchexit
;------------------------------------------------------------------------------
align 4
mouse: 	      ; MOUSE EVENT HANDLER
	mcall	37,0
	mov	[screen_mouse_position],eax ; eax = [ Y | X ] relative to screen

	mcall	37,2
	test	eax,eax	   ; check buttons state
	jnz	click
	mcall	37,1
	ror	eax,16	  ; eax = [ Y | X ] relative to window
	cmp	ax,BTN_WIDTH	   ; pointer in window?
	ja	noinwindow
; *** in window ***
	shr	eax,16	  ; eax = [ 0 | Y ]
	xor	edx,edx
	mov	ebx,BTN_HEIGHT
	div	ebx
	inc	eax		  ; number of "button" in eax
	movzx	ebx,[edi + rows]    ; total strings in ebx
	cmp	eax,ebx
	ja	noinwindow
	cmp	[edi + cur_sel],al
	je	noredrawbut
	mov	bl,[edi + cur_sel]
;;;;;;
	cmp	[edi + child],0
	jne	noredrawbut
;;;;;;
	mov	[edi + cur_sel],al
	mov	[edi + prev_sel],bl
;--------------------------------------
align 4
redrawbut:
	call	draw_only_needed_buttons
;--------------------------------------
align 4
noredrawbut:
	call	backconvert
	bts	[mousemask],eax
	jmp	still
;--------------------------------------
align 4
noinwindow:
	call	backconvert
	btr	[mousemask],eax
	jmp	still
;------------------------------------------------------------------------------
align 4
click:
	cmp	[mousemask],0  ; not in a window (i.e. menu)
	jne	still
; checking for pressing 'MENU' on the taskbar	
	mov	eax,[screen_mouse_position]
	
	cmp	[panel_attachment],byte 1
	je	@f

	xor	ebx,ebx
	jmp	.check_y
;--------------------------------------
align 4
@@:
	mov	ebx,[screen_size]
	sub	bx,word [panel_height]	;PANEL_HEIGHT
;--------------------------------------
align 4
.check_y:
	add	bx,word [menu_button_y.start]
	cmp	bx,ax
	ja	close

	add	bx,word [menu_button_y.size]
	cmp	bx,ax
	jb	close
	
	shr	eax,16
	
	mov	ebx,[menu_button_x.start]
	cmp	bx,ax	; MENU_BOTTON_X_SIZE
	ja	close
	
	add	bx,[menu_button_x.size]
	cmp	bx,ax	; MENU_BOTTON_X_POS
	ja	still
;------------------------------------------------------------------------------
align 4
close:
	
	movzx	ebx,[edi+parent]       ; parent id
	shl	ebx,4
	add	ebx,[menu_data]          ; ebx = base of parent info
	call	backconvert
	cmp	[ebx + child],al       ; if i am the child of my parent...
	jnz	@f
	mov	[ebx + child],-1       ; ...my parent now has no children
;--------------------------------------
align 4
@@:
	or	eax,-1                 ; close this thread
	mov	[edi + child],al       ; my child is not mine
	
	call	free_area_if_set_mutex
	call	set_mutex_for_free_area
	
	mcall
;--------------------------------------
align 4
backconvert:		  ; convert from pointer to process id
	mov	eax,edi
	sub	eax,[menu_data]
	shr	eax,4
	ret
;------------------------------------------------------------------------------
align 4
set_mutex_for_free_area:
; set mutex for free thread stack area	
	push	eax ebx
;--------------------------------------
align 4
.wait_lock:
        cmp     [free_my_area_lock], 0
        je      .get_lock
	mcall	68,1
        jmp     .wait_lock
;--------------------------------------
align 4
.get_lock:
        mov     eax, 1
        xchg    eax, [free_my_area_lock]
        test    eax, eax
        jnz     .wait_lock
	mov	[free_my_area],ebp
	pop	ebx eax
	ret
;------------------------------------------------------------------------------
align 4
free_area_if_set_mutex:
	cmp	[free_my_area_lock],0
	je	.end

	push	eax ebx ecx
	mov	ecx,[free_my_area]

	test	ecx,ecx
	jz	@f
	mcall	68,13
;--------------------------------------
align 4
@@:
	xor	eax,eax
	mov	[free_my_area_lock],eax
	pop	ecx ebx eax
;--------------------------------------
align 4
.end:	
	ret
;------------------------------------------------------------------------------
;==================================
; get_number
;    load number from [edi] to ebx
;==================================
align 4
get_number:
	push	edi
	xor	eax,eax
	xor	ebx,ebx
;--------------------------------------
align 4
.get_next_char:
	mov	al,[edi]
	inc	edi
	cmp	al, '0'
	jb	.finish
	cmp	al, '9'
	ja	.finish
	sub	al, '0'
	imul	ebx,10
	add	ebx,eax
	jmp	.get_next_char
;-------------------------------------
align 4
.finish:
	pop	edi
	ret
;------------------------------------------------------------------------------
align 4
get_process_ID:
	mcall	9,procinfo,-1
	mov	edx,eax
	mov	ecx,[ebx+30]	; PID
	ret
;------------------------------------------------------------------------------
align 4
program_exist:
	call	get_process_ID
	mov	[main_process_ID],ecx
	mcall	18,21
	mov	[active_process],eax	; WINDOW SLOT
	mov	ecx,edx
	xor	edx,edx
;-----------------------------------------
align 4
.loop:
	push	ecx
	mcall	9,procinfo
	mov	eax,[menu_mame]
	cmp	[ebx+10],eax
	jne	@f
	; temporary to fit into 3 IMG sectors
	;mov	ax,[menu_mame+4]
	;cmp	[ebx+14],ax
	;jne	@f
	cmp	ecx,[active_process]
	je	@f
; dph ecx
	mcall	18,2
	mov	edx,1
;--------------------------------------
align 4
@@:
	pop	ecx
	loop	.loop

	test	edx,edx
	jz	@f
	mcall	-1
;--------------------------------------
align 4
@@:
	ret
;------------------------------------------------------------------------------
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
align 4
draw_window:
	mcall	48,5 ; get working area
	mov	[x_working_area],eax
	mov	[y_working_area],ebx

	mcall	12,1	; 1,start of draw
	movzx	ebx,[edi + rows]
	imul	eax,ebx,BTN_HEIGHT	    ; eax = height of window
	movzx	ecx,[edi + y_end]
	cmp	[panel_attachment],byte 1
	je	@f
	
	
	;cmp	ebp,0x000 ; if this is first started thread
	;je .1            ; then show it at the very top
	
	push  ebx eax
	; if attachement=top 
	; then NEW_WIN_Y = PRIOR_WIN_Y + PRIOR_WIN_H - ITEM_H + 1 - SEL_ITEM_Y

	mov ecx, [prior_thread_y]
	add ecx, [prior_thread_h]
	sub ecx, BTN_HEIGHT
	inc ecx

	xor eax, eax
	mov al, [prior_thread_selected_y_end]
	mov ebx, BTN_HEIGHT
	mul ebx
		
	sub ecx, eax

	mov	[edi + cur_sel],1 ;if attachement=top then set item=1 selected
	
	pop eax ebx

	jmp	.1
;--------------------------------------
align 4
@@:	
	sub	ecx,eax	    ; ecx = Y_START
;--------------------------------------
align 4
.1:
	shl	ecx,16
	add	ecx,eax	    ; ecx = [ Y_START | Y_SIZE ]
	dec ecx

	movzx	ebx,[edi + x_start]
	shl	ebx,16
	mov	bx,BTN_WIDTH	    ; ebx = [ X_START | X_SIZE ]
	mov	edx,0x01000000       ; color of work area RRGGBB,8->color gl
	mov	esi,edx	    ; unmovable window
	
	mov	eax,[y_working_area]
	shr	eax,16
	ror	ecx,16
	test	cx,0x8000
	jz	@f
	mov	cx,ax
;--------------------------------------
align 4
@@:
	cmp	cx,ax
	ja	@f
	mov	cx,ax	
;--------------------------------------
align 4
@@:
	rol	ecx,16
	xor	eax,eax	    ; function 0 : define and draw window
	mcall
	
;	dps	"[ Y_START | Y_SIZE ] : "
;	dph	ecx
;	newline

;	dps	"[ X_START | X_SIZE ] : "
;	dph	ebx
;	newline

	call	draw_all_buttons
	mcall	12,2
	ret
;------------------------------------------------------------------------------
align 4
draw_all_buttons:
	xor	edx,edx
;--------------------------------------
align 4
.new_button:
	call	draw_one_button
	inc	edx
	cmp	dl,[edi + rows]
	jb	.new_button
	ret
;------------------------------------------------------------------------------
align 4
draw_only_needed_buttons:
	xor	edx,edx
	mov	dl,[edi + cur_sel]
	dec	dl
	call	draw_one_button
	mov	dl,[edi + prev_sel]
	dec	dl
	call	draw_one_button
	ret
;------------------------------------------------------------------------------
align 4
draw_one_button:
; receives number of button in dl
	push	edx
	mov	eax,8
	mov	ebx,BTN_WIDTH
	movzx	ecx,dl
	imul	ecx,BTN_HEIGHT
	mov [draw_y], ecx
	shl	ecx,16
	add	ecx,BTN_HEIGHT
; edx = button identifier
	mov	esi, [work_color]

	mov [is_icon_active], 0
	inc	dl
	cmp	[edi + cur_sel],dl
	jne	.nohighlight
	mov [is_icon_active], 1

	mov	esi, [active_color]
;--------------------------------------
align 4
.nohighlight:
	or	edx,BT_NOFRAME + BT_HIDE
				; dunkaist[
	add	edx,0xd1ff00	; This makes first menu buttons differ
				; from system close button with 0x000001 id
				; dunkaist]
	mcall
	push edx 
	
	mov edx, esi
	mcall 13 ; draw rect
	
	mcall , BTN_WIDTH,<[draw_y],1>,[sc.work_light]
	add     ecx, BTN_HEIGHT-1
	mcall , 1
	inc     ecx
	mcall , <BTN_WIDTH,1>, , [sc.work_dark]
	add     [draw_y], BTN_HEIGHT-1
	mcall , BTN_WIDTH,<[draw_y],1>
	
	pop edx
	movzx	edx,dl
	dec	dl
	imul	ebx,edx,BTN_HEIGHT
	add	ebx,((4 + 18) shl 16) + TXT_Y ; added + 18 (icon size)
	movzx	ecx,dl
	inc	ecx
	mov	edx,[edi + pointer]
;--------------------------------------
align 4
.findline:
	cmp	byte [edx],13 ; if \r encountered => line found
	je	.linefound
	inc	edx ; go to next char
	jmp	.findline
;------------------------------------------------------------------------------
align 4
.linefound:
	inc	edx ; go to next char after \r
	cmp	byte [edx],10 ; if it is not \n then again findline
	jne	.findline
	dec	ecx ; TODO what in ecx? button number?
	jnz	.findline
	
	mov ecx, [sc.work_text]
	add ecx, FONT_TYPE

	push ecx esi edi ebp
	push ebx ; preserve ebx, it stores coordinates
	mov [tmp], edx
	mov [has_icon], 1
	xor ebx, ebx
@@: ; parse icon number
	inc	edx
	mov	al,[edx]
	; DEBUGF DBG_INFO, "(%u)\n", al
	cmp	al, '0'
	jb	@f
	cmp	al, '9'
	ja	@f
	sub	al, '0'
	imul	ebx,10
	add	ebx,eax
	jmp @b
@@:
	; DEBUGF DBG_INFO, "icon_number = %x al = %u\n", ebx, al
	mov [icon_number], ebx
	cmp al, ' '
	je @f
	; if no space after number then consider that number is a part of caption
	mov edx, [tmp] ; restore edx
	mov [has_icon], 0 ; no icon
@@:
	pop ebx

	mcall	4,,,,21 ; draw menu element caption

	cmp [no_shared_resources], 1
	je @f
	cmp [has_icon], 1
	jne @f
	; draw icon:
	mov eax, ebx
	shr eax, 16
	sub eax, 18 ; 18 - icon width
	movzx ebx, bx

	sub ebx, 2
	shl eax, 16
	add eax, ebx
	add eax, 1 shl 16
	mov [tmp], eax
	mov ebx, [icon_number]
	imul ebx, 18*18*4

	mov ecx, [shared_icons_ptr]
	; DEBUGF DBG_INFO, "is_icon_active = %x\n", [is_icon_active]
 	cmp [is_icon_active], 1
 	jne .not_active_icon
 	mov ecx, [shared_icons_active_ptr]
.not_active_icon:	
	add ebx, ecx
	mcall 65, ebx, <18,18>, [tmp], 32, 0, 0

@@:
	pop ebp edi esi ecx
	
	pop	edx
	ret
;------------------------------------------------------------------------------
align 4
searchstartstring:
	mov	ecx,40
	mov	al,13
	cld
	repne	scasb
	cmp	byte [edi],10
	jne	searchstartstring
	ret
;------------------------------------------------------------------------------
;*** DATA AREA ****************************************************************
menu_mame:   db '@MENU',0
default_dir: db '/sys',0

align 4
free_my_area_lock	dd 0
free_my_area	dd 0

processes      dd 0
;--------------------------------------
menu_button_x:
.start:	dd MENU_BOTTON_X_POS
.size:	dd MENU_BOTTON_X_SIZE
;--------------------------------------
menu_button_y:
.start:	dd 2
.size:	dd 18
;--------------------------------------
panel_height:		dd PANEL_HEIGHT
panel_attachment:	dd 1
;--------------------------------------
align 4
fileinfo:
 .subfunction	 dd 5		; 5 - file info; 0 - file read
 .start 	 dd 0		; start byte
 .size_high	 dd 0		; rezerved
 .size		 dd 0		; bytes to read
 .return	 dd procinfo	; return data pointer
 .name:
     db   'SETTINGS/MENU.DAT',0   ; ASCIIZ dir & filename
;--------------------------------------
align 4
fileinfo_start:
 .subfunction	dd 7	; 7=START APPLICATION
 .flags		dd 0	; flags
 .params	dd 0x0	; nop
 .rezerved	dd 0x0	; nop
 .rezerved_1	dd 0x0	; nop
 .name:
   times 50 db ' '
;--------------------------------------
align 4
file_open:
 .subfunction	dd 7	; 7=START /SYS/@OPEN APP WITH PARAM
 .flags		dd 0	; flags
 .params	dd 0x0	; nop
 .rezerved	dd 0x0	; nop
 .rezerved_1	dd 0x0	; nop
 .name:
   db   '/SYS/@OPEN',0
;------------------------------------------------------------------------------
IM_END:
;------------------------------------------------------------------------------
align 4
close_now	dd ?   ; close all processes immediately
end_pointer	dd ?
buffer		dd ?
mousemask	dd ?   ; mask for mouse pointer location

active_process	dd ?
main_process_ID	dd ?
;--------------------------------------
screen_mouse_position:
.y	dw ?
.x	dw ?
;--------------------------------------
screen_size:
.y	dw ?
.x	dw ?
;--------------------------------------
draw_y dd ?
;--------------------------------------
x_working_area:
.right:		dw ?
.left:		dw ?
y_working_area:
.bottom:	dw ?
.top:		dw ?
;--------------------------------------
sc system_colors
;--------------------------------------
last_key	db ?
prior_thread_y dd ?
prior_thread_h dd ?
prior_thread_selected_y_end db ?
;------------------------------------------------------------------------------
align 4
menu_data   dd ?
;--------------------------------------
virtual     at 0       ; PROCESSES TABLE (located at menu_data)
  pointer   dd ?   ; +0    pointer in file
  rows      db ?   ; +4    numer of strings
  x_start   dw ?   ; +5    x start
  y_end     dw ?   ; +7    y end
  child     db ?   ; +9    id of child menu
  parent    db ?   ; +10   id of parent menu
  cur_sel   db ?   ; +11   current selection
  prev_sel  db ?   ; +12   previous selection
  rb        16-$+1 ; [16 bytes per element]
end virtual

include_debug_strings ; for debug-fdo

icons_resname db 'ICONS18W', 0
shared_icons_ptr dd ?
shared_icons_active_ptr dd ?
shared_icons_size dd ?
has_icon db ?
icon_number dd ?
is_icon_active dd ?
no_shared_resources dd 0
tmp dd ?
active_color dd ?
work_color dd ?
;------------------------------------------------------------------------------
align 4
bootparam:
procinfo:
	rb 1024
;------------------------------------------------------------------------------
align 4
	rb 0x1000
stack_area:
;------------------------------------------------------------------------------
mem_end:
;------------------------------------------------------------------------------
