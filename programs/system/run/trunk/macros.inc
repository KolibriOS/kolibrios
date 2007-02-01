;--- другие макросы ---
include 'editbox.inc'
include 'txtbut.inc'
;include 'gp.inc'
;include 'label.inc'
;include 'checkbox.inc'
;include 'file_sys.inc'
;include 'textwork.inc'
;include 'ini.inc'

lang fix ru;en

;--- заголовок ---
macro meos_header par_buf,cur_dir_buf
{
	use32
	org	0x0

	db	'MENUET01'
	dd	0x01
	dd	__app_start
	dd	__app_end
	dd	__app_end
	dd	__app_end
if <par_buf> eq <>
	dd	0x0
else
	dd	par_buf
end if
if <cur_dir_buf> eq <>
	dd	0x0
else
	dd	cur_dir_buf
end if
}

;--- определить точку входа ---
macro app_start
{
__app_start:
}

;--- определить конец приложения и размер стека ---
macro app_end stack_size
{
if <stack_size> eq <>
	rb	1024
else
	rb	stack_size
end if
__app_end:
}

;--- завершить текущий поток ---
macro app_close
{
	xor	eax,eax
	dec	eax
	int	0x40
}

;--- получить код нажатой клавиши ---
macro get_key
{
	mov	eax,2
	int	0x40
}

;--- получить код нажатой кнопки ---
macro get_pressed_button
{
	mov	eax,17
	int	0x40
}

;--- сообщить системе о начале перерисовки окна ---
macro start_draw_window
{
	mov	eax,12
	xor	ebx,ebx
	inc	ebx
	int	0x40
}

;--- сообщить системе о завершении перерисовки окна ---
macro stop_draw_window
{
	mov	eax,12
	mov	ebx,2
	int	0x40
}

;--- установить маску ожидаемых событий ---
macro set_events_mask mask
{
	mov	eax,40
	mov	ebx,mask
	int	0x40
}

;--- получить информацию о процессе ---
macro get_procinfo proc_inf_buf,slot_num
{
	mov	eax,9
	mov	ebx,proc_inf_buf
if <slot_num> eq <>
	xor	ecx,ecx
	dec	ecx
else
	mov	ecx,slot_num
end if
	int	0x40
}

macro get_sys_colors col_buf
{
	mov	eax,48
	mov	ebx,3
	mov	ecx,col_buf
	mov	edx,40
	int	0x40
}

macro get_grab_area
{
	mov	eax,48
	mov	ebx,7
	int	0x40
}

macro get_skin_height
{
	mov	eax,48
	mov	ebx,4
	int	0x40
}

macro min_window
{
	mov	eax,18
	mov	ebx,10
	int	0x40
}

macro activ_window slot_n
{
	mov	eax,18
	mov	ebx,3
if <slot_n> eq <>
else
	mov	ecx,slot_n
end if
	int	0x40
}

macro get_active_window
{
	mov	eax,18
	mov	ebx,7
	int	0x40
}

macro delay time
{
	mov	eax,5
if <time> eq <>
else
	mov	ebx,time
end if
	int	0x40
}

;--- ожидать события ---
macro wait_event redraw,key,button,mouse,ipc,other
{
	mov	eax,10
	int	0x40
	dec	ax
if <redraw> eq <>
else
	jz	redraw
end if
	dec	ax
if <key> eq <>
else
	jz	key
end if
	dec	ax
if <button> eq <>
else
	jz	button
end if
	dec	ax
	dec	ax
	dec	ax
if <mouse> eq <>
else
	jz	mouse
end if
if <ipc> eq <>
else
	dec	ax
	jz	ipc
end if
if <other> eq <>
	jmp	still
else
	jmp	other
end if
}

;--- получить размеры экрана ---
macro get_screen_size
{
	mov	eax,14
	int	0x40
}

macro get_screen_prop  struc_ptr
{
	mov	eax,61
	push	eax eax
	xor	ebx,ebx
	inc	ebx
	int	0x40
	mov	[struc_ptr],eax
	pop	eax
	inc	ebx
	int	0x40
	mov	[struc_ptr+4],ax
	pop	eax
	inc	ebx
	int	0x40
	mov	[struc_ptr+6],eax
}

macro resize_mem mem_size
{
	mov	eax,64
	xor	ebx,ebx
	inc	ebx
if <mem_size> eq <>
else
	mov	ecx,mem_size
end if
	int	0x40
}

evm_redraw equ 1
evm_key equ 10b
evm_button equ 100b
evm_mouse equ 100000b
evm_ipc equ 1000000b

struc procinfo
{
.takts_per_second: dd ?
.window_stack_pos: dw ?
.slot_number: dw ?
dw ?
.name: rb 11
.align: db ?
.addres: dd ?
.use_mem: dd ?
.pid: dd ?
.left: dd ?
.top: dd ?
.width: dd ?
.height: dd ?
.slot_state: dw ?
rb (1024-56)
}

struc sys_color_table
{
.frames: dd ?  ;+0
.grab: dd ?    ;+4
.grab_button: dd ? ;+8
.grab_button_text: dd ?  ;+12
.grab_text: dd ?  ;+16
.work: dd ?	  ;+20
.work_button: dd ?;+24
.work_button_text: dd ? ;+28
.work_text: dd ?	;+32
.work_graph: dd ?	;+36
}

struc screen_size
{
.height: dw ?
.width: dw ?
}

struc screen_prop
{
.height: dw ? ;+0
.width: dw ?  ;+2
.bitspp: dw ? ;+4
.bytesps: dd ?;+6
}

struc ipc_buffer size
{
.block: dd ?
.in_use: dd ?
.messages: rb size
}

;--- цвета ---
cl_white=0xffffff
cl_black=0x000000