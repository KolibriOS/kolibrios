;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                           ;
; FILE COPY - system module for copy        ;
; files.Prog for SYS X-TREE BROWSER v22     ;
;                                           ;
; Create by Pavlushin Evgeni waptap@mail.ru ;
;              homepage www.deck4.narod.ru  ;
;                                           ;
; On base SYSTREE FILE COPIER 1.02          ;
;    Ivan Poddubny ivan-yar@bk.ru           ;
;                                           ; 
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Данная прога еще сырая и глючная но уже кое как работает
    use32
    org     0x0

    db      'MENUET01'     ; 8 byte id
    dd      0x01           ; header version
    dd      START          ; start of code
    dd      I_END          ; size of image
    dd      0x10000        ; memory for app
    dd      0x10000        ; esp
    dd      param_area , 0x0      ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'       ; very useful stuff for MeOS
include 'ascl.inc'

START:                     ; start of execution

; Параметры:
; db n1 = длина пути к источнику
; times n1 db ? = путь к источнику
; db n2 = длина пути к приёмнику
; times n2 db ? = путь к приёмнику
; db 0

;get param
        mov     eax, 15
        lea     esi, [param_area+1]
        movzx   ecx, byte [esi-1]
        jecxz   err_exit
        mov     edi, source
        rep     movsb
        mov     byte [edi], 0
        inc     eax
        movzx   ecx, byte [esi]
        inc     esi
        jecxz   err_exit
        mov     edi, destination
        rep     movsb
        mov     byte [edi], 0

    call draw_window
    call copy_file

dexit:
    wtevent red,key,button
    jmp dexit
red:
    call draw_window
    jmp dexit
key:
button:

exit:
    close

err_exit:
    push eax
    call draw_window
    pop  eax
;    jmp copy_error

; print message now
copy_error:
        imul    ebp, eax, 43

    mov  eax,4
    mov  ebx,20*65536+70
    mov  ecx,0x10ff0000
    lea  edx,[errors+ebp]
    mov  esi,43  ;[errors+edi*8+4]
    int  0x40
    jmp dexit

;closep:
;    or   eax,-1        ; close program
;    int  0x40


;====================================================
; copy_file
;   This piece of code copies src file to dst file,
;   then it pass the control to copy_error routine,
;   which returns to the main cycle of the app.
;   It's NOT a function! It's reached by direct jump
;   from the button handler.
;====================================================
copy_file:
    ; at first we must get the size of the source file
        mov     dword [source_attr+32], 0
        mov     eax, 70
        mov     ebx, source_attr_info
        int     0x40

    ; now eax contains error code
    ; and ebx contains file size in bytes
    test eax,eax      ; check if eax is equal to zero (success)
    je   .ok_getsize  ;   eax = 0 => continue
    cmp  eax,6
    jna  @f
    mov  eax,7        ; if error code is above 6, it will be 7
  @@:
    cmp  eax,5        ; file might be copied successfully altrough
                      ; the system reports an error 5
    jne  copy_error   ; print error code now
  .ok_getsize:

    ; allocate memory
    mov  ebx,[source_attr+32]
    push ebx         ; save file size
    lea  ecx,[ebx+0x10000] ; size of memory needed = 0x10000+filesize
    mov  eax,64      ; func 64
    mov  ebx,1       ; resize application memory
    int  0x40
    pop  ebx         ; restore filesize

    ; check if alloc function failed
    test eax,eax     ; non-zero value means error
    je   .ok_memory
    mov  eax,5       ; error 5 - out of memory
    jmp  copy_error  ; print error code now
  .ok_memory:

    ; save number of blocks to source_info
        mov     [source_info.bytes], ebx
    ; read the source file
    mov  eax,70
    mov  ebx,source_info
    int  0x40

    ; ebx = number of read bytes = file size
    ; save loaded file
    mov  [dest_info.bytes],ebx ; file size in bytes
    mov  eax,70
    mov  ebx,dest_info
    int  0x40

    ; check if 58 function failed
    test eax,eax
    je   .ok_write
    add  eax,7        ; error number += 7
    cmp  eax,6+7
    jna  copy_error
    mov  eax,7+7
    jmp  copy_error
  .ok_write:

    ; return to the initial amount of memory
    mov  eax,64
    mov  ebx,1
    mov  ecx,0x10000
    int  0x40

    xor  eax,eax      ; eax = message number (0-OK)
    jmp  copy_error


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,160*65536+415         ; [x start] *65536 + [x size]
    mov  ecx,160*65536+90         ; [y start] *65536 + [y size]
    mov  edx,0x03DDDDDD            ; color of work area RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x10ffffff            ; color of text RRGGBB
    mov  edx,labelt                ; pointer to text beginning
    mov  esi,labellen-labelt       ; text length
    int  0x40

    mov  eax,8
    mov  ebx,105*65536+290
    mov  ecx,33*65536+12
    mov  edx,4
    mov  esi,0xEBEBEB
    int  0x40
    mov  ebx,105*65536+290
    mov  ecx,49*65536+12
    mov  edx,5
    mov  esi,0xEBEBEB
    int  0x40

    mov  esi,source
    mov  edi,text+14
    mov  ecx,47
    rep  movsb

    mov  esi,destination
    mov  edi,text+62+14
    mov  ecx,47
    rep  movsb

    mov  ebx,25*65536+36   ; print filenames
    xor  ecx,ecx
    mov  edx,text
    mov  esi,62
  newline:
    mov  eax,4
    int  0x40
    add  ebx,16
    add  edx,62
    cmp  [edx],byte 'x'
    jnz  newline

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


; DATA AREA

  align 4
  addr  dd  0x0
  ya    dd  0x0
  temp  dd  0

if lang eq ru
text:
      db '   ОТКУДА:    |Россия, Селятино, МПК Москва  , 1 Курс         '
      db '    КУДА:     |        Павлюшин Евгений, waptap@mail.ru       '
      db '                                                              '
      db 'x' ; <- END MARKER, DONT DELETE
labelt:
      db   'КОПИРОВАНИЕ ФАЙЛА'
labellen:

errors:
  db     "файл скопирован успешно                    "
  db     "(чтение) не задана база жд                 "
  db     "(чтение) файловая система не поддерживается"
  db     "(чтение) неизвестная файловая система      "
  db     "(чтение) не задан раздел жд                "
  db     "недостаточно памяти                        "
  db     "(чтение) конец файла                       "
  db     "(чтение) неизвестная ошибка                "
  db     "(запись) не задан раздел жд                "
  db     "(запись) файловая система не поддерживается"
  db     "(запись) неизвестная файловая система      "
  db     "(запись) не задан раздел жд                "
  db     "oh shit!                                   "
  db     "(запись) файл не найден                    "
  db     "(запись) неизвестная ошибка                "
  db     "Путь к источнику и приемнику не указаны!!! "
  db     "Путь к приемнику не указан!!!              "
else
text:
      db 'SOURCE:       |                                               '
      db 'DESTINATION:  |                                               '
      db '                                                              '
      db 'x' ; <- END MARKER, DONT DELETE
labelt:
      db   'SYSTREE FILE COPIER'
labellen:

errors:
  db     "Success!                                   "
  db     "(read) no hd base or partition defined     "
  db     "(read) unsupported file system             "
  db     "(read) unknown file system                 "
  db     "(read) hd partition not defined            "
  db     "out of memory                              "
  db     "(read) end of file                         "
  db     "(read) unknown error                       "
  db     "(write) no hd base or partition defined    "
  db     "(write) unsupported file system            "
  db     "(write) unknown file system                "
  db     "(write) hd partition not defined           "
  db     "oh shit!                                   "
  db     "(write) file not found                     "
  db     "(write) unknown error                      "
  db     "Path to source is not given!!!             "
  db     "Path to destination is not given!!!        "
end if

         ;0123456789012345678901234567890123456789012

source_attr_info:
        dd      5
        dd      0
        dd      0
        dd      0
        dd      source_attr
        db      0
        dd      source

source_info:
        dd      0
        dd      0       ; start from 1st byte
        dd      0
.bytes  dd      ?
        dd      0x10000
        db      0
        dd      source

dest_info:                   ; DESTINATION FILEINFO
        dd      2
        dd      0
        dd      0
.bytes  dd      ?
        dd      0x10000

I_END:

destination:
        rb      256
source:
        rb      256
source_attr:
        rb      40

param_area      rb      256
