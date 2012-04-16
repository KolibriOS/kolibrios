;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                   ;
;   TASK PANEL for KolibriOS  - Compile with fasm   ;
;                                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;------------------------------------------------------------------------------
; version:	2.0
; last update:  17/04/2012
; changed by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      New logic of switching windows (turnoff/restore)
;               New logic of button "clear desktop".
;               Win+D (restore/clear desktop), Win+R (start RUN application).
;               Using the library LibINI to set the parameters.
;               New style of panel. Start application Menu with boot options.
;               two versions of the location of the panel - 
;               the bottom of the desktop and on top of the desktop.
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
	dd IM_END	; program image size
	dd I_END	; reguired amount of memory - 10 Kb
	dd stack_top	; esp
	dd 0x0		; boot parameters
	dd path		; application pach
;------------------------------------------------------------------------------
include 'lang.inc'
include '../../../macros.inc'
include '../../../proc32.inc'
;include 'debug.inc'
include 'MOI.INC'	;раскладки клавиатуры
include '../../../develop/libraries/box_lib/load_lib.mac'
        @use_library    ;use load lib macros
;------------------------------------------------------------------------------
caps_lock_check fix 1
;------------------------------------------------------------------------------
time_bgr_color = 0x66cc
;PANEL_HEIGHT = 18
ML_SIZE = 10
MR_SIZE = 10
MENU_SIZE = 50
CLD_SIZE = 20
TAB_SIZE = 60
TRAY_SIZE = 140

CLOCK_SIZE = 40
CPU_USAGE_SIZE = 10
CHLANG_SIZE = 20
PAGE_LIST_SIZE = 36
;------------------------------------------------------------------------------
align 4
handle_key:
	mcall	18,7
	mov	[active_process],eax

	mcall	2
	
;	dps	"panel key: "
;	dph	eax
;	newline

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
;align 4
;prod:
if caps_lock_check
	cmp	al,58	;CAPS LOCK DOWN
	jne	@f

	pusha
	mcall	26,2,9
	mov	ebx,eax
	mov	eax,2
;	call	draw_window ;;;???
	mov	[draw_flag_certainly],1
   	call	draw_flag	; language
	popa
	ret
@@:
end if
	cmp	al, 15	; Alt+Tab DOWN
	jz	alt_tab_pressed

	cmp	al, 88	; Ctrl+Alt+F12
	jz	start_end_application

	
	
	cmp	al, 91	; RWin DOWN
	jz	set_win_key_flag

	cmp	al, 92	; LWin DOWN
	jz	set_win_key_flag
	
	
	cmp	al, 219	; RWin UP
	jz	cut_win_key_flag	;start_menu_application

	cmp	al, 220	; LWin UP
	jz	cut_win_key_flag	;start_menu_application
	

	cmp	al, 62	; Alt+F4 
	jz	kill_active_application

;	cmp	al, 71
;	jz	page_list_next

;	cmp	al, 72
;	jz	page_list_prev

	cmp	al, 69	; Alt+Shift+NumLock
	jz	start_mousemul_application
	
	cmp	al, 19	; Win+R
	jz	start_run_application

	cmp	al, 32	; Win+D
	jz	minimize_all_windows
;--------------------------------------
align 4
prod:
;	cmp	eax,0x20000
;	je	start_menu_application
	
;	cmp	eax,0x40000
;	je	start_menu_application

	cmp	[current_alt_tab_app], -1
	jz	@f

	test	ah, 0x30	; Alt+Tab UP
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
set_win_key_flag:
	mov	[win_key_flag],1
	ret
;------------------------------------------------------------------------------
align 4
cut_win_key_flag:
	xor	eax,eax
	mov	[win_key_flag],al
	xchg	[start_menu_flag],al
	test	al,al
	jz	start_menu_application
	ret
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
	call	menu_handler
	jmp	begin_1.ret
;------------------------------------------------------------------------------
align 4
start_run_application:
	cmp	[win_key_flag],1
	je	@f
	ret
@@:
	mov	[start_menu_flag],1
	mov	[draw_window_1], 1
	mov	dword [ebx+21], run_name
	mcall	70
	jmp	begin_1.ret
;------------------------------------------------------------------------------
align 4
minimize_all_windows:
	cmp	[win_key_flag],1
	je	@f
	ret
@@:
	mov	[start_menu_flag],1
	call	clean_desktop
	ret
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
	mov	ebx, procinfo_window_tabs
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

exit:
	mcall -1
;------------------------------------------------------------------------------
START:
	mcall	68,11
	mcall	66,1,1
	
	mcall	9,process_info_buffer,-1
	mov	ecx,[ebx+30]	; PID
	mcall	18,21
	mov	[my_active_slot],eax	; WINDOW SLOT
	
	
load_libraries l_libs_start,end_l_libs

	test	eax,eax
	jnz	exit

	call	Get_ini

	mcall	48,1,[button_style]
	
	mcall	66,4,0,2	; LShift+RShift
	mcall	66,,,11h	; Ctrl+Shift
	mcall	66,,88,110h	; Alt+Ctrl+F12
	mcall	66,,91,0h 	; RWin DOWN
	mcall	66,,92 		; LWin DOWN
	mcall	66,,219 	; RWin UP
	mcall	66,,220		; LWin UP

;	mcall	66,,147,0h 	; Win+R UP
	mcall	66,,19,0h 	; Win+R DOWN

;	mcall	66,,23,0h 	; Win+I DOWN

	mcall	66,,32,0h 	; Win+D DOWN
;	mcall	66,,160,0h 	; Win+D UP


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
	mcall	14
	mov	[screen_size],eax

	
	mov	ecx,eax
;eax = [xsize]*65536 + [ysize], где
	shr	ecx,16
	and	eax,0xFFFF
	
	cmp	[place_attachment],1
	je	@f
	ror	eax,16
	add	eax,[height]
	rol	eax,16
	mov	edx,eax
	jmp	.selected
;--------------------------------------
align 4
@@:
	sub	eax,[height]
	lea	edx,[eax-1]
;--------------------------------------
align 4
.selected:
	mcall	48,6

	mcall	9,process_info_buffer,-1
	mov	ecx,[process_info_buffer+30]
	mcall	18,21
	mov	[this_slot],eax
	mov	[max_slot],255
;--------------------------------------
align 4
start_after_minimize:
	call	draw_window
;	call	draw_tray
	call	draw_running_applications

	mcall	23,30
	call	load_ini
	call	fir_lng
;------------------------------------------------------------------------------
align 4
still:
;	call	draw_tray
	call	draw_time
   	call	draw_cpu_usage
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
	
	push	eax
	mcall	18,7
	cmp	[my_active_slot],eax
	je	@f
	mov	[current_active_slot],eax
@@:
	pop	eax
	
	cmp	eax,1		; redraw ?
	jz	red

	cmp	eax,2
	jnz	@f
	call	handle_key
	jmp	.key
@@:
	cmp	eax,3		; button ?
	jz	button

;	call	handle_key
.key:
	cmp	dword [active_window_changed], 0
	jnz	red_active

   	call	draw_flag	; language
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
	sub	ecx,[height]
	mcall	67,0,,,[height]
;--------------------------------------
align 4
.lp1:
	call	draw_window
;	call	draw_tray
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
	
	mcall	9,process_info_buffer

	test	[ebx+70],dword 10b	; window is minimized to panel
	jnz	@f

	cmp	ecx,[current_active_slot]
	je	.turn_off
@@:
	mcall	18,3,
	jmp	.task_switching
;--------------------------------------
align 4
.turn_off:
	mov	edx,ecx
	mcall	18,22,0
.task_switching:
	mcall	68,1
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
	je	clean_desktop_1		    ; minimize all windows

;	cmp	ah, 104
;	je	restore_desktop	    ; restore minimized windows

;	cmp	ah, 105
;	je	swap_minimized_desktop     ; minimize normal windows, and restore minimized windows

	cmp	ah,byte 1	  ; start/terminate menu
	jnz	noselect

	call	menu_handler

	jmp	still
;--------------------------------------
align 4
clean_desktop_1:


	call	clean_desktop	
	
	
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

;	mov	ebx, setup_exec
;	mov	dword [ebx+8], chlang
;	mcall	70
	call	karu
   	call	draw_flag	; language
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
;	cmp	ah,19	; speaker
;	jne	noid19

;	mcall	18,8,2
;	mcall	15,3
;	jmp	red
;--------------------------------------
;align 4	
;noid19:
;	cmp	ah,20		   ; start system setup
;	jnz	noid20
;
;	mov	ebx, setup_exec
;	and	dword [ebx+8], 0
;	mcall	70
;	jmp	still
;--------------------------------------
;align 4	
;noid20:
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
;	jmp	still
	ret
;------------------------------------------------------------------------------
;align 4
;swap_minimized_desktop:
;	xor	esi, esi
;	inc	esi
;	jmp	min_swap_desktop
;------------------------------------------------------------------------------
align 4
clean_desktop:
	mov	[clean_desktop_minimize],0
;	xor	esi,esi
;--------------------------------------
;align 4	
;min_swap_desktop:		   ; input esi 0 - min, <>0 swap
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

	pusha
	mov	edx, ecx
	xor	ecx, ecx
;	or	esi, esi
;	jz	@f

	movzx	eax, byte [process_info_buffer+70]
	and	eax, 2	      ; mask minimize
	jnz	@f

	mov	[clean_desktop_minimize],1
;	mov	ecx, 2	      ; restore
	mcall	18, 22
;--------------------------------------
align 4
@@:
	popa
;--------------------------------------
align 4
.loop1xx:
	inc	ecx
	cmp	ecx, [max_slot]
	jbe	.loop1
	
	cmp	[clean_desktop_minimize],1
	je	@f
	call	restore_desktop
@@:
	ret
;	jmp	still
;------------------------------------------------------------------------------
align 4
conversion_HEX_to_ASCII:
	ror	eax,12
	mov	ecx,4
;--------------------------------------
align 4
.loop:
	mov	bl,al
	rol	eax,4
	and	bl,0xf
	cmp	bl,0xA	; check for ABCDEF
	jae	@f

	add	bl,0x30	; 0-9
	jmp	.store
;--------------------------------------
align 4
@@:
	add	bl,0x57 ; A-F
;--------------------------------------
align 4
.store:
;	dps	"param_store: "
	mov	[edi],bl
	inc	edi
	dec	ecx
	jnz	.loop
	ret
;------------------------------------------------------------------------------
align 4
menu_handler:
	mov	edi,bootparam
	
	mov	eax,[menu_button_x.start]
	call	conversion_HEX_to_ASCII

	mov	eax,[menu_button_x.size]
	call	conversion_HEX_to_ASCII
	
	mov	eax,[menu_button_y.start]
	call	conversion_HEX_to_ASCII
	
	mov	eax,[menu_button_y.size]
	call	conversion_HEX_to_ASCII
	
	mov	eax,[height]
	call	conversion_HEX_to_ASCII
	
	mov	eax,[place_attachment]
	call	conversion_HEX_to_ASCII
	
	xor	eax,eax	; terminator for boot parameters string
	stosd
	
	mov	ebx, exec_fileinfo
	mov	[ebx+21],dword menu_name
	mov	[ebx+8],dword bootparam
	mcall	70
	mov	[ebx+8],dword 0
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
	mov	cx,[height]
	mcall	8,<0,9>,,1,[wcolor]

	mov	ebx,2*65536	;+6
	mov	bx,[height]
	shr	bx,1
	sub	bx,3
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

	xor	ecx,ecx
	mov	cx,[height]
	mcall	8,<0,9>,,1,[wcolor]

	mov	ebx,3*65536	;+6
	mov	bx,[height]
	shr	bx,1
	sub	bx,3
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
;align 4
;set_variables:
;	pusha
;	mov	[button_frames],0x0
;	cmp	[buttons],0
;	jne	no_frames

;	mov	[button_frames],0x40000000
;--------------------------------------
;align 4
;no_frames:
;	mcall	48,1,1	; 3d button look
;	popa
;	ret
;------------------------------------------------------------------------------
include 'libini.inc'
;------------------------------------------------------------------------------
include 'dll.inc'
;------------------------------------------------------------------------------
include 'drawappl.inc'
;------------------------------------------------------------------------------
include 'drawbutt.inc'
;------------------------------------------------------------------------------
include 'drawwind.inc'
;------------------------------------------------------------------------------
include 'drawtray.inc'
;------------------------------------------------------------------------------
include 'ctx_menu.asm'
;------------------------------------------------------------------------------
include 'i_data.inc'
;------------------------------------------------------------------------------
IM_END:
;------------------------------------------------------------------------------
include 'u_data.inc'
;------------------------------------------------------------------------------
I_END:
;------------------------------------------------------------------------------
