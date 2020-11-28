use32
org 0
	db 'MENUET01'
	dd 1,START,I_END,MEM,STACKTOP,0,cur_dir_path

include '../../../../proc32.inc'
include '../../../../macros.inc'
include '../../../../KOSfuncs.inc'
include '../../../../load_lib.mac'

include 'deflate.inc'
include 'debug.inc'
include 'zlib.inc'

@use_library

align 4
m0size dd 90 ;размер данных для упаковки
m1size dd 1024 ;размер буфера данных для упаковки
m2size dd 0 ;размер распакованных данных

align 4
m0: ;данные для упаковки
file 'zlib.txt'

align 4
m1 rb 1024 ;буфер для упакованных данных
m2 dd 0 ;указатель на распакованные данные

buf rb 1024 ;буфер для вывода сжатых данных в окно
strategy dd Z_DEFAULT_STRATEGY ;стратегия сжатия

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

;	mcall SF_SYS_MISC, SSF_HEAP_INIT

	call test_code

align 4
red:                    ; перерисовать окно
    call draw_window    ; вызываем процедуру отрисовки окна

align 4
still:
    mcall SF_WAIT_EVENT
    cmp  eax,1          ; перерисовать окно ?
    je   red
    cmp  eax,2          ; нажата клавиша ?
    je   key
    cmp  eax,3          ; нажата кнопка ?
    je   button
    jmp  still

align 4
key:
    mcall SF_GET_KEY

	cmp ah,178 ;Up
	jne @f
		cmp dword[strategy],0
		jle @f
		dec dword[strategy]
		call test_code
		call draw_window
	@@:
	cmp ah,177 ;Down
	jne @f
		cmp dword[strategy],4
		jge @f
		inc dword[strategy]
		call test_code
		call draw_window
	@@:
	cmp ah,176 ;Left
	jne @f
		cmp dword[m0size],8
		jl @f
		dec dword[m0size]
		call test_code
		call draw_window
	@@:
	cmp ah,179 ;Right
	jne @f
		inc dword[m0size]
		call test_code
		call draw_window
	@@:
    jmp  still          ; вернуться к началу цикла

;---------------------------------------------------------------------
align 4
button:
	mcall SF_GET_BUTTON

	cmp ah,1
	jne still

.exit: ; конец программы
  	mcall SF_SYS_MISC,SSF_MEM_FREE,[m2]
    mcall SF_TERMINATE_PROCESS

align 4
draw_window:
    mcall SF_REDRAW, SSF_BEGIN_DRAW
    mcall SF_STYLE_SETTINGS, SSF_GET_COLORS, sc,sizeof.system_colors
    mov   edx, [sc.work]         ; цвет фона
    or    edx, 0x33000000        ; и тип окна 3
    mcall SF_CREATE_WINDOW, <50,600>, <50,180>, , ,title

	cStr edx,'Strategy:'
	mcall SF_DRAW_TEXT, <10,10>,0x40f0,,9
	cStr edx,'Input size:'
	mcall , <10,20>,,,11
	cStr edx,'Compr. size:'
	mcall , <10,30>,,,12
	cStr edx,'Outp. size:'
	mcall , <10,120>,,,11

	mov edx,[strategy]
	imul edx,12
	add edx,capt_strateg
	mcall , <90,10>,0,,12

	mcall SF_DRAW_NUMBER, (5 shl 16)+1, m0size, <90,20>
	mcall , (5 shl 16)+1, m1size, <90,30>
	mcall , (5 shl 16)+1, m2size, <90,120>
	;mov ecx,(1 shl 31)
	mov esi,[m2size]
	cmp esi,95
	jle @f
		mov esi,95
	@@:
	mcall SF_DRAW_TEXT, <10,130>, 0, [m2]

	mov esi,7
	mov ebx,(10 shl 16)+45 ;(x shl 16)+y
	mov edx,buf
	.cycle1: ;rows
		mcall SF_DRAW_TEXT,, (1 shl 31)
		add ebx,10
		add edx,32*3
	dec esi
	jnz .cycle1

    mcall SF_REDRAW, SSF_END_DRAW
    ret

align 4
test_code:
	stdcall [deflateInit2], my_strm,\
		-1, Z_DEFLATED, MAX_WBITS, DEF_MEM_LEVEL, [strategy]
;Стратегия:
; Z_DEFAULT_STRATEGY, Z_FILTERED, Z_HUFFMAN_ONLY, Z_RLE, Z_FIXED

	mov eax,my_strm
	mov [eax+z_stream.next_in],m0 ;устанавливаем память для сжатия
	mov ecx,[m0size]
	mov [eax+z_stream.avail_in],ecx ;размер сжимаемыж данных
	mov [eax+z_stream.next_out],m1 ;устанавливаем буфер для сжатия
	mov dword[eax+z_stream.avail_out],1024 ;размер буфера для сжатия (максимум 16 Кб)

	stdcall [deflate], my_strm, Z_FINISH

	;размер сжатых данных: 1024-[my_strm.avail_out]
	mov ecx,1024
	sub ecx,[my_strm.avail_out]
	mov [m1size],ecx

	;assert(ret != Z_STREAM_ERROR)
	;while (strm.avail_out == 0)

	;формирование текста для отображения сжатых данных
	;в 16-ричном виде, нужно только для примера
	mov ebx,[m1size]
	mov esi,m1
	mov edi,buf
	mov edx,7
align 4
	.cycle1: ;rows
	mov ecx,32
align 4
	.cycle0: ;cols
		stdcall hex_in_str, edi,[esi],2
		add edi,2
		inc esi
		mov byte[edi],' ' ;format space
		dec ebx
		jz .cycle1end ;if end file
		inc edi
		loop .cycle0
	mov byte[edi-1],0
	dec edx
	jnz .cycle1
	.cycle1end:
	mov byte[edi],0

	;удаление буфера с ранее распакованными данными
	mcall SF_SYS_MISC,SSF_MEM_FREE,[m2]
	
	mov eax,[m1size]
	sub eax,2 ;;; 2? or 6?
	mov [m2size],eax
	mov eax,m1
	add eax,2
	stdcall [deflate_unpack],eax,m2size
	mov [m2],eax ;запись новых распакованных данных
	mov ecx,[m0size] ;;; ???
	mov [m2size],ecx
	ret

align 4
proc print_z_struct uses eax ebx
	mov eax,my_strm
	mov ebx,[eax+z_stream.state]
	stdcall debug_fields,eax,sv_2
	stdcall debug_fields,ebx,sv_3
	ret
endp

align 4
sc system_colors
align 4
title db 'Zlib test, press on [Up], [Down], [Left], [Right]',0

align 4
capt_strateg db '0) Default ',0
db '1) Filtered',0
db '2) Huffman ',0
db '3) Rle     ',0
db '4) Fixed   ',0

align 4
import_archiver:
	deflate_unpack dd sz_deflate_unpack
	deflateInit		dd sz_deflateInit
	deflateInit2	dd sz_deflateInit2
	deflateReset	dd sz_deflateReset
	deflate			dd sz_deflate
	deflateEnd		dd sz_deflateEnd
	calc_crc32		dd sz_calc_crc32
	dd 0,0
	sz_deflate_unpack db 'deflate_unpack',0
	sz_deflateInit db 'deflateInit',0
	sz_deflateInit2 db 'deflateInit2',0
	sz_deflateReset db 'deflateReset',0
	sz_deflate db 'deflate',0
	sz_deflateEnd db 'deflateEnd',0
	sz_calc_crc32 db 'calc_crc32',0

;--------------------------------------------------
system_dir_0 db '/sys/lib/'
lib_name_0 db 'archiver.obj',0

l_libs_start:
	lib0 l_libs lib_name_0, library_path, system_dir_0, import_archiver
load_lib_end:
;---------------------------------------------------------------------

align 16
I_END:
my_strm z_stream
	rd 4096
align 16
STACKTOP:
cur_dir_path:
	rb 4096
library_path:
	rb 4096
MEM:
