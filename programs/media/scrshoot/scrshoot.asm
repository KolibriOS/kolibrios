; Автор программы Евтихов Максим (Maxxxx32)
; 01.02.07 - обновлён editbox
; 31.01.07 - всё теперь рисуется относительно клиентской области
macro draw_status text_ptr
{
	mov	[status.text],dword text_ptr
	call	send_draw_status
}


title equ 'Screenshooter v 0.9' ; Заголовок окна
include 'macros.inc'  ; вставляем макросы
	meos_header  cmdstr ; вставляем заголовок программы
include 'scrwin.inc'  ; вставляем код окна прдпросмотра
include 'scrsavef.inc'; вставляем процедуру сохранения файла
	use_edit_box   ; \
	use_txt_button ;  |
	use_label      ;  |-- GUI компоненты и некоторые процедуры
	use_check_box  ;  |
	use_text_work  ; /
include 'gp.inc'
include 'srectwin.inc'
include 'lang.inc'
;--- начало программы ---
	app_start
	;mov     al,[gs:1280*4*1024]
		    ; устанавливаем ipc буффер
	push	60
	pop	eax
	;mov     ebx,1
	xor	ebx,ebx
	inc	ebx
	mov	ecx,app_ipc
	mov	edx,32
	int	0x40

	push	66   ; устанавливаем глобальную горячую клавишу
	pop	eax
	push	eax
	mov	ebx,4
	mov	cl,55	 ; 55 - PrintScrn
	xor	edx,edx
	int	0x40

	mov	ebx,app
	call	get_slot_n
	mov	[slot_n],ecx

	; устанавливаем маску событий
	set_events_mask (evm_redraw+evm_key+evm_button+evm_mouse+evm_ipc)

; сюда прыгаем при каждой перерисовке
red:
	get_procinfo app   ; получаем информацию о процессе
	get_sys_colors sc  ; получаем системные цвета

	; устанавливаем системные цвета у GUI компонентов
	txt_but_set_sys_color buttons,buttons_end,sc		 ; \
	labels_set_sys_color labels,labels_end,sc		 ; |
	check_boxes_set_sys_color check_boxes,check_boxes_end,sc ; |
	edit_boxes_set_sys_color editboxes,editboxes_end,sc	 ; /

	get_screen_prop scr ; получаем информацию об экране
	call	draw_window ; перерисовываем окно
still:
	wait_event red,key,button,mouse,ipc,still ; ждем событий

key:
	get_key
	cmp	al,2
	jne	@f
	cmp	ah,55
	jne	@f
	call	shoot
	jmp	still
@@:
	key_edit_boxes editboxes,editboxes_end
	jmp	still
button:
	get_pressed_button
	cmp	ah,1
	je	p_close
	txt_but_ev buttons,buttons_end
	jmp	still
mouse:
	get_active_window
	cmp	eax,[slot_n]
	jne	still
	mouse_check_boxes check_boxes,check_boxes_end
	mouse_edit_boxes editboxes,editboxes_end
	jmp	still
ipc:
	cmp	word [app_ipc+8],2
	jne	@f
	min_window
	jmp	.clear_ipc
@@:

	call	draw_number
	call	dr_st

.clear_ipc:
	cld
	xor	eax,eax
	movr	 ecx,32
	mov	edi,app_ipc
	rep	stosb
	jmp	still

p_close:
	btr	dword [flags],1

	bt	dword [flags],3
	jnc	@f
	movr	eax,18
	mov	ebx,eax
	mov	ecx,[set_rect_window_pid]
	int	0x40
@@:

close:
	app_close

;--------------------------------------------------------------------
;--- прооцедура перерисовки главного окна ---------------------------
;--------------------------------------------------------------------
draw_window:
start_draw_window	 ; начало перерисовки
	; определяем окно
	get_skin_height
	mov	ebx,100*65536+320
	mov	ecx,100*65536+220
	add	cx,ax
	mov	edx,[sc.work]
	add	edx,0x33000000;0x33000000
	xor	esi,esi
	;xor     edi,edi
	mov	edi,grab_text
	xor	eax,eax
	int	0x40

	movr	 eax,47 	    ; выводим числа в окно
	mov	bx,4
	shl	ebx,16
	movsx	ecx,word [scr.width] ; ширина экрана
	mov	dx,95
	shl	edx,16
	mov	dx,[label1.top]
	push	dx
	mov	esi,[sc.work_text]
	int	0x40
	movsx	ecx,word [scr.height]	; высота экрана
	mov	dx,[label2.top]
	int	0x40
	mov	bx,2
	shl	ebx,16
	movsx	ecx,word [scr.bitspp]	; бит на пиксель
	mov	dx,240
	shl	edx,16
	pop	dx
	int	0x40
	mov	bx,6
	shl	ebx,16
	mov	ecx,[scr.bytesps]	; байт на строку
	mov	dx,[label2.top]
	int	0x40

	call	draw_number	 ; прорисовываем номер снимка
	call	dr_st

	draw_labels labels,labels_end		 ; метки
	draw_edit_boxes editboxes,editboxes_end  ; edit_box
	draw_txt_buttons buttons,buttons_end	 ; кнопки
	draw_check_boxes check_boxes,check_boxes_end ; флажки
stop_draw_window      ; конец перерисовки
ret

shoot:
	bt	dword [ch4.flags],1   ; включена ли задержка ?
	jnc	@f
	draw_status delay_now
	mov	edi,ed_buffer.2
	call	zstr_to_int
	mov	ebx,eax
	delay
@@:
	call	get_mem_for_shoot; получаем память для снимка


	bts	dword [flags],0       ; нулевой флаг - снимок есть в памяти

	; делаем снимок
	draw_status shooting
	mov	esi,scr
	mov	edi,[scr_buf.ptr]
	call	copy_screen_to_image

	bt	dword [use_rect.flags],1
	jc	.use_rect
	push	dword [scr]
	jmp	@f
.use_rect:
	push	dword [rect.height]
@@:
	pop	dword [scr_buf.size]

	bt	dword [ch5.flags],1  ; включено ли автосохранение ?
	jnc	@f
	call	save_file
@@:
	draw_status	shooted_ok

	bt	dword [flags],1
	jnc	@f
	ret
@@:

	bt	dword [ch2.flags],1  ; показать окно предпросмотра ?
	jnc	@f
	call	show_scr_window
@@:
	bt	word [ch3.flags],1   ; восстановить окно ?
	jnc	@f

	delay 100
	mov	ecx,[slot_n]
	activ_window
@@:
ret

;--- получить память для снимка ---
get_mem_for_shoot:
	mov	[scr_buf.ptr],dword __app_end
	bt	dword [use_rect.flags],1 ; фоткать область экрана
	jc	.use_area
	movzx	ecx, word [scr.width]
	movzx	ebx, word [scr.height]
	jmp	@f
.use_area:
	call	read_rect
	movzx	ecx, word [rect.width]
	movzx	ebx, word [rect.height]
@@:
	imul	ecx,ebx
	lea	ecx,[ecx*3]
	add	ecx,__app_end
	mov	[scr_buf.end_ptr],ecx
	xor	ebx,ebx
	inc	ebx
	mov	eax,64
	int	0x40
ret

;--- содать окно проедпросмотра ---
show_scr_window:
pusha
	bt	dword [flags],0
	jnc	@f
	mov	eax,51
	xor	ebx,ebx
	inc	ebx
	movr	 ecx,scr_window
	mov	edx,__app_end-1000
	int	0x40
@@:
popa
ret

;--- прочитать и нарисовать текущий номер ---
apply_number:
	mov	edi,ed_buffer.3
	call	zstr_to_int
	mov	[cur_number],eax
	call	draw_number
ret

;--- процедура прорисовки текущего номера ---
draw_number:
	mov	edi,sign_n_input
	call	zstr_to_int
	mov	[sign_n],al
	movr	 eax,13
	mov	ebx,150*65536+96
	mov	cx,[label9.top]
	shl	ecx,16
	mov	cx,10
	mov	edx,[sc.work]
	int	0x40
	movr	 eax,47
	movsx	bx,byte [sign_n]
	shl	ebx,16
	mov	ecx,[cur_number]
	mov	dx,150
	shl	edx,16
	mov	dx,[label9.top]
	mov	esi,[sc.work_text]
	int	0x40
ret

;--- процедура, запускающая поток автосъемки ---
start_autoshoot:
	bts	dword [flags],1
	jc	@f
	movr	 eax,51
	;mov     ebx,1
	xor	ebx,ebx
	inc	ebx
	mov	ecx,autoshoot
	mov	edx,__app_end-512
	int	0x40
@@:
ret

;--- останавливает этот поток ---
stop_autoshoot:
	btr	dword [flags],1
ret

;--- цикл потока автосъемки ---
autoshoot:
	mov	ecx,[slot_n]
	activ_window
.next:
	bt	dword [flags],1
	jnc	close
	movr	 eax,60
	movr	 ebx,2
	mov	ecx,[app.pid]
	mov	edx,messages.draw_number
	mov	esi,2
	int	0x40
	call	shoot
	jmp	autoshoot.next

;--- процедура прорисовки строки состояния ---
; (должна вызываться потоком главного окна)
dr_st:
	movr	 eax,38 	     ; отрезок
	mov	ebx,0*65536+310
	mov	ecx,198*65536+198
	mov	edx,[sc.work_graph]
	int	0x40

	movr	 eax,13 	     ; полоска
	mov	bx,310
	mov	ecx,199*65536+15
	mov	edx,[sc.work]
	int	0x40

	mov	edi,status
	call	draw_label
ret

;--- процедура получения номера слота потока ---
; вход ebx - буффер 1024 байт
; выход ecx - номер слота
get_slot_n:
	movr	 eax,9
	xor	ecx,ecx
	dec	ecx
	int	0x40

	mov	edx,[ebx+30]
	xor	ecx,ecx
@@:
	movr	 eax,9
	inc	ecx
	int	0x40
	cmp	[ebx+30],edx
	je	@f
	jmp	@b
@@:
ret

;--- процедура, запускающая поток, делающий 1 снимок ---
one_shoot:
	mov	ecx,one_shoot_thread
	mov	edx,shoot_esp
	jmp	@f
;--- процедра, запускающая поток, сохраняющий снимок ---
save_shoot:
	mov	ecx,save_shoot_thread
	mov	edx,shoot_esp
@@:
	bts	dword [flags],2
	jc	.running
	bt	dword [flags],1
	jc	.running

	movr	 eax,51
	;mov     ebx,1
	xor	ebx,ebx
	inc	ebx
	int	0x40
.running:
ret

;--- поток, делающий 1 снимок ---
one_shoot_thread:
	mov	ecx,[slot_n]
	activ_window
	bt	dword [ch1.flags],1   ; минимизировать окно ?
	jnc	 @f
	movr	 eax,60
	movr	 ebx,2
	mov	ecx,[app.pid]
	mov	edx,messages.min_window
	mov	esi,2
	int	0x40
@@:
	call	shoot
	btr	dword [flags],2
	jmp	close

;--- процедура, отправляющая главному окну сообщение о перерисовке
; строки состояния ---
send_draw_status:
	movr	 eax,60
	movr	 ebx,2
	mov	ecx,[app.pid]
	mov	edx,messages.draw_status
	mov	esi,2
	int	0x40
ret

;--- поток, сохраняюий файл ---
save_shoot_thread:
	mov	ecx,[slot_n]
	activ_window
	call	save_file
	btr	dword [flags],2
	jmp	close

;--- процедура, запускающая поток окна установки области съемки ---
show_set_rect_window:
	bts	dword [flags],3
	jc	@f
	movr	 eax,51
	xor	ebx,ebx
	inc	ebx
	mov	ecx,set_rect_window
	mov	edx,set_rect_window_esp
	int	0x40

	mov	[set_rect_window_pid],eax
ret

@@:
	movr	eax,18
	mov	ebx,eax
	mov	ecx,[set_rect_window_pid]
	int	0x40
	btr	dword [flags],3
ret

;--- получение информации об активном окне ---
get_active_window_info:
	movr	eax,18
	movr	ebx,7
	int	0x40

	mov	ecx,eax
	movr	eax,9
	mov	ebx,active_app
	int	0x40
ret

;====================================================================
;=== данные программы ===============================================
;====================================================================
messages:
.draw_number dw 0
.draw_status dw 1
.min_window  dw 2


grab_text:
	db	title,0

labels:
label1 label 5,5,0,text.1   ; ширина экрана
label2 label 5,15,0,text.2   ; высота экрана
label3 label 5,25,0,text.3   ; введите имя файла
label4 label 150,5,0,text.4  ; бит на пиксель
label5 label 150,15,0,text.5  ; байт на строку
label6 label 115,138,0,text.6  ; 100 = 1 сек.
;label7 label 10,190,0,text.7
;label8 label 10,225,0,text.8
label9 label 5,52,0,text.9   ; номер текущего симка
label10 label 5,185,0,text.10
status label 5,201,0,no_shoot
labels_end:

editboxes:
edit1 edit_box 300,5,35,cl_white,0,0,0,1024,ed_buffer.1,ed_focus	 ; путь к файлу
edit2 edit_box 35,75,134,cl_white,0,0,0,9,ed_buffer.2,ed_figure_only	 ; задержка
edit3 edit_box 35,165,164,cl_white,0,0,0,9,ed_buffer.3,ed_figure_only
edit4 edit_box 16,165,181,cl_white,0,0,0,1,sign_n_input,ed_figure_only,1
editboxes_end:

buttons:
but1 txt_button 150,5,15,65,2,0,0,but_text.1,one_shoot		  ; сделать снимок
but2 txt_button 145,160,15,65,3,0,0,but_text.2,save_shoot	   ; сохранить снимок
but3 txt_button 140,115,12,120,4,0,0,but_text.3,show_scr_window    ; показать снимок
but4 txt_button 80,205,15,163,5,0,0,but_text.4,apply_number	   ; применить номер
but5 txt_button 150,5,15,85,6,0,0,but_text.5,start_autoshoot	 ; начать автосъёмку
but6 txt_button 145,160,15,85,7,0,0,but_text.6,stop_autoshoot	  ; остановить автосъёмку
but7 txt_button 40,205,10,150,8,0,0,but_text.7,show_set_rect_window ; задать область
buttons_end:

check_boxes:
ch1 check_box 5,105,cl_white,0,0,ch_text.1,(ch_text.2-ch_text.1)  ; свернуть окно
ch2 check_box 5,120,cl_white,0,0,ch_text.2,(ch_text.3-ch_text.2)  ; затем сделать активным
ch3 check_box 145,105,cl_white,0,0,ch_text.3,(ch_text.4-ch_text.3) ; показать снимок
ch4 check_box 5,135,cl_white,0,0,ch_text.4,(ch_text.5-ch_text.4)  ;
ch5 check_box 5,150,cl_white,0,0,ch_text.5,(ch_text.6-ch_text.5)
ch6 check_box 5,165,cl_white,0,0,ch_text.6,(ch_text.7-ch_text.6)
use_rect check_box 145,150,cl_white,0,0,ch_text.7,(ch_text.8-ch_text.7) ; исп. область
; автонумерация
check_boxes_end:

if lang eq ru
text:
.2 db 'Высота экрана:',0
.1 db 'Ширина экрана:',0
.3 db 'Введите полный путь к файлу:',0
.4 db 'Бит на пиксель:',0
.5 db 'Байт на строку:',0
.6 db '100 = 1 секунда',0
;.7: db 'Введите имя файла:',0
;.8: db 'Вместо "*" в имени файла будет вставляться номер.',0
.9 db 'Номер текущего снимка:',0
.10 db 'Количество знаков в номере:',0

but_text:
.1 db 'Сделать снимок экрана',0
.2 db 'Сохранить снимок экрана',0
.3 db 'Показать снимок сейчас',0
.4 db 'Применить',0
.5 db 'Начать автосъемку',0
.6 db 'Остановить автосъемку',0
.7 db 'задать',0

ch_text:
.1 db 'Свернуть окно';,0
.2 db 'Показать снимок';,0
.3 db 'затем сделать активным';,0
.4 db 'Задержка:';,0
.5 db 'Автосохранение';,0
.6 db 'Автонумерация, начиная с';,0
.7 db 'Область'
.8:

no_shoot db 'Снимок не сделан',0
shooting db 'Фотографирование...',0
shooted_ok db 'Снимок сделан',0
saving db 'Сохранение...',0
saved_ok db 'Снимок сохранен',0
delay_now db 'Задержка...',0
bad_file_name db 'Имя файла введено неверно',0
disk_filled db 'Диск заполнен',0
bad_fat_table db 'Таблица FAT разрушена',0
ac_den db 'Доступ запрещен',0
device_er db 'Ошибка устройства',0
not_shooted db 'Сделайте снимок !!!',0
no_file_name db 'Введите имя файла !!!',0
invalid_rect db 'Недопустимые размеры области',0

else
text:
.2 db 'Screen width:',0
.1 db 'Screen height:',0
.3 db 'Enter full path to the file:',0
.4 db 'Bits per pixel:',0
.5 db 'Bytes per line:',0
.6 db '100 = 1 second',0
;.7: db 'Enter name of file:',0
;.8: db 'Instead of "*" in filename the number will be included.',0
.9 db 'Current photo number:',0
.10 db 'Amount of digits in number:',0

but_text:
.1 db 'Make screen photo',0
.2 db 'Save screen photo',0
.3 db 'Show photo now',0
.4 db 'Apply',0
.5 db 'Start autoshooting',0
.6 db 'Stop autoshooting',0
.7 db 'Set',0

ch_text:
.1 db 'Minimize window';,0
.2 db 'Show photo';,0
.3 db 'then make active';,0
.4 db 'Delay:';,0
.5 db 'Autosave';,0
.6 db 'Start numeration from';,0
.7 db 'Area'
.8:

no_shoot db 'There is no photo',0
shooting db 'Photographing...',0
shooted_ok db 'Photo created',0
saving db 'Saving...',0
saved_ok db 'Photo saved',0
delay_now db 'Delay...',0
bad_file_name db 'File name is wrong',0
disk_filled db 'Disk is full',0
bad_fat_table db 'FAT table destroyed',0
ac_den db 'Access denied',0
device_er db 'Device error',0
not_shooted db 'Make photo !!!',0
no_file_name db 'Enter file name !!!',0
invalid_rect db 'Wrong area size',0

end if

sign_n_input:
	db	'2',0

app_ipc ipc_buffer 32

mouse_flag: dd 0x0
structure_of_potock:
rb 100

cur_number	  dd	  ?

sign_n	      db      ?

slot_n	      dd ?

flags	     dd ?

scr_buf:
.ptr	    dd	    ?
.end_ptr	dd	?
.size:
.height        dw      ?
.width	      dw      ?

fs_struc:
.funk_n        dd      ?
	dd	?
	dd	?
.bytes_to_write  dd	 ?
.data_ptr	 dd    ?,?
.fn_ptr        dd      ?

sf_buf:
.bmp_header   dd      ?
.bmp_area     dd      ?
.end	      dd      ?

set_rect_window_pid dd ?
set_rect_window_slot dd ?

rect_input_buffer:
.left rb 6
.top  rb 6
.width	rb 6
.height rb 6

cmdstr rb 257

ed_buffer:
.1: rb 1025
.2: rb 10
.3: rb 10

file_name:
	rb	1058

scr screen_prop
rect:
.left dw ?
.top dw ?
.height dw ?
.width dw ?


sc sys_color_table
app procinfo	    ; информация о главном окне
active_app procinfo ; информация об активном окне
set_rect_window_procinfo procinfo  ; информация об окне области
shoot_esp rb 512		   ; стек потока фотканья
set_rect_window_esp rb 512	   ; стек окна области
	app_end    ; конец программы