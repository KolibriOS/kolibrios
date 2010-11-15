; v. 0.2
; last update:  19/09/2010
; written by:   Marat Zakiyanov aka Mario79, aka Mario
; changes:      select path with OpenDialog
;               show 2 different path for Info and SMART files
;               draw number of page
;---------------------------------------------------------------------
; v. 0.11: 15.09.2009 staper@inbox.ru
;---------------------------------------------------------------------
; Макросы load_lib.mac, editbox_ex и библиотеку box_lib.obj создали:
; <Lrz> - Alexey Teplov / Алексей Теплов
; Mario79, Mario - Marat Zakiyanov / Марат Закиянов
; Diamondz - Evgeny Grechnikov / Евгений Гречников и др.
;---------------------------------------------------------------------
use32
	org	0

	db	'MENUET01'
	dd	1
	dd	START
	dd	IM_END
	dd	I_END		;(i_end+200) and not 3
	dd	stacktop	;(i_end+200) and not 3
	dd	0x0		;buf_cmd_lin
	dd	cur_dir_path

PRIMARY_CHANNEL equ 0x1f7
SECONDARY_CHANNEL equ 0x177
; Ручной поиск портов по шине PCI (преимущественно для SATA):
; mcall   21,12,1 ;разрешить низкоуровневый доступ к PCI
; mcall   62,0x0006,((((0x1f shl 3) + 01) shl 8) + 0x10) ;см. ф-ю 62
; ax = 1 - стандартные порты (константы выше), или нестандартные (168-16f,1e8-1ef)
; Пример:
;  Fnc 02: 10-0x1c01,14-0x18f5,18-0x18f9,1c-0x18f1,BMA-0x1811,00000000
;  Fnc 01: 10-1,14-1,18-1,1c-1,BMA-0x18e1,0x18d1
; Регистр по смещению 0х10 для функции 02 содержит 0x1c01 (порты 0х1с00-0х1с07)

; Режимы Legacy, Native и пр. меняются через BIOS.
include '../../../config.inc'		;for nightbuild
include '..\..\..\macros.inc'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include '../../../develop/libraries/box_lib/load_lib.mac'
	@use_library
;---------------------------------------------------------------------
macro ab {
	add	ebx,455*65536
	}
;---------------------------------------------------------------------
macro sb {
	sub	ebx,455*65536-12
	}
;---------------------------------------------------------------------
macro ad {
	add	ebx,12
	}
;---------------------------------------------------------------------
macro wba num,text {
@@:
	bt	di,num
	jnc	@f
	ad
	mcall	4,,0x80000000,text
	}
;---------------------------------------------------------------------
macro wba num,text {
@@:
	bt	di,num
	jnc	@f
	ad
	mcall	4,,0x80000000,text
	}
;---------------------------------------------------------------------
macro sc num,text {
	cmp	al,num
	jne	@f
	mov	edx,text
@@:
	}
;---------------------------------------------------------------------
macro mz text,str1,str2,str3 {
	mcall	4,,0x80444444,text
	ab
	mov	edx,ebx
	push	ebx
	str1
	str2
	str3
	mcall	47,0x800a0000,,,0x0
	pop	ebx
	sb
	}
;---------------------------------------------------------------------
START:
;	load_library	boxlib_name,cur_dir_path,buf_cmd_lin,system_path,\
;	err_message_found_lib,head_f_l,Box_lib_import,err_message_import,head_f_i
	mcall	68,11

load_libraries l_libs_start,end_l_libs

	cmp	eax,-1
	jz	close
;---------------------------------------------------------------------
	mov	edi,filename_area
	mov	esi,default_Info+5	;default.info
	call	copy_str_1

	mov	edi,filename_area2
	mov	esi,default_SMART+5	;default.smart
	call	copy_str_1

	mov	edi,fname_Info
	mov	esi,default_Info	;/sys/default.info
	call	copy_str_1

	mov	edi,fname_SMART	;/sys/default.smart
;	cmp     byte [edi], 0
;	jne	skin_path_ready
	mov	esi,default_SMART
	call	copy_str_1
;skin_path_ready:	
;---------------------------------------------------------------------
;OpenDialog	initialisation
	push    dword OpenDialog_data
	call    [OpenDialog_Init]

	push    dword OpenDialog_data2
	call    [OpenDialog_Init]
	
; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]
	
	push	dword PathShow_data_2
	call	[PathShow_prepare]
;---------------------------------------------------------------------

	mcall	40,0x27
;---------------------------------------------------------------------
redraw_all:
redraw:
	call	draw_window
;---------------------------------------------------------------------
still:
	mcall	10
	dec	al
	jz	redraw_all
	dec	al
	jz	key
	dec	al
	jz	button
;---------------------------------------------------------------------
mouse:
	bt	[flags],2
	jnc	still
	mcall	37,2
	test	al,al
	jz	still
	push	dword Option_boxs
	call	[option_box_mouse]
	jmp	still
;---------------------------------------------------------------------
copy_str_1:
	xor	eax,eax
	cld
@@:
	lodsb
	stosb
	test	eax,eax
	jnz	@b
	ret
;---------------------------------------------------------------------
draw_PathShow:
	pusha
	mcall	13,<125,420>,<83,15>,0xFFFFED
	mcall	13,,<100,15>,
; draw for PathShow
	push	dword PathShow_data_1
	call	[PathShow_draw]
	
	push	dword PathShow_data_2
	call	[PathShow_draw]
	popa
	ret
;---------------------------------------------------------------------
draw_window:
	mcall	12,1
	xor	esi,esi
	mcall	0,<100,580>,<100,350>,0x34ffffff,,title
	mcall	8,<1,30>,<1,15>,2,0x365732
	mcall	,<35,38>,,3,
;        mcall   ,<77,38>,,6,
	mcall	,<120,45>,,7,

	mcall	,<195,20>,,4,	;влево,вправо
	mcall	,<217,20>,,5,
	mcall	4,<4,6>,0x80ffffff,menu_text

	mcall	,<260,6>,0x80000000,page_text
	movzx	ecx, byte [page_num]
	mcall	47,0x800a0000,,<300,6>,0x0

	mcall	38,<2,535>,<20,20>,0x00aabbaa
	bt	[flags],2
	jnc	@f
	call	show_ControlBlock
	mcall	4,<4,125>,0x80FF0000,[error_text]
	jmp	.end
@@:
	bt	[flags],6
	jnc	@f
	call	show_TestBlock
	jmp	.end
@@:
	bt	[flags],7
	jnc	@f
	call	show_InfoBlock
	jmp	.end
@@:
	bt	[flags],8
	jnc	.end
	call	show_SmartBlock
.end:
	mcall	12,2
	ret
;---------------------------------------------------------------------
key:
	mcall	2
;	push	edit1
;	call	[edit_box_key]
;	push	edit1
;	call	[edit_box_draw]
	jmp	still
;---------------------------------------------------------------------
button:
	mcall	17
	cmp	ah,1
	jne	@f
close:
	mcall	-1
@@:
	cmp	ah,2
	jne	@f
	call	get_InfoBlock
	mov	ax,[flags]
	btr	ax,2
	btr	ax,6
	bts	ax,7
	btr	ax,8
	mov	[flags],ax
	mov	[page_num],1
	jmp	redraw
;---------------------------------------------------------------------
@@:
	cmp	ah,3
	jne	@f
	call	get_SmartBlock
	mov	[page_num],1
	mov	ax,[flags]
	btr	ax,2
	btr	ax,6
	btr	ax,7
	bts	ax,8
	mov	[flags],ax
	jmp	redraw
;---------------------------------------------------------------------
@@:
	cmp	ah,4
	jne	@f
	cmp	[page_num],1
	je	still
	dec	[page_num]
	jmp	redraw
;---------------------------------------------------------------------
@@:
	cmp	ah,5
	jne	@f
	cmp	[page_num],5
	je	still
	inc	[page_num]
	jmp	redraw
;---------------------------------------------------------------------
@@:
	cmp	ah,6
	jne	@f
	mov	ax,[flags]
	btr	ax,2
	btr	ax,6
	btr	ax,7
	btr	ax,8
	mov	[flags],ax
	jmp	redraw
;---------------------------------------------------------------------
@@:
	cmp	ah,7
	jne	@f
	mov	ax,[flags]
	bts	ax,2
	btr	ax,6
	btr	ax,7
	btr	ax,8
	mov	[flags],ax
@@:
	cmp	ah,8
	jne	@f
	bts	[flags],9
	call	get_SmartBlock
	jmp	still
;---------------------------------------------------------------------
@@:
	cmp	ah,9	; Save Info
	jne	@f
; invoke OpenDialog
	mov	[OpenDialog_data.type],dword 1
	push    dword OpenDialog_data
	call    [OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	jne	still
; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]
	call	draw_PathShow

	btr	[flags],4
	jmp	save_file
;---------------------------------------------------------------------
@@:
	cmp	ah,10	; Load Info
	jne	@f
; invoke OpenDialog
	mov	[OpenDialog_data.type],dword 0
	push    dword OpenDialog_data
	call    [OpenDialog_Start]
	cmp	[OpenDialog_data.status],1
	jne	still
; prepare for PathShow
	push	dword PathShow_data_1
	call	[PathShow_prepare]
	call	draw_PathShow

	btr	[flags],4
	jmp	open_file
;---------------------------------------------------------------------
@@:
	cmp	ah,11	; Save SMART
	jne	@f
; invoke OpenDialog
	mov	[OpenDialog_data2.type],dword 1
	push    dword OpenDialog_data2
	call    [OpenDialog_Start]
	cmp	[OpenDialog_data2.status],1
	jne	still
; prepare for PathShow
	push	dword PathShow_data_2
	call	[PathShow_prepare]
	call	draw_PathShow

	bts	[flags],4
	jmp	save_file
;---------------------------------------------------------------------
@@:
	cmp	ah,12	; Load SMART
	jne	redraw
; invoke OpenDialog
	mov	[OpenDialog_data2.type],dword 0
	push    dword OpenDialog_data2
	call    [OpenDialog_Start]
	cmp	[OpenDialog_data2.status],1
	jne	still
; prepare for PathShow
	push	dword PathShow_data_2
	call	[PathShow_prepare]
	call	draw_PathShow

	bts	[flags],4
	jmp	open_file
;---------------------------------------------------------------------
open_file:
	mov	[func_70.func_n],0
	mov	[func_70.param3],512
	mov	[func_70.param4],InfoArray
	mov	[func_70.name],filename_area
	bt	[flags],4
	jnc	@f
	mov	[func_70.param4],SmartArray
	mov	[func_70.param3],1024
	mov	[func_70.name],filename_area2
@@:
	mcall	70,func_70
	test	al,al		;файл найден?
	jnz	.1
	mov	ax,[flags]
	bts	ax,0
	btr	ax,7
	btr	ax,8
	btr	ax,6
	btr	ax,2
	bt	ax,4
	jc     @f
	bts	ax,7
	mov	[flags],ax
	jmp	good
@@:
	bts	ax,8
	mov	[flags],ax
	jmp	redraw_all
.1:
	bt	[flags],4
	jnc	@f
	mov	[error_text],error_open_file_string_SMART
	jmp	redraw_all
@@:
	mov	[error_text],error_open_file_string_Info
	jmp	redraw_all
;---------------------------------------------------------------------
save_file:			;сохраняем файл
	mov	[func_70.func_n],2
	mov	[func_70.param3],512
	mov	[func_70.param4],InfoArray
	mov	[func_70.name],filename_area
	bt	[flags],4
	jnc	@f
	mov	[func_70.param4],SmartArray
	mov	[func_70.param3],1024
	mov	[func_70.name],filename_area2
@@:
	mcall	70,func_70
	test	al,al			 ;сохранён удачно?
	jz	good
	bt	[flags],4
	jnc	@f
	mov	[error_text],error_save_file_string_SMART
	jmp	redraw_all
@@:
	mov	[error_text],error_save_file_string_Info
	jmp	redraw_all
;---------------------------------------------------------------------
good:
	mov	[error_text],no_error_text
	jmp	redraw_all
;---------------------------------------------------------------------
ports_:
	mov	eax,[option_group1]
	cmp	eax,op1
	jne	@f
	mov	edx,PRIMARY_CHANNEL
	btr	[flags],5
	jmp	.0
;---------------------------------------------------------------------
@@:
	cmp	eax,op2
	jne	@f
	mov	edx,PRIMARY_CHANNEL
	bts	[flags],5
	jmp	.0
;---------------------------------------------------------------------
@@:
	cmp	eax,op3
	jbe	@f
	mov	edx,SECONDARY_CHANNEL
	btr	[flags],5
	jmp	.0
;---------------------------------------------------------------------
@@:
	mov	edx,SECONDARY_CHANNEL
	bts	[flags],5
.0:
	mov	ecx,edx
	sub	ecx,7
	mcall	46,0
	ret
;---------------------------------------------------------------------
get_InfoBlock:
	call	ports_
	push	ecx edx
	xor	ecx,ecx
.1:
	in	al,dx		;Проверить готовность
	dec	cx		;необходимо проверять и бит 7, однако работает ...
	jz	.error
	bt	ax,6
	jnc	.1

	dec	dx
	in	al,dx
	mov	bx,ax	   ;сохраняем бит выбора устройтсва
	btr	ax,4
	bt	[flags],5
	jnc	@f
	bts	ax,4
@@:
	out	dx,al
	inc	dx
	mov	al,0xec
	out	dx,al

	push	bx
	mcall	5,10
	pop	bx
	xor	cx,cx
.2:
	in	al,dx		;Проверить готовность
	dec	cx
	jz	.error
	bt	ax,6
	jnc	.2

	mov	esi,InfoArray	;Получить информацию
	xor	edi,edi
	sub	dx,7
.3:
	in	ax,dx
	xchg	al,ah
	mov	[esi+edi*2],ax
	inc	edi
	cmp	edi,256
	jne	.3
	bts	[flags],0
	btr	[flags],2
	btr	[flags],3

	push	bx
	mcall	5,10
	pop	bx

	mov	ax,bx
	add	dx,6
	out	dx,al
	pop	edx ecx
	mcall	46,1
	ret
;---------------------------------------------------------------------
.error:
	bts	[flags],3
	pop	edx ecx
	mcall	46,1
	ret
;---------------------------------------------------------------------
get_SmartBlock:
	call	ports_
	push	ecx edx
	xor	ecx,ecx
.1:
	in	al,dx		;Проверить готовность
	dec	cx
	jz	.error
	bt	ax,6
	jnc	.1

	dec	dx
	in	al,dx
	mov	cx,ax	;сохраняем бит выбора устройтсва
	btr	ax,4
	bt	[flags],5
	jnc	@f
	bts	ax,4
@@:
	out	dx,al
	inc	dx

	bt	[flags],9 ;S.M.A.R.T. on/off
	jnc	@f
	sub	dx,3
	mov	al,0x4f
	out	dx,al
	inc	dx
	mov	al,0xc2
	out	dx,al
	sub	dx,4
	mov	al,0xd8
	btc	[flags],10
	jnc	.2
	inc	al
.2:
	out	dx,al
	add	dx,6
	mov	al,0xb0
	out	dx,al
	dec	dx
	mov	ax,cx
	out	dx,ax
	btr	[flags],9
	btr	[flags],1
	jmp	.end
;---------------------------------------------------------------------
@@:
	sub	dx,3
	mov	al,0x4f
	out	dx,al
	inc	dx
	mov	al,0xc2
	out	dx,al
	sub	dx,4
	mov	al,0xd0
	out	dx,al
	add	dx,6
	mov	al,0xb0
	out	dx,al

	mcall	5,10

	mov	esi,SmartArray	 ;Получить информацию
	xor	edi,edi
	sub	dx,7
.3:
	in	ax,dx
	xchg	al,ah
	mov	[esi+edi*2],ax
	inc	edi
	cmp	edi,256
	jne	.3

	mcall	5,10

	add	dx,4
	mov	al,0x4f
	out	dx,al
	inc	dx
	mov	al,0xc2
	out	dx,al
	sub	dx,4
	mov	al,0xd1
	out	dx,al
	add	dx,6
	mov	al,0xb0
	out	dx,al

	mcall	5,10

	sub	dx,7
.4:
	in	ax,dx
	xchg	al,ah
	mov	[esi+edi*2],ax
	inc	edi
	cmp	edi,256*2
	jne	.4

	bts	[flags],1
	btr	[flags],3
	mov	ax,cx
	add	dx,6
	out	dx,al
.end:
	pop	edx ecx
	mcall	46,1
	ret
;---------------------------------------------------------------------
.error:
	bts	[flags],3
	pop	edx ecx
	mcall	46,1
	ret
;---------------------------------------------------------------------
show_ControlBlock:
	mcall	4,<4,30>,0x80000000,ctrl_text1
	mcall	,<4,45>,,ctrl_text2
	push	dword Option_boxs
	call	[option_box_draw]
;	push	dword edit1
;	call	[edit_box_draw]
	call	draw_PathShow

	mcall	8,<3,80>,<60,15>,8,0xf0f0f0
	mcall	,<50,30>,<82,15>,9,	; Save Info
	mcall	,<90,30>,,10,		; Load Info
	mcall	,<50,30>,<100,15>,11,	; Save SMART
	mcall	,<90,30>,,12,		; Load SMART
	mcall	4,<4,87>,0x80000000,edbx_text1
	mcall	,<7,64>,,smrt_text1
	mcall	,<4,105>,,edbx_text2
show_TestBlock:
	ret
;---------------------------------------------------------------------
show_InfoBlock:
	bt	[flags],0
	jnc	.end
	bt	[flags],3
	jc	.end
	mov	al,[page_num]
	dec	al
	jz	.page1
	dec	al
	jz	.page2
	dec	al
	je	.page3
	dec	al
	jz	.page4
	dec	al
	jz	.page5
.end:
	ret
;---------------------------------------------------------------------
.page1:
;TABLE 1 of 11
	mcall	4,5*65536+25,0x80444444,t.10_19
	ab	;        add     ebx,300*65536
	mcall	,,0x0,(InfoArray+10*2),((19-10)+1)*2
	sb	;        sub     ebx,300*65536-12
	mcall	,,0x80444444,t.23_26
	ab
	mcall	,,0x0,(InfoArray+23*2),((26-23)+1)*2
	sb
	mcall	,,0x80444444,t.27_46
	ab
	mcall	,,0x0,(InfoArray+27*2),((46-27)+1)*2
	sb
	mcall	,,0x80444444,t.type
	ab
	mov	di,[InfoArray+222*2]
@@:
	bt	di,4
	jnc	 @f
	mcall	4,,0x80000000,t.222.12.1
@@:
	bt	di,4
	jc	@f
	mcall	4,,0x80000000,t.222.12.0
@@:
	sb
	mcall	4,,0x80444444,t.60_61
	ab
	mov	edx,ebx
	push	ebx
	mov	ecx, [InfoArray+60*2]
	bswap	ecx
	ror	ecx,16
	mcall	47,0x800a0000,,,0x0
	shr	ecx,11	;LBA in MB
	add	edx,70*65536
	mcall	,,,,0x459a
	pop	ebx

	sb				;CHS
	mcall	4,,0x80444444,t.chs
	ab
	mov	edx,ebx
	push	ebx
	movzx	ecx, word [InfoArray+2]
	xchg	ch,cl
	mcall	47,0x80050000,,,0x0
	add	edx,35*65536
	movzx	ecx, word [InfoArray+6]
	xchg	ch,cl
	mcall	47,0x80040000,,,0x0
	add	edx,15*65536
	movzx	ecx, word [InfoArray+12]
	xchg	ch,cl
	mcall	;47,0x80040000,,,0x0
	add	edx,20*65536
	push	ebx edx
	xor	edx,edx
	movzx	eax,word[InfoArray+2]
	xchg	ah,al
	movzx	ebx, word [InfoArray+6]
	xchg	bh,bl
	mul	ebx
	mul	ecx
	xchg	eax,ecx
	pop	edx ebx
	shr	ecx,11
	mcall	47,0x800a0000,,,0x0
	pop	ebx

	mov	ax,[InfoArray+48*2]
	bt	ax,8
	jnc	@f
	sb
	mcall	4,,0x80444444,t.48.0
	ab
	mcall	,,0x80000000,t.sup
@@:
	mov	ax,[InfoArray+49*2]
	xchg	al,ah
	mov	di,ax
	bt	di,13
	jnc	@f
	sb
	mcall	4,,0x80444444,t.49.13.1
	ab
	mcall	,,0x80000000,t.sup
@@:
	bt	di,13
	jc	@f
	sb
	mcall	4,,0x80444444,t.49.13.0
	ab
@@:
	bt	di,11
	jnc	@f
	sb
	mcall	4,,0x80444444,t.49.11.1
	ab
	mcall	,,0x80000000,t.sup
@@:
	bt	di,11
	jc	@f
	sb
	mcall	4,,0x80444444,t.49.11.0
	ab
	mcall	,,0x80000000,t.sup
@@:
	bt	di,10
	jnc	@f
	sb
	mcall	4,,0x80444444,t.49.10.1
	ab
@@:
	bt	di,9
	jnc	@f
	sb
	mcall	4,,0x80444444,t.49.9
	ab
	mcall	,,0x80000000,t.sup
@@:
	bt	di,8
	jnc	@f
	sb
	mcall	4,,0x80444444,t.49.8
	ab
	mcall	,,0x80000000,t.sup
@@:
	sb
	mcall	4,,0x80444444,t.47
;        add     ebx,10
;        mcall   4,,0x80444444,t.47_
	ab
	mov	edx,ebx
	movzx	ecx, word [InfoArray+47*2]
	xchg	ch,cl
	mcall	47,0x80050000,,,0x80000000
	mov	ebx,edx


;PART 2 of 11
	sb
	mcall	4,,0x80444444,t.59
;        add     ebx,10
;        mcall   4,,0x80444444,t.59_
	ab
	mov	edx,ebx
	movzx	ecx, word [InfoArray+59*2]
	xchg	ch,cl
	mcall	47,0x80050000,,,0x0
	mov	ebx,edx

	mov	ax,[InfoArray+63*2]
	xchg	al,ah
	mov	di,ax
	bt	di,10
	jnc	@f
	mov	edx,ebx
;        push    ebx
	sb
	mcall	4,,0x80444444,t.63.10
	ab
	mcall	,,0x80000000,t.sel
@@:
	bt	di,9
	jnc	@f
	sb
	mcall	4,,0x80444444,t.63.9
	ab
	mcall	,,0x80000000,t.sel
@@:
	bt	di,8
	jnc	@f
	sb
	mcall	4,,0x80444444,t.63.8
	ab
	mcall	,,0x80000000,t.sel
@@:
	bt	di,2
	jnc	@f
	sb
	mcall	4,,0x80444444,t.63.2
	ab
	mcall	,,0x80000000,t.sup
@@:
	bt	di,1
	jnc	@f
	sb
	mcall	4,,0x80444444,t.63.1
	ab
	mcall	,,0x80000000,t.sup
@@:
	bt	di,0
	jnc	@f
	sb
	mcall	4,,0x80444444,t.63.0
	ab
	mcall	,,0x80000000,t.sup
@@:

;PART 3 of 11
	sb
	mcall	4,,0x80444444,t.75.0_4
	ab
	mov	edx,ebx
	push	ebx
	movzx	ecx, byte [InfoArray+75*2+1]
	mcall	47,0x800a0000,,,0x0
	pop	ebx

;words 76-79 - SATA
;.word80:
;        sb
;        mcall   4,,0x80444444,t.80
;        ab
;        mov     cx,[InfoArray+80*2]
;        test    cx,cx
;        jnz     @f
;        mcall   4,,0x80444444,t.unk
;        ret
;@@:     cmp     cx,-1
;        jne     @f
;        mcall   4,,0x80444444,t.unk
;        ret
;@@:     xchg    ch,cl
;        mov     edx,ebx
;        mcall   47,0x800a0000,,,0x0
;        mov     ebx,edx
	sb
	mcall	4,,0x80444444,t.80__
	mov	ax,[InfoArray+80*2]
	ab
	xchg	al,ah
	mov	di,ax
	bt	di,8
	jnc	@f
	mcall	4,,0x80000000,t.80.8
	jmp	.end
;---------------------------------------------------------------------
@@:
	bt	di,7
	jnc	@f
	mcall	4,,0x80000000,t.80.7
	jmp	.end
;---------------------------------------------------------------------
@@:
	bt	di,6
	jnc	@f
	mcall	4,,0x80000000,t.80.6
	jmp	.end
;---------------------------------------------------------------------
@@:
	bt	di,5
	jnc	@f
	mcall	4,,0x80000000,t.80.5
	jmp	.end
;---------------------------------------------------------------------
@@:
	bt	di,4
	jnc	@f
	mcall	4,,0x80000000,t.80.4
	jmp	.end
@@:
	ret
;---------------------------------------------------------------------
.page2:
;PART 4 of 11
	mcall	4,5*65536+25,0x80444444,t.82
	add	ebx,10*65536
	mov	ax,[InfoArray+82*2]
	xchg	al,ah
	mov	di,ax
	bt	di,14
	jnc	@f
	ad
	mcall	4,,0x80000000,t.82.14
	wba	13,t.82.13
	wba	12,t.82.12
	wba	10,t.82.10
	wba	9,t.82.9
	wba	8,t.82.8
	wba	7,t.82.7
	wba	6,t.82.6
	wba	5,t.82.5
	wba	4,t.82.4
	wba	3,t.82.3
	wba	1,t.82.1
	wba	0,t.82.0
@@:
	mov	ax,[InfoArray+83*2]
	xchg	al,ah
	mov	di,ax

	wba	13,t.83.13
	wba	12,t.83.12
	wba	11,t.83.11
	wba	10,t.83.10
	wba	9,t.83.9
	wba	8,t.83.8
	wba	6,t.83.6
	wba	5,t.83.5
	wba	3,t.83.3
	wba	2,t.83.2
	wba	1,t.83.1
	wba	0,t.83.0
@@:

;PART 5 of 11
	mov	ebx,290*65536+25
	mov	ax,[InfoArray+84*2]
	xchg	al,ah
	mov	di,ax
	wba	13,t.84.13
	wba	8,t.84.8
	wba	7,t.84.7
	wba	6,t.84.6
	wba	5,t.84.5
	wba	4,t.84.4
	wba	3,t.84.3
	wba	2,t.84.2
	wba	1,t.84.1
	wba	0,t.84.0
@@:
	ret
;---------------------------------------------------------------------
.page3:
	mcall	4,5*65536+25,0x80444444,t.85
	add	ebx,10*65536
	mov	ax,[InfoArray+85*2]
	xchg	al,ah
	mov	di,ax
	bt	di,14
	jnc	@f
	ad
	mcall	4,,0x80000000,t.85.14
	wba	13,t.85.13
	wba	12,t.85.12
	wba	10,t.85.10
;        ad
;        mcall   4,,0x80000000,t.85.10_
	wba	9,t.85.9
	wba	8,t.85.8
	wba	7,t.85.7
	wba	6,t.85.6
	wba	5,t.85.5
	wba	4,t.85.4
	wba	3,t.85.3
	wba	1,t.85.1
	wba	0,t.85.0
@@:
	mov	ax,[InfoArray+86*2]
	xchg	al,ah
	mov	di,ax
	bt	di,13
	jnc	@f
	ad
	mcall	4,,0x80000000,t.86.13
	wba	12,t.86.12
	wba	11,t.86.11
	wba	10,t.86.10
	wba	9,t.86.9
	wba	8,t.86.8
	wba	6,t.86.6
	wba	5,t.86.5
	wba	3,t.86.3
	wba	2,t.86.2
	wba	1,t.86.1
	wba	0,t.86.0
@@:
	ret
;---------------------------------------------------------------------
.page4:
	mov	ebx,5*65536+25
	mov	ax,[InfoArray+88*2]
	xchg	al,ah
	xor	di,di
	bt	ax,8
	jnc	@f
	mov	di,1
@@:
	bt	ax,9
	jnc	@f
	mov	di,2
@@:
	bt	ax,10
	jnc	@f
	mov	di,3
@@:
	bt	ax,11
	jnc	@f
	mov	di,4
@@:
	bt	ax,12
	jnc	@f
	mov	di,5
@@:
	bt	ax,13
	jnc	@f
	mov	di,6
@@:
	test	di,di
	jz	@f
	mcall	4,,0x80444444,t.88
	mov	edx,ebx
	push	ebx
	add	edx,90*65536
	movzx	ecx, word di
	dec	cl
	mcall	47,0x800a0000,,,0x0
	pop	ebx
	ad
@@:
	xor	di,di
	mov	ax,[InfoArray+88*2]
	xchg	al,ah
	bt	ax,0
	jnc	@f
	mov	di,1
@@:
	bt	ax,1
	jnc	@f
	mov	di,2
@@:
	bt	ax,2
	jnc	@f
	mov	di,3
@@:
	bt	ax,3
	jnc	@f
	mov	di,4
@@:
	bt	ax,4
	jnc	@f
	mov	di,5
@@:
	bt	ax,5
	jnc	@f
	mov	di,6
@@:
	bt	ax,6
	jnc	@f
	mov	di,7
@@:
	test	di,di
	jz	@f
	mcall	4,,0x80444444,t.88.1_6
	mov	edx,ebx
	push	ebx
	add	edx,90*65536
	movzx	ecx, word di
	dec	cl
	mcall	47,0x800a0000,,,0x0
	pop	ebx
	ad
@@:
	mz	t.89,<movzx   ecx, word [InfoArray+89*2]>,
	mz	t.90,<movzx   ecx, word [InfoArray+90*2]>,
	mz	t.91,<movzx   ecx, word [InfoArray+91*2]>,<xchg    ch,cl>
	mz	t.92,<movzx   ecx, word [InfoArray+92*2]>,<xchg    ch,cl>
;WORD 94
	mz	t.94.8_15,<movzx   ecx, byte [InfoArray+94*2+1]>,
	mz	t.94.0_7,<movzx   ecx, byte [InfoArray+94*2]>,
	mz	t.95,<movzx   ecx, word [InfoArray+95*2]>,<xchg    ch,cl>
	mz	t.96,<movzx   ecx, word [InfoArray+96*2]>,<xchg    ch,cl>
	mz	t.97,<movzx   ecx, word [InfoArray+97*2]>,<xchg    ch,cl>
	mz	t.98_99,<mov   cx,[InfoArray+98*2]>,<bswap ecx>,<mov   cx,[InfoArray+98*2+2]>
	mz	t.100_103,<mov	ecx,[InfoArray+100*2]>,<bswap ecx>,<ror ecx,16>
	mz	t.104,<movzx   ecx, word [InfoArray+104*2]>,

;PART 6 of 11 ?
	mcall	4,,0x80444444,t.106
	add	ebx,12
	mov	ax,[InfoArray+106*2]
	xchg	ah,al
	mov	di,ax
	bt	di,13
	jnc	@f
	mcall	4,,0x80444444,t.106.13
	add	ebx,15
@@:
	bt	di,12
	jnc	@f
	mcall	4,,0x80444444,t.106.12
	add	ebx,15
@@:
	mcall	4,,0x80444444,t.106.0_3
	ab
	shl	di,12
	shr	di,12
	movzx	ecx,di
	mov	edx,ebx
	mcall	47,0x800a0000,,,0x0
	mov	ebx,edx
	sb

	mz	t.107,<movzx	 ecx,word [InfoArray+107*2]>,
;108 - 111 добавить
	mz	t.117_118,<mov	   ecx,[InfoArray+117*2]>,
	ret
;---------------------------------------------------------------------
.page5:
;        mcall   4,5*65536+25,0x80444444,t.85
	ret
;---------------------------------------------------------------------
show_SmartBlock:
	bt	[flags],1
	jnc	.end
	bt	[flags],3
	jc	.end

	cmp	byte [SmartArray+2],0
	je	.end
	mcall	4,5*65536+25,0x80444444,s.title
	ad
	movzx	ax, byte [SmartArray+2]
	cmp	ax,30	;максимум 30 атрибутов
	jg	.end
	push	ax
	push	dword (3+SmartArray)
@@:
	xor	edx,edx
	call	.find
	push	edx

	mov	edx,ebx
	movzx	ecx,al
	mcall	47,0x80030000,,,0x0
	mov	ebx,edx
	add	ebx,30*65536
	pop	edx
	test	edx,edx
	jnz	.0
	mov	edx,s.0
  .0:	mov	eax,[esp]
	mov	al,[eax+2]
	mov	ecx,0x80000000
	and	al,1
	jz	.1
;       mov     ecx,0x80ff0000
  .1:	mcall	4,,,
	sub	ebx,30*65536

	mov	ecx,[esp]
	inc	ecx
	movzx	ecx, byte [ecx]
	add	ebx,400*65536
	mov	edx,ebx
	mcall	47,0x800a0000,,,0x0
	mov	ecx,[esp]
	add	ecx,4
	movzx	ecx, byte [ecx]
	add	edx,30*65536
	mcall	47,0x800a0000,,,0x0
	mov	ebx,edx
	sub	ebx,(30+400)*65536

	ad
	add	dword [esp],12
	dec	word [esp+4]
	jnz	@b
	add	esp,6
.end:
	ret
;---------------------------------------------------------------------
.find:
	mov	eax, dword [esp+4]
	mov	al,[eax]
	sc	1,s.1
	sc	2,s.2
	sc	3,s.3
	sc	4,s.4
	sc	5,s.5
	sc	6,s.6
	sc	7,s.7
	sc	8,s.8
	sc	9,s.9
	sc	10,s.10
	sc	11,s.11
	sc	12,s.12
	sc	13,s.13
	sc	190,s.190
	sc	191,s.191
	sc	192,s.192
	sc	193,s.193
	sc	194,s.194
	sc	195,s.195
	sc	196,s.196
	sc	197,s.197
	sc	198,s.198
	sc	199,s.199
	sc	200,s.200
	sc	201,s.201
	sc	202,s.202
	sc	203,s.203
	sc	204,s.204
	sc	205,s.205
	sc	206,s.206
	sc	207,s.207
	sc	208,s.208
	sc	209,s.209
	sc	220,s.220
	sc	221,s.221
	sc	222,s.222
	sc	223,s.223
	sc	224,s.224
	sc	225,s.225
	sc	226,s.226
	sc	227,s.227
	sc	228,s.228
	sc	230,s.230
	sc	231,s.231
	sc	240,s.240
	sc	250,s.250
	ret
;---------------------------------------------------------------------
;DATA AREA
t:
.sup	db 'supported',0
;.usp	db 'unsupported',0
.sel	db 'selected',0
;.usl	db 'not selected',0
.enb	db 'enabled',0
;.dis	db 'disabled',0
.unk	db 'unknown',0
.type	db 'Type',0
.chs	db 'CHS',0

.10_19	db 'Serial number',0
.23_26	db 'Firmware revision',0
.27_46	db 'Model number',0
.47	db 'Max. number of sectors that shall be transferred per DRQ data block',0
;.47	db 'Maximum number of logical sectors that shall be transferred per DRQ',0
;.47_	db 'data block on READ/WRITE MULTIPLE commands',0
.48.0	db 'Trusted Computing feature set is',0 ;1 sup
.49.13.1	db 'Standby timer values are',0 ;sup
.49.13.0	db 'Standby timer values shall be managed by the device',0
.49.11.1	db 'IORDY',0 ;sup
.49.11.0	db 'IORDY may be',0 ;sup
.49.10.1	db 'IORDY may be disabled',0
.49.9	db 'LBA',0 ;1 sup
.49.8	db 'DMA',0 ;1 sup

;.53  db 'Free-fall Control Sensitivity',0
;.53.6_15 db 'Sensitivity level',0
.59	db 'Current number of sectors that shall be transferred per DRQ data block',0
;.59	db 'Current setting for number of logical sectors that shall be',0
;.59_	db 'transferred per DRQ data block on READ/WRITE MULTIPLE commands',0
.60_61 db 'Total number of user addressable logical sectors',0
.63.10 db 'Multiword DMA mode 2 is',0 ;1 sel
.63.9	db 'Multiword DMA mode 1 is',0 ;1 sel
.63.8	db 'Multiword DMA mode 0 is',0 ;1 sel
.63.2	db 'Miltiword DMA mode 2 and below are',0 ;sup
.63.1	db 'Miltiword DMA mode 1 and below are',0 ;sup
.63.0	db 'Miltiword DMA mode 0 is',0 ;sup
.64.0_7  db 'PIO modes',0 ;sup

;.65 - .68 добавить

.75.0_4	db 'Maximum queue depth -1 ',0
.76	db 'Serial ATA Capabilities:',0
.76.10	db 'Supports Phy Event Counters',0 ;1
.76.9	db 'Supports receit of host initiated power management request',0 ;1
.76.8	db 'Supports native Command Queuing',0 ;1
.76.2	db 'Supports SATA Gen2 Signaling Speed (3.0Gb/s)',0
.76.1	db 'Supports SATA Gen1 Signaling Speed (1.5Gb/s)',0
.78	db 'SATA Features Supported',0
.78_	db 'Device supports:',0
.78.6	db 'Software Setting Preservation',0 ;1
.78.4	db 'in-order data delivery',0 ;1
.78.3	db 'initiating power management',0 ;1
.78.2	db 'DMA Setup auto-activation',0 ;1
.78.1	db 'non-zero buffer offsets',0 ;1
.79	db 'SATA Features Enabled:',0
.79.6 = .78.6;1 enb
.79.4 = .78.4;1 enb
.79.3 = .78.3;1 enb
.79.2 = .78.2;1 enb
.79.1 = .78.1;1 enb
.80	db 'Major revision number',0
.80__	db 'supports:',0
.80.8	db 'ATA8-ACS',0 ;1
.80.7	db 'ATA/ATAPI-7',0 ;1
.80.6	db 'ATA/ATAPI-6',0 ;1
.80.5	db 'ATA/ATAPI-5',0 ;1
.80.4	db 'ATA/ATAPI-4',0 ;1
.81	db 'Minor revision number',0

.82	db 'Command set/feature supported:',0
.82.14	db 'NOP command',0 ;1 sup
.82.13	db 'READ BUFFER command',0 ;1 sup
.82.12	db 'WRITE BUFFER command',0 ;1 sup
.82.10	db 'Host Protected Area feature set',0 ;1 sup
.82.9	db 'DEVICE RESET command',0 ;1 sup
.82.8	db 'SERVICE interrupt',0 ;1 sup
.82.7	db 'release interrupt',0 ;1 sup
.82.6	db 'read look-ahead',0 ;1 sup
.82.5	db 'volatile write cache',0 ;1 sup
.82.4	db 'PACKET feature set',0 ;1 sup
.82.3	db 'Mandatory Power Management feature set',0 ;1 sup
.82.1	db 'Security feature set',0 ;1 sup
.82.0	db 'SMART feature set',0 ;1 sup
;.83	db 'Command sets supported:',0
.83.13	db 'FLUSH CACHE EXT command',0 ;1 sup
.83.12	db 'Mandatory FLUSH CACHE command',0 ;1 sup
.83.11	db 'Device Configuration Overlay feature set',0 ;1 sup
.83.10	db '48-bit Address feature set',0 ;1 sup
.83.9	db 'Automatic Acoustic Management feature set',0 ;1 sup
.83.8	db 'SET MAX security extension',0 ;1 sup
.83.6	db 'SET FEATURES subcommand required to spin-up after power-up',0 ;1
.83.5	db 'Power-Up In Standby feature set',0 ;1 sup
.83.3	db 'Advanced Power Management feature set',0 ;1 sup
.83.2	db 'CFA feature set',0 ;1 sup
.83.1	db 'READ/WRITE DMA QUEUED',0 ;1 sup
.83.0	db 'DOWNLOAD MICROCODE command',0 ;1 sup

;.84	db 'Command set/feature supported:',0
.84.13	db 'IDLE IMMEDIATE with UNLOAD FEATURE',0 ;1 sup
.84.8	db '64-bit World wide name supported',0 ;1 sup
.84.7	db 'WRITE DMA QUEUED FUA EXT command',0 ;1 sup
.84.6	db 'WRITE DMA FUA EXT and WRITE MULTIPLE FUA EXT commands',0 ;1 sup
.84.5	db 'General Purpose Logging feature set',0 ;1 sup
.84.4	db 'Streaming feature set',0 ;1 sup
.84.3	db 'Media Card Pass Through Command feature set',0 ;1 sup
.84.2	db 'Media serial number',0 ;1 sup
.84.1	db 'SMART self-test',0 ;1 sup
.84.0	db 'SMART error logging',0 ;1 sup
.85	db 'Command set/feature enabled/supported:',0
.85.14 = .82.14 ;1 sup
.85.13 = .82.13 ;1 sup
.85.12 = .82.12 ;1 sup
.85.10	db 'Host Protected Area has been established',0;1 (i.e., the maximum LBA is less than the maximum native LBA)
.85.9 = .82.9; 1 sup
.85.8 = .82.8; 1 enb
.85.7 = .82.7; 1 enb
.85.6 = .82.6; 1 enb
.85.5 = .82.5; 1 enb
.85.4 = .82.4; 1 sup
.85.3 = .82.3; 1 sup
.85.1 = .82.1; 1 enb
.85.0 = .82.0; 1 enb

;.86   db 'Command set/feature enabled/supported:',0
.86.13 = .83.13; 1 sup
.86.12	db 'FLUSH CACHE command',0 ;1 sup
.86.11	db 'Device Configuration Overlay',0 ;1 sup
.86.10 = .83.10 ;1 sup
.86.9 = .83.9 ;1 enb
.86.8	db 'SET MAX security extension enabled by SET MAX SET PASSWORD',0 ;1
.86.6 = .83.6 ;1
.86.5 = .83.5 ;1 enb
.86.3 = .83.3 ;1 enb
.86.2 = .83.2 ;l sup
.86.1 = .83.1 ;1 sup
.86.0 = .83.0 ;1 sup
;.87   db 'Command set/feature enabled/supported:',0
;.87.x см..84

.88	db 'Ultra DMA mode   selected',0
;бит 14 установлен - mode 6 is selected, 13-5, 12-4, 11-3, 10-2, 9-1, 8-0
.88.1_6	db 'Ultra DMA mode   and below are supported',0 ;бит 1-1, ..., 6-6
;.88.0:  db 'Ultra DMA mode 0 is',0 ;1 sup
.89	db 'Time required for security erase unit completion',0
.90	db 'Time required for Enhanced security erase completion',0
.91	db 'Current advanced power management value',0
.92	db 'Master Password Identifier',0

;.93   db 'Hardware reset result',0 ;добавить
.94.8_15	db 'Vendor"s recommended acoustic management value',0
.94.0_7	db 'Current automatic acoustic management value',0
.95	db 'Stream Minimum Request Size',0
.96	db 'Streaming Transfer Time - DMA',0
.97	db 'Streaming Access Latency - DMA and PIO',0
.98_99	db 'Streaming Performance Granularity',0
.100_103	db 'Total Number of User Addressable Sectors for the 48-bit Address feature set',0
.104	db 'Streaming Transfer Time - PIO',0;

.106	db 'Physical sector size / Logical Sector Size:',0
.106.13	db 'Device has multiple logical sectors per physical sector',0 ;1
.106.12	db 'Device Logical Sector Longer than 256 Words',0 ;1
.106.0_3	db '2^X logical sectors per physical sector',0
.107	db 'Inter-seek delay for ISO-7779 acoustic testing in microseconds',0
.108.12_15	db 'NAA (3:0)',0
.108.0_11	db 'IEEE OUI (23:12)',0
.109.4_15	db 'IEEE OUI (11:0)',0
.109.0_3	db 'Unique ID (35:32)',0
.110	db 'Unique ID (31:16)',0
.111	db 'Unique ID (15:0)',0;
.117_118	db 'Words per Logical Sector',0
.119	db 'Supported Settings:',0
.119.5	db 'Free-fall Control feature set is',0 ;1 sup
.119.4	db 'The Segmented feature for DOWNLOAD MICROCODE is',0 ;1 sup
.119.3	db 'READ and WRITE DMA EXT GPL optional commands are',0 ;1 sup
.119.2	db 'WRITE UNCORRECTABLE EXT is',0 ;1 sup
.119.1	db 'Write-Read-Verify feature set is',0 ;1 sup
.120	db 'Command set/feature enabled/supported',0
.120.5	db 'Free-fall Control feature set is',0 ;1 enb
.120.4	db 'The Segmented feature for DOWNLOAD MICROCODE is',0 ;1 sup
.120.3	db 'READ and WRITE DMA EXT GPL optional commands are',0 ;1 sup
.120.2	db 'WRITE UNCORRECTABLE EXT is',0 ;1 sup
.120.1	db 'Write-Read-Verify feature set is',0 ;1 enb

.128	db 'Security status',0
.128.8.0	db 'Security level = High',0
.128.8.1	db 'Security level = Maximum',0
.128.5	db 'Enhanced security erase',0 ;1 sup
.128.4	db 'Security count expired',0 ;1
.128.3	db 'Security frozen',0 ;1
.128.2	db 'Security locked',0 ;1
.128.1	db 'Security enabled',0 ;1
.128.0	db 'Security supported',0 ;1

;.160 добавить

.176_205	db 'Current media serial number',0
.206	db 'SCT Command Transport:',0
.206.5	db 'Data Tables',0 ;1 sup
.206.4	db 'Features Control',0 ;1 sup
.206.3	db 'Error Recovery Control',0 ;1 sup
.206.2	db 'Write Same',0 ;1 sup
.206.1	db 'Long Sector Access',0 ;1 sup
.206.0	db '',0 ;1 sup
.209	db 'Alignment of logical blocks within a larger physical block',0
.209.0_13	db '"Logical sector" offset within the first physical sector where the first logical sector is placed.',0;
.210_211	db 'Write-Read-Verify Sector Count Mode 3 Only',0
.212_213	db 'Verify Sector Count Mode 2 Only',0
.214	db 'NV Cache Capabilities:',0
.214.12_15	db 'NV Cache feature set version',0
.214.8_11	db 'NV Cache Power Mode feature set version',0
.214.4	db 'NV Cache feature set',0 ;1 enb
.214.1	db 'NV Cache Power Mode feature set',0 ;1 enb
.214.0	db 'NV Cache Power Mode feature set',0 ;1 sup
.215	db 'NV Cache Size in Logical Blocks (15:0)',0

.216	db 'NV Cache Size in Logical Blocks (31:16)',0
.217	db 'Nominal media rotation rate',0
.219	db 'NV Cache Options',0
.219.0_7	db 'Device Estimated Time to Spin Up in Seconds',0
.220.0_7	db 'Write-Read-Verify feature set current mode',0;
.222	db 'Transport Major revision number',0
.222.12.0	db 'Parallel',0
.222.12.1	db 'Serial',0
;.222.0_11 добавить
.223	db 'Transport Major revision number',0
.234	db 'Minimum number of 512 byte units per DOWNLOAD MICROCODE command for mode 03h',0
.235	db 'Maximum number of 512 byte units per DOWNLOAD MICROCODE command for mode 03h',0
.255	db 'Integrity word',0
.255.8_15	db 'Checksum',0
.255.0_7	db 'Signature',0
;---------------------------------------------------------------------
s:
.title	db 'ID/Name/Value/Wrst',0
.0	db '?',0
.1	db 'Raw Read Error Rate',0
.2	db 'Throughput Performance',0
.3	db 'Spin Up Time',0
.4	db 'Start/Stop Count',0
.5	db 'Reallocated Sector Count',0
.6	db 'Read Channel Margin',0
.7	db 'Seek Error Rate',0
.8	db 'Seek Time Performance',0
.9	db 'Power-On Hours Count',0
.10	db 'Spin Retry Count',0
.11	db 'Recalibration Retries',0
.12	db 'Device Power Cycle Count',0
.13	db 'Soft Read Error Rate',0
;??	db 'Emergency Re-track (Hitachi)',0
;??	db 'ECC On-The-Fly Count (Hitachi)',0
;96	db '? (Maxtor)
;97	db '? (Maxtor)
;98	db '? (Maxtor)
;99	db '? (Maxtor)
;100	db '? (Maxtor)
;101	db '? (Maxtor)
.190	db 'Airflow Temperature (WDC)',0
.191	db 'G-Sense Error Rate',0
.192	db 'Power-Off Retract Cycle',0
.193	db 'Load/Unload Cycle Count',0
.194	db 'Temperature',0
.195	db 'Hardware ECC Recovered',0
.196	db 'Reallocation Events Count',0
.197	db 'Current Pending Sector Count',0
.198	db 'Uncorrectable Sector Count',0
.199	db 'UltraDMA CRC Error Rate',0
.200	db 'Write Error Rate (WD - MultiZone Error Rate)',0
.201	db 'TA Counter Detected (or Soft read error rate ?)',0
.202	db 'TA Counter Increased (or Data Address Mark errors ?)',0
.203	db 'Run out cancel',0
.204	db 'Soft ECC correction',0
.205	db 'Thermal asperity rate (TAR)',0
.206	db 'Flying height',0
.207	db 'Spin high',0
.208	db 'Spin buzz',0
.209	db 'Offline seek performance',0
.220	db 'Disk Shift',0
.221	db 'G-Sense Error Rate (Hitachi - Shock Sense Error Rate)',0
.222	db 'Loaded Hours',0
.223	db 'Load/Unload Retry Count',0
.224	db 'Load Friction',0
.225	db 'Load/Unload Cycle Count',0
.226	db 'Load-in Time',0
.227	db 'Torque Amplification Count',0
.228	db 'Power-Off Retract Count',0
;229	db '? (IBM DTTA)',0
.230	db 'GMR Head Amplitude',0
.231	db 'Temperature',0
.240	db 'Head Flying Hours (Hitachi)',0
.250	db 'Read Error Retry Rate',0
;---------------------------------------------------------------------
title	db 'Hard Disk Drive INFOrmer v0.2',0
menu_text	db 'Info  SMART  Tests  Control      <   >',0
ctrl_text1	db 'Ports:    1f0-1f7   170-177',0
ctrl_text2	db 'Device:   0   1     0   1',0
edbx_text1	db 'Info:   Save   Load',0
edbx_text2	db 'SMART:  Save   Load',0
smrt_text1	db 'SMART on/off',0
page_text	db 'Page:',0
;system_path	db '/sys/lib/'
;boxlib_name	db 'box_lib.obj',0
;head_f_i:
;head_f_l	db 'error',0
;err_message_found_lib	db 'box_lib.obj was not found',0
;err_message_import	db 'box_lib.obj was not imported ',0
error_open_file_string_Info	db 'Can not open Info file!',0
error_save_file_string_Info	db 'Can not save Info file!',0

error_open_file_string_SMART	db 'Can not open SMART file!',0
error_save_file_string_SMART	db 'Can not save SMART file!',0
no_error_text	db 0
align 4
error_text	dd no_error_text
;---------------------------------------------------------------------
system_dir_Boxlib	db '/sys/lib/box_lib.obj',0
system_dir_ProcLib	db '/sys/lib/proc_lib.obj',0
;---------------------------------------------------------------------
head_f_i:
head_f_l	db 'System error',0

err_message_found_lib1	db 'box_lib.obj - Not found!',0
err_message_found_lib2	db 'proc_lib.obj - Not found!',0

err_message_import1	db 'box_lib.obj - Wrong import!',0
err_message_import2	db 'proc_lib.obj - Wrong import!',0
;---------------------------------------------------------------------
align 4
l_libs_start:

library01  l_libs system_dir_Boxlib+9, cur_dir_path, library_path, system_dir_Boxlib, \
err_message_found_lib1, head_f_l, Box_lib_import, err_message_import1, head_f_i

library02  l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

end_l_libs:
;---------------------------------------------------------------------


;edit1 edit_box 200,2,85,0xaaaaaa,0x6a9480,0,0xAABBCC,0,128,file_name,ed_focus,10,10
op1 option_box option_group1,72,44,6,9,0xffffff,0x0,0,0,0
op2 option_box option_group1,94,44,6,9,0xffffff,0x0,0,0,0
op3 option_box option_group1,132,44,6,9,0xffffff,0x0,0,0,0
op4 option_box option_group1,154,44,6,9,0xffffff,0x0,0,0,0
option_group1	dd op1
Option_boxs	dd op1,op2,op3,op4,0
;---------------------------------------------------------------------
;file_name	db '/rd/1/hdd_',0
;times 128 db (0)
;---------------------------------------------------------------------
struct f70
	func_n	dd ?
	param1	dd 0
	param2	dd 0
	param3	dd ?
	param4	dd ?
	rezerv	db 0
	name	dd filename_area
ends
;---------------------------------------------------------------------
func_70	f70
;	Info_name	dd filename_area	;file_name
;---------------------------------------------------------------------
;	SMART_name	dd filename_area2	;file_name
;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init		dd aOpenDialog_Init
OpenDialog_Start	dd aOpenDialog_Start
;OpenDialog__Version	dd aOpenDialog_Version
        dd      0
        dd      0
aOpenDialog_Init	db 'OpenDialog_init',0
aOpenDialog_Start	db 'OpenDialog_start',0
;aOpenDialog_Version	db 'Version_OpenDialog',0
;---------------------------------------------------------------------
align 4
Box_lib_import:	
;init_lib		dd a_init
;version_lib		dd a_version


edit_box_draw		dd aEdit_box_draw
edit_box_key		dd aEdit_box_key
edit_box_mouse		dd aEdit_box_mouse
;version_ed		dd aVersion_ed

;check_box_draw		dd aCheck_box_draw
;check_box_mouse	dd aCheck_box_mouse
;version_ch		dd aVersion_ch

option_box_draw		dd aOption_box_draw
option_box_mouse	dd aOption_box_mouse
;version_op		dd aVersion_op

;scrollbar_ver_draw	dd aScrollbar_ver_draw
;scrollbar_ver_mouse	dd aScrollbar_ver_mouse
;scrollbar_hor_draw	dd aScrollbar_hor_draw
;scrollbar_hor_mouse	dd aScrollbar_hor_mouse
;version_scrollbar	dd aVersion_scrollbar

;dinamic_button_draw	dd aDbutton_draw
;dinamic_button_mouse	dd aDbutton_mouse
;version_dbutton	dd aVersion_dbutton

;menu_bar_draw		dd aMenu_bar_draw
;menu_bar_mouse		dd aMenu_bar_mouse
;menu_bar_activate	dd aMenu_bar_activate
;version_menu_bar	dd aVersion_menu_bar

;FileBrowser_draw	dd aFileBrowser_draw
;FileBrowser_mouse	dd aFileBrowser_mouse
;FileBrowser_key	dd aFileBrowser_key
;Version_FileBrowser	dd aVersion_FileBrowser

PathShow_prepare	dd sz_PathShow_prepare
PathShow_draw		dd sz_PathShow_draw
;Version_path_show	dd szVersion_path_show
			dd 0
			dd 0

;a_init			db 'lib_init',0
;a_version		db 'version',0

aEdit_box_draw		db 'edit_box',0
aEdit_box_key		db 'edit_box_key',0
aEdit_box_mouse		db 'edit_box_mouse',0
;aVersion_ed		db 'version_ed',0

;aCheck_box_draw	db 'check_box_draw',0
;aCheck_box_mouse	db 'check_box_mouse',0
;aVersion_ch		db 'version_ch',0

aOption_box_draw	db 'option_box_draw',0
aOption_box_mouse	db 'option_box_mouse',0
;aVersion_op		db 'version_op',0

;aScrollbar_ver_draw	db 'scrollbar_v_draw',0
;aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
;aScrollbar_hor_draw	db 'scrollbar_h_draw',0
;aScrollbar_hor_mouse	db 'scrollbar_h_mouse',0
;aVersion_scrollbar	db 'version_scrollbar',0

;aDbutton_draw		db 'dbutton_draw',0
;aDbutton_mouse		db 'dbutton_mouse',0
;aVersion_dbutton	db 'version_dbutton',0

;aMenu_bar_draw		db 'menu_bar_draw',0
;aMenu_bar_mouse		db 'menu_bar_mouse',0
;aMenu_bar_activate	db 'menu_bar_activate',0
;aVersion_menu_bar	db 'version_menu_bar',0

;aFileBrowser_draw	db 'FileBrowser_draw',0
;aFileBrowser_mouse	db 'FileBrowser_mouse',0
;aFileBrowser_key	db 'FileBrowser_key',0
;aVersion_FileBrowser	db 'version_FileBrowser',0

sz_PathShow_prepare	db 'PathShow_prepare',0
sz_PathShow_draw	db 'PathShow_draw',0
;szVersion_path_show	db 'version_PathShow',0
;---------------------------------------------------------------------
PathShow_data_1:
.type			dd 0	;+0
.start_y		dw 85	;+4
.start_x		dw 128	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 415	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd fname_Info	;+28
.work_area_pointer	dd text_work_area	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
PathShow_data_2:
.type			dd 0	;+0
.start_y		dw 104	;+4
.start_x		dw 127	;+6
.font_size_x		dw 6	;+8	; 6 - for font 0, 8 - for font 1
.area_size_x		dw 415	;+10
.font_number		dd 0	;+12	; 0 - monospace, 1 - variable
.background_flag	dd 0	;+16
.font_color		dd 0x0	;+20
.background_color	dd 0x0	;+24
.text_pointer		dd fname_SMART	;+28
.work_area_pointer	dd text_work_area2	;+32
.temp_text_length	dd 0	;+36
;---------------------------------------------------------------------
OpenDialog_data:
.type			dd 0
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status			dd 0	;+32
.openfile_pach 		dd fname_Info	;+36
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

OpenDialog_data2:
.type			dd 0
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name2	;+8
.com_area		dd 0	;+12
.opendir_pach		dd temp_dir_pach2	;+16
.dir_default_pach	dd communication_area_default_pach	;+20
.start_path		dd open_dialog_path	;+24
.draw_window		dd draw_window	;+28
.status			dd 0	;+32
.openfile_pach 		dd fname_SMART	;+36
.filename_area		dd filename_area2	;+40
.filter_area		dd Filter2
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

communication_area_name2:
	db 'FFFFFFFF_open_dialog',0

communication_area_name:
	db 'FFFFFFFF_open_dialog2',0
open_dialog_path:
if __nightbuild eq yes
    db '/sys/MANAGERS/opendial',0
else
    db '/sys/File Managers/opendial',0
end if
communication_area_default_pach:
	db '/sys',0

Filter:
dd	Filter.end - Filter
.1:
db	'INFO',0
.end:
db	0

Filter2:
dd	Filter.end - Filter
.1:
db	'SMART',0
.end:
db	0

default_Info:
	db '/sys/default.info',0
default_SMART:
	db '/sys/default.smart',0
;---------------------------------------------------------------------
page_num db 1
flags dw 100b
;бит 0: 0/1 - InfoBlock не/прочитан
;1: 0/1 - SmartBlock не/прочитан
;2: 1 - показ ControlBlock
;3: 1 - при попытке чтения возникла ошибка (превышено время ожидания)
;4: 0/1 - сохраняем/загружаем InfoBlock/Smart
;5: 0/1 - выбор Master/Slave
;6: 1 - показ Tests
;7: 1 - показ InfoBlock
;8: 1 - показ Smart
;9: 1 - выключаем или включаем SMART
;10: 1 - включаем SMART
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
align 16
InfoArray: times 256 dw 0
SmartArray: times 512 dw 0

;buf_cmd_lin	rb 0
;threath_buf	rb 0x400
align 4
;---------------------------------------------------------------------
cur_dir_path:
	rb 4096
;---------------------------------------------------------------------
fname_Info:
	rb 4096            ; filename
;---------------------------------------------------------------------
fname_SMART:
	rb 4096            ; filename
;---------------------------------------------------------------------
text_work_area:
	rb 1024
;---------------------------------------------------------------------
text_work_area2:
	rb 1024
;---------------------------------------------------------------------
procinfo:
	rb 1024
;---------------------------------------------------------------------
temp_dir_pach:
	rb 4096
;---------------------------------------------------------------------
temp_dir_pach2:
	rb 4096
;---------------------------------------------------------------------
filename_area:
	rb 256
;---------------------------------------------------------------------
filename_area2:
	rb 256
;---------------------------------------------------------------------
library_path:
	rb 4096
;---------------------------------------------------------------------
	rb 4096
stacktop:
;---------------------------------------------------------------------
I_END:
;---------------------------------------------------------------------