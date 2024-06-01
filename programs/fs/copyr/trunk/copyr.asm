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

;������ �ண� �� ���� � ���筠� �� 㦥 ��� ��� ࠡ�⠥�
    use32
    org     0x0

    db      'MENUET01'     ; 8 byte id
    dd      0x01           ; header version
    dd      START          ; start of code
    dd      I_END          ; size of image
    dd      0x10000        ; memory for app
    dd      0x10000        ; esp
    dd      param_area , 0x0      ; I_Param , I_Icon

include 'lang.inc'              ; Language support for locales: ru_RU (CP866), en_US.
include '..\..\..\macros.inc'   ; very useful stuff for MeOS
include 'ascl.inc'

START:                          ; start of execution

; ��ࠬ����:
; db n1 = ����� ��� � ���筨��
; times n1 db ? = ���� � ���筨��
; db n2 = ����� ��� � ��񬭨��
; times n2 db ? = ���� � ��񬭨��
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
    mcall
    jmp dexit

;closep:
;    or   eax,-1        ; close program
;    mcall


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
    mcall
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
    mcall

    ; ebx = number of read bytes = file size
    ; save loaded file
    mov  [dest_info.bytes],ebx ; file size in bytes
    mov  eax,70
    mov  ebx,dest_info
    mcall

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
    mcall

    xor  eax,eax      ; eax = message number (0-OK)
    jmp  copy_error


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax,eax                   ; function 0 : define and draw window
    mov  ebx,160*65536+415         ; [x start] *65536 + [x size]
    mov  ecx,160*65536+90          ; [y start] *65536 + [y size]
    mov  edx,0x14DDDDDD            ; color of work area RRGGBB
    mov  edi,labelt                ; WINDOW LABEL
    mcall


    mov  eax,8
    mov  ebx,105*65536+290
    mov  ecx,33*65536+12
    mov  edx,4
    mov  esi,0xEBEBEB
    mcall
    mov  ebx,105*65536+290
    mov  ecx,49*65536+12
    mov  edx,5
    mov  esi,0xEBEBEB
    mcall

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
    mov  eax,4
  newline:
    mcall
    add  ebx,16
    add  edx,62
    cmp  [edx],byte 'x'
    jnz  newline

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


; DATA AREA

  align 4
  addr  dd  0x0
  ya    dd  0x0
  temp  dd  0

if lang eq ru_RU
text:
      db '   ������:    |�����, ����⨭�, ��� ��᪢�  , 1 ����         '
      db '    ����:     |        �����設 �������, waptap@mail.ru       '
      db '                                                              '
      db 'x' ; <- END MARKER, DONT DELETE

labelt:
      db   '����������� �����',0

errors:
  db     "䠩� ᪮��஢�� �ᯥ譮                    "
  db     "(�⥭��) �� ������ ���� ��                 "
  db     "(�⥭��) 䠩����� ��⥬� �� �����ন������"
  db     "(�⥭��) �������⭠� 䠩����� ��⥬�      "
  db     "(�⥭��) �� ����� ࠧ��� ��                "
  db     "�������筮 �����                        "
  db     "(�⥭��) ����� 䠩��                       "
  db     "(�⥭��) �������⭠� �訡��                "
  db     "(������) �� ����� ࠧ��� ��                "
  db     "(������) 䠩����� ��⥬� �� �����ন������"
  db     "(������) �������⭠� 䠩����� ��⥬�      "
  db     "(������) �� ����� ࠧ��� ��                "
  db     "oh shit!                                   "
  db     "(������) 䠩� �� ������                    "
  db     "(������) �������⭠� �訡��                "
  db     "���� � ���筨�� � �ਥ����� �� 㪠����!!! "
  db     "���� � �ਥ����� �� 㪠���!!!              "

else ; Default to en_US
text:
      db 'SOURCE:       |                                               '
      db 'DESTINATION:  |                                               '
      db '                                                              '
      db 'x' ; <- END MARKER, DO NOT DELETE
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
