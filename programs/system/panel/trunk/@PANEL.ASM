;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                                   ;
;    MENUBAR for KolibriOS  - Compile with fasm     ;
;                                                   ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

use32
  org  0x0
  db   'MENUET01'   ; 8 byte id
  dd   0x01	    ; header version
  dd   START	    ; program start
  dd   I_END	    ; program image size
  dd   0xA000	    ; reguired amount of memory - 10 Kb
  dd   0xA000	    ; esp
  dd   0x0,0x0	    ; param, icon

include 'lang.inc'
include '..\..\..\macros.inc'
include 'moi.inc'   ;раскладки клавиатуры

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


handle_key:

    mcall 18, 7
    mov  [active_process],eax

	mcall 2
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
prod:
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
@@:
; this is hotkey Ctrl+Shift ;or LShift+RShift

	 jmp	 karu
;        mov     ebx, setup_exec
;        test    ah, 001100b
;        jz      change_sys_lang
change_key_lang:
	mov	dword [ebx+8], chlang
	mcall	70
	call	chlang_music
;       mcall   5, 25
begin_1:
    mov  ecx,[active_process]
    mcall 18, 3
    mcall 5, 25
.ret:
	ret

;change_sys_lang:
;        mov     dword [ebx+8], syslang
;        mcall   70
;        call    syslang_music
;;       mcall   5, 25
;        jmp     begin_1

  start_end_application:
	mov	dword [ebx+21], end_name
	mcall	70
	ret
  start_mousemul_application:
	mov	dword [ebx+21], mousemul_name
	mcall	70
	ret

   kill_active_application:
;     mcall  18, 7
;     mov    ecx,eax
;
;     ;//{SPraid.simba do not kill panel and icon
;     push eax
;     mov eax,9
;     mov ebx, process_info_buffer
;     int 0x40
;     mov eax,process_info_buffer
;     add eax,10
;     mov ebx,[eax]
;     cmp ebx,'ICON'
;     je  no_kill
;     cmp ebx,'@PAN'
;     jne  kill_app
;     add eax,4
;     mov ebx,[eax]
;     and ebx,0x0000FFFF
;     cmp ebx,'EL'
;     je  no_kill
;   kill_app:
;     pop ecx
;     mcall  18, 2
;     jmp if_kill
;   no_kill:
;     pop eax
;   if_kill:
;     ;// }SPraid.simba
	mcall 72,1,3,1

     jmp    begin_1.ret

   start_menu_application:
	mov	[draw_window_1], 1
	mov	dword [ebx+21], menu_name
	mcall	70
	call	menu_music
     mcall 5,50
     jmp   begin_1.ret

;page_list_next:
;    cmp  [page_list],15
;    je	   @f
;    inc  [page_list]
;    mov  [draw_window_1],1
;  @@:
;    jmp  begin_1.ret
;
;page_list_prev:
;    cmp  [page_list],0
;    je	   @f
;    dec  [page_list]
;    mov  [draw_window_1],1
;  @@:
;    jmp  begin_1.ret

alt_tab_pressed:
; handle Alt+Tab and Alt+Shift+Tab
	mov	ebp, eax
	cmp	[current_alt_tab_app], -1
	jnz	has_alt_tab_app
; заполняем таблицу приложений, подлежащих переключению
	xor	edx, edx
	mov	ebx, 0x8000
	mov	ecx, 1
	mov	eax, 9
.fill:
	inc	ecx
	int	0x40
	call	need_window_tab
	jz	@f
	cmp	edx, 256
	jz	@f
	mov	[alt_tab_list+edx*8], ecx
	movzx	esi, word [ebx+4]
	mov	[alt_tab_list+edx*8+4], esi
	inc	edx
@@:
	cmp	ecx, eax
	mov	eax, 9
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
.findmax:
	cmp	[alt_tab_list+ecx*8+4], eax
	jb	@f
	mov	edx, ecx
	mov	eax, [alt_tab_list+ecx*8+4]
@@:
	inc	ecx
	cmp	ecx, [alt_tab_list_size]
	jb	.findmax
	mov	[current_alt_tab_app], edx
has_alt_tab_app:
	mov	eax, [current_alt_tab_app]
	mov	edx, [alt_tab_list+eax*8+4]	; slot
	xor	ecx, ecx
	or	eax, -1
	test	ebp, 300h
	jz	.notshift
	or	esi, -1
.loop1:
	cmp	[alt_tab_list+ecx*8+4], edx
	jbe	@f
	cmp	[alt_tab_list+ecx*8+4], esi
	jae	@f
	mov	eax, ecx
	mov	esi, [alt_tab_list+ecx*8+4]
@@:
	inc	ecx
	cmp	ecx, [alt_tab_list_size]
	jb	.loop1
	cmp	eax, -1
	jnz	.found
	xor	edx, edx
	xor	ecx, ecx
	jmp	.loop1
.notshift:
	xor	esi, esi
.loop2:
	cmp	[alt_tab_list+ecx*8+4], edx
	jae	@f
	cmp	[alt_tab_list+ecx*8+4], esi
	jbe	@f
	mov	eax, ecx
	mov	esi, [alt_tab_list+ecx*8+4]
@@:
	inc	ecx
	cmp	ecx, [alt_tab_list_size]
	jb	.loop2
	cmp	eax, -1
	jnz	.found
	or	edx, -1
	xor	ecx, ecx
	jmp	.loop2
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
@@:
	cmp	ecx, [alt_tab_list_size]
	jae	redraw_window_tabs
	mov	eax, [alt_tab_list+ecx*8]
	stosd
	inc	ecx
	jmp	@b

alt_tab_released:
	mcall	66,5,0,0	; уже поймали, хватит :)
	or	eax, -1
	xchg	eax, [current_alt_tab_app]
	mov	ecx, [alt_tab_list+eax*8]
	mov	eax, 18
	mov	ebx, 3
	int	0x40
	jmp	redraw_window_tabs

active_process	dd 0

calendar_music:
    mcall 55, eax, , , calendarmusic
    ret
setup_music:
    mcall 55,eax, , ,setupmusic
    ret
sysmeter_music:
    mcall 55,eax, , ,sysmetermusic
    ret
button_music:
    mcall 55,eax, , ,buttonmusic
    ret
;syslang_music:
;    mcall 55, eax, , , syslangmusic
;    ret
chlang_music:
    mcall 55, eax, , , chlangmusic
    ret
menu_music:
    mcall 55,eax, , ,menumusic
    ret

chlangmusic:	db 0x82,0x60,0x83,0x65,0x82,0x60,0

;syslangmusic:   db 0x82,0x65,0x83,0x70,0x82,0x65,0

menumusic:	db 0x82,0x50,0x84,0x48,0x82,0x50,0x84,0x53,0x82,0x51,0

activatemusic:	db 0x83,0x30,0x85,0x60,0

buttonmusic:	db 0x85,0x25,0x85,0x40,0

sysmetermusic:	db 0x85,0x35,0x85,0x45,0

setupmusic:	db 0x85,0x40,0x85,0x50,0

calendarmusic:	db 0x85,0x37,0x85,0x48,0

;  .exit: mcall -1


START:
;       mov     eax, 51
;       mov     ebx, 1
;       mov     ecx, detect_start
;       mov     edx, detect_stack
;       int     0x40
	mcall 66,4,0,2		; LShift+RShift
	mcall 66, , ,11h	; Ctrl+Shift
	mcall 66,,88,110h	; Alt+Ctrl+F12
	mcall 66,,91,0h 	; LWin
	mcall 66,,92 		; RWin
;	mcall 66,,91,100h	; Alt+LWin
;	mcall 66,,92		; Alt+RWin
	mcall 66,,62,100h	; Alt+F4
;	mcall 66,,71		; Alt+Home
;	mcall 66,,72		; Alt+Up
	mcall 66,,15		; Alt+Tab
	mcall 66,,2		; Alt+1
	mcall 66,,3		; Alt+2
	mcall 66,,4		; Alt+3
	mcall 66,,5		; Alt+4
	mcall 66,,6		; Alt+5
	mcall 66,,7		; Alt+6
	mcall 66,,8		; Alt+7
	mcall 66,,,101h 	; Alt+Shift+Tab
	mcall 66,,69		; Alt+Shift+NumLock

    mcall 18, 8, 1
    test eax, eax
    jne  @f
    mcall 18, 8, 2
  @@:
	mov	eax, 70
	mov	ebx, dat_fileinfo
	int	0x40

    mov  edi,width
    mov  esi,I_END
    xor  eax,eax
  new_number:
    cmp  [esi],byte ';'
    je	 number_ready
    imul eax,10
    movzx ebx,byte [esi]
    sub  ebx,'0'
    add  eax,ebx
    inc  esi
    jmp  new_number
  number_ready:
    stosd
    xor  eax,eax
    inc  esi
    cmp  [esi],byte 'x'
    jne  new_number

	mcall	14
	mov	[screen_size],eax

	;mcall  48,5            ; barsuk {
	mov	eax, 14
	int	0x40
	mov	ecx,eax
;eax = [xsize]*65536 + [ysize], где
	shr	ecx, 16
	and	eax, 0xFFFF
	lea	edx,[eax-PANEL_HEIGHT-1]
				; barsuk }

	mcall	48,6

; // Alver 26.08.2007 // {
   mov	eax, 9			    ; info of process
   mov	ebx, process_info_buffer
   mov	ecx, -1
   mcall
   mov	ecx, dword [process_info_buffer+30]
   mcall 18,21
   mov	[this_slot], eax
   mov	[max_slot], 255
; } \\ Alver \\

    call set_variables

start_after_minimize:

    call draw_window
    call draw_info
    call draw_running_applications

    mov  eax, 23
    mov  ebx, 30
    int  0x40
    call    load_ini
    call    fir_lng

still:
;     mcall  13,<390,70>,<3,11>,0xffffff
;     mov    ecx,[button_presssed_alt]
;     mcall  47,0x80100,ecx ,400 shl 16+5,0

    call draw_info
    call draw_running_applications

	mov	eax, 18 			; check if active window changed
	mov	ebx, 7				; barsuk
	int	0x40
	cmp	eax, [last_active_window]
	jz	@f

; need_window_tab:
; in: ebx->process info
; out: ZF set <=> do not draw

	mov	ebx, [last_active_window]
	mov	[prev_active_window], ebx
	mov	[last_active_window], eax

	mov	ecx, eax
	mov	eax, 9
	mov	ebx, process_info_buffer
	int	0x40

	call	need_window_tab
	jnz	.need_repaint

	mov	eax, 9
	mov	ebx, process_info_buffer
	mov	ecx, [prev_active_window]
	int	0x40
	call	need_window_tab
	jz	@f

.need_repaint:
	mov	dword [active_window_changed], 1

@@:

    mov  eax, 23
    mov  ebx, 20
    int  0x40

    cmp  eax,1		; redraw ?
    jz	 red
    cmp  eax,3		; button ?
    jz	 button
	call	handle_key

	cmp  dword [active_window_changed], 0
	jnz   red_active

	jmp	still

  red_active:		; barsuk

  red:			 ; redraw window

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
    @@: mov	[screen_size],eax
	sub	ecx,PANEL_HEIGHT
	mcall	67,0,,,PANEL_HEIGHT

  .lp1:

    call draw_window
    call draw_info
    jmp  still

  button:		 ; button
    mov  eax,17
    int  0x40

    cmp  al, 0			; barsuk
    jnz  right_mouse

    cmp  ah,50
    jb	 no_activate
    cmp  ah,70
    jg	 no_activate

    movzx ecx,byte ah
    sub  ecx,52
    shl  ecx,2

    mov  eax,18
    mov  ebx,3
    mov  ecx,[app_list+ecx]
    int  0x40
;    cmp  [music_type],0
;    je   still
    mcall 55,eax, , ,activatemusic
    jmp  still

right_mouse:			; barsuk

    cmp  ah,50
    jb	 still
    cmp  ah,70
    jg	 still

    movzx ecx,byte ah
    sub  ecx,52

    cmp  ecx, [app_tab_count]
    jge  still

    shl  ecx,2

    mov  eax, 37
    mov  ebx, 0
    int  0x40
    mov  ebx, eax
    shr  eax, 16
    mov  [x_coord], ax
    and  ebx, 0xFFFF
    mov  [y_coord], bx

    mov  eax, 9
    mov  ecx,[app_list+ecx]	; ecx->selected app.slot
    mov  [n_slot], ecx
    mov  ebx, procinfo_for_detect
    int  0x40

;    cmp  dword [procinfo_for_detect+process_information.slot_state],9
;    jz   still

	mov	eax, 51
	mov	ebx, 1
	mov	ecx, context_menu_start
	mov	edx, ctx_menu_stack
	int	0x40

	mov	[ctx_menu_PID], eax

    jmp  still


  no_activate:


    cmp  ah,101 	  ; minimize to left
    je	 left_button

    cmp  ah,102 	  ; minimize to right
    je	 right_button

; // Alver 26.08.2007 // {
    cmp  ah, 103
    je	 clean_desktop		    ; minimize all windows
    cmp  ah, 104
    je	 restore_desktop	    ; restore minimized windows
    cmp  ah, 105
    je	 swap_minimized_desktop     ; minimize normal windows, and restore minimized windows
; } \\ Alver \\

    cmp  ah,byte 1	  ; start/terminate menu
    jnz  noselect
    call menu_handler
;    cmp  [music_type],0
;    je   still
    call menu_music
    jmp  still
  noselect:

	mov	ebx, exec_fileinfo
    cmp  ah,byte 2	       ; start calendar
    jnz  noid15  ;noclock
	mov	dword [ebx+21], calendar_name
	mov	eax, 70
	int	0x40
    call calendar_music
    jmp  still

  noid15:
    cmp  ah,16
    jne  noid16
	mov	ebx, setup_exec
	mov	dword [ebx+8], chlang
	mov	eax, 70
	int	0x40
    call chlang_music
    mcall 5, 25
    jmp  still

  noid16:
;    cmp  ah,17
;    jne  noid17
;        mov     ebx, setup_exec
;        mov     dword [ebx+8], syslang
;        mov     eax, 70
;        int     0x40
;    call syslang_music
;    mcall 5, 25
;    jmp  still
;
;  noid17:
    cmp  ah,18
    jne  noid18
	mov	dword [ebx+21], sysmeter_name
	mov	eax, 70
	int	0x40
    call sysmeter_music
    jmp  still

  noid18:
    cmp  ah,19
    jne  noid19
;    inc  [music_type]
;    and  [music_type],1
    mcall 18,8,2
;    mcall 18,8
;    mov [sound_flag],al

;    mcall 15,4,2
    mcall 15,3
    jmp  red

  noid19:
    cmp  ah,20		   ; start system setup
    jnz  noid20
	mov	ebx, setup_exec
	and	dword [ebx+8], 0
	mov	eax, 70
	int	0x40
    call setup_music
    jmp still

 noid20:
    cmp  ah,21
    jnz  noid21
; // Alver 25.08.2007 // {
    cmp  [page_list], 99
    jnc  @f
  ;  cmp  [page_list],15
  ;  je    @f
; } \\ Alver \\
    inc  [page_list]
    jmp  red
  @@:
    jmp still

 noid21:
    cmp  ah,22
    jnz  noid22
    cmp  [page_list],0
    je	   @f
    dec  [page_list]
    jmp  red
  @@:
    jmp  still

 noid22:

    jmp  still

; // Alver 26.08.2007 //{
restore_desktop:
    mcall   9,process_info_buffer,-1
    mov     [max_slot], eax
    mov     ecx, 2
    mov     edx, 2
@@:
    mcall   18, 22
    inc     edx
    cmp     edx, [max_slot]
    jbe     @b
    jmp  still

swap_minimized_desktop:
    xor  esi, esi
    inc esi
    jmp  min_swap_desktop
clean_desktop:
    xor esi,esi
 min_swap_desktop:		   ; input esi 0 - min, <>0 swap
     mov    ecx, 2		   ; Начальный номер слота процесса
     mov    ebx, process_info_buffer

.loop1:
     cmp    ecx, [this_slot]
     je     .loop1xx
     mcall  9
     mov    [max_slot], eax
     mov    dx, word [process_info_buffer+50]
     cmp    dx, 9		    ; Проверка свободен ли данный слот
     jz     .loop1xx
     mov    edx, dword [ebx+10]
     cmp    dl, '@'
     je     .loop1xx
     cmp    dword [ebx+10], 'ICON'
     jnz    @f
     cmp    [ebx+42], dword 51
     jnz    @f
     cmp    [ebx+46], dword 51
     jz     .loop1xx
@@:
     cmp    [ebx+10], dword '    '
     jz     .loop1xx

     pushad
     mov    edx, ecx
     xor    ecx, ecx
     or     esi, esi
     jz     @f
     movzx  eax, byte [process_info_buffer+70]
     and    eax, 2	      ; mask minimize
     jz     @f
     mov    ecx, 2	      ; restore
@@:
     mcall  18, 22
     popad
.loop1xx:
     inc    ecx
     cmp    ecx, [max_slot]
     jbe    .loop1
     jmp    still
; } \\ Alver \\

draw_running_applications:

    pusha

    cmp  [icons],1
    jne  dr_ret

    call calculate_applications

    cmp  edi,[running_applications]
    jne  noret
    popa
    ret
  noret:

;    cmp  edi,[running_applications]
;    jge  no_application_decrease
    call draw_window
;  no_application_decrease:

    mov  [running_applications],edi

	call	redraw_window_tabs

  dr_ret:

    popa

    ret

need_window_tab:
; in: ebx->process info
; out: ZF set <=> do not draw
	cmp	byte [ebx+10], '@'
	jz	.nodraw
; \begin{diamond}[29.03.2007]
; do not draw undefined (zero-sized) windows
	cmp	dword [ebx+42], 0
	jnz	@f
	cmp	dword [ebx+46], 0
	jz	.nodraw
@@:
; \end{diamond}[29.03.2007]
; <begin> (pterox)[12.10.2010] 
; do not draw OpenDialog windows
	push	EDI
	mov	EDI, dword [EBX+10]
	or	EDI, 0x20202020
	cmp	EDI, 'open'
	jne	@f
	mov	EDI, dword [EBX+14]
	or	EDI, 0x20202020
	cmp	dword [EBX+14], 'dial'
	jne	@f
	pop	EDI
	jmp	.nodraw

@@:
	pop	EDI
; <end> (pterox)[12.10.2010] 
	cmp	dword [ebx+10], 'ICON'
	jnz	@f
	cmp	[ebx+42], dword 51
	jnz	@f
	cmp	[ebx+46], dword 51
	jz	.nodraw
@@:
	cmp	[ebx+10], dword '    '
.nodraw:
	ret

redraw_window_tabs:
	xor	edi, edi
	mov	[contrast], 0

	push	ebp			; barsuk {
	mov	eax, 18
	mov	ebx, 7
	int	0x40
	mov	ebp, eax		; barsuk }

.loop:
	mov	ecx, [app_list+edi*4]
	cmp	ecx, -1
	jz	.done

	push	ecx
	mov	eax, 9
	mov	ebx, 0x8000
	int	0x40

	mov	eax, 13
	imul	ebx, edi, 6*10*10000h
; // Alver 26.08.2007 // {
;       add     ebx, 6*10*10000h + 7*10000h + 54
	add	ebx, 12*10*10000h + 7*10000h + 54
; } // Alver //
	mov	ecx, 3*10000h + 14

;       xor     edx, edx
	mov	edx, 0x383838		; barsuk
	int	0x40
	sub	ebx, 10000h + 53
	mov	ecx, 4*10000h + 12
	int	0x40
	sub	ebx, 10000h
	mov	ecx, 5*10000h + 10
	int	0x40
	add	ebx, 56*10000h
	mov	ecx, 4*10000h + 12
	int	0x40
	add	ebx, 10000h
	mov	ecx, 5*10000h + 10
	int	0x40

	mov	edx, [wcolor]	; barsuk
	pop	ecx
	cmp	ebp, ecx	; ecx=active slot number
	jnz	@f
	mov	edx, [system_colours.work_button]
@@:
	mov	esi, [current_alt_tab_app]
	cmp	esi, -1
	jz	@f
	cmp	ecx, [alt_tab_list+esi*8]
	jnz	@f
;	xor	edx, 0xFFFFFF
	mov	edx, 0xFF8000
@@:
	sub	ebx, 55*10000h - 53
	mov	ecx, 4*10000h + 12
	int	0x40
	sub	ebx, 10000h + 53
	mov	ecx, 5*10000h + 10
	int	0x40
	add	ebx, 55*10000h
	int	0x40

	mov	eax, 4
	sub	ebx, 51*10000h - 6
	mov	ecx, [system_colours.work_button_text]	;0xffffff
	mov	edx, 0x8000+10
	mov	esi, 11
	int	0x40

.nodraw:
	inc	edi
	cmp	edi, [max_applications]
	jb	.loop
.done:
	pop	ebp
	ret

calculate_applications:

    mov  eax,[max_applications]
    mul  [page_list]
    test eax,eax
    je	  @f
    inc  eax
  @@:
    mov  [draw_start_position],eax

    mov  edi,app_list
    mov  ecx,20
    mov  eax,-1
    cld
    rep  stosd

    mov  edi,0
    mov  ecx,2

  cnewpr:

    mov  eax,9
    mov  ebx,0x8000
    int  0x40

	call	need_window_tab
	jz	cnorpl
	sub	[draw_start_position], 1
	jg	cnorpl

    mov  [app_list+edi*4],ecx

    inc  edi

  cnorpl:
    inc  ecx

    cmp  eax,ecx
    jge  cnewpr

    mov  [app_tab_count], edi	; barsuk

    ret


draw_application_buttons:

    pusha

    cmp [icons],1
    jne da_ret

    mov  eax,14
    int  0x40

    shr  eax,16

; // Alver 25.08.2007 // {
 ;   cmp  eax,639
 ;   jne  now1
 ;   mov  [max_applications],7  ;6
 ; now1:
 ;   cmp  eax,799
 ;   jne  now2
 ;   mov  [max_applications],9 ;10    ;8
 ; now2:
 ;   cmp  eax,1023
 ;   jne  now3
 ;   mov  [max_applications],12 ;13   ;8
 ; now3:
 ;   cmp  eax,1279
 ;   jne  now4
 ;   mov  [max_applications],17 ;18    ;8
 ; now4:
 ;   mov  edi,1

     sub  eax, 281
     jnc  ._01
     xor  eax, eax
._01:
     mov  bl, 60
     div  bl
     xor  ah, ah		       ; eax - количество умещаюшихся пиктов
     mov  [max_applications], eax
     mov  edi,1
;  } \\ Alver \\

  nb:

    mov  eax,8
    mov  ebx,edi
    shl  ebx,16
    imul ebx,6*10	     ;13
    add  ebx,75*65536+10*6-1 ; << Alver 25.08.2007 >> 15*65536+10*6-1  ;13
    mov  ecx,1*65536+17
    mov  edx,edi
    add  edx,51
    cmp  [buttons],1
    je	 bufr
    or	 edx,0x60000000
  bufr:
    mov  esi,[wcolor]
    sub  ebx,11 shl 16
    int  0x40

    inc  edi
    cmp  edi,[max_applications]
    jbe  nb

  da_ret:

    popa

    ret


menu_handler:
	mov	eax, 70
	mov	ebx, exec_fileinfo
	mov	dword [ebx+21], menu_name
	int	0x40
	ret

draw_small_right:

    pusha

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  eax,0
    mov  edx,[wcolor]
    mov  esi,edx
    mov  edi,edx
    or	 edx, 0x01000000
    int  0x40

    mov  eax,8
    mov  ebx,0*65536+9
    mov  ecx,0*65536
    mov  cx,[b_size_y]
    mov  edx,1
    mov  esi,[wcolor]
    int  0x40

    mov  eax,4
    mov  ebx,2*65536+16
    cmp  [graph_text],1
    jne  nos3
    mov  ebx,2*65536+7
  nos3:
    mov  ecx,[wcolor]
    add  ecx,0x303030
    mov  edx,hidetext
    mov  esi,1
    int  0x40

    mov  eax,12
    mov  ebx,2
    int  0x40

    popa

    ret



draw_small_left:

    pusha

    mov  eax,12
    mov  ebx,1
    int  0x40

    mov  eax,0
    mov  edx,[wcolor]
    mov  esi,edx
    mov  edi,edx
    or	 edx, 0x01000000
    int  0x40

    cmp  [graph_text],1
    je	 nos4

    mov  eax,8
    mov  ebx,0*65536+9
    mov  ecx,0*65536+18-6
    mov  edx,2
    mov  esi,[wcolor]
    int  0x40

    mov  eax,4
    mov  ebx,2*65536+4
    mov  ecx,[wcolor]
    add  ecx,0x303030
    mov  edx,hidetext+2
    mov  esi,1
    int  0x40

  nos4:

    mov  eax,8
    mov  ebx,0*65536+9
    mov  ecx,13*65536+25
    cmp  [graph_text],1
    jne  nos6
    mov  ecx,0*65536
    mov  cx,word [b_size_y]
  nos6:
    mov  edx,1
    mov  esi,[wcolor]
    int  0x40

    mov  eax,4
    mov  ebx,3*65536+22
    cmp  [graph_text],1
    jne  nos7
    mov  ebx,3*65536+7
  nos7:
    mov  ecx,[wcolor]
    add  ecx,0x303030
    mov  edx,hidetext+1
    mov  esi,1
    int  0x40

    mov  eax,12
    mov  ebx,2
    int  0x40

    popa
    ret


;-------------------------------------------------

right_button:

    call button_music

    mov  [small_draw],dword draw_small_right

    mcall 14
    shr eax, 16
    mov ebx, eax
    mov ecx, -1
    mov edx, 9
    sub ebx, edx
    mov esi, -1
    mcall 67

    call draw_small_right

    jmp  small_wait

;-------------------------------------------------

left_button:

    call  button_music

    mov  [small_draw],dword draw_small_left

    mov   ebx, 0
    mov   edx, 9
    mov   ecx, -1
    mov   esi, -1
    mcall 67

    call draw_small_left

;-------------------------------------------------

  small_wait:

    mov  eax, 10
    int  0x40

    cmp  eax,1
    jne  no_win
    call [small_draw]
    jmp  small_wait
  no_win:
    cmp  eax,2
    jne  no_key
    call handle_key
    jmp  small_wait
no_key:

    mov  eax,17
    int  0x40

    cmp  ah,1
    jne  no_full

    mov   eax, 14		    ; get screen max x & max y
    int   0x40
    mov   edx, eax
    shr   edx, 16
    xor   ebx, ebx
    mov   ecx, -1
    mov   esi, -1
    mcall 67 ; x0 y0 xs ys

    call  button_music

    jmp   still


  no_full:

    call menu_handler

    jmp  small_wait



set_variables:

     pusha

     mov  [b_size_y],dword 38
     cmp  [graph_text],1
     jne  noy2
     mov  [b_size_y],dword 18
   noy2:

     mov  [button_frames],0x0
     cmp  [buttons],0
     jne  no_frames
     mov  [button_frames],0x40000000
   no_frames:


     mov  eax,48	   ; 3d button look
     mov  ebx,1
     mov  ecx,1
     int  0x40

     mov  eax,0x40404040   ; dividers for processes
     mov  edi,pros
     mov  ecx,10
     cld
     rep  stosd

     popa
     ret



; eax = number (1 or 2)
; ebx = language id
draw_flag:
    pusha

;    cmp  [graph_text],0
;    je   mini_flag

; eax = 2 BIG
; eax = 1 small

    mov  edx,ebx

    mov  ebx,[maxx]
    and  eax,1
    imul eax,17  ;17
    sub  ebx,eax
    sub  ebx,76 ;79 ;28

    pushad
;    dec  ebx
    sub  ebx,2
    shl  ebx, 16
    add  ebx, 15 ;25
    mov  ecx, 4*65536+13
    mov  edx,0
    mov  eax,13
    int  0x40
    add  ebx,1 shl 16
    sub  ebx,2
    mov  ecx, 5 shl 16+11
    cmp  [type_lang],1
    je	label_1
    mov  edx,0xff ;[wcolor]
    jmp  label_2
label_1:
    mov  edx,0x7700
label_2:
    mov  eax, 13
    int  0x40
    popad

    shl  ebx,16
    add  ebx,7	;24

    mov  ecx,[bte] ; color

    dec  edx
    shl  edx,1
    add  edx,flag_text
    mov  esi,2
    mov  eax,4
    int  0x40

    mov  ebx,[maxx]
    sub  ebx,48
    shl  ebx,16
    mov  bx,34
    mov  ecx,3 shl 16+14
    xor  edx,edx
    mov  eax,13
    int  0x40
    add  ebx,1 shl 16
    sub  ebx,2
    mov  ecx,4 shl 16+12
    mov  edx,time_bgr_color
    int  0x40

    popa
    ret

;mini_flag:
;    popa
;    ret




; ***************************************************
; ********* WINDOW DEFINITIONS AND DRAW *************
; ***************************************************


draw_window:

    pusha

    mov  [running_applications],-1
    mov  [checks],-1

    mov  eax, 12		   ; tell os about redraw
    mov  ebx, 1
    int  0x40

    mov  eax, 48
    mov  ebx, 3
    mov  ecx, system_colours
    mov  edx, 10*4
    int  0x40

    mov  eax, [system_colours+4*6]
    sub  eax, 0x101010
    mov  [wcolor], eax

    mov  eax,14 		   ; get screen max x & max y
    int  0x40

    cmp  [width],0
    je	 no_def_width
    and  eax,0xffff
    mov  ebx,[width]
    shl  ebx,16
    add  eax,ebx
  no_def_width:

    mov  ebx,eax
    mov  [screenxy],ebx
    shr  ebx,16
    sub  ax,38
    shl  eax,16
    mov  ecx,eax
    add  ecx,0*65536+38
    cmp  [graph_text],1
    jne  no_text_1
    mov  cx,PANEL_HEIGHT
    add  ecx,20*65536
  no_text_1:
    mov  eax, 0 		    ; DEFINE AND DRAW WINDOW
    mov  edx, [wcolor]
    or	 edx, 0x01000000 ; do not draw the window
    mov  esi, [wcolor]
    or	 esi, 0x01000000 ; unmovable window
    mov  edi, [wcolor]

    mov  [panel_x_pos], ebx
    mov  [panel_y_pos], ecx	 ; Пока что так.

    int  0x40

    movzx ebx,word [screenxy+2]
    mov  ecx,0*65536+0
    mov  edx,[wcolor]
    add  edx,0x161616
  newline_:			; debug.inc has macros with the same name
    sub  edx,0x040404
    mov  eax,38
    cmp  [soften_up],1
    jne  no_su
    and  edx,0x00FFFFFF
    int  0x40
  no_su:

    pusha
    cmp  [soften_down],1
    jne  no_sd
    sub  edx,0x141414
    mov  edi,[b_size_y]
    shl  edi,16
    add  edi,[b_size_y]
    add  ecx,edi
    sub  ecx,3*65536+3
    and  edx,0x00FFFFFF
    int  0x40
  no_sd:
    popa

    add  ecx,1*65536+1
    cmp  cx,5
    jb	 newline_

    cmp   [soften_middle],1
    jne   no_sm

    movzx ebx,word [screenxy+2]
    mov   ecx,5*65536+5
    mov   esi,stripe
    mov   edx,[wcolor]
  newline3:
    add  edx,[esi]
    add  esi,4

    mov  eax,38
    and  edx,0x00FFFFFF
    int  0x40
    add  ecx,1*65536+1
    cmp  cx,15
    jb	 newline3

  no_sm:

    cmp  [minimize_left],1
    jne  no_mleft
    mov  eax,8				     ; ABS LEFT
    mov  ebx,0 *65536+9
    mov  ecx,1 *65536
    add  ecx,[b_size_y]
    dec  ecx
    mov  edx,101
    add  edx,[button_frames]
    mov  esi,[wcolor]
    int  0x40
    mov  eax,4				     ; HIDE TEXT
    mov  ebx,2*65536+17
    cmp  [graph_text],1
    jne  no_y1
    mov  bx,7
  no_y1:
    mov  ecx,[wcolor]
    add  ecx,0x303030
    mov  edx,hidetext
    mov  esi,1
    int  0x40
  no_mleft:

    movzx eax,word [screenxy+2]
    mov  [maxx],eax

    cmp  [minimize_right],1
    jne  no_mright
    mov  eax,[maxx]
    sub  eax,77
    shl  eax,16
    mov  ebx,eax
    add  ebx,67
    mov  eax,8				     ; ABS RIGHT
    mov  ecx,1 *65536
    add  ecx,[b_size_y]
    dec  ecx
    add  ebx,68*65536
    mov  bx,9
    mov  edx,102
    add  edx,[button_frames]
    mov  esi,[wcolor]
    int  0x40
    mov  edx,hidetext+1
    mov  eax,4
    mov  ebx,[maxx]
    sub  ebx,6
    shl  ebx,16
    mov  bx,17
    cmp  [graph_text],1
    jne  no_y2
    mov  bx,7
  no_y2:
    mov  ecx,[wcolor]
    add  ecx,0x303030
    mov  esi,1
    int  0x40
  no_mright:

    call draw_menuet_icon

    call draw_program_icons

    mov  [ptime],0
    call draw_info

    call draw_application_buttons

;     mov    ecx,[button_presssed_alt]
;     mcall  47,0x80100,ecx ,400 shl 16+5,0

    mov  eax,12
    mov  ebx,2
    int  0x40

    popa
    ret

; // Alver 26.08.2007 // {
dr_button_clean:
    mov  ecx, 3*65536+13
    mcall 8
    ret
; } \\ Alver \\

draw_menuet_icon:

    pusha

; // Alver 26.08.2007 // {
; Inserted code for drowing buttons 103, 104, 105 (Clean, Restore, Exchange windows)
    mov  esi, dword [system_colours+24]    ; drawing buttons
    and  esi, 0x00ffffff
    mov  edx, 103
    mov  ebx, 67*65536+13
    call dr_button_clean
    inc  edx
    mov  ebx, 86*65536+13
    call dr_button_clean
    inc  edx
    mov  ebx, 105*65536+13
    call dr_button_clean
    mov  ecx, dword [system_colours+28]
    and  ecx, 0x00ffffff
    xor  esi, esi
    inc  esi
    mov  edx, page_clean_but
    mov  eax, 4
    mov  ebx, 71*65536+6
    int  0x40
    inc  edx
    mov  ebx, 90*65536+6
    int  0x40
    inc  edx
    mov  ebx, 109*65536+6
    int  0x40
; } \\ Alver \\

    cmp  [menu_enable],1
    jne  no_menu


    mov  eax, 8 			      ; M BUTTON
    mov  ebx, 10*65536 + 47
    cmp  [minimize_left], 0
    jne  @f
    sub  ebx, 10*65536
  @@:
    mov  ecx, 1*65536
    add  ecx, [b_size_y]
    dec  ecx
    mov  edx, 0x20000001
    add  edx, [button_frames]
    mov  esi, [wcolor]
    int  0x40

    cmp  [graph_text], 1
    jne  no_mtext

    push ebx
    mov  eax,13
    mov  ebx,12 shl 16+44  ;51
    mov  ecx,1 shl 16+17
;    xor  edx,edx
    mov edx, 0x383838
    int  0x40
;    mov  ebx,63 shl 16+1
    mov  ebx,56 shl 16+1
    mov  ecx,2 shl 16+15
    int  0x40
    mov  ebx,57 shl 16+1
    mov  ecx,4 shl 16+11
    int  0x40
    mov  ebx,58 shl 16+1
    mov  ecx,6	shl 16+7
    int  0x40
;    mov  ebx,66  shl 16+1
;    mov  ecx,9 shl 16+1
;    int  0x40
    mov  ebx,13 shl 16+43 ;50
    mov  ecx,2 shl 16+15
    mov  edx,0x7700
    int  0x40
;    mov  ebx,62 shl 16+1
;    mov  ecx,3 shl 16+14
;    int  0x40
    mov  ebx,56 shl 16+1
    mov  ecx,4 shl 16+11
    int  0x40
    mov  ebx,57 shl 16+1
    mov  ecx,6 shl 16+7
    int  0x40
    pop  ebx

    mov  eax, 4
    mov  bx,  7
    add  ebx, 8*65536
    mov  ecx, 0x10ffffff
    mov  edx, m_text
    mov  esi, 4
    int  0x40

    popa
    ret

  no_mtext:



    mov  eax,[wcolor]
    mov  [m_icon+4],eax

; load & display menuet.bmp
	mov	eax, 70
	mov	ebx, m_bmp_fileinfo
	int	0x40

    mov  eax,40
    mov  ebx,0
    mov  edi,image+53

   new_m_pix:

;    movzx ecx,byte [edi]
;    shr  ecx,5

    cmp    byte [edi], 10
    jb	   nopix
    cmp    byte [edi+1], 10
    jb	   nopix
    cmp    byte [edi+2], 10
    jb	   nopix

    pusha
    cmp  [minimize_left],0
    jne  no_m_s2
    sub  ebx,10
  no_m_s2:
;    mov  edx,[ecx*4+m_icon]
    mov  edx,[edi+1]

    mov  ecx,eax
    mov  eax,1
    add  ebx,12
    int  0x40
    popa

   nopix:

    add  edi,3
    add  ebx,1
    cmp  ebx,40
    jnz  new_m_pix

    mov  ebx,0
    dec  eax
    jnz  new_m_pix

  no_menu:

    popa
    ret


draw_program_icons:

    pusha

    cmp  [icons],0
    jne  dp_ret

    mov  edi,1
    push edi

  new_icon_file:

    pusha
    mov  edx,[esp+32]
    add  edx,10
    push edx
    mov  esi,[wcolor]
    mov  ecx,1*65536
    add  ecx,[b_size_y]
    dec  ecx
    mov  eax,edi
    dec  eax
    imul eax,40
    mov  ebx,eax
    add  ebx,[icons_position]
    shl  ebx,16
    mov  bx,39
    pop  edx
    add  edx,[button_frames]
    or	 edx, 0x20000000
    mov  eax,8
    int  0x40
    popa

    mov  ecx,[esp]
    add  ecx,48
    mov  [iconf+6],cl

	mov	eax, 70
	mov	ebx, iconf_fileinfo
	int	0x40

    mov  eax,0
    mov  ebx,32
    mov  edi,image+51+32*33*3

   np2: 			    ; new pixel of file

    mov  edx,[edi]
    and  edx,0xffffff

    cmp  eax,3			    ; Y draw limits
    jb	 nopix2
    cmp  eax,36
    jg	 nopix2
    cmp  ebx,38 		    ; X draw limits
    jg	 nopix2
    cmp  ebx,2
    jb	 nopix2

    cmp  edx,0
    jz	 nopix2

    cmp  [graph_text],1
    jne  no_icon_text

    pusha

    mov  ebx,[esp+32]
    dec  ebx
    imul ebx,40
    add  ebx,8
    add  ebx,[icons_position]
    shl  ebx,16
    mov  bx,7

    mov  eax,4
    mov  ecx,0xffffff
    mov  edx,[esp+32]
    dec  edx
    imul edx,4
    add  edx,mi_text
    mov  esi,4
    int  0x40

    popa

    jmp  nopix2

  no_icon_text:

    mov  esi,[esp]
    pusha
    push edx
    mov  ecx,eax
    add  ecx,2
    mov  eax,esi
    dec  eax
    imul eax,40
    add  ebx,eax
    add  ebx,3
    add  ebx,[icons_position]
    pop  edx
    mov  eax,1
    int  0x40
    popa

  nopix2:

    sub  edi,3
    dec  ebx
    jnz  np2

    mov  ebx,32
    add  eax,1
    cmp  eax,32
    jnz  np2

    add  dword [esp],1
    mov  edi,[esp]
    cmp  dword [esp],4
    jbe  new_icon_file
    add  esp,4

    mov  eax,4
    mov  ebx,40
    imul ebx,3
    add  ebx,[icons_position]
    add  ebx,10
    shl  ebx,16
    mov  bx,23
    mov  ecx,[wcolor]
    mov  edx,gpl
    mov  esi,3
    int  0x40

  dp_ret:

    popa
    ret



draw_info:    ; draw cpu usage, time, date

    pusha

    cmp  [setup_enable],1
    jne  no_setup

    cmp  [minimize_right],0
    jne  no_m_r
    add  [maxx],10

   no_m_r:

    mov  eax,3
    int  0x40
    cmp  eax,[ptime]
    jz	 _ret
    mov  [ptime],eax

    call draw_cpu_usage

    mov  eax,[maxx]   ; blink sec
    sub  eax,33
    shl  eax,16
    mov  ebx,eax
    add  ebx,9
    mov  eax,3
    int  0x40
    cmp  [graph_text],1
    jne  no_y4
    sub  bx,2
  no_y4:
    mov  ecx,eax
    shr  ecx,16
    and  ecx,1
    mov  edx,[bte]
    sub  edx,time_bgr_color;[wcolor]
    imul ecx,edx
    add  ecx,time_bgr_color;[wcolor]
    mov  edx,sec
    mov  eax,4
    mov  esi,1
    int  0x40


;    mov  eax,26          ; check for change in time or country
;    mov  ebx,5
;    int  0x40
;    mov  edx,eax
    mov  eax,26
    mov  ebx,2
    mov  ecx,9
    int  0x40
;    add  edx,eax
    mov  edx,eax
    mov  eax,3
    int  0x40
    and  eax,0xffff
    add  edx,eax
    cmp  edx,[checks]
    je	 _ret
    mov  [checks],edx

    mov  ebx,[maxx]
    sub  ebx,48 ;;94 ;;74
    shl  ebx,16
    add  ebx,33 ;;84 ;;64

    mov  eax,8		     ; time/date button
    mov  ecx,3 *65536
    add  ecx,[b_size_y]
;    dec  ecx
    sub  cx,5
    mov  edx,2+0x20000000
    mov  esi,[wcolor]
    int  0x40
    pusha
    mov  eax,13
    add  ebx,10*65536-16
    add  ecx,5*65536-8
    mov  edx,[wcolor]
    int  0x40
    popa
    and  edx,0xffff
    add  edx,[button_frames]
    int  0x40

    mov  eax,8
    mov  ebx,[maxx]
    sub  ebx,77 ;80
    shl  ebx,16
    add  ebx,12
    mov  ecx,5 shl 16+10
    mov  edx,16+0x20000000  ;button 16
    mov  esi,[wcolor]
    int  0x40
    sub  ebx,17 shl 16
    inc  edx		    ;button 17
;    int  0x40
    add  ebx,33 shl 16
    mov  bx,8
    inc  edx		    ;button 18
    int  0x40
    sub  ebx,30 shl 16
    mov  bx,10
    inc  edx		    ;button 19
    int  0x40
    sub  ebx,14 shl 16
    inc  edx		    ;button 20
    int  0x40
    sub  ebx,12 shl 16
    mov  bx,8
    mov  ecx,6 shl 16+10
    inc  edx	    ;button 21
    int  0x40
    sub  ebx,24 shl 16	; << Alver 25.08.2007 >> 18 shl 16
    inc  edx	    ;button 22
    int  0x40

    ; flags

;    mov  eax,26
;    mov  ebx,5
;    int  0x40
;    mov  ebx,eax
;
;    mov  eax,1
;    mov  [type_lang],al
;    call draw_flag

    mov  eax,26
    mov  ebx,2
    mov  ecx,9
    int  0x40
    mov  ebx,eax

    mov  eax,2
    mov  [type_lang],al
    call draw_flag

    mcall 18,8,1
    mov  [sound_flag],al

    mov  ebx,[maxx]
    sub  ebx,92 ;109 ;112 ;28
    shl  ebx,16
    mov  bx,12
    mov  ecx, 4*65536+13
    mov  edx,0
    mov  eax,13
    int  0x40
    add  ebx,1 shl 16
    sub  bx,2
    mov  ecx,5 shl 16+11
    mov  edx,0xcc
    int  0x40
    add  ebx,1 shl 16
    mov  bx,5
    mov  ecx,8 shl 16+5
    mov  edx,0xdddd00
    int  0x40
    add  ebx,5 shl 16
    mov  bx,1
    mov  ecx,7 shl 16+7
    int  0x40
    add  ebx,1 shl 16
    mov  ecx,6 shl 16+9
    int  0x40
    add  ebx,1 shl 16
    mov  ecx,5 shl 16+11
    int  0x40

;    cmp  [music_type],0
;    jne   dalshe
    cmp  [sound_flag],0
    je	 dalshe

    sub  ebx,8 shl 16
    ror  ebx,16
    mov  cx,bx
    rol  ebx,16
    mov  bx,cx
    add  bx,8
    mov  ecx,5 shl 16+15
    mov  edx,0xff0000
    mov  eax,38
    int  0x40
    add  ebx,1 shl 16
    inc  bx
    int  0x40
    rol  ecx,16
    int  0x40
    sub  ebx,1 shl 16
    dec  bx
    int  0x40

dalshe:

    mov  ebx,[maxx]
    sub  ebx,106;123
    shl  ebx,16
    mov  bx,12
    mov  ecx, 4*65536+13
    mov  edx,0
    mov  eax,13
    int  0x40
    add  ebx,1 shl 16
    sub  bx,2
    mov  ecx,5 shl 16+11
    mov  edx,0xffcc00
    int  0x40
    mov  eax,4
    mov  ebx,[maxx]
    sub  ebx,104;121
    shl  ebx,16
    mov  bx,7
    mov  ecx,0x10000000
    mov  edx,file_sys
    mov  esi,1
    int  0x40
    add  ebx,1 shl 16
    int  0x40

    mov  edx,0
    mov  eax,13
    mov  ebx,[maxx]
    sub  ebx,117;134
    shl  ebx,16
    mov  bx,9
    mov  ecx,6 shl 16+11
    int  0x40
; // Alver 25.08.2007 // {
  ;  sub  ebx,18 shl 16
  ;  int  0x40
  ;  add  ebx,19 shl 16
    sub  ebx,24 shl 16 ; 18 shl 16
    int  0x40
    add  ebx,25 shl 16 ; 19 shl 16
; } \\ Alver \\
    sub  bx,2
    mov  ecx,7 shl 16+9
    mov  edx,0xffffff
    int  0x40
; // Alver 25.08.2007 // {
 ;   sub  ebx,18 shl 16
    sub  ebx,24 shl 16
; } \\ Alver \\
    int  0x40

    mov  eax,4
    mov  edx,page_a1
    mov  ebx,[maxx]
; // Alver 25.08.2007 // {
   ; sub  ebx,133;150
    sub  ebx, 139
; } \\ Alver \\
    shl  ebx,16
    mov  bx,8
    mov  esi,5	   ;4
    int  0x40
    add  ebx,1 shl 16
    int  0x40

    mov  eax,47
    mov  ebx,0x20000  ; 0x10100 (<Alver)
    mov  ecx,[page_list]
    mov  edx,[maxx]
    sub  edx,130 ; 124 (<Alver);141
    shl  edx,16
    mov  dx,7
    mov  esi,0xffffff
    int  0x40

;    sub  ebx,14 shl 16
;    mov  bx,7
;    mov  edx,turn_text
;    mov  esi,1

;    mov  ecx,0x60a060 ;[wcolor]
;    add  ecx,0x303030
;    mov  eax,4
;    int  0x40
;    add  ebx,1 shl 16
;    int  0x40
;    add  ebx,1 shl 16
;    int  0x40
;    add  ebx,1 shl 16
;    int  0x40

;    add  ebx,1 shl 16
;    mov  ecx,0x60a060 ;[wcolor]
;    int  0x40
;    add  ebx,1 shl 16
;    int  0x40
;    add  ebx,1 shl 16
;    sub  ecx,0x303030
;    int  0x40

;    sub  ebx,6 shl 16
;    mov  bx,1
;    mov  ecx,2 shl 16+15
;    mov  edx,0x60a060  ;[wcolor]
;    add  edx,0x303030
;    mov  eax,13
;    int  0x40
;    add  ebx,1 shl 16
;    mov  bx,1
;    mov  edx,0x60a060  ;[wcolor]
;    int  0x40
;    add  ebx,1 shl 16
;    mov  bx,1
;    sub  edx,0x303030
;    int  0x40
;    add  ebx,1 shl 16
;    mov  edx,[wcolor]
;    int  0x40

    mov  eax,3			; get time
    int  0x40

    movzx ebx,al
    shr   eax,8
    movzx ecx,al
    shr   eax,8
    movzx edx,al

    ; ebx ecx edx h m s

    push ebx
    push ecx

    mov  eax,[maxx]
    sub  eax,32
    shl  eax,16
    mov  ebx,eax
    add  ebx,9

    mov  ecx,[bte]

    cmp  [graph_text],1
    jne  no_y3
    sub  bx,2
    mov  ecx,0xffffff
  no_y3:


    mov  edx,[esp]	       ; __:_X
    and  edx,15
    mov  eax,4
    add  ebx,10*65536
    add  edx,text
    mov  esi,1
    int  0x40

    pop  edx			; __:X_
    shr  edx,4
    and  edx,15
    mov  eax,4
    sub  ebx,6*65536
    add  edx,text
    mov  esi,1
    int  0x40

    mov  edx,[esp]	       ; _X:__
    and  edx,15
    mov  eax,4
    sub  ebx,11*65536
    add  edx,text
    mov  esi,1
    int  0x40

    pop  edx			; X_:__
    shr  edx,4
    and  edx,15
    mov  eax,4
    sub  ebx,6*65536
    add  edx,text
    mov  esi,1
    int  0x40

    call draw_cpu_usage

  _ret:

    cmp  [minimize_right],0
    jne  no_m_r2
    sub  [maxx],10
   no_m_r2:

   no_setup:

    popa
    ret



draw_cpu_usage:

    pushad

    mov  [ysi],30
    cmp  [graph_text],1
    jne  @f
    mov  [ysi],12
  @@:


    mov  eax,18    ; TSC / SEC
    mov  ebx,5
    int  0x40
    shr  eax,20
    push eax
    mov  eax,18    ; IDLE / SEC
    mov  ebx,4
    int  0x40
    shr  eax,20
    xor  edx,edx
    imul eax,[ysi]

    cdq
    pop  ebx
    inc  ebx
    div  ebx
    cmp  eax,[ysi]
    jng  no_bug
    mov  eax,[ysi]
no_bug:
    push eax

    mov  eax,13
    mov  ebx,[maxx]
    sub  ebx,60
    shl  ebx,16
    add  ebx,8
    mov  ecx,5 shl 16
    add  ecx,[ysi]
    mov  edx,0xdd2222
    int  0x40

    pop  eax
    mov  ecx,5 shl 16
    add  ecx,eax
    mov  eax,13
    mov  edx,0x44aa44
    int  0x40

    popad
    ret

;;;include 'detect.asm'
include 'ctx_menu.asm'

; DATA

stripe:
    dd	-0x010101
    dd	-0x010101
    dd	-0x020202
    dd	-0x010101
    dd	-0x000000

    dd	 0x000000
    dd	 0x010101
    dd	 0x020202
    dd	 0x010101
    dd	 0x010101

m_icon:
    dd	0x0
    dd	0x808080
    dd	0x000000
    dd	0x000000
    dd	0xffffff


lsz m_text,\
  ru, "МЕНЮ",\
  en, "MENU",\
  et, "MEN--"

mi_text   db   'WAVETETRBGRDGPL '

flag_text db 'EnFiGeRuFrEtUa'

type_lang db 0
;music_type db 1
sound_flag db 0
button_frames  dd  0x0

checks	  dd -1
hidetext  db 0x11,0x10,0x1e

turn_text db  '><'
gpl	  db  'GPL'

chlang db 'LANG',0
;syslang db 'SLAN',0

contrast  db 0

running_applications  dd  0x100
max_applications      dd  11

current_alt_tab_app	dd	-1

page_list  dd 0
draw_start_position dd 0
draw_window_1 db 0

b_size_y:  dd  0x0
ysi  dd  0
small_draw dd 0x0

ptime	dd 0x0
maxx	dd 0x0
text	db '0123456789'
; // Alver 25.08.2007 // {
;page_a1 db '<  >'
page_a1        db '<   >'
page_clean_but db 25,24,18
; } \\ Alver \\
bte	dd 0xccddee

wcolor	dd 0x506070

sec	db ': '
pros	db '                                                  '
	db '                                                  '

screenxy    dd	0x0
stcount     dd	0x0

setup_exec:
	dd	7
	dd	0
.cmdline dd	?
	dd	0
	dd	0
	db	'/sys/'
file_sys db	'SETUP',0

exec_fileinfo:
	dd	7
	dd	0
	dd	0
	dd	0
	dd	0
	db	0
.name	dd	?

end_name	db	'/sys/END',0
menu_name	db	'/sys/@MENU',0
calendar_name	db	'/sys/CALENDAR',0
sysmeter_name	db	'/sys/GMON',0
mousemul_name	db	'/sys/MOUSEMUL',0

dat_fileinfo:
	dd	0
	dd	0
	dd	0
	dd	1024
	dd	I_END
	db	'/sys/PANEL.DAT',0

m_bmp_fileinfo:
	dd	0
	dd	0
	dd	0
	dd	8192
	dd	image
	db	'/sys/MENUET.BMP',0

iconf_fileinfo:
	dd	0
	dd	0
	dd	0
	dd	8192
	dd	image
	db	'/sys/'
iconf	db	'MBAR_IX.BMP',0

I_END:

screen_size:
  .height dw ?
  .width  dw ?

this_slot      dd 0	      ; Slot of this process
max_slot       dd 255
system_colours	system_colors ; rd 10
app_list rd 50
alt_tab_list	rd	256*2
alt_tab_list_size dd	?
process_info_buffer  rb 1024
procinfo_for_detect  rb 1024
tictable:
  rd 256

	rb	256
detect_stack:
	rb	256
ctx_menu_stack:

panel_x_pos	rd	1
panel_y_pos	rd	1

last_active_window	dd	0
prev_active_window	dd	0
active_window_changed	dd	0
app_tab_count		dd	0

image: