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
; THIS SOFTWARE IS PROVIDED BY Alexey Teplov aka <Lrz> ''AS IS'' AND ANY
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

;start of the project 13.02.2008 year.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Secondary Loader copyright Alexey Teplov aka <Lrz> & K Soft team
;if you need log preproc
;/////////////
;include 'listing.inc'
;enable listing
;////////////
;;;;;;;;;;;;;;;;;;;;;;;;;;
;start of code:
;;;;;;;;;;;;;;;
DEBUG equ 1
root_dir_entry_count	equ	224	;кол-во элементов в корневой дирректории
;point_to_fat_struc	equ	0xA000	;временный буфер, куда будет размещена Fat таблица, и затем перенесена за 1 мб
ini_data_       	equ     0x2000  ;файл где размещен файл сценария загрузки, там происходит синтаксический разбор
size_show_section 	equ 	18
default_timeout_value	equ	5	;default value to timeout is will was some errors
flag_found_default	equ	0x1	;default value is found
flag_found_timeout	equ	0x2	;timeout value is found
flag_found_LM		equ	0x1	;found LM value
flag_found_RS		equ	0x2	;found RS value
flag_found_GTRFMS	equ	0x4	;found type RamFS
flag_found_RamdiskSector equ	0x8	;found RamdiskSector
flag_found_RamdiskCluster equ   0x16	;found RamdiskCluster
;statick data эти данные не предопределяются в течении выполнения всей программы.
save_cx		equ	word [bp-2]	;save cx size ini file
ret_on_ch	equ	word [bp-4]     ;point to return   разрушаемое значение
save_cx_d	equ	word [bp-6]     ;save cx - size default section and working section                                
status_flag	equ	word [bp-8]	;status flag
point_loader    equ	word [bp-10]
point_default   equ	word [bp-12]    ;point to default

;данные которые зависимы от ветки выполнения и которые могут быть переопределены в процессе выполнения программы.
point_to_hframe equ	word [bp-14]    ;point on start frame (for change section)
point_to_1	equ	word [bp-16]
point_to_2	equ	word [bp-18]
point_to_3	equ	word [bp-20]
point_to_4	equ	word [bp-22]
point_to_5	equ	word [bp-24]
point_to_6	equ	word [bp-26]
point_to_7	equ	word [bp-28]
point_to_8	equ	word [bp-30]
point_to_9	equ	word [bp-32]
point_to_10	equ	word [bp-34]
point_to_11	equ	word [bp-36]
point_to_12	equ	word [bp-38]
point_to_13	equ	word [bp-40]
point_to_14	equ	word [bp-42]
point_to_15	equ	word [bp-44]
point_to_16	equ	word [bp-46]
point_to_16	equ	word [bp-48]
point_to_17	equ	word [bp-50]
point_to_18	equ	word [bp-52]
;here array for fast scroling  16 word - poin to start section
point_to_point_def	equ	word [bp-54]
point_to_eframe equ	word [bp-56]    ;point on point frame



; тут расположено временное хранилище для cx и di при переходе на следующий буфер при поиске секций
find_sec_di	equ	word [bp-58]	;тут будет храниться di
info_real_mode_size	equ 	word [bp-60];тут храниться информация о занятой области т.е. размер, можно узнать сколько осталось места вычислив
free_ad_memory  equ	word	[bp-62] ;сколько у нас расширенной памяти для формирования рам диска и загрузки модулей
show_errors_sect equ    word    [bp-64] ;переменая которая хранит биты ошибок для каждой логической секции. 
save_descript_size	equ  	word	[bp-66]	;save descript size previos section сохраним размер предыдущей секции которую выводили
save_ramdisksize	equ 	dword	[bp-70] ;save size of ramdisk in byte
save_file_size		equ	dword   [bp-74] ;save size of reading file
set_ramfs		equ	word	[bp-76]	;определенный тип файловой системы,нужно для формирования рам диска
point_next_fat_str	equ	word	[bp-78] ;указатель на следующий элемент fat таблицы
size_root_dir		equ	word	[bp-80] ;кол-во элементов в секторах по 512 байт корневой директории
firstDataSect		equ	word	[bp-82] ;первый сектор данных в сеторах от 0
DataClasters		equ	word	[bp-84] ;размер массива доступной для записи данных в кластерах.
point_to_free_root	equ	word	[bp-86]	;указатель на следующий пустую запись в рут дир
point_to_dest_file_name equ	word	[bp-88]	;указывает на начало имени файла назначения.
data_offset		equ	word	[bp-90] ;смещение в кластерах для записанных данных т.е перекинутых за 1-й мб
first_input		equ	word	[bp-92]	;поле для флагов в преобразовании имени.
save_di_RAMDISK		equ	word	[bp-94]	;сохраним di -указателя при обработке секции
save_cx_RAMDISK		equ	word	[bp-96]	;сохраним размер остатка секции
status_flag_loader_f	equ	word	[bp-98] ;сохраним результат выполенения загрузки файла
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;данные которые используются при обработке секции, т.е. после нажатия Enter, уже не возможно вернуться в первоначальный экран
;для возврата, необходимо перезапустить полностью код т.е. стартовать с 0х1000:0000




use16
                  org   0x0
                  jmp   start
include 'boot_st.inc'
include 'debug_msg.inc'   ;here is message from debug
include 'parse_dat.inc'
include 'sl_proc.inc'
include 'parse.inc'
include 'parse_loader.inc'
include 'parse_any.inc'
include 'parse_def_sect.inc'
include	'parse_err.inc'

file_data  dw 0x0,ini_data_  ;формат: смещение: сегмент т.к. используется les
size_data  dw 16 ;16 блоков по 4 кб т.е предел до 64 кб
name_ini_f db 'kord/startos.ini',0 

;////////////
loader_callback dd ?
load_drive dw ?
load_ft    dw ?
;Start code

start:
; Save far pointer to callback procedure, ds:si is point
        mov     word [cs:loader_callback], si
        mov     word [cs:loader_callback+2], ds
; Save type of drive
        mov     word [cs:load_drive],ax
; Save type of FT
        mov     word [cs:load_ft],bx
; set up stack
        mov     ax, cs
        mov     ss, ax
        xor     sp, sp
; set up segment registers
        mov     ds,ax
        mov     es,ax
; just to be sure: force DF=0, IF=1
        cld
        sti

; set videomode
        mov     ax,3
        int     0x10

        mov     si,version
        call    printplain
	mov	al,'#'
	mov	cx,80
;input cx=size al=char будет вывден символ сколько раз указано в cx
@@:
	call	putchar
	loop	@b



  if DEBUG
        mov si,stack_msg
        call printplain
  end if

; Require 586 or higher processor (cpuid and rdtsc,rdmsr/wrmsr commands)
; install int 6 (#UD) handler
        xor     bx, bx
        mov     ds, bx
        push    word [bx+6*4+2]
        push    word [bx+6*4]
        mov     word [bx+6*4], ud16
        mov     word [bx+6*4+2], cs
; issue CPUID command
        xor     eax, eax        ; N.B.: will cause #UD before 386
        cpuid                   ; N.B.: will cause #UD before later 486s
        test    eax, eax
        jz      cpubad
; get processor features
        xor     eax, eax
        inc     ax
        cpuid
        test    dl, 10h         ; CPUID[1].edx[4] - TSC support
        jz      cpubad
        test    dl, 20h         ; CPUID[1].edx[5] - MSR support
        jnz     cpugood

ud16:   ; #UD handler, called if processor did not recognize some commands
cpubad:
; restore int 6 (#UD) handler
        pop     word [6*4]
        pop     word [6*4+2]
; say error
        push    cs
        pop     ds
        mov     si, badprocessor
sayerr:
        call    printplain
        jmp     $

cpugood:
; restore int 6 (#UD) handler
        pop     dword [6*4]
        push    cs
        pop     ds

; set up esp
        movzx   esp, sp

;Load  startos.ini
        xor     ax,ax   ; function 1 - read file
        mov     di,file_data
	inc	ax
        call    far  dword [loader_callback]
        push    cs
	push	cs
        pop     ds
        pop     es

        test    bx,bx
        jnz     error_ini

;Check config file in current dir
        push    ax      ;save size file
  if DEBUG
        mov     cx,0x0a
        mov     di,show_decode
        call    decode
;Show size
        mov     si,show_string
        call    printplain
  end if


;Show message
        mov     si,load_ini
        call    printplain

        pop     cx      ;restore size file
use_parse               ;parsing startos.ini
;
        jmp     ini_loaded

;SET DEFAULT Not use ini file 
error_ini:
        mov     si, error_ini_f1    ;Error: cannot load ini file, buffer is full
        dec     bx
        jz      err_show_ini
        mov     si, error_ini_f2    ;Error: ini file not found
        dec     bx
        jz      err_show_ini
        mov     si, error_ini_f3    ;Error: cannot read ini file
        dec     bx
        jz      err_show_ini

        mov     si, error_ini_nf    ;Error: unrecognized error when loading ini file
err_show_ini:
        call    printplain
        mov     si, error_ini_common
        call    printplain
; wait for keypress
        mov     ah, 0
        int     16h

ini_loaded:

        jmp $




;;;;;;;;;;;;;;;;;;;;;;;;;;;
;DATA
;;;;;;;;;;;;;;;;;;;;;;;;;;;
; table for move to extended memory (int 15h, ah=87h)
align 8
table_15_87:
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

        db      0xff,0xff
	db	0x0,0x10 
	db	0x00,0x93,0x0,0x0

        db      0xff,0xff,0x0,0x00,0x10,0x93,0x0,0x0

        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0
        db      0x00,0x00,0x0,0x00,0x00,0x00,0x0,0x0

fat12_buffer:
.BS_jmpBoot 		db 0x90,0x90,0x90	;3 байта   NOP инструкция - ничего не делать 
.BS_OEMName 		db 'K SyS 64'	;8 байт
.BPB_BytsPerSec 	dw	512	;кол-во байтов в секторе может быть любое 512 1024 2048 4096 2 байта
.BPB_SecPerClus 	db	0x1	;кол-во секторов в кластере
.BPB_RsvdSecCnt		dw	0x1	;для FAt12/16 только 1, для FAT32 обычно 32	
.BPB_NumFATs		db	0x1	;кол-во фат таблиц, на тот случай если будет сброс на дискету образа рам диска
.BPB_RootEntCnt		dw	512     ;для мак совместимости с fat16 
.BPB_TotSec16		dw	0x0	;кл-во секторов
.BPB_Media		db	0xF0
.BPB_FATSz16		dw	0x0
.BPB_SecPerTrk 		dw	0x0	;содержит геометрию диска для RAMFS на как бы без разницы, пока пустое поле, позже внести реальные значения.
.BPB_NumHeads		dw	0x0
.BPB_HiddSec 		dd	0x0	;кол-во скрытых секторов
.BPB_TotSec32		dd	0x0
.BS_DrvNum		db	'R'	;от слова RAM
.BS_Reserved1		db	0x0
.BS_BootSig		db	0x29
.BS_VolID		db	'RFKS'	
.BS_VolLab		db	'RAM DISK FS'	;11 символов
.BS_FilSysType		db	'FAT12   '	;8 символов
;62 байта структура fat12.




;структура для дирректории fat
struc FAT_32_entry ;Byte Directory Entry Structure
{
.DIR_Name	rb	11
.DIR_Attr	db	?
.DIR_NTRes	db	?
.DIR_CrtTimeTenth db	?
.DIR_CrtTime	dw	?
.DIR_CrtDate	dw	?
.DIR_LstAccDate dw	?
.DIR_FstClusHI	dw	?
.DIR_WrtTime	dw	?
.DIR_WrtDate	dw	?
.DIR_FstClusLO	dw	?
.DIR_FileSize	dd	?


}
;Тут будут распологатсья данные, которые затруднительно распологать в стековой области....
;;;
;timer
shot_name_fat 	rb	11
if DEBUG
		rb	1	;нужен для отладки и вывода имени файла после преобразования
dest_name_fat	rb	12
end if
value_timeout   rw	1	;value to timeout
old_timer	rd	1	;старое значение вектора таймера
start_timer	rd	1	;значение таймера
timer_		rd	1       ;новое значение вектора таймера т.е. SL
start_stack	rw	1	;save stack 
save_bp_from_timer rw	1	;save bp from timer

