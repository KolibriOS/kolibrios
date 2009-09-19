; Copyright (c) 2009, <Lrz>
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;       * Redistributions of source code must retain the above copyright
;       notice, this list of conditions and the following disclaimer.
;       * Redistributions in binary form must reproduce the above copyright
;       notice, this list of conditions and the following disclaimer in the
;       documentation and/or other materials provided with the distribution.
;       * Neither the name of the <organization> nor the
;       names of its contributors may be used to endorse or promote products
;       derived from this software without specific prior written permission.
;
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov nickname <Lrz> ''AS IS'' AND ANY
; EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
; WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
; DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
; DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
; (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
; ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
; (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
; SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;*****************************************************************************

;;;;;;;;;;;;;;;;;;

format MS COFF

public EXPORTS

section '.flat' code readable align 16
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Get info about 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
include '../../../../macros.inc'
align 4
initialization_font:
; esp+0 = dd back
; заполняем массив - структу для получения кол-ва файлов

pushad
        xor     eax,eax
        inc     eax
        mov     [number_function],eax
;structure is completed
        mov     eax,70
        mov     ebx,dword struct_f_info
        mcall
;IF ERROR FT then exit       
        test    eax,eax
        jnz     .exit
;IF ERROR folder is not found then exit
        test    ebx,ebx
        mov     eax,ebx
        jnz     .exit

;Get quantity files
        mov     eax,[buffer_32.quantity_files]
        imul    eax,304
        add     eax,32          ; ??
        push    eax         ;save eax в eax размер нужной нам памяти в байтах
;init memory
        mov     eax,68
        mov     ebx,11
        mcall
        test    eax,eax
        jz     .exit
        inc     ebx     ;выделить блок в ecx байт

        pop     ecx
        mov     eax,68
        mcall
        mov     dword [fulder_array_point],eax  ;save point for memory's fulder
;в eax указатель на блок
        mov     [buffer_read_d],eax             ;point to new array
        mov     eax,[buffer_32.quantity_files]
        mov     [read_block],eax
;get in buffer info of files and sub dir current dir 
        mov     eax,70
        mov     ebx,dword struct_f_info
        mcall
;получили информацию о файлах.
        mov     ebp, dword [buffer_read_d] ;point to structure
        mov     ecx, dword [ebp+4]         ;кол-во блоков БДВК
        mov     dword [save_ecx],ecx
        add     ebp, 32 ;смещаемся до БДВК
        mov     dword [save_ebp],ebp
        cld     ;установка флага направления
align 4 
.start_loop:
        push    ecx
        lea     edi,[ebp+0x28] ;в edi указатель на начало имени.
        xor     eax,eax
        mov     ecx,263
        repne    scasb   ;найдем конец строки
;;;;;;;;;;;;;;;;;;;;;;;
        sub     edi,5   ;.ksf,0
        lea     esi,[name_font]
        mov     eax,dword[edi]
        or      eax,0x20202000  ; сделаем расширение строчными буквами.
        cmp     eax,dword[esi]
        jnz     @f

;;;;;;;; ура это наш файл т.е. наш шрифт мы нашли нужный файлик ))
;теперь нужно точно убедиться что это наш и только наш файлик 
;загрузим заголовок файла и узнаем его размер глифа.

        call    alloc_mem       ; выделим озу своих переменных
; сейчас нам нужно сформировать имя файла с полным путем        
        mov     eax,dword [alloc_memory_point]
        lea     edi,dword [eax+4]
        lea     esi,[name_fulder]
        mov     ecx,name_fuld_end
        rep     movsb   ;переносим путь
        mov     al,'/'
        stosb
        lea     esi,[ebp+0x28] ;в esi указатель на начало имени.
align 4
.loop_b:
        lodsb
        stosb
        test    al,al
        jnz     .loop_b

;        mov     edi,name_of_file ; [ebp+0x28] ;в edi указатель на начало имени.       
;get in buffer info above own file
        xor     eax,eax
        mov     dword [number_function],eax
        mov     dword [index_start_block],eax   ;позиция в файле для чтения данных
        mov     dword [flags_],eax              ;старшая позиция для чтения данных
        mov     dword [read_block],8            ;сколько байт читать нам нужно 8 байт
        mov     dword [buffer_read_d],file_buffer ; указатель на буфер куда будут записаны данные

        mov     eax,dword [alloc_memory_point]
        lea     edi,dword [eax+4]
        mov     dword [offset_to_string],edi

        mov     eax,70
        mov     ebx,dword struct_f_info
        mcall
;;;;;;;;;;;;;;;; проверим формат заголовка
        mov     eax,ebx
        mov     eax,dword [file_buffer]
        mov     ebx,dword [type_fnt]
        cmp     eax,ebx
        jnz     @f

;;;;;;; отлично сейчас проверили что заголовок именно тот который нам нужен
; сейчас нужно перераспределить блоки т.е. по мере нахождения будем переносить блоки
; в начало буфера

        mov     dword [save_point_nt],ebp
        pop     ecx
        xor     eax,eax
        jmp     .exit

align 4
@@:     pop     ecx
        add     ebp,304
        dec     ecx
        jnz     .start_loop
;        loop    .start_loop
        or     eax,-1
;;;;;;;; прошлись - все интересущющие шрифты переместили в начало блока.

align 4
.exit:  mov     dword [esp+28],eax
popad
ret


align 4
get_font:
; поиск в массиве шрифта при совпадении уcловий загрузка шрифта
; esp+4 = dd width font shl 16 +hight font
; esp+0 = dd back
;pop     eax
;        pop     dword [font_x_y]
;push    eax
pushad
        mov     eax,dword [esp+32+4]
        mov     dword [font_x_y],eax
        xor     eax,eax
        mov     dword [number_function],eax
        mov     dword [index_start_block],eax   ;позиция в файле для чтения данных
        mov     dword [flags_],eax              ;старшая позиция для чтения данных
        mov     dword [read_block],8            ;сколько байт читать нам нужно 8 байт
        mov     dword [buffer_read_d],file_buffer ; указатель на буфер куда будут записаны данные

        mov     eax,dword [alloc_memory_point]
        lea     edi,dword [eax+4]
        mov     dword [offset_to_string],edi


        mov     eax,70
        mov     ebx,dword struct_f_info
        mcall
;;;;;;;;;;;;;;;; проверим формат заголовка
        mov     eax,ebx
        mov     eax,dword [file_buffer]
        mov     ebx,dword [type_fnt]
        cmp     eax,ebx

        mov     ecx,dword [file_buffer.font_size]
        mov     edx,dword [font_x_y]
        cmp     ecx,edx

        mov     ecx, 4096;dword [ebp+32]     ;размер файла до 4294967296 байт т.е. 4 Гб

        mov     dword [read_block],ecx
        mov     eax,68
        mov     ebx,12
        mcall

        mov     dword [font_array_point],eax    ;save point to array
        mov     dword [buffer_read_d],eax

;load font
        mov     ebx,dword struct_f_info
        mov     eax,70
        mcall
        test    eax,eax
        jnz     .exit 

        lea     eax,[buffer_read_d]
        mov     eax,dword [eax]
        mov     ebx,dword [eax+8]
        add     eax,ebx
;        add     eax,dword [eax+8]
;        add     eax,dword [buffer_read_d]
        mov     dword [font_array_data],eax
        xor     eax,eax
        jmp     .ok

;here error file system
align 4
.exit:
        or      eax,-1
align 4
.ok:    mov     dword [esp+28],eax       ;вернем ошибку 
        popad
        ret 4


; поиск вывод сторочки по символьно сфорированного шрифтом текста
; esp+12= dd x shl 16 + y x- координата по Х, y - координата по Y
; esp+8 = dd point to color of background and font
; esp+4 = dd point to ASCIIZ
; esp+0 = dd back
align 4
font_draw_on_string:
pushad

        mov     esi,dword [esp+4+32] ;ASCIIZ
        mov     edx,dword [esp+12+32]  ; x shl 16 +y
        mov     ecx,dword [font_x_y]  ;размер глифа x shl 16 +y
        mov     edi,dword [esp+8+32]     ;указатель на палитру из цвета и фона 
        xor     ebp,ebp         ;см 65 функцию )) 
        cld
align 4
@@:     ;pushad
        xor     eax,eax
        lodsb
        test    al,al
        jz      .return
        shl     eax,4         ;умножаем на 16 т.к. это высота глифа

        mov     ebx,dword [font_array_data]    ; тут распологаются глифы символов
        add     ebx,eax
        push    esi
        mov     esi,1
        mov     eax,65
        mcall
        pop     esi
        add     edx,8 shl 16    ;следующий символ ширина символа = 8 точкам
        jmp     @b

align 4
.return: 
popad
ret 12


align 4
; esp+4 = dd point to color of background and font
show_all_glif:
pushad

        mov     edx,10 shl 16+20 ;dword [esp+12+32]  ; x shl 16 +y
        mov     ecx,128 shl 16 +256;dword [font_x_y]  ;размер глифа x shl 16 +y
        mov     edi,dword [esp+4+32]     ;указатель на палитру из цвета и фона 
        xor     ebp,ebp         ;см 65 функцию )) 

align 4
@@:     
        mov     ebx,dword [font_array_data]    ; тут распологаются глифы символов
        mov     esi,1
        mov     eax,65
        mcall
popad
ret 4






align 4
free_fulder_info:
;освободить память выделеную для получения блоков БДВК
pushad
        mov     ecx, dword [fulder_array_point]
        mov     eax,68
        mov     ebx,13
        mcall
        test    eax,eax
;        jnz     @f
        mov      dword [esp+28],eax       ;вернем ошибку 
;align 4
;@@:
popad
        ret
align 4
free_font:
;освободить память выделеную для получения блоков БДВК
pushad
        mov     ecx, dword [font_array_point]
        mov     eax,68
        mov     ebx,13
        mcall
        test    eax,eax
;        jnz     @f
        mov      dword [esp+28],eax       ;вернем ошибку 
;align 4
;@@:
popad
        ret

align 4
; alloc mem get 4 Kb for own data
alloc_mem:
        pushad
        mov     eax, dword [alloc_memory_point]
        test    eax,eax
        jnz     .mem_allocating
        mov     eax,68
        mov     ebx,12
        mov     ecx,4096
        mcall
        mov     dword [alloc_memory_point],eax  ; при первом вхождении выделяем 4 кб ОЗУ и сохраняем указатель
        mov     dword [eax],-1
        popad
        ret
align 4
.mem_allocating:
        mov     ebp, dword [eax]
        mov     eax,ebp
        cmp     eax, -1
        jnz     .mem_allocating

        mov     eax,68
        mov     ebx,12
        mov     ecx,4096
        mcall
        mov     dword [ebp],eax ; при первом вхождении выделяем 4 кб ОЗУ и сохраняем указатель
        mov     dword [ebp],-1
        popad
        ret

align 16
EXPORTS:

        dd      sz_initialization_font, initialization_font
        dd      sz_get_font,            get_font
        dd      sz_free_fulder_info,    free_fulder_info
        dd      sz_free_font,           free_font
        dd      sz_font_draw_on_string, font_draw_on_string
        dd      sz_show_all_glif,       show_all_glif
;        dd      szVersion_ch,           0x00000001
;        dd      sz_option_box_draw,     option_box_draw
;        dd      sz_option_box_mouse,    option_box_mouse
        dd      szVersion_fn,           0x00000001
        dd      0,0

sz_initialization_font db 'initialization_font',0
sz_get_font            db 'get_font',0
sz_free_fulder_info    db 'free_fulder_info',0
sz_free_font           db 'free_font',0
sz_font_draw_on_string db 'font_draw_on_string',0
sz_show_all_glif       db 'show_all_glif',0
;sz_check_box_mouse     db 'check_box_mouse',0
;szVersion_ch           db 'version_ch',0
;sz_option_box_draw     db 'option_box_draw',0
;sz_option_box_mouse    db 'option_box_mouse',0
szVersion_fn           db 'version_fn',0

;;;;;;;;;;;
;;Data
;;;;;;;;;;;
align 4
;buffer_font file 'font_8x16.fon' ;шрифт которым будем выводить raw формат 1bpp
save_ecx        dd      0x0
;buffer_font file 'font01.ksf' ;шрифт которым будем выводить raw формат 1bpp
alloc_memory_point      dd      0x0     ; point to mem block in RAM 4096-4
;структура такая в начале выделеного блока размещается указатель на другой выделлный блок в 4096-4, если это блок последний, то dd -1
font_x_y          dd 0x0
fulder_array_point dd 0x0
font_array_point  dd 0x0
font_array_data   dd 0x0        ; реальные данные т.е. матрица шрифта
save_ebp          dd 0x0
save_point_nt     dd 0x0
name_fulder       db '/sys/FONTS',0
name_fuld_end=     ($-name_fulder) -1
name_font         db '.ksf',0
type_fnt          db 'kf01'



align 16
struct_f_info:
number_function   dd 0x0
index_start_block dd 0x0
flags_            dd 0x0
read_block        dd 0x0
buffer_read_d     dd buffer_32
free_byte         db 0x0
offset_to_string  dd name_fulder

align 16
file_buffer:
.type_fnt       dd 0x0
.font_size      dd 0x0

align 16
buffer_32:
.head             dd ?
.quantity_block   dd ?
.quantity_files   dd ?
                  rb 20
