format binary as "kex"

use32
        org     0x0
        db      'MENUET01'      
        dd      0x01            
        dd      START           
        dd      IM_END          
        dd      MEM  
        dd      MEM
        dd      0
        dd      0

include '../../../../macros.inc'
include '../../../../proc32.inc'
include '../../../../KOSfuncs.inc'
include '../../../../dll.inc'
include '../mtar.inc'
;include '../../../../debug-fdo.inc'

;__DEBUG__               = 1
;__DEBUG_LEVEL__         = 2   


START:
        stdcall dll.Load, @IMPORT ; Имортироуем функции из mtar.obj
        test    eax, eax
        jnz     exit
     
        ccall [mtar_init] ; Инициализируем библиотеку (на самом деле подгружается libc.obj
        ccall [mtar_open], tar, tar_fname, tar_fmode ; Открываем для чтения файл 'test.tar'
        
       ; DEBUGF 2, "%d", eax
        
print_next:
        ccall [mtar_read_header], tar, header ; Читаем заголовок
        cmp eax, MTAR_ENULLRECORD ; Если заголовок не был прочитан (return -7) выходим из цикла
        je exit
        ccall [printf], format_str, header+mtar_header_t.name, dword[header+mtar_header_t.size] ; Выводим в консоль имя файла и размер в байтах
        ccall [mtar_next], tar ; Переходим к следующему заголовку
        jmp print_next ; прыгаем в начало цикла
        
exit:
        ccall [mtar_close], tar ; Закрываем 'test.tar'
        mcall SF_TERMINATE_PROCESS ; Выходим из программы
        
; data
       
tar_fname db 'test.tar', 0
tar_fmode db 'r', 0

tar    rb sizeof.mtar_t
header rb sizeof.mtar_header_t 

format_str db '%-10s (%-4d bytes)', 0x0A,0

align 4

@IMPORT:
library mtar,   'mtar.obj', libc , 'libc.obj'
import  mtar,   \
        mtar_init, 'mtar_init', \
        mtar_open, 'mtar_open', \
        mtar_next, 'mtar_next', \
        mtar_strerror, 'mtar_strerror', \
        mtar_read_header, 'mtar_read_header', \
        mtar_write_data, 'mtar_write_data', \
        mtar_finalize, 'mtar_finalize', \
        mtar_close, 'mtar_close'

import  libc,   \
        printf, 'printf'


IM_END:
align   4
rb      4096    ; stack
MEM:  
