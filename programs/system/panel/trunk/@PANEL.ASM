;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                   ;
;    MENUBAR for KolibriOS  - Compile with fasm     ;
;                                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;------------------------------------------------------------------------------	
; last update:  31/03/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      The program uses only 20 Kb memory is now.
;               Code optimizing and refactoring.
;------------------------------------------------------------------------------
	use32
	org 0x0
	db 'MENUET01'	; 8 byte id
	dd 0x01		; header version
	dd START	; program start
	dd IM_END	;I_END	    ; program image size
	dd I_END	;0xA000	    ; reguired amount of memory - 10 Kb
	dd stack_top	;0xA000	    ; esp
	dd 0x0		; boot parameters
	dd 0x0		; path
;------------------------------------------------------------------------------	
include 'lang.inc'
include '..\..\..\macros.inc'
include 'MOI.INC'	;раскладки клавиатуры

caps_lock_check fix 0

time_bgr_color = 0x66cc

width		dd  305
buttons 	dd    1  ;  0 no frames  ; 1 frames
soften_up	dd    1  ;  0 no         ; 1 yes
soften_down	dd    0  ;  0 no         ; 1 yes
minimize_left	dd    1
minimize_right	dd    1
icons_position	dd    95
menu_enable	dd    1
setup_enable	dd    0
graph_text	dd    1
soften_middle	dd    1  ;  0 no         ; 1 yes
icons		dd    1  ;  0 defaults   ; 1 activate

PANEL_HEIGHT = 18
;------------------------------------------------------------------------------	
align 4
handle_key:
	mcall	18, 7
	mov	[active_process],eax

	mcall	2
	cmp	al, 2
	jnz	begin_1.ret

	mov	ebx, exec_fileinfo
	shr	eax, 8
	cmp	al, 0
	je	prod

	mov	[key_r],al
	sub	[key_r],2
	cmp	al, 2
	jz	alter

	cmp	al, 3
	jz	alter

	cmp	al, 4
	jz	alter

	cmp	al, 5
	jz	alter

	cmp	al, 6
	jz	alter

	cmp	al, 7
	jz	alter

	cmp	al, 8
	jz	alter
;--------------------------------------
align 4
prod:
if caps_lock_check
	cmp	al,58
	jne	@f

	pusha
	mcall	26,2,9
	mov	ebx,eax
	mov	eax,2
	call	draw_window ;;;???
	popa
	ret
@@:
end if
	cmp	al, 15
	jz	alt_tab_pressed

	cmp	al, 88
	jz	start_end_application

	cmp	al, 91
	jz	start_menu_application

	cmp	al, 92
	jz	start_menu_application

	cmp	al, 62
	jz	kill_active_application

;	cmp	al, 71
;	jz	page_list_next

;	cmp	al, 72
;	jz	page_list_prev

	cmp	al, 69
	jz	start_mousemul_application

	cmp	[current_alt_tab_app], -1
	jz	@f

	test	ah, 0x30
	jz	alt_tab_released
;--------------------------------------
align 4
@@:
; this is hotkey Ctrl+Shift ;or LShift+RShift
	jmp	karu
;	mov	ebx, setup_exec
;	test	ah, 001100b
;	jz	change_sys_lang
;------------------------------------------------------------------------------	
align 4
change_key_lang:
	mov	dword [ebx+8], chlang
	mcall	70
;--------------------------------------
align 4
begin_1:
	mov	ecx,[active_process]
	mcall	18, 3
	mcall	5, 25
;--------------------------------------
align 4
.ret:
	ret
;------------------------------------------------------------------------------	
;align 4
;change_sys_lang:
;    	mov	   dword [ebx+8], syslang
;        mcall   70
;        jmp     begin_1
;------------------------------------------------------------------------------	
align 4
start_end_application:
	mov	dword [ebx+21],end_name
	mcall	70
	ret
;------------------------------------------------------------------------------	
align 4
start_mousemul_application:
	mov	dword [ebx+21],mousemul_name
	mcall	70
	ret
;------------------------------------------------------------------------------	
align 4
kill_active_application:
	mcall	72,1,3,1
	jmp	begin_1.ret
;------------------------------------------------------------------------------	
align 4
start_menu_application:
	mov	[draw_window_1], 1
	mov	dword [ebx+21], menu_name
	mcall	70
	jmp	begin_1.ret
;------------------------------------------------------------------------------	
;align 4
;page_list_next:
;	cmp	[page_list],15
;	je	@f
;
;	inc	[page_list]
;	mov	[draw_window_1],1
;@@:
;	jmp  begin_1.ret
;------------------------------------------------------------------------------	
;align 4
;page_list_prev:
;	cmp	[page_list],0
;	je	@f
;
;	dec	[page_list]
;	mov	[draw_window_1],1
;@@:
;	jmp  begin_1.ret
;------------------------------------------------------------------------------	
align 4
alt_tab_pressed:
; handle Alt+Tab and Alt+Shift+Tab
	mov	ebp, eax
	cmp	[current_alt_tab_app], -1
	jnz	has_alt_tab_app
; заполняем таблицу приложений, подлежащих переключению
	xor	edx, edx
	mov	ebx, procinfo_window_tabs	;0x8000 ; WTF???!!!
	mov	ecx, 1
.fill:
	inc	ecx
	mcall	9
	call	need_window_tab
	jz	@f

	cmp	edx, 256
	jz	@f

	mov	[alt_tab_list+edx*8], ecx
	movzx	esi, word [ebx+4]
	mov	[alt_tab_list+edx*8+4], esi
	inc	edx
;--------------------------------------
align 4
@@:
	cmp	ecx,eax
	jb	.fill

	mov	[alt_tab_list_size], edx
	test	edx, edx
	jz	begin_1.ret

	mcall	66,4,0,0	; ловим момент отпускания всех управляющих клавиш
	test	eax, eax
	jnz	begin_1.ret

	xor	edx, edx
	mov	eax, [alt_tab_list+4]
	xor	ecx, ecx
	inc	ecx
;--------------------------------------
align 4
.findmax:
	cmp	[alt_tab_list+ecx*8+4], eax
	jb	@f

	mov	edx, ecx
	mov	eax, [alt_tab_list+ecx*8+4]
;--------------------------------------
align 4
@@:
	inc	ecx
	cmp	ecx, [alt_tab_list_size]
	jb	.findmax

	mov	[current_alt_tab_app], edx
;--------------------------------------
align 4
has_alt_tab_app:
	mov	eax, [current_alt_tab_app]
	mov	edx, [alt_tab_list+eax*8+4]	; slot
	xor	ecx, ecx
	or	eax, -1
	test	ebp, 300h
	jz	.notshift

	or	esi, -1
;--------------------------------------
align 4
.loop1:
	cmp	[alt_tab_list+ecx*8+4], edx
	jbe	@f

	cmp	[alt_tab_list+ecx*8+4], esi
	jae	@f

	mov	eax, ecx
	mov	esi, [alt_tab_list+ecx*8+4]
;--------------------------------------
align 4
@@:
	inc	ecx
	cmp	ecx, [alt_tab_list_size]
	jb	.loop1

	cmp	eax, -1
	jnz	.found

	xor	edx, edx
	xor	ecx, ecx
	jmp	.loop1
;--------------------------------------
align 4
.notshift:
	xor	esi, esi
;--------------------------------------
align 4
.loop2:
	cmp	[alt_tab_list+ecx*8+4], edx
	jae	@f

	cmp	[alt_tab_list+ecx*8+4], esi
	jbe	@f

	mov	eax, ecx
	mov	esi, [alt_tab_list+ecx*8+4]
;--------------------------------------
align 4
@@:
	inc	ecx
	cmp	ecx, [alt_tab_list_size]
	jb	.loop2

	cmp	eax, -1
	jnz	.found

	or	edx, -1
	xor	ecx, ecx
	jmp	.loop2
;--------------------------------------
align 4
.found:
	mov	[current_alt_tab_app], eax
	push	eax
	xor	edx, edx
	div	[max_applications]
	mov	[page_list], eax
	mov	[draw_window_1], 1
	mov	edi, app_list
	push	edi
	mov	ecx, 20
	or	eax, -1
	rep	stosd
	pop	edi
	pop	ecx
	sub	ecx, edx
;--------------------------------------
align 4
@@:
	cmp	ecx, [alt_tab_list_size]
	jae	redraw_window_tabs

	mov	eax, [alt_tab_list+ecx*8]
	stosd
	inc	ecx
	jmp	@b
;--------------------------------------
align 4
alt_tab_released:
	mcall	66,5,0,0	; уже поймали, хватит :)
	or	eax, -1
	xchg	eax, [current_alt_tab_app]
	mov	ecx, [alt_tab_list+eax*8]
	mcall	18,3
	jmp	redraw_window_tabs
;------------------------------------------------------------------------------	
align 4
active_process	dd 0

;  .exit: mcall -1


START:
	mcall	66,4,0,2	; LShift+RShift
	mcall	66,,,11h	; Ctrl+Shift
	mcall	66,,88,110h	; Alt+Ctrl+F12
	mcall	66,,91,0h 	; LWin
	mcall	66,,92 		; RWin
;	mcall	66,,91,100h	; Alt+LWin
;	mcall	66,,92		; Alt+RWin
	mcall	66,,62,100h	; Alt+F4
;	mcall	66,,71		; Alt+Home
;	mcall	66,,72		; Alt+Up
	mcall	66,,15		; Alt+Tab
	mcall	66,,2		; Alt+1
	mcall	66,,3		; Alt+2
	mcall	66,,4		; Alt+3
	mcall	66,,5		; Alt+4
	mcall	66,,6		; Alt+5
	mcall	66,,7		; Alt+6
	mcall	66,,8		; Alt+7
	mcall	66,,,101h 	; Alt+Shift+Tab
	mcall	66,,69		; Alt+Shift+NumLock

if caps_lock_check
	xor	edx,edx
	mcall	66,4,58
end if
	mcall	18,8,1
	test	eax,eax
	jne	@f
	mcall	18,8,2
;--------------------------------------
align 4
@@:
	mcall	70,dat_fileinfo

	mov	edi,width
	mov	esi,I_END
	xor	eax,eax
;--------------------------------------
align 4
new_number:
	cmp	[esi],byte ';'
	je	number_ready

	imul	eax,10
	movzx	ebx,byte [esi]
	sub	ebx,'0'
	add	eax,ebx
	inc	esi
	jmp	new_number
;--------------------------------------
align 4
number_ready:
	stosd
	xor	eax,eax
	inc	esi
	cmp	[esi],byte 'x'
	jne	new_number

	mcall	14
	mov	[screen_size],eax

	mov	ecx,eax
;eax = [xsize]*65536 + [ysize], где
	shr	ecx,16
	and	eax,0xFFFF
	lea	edx,[eax-PANEL_HEIGHT-1]

	mcall	48,6

	mcall	9,process_info_buffer,-1
	mov	ecx,[process_info_buffer+30]
	mcall	18,21
	mov	[this_slot],eax
	mov	[max_slot],255

	call	set_variables
;--------------------------------------
align 4
start_after_minimize:
	call	draw_window
	call	draw_info
	call	draw_running_applications

	mcall	23,30
	call	load_ini
	call	fir_lng
;------------------------------------------------------------------------------
align 4
still:
	call	draw_info
	call	draw_running_applications

	mcall	18,7	; check if active window changed
	cmp	eax, [last_active_window]
	jz	@f
; need_window_tab:
; in: ebx->process info
; out: ZF set <=> do not draw
	mov	ebx,[last_active_window]
	mov	[prev_active_window], ebx
	mov	[last_active_window], eax

	mov	ecx, eax
	mcall	9,process_info_buffer

	call	need_window_tab
	jnz	.need_repaint

	mcall	9,process_info_buffer,[prev_active_window]
	call	need_window_tab
	jz	@f
;--------------------------------------
align 4
.need_repaint:
	mov	dword [active_window_changed], 1
;--------------------------------------
align 4
@@:
	mcall	23,20
	cmp	eax,1		; redraw ?
	jz	red

	cmp	eax,3		; button ?
	jz	button

	call	handle_key
	cmp	dword [active_window_changed], 0
	jnz	red_active

	jmp	still
;------------------------------------------------------------------------------	
align 4
red_active:
red:
	mov	dword [active_window_changed], 0

	mcall	14
	movzx	ecx,ax
	mov	edx,eax
	shr	edx,16
	cmp	[screen_size.height],ax
	jne	@f

	rol	eax,16
	cmp	[screen_size.width],ax
	je	.lp1
	rol	eax,16
@@:
	mov	[screen_size],eax
	sub	ecx,PANEL_HEIGHT
	mcall	67,0,,,PANEL_HEIGHT
;--------------------------------------
align 4
.lp1:
	call	draw_window
	call	draw_info
	jmp	still
;------------------------------------------------------------------------------	
align 4
button:
	mcall	17
	test	eax,0xfffffe00	; is it close button? (signal from @panel)
	jz	still		; if so - wait for another event, because @panel shouldn't be closed

	cmp	al, 0
	jnz	right_mouse

	cmp	ah,50
	jb	no_activate
	cmp	ah,70
	jg	no_activate

	movzx	ecx,byte ah
	sub	ecx,52
	shl	ecx,2

	mov	ecx,[app_list+ecx]
	mcall	18,3,

;	mcall	55,eax,,,activatemusic
	jmp	still
;------------------------------------------------------------------------------	
align 4
right_mouse:
	cmp	ah,50
	jb	still

	cmp	ah,70
	jg	still

	movzx	ecx,byte ah
	sub	ecx,52

	cmp	ecx, [app_tab_count]
	jge	still

	shl	ecx,2
	mcall	37,0
	mov	ebx, eax
	shr	eax, 16
	mov	[x_coord], ax
	and	ebx, 0xFFFF
	mov	[y_coord], bx

	mov	ecx,[app_list+ecx]	; ecx->selected app.slot
	mov	[n_slot], ecx
	mcall	9,procinfo_for_detect

	mcall	51,1,context_menu_start,ctx_menu_stack

	mov	[ctx_menu_PID], eax
	jmp	still
;------------------------------------------------------------------------------	
align 4
no_activate:
	cmp	ah,101 	  ; minimize to left
	je	left_button

	cmp	ah,102 	  ; minimize to right
	je	right_button

	cmp	ah, 103
	je	clean_desktop		    ; minimize all windows

	cmp	ah, 104
	je	restore_desktop	    ; restore minimized windows

	cmp	ah, 105
	je	swap_minimized_desktop     ; minimize normal windows, and restore minimized windows

	cmp	ah,byte 1	  ; start/terminate menu
	jnz	noselect

	call	menu_handler

	jmp	still
;--------------------------------------
align 4	
noselect:
	mov	ebx, exec_fileinfo
	cmp	ah,byte 2	       ; start calendar
	jnz	noid15  ;noclock

	mov	dword [ebx+21], calendar_name
	mcall	70
	jmp	still
;--------------------------------------
align 4	
noid15:
	cmp	ah,16
	jne	noid16

	mov	ebx, setup_exec
	mov	dword [ebx+8], chlang
	mcall	70
	jmp	still
;--------------------------------------
align 4	
noid16:
;	cmp	ah,17
;	jne	noid17
;
;	mov	ebx, setup_exec
;	mov	dword [ebx+8], syslang
;	mcall	70
;	jmp	still
;--------------------------------------
;align 4	
;  noid17:
	cmp	ah,18
	jne	noid18

	mov	dword [ebx+21], sysmeter_name
	mcall	70
	jmp	still
;--------------------------------------
align 4	
noid18:
	cmp	ah,19
	jne	noid19

	mcall	18,8,2
	mcall	15,3
	jmp	red
;--------------------------------------
align 4	
noid19:
	cmp	ah,20		   ; start system setup
	jnz	noid20

	mov	ebx, setup_exec
	and	dword [ebx+8], 0
	mcall	70
	jmp	still
;--------------------------------------
align 4	
noid20:
	cmp	ah,21
	jnz	noid21

	cmp	[page_list], 99
	jnc	still
	inc	[page_list]
	jmp	red
;--------------------------------------
align 4	
noid21:
	cmp	ah,22
	jnz	still

	cmp	[page_list],0
	je	still

	dec	[page_list]
	jmp	red
;------------------------------------------------------------------------------	
align 4
restore_desktop:
	mcall	9,process_info_buffer,-1
	mov	[max_slot], eax
	mov	ecx, 2
	mov	edx, 2
;--------------------------------------
align 4	
@@:
	mcall	18, 22
	inc	edx
	cmp	edx, [max_slot]
	jbe	@b
	jmp	still
;------------------------------------------------------------------------------	
align 4
swap_minimized_desktop:
	xor	esi, esi
	inc	esi
	jmp	min_swap_desktop
;------------------------------------------------------------------------------	
align 4
clean_desktop:
	xor	esi,esi
;--------------------------------------
align 4	
min_swap_desktop:		   ; input esi 0 - min, <>0 swap
	mov	ecx, 2		   ; Начальный номер слота процесса
	mov	ebx, process_info_buffer
;--------------------------------------
align 4
.loop1:
	cmp	ecx, [this_slot]
	je	.loop1xx

	mcall	9
	mov	[max_slot], eax
	mov	dx, word [process_info_buffer+50]
	cmp	dx, 9		    ; Проверка свободен ли данный слот
	jz	.loop1xx

	mov	edx, dword [ebx+10]
	cmp	dl, '@'
	je	.loop1xx

	cmp	dword [ebx+10], 'ICON'
	jnz	@f

	cmp	[ebx+42], dword 51
	jnz	@f

	cmp	[ebx+46], dword 51
	jz	.loop1xx
;--------------------------------------
align 4
@@:
	cmp	[ebx+10], dword '    '
	jz	.loop1xx

	pushad
	mov	  edx, ecx
	xor	  ecx, ecx
	or     esi, esi
	jz     @f

	movzx  eax, byte [process_info_buffer+70]
	and	  eax, 2	      ; mask minimize
	jz     @f

	mov	  ecx, 2	      ; restore
;--------------------------------------
align 4
@@:
	mcall	18, 22
	popad
;--------------------------------------
align 4
.loop1xx:
	inc	ecx
	cmp	ecx, [max_slot]
	jbe	.loop1
	jmp	still
;------------------------------------------------------------------------------	
align 4
draw_running_applications:
	pusha

	cmp	[icons],1
	jne	dr_ret

	call	calculate_applications

	cmp	edi,[running_applications]
	jne	noret
	popa
	ret
;--------------------------------------
align 4
noret:
	call	draw_window
	mov	[running_applications],edi
	call	redraw_window_tabs
;--------------------------------------
align 4
dr_ret:
	popa
	ret
;------------------------------------------------------------------------------	
align 4
need_window_tab:
; in: ebx->process info
; out: ZF set <=> do not draw
	cmp	byte [ebx+10], '@'
	jz	.nodraw
; do not draw undefined (zero-sized) windows
	cmp	dword [ebx+42], 0
	jnz	@f
	cmp	dword [ebx+46], 0
	jz	.nodraw
;--------------------------------------
align 4
@@:
; do not draw OpenDialog windows
	push	edi
	mov	edi, dword [ebx+10]
	or	edi, 0x20202020
	cmp	edi, 'open'
	jne	@f

	mov	edi, dword [ebx+14]
	or	edi, 0x20202020
	cmp	dword [ebx+14], 'dial'
	jne	@f

	pop	edi
	jmp	.nodraw
;--------------------------------------
align 4
@@:
	pop	edi
	cmp	dword [ebx+10], 'ICON'
	jnz	@f

	cmp	[ebx+42], dword 51
	jnz	@f

	cmp	[ebx+46], dword 51
	jz	.nodraw
;--------------------------------------
align 4
@@:
	cmp	[ebx+10], dword '    '
;--------------------------------------
align 4
.nodraw:
	ret
;------------------------------------------------------------------------------	
align 4
redraw_window_tabs:
	xor	edi, edi
	mov	[contrast], 0

	push	ebp
	mcall	18,7
	mov	ebp,eax
;--------------------------------------
align 4
.loop:
	mov	ecx,[app_list+edi*4]
	cmp	ecx,-1
	jz	.done

	push	ecx
	mcall	9,procinfo_window_tabs	;0x8000	; WTF???!!!

	imul	ebx, edi, 6*10*10000h
	add	ebx, 12*10*10000h + 7*10000h + 54

	mcall	13,,<3,14>,0x383838

	sub	ebx, 10000h + 53
	mcall	,,<4,12>

	sub	ebx, 10000h
	mcall	,,<5,10>

	add	ebx, 56*10000h
	mcall	,,<4,12>

	add	ebx, 10000h
	mcall	,,<5,10>

	mov	edx, [wcolor]	; barsuk
	pop	ecx
	cmp	ebp, ecx	; ecx=active slot number
	jnz	@f

	mov	edx, [system_colours.work_button]
;--------------------------------------
align 4
@@:
	mov	esi, [current_alt_tab_app]
	cmp	esi, -1
	jz	@f

	cmp	ecx, [alt_tab_list+esi*8]
	jnz	@f

	mov	edx, 0xFF8000
;--------------------------------------
align 4
@@:
	sub	ebx, 55*10000h - 53
	mcall	,,<4,12>
	
	sub	ebx, 10000h + 53
	mcall	,,<5,10>

	add	ebx, 55*10000h
	mcall

	sub	ebx, 51*10000h - 6
	mov	edx, procinfo_window_tabs+10	;0x8000+10		; WTF???!!!
	mcall	4,,[system_colours.work_button_text],,11
;--------------------------------------
align 4
.nodraw:
	inc	edi
	cmp	edi, [max_applications]
	jb	.loop
;--------------------------------------
align 4
.done:
	pop	ebp
	ret
;------------------------------------------------------------------------------	
align 4
calculate_applications:
	mov	eax,[max_applications]
	mul	[page_list]
	test	eax,eax
	je	@f

	inc	eax
;--------------------------------------
align 4
@@:
	mov	[draw_start_position],eax

	mov	edi,app_list
	mov	ecx,20
	mov	eax,-1
	cld
	rep	stosd

	mov	edi,0
	mov	ecx,2
;--------------------------------------
align 4
cnewpr:
	mcall	9,procinfo_window_tabs	;0x8000	; WTF???!!!

	call	need_window_tab
	jz	cnorpl

	sub	[draw_start_position], 1
	jg	cnorpl

	mov	[app_list+edi*4],ecx
	inc	edi
;--------------------------------------
align 4
cnorpl:
	inc	ecx
	cmp	eax,ecx
	jge	cnewpr

	mov	[app_tab_count], edi
	ret
;------------------------------------------------------------------------------	
align 4
draw_application_buttons:
	pusha

	cmp	[icons],1
	jne	da_ret

	mcall	14
	shr	eax,16

	sub	eax, 281
	jnc	._01
	xor	eax, eax
;--------------------------------------
align 4
._01:
	mov	bl, 60
	div	bl
	xor	ah, ah		       ; eax - количество умещаюшихся пиктов
	mov	[max_applications], eax
	mov	edi,1
;--------------------------------------
align 4
nb:
	mov	ebx,edi
	shl	ebx,16
	imul	ebx,6*10	     ;13
	add	ebx,75*65536+10*6-1 ; << Alver 25.08.2007 >> 15*65536+10*6-1  ;13
	mov	edx,edi
	add	edx,51
	cmp	[buttons],1
	je	bufr

	or	edx,0x60000000
;--------------------------------------
align 4
bufr:
	mov	esi,[wcolor]
	sub	ebx,11 shl 16
	mcall	8,,<1,17>

	inc	edi
	cmp	edi,[max_applications]
	jbe	nb
;--------------------------------------
align 4
da_ret:
	popa
	ret
;------------------------------------------------------------------------------	
align 4
menu_handler:
	mov	ebx, exec_fileinfo
	mov	[ebx+21],dword menu_name
	mcall	70
	ret
;------------------------------------------------------------------------------	
align 4
draw_small_right:
	pusha
	mcall	12,1

	xor	eax,eax
	mov	edx,[wcolor]
	mov	esi,edx
	mov	edi,edx
	or	edx, 0x01000000
	mcall

	xor	ecx,ecx
	mov	cx,[b_size_y]
	mcall	8,<0,9>,,1,[wcolor]

	mov	ebx,2*65536+16
	cmp	[graph_text],1
	jne	nos3

	mov	ebx,2*65536+7
;--------------------------------------
align 4
nos3:
	mov	ecx,[wcolor]
	add	ecx,0x303030
	mcall	4,,,hidetext,1

	mcall	12,2
	popa
	ret
;------------------------------------------------------------------------------	
align 4
draw_small_left:
	pusha
	mcall	12,1

	xor	eax,eax
	mov	edx,[wcolor]
	mov	esi,edx
	mov	edi,edx
	or	edx, 0x01000000
	mcall

	cmp	[graph_text],1
	je	nos4

	mcall	8,<0,9>,<0,12>,2,[wcolor]

	mov	ecx,[wcolor]
	add	ecx,0x303030
	mcall	4,<2,4>,,hidetext+2,1
;--------------------------------------
align 4
nos4:
	mov	ecx,13*65536+25
	cmp	[graph_text],1
	jne	nos6

	xor	ecx,ecx
	mov	cx,word [b_size_y]
;--------------------------------------
align 4
nos6:
	mcall	8,<0,9>,,1,[wcolor]

	mov	ebx,3*65536+22
	cmp	[graph_text],1
	jne	nos7

	mov	ebx,3*65536+7
;--------------------------------------
align 4
nos7:
	mov	ecx,[wcolor]
	add	ecx,0x303030
	mcall	4,,,hidetext+1,1

	mcall	12,2
	popa
	ret
;------------------------------------------------------------------------------	
align 4
right_button:
	mov	[small_draw],dword draw_small_right

	mcall	14

	shr	eax, 16
	mov	ebx, eax
	mov	ecx, -1
	mov	edx, 9
	sub	ebx, edx
	mov	esi, -1
	mcall	67

	call	draw_small_right
	jmp	small_wait
;------------------------------------------------------------------------------	
align 4
left_button:
	mov	[small_draw],dword draw_small_left

	xor	ebx,ebx
	mcall	67,,-1,9,-1

	call	draw_small_left
;--------------------------------------
align 4
small_wait:
	mcall	10
	cmp	eax,1
	jne	no_win

	call	[small_draw]
	jmp	small_wait
;--------------------------------------
align 4
no_win:
	cmp	eax,2
	jne	no_key

	call	handle_key
	jmp	small_wait
;--------------------------------------
align 4
no_key:
	mcall	17
	cmp	ah,1
	jne	no_full

	mcall	14	; get screen max x & max y

	mov	edx, eax
	shr	edx, 16
	xor	ebx, ebx
	mcall	67,,-1,,-1 ; x0 y0 xs ys

	jmp	still
;--------------------------------------
align 4
no_full:
	call	menu_handler
	jmp	small_wait
;------------------------------------------------------------------------------	
align 4
set_variables:
	pusha
	mov	[b_size_y],dword 38
	cmp	[graph_text],1
	jne	noy2

	mov	[b_size_y],dword 18
;--------------------------------------
align 4
noy2:
	mov	[button_frames],0x0
	cmp	[buttons],0
	jne	no_frames

	mov	[button_frames],0x40000000
;--------------------------------------
align 4
no_frames:
	mcall	48,1,1	; 3d button look

	mov	eax,0x40404040   ; dividers for processes
	mov	edi,pros
	mov	ecx,10
	cld
	rep	stosd

	popa
	ret
;------------------------------------------------------------------------------	
align 4
; eax = number (1 or 2)
; ebx = language id
draw_flag:
	pusha
; eax = 2 BIG
; eax = 1 small
	mov	edx,ebx

	mov	ebx,[maxx]
	and	eax,1
	imul	eax,17  ;17
	sub	ebx,eax
	sub	ebx,76 ;79 ;28

	pushad
;	dec	ebx
	sub	ebx,2
	shl	ebx, 16
	add	ebx, 15 ;25
	xor	edx,edx
	mcall	13,,<4,13>

	add	ebx,1 shl 16
	sub	ebx,2
	mov	ecx, 5 shl 16+11
	cmp	[type_lang],1
	je	label_1

	mov	edx,0xff ;[wcolor]
	jmp	label_2
;--------------------------------------
align 4
label_1:
	mov	edx,0x7700
;--------------------------------------
align 4
label_2:
	mcall
	popad

	shl	ebx,16
	add	ebx,7	;24

	mov	ecx,[bte] ; color

if caps_lock_check
; make flag_text 'enfigerufretua' or 'ENFIGERUFRETUA' depending on CapsLock state.
	call	flag_text_setup
end if
	dec	edx
	shl	edx,1
	add	edx,flag_text
	mcall	4,,,,2

	mov	ebx,[maxx]
	sub	ebx,48
	shl	ebx,16
	mov	bx,34
	xor	edx,edx
	mcall	13,,<3,14>

	add	ebx,1 shl 16
	sub	ebx,2
	mov	ecx,4 shl 16+12
	mov	edx,time_bgr_color
    mcall

    popa
    ret
;------------------------------------------------------------------------------	
align 4
if caps_lock_check
flag_text_setup:
	pusha
	mcall	66,3
	mov	edi, flag_text
	mov	esi, flag_text_caps_off
	test	eax, 0x40
	jz	@f
	
	add	esi, 14
;--------------------------------------
align 4
@@:
	mov	ecx, 14
	rep	movsb
	
	popa
	ret

flag_text_caps_off db 'enfigerufretua' 
db 'ENFIGERUFRETUA'
end if
;------------------------------------------------------------------------------	
; ***************************************************
; ********* WINDOW DEFINITIONS AND DRAW *************
; ***************************************************
align 4
draw_window:
	pusha
	mov	[running_applications],-1
	mov	[checks],-1
	mcall	12,1

	mcall	48,3,system_colours,10*4

	mov	eax, [system_colours+4*6]
	sub	eax, 0x101010
	mov	[wcolor], eax

	mcall	14	; get screen max x & max y

	cmp	[width],0
	je	no_def_width

	and	eax,0xffff
	mov	ebx,[width]
	shl	ebx,16
	add	eax,ebx
;--------------------------------------
align 4
no_def_width:
	mov	ebx,eax
	mov	[screenxy],ebx
	shr	ebx,16
	sub	ax,38
	shl	eax,16
	mov	ecx,eax
	add	ecx,0*65536+38
	cmp	[graph_text],1
	jne	no_text_1
	mov	cx,PANEL_HEIGHT
	add	ecx,20*65536
;--------------------------------------
align 4
no_text_1:
	xor	eax,eax 		    ; DEFINE AND DRAW WINDOW
	mov 	edx, [wcolor]
	or	edx, 0x01000000 ; do not draw the window
	mov	esi, [wcolor]
	or	esi, 0x01000000 ; unmovable window
	mov 	edi, [wcolor]
	mov	[panel_x_pos], ebx
	mov	[panel_y_pos], ecx	 ; Пока что так.
	mcall

	movzx	ebx,word [screenxy+2]
	mov	ecx,0*65536+0
	mov	edx,[wcolor]
	add	edx,0x161616
;--------------------------------------
align 4
newline_:			; debug.inc has macros with the same name
	sub	edx,0x040404
	mov	eax,38
	cmp	[soften_up],1
	jne	no_su

	and	edx,0x00FFFFFF
	mcall
;--------------------------------------
align 4
no_su:
	pusha
	cmp	[soften_down],1
	jne	no_sd

	sub	edx,0x141414
	mov	edi,[b_size_y]
	shl	edi,16
	add	edi,[b_size_y]
	add	ecx,edi
	sub	ecx,3*65536+3
	and	edx,0x00FFFFFF
	mcall
;--------------------------------------
align 4
no_sd:
	popa

	add	ecx,1*65536+1
	cmp	cx,5
	jb	newline_

	cmp	[soften_middle],1
	jne	no_sm

	movzx	ebx,word [screenxy+2]
	mov	ecx,5*65536+5
	mov	esi,stripe
	mov	edx,[wcolor]
;--------------------------------------
align 4
newline3:
	add	edx,[esi]
	add	esi,4

	and	edx,0x00FFFFFF
	mcall	38
	add	ecx,1*65536+1
	cmp	cx,15
	jb	newline3
;--------------------------------------
align 4
no_sm:
	cmp	[minimize_left],1
	jne	no_mleft

	mov	ecx,1 *65536
	add	ecx,[b_size_y]
	dec	ecx
	mov	edx,101
	add	edx,[button_frames]
	mcall	8,<0,9>,,,[wcolor]	; ABS LEFT

	mov	ebx,2*65536+17
	cmp	[graph_text],1
	jne	no_y1

	mov	bx,7
;--------------------------------------
align 4
no_y1:
	mov	ecx,[wcolor]
	add	ecx,0x303030
	mcall	4,,,hidetext,1	; HIDE TEXT
;--------------------------------------
align 4
no_mleft:
	movzx	eax,word [screenxy+2]
	mov	[maxx],eax
	cmp	[minimize_right],1
	jne	no_mright
	mov	eax,[maxx]
	sub	eax,77
	shl	eax,16
	mov	ebx,eax
	add	ebx,67

	mov	ecx,1 *65536
	add	ecx,[b_size_y]
	dec	ecx
	add	ebx,68*65536
	mov	bx,9
	mov	edx,102
	add	edx,[button_frames]
	mcall	8,,,,[wcolor]	; ABS RIGHT

	mov	edx,hidetext+1
	mov	eax,4
	mov	ebx,[maxx]
	sub	ebx,6
	shl	ebx,16
	mov	bx,17
	cmp	[graph_text],1
	jne	no_y2

	mov	bx,7
;--------------------------------------
align 4
no_y2:
	mov	ecx,[wcolor]
	add	ecx,0x303030
	mov	esi,1
	mcall
;--------------------------------------
align 4
no_mright:
	call	draw_menuet_icon
;	call	draw_program_icons
	mov	[ptime],0
	call	draw_info
	call	draw_application_buttons

	mcall	12,2
	popa
	ret
;------------------------------------------------------------------------------	
align 4
dr_button_clean:
	mcall	8,,<3,13>
	ret
;------------------------------------------------------------------------------	
align 4
draw_menuet_icon:
	pusha
; Inserted code for drawing buttons 103, 104, 105 (Clean, Restore, Exchange windows)
	mov	esi, dword [system_colours+24]    ; drawing buttons
	and	esi, 0x00ffffff
	mov	edx, 103
	mov	ebx, 67*65536+13
	call	dr_button_clean
	
	inc	edx
	mov	ebx, 86*65536+13
	call	dr_button_clean
	
	inc	edx
	mov	ebx, 105*65536+13
	call	dr_button_clean
	
	mov	ecx, dword [system_colours+28]
	and	ecx, 0x00ffffff
	mcall	4,<71,6>,,page_clean_but,1

	inc	edx
	mcall	,<90,6>

	inc	edx
	mcall	,<109,6>

	cmp	[menu_enable],1
	jne	no_menu

	mov	ebx, 10*65536 + 47
	cmp	[minimize_left], 0
	jne	@f

	sub	ebx, 10*65536
;--------------------------------------
align 4
@@:
	mov	ecx, 1*65536
	add	ecx, [b_size_y]
	dec	ecx
	mov	edx, 0x20d1ff01
	add	edx, [button_frames]
	mcall	8,,,,[wcolor]	; M BUTTON

;	cmp	[graph_text], 1
;	jne	no_mtext

	push	ebx
	mcall	13,<12,44>,<1,17>,0x383838

	mcall	,<56,1>,<2,15>

	mcall	,<57,1>,<4,11>

	mcall	,<58,1>,<6,7>

	mcall	,<13,43>,<2,15>,0x7700

	mcall	,<56,1>,<4,11>

	mcall	,<57,1>,<6,7>
	pop	ebx

	mov	bx,  7
	add	ebx, 8*65536

	mcall	4,,0x10ffffff,m_text,4

	popa
	ret
;--------------------------------------
;align 4
;no_mtext:
;	mov	eax,[wcolor]
;	mov	[m_icon+4],eax
;; load & display menuet.bmp
;	mcall	70,m_bmp_fileinfo

;	mov	eax,40
;	mov	ebx,0
;	mov	edi,image+53
;--------------------------------------
;align 4
;new_m_pix:
;	cmp	byte [edi], 10
;	jb	nopix

;	cmp	byte [edi+1], 10
;	jb	nopix

;	cmp	byte [edi+2], 10
;	jb	nopix

;	pusha
;	cmp	[minimize_left],0
;	jne	no_m_s2

;	sub	ebx,10
;--------------------------------------
;align 4
;no_m_s2:
;	mov	edx,[edi+1]
;	mov	ecx,eax
;	add	ebx,12
;	mcall	1	; point - WTF???!!!

;	popa
;--------------------------------------
;align 4
;nopix:
;	add	edi,3
;	add	ebx,1
;	cmp	ebx,40
;	jnz	new_m_pix

;	mov	ebx,0
;	dec	eax
;	jnz	new_m_pix
;--------------------------------------
align 4
no_menu:
	popa
	ret
;------------------------------------------------------------------------------	
;align 4
;draw_program_icons:
;	pusha

;	cmp	[icons],0
;	jne	dp_ret

;	mov	edi,1
;	push	edi
;--------------------------------------
;align 4
;new_icon_file:
;	pusha
;	mov	edx,[esp+32]
;	add	edx,10

;	push	edx
;	mov	ecx,1*65536
;	add	ecx,[b_size_y]
;	dec	ecx
;	mov	eax,edi
;	dec	eax
;	imul	eax,40
;	mov	ebx,eax
;	add	ebx,[icons_position]
;	shl	ebx,16
;	mov	bx,39
;	pop	edx

;	add	edx,[button_frames]
;	or	edx, 0x20000000
;	mcall	8,,,,[wcolor]
;	popa

;	mov	ecx,[esp]
;	add	ecx,48
;	mov	[iconf+6],cl

;	mcall	70,iconf_fileinfo

;	mov	eax,0
;	mov	ebx,32
;	mov	edi,image+51+32*33*3
;--------------------------------------
;align 4
;np2: 			    ; new pixel of file
;	mov	edx,[edi]
;	and	edx,0xffffff

;	cmp	eax,3		; Y draw limits
;	jb	nopix2

;	cmp	eax,36
;	jg	nopix2

;	cmp	ebx,38		; X draw limits
;	jg	nopix2

;	cmp	ebx,2
;	jb	nopix2

;	cmp	edx,0
;	jz	nopix2

;	cmp	[graph_text],1
;	jne	no_icon_text

;	pusha
;	mov	ebx,[esp+32]
;	dec	ebx
;	imul	ebx,40
;	add	ebx,8
;	add	ebx,[icons_position]
;	shl	ebx,16
;	mov	bx,7

;	mov	edx,[esp+32]
;	dec	edx
;	imul	edx,4
;	add	edx,mi_text
;	mcall	4,,0xffffff,,4
;	popa
;	jmp  nopix2
;--------------------------------------
;align 4
;no_icon_text:
;	mov	esi,[esp]
;	pusha
	
;	push	edx
;	mov	ecx,eax
;	add	ecx,2
;	mov	eax,esi
;	dec	eax
;	imul	eax,40
;	add	ebx,eax
;	add	ebx,3
;	add	ebx,[icons_position]
;	pop	edx

;	mcall	1	; WTF???!!!
;	popa
;--------------------------------------
;align 4
;nopix2:
;	sub	edi,3
;	dec	ebx
;	jnz	np2

;	mov	ebx,32
;	add	eax,1
;	cmp	eax,32
;	jnz	np2

;	add	dword [esp],1
;	mov	edi,[esp]
;	cmp	dword [esp],4
;	jbe	new_icon_file

;	add	esp,4

;	mov	ebx,40
;	imul	ebx,3
;	add	ebx,[icons_position]
;	add	ebx,10
;	shl	ebx,16
;	mov	bx,23
;	mcall	4,,[wcolor],gpl,3
;--------------------------------------
;align 4
;dp_ret:
;	popa
;	ret
;------------------------------------------------------------------------------	
align 4
draw_info:    ; draw cpu usage, time, date
    pusha
	cmp	[setup_enable],1
	jne	no_setup

	cmp	[minimize_right],0
	jne	no_m_r
	add	[maxx],10
;--------------------------------------
align 4
no_m_r:
	mov	eax,3
	mcall
	cmp	eax,[ptime]
	jz	_ret

	mov	[ptime],eax

   	call	draw_cpu_usage

	mov	eax,[maxx]   ; blink sec
	sub	eax,33
	shl	eax,16
	mov	ebx,eax
	add	ebx,9
	mcall	3

	cmp	[graph_text],1
	jne	no_y4

	sub	bx,2
;--------------------------------------
align 4
no_y4:
	mov	ecx,eax
	shr	ecx,16
	and	ecx,1
	mov	edx,[bte]
	sub	edx,time_bgr_color;[wcolor]
	imul	ecx,edx
	add	ecx,time_bgr_color;[wcolor]

	mcall	4,,,sec,1

	mcall	26,2,9

	mov	edx,eax
	mcall	3

	and	eax,0xffff
	add	edx,eax
	cmp	edx,[checks]
	je	_ret

	mov	[checks],edx

	mov	ebx,[maxx]
	sub	ebx,48 ;;94 ;;74
	shl	ebx,16
	add	ebx,33 ;;84 ;;64

	mov	ecx,3 *65536
	add	ecx,[b_size_y]
	sub	cx,5
	mcall	8,,,0x40000002,[wcolor]	; time/date button

	and	edx,0xffff
	add	edx,[button_frames]
	mcall

	mov	ebx,[maxx]
	sub	ebx,77 ;80
	shl	ebx,16
	add	ebx,14

	mcall	,,<4,12>,0x60000010,[wcolor]	;button 16

	sub	ebx,17 shl 16
	inc	edx		    ;button 17

	add	ebx,33 shl 16
	mov	bx,8
	inc	edx		    ;button 18
	mcall

	sub	ebx,30 shl 16
	mov	bx,10
	inc	edx		    ;button 19
	mcall

	sub	ebx,14 shl 16
	inc	edx		    ;button 20
	mcall

	sub	ebx,12 shl 16
	mov	bx,8
	mov	ecx,6 shl 16+10
	inc	edx	    ;button 21
	mcall

	sub	ebx,24 shl 16
	inc	edx	    ;button 22
	mcall

	mcall	26,2,9

	mov	ebx,eax

	mov	eax,2
	mov	[type_lang],al
   	call	draw_flag

	mcall	18,8,1

	mov	[sound_flag],al

	mov	ebx,[maxx]
	sub	ebx,92 ;109 ;112 ;28
	shl	ebx,16
	mov	bx,12
	xor	edx,edx
	mcall	13,,<4,13>

	add	ebx,1 shl 16
	sub	bx,2
	mcall	,,<5,11>,0xcc

	add	ebx,1 shl 16
	mov	bx,5
	mcall	,,<8,5>,0xdddd00

	add	ebx,5 shl 16
	mov	bx,1
	mcall	,,<7,7>

	add	ebx,1 shl 16
	mcall	,,<6,9>

	add	ebx,1 shl 16
	mcall	,,<5,11>

	cmp	[sound_flag],0
	je	dalshe

	sub	ebx,8 shl 16
	ror	ebx,16
	mov	cx,bx
	rol	ebx,16
	mov	bx,cx
	add	bx,8
	mcall	38,,<5,15>,0xff0000

	add	ebx,1 shl 16
	inc	bx
	mcall

	rol	ecx,16
	mcall

	sub	ebx,1 shl 16
	dec	bx
	mcall
;--------------------------------------
align 4
dalshe:
	mov	ebx,[maxx]
	sub	ebx,106;123
	shl	ebx,16
	mov	bx,12
	xor	edx,edx
	mcall	13,,<4,13>

	add	ebx,1 shl 16
	sub	bx,2
	mcall	,,<5,11>,0xffcc00

	mov	ebx,[maxx]
	sub	ebx,104;121
	shl	ebx,16
	mov	bx,7

	mcall	4,,0x10000000,file_sys,1

	add	ebx,1 shl 16
	mcall

	mov	ebx,[maxx]
	sub	ebx,117;134
	shl	ebx,16
	mov	bx,9
	xor	edx,edx
	mcall	13,,<6,11>

	sub	ebx,24 shl 16 ; 18 shl 16
	mcall

	add	ebx,25 shl 16 ; 19 shl 16
	sub	bx,2
	mcall	,,<7,9>,0xffffff

	sub	ebx,24 shl 16
	mcall

	mov	ebx,[maxx]
	sub	ebx, 139
	shl	ebx,16
	mov	bx,8
	mcall	4,,,page_a1,5

	add	ebx,1 shl 16
	mcall

	mov	edx,[maxx]
	sub	edx,130
	shl	edx,16
	mov	dx,7
	mcall	47,0x20000,[page_list],,0xffffff

	mcall	3	; get time

	movzx	ebx,al
	shr	eax,8
	movzx	ecx,al
	shr	eax,8
	movzx	edx,al
; ebx ecx edx h m s
	push	ebx
	push	ecx

	mov	eax,[maxx]
	sub	eax,32
	shl	eax,16
	mov	ebx,eax
	add	ebx,9

	mov	ecx,[bte]

	cmp	[graph_text],1
	jne	no_y3

	sub	bx,2
;--------------------------------------
align 4
no_y3:
	mov	edx,[esp]	       ; __:_X
	and	edx,15
	add	ebx,10*65536
	add	edx,text
	mcall	4,,0xffffff,,1

	pop	edx			; __:X_
	shr	edx,4
	and	edx,15
	sub	ebx,6*65536
	add	edx,text
	mcall	

	mov	edx,[esp]	       ; _X:__
	and	edx,15
	sub	ebx,11*65536
	add	edx,text
	mcall

	pop	edx			; X_:__
	shr	edx,4
	and	edx,15
	sub	ebx,6*65536
	add	edx,text
	mcall

   	call	draw_cpu_usage
;--------------------------------------
align 4
_ret:
	cmp	[minimize_right],0
	jne	no_m_r2

	sub	[maxx],10
;--------------------------------------
align 4
no_m_r2:
no_setup:
	popa
	ret
;------------------------------------------------------------------------------	
align 4
draw_cpu_usage:
	pushad
	mov	[ysi],30
	cmp	[graph_text],1
	jne	@f

	mov	[ysi],12
;--------------------------------------
align 4
@@:
	mcall	18,5	; TSC / SEC

	shr	eax,20
	push	eax
	mcall	18,4	; IDLE / SEC

	shr	eax,20
	xor	edx,edx
	imul	eax,[ysi]
	cdq
	pop	ebx
	inc	ebx
	div	ebx
	cmp	eax,[ysi]
	jng	no_bug

	mov	eax,[ysi]
;--------------------------------------
align 4
no_bug:
	push	eax

	mov	ebx,[maxx]
	sub	ebx,60
	shl	ebx,16
	add	ebx,8
	mov	ecx,5 shl 16
	add	ecx,[ysi]
	mcall	13,,,0xdd2222

	pop	eax
	mov	ecx,5 shl 16
	add	ecx,eax
	mov	eax,13
	mcall	13,,,0x44aa44

	popad
	ret
;------------------------------------------------------------------------------	
align 4
include 'ctx_menu.asm'
;------------------------------------------------------------------------------	
align 4
; DATA
stripe:
	dd -0x010101
	dd -0x010101
	dd -0x020202
	dd -0x010101
	dd -0x000000

	dd 0x000000
	dd 0x010101
	dd 0x020202
	dd 0x010101
	dd 0x010101

m_icon:
	dd 0x0
	dd 0x808080
	dd 0x000000
	dd 0x000000
	dd 0xffffff


lsz m_text,\
  ru, "МЕНЮ",\
  en, "MENU",\
  et, "MEN--"

mi_text		db 'WAVETETRBGRDGPL '

flag_text	db 'EnFiGeRuFrEtUa'

type_lang	db 0

sound_flag	db 0
button_frames	dd 0x0

checks		dd -1
hidetext	db 0x11,0x10,0x1e

turn_text	db  '><'
gpl		db  'GPL'

chlang	db 'LANG',0
;syslang db 'SLAN',0

contrast	db 0

running_applications	dd 0x100
max_applications	dd 11

current_alt_tab_app	dd -1

page_list		dd 0
draw_start_position	dd 0
draw_window_1	db 0

b_size_y:	dd 0x0
ysi		dd 0
small_draw	dd 0x0

ptime		dd 0x0
maxx		dd 0x0
text		db '0123456789'

page_a1		db '<   >'
page_clean_but	db 25,24,18

bte	dd 0xccddee

wcolor	dd 0x506070

sec	db ': '
pros	db '                                                  '
	db '                                                  '

screenxy    dd	0x0
stcount     dd	0x0

setup_exec:
		dd 7
		dd 0
.cmdline	dd 0
		dd 0
		dd 0
		db '/sys/'
file_sys	db 'SETUP',0

exec_fileinfo:
	dd 7
	dd 0
	dd 0
	dd 0
	dd 0
	db 0
.name	dd 0

end_name	db '/sys/END',0
menu_name	db '/sys/@MENU',0
calendar_name	db '/sys/CALENDAR',0
sysmeter_name	db '/sys/GMON',0
mousemul_name	db '/sys/MOUSEMUL',0

dat_fileinfo:
	dd 0
	dd 0
	dd 0
	dd 1024
	dd I_END
	db '/sys/PANEL.DAT',0

;m_bmp_fileinfo:
;	dd 0
;	dd 0
;	dd 0
;	dd 8192
;	dd image
;	db '/sys/MENUET.BMP',0

;iconf_fileinfo:
;	dd 0
;	dd 0
;	dd 0
;	dd 8192
;	dd image
;	db '/sys/'
;iconf	db 'MBAR_IX.BMP',0

this_slot	dd 0	      ; Slot of this process
max_slot	dd 255
;------------------------------------------------------------------------------	
IM_END:
;------------------------------------------------------------------------------	
align 4
last_active_window	rd 1
prev_active_window	rd 1
active_window_changed	rd 1
app_tab_count		rd 1

alt_tab_list_size	rd 1

panel_x_pos	rd	1
panel_y_pos	rd	1

screen_size:
.height	rw 1
.width	rw 1
;------------------------------------------------------------------------------	
align 4
system_colours	system_colors ; rd 10
;------------------------------------------------------------------------------	
align 4
app_list	rd 50
;------------------------------------------------------------------------------	
align 4
alt_tab_list	rd 256*2
;------------------------------------------------------------------------------	
align 4
process_info_buffer:
	rb 1024
;------------------------------------------------------------------------------	
align 4
procinfo_for_detect:
	rb 1024
;------------------------------------------------------------------------------	
align 4
procinfo_window_tabs:
	rb 1024
;-----------------------------------------------------------------------------
align 4
tictable:
	rd 256
;------------------------------------------------------------------------------	
align 4
	rb 256
detect_stack:
;------------------------------------------------------------------------------	
align 4
	rb 256
ctx_menu_stack:
;------------------------------------------------------------------------------
align 4
	rb 0x1000
stack_top:
;------------------------------------------------------------------------------	
;align 4
;image:
;------------------------------------------------------------------------------	
I_END: