use32
  org 0x0
  db 'MENUET01' ;идентиф. исполняемого файла всегда 8 байт
  dd 0x1
  dd start
  dd i_end ; размер приложения
  dd mem
  dd stacktop
  dd 0x0
  dd cur_dir_path

include 'macros.inc'
include 'vectors.inc' ;vectors functions constant
include 'load_lib.mac'

@use_library

delt_angl dd 0.15708 ;угол поворота при нажатии курсоров
delt_size dd 0.05 ;шаг изменения масштаба
scale_min dd 0.05 ;минимальный масштаб
delt_x dd 10.0 ;передвижение по оси x
delt_y equ delt_x

arr0c:
  .x dd 200.0
  .y dd 150.0
  .a dd 0.0 ;angle
  .s dd 0.35 ;scale
arr1v:
  dw 12+VECT_PARAM_PROP_L+VECT_PARAM_COLOR ;туловище
  dd 0xffd040
  dd VECT_CONT_BEZIER
  dd 6.0,128.0, 41.0,113.0, 175.0,94.0, 211.0,78.0
  dd 263.0,48.0, 300.0,29.0, 345.0,30.0, 383.0,52.0
  dd 415.0,85.0, 443.0,100.0, 476.0,112.0, 519.0,135.0
  dw 32+VECT_PARAM_PROP_L
  dd VECT_CONT_BEZIER
  dd 6.0,128.0, 48.0,127.0, 119.0,116.0, 202.0,114.0
  dd 233.0,127.0, 274.0,185.0, 300.0,211.0, 351.0,243.0
  dd 386.0,271.0, 398.0,292.0, 429.0,328.0, 507.0,384.0
  dd 517.0,401.0, 534.0,402.0, 545.0,388.0, 568.0,386.0
  dd 587.0,370.0, 612.0,393.0, 660.0,409.0, 712.0,447.0
  dd 711.0,422.0, 764.0,438.0, 837.0,481.0, 853.0,480.0
  dd 838.0,470.0, 860.0,466.0, 836.0,451.0, 851.0,447.0
  dd 796.0,412.0, 743.0,361.0, 719.0,335.0, 677.0,290.0

  dw 23+VECT_PARAM_PROP_L ;крылья
  dd VECT_CONT_BEZIER
  dd 437.0,130.0, 519.0,135.0, 591.0,139.0, 616.0,150.0
  dd 697.0,163.0, 776.0,183.0, 849.0,230.0, 811.0,245.0
  dd 808.0,265.0, 771.0,258.0, 769.0,281.0, 729.0,277.0
  dd 718.0,296.0, 679.0,284.0, 665.0,294.0, 642.0,290.0
  dd 636.0,304.0, 609.0,296.0, 596.0,306.0, 574.0,301.0
  dd 541.0,299.0, 514.0,302.0, 495.0,299.0
  dw 16+VECT_PARAM_PROP_L
  dd VECT_CONT_BEZIER
  dd 484.0,116.0, 514.0,110.0, 564.0,82.0, 618.0,63.0
  dd 712.0,25.0, 774.0,25.0, 796.0,39.0, 778.0,51.0
  dd 779.0,64.0, 754.0,78.0, 745.0,97.0, 720.0,113.0
  dd 713.0,132.0, 688.0,146.0, 683.0,153.0, 668.0,158.0

  dw 7+VECT_PARAM_PROP_L+VECT_PARAM_COLOR ;глаз
  dd 0x0
  dd VECT_CONT_BEZIER
  dd 292.0,88.0, 278.0,78.0, 262.0,86.0, 261.0,101.0
  dd 274.0,110.0, 291.0,105.0, 292.0,88.0, 778.0,51.0
  dw 0

align 4
start:
  mov ecx,sc
  mov edx,sizeof.system_colors
  mcall 48,3
  mcall 40,0x27

  sys_load_library vectors_name, cur_dir_path, library_path, system_path, \
    err_message_found_lib, head_f_l, vectors_lib_import, err_message_import, head_f_i

  push 5*65536+40
  push 500*65536+350
  push 0xffffd0 ;цвет фона
  push word 24*256+1
  call [vect_buf_create]

  push word 1
  call [vect_buf_set_active] ;устанавливаем режим рисования в 1-й буфер

red_win:
  call draw_window

still:
  mcall 10

  cmp al,0x1 ;изм. положение окна
  jz red_win
  cmp al,0x2
  jz key
  cmp al,0x3
  jz button
  jmp still

draw_window:
  mcall 12,1

  xor eax,eax
  mov ebx,10*65536+520
  mov ecx,10*65536+430
  mov edx,0xffffff;[sc.work]
  or  edx,0x33000000
  mov edi,hed
  mcall

  mov eax,4
  mov ebx,5*65536+5
  mov ecx,0x808000
  or  ecx,0x80000000
  mov edx,txtInfo1
  int 0x40
  mov ebx,5*65536+20
  mov edx,txtInfo2
  int 0x40

  call draw_vect_image

  mcall 12,2
  ret

draw_vect_image:
  push word 1
  call [vect_buf_clear] ;чистим 1-й буфер

  push dword 0x808000
  push dword arr1v
  push dword arr0c
  call [vect_draw_cont]

  push word 1
  call [vect_buf_draw] ;выводим 1-й буфер на экран
  ret

button:
;  mcall 17 ;получить код нажатой кнопки
;  cmp ah,1
;  jne still
.exit:
  push word 1
  call [vect_buf_delete] ;удаляем 1-й буфер
  mcall -1 ;выход из программы

key:
  mcall 2

  cmp ah,27 ;Esc
  je button.exit

  cmp ah,176 ;Left
  jne @f
    call Image_RotLeft
    call draw_vect_image
  @@:
  cmp ah,179 ;Right
  jne @f
    call Image_RotRight
    call draw_vect_image
  @@:
  cmp ah,178 ;Up
  jne @f
    fld dword[arr0c.s]
    fadd dword[delt_size]
    fstp dword[arr0c.s]
    call draw_vect_image
  @@:
  cmp ah,177 ;Down
  jne @f
    call ScaleDec
    call draw_vect_image
  @@:
  cmp ah,119 ;w
  jne @f
    call Image_MoveUp
    call draw_vect_image
  @@:
  cmp ah,115 ;s
  jne @f
    call Image_MoveDown
    call draw_vect_image
  @@:
  cmp ah,100 ;d
  jne @f
    call Image_MoveRight
    call draw_vect_image
  @@:
  cmp ah,97 ;a
  jne @f
    call Image_MoveLeft
    call draw_vect_image
  @@:

  jmp still

;d
Image_MoveRight:
  fld dword[arr0c.x]
  fadd dword[delt_x]
  fstp dword[arr0c.x]
  ret

;a
Image_MoveLeft:
  fld dword[arr0c.x]
  fsub dword[delt_x]
  fstp dword[arr0c.x]
  ret

;w
Image_MoveUp:
  fld dword[arr0c.y]
  fsub dword[delt_y]
  fstp dword[arr0c.y]
  ret

;s
Image_MoveDown:
  fld dword[arr0c.y]
  fadd dword[delt_y]
  fstp dword[arr0c.y]
  ret

Image_RotLeft:
  mov bl,1
  fld dword[arr0c.a]
  fsub dword[delt_angl]
  fstp dword[arr0c.a]
  ret

Image_RotRight:
  mov bl,1
  fld dword[arr0c.a]
  fadd dword[delt_angl]
  fstp dword[arr0c.a]
  ret

ScaleDec:
  push ax
    finit
    fld dword[arr0c.s]
    fsub dword[delt_size]

    fcom dword[scale_min]
    fstsw ax
    sahf
    jbe @f
      fstp dword[arr0c.s]
    @@:
  pop ax
ret

o_dan dd ?
hed db 'Drawing vector image',0
txtInfo1 db 'Љгаб®ал: [',27,'], [',26,'] - Ї®ў®а®в; [',24,'], [',25,'] - а §¬Ґа',0
txtInfo2 db 'Љ­®ЇЄЁ: [a], [d] - ў«Ґў®, ўЇа ў®; [w], [s] - ўўҐае, ў­Ё§',0
sc system_colors

;--------------------------------------------------
align 4
vectors_lib_import:
  vect_buf_create dd av_buf_create
  vect_buf_set_active dd av_buf_set_active
  vect_buf_clear dd av_buf_clear
  vect_buf_draw dd av_draw_buf
  vect_buf_delete dd av_buf_delete
  vect_line dd av_line
  vect_c_bezier dd av_c_bezier
  vect_conv_cont dd av_conv_cont
  vect_draw_cont dd av_draw_cont
  vect_opred2i dd av_opred2i
  vect_line_len4i dd av_line_len4i
  vect_o_len dd av_o_len
  vect_o_ang dd av_o_ang

  dd 0,0
  av_buf_create db 'vect_buf_create',0
  av_buf_set_active db 'vect_buf_set_active',0
  av_buf_clear db 'vect_buf_clear',0
  av_draw_buf db 'vect_buf_draw',0
  av_buf_delete db 'vect_buf_delete',0
  av_line db 'vect_line',0
  av_c_bezier db 'vect_c_bezier',0
  av_conv_cont db 'vect_conv_cont',0
  av_draw_cont db 'vect_draw_cont',0
  av_opred2i db 'vect_opred2i',0
  av_line_len4i db 'vect_line_len4i',0
  av_o_len db 'vect_o_len',0
  av_o_ang db 'vect_o_ang',0

;--------------------------------------------------
system_path db '/sys/lib/'
vectors_name db 'vectors.obj',0
err_message_found_lib db 'Sorry I cannot load library vectors.obj',0
head_f_i:
head_f_l db 'System error',0
err_message_import db 'Error on load import library vectors.obj',0
;--------------------------------------------------

i_end: ;конец кода
  rb 1024
stacktop:
  cur_dir_path:
    rb 4096
  library_path:
    rb 4096
mem:

