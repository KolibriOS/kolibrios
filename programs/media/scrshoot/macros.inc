;--- другие макросы ---
include '..\..\develop\examples\editbox\trunk\editbox.inc'
include 'txtbut.inc'
;include 'gp.inc'
include 'label.inc'
;include '..\..\develop\examples\checkbox\trunk\checkbox.inc'
include 'check.inc'
include 'file_sys.inc'
include 'textwork.inc'
;include 'ini.inc'

__CPU_type	equ	p5
SYSENTER_VAR	equ	0


macro movr reg,val
{
if val < 65536
	push	val
	pop	reg
else
	mov	reg,val
end if
}

macro mcall a,b,c,d,e,f {   ; mike.dld, updated by Ghost for Fast System Calls
 local	..ret_point
 __mov eax,a
 __mov ebx,b
 __mov ecx,c
 __mov edx,d
 __mov esi,e
 __mov edi,f

 if __CPU_type eq p5
	int	0x40
 else
  if __CPU_type eq p6
	push	ebp
	mov	ebp, esp
	push	..ret_point	; it may be 2 or 5 byte
	sysenter
 ..ret_point:
	pop	edx
	pop	ecx

  else
   if __CPU_type eq k6
	push	ecx
	syscall
	pop	ecx
   else
	display 'ERROR : unknown CPU type'
	int	0x40
   end if
  end if
 end if
}

macro __mov reg,a,b {	    ; mike.dld
 if (~a eq)&(~b eq)
   mpack reg,a,b
 else if (~a eq)&(b eq)
   mov reg,a
 end if
}


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
	;mov     eax,2
	push	2
	pop	eax
	int	0x40
}

;--- получить код нажатой кнопки ---
macro get_pressed_button
{
	;mov     eax,17
	push	17
	pop	eax
	int	0x40
}

;--- сообщить системе о начале перерисовки окна ---
macro start_draw_window
{
	;mov     eax,12
	push	12
	pop	eax
	xor	ebx,ebx
	inc	ebx
	int	0x40
}

;--- сообщить системе о завершении перерисовки окна ---
macro stop_draw_window
{
	;mov     eax,12
	push	12
	pop	eax
	;mov     ebx,2
	push	2
	pop	ebx
	int	0x40
}

;--- установить маску ожидаемых событий ---
macro set_events_mask mask
{
	;mov     eax,40
	push	40
	pop	eax
	;mov     ebx,mask
	push	mask
	pop	ebx
	int	0x40
}

;--- получить информацию о процессе ---
macro get_procinfo proc_inf_buf,slot_num
{
	;mov     eax,9
	push	9
	pop	eax
	;mov     ebx,proc_inf_buf
	push	proc_inf_buf
	pop	ebx
if <slot_num> eq <>
	xor	ecx,ecx
	dec	ecx
else
	;mov     ecx,slot_num
	push	slot_num
	pop	ecx
end if
	int	0x40
}

macro get_sys_colors col_buf
{
	;mov     eax,48
	push	48
	pop	eax
	;mov     ebx,3
	push	3
	pop	ebx
	;mov     ecx,col_buf
	push	col_buf
	pop	ecx
	;mov     edx,40
	push	40
	pop	edx
	int	0x40
}

macro get_grab_area
{
	movr	 eax,48
	movr	 ebx,7
	int	0x40
}

macro get_skin_height
{
	movr	 eax,48
	movr	 ebx,4
	int	0x40
}

macro min_window
{
	movr	 eax,18
	movr	 ebx,10
	int	0x40
}

macro activ_window slot_n
{
	movr	 eax,18
	movr	 ebx,3
if <slot_n> eq <>
else
	movr	 ecx,slot_n
end if
	int	0x40
}

macro get_active_window
{
	movr	 eax,18
	movr	 ebx,7
	int	0x40
}

macro delay time
{
	movr	 eax,5
if <time> eq <>
else
	movr	 ebx,time
end if
	int	0x40
}

;--- ожидать события ---
macro wait_event redraw,key,button,mouse,ipc,other
{
	movr	 eax,10
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
	movr	 eax,14
	int	0x40
}

macro get_screen_prop  struc_ptr
{
	movr	 eax,61
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
	movr	 eax,64
	xor	ebx,ebx
	inc	ebx
if <mem_size> eq <>
else
	movr	 ecx,mem_size
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

;==============================================================================
;============== Константы =====================================================
;==============================================================================
