use32
	org 0x0

db 'MENUET01'
dd 1,START,I_END,MEM,STACKTOP,0,cur_dir_path


include '../../../../../proc32.inc'
include '../../../../../macros.inc'
include '../../../../../KOSfuncs.inc'
include '../../../../../load_lib.mac'
include '../../../../../dll.inc'
include '../../libimg/libimg.inc'

macro cStr dest,txt
{
local .end_t
local .m_txt
jmp .end_t
align 4
	.m_txt db txt,0
align 4
.end_t:
if dest eq
	mov eax,.m_txt
else
	mov dest,.m_txt
end if
}

@use_library mem.Alloc,mem.Free,mem.ReAlloc, dll.Load

align 4
m1size dd 16*1024
m1 rb 16*1024

align 4
START:
load_libraries l_libs_start,load_lib_end
mov	ebp,lib0
.test_lib_open:
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
	mcall SF_TERMINATE_PROCESS ;exit not correct
@@:
	add ebp,ll_struc_size
	cmp ebp,load_lib_end
	jl .test_lib_open

	;create image data
	stdcall [buf2d_create], buf_0
	stdcall [buf2d_line], buf_0, 110, 20, 125, 90, 0xffff00
	stdcall [buf2d_line], buf_0, 60, 120, 110, 20, 0xd000
	stdcall [buf2d_curve_bezier], buf_0, (10 shl 16)+20,(110 shl 16)+10,(50 shl 16)+90, dword 0xff
	stdcall [buf2d_circle], buf_0, 125, 90, 30, 0xffffff
	stdcall [buf2d_circle], buf_0, 25, 70, 15, 0xff0000

	;create image struct
	stdcall [img.create], [buf_0.w], [buf_0.h], Image.bpp24
	test eax,eax
	jz @f
		;copy image
		mov edi,[eax+Image.Data]
		mov esi,[buf_0]
		mov ecx,[buf_0.w]
		imul ecx,[buf_0.h]
		imul ecx,3
		rep movsb

		;encode image
		stdcall [img.encode], eax, LIBIMG_FORMAT_PNG, 0
		test eax,eax
		jz @f

		;copy output image
		mov edi,m1
		mov esi,eax
		mov [m1size],ecx
		rep movsb
	@@:

align 4
red:
    call draw_window

align 4
still:
    mcall SF_WAIT_EVENT ; функция 10 - ждать события

    cmp  eax,1		; перерисовать окно ?
    je	 red		; если да - на метку red
    cmp  eax,2		; нажата клавиша ?
    je	 key		; если да - на key
    cmp  eax,3		; нажата кнопка ?
    je	 button 	; если да - на button

    jmp  still		; если другое событие - в начало цикла

align 4
key:		      ; нажата клавиша на клавиатуре
    mcall SF_GET_KEY  ; функция 2 - считать код символа (в ah)

	cmp ah,178 ;Up
	jne @f
		call but_save_file
	@@:
    jmp  still		; вернуться к началу цикла

;---------------------------------------------------------------------
align 4
button:
    mcall SF_GET_BUTTON
    cmp   ah, 1
    jne   still
.exit:
	stdcall [buf2d_delete],buf_0
    mcall SF_TERMINATE_PROCESS

align 4
draw_window:
    mcall SF_REDRAW, SSF_BEGIN_DRAW
    mcall SF_STYLE_SETTINGS, SSF_GET_COLORS, sc,sizeof.system_colors
    mov   edx, [sc.work]	 ; цвет фона
    or	  edx, 0x33000000	 ; и тип окна 3
    mcall SF_CREATE_WINDOW, <50,400>, <50,230>, , ,title

	stdcall [buf2d_draw], buf_0

	cStr edx,'Img. size:'
	mcall SF_DRAW_TEXT, <10,130>,0x40f0,,10
	mcall SF_DRAW_NUMBER, (5 shl 16)+1, m1size, <80,130>, 0

    mcall SF_REDRAW, SSF_END_DRAW
    ret

align 4
title db 'Press button [Up] and see '
openfile_path db '/sys/t1.png',0

struct FileInfoBlock
	Function dd ?
	Position dd ?
	Flags	 dd ?
	Count	 dd ?
	Buffer	 dd ?
		db ?
	FileName dd ?
ends

align 4
run_file_70 FileInfoBlock

align 4
but_save_file:
	pushad
	mov eax,SF_FILE
	mov [run_file_70.Function], SSF_CREATE_FILE
	mov [run_file_70.Position], 0
	mov [run_file_70.Flags], 0
	mov [run_file_70.Buffer], m1
	mov ebx,[m1size]
	mov dword[run_file_70.Count], ebx
	mov byte[run_file_70+20], 0
	mov dword[run_file_70.FileName], openfile_path
	mov ebx,run_file_70
	int 0x40
	;cmp ebx,0xffffffff
	;je .end_save_file
	;... if error ...
	;.end_save_file:
	popad
	ret

align 4
buf_0:
	dd 0
	dw 10 ;+4 left
	dw 4 ;+6 top
.w:	dd 360 ;+8 w
.h:	dd 120 ;+12 h
	dd 0x80 ;+16 color
	db 24 ;+20 bit in pixel

align 4
sc system_colors

align 4
import_buf2d_lib:
	dd sz_lib_init
	buf2d_create dd sz_buf2d_create
	buf2d_create_f_img dd sz_buf2d_create_f_img
	buf2d_clear dd sz_buf2d_clear
	buf2d_draw dd sz_buf2d_draw
	buf2d_delete dd sz_buf2d_delete
	buf2d_line dd sz_buf2d_line
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
	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_buf2d_create db 'buf2d_create',0
	sz_buf2d_create_f_img db 'buf2d_create_f_img',0
	sz_buf2d_clear db 'buf2d_clear',0
	sz_buf2d_draw db 'buf2d_draw',0
	sz_buf2d_delete db 'buf2d_delete',0
	sz_buf2d_line db 'buf2d_line',0
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

align 4
import_libimg:
	libimg.init dd sz_lib_init1
	img.draw    dd sz_img_draw
	img.decode  dd sz_img_decode
	img.encode  dd sz_img_encode
	img.create  dd sz_img_create
	img.destroy dd sz_img_destroy
	img.to_rgb2 dd sz_img_to_rgb2
	img.formats_table dd sz_img_formats_table
dd 0,0
	sz_lib_init1   db 'lib_init',0
	sz_img_draw    db 'img_draw',0
	sz_img_decode  db 'img_decode',0
	sz_img_encode  db 'img_encode',0
	sz_img_create  db 'img_create',0
	sz_img_destroy db 'img_destroy',0
	sz_img_to_rgb2 db 'img_to_rgb2',0
	sz_img_formats_table db 'img_formats_table',0

;--------------------------------------------------
system_dir_0 db '/sys/lib/'
lib_name_0 db 'buf2d.obj',0

system_dir_1 db '/sys/lib/'
lib_name_1 db 'libimg.obj',0

l_libs_start:
	lib0 l_libs lib_name_0, library_path, system_dir_0, import_buf2d_lib
	lib1 l_libs lib_name_1, library_path, system_dir_1, import_libimg
load_lib_end:
;---------------------------------------------------------------------

align 16
I_END:
	rd 4096
STACKTOP:
cur_dir_path rb 4096
library_path rb 4096
MEM:
