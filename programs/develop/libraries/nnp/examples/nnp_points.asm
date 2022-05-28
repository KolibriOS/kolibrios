use32
	org 0
	db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
	dd 1, start, i_end, mem, stacktop, 0, sys_path

include '../../../../macros.inc'
include '../../../../proc32.inc'
include '../../../../KOSfuncs.inc'
include '../../../../load_img.inc'
include '../../../../load_lib.mac'
include '../../../../develop/libraries/box_lib/trunk/box_lib.mac'

@use_library mem.Alloc,mem.Free,mem.ReAlloc,dll.Load
caption db 'NNP example 28.05.22',0 ;подпись окна

run_file_70 FileInfoBlock

IMAGE_TOOLBAR_ICON_SIZE equ 16*16*3
image_data_toolbar dd 0

memory_file_size dd 512*1024 ;размер памяти для открытия файлов (в начале 512 Kb, но может увеличиватся при необходимости)

NNP_FF_BIN  equ 0x6e6962
NNP_FF_JSON equ 0x6e6f736a

struct NeuralNetwork
	learningRate  dq ? ;+ 0 скорость обучения
	layers        dd ? ;+ 8 [] слои
	layers_length dd ? ;+12 число слоев
	activation    dd ? ;+16 указатель на функцию активации
	derivative    dd ? ;+20 указатель на функцию
	errors        dd ? ;+24 массив для вычислений
	errorsNext    dd ? ;+28
	gradients     dd ? ;+32
	deltas        dd ? ;+36
ends

struct Point
	x dq ? ;double
	y dq ?
	t dd ? ;long
ends

align 4
NNLOPT_LEN equ 4
nnlopt dd 2,3,3,2
_nn NeuralNetwork
lea_rate dq 0.01

POINTS_COUNT equ 8 ;число точек
_p rb sizeof.Point*POINTS_COUNT

_cycles_st dd 0
_r_op rb 128
txt_error  db '"Error open: ',39,'%s',39,'" -tE',0
txt_cycles db 'Cycles = %i000',0

;Макрос для параметров типа double (8 байт)
macro glpush double_v {
	push dword[double_v+4]
	push dword[double_v]
}

align 8
proc __ftol
	sub   esp,12
	wait
	fstcw word[esp+8]
	wait
	mov   al,[esp+9]
	or    byte[esp+9],0x0c
	fldcw word[esp+8]
	fistp qword[esp]
	mov   [esp+9],al
	fldcw word[esp+8]
	mov   eax,[esp]
	mov   edx,[esp+4]
	add   esp,12
	ret
endp

align 16
Math_random:
	imul  eax,dword[_rand_x],22695477
	inc   eax
	push  ecx
	mov   dword[_rand_x],eax
	and   eax,65535
	mov   dword[esp],eax
	fild  dword[esp]
	fmul  dword[@f]
	pop   edx
	ret 
align 4
@@:
	db 0,0,128,55 ;dd 1.0/65536.0
_rand_x dd 0


align 8
start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась библиотека
	mov	ebp,lib_0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall SF_TERMINATE_PROCESS
	@@:
	mcall SF_STYLE_SETTINGS,SSF_GET_COLORS,sc,sizeof.system_colors
	mcall SF_SET_EVENTS_MASK,0xC0000027
	stdcall [OpenDialog_Init],OpenDialog_data ;подготовка диалога
	stdcall [buf2d_create], buf_0 ;создание буфера

	include_image_file 'toolbar.png', image_data_toolbar

	stdcall mem.Alloc,[memory_file_size]
	mov dword[open_file],eax

	push NNLOPT_LEN
	push nnlopt
	push 0
	push 0
	glpush lea_rate
	stdcall [NNP_Create], _nn
	call but_update

align 8
red_win:
	call draw_window

align 16
still:
	mcall SF_WAIT_EVENT

	cmp al,1
	jz red_win
	cmp al,2
	jz key
	cmp al,3
	jz button
	cmp al,6 ;мышь
	jne @f
		jmp mouse
	@@:
	jmp still

align 8
draw_window:
pushad
	mcall SF_REDRAW,SSF_BEGIN_DRAW

	; *** рисование главного окна (выполняется 1 раз при запуске) ***
	mov edx,[sc.work]
	or  edx,(3 shl 24)+0x30000000
	mov edi,caption
	mcall SF_CREATE_WINDOW, (20 shl 16)+590, (20 shl 16)+540

	; *** создание кнопок на панель ***
	mov esi,[sc.work_button]
	mcall SF_DEFINE_BUTTON, (5 shl 16)+20, (5 shl 16)+20, 3

	add ebx,(25 shl 16)
	mov edx,4
	int 0x40

	add ebx,(25 shl 16)
	mov edx,5
	int 0x40

	add ebx,(30 shl 16)
	mov edx,6
	int 0x40

	add ebx,(25 shl 16)
	mov edx,7
	int 0x40

	; *** рисование иконок на кнопках ***
	mcall SF_PUT_IMAGE, [image_data_toolbar], (16 shl 16)+16, (7 shl 16)+7 ;icon new

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon open
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon save
	int 0x40

	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(30 shl 16) ;icon update points
	int 0x40
	add ebx,IMAGE_TOOLBAR_ICON_SIZE
	add edx,(25 shl 16) ;icon calculate
	int 0x40

	call PointsDraw
	; *** рисование буфера ***
	stdcall [buf2d_draw], buf_0

	mcall SF_REDRAW,SSF_END_DRAW
popad
	ret

align 8
key:
	mcall SF_GET_KEY
	jmp still

align 8
mouse:

	jmp still

align 8
button:
	mcall SF_GET_BUTTON
	cmp ah,3
	jne @f
		call but_new_file
		jmp red_win
	@@:
	cmp ah,4
	jne @f
		call but_open_file
		jmp red_win
	@@:
	cmp ah,5
	jne @f
		call but_save_file
		jmp red_win
	@@:
	cmp ah,6
	jne @f
		call but_update
		jmp red_win
	@@:
	cmp ah,7
	jne @f
		call but_calc
		jmp red_win
	@@:
	cmp ah,1
	jne still
.exit:
	stdcall [buf2d_delete],buf_0
	stdcall mem.Free,[image_data_toolbar]
	stdcall mem.Free,[open_file]
	stdcall [NNP_Destroy], _nn
	mcall SF_TERMINATE_PROCESS

align 8
but_calc:
	push      esi edi ebp
	add       esp,-32
	mov       ebp,_p
	xor       edi,edi ;i=0
.cycle_0: ;for(i=0;i<50000;i++)
	call      Math_random
	fimul     dword[.172]
	call      __ftol
	mov       esi,eax ;k=(long)(Math_random()*POINTS_COUNT)
	imul      esi,sizeof.Point
	fld       qword[ebp+esi+Point.x]
	fsub      dword[f_0_5]
	fstp      qword[esp] ;v[0]=p[k].x-.5
	fld       qword[ebp+esi+Point.y]
	fsub      dword[f_0_5]
	fstp      qword[esp+8] ;v[1]=p[k].y-.5
	stdcall   [NNP_FeedForward], _nn,esp ;r=NNP_FeedForward(&nn,v)

	xor       eax,eax
	mov       dword[esp+16],eax
	mov       dword[esp+20],eax ;t[0]=0.0
	mov       dword[esp+24],eax
	mov       dword[esp+28],eax ;t[1]=0.0
	cmp       dword[ebp+esi+Point.t],eax
	je        .173
	mov       dword[esp+16],eax
	mov       dword[esp+20],1072693248 ;if(p[k].t) t[0]=1.0
	jmp       .174
.173:
	mov       dword[esp+24],eax
	mov       dword[esp+28],1072693248 ;else t[1]=1.0
.174:
	lea       edx,dword[esp+16]
	stdcall   [NNP_BackPropagation], _nn,edx ;NNP_BackPropagation(&nn,t)
	inc       edi ;i++
	cmp       edi,50000
	jl        .cycle_0
	add       dword[_cycles_st],50
	stdcall   [sprintf], _r_op,txt_cycles,[_cycles_st]
	add       esp,12
	call      NNP_DrawInBuf
	call      PointsDraw
;		SaveNN("/tmp0/1/nnp_end.txt")
	;push       s@+835
	;call      @@SaveNN$qpxc
	;pop       ecx
	mcall     SF_SET_CAPTION,1,_r_op
	add       esp,32
	pop       ebp edi esi
	ret
align 4        
.172:
	dd POINTS_COUNT

align 8
but_new_file:
	mov dword[_cycles_st],0
	stdcall [NNP_Reset], _nn
	call NNP_DrawInBuf
	call PointsDraw
	ret

align 8
but_update:
	mov dword[_cycles_st],0
	call PointsInit
	call NNP_DrawInBuf
	call PointsDraw
	ret

align 8
PointsInit:
	push      ebx esi
	xor       esi,esi ;i=0
	mov       ebx,_p
.cycle_0: ;for(i=0;i<POINTS_COUNT;i++)
	call      Math_random
	fstp      qword[ebx+Point.x] ;p[i].x=Math_random()
	call      Math_random
	fstp      qword[ebx+Point.y] ;p[i].y=Math_random()
  	mov       eax,esi
	and       eax,1
	mov       dword[ebx+Point.t],eax ;p[i].t=i&1
	add       ebx,sizeof.Point
	inc       esi ;i++
	cmp       esi,POINTS_COUNT
	jl        .cycle_0
	pop       esi ebx
	ret

align 8
PointsDraw:
	push      ebx ecx esi edi ebp
	xor       ecx,ecx
	mov       ebx,_p
align 4
.cycle_0: ;for(i=0;i<POINTS_COUNT;i++)
	fild      dword[buf_0.w]
	fmul      qword[ebx+Point.x]
	call      __ftol
	fild      dword[buf_0.h]
	mov       esi,eax ;x=p[i].x*buf0.w
	fmul      qword[ebx+Point.y]
	call      __ftol
	mov       edi,eax ;y=p[i].y*buf0.h

	mov       ebp,255 ;c=0xff
	cmp       dword[ebx+Point.t],0
	je        @f
	shl       ebp,8 ;if(p[i].t) c<<=8
@@:
	sub       esi,4
	sub       edi,4
	stdcall   [buf2d_rect_by_size], buf_0,esi,edi,7,7,0xffffff
	inc       esi
	inc       edi
	stdcall   [buf2d_filled_rect_by_size], buf_0,esi,edi,5,5,ebp
	inc       ecx
	add       ebx,sizeof.Point
	cmp       ecx,POINTS_COUNT
	jl        .cycle_0
	pop       ebp edi esi ecx ebx
	ret

align 8
NNP_DrawInBuf:
	push      ebx esi ebp
	add       esp,-32
	lea       ebp,dword [esp+16]
; ebp = &v
	xor       eax,eax
	mov       dword [esp+8],eax
	mov       dword [esp+12],eax
	jmp       .cycle_0_end
.cycle_0: ;for(y=0;y<buf0.h;y++)
	fild      dword [buf_0.h]
	fdivr     qword [esp+8]
	fsub      dword [f_0_5]
	fstp      qword [ebp+8] ;v[1]=(double)y/buf0.h-.5
	xor       eax,eax
	mov       dword [esp],eax
	mov       dword [esp+4],eax
	jmp       .cycle_1_end
.cycle_1: ;for(x=0;x<buf0.w;x++)
	fild      dword [buf_0.w]
	fdivr     qword [esp]
	fsub      dword [f_0_5]
	fstp      qword [ebp] ;v[0]=(double)x/buf0.w-.5
	stdcall   [NNP_FeedForward], _nn,ebp
	mov       esi,eax ;r=NNP_FeedForward(&nn,v)
	fld       qword [esi]
	fmul      dword [f_255_0]
	call      __ftol
	movzx     ebx,al ;k=(unsigned char)(r[0]*255.0)
	shl       ebx,8 ;k<<=8
	fld       qword [esi+8]
	fmul      dword [f_255_0]
	call      __ftol
	and       eax,0xff
	add       ebx,eax ;k+=(unsigned char)(r[1]*255.0)
	push      ebx
	fld       qword [esp+12]
	call      __ftol
	push      eax
	fld       qword [esp+8]
	call      __ftol
	stdcall   [buf2d_set_pixel], buf_0,eax ;buf2d_set_pixel(&buf0,x,y,k)
	fld1
	fadd      qword [esp]
	fstp      qword [esp]
.cycle_1_end:
	fild      dword [buf_0.w]
	fcomp     qword [esp]
	fnstsw ax
	sahf
	ja        .cycle_1
	fld1
	fadd      qword [esp+8]
	fstp      qword [esp+8]
.cycle_0_end:
	fild      dword [buf_0.h]
	fcomp     qword [esp+8]
	fnstsw ax
	sahf
	ja        .cycle_0
	add       esp,32
	pop       ebp esi ebx
	ret 

align 4
f_0_5 dd 0.5
f_255_0 dd 255.0
open_file dd 0 ;указатель на память для открытия файлов
open_file_size dd 0 ;размер открытого файла (должен быть не больше memory_file_size)

align 8
but_open_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],0
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_open_file
	;код при удачном открытии диалога

	mov [run_file_70.Function], SSF_GET_INFO
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov dword[run_file_70.Count], 0
	m2m [run_file_70.Buffer], [open_file]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70
	cmp eax,0
	jne .end_open_file

	mov eax,[open_file]
	mov ebx,[eax+32] ;dword[eax+32] - размер открываемого файла
	mov [open_file_size],ebx ;ebx - размер открываемого файла
	;memory_file_size - размер выделенной памяти для файла
	cmp [memory_file_size],ebx
	jge @f
		;увеличиваем память если не хватило
		mov [memory_file_size],ebx
		stdcall mem.ReAlloc, [open_file],ebx
		mov [open_file],eax
	@@:

	mov [run_file_70.Function], SSF_READ_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	m2m dword[run_file_70.Count], dword[open_file_size]
	m2m dword[run_file_70.Buffer],dword[open_file]
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70 ;загружаем файл
	cmp ebx,0xffffffff
	je .end_open_file

	mov [open_file_size],ebx
	mcall SF_SET_CAPTION,1,openfile_path

	stdcall [NNP_SetMemData], _nn,NNP_FF_JSON,[open_file]
	or eax,eax
	jnz @f
		mov dword[_cycles_st],0
		call NNP_DrawInBuf
		call PointsDraw
		jmp .end_open_file
	@@:
		stdcall [sprintf], _r_op,txt_error,eax
		add esp,12
		notify_window_run _r_op
	.end_open_file:
	popad
	ret

align 8
but_save_file:
	pushad
	copy_path open_dialog_name,communication_area_default_path,file_name,0
	mov [OpenDialog_data.type],1
	stdcall [OpenDialog_Set_file_ext],OpenDialog_data,Filter.1
	stdcall [OpenDialog_Start],OpenDialog_data
	cmp [OpenDialog_data.status],2
	je .end_save_file
	;код при удачном открытии диалога

	mov [run_file_70.Function], SSF_CREATE_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0

	stdcall [NNP_GetMemData], _nn,NNP_FF_JSON,[open_file]
	stdcall [strlen], [open_file]
	pop ebx ;add esp,4
	mov ebx, [open_file]
	mov [run_file_70.Buffer], ebx
	mov [open_file_size],eax

	mov dword[run_file_70.Count], eax ;размер файла
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mcall SF_FILE,run_file_70 ;сохраняем файл
	;cmp ebx,0xffffffff
	;je .end_save_file
	; ... сообщение о неудачном сохранении ...

	.end_save_file:
	popad
	ret


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
db 'TXT',0
db 'JSON',0
db 'BIN',0
.end:
db 0


system_dir_0 db '/sys/lib/'
lib_name_0 db 'proc_lib.obj',0
system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0
system_dir_2 db '/sys/lib/'
lib_name_2 db 'buf2d.obj',0
system_dir_3 db '/sys/lib/'
lib_name_3 db 'nnp.obj',0
system_dir_4 db '/sys/lib/'
lib_name_4 db 'libc.obj',0

l_libs_start:
	lib_0 l_libs lib_name_0, file_name, system_dir_0, import_proclib
	lib_1 l_libs lib_name_1, file_name, system_dir_1, import_libimg
	lib_2 l_libs lib_name_2, file_name, system_dir_2, import_buf2d
	lib_3 l_libs lib_name_3, file_name, system_dir_3, import_nnp
	lib_4 l_libs lib_name_4, file_name, system_dir_4, import_libc
l_libs_end:

sz_lib_init db 'lib_init',0

align 4
import_libimg:
	dd sz_lib_init
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
import_proclib: ;описание экспортируемых функций
	OpenDialog_Init dd aOpenDialog_Init
	OpenDialog_Start dd aOpenDialog_Start
	;OpenDialog_Set_file_name dd aOpenDialog_Set_file_name
	OpenDialog_Set_file_ext dd aOpenDialog_Set_file_ext
dd 0,0
	aOpenDialog_Init db 'OpenDialog_init',0
	aOpenDialog_Start db 'OpenDialog_start',0
	;aOpenDialog_Set_file_name db 'OpenDialog_set_file_name',0
	aOpenDialog_Set_file_ext db 'OpenDialog_set_file_ext',0

align 4
import_buf2d:
	dd sz_lib_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear  dd sz_buf2d_clear
	buf2d_draw   dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_resize dd sz_buf2d_resize
	buf2d_rect_by_size dd sz_buf2d_rect_by_size
	buf2d_filled_rect_by_size dd sz_buf2d_filled_rect_by_size
	buf2d_circle dd sz_buf2d_circle
	buf2d_conv_24_to_8 dd sz_buf2d_conv_24_to_8
	buf2d_bit_blt dd sz_buf2d_bit_blt
	;buf2d_convert_text_matrix dd sz_buf2d_convert_text_matrix
	;buf2d_draw_text dd sz_buf2d_draw_text
	buf2d_set_pixel dd sz_buf2d_set_pixel
	dd 0,0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear  db 'buf2d_clear',0
	sz_buf2d_draw   db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_resize db 'buf2d_resize',0
	sz_buf2d_rect_by_size db 'buf2d_rect_by_size',0
	sz_buf2d_filled_rect_by_size db 'buf2d_filled_rect_by_size',0
	sz_buf2d_circle db 'buf2d_circle',0
	sz_buf2d_conv_24_to_8 db 'buf2d_conv_24_to_8',0
	sz_buf2d_bit_blt db 'buf2d_bit_blt',0
	;sz_buf2d_convert_text_matrix db 'buf2d_convert_text_matrix',0
	;sz_buf2d_draw_text db 'buf2d_draw_text',0
	sz_buf2d_set_pixel db 'buf2d_set_pixel',0

align 4
import_nnp:
	dd sz_lib_init
	NNP_Create      dd sz_create
	NNP_Reset       dd sz_reset
	NNP_FeedForward dd sz_feedforward
	NNP_BackPropagation dd sz_backpropagation
	NNP_GetMemData  dd sz_getmemdata
	NNP_SetMemData  dd sz_setmemdata
	NNP_Destroy     dd sz_destroy
dd 0,0
	sz_create       db 'NNP_Create',0
	sz_reset        db 'NNP_Reset',0
	sz_feedforward  db 'NNP_FeedForward',0
	sz_backpropagation db 'NNP_BackPropagation',0
	sz_getmemdata   db 'NNP_GetMemData',0
	sz_setmemdata   db 'NNP_SetMemData',0
	sz_destroy      db 'NNP_Destroy',0

align 4
import_libc:
	strlen dd sz_strlen
	sprintf dd sz_sprintf
dd 0,0
	sz_strlen db 'strlen',0
	sz_sprintf db 'sprintf',0

sc system_colors 

align 16
procinfo process_information 

align 4
buf_0: dd 0 ;указатель на буфер изображения
	dw 5 ;+4 left
	dw 31 ;+6 top
.w: dd 570 ;+8 w
.h: dd 480 ;+12 h
.color: dd 0xffffd0 ;+16 color
	db 24 ;+20 bit in pixel

align 16
i_end:
	rb 2048
stacktop:
	sys_path rb 1024
	file_name rb 4096 
	plugin_path rb 4096
	openfile_path rb 4096
	filename_area rb 256
mem:
