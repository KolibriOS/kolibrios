;   RDsave для Kolibri (0.6.5.0 и старше)
;   Save RAM-disk to hard or floppy drive
;---------------------------------------------------------------------
;   Mario79 2005
;   Heavyiron 12.02.2007
;   <Lrz>     11.05.2009 - для работы нужна системная библиотека box_lib.obj
;   Mario79   08.09.2010 - select path with OpenDialog,keys 1,2,3,4 for select options
;   Heavyiron 01.12.2013 - new logic
;---------------------------------------------------------------------
appname equ 'RDsave '
version equ '1.41'
debug   equ no

use32        ; включить 32-битный режим ассемблера
org 0x0      ; адресация с нуля

db 'MENUET01'    ; 8-байтный идентификатор MenuetOS
dd 0x01          ; версия заголовка (всегда 1)
dd START         ; адрес первой команды
dd IM_END        ; размер программы
dd I_END         ; количество памяти
dd stacktop      ; адрес вершины стека
dd PARAMS        ; адрес буфера для параметров
dd cur_dir_path


include 'lang.inc'
include '../../../macros.inc'
if debug eq yes
include '../../../debug.inc'
end if
include '../../../proc32.inc'
include '../../../dll.inc'
include '../../../develop/libraries/box_lib/load_lib.mac'
include '../../../develop/libraries/box_lib/trunk/box_lib.mac'
include 'str.inc'

        @use_library
;---------------------------------------------------------------------
;---  НАЧАЛО ПРОГРАММЫ  ----------------------------------------------
;---------------------------------------------------------------------
align 4
START:
;---------------------------------------------------------------------
        mcall  48,3,sc,sizeof.system_colors
        mcall  68,11

load_libraries l_libs_start,end_l_libs
        inc     eax
        test    eax,eax
        jz      close

stdcall dll.Init,[init_lib]

invoke  ini_get_int,ini_file,asettings,aautoclose,0
        mov   [autoclose],eax
invoke  ini_get_str,ini_file,apath,apath,fname_buf,4096,path
stdcall _lstrcpy,ini_path,fname_buf

stdcall _lstrcpy,filename_area,start_temp_file_name

        mov   eax,PARAMS
        cmp   byte[eax], 0
        je    no_params
        cmp   byte[eax], 'h'
        je    @f
        cmp   byte[eax], 'H'
        jne   .no_h
@@:
        mov   [hidden],1
        jmp   no_params
.no_h:
        mov   [param],1
        stdcall _lstrcpy,fname_buf,eax
        xor   eax,eax
        mov   ah,2
        jmp   noclose

;---------------------------------------------------------------------
no_params:
stdcall _lstrcpy,check_dir,ini_path
        call    check_path
        test    eax,eax
        jz      path_ok
        cmp     eax,6
        je      path_ok
;---------------------------------------------------------------------
if debug eq yes
dps 'read_folder_error'
newline
end if
;---------------------------------------------------------------------

stdcall _lstrcpy,fname_buf,communication_area_default_path

        mov     [hidden],0

;OpenDialog     initialisation
        push    dword OpenDialog_data
        call    [OpenDialog_Init]

; prepare for PathShow
        push    dword PathShow_data_1
        call    [PathShow_prepare]
        call    draw_window
        mov     ah,3
        mov     ecx,fname_buf
        jmp     noclose
;---------------------------------------------------------------------
path_ok:
;OpenDialog     initialisation
        push    dword OpenDialog_data
        call    [OpenDialog_Init]

; prepare for PathShow
        push    dword PathShow_data_1
        call    [PathShow_prepare]

        mcall   40,0x00000027

        cmp     [hidden],1
        jne     red
        mov     ah,2
        jmp     noclose
red:
        call    draw_window
;---------------------------------------------------------------------
still:
        mcall 10

        dec     eax      ; перерисовать окно?
        jz      red      ; если да - на метку red
        dec     eax
        jz      key
        dec     eax
        jz      button
        jmp     still
;---------------------------------------------------------------------
button:
        mcall   17      ; получить идентификатор нажатой кнопки
        dec     ah
        jz      close
        cmp     ah,1             ; кнопка с id=1("закрыть")?
        jne     noclose
close:
        or       eax,-1          ; функция -1: завершить программу
        mcall
;---------------------------------------------------------------------
key:
        mcall   2
        cmp     ah,0x1b
        je      close
        cmp     ah,0x0D
        jne     @f
        mov     ah,2
        jmp     noclose
@@:
        cmp     ah,9h
        jne     still
;---------------------------------------------------------------------
noclose:
        mov     ecx,fname_buf
        push  16
        mov   ebx,1
        cmp   byte[ecx+1],'f'
        je    @f
        cmp   byte[ecx+1],'F'
        jne   not_fdd
@@:
        cmp   byte[ecx+4],'1'
        jne   @f
        cmp   ah,2
        je    doit
@@:
        inc   ebx
        cmp   ah,2
        je    doit
not_fdd:
        push  18
        mov   ebx,6     ; 18.6 = save to specified folder
        cmp   ah,2
        je    doit

; invoke OpenDialog
        push    dword OpenDialog_data
        call    [OpenDialog_Start]
        cmp     [OpenDialog_data.status],1
        jne     still

; prepare for PathShow
        push    dword PathShow_data_1
        call    [PathShow_prepare]
        call    draw_window
        mov     ecx,fname_buf
        mov     ah,2
        jmp     noclose

doit:
        cmp     [param],0
        jne     @f
        call    save_ini
@@:
        cmp   byte[ecx+1],'r'
        je    @f
        cmp   byte[ecx+1],'R'
        jne   not_rd
@@:
        mov   edx,rdError
        call  print_err
        cmp     [param],1
        je      @f
        jmp     still
@@:
        mov     [param],0
        jmp     no_params
not_rd:
        pop     eax
        mcall
        call    check_for_error
        cmp     [param],1
        je      @f
        jmp     still
@@:
        mov     [param],0
        jmp     no_params

;---------------------------------------------------------------------
check_for_error:                      ;Обработчик ошибок
stdcall _lstrcpy,check_dir,ok
stdcall _lstrcat,check_dir,fname_buf
        mov     edx,check_dir
        test    eax,eax
        jz      print_ok
        cmp     ebx,6
        je      @f
        mov     edx,error11
        jmp     print_err
@@:     
        cmp     eax, 11
        ja      .unknown
        mov     edx, [errors+eax*4]
        stdcall _lstrcat,error_msg,edx
        mov     edx, error_msg
        jmp     print_err
.unknown:
        mov     edx, aUnknownError
print_err:
        pushad
        stdcall _lstrlen,ini_path
        invoke  ini_set_str,ini_file,apath,apath,ini_path,eax
        stdcall _lstrcpy,fname_buf,ini_path
        popad
        cmp     [hidden],1
        je      @f
        cmp     [param],1
        je      @f
        stdcall _lstrlen,edx
        imul    eax,6
        mov     ebx,390
        sub     ebx,eax
        sar     ebx,1
        shl     ebx,16
        add     ebx,96
        mov     ecx,[sc.work_text]
        or      ecx,0xc0880000
        mcall   4, , , , ,[sc.work]
        ret
@@:
        mov     dword [is_notify + 8], edx
        mcall   70, is_notify
        ret
print_ok:
        cmp     [hidden],1
        je       @f
        cmp     [param],1
        je       @f
        stdcall _lstrlen,edx
        imul    eax,6
        mov     ebx,390
        sub     ebx,eax
        sar     ebx,1
        shl     ebx,16
        add     ebx,96
        mov     ecx,[sc.work_text]
        or      ecx,0xc0008800
        mcall   4, , , , ,[sc.work]
        mcall   5,100
        cmp     [autoclose],1
        je      close
        ret
@@:
        mov     dword [is_notify + 8], edx
        mcall   70, is_notify
        mcall   5,100
        jmp     close
;---------------------------------------------------------------------
draw_PathShow:
        pushad
        mcall   13,<15,280>,<32,16>,0xffffff
        push    dword PathShow_data_1
        call    [PathShow_draw]
        popad
        ret
;---------------------------------------------------------------------
save_ini:
        pushad
        stdcall _lstrlen,fname_buf
        invoke  ini_set_str,ini_file,apath,apath,fname_buf,eax
        invoke  ini_set_int,ini_file,asettings,aautoclose,[autoclose]
        popad
        ret
;---------------------------------------------------------------------
check_path:
stdcall _lstrlen,check_dir 
        mov     edi,check_dir
        add     edi,eax
@@:
        mov     byte [edi],0 
        dec     edi 
        cmp     byte [edi],'/' 
        jne     @b

if debug eq yes
dps     'read_folder_name: '
        mov     edx,check_dir
        call    debug_outstr
newline
end if
        mcall   70,read_folder
        ret
;---------------------------------------------------------------------
;---  Draw window  ---------------------------------------------------
;---------------------------------------------------------------------
draw_window:
        mcall   12,1

        mov     edx,[sc.work]
        or      edx,0x34000000
        mcall   0,<200,400>,<200,130>, , ,title

;buttons
        mcall   8,<198,70>,<68,20>,2,[sc.work_button]
        inc     edx
        mcall    ,<125,70>,
        inc     edx
        mcall    ,<300,75>,<30,20>

;labels
        mov     ecx,[sc.work_button_text]
        or      ecx,0x80000000
        mcall   4,<134,75>, ,save
        mcall    ,<215,75>, ,cansel
        mcall    ,<315,36>, ,select
        
        mov     ecx,[sc.work]
        mov     dword [frame_data.font_backgr_color],ecx
        push    dword frame_data
        call    [Frame_draw]

        call    draw_PathShow

        mcall   12,2
        ret

;---------------------------------------------------------------------
;---  Data  ----------------------------------------------------------
;---------------------------------------------------------------------
if lang eq ru
save            db 'Сохранить',0
cansel          db 'Отмена',0
select          db 'Изменить',0
label1          db ' Образ будет сохранен в: ',0
ok              db 'RAM-диск сохранен успешно в ',0
error1          db 'Не определена база и/или раздел жёсткого диска',0
error2          db 'Функция не поддерживается для данной файловой системы',0
error3          db 'Неизвестная файловая система',0
error4          db 'Странно... Ошибка 4',0
error5          db 'Несуществующий путь',0
error6          db 'Файл закончился',0
error7          db 'Указатель вне памяти приложения',0
error8          db 'Диск заполнен',0
error9          db 'Файловая структура разрушена',0
error10         db 'Доступ запрещён',0
error11         db 'Ошибка устройства',0
aUnknownError   db 'Неизвестная ошибка',0
rdError         db 'Нельзя сохранять образ в самого себя',0
error_msg       db 'Ошибка: ',0
;---------------------------------------------------------------------
else if lang eq et
save            db 'Salvesta',0
cansel          db 'Cansel',0
select          db ' Valige',0
label1          db ' RAM-drive will be saved as: ',0
ok              db 'RAM-ketas salvestatud edukalt ',0
error1          db 'Hard disk base and/or partition not defined',0
error2          db 'The file system does not support this function',0
error3          db 'Tundmatu failis№steem',0
error4          db 'Strange... Error 4',0
error5          db 'Vigane teekond',0
error6          db 'End of file',0
error7          db 'Pointer is outside of application memory',0
error8          db 'Ketas tфis',0
error9          db 'FAT tabel vigane',0
error10         db 'Juurdepффs keelatud',0
error11         db 'Seadme viga',0
aUnknownError   db 'Tundmatu viga',0
rdError         db 'You can't save image on itself',0
error_msg       db 'Viga: ',0
;---------------------------------------------------------------------
else if lang eq it
save            db '  Salva',0
cansel          db 'Cansel',0
select          db 'Seleziona',0
label1          db ' RAM-drive will be saved as: ',0
ok              db 'Il RAM-drivet e stato salvato ',0
error1          db 'Hard disk base and/or partition not defined',0
error2          db 'The file system does not support this function',0
error3          db 'Filesystem sconosciuto',0
error4          db 'Strange... Error 4',0
error5          db 'Percorso non valido',0
error6          db 'End of file',0
error7          db 'Pointer is outside of application memory',0
error8          db 'Disco pieno',0
error9          db 'Tabella FAT corrotta',0
error10         db 'Accesso negato',0
error11         db 'Errore di device',0
aUnknownError   db 'Errore sconosciuto',0
rdError         db 'You can't save image on itself',0
error_msg       db 'Errore: ',0
;---------------------------------------------------------------------
else
save            db '  Save',0
cansel          db 'Cansel',0
select          db ' Select',0
label1          db ' RAM-drive will be saved as: ',0
ok              db 'RAM-drive was saved successfully in ',0
error1          db 'Hard disk base and/or partition not defined',0
error2          db 'The file system does not support this function',0
error3          db 'Unknown file system',0
error4          db 'Strange... Error 4',0
error5          db 'Incorrect path',0
error6          db 'End of file',0
error7          db 'Pointer is outside of application memory',0
error8          db 'Disk is full',0
error9          db 'File structure is destroyed',0
error10         db 'Access denied',0
error11         db 'Device error',0
aUnknownError   db 'Unknown error',0
rdError         db 'You can't save image on itself',0
error_msg       db 'Error: ',0
end if
;---------------------------------------------------------------------
errors:
        dd      ok
        dd      error1
        dd      error2
        dd      error3
        dd      error4
        dd      error5
        dd      error6
        dd      error7
        dd      error8
        dd      error9
        dd      error10
        dd      error11
;---------------------------------------------------------------------

title   db appname,version,0

;Lib_DATA
;Всегда соблюдать последовательность в имени.
system_dir_Boxlib       db '/sys/lib/box_lib.obj',0
system_dir_ProcLib      db '/sys/lib/proc_lib.obj',0
system_dir_libini       db '/sys/lib/libini.obj',0
;---------------------------------------------------------------------
head_f_i:
head_f_l        db 'System error',0

err_message_found_lib1  db 'box_lib.obj - Not found!',0
err_message_found_lib2  db 'proc_lib.obj - Not found!',0
err_message_found_lib3  db 'libini.obj - Not found!',0

err_message_import1     db 'box_lib.obj - Wrong import!',0
err_message_import2     db 'proc_lib.obj - Wrong import!',0
err_message_import3     db 'libini.obj - Wrong import!',0
;---------------------------------------------------------------------
l_libs_start:

library01  l_libs system_dir_Boxlib+9, cur_dir_path, library_path, system_dir_Boxlib, \
err_message_found_lib1, head_f_l, Box_lib_import, err_message_import1, head_f_i

library02  l_libs system_dir_ProcLib+9, cur_dir_path, library_path, system_dir_ProcLib, \
err_message_found_lib2, head_f_l, ProcLib_import, err_message_import2, head_f_i

library03  l_libs system_dir_libini+9, cur_dir_path, library_path, system_dir_libini, \
err_message_found_lib3, head_f_l, libini_import, err_message_import3, head_f_i

end_l_libs:
;---------------------------------------------------------------------
OpenDialog_data:
.type                   dd 1    ; Save
.procinfo               dd procinfo     ;+4
.com_area_name          dd communication_area_name      ;+8
.com_area               dd 0    ;+12
.opendir_path           dd temp_dir_path        ;+16
.dir_default_path       dd communication_area_default_path      ;+20
.start_path             dd open_dialog_path     ;+24
.draw_window            dd draw_window  ;+28
.status                 dd 0    ;+32
.openfile_pach          dd fname_buf    ;+36
.filename_area          dd filename_area        ;+40
.filter_area            dd Filter
.x:
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 200 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 120 ;+54 ; Window Y position

communication_area_name:
        db 'FFFFFFFF_open_dialog',0
open_dialog_path:
if __nightbuild eq yes
    db '/sys/MANAGERS/opendial',0
else
    db '/sys/File Managers/opendial',0
end if
communication_area_default_path:
        db '/',0

Filter:
dd      Filter.end - Filter
.1:
db      'IMG',0
db      'IMA',0
.end:
db      0

start_temp_file_name:   db 'kolibri.img',0

;---------------------------------------------------------------------
align 4
ProcLib_import:
OpenDialog_Init         dd aOpenDialog_Init
OpenDialog_Start        dd aOpenDialog_Start
        dd      0
        dd      0
aOpenDialog_Init        db 'OpenDialog_init',0
aOpenDialog_Start       db 'OpenDialog_start',0
;---------------------------------------------------------------------
PathShow_data_1:
.type                   dd 0    ;+0
.start_y                dw 36   ;+4
.start_x                dw 20   ;+6
.font_size_x            dw 6    ;+8     ; 6 - for font 0, 8 - for font 1
.area_size_x            dw 270  ;+10
.font_number            dd 0    ;+12    ; 0 - monospace, 1 - variable
.background_flag        dd 0    ;+16
.font_color             dd 0    ;+20
.background_color       dd 0    ;+24
.text_pointer           dd fname_buf    ;+28
.work_area_pointer      dd text_work_area       ;+32
.temp_text_length       dd 0    ;+36
;---------------------------------------------------------------------
align 4
Box_lib_import:
;edit_box_draw           dd aEdit_box_draw
;edit_box_key            dd aEdit_box_key
;edit_box_mouse          dd aEdit_box_mouse
;version_ed              dd aVersion_ed

PathShow_prepare        dd sz_PathShow_prepare
PathShow_draw           dd sz_PathShow_draw
Frame_draw              dd sz_Frame_draw
                        dd 0
                        dd 0

;aEdit_box_draw          db 'edit_box',0
;aEdit_box_key           db 'edit_box_key',0
;aEdit_box_mouse         db 'edit_box_mouse',0
;aVersion_ed             db 'version_ed',0

sz_PathShow_prepare     db 'PathShow_prepare',0
sz_PathShow_draw        db 'PathShow_draw',0

sz_Frame_draw           db 'frame_draw',0
;szVersion_frame        db 'version_frame',0
;---------------------------------------------------------------------
frame_data:
.type                   dd 0 ;+0
.x:
.x_size                 dw 374 ;+4
.x_start                dw 8 ;+6
.y:
.y_size                 dw 45 ;+8
.y_start                dw 17 ;+10
.ext_fr_col             dd 0x888888 ;+12
.int_fr_col             dd 0xffffff ;+16
.draw_text_flag         dd 1 ;+20
.text_pointer           dd label1 ;+24
.text_position          dd 0 ;+28
.font_number            dd 0 ;+32
.font_size_y            dd 9 ;+36
.font_color             dd 0x0 ;+40
.font_backgr_color      dd 0xdddddd ;+44
;---------------------------------------------------------------------
align 4
libini_import:
init_lib     dd a_init
ini_get_str  dd aini_get_str
ini_get_int  dd aini_get_int
ini_set_str  dd aini_set_str
ini_set_int  dd aini_set_int
             dd 0
             dd 0
a_init       db 'lib_init',0
aini_get_str db 'ini_get_str',0
aini_get_int db 'ini_get_int',0
aini_set_str db 'ini_set_str',0
aini_set_int db 'ini_set_int',0
;---------------------------------------------------------------------

apath db 'path',0
asettings db 'settings',0
aautoclose db 'autoclose',0
path    db '/hd2/1/kolibri.img',0
ini_file db  '/sys/settings/rdsave.ini',0
;ini_file db  '/sys/rdsave.ini',0
;---------------------------------------------------------------------
is_notify:
    dd    7, 0, ok, 0, 0
    db    "/rd/1/@notify", 0
    
read_folder:
.subfunction    dd 1
.start          dd 0
.flags          dd 0
.size           dd 1
.return         dd folder_data
                db 0
.name:          dd check_dir

param dd 0
hidden dd 0
;---------------------------------------------------------------------
IM_END:
;---------------------------------------------------------------------
align 4
PARAMS:
       rb 256
ini_path:
        rb 4096
check_dir:
        rb 4096

sc     system_colors

autoclose rd 1

folder_data:
        rb 304*32+32 ; 9 Kb
;---------------------------------------------------------------------
cur_dir_path:
        rb 4096
;---------------------------------------------------------------------
library_path:
        rb 4096
;---------------------------------------------------------------------
temp_dir_path:
        rb 4096
;---------------------------------------------------------------------
fname_buf:
        rb 4096
;---------------------------------------------------------------------
procinfo:
        rb 1024
;---------------------------------------------------------------------
filename_area:
        rb 256
;---------------------------------------------------------------------
text_work_area:
        rb 1024
;---------------------------------------------------------------------
align 32
        rb 4096
stacktop:
I_END:  ; метка конца программы