; project name:   SYSTREE FILE COPIER
; version:        1.1b
; last update:    18/07/2004
; compiler:       FASM 1.52
; written by:     Ivan Poddubny
; e-mail:         ivan-yar@bk.ru
; copying-policy: GPL

; History:
; 18/07/2004 strings using "lsz" macro + french language (not 100%!)
; 04/06/2004 Bugfix for memory - thanks to Ville
; ...

    use32
    org     0x0

    db      'MENUET01'     ; 8 byte id
    dd      0x01           ; header version
    dd      START          ; start of code
    dd      I_END          ; size of image
    dd      0x20201        ; memory for app
    dd      0x10000        ; esp
    dd      0x0 , 0x0      ; I_Param , I_Icon

include 'lang.inc'
include 'macros.inc'       ; very useful stuff for MeOS
STRLEN = 48                ; maximal length of filename


START:                     ; start of execution

red:
    call draw_window       ; at first, draw the window

still:                     ; main cycle of application begins here

    mov  eax,10     ; wait here for event
    int  0x40

    dec  eax        ; redraw request ?
    jz   red
    dec  eax        ; key in buffer ?
    jz   key
    dec  eax        ; button in buffer ?
    jz   button

    jmp  still

  key:              ; key event handler
    mov  eax,2      ;   just read it and ignore
    int  0x40
    jmp  still      ;   return to main loop

  button:           ; button
    mov  eax,17     ;   get id
    int  0x40

    cmp  ah,1       ;   button id=1 ?  (close_btn)
    jz   close

    cmp  ah,2       ;   copy ?
    je   copy_file

; read_string:

    cmp  ah,5       ; user pressed dest button ?
    jz   dstbtn
    cmp  ah,4       ; user pressed src button ?
    jnz  still

  srcbtn:
    mov  [addr],dword source
    mov  [ya],dword 36
    jmp  rk
  dstbtn:
    mov  [addr],dword destination
    mov  [ya],dword 36+16

  rk:
    mov  edi,[addr]    ; load the address of the string
    mov  al,0          ; the symbol we will search for
    mov  ecx,STRLEN+1  ; length of the string (+1)
    cld                ; search forward
  repne  scasb         ; do search now
    inc  ecx           ; we've found a zero or ecx became 0
    mov  eax,STRLEN+1
    sub  eax,ecx       ; eax = address of <0> character
    mov  [temp],eax    ; position

    call print_text

    mov  edi,[addr]    ; address of string
    add  edi,[temp]    ; cursor position

  .waitev:
    mov  eax,10
    int  0x40
    cmp  eax,2
    jnz  still
;   mov  eax,2
    int  0x40
    shr  eax,8
    cmp  eax,8
    jnz  .nobs         ; BACKSPACE
    cmp  edi,[addr]
    jz   .waitev
    dec  edi
    mov  [edi],byte 0
    call print_text
    jmp  .waitev
  .nobs:
    cmp  eax,13        ; ENTER
    je   still
    cmp  eax,192
    jne  .noclear
    xor  al,al
    mov  edi,[addr]
    mov  ecx,STRLEN
    rep  stosb
    mov  edi,[addr]
    call print_text
    jmp  .waitev

  .noclear:
    mov  [edi],al

    call print_text

    inc  edi
    mov  esi,[addr]
    add  esi,STRLEN
    cmp  esi,edi
    jnz  .waitev

    jmp  still


  close:
    or   eax,-1        ; close program
    int  0x40


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
    mov  [source_info.blocks],1 ; load only 512 bytes
    mov  eax,58
    mov  ebx,source_info
    int  0x40

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
    push ebx         ; save file size
    mov  ecx,ebx
    add  ecx,0x20000 ; size of memory needed = 0x20000+filesize
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
    add  ebx,511
    shr  ebx,9       ; round up to 512 boundary
    mov  [source_info.blocks],ebx
    ; read the source file
    mov  eax,58
    mov  ebx,source_info
    int  0x40

    ; ebx = file size
    ; save loaded file
    mov  [dest_info.bytes2write],ebx ; file size in bytes
    mov  eax,58
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
    mov  ecx,0x20201
    int  0x40

    xor  eax,eax      ; eax = message number (0-OK)


; print message now
copy_error:
    mov  edi,eax
    mov  eax,4
    mov  ebx,20*65536+83
    mov  ecx,0x10ff0000
    mov  edx,[errors+edi*8]
    mov  esi,[errors+edi*8+4]
    int  0x40
jmp still


; print strings (source & destination)
print_text:
    mov  eax,13
    mov  ebx,107*65536+STRLEN*6
    mov  ecx,[ya]
    shl  ecx,16
    add  ecx,9
    mov  edx,0xf2f2f2
    int  0x40

    mov  eax,4
    mov  ebx,109*65536
    add  ebx,[ya]
    xor  ecx,ecx
    mov  edx,[addr]
    mov  esi,STRLEN
    int  0x40

    ret


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax, 12                   ; function 12:tell os about windowdraw
    mov  ebx, 1                    ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    xor  eax, eax                  ; function 0 : define and draw window
    mov  ebx, 160*65536+415        ; [x start] *65536 + [x size]
    mov  ecx, 160*65536+100        ; [y start] *65536 + [y size]
    mov  edx, 0x03DDDDDD           ; color of work area RRGGBB
    int  0x40

                                   ; WINDOW LABEL
    mov  eax, 4                    ; function 4 : write text to window
    mov  ebx, 8*65536+8            ; [x start] *65536 + [y start]
    mov  ecx, 0x10ffffff           ; color of text RRGGBB
    mov  edx, header               ; pointer to text beginning
    mov  esi, header.size          ; text length
    int  0x40

    mov  eax, 8                    ; COPY BUTTON
    mov  ebx, 20*65536+375
    mov  ecx, 63*65536+16
    mov  edx, 2
    mov  esi, 0xCCCCCC
    int  0x40

    mov  ebx, 105*65536+290
    mov  ecx, 33*65536+12
    mov  edx, 4
    mov  esi, 0xEBEBEB
    int  0x40

    mov  ebx, 105*65536+290
    mov  ecx, 49*65536+12
    mov  edx, 5
    mov  esi, 0xEBEBEB
    int  0x40

    mov  esi, source
    mov  edi, text+14
    mov  ecx, STRLEN
    rep  movsb

    mov  esi, destination
    mov  edi, text+STRLEN+59-45+14
    mov  ecx, STRLEN
    rep  movsb

    mov  ebx, 25*65536+36   ; print filenames
    xor  ecx, ecx
    mov  edx, text
    mov  esi, STRLEN+59-45
  newline:
    mov  eax, 4
    int  0x40
    add  ebx, 16
    add  edx, STRLEN+59-45
    cmp  [edx], byte 'x'
    jnz  newline

    mov  eax, 12                   ; function 12:tell os about windowdraw
    mov  ebx, 2                    ; 2, end of draw
    int  0x40

    ret


; DATA AREA
align 4
source_info:                 ; SOURCE FILEINFO
  .mode            dd 0      ; read file
  .start_block     dd 0x0    ; block to read
  .blocks          dd 0x700  ; num of blocks
  .address         dd 0x20000
  .workarea        dd 0x10000
  source           db '/HD/1/KERNEL/KERNEL.MNT',0
    times (STRLEN-23) db 0

dest_info:                   ; DESTINATION FILEINFO
  .mode            dd 1      ; write
  .notused         dd 0x0    ; not used
  .bytes2write     dd 0      ; bytes to write
  .address         dd 0x20000
  .workarea        dd 0x10000
  destination      db '/RD/1/KERNEL.MNT',0
    times (STRLEN-16) db 0

  align 4
  addr  dd  0x0
  ya    dd  0x0
  temp  dd  0


lsz  text,\
  ru, '   ОТКУДА:    |   Россия, Ярославль                           ',\
  ru, '    КУДА:     |        Поддубный Иван, ivan-yar@bk.ru         ',\
  ru, '                         КОПИРОВАТЬ                           ',\
  ru, 'x',\ ; <- END MARKER, DONT DELETE
\
  en, 'SOURCE:       |   Russia, Yaroslavl                           ',\
  en, 'DESTINATION:  |        Poddubny Ivan, ivan-yar@bk.ru          ',\
  en, '                   COPY SOURCE -> DESTINATION                 ',\
  en, 'x',\ ; <- END MARKER, DONT DELETE
\
  de, 'QUELLE:       |   Russia, Yaroslavl                           ',\
  de, 'ZIEL:         |        Poddubny Ivan, ivan-yar@bk.ru          ',\
  de, '                   KOPIERE QUELLE -> ZIEL                     ',\
  de, 'x',\ ; <- END MARKER, DONT DELETE
\
  fr, 'SOURCE:       |                                               ',\
  fr, 'DESTINATION:  |                                               ',\
  fr, '                           COPIER                             ',\
  fr, 'x'


lsz  header,\
  ru, 'КОПИРОВАТЬ ФАЙЛ',\
  en, 'SYSTREE FILE COPIER',\
  de, 'SYSTREE DATEIKOPIERER',\
  fr, 'COPIER LE FICHIER'


;  This macro is used to define a string table in format <pointer;length>
macro strtbl name,[string]
 {
  common
    label name dword
  forward
    local str,size
    dd str,size
  forward
    str db string
    size = $ - str
 }

if lang eq ru
strtbl errors,\
       "файл скопирован успешно",\
       "(чтение) не задана база жд",\
       "(чтение) файловая система не поддерживается",\
       "(чтение) неизвестная файловая система",\
       "(чтение) не задан раздел жд",\
       "недостаточно памяти",\
       "(чтение) конец файла",\
       "(чтение) неизвестная ошибка",\
       "(запись) не задан раздел жд",\
       "(запись) файловая система не поддерживается",\
       "(запись) неизвестная файловая система",\
       "(запись) не задан раздел жд",\
       "?",\
       "(запись) файл не найден",\
       "(запись) неизвестная ошибка"
else if lang eq en
strtbl errors,\
       "Success!",\
       "(read) no hd base or partition defined",\
       "(read) unsupported file system",\
       "(read) unknown file system",\
       "(read) hd partition not defined",\
       "out of memory",\
       "(read) end of file",\
       "(read) unknown error",\
       "(write) no hd base or partition defined",\
       "(write) unsupported file system",\
       "(write) unknown file system",\
       "(write) hd partition not defined",\
       "?",\
       "(write) end of file",\
       "(write) unknown error"
else
strtbl errors,\
       "Erfolgreich!",\
       "(lesen) Keine Festplatte oder Partition definiert",\
       "(lesen) Dateisystem nicht unterstuetzt",\
       "(lesen) Unbekanntes Dateisystem",\
       "(lesen) Keine Partition definiert",\
       "Zu wenig Speicher",\
       "(lesen) Dateiende erreicht",\
       "(lesen) Unbekanner Fehler",\
       "(schreiben) Keine Festplatte oder Partition definiert",\
       "(schreiben) Dateisystem nicht unterstuetzt",\
       "(schreiben) Unbekanntes Dateisystem",\
       "(schreiben) Keine Partition definiert",\
       "?",\
       "(schreiben) Dateiende erreicht",\
       "(schreiben) Unbekanner Fehler"
end if

I_END:
