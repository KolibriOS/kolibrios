; project name:   SYSTREE FILE COPIER
; version:        1.2
; Mario79 23/10/06
;
; version:        1.1b
; last update:    18/07/2004
; compiler:       FASM 1.52
; written by:     Ivan Poddubny
; e-mail:         ivan-yar@bk.ru
; copying-policy: GPL

; History:
; 23/10/06 application use function 70
; 18/07/2004 strings using "lsz" macro + french language (not 100%!)
; 04/06/2004 Bugfix for memory - thanks to Ville
; ...

    use32
    org     0x0

    db      'MENUET01'     ; 8 byte id
    dd      0x01           ; header version
    dd      START          ; start of code
    dd      I_END          ; size of image
    dd      0x10000        ; memory for app
    dd      0x10000        ; esp
    dd      0x0 , 0x0      ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\macros.inc'       ; very useful stuff for MeOS
STRLEN = 48                ; maximal length of filename


START:                     ; start of execution

red:
    call draw_window       ; at first, draw the window

still:                     ; main cycle of application begins here

    mov  eax,10     ; wait here for event
    mcall

    dec  eax        ; redraw request ?
    jz   red
    dec  eax        ; key in buffer ?
    jz   key
    dec  eax        ; button in buffer ?
    jz   button

    jmp  still

  key:              ; key event handler
    mov  eax,2      ;   just read it and ignore
    mcall
    jmp  still      ;   return to main loop

  button:           ; button
    mov  eax,17     ;   get id
    mcall

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
    mov  [addr],dword source_info.name  ;source
    mov  [ya],dword 36
    jmp  rk
  dstbtn:
    mov  [addr],dword dest_info.name  ;destination
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
    mcall
    cmp  eax,2
    jnz  still
;   mov  eax,2
    mcall
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
    mcall


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
    mcall 70, get_param_info

    ; now eax contains error code
    test eax,eax      ; check if eax is equal to zero (success)
    jne  copy_error   ; print error code now

    ; allocate memory
    mov  ecx,[param_info+32]   ;ebx
    add  ecx,0x10000 ; size of memory needed = 0x10000+filesize
    mov  eax,64      ; func 64
    mov  ebx,1       ; resize application memory
    mcall

    ; check if alloc function failed
    test eax,eax     ; non-zero value means error
    je   .ok_memory
    mov  eax,5       ; error 5 - out of memory
    jmp  copy_error  ; print error code now
  .ok_memory:

    ; save size to source_info
    mov  ebx,[param_info+32]
    mov  [source_info.size],ebx    ; read the source file
    mcall 70,source_info

    ; now eax contains error code
    test eax,eax      ; check if eax is equal to zero (success)
    jne  copy_error   ; print error code now

    ; file size in bytes
    mov   [dest_info.size],ebx

    ; save loaded file
    mcall 70,dest_info

    ; now eax contains error code
    test eax,eax
    jne  copy_error

    ; return to the initial amount of memory
    mov  eax,64
    mov  ebx,1
    mov  ecx,0x10000
    mcall

    xor  eax,eax      ; eax = message number (0-OK)


; print message now
copy_error:
    mov  edi,eax
    mov  eax,4
    mov  ebx,20*65536+83
    mov  ecx,0x10ff0000
    mov  edx,[errors+edi*8]
    mov  esi,[errors+edi*8+4]
    mcall
jmp still


; print strings (source & destination)
print_text:
    mov  eax,13
    mov  ebx,107*65536+STRLEN*6
    mov  ecx,[ya]
    shl  ecx,16
    add  ecx,9
    mov  edx,0xf2f2f2
    mcall

    mov  eax,4
    mov  ebx,109*65536
    add  ebx,[ya]
    xor  ecx,ecx
    mov  edx,[addr]
    mov  esi,STRLEN
    mcall

    ret


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax, 12                   ; function 12:tell os about windowdraw
    mov  ebx, 1                    ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax, eax                  ; function 0 : define and draw window
    mov  ebx, 160*65536+415        ; [x start] *65536 + [x size]
    mov  ecx, 160*65536+100        ; [y start] *65536 + [y size]
    mov  edx, 0x13DDDDDD           ; color of work area RRGGBB
    mov  edi, title                ; WINDOW LABEL
    mcall

    mov  eax, 8                    ; COPY BUTTON
    mov  ebx, 20*65536+375
    mov  ecx, 63*65536+16
    mov  edx, 2
    mov  esi, 0xCCCCCC
    mcall

    mov  ebx, 105*65536+290
    mov  ecx, 33*65536+12
    mov  edx, 4
    mov  esi, 0xEBEBEB
    mcall

    mov  ebx, 105*65536+290
    mov  ecx, 49*65536+12
    mov  edx, 5
    mov  esi, 0xEBEBEB
    mcall

    mov  esi, source_info.name  ;source
    mov  edi, text+14
    mov  ecx, STRLEN
    rep  movsb

    mov  esi, dest_info.name  ;destination
    mov  edi, text+STRLEN+59-45+14
    mov  ecx, STRLEN
    rep  movsb

    mov  ebx, 25*65536+36   ; print filenames
    xor  ecx, ecx
    mov  edx, text
    mov  esi, STRLEN+59-45
  newline:
    mov  eax, 4
    mcall
    add  ebx, 16
    add  edx, STRLEN+59-45
    cmp  [edx], byte 'x'
    jnz  newline

    mov  eax, 12                   ; function 12:tell os about windowdraw
    mov  ebx, 2                    ; 2, end of draw
    mcall

    ret


; DATA AREA
get_param_info:
 .subfunction	 dd   5 	      ; 5 - get parameters of file
 .start        dd   0        ; rezerved
 .size_high    dd   0 	      ; rezerved
 .size         dd   0	      ; rezerved
 .return	      dd   param_info
 .name:
     db  0
     dd  source_info.name

source_info:                 ; SOURCE FILEINFO
 .subfunction	 dd   0 	      ; 0=READ
 .start        dd   0        
 .size_high    dd   0 	     
 .size         dd   0	      
 .return	      dd   0x10000
 .name:
     db   '/hd0/1/kernel/kernel.mnt',0   ; ASCIIZ dir & filename
     times (STRLEN-24) db 0

dest_info:                   ; DESTINATION FILEINFO
 .subfunction	 dd   2 	      ; 0=WRITE
 .start        dd   0        
 .size_high    dd   0 	     
 .size         dd   0	      
 .return	      dd   0x10000
 .name:
     db   '/sys/kernel.mnt',0   ; ASCIIZ dir & filename
    times (STRLEN-16) db 0

param_info:
     rb 40


;align 4
;source_info:                 ; SOURCE FILEINFO
;  .mode            dd 0      ; read file
;  .start_block     dd 0x0    ; block to read
;  .blocks          dd 0x700  ; num of blocks
;  .address         dd 0x20000
;  .workarea        dd 0x10000
;  source           db '/HD/1/KERNEL/KERNEL.MNT',0
;    times (STRLEN-23) db 0
;
;dest_info:                   ; DESTINATION FILEINFO
;  .mode            dd 1      ; write
;  .notused         dd 0x0    ; not used
;  .bytes2write     dd 0      ; bytes to write
;  .address         dd 0x20000
;  .workarea        dd 0x10000
;  destination      db '/RD/1/KERNEL.MNT',0
;    times (STRLEN-16) db 0

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


lsz  title,\
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
