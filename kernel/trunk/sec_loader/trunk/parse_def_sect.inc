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

; в этой секции идет разбор параметров указатель на секцию храниться в point_default
;типы ошибок при обработке макроса
;Макрос RamdiskFS
;/определение флагов в записи корневой директории
ATTR_READ_ONLY  equ     0x01
ATTR_HIDDEN     equ     0x02
ATTR_SYSTEM     equ     0x04
ATTR_VOLUME_ID  equ     0x08
ATTR_DIRECTORY  equ     0x10
ATTR_ARCHIVE    equ     0x20



show_error_1    equ     0x1     ;кончились данные - не запланированный конец секции
show_error_2    equ     0x2     ;нет завершающего символа в размере рам диска.
show_error_3    equ     0x4     ; рам диск будет иметь размер =64 кб. 
show_error_4    equ     0x8     ;

macro use_parse_def_sect
{
        mov     di,point_default
        push    ini_data_
        pop     es
        mov     si,point_to_point_def
        sub     si,2
        mov     cx,[si]         ;загрузим указатель наследующию секцию

        xor     ax,ax   ;обнулим аx для очистки флагов  

        sub     cx,di           ;вот теперь имеем истиный размер
        mov     save_cx_d,cx    ;сохраним значение cx своей переменной
;обнулим переменную флагов, это необходимо, для того, что бы избежать обработку повторяющихся значений

        mov     status_flag,ax
;;;;
;ВХод в обработку парсинга значений секций. es:di - указатель на начало секции cx размер секции доступной для парсинга
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;соглашение не разрушаем bp, es, cs, sp         
;use_Loader_Image       ;загрузить образ выше 1 мб
use_RamdiskFS
;проверяется самый последний.
use_LoaderModule    ;особенность - передает управление на загруженный модуль.
}

macro use_LoaderModule
;как вариант сейчас используется модель, при загрузке модуля на него передается управление, решение временое
;управление будет передаваться только после обработки всей секции 
{
local .found_end_str

        mov     di,point_default   ;restore value
        mov     cx,save_cx_d
;обработка конструкции типа LoaderModule=kord/kolibri.ldm
.start_p_LM:
        call    get_firs_sym    ;get first symbol on new line
        test    cx,cx
        jz      ._afterLoaderModule     ;нету? ну ладно - следующее значение тогда ) 
        cmp     al,'L'
        jnz     .start_p_LM
;проверка на значение LoaderModule
;        parse_LoaderModule
        mov     bx,cx
        mov     ax,di

        mov     si,parse_LoaderModule
        mov     cx,parse_LoaderModule_e - parse_LoaderModule
        repe    cmpsb
        jnz     .rest_value_loop_LM        ;is not compare

        sub     bx,parse_LoaderModule_e - parse_LoaderModule ;correct cx
        add     bx,cx
        mov     cx,bx

        test    status_flag,flag_found_LM               ;оценка флагов
        jz      .correct_is_not_set_LM          

;       mov     si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
;       call    printplain
;       jmp     .get_next_str

.correct_is_not_set_LM:
        mov     ax,0x3d20          ;cut al=' ' ah='='
        repe    scasb 
        jcxz    .rest_value_loop_LM          ;not found param timeout
        
        cmp     ah,byte [es:di-1]    ;find '='
        jnz     .rest_value_loop_LM
        
        repe    scasb              ;cut ' '
        inc     cx
        dec     di
;di указывает на начало блока информации, в cx длинна до конца секции.
;после загрузки заноситься значение занятой памяти.
;для того что бы загрузить модуль, воспользуемся callback сервисом
;оригинальное решение - разместим dd перед строчкой и после строчки разместим byte =0
;это выглядит так: в ini файле существует строчка LoaderModule = kord/kernel.loader
;мы ее модифицируем до такого состояния       dw,dw,db'kord/kernel.loader',0 конечно сохранив те значения которые мы заменяем
;сохранили певые 2 word
        push    dword [es:di-6]
        lea     si,[di-6]

        push    word  [es:di-2]
        xor     ax,ax
        mov     word [es:di-6],ax       ;вносим нужные значения
;info_real_mode_size размер и указатель на область в которую можно загрузиться
        mov     ax,info_real_mode_size  ;0x3000   ;следующий сегмент за данными


        mov     word [es:di-4],ax
        mov     word [es:di-2],16       ;кол-во блоков по 4 кб =64 кб т.е. больше не считаем 
;;;;;; поиск конца строчки
@@:     mov     al,byte [es:di]
        cmp     al,' '
        jz      .found_end_str
        cmp     al,0xa
        jz      .found_end_str
        cmp     al,0xd
        jz      .found_end_str
        inc     di
        dec     cx
        jnz     @b
;;;not found допустим,что это конец файла и он не имеет привычного заверешния строки
.found_end_str:

        push    word [es:di]
        xor     ax,ax
        mov     word [es:di],ax
;        xor     ax,ax   ; function 1 - read file
        mov     di,si   ;file_data
        inc     ax
        push    si
        push    es

        push    es
        pop     ds
        push    cs
        pop     es

        call    far  dword [es:loader_callback]

        push    cs
        pop     ds

        pop     es
        pop     si

        test    bx,bx
        jnz     .error_LM


        jmp     far     dword [es:si]


.error_LM:
        call    error.LoaderModule
.rest_value_loop_LM:
        mov     di,ax
        mov     cx,bx
        jmp     .start_p_LM

._afterLoaderModule:
}

macro use_RamdiskFS
; формирование рам диска, + обработка всего связанного.
{
if DEBUG
local ._not_memory_in_sys
;//////// clear window
        mov     ax,3
        int     0x10
;\\\\\\\\\ clear window is end
        mov     si,ramdiskFS_st
        call    printplain
end if
; обнулим регистр состояния ошибок
        xor     ax,ax
        mov     show_errors_sect,ax
use_free_memory ; узнаем какого объема у нас доступна память. значение возаращается в ax
;узнаем сколько у нас есть памяти и сможем ли мы сформировать нужного размера рам диск.
use_RamdiskSize ;значение возвращается в bx
        cmp     free_ad_memory,bx       ; размерность в кб.
        jbe     ._not_memory_in_sys
        movzx   eax,bx
        shl     eax,10  ;*1024 = get size in byte
        mov     save_ramdisksize,eax    ; сорханим размер в byte

get_type_FS     ;получим тип файловой системы + создадим ее 

        
._not_memory_in_sys:
        
if DEBUG
;pause
        xor     ax,ax
        int     0x16
end if
}
macro use_RamdiskSize
{
local .start_p_RS
local .correct_is_not_set_RS
local .CS
local .correct_val_RS
local .correct_size_RS
local .rest_value_loop_RS
local .end_get_RS_ERROR_1
local .end_get_RS_ERROR_2
local ._end_parse_RS
;обрабатывается размер формируемого рам диска
;загрузим начало секции, т.к. будем просматривать с начала и всю секцию
        mov     di,point_default   ;restore value
        mov     cx,save_cx_d
.start_p_RS:
        call    get_firs_sym    ;get first symbol on new line
        test    cx,cx
        jz      ._end_parse_RS  ;нету? ну ладно - следующее значение тогда ) 
        cmp     al,'R'
        jnz     .start_p_RS     
;проверка на значения RamdiskSize
;        parse_RamdiskSize
        mov     bx,cx
        mov     ax,di

        mov     si,parse_RamdiskSize
        mov     cx,parse_RamdiskSize_e - parse_RamdiskSize
        repe    cmpsb
        jnz     .rest_value_loop_RS    ;is not compare

        sub     bx,parse_RamdiskSize_e - parse_RamdiskSize ;correct cx
        add     bx,cx
        mov     cx,bx

        test    status_flag,flag_found_RS               ;оценка флагов
        jz      .correct_is_not_set_RS          

;       mov     si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
;       call    printplain
;       jmp     .get_next_str

.correct_is_not_set_RS:
        mov     ax,0x3d20          ;cut al=' ' ah='='
        repe    scasb 
        jcxz    .end_get_RS_ERROR_1          ;not found param 
        
        cmp     ah,byte [es:di-1]    ;find '='
        jnz     .start_p_RS                     ; перейдем на начало и попробуем найти еще секцию 
        
        repe    scasb              ;cut ' '
        inc     cx
        dec     di
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Тут нужно преобразовывать строчку в цифровое значение.
;;;;;;;;;;;;;;;;;;;;;;;;;;
        xor     bx,bx
        mov     cx,5
@@:     mov     al,byte [es:di]
        cmp     al,'0'
        jb      .CS
        cmp     al,'9'
        jbe     .correct_val_RS
.CS:
        cmp     al,'K'
        jz      .correct_size_RS
        jmp     .end_get_RS_ERROR_2
.correct_val_RS:        
        imul    bx,10
        xor     al,0x30
        add     bl,al
        inc     di
        loop    @b

.correct_size_RS:
;возможен 1 вариант, когда размер задан в K киллобайтах
;внутренный формат данных это кол-во запрощеной памяти в кб.
        test    bx,bx
        jnz     @f      ;если значение отлично от 0
;;;;;сообщение об ошибке, размер "найденого" блока =0 минимально мы должны
;установить 64 кб размер рам диска.
        or      show_errors_sect,show_error_3
        mov     bx,64
@@:
        jmp     ._end_parse_RS


.rest_value_loop_RS:
        mov     di,ax
        mov     cx,bx
        jmp     .start_p_RS
        
        
        
.end_get_RS_ERROR_1:
;сообщение об ошибке - данный участок кода не был корректно обработан :(
        or      show_errors_sect,show_error_1
        jmp     ._end_parse_RS
.end_get_RS_ERROR_2:    
        or      show_errors_sect,show_error_2

._end_parse_RS:
if DEBUG
        pusha
        movzx   eax,bx
        mov     cx,0x0a
        mov     di,RamdiskSize_msg
        mov     dword[ds:di],'    '
        mov     word [ds:di+4],'  '
        call    decode
;Show size
        mov     si,RamdiskSize_msg
        call    printplain

        popa
end if  

}

macro use_free_memory
{       
local _support_function_use_free_memory
;макрос для получения общего числа доступной памяти в кб, для формирования рам диска за пределами 1 мб.
;используется 0х88 функция 0х15 прерывания
; если поддерживается функция, то в ax значение в кб, если нет, то в ax=0
        mov     ah,0x88  ;ah,0x88
        int     0x15
        jnc     ._support_function_use_free_memory
        xor     ax,ax
;возвращает в ax число в кб 
._support_function_use_free_memory:
        mov     free_ad_memory,ax  ; если не поддерживается биосом, то в ax=0
if DEBUG
        pushad
        movzx   eax,ax
        mov     cx,0x0a
        mov     di,free_memory_msg
        mov     dword[ds:di],'    '
        mov     word [ds:di+4],'  '
        call    decode
;Show size
        mov     si,free_memory_msg
        call    printplain

        popad
end if  




}
macro show_ERRORS
{

}

macro get_type_FS ;получить и создать образ для заданной RFS.
{
        mov     di,point_default   ;restore value
        mov     cx,save_cx_d
.start_g_tpe_RFS:
        call    get_firs_sym    ;get first symbol on new line
        test    cx,cx
        jz      ._end_parse_FRS ;._end_get_type_RFS     ;нету? ну ладно - следующее значение тогда ) 
        cmp     al,'R'
        jnz     .start_g_tpe_RFS
;проверка на значения RamdiskSize
;        parse_RamdiskSize
        mov     bx,cx
        mov     ax,di

        mov     si,parse_RamdiskFS
        mov     cx,parse_RamdiskFS_e - parse_RamdiskFS
        repe    cmpsb
        jnz     .start_g_tpe_RFS_rest_v    ;is not compare

        sub     bx,parse_RamdiskFS_e - parse_RamdiskFS ;correct cx
        add     bx,cx
        mov     cx,bx

        test    status_flag,flag_found_GTRFMS           ;оценка флагов
        jz      .correct_is_not_set_FRS          

;       mov     si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
;       call    printplain
;       jmp     .get_next_str

.correct_is_not_set_FRS:
        mov     ax,0x3d20          ;cut al=' ' ah='='
        repe    scasb 
        test    cx,cx
        jz      .end_get_FRS_ERROR_1          ;not found param 
        
        cmp     ah,byte [es:di-1]    ;find '='
        jnz     .start_g_tpe_RFS                        ; перейдем на начало и попробуем найти еще секцию 
        
        repe    scasb              ;cut ' '
        inc     cx
        dec     di
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Тут нужно преобразовывать строчку в цифровое значение.
;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov     bx,cx
        mov     ax,di

        mov     si,parse_RFS_FAT
        mov     cx,parse_RFS_FAT_e - parse_RFS_FAT
        repe    cmpsb
        jnz     .krfs_cmp          ;is not compare

make_FAT_RamFS  ;сделать 

if DEBUG
        pusha
        mov     si,make_fat12_RFS_msg
        call    printplain
        popa
end if  
        jmp     ._end_parse_FRS

.krfs_cmp:              
        mov     cx,bx
        mov     di,ax

        mov     si,parse_RFS_KRFS
        mov     cx,parse_RFS_KRFS_e - parse_RFS_KRFS
        repe    cmpsb
;       jnz     @f         ;is not compare

        jmp ._end_parse_FRS


.start_g_tpe_RFS_rest_v:
        mov     cx,bx
        mov     di,ax
        jmp     .start_g_tpe_RFS
        
        
        
.end_get_FRS_ERROR_1:
;сообщение об ошибке - данный участок кода не был корректно обработан :(
        or      show_errors_sect,show_error_1
        jmp     ._end_parse_FRS
.end_get_FRS_ERROR_2:   
        or      show_errors_sect,show_error_2

._end_parse_FRS:
if DEBUG
        pusha
        mov     si,get_type_FS_msg
        call    printplain
        popa
end if  



}
macro make_FAT_RamFS
{
local .RS1
local .fat12
local .fat16
; мы должны сформировать в начальный образ Ram FS, а потом записать его за область выше 1 мб..
;для случая с FAT12
;       mov     di,fat12_buffer ;ds должен быть = cs
;es:di - указывают на начало блока для формирования рам фс.
use_RamdiskSector       ;возращаемое значение в ax размер сектора в байтах
        cmp     ax,4096 ;по спецификации значение должно быть в пределах от 1 до 4096
        ja      .RS1
        test    ax,ax
        jnz     @f      ;ошибка если сюда прыгнули все таки ... 

.RS1:   mov     word [fat12_buffer.BPB_BytsPerSec],512
;;;;;;;;;;скажем что по дефолту будем юзать значение... 
@@:     mov     word [fat12_buffer.BPB_BytsPerSec],ax ;тут все ок

;BPB_SecPerClus кол-во секторов в кластере
use_RamdiskCluster      ;возращаемое значение в al
        cmp     al,128
        ja      @f
;       test    al,0x1  ;проверка на кратность ) 
;       jnz     @f

        mov     byte [fat12_buffer.BPB_SecPerClus],al

        ;incorrect value will be set dafault

;ниже некорректное значение в т.к. размер кратен 2 и в диапазоне от 1 до 128 включительно
; мы должны ругнуться на это 
;@@:    ;mov    byte [fat12_buffer.BPB_SecPerClus],1    

;;;;; определеим какая у нас будет использоваться FAT 
;по условию, fat12<4085<=fat16<65525<=fat32
; fat12_buffer.BPB_BytsPerSec*fat12_buffer.BPB_SecPerClus = кол-во секторов
        movzx   eax,word [fat12_buffer.BPB_BytsPerSec]
        movzx   ebx,byte [fat12_buffer.BPB_SecPerClus]

        imul    ebx,eax ;тут размерность сектора 
        mov     eax,save_ramdisksize  ;размер запрошенного рам диска в байтах
        cdq
        idiv    ebx
;;;;;;;; сейчас частное в eax, а остаток в edx
;получим кол-во секторов, и можем уже определить тип FAT которую нужно делать.
        cmp     eax,4085
        jb      .fat12
        cmp     eax,65525
        jb      .fat16
;;;;;;;;;;;;;;;;;;;;;;;; тут fat32
        mov     set_ramfs,32    ;установим тип файловой системы
        mov     word [fat12_buffer.BPB_RsvdSecCnt],32
        xor     eax,eax
        mov     word [fat12_buffer.BPB_RootEntCnt],ax
        mov     word [fat12_buffer.BPB_TotSec16],ax
        mov     dword [fat12_buffer.BPB_TotSec32],eax


.fat16: ;fat16
;Для FAT12 и FAT16 дисков это поле содержит количество секторов, а BPB_TotSec32 равно 0, если значение <умещается> (меньше 0x10000).
        jmp     $
        mov     set_ramfs,16    ;установим тип файловой системы
        movzx   ebx,byte [fat12_buffer.BPB_SecPerClus]
        imul    eax,ebx    

        cmp     eax,0x10000
        jae     @f
        mov     word [fat12_buffer.BPB_TotSec16],ax
        mov     dword [fat12_buffer.BPB_TotSec32],0
@@:
;количество секторов занимаемое одной копией фат
;       mov     word [fat12_buffer.BPB_FATSz16],0x9     ;Для FAT12/FAT16 это количество секторов одной FAT. ??
;;;; заполним BPB_RootEntCnt Для FAT12 и FAT16 дисков, это поле содержит число
;32-байтных элементов корневой директории. Для FAT32 дисков, это поле должно
;быть 0. Пока константа, нужно будет позже доделать. 
        mov     eax,root_dir_entry_count
        mov     word [fat12_buffer.BPB_RootEntCnt],ax   ; count of 32-byte dir. entries (224*32 = 14 sectors= 7 kb)
;по документации рекомендуют отрезать 16 кб для рут дир но это оч много, даже для коос. имхо для начала хватит и 7 кб
;;;;;;;
;Для FAT16 это количество секторов одной FAT. Для FAT32 это значение
;равно 0, а количество секторов одной FAT содержится в BPB_FATSz32.
;RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec - 1)) / BPB_BytsPerSec;

;TmpVal1 = DskSize - (BPB_ResvdSecCnt + RootDirSectors);
;TmpVal2 = (256 * BPB_SecPerClus) + BPB_NumFATs;
;If(FATType == FAT32)
;    TmpVal2 = TmpVal2 / 2;
;FATSz = (TMPVal1 + (TmpVal2 - 1)) / TmpVal2;
;If(FATType == FAT32) {
;    BPB_FATSz16 = 0;
;    BPB_FATSz32 = FATSz;
;} else {
;    BPB_FATSz16 = LOWORD(FATSz);
;    /* there is no BPB_FATSz32 in a FAT16 BPB */
;}
;=====================================
;RootDirSectors
        movzx   ebx, word [fat12_buffer.BPB_BytsPerSec]
        imul    eax,32
        add     eax,ebx
        dec     eax

        cdq
        idiv    ebx
;;;;;;;; сейчас частное в eax, а остаток в edx для дискеты 1.44 у нас должно быть значение =14
;BPB_ResvdSecCnt + RootDirSectors
        movzx   ebx, word [fat12_buffer.BPB_RsvdSecCnt]
        add     ebx,eax 
         
;DskSize у нас это значение уже получено и доступно 
        movzx   eax,word [fat12_buffer.BPB_TotSec16]    ;должен быть в секторах
        sub     eax,ebx


;TmpVal1=eax
        shl     edi,8   ;=edi*256
        movzx   ecx,byte [fat12_buffer.BPB_NumFATs]
        add     edi,ecx
;TmpVal2=edi
        add     eax,edi
        dec     eax
        cdq
        idiv    edi
;FATSz = сейчас частное в eax, а остаток в edx
        mov     word [fat12_buffer.BPB_FATSz16],ax








.fat12: ;fat12
if DEBUG
; выведем в отладке, что собираемся делать образ диска c FS=fat12
	pushad
	mov	si,start_making_FAT12_msg
	call	printplain
	popad
end if



;Для FAT12 и FAT16 дисков это поле содержит количество секторов, а BPB_TotSec32 равно 0, если значение <умещается> (меньше 0x10000).
        mov     set_ramfs,12    ;установим тип файловой системы
        movzx   ebx,byte [fat12_buffer.BPB_SecPerClus]
        imul    eax,ebx    

        cmp     eax,0x10000
        jae     @f
        mov     word [fat12_buffer.BPB_TotSec16],ax
        mov     dword [fat12_buffer.BPB_TotSec32],0
@@:
;количество секторов занимаемое одной копией фат
;       mov     word [fat12_buffer.BPB_FATSz16],0x9     ;Для FAT12/FAT16 это количество секторов одной FAT. ??
;;;; заполним BPB_RootEntCnt Для FAT12 и FAT16 дисков, это поле содержит число
;32-байтных элементов корневой директории. Для FAT32 дисков, это поле должно
;быть 0. Пока константа, нужно будет позже доделать. 
        mov     eax,root_dir_entry_count
        mov     word [fat12_buffer.BPB_RootEntCnt],ax   ; count of 32-byte dir. entries (224*32 = 14 sectors= 7 kb)
;по документации рекомендуют отрезать 16 кб для рут дир но это оч много, даже для коос. имхо для начала хватит и 7 кб
;;;;;;;
;DskSize(в секторах)*12 (размерность файловой системы, т.е предположим сколько битов потребуется для адресации этого объема) /8 (что получить размер в байтах)
;полученное число округляем в большую сторону кратное сектору т.е. 512 байт Такой подход не универсален, но пока пойдет
;вообще у мелкософт это все считается ручками, но мы будем юзать только под коос рам диск с фат12
        movzx   eax, word [fat12_buffer.BPB_TotSec16]
        imul    eax,12
        shr     eax,3   ;делим на 8 но т.е. нам нужно делить еще и на 512 или более в зависимости от размеров кластера
        movzx   ebx,word  [fat12_buffer.BPB_BytsPerSec] ;размер сектора
        cdq
        idiv    ebx     ;разделим на размер кластера
;сейчас у нас в eax значение его нужно округлить в большую сторону кратному 512 байтам
;применим следующее очистим and и добавим 512 байт. таким образом выравним на 512 байт
;но т.к. все равно делить нижний код нам не нужен
;       and     eax,0xfff200
;       add     eax,0x200       ;добавим 512 байт        для 1.44 дискеты идеально подходит ))  

        inc     ax
;по идее должно на каждую фат таблицу
;резервироваться 9 секторов т.е. получается 2*9=18+1 =19 секторов т.е. рут дир находиться на с 20 сетора т.е. с адреса 0х2600
;сейчас нужно вычислить сколько будет секторов занимать фат ) нужно разделить на 512
;FATSz = сейчас частное в eax
        mov     word [fat12_buffer.BPB_FATSz16],ax
;;;;;;;;;;;;;;;;;;;;;;;;;;;;
get_firstDataSector ;получить смещение до данных 
;создадим певую запись в фат по определенному адресу.
first_create_fat_table
;закиним BPB файловой системы за 1 мб.
use_BPB_RAM
;
;копирование файла.
use_RamdiskFile

;;;; вычисляем указатель на корневую дир FirstRootDirSecNum = BPB_ResvdSecCnt + (BPB_NumFATs * BPB_FATSz16);
;       movzx   ebx, [fat12_buffer.BPB_NumFATs]
;       movzx   eax,ax
;       imul    eax,ebx
;eax=(BPB_NumFATs * BPB_FATSz16)
;       inc     eax
; BPB_ResvdSecCnt значение только 1 для fat12/16
;в eax указатель на root dir. для дискеты fat12 должно получиться при кол-во копий fat 1  = 1+ (1*1) =2 или 3

if DEBUG        
        pusha
;       mov     ax,point_default
;        mov     ax,cx
        mov     cx,0x0a
        mov     di,show_db1
;        mov     dword[ds:di],'    '
;       mov     word [ds:di+4],'  '
        call    decode                                                                    
;Show size
        mov     si,show_db1
        call    printplain
;
;       xor     ax,ax
;       int     0x16
        popa
end if  







}

macro use_RamdiskSector
{
;для некоторых FS будет игнорироваться 
        mov     di,point_default   ;restore value
        mov     cx,save_cx_d

.start_RamdiskSector:
        call    get_firs_sym    ;get first symbol on new line
        test    cx,cx
        jz      .end_RamdiskSector      ;нету? ну ладно - следующее значение тогда ) 

        cmp     al,'R'
        jnz     .start_RamdiskSector
;проверка на значения RamdiskSize
;        parse_RamdiskSize

        mov     bx,cx
        mov     ax,di

        mov     si,parse_RamdiskSector
        mov     cx,parse_RamdiskSector_e - parse_RamdiskSector
        repe    cmpsb
        jnz     .RamdiskSector_rest_val    ;is not compare

        sub     bx,parse_RamdiskSector_e - parse_RamdiskSector ;correct cx
        add     bx,cx
        mov     cx,bx

        test    status_flag,flag_found_RamdiskSector            ;оценка флагов
        jz      .correct_is_not_set_RamdiskSector          

;       mov     si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
;       call    printplain
;       jmp     .get_next_str

.correct_is_not_set_RamdiskSector:
        mov     ax,0x3d20          ;cut al=' ' ah='='
        repe    scasb 
        jcxz    .end_get_RamS_ERROR_1          ;not found param 
        
        cmp     ah,byte [es:di-1]    ;find '='
        jnz     .start_RamdiskSector                    ; перейдем на начало и попробуем найти еще секцию 
        
        repe    scasb              ;cut ' '
        inc     cx
        dec     di
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
        xor     bx,bx
        mov     cx,4
@@:     movzx   ax,byte [es:di]
        cmp     al,'0'
        jb      .end_RamdiskSector
        cmp     al,'9'
        ja      .end_RamdiskSector
;;;;;;;;;;;;;;;;;;;

        imul    bx,10
        xor     al,0x30
        add     bx,ax

        inc     di

        loop    @b
        jmp     .end_RamdiskSector


.RamdiskSector_rest_val:
        mov     cx,bx
        mov     di,ax
        jmp     .start_RamdiskSector
.end_get_RamS_ERROR_1:

.end_RamdiskSector:
        mov     ax,bx

if DEBUG
        pusha
        movzx   eax,bx;save_cx_d;point_default
        mov     cx,0x0a
        mov     di,RamdiskSector_msg
        mov     dword[ds:di],'    '
        mov     dword [ds:di+4],'    '
        call    decode
;Show size
        mov     si,RamdiskSector_msg
        call    printplain

        popa
end if  

;       pop     di
;       pop     es
}

macro use_RamdiskCluster
{
;для некоторых FS будет игнорироваться 
;       push    es
;       push    di
        mov     di,point_default   ;restore value
        mov     cx,save_cx_d
;       push    ini_data_
;       pop     es
.start_RamdiskCluster:
        call    get_firs_sym    ;get first symbol on new line
        test    cx,cx
        jz      .end_RamdiskCluster     ;нету? ну ладно - следующее значение тогда ) 
        cmp     al,'R'
        jnz     .start_RamdiskCluster
;проверка на значения RamdiskSize
;        parse_RamdiskSize

        mov     bx,cx
        mov     ax,di

        mov     si,parse_RamdiskCluster
        mov     cx,parse_RamdiskCluster_e - parse_RamdiskCluster
        repe    cmpsb
        jnz     .RamdiskCluster_rest_val           ;is not compare

        sub     bx,parse_RamdiskCluster_e - parse_RamdiskCluster ;correct cx
        add     bx,cx
        mov     cx,bx

        test    status_flag,flag_found_RamdiskCluster           ;оценка флагов
        jz      .correct_is_not_set_RamdiskCluster          

;       mov     si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
;       call    printplain
;       jmp     .get_next_str

.correct_is_not_set_RamdiskCluster:
        mov     ax,0x3d20          ;cut al=' ' ah='='
        repe    scasb 
        jcxz    .end_get_RamSC_ERROR_1          ;not found param 
        
        cmp     ah,byte [es:di-1]    ;find '='
        jnz     .start_RamdiskCluster                   ; перейдем на начало и попробуем найти еще секцию 
        
        repe    scasb              ;cut ' '
        inc     cx
        dec     di
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
@@:     movzx   ax,byte [es:di]
        cmp     al,'0'
        jb      .end_RamdiskCluster
        cmp     al,'9'
        ja      .end_RamdiskCluster
;;;;;;;;;;;;;;;;;;;
        xor     al,0x30

        jmp     .end_RamdiskCluster


.RamdiskCluster_rest_val:
        mov     cx,bx
        mov     di,ax
        jmp     .start_RamdiskCluster
.end_get_RamSC_ERROR_1:

.end_RamdiskCluster:
if DEBUG
        pusha
        mov     cx,0x0a
        mov     di,RamdiskCluster_msg
;        mov     word[ds:di],'  '
        call    decode
;Show size
        mov     si,RamdiskCluster_msg
        call    printplain

        popa
end if  

}

macro use_Loader_Image
;предназначен для загрузки образов выше 1 Мб.
;первоначальная версия загружает образ дискеты 1.44 мб
{
local .start_p_LI
local .exit
local .error_LI
local .rest_value_loop
local .found_end_str
        mov     di,point_default   ;restore value
        mov     cx,save_cx_d
;обработка конструкции типа LoaderModule=kord/kolibri.ldm
.start_p_LI:
        call    get_firs_sym    ;get first symbol on new line
        test    cx,cx
        jz      .exit   ;нету? ну ладно - следующее значение тогда ) 
        cmp     al,'L'
        jnz     .start_p_LI
;проверка на значение LoaderModule
;        parse_LoaderModule
        mov     bx,cx
        mov     ax,di

        mov     si,parse_LoaderImage
        mov     cx,parse_LoaderImage_e - parse_LoaderImage
        repe    cmpsb
        jnz     .rest_value_loop           ;is not compare

        sub     bx,parse_LoaderImage_e - parse_LoaderImage ;correct cx
        add     bx,cx
        mov     cx,bx

;       test    status_flag,flag_found_LM               ;оценка флагов
;       jz      .correct_is_not_set_LI          

;       mov     si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
;       call    printplain
;       jmp     .get_next_str

;.correct_is_not_set_LI:
        mov     ax,0x3d20          ;cut al=' ' ah='='
        repe    scasb 
        jcxz    .rest_value_loop_LI          ;not found param timeout
        
        cmp     ah,byte [es:di-1]    ;find '='
        jnz     .rest_value_loop_LI
        
        repe    scasb              ;cut ' '
        inc     cx
        dec     di
;di указывает на начало блока информации, в cx длинна до конца секции.
;после загрузки заноситься значение занятой памяти.
;для того что бы загрузить модуль, воспользуемся callback сервисом
;оригинальное решение - разместим dd перед строчкой и после строчки разместим byte =0
;это выглядит так: в ini файле существует строчка LoaderModule = kord/kernel.loader
;мы ее модифицируем до такого состояния       dw,dw,db'kord/kernel.loader',0 конечно сохранив те значения которые мы заменяем
;сохранили певые 2 word
        push    dword [es:di-6]
        lea     si,[di-6]

        push    word  [es:di-2]
        xor     ax,ax
        mov     word [es:di-6],ax       ;вносим нужные значения
;info_real_mode_size размер и указатель на область в которую можно загрузиться
        mov     ax,info_real_mode_size  ;0x3000   ;следующий сегмент за данными


        mov     word [es:di-4],ax
        mov     word [es:di-2],16       ;кол-во блоков по 4 кб =64 кб т.е. больше не считаем 
;;;;;; поиск конца строчки
@@:     mov     al,byte [es:di]
        cmp     al,' '
        jz      .found_end_str
        cmp     al,0xa
        jz      .found_end_str
        cmp     al,0xd
        jz      .found_end_str
        inc     di
        dec     cx
        jnz     @b
;;;not found допустим,что это конец файла и он не имеет привычного заверешния строки
.found_end_str:
; чтение блока по 64 кб в сегмент и забрасывание его выше 1 мб.
        push    word [es:di]
        xor     ax,ax
        mov     word [es:di],ax
;        xor     ax,ax   ; function 1 - read file
        mov     di,si   ;file_data
        inc     ax
        push    si
        push    es
        call    far  dword [loader_callback]
        push    cs
        pop     ds

        pop     es
        pop     si

        test    bx,bx
        jnz     .error_LM


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; забрасывание блока в 64 кб выше 1 мб.
        mov     si,table_15_87
        push    es
        push    ds
        pop     es
        mov     cx, 256*18
        mov     ah, 0x87
        int     0x15
        pop     es
        pop     dx cx
        test    ah, ah



        jmp     far     dword [es:si]




.rest_value_loop:
        mov     di,ax
        mov     cx,bx
        jmp     .start_p_LI

.exit:



}



macro name_in_root_fat
;макрос, который записывает информацию о загруженном файле в корневую фат таблицу
{

}



macro use_RamdiskFile
{
;загрузка файлов с использование callback сервиса первичного загрузчика
;используется только для загрузки необходимых и небольших файлов, т.к. достаточно медленно работает
;для загрузки использует 0х87 функцию int 0x15 прерывания - загрузка блоков данных до 64 кб выше 1 мб
local   .start_loop
local   ._end
local   .rest_value_loop
local   .error
        mov     di,point_default   ;restore value
        mov     cx,save_cx_d
        mov     data_offset,0   ;clean offset
;обработка конструкции типа LoaderModule=kord/kolibri.ldm
.start_loop:
        call    get_firs_sym    ;get first symbol on new line
        test    cx,cx
        jz      ._end   ;нету? ну ладно - следующее значение тогда ) 
        cmp     al,'R'
        jnz     .start_loop
;проверка на значение RamdiskFile
        mov     bx,cx
        mov     ax,di

        mov     si,parse_RamdiskFile
        mov     cx,parse_RamdiskFile_e - parse_RamdiskFile
        repe    cmpsb
        jnz     .rest_value_loop           ;is not compare

        sub     bx,parse_RamdiskFile_e - parse_RamdiskFile ;correct cx
        add     bx,cx
        mov     cx,bx
;       test    status_flag,flag_found_LM               ;оценка флагов
;       jz      .correct_is_not_set_LM          

;       mov     si,found_equal_timeout                          ;мы нашли что флаг уже установлен, информируем
;       call    printplain
;       jmp     .get_next_str

;.correct_is_not_set_LM:
        mov     ax,0x3d20          ;cut al=' ' ah='='
        repe    scasb 
        test    ecx,ecx
        jz      .rest_value_loop   ;not found param timeout
        
        cmp     ah,byte [es:di-1]  ;find '='
        jnz     .rest_value_loop
        
        repe    scasb              ;cut ' '
        inc     cx
        dec     di

        mov     save_di_RAMDISK,di
        mov     save_cx_RAMDISK,cx
;di указывает на начало блока информации, в cx длинна до конца секции.
;после загрузки заноситься значение занятой памяти.
;для того что бы загрузить модуль, воспользуемся callback сервисом
;оригинальное решение - разместим dd перед строчкой и после строчки разместим byte =0
;это выглядит так: в ini файле существует строчка RamdiskFile = @menu,@menu
;мы ее модифицируем до такого состояния       dw,dw,db'@menu',0 конечно сохранив те значения которые мы заменяем
;сохранили певые 2 word

;
@@:     mov     al,byte [es:di]
        cmp     al,','          ; т.е. ищем разделитель 
        jz      .found_end_str
        inc     di
        dec     cx
        jnz     @b
;;;not found допустим,что это конец файла и он не имеет привычного завершения строки
.found_end_str:
;        mov    al,byte [es:di]
;       cmp     al,' '          ; убираем пробелы, если они есть
;       jnz     @f
;       inc     di
;       dec     cx
;       jnz     .found_end_str

;@@:
        mov     point_to_dest_file_name,di
        inc     di
;проверка индивидуальности имени файла
check_name_file
;/restore di - point and cx -size section
        mov     di,save_di_RAMDISK
        mov     cx,save_cx_RAMDISK

        test    al,al
        jnz     .start_loop     ;если в al значение не =0, то такое имя уже существует в системе.



        push    dword [es:di-6]
        lea     si,[di-6]

        push    word  [es:di-2]
        push    di
        xor     ax,ax
        mov     word [es:di-6],ax       ;вносим нужные значения
;info_real_mode_size размер и указатель на область в которую можно загрузиться
        mov     ax,info_real_mode_size  ;0x3000   ;следующий сегмент за данными


        mov     word [es:di-4],ax
        mov     word [es:di-2],16       ;кол-во блоков по 4 кб =64 кб т.е. больше не читаем 

        mov     di,point_to_dest_file_name

if DEBUG
        pushad
;	mov	ax,di
        mov     cx,0x0a
        mov     di,name_of_seg_get_64
        mov     dword[ds:di],'    '
        mov     word[ds:di+4],'  '
        call    decode
;Show size
        mov     si,name_of_seg_get_64
        call    printplain

        popad
end if  

        push    word [es:di]
        push    cx
        xor     ax,ax
        mov     word [es:di],ax
;        xor     ax,ax   ; function 1 - read file
        push    di
        mov     di,si   ;file_data
        inc     ax
        push    si
        push    es
        push    bp

        push    es
        pop     ds
        push    cs
        pop     es

        call    far  dword [es:loader_callback]


        push    cs
        pop     ds

        pop     bp
        pop     es
        pop     si

        cmp    bx,2
        ja     .error
; сейчас у нас в dx:ax размер файла, который мы загрузили.
; возможна ситуация, когда в bx=1 т.е. есть еще данные на диске 
        mov     status_flag_loader_f,bx

        shl     edx,16
        mov     dx,ax
;       shr     edx,10  ;размер файла в кб.
;;в edx размер в байтах.        
        mov     save_file_size,edx
        mov     eax,edx
;восстановим полностью файл сценария
        pop     di
        pop     cx      ;длинна остатка с 2-ой частью имени т.е. с именем назначением.
        pop     word [es:di]
        pop     di
        pop     word  [es:di-2]
        pop     dword [es:di-6]
        

if DEBUG
        pushad
        mov     cx,0x0a
        mov     di,RamdiskFile_msg
        mov     dword[ds:di],'    '
        call    decode
;Show size
        mov     si,RamdiskFile_msg
        call    printplain

        popad
end if  










; загрузим чему у нас равен кластер
;       mov     ax,word [fat12_buffer.BPB_BytsPerSec] ;кол-во байтов в секторе может быть любое 512 1024 2048 4096 2 байта
;       movzx   bx,byte [fat12_buffer.BPB_SecPerClus] ;кол-во секторов в кластере
;       imul    ax,bx
;сейчас в eax размер кластера (512) байт
;в edx длина файла в байтах до 64 кб    
;закиним файл за 1 мб
;1 нам нужно составить фат таблицу т.е. произвести разметку рамдиска, затем перенесем по адресу файл

;записать инфорамацию о файле в корневую директорию
register_file_in_fat
;перенести за 1 мб содержимое файла
move_file_up

;проверим, загружен ли до конца файл? т.е. если размер файла больше чем 64 кб, то будет подгружать оставшиеся блоки
        cmp     status_flag_loader_f,0x1
        jnz     @f
;нужно дозагузить данные файла и перенести их за 1-ый мб согласно фат структуре 









@@:
;тут организован цикл по загрузке файлов в корневую директорию
        mov     di,save_di_RAMDISK
        mov     cx,save_cx_RAMDISK
if DEBUG        
        pusha
        xor     ax,ax
        int     0x16
        popa
end if  


        jmp     .start_loop


.error:
        ;call   error.LoaderModule
;fixme!
.rest_value_loop:
        mov     di,ax
        mov     cx,bx
        jmp     .start_loop

._end:
;перенесем за 1-ый мб фат и рут дир
move_up_fat_and_root_d






;загрузка блока 
;       mov     ah,0x87
;       mov     cx,     ;size in byte
        

;es:si point to descripts


}

macro use_BPB_RAM ;закинуть самые первые 512 байт за 1-й мб
;данный макрос закидывает BPB структуру т.е. первые 512 байт, пока только фат12 за 1 мб
{
        mov     ax,fat12_buffer
        mov     si,table_15_87
        add     word [si+8*2+2],ax
        push    es
        push    ds
        pop     es
        mov     cx,256   ;бут сектор  укладывается в 512 байт 512/2=256
        mov     ah, 0x87
        int     0x15
        pop     es
;add 512 byte for destination adress
;       add     dword [si+8*3+2], 512
;        test    ah, ah
;        jz     
if DEBUG        
        pusha
        mov     ax,word [si+8*2+2]
        mov     cx,0x0a
        mov     di,BPB_msg
        call    decode                                                                    
;Show size
        mov     si,BPB_msg
        call    printplain
        popa
end if  
}
macro first_create_fat_table
;данный макрос создает оформляет 3 первых байта fat таблицы, и устанавливает указатель на следующий блок, и вносит 0 значение
;для смещения в корневой таблице.
{
        mov     al,byte [fat12_buffer.BPB_Media]


        push    ds


        mov     di,info_real_mode_size
        add     di,0x1000

if DEBUG
        pushad

        mov     ax,info_real_mode_size
        add     ax,0x1000
;	mov	ax,ds
	mov	cx,0xa
	       
        mov     di,first_entry_in_fat
        mov     dword [di],'    '
        mov     word  [di+4],'  '
        call    decode                                                                    
;Show size
        mov     si,first_entry_in_fat
        call    printplain

        xor     ax,ax
        int     0x16
      
        popad
end if


        push    di   ;  push    word info_real_mode_size+0x1000   ;cледующий сегмент за загруженным участком
        
        xor     di,di
        mov     point_to_free_root,di   ;значение смещения =0 в корневой фат таблице описания 

        pop     ds   ; загружен следующий сегмент т.е. пустой сегмент

        mov     byte [di],al
        or      ax,-1
        inc     di
        mov     word [di],ax

        pop     ds
        mov     point_next_fat_str,3

if DEBUG        
        pushad
        mov     ax,point_next_fat_str
        mov     cx,0x0a
        mov     di,fat_create_msg
        call    decode                                                                    
;Show size
        mov     si,fat_create_msg
        call    printplain
        popad
end if  

}
macro register_file_in_fat
;макрос регистрации файла в файловой структуре Fat
;пока поддерживается только фат12, пока )) 
;вычисление смежных кластеров и занесение инфы в fat/
{
local   .step2
local   .step3
local   .end
local   .eof_file

;di point on root dir на фри секцию.
        push    es

        mov     ax,info_real_mode_size
        add     ax,0x1000
        mov     es,ax  ;        push    word info_real_mode_size+0x1000   ;сегмент следующий за загруженным блоком в 64 кб

; определяем тип фат пока не определяем, пока только фат 12 
; 12 бит, для вычесления соседних каластеров. 
        mov     di,firstDataSect     ;в секторах
        sub     di,size_root_dir
;теперь в ax размер в секторах начала рут дир
        shl     di,9 ;imul 512
        add     di,point_to_free_root   ;смещение в уже записанных 32-х структурах.
;необходимо внести значение в рут дир т.е. 32 байта
if DEBUG        
        pushad
;       mov     ax,point_default
;        mov     ax,
        mov     cx,0x0a
        mov     di,show_db2
        mov     dword[ds:di],'    '
       mov     word [ds:di+4],'  '
        call    decode                                                                    
;Show size
        mov     si,show_db2
        call    printplain
;
;       xor     ax,ax
;       int     0x16
        popad
end if  



;gs:di - указатель для внесения инфорации в рут область фат таблицы инормации о файле.
        mov     si,shot_name_fat
        mov     cx,11
;запишем в структуру имя
@@:     lodsb
        stosb
        loop    @b

;запишем атрибуты файла и DIR_NTRes - зарезеврированный байт =0
        xor     ax,ax
        mov     ah,ATTR_VOLUME_ID
        mov     word [es:di],ax
        add     di,2
;DIR_CrtTimeTenth
        mov     byte [es:di],100
        inc     di
;DIR_CrtTime
        mov     word [es:di],0x032b    ;дата
        add     di,2
;DIR_CrtDate
        mov     word [es:di],0x0       ;время ><
        add     di,2
;DIR_LstAccDate
        mov     word [es:di],0x032b    ;дата моего
        add     di,2
;DIR_FstClusHI
        mov     word [es:di],0x0       ;время для фат12 /16 всегда 0
        add     di,2
;DIR_WrtTime
        mov     word [es:di],0x0       ;время ><
        add     di,2
;DIR_WrtDate
        mov     word [es:di],0x032b    
        add     di,2
        
        mov     ax,point_next_fat_str
        mov     word [es:di],ax 
        add     di,2

        push    di
;DIR_FstClusLO                  Младшее слово номера первого кластера.
 ;       mov     ax,point_next_fat_str   ;загрузим указатель на элемент фат таблицы т.е. это номер фат записи
;FATOffset = N + (N / 2) т.е. это уже у нас смещение мы знаем что -начинается все с 3-го элемента записи фат
        mov     bx,ax
        shr     bx,1
        add     ax,bx
;в ах сейчас FATOffset
;ThisFATEntOffset = BPB_ResvdSecCnt + (FATOffset / BPB_BytsPerSec);
        mov     bx, word [fat12_buffer.BPB_BytsPerSec]
        cwd
        idiv    bx
;ax=ThisFATEntOffset= rem (FATOffset / BPB_BytsPerSec) четный или нечетный указатель.
        mov     si,ax
;нам нужно в цикле записать все кластеры которые будут использованы для размещения файла.
;узнаем размер кластера. 
        movzx   eax,word [fat12_buffer.BPB_BytsPerSec] 
        movzx   ebx,byte [fat12_buffer.BPB_SecPerClus]
        imul    eax,ebx
;ax - размер кластера.
;сейчас будем записывать во временный буфер фат таблицу для выбранного файла. Поскольку мы его загрузили возможно не полностью
;мы обработаем запись для фат полностью, в не зависимости от предела буфера где возможна часть файла.   
        mov     ebx,save_file_size      ;размер файла в байтах
        
@@:     sub     ebx,eax
        cmp     ebx,eax
        jbe     .eof_file

        inc     point_next_fat_str
        mov     cx,point_next_fat_str   ;загрузим указатель на элемент фат таблицы т.е. это номер фат записи
;FATOffset = N + (N / 2) т.е. это уже у нас смещение мы знаем что -начинается все с 3-го элемента записи фат
        mov     dx,ax
        shr     dx,1
        add     cx,dx



        test    si,0x1
        jz      .step2
        shl     cx,4
        mov     word[es:si],cx
        inc     si
        add     cx,ax
        jmp     @b

.step2: and     cx,0x0FFF
        mov     word[es:si],cx
        inc     si
        add     cx,ax
        jmp     @b

.eof_file:
        mov     cx,0x0fff
        test    si,0x1
        jz      .step3
        shl     cx,4
        mov     word[es:si],cx
        jmp     .end

.step3: and     cx,0x0FFF
        mov     word[es:si],cx

.end:   inc     point_next_fat_str

        pop     di
;DIR_FileSize 32-битный DWORD содержит размер файла в байтах.
        mov     eax,save_file_size
        mov     dword [es:di],eax

if DEBUG        
        pushad

        mov     di,firstDataSect     ;в секторах
        sub     di,size_root_dir
;теперь в ax размер в секторах начала рут дир
        shl     di,9 ;imul 512
        add     di,point_to_free_root   ;смещение в уже записанных 32-х структурах.

	push	di

        mov     si,dest_name_fat
        mov     cx,11

;запишем в структуру имя
@@:     mov     al,byte [es:di]
        inc     di
        mov     byte [ds:si],al
        inc     si
        loop    @b
        
        mov	di,si
	inc 	di
	pop	ax
	mov	cx,0xa
	call	decode

        mov     si,dest_name_fat
        call    printplain
        popad

END IF





        add     point_to_free_root,32   ;увелицим смещение до следующего значения.
        pop     es

}





macro get_firstDataSector
;макрос для вычисления певого сектора данных т.е. данных файлов в фате
;вычислим FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors;
{
        mov     ax,word [fat12_buffer.BPB_FATSz16]
        movzx   bx,byte [fat12_buffer.BPB_NumFATs]
        imul    ax,bx   ;9x1=9
;ax=BPB_NumFATs * FATSz
        mov     bx,word [fat12_buffer.BPB_RootEntCnt]   ; count of 32-byte dir. entries (224*32 = 14 sectors= 7 kb)
        shr     bx,4    ;imul bx,32 and then div 512 -> in bx size in sectors
        add     ax,bx   ;9+14=23
        mov     size_root_dir,bx
        movzx   bx,byte [fat12_buffer.BPB_RsvdSecCnt]   ;add 1 for fat 16/12
        add     ax,bx
;ax=firstDataSector - где начинается первый секторо от 0 сектора в секторах. - фактически = 24 сектор
        mov     firstDataSect,ax        ;сохраним для вычисления
;       получимзначение кластеров, это объем в который мы можем записать данные
        mov     bx,word [fat12_buffer.BPB_TotSec16]
        sub     bx,ax
        mov     ax,bx
        movzx   bx,byte [fat12_buffer.BPB_SecPerClus]
        cwd
        idiv    bx
        mov     DataClasters,ax

if DEBUG        
        pushad
        mov     ax,firstDataSect        ;первый сектор данных
        mov     cx,0x0a
        mov     di,firstDataSect_msg
        call    decode                                                                    
;Show size
        mov     si,firstDataSect_msg
        call    printplain
;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov     ax,size_root_dir        ;размер рут дир в сетокторах
        mov     cx,0x0a
        mov     di,size_root_dir_msg
        call    decode                                                                    
;Show size
        mov     si,size_root_dir_msg
        call    printplain
;;;;;;;;;;;;;;;;;;;;;;;;;;
        mov     ax,DataClasters ;кластеры
        mov     cx,0x0a
        mov     di,DataClasters_msg
        call    decode                                                                    
;Show size
        mov     si,DataClasters_msg
        call    printplain
        popad

end if  

}

macro use_RamdiskPATHS
;парсинг пути источника файлов.
{

}

macro use_RamdiskPATHD
;парсинг пути назначения файлов.
{

}
macro check_name_file
;макрос проверки имени на повтор, имя должно быть уникальным.
;входные данные: es- сегмент где лежит файл для парсинга т.е. startos.ini
;di - указатель на имя файла т.е. es:di указывает на имя файла назначения
;выходные данные eax =-1 имя совпало, eax=0 имя не совпало.
{
local   .no_equal
local   .exit
local 	.loop_size_root_dir
;вычислим длинну строчки имени назначения, которую будем сравнивать с уже записанными данными.
;преобразуем в аналог фат записи сточку с именем назначения
        convertion_file_name	; преобразовали имя по нужным правилам
        test    ax,ax
        jnz     .exit

        mov     si,shot_name_fat

;вычислим указатель на корневую директорию
        mov     di,firstDataSect
        sub     di,size_root_dir
;теперь в ax размер в секторах начала рут дир
        shl     di,9 ;imul 512
;di= Это смещение от начала буфера до рут директории. в пределах 64 кб.
;загрузим значение - т.е. кол-во элементов, которые мы можем просматривать.
        mov     dx,root_dir_entry_count
        mov     si,point_to_dest_file_name
        
        mov     ax,info_real_mode_size
        add     ax,0x1000


        mov     gs,ax
.loop_size_root_dir:

if DEBUG        
;        pushad
;	push	di
;        mov	eax,dword[gs:di]
;        mov	si,(check_root_fat_+14)
;        mov     dword [ds:si],'____'
;	mov     dword [ds:si+4],'____'
;        mov	dword[ds:si],eax
;        mov	eax,dword[gs:di+4]
;        mov	dword[ds:si+4],eax
;
;
;	xor	eax,eax
;        mov     ax,gs;point_next_fat_str
;        mov     cx,0x0a
;        mov     di,check_root_fat_
;        mov     dword [di],'    '
;        mov     word  [di+4],'  '
;        call    decode
;	xor	eax,eax
;	pop	ax
;	mov	di,(check_root_fat_+7)
;        mov     dword [di],'    '
;        mov     word  [di+4],'  '
;        call    decode

;Show size
;        mov     si,check_root_fat_
;        call    printplain

;	xor	ax,ax
;	int	0x16
;        popad
end if  

	xor	bx,bx
        mov     cx,11   ;size of name in struct FAT

@@:     
        mov     al,byte [es:si+bx]
        mov     ah,byte [gs:di+bx]      ;gs:di - point to name in fat struct
        inc     bx

        cmp     ah,al
        jnz     .no_equal

;       dec     cx
;       jnz     @b
        loop    @b

;.succesfuly:
;печально, такое имя уже имеется :(
        or      ax,-1
        jmp     .exit


.no_equal:
        add     di,32           ;fat struct =32 byte
        dec     dx
        jnz     .loop_size_root_dir

;.exit_check_name:
        and     ax,0

.exit:

if DEBUG        
        pushad
;Show size
        mov     si,check_name_fat_msg_n
	test	ax,ax
	jz	@f
        mov     si,check_name_fat_msg_y
@@:     call    printplain
        popad
end if  

}


macro convertion_file_name
;макрос конвертации имени, это нужно поскольку формат представленный не соответсвует фат и напрямую редко можно когда использовать
;преобразование имени типа hello.asm в 'HELLO   ASM', в соответствии с правилами fat.
;входные параметры es:di указатель на имя файла которое нужно преобразовать, конечный буфер shot_name_fat
{
local   .next_step
local   .error
local   .st1
local   .st2
local   .st2_l
local   .st3
local   .st4_s
local   .st4
local   .st5

;вычислим длинну строчки имени назначения, которую будем сравнивать с уже записанными данными.
;       mov     di,point_to_dest_file_name   входной параметр
        mov     si,shot_name_fat
        or      first_input,-1  ;при первом входе устанавливаем флаг
        mov     cx,11   ;длинна имени в стуктуре фат таблицы

@@:     
        mov     al,byte [es:di]
        cmp     al,0xa
        jz      .st4_s
        cmp     al,0xd
        jz      .st4_s
        cmp     al,0x20
        jz      .st4_s

        cmp     al,0x20
        jb      .error
        cmp     al,0x22
        jz      .error
        cmp     al,0x2a
        jz      .error
        cmp     al,0x2b
        jz      .error
        cmp     al,0x2c
        jz      .error
        cmp     al,0x2F
        jz      .error

        cmp     al,0x3a
        jz      .error
        cmp     al,0x3b
        jz      .error
        cmp     al,0x3c
        jz      .error
        cmp     al,0x3d
        jz      .error
        cmp     al,0x3E
        jz      .error
        cmp     al,0x3F
        jz      .error

        cmp     al,0x5b
        jz      .error
        cmp     al,0x5c
        jz      .error
        cmp     al,0x5d
        jz      .error

        cmp     al,0x7c
        jz      .error

        
        cmp     first_input,-1
        jnz     .next_step
        and     first_input,0   ;сборосим флаг.
        cmp     al,'.'
        jz      .error  ;обработка точки, файл не может начинаться с точки

.next_step:
        cmp     al,0x2e
        jnz     .st2            ;обработка точки, в середине файла
;тут у нас установлен разделитель
;все остальнео место займут пробелы
        mov     al,' '

;!fixme обработаны не все исключения :(
        cmp     cl,3    ;формат файла такой GIDGIDIIASM т.е. gidgidii.asm
        jbe     .st2


.st3:   
        mov     byte [si],al
        inc     si
        dec     cx
        cmp     cx,3
        ja      .st3
;       inc     cx
        inc     di
        jmp     @b

.st2:
        cmp     al,0x60
        jbe     .st2_l
        
        xor     al,0x20 ;сделаем заглавные буквы
.st2_l: mov     byte [si],al
        inc     di
        inc     si
;        dec    cx
;       jnz     @b
        loop    @b
.st5:   xor     ax,ax
        jmp     @f

;;;;;;;;файл закончился, и нужно внести в конец пробелы
.st4_s: mov     al,' '
.st4:   mov     byte [si],al
        inc     si
        loop    .st4
        jmp     .st5

.error: or      ax,-1
@@:

if DEBUG        
        pushad
	
        mov     si,convertion_file_name_msg_y
	test	ax,ax
	jz	@f
        mov     si,convertion_file_name_msg_n
@@:     call    printplain

        mov     si,shot_name_fat
        mov     byte [si+12],0
        call    printplain
        popad

end if  
}

macro move_file_up
;макрос который перемещает за 1 мб с правилами фат данные файла.
{
local   .st1
local   .correct_on_byte
;сейчас имеет быть ситуация, когда BPB уже перемещен за 1 мб, фат, и рут дир будут позже перемещены,
;а нам нужно вычислить место, и перенести туда содержимое файла
;полученое значение указывает в байтах на начало данных

        mov     ax,info_real_mode_size  ; сегмент где расположены данные
        mov     si,table_15_87
        mov     word [si+8*2+2],ax
;смещение до данных уже за 1-м мб
        movzx   eax,firstDataSect
        movzx   edx,data_offset
        add     eax,edx

        movzx   ebx,word [fat12_buffer.BPB_BytsPerSec] 
        movzx   edx,byte [fat12_buffer.BPB_SecPerClus]
        imul    bx,dx   ;получим размер кластера



        push    ebx     ;save bx

        imul    eax,ebx
;       shl     eax,9   ;умножим на 512
        
if DEBUG
        pushad
	xor	eax,eax
        mov     ax,info_real_mode_size
        mov     cx,0x0a
        mov     di,seg_where_get_data
        mov     dword [di],'    '
        mov     word  [di+4],'  '
        call    decode                                                                    
;Show size
        mov     si,seg_where_get_data
        call    printplain
        popad

end if

;       mov     bx,word [fat12_buffer.BPB_BytsPerSec] 
;       movzx   dx,byte [fat12_buffer.BPB_SecPerClus]
;       imul    bx,dx
;       cwd
;       idiv    bx

        mov     dl,0x10

@@:     cmp     eax,0x00010000
        jb      @f

        sub     eax,0x00010000
        inc     dl
        jmp     @b


@@:     mov     byte [si+8*3+3],dl      ;куда писать
        mov     word [si+8*3+2],ax

        mov     ecx,save_file_size      ;размер файла в байтах.
        cmp     ecx,0x0000ffff          ;размер блока т.е. 64 кб
        jbe     .correct_on_byte        ;корректировка на байт значения



        mov     ecx,0x00010000          ;65536 
        sub     save_file_size,ecx      ;отнимим
;       jmp     .st1                    ;получим 0х8000




;корректировка значения должна быть выполенена на размер кластера
.correct_on_byte:
;/узнаем размер кластера
        pop     eax     ;restore size of claster
        push    ecx
@@:     inc     data_offset

        cmp     eax,ecx
        jae     @f
        sub     ecx,eax
        jmp     @b
@@:     pop     ecx




        test    ecx,0x1
        jz      .st1
        inc     ecx
.st1:   shr     ecx,1     ; преобразовать значение для 0x87 function

;перенесем блок за 1 мб        
        push    es
        push    ds
        pop     es

        mov     ah, 0x87
        int     0x15
        pop     es

if DEBUG
        pusha
;       mov     ax,point_next_fat_str
        mov     cx,0x0a
        mov     di,return_code_af_move
        call    decode                                                                    
;Show size
        mov     si,return_code_af_move
        call    printplain
        popa

end if

}


macro move_up_fat_and_root_d
;макрос, который позволяет перенести выше 1 мб в структуру образа фат таблицу и рут директорию
{
local  .st1

        mov     ax,info_real_mode_size
        add     ax,0x1000

        mov     si,table_15_87
        mov     word [si+8*2+2],ax
;смещение до данных
        mov     ax,512
        mov     word [si+8*3+2],ax
;fixme! тут необходимо сделать подержку т.е. формировать смещение файла в уже записанных данных.

        movzx   ecx,word [fat12_buffer.BPB_FATSz16]
        movzx   bx,byte [fat12_buffer.BPB_NumFATs]
        imul    cx,bx   ;9x1=9

        add     cx,size_root_dir        ;размер корневой дирректории
        shl     ecx,9   ;imul 512


;корректировка значения
        test    ecx,0x1
        jz      .st1
        inc     ecx
.st1:   shr     ecx,1

        push    es
        push    ds
        pop     es

        mov     ah, 0x87
        int     0x15
        pop     es

if DEBUG
        pusha
;       mov     ax,point_next_fat_str
        mov     cx,0x0a
        mov     di,return_code_af_fat_m
        call    decode                                                                    
;Show size
        mov     si,return_code_af_fat_m
        call    printplain
        popa

end if

}