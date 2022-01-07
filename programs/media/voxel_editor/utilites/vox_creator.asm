use32
	org 0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 1,start,i_end,mem,stacktop,openfile_path,sys_path

include '../../../macros.inc'
include '../../../proc32.inc'
include '../../../KOSfuncs.inc'
include '../../../load_img.inc'
include '../../../load_lib.mac'
include '../trunk/str.inc'
include 'lang.inc'

vox_offs_tree_table equ 4
vox_offs_data equ 12
txt_buf rb 8
include '../trunk/vox_rotate.inc'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
if lang eq ru
caption db 'Создатель вокселей 04.05.20',0 ;подпись окна
else
caption db 'Voxel creator 04.05.20',0
end if

BUF_STRUCT_SIZE equ 21
buf2d_data equ dword[edi] ;данные буфера изображения
buf2d_w equ dword[edi+8] ;ширина буфера
buf2d_h equ dword[edi+12] ;высота буфера
buf2d_l equ word[edi+4]
buf2d_t equ word[edi+6] ;отступ сверху
buf2d_size_lt equ dword[edi+4] ;отступ слева и справа для буфера
buf2d_color equ dword[edi+16] ;цвет фона буфера
buf2d_bits equ byte[edi+20] ;количество бит в 1-й точке изображения

vox_obj_size dd 0 ;размер воксельного объекта (для ускорения вставки)
txt_space db ' ',0
if lang eq ru
txt_pref db ' б ',0,' Кб',0,' Мб',0,' Гб',0 ;приставки: кило, мега, гига
txt_f_size: db 'Размер: '
else
txt_pref db ' b ',0,' Kb',0,' Mb',0,' Gb',0 ;приставки: кило, мега, гига
txt_f_size: db 'Size: '
end if
.size: rb 16

IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
image_data_toolbar dd 0

max_open_file_size equ 1024*1024 ;1 Mb

align 4
start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась библиотека
	mov	ebp,lib_0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0x27
	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога

	stdcall [buf2d_create], buf_0 ;создание буфера
	stdcall [buf2d_create], buf_0z
	stdcall [buf2d_vox_brush_create], buf_vox, vox_6_7_z

	include_image_file 'toolbar.png', image_data_toolbar

	stdcall mem.Alloc,max_open_file_size
	mov dword[open_file_vox],eax
	stdcall mem.Alloc,max_open_file_size
	mov dword[open_file_img],eax

	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax
	;проверка командной строки
	cmp dword[openfile_path],0
	je @f
		call but_open_file_cmd_lin
	@@:

align 4
red_win:
	call draw_window

align 4
still:
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov ebx,[last_time]
	add ebx,10 ;задержка
	cmp ebx,eax
	jge @f
		mov ebx,eax
	@@:
	sub ebx,eax
	mcall SF_WAIT_EVENT_TIMEOUT
	cmp eax,0
	je timer_funct

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button

	jmp still

align 4
timer_funct:
	cmp byte[calc],0
	je still

	pushad
	mcall SF_SYSTEM_GET,SSF_TIME_COUNT
	mov [last_time],eax

	; скидываем указатели буферов buf_npl_p, buf_npl, buf_npl_n
	mov edi,buf_npl_p
	mov eax,buf2d_data
	mov edi,buf_npl
	mov ebx,buf2d_data
	mov edi,buf_npl_n
	mov ecx,buf2d_data
	; change buffer data pointers
	mov buf2d_data,eax
	mov edi,buf_npl_p
	mov buf2d_data,ebx
	mov edi,buf_npl
	mov buf2d_data,ecx

	mov eax,[n_plane]
	mov ebx,buf2d_w
	dec ebx

	cmp ebx,eax
	jg @f
		stdcall create_obj_from_plane,buf_npl,eax ;создаем завершающее сечение
		;вывод конечного результата
		call draw_object
		mov byte[calc],0
		jmp .end_f
	@@:

	inc eax
	stdcall create_plane, buf_npl_n,eax

	; создание воскельного сечения на основе буфера buf_npl
	mov edi,buf_npl
	mov edx,[bby_min] ;0
	.cycle_0:
	mov ecx,[btx_min] ;0
	.cycle_1:
		stdcall [buf2d_get_pixel], edi,ecx,edx
		cmp eax,buf2d_color
		je @f
			mov esi,eax
			call need_node
			cmp eax,buf2d_color
			jne @f ;отсеиваем внутренние воксели, для оптимизации модели
			mov eax,ebx
			sub eax,edx
			stdcall buf2d_vox_obj_create_node, [open_file_vox],ecx,[n_plane],\
				eax,[k_scale],esi
		@@:
		inc ecx
		cmp ecx,ebx
		jle .cycle_1
		inc edx
		cmp edx,ebx
		jle .cycle_0

	stdcall [buf2d_draw], buf_npl
	inc dword[n_plane] ;перемещаем плоскость сечения
	call draw_pok
	.end_f:
	popad
	jmp still

align 4
proc create_plane, buf_img:dword, n_plane:dword
	pushad
	; копируем лицевое изображение в буфер buf_img
	; bby_min - используем для оптимизации (если сверху изображение пустое)
	mov eax,[bby_min]
	mov esi,buf_i2
	mov esi,[esi] ;buf2d_data
	mov edi,[buf_img]
	mov ecx,buf2d_w
	imul eax,ecx
	mov ebx,ecx
	imul ecx,ebx
	sub ecx,eax
	lea ecx,[ecx+ecx*2]
	mov edi,buf2d_data
	lea eax,[eax+eax*2]
	add edi,eax
	add esi,eax
	cld
	rep movsb

	mov ecx,ebx
	dec ebx
	mov edi,buf_i0
	.cycle_0:
		mov eax,ebx
		sub eax,[n_plane] ;eax - перевернутая координата n_plane
		stdcall [buf2d_get_pixel], edi,ecx,eax ;[n_plane]
		cmp eax,buf2d_color
		jne @f
			;вычеркивание вертикальной линии из сечения buf_img
			stdcall [buf2d_line], [buf_img],ecx,[bby_min],ecx,ebx, buf2d_color
			jmp .end_1
		@@:
			mov edx,[bby_min] ;xor edx,edx
			mov esi,eax
			.cycle_1: ;цикл для наложения верхней текстуры
			stdcall [buf2d_get_pixel], [buf_img],ecx,edx
			cmp eax, buf2d_color
			je .end_0
				stdcall [buf2d_set_pixel], [buf_img],ecx,edx, esi ;наложение верхней текстуры
				jmp .end_1
			.end_0:
			inc edx
			cmp edx,ebx
			jle .cycle_1
		.end_1:
		loop .cycle_0

	;горизонтальные линии на основе боковой грани
	mov ecx,[bby_min]
	mov edi,buf_i1
	.cycle_2:
		stdcall [buf2d_get_pixel], edi,[n_plane],ecx
		cmp eax,buf2d_color
		jne @f
			;вычеркивание горизонтальной линии из сечения buf_img
			stdcall [buf2d_line], [buf_img],[btx_min],ecx,[btx_max],ecx, buf2d_color
			jmp .end_3
		@@:
			mov edx,[btx_max] ;ebx
			mov esi,eax
			.cycle_3: ;цикл для наложения боковой текстуры
			stdcall [buf2d_get_pixel], [buf_img],edx,ecx
			cmp eax, buf2d_color
			je .end_2
				stdcall [buf2d_set_pixel], [buf_img],edx,ecx, esi ;наложение боковой текстуры
				jmp .end_3
			.end_2:
			dec edx
			cmp edx,[btx_min]
			jge .cycle_3
		.end_3:
		inc ecx
		cmp ecx,ebx
		jle .cycle_2
	popad
	ret
endp

align 4
proc create_obj_from_plane, buf_img:dword, n_plane:dword
pushad
	; создание воскельного сечения на основе буфера buf_img
	mov edi,[buf_img]
	mov ebx,buf2d_w
	dec ebx
	mov edx,[bby_min] ;0
	.cycle_0:
	mov ecx,[btx_min] ;0
	.cycle_1:
		stdcall [buf2d_get_pixel], edi,ecx,edx
		cmp eax,buf2d_color
		je @f
			mov esi,eax
			;call need_node
			;cmp eax,buf2d_color
			;jne @f ;отсеиваем внутренние воксели, для оптимизации модели
			mov eax,ebx
			sub eax,edx
			stdcall buf2d_vox_obj_create_node, [open_file_vox],ecx,[n_plane],\
				eax,[k_scale],esi
		@@:
		inc ecx
		cmp ecx,ebx
		jle .cycle_1
		inc edx
		cmp edx,ebx
		jle .cycle_0
	ret
popad
endp

;description:
; функция проверяет нужно ли отсеивать воксель с координатами [n_plane],ecx,edx
;input:
; ebx = max size y - 1
;output:
; eax = buf2d_color if node need
align 4
need_node:
	mov eax,buf2d_color
	cmp ecx,[btx_min] ;0
	jle .end_f
	cmp ecx,[btx_max] ;0
	jge .end_f
	cmp edx,[bby_min] ;0
	jle .end_f
	cmp edx,ebx ;max-1
	jge .end_f
		push ecx edx
		stdcall [buf2d_get_pixel], buf_npl_p,ecx,edx ;проверка предыдущего сечения
		cmp eax,buf2d_color
		je @f
		stdcall [buf2d_get_pixel], buf_npl_n,ecx,edx ;проверка последующего сечения
		cmp eax,buf2d_color
		je @f
		dec ecx
		stdcall [buf2d_get_pixel], edi,ecx,edx
		cmp eax,buf2d_color
		je @f
		add ecx,2
		stdcall [buf2d_get_pixel], edi,ecx,edx
		cmp eax,buf2d_color
		je @f
		dec ecx
		dec edx
		stdcall [buf2d_get_pixel], edi,ecx,edx
		cmp eax,buf2d_color
		je @f
		add edx,2
		stdcall [buf2d_get_pixel], edi,ecx,edx
		;cmp eax,buf2d_color
		;je @f
		@@:
		pop edx ecx
	.end_f:
	ret

align 4
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x30000000
	mcall SF_CREATE_WINDOW,(20 shl 16)+410,(20 shl 16)+520,,,caption

	; *** создание кнопок на панель ***
	mov esi,[sc.work_button]
	mcall SF_DEFINE_BUTTON,(5 shl 16)+20,(5 shl 16)+20,3

	mov ebx,(30 shl 16)+20
	mov edx,4
	int 0x40

	mov ebx,(55 shl 16)+20
	mov edx,5
	int 0x40

	mov ebx,(85 shl 16)+20
	mov edx,6
	int 0x40

	add ebx,25 shl 16
	mov edx,7
	int 0x40

	add ebx,25 shl 16
	mov edx,8
	int 0x40

	add ebx,25 shl 16
	mov edx,9
	int 0x40

	add ebx,25 shl 16
	mov edx,10
	int 0x40

	add ebx,25 shl 16
	mov edx,11
	int 0x40

	add ebx,25 shl 16
	mov edx,12
	int 0x40

	; *** рисование иконок на кнопках ***
	mov edx,(7 shl 16)+7 ;icon new
	mcall SF_PUT_IMAGE,[image_data_toolbar],(16 shl 16)+16

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon open
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon save
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;
	int 0x40

	call draw_buffers
	call draw_pok

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 4
draw_buffers:
	; *** рисование буфера ***
	stdcall [buf2d_draw], buf_0
	stdcall [buf2d_draw], buf_i0
	stdcall [buf2d_draw], buf_i1
	stdcall [buf2d_draw], buf_i2
	ret

align 4
draw_pok:
	;обновление подписи размера файла
	mov edi,txt_f_size.size
	mov eax,dword[vox_obj_size]
	mov ebx,txt_pref
	.cycle:
		cmp eax,1024
		jl @f
		shr eax,10
		add ebx,4
		jmp .cycle
	@@:

	stdcall convert_int_to_str, 16
	stdcall str_cat, edi,ebx
	stdcall str_cat, edi,txt_space ;завершающий пробел

	;рисование текста
	mov ecx,[sc.work_text]
	or  ecx,0x80000000 or (1 shl 30)
	mov edi,[sc.work] ;цвет фона окна
	mcall SF_DRAW_TEXT,(275 shl 16)+7,,txt_f_size

	ret

align 4
key:
	mcall SF_GET_KEY
	jmp still


align 4
button:
	mcall SF_GET_BUTTON
	cmp ah,3
	jne @f
		call but_new_file
		jmp still
	@@:
	cmp ah,4
	jne @f
		call but_open_file ;открытие воксельного файла
		jmp still
	@@:
	cmp ah,5
	jne @f
		call but_save_file
		jmp still
	@@:
	cmp ah,6
	jne @f
		call but_1
		jmp still
	@@:
	cmp ah,7
	jne @f
		call but_2
		jmp still
	@@:
	cmp ah,8
	jne @f
		call but_3
		jmp still
	@@:
	cmp ah,9
	jne @f
		stdcall but_run, 0
		jmp still
	@@:
	cmp ah,10
	jne @f
		stdcall but_run, 1
		jmp still
	@@:
	cmp ah,11
	jne @f
		call but_stop
		jmp still
	@@:
	cmp ah,12
	jne @f
		call but_rot_z
		jmp still
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall [buf2d_delete],buf_0z
	stdcall [buf2d_vox_brush_delete],buf_vox
	stdcall [buf2d_delete],buf_i0
	stdcall [buf2d_delete],buf_i1
	stdcall [buf2d_delete],buf_i2
	stdcall [buf2d_delete],buf_npl_p
	stdcall [buf2d_delete],buf_npl
	stdcall [buf2d_delete],buf_npl_n
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file_vox]
	stdcall mem.Free,[open_file_img]
	mcall SF_TERMINATE_PROCESS


align 4
vox_new_data:
	db 2,0,0,0
	db 000b,001b,010b,011b, 100b,101b,110b,111b ;default table
	dd 0 ;null node

align 4
proc but_new_file uses ecx edi esi
	mov ecx,vox_offs_data+4
	mov [vox_obj_size],ecx
	mov esi,vox_new_data
	mov edi,[open_file_vox]
	cld
	rep movsb
	ret
endp

align 4
open_file_vox dd 0 ;указатель на память для открытия файлов
open_file_img dd 0 ;указатель на память для открытия текстур

align 4
but_open_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je @f
		;код при удачном открытии диалога
		call but_open_file_cmd_lin
	@@:
popad
	ret

align 4
but_open_file_cmd_lin:
pushad
	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword max_open_file_size
	m2m [run_file_70.Buffer],dword[open_file_vox]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70
	cmp ebx,0xffffffff
	je .end_open_file
		; проверка на правильность воксельного формата
		mov edi,[open_file_vox]
		add edi,vox_offs_tree_table
		xor bx,bx
		mov ecx,8
		cld
		@@:
			movzx ax,byte[edi]
			add bx,ax
			inc edi
			loop @b
		cmp bx,28 ;28=0+1+2+...+7
		jne .err_open

		mcall SF_SET_CAPTION,1,openfile_path
		stdcall buf2d_vox_obj_get_size,[open_file_vox]
		mov [vox_obj_size],eax
		call draw_object
		
		jmp .end_open_file
	.err_open:
		call but_new_file
		stdcall [mb_create],msgbox_4,thread
	.end_open_file:
	popad
	ret

align 4
but_save_file:
	pushad
		copy_path open_dialog_name,communication_area_default_path,file_name,0
		mov [OpenDialog_data.type],1
		stdcall [OpenDialog_Start],OpenDialog_data
		cmp [OpenDialog_data.status],2
		je .end_save_file
		;код при удачном открытии диалога

		mov eax,6 ;dword[v_zoom] ;задаем масштаб по умолчанию
		mov ebx,[open_file_vox]
		mov byte[ebx],al

		stdcall buf2d_vox_obj_get_size, ebx
		mov dword[run_file_70.Count], eax ;размер файла
		mov [run_file_70.Function], SSF_CREATE_FILE
		mov [run_file_70.Position], 0
		mov [run_file_70.Flags], 0
		mov ebx, dword[open_file_vox]
		mov [run_file_70.Buffer], ebx
		mov byte[run_file_70+20], 0
		mov dword[run_file_70.FileName], openfile_path
		mcall SF_FILE,run_file_70
		cmp ebx,0xffffffff
		je .end_save_file

		.end_save_file:
	popad
	ret

align 4
but_1:
	stdcall open_image_in_buf, buf_i0
	ret

align 4
but_2:
	stdcall open_image_in_buf, buf_i1
	call set_buf_tabs
	ret

align 4
but_3:
	stdcall open_image_in_buf, buf_i2
	call set_buf_tabs
	ret

;description:
; в зависимости от ширины 1-го буфера устанавливаем отступы для 2-го и 3-го буферов
align 4
set_buf_tabs:
push eax ebx edi
	mov edi,buf_i0
	cmp buf2d_data,0
	je @f
		movzx eax,buf2d_l
		mov ebx,buf2d_w
		cmp ebx,128
		jle .end_0
			mov ebx,128
		.end_0:
		mov edi,buf_i1
		mov buf2d_l,ax
		add buf2d_l,bx
		add buf2d_l,2
		shl ebx,1
		mov edi,buf_i2
		mov buf2d_l,ax
		add buf2d_l,bx
		add buf2d_l,4
	@@:	
pop edi ebx eax
	ret

align 4
get_scale:
push eax edi
	mov edi,buf_i0
	mov eax,buf2d_w

	mov dword[k_scale],-1
	.cycle_s:
	cmp eax,0
	je @f
		shr eax,1
		inc dword[k_scale]
		jmp .cycle_s
	@@:
pop edi eax
	ret

align 4
proc set_borders uses eax ebx ecx edx edi
	mov ecx,dword[k_scale]
	mov edx,1
	shl edx,cl

	;определяем минимальную координату y на лицевой грани
	mov edi,buf_i2
	mov dword[bby_min],0
	xor ecx,ecx
	.cycle_0:
	xor ebx,ebx
	.cycle_1:
		stdcall [buf2d_get_pixel],edi,ebx,ecx
		cmp eax,buf2d_color
		jne @f
		inc ebx
		cmp ebx,edx
		jl .cycle_1
		inc dword[bby_min]
		inc ecx
		cmp ecx,edx
		jl .cycle_0
	@@:

	;определяем минимальную координату x на лицевой грани
	mov dword[btx_min],0
	xor ebx,ebx
	.cycle_2:
	mov ecx,[bby_min]
	.cycle_3:
		stdcall [buf2d_get_pixel],edi,ebx,ecx
		cmp eax,buf2d_color
		jne @f
		inc ecx
		cmp ecx,edx
		jl .cycle_3
		inc dword[btx_min]
		inc ebx
		cmp ebx,edx
		jl .cycle_2
	@@:

	;определяем максимальную координату x на лицевой грани
	mov [btx_max],edx
	dec dword[btx_max]
	mov ebx,[btx_max]
	.cycle_4:
	mov ecx,[bby_min]
	.cycle_5:
		stdcall [buf2d_get_pixel],edi,ebx,ecx
		cmp eax,buf2d_color
		jne @f
		inc ecx
		cmp ecx,edx
		jl .cycle_5
		dec dword[btx_max]
		dec ebx
		cmp ebx,[btx_min]
		jg .cycle_4
	@@:
	
	;stdcall [buf2d_line], edi, 0,[bby_min],50,[bby_min],255
	ret
endp

align 4
btx_min dd 0 ;буфер верхний мин. x
btx_max dd 0
bty_min dd 0 ;буфер верхний мин. y
bty_max dd 0
;bbx_min dd 0
;bbx_max dd 0
bby_min dd 0 ;буфер боковой мин. y
bby_max dd 0
k_scale dd 0
n_plane dd 0

calc db 0 ;если =1, то идет создание объекта

; создание вокселя в 3 этапа:
; 1) ищем место в структуре дерева, куда надо вставить (если ветвь существует, 2-й этап пропускаем)
; 2) вставляем новую ветвь с вокселем (3-й этап уже не делаем)
; 3) меняем цвет вокселя
align 4
proc buf2d_vox_obj_create_node, v_obj:dword,coord_x:dword,coord_y:dword,\
coord_z:dword,k_scale:dword,color:dword
pushad
locals
	p_node dd 0 ;родительский узел
endl

	mov edi,[v_obj]
	add edi,vox_offs_data
	mov esi,[k_scale]
	cmp esi,1
	jl .change
	; *** (1) ***
	.found:
	stdcall vox_obj_get_node_position, [v_obj],[coord_x],[coord_y],[coord_z],esi
	movzx bx,byte[edi+3]
	mov [p_node],edi
	add edi,4

	cmp eax,0
	je .end_1
	mov ecx,eax
	cld
	@@: ;цикл для пропуска предыдущих поддеревьев в узле
		bt bx,0 ;проверяем есть ли дочерние узлы
		jnc .end_0
			xor eax,eax
			stdcall vox_obj_rec0 ;в eax вычисляется число дочерних узлов, в данной ветви
		.end_0:
		shr bx,1
		loop @b
	.end_1:
	bt bx,0
	jnc .creat ;если поддерева не существует, переходим к созданию
	dec esi
	cmp esi,0
	jg .found
	jmp .change

	; *** (2) ***
	.creat:
	mov edx,[color] ;меняем цвет
	and edx,0xffffff ;для профилактики
	mov ecx,esi
	stdcall vox_obj_add_nodes_mem, [v_obj],edi,ecx ;расширяем место занимаемое объектом, для добавления новых узлов
	mov ebx,[p_node]
	cld
	@@:
		mov dword[edi],edx
		stdcall vox_obj_get_node_bit_mask, [v_obj],[coord_x],[coord_y],[coord_z],esi
		or byte[ebx+3],al

		mov ebx,edi
		add edi,4
		dec esi
		loop @b
	jmp .end_2

	; *** (3) ***
	.change:
	mov eax,[color] ;меняем цвет
	mov word[edi],ax
	shr eax,16
	mov byte[edi+2],al

	.end_2:
popad
	ret
endp

; сдвигает узлы для добавления новых узлов
;input:
; p_insert - позиция для вставки
; count - колличество вставляемых узлов
align 4
proc vox_obj_add_nodes_mem uses eax ecx edi esi, v_obj:dword,p_insert:dword,count:dword
	mov esi,[v_obj]
	;stdcall buf2d_vox_obj_get_size,esi
	add esi,[vox_obj_size] ;esi - указатель на конец файла
	mov edi,[count]
	shl edi,2
	add [vox_obj_size],edi
	add edi,esi ;edi - указатель на будущий конец файла
	mov ecx,esi
	sub ecx,[p_insert]
	shr ecx,2 ;ecx - число циклов для копирования
	sub esi,4 ;esi - указатель на последний узел
	sub edi,4 ;edi - указатель на будущий последний узел
	std
	rep movsd ;сдвигаем память
	ret
endp

;???
; определение позиции узла в дереве (от 0 до 7)
align 4
proc vox_obj_get_node_position uses ebx ecx edi, v_obj:dword,\
coord_x:dword,coord_y:dword,coord_z:dword,k_scale:dword
	mov ecx,[k_scale]
	dec ecx
	mov eax,[coord_x]
	mov ebx,[coord_y]
	mov edi,[coord_z]
	cmp ecx,1
	jl .end_0
		shr eax,cl
		shr ebx,cl
		shr edi,cl
	.end_0:
	and eax,1
	bt ebx,0
	jnc @f
		bts eax,1
	@@:
	bt edi,0
	jnc @f
		bts eax,2
	@@:

	mov edi,[v_obj]
	add edi,vox_offs_tree_table
	@@:
		cmp al,byte[edi]
		je @f
		inc edi
		jmp @b
	@@:
	sub edi,[v_obj]
	sub edi,vox_offs_tree_table
	mov eax,edi
	
	ret
endp

;???
;input:
; edi - указатель на данные воксельного объекта
;output:
; eax - eax + число узлов в данных вокс. объекта
; edi - указатель на смещенные данные вокс. объекта
align 4
proc vox_obj_rec0
	inc eax
	cmp byte[edi+3],0 ;смотрим есть ли поддеревья
	je .sub_trees

		;рекурсивный перебор поддеревьев
		push ebx ecx
		mov bh,byte[edi+3]
		add edi,4
		mov bl,8
		.cycle:
			bt bx,8 ;тестируем только bh
			jnc .c_next
				stdcall vox_obj_rec0
			.c_next:
			shr bh,1
			dec bl
			jnz .cycle
		pop ecx ebx

		jmp .end_f
	.sub_trees:
		add edi,4
	.end_f:
	ret
endp

;output:
; eax - 1,2,4,8,16, ... ,128
align 4
proc vox_obj_get_node_bit_mask uses ebx ecx edi, v_obj:dword,\
coord_x:dword,coord_y:dword,coord_z:dword,k_scale:dword
	mov ecx,[k_scale]
	dec ecx
	mov eax,[coord_x]
	mov ebx,[coord_y]
	mov edi,[coord_z]
	cmp ecx,1
	jl .end_0
		shr eax,cl
		shr ebx,cl
		shr edi,cl
	.end_0:
	and eax,1
	bt ebx,0
	jnc @f
		bts eax,1
	@@:
	bt edi,0
	jnc @f
		bts eax,2
	@@:

	mov ecx,[v_obj]
	add ecx,vox_offs_tree_table
	@@:
		cmp al,byte[ecx]
		je @f
		inc ecx
		jmp @b
	@@:
	mov eax,1 ;устанавливаем первоначальное значение бита
	sub ecx,[v_obj]
	sub ecx,vox_offs_tree_table
	jz @f
		shl eax,cl ;сдвигаем бит
	@@:
	
	ret
endp

;output:
; eax - размер в байтах занимаемый объектом v_obj
align 4
proc buf2d_vox_obj_get_size uses edi, v_obj:dword
	mov edi,[v_obj]
	add edi,vox_offs_data
	xor eax,eax
	stdcall vox_obj_rec0 ;eax - число узлов в объекте v_obj
	shl eax,2
	add eax,vox_offs_data
	ret
endp

;input:
; mode_add - если не равно 0 тогда создание в режиме добавления вокселей
align 4
proc but_run uses eax ebx edi, mode_add:dword
	; проверка размеров изображений (текстур)
	mov edi,buf_i0
	mov ebx,buf2d_h
	cmp ebx,2
	jle .err_size_t
	cmp buf2d_w,ebx
	jne .err_size_t
	;
	mov edi,buf_i1
	cmp ebx,buf2d_h
	jne .err_size_tb
	mov ebx,buf2d_h
	cmp ebx,2
	jle .err_size_b
	cmp buf2d_w,ebx
	jne .err_size_b

		cmp dword[vox_obj_size],vox_offs_data
		jl .n_file ;если раньше не было открытых файлов
		cmp dword[mode_add],0
		jne @f
		.n_file:
			call but_new_file
		@@:

		mov edi,buf_i0
		mov eax,buf2d_w
		mov edi,buf_npl
		cmp buf2d_data,0
		jne @f
			; *** создаем буфера
			m2m buf2d_w,eax
			m2m buf2d_h,eax
			stdcall [buf2d_create],edi
			mov edi,buf_npl_p
			m2m buf2d_w,eax
			m2m buf2d_h,eax
			stdcall [buf2d_create],edi
			mov edi,buf_npl_n
			m2m buf2d_w,eax
			m2m buf2d_h,eax
			stdcall [buf2d_create],edi
			jmp .end_0
		@@:
			; *** изменяем размеры буферов
			stdcall [buf2d_resize], edi, eax,eax,1
			mov edi,buf_npl_p
			stdcall [buf2d_resize], edi, eax,eax,1
			mov edi,buf_npl_n
			stdcall [buf2d_resize], edi, eax,eax,1
		.end_0:
		mov dword[n_plane],1
		mov byte[calc],1
		call get_scale
		call set_borders

		stdcall create_plane,buf_npl,0
		stdcall create_plane,buf_npl_n,1
		stdcall create_obj_from_plane,buf_npl,0 ;создаем начальное сечение

		jmp @f
	.err_size_t:
		stdcall [mb_create],msgbox_0,thread
		jmp @f
	.err_size_b:
		stdcall [mb_create],msgbox_1,thread
		jmp @f
	.err_size_tb:
		stdcall [mb_create],msgbox_3,thread
		;jmp @f
	@@:
	ret
endp

;прекратить создание объекта
align 4
but_stop:
	cmp byte[calc],0
	je @f
		call draw_object
		mov byte[calc],0
	@@:
	ret

align 4
but_rot_z:
	stdcall vox_obj_rot_z, [open_file_vox]
	call draw_object
	ret

align 4
draw_object:
	;вывод результата на экран
	stdcall [buf2d_clear], buf_0, [buf_0.color] ;чистим буфер
	stdcall [buf2d_clear], buf_0z, 0 ;чистим буфер
	stdcall [buf2d_vox_obj_draw_3g], buf_0, buf_0z, buf_vox,\
		[open_file_vox], 0,0, 0, 6 ;[k_scale]
	stdcall [buf2d_draw], buf_0 ;обновляем буфер на экране
	ret

align 4
proc open_image_in_buf, buf:dword
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file

	;stdcall mem.Alloc, dword size ;выделяем память для изображения
	;mov [buf],eax

	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Count], dword max_open_file_size
	m2m [run_file_70.Buffer],dword[open_file_img]
	mov byte[run_file_70+20], 0
	mov [run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70
	cmp ebx,0xffffffff
	je .end_0
		;определяем вид изображения
		stdcall dword[img_decode], dword[open_file_img],ebx,0
		or eax,eax
		jz .end_0 ;если нарушен формат файла
		mov ebx,[eax+4] ;+4 = image width
		cmp ebx,[eax+8] ;+8 = image height
		jne .err_s0
		mov ebx,eax
		;преобразуем изображение к формату rgb
		stdcall dword[img_to_rgb2], ebx,dword[open_file_img]

		mov edi,[buf]
		cmp buf2d_data,0
		jne @f
			m2m buf2d_w,dword[ebx+4] ;+4 = image width
			m2m buf2d_h,dword[ebx+8] ;+8 = image height
			stdcall [buf2d_create_f_img], edi,[open_file_img]
			jmp .end_1
		@@:
			mov ecx,[ebx+8]
			stdcall [buf2d_resize], edi, [ebx+4],ecx,1 ;изменяем размеры буфера
			imul ecx,[ebx+4]
			lea ecx,[ecx+ecx*2]
			mov edi,buf2d_data
			mov esi,[open_file_img]
			cld
			rep movsb ;copy image
			jmp .end_1
		.err_s0: ;ошибка, изображение для преобразования не подходит (не квадратное)
			mov ebx,eax
			notify_window_run txt_img_not_square
		.end_1:
		;удаляем временный буфер в ebx
		stdcall dword[img_destroy], ebx
	.end_0:

	call draw_buffers
	.end_open_file:
	popad
	ret
endp

msgbox_0:
	db 1,0
	db 'Внимание',0
	db 'Размер верхнего изображения не коректный',0
	db 'Закрыть',0
	db 0

msgbox_1:
	db 1,0
	db 'Внимание',0
	db 'Размер бокового изображения не коректный',0
	db 'Закрыть',0
	db 0

msgbox_2:
	db 1,0
	db 'Внимание',0
	db 'Размер переднего изображения не коректный',0
	db 'Закрыть',0
	db 0

msgbox_3:
	db 1,0
	db 'Внимание',0
	db 'Размеры верхнего и бокового изображений не совпадают',13,\
		'Откройте изображения одинаковых размеров',0
	db 'Закрыть',0
	db 0

msgbox_4:
	db 1,0
	db 'Внимание',0
	db 'Открываемый файл содержит не воксельный формат',0
	db 'Закрыть',0
	db 0

txt_img_not_square db '"Внимание',13,10,'Открываемое изображение не квадратное" -tW',0

;данные для диалога открытия файлов
align 4
OpenDialog_data:
.type			dd 0 ;0 - открыть, 1 - сохранить, 2 - выбрать дтректорию
.procinfo		dd procinfo	;+4
.com_area_name		dd communication_area_name	;+8
.com_area		dd 0	;+12
.opendir_path		dd plugin_path	;+16
.dir_default_path	dd default_dir ;+20
.start_path		dd file_name ;+24 путь к диалогу открытия файлов
.draw_window		dd draw_window	;+28
.status 		dd 0	;+32
.openfile_path		dd openfile_path	;+36 путь к открываемому файлу
.filename_area		dd filename_area	;+40
.filter_area		dd Filter
.x:
.x_size 		dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size 		dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

default_dir db '/sys',0

communication_area_name:
	db 'FFFFFFFF_open_dialog',0
open_dialog_name:
	db 'opendial',0
communication_area_default_path:
	db '/sys/File managers/',0

Filter:
dd Filter.end - Filter ;.1
.1:
db 'PNG',0
db 'JPG',0
db 'JPEG',0
db 'BMP',0
db 'GIF',0
db 'VOX',0
.end:
db 0



system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'msgbox.obj',0

align 4
l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, file_name, system_dir_2, import_buf2d
	lib_3 l_libs lib_name_3, file_name, system_dir_3, import_msgbox_lib
l_libs_end:

align 4
import_libimg:
	dd alib_init1
	img_is_img  dd aimg_is_img
	img_info    dd aimg_info
	img_from_file dd aimg_from_file
	img_to_file dd aimg_to_file
	img_from_rgb dd aimg_from_rgb
	img_to_rgb  dd aimg_to_rgb
	img_to_rgb2 dd aimg_to_rgb2
	img_decode  dd aimg_decode
	img_encode  dd aimg_encode
	img_create  dd aimg_create
	img_destroy dd aimg_destroy
	img_destroy_layer dd aimg_destroy_layer
	img_count   dd aimg_count
	img_lock_bits dd aimg_lock_bits
	img_unlock_bits dd aimg_unlock_bits
	img_flip    dd aimg_flip
	img_flip_layer dd aimg_flip_layer
	img_rotate  dd aimg_rotate
	img_rotate_layer dd aimg_rotate_layer
	img_draw    dd aimg_draw

	dd 0,0
	alib_init1   db 'lib_init',0
	aimg_is_img  db 'img_is_img',0 ;определяет по данным, может ли библиотека сделать из них изображение
	aimg_info    db 'img_info',0
	aimg_from_file db 'img_from_file',0
	aimg_to_file db 'img_to_file',0
	aimg_from_rgb db 'img_from_rgb',0
	aimg_to_rgb  db 'img_to_rgb',0 ;преобразование изображения в данные RGB
	aimg_to_rgb2 db 'img_to_rgb2',0
	aimg_decode  db 'img_decode',0 ;автоматически определяет формат графических данных
	aimg_encode  db 'img_encode',0
	aimg_create  db 'img_create',0
	aimg_destroy db 'img_destroy',0
	aimg_destroy_layer db 'img_destroy_layer',0
	aimg_count   db 'img_count',0
	aimg_lock_bits db 'img_lock_bits',0
	aimg_unlock_bits db 'img_unlock_bits',0
	aimg_flip    db 'img_flip',0
	aimg_flip_layer db 'img_flip_layer',0
	aimg_rotate  db 'img_rotate',0
	aimg_rotate_layer db 'img_rotate_layer',0
	aimg_draw    db 'img_draw',0

align 4
import_proclib:
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0

align 4
import_buf2d:
	init dd sz_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_resize dd sz_buf2d_resize
	buf2d_line dd sz_buf2d_line
	buf2d_rect_by_size dd sz_buf2d_rect_by_size
	buf2d_filled_rect_by_size dd sz_buf2d_filled_rect_by_size
	buf2d_circle dd sz_buf2d_circle
	buf2d_img_hdiv2 dd sz_buf2d_img_hdiv2
	buf2d_img_wdiv2 dd sz_buf2d_img_wdiv2
	buf2d_conv_24_to_8 dd sz_buf2d_conv_24_to_8
	buf2d_conv_24_to_32 dd sz_buf2d_conv_24_to_32
	buf2d_bit_blt dd sz_buf2d_bit_blt
	buf2d_bit_blt_transp dd sz_buf2d_bit_blt_transp
	buf2d_bit_blt_alpha dd sz_buf2d_bit_blt_alpha
	buf2d_curve_bezier dd sz_buf2d_curve_bezier
	buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	buf2d_draw_text dd sz_buf2d_draw_text
	buf2d_crop_color dd sz_buf2d_crop_color
	buf2d_offset_h dd sz_buf2d_offset_h
	buf2d_flood_fill dd sz_buf2d_flood_fill
	buf2d_set_pixel dd sz_buf2d_set_pixel
	buf2d_get_pixel dd sz_buf2d_get_pixel
	buf2d_vox_brush_create dd sz_buf2d_vox_brush_create
	buf2d_vox_brush_delete dd sz_buf2d_vox_brush_delete
	buf2d_vox_obj_get_img_w_3g dd sz_buf2d_vox_obj_get_img_w_3g
	buf2d_vox_obj_get_img_h_3g dd sz_buf2d_vox_obj_get_img_h_3g
	buf2d_vox_obj_draw_1g dd sz_buf2d_vox_obj_draw_1g
	buf2d_vox_obj_draw_3g dd sz_buf2d_vox_obj_draw_3g
	buf2d_vox_obj_draw_3g_scaled dd sz_buf2d_vox_obj_draw_3g_scaled
	buf2d_vox_obj_draw_3g_shadows dd sz_buf2d_vox_obj_draw_3g_shadows
	buf2d_vox_obj_draw_pl dd sz_buf2d_vox_obj_draw_pl
	buf2d_vox_obj_draw_pl_scaled dd sz_buf2d_vox_obj_draw_pl_scaled
	dd 0,0
	sz_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
	sz_buf2d_line db 'buf2d_line',0
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_circle db 'buf2d_circle',0
	sz_buf2d_img_hdiv2 db 'buf2d_img_hdiv2',0
	sz_buf2d_img_wdiv2 db 'buf2d_img_wdiv2',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_conv_24_to_32 db 'buf2d_conv_24_to_32',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	sz_buf2d_bit_blt_transp db 'buf2d_bit_blt_transp',0
	sz_buf2d_bit_blt_alpha db 'buf2d_bit_blt_alpha',0
	sz_buf2d_curve_bezier db 'buf2d_curve_bezier',0
	sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_crop_color db 'buf2d_crop_color',0
	sz_buf2d_offset_h db 'buf2d_offset_h',0
	sz_buf2d_flood_fill db 'buf2d_flood_fill',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0
	sz_buf2d_get_pixel db 'buf2d_get_pixel',0
	sz_buf2d_vox_brush_create db 'buf2d_vox_brush_create',0
	sz_buf2d_vox_brush_delete db 'buf2d_vox_brush_delete',0
	sz_buf2d_vox_obj_get_img_w_3g db 'buf2d_vox_obj_get_img_w_3g',0
	sz_buf2d_vox_obj_get_img_h_3g db 'buf2d_vox_obj_get_img_h_3g',0
	sz_buf2d_vox_obj_draw_1g db 'buf2d_vox_obj_draw_1g',0
	sz_buf2d_vox_obj_draw_3g db 'buf2d_vox_obj_draw_3g',0
	sz_buf2d_vox_obj_draw_3g_scaled db 'buf2d_vox_obj_draw_3g_scaled',0
	sz_buf2d_vox_obj_draw_3g_shadows db 'buf2d_vox_obj_draw_3g_shadows',0
	sz_buf2d_vox_obj_draw_pl db 'buf2d_vox_obj_draw_pl',0
	sz_buf2d_vox_obj_draw_pl_scaled db 'buf2d_vox_obj_draw_pl_scaled',0

align 4
import_msgbox_lib:
	mb_create dd amb_create
;       mb_reinit dd amb_reinit
;       mb_setfunctions dd amb_setfunctions
dd 0,0
	amb_create db 'mb_create',0
;       amb_reinit db 'mb_reinit',0
;       amb_setfunctions db 'mb_setfunctions',0

align 4
buf_0: dd 0
	dw 5 ;+4 left
	dw 35 ;+6 top
.w: dd 6*64 ;+8 w
.h: dd 7*64 ;+12 h
.color: dd 0xffffff ;+16 color
	db 24 ;+20 bit in pixel

align 4
buf_0z: dd 0
	dw 5 ;+4 left
	dw 35 ;+6 top
.w: dd 6*64 ;+8 w
.h: dd 7*64 ;+12 h
.color: dd 0 ;+16 color
	db 32 ;+20 bit in pixel

;текстура 1 (верхняя)
align 4
buf_i0: dd 0
	dw 5 ;+4 left
	dw 35 ;+6 top
.w: dd 0 ;+8 w
.h: dd 0 ;+12 h
.color: dd 0 ;+16 color
	db 24 ;+20 bit in pixel

;текстура 2
align 4
buf_i1: dd 0
	dw 105 ;+4 left
	dw 35 ;+6 top
.w: dd 0 ;+8 w
.h: dd 0 ;+12 h
.color: dd 0 ;+16 color
	db 24 ;+20 bit in pixel

;текстура 3
align 4
buf_i2: dd 0
	dw 205 ;+4 left
	dw 35 ;+6 top
.w: dd 0 ;+8 w
.h: dd 0 ;+12 h
.color: dd 0 ;+16 color
	db 24 ;+20 bit in pixel

;алгоритм создания модели использует 3 сечения: 
; предыдущее, текущее, последующее (это необходимо для отсеивания вокселей внутри объекта)

;предыдущее сечение
align 4
buf_npl_p: dd 0
	dw 0 ;+4 left
	dw 0 ;+6 top
.w: dd 0 ;+8 w
.h: dd 0 ;+12 h
.color: dd 0 ;+16 color
	db 24 ;+20 bit in pixel

;текущее сечение
align 4
buf_npl: dd 0
	dw 5 ;+4 left
	dw 35 ;+6 top
.w: dd 0 ;+8 w
.h: dd 0 ;+12 h
.color: dd 0 ;+16 color
	db 24 ;+20 bit in pixel

;последующее сечение
align 4
buf_npl_n: dd 0
	dw 0 ;+4 left
	dw 0 ;+6 top
.w: dd 0 ;+8 w
.h: dd 0 ;+12 h
.color: dd 0 ;+16 color
	db 24 ;+20 bit in pixel

;данные для создания минимального единичного вокселя
align 4
vox_6_7_z:
dd 0,0,1,1,0,0,\
   0,2,2,2,2,0,\
   2,2,2,2,2,2,\
   2,3,2,2,3,2,\
   2,3,3,3,3,2,\
   0,3,3,3,3,0,\
   0,0,3,3,0,0

align 4
buf_vox:
	db 6,7,4,3 ;w,h,h_osn,n
	rb BUF_STRUCT_SIZE*(2+1)


align 16
i_end:
	procinfo process_information
	sc system_colors
	run_file_70 FileInfoBlock
	mouse_dd dd ?
	last_time dd ?
		rb 2048
	thread:
		rb 2048
stacktop:
	sys_path rb 1024
	file_name rb 2048 ;4096 
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:
