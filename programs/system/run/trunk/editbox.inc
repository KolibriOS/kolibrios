; <Lrz> 21.07.2006 добавлена функция кнопки Del, теперь можно удалять символы при помощи данной колавиши
; Автор: Евтихов Максим (Maxxxx32) email: maxxxxm@mail.ru
; Дата последних изменений: 13.07.06 10:40
;  Напишите в исходном коде своей программы use_edit_box,
; это вставит необходимые процедуры в код вашей программы.
;  Процедуры:
;   edit_box.draw - полная перерисовка;
;   edit_box.key - обработка клавиатуры;
;   edit_box.mouse - обработка мыши;
;   edit_box.focus - установка фокуса;
;   edit_box.blur - его размывание;
;   edit_box.get_n - получить количество прорисовываемых символов.
;  При вызове любых из этих процедур в регистре edi следует
; оставить указатель на структуру.
;  Пример структуры:
; some_edit edit_box 100,10,30,0x00ffffff,0,0x00aaaaaa,0,255,some_edit_text
; длина, лево, верх, цвет фона, цвет рамки, если выбран,
; цвет рамки, если не выбран, максимальное количество символов,
; указатель на буфер, где будут хранится символы. Буфер должен
; оканчиваться нулем, например если максимальное количество 255:
;  some_edit_text:
;       rb      256 ;255+1

;  Пример вызова процедуры:
;       mov     edi,some_edit
;       call    edit_box.draw

;  При вызове процедуры edit_box.key код клавиши должен
; находится в ah, то есть перед вызовом этой процедуры надо вызвать
; 2-ую сисемную функцию, например:
;       mov     eax,2
;       int     0x40
;       mov     edi,some_edit1
;       call    edit_box.key
;       mov     edi,some_edit2
;       call    edit_box.key
;  Перед вызовом обработчика мыши следует проверить, является ли окно
; активным.
;  Если щелчок мыши был пройзведен за пределами edit box
; фокус теряется.
macro use_edit_box
{
edit_box:
ed_width        equ [edi]
ed_left         equ [edi+2]
ed_top          equ [edi+4]
ed_color        equ [edi+6]
ed_focus_border_color   equ [edi+10]
ed_blur_border_color    equ [edi+14]
ed_text_color   equ [edi+18]
ed_flags        equ [edi+22]
ed_max  equ [edi+24]
ed_size equ [edi+26]
ed_pos  equ [edi+28]
ed_offset       equ [edi+30]
ed_text equ [edi+32]
ed_height=14 ; высота
;==========================================================
;=== процедура прорисовки =================================
;==========================================================
.draw:
pusha
;--- рисуем рамку ---
        call    .draw_border

.draw_bg_cursor_text:
;--- изменяем смещение, если надо ---
        call    .check_offset
;--- рисуем внутреннюю область ---
        call    .draw_bg
.draw_cursor_text:
;--- рисуем курсор ---
        ;--- может его не надо рисовать ----
        bt      word ed_flags,1
        jnc     @f
        call    .draw_cursor
@@:
        call    .draw_text
popa
ret

;----------------------------------------------------------
;--- процедура прорисовки текста --------------------------
;----------------------------------------------------------
.draw_text:
;--- вычисляем, сколько помещается символов ---
        call    .get_n
;--- чтобы мусор не рисовать ---
        movsx   esi,word ed_size
        sub     si,ed_offset
        cmp     ax,si
        ja      @f
        mov     si,ax
@@:

;--- рисуем текст ---
        mov     eax,4
        mov     bx,ed_left
        add     bx,2
        shl     ebx,16
        mov     bx,ed_top
        add     bx,4
        mov     ecx,ed_text_color
        movsx   edx,word ed_offset
        add     edx,ed_text
        int     0x40
ret

;----------------------------------------------------------
;--- процедура прорисовки фона ----------------------------
;----------------------------------------------------------
.draw_bg_eax:
pusha
        movsx   ebx,ax
        lea     ebx,[ebx*2+ebx]
        shl     bx,1
        shl     ebx,16
        shr     eax,16
        lea     eax,[eax*2+eax]
        shl     ax,1
        add     ax,ed_left
        add     ax,2
        mov     bx,ax
        ror     ebx,16
        jmp     @f
.draw_bg:
pusha
        mov     bx,ed_left
        inc     bx
        shl     ebx,16
        mov     bx,ed_width
        dec     bx

@@:
        mov     cx,ed_top
        inc     cx
        shl     ecx,16
        mov     cx,ed_height
        dec     cx

        mov     edx,ed_color
        mov     eax,13
        int     0x40
popa
ret

;----------------------------------------------------------
;--- процедура получения количества символов --------------
;----------------------------------------------------------
.get_n:
        xor     edx,edx
        movsx   eax,word ed_width
        sub     ax,4
        mov     bx,6
        div     bx
ret

;----------------------------------------------------------
;--- процедура рисования курсора --------------------------
;----------------------------------------------------------
.clear_cursor:
        mov     edx,ed_color
        jmp     @f
.draw_cursor:
        mov     edx,ed_text_color
@@:
        movsx   ebx,word ed_pos
        sub     bx,ed_offset
        ;lea     ebx,[ebx*2+ebx]
        ;shl     ebx,1
        imul    ebx,6
        add     bx,ed_left
        inc     bx
        push    bx
        shl     ebx,16
        pop     bx
        mov     cx,ed_top
        add     cx,2
        push    cx
        shl     ecx,16
        pop     cx
        add     cx,ed_height-4
        mov     eax,38
        int     0x40
ret

;----------------------------------------------------------
;--- процедура рисования рамки ----------------------------
;----------------------------------------------------------
.draw_border:
;--- цвет рамки ---
        bt      word ed_flags,1
        mov     edx,ed_focus_border_color
        jc     @f
        mov     edx,ed_blur_border_color
@@:
;--- сверху ---
        mov     bx,ed_left
        push    bx
        shl     ebx,16
        pop     bx
        add     bx,ed_width
        mov     cx,ed_top
        push    cx
        shl     ecx,16
        pop     cx
        mov     eax,38
        int     0x40

;--- снизу ---
        push    cx
        add     cx,ed_height
        push    cx
        shl     ecx,16
        pop     cx
        int     0x40

;--- слева ---
        pop     cx
        push    bx
        sub     bx,ed_width
        int     0x40

;--- справа ---
        pop     bx
        push    bx
        shl     ebx,16
        pop     bx
        int     0x40
ret

;----------------------------------------------------------
;--- проверка, зашел ли курсор за границы и, если надо, ---
;--- изменяем смещение ------------------------------------
;--- eax = 1, если не изменилось или eax = 0, если
; изменилось
;----------------------------------------------------------
.check_offset:
        push    word ed_offset
        call    .get_n
;--- правее ---
        mov     bx,ed_offset
        add     bx,ax
        cmp     bx,ed_pos
        ja      @f
        mov     bx,ed_pos
        mov     cx,ax
        shr     cx,2
        sub     bx,cx
        mov     cx,ax
        shr     cx,1
        sub     bx,cx
        mov     ed_offset,bx
@@:
;--- левее ---
        mov     bx,ed_offset
        cmp     bx,ed_pos
        jb      @f
        mov     bx,ed_pos
        mov     cx,ax
        shr     cx,2
        sub     bx,cx
        jnc     $+5
        xor     bx,bx
        mov     ed_offset,bx
@@:
        xor     eax,eax
        pop     bx
        cmp     bx,ed_offset
        jne     @f
        inc     eax
@@:
ret

;==========================================================
;=== обработка клавиатуры =================================
;==========================================================
.key:
pusha
        bt      word ed_flags,1 ; если не в фокусе, выходим
        jc     @f
        popa
        ret
@@:

;----------------------------------------------------------
;--- проверяем, что нажато --------------------------------
;----------------------------------------------------------
        cmp     ah,8
        je      .backspace
	cmp	ah,0xb6
	je	.delete
        cmp     ah,176
        je      .left
        cmp     ah,179
        je      .right
        cmp     ah,180
        je      .home
        cmp     ah,181
        je      .end

;--- нажата другая клавиша ---
        bt      word ed_flags,15  ; только цифры ?
        jnc     @f
        cmp     ah,'0'
        jb      .no_figure
        cmp     ah,'9'
        ja      .no_figure
        jmp     @f
.no_figure:
        popa
        ret
@@:
        ; проверяем, находится ли курсор в конце
        mov     bx,ed_max
        cmp     bx,ed_pos
        jne     @f ; если да, в конец процедуры
        popa
        ret
@@:
        ; сдвигаем символы после курсора вправо
        mov     edx,ed_text
        dec     edx
        xor     ecx,ecx
        mov     cx,ed_size
        cmp     cx,ed_pos
        je      @f
.mov_right_next:
        mov     bh,[edx+ecx]
        mov     [edx+ecx+1],bh
        dec     cx
        cmp     cx,ed_pos
        jne     .mov_right_next
@@:


        ; вставляем код клавиши туда, где курсор
        mov     edx,ed_text
        movsx   ebx,word ed_pos
        mov     [ebx+edx],ah

        call    .clear_cursor
        ; увеличиваем значение размера и позиции
        inc     word ed_pos
        mov     bx,ed_size
        cmp     bx,ed_max
        je      .draw_bg_cursor_text;.key_end_draw
        inc     word ed_size

        ;push    word ed_offset
        call    .check_offset
        ;pop     ax
        ;cmp     ax,ed_offset
        ;jne     .draw_bg_cursor_text
        test    eax,eax
        jz      .draw_bg_cursor_text

        call    .get_n
        sub     ax,ed_pos
        add     ax,ed_offset
        inc     ax
        shl     eax,16
        mov     ax,ed_pos
        sub     ax,ed_offset
        dec     ax
        ror     eax,16
        call    .draw_bg_eax

        jmp    .draw_cursor_text

        ;jmp     .draw_cursor_text;.key_end_draw

.delete:	
	mov	dx,ed_size
	mov	cx,ed_pos
	;inc	edx
	;add	ecx,900
	;sub	ecx,1
	cmp	dx,cx
	jg	@f
	popa
	ret

@@:	
	mov	edx,ed_text
	;dec	edx
.mov_stop_next:
	mov	bh,[edx+ecx+1]
	mov	[edx+ecx],bh
	inc	cx
	mov	bx,ed_size
	;dec	bx
	;dec	bx
	cmp	cx,bx
	jne	.mov_stop_next

	; уменьшаем значение позиции и размера
	;dec	word ed_pos
	dec	word ed_size
		;push    word ed_offset
	call	.check_offset
	;pop     ax
	;cmp     ax,ed_offset
	;jne     .draw_bg_cursor_text
	test	eax,eax
	jz	.draw_bg_cursor_text

	call	.get_n
	sub	ax,ed_pos
	add	ax,ed_offset
	shl	eax,16
	mov	ax,ed_pos
	sub	ax,ed_offset
	ror	eax,16
	call	.draw_bg_eax

	jmp    .draw_cursor_text
popa
ret

;--- нажата клавиша backspace ---
.backspace:
        ; проверяем, курсор у левого края ?
        mov     bx,ed_pos
        ;cmp     bx,0
        ;jne     @f; .key_end ; если да, в конец
        test    bx,bx
        jnz     @f
        popa
        ret
@@:

        ; сдвигаем символы после курсора влево
        mov     edx,ed_text
        dec     edx
        movsx   ecx,word ed_pos

.mov_left_next:
        mov     bh,[edx+ecx+1]
        mov     [edx+ecx],bh
        inc     cx
        mov     bx,ed_size
        inc     bx
        cmp     cx,bx
        jne     .mov_left_next

        ; уменьшаем значение позиции и размера
        dec     word ed_pos
        dec     word ed_size

        ;push    word ed_offset
        call    .check_offset
        ;pop     ax
        ;cmp     ax,ed_offset
        ;jne     .draw_bg_cursor_text
        test    eax,eax
        jz      .draw_bg_cursor_text

        call    .get_n
        sub     ax,ed_pos
        add     ax,ed_offset
        shl     eax,16
        mov     ax,ed_pos
        sub     ax,ed_offset
        ror     eax,16
        call    .draw_bg_eax

        jmp    .draw_cursor_text
popa
ret

;--- нажата клавиша left ---
.left:
        mov     bx,ed_pos
        ;cmp     bx,0
        ;je      @f
        test    bx,bx
        jz      @f
        call    .clear_cursor
        dec     word ed_pos
        call    .check_offset
        test    eax,eax
        jz      .draw_bg_cursor_text
        call    .draw_cursor
@@:
popa
ret

;--- нажата клавиша right ---
.right:
        mov     bx,ed_pos
        cmp     bx,ed_size
        je      @f
        call    .clear_cursor
        inc     word ed_pos
        call    .check_offset
        test    eax,eax
        jz      .draw_bg_cursor_text
        call    .draw_cursor
@@:
popa
ret

.home:
        mov     bx,ed_pos
        test    bx,bx
        jz      @f
        call    .clear_cursor
        xor     eax,eax
        mov     ed_pos,ax
        call    .check_offset
        test    eax,eax
        jz      .draw_bg_cursor_text
        call    .draw_cursor
@@:
popa
ret

.end:
        mov     bx,ed_pos
        cmp     bx,ed_size
        je      @f
        call    .clear_cursor
        mov     ax,ed_size
        mov     ed_pos,ax
        call    .check_offset
        test    eax,eax
        jz      .draw_bg_cursor_text
        call    .draw_cursor
@@:
popa
ret


;==========================================================
;=== обработка мыши =======================================
;==========================================================
.mouse:
pusha
;----------------------------------------------------------
;--- получаем состояние кнопок мыши -----------------------
;----------------------------------------------------------
        mov     eax,37
        mov     ebx,2
        int     0x40
;----------------------------------------------------------
;--- проверяем состояние ----------------------------------
;----------------------------------------------------------
        bt      eax,0
        jc      .mouse_left_button

        bt      eax,1
        jc      @f
        btr     word ed_flags,2
@@:

        popa
        ret

.mouse_left_button:
;----------------------------------------------------------
;--- получаем координаты мыши -----------------------------
;----------------------------------------------------------
        bts     word ed_flags,2
        jc      .mouse_pressed
        mov     eax,37
        ;mov     ebx,1
        xor     ebx,ebx
        inc     ebx
        int     0x40
;----------------------------------------------------------
;--- проверяем, попадает ли курсор в edit box -------------
;----------------------------------------------------------
        mov     bx,ed_top
        cmp     ax,bx
        jl      ._blur;.mouse_end_no_focus

        add     bx,ed_height
        cmp     ax,bx
        jg      ._blur;.mouse_end_no_focus

        shr     eax,16

        mov     bx,ed_left
        cmp     ax,bx
        jl      ._blur;.mouse_end_no_focus

        add     bx,ed_width
        cmp     ax,bx
        jg      ._blur;.mouse_end_no_focus

;--- изменяем позицию курсора ---
        push    eax
        call    .clear_cursor
        pop     eax
        xor     dx,dx
        sub     ax,ed_left
        add     ax,2
        mov     bx,6
        div     bx
        add     ax,ed_offset
        cmp     ax,ed_size
        jna      @f
        mov     ax,ed_size
@@:
        mov     ed_pos,ax
        call    .check_offset
        test    eax,eax
        jz      .draw_bg_cursor_text
        call    .draw_cursor

        call    .focus

.mouse_pressed:
popa
ret

;.mouse_end_no_focus:
;        call    .blur
;popa
;ret


;----------------------------------------------------------
;--- процедура установки фокуса ---------------------------
;----------------------------------------------------------
.blur:
pusha
._blur:
        bt      word ed_flags,14
        jc      @f
        btr     word ed_flags,1
        jnc     @f
        call    .clear_cursor
        jmp     .blur_end
.focus:
pusha
        bts     word ed_flags,1
        jc      @f
        call    .draw_cursor
.blur_end:
        call    .draw_border
@@:
popa
ret

;----------------------------------------------------------
;--- процедура размывания фокуса --------------------------
;----------------------------------------------------------
;.blur:
;pusha
;._blur:
        ;btr      word ed_flags,1
        ;jnc     @f
        ;call    .draw_border
        ;call    .clear_cursor
;@@:
;popa
;ret
}
ed_figure_only=1000000000000000b
ed_always_focus=100000000000000b
ed_focus=10b
ed_struc_size=36
macro draw_edit_boxes start,end
{
        mov     edi,start
        mov     ecx,((end-start)/ed_struc_size)
@@:
        call    edit_box.draw
        add     edi,ed_struc_size
        loop    @b
}

macro mouse_edit_boxes start,end
{
        mov     edi,start
        mov     ecx,((end-start)/ed_struc_size)
@@:
        call    edit_box.mouse
        add     edi,ed_struc_size
        loop    @b
}

macro key_edit_boxes start,end
{
        mov     edi,start
        mov     ecx,((end-start)/ed_struc_size)
@@:
        call    edit_box.key
        add     edi,ed_struc_size
        loop    @b
}

struc  edit_box width,left,top,color,focus_border_color,\
       blur_border_color,text_color,max,text,flags,size
{
.width dw width
.left dw left
.top dw top
.color dd color
.focus_border_color dd focus_border_color
.blur_border_color dd blur_border_color
.text_color dd text_color
.flags dw flags+0
.max dw max
.size dw size+0
.pos dw 0
.offset dw 0
.text dd text
}

macro edit_boxes_set_sys_color start,end,color_table
{
        mov     edi,start
        mov     ecx,((end-start)/ed_struc_size)
        mov     esi,color_table
@@:
        mov     eax,[esi+36]
        mov     ebx,[esi+20]
        mov     ed_focus_border_color,eax
        shr     bh,1
        shr     bl,1
        shr     ah,1
        shr     al,1
        add     ah,bh
        add     al,bl
        ror     eax,16
        ror     ebx,16
        shr     bl,1
        shr     al,1
        add     al,bl
        ror     eax,16
        mov     ed_blur_border_color,eax
        add     edi,ed_struc_size
        loop    @b
}

macro draw_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.draw
}

macro mouse_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.mouse
}

macro key_edit_box ed_ptr
{
        mov     edi,ed_ptr
        call    edit_box.key
}