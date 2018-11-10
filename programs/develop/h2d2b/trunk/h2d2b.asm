;;      h2d2b v0.5 big fonts by Leency      ;;
;;      09.11.2016                          ;;

;;      h2d2b v0.4 use editbox by IgorA     ;;
;;      30.08.2011                          ;;

;;      h2d2b v0.3 system colors by Leency  ;;
;;      21.08.2011                          ;;

;;      hex2dec2bin 0.2 by Alexei Ershov    ;;
;;      16.11.2006                          ;;

WIN_W = 364

use32
    org 0x0
    db  'MENUET01'
    dd  0x01,start,i_end,e_end,e_end,0,sys_path

include '../../../proc32.inc'
include '../../../macros.inc' ; макросы облегчают жизнь ассемблерщиков!
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'lang.inc'

@use_library

start:
	load_libraries l_libs_start,l_libs_end
	;проверка на сколько удачно загузилась наша библиотека
	mov	ebp,lib_0
	cmp	dword [ebp+ll_struc_size-4],0
	jz	@f
		mcall -1 ;exit not correct
	@@:
	mcall 40,0x80000027
	mcall 48, 3, sys_colors, 40
	edit_boxes_set_sys_color edit1,editboxes_end,sys_colors

red:
    call draw_window

still:
    mcall 10		; функция 10 - ждать события

    cmp  eax,1		; перерисовать окно ?
    je	 red		; если да - на метку red
    cmp  eax,2		; нажата клавиша ?
    je	 key		; если да - на key
    cmp  eax,3		; нажата кнопка ?
    je	 button 	; если да - на button
    cmp  eax,6
    je	 mouse

    jmp  still		; если другое событие - в начало цикла

;---------------------------------------------------------------------

key:		       ; нажата клавиша на клавиатуре
    mcall 2
	cmp ah,13 ;обработка кнопки Enter
	je @f
		stdcall [edit_box_key], dword edit1
		jmp still
	@@:
	mov edi,string1
	add edi,[edit1.size] ;устанавливаем в edi конец строки
	jmp read_str
    jmp still		; вернуться к началу цикла

read_str:
    dec   edi
    mov   esi, string1

    call  GetBase

    xor   ecx, ecx
    inc   ecx		; ecx = 1

make_bin:
    xor   eax, eax

next_digit:
    xor   edx, edx
    cmp   edi, esi
    jb	  .done

    mov   dl, [edi]
    cmp   dl, '-'
    jne   @f
    neg   eax
    jmp   .done
@@:
    cmp   dl, 'F'
    jbe   @f
    and   dl, 11011111b
@@:
    sub   dl, '0'
    cmp   dl, 9
    jbe   @f
    sub   dl, 'A'-'9'-1
@@:
    cmp   dl, bl
    jb	  @f
    ; Здесь обработать ошибку

    jmp   .done
@@:
    push  ecx
    xchg  eax, ecx
    mul   edx ;        edx:eax = eax * edx
    add   ecx, eax
    pop   eax
    mul   ebx
    xchg  eax, ecx
    dec   edi
    jmp   next_digit

.done:
   mov	  [num], eax	; сохраняем введенное число
   jmp	  red

;---------------------------------------------------------------------

button:
	mcall 17		; 17 - получить идентификатор нажатой кнопки
	cmp   ah, 1 	; если НЕ нажата кнопка с номером 1,
	jne   @f
		mcall -1
	@@:
	cmp ah, 2
	jne @f
		shl [num], 1
		jmp red
	@@:
	cmp ah, 3
	jne @f
		shr [num], 1
		jmp red
	@@:
	cmp ah, 4
	jne @f
		sar [num], 1
		jmp red
	@@:
	cmp ah, 5
	jne @f
		mov edi,string1
		add edi,[edit1.size] ;устанавливаем в edi конец строки
		jmp read_str
		;jmp red
	@@:
	jmp still

mouse:
	stdcall [edit_box_mouse], edit1
	jmp still

;------------------------------------------------
    draw_window:
;------------------------------------------------
	mcall	48, 3, sys_colors, 40

    mcall 12, 1
	mov	edx, 0x14000000
	or	edx, [sys_colors.work]
	;mov	esi, 0x80000000
	;or	esi, [sys_colors.grab_text]
    mcall 0, 200*65536+WIN_W, 200*65536+179, ,,title

	
    mcall  8,         15*65536+42,106*65536+ 21, 2, [sys_colors.work_button] ; кнопка shl
    mcall   ,         70*65536+42,             ,  , ; кнопка sal
    mcall   , (WIN_W-55)*65536+42,             , 3, ; кнопка shr
    mcall   ,(WIN_W-111)*65536+42,             , 4, ; кнопка sar
    mcall   , (WIN_W-72)*65536+58,145*65536+ 21, 5, ; кнопка Ok

	mov	ecx, 0x90000000
	or	ecx, [sys_colors.work_text]
    mcall  4, 15*65536+30,   , binstr,
    mcall   , 15*65536+46,   , decstr,
    mcall   , 15*65536+62,   ,sdecstr,
    mcall   , 15*65536+78,   , hexstr,
    mcall   , 15*65536+150,  , numstr,

	mov	ecx, 0x90000000
	or	ecx, [sys_colors.work_button_text]
    mcall  , 23*65536+109, , shl_sal_sar_shr_button_caption
	mcall  , (WIN_W-59)*65536+149,	, Okstr,
    mov    ecx, [num]

	mov esi, [sys_colors.work_text]
	or  esi, 0x90000000

    mcall  47, 10*65536,   ,(WIN_W-92)*65536+62,    ; 10-ная со знаком
	BIN_LINE_BLOCK_W = 76
    mcall 47, 8*65536+512,,(WIN_W-BIN_LINE_BLOCK_W)*65536+30 ; 2-ная	
    ror    ecx, 8
	mov    edx, (WIN_W-BIN_LINE_BLOCK_W*2)*65536+30
	mcall
    ror    ecx, 8
    mov    edx, (WIN_W-BIN_LINE_BLOCK_W*3)*65536+30
	mcall
    ror    ecx, 8
    mov    edx, (WIN_W-BIN_LINE_BLOCK_W*4)*65536+30
	mcall
    ror    ecx, 8
    mov    [minus], '+'
    jnc    @f
    mov    [minus], '-'
    neg    ecx
@@:
    mcall   , 10*65536,   ,(WIN_W-92)*65536+46,    ; 10-ная
	mcall   , 8*65536+256,,(WIN_W-76)*65536+78,    ; 16-ная
	mov   ecx,esi
    mcall  4, (WIN_W-102)*65536+61, , minus, 1
    mcall 38, 15*65536+WIN_W-15, 137*65536+137, [sys_colors.work_graph]
	stdcall [edit_box_draw], edit1
    mcall 12, 2 		   ; функция 12: сообщить ОС об отрисовке окна

ret


;-------------------------------------------------
    GetBase:
;-------------------------------------------------
    mov   ebx, 10
    cmp   edi, esi
    jb	  .done

    mov   al, [edi]
    cmp   al, 'H'
    jbe   @f
    and   al, 11011111b
@@:
    cmp   al, 'H'
    jne   @f
    mov   ebx, 16
    dec   edi
    jmp   .done

@@:
    cmp   al, 'D'
    jne   @f
    mov   ebx, 10
    dec   edi
    jmp   .done

@@:
    cmp   al, 'B'
    jne   .done
    mov   ebx, 2
    dec   edi

.done:
ret

;-------------------------------------------------
string1:
  db  34 dup(' ')
string1_end:
  num	dd  0


 title db 'hex2dec2bin 0.5',0
 minus	db '-',0
 hexstr db 'hex:',0
 binstr db 'bin:',0
 decstr db 'dec:',0
 sdecstr db 'signed dec:',0
 shl_sal_sar_shr_button_caption db 'shl    sal                    sar    shr',0

if lang eq ru
	numstr db 'Число:',0
	Okstr db 'Ввод',0
	head_f_i:
	head_f_l db 'Системная ошибка',0
else
	numstr db 'Number:',0
	Okstr db 'Enter',0
	head_f_i:
	head_f_l db 'System error',0
end if

mouse_dd dd 0
edit1 edit_box (WIN_W-67-82), 67, 146, 0xffffff, 0xff, 0x80ff, 0, 0x90000000, (string1_end-string1), string1 , mouse_dd, ed_focus+ed_always_focus

editboxes_end:

system_dir_0 db '/sys/lib/'
lib_name_0 db 'box_lib.obj',0
err_msg_found_lib_0 db 'Не найдена библиотека ',39,'box_lib.obj',39,0
err_msg_import_0 db 'Ошибка при импорте библиотеки ',39,'box_lib',39,0

l_libs_start:
	lib_0 l_libs lib_name_0, sys_path, library_path, system_dir_0,\
		err_msg_found_lib_0,head_f_l,import_box_lib,err_msg_import_0,head_f_i
l_libs_end:

align 4
import_box_lib:
	;dd sz_init1
	edit_box_draw dd sz_edit_box_draw
	edit_box_key dd sz_edit_box_key
	edit_box_mouse dd sz_edit_box_mouse
	;edit_box_set_text dd sz_edit_box_set_text
dd 0,0
	;sz_init1 db 'lib_init',0
	sz_edit_box_draw db 'edit_box',0
	sz_edit_box_key db 'edit_box_key',0
	sz_edit_box_mouse db 'edit_box_mouse',0
	;sz_edit_box_set_text db 'edit_box_set_text',0

i_end:
 sys_colors		system_colors
 rb 0x400					;stack
 sys_path rb 4096
 library_path rb 4096
e_end:				   ; метка конца программы
