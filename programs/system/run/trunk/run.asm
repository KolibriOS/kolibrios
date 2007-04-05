; 01.02.07 - обновлён editbox
; 31.01.07 - исправлена некорректная отрисовка при большом значении высоты скина
;            выравнивание снизу относительно рабочей области экрана
window_y=67
;window_x=320
window_x=640
include 'macros.inc'
include 'lang.inc'
	meos_header par
	use_edit_box
	use_txt_button
	app_start
	cmp	[par],byte 0
	jne	read_par
	set_events_mask evm_mouse+evm_button+evm_key+evm_redraw
red:
	get_sys_colors sc
	set_sys_colors_txt_button run_but,sc
	push	dword [sc.work_graph]
	pop	[input_fn.focus_border_color]
	call	draw_window
still:
	wait_event red,key,button,mouse,,still
key:
	get_key
	cmp	ah,13
	je	run
	key_edit_box input_fn
	jmp	still
button:
	get_pressed_button
	dec	ah
	jz	close
	dec	ah
	jz	run
	jmp	still
mouse:
	mouse_edit_box input_fn
	jmp	still

read_par:
	mov	esi,par
	mov	edi,fn
	mov	ecx,256
	rep	movsb
run:
	xor	eax,eax
	mov	edi,file_info.name
	mov	ecx,512
	rep	stosb
	mov	edi,run_par
	mov	ecx,256
	rep	stosb

	mov	esi,fn
	mov	edi,file_info.name
	cmp	[esi],byte '"'
	je	copy_fn_with_spaces
copy_fn:
	cmp	[esi],byte ' '
	je	.stop
	cmp	[esi],byte 0
	je	.stop
	mov	al,[esi]
	mov	[edi],al
	inc	esi
	inc	edi
	jmp	copy_fn
.stop:

	jmp	copy_par

copy_fn_with_spaces:
	inc	esi
@@:
	cmp	[esi],byte '"'
	je	.stop
	cmp	[esi],byte 0
	je	.stop
	mov	al,[esi]
	mov	[edi],al
	inc	esi
	inc	edi
	jmp	@b
.stop:

copy_par:
@@:
	inc	esi
	cmp	[esi],byte ' '
	je	@b
	mov	edi,run_par
@@:
	cmp	[esi],byte 0
	je	.stop
	mov	al,[esi]
	mov	[edi],al
	inc	esi
	inc	edi
	jmp	@b
.stop:

	mov	eax,70
	mov	ebx,file_info
	int	0x40

	cmp	eax,0
	jl	error
	mov	[status],run_ok
	call	draw_status
	jmp	still
close:
	app_close

error:

macro cmp_err code,text_ptr
{
	cmp	al,code
	jne	@f
	mov	[status],text_ptr
@@:
}
	neg	eax

	cmp_err 3,bad_file_sys

	cmp_err 5,file_not_find

	cmp_err 9,bad_fat_table

	cmp_err 10,acces_denyied

	cmp_err 11,device_error

	cmp_err 30,out_of_memory

	cmp_err 31,file_not_executable

	cmp_err 32,many_processes


	call	draw_status
	jmp	still

draw_window:
	push	48
	pop	eax
	mov	ebx,5
	int	0x40
	mov	si,bx
	start_draw_window
	get_skin_height
	mov	dx,ax
	get_screen_size
	xor	ecx,ecx
	sub	cx,window_y+2
	sub	cx,dx
	add	cx,si
	shl	ecx,16
	mov	cx,dx
	add	cx,window_y
	shr	eax,16
	mov	bx,ax
	sub	bx,window_x
	shl	ebx,15
	mov	bx,window_x
	mov	edx,[sc.work]
	or	edx,0x33000000
	xor	eax,eax
	xor	esi,esi
	mov	edi,grab_text
	int	0x40

	get_procinfo app

	mov	eax,[app.width]
	sub	eax,20
	mov	[input_fn.width],eax
	mov	[run_but.width],ax

	xor	bx,bx
	shl	ebx,16
	mov	bx,ax
	add	bx,10
	mov	cx,45
	push	cx
	shl	ecx,16
	pop	cx
	mov	edx,[sc.work_graph]
	mov	eax,38
	int	0x40

	draw_edit_box input_fn
	draw_txt_button run_but

	call	draw_status_text

	stop_draw_window
ret

draw_status:
	mov	ebx,[app.width]
	sub	bx,10
	mov	ecx,(50)*65536+12
	mov	edx,[sc.work]
	mov	eax,13
	int	0x40
draw_status_text:
	mov	edx,[status]
	xor	esi,esi
@@:
	cmp	[edx+esi],byte 0
	je	@f
	inc	esi
	jmp	@b
@@:
	mov	ebx,5*65536+(50)
	mov	ecx,[sc.work_text]
	mov	eax,4
	int	0x40
ret

run_but txt_button 0,5,15,25,2,0,0,run_but_text,
input_fn edit_box 0,5,5,0xffffff,0,0xaaaaaa,0,511,fn,ed_focus+\
ed_always_focus

if lang eq ru
hello db 'Введите полный путь к файлу и нажмите Enter',0
bad_file_sys db 'Неизвестная файловая система',0 ; 3
file_not_find db 'Файл не найден',0		 ; 5
bad_fat_table db 'Таблица FAT разрушена',0	 ; 9
acces_denyied db 'Доступ запрещен',0		 ; 10
device_error db 'Ошибка устройства',0		 ; 11
out_of_memory db 'Недостаточно памяти',0	 ; 30
file_not_executable db 'Файл не является исполняемым',0 ; 31
many_processes db 'Слишком много процессов',0	 ; 32
run_ok db 'Программа успешно запущена',0
grab_text db 'ЗАПУСК ПРОГРАММЫ',0
run_but_text db 'ЗАПУСТИТЬ',0
else
hello db 'Enter full path to file and press <Enter>',0
bad_file_sys db 'Unknown file system',0                ; 3
file_not_find db 'File not found',0                    ; 5
bad_fat_table db 'FAT table corrupted',0               ; 9
acces_denyied db 'Access denied',0                     ; 10
device_error db 'Device error',0                       ; 11
out_of_memory db 'Out of memory',0                     ; 30
file_not_executable db 'File is not executable',0      ; 31
many_processes db 'Too many processes',0               ; 32
run_ok db 'The program was started successfully',0
grab_text db 'RUN',0
run_but_text db 'RUN',0
end if

mouse_flag: dd 0x0

status dd hello

file_info:
.mode dd 7
.flags dd 0
.par dd run_par
dd 0,0
.name rb 512

flags dw ?

structure_of_potock:
rb 100

fn rb 512

sc sys_color_table
app procinfo
run_par rb 256
par rb 256
	app_end