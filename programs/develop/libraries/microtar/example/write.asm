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
        ccall [mtar_open], tar, tar_fname, tar_fmode ; Создаём новый файл 'test.tar'
        ccall [mtar_write_file_header], tar, test1_txt , str1_len ; Создаём внутри 'test.tar' пустрой файл 'test1.txt'
        
        ccall [mtar_write_data], tar, str1,  str1_len ; Записываем данныев в этот файл
        
        ccall [mtar_finalize], tar  ; Указываем что больше с tar работать не будем
        ccall [mtar_close], tar ; Закрываем 'test.tar'
        
exit:
        mcall   SF_TERMINATE_PROCESS ; Выходим из программы
        
; data

str1 db 'Hello world!', 0
str1_len = $ - str1

str2 db 'Goodbye world!', 0
       
tar_fname db 'test.tar', 0
tar_fmode db 'w', 0

test1_txt db 'test1.txt', 0

tar rb 32 

align 4

@IMPORT:
library mtar,   'mtar.obj'
import  mtar,   \
        mtar_init, 'mtar_init', \
        mtar_open, 'mtar_open', \
        mtar_write_file_header, 'mtar_write_file_header', \
        mtar_write_data, 'mtar_write_data', \
        mtar_finalize, 'mtar_finalize', \
        mtar_close, 'mtar_close'

IM_END:
align   4
rb      4096    ; stack
MEM:  
