;--------------------------------------------------------------
;DRAW BLOCK
;
;--------------------------------------------------------------
draw_block:	mov eax,13
		mov edx,[color_table+edx*4]

		mov ebx,[current_block_x]
		mov ecx,[current_block_y]
		mov edi,[current_block_pointer]

		sub ebx,BORDER_LEFT
		imul ebx,ADOBE_SIZE
		add ebx,X_LOCATION
		shl ebx,16
		mov bx,ADOBE_SIZE

		sub ecx,BORDER_TOP
		imul ecx,ADOBE_SIZE
		add ecx,Y_LOCATION
		shl ecx,16
		mov cx,ADOBE_SIZE

		mov dword [TMP_1],4
adr_122:	mov dword [TMP_0],4
adr_121:	cmp byte [edi],0
		je adr_120

		int 040h

		call draw_frames

adr_120:	inc edi
		add ebx,ADOBE_SIZE*65536
		dec dword [TMP_0]
		jnz adr_121
		sub ebx,4*ADOBE_SIZE*65536
		add ecx,ADOBE_SIZE*65536
		dec dword [TMP_1]
		jnz adr_122

		ret

draw_frames:
		 cmp edx,0
		 jne df1
		 ret
	     df1:
		 pusha
		 mov bx,1
		 add edx,0x282828
		 mov eax,13
		 int 0x40
		 popa

		 pusha
		 mov cx,1
		 add edx,0x282828
		 mov eax,13
		 int 0x40
		 popa

		 pusha
		 push ebx
		 sub  bx,1
		 add  [esp+2],bx
		 pop  ebx
		 mov  bx,1
		 shr  edx,1
		 and  edx,0x7f7f7f
		 mov  eax,13
		 int  0x40
		 popa

		 pusha
		 push ecx
		 sub  cx,1
		 add  [esp+2],cx
		 pop  ecx
		 mov  cx,1
		 shr  edx,1
		 and  edx,0x7f7f7f
		 mov  eax,13
		 int  0x40
		 popa

		 ret

;-------------------------------------------------------------
; FIX BLOCK
;-------------------------------------------------------------
fix_block:	mov ebx,[current_block_pointer]

		mov edx,[current_block_y]
		imul edx,LEN_X
		add edx,[current_block_x]	;find the offset in tetris_t

		add edx,table_tetris

		mov ecx,4
		mov al,[current_block_color]

adr_21: 	cmp byte [ebx],1
		jne adr_22
		mov [edx],al
adr_22: 	inc ebx
		inc edx

		cmp byte [ebx],1
		jne adr_23
		mov [edx],al
adr_23: 	inc ebx
		inc edx

		cmp byte [ebx],1
		jne adr_24
		mov [edx],al
adr_24: 	inc ebx
		inc edx

		cmp byte [ebx],1
		jne adr_25
		mov [edx],al
adr_25: 	inc ebx
		add edx,LEN_X-3

		loop adr_21
		ret

;-------------------------------------------------------------
;NEW BLOCK
;-------------------------------------------------------------
new_block:	mov dword [current_block_y],1
		mov dword [current_block_x],7

		mov eax,dword [next_block_pointer]
		mov dword [current_block_pointer],eax

		mov eax,dword [next_block_color]
		mov dword [current_block_color],eax

		call random
		and al,7
		setz ah
		add al,ah
		mov [next_block_color],al

		call random
		;and eax,15
	    ; ---- Ivan ----
		and eax,0xff
	      @@:
		cmp eax,_MAXBLOCKS_
		jl  @f
		add eax,-(_MAXBLOCKS_)
		jmp @b
	      @@:
	    ; ---- Ivan ----
		mov edx,[block_table+eax*4]
		mov [next_block_pointer],edx

		mov dword[delay],5
		sub dword[delay],speed

		ret


;-------------------------------------------------------------
;DRAW TITLE BLOCK
;-------------------------------------------------------------
draw_title_block:
;    movzx edx,byte [current_block_color]
    mov eax,13

;    mov edx,[color_table+edx*4]

;    mov ebx,[current_block_x]
;    mov ecx,[current_block_y]
;    mov edi,[current_block_pointer]

    sub ebx,BORDER_LEFT
    imul ebx,ADOBE_SIZE
    add ebx,X_LOCATION
    shl ebx,16
    mov bx,ADOBE_SIZE

    sub ecx,BORDER_TOP
    imul ecx,ADOBE_SIZE
    add ecx,Y_LOCATION
    shl ecx,16
    mov cx,ADOBE_SIZE

    mov dword [TMP_1],5
    call adr_122

ret

;-------------------------------------------------------------
;FIRST BLOCK
;-------------------------------------------------------------
first_block:	call random
		and al,7
		setz ah
		add al,ah
		mov [next_block_color],al

		call random
		;and eax,15
	    ; ---- Ivan ----
		and eax,0xff
	      @@:
		cmp eax,_MAXBLOCKS_
		jl  @f
		add eax,-(_MAXBLOCKS_)
		jmp @b
	      @@:
	    ; ---- Ivan ----
		mov edx,[block_table+eax*4]
		mov [next_block_pointer],edx

;                call draw_next_block

;                mov byte [delay],5 ;19 ;!!! 15
		ret
ret

;-------------------------------------------------------------
;DRAW NEXT BLOCK
;-------------------------------------------------------------

draw_next_block:
		movzx edx,byte [next_block_color]
		mov eax,13
		mov edx,[color_table+edx*4]

		mov ebx,LEN_X+1
		mov ecx,5
		mov edi,[next_block_pointer]

		sub ebx,BORDER_LEFT
		imul ebx,ADOBE_SIZE
		add ebx,X_LOCATION
		shl ebx,16
		mov bx,ADOBE_SIZE

		sub ecx,BORDER_TOP
		imul ecx,ADOBE_SIZE
		add ecx,Y_LOCATION
		shl ecx,16
		mov cx,ADOBE_SIZE

		mov dword [TMP_1],4
		jmp adr_122

ret

;-------------------------------------------------------------
; ROTATE BLOCK
;-------------------------------------------------------------

rotate_block:
		mov  edx,[current_block_pointer]
		mov  edx,[edx+16]
		mov  esi,[current_block_pointer]
		mov  [current_block_pointer],edx
		call check_crash
		call attesa
		mov  [current_block_pointer],esi

ret


;-------------------------------------------------------------
; CHECK CRASH
; output        Z  flag => OK
;               NZ flag => NO
;-------------------------------------------------------------

check_crash:	mov ebx,[current_block_pointer]

		mov edx,[current_block_y]
		imul edx,LEN_X
		add edx,[current_block_x]	   ;find the offset in tetris_t

		add edx,table_tetris

		mov ecx,4
		xor ax,ax

adr_1:		cmp byte [ebx],1
		jne adr_2
		add al,[edx]
		adc ah,0
adr_2:		inc ebx
		inc edx

		cmp byte [ebx],1
		jne adr_3
		add al,[edx]
		adc ah,0
adr_3:		inc ebx
		inc edx

		cmp byte [ebx],1
		jne adr_4
		add al,[edx]
		adc ah,0
adr_4:		inc ebx
		inc edx

		cmp byte [ebx],1
		jne adr_5
		add al,[edx]
		adc ah,0
adr_5:		inc ebx
		add edx,LEN_X-3

		loop adr_1
		or ax,ax
		ret

;--------------------------------------------------------------
;CHECK LINE
;--------------------------------------------------------------
;edx = pointer
;ebx = contatore
check_full_line:
		std
		mov al,0
		mov edx,table_tetris+LEN_X*(LEN_Y-BORDER_BOTTOM)-1
		mov ebx,(LEN_Y-BORDER_TOP-BORDER_BOTTOM-1)*LEN_X

adr_5000:	mov edi,edx
		mov ecx,LEN_X-BORDER_LEFT-BORDER_RIGHT
		repne scasb
		jz no_full_line

		lea esi,[edx-LEN_X]
		mov edi,edx
		mov ecx,ebx
		rep movsb
		sub edi,BORDER_RIGHT
		mov ecx,LEN_X-BORDER_LEFT-BORDER_RIGHT
		rep stosb

		add dword [score],100
		add dword [lines],1

;                mov esi,dword[score]
;
;
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;                mov esi,dword [speed]
;                imul esi,linestonewlevel
;                add esi,linestonewlevel
;                cmp dword [lines],esi
;                jne  adr_5000                          1 line : 100
;                cmp dword[speed],4                     2 lines: 300
;                je  adr_51                             3 lines: 700
;                inc dword[speed]                       4 lines:1500
;                jmp adr_5000
;               adr_51:
;                mov dword[speed],0
;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


		jmp adr_5000

no_full_line:	sub edx,LEN_X
		sub ebx,LEN_X
		jnz adr_5000

		ret