format binary as ""
use32
org 0

; ================================================================

db 'MENUET01'
dd 1
dd START
dd I_END
dd MEM
dd STACKTOP
dd 0, 0

; ================================================================

include '../macros.inc'

START:                                  ; start of execution
        call    draw_window             ; draw the window

event_wait:
        mov     eax, 10                 ; function 10 : wait until event
        mcall                           ; event type is returned in eax

        cmp     eax, 1                  ; Event redraw request ˜
        je      red                     ; Expl.: there has been activity on screen and
                                        ; parts of the applications has to be redrawn.
        cmp     eax, 2                  ; Event key in buffer ˜
        je      key                     ; Expl.: User has pressed a key while the
                                        ; app is at the top of the window stack.
        cmp     eax, 3                  ; Event button in buffer ˜
        je      button                  ; Expl.: User has pressed one of the
                                        ; applications buttons.
        jmp     event_wait

red:                                    ; Redraw event handler
        call    draw_window             ; We call the window_draw function and
        jmp     event_wait              ; jump back to event_wait

key:                                    ; Keypress event handler
        mov     eax, 2                  ; The key is returned in ah. The key must be
        mcall                           ; read and cleared from the system queue.
        jmp     event_wait              ; Just read the key, ignore it and jump to event_wait.

button:                                  ; Buttonpress event handler
        mov     eax, 17                  ; The button number defined in window_draw
		mcall

        .close:
		        cmp     ah, 1
		        jne     .button_b
		        mov     eax, -1
		        mcall

		.button_b:
		        cmp     ah, 0x0B
		        jne     .button_c
		        mov     [charset], 0x80
		        mov     [curr_cs], cp6x9
		        call    draw_update
		        jmp     event_wait

        .button_c:
		        cmp     ah, 0x0C
		        jne     .button_d
		        mov     [charset], 0x90
		        mov     [curr_cs], cp8x16
		        call    draw_update
		        jmp     event_wait

        .button_d:
                cmp     ah, 0x0D
		        jne     .button_e
		        mov     [charset], 0xA0
		        mov     [curr_cs], utf16le
		        call    draw_update
		        jmp     event_wait

        .button_e:
                cmp     ah, 0x0E
		        jne     .button_f
		        mov     [charset], 0xB0
		        mov     [curr_cs], utf8
		        call    draw_update
		        jmp     event_wait

        .button_f:
                cmp     ah, 0x0F
		        jne     .button_10
		        mov     bl, [page]
		        dec     bl
		        mov     [page], bl
		        mov     cx, [letter]
		        mov     ch, bl
		        mov     [letter], cx
		        call    draw_update
		        jmp     event_wait

        .button_10:
                cmp     ah, 0x10
		        jne     event_wait
		        mov     bl, [page]
		        inc     bl
		        mov     [page], bl
		        mov     cx, [letter]
		        mov     ch, bl
		        mov     [letter], cx
		        call    draw_update
		        jmp     event_wait

        jmp     event_wait

draw_window:

        mcall   12, 1

        mcall   48, 3, window_colors, 40

        mcall     , 4
		push    eax
		; push    eax

        mov     eax, 0
        mov     ebx, 100 * 65536 + 436
        ; pop     ecx
        ; add     ecx, 100 * 65536 + 495
        mov     ecx, 100 * 65536 + 518
        mov     edx, [window_colors.work]
        add     edx, 0x34000000
        mov     edi, title
        mcall

        pop     esi
        add     esi, 495
        mcall   67, -1, -1, -1,

        call    draw_base
        call    draw_update

        mcall   12, 2

        ret

; ================================================================

; unchangeble base - table, headers and buttons
draw_base:

		.table:
				;light table background
        		; mcall   13, 0x0009019A, 0x0009019A, [window_colors.work]

        		; table borders lines
		        mcall     13, 0x0008019B, 0x00080001, [window_colors.work_text]
		        mcall       ,           , 0x00210001,
		        mcall       ,           , 0x01A20001,
		        mcall       , 0x00080001, 0x0008019A,
		        mcall       , 0x00210001,           ,
		        mcall       , 0x01A20001,           ,

		.headers:
				; horizontal table headers
				mov     eax, 4
				mov     ebx, 0x0026000E
				mov     ecx, [window_colors.work_text]
				add     ecx, 0x90000000
				mov     esi, 16

				.loop_hx:
				        mov     edx, header
				        mcall

				        mov     dx, [header]
				        add     dx, 0x0100

				        cmp     dx, 0x3A2D
				        jne     .hx_af
				        add     dx, 0x0700

				        .hx_af:
				        mov     [header], dx
				        add     ebx, 0x00180000
				        dec     esi
				        jnz     .loop_hx

				; vertical headers
				mov     ebx, 0x000D0027
				mov     esi, 16
				mov     [header], 0x2D30

				.loop_hy:
				        mov     edx, header
				        mcall

				        mov     dx, [header]
				        add     dx, 0x0001

				        cmp     dx, 0x2D3A
				        jne     .hy_af
				        add     dx, 0x0007

				        .hy_af:
				        mov     [header], dx
				        add     ebx, 0x00000018
				        dec     esi
				        jnz     .loop_hy

		        ; reset headers
		        mov     [header], 0x302D

		.buttons:
				; charsets change buttons
		        mcall    8, 0x0008005F, 0x01AB0017, 0x0000000B, [window_colors.work_button]
		        mcall     , 0x0071005F,           , 0x0000000C,
		        mcall     , 0x00DA005F,           , 0x0000000D,
		        mcall     , 0x0143005F,           , 0x0000000E,

				; page swap buttons
		        mcall     , 0x016B0017, 0x01CB0017, 0x0000000F,
		        mcall     , 0x018B0017,           , 0x00000010,

		        ; charsets change buttons subscriptions
		        mov      ecx, [window_colors.work_button_text]
		        add      ecx, 0xB0000000
		        mcall    4, 0x001401B0, , cp6x9
		        mcall     , 0x007901B0, , cp8x16
		        mcall     , 0x00DE01B0, , utf16le
		        mcall     , 0x014B01B0, , utf8

		        ; page swap buttons subscriptions
		        mcall     , 0x017201D0, , left
		        mcall     , 0x019301D0, , right

		ret

; changable data: current charset, page and letters
draw_update:
		; current charset and charpage
		.charpage:
				; temporary, background for letters
				mcall   13, 0x00220180, 0x00220180, [window_colors.work]

				; current charpage
		        mov     esi, [window_colors.work_text]
		        add     esi, 0x50000000
		        mcall   47, 0x00020101, page, 0x000D000E, , [window_colors.work]

				; current charset
				mov     ecx, [window_colors.work_text]
				add     ecx, 0xD0000000
		        mcall   4, 0x000801D0, , [curr_cs], , [window_colors.work]

		.letters:
				; 16x16 table of letters
				; mov     eax, 4

				;different coordinates for 6x9 charset
				mov     bl, [charset]
				cmp     bl, 0x80
                jne     .char_big

				.char_sm:
				        mov     ebx, 0x002C002A
				        jmp     .char_draw

				.char_big:
				        mov     ebx, 0x002A0027

				.char_draw:
						mov     cl,  [charset]
						shl     ecx, 24
						add     ecx, [window_colors.work_text]
						mov     esi, 16

				; letters draw loop
                .loop_ly:
				        mov     edi, 16

				        .loop_lx:
						        mov     edx, letter

						        cmp     [curr_cs], utf8
						        jne     .skip_lx

						        ;utf 8 to 16
						        xor     edx, edx
						        mov     dx, [letter]
						        push    esi
						        mov     esi, letutf
						        call    utf16to8
						        pop     esi
						        mov     edx, letutf

						        .skip_lx:

						        mcall

						        mov     dx, [letter]
						        add     dx, 0x01
						        mov     [letter], dx

						        add     ebx, 0x00180000

						        dec     edi
						        jnz     .loop_lx

				        ; start new row of letters
						sub     ebx, 0x017FFFE8

						dec     esi
						jnz     .loop_ly

				; reset letter from 0x0100 to 0x0000
				mov     dx, [letter]
				dec     dh
				mov     [letter], dx

		ret

; edx = num
; esi -> buffer, size 4
utf16to8:
		push    eax ecx edx
		xor     ecx, ecx
		mov     dword [esi], 0
		or      ecx, 3
		mov     eax, 0x80808000 + 11110000b

		cmp     edx, 0x00010000
		jae     @f
		mov     eax, 0x00808000 + 11100000b
		dec     ecx

		cmp     edx, 0x00000800
		jae     @f
		mov     eax, 0x00008000 + 11000000b
		dec     ecx

		cmp     edx, 0x00000080
		jae     @f
		mov     eax, edx
		dec     ecx

		@@:
				mov     [esi], eax

		@@:
		        mov     eax, edx
		        and     eax, 0x3F
		        shr     edx, 6
		        or      byte[esi + ecx], al
		        dec     ecx
		        jns     @b

		pop     edx ecx eax

		ret

; ================================================================

title   db "Charset Checker 0.2.5", 0

cp6x9   db "CP866 6x9  ", 0
cp8x16  db "CP866 8x16 ", 0
utf16le db "UTF-16 8x16", 0
utf8    db "UTF-8 8x16 ", 0

left    db "<", 0
right   db ">", 0

header  dw 0x302D, 0        ; "-0" symbols
letter  dw 0x0000, 0
letutf  dd 0x00000000, 0
charset db 0xB0
page    db 0x00

curr_cs dd utf8

window_colors system_colors
;window_height dd 0x00000000

; ================================================================

I_END:
        rb 4096
align 16
STACKTOP:

MEM:
