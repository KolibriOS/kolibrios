; 24.07.2008 <Lrz> обновлен Edit бокс
; 01.02.07 - обновлён editbox
; 31.01.07 - исправлена некорректная отрисовка при большом значении высоты скина
;            выравнивание снизу относительно рабочей области экрана
window_y=67
;window_x=320
window_x=640
;--- другие макросы ---
;include '../../../develop/examples/editbox/trunk/editbox.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../develop/libraries/box_lib/asm/trunk/editbox_ex.mac'
;include 'editbox.inc'
include 'txtbut.inc'
include '../../../macros.inc'
;include 'macros.inc'
include 'run.mac'
include 'lang.inc'
use32                
    org 0x0
    db 'MENUET01'
    dd 0x1
    dd start
    dd i_end
    dd mem
    dd mem
    dd par
    dd cur_dir_path


;        meos_app_start
;        use_edit_box
	@use_library
        use_txt_button
;        code
;load system lib
align 4
start:
sys_load_library  library_name, cur_dir_path, library_path, system_path, \
err_message_found_lib, head_f_l, myimport, err_message_import, head_f_i
        cmp     eax,-1
        jz      close

        cmp     [par],byte 0
        jne     read_par
        mcall   40,EVM_MOUSE+EVM_BUTTON+EVM_KEY+EVM_REDRAW
red:
        mcall   48,3,sc,40
        edit_boxes_set_sys_color input_fn,input_fn_end,sc
        set_sys_colors_txt_button run_but,sc
        push    dword [sc.work_graph]
        pop     [input_fn.focus_border_color]
        call    draw_window
still:                          ;основной обработчик 
        mcall   10              ;Ожидать события
        dec  eax
        jz   red
        dec  eax
        jz   key
        dec  eax
        jz   button
;        mouse_edit_box input_fn 
        push    dword input_fn
        call    [edit_box_mouse]

        jmp still    ;если ничего из перечисленного то снова в цикл
key:
        mcall   2
        cmp     ah,13
        je      run
;        key_edit_box input_fn
        push    dword input_fn
        call    [edit_box_key]

        jmp     still
button:
        mcall   17
        dec     ah
        jz      close
        dec     ah
        jz      run
        jmp     still

read_par:
        mov     esi,par
        mov     edi,fn
        mov     ecx,256
        rep     movsb
run:
        xor     eax,eax
        mov     edi,file_info.name
        mov     ecx,512
        rep     stosb
        mov     edi,run_par
        mov     ecx,256
        rep     stosb

        mov     esi,fn
        mov     edi,file_info.name
        cmp     [esi],byte '"'
        je      copy_fn_with_spaces
copy_fn:
        cmp     [esi],byte ' '
        je      .stop
        cmp     [esi],byte 0
        je      .stop
        mov     al,[esi]
        mov     [edi],al
        inc     esi
        inc     edi
        jmp     copy_fn
.stop:

        jmp     copy_par

copy_fn_with_spaces:
        inc     esi
@@:
        cmp     [esi],byte '"'
        je      .stop
        cmp     [esi],byte 0
        je      .stop
        mov     al,[esi]
        mov     [edi],al
        inc     esi
        inc     edi
        jmp     @b
.stop:

copy_par:
@@:
        inc     esi
        cmp     [esi],byte ' '
        je      @b
        mov     edi,run_par
@@:
        cmp     [esi],byte 0
        je      .stop
        mov     al,[esi]
        mov     [edi],al
        inc     esi
        inc     edi
        jmp     @b
.stop:
        mcall   70,file_info

        cmp     eax,0
        jl      error
        mov     [status],run_ok
        call    draw_status
        jmp     still
close:
        mcall -1

error:
        neg     eax
        cmp_err 3,bad_file_sys
        cmp_err 5,file_not_find
        cmp_err 9,bad_fat_table
        cmp_err 10,acces_denyied
        cmp_err 11,device_error
        cmp_err 30,out_of_memory
        cmp_err 31,file_not_executable
        cmp_err 32,many_processes

        call    draw_status
        jmp     still

draw_window:
        mcall   48,5
        mov     si,bx

        mcall   12,1
        mcall   48,4
        mov     dx,ax
        mcall   14
        xor     ecx,ecx
        sub     cx,window_y+2
        sub     cx,dx
        add     cx,si
        shl     ecx,16
        mov     cx,dx
        add     cx,window_y
        shr     eax,16
        mov     bx,ax
        sub     bx,window_x
        shl     ebx,15
        mov     bx,window_x
        mov     edx,[sc.work]
        or      edx,0x33000000
        xor     esi,esi
        mov     edi,grab_text
        mcall   0

        mcall   9,procinfo,-1

        mov     eax,[procinfo.box.width]
        sub     eax,20
        mov     [input_fn.width],eax
        mov     [run_but.width],ax

        xor     bx,bx
        shl     ebx,16
        mov     bx,ax
        add     bx,10
        mov     cx,45
        push    cx
        shl     ecx,16
        pop     cx
        mov     edx,[sc.work_graph]
        mcall   38
;        draw_edit_box input_fn
        push    dword input_fn
        call    [edit_box_draw]

        draw_txt_button run_but

        call    draw_status_text

        mcall   12,2
ret

draw_status:
        mov     ebx,[procinfo.box.width]
        sub     bx,10
        mov     ecx,(50)*65536+12
        mov     edx,[sc.work]
        mcall   13
draw_status_text:
        mov     edx,[status]
        xor     esi,esi
@@:
        cmp     [edx+esi],byte 0
        je      @f
        inc     esi
        jmp     @b
@@:
        mov     ecx,[sc.work_text]
        mcall   4,5*65536+(50)
ret

run_but txt_button 0,5,15,25,2,0,0,run_but_text,
input_fn edit_box 0,5,5,0xffffff,0x6a9480,0,0xaaaaaa,0,511,fn,mouse_dd,ed_focus+ed_always_focus
;mouse_flag: dd 0x0
input_fn_end:
if lang eq ru
hello db 'Введите полный путь к файлу и нажмите Enter',0
bad_file_sys db 'Неизвестная файловая система',0 ; 3
file_not_find db 'Файл не найден',0              ; 5
bad_fat_table db 'Таблица FAT разрушена',0       ; 9
acces_denyied db 'Доступ запрещен',0             ; 10
device_error db 'Ошибка устройства',0            ; 11
out_of_memory db 'Недостаточно памяти',0         ; 30
file_not_executable db 'Файл не является исполняемым',0 ; 31
many_processes db 'Слишком много процессов',0    ; 32
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
status dd hello

;>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
;DATA данные
;Всегда соблюдать последовательность в имени.
system_path      db '/sys/lib/'
library_name     db 'box_lib.obj',0
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

err_message_found_lib   db 'Sorry I cannot load library box_lib.obj',0
head_f_i:
head_f_l        db 'System error',0
err_message_import      db 'Error on load import library box_lib.obj',0

myimport:   

edit_box_draw   dd      aEdit_box_draw
edit_box_key    dd      aEdit_box_key
edit_box_mouse  dd      aEdit_box_mouse
version_ed      dd      aVersion_ed

                dd      0
                dd      0

aEdit_box_draw  db 'edit_box',0
aEdit_box_key   db 'edit_box_key',0
aEdit_box_mouse db 'edit_box_mouse',0
aVersion_ed     db 'version_ed',0




file_info:
.mode dd 7
.flags dd 0
.par dd run_par
dd 0,0
.name rb 512

flags rw 1

sc system_colors

procinfo process_information

run_par rb 256
par rb 256
fn rb 512
mouse_dd	rd 1
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
cur_dir_path    rb 4096
library_path    rb 4096
i_end:
rb 1024
mem:
;meos_app_end
;udata
