; 4.11.2009 staper@inbox.ru

; 2.06.2010 fixed a little bug in check

;based on http://sources.codenet.ru/download/1599/Sudoku.html

; В правом нижнем углу располагается серая кнопка,
;при нажатию на которую окно увеличивается в размере.

; Программа проверяет строку параметров; просто передайте
;путь к файлу с задачей.

; Сочетания клавиш:
;	n - сгенерировать таблицу
;	c - проверить решение
;	пробел - показать решение
;	+-  - изменить уровень сложности
;	t - приостановить/запустить секундомер
;	i - ввести свой пример
;	r - решить
;	l - загрузить задачу из файла
;	s - сохранить задачу в файл
;	a - сохранить решение в файл

use32
org	0x0
 db	'MENUET01'
 dd	0x1, START, I_END, (D_END+10000) and not 3,  (D_END+10000) and not 3, buf_cmd_lin, cur_dir_path

Difficult db 0	;сложность [0..9]
Difficult_array db 80,75,68,59,50,45,40,36,32,25

;Цвета:
Bckgrd_clr	equ 0xffffff	;фон
Brdr_line_clr	equ 0x780000	;линии по границам
Inter_line_clr	equ 0xb0b0b0	;внутренние линии
Square_clr	equ 0xdddddd	;цвет курсора
Fix_nmb_clr	equ 0x335533;0	;статичное значение
Chg_nmb_clr	equ 0;x008d8d	;переменное значение
Text_clr	equ 0x000000	;текст
Message_clr	equ 0x0000ff	;сообщения

DEBUG equ 0

macro dbg_dec num
{pushad
newline
debug_print_dec num
popad
}

include 'macros.inc'
;include 'debug.inc'
include 'editbox_ex.mac'
include 'lang.inc'
include 'SudokuSolve.pas'


START:
	mcall	40,7
	mcall	3
	mov	[rsx1],ax
	ror	eax,16
	mov	[rsx2],ax
	rol	eax,7
	mov	[rsx3],ax
	rol	eax,7
	mov	[rsx4],ax

;	call	LOAD_LIB	;loading Box_Lib library

; This is the part of the macros for load any library/libraries by <Lrz>
LOAD_LIB:
	mcall	68,19,system_path   ; load of sys directory
	test	eax,eax
	jnz	.end_steep

	bts	[flags],7
;	ret
	jmp	.end

.end_steep:

; initialize import
	mov	edx, eax
	mov	esi,myimport
.import_loop:
	lodsd
	test	eax, eax
	jz	.import_done
	push	edx
.import_find:
	mov	ebx, [ds:edx]
	test	ebx, ebx
	jz	.exit   ;import_not_found
	push	eax
.lp:
	mov	cl, [ds:eax]
	cmp	cl, [ds:ebx]
	jnz	.import_find_next
	test	cl, cl
	jz	.import_found
	inc	eax
	inc	ebx
	jmp	.lp
.import_find_next:
	pop	eax
	add	edx, 8
	jmp	.import_find
.import_found:
	pop	eax
	mov	eax, [ds:edx+4]
	mov	[esi-4], eax
	pop	edx
	jmp	.import_loop
.exit:
	add	esp,4

	bts	[flags],7
;ret
	jmp	.end

.import_done:
	btr	[flags],7

;определяем длину строки с параметрами
	mov	esi,buf_cmd_lin
	xor	eax,eax
@@:	cmp	byte [esi+eax],0
	je	@f
	inc	eax
	jmp	@b
@@:	mov	dword [edit1.size],eax
	mov	dword [edit1.pos],eax
;ret
.end:

	bt	[flags],7
	jc	@f
	mcall	68,11
	cmp	dword [edit1.pos],0
	je	@f
	call	load_sudoku
	jnc	redraw_all
@@:	jmp	key.new_game


redraw_all:
	mcall	12,1
	mcall	48,4
	add	eax,100*65536+(24*9+67)
	mov	ecx,eax
	mcall	0,100*65536+(24*9+12),,(0x34000000+Bckgrd_clr),,title
	mcall	38,1*65536+(24*9+1),21*65536+21,Brdr_line_clr
	mov	edx,Inter_line_clr
	mov	edi,3
	mov	esi,3
  @@:	add	ecx,24*65536+24
	mcall
	dec	esi
	jnz	@b
	mov	esi,3
	push	edx
	mcall	,,,Brdr_line_clr
	pop	edx
	dec	edi
	jnz	@b
	mcall	,1*65536+1,22*65536+236,Inter_line_clr;0xe7e6a0
	mov	edx,Inter_line_clr
	mov	edi,3
	mov	esi,3
	push	edx
	mcall	,,,Brdr_line_clr
	pop	edx

  @@:	add	ebx,24*65536+24
	mcall
	dec	esi
	jnz	@b
	mov	esi,3
	push	edx
	mcall	,,,Brdr_line_clr
	pop	edx
	dec	edi
	jnz	@b

	mcall	8,<208,8>,<257,8>,2,0xbbbbbb
	mcall	4,<5,5>,(0x80000000+Text_clr),txt.new
	mcall	,<105,5>,,txt.dif
	mcall	,<5,258>,,txt.space
	mcall	,<5,246>,,txt.check
	mcall	,<129,246>,,txt.time
	mcall	,<5,285>,,txt.own_map
	mcall	,<5,296>,,txt.solve
	mcall	,<90,296>,,txt.load
	mcall	,<5,306>,,txt.save
	mcall	,<90,306>,,txt.save_solve

	bts	[flags],5
	call	Timer.0
	call	show_level

	push	dword Map;esi;  mov     esi,Map
	mcall	12,2
draw_pole:
	if DEBUG
	call	SysMsgBoardNum	;show esp
	endf

	movzx	eax,[Y]
	dec	al
	mov	ebx,9
	mul	bl
	mov	bl,[X]
	add	al,bl
	pop	esi	;       mov     esi,Map
	push	eax	;курсорчик
	mov	edi,81-9
	mov	ebp,9
	mov	ebx,1*65536+21
	mov	ecx,21*65536+41
	call	out_numbers
	pop	eax
	bt	[flags],2
	jc	key.0

still:
	mcall	23,10
	test	eax,eax
	jz	Timer

	dec	al
	jz	redraw_all
	dec	al
	jz	key
	dec	al
	jnz	still
;button:
	mcall	17
	cmp	ah,1
	jne	@f
	mcall	-1
@@:	cmp	ah,2
	jne	still
	btc	[flags],9
	mcall	48,4
	add	eax,(24*9+67)
	mov	esi,eax
	bt	[flags],9
	jnc	@f
	add	esi,40
@@:	mcall	67,100,100,(24*9+12),
	jmp	still
key:
	mcall	2
	cmp ah,32		;пробел
jne	@f
	btc	[flags],3
	jc	.todraw
	push	dword SolveMap
	jmp	draw_pole
	.todraw:
	push	dword Map
	jmp	draw_pole
@@:
	bt	[flags],3
	jnc	@f
	bts	[flags],2
	push	eax
	push	dword Map
	jmp	draw_pole
.0:	pop	eax
	btr	[flags],2
	btr	[flags],3
@@:	cmp	ah,108		;l
	jne	@f
	bt	[flags],7
	jc	still
	bt	[flags],6
	jc	still
	bts	[flags],8
	call	window_load_save
	jmp	still
@@:	cmp	ah,115		;s
	jne	@f
	btr	[flags],10
.sresh: bt	[flags],7
	jc	still
	bt	[flags],6
	jc	still
	btr	[flags],8
	call	window_load_save
	jmp	still
@@:	cmp	ah,97		;a
	jne	@f
	bts	[flags],10
	jmp	.sresh
@@:	cmp	ah,116		;t
	jne	@f
	btc	[flags],1
	jnc	still
	mcall	26,9
	sub	eax,[Ticks_new]
	push	eax
	mov	eax,[Ticks]
	mov	ebx,100
	mul	ebx
	pop	ecx
	sub	ecx,eax
	add	[Ticks_new],ecx
	jmp	still

@@:	cmp	ah,105		;i
	jne	@f
		xor	ecx,ecx
		.105_0:
		mov	byte [SolveMap+ecx],0
		mov	byte [Map+ecx],0
		inc	ecx
		cmp	ecx,9*9
		jb	.105_0
		jmp	.todraw

@@:	cmp	ah,114		;r
	jne	.43
	xor	ecx,ecx
	.114_0:
		mov	al,[Map+ecx]
		cmp	al,9
		jbe	@f
		sub	al,10
		@@:
		mov	[SolveMap+ecx],al
		inc	ecx
		cmp	ecx,9*9
		jb	.114_0
	mov	esi,SolveMap
	call	Solve
	cmp	[_iRet],1
	jne	@f
		mov	edx,txt.check_yes
		call	Show_message
		jmp	still
	@@: ;нет решений
		xor	ecx,ecx
		@@:
		mov	byte [SolveMap+ecx],0
		inc	ecx
		cmp	ecx,9*9
		jb	@b
		mov	edx,txt.nosolve
		call	Show_message
		jmp	still

.43:	cmp	ah,43		;+
	jne	.45
	cmp	[Difficult],9
	je	still
	inc	[Difficult]
	call	show_level
	jmp	still
.45:				;-
	cmp	ah,45
	jne	.99
	cmp	[Difficult],0
	je	still
	dec	[Difficult]
	call	show_level
	jmp	still

.99:				;Check
	cmp	ah,99
	jne	.39
	bts	[flags],15
	xor	ecx,ecx
	mov	edx,txt.check_no
; @@:	mov	al,byte [Map+ecx]
;	cmp	byte [SolveMap+ecx],al
;	jne	@f
;	inc	ecx
;	cmp	ecx,9*9
;	jb	@b
 @@:	mov	al,byte [Map+ecx]	;переносим значения во временный массив
	cmp	al,9
	jbe	.991
		sub	al,10
	.991:
	mov	[TempMap+ecx],al
	inc	ecx
	cmp	ecx,9*9
	jb	@b
	mov	esi,TempMap
	call	CheckSudoku
	jc	@f
	mov	edx,txt.check_yes
 @@:	btr	[flags],15
	call	Show_message
	jmp	.todraw

.39:	cmp	ah,0x39
	ja	.110
	cmp	ah,0x30
	jb	still
	sub	ah,0x30
	mov	cl,ah

	movzx	eax,[Y]
	dec	al
	mov	ebx,9
	mul	bl
	mov	bl,[X]
	dec	bl
	add	al,bl
	mov	esi,Map
	cmp	byte [esi+eax],9
	jg	still
	mov	[esi+eax],cl
	jmp	.onedraw

.110:	cmp	ah,110		;n
	jne	.176
.new_game:
	call	GeneratePlayBoard
	mov	[Ticks],0
	mcall	26,9
	mov	[Ticks_new],eax
	jmp	redraw_all

.176:	cmp	ah,176 ;курсоры
	jne	.177
	call	draw_one_symbol
	dec	[X]
	cmp	[X],1
	jge	@f
	mov	[X],9
@@:	jmp	.onedraw
.177:	cmp	ah,177
	jne	.178
	call	draw_one_symbol
	inc	[Y]
	cmp	[Y],9
	jbe	@f
	mov	[Y],1
@@:	jmp	.onedraw
.178:	cmp	ah,178
	jne	.179
	call	draw_one_symbol
	dec	[Y]
	cmp	[Y],1
	jge	@f
	mov	[Y],9
@@:	jmp	.onedraw
.179:	cmp	ah,179
	jne	still
	call	draw_one_symbol
	inc	[X]
	cmp	[X],9
	jbe	@f
	mov	[X],1
@@:
.onedraw:
	bts	[flags],4
	call	draw_one_symbol
	jmp	still ;.todraw

show_level:
	movzx	ecx,[Difficult]
	mcall	47,0x10000,,<205,5>,(0x50000000+Text_clr),Bckgrd_clr
ret

Show_message:
	mcall	4,<5,269>,(0xd0000000+Message_clr),,,Bckgrd_clr
ret

Timer:
	bt	[flags],1
	jc	still
	mcall	26,9
	sub	eax,[Ticks_new]
	mov	ebx,100
	xor	edx,edx
	div	ebx
	cmp	eax,[Ticks]
	je	still
	mov	[Ticks],eax
.1:	mov	ebx,60
	xor	edx,edx
	div	ebx
	push	eax
	mov	ecx,edx
	mcall	47,0x00020000,,<206,246>,(0x40000000+Text_clr),Bckgrd_clr
	pop	ecx
	mov	edx,189*65536+246
	mcall
	bt	[flags],5
	jnc	@f
	btr	[flags],5
	ret
@@:	jmp	still
.0:	mov	eax,[Ticks]
	jmp	.1

draw_one_symbol:
	movzx	eax,[X]
	mov	ebx,24*65536+24
	mul	ebx
	xchg	eax,ebx
	add	ebx,(1*65536+21-24*65536+24)
	movzx	eax,[Y]
	mov	ecx,24*65536+24
	mul	ecx
	xchg	eax,ecx
	add	ecx,(21*65536+41-24*65536+24)
	movzx	eax,[Y]
	dec	al
	push	ebx
	mov	ebx,9
	mul	bl
	mov	bl,[X]
	add	al,bl
	dec	al
	pop	ebx
	mov	esi,Map
	add	esi,eax
	push	dword 0 ;не курсор
	bt	[flags],4
	jnc	@f
	mov	dword [esp],1 ;курсор
	btr	[flags],4
@@:	mov	edi,0
	mov	ebp,1
	call	out_numbers
	pop	eax
ret


out_numbers:
	push	ebx ecx esi
	shr	ebx,16
	inc	bx
	shl	ebx,16
	add	ebx,23
	shr	ecx,16
	inc	cx
	shl	ecx,16
	add	ecx,23
	mov	edx,Bckgrd_clr
	push	ebp
	dec	dword [esp+4*5]
	jnz	@f
	mov	edx,Square_clr
@@:	mcall	13
	pop	ebp
	pop	esi

	cmp	byte [esi],0
	je	.null
	cmp	byte [esi],9
	jbe	.changeable_number
	cmp	byte [esi],19
	jbe	.fixed_number
	jmp	.null
.end:
	inc	esi
	dec	ebp
	jnz	out_numbers
	test	edi,edi
	jz	@f
	sub	edi,9
	mov	ebp,9
	add	ebx,-9*24*65536-9*24
	add	ecx,24*65536+24
	jmp	out_numbers
  @@:
ret

.fixed_number:
	push	esi
	shr	ebx,16
	shr	ecx,16
	mov	dx,bx
	shl	edx,16
	mov	dx,cx
	add	edx,8*65536+4
	movzx	ebx,byte [esi]
	sub	ebx,10
	dec	ebx
	shl	ebx,4
	add	ebx,FONT
	mov	ecx,8*65536+16
	push	esi ebp edi
	mov	edi,Pltr.fx
	cmp	dword [esp+4*7],0
	jne	@f
	mov	edi,Pltr.fxk
@@:	mov	esi,1
	mov	ebp,0
	mcall	65
	pop	edi ebp esi
.1:	pop	esi ecx ebx
	add	ebx,24*65536+24
	jmp	.end

.null:
	pop	ecx ebx
	add	ebx,24*65536+24
	jmp	.end
.changeable_number:
	push	esi
	shr	ebx,16
	shr	ecx,16
	mov	dx,bx
	shl	edx,16
	mov	dx,cx
	add	edx,8*65536+4
	movzx	ebx,byte [esi]
	dec	ebx
	shl	ebx,4
	add	ebx,FONT
	mov	ecx,8*65536+16
	push	esi ebp edi
	mov	edi,Pltr.ch
	cmp	dword [esp+4*7],0
	jne	@f
	mov	edi,Pltr.chk
@@:	mov	esi,1
	mov	ebp,0
	mcall	65
	pop	edi ebp esi
	jmp	.1



GeneratePlayBoard:
;i db 0
;j db 0
;RandI db 0
;RandJ db 0
;iRet db 0
;//генерируем решенную матрицу
;m:
;for i:=0 to 8 do
;  for j:=0 to 8 do
;    begin
;    Map[i,j]:=0;
;    SolveMap[i,j]:=0;
;    RealMap[i,j]:=0;
;    end;
	mov	edi,Map
	mov	esi,SolveMap
	mov	edx,RealMap
	xor	ecx,ecx
	@@:
	mov	byte [edi+ecx],0
	mov	byte [esi+ecx],0
	mov	byte [edx+ecx],0
	inc	ecx
	cmp	ecx,9*9
	jb	@b

;//ставим рандомно несколько чисел на поле
;for i:=1 to 21 do
;  begin
;  RandI:=random(9);
;  RandJ:=random(9);
;  if SolveMap[RandI,RandJ]=0 then
;     begin
;     SolveMap[RandI,RandJ]:=random(9)+1;
;     if not CheckSudoku(SolveMap) then
;       begin
;       SolveMap[RandI,RandJ]:=0;
;       Continue;
;       end;
;     end else Continue;
;  end;

	mov	ecx,21
.1:	mov	eax,9
	call	random
	mov	ebx,eax
	mov	eax,9
	call	random
	mov	ah,9
	mul	ah
	add	eax,ebx ;RandI,RandJ
	cmp	byte [esi+eax],0
	jne	.loop
		mov	ebx,eax
		mov	eax,9
		call	random
		mov	byte [esi+ebx],al
		call	CheckSudoku
		jnc	.loop
		mov	byte [esi+ebx],0
	.loop:
	loop	.1


;//решаем Судоку
;iRet:=Solve(SolveMap);
;if iRet<>1 then goto m;
;i:=1;

	mov	esi,SolveMap
	call	Solve
	cmp	[_iRet],1
	jne	GeneratePlayBoard

	movzx	ecx,[Difficult]
	movzx	ecx,byte [Difficult_array+ecx]

;case Difficult of
;1:
;   while i<=42 do
;   begin
;        RandI:=random(9);
;        RandJ:=random(9);
;        if RealMap[RandI,RandJ]<>0 then Continue else
;        RealMap[RandI,RandJ]:=SolveMap[RandI,RandJ];
;        inc(i);
;   end;
;2:
;   while i<=32 do
;   begin
;        RandI:=random(9);
;        RandJ:=random(9);
;        if RealMap[RandI,RandJ]<>0 then Continue else
;        RealMap[RandI,RandJ]:=SolveMap[RandI,RandJ];
;        inc(i);
;   end;
;3:
;   while i<=25 do
;   begin
;        RandI:=random(9);
;        RandJ:=random(9);
;        if RealMap[RandI,RandJ]<>0 then Continue else
;        RealMap[RandI,RandJ]:=SolveMap[RandI,RandJ];
;        inc(i);
;   end;
;end;

.2:
	mov	eax,9
	call	random
	mov	ebx,eax
	mov	eax,9
	call	random
	mov	ah,9
	mul	ah
	cmp	al,81
	jb	@f
	dec	al
	@@:
	add	eax,ebx ;RandI,RandJ
	cmp	byte [RealMap+eax],0
	jne	.loop2
		add	byte [SolveMap+eax],10
		mov	bl,[SolveMap+eax]
		mov	byte [RealMap+eax],bl
	.loop2:
	loop	.2

;for i:=0 to 8 do
;   for j:=0 to 8 do
;      Map[i,j]:=RealMap[i,j];
;end;

	xor	ecx,ecx
@@:	mov	al,[RealMap+ecx]
	mov	[Map+ecx],al
	inc	ecx
	cmp	ecx,9*9
	jb	@b
ret




align 4
rsx1 dw ?;0x4321
rsx2 dw ?;0x1234
rsx3 dw ?;0x62e9
rsx4 dw ?;0x3619
random: 	; из ASCL
	push ecx ebx esi edx
	mov cx,ax
	mov ax,[rsx1]
	mov bx,[rsx2]
	mov si,ax
	mov di,bx
	mov dl,ah
	mov ah,al
	mov al,bh
	mov bh,bl
	xor bl,bl
	rcr dl,1
	rcr ax,1
	rcr bx,1
	add bx,di
	adc ax,si
	add bx,[rsx3]
	adc ax,[rsx4]
	sub [rsx3],di
	adc [rsx4],si
	mov [rsx1],bx
	mov [rsx2],ax
	xor dx,dx
	cmp ax,0
	je nodiv
	cmp cx,0
	je nodiv
	div cx
nodiv:
	mov ax,dx
	pop edx esi ebx ecx
	and eax,0000ffffh
ret



if DEBUG
SysMsgBoardNum: ;warning: destroys eax,ebx,ecx,esi
	mov	ebx,esp
	mov	ecx,8
	mov	esi,(number_to_out+1)
.1:
	mov	eax,ebx
	and	eax,0xF
	add	al,'0'
	cmp	al,(10+'0')
	jb	@f
	add	al,('A'-'0'-10)
@@:
	mov	[esi+ecx],al
	shr	ebx,4
	loop	.1
	dec	esi
	mcall	71,1,number_to_out
ret

number_to_out	db '0x00000000',13,10,0
endf



window_load_save:
	popad
	mcall	51,1,.thread,(threat_stack+32*4)
	pushad
	ret
.thread:
	bts	[flags],6
.red:
	mcall	12,1
	mov	edi,txt.load
	bt	[flags],8
	jc	@f
	mov	edi,txt.save
	bt	[flags],10
	jnc	@f
	mov	edi,txt.save_solve
@@:	mcall	0,50*65536+300,0x70*65536+60,(0x34000000+Bckgrd_clr),,
	mcall	8,<245,43>,<2,14>,100,0xaaaaaa
	mcall	4,<252,5>,(0x80000000+Text_clr),txt.enter
	push	dword edit1
	call	[edit_box_draw]
	mcall	12,2
.still:
	mcall	10
	dec	al
	jz	.red
	dec	al
	jz	.key
	dec	al
	jz	.button
	jmp	.still
.button:
	mcall	17,1
	cmp	ah,1
	jne	@f
  .end: btr	[flags],6
	mcall	-1
  @@:	cmp	ah,100
	jne	.still
	bt	[flags],8
	jc	.ld
  .sv:	call	save_sudoku
	jnc	.end
	jmp	.err
  .ld:	call	load_sudoku
	jnc	.end
  .err: mcall	4,<5,19>,(0x80000000+Text_clr),txt.error
	jmp	.still


.key:
	mcall	2
	cmp	ah,13
	jne	@f
	bt	[flags],8
	jc	.ld
	jmp	.sv
   @@:	cmp	ah,27
	je	.end
	push	dword edit1
	call	[edit_box_key]
	jmp	.still


save_sudoku:
	mov	[_size],9*(9+2)
	mcall	68,12,[_size]
	test	eax,eax
	jnz	@f
	stc
	ret

@@:	mov	[_buffer],eax
	mov	edx,Map
	bt	[flags],10
	jnc	@f
	mov	edx,SolveMap
@@:	mov	esi,eax;[_buffer]
	mov	ecx,[_size]
	xor	edi,edi
	dec	edi
	mov	ebx,9
.1:	test	ecx,ecx
	jz	.end
	test	ebx,ebx
	jz	.2
	inc	edi
	dec	ebx
	mov	al,[edx+edi]
	test	al,al
	jnz	@f
	mov	byte [esi+edi],0x23
	loop	.1
	jmp	.end
@@:	cmp	al,9
	jbe	@f
	sub	al,10
@@:	add	al,48
	mov	byte [esi+edi],al
	loop	.1
	jmp	.end
.2:	mov	ebx,9
	mov	byte [esi+edi+1],13
	mov	byte [esi+edi+2],10
	add	esi,2
	sub	ecx,2
	jmp	.1


.end:	mov	[func_70.func_n],2
	push	[_size]
	pop	[func_70.param3]
	push	[_buffer]
	pop	[func_70.param4]
	mov	[func_70.name],buf_cmd_lin
	mcall	70,func_70
	cmp	al,0			;сохранён удачно?
	je	@f
	mcall	68,13,[_buffer]
	stc
	ret
@@:	mcall	68,13,[_buffer]
	clc
ret

load_sudoku:
	mov	[func_70.func_n],5
	mov	[func_70.param3],0
	mov	[func_70.param4],bufferfinfo
	mov	[func_70.name],buf_cmd_lin
	mcall	70,func_70
	test	al,al		;файл найден?
	jz	@f
	stc
	ret
  @@:	mov	eax, dword [bufferfinfo+32]	;копируем размер файла
	cmp	eax,81
	jge	@f
	stc
	ret
@@:	cmp	eax,100
	jb	@f
	stc
	ret

_size dd 0
_buffer dd 0

@@:	mov	[_size],eax
	mcall	68,12,[_size]
	test	eax,eax
	jnz	@f
	stc
	ret	;ошибка на выделение блока
  @@:
	mov	[_buffer],eax
	mov	[func_70.func_n],0
	mov	[func_70.name],buf_cmd_lin
	push	dword [_size]
	pop	dword [func_70.param3]
	push	dword [_buffer]
	pop	dword [func_70.param4]
	mcall	70,func_70
	test	eax,eax
	jz	@f
	stc
	ret	;ошибка чтения
@@:

	mov	edx,Map
	mov	esi,[_buffer]
	mov	ecx,[_size]
	xor	edi,edi
	dec	edi
.1:	test	ecx,ecx
	jz	.end
	inc	edi
	mov	al,[esi+edi]
	cmp	al,0x23
	jne	@f
	mov	byte [edx+edi],0
	loop	.1
	jmp	.end
@@:	cmp	al,13
	jne	@f
	add	esi,2
	dec	edi
	sub	ecx,2
	jmp	.1
@@:	sub	al,48
	mov	byte [edx+edi],al
	loop	.1
.end:	mcall	68,13,[_buffer]

	xor	ecx,ecx
@@:	mov	byte [SolveMap+ecx],0
	inc	ecx
	cmp	ecx,9*9
	jb	@b

	mov	[Ticks],0
	mcall	26,9
	mov	[Ticks_new],eax

	clc
ret




align 4
myimport:
edit_box_draw	dd	aEdit_box_draw
edit_box_key	dd	aEdit_box_key
edit_box_mouse	dd	aEdit_box_mouse
version_ed	dd	aVersion_ed
		dd	0
		dd	0
aEdit_box_draw	db 'edit_box',0
aEdit_box_key	db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed	db 'version_ed',0

edit1 edit_box 240,2,2,Bckgrd_clr,0x6a9480,0,0xAABBCC,0,4096,buf_cmd_lin,ed_focus,2,0,0

func_70:
 .func_n dd ?
 .param1 dd 0
 .param2 dd 0
 .param3 dd ?
 .param4 dd ?
 .rezerv db 0
 .name dd ?

if lang eq ru
title db 'Судоку',0
txt:
.dif db "Сложность (+/-):",0
.new db 'Новая (N)',0
.space db 'Решение (Пробел)',0
.check db 'Проверить (C)',0
.check_yes db		'Решение найдено',0
.check_no = .nosolve
.time db "Время (T)   :",0
.own_map db 'Ввести свою Судоку для решения (I)',0
.nosolve db		'Не решено         ',0
.solve db 'Решить (R)',0
.save db 'Сохранить (S)',0
.save_solve db 'Сохранить решение (A)',0
.error db 'Ошибка',0
.load db 'Загрузить (L)',0
.enter db 'Enter',0
else
title db 'Sudoku',0
txt:
.dif db "Difficult (+/-)",0
.new db 'New (N)',0
.space db 'Solution (Space)',0
.check db 'Check (C)',0
.check_yes db		'Right            ',0
.check_no db		'Not right        ',0
.time db " Time (T)   :",0
.own_map db 'Input your own Sudoku (I)',0
.nosolve db		'It is not solved',0
.solve db 'Solve (R)',0
.save db 'Save (S)',0
.save_solve db 'Save solution (A)',0
.error db 'Error';,0
.load db 'Load (L)',0
.enter db 'Enter',0
endf

system_path db '/sys/lib/'
boxlib_name db 'box_lib.obj',0

X db 1
Y db 1

Pltr:
.ch dd Bckgrd_clr,Chg_nmb_clr
.chk dd Square_clr,Chg_nmb_clr
.fx dd Bckgrd_clr,Fix_nmb_clr
.fxk dd Square_clr,Fix_nmb_clr

align 4
FONT file "SUDOKU.FNT"
;Палитры:


I_END:
align 16
Map	rb 9*9
SolveMap rb 9*9
RealMap rb 9*9
TempMap rb 9*9

cur_dir_path	rb 4096
buf_cmd_lin	rb 4096
bufferfinfo	rb 40
Ticks_new rd 1	;dd 0
Ticks rd 1	;dd 0
flags rw 1
threat_stack rd 32	;: times 32 dd 0

D_END:
;бит 0: см. перед draw_pole
;1: 1-таймер включён
;2: в draw_pole и key
;3: 1-показать решённую карту
;4: in draw_one_symbol
;5: in Timer
;6: 1 на экране окно Сохранения/Загрузки
;7: box_lib is not loaded
;8: 0 - сохранить 1 - загрузить
;9: 1 - увеличить высоту окна
;10: 1 - сохранить решение
;15 1 - проверка при нажатии клавиши С (для CheckSudoku)