; 15/III 2010 staper@inbox.ru

appname equ 'Программируемый микрокалькулятор '
version    equ ''

include 'macros.inc'

header '01',1,START,I_END,(D_END+0x100),(D_END+0x100),0,cur_dir_path

include 'opcodes.inc'
include 'proc32.inc'
include 'MASMFpuLib.asm'
include 'editbox_ex.mac'
include 'load_lib.mac'
include 'macroPRMK.inc'

_flags = 0x0
;0  1 - исполнение программы
;1  1 - аварийный останов, не используется
;2- 
;3  1 - нажата F
;4  1 - нажата K
;5  1 - режим программирования
;6  1 - заполнение экспоненты
;7  1 - регистры подняты
;8  1 - дополнение команды (0х4?,0х6? и пр.)
;9  1 - дозапись команды (адрес перехода после ОПКОДа)
;10 1 - необходимо обнулить экранную строку, но не показывать её, поднять регистры
;11 1 - нажата П
;12 1 - нажата ИП
;13 1 - скрывать регистры и программу
;25-27 - используются в диалоге сохранениея/загрузки
;28-
;29 - "sk" в key
;30 0 - трансцендентые функции представлены в радианах, иначе 31
;31 0/1 - трансцендентые функции представлены в градах/градусах

START:
	load_library	boxlib_name,cur_dir_path,buf_cmd_lin,system_path,\
	err_message_found_lib,head_f_l,myimport,err_message_import,head_f_i

	mcall	40,0x7
	mcall	48,4
	mov	[scin_height],eax

;определяем длину строки с параметрами
	mov	esi,buf_cmd_lin
	xor	ecx,ecx
@@:	cmp	byte [esi+ecx],0
	je	@f
	inc	ecx
	jmp	@b
@@:	mov	dword [edit2.size],ecx
	mov	dword [edit2.pos],ecx

	mcall	68,11
	cmp	dword [edit2.pos],0
	je	@f
	call	load_prog
@@:

red:
	call draw_window
still:
	test	[flags],(1 shl 0)
	jnz	main_loop
	mcall	10
.0:	dec	eax
	jz	red
	dec	eax
	jz	key
	dec	eax
	jz	button
	sub	eax,3
	jz	mouse
	jmp	still
main_loop:
	mcall	11
	test	eax,eax
	jnz	still.0
	cmp	[schk],-1
	jne	@f
	btr	[flags],0
	mcall	55,55,,,sound_data
	jmp	still
@@:	movzx	esi,[schk]
	movzx	eax, byte [esi+PMEM]
	shl	eax,2
	add	eax,OPCODEtable
	call	dword [eax]
	call	draw_schk
	call	draw_prog
	test	[flags],(1 shl 0)
	jz	@f
	mcall	5,[_pause]
	jmp	still
@@:	mcall	55,55,,,sound_data
	jmp	still

mouse:	if 0
	push	dword scroll_bar_data_vertical
	call	[scrollbar_ver_mouse]
	endf
	jmp	still

button:
	mcall	17
	dec	ah
	jnz	@f
	mcall	-1
@@:	cmp	ah,31
	jae	.grd
	movzx	eax,ah
	cmp	eax,30
	jg	still
	shl	eax,2
	call	dword [eax+but_table-4]
	test	[flags],(1 shl 5)
	jz	@f
	call	ftos
	call	draw_string
	call	draw_prog
@@:	call	draw_schk
	jmp	still
.grd:	cmp	ah,31
	jne	@f
	btr	[flags],30
	call	set_but
	jmp	still
@@:	cmp	ah,32
	jne	@f
	bts	[flags],30
	btr	[flags],31
	call	set_but
	jmp	still
@@:	cmp	ah,33
	jne	@f
	bts	[flags],30
	bts	[flags],31
	call	set_but
@@:	cmp	ah,34
	jne	@f
	btc	[flags],13
	jc	.331
	mcall	67,-1,-1,198,-1
	jmp	still
	.331:
	mcall	67,-1,-1,485,-1
@@:	jmp	still

key:
	mcall	2
@@:	cmp	ah,12		;load
	jne	@f
	bt	[flags],27
	jc	still
	bt	[flags],26
	jc	still
	bts	[flags],25
	call	window_load_save
	jmp	still
@@:	cmp	ah,19		;save
	jne	@f
	bt	[flags],27
	jc	still
	bt	[flags],26
	jc	still
	btr	[flags],25
	call	window_load_save
	jmp	still

@@:	push	dword edit1
	call	[edit_box_key]

.0:	call	draw_schk

	cmp	[edit1.pos],2
	jb	still
	test	[flags],(1 shl 29)
	jnz	@f
	cmp	word [txt.edbox],"sk"; or "BP" or "Bp" or "bP"
	jne	@f
	bts	[flags],29
	mov	word [txt.edbox],0x2020
	mov	[edit1.size],0
	mov	[edit1.pos],0
	push	dword edit1
	call	[edit_box_draw]
	jmp	key.0
@@:	mov	ah,[txt.edbox]
	sub	ah,48
	cmp	ah,9
	jbe	.1
	sub	ah,7
	cmp	ah,15
	jbe	.1
	sub	ah,32
.1:	mov	al,[txt.edbox+1]
	sub	al,48
	cmp	al,9
	jbe	.2
	sub	al,7
	cmp	al,15
	jbe	.1
	sub	al,32
.2:	shl	al,4
	shr	ax,4
	mov	[edit1.size],0
	mov	[edit1.pos],0
	test	[flags],(1 shl 29)
	jnz	.4
	movzx	ebx,[schk]
	add	ebx,PMEM
	mov	[ebx],al
	inc	[schk]
	jmp	.3
.4:	mov	[schk],al
	btr	[flags],29
.3:	call	draw_schk
	mov	word [txt.edbox],0x2020
	push	dword edit1
	call	[edit_box_draw]
	call	draw_prog
	test	[flags],(1 shl 5)
	jz	@f
	call	ftos
	call	draw_string
@@:	jmp	key.0


 align 4

but_table:

dd	.Cx,	.vp,	.sign,	.dot,	.0
dd	.up,	.xy,	_3_,	_2_,	_1_
dd	.min,	.div,	_6_,	_5_,	_4_
dd	.plus,	.x,	_9_,	_8_,	_7_
dd	.PP,	.BP,	.p,	.ip,	.K
dd	.Sp,	.Vo,	.shgl,	.shgr,	.F

.BP:	test [flags],(1 shl 5)
	jnz @f
	bts [flags],9
	ret    
    @@:	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .BP0
		mov [dop8],0x80
		bts [flags],8
		btr [flags],4
		ret
	.BP0:	mov byte [eax],0x51
		bts [flags],9
		ret
	@@:	mov byte [eax],0x58
		bts [flags],9
		btr [flags],3
		ret
.shgl:	test [flags],(1 shl 5)
	jnz @f
	ret
    @@: test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .shgl0
	mov [dop8],0xE0
	bts [flags],8
	btr [flags],4
	ret
	.shgl0:
	dec [schk]
	ret
    @@: movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	mov byte [eax],0x5E
	bts [flags],9
	btr [flags],3
	ret

.shgr:	test [flags],(1 shl 5)
	jnz @f
	ret
    @@: test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .shgr0
	mov [dop8],0xC0
	bts [flags],8
	btr [flags],4
	ret
	.shgr0:
	inc [schk]
	ret
    @@: movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	mov byte [eax],0x5C
	bts [flags],9
	btr [flags],3
	ret

.PP:	test [flags],(1 shl 5)
	jnz .PPprg
	mov eax,0x53
	jmp .to_opcode
	.PPprg:
	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .PP0
	mov [dop8],0xA0
	bts [flags],8
	btr [flags],4
	dec [schk]
	ret
	.PP0:
	mov byte [eax],0x53
	bts [flags],9
	ret
    @@: mov byte [eax],0x5A
	bts [flags],9
	btr [flags],3
	ret
.Vo:	test [flags],(1 shl 5)
	jnz .Voprg
	mov eax,0x52
	jmp .to_opcode
	.Voprg:
	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .Vo0
	mov [dop8],0x90
	bts [flags],8
	btr [flags],4
	dec [schk]
	ret
	.Vo0:
	mov byte [eax],0x52
	ret
    @@: mov byte [eax],0x59
	bts [flags],9
	btr [flags],3
	ret

.Sp:	test [flags],(1 shl 5)
	jnz .Spprg
	mov eax,0x50
	jmp .to_opcode
	.Spprg:
	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .Sp0
	mov [dop8],0x70
	bts [flags],8
	btr [flags],4
	dec [schk]
	ret
	.Sp0:
	mov byte [eax],0x50
	ret
    @@: mov byte [eax],0x57
	bts [flags],9
	btr [flags],3
	ret
.ip:	test [flags],(1 shl 5)
	jnz .ipprg
	bts [flags],12
	ret
	.ipprg:
	test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .ip0
	mov [dop8],0xD0
	bts [flags],8
	btr [flags],4
	ret
	.ip0:
	mov [dop8],0x60
	bts [flags],8
	ret
    @@: movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	mov byte [eax],0x5D
	bts [flags],9
	btr [flags],3
	ret



.p:	test [flags],(1 shl 5)
	jnz .pprg
	bts [flags],11
.nop:	ret
	.pprg:
	test [flags],(1 shl 3)
	jnz @f
	test [flags],(1 shl 4)
	jz .p0
	mov [dop8],0xB0
	bts [flags],8
	btr [flags],4
	ret
	.p0:
	mov [dop8],0x40
	bts [flags],8
	ret
    @@: movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	mov byte [eax],0x5B
	bts [flags],9
	btr [flags],3
	ret



.F:	bts [flags],3
	ret
.K:	bts [flags],4
	ret
.Cx:	test [flags],(1 shl 5)
	jnz .Cxprg
	test [flags],(1 shl 11)
	jnz .cx4d
	test [flags],(1 shl 12)
	jnz .cx6d
	test [flags],(1 shl 9)
	jnz .Cx0
	mov eax,0x0D
	jmp .to_opcode
		.cx4d:	mov eax,0x4d
		jmp .to_opcode
		.cx6d:	mov eax,0x6d
		jmp .to_opcode
	.Cxprg:
	test [flags],(1 shl 8)
	jz .Cx0
	add [dop8],0x0D
	jmp .dop8
	.Cx0:
	test [flags],(1 shl 9)
	jz .Cx1
	mov al,0x0D
	jmp .dop9
	.Cx1:
	movzx eax,[schk]
	add eax,PMEM
	mov byte [eax],0x0D
	inc [schk]
	ret
.vp:	test [flags],(1 shl 3)
	jnz .vpsetprg
	test [flags],(1 shl 5)
	jnz .vpprg
	test [flags],(1 shl 11)
	jnz .vp4c
	test [flags],(1 shl 12)
	jnz .vp6c
	test [flags],(1 shl 9)
	jnz .vp0
	mov eax,0x0C
	jmp .to_opcode
		.vp4c:	mov eax,0x4c
		jmp .to_opcode
		.vp6c:	mov eax,0x6c
		jmp .to_opcode
	.vpsetprg:
	test [flags],(1 shl 5)
	jz @f
	ret
	.vpprg:
	test [flags],(1 shl 8)
	jz .vp0
	add [dop8],0x0c
	jmp .dop8
	.vp0:
	test [flags],(1 shl 9)
	jz .vp1
	mov al,0x0C
	jmp .dop9
	.vp1:
	movzx eax,[schk]
	add eax,PMEM
	mov byte [eax],0x0C
	inc [schk]
	ret
    @@: bts [flags],5
	btr [flags],3
	ret
.sign:	test [flags],(1 shl 5)
	jnz .signprg
	test [flags],(1 shl 11)
	jnz .sign4b
	test [flags],(1 shl 12)
	jnz .sign6b
	test [flags],(1 shl 9)
	jnz .sign0
	mov eax,0x0B
	jmp .to_opcode
		.sign4b:	mov eax,0x4b
				jmp .to_opcode
		.sign6b:	mov eax,0x6b
				jmp .to_opcode
	.signprg:
	test [flags],(1 shl 8)
	jz .sign0
		add [dop8],0x0B
		jmp .dop8
	.sign0:	test [flags],(1 shl 9)
		jz .sign1
			mov al,0x0B
			jmp .dop9
	.sign1:	test [flags],(1 shl 3)
		jnz @f
			movzx eax,[schk]
			add eax,PMEM
			mov byte [eax],0x0B
			inc [schk]
			ret
 	@@:	btr [flags],5
		btr [flags],3
		fld qword [S.X]
		call ftos
		call draw_string
		ret
.up:	test [flags],(1 shl 5)
	jnz .upprg
	test [flags],(1 shl 3)
	jnz .up0f
	test [flags],(1 shl 4)
	jnz .upK
	test [flags],(1 shl 11)
	jnz .up4e
	test [flags],(1 shl 12)
	jnz .up6e
	test [flags],(1 shl 9)
	jnz .up0
	mov eax,0x0E
	jmp .to_opcode
		.up0f:	mov eax,0x0F
		jmp .to_opcode
		.up4e:	mov eax,0x4e
		jmp .to_opcode
		.up6e:	mov eax,0x6e
		jmp .to_opcode
	.upprg:
	test [flags],(1 shl 8)
	jz .up0
		add [dop8],0x0E
		jmp .dop8
	.up0:	test [flags],(1 shl 9)
		jz .up1
			mov al,0x0E
			jmp .dop9
	.up1:	movzx eax,[schk]
		add eax,PMEM
		inc [schk]
		test [flags],(1 shl 3)
		jnz @f
		mov byte [eax],0x0E
		ret
	@@:	mov byte [eax],0x0F
		btr [flags],3
		ret
	.upK:	mov eax,0x3B
		jmp .to_opcode

.xy:	test [flags],(1 shl 5)
	jnz .xyprg
	test [flags],(1 shl 3)
	jnz .xy24
	mov eax,0x14
	jmp .to_opcode
		.xy24:	mov eax,0x24
		jmp .to_opcode
	.xyprg:
		movzx eax,[schk]
		add eax,PMEM
		inc [schk]
		test [flags],(1 shl 3)
		jnz @f
		mov byte [eax],0x14
		ret
	@@:	mov byte [eax],0x24
		btr [flags],3
		ret
.min:	test [flags],(1 shl 5)
	jnz .minprg
	test [flags],(1 shl 3)
	jnz .min21
	test [flags],(1 shl 9)
	jnz .min0
	mov eax,0x11
	jmp .to_opcode
		.min21: mov eax,0x21
		jmp .to_opcode
	.minprg:
		test [flags],(1 shl 8)
		jz .min0
			add [dop8],0x0F
			jmp .dop8
	.min0:	test [flags],(1 shl 9)
		jz .min1
			mov al,0x0F
			jmp .dop9
	.min1:	movzx eax,[schk]
		add eax,PMEM
		inc [schk]
		test [flags],(1 shl 3)
		jnz @f
		mov byte [eax],0x11
		ret
	@@:	mov byte [eax],0x21
		btr [flags],3
		ret
.div:	test [flags],(1 shl 5)
	jnz .divprg
	test [flags],(1 shl 3)
	jnz .div23
	mov eax,0x13
	jmp .to_opcode
		.div23: mov eax,0x23
		jmp .to_opcode
	.divprg:
	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	mov byte [eax],0x13
	ret
    @@: mov byte [eax],0x23
	btr [flags],3
	ret
.plus:	test [flags],(1 shl 5)
	jnz .plusprg
	test [flags],(1 shl 3)
	jnz .plus10
	mov eax,0x10
	jmp .to_opcode
		.plus10:	mov eax,0x20
		jmp .to_opcode
	.plusprg:
	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	mov byte [eax],0x10
	ret
    @@: mov byte [eax],0x20
	btr [flags],3
	ret
.x:	test [flags],(1 shl 5)
	jnz .xprg
	test [flags],(1 shl 3)
	jnz .x22
	mov eax,0x12
	jmp .to_opcode
		.x22:	mov eax,0x22
		jmp .to_opcode
	.xprg:
	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	test [flags],(1 shl 3)
	jnz @f
	mov byte [eax],0x12
	ret
    @@: mov byte [eax],0x22
	btr [flags],3
	ret
.dot:	test [flags],(1 shl 5)
	jnz .dotprg
	test [flags],(1 shl 3)
	jnz .dot25
	test [flags],(1 shl 11)
	jnz .dot4a
	test [flags],(1 shl 12)
	jnz .dot6a
	test [flags],(1 shl 9)
	jnz .dot0
	mov eax,0x0A
	jmp .to_opcode
		.dot25: mov eax,0x25
		jmp .to_opcode
		.dot4a: mov eax,0x4a
		jmp .to_opcode
		.dot6a: mov eax,0x6a
		jmp .to_opcode
	.dotprg:
		test [flags],(1 shl 8)
		jz .dot0
			add [dop8],0x0A
			jmp .dop8
	.dot0:	test [flags],(1 shl 9)
		jz .dot1
		mov al,0x0A
		jmp .dop9
	.dot1:	movzx eax,[schk]
		add eax,PMEM
		inc [schk]
		test [flags],(1 shl 3)
		jnz @f
		mov byte [eax],0x0A
		ret
	@@:	mov byte [eax],0x25
		btr [flags],3
		ret

.0:	test [flags],(1 shl 5)
	jnz .0prg
	test [flags],(1 shl 3)
	jnz .015
	test [flags],(1 shl 11)
	jnz .040
	test [flags],(1 shl 12)
	jnz .060
	test [flags],(1 shl 9)
	jnz .00
	mov eax,0
	jmp .to_opcode
		.015:	mov eax,0x15
		jmp .to_opcode
		.040:	mov eax,0x40
		jmp .to_opcode
		.060:	mov eax,0x60
		jmp .to_opcode
	.0prg:
		test [flags],(1 shl 8)
		jz .00
			add [dop8],0x00
			jmp .dop8
	.00:	test [flags],(1 shl 9)
		jz .01
			mov al,0
			jmp .dop9
	.01:	movzx eax,[schk]
		add eax,PMEM
		inc [schk]
		test [flags],(1 shl 4)
		jz @f
		mov ebx,0
		test [flags],(1 shl 8)
		jnz .dop8
		test [flags],(1 shl 9)
		jnz .dop9
		mov byte [eax],0x54
		bts [flags],4
		ret
	@@:	test [flags],(1 shl 3)
		jnz @f
		mov byte [eax],0x00
		ret
	@@:	mov byte [eax],0x15
		btr [flags],3
		ret

.dop8:	movzx eax,[schk]
	add eax,PMEM
	inc [schk]
	mov bl,[dop8]
	mov byte [eax],bl
	btr [flags],8
	ret
.dop9:	cmp [mov3],0
	jne @f
	inc [mov3]
	shl al,4
	mov [dop9],al
	ret
    @@: mov [mov3],0
	add al,[dop9]
	test [flags],(1 shl 5)
	jz @f
	movzx ebx,[schk]
	add ebx,PMEM
	mov [ebx],al
	inc [schk]
	btr [flags],9
	ret
    @@: mov [schk],al
	btr [flags],9
	call draw_prog
	ret
	
 align 4
.to_opcode:
	shl eax,2
	add eax,OPCODEtable
	mov ebx,[eax]
	jmp ebx

dop8 db 0
dop9 db 0
mov3 db 0


_1_:	buttabnum 1,0x16,0x41,0x61,0
_2_:	buttabnum 2,0x17,0x42,0x62,0
_3_:	buttabnum 3,0x18,0x43,0x63,0
_4_:	buttabnum 4,0x19,0x44,0x64,0x31
_5_:	buttabnum 5,0x1A,0x45,0x65,0x32
_6_:	buttabnum 6,0x1B,0x46,0x66,0
_7_:	buttabnum 7,0x1C,0x47,0x67,0x34
_8_:	buttabnum 8,0x1D,0x48,0x68,0x35
_9_:	buttabnum 9,0x1E,0x49,0x69,0x36




 align 4
stof:
	mov esi,string
	inc esi
	cmp word [esi],0x2030
	je .null
	mov dword [buf],0
	mov dword [buf+4],0
	mov word [buf+8],0
	xor ecx,ecx
@@:	cmp byte [esi+ecx],'.'
	je .dot
	cmp byte [esi+ecx],0x20
	je .int
	inc ecx
	cmp ecx,8
	jne @b
.int:	dec ecx
	call .tobcd
	fbld [buf]
	jmp .tosign

.dot:	push ecx
	dec ecx
	call .tobcd
	fbld [buf]
	pop ecx
	add esi,ecx
	inc esi
	mov ebx,ecx
	dec ebx
	xor ecx,ecx
@@:	cmp byte [esi+ecx],0x20
	je @f
	cmp ebx,8
	je @f
	inc ecx
	inc ebx
	jmp @b
@@:	cmp ecx,0
	je .tosign
	push ecx
	dec ecx
	mov dword [buf],0
	mov dword [buf+4],0
	mov word [buf+8],0
	call .tobcd
	fbld [buf]
	pop ecx
	mov [perem],10
@@:	fidiv [perem]
	loop @b
	faddp
	jmp .tosign


.tobcd:
	mov edi,buf
@@:	mov al,[esi+ecx]
	sub al,48
	test ecx,ecx
	jz @f
	shl al,4
	mov ah,[esi+ecx-1]
	sub ah,48
	shr ax,4
	mov [edi],al
	inc edi
	dec ecx
	jz .tobcd1
	dec ecx
	jnz @b
	mov al,[esi]
	sub al,48
@@:	mov [edi],al
.tobcd1:
ret


.tosign:
	cmp byte [string],'-'
	jne @f
	fchs
@@:	cmp byte [string+12],0x20
	je .ret
	cmp byte [string+12],0x30
	jne @f
	cmp byte [string+11],0x30
	je .ret
@@:	mov al,[string+12]
	sub al,48
	mov ah,[string+11]
	sub ah,48
	shl al,4
	shr ax,4
	mov [buf],al
	mov dword [buf+1],0
	mov dword [buf+5],0
	fbld [buf]
	fistp [perem]
	mov ecx,[perem]
	mov [perem],10
	cmp byte [string+10],'-'
	je  .@@f
@@:	fimul [perem]
	loop @b
	ret
.@@f:	fidiv [perem]
	loop .@@f
.ret:	ret
.null:	fldz
ret

 align 4
ftos:
	test [flags],(1 shl 5)
	jnz .prg
	mov esi,string
	mov dword [esi],0x20202020
	mov dword [esi+4],0x20202020
	mov dword [esi+8],0x20202020
	mov byte [esi+12],0x20
;       fld st0
;       fxtract
;       fstp st0
;       fabs
;       fistp [perem]
;       mov eax,[perem]
;       cmp eax,0x1a
;       jg .1
;       stdcall FpuFLtoA,0,8,buf2,SRC1_FPU or SRC1_REAL; or STR_SCI
;       mov ecx,9
;       mov esi,string
;       mov edi,buf2
;@@:    mov al,[edi]
;       test al,al
;       jz @f
;       mov [esi],al
;       inc esi
;       inc edi
;       loop @b
;@@:    ret

.1:	stdcall FpuFLtoA,0,8,buf2,SRC1_FPU or SRC1_REAL or STR_SCI
	mov ecx,10
	mov esi,string
	mov edi,buf2
@@:	mov al,[edi]
	test al,al
	jz .ret
	mov [esi],al
	inc esi
	inc edi
	loop @b
;       add edi,9
@@:	cmp byte [edi],0
	je @f
	inc edi
	jmp @b
@@:	mov ax,[edi-2]
	mov [string+11],ax
	mov al,[edi-5]
	mov [string+10],al
.ret:
;        cmp word [string+11],0x2020
;        je .ret2
	cmp byte [string+10],'-'
	je .ret2
	mov ax,[string+11]
	sub ax,0x3030
	xchg ah,al
	shl al,4
	shr ax,4
	cmp al,7
	jge .ret2
	mov word [string+10],'  '
	mov byte [string+12],' '
	cmp al,0
	je .ret2
	xor edx,edx
@@:	mov bl,[string+2+edx]
	mov bh,[string+2+edx+1]
	xchg bh,bl
	mov [string+2+edx],bx
	inc edx
	dec al
	jnz @b
.ret2:	mov ecx,7
@@:	cmp byte [string+2+ecx],'0'
	jne @f
	mov byte [string+2+ecx],' '
	loop @b
@@:	cmp byte [string+2+ecx],'.'
	jne @f
	mov byte [string+2+ecx],' '
@@:	cmp byte [string+10],'+'
	jne @f
	mov byte [string+10],' '
@@:
ret
.toascii:
	shl ax,4
	shr al,4
	cmp al,10
	sbb al,69h
	das
	rol ax,8
	cmp al,10
	sbb al,69h
	das
	ret
.prg:	mov eax,0x20202020
	mov dword [string],eax
	mov dword [string+4],eax
	mov dword [string+8],eax
	xor eax,eax
	mov al,[schk]
	mov ebx,eax
	call .toascii
	mov [string+11],ax
	cmp	[schk],3
	jb @f
	movzx eax,byte [PMEM+ebx-3]
	call .toascii
	mov [string+8],ax
@@:	cmp	[schk],2
	jb @f
	movzx eax,byte [PMEM+ebx-2]
	call .toascii
	mov [string+5],ax
@@:	cmp	[schk],1
	jb @f
	movzx eax,byte [PMEM+ebx-1]
	call .toascii
	mov [string+2],ax
@@:
ret


;   *********************************************
;   ******* WINDOW DEFINITIONS AND DRAW *********
;   *********************************************

draw_window:
	mcall	12,1

	mcall	48,3,sc,sizeof.system_colors

	mov	edx,[sc.work]
	or	edx,0x34000000
	mov	ecx,200
	shl	ecx,16
	add	ecx,[scin_height]
	add	ecx,343
	mcall	0,160 shl 16+485,,,,title

	mov	ebp,smesh
	mcall	65,bmp_file+8,185*65536+262,2*65536+75,4,palitra

	mcall	13,13*65536+122,24*65536+20,0xffffff

	call	draw_string

	mcall	13,137*65536+32,48*65536+12,0x888888

	call	set_but
	call	draw_registers
	call	draw_stack
	call	draw_prog

	mcall	8,176*65536+11,61*65536+10,35,0xdadada

	mov	edi,[sc.work]
	or	edi,0x34000000
	mcall	4,133*65536+63,0x81000000,txt.perek
	mcall	,403*65536+4,,txt.prog
	mcall	,403*65536+12,,txt.prog1
	mcall	,380*65536+323,,txt.sk

	if 0
	mov	[scroll_bar_data_vertical.all_redraw],1
	push	dword scroll_bar_data_vertical
	call	[scrollbar_ver_draw]
	mov	[scroll_bar_data_vertical.all_redraw],0
	endf
	push	dword edit1
	call	[edit_box_draw]

	;CK
	call	draw_schk

	;обозначения регистров
	mcall	4,240*65536+4,0x81000000,txt.regs
	mov	word [perem],"0:"
	mov	byte [perem+2],0
	mov	ebx,210*65536+20
	mov	edx,perem
	push	dword 15
@@:	cmp	dword [esp],5
	je	@f
.prevr: int	0x40
	add	ebx,12
	inc	byte [perem]
	dec	dword [esp]
	jnz	@b
	jmp	.nextr
@@:	add	byte [perem],7
	jmp	.prevr
.nextr: add	esp,4

	;обозначения стека
	mcall	4,240*65536+220,0x81000000,txt.stk
	mov	word [perem],"T:"
	mov	byte [perem+2],0
	mov	ebx,210*65536+240
	mov	edx,perem
	push	dword 5
.firsts:	cmp	dword [esp],4
	jne	@f
	mov	byte [perem],"Z"
	jmp	.prevs
@@:	cmp	dword [esp],3
	jne	@f
	mov	byte [perem],"Y"
	jmp	.prevs
@@:	cmp	dword [esp],2
	jne	@f
	mov	byte [perem],"X"
	jmp	.prevs
@@:	cmp	dword [esp],1
	jne	.prevs
	mov	dword [perem],"X1"
.prevs: int	0x40
	add	ebx,12
	dec	dword [esp]
	jnz	.firsts
.nexts: add	esp,4

	mcall	12,2
ret

 align 4
draw_string:
;       mov     edi,[sc.work]
	or	edi,0x34ffffff
	mov	edx,edi
	mcall	13,20*65536+105,30*65536+7
	mcall	4,20*65536+30,0x900000ff,string,,
ret

 align 4
draw_schk:
	test	[flags],(1 shl 13)
	jnz	.ret
	mov	edi,[sc.work]
	or	edi,0x34000000
	movzx	ecx,[schk]
	mcall	47,0x020100,,400*65536+323,0x50000000,
.ret:
ret

 align 4
set_but:
	mov	ecx,34
	mov	eax,8
@@:	push	ecx
	lea	ebx,[ecx+0x80000000+1]
	int	0x40
	pop	ecx
	loop	@b

	xor	edi,edi
	mov	esi,0xffffff
	bt	[flags],30
	jnc	@f
	mov	esi,0x0
@@:	mcall	8,140*65536+7,50*65536+7,32
	mov	esi,0x0
	add	ebx,9*65536
	inc	edx
	bt	[flags],30
	jnc	@f
	bt	[flags],31
	jc	@f
	mov	esi,0xffffff
@@:	mcall
	mov	esi,0
	add	ebx,9*65536
	inc	edx
	bt	[flags],30
	jnc	@f
	bt	[flags],31
	jnc	@f
	mov	esi,0xffffff
@@:	mcall

	mov	ecx,30
	push	ecx
	mov	ecx,89*65536+22
.00:	mov	ebx,8*65536+27
.0:	pop	eax
	lea	edx,[eax+(1 shl 30) + 1]
	push	eax
	mcall	8,,,
	mov	eax,ecx
	pop	ecx
	dec	ecx
	jz	.ret
	push	ecx
	cmp	ecx,5
	je	.1
	cmp	ecx,10
	je	.1
	cmp	ecx,15
	je	.1
	cmp	ecx,20
	je	.1
	cmp	ecx,25
	je	.1
	add	ebx,36*65536
	mov	ecx,eax
	jmp	.0
.1:	add	eax,42*65536
	mov	ecx,eax
	jmp	.00
.ret:
ret

 align 4
draw_registers:
	test	[flags],(1 shl 13)
	jnz	.ret
	mov	esi,R.0
	mov	ecx,15
	mov	edx,[sc.work]
	or	edx,0x34000000
	push	edx
	mov	ebx,220*65536+150
	push	ebx
	mov	ebx,20*65536+9
	push	ebx
	push	esi ecx
@@:	call	.ftoa
	mov	ebx,[esp+12]
	mov	ecx,[esp+8]
	mov	edx,[esp+16]
	mcall	13
	mov	ebx,[esp+12]
	mov	eax,[esp+8]
	shr	eax,16
	mov	bx,ax
	mov	ecx,0;[esp+16]
	add	ecx,0x80000000
	mov	edx,buf2
	mcall	4
	dec	dword [esp]
	jz	@f
	add	dword [esp+4],8
	mov	esi,[esp+4]
	add	dword [esp+8],12*65536
	jmp	@b
@@:	add	esp,20
.ret:
ret

.ftoa:
	fld qword [esi]
	fld st0
	fabs
	fxtract
	fstp st0
	fabs
	fistp [perem]
	mov eax,[perem]
	cmp eax,0x1a+16383
	jge @f
	cmp eax,0x1a
	jge @f
	stdcall FpuFLtoA,0,8,buf2,SRC1_FPU or SRC1_REAL; or STR_SCI
	fstp st0
	ret
@@:	stdcall FpuFLtoA,0,8,buf2,SRC1_FPU or SRC1_REAL or STR_SCI
	fstp st0
	ret

 align 4
draw_stack:
;       test    [flags],(1 shl 13)
;       jnz     .ret
	mov	esi,S.T
	mov	ecx,5
	mov	edx,[sc.work]
	or	edx,0x34000000
	push	edx
	mov	ebx,220*65536+150
	push	ebx
	mov	ebx,240*65536+9
	push	ebx
	push	esi ecx
@@:	call	draw_registers.ftoa
	mov	ebx,[esp+12]
	mov	ecx,[esp+8]
	mov	edx,[esp+16]
	mcall	13
	mov	ebx,[esp+12]
	mov	eax,[esp+8]
	shr	eax,16
	mov	bx,ax
	mov	ecx,0;[esp+16]
	add	ecx,0x80000000
	mov	edx,buf2
	mcall	4
	dec	dword [esp]
	jz	@f
	sub	dword [esp+4],8
	mov	esi,[esp+4]
	add	dword [esp+8],12*65536
	jmp	@b
@@:	add	esp,20
;.ret:
ret

 align 4
draw_prog:
	test	[flags],(1 shl 13)
	jnz	.ret
	pushd	26
	movzx	eax,[schk]
	mov	[.sprog2],al
	xor	edx,edx
	div	dword [esp]
	xor	edx,edx
	mul	dword [esp]
	mov	[.sprog],al
	mov	edi,[sc.work]
	or	edi,0x34000000
	movzx	ecx, [.sprog]
	add	ecx,26
	sub	ecx,[esp]
	mov	esi,0x5000459a
	push	esi
	cmp	cl,[.sprog2]
	jne	@f
	mov	esi,0x50cc0000
@@:	mcall	47,0x020100,,400*65536+24,,
	pop	esi
	add	edx,30*65536
	movzx	ecx,[.sprog]
	add	ecx,PMEM+26
	sub	ecx,[esp]
	movzx	ecx, byte [ecx]
	mcall
	sub	edx,30*65536
@@:	add	edx,11
	dec	dword [esp]
	jz	@f
	movzx	ecx, [.sprog]
	add	ecx,26
	sub	ecx,[esp]
	push	esi
	cmp	cl,[.sprog2]
	jne	.2
	mov	esi,0x50cc0000
.2:	mcall
	pop	esi
	add	edx,30*65536
	movzx	ecx,[.sprog]
	add	ecx,PMEM+26
	sub	ecx,[esp]
	movzx	ecx, byte [ecx]
	mcall
	sub	edx,30*65536
	jmp	@b
@@:	pop	eax
.ret:
ret
.sprog db 0
.sprog2 db 0





window_load_save:
	popad
	mcall	51,1,.thread,(.threat_stack+32*4)
	pushad
	ret
.thread:
	bts	[flags],26
.red:
	mcall	12,1
	mov	edi,txt.load
	bt	[flags],25
	jc	@f
	mov	edi,txt.save
@@:	mcall	0,50*65536+300,0x70*65536+60,(0x34ffffff),,
	mcall	8,<245,43>,<2,14>,100,0xaaaaaa
	mcall	4,<252,5>,(0x80000000),txt.enter
	push	dword edit2
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
  .end: btr	[flags],26
	mcall	-1
  @@:	cmp	ah,100
	jne	.still
	bt	[flags],25
	jc	.ld
  .sv:	call	save_prog
	jnc	.end
	jmp	.err
  .ld:	call	load_prog
	jnc	.end
  .err: mcall	4,<5,19>,(0x80000000),txt.error
	jmp	.still


.key:
	mcall	2
	cmp	ah,13
	jne	@f
	bt	[flags],25
	jc	.ld
	jmp	.sv
   @@:	cmp	ah,27
	je	.end
	push	dword edit2
	call	[edit_box_key]
	jmp	.still

.threat_stack: times 32 dd 0


save_prog:
	mov	[_size],256+8*(15+5)+6*3
	mcall	68,12,[_size]
	test	eax,eax
	jnz	@f
	stc
	ret

@@:	mov	[_buffer],eax
	mov	esi,S.X1
	mov	edi,eax
	mov	byte [edi],"<"
	mov	dword [edi+1],"СТЕК"
	mov	byte [edi+5],">"
	add	edi,6
	mov	ecx,5*2
	cld
@@:	movsd
	loop	@b
	mov	esi,R.0
	mov	byte [edi],"<"
	mov	dword [edi+1],"РГСТ"
	mov	byte [edi+5],">"
	add	edi,6
	mov	ecx,15*2
	cld
@@:	movsd
	loop	@b
	mov	esi,PMEM
	mov	byte [edi],"<"
	mov	dword [edi+1],"ПРГМ"
	mov	byte [edi+5],">"
	add	edi,6
	mov	ecx,256/4
	cld
@@:	movsd
	loop	@b

	mov	[func_70.func_n],2
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

load_prog:
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
	cmp	eax,256+8*(15+5)+6*3
	jbe	@f
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

	mov	esi,[_buffer]
	mov	ecx,[_size]
@@:	cmp	byte [esi],"<"
	je	.@f1
	inc	esi
	loop	@b
.end:	mcall	68,13,[_buffer]
	clc
	ret
.@f1:	inc	esi
	cmp	dword [esi],"СТЕК"
	je	.st
	cmp	dword [esi],"РГСТ"
	je	.rg
	cmp	dword [esi],"ПРГМ"
	je	.pr
	jmp	@b
.st:	mov	edi,S.X1
	add	esi,5
	mov	ebx,5*2*4
	cld
 .st@:	movsb
	cmp	byte [esi],"<"
	jne	.stB
	inc	esi
	cmp	dword [esi],"РГСТ"
	je	.rg
	cmp	dword [esi],"ПРГМ"
	je	.pr
	dec	esi
 .stB:	dec	ecx
	jz	.end
	dec	ebx
	jnz	.st@
	jmp	@b
.rg:	mov	edi,R.0
	add	esi,5
	mov	ebx,15*2*4
	cld
 .rg@:	movsb
	cmp	byte [esi],"<"
	jne	.rgB
	inc	esi
	cmp	dword [esi],"СТЕК"
	je	.st
	cmp	dword [esi],"ПРГМ"
	je	.pr
	dec	esi
 .rgB:	dec	ecx
	jz	.end
	dec	ebx
	jnz	.rg@
	jmp	@b
.pr:	mov	edi,PMEM
	mov	eax,0
	mov	ebx,256/4
 .pr0:	mov	[edi],eax
	add	edi,4
	dec	ebx
	jnz	.pr0
	mov	edi,PMEM
	add	esi,5
	mov	ebx,256
	cld
 .pr@:	movsb
	cmp	byte [esi],"<"
	jne	.prB
	inc	esi
	cmp	dword [esi],"СТЕК"
	je	.st
	cmp	dword [esi],"РГСТ"
	je	.rg
	dec	esi
 .prB:	dec	ecx
	jz	.end
	dec	ebx
	jnz	.pr@
jmp	@b






align 4

S:
.X1: dq 0.0
.X: dq 0.0
.Y: dq 0.0
.Z: dq 0.0
.T: dq 0.0

RS:
times 10 db 0

align 4
R:
.0: dq 0.0
.1: dq 0.0
.2: dq 0.0
.3: dq 0.0
.4: dq 0.0
.5: dq 0.0
.6: dq 0.0
.7: dq 0.0
.8: dq 0.0
.9: dq 0.0
.A: dq 0.0
.B: dq 0.0
.C: dq 0.0
.D: dq 0.0
.E: dq 0.0
.F: dq 0.0
dq 0.0


align 4
myimport:
edit_box_draw	dd	aEdit_box_draw
edit_box_key	dd	aEdit_box_key
edit_box_mouse	dd	aEdit_box_mouse
version_ed	dd	aVersion_ed

scrollbar_ver_draw	dd aScrollbar_ver_draw
scrollbar_ver_mouse	dd aScrollbar_ver_mouse
version_scrollbar	dd aVersion_scrollbar

		dd	0
		dd	0

aEdit_box_draw	db 'edit_box',0
aEdit_box_key	db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed	db 'version_ed',0

aScrollbar_ver_draw	db 'scrollbar_v_draw',0
aScrollbar_ver_mouse	db 'scrollbar_v_mouse',0
aVersion_scrollbar	db 'version_scrollbar',0

if 0

align 4
scroll_bar_data_vertical:
.x:
.size_x     dw 15;+0
.start_x    dw 455 ;+2
.y:
.size_y     dw 284 ;+4
.start_y    dw 19 ;+6
.btn_high   dd 15 ;+8
.type	    dd 1  ;+12
.max_area   dd 300+20  ;+16
.cur_area   dd 50  ;+20
.position   dd 0  ;+24
.bckg_col   dd 0xAAAAAA ;+28
.frnt_col   dd 0xCCCCCC ;+32
.line_col   dd 0  ;+36
.redraw     dd 0  ;+40
.delta	    dw 0  ;+44
.delta2     dw 0  ;+46
.run_x:
.r_size_x   dw 0  ;+48
.r_start_x  dw 0  ;+50
.run_y:
.r_size_y   dw 0 ;+52
.r_start_y  dw 0 ;+54
.m_pos	    dd 0 ;+56
.m_pos_2    dd 0 ;+60
.m_keys     dd 0 ;+64
.run_size   dd 0 ;+68
.position2  dd 0 ;+72
.work_size  dd 0 ;+76
.all_redraw dd 0 ;+80
.ar_offset	dd 10 ;+84

endf

func_70:
 .func_n dd ?
 .param1 dd 0
 .param2 dd 0
 .param3 dd ?
 .param4 dd ?
 .rezerv db 0
 .name dd ?

flags dd _flags

_pause dd 10
string_zero db " 0           ",0
buf: times 10 db 0
perem dd 0
buf2: times 25 db 0
string: db " 0           ",0
buf3: times 25 db 0
schk db 0

title db appname,version,0

txt:
.save db 'Сохранить (Ctrl+S)',0
.error db 'Ошибка',0
.load db 'Загрузить (Ctrl+L)',0
.enter db 'Enter',0
.regs db "Регистры",0
.stk db "Стек",0
.perek db "Р ГРД Г >",0
.prog db "Программа",0
.prog1 db "Шаг Код",0
.sk db "СК:",0
.edbox db "  ",0

system_path db '/sys/lib/'
boxlib_name db 'box_lib.obj',0
head_f_i:
head_f_l	db 'error',0
err_message_found_lib	db 'box_lib.obj was not found',0
err_message_import	db 'box_lib.obj was not imported',0

edit1 edit_box 20,427,320,0xffffff,0x6a9480,0,0xAABBCC,0,2,txt.edbox,ed_focus,ed_focus,0,0
edit2 edit_box 240,2,2,0xffffff,0x6a9480,0,0xAABBCC,0,4096,buf_cmd_lin,ed_focus,2,0,0

virtual at 0
file 'MK_b3-34_hand.BMP':0xA,4
load offbits dword from 0
end virtual

palitra:
	file 'MK_b3-34_hand.BMP':0x36,offbits-0x36

sizey = 262
sizex = 185 + 7
smesh = 3

bmp_file:
    file 'MK_b3-34_hand.BMP':110
repeat sizey/2
y = % - 1
z = sizey - %
repeat sizex/2/4
load a dword from $ - sizex*sizey/2 + sizex*y/2+(%-1)*4
load b dword from $ - sizex*sizey/2 + sizex*z/2+(%-1)*4
store dword a at $ - sizex*sizey/2 + sizex*z/2+(%-1)*4
store dword b at $ - sizex*sizey/2 + sizex*y/2+(%-1)*4
end repeat
end repeat

sound_data:
db	40
dw	670
db 0

I_END:

sc     system_colors

procinfo:	rb 1024
buf_cmd_lin	rb 0
cur_dir_path	rb 4096


PMEM:	rb 256

bufferfinfo	rb 40
scin_height	rd 1

D_END: