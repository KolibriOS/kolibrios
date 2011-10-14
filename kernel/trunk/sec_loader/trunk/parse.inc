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

; Модуль парсинга - это стандартный компонент, встраиваемый во вторичный загрузчик.
; Данный модуль позволяет стандартно произвести разбор ini файла
; (и с использованием полученных данных ОС будет загружаться дальше).
; В начале найдем открывающий "[" - это будет указывать на начало
; секции. Поддерживается 1 секция это [loader], остальные секции могут иметь 
; любые имена, но они должны быть заключены в в скобки []
macro  use_parse
{
;input cx=size of ini file 
parse_start:
;es:di as 2000:0000 new segment
;установим указатель на загруженный блок
        enter   256, 0  ;set 16 byte for current task in stack
;we are is not use bp because bp is pointer on array 16 byte
        mov     word [save_bp_from_timer], bp   ;save point to own data array
        mov     save_cx, cx     ;it's placed size of ini file
        les     di, dword [file_data]
;обнулим все переменные выделенные из стека
;init flag
        xor     ax, ax
        mov     status_flag, ax
;set data size
        mov     info_real_mode_size, ini_data_ +0x1000   ;изменим значение занятости памяти

;поиск начала блока.
;///////////check [loader]
        cld

        mov     ret_on_ch, .start    ;set return 
        mov     al, byte [es:di]
        push    word .first_ret
        cmp     al, ' '
        jz      .first_sp_1
        jmp     get_firs_sym.not_space
.first_sp_1:
        jmp     get_firs_sym.first_sp

.start:
        call    get_firs_sym    ;get first symbol on new line
.first_ret:                     ;первый возврат
;        jcxz   .end_file       ;.end_loader     ;found or not found parametrs in section  exit in section
        test    cx, cx
        jz      error.not_loader
        cmp     al, '['
        jz      .parse_loader
        jmp     .start
;////// проверка на наличее секции loader
use_parse_loader
;pause
if DEBUG
        xor     ax, ax
        int     16h
end if  
;////// вывод графического экрана, выбор, секции под дефолту
use_any_sec
;парсинг выбраной или дефолтной секции т.е. разбор параметров выполнение сценария
use_parse_def_sect

;//////////////////
;/end parse block
;//////////////////
;.end_bl:
;        mov     cx,bx
;
;        jmp     .start

.exit:

;        mov     si,parse_ini_end
;        call    printplain
;        
;if DEBUG        
;        pusha
;        mov     ax,cx
;        mov     cx,0x0a
;        mov     di,show_db1_dec
;        mov     dword[ds:di],'    '
;        call    decode
;Show size
;        mov     si,show_db1
;        call    printplain
;
;        popa
;end if  
        jmp     $

;///////////////////procedure //////////
;input es:di - is pointer to date
;cx - counter 
;return: cx - status if =0 - end of date else es:di point to first symbol on new line

}
