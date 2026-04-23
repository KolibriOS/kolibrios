; ---------------------------------------------------------------------------- ;
; Tooltip widget
;
; Created by Siemargl, 2016
; 
; Changelist
; 161107 - initial version + test


; http://stackoverflow.com/questions/8976600/fasm-how-to-send-struct-to-proc
virtual at edi
	ttip tooltip ?, ?, ?, ?, ?, ?, ?, ?, ?
end virtual

; ---------------------------------------------------------------------------- ;
; инициализация всей цепочки тултипов, формирование размеров и буферов памяти
; return eax zero if fails
align 16
proc tooltip_init uses esi edi, ttip:dword
locals
    max_len dw ? ;maximum chars in line
    lines   dw ? ;lines in tooltip
endl

    mov edi, [ttip]
    jmp .tail_call
.list_next:
    ; init statics
    mov [ttip.mouse], 0
    mov [ttip.tm_strt], 0
    mov [ttip.video_y], 0
    mov [ttip.video_x], 0
    mov [ttip.video], 0
    stdcall get_font_size, [ttip.col_txt]
    mov [ttip.font_sz], eax

    ; count num of lines and max len
    mov esi, [ttip.txt]
    mov [lines], 1  ; lines
    mov [max_len], 0  ; max_len
.line:
	mov ecx, 0  ; len
.symb:
    mov al, [esi]
    cmp al, 0
    je .eos
    cmp al, 13
	jne .next
	inc [lines]
	inc esi
	cmp cx, [max_len]    ; detect max
	jle @f
	    mov [max_len], cx
	@@:
	jmp .line
.next:
    inc ecx
    inc esi
    jmp .symb
.eos:; string ended, we have correct width and hight
    cmp cx, [max_len]	 ; detect max
    jle @f
       mov [max_len], cx
    @@:

    mov eax, [ttip.font_sz]
    shr eax, 16
    imul [max_len]
    mov dx, ax	 ; width in pixels
    mov [ttip.video_w], ax
    mov eax, [ttip.font_sz] ; lo word == h
    and eax, $FFFF
    imul ax, [lines]
    mov [ttip.video_h], ax
    imul [ttip.video_w]
    imul eax, 3  ; eax have now width * height in pixels *3
    ; getmem
    invoke mem.alloc, eax
    mov [ttip.video], eax
    test eax, eax
    je .exitp	 ; malloc fails

    ; cycle list
    mov edi, [ttip.next]
.tail_call:
    test edi, edi
    jne .list_next
    xor eax, eax
    inc eax ; good return
.exitp:
	ret
endp

; ---------------------------------------------------------------------------- ;
; очистка памяти всей цепочки тултипов
align 4
proc tooltip_delete uses edi, ttip:dword
    mov edi, [ttip]
    jmp .tail_call
.list_next:
    mov eax, [ttip.video]
    test eax, eax
    je @f
	invoke mem.free, eax
    @@:
    mov edi, [ttip.next]
.tail_call:
    test edi, edi
    jne .list_next
    ret
endp

; ---------------------------------------------------------------------------- ;
; показ нужного из всей цепочки тултипов при бездействии (event 0)
align 4
proc tooltip_test_show uses edi ebx, ttip:dword
    mov edi, [ttip]
    jmp .tail_call
.list_next:
    cmp [ttip.tm_strt], 0   ; таймер 0, значит мы не в зоне
    je .nextp
    cmp [ttip.video_y], 0   ; тултип уже отображен
 ;;;   jne .redraw
    jne .nextp
    mcall SF_SYSTEM_GET, SSF_TIME_COUNT
    movzx ebx, [ttip.tm_wait]
    sub eax, ebx
    cmp eax, [ttip.tm_strt]
    jl .exitp ; мог быть только один
    ; время оттикало, сохраняем область и рисуемся
;CopyScreen(shadow_buf, 5*skinned+x+wForm.left, GetSkinHeight()*skinned+y+wForm.top, w, h);
    mcall SF_THREAD_INFO, proc_info, -1
    movzx edx, word [proc_info + 34] ; window x position
    add edx, 5
    shl edx, 16
    mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    movzx ebx, word [proc_info + 38] ; window y position
    add eax, ebx
    add edx, eax    ; x_y
    add edx, [ttip.mouse]
    add edx, 20 ; pixels below mouse
    mov ecx, dword [ttip.video_h] ; w_h
    mcall SF_GET_IMAGE, [ttip.video]

    mov eax, [ttip.mouse]  ; сохраним позицию где рисуемся
    add eax, 20 ; pixels below mouse
    mov [ttip.video_y], ax
    shr eax, 16
    mov [ttip.video_x], ax
 .redraw:    ; рисуемся относительно мыши!!!
    stdcall tooltip_draw, edi

    jmp .exitp	 ; мог быть только один
.nextp:
    mov edi, [ttip.next]
.tail_call:
    test edi, edi
    jne .list_next
.exitp:
    ret
endp

; рисуемся относительно мыши!!!
; internal func
proc tooltip_draw uses esi edi ebx, ttip:dword
locals
    line   dw ? ;line # drawing
    ptr_li dd ? ;line begin
endl
    mov edi, [ttip]
    ; draw square
    movzx ebx, [ttip.video_x]
    shl ebx, 16
    mov bx, [ttip.video_w]
    movzx ecx, [ttip.video_y]
    shl ecx, 16
    mov cx, [ttip.video_h]
    mcall SF_DRAW_RECT, , , [ttip.col_bkg]

    ; цикл по строкам
    ; count num of lines and max len
    mov esi, [ttip.txt]
    mov [line], 0  ; line #
.line:
    mov ecx, 0	; len
    mov [ptr_li], esi
.symb:
    mov al, [esi]
    cmp al, 0
    je @f
	cmp al, 13
	jne .next
    @@:
    ; draw line if len > 0
    test ecx, ecx
    je @f
       ; draw single line
	pushad
	mov esi, ecx ; length
	movzx ebx, [ttip.video_x]
	shl ebx, 16
	mov eax, [ttip.font_sz]
	and eax, $FFFF
	imul ax, [line]
	add ax, [ttip.video_y]
	add ebx, eax
	mov ecx, [ttip.col_txt]
	and ecx, $37FFFFFF
	mov edx, [ptr_li]
	mcall SF_DRAW_TEXT
	popad
    @@:
    cmp byte ptr esi, 0
    je .exitp
       inc [line]
       inc esi
       jmp .line
.next:
    inc ecx
    inc esi
    jmp .symb
.exitp:
    ret
endp

; ---------------------------------------------------------------------------- ;
; убрать тултипы при щелчке мыши или съезде из зоны. для всей цепочки тултипов при событии мыши
; если mouse_coord == -1 опросит мышь
align 4
proc tooltip_mouse, ttip:dword
locals
    mouse_coord dd ?
    mouse_but	dd ?
endl
    pushad
    ; опросим мышь и рассуем по тултипам
    mcall SF_MOUSE_GET, SSF_WINDOW_POSITION
    mov [mouse_coord], eax
    mcall SF_MOUSE_GET, SSF_BUTTON
    mov [mouse_but], eax
    mov edi, [ttip]
.list_next:
    test edi, edi
    je .exitp
    cmp [mouse_but], 0	; при щелчке стираемся
    jne @f
	; попадаем ли в зону контроля
	mov eax, [mouse_coord]
	mov [ttip.mouse], eax ; место где была замечена мышь! (хз зачем)
	mov ecx, eax
	shr ecx, 16
	cmp cx, [ttip.zone_x] ; zone_x is higher word
	jl @f
	cmp ax, [ttip.zone_y]
	jl @f
	mov cx, [ttip.zone_w]
	add cx, [ttip.zone_x]
	shl ecx, 16
	cmp eax, ecx	; x+w < mouse_x (mouse_y in low word ignored)
	jge @f
	mov cx, [ttip.zone_y]
	add cx, [ttip.zone_h]
	cmp ax, cx
	jge @f
	; мы в зоне - засекаем старт, если его не было
	cmp [ttip.tm_strt], 0
	jne .nextp
	    mcall SF_SYSTEM_GET, SSF_TIME_COUNT
	    mov [ttip.tm_strt], eax
	    jmp .nextp
    @@:
    ; если есть буфер, вернуть картинку взад, иначе идем дальше по цепочке
    mov [ttip.tm_strt], 0
    cmp [ttip.video_y], 0
    je .nextp
	movzx ecx, [ttip.video_w]
	shl ecx, 16
	mov cx, [ttip.video_h]
	movzx edx, [ttip.video_x]
	shl edx, 16
	mov dx, [ttip.video_y]
	mcall SF_PUT_IMAGE, [ttip.video]
	mov [ttip.video_y], 0
	;jmp exitp   ; мог быть только один - отмена - обработать нужно все сбросы таймеров
.nextp:
    ;mov eax, [ttip].next
    ;stdcall tooltip_mouse, [ttip].next, [mouse_coord], [mouse_but]
    ; tail call unroll recursion
    mov edi, [ttip.next]
    jmp .list_next
.exitp:
    popad
    ret
endp

; ---------------------------------------------------------------------------- ;
; decrypt font size as of SysFn 4
; returns eax = (x,y)
align 4
proc get_font_size, color:dword  ; x86 calling convention uses eax ecx edx
    ;font = color >> 24;
    ;int font_multipl = (font & 7) + 1;
    ;if (font & 0x10) // 8x16
    ;{
    ;    ed->width_sym = 8 * font_multipl;
    ;    ed->hight_sym = 16 * font_multipl;
    ;} else   // 6x9
    ;{
    ;    ed->width_sym = 6 * font_multipl;
    ;    ed->hight_sym = 9 * font_multipl;
    ;}
    mov edx, [color]
    shr edx, 24     ; font

    mov ecx, edx    ; font_multipl
    and ecx, 7
    inc ecx

    test edx, $10
	mov edx, ecx
    jz @f
	; 8x16
	shl edx, 3 + 16 ; x == width
	mov eax, edx
	mov edx, ecx
	shl edx, 4	; y == hight
	jmp .exitp
    @@:
	; 6x9
	imul edx, 6 ; x == width
	shl edx, 16
	mov eax, edx
	mov edx, ecx
	imul edx, 9	 ; y == hight
.exitp:
	or  eax, edx
    ret
endp

;---------------------------------------------------------------------
;--- ДАННЫЕ ПРОГРАММЫ ----------------------------------------------
;---------------------------------------------------------------------

proc_info   rd 1024

