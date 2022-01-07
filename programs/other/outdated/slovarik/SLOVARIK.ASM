;
;     Slovarik 0.3.1 (10 August 2005)
;                    DoomEd Archangel
;

;*********HEADER************
	use32
	org 0x0
	db 'MENUET01'
	dd 0x01
	dd lblSTART
	dd lblIMG_END
	dd lblMEMORY
	dd lblSTACK
	dd 0x0,0x0
;****************************
include 'ascl.inc'
lblSTART:

	call load_ini  ;Load ini file (path to last opened dictionary
	call load_dict ;Load dictionary

;----if no errors then continue

win_redraw:
call draw_window

still:
	mov eax,10
	int 0x40
	cmp eax,1
	je win_redraw
	cmp eax,2
	je key_pressed
	cmp eax,3
	je button_pressed
jmp still


key_pressed:
	;mov eax,2
	int 0x40
	cmp ah,27
	je clear_all_2
	cmp ah,13
	je translate_now

	cmp [inputboxpressed],1
	jne not_print

	cmp ah,8
	jne not_backspace
	mov bl,[inputboxlen]
	cmp bl,1
	jb not_print
	dec [inputboxcurchar]
	dec [inputboxlen]
	xor ebx,ebx
	mov bl,[inputboxcurchar]
	mov byte [inputbox+ebx],0
	sub [xcursor],6
	call draw_inputbox
	call draw_cursor
	jmp still

	not_backspace:
	cmp [inputboxlen],52
	je not_print
	xor ebx,ebx

	mov bl,[inputboxcurchar]
	mov [inputbox+ebx],ah

	inc [inputboxcurchar]
	inc [inputboxlen]
	call draw_inputbox

	add [xcursor],6
	call draw_cursor
	not_print:

jmp still

button_pressed:
	mov eax,17
	int 0x40
	cmp ah,1
	je lbl_close_now
	cmp ah,2
	je inputbox_pressed
	cmp ah,3
	je translate_now
	cmp ah,4
	je change_dict
jmp still


draw_window:
pusha
	mov eax,12
	mov ebx,1
	int 0x40

	mov eax,0
	mov ebx,30*65536+400
	mov ecx,30*65536+170
	mov edx,0x14cccccc
	mov edi,win_header
	int 0x40


	mov eax,4
	mov ebx,10*65536+41
	mov ecx,0x00000000
	mov edx,txtWord
	mov esi,txtWord_l-txtWord ; 6
	int 0x40

	;mov eax,4
	mov ebx,10*65536+67
	;mov ecx,0x00000000
	mov edx,txtVariants
	mov esi,txtVariants_l-txtVariants ;9
	int 0x40

	;mov eax,4
	mov ebx,10*65536+91
	;mov ecx,0x00000000
	mov edx,txtTranslate
	mov esi,txtTranslate_l-txtTranslate ;8
	int 0x40

	;mov eax,4
	mov ebx,10*65536+115
	;mov ecx,0x00000000
	mov edx,txtTranscript
	mov esi,txtTranscript_l-txtTranscript ;9
	int 0x40

	mov eax,8
	mov ebx,125*65536+150
	mov ecx,135*65536+20
	mov edx,3
	mov esi,0x00dddddd
	int 0x40
	
	mov eax,4
	mov ebx,168*65536+142
	mov ecx,0x00000000
	mov edx,txtTranslatenow
	mov esi,txtTranslatenow_l-txtTranslatenow ;9
	int 0x40




	mov eax,8
	mov ebx,10*65536+50
	mov ecx,135*65536+20
	mov edx,4
	mov esi,0x00dddddd
	int 0x40
	mov eax,4
	mov ebx,14*65536+142
	mov ecx,0x00000000
	mov edx,txtChangeDict
	mov esi,txtChangeDict_l-txtChangeDict  ;7
	int 0x40
;-------------------------------
	call draw_inputbox
	call print_variants
	call print_trans_word
	call print_transcript
;--------------------------------
	mov eax,12
	mov ebx,2
	int 0x40
popa
ret

draw_inputbox:
	mov eax,8
	mov ebx,68*65536+320
	mov ecx,38*65536+14
	mov edx,2
	int 0x40

	mov eax,13
	mov ebx,68*65536+320
	mov ecx,38*65536+14
	mov edx,0x00ffffff
	int 0x40
	call draw_inputbox_text
ret

draw_inputbox_text:
	mov eax,4
	mov ebx,70*65536+40
	mov ecx,0x00000000
	mov edx,inputbox
	mov esi,52
	int 0x40
ret

draw_cursor:
	mov eax,13
	mov bx,[xcursor]
	shl ebx,16
	mov bx,1
;        mov ebx,68*65536+1
	mov ecx,40*65536+10
	mov edx,0x000000ff
	int 0x40
ret
; END DRAW WINDOW

;-------------------PROGRAM-------------------

lbl_close_now:
	call save_ini
	mov eax,-1
	int 0x40

inputbox_pressed:
	mov [inputboxpressed],1
jmp still

print_variants:
	mov eax,13
	mov ebx,68*65536+320
	mov ecx,65*65536+14
	mov edx,0x00eeeeee
	int 0x40

	mov eax,4
	mov ebx,70*65536+67
	mov ecx,0x00000000
	mov edx,variants
	mov esi,52
	int 0x40
ret

translate_now:
	mov [inputboxpressed],0

	mov    edi,inputbox	;определяем длину введённой строки
	mov    ecx,52 ;50
	xor    eax,eax
	repne  scasb
	sub    edi,inputbox
	dec    edi
	mov    [search_len],edi
	inc    [search_len]   ;------------increase

	cmp    [search_len],1
	je     nothing_in_inputbox
	call count_dict_size

	cmp [dict_size],0
	je still

	call   clear_tr_word
	call   clear_variants
	call clear_transcript
jmp search

count_dict_size:
pusha
	mov    edi,dictionary	  ;определяем длину словаря
	mov    ecx,0x7FFFF
	xor    eax,eax
	repne  scasb
	sub    edi,dictionary
	dec    edi
	mov    [dict_size],edi
popa
ret

nothing_in_inputbox:
	call clear_tr_word
	call clear_variants
	call clear_transcript
	cld
	mov esi,errNothingEntered
	mov edi,tr_word
	mov ecx,14
	rep movsb
jmp win_redraw

clear_inputbox:
    cld
    mov edi,inputbox
    mov ecx,13 ;52/4
    xor eax,eax
    rep stosd
    mov [inputboxcurchar],0
    mov [xcursor],69
ret

clear_tr_word:
    cld
    mov edi,tr_word
    mov ecx,13 ;52/4
    xor eax,eax
    rep stosd
ret

clear_variants:
    cld
    mov edi,variants
    mov ecx,13 ;52/4
    xor eax,eax
    rep stosd
ret

clear_transcript:
    cld
    mov edi,trscript_word
    mov ecx,13 ;52/4
    xor eax,eax
    rep stosd
ret

clear_all_in_1:
    cld
    mov edi,inputbox ; 52*4 = 208 / 4 = 52
    mov ecx,52
    xor eax,eax
    rep stosd
    mov [inputboxpressed],1
    mov [inputboxcurchar],0
    mov [xcursor],69


ret

clear_all_2:
	call clear_all_in_1
	call draw_window
jmp still

print_trans_word:

	mov eax,13
	mov ebx,68*65536+320
	mov ecx,89*65536+14
	mov edx,0x00eeeeee
	int 0x40

	mov eax,4
	mov ebx,70*65536+91
	mov ecx,0x00000000
	mov edx,tr_word
	mov esi,52
	int 0x40
ret

trans_not_found:
    cld
    mov esi,tr_not_found_text
    mov edi,tr_word
    mov ecx,18 ;200
    rep movsb
    call clear_transcript
    call clear_variants
    call draw_window
ret

print_transcript:
	mov eax,13
	mov ebx,68*65536+320
	mov ecx,113*65536+14
	mov edx,0x00eeeeee
	int 0x40

	mov eax,4
	mov ebx,70*65536+115
	mov ecx,0x00000000
	mov edx,trscript_word
	mov esi,52
	int 0x40
ret
;-----------------------------------------------------
; ПОИСК ПО ФАЙЛУ СПРАВОЧНИКА
search:
    xor  esi,esi
    mov edi,dictionary	;НАЧАЛО В ЕДИ
    mov ecx,[dict_size] ;РАЗМЕР В ЕСХ

  news:
    push edi
  news2:

     ; 1) LOAD BYTES
     mov  al,[esi+inputboxplus] ; [esi+inputbox]
     mov  bl,[edi]

     ; 2) COMPARE BYTES
     cmp  al,bl
     je   yes_char

     ; 3) LETTER?
      cmp al,'A'
      jb notequal
      cmp al,'ё'
      ja notequal
      ;----------

      cmp al,'z' ;122 ;z=122
      jbe cpLAT
;cpCYR
;---------------------TEST
     cmp  al,'а' ;cyr A
     jb   .bigletter ;if letter<128
       ;--------
	  cmp al,'р'
	  jae .more_175  ;if letter>175
	  sub  al,32
     jmp  compare_bytes
	  .more_175:
	  sub al,80
	  jmp compare_bytes

    .bigletter:
     cmp  al,'Р'
     jae   .more_175_2
     add  al,32
     jmp compare_bytes
     .more_175_2:
     add al,80
     jmp compare_bytes
;---------------TEST

;cpLAT
      cpLAT:
     ; 4) A->a OR a->A
     cmp  al,'a'
     jb   @f
     add  al,-32
     jmp  compare_bytes
    @@:
     cmp  al,'Z'
     ja   notequal
     add  al,32
;-----
    compare_bytes:
     cmp  al,bl
     jne  notequal

  yes_char:
    inc  edi
    inc  esi
    cmp  esi,[search_len]
    jge  sfound
    jmp  news2

  notequal:
    pop  edi
    xor  esi,esi
    inc  edi
    loop news
    call trans_not_found
    jmp  still

    sfound:
    mov [total_smesh],edi ;смещение по файлу словаря

;-------------------
;-------search for transcription

    mov esi,[total_smesh]
    cld
  again0:
    lodsb
    cmp al,'|'
    jne again0

    mov [starting_transcrtiontion],esi

    cld
  again00:
    lodsb
    cmp al,'|' ;13
    jne again00
    mov [ending_transcrtiontion],esi
    mov eax,[starting_transcrtiontion]
    sub [ending_transcrtiontion],eax
    dec [ending_transcrtiontion]



	cld  ;ВЫВОД СЛОВА
	mov esi,[starting_transcrtiontion]
	mov edi,trscript_word
	mov ecx,[ending_transcrtiontion]
	rep movsb

;-------search for translation
    mov esi,[starting_transcrtiontion]
    add esi,[ending_transcrtiontion]
    cld
  again:
    lodsb
    cmp al,'|'
    jne again

    mov [starting_kav],esi

    cld
  again2:
    lodsb
    cmp al,10 ;13
    jne again2
    mov [ending_kav],esi
    mov eax,[starting_kav]
    sub [ending_kav],eax
    dec [ending_kav]

;-----------------------------------------------

    cld  ;ВЫВОД СЛОВА
    mov esi,[starting_kav]
    mov edi,tr_word
    mov ecx,[ending_kav]
    rep movsb

;--------
;search backward


	mov esi,[total_smesh]

	cld
	again3:
	lodsb
	cmp al,'|'
	jne again3
	mov [endingword],esi
	dec [endingword]
	again4:
	dec esi
	dec esi

	cld
	lodsb
	cmp al,10 ;13;' '
	jne again4
	mov [startingword],esi
	mov eax,[startingword]
	sub [endingword],eax


	cld  ;ВЫВОД СЛОВА
	mov esi,[startingword]
	mov edi,variants
	mov ecx,[endingword]
	rep movsb

;-----------------------------------------------





    call draw_window
jmp still


;-----------------------------------------------------

;---clear dictionary
clear_dict:
    mov edi,dictionary
    mov ecx,131072 ; 0x80000 / 4
    xor eax,eax
    rep stosd
ret

;---- change dictionary
change_dict:
	opendialog draw_window,open_ok,open_err,fileinfo.path
open_ok:
	call clear_dict
	call clear_all_in_1
	call load_dict
jmp still
open_err:
	call load_dict.call_error
jmp still


;---- load dictionary
load_dict:
;-open                from ASCL 9
       mov  dword [fileinfo+8],1
       mov  eax,58
       mov  ebx,fileinfo
       int  0x40
       mov  eax,[dictionary+2]
       shr  eax,9
       inc  eax
       mov  dword [fileinfo+8],eax
       mov  eax,58
       mov  ebx,fileinfo
       int  0x40
;--------
	cmp eax,0  ;0- open ok
	je .return ; win_redraw
	cmp eax,5  ;5- open ok
	je .return
	cmp eax,6  ;6- open ok???????????
	je .return
; if we can't open the dictionary
	.call_error:
	cld
	mov esi,errDictNotFound
	mov edi,tr_word
	mov ecx,errDictNotFound_l-errDictNotFound ;23
	rep movsb

	call draw_window

	mov eax,5   ;delay
	mov ebx,100 ;1 sec
	int 0x40

	.return:
	call check_header
ret




;--- copy path to dictionary
copy_path:
	call count_dict_size ;int [dict_size] will be size of path to dictionary
	mov esi,dictionary
	mov edi,fileinfo.path ;40
	mov ecx,[dict_size]
	cld
	rep movsb
ret


;----------load config file
load_ini:
       cld
       mov esi,fileinfo_ini.inipath
       mov edi,fileinfo.path
       mov ecx,19
       rep movsb

       mov  eax,58
       mov  ebx,fileinfo
       int  0x40

       call copy_path
       mov [dict_size],0
       call clear_dict
ret
;----------------------------

clear_fileinfo_path:
    mov edi,fileinfo.path
    mov ecx,256/4
    xor eax,eax
    rep stosd
ret

;-------save config file
save_ini:
       cmp dword [dictionary],0
       jne .save

       call clear_fileinfo_path

       cld
       mov esi,default_dict_path
       mov edi,fileinfo.path
       mov ecx,14
       rep movsb

       .save:
       mov    edi,fileinfo.path     ;определяем длину словаря
       mov    ecx,256
       xor    eax,eax
       repne  scasb
       sub    edi,fileinfo.path
       dec    edi
       mov    [fileinfo_ini.size],edi

       mov  eax,58
       mov  ebx,fileinfo_ini
       int  0x40

ret

;------ check dictionary header
check_header:

	mov eax,'DICT'
	cmp dword [dictionary],eax
	je .return
		call clear_tr_word
	cld
	mov esi,errDictHeader
	mov edi,tr_word
	mov ecx,errDictHeader_l-errDictHeader ;23
	rep movsb
	.return:
ret


;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;data;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;



win_header:
db 'Словарик', 0
win_header_l:

tr_not_found_text:
db 'ПЕРЕВОД НЕ НАЙДЕН!'
tr_not_found_text_l:

txtWord:
db 'Слово:'
txtWord_l:

txtTranslate:
db 'Перевод:'
txtTranslate_l:

txtVariants:
db 'Варианты:'
txtVariants_l:

errDictNotFound:
db 'Ошибка открытия файла или открытие отменено!'
errDictNotFound_l:

txtTranslatenow:
db 'Перевести'
txtTranslatenow_l:

errNothingEntered:
db 'Введите слово!'
errNothingEntered_l:

txtTranscript:
db 'Транскр.:'
txtTranscript_l:

txtChangeDict:
db 'Словарь'
txtChangeDict_l:

errDictHeader:
db 'Этот файл не является словарем!'
errDictHeader_l:

xcursor dw 69

default_dict_path db '/SYS/DICT.TXT'

fileinfo_ini:
.oper:
dd   1
dd   0x0
.size:
dd   256
dd   fileinfo.path
dd  os_work_mem
.inipath:
db '/sys/slovarik.ini',0

fileinfo:
dd   0	     ; 0 = ЧТЕНИЕ ФАЙЛА
dd   0x0       ; номер блока файла (512 байт, нумерация начинается с нуля)
dd   0x1       ; сколько блоков считывать
dd   dictionary  ; куда считывать
dd   os_work_mem ; 0x10000  ; память для работы ОС - 4096 байт
.path:
db '',0  ;'/SYS/DICT.TXT',0

lblIMG_END:
rb 257 ;times 257 db 0 ;for path


starting_kav dd 0
ending_kav dd 0
starting_transcrtiontion dd 0
ending_transcrtiontion dd 0

total_smesh dd 0

inputboxpressed db 0
search_len  dd 0
inputboxcurchar db 0
inputboxlen db 0

inputboxplus:
;db ' '
db 10 ;for search

inputbox:
rb 52 ;times 52 db 0

tr_word:
rb 52 ;times 52 db 0

trscript_word:
rb 52 ;times 52 db 0

variants:
rb 52  ;times 52 db 0

startingword dd 0
endingword dd 0


dict_size dd 0

dictionary:
rb 0x80000;~524 kb for dictionary
;0x100000 ;1 mb for future BIG dictionary :)

os_work_mem:
rb  0x10000

rb 4096
lblMEMORY:
lblSTACK:
