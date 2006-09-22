;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;                                              ;
;   SYSTEM X-TREE BROWSER                      ;
;                                              ;
;   Author: Pavlushin Evgeni waptap@mail.ru    ;
;                   site: www.deck4.narod.ru   ;
;                                              ;
;   Compile with FASM for MenuetOS             ;
;                                              ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;Bug report +bug deleted  -bug not deleted
;show/fade del notwork+
;64Ver Run file from HD bug deleted.
;65Ver The bad scroll realization
;66Ver The good scroll realization, url line anti-flick
;67Ver Url line monolith procedure
;68Ver Mini icon on left of file name
;69Ver Getimg proc size minus 900 bytes
;70Ver Del data area ramsize minus 140000 bytes
;72Ver Quick sort, ramsize minus 200000 bytes
;73Ver Url flick and out bugs delete
;74Ver scroll bug deleted
;75Ver hd partition fast change button. Add bmp,txt,exec,asm and inc icons.
; sort type in headmenu bug del
;78Ver directory extension bug fix by Asko Vuori
;79Ver Asko Vuori volume label add and "put in command line" bug fix
;80Ver prompt edit fatal bug deleted, antiflick when move list up/down
;81Ver Save Dialog bug deleted
;82Ver Rewritten to function 70. Corrected work with scrollbar.
;83Ver CD-ROM support
;84Ver URL line editor corrected
;1560000 bytes memory!

;******************************************************************************
  use32
  org	   0x0
  db	 'MENUET01'   ; 8 byte id
  dd	 0x01		; header version
  dd	 START	      ; program start
  dd	 I_END	      ; program image size
  dd	 RAM_END      ; memory
  dd	 RAM_END      ; stack
  dd	 param_area ,0x0   ; param,icon
;  dd 0,0

;******************************************************************************
include 'macros.inc'
include 'ascl.inc'
include 'ascgl.inc'

    gif_hash_offset = gif_hash

START:		    ; start of execution
; //// Willow
;    mov eax,58
;    mov ebx,MRUfile
;    int 0x40
; //// Willow
    mcall 18,11,1,table_area
    cmp [edx+2],byte 0
    je	  no_hdpartition_on_hd0
    mov [hdimg1], aHD0
 no_hdpartition_on_hd0:
    cmp [edx+3],byte 0
    je	  no_hdpartition_on_hd1
    mov [hdimg2], aHD1
 no_hdpartition_on_hd1:
    cmp [edx+4],byte 0
    je	  no_hdpartition_on_hd2
    mov [hdimg3], aHD2
 no_hdpartition_on_hd2:
    cmp [edx+5],byte 0
    je	  no_hdpartition_on_hd3
    mov [hdimg4], aHD3
 no_hdpartition_on_hd3:
        test    byte [edx+1], 80h
        jz      @f
        mov     [hdimg1], aCD0
        mov     [hdimg1+4], 'CD-R'
        mov     [hdimg1+8], 'OM  '
@@:
        test    byte [edx+1], 20h
        jz      @f
        mov     [hdimg2], aCD1
        mov     [hdimg2+4], 'CD-R'
        mov     [hdimg2+8], 'OM  '
@@:
        test    byte [edx+1], 8
        jz      @f
        mov     [hdimg3], aCD2
        mov     [hdimg3+4], 'CD-R'
        mov     [hdimg3+8], 'OM  '
@@:
        test    byte [edx+1], 2
        jz      @f
        mov     [hdimg4], aCD3
        mov     [hdimg4+4], 'CD-R'
        mov     [hdimg4+8], 'OM  '
@@:

    mov eax,40
    mov ebx,0100111b
    int 0x40

    cmp byte [param_area],0 ;test parameters line
    jne no_brow     ;it's dialog
    mov [browser], 1   ;it's browser
    jmp no_dlg
no_brow:

    mov al,byte [param_area+5]
    mov [dlg_type],al

    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    int 0x40

    mov eax,dword [procinfo+30]
    mov edi,MYPID+4-1
    mov ecx,4
    mov ebx,10
    cld

new_d:
    xor edx,edx
    div ebx
    add dl,'0'
    mov [edi],dl
    dec edi
    loop new_d

    movzx eax,byte [param_area]
    sub eax,48
    imul eax,10
    movzx ebx,byte [param_area+1]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [param_area+2]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [param_area+3]
    add eax,ebx
    sub eax,48

    mov ecx,eax ;pid to ecx
    mov eax,60
    mov ebx,2
    mov edx,MYPID
    mov esi,4
    int 0x40
no_dlg:

    giftoimg but_file,tempimg

;************************
;      Get images
;************************

    mov eax,0  ;x
    mov ebx,0	    ;y
    mov esi,286     ;xs
    mov edi,16		;ys
    mov ecx,tempimg ;src
    mov edx,butimg   ;dest
    call getimgproc
    mov eax,288
    mov esi,60
    mov edx,logoimg  ;dest
    call getimgproc
    mov eax,0  ;x
    mov ebx,16	       ;y
    mov esi,51	       ;xs
    mov edi,esi      ;ys
    mov edx,logoinfimg	     ;dest
    call getimgproc
    mov eax,51	    ;x
    mov esi,8	    ;xs
    mov edi,9	     ;ys
    mov edx,upsb   ;dest
    call getimgproc
    mov eax,51+8  ;x
    mov edx,dnsb   ;dest
    call getimgproc

    mov eax,51+16  ;x
    mov ebx,16	       ;y
    mov esi,12	       ;xs
    mov edi,9	     ;ys

    mov ecx,tempimg ;src
    mov edx,hdico   ;dest
    mov ebp,10
loogetimg:
    call getimgproc
    add edx,9*12*3+8
    add eax,12
    dec ebp
    jnz loogetimg

    call read_directory
;    call convertation
red:
    call draw_window	    ; at first, draw the window

still:
    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    int 0x40

    wtevent red,key,button

scrolltest:
    mov eax,37
    mov ebx,2
    int 0x40
    dec eax
    jne still

scrl:
    mov eax,37
    mov ebx,1
    int 0x40
        movzx   ebx, ax         ; y
        shr     eax, 16         ; x
    mov ebp,eax
    sub ebp,[listx] ;[procinfo.x_size]
    sub ebp,[listxsize]
    add ebp,[scrollsize]
    cmp ebp,dword [scrollsize] ;8
    jae menu_test ; still

        lea     ebp, [ebx-scrollbutsize]
        sub     ebp, [listy]    ; up scroll
        jl      menu_test
        mov     ecx, [listysize]
        sub     ecx, 2*scrollbutsize
        mov     eax, [scroll_height]
        cmp     ebp, ecx
        jge     menu_test
        sub     ecx, eax
        shr     eax, 1
        sub     ebp, eax
        jge     @f
        xor     ebp, ebp
@@:
        cmp     ebp, ecx
        jl      @f
        mov     ebp, ecx
@@:
        xor     edx, edx
        mov     eax, [listysize]
        sub     eax, 2
        mov     ebx, 10
        div     ebx
        mov     ebx, eax
        cmp     ebx, [listsize]
        jae     notusescrl
        mov     eax, [listsize]
        sub     eax, ebx
        mul     ebp
        div     ecx
        cmp     eax, [filecursor]
        jz      still
        mov     [filecursor], eax
        jmp     usescrl
notusescrl:
        cmp     [filecursor], 0
        jz      still
    mov [filecursor],0 ;ebp
usescrl:

    mov [flick],1
    jmp anti_flick ;red

menu_test:
    cmp [pmenu_draw],1 ;if menu is show, update all
    jne still
    mov [pmenu_draw],0
    jmp red	     ;update all window

;this function not use in dialog when poup menu's is not used
;in dialog's

;===================
; Test keyboard
;===================
key:		  ; key
    mov  eax,2
    int  0x40
    cmp dword [focus],0
    jne con_edit
    cmp  ah,key_Up
    jne  no_upk
    mov  ebx,1
    jmp  up
no_upk:
    cmp  ah,key_Down
    jne  no_downk
    mov  ebx,1
    jmp  down
no_downk:
    cmp  ah,key_PgUp
    jne  no_pgup
    mov  ebx,10
    jmp  up
no_pgup:
    cmp  ah,key_PgDown
    jne  no_pgdown
    mov  ebx,10
    jmp  down
no_pgdown:
    cmp  ah,key_Enter
    je   kfad
    cmp  ah,key_Bspace
    je	     back
    cmp  ah,key_F2
    je	     viewset
    cmp  ah,key_F3
    je	     textopen
    cmp  ah,key_F5
    je	     copy_to_clip
    cmp  ah,key_F6
    je	     paste_from_clip
    cmp  ah,key_F11
    je	     edit_prompt
    cmp  ah,key_F12
    je	     update
    jmp  still

; test input string
con_edit:
    cmp  ah,key_Enter
    jne  no_con_ent
    not  [focus]
    jmp  savetest
;    jmp  update
no_con_ent:
    cmp  ah,key_Left
    jne  no_con_left
        cmp     [cursor], 0
        jz      still
    dec  [cursor]
    mov  [flick],2
    jmp  anti_flick ;red
no_con_left:
    cmp  ah,key_Right
    jne  no_con_right
        mov     eax, [cursor]
        cmp     byte [path+eax], 0
        jz      still
    inc  [cursor]
    mov  [flick],2
    jmp  anti_flick ;red
no_con_right:
    cmp  ah,key_Bspace
    jne  no_con_bspace

        mov     ebp, [cursor]
        test    ebp, ebp
        jz      still
lobsp:
    mov  bl,byte [path+ebp]
    mov  byte [path+ebp-1],bl
    inc  ebp
    cmp  ebp,1024
    jne  lobsp
    dec  [cursor]
    mov  [flick],2
    jmp  anti_flick ;red
no_con_bspace:

    mov  ecx,[cursor]
    mov  ebp,1022
        cmp     byte [path+ebp], 0
        jnz     still
losx:
        cmp     ebp, ecx
        jbe     @f
    mov  bl,byte [path+ebp]
    mov  byte [path+ebp+1],bl
    dec  ebp
    jmp  losx
@@:

    mov  byte [path+ebp],ah
    inc  dword [cursor]

    mov  [flick],2
    jmp  anti_flick


;----------------------------
;Test on mouse button
;-----------------------------

  button:	  ; button
    mov eax,17
    int 0x40

    cmp ah,2	       ;Edit prompt line?
    je	    edit_prompt

    cmp ah,4
    jne no_filelist

mousetest:
    mov eax,37
    mov ebx,1
    int 0x40
    mov ebx,eax
    shr eax,16		 ;x
    and ebx,0xffff   ;y

    sub ebx,[listy] ;80
    mov [mousey],ebx

    mov ecx,[listx]
    cmp eax,ecx
    jl	    still
    add ecx,[listxsize]
    cmp eax,ecx
    jg	    still

filexadd:

; Enter in directory
file_add:
    mov edx,0  ;for div correct work div
    mov eax,dword [mousey]
    mov ebx,10
    div ebx

    add eax,[filecursor]
    jmp no_kfad
kfad:
    mov eax,[filecursor]
no_kfad:
    mov ebx,27
    mul ebx
    lea ebp,[convinfo+eax]
    mov esi,[ebp]
    cmp esi,-1
    jz  still

    mov edi,paramtest ;clear param string
    mov ecx,1024/4
    xor eax,eax
    rep stosd

        mov     edi, path
        mov     ecx, 1024
        repnz   scasb
        dec     edi
        cmp     [ebp+15], dword 'FOL '
        jnz     openf
; open directory - append its name to path
        cmp     ecx, 2
        jb      still
        push    edi
        mov     al, '/'
        stosb
        dec     ecx
@@:
        lodsb
        stosb
        test    al, al
        jz      @f
        dec     ecx
        jnz     @b
; name is too long - do not do anything
        pop     edi
        xor     eax, eax
        mov     ecx, path+1024
        sub     ecx, edi
        rep     stosb
        jmp     still
@@:
        sub     edi, path+1
        mov     [cursor], edi
        pop     edi
; name appended, now read new directory
    mov [filecursor],0

    call read_directory
;;    call convertation
    call draw_window
    jmp still

;Savetest
savetest:
    cmp byte [dlg_type],'S'
    jne still
; always force open file - this is better
; because there exists files without extension and folders with extension
        jmp     openf
;    mov ecx,100
;savetestloop:
;    cmp [path+ecx],byte 0
;    je	    next_byte
;    cmp [path+ecx],byte 32
;    je	    next_byte
;    cmp [path+ecx],byte '.'
;    je	    openf  ;it's file
;;    cmp [path+ecx],byte '/'
;;    je  no_save  ;it's dir
;next_byte:
;    dec ecx
;    jnz savetestloop
;    jmp still

;Open/Run file

openf:
        push    esi edi
        mov     esi, path
        mov     edi, paramtest
        mov     ecx, 1024/4
        rep     movsd
        pop     edi esi
        add     edi, paramtest-path

    cmp dword [focus],0 ;if prompt line with focus no add file name from frame
    jne file_set

        cmp     edi, paramtest+1022
        jae     still
        mov     al, '/'
        stosb
@@:
        lodsb
        stosb
        test    al, al
        jz      file_set
        cmp     edi, paramtest+1024
        jb      @b
        jmp     still

file_set:

    cmp [browser],0
    jne is_brow

    movzx eax,byte [param_area]
    sub eax,48
    imul eax,10
    movzx ebx,byte [param_area+1]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [param_area+2]
    add eax,ebx
    sub eax,48
    imul eax,10
    movzx ebx,byte [param_area+3]
    add eax,ebx
    sub eax,48

    mov ecx,eax ;pid to ecx
    mov eax,60
    mov ebx,2
    mov edx,paramtest
    mov esi,1024
    int 0x40

    jmp exit

is_brow:

;    cmp [convinfo+edi+26],dword 'Fvol'
;    je  edit

; find extension
        mov     eax, paramtest+1023
@@:
        dec     eax
        cmp     byte [eax+1], 0
        jz      @b
@@:
        cmp     byte [eax], '/'
        jz      .noext
        cmp     byte [eax], '.'
        jz      .ext
        dec     eax
        cmp     eax, paramtest
        jae     @b
.noext:
; file has no extension
;        xor     ebx, ebx
        jmp     execute
.ext:
        inc     eax
; eax points to extension
        cmp     byte [eax], 0
        jz      .noext
        mov     ecx, dword [eax]
        and     ecx, not 0x20202020
        mov     ebx, jpgview
        cmp     ecx, 'JPG'
        jz      run
        cmp     ecx, 'JPEG'     ; note that it will select also *.JPEG*
        jz      run
        mov     ebx, gifview
        cmp     ecx, 'GIF'
        jz      run
        mov     ebx, ac97wav
        cmp     ecx, 'WAV'
        jz      run
        mov     ebx, midamp
        cmp     ecx, 'MID'
        jz      run
        mov     ebx, bmpview
        cmp     ecx, 'BMP'
        jz      run
; //// Willow
        mov     ebx, pngview
        cmp     ecx, 'PNG'
        jz      run
; //// Willow
        mov     ebx, rtfread
        cmp     ecx, 'RTF'
        jz      run
        mov     ebx, editor
        cmp     ecx, 'ASM'
        jz      run
        cmp     ecx, 'TXT'
        jz      run
        cmp     ecx, 'INC'
        jz      run
        jmp     still

execute:
        mov     ebx, fileinfo_start
        and     dword [ebx+8], 0                ; no parameters
        mov     dword [ebx+21], paramtest       ; program name
.do:
        mov     eax, 70
        int     0x40
        jmp     still

run:
        mov     [fileinfo_name], ebx            ; program name
        mov     ebx, fileinfo_start
        mov     dword [ebx+8], paramtest        ; parameters
        jmp     execute.do

no_filelist:

    cmp ah,5	;OPEN/SAVE button
    je	    kfad

    cmp ah,6	;Scroll up
    jne no_scrlup
    mov ebx,1
    jmp up
no_scrlup:

    cmp ah,7	;Scroll down
    jne no_scrldown
    mov ebx,1
    jmp down
no_scrldown:

    cmp ah,8
    jne no_update
update:
    call read_directory
;    call convertation
;    mov [filecursor],0
    call draw_window
no_update:

    cmp ah,9
    jne no_textopen
textopen:
        mov     eax, 27
        mul     [filecursor]
        cmp     dword [eax+convinfo+15], 'FOL '
        jz      still
        push    eax
        mov     esi, path
        mov     edi, paramtest
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        mov     al, '/'
        stosb
        pop     eax
        mov     esi, [eax+convinfo]
        cmp     esi, -1
        jz      still
@@:
        lodsb
        stosb
        test    al, al
        jz      @f
        cmp     edi, paramtest+1024
        jb      @b
        jmp     still
@@:
        mov     ebx, editor
        jmp     run

no_textopen:

    cmp  ah,11
    jne  no_view
viewset:
        inc     [viewmode]
        cmp     [viewmode], 4
        jb      @f
        mov     [viewmode], 0
@@:
    call read_directory
;    call convertation
    mov [filecursor],0
    call draw_window
    jmp still
no_view:

    cmp  ah,12	      ;move back
    jne  no_back
back:
        mov     edi, path+1024
        mov     ecx, edi
@@:
        dec     edi
        cmp     byte [edi], '/'
        jz      @f
        cmp     edi, path
        ja      @b
        jmp     still
@@:
        sub     ecx, edi
        lea     eax, [edi-path]
        mov     [cursor], eax
        xor     eax, eax
        rep     stosb
    mov [filecursor],0
    call read_directory
;    call convertation
    call draw_window
    jmp still

no_back:
    cmp  ah,13	      ;string up?
    jne  no_up
    mov  ebx,1	      ;step
up:
    mov  [downstop],0
    sub  [filecursor],ebx
    cmp  [filecursor],0
    jnl  cr_ok
    mov  [filecursor],0
cr_ok:
    jmp  draw_wd
no_up:
    cmp  ah,14		 ;string down?
    jne  no_dn
    mov  ebx,1		 ;step
down:
    cmp  [downstop],1
    je	     no_dn
    add  [filecursor],ebx
    jmp  draw_wd
no_dn:

    cmp  ah,15
    jne  no_copyclip	;copy to clipboard
copy_to_clip:
        mov     eax, 27
        mul     [filecursor]
        cmp     dword [convinfo+eax+15], 'FOL '
        jz      still
        push    eax
        mov     esi, path
        mov     edi, paramtest
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        pop     eax
        mov     esi, [convinfo+eax]
        cmp     esi, -1
        jz      still
        mov     al, '/'
        stosb
@@:
        lodsb
        stosb
        test    al, al
        jz      @f
        cmp     edi, paramtest+1024
        jb      @b
        jmp     still
@@:
        sub     edi, paramtest+1
        mov     ebx, clipfile_info
        mov     byte [ebx], 2
        mov     [ebx+12], edi
        mov     eax, 70
        int     0x40
        jmp     still
no_copyclip:

    cmp ah,16
    jne no_clippaste
paste_from_clip:
        mov     ebx, clipfile_info
        mov     byte [ebx], 0
        mov     dword [ebx+12], 1023
        mov     eax, 70
        int     0x40
        cmp     ebx, 0
        jle     still
        mov     byte [paramtest+ebx], 0
; OS allows only 256 symbols in command line
        cmp     ebx, 250
        jae     still
; we use Pascal-style strings for /RD/1/COPYR
; source file
        mov     edi, copyr_param
        mov     al, bl
        stosb
        mov     ecx, ebx
        mov     esi, paramtest
        rep     movsb
; destination path
        inc     edi
        mov     ebp, edi
        mov     esi, path
@@:
        cmp     edi, copyr_param+255
        jae     still
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
; destination file name
@@:
        dec     ebx
        jz      still
        cmp     byte [paramtest+ebx], '/'
        jnz     @b
        lea     esi, [paramtest+ebx]
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        cmp     edi, copyr_param+255
        jae     still
        jmp     @b
@@:
        mov     byte [edi], 0
        sub     edi, ebp
        mov     eax, edi
        mov     [ebp-1], al
; display
    cmp [browser], 1
    jne no_outpath
    mov  eax,4		   ; function 4 : write text to window
    mov  ebx,10*65536+67     ; [x start] *65536 + [y start]
    mov  ecx,0x00000000 ;[sc.grab_text] ; color of text RRGGBB
        mov     edx, copyr_param+1
        movzx   esi, byte [edx-1]
    int  0x40
    mov  ebx,250*65536+67	; [x start] *65536 + [y start]
    mov  ecx,0x00000000 ;[sc.grab_text] ; color of text RRGGBB
        mov     edx, ebp
        mov     esi, edi
    int  0x40
no_outpath:

; run COPYR
        mov     eax, 70
        mov     ebx, copyr_run
        int     0x40
    delay 50   ;wait recoed file
    jmp update ;still
no_clippaste:

    cmp ah,19		;Delete from floppy
    jne no_delt
delete_file:
; OS now do not support file delete
        jmp     still
no_delt:

    cmp ah,20		;I - Help
    je	    help_scr

    cmp ah,22
    jne no_headfile
    mov [drawhf],1
    call draw_window
    mov [drawhf],0
    mov [pmenu_draw],1
    jmp still
no_headfile:

    cmp ah,23
    jne no_headview
    mov [drawhv],1
    call draw_window
    mov [drawhv],0
    mov [pmenu_draw],1
    jmp still
no_headview:

    cmp ah,24
    jne no_headinfo
    mov [drawhi],1
    call draw_window
    mov [drawhi],0
    mov [pmenu_draw],1
    jmp still
no_headinfo:

;FILE MENU
    cmp ah,30
    je kfad

    cmp ah,31
    je	    copy_to_clip      ;Copy

    cmp ah,32
    je	    paste_from_clip ;Paste

    cmp ah,33
    je	    delete_file     ;Delte

    cmp ah,34
    je	    textopen	;Edit in Tinypad

    cmp ah,37
    je	    exit

;VIEW MENU
    cmp ah,40		;Sort by name
    jne no_sn
        mov     [viewmode], 0
    jmp update
no_sn:

    cmp ah,41		;Sort by extension
    jne no_se
        mov     [viewmode], 1
    jmp update
no_se:

    cmp ah,42		;Sort by size
    jne no_ss
        mov     [viewmode], 2
    jmp update
no_ss:

    cmp ah,43		;Sort by date
    jne no_sd
        mov     [viewmode], 3
    jmp update
no_sd:

;HELP MENU
    cmp ah,50		;Help?
    je	    help_scr

    cmp ah,51		;Info?
    je	    info_scr

    cmp ah,1	       ; test on exit button
    je	    exit

    jmp still

exit:
; //// Willow
;    mov eax,58
;    mov ebx,MRUfile
;    mov dword[ebx+8],255
;    inc dword[ebx]
;    int 0x40
; //// Willow
    mov eax,-1
    int 0x40

draw_wd:
;    call draw_window
;    jmp still
    mov [flick],1
    jmp anti_flick

edit_prompt:
    not  [focus]
    jmp  red

help_scr:
    mov esi,14
    mov ebp,help_text
    jmp screen

info_scr:
    mov esi,6
    mov ebp,info_text
    jmp screen

screen:
    cmp [browser], 1 ;it's browser?
    jne dialogscr

    mov eax,[procinfo.y_size]
    sub eax,90
    drawfbox 40,76,300,eax,0x00000000
    mov eax,[procinfo.y_size]
    sub eax,92
    drawfbox 41,77,298,eax,0x00ffffff
    mov edi,esi ;14
    mov ebx,(41+26)*65536+(77+20)
    mov ecx,cl_Black
    mov edx,ebp ;help_text
    jmp outlab

dialogscr:
    mov eax,[procinfo.y_size]
    sub eax,84
    drawfbox 16,54,270,eax,0x00000000
    mov eax,[procinfo.y_size]
    sub eax,86
    drawfbox 17,55,268,eax,0x00ffffff
    mov edi,esi ;14
    mov ebx,(17+10)*65536+(55+1)
    mov ecx,cl_Black
    mov edx,ebp ;help_text

outlab:     ;out labels
    mov eax,4
    mov esi,40
helploo:
    int 0x40
    add ebx,11
    add edx,40
    dec edi
    jnz helploo

    setimg 48,84,logoinfimg

    jmp still

getimgproc:
    pushad
    mov dword [edx],esi ;xs
    mov dword [edx+4],edi ;ys

    mov ebp,eax
    mov eax,dword [ecx] ;getx size
    push edx
    push ecx
    lea ecx,[eax+2*eax]

    mul ebx
    add eax,ebp ;x
    mov edx,ecx
    lea eax,[eax+2*eax]  ;eax=offset on imsrc

    mov ebp,esi ;xs
    mov ebx,edi ;ys

    pop edi
    lea esi,[eax+8+edi]

    pop edi
    add edi,8

    cld
cyc:
        push    esi
        lea     ecx, [ebp+ebp*2]
        rep     movsb
        pop     esi
    add esi,edx
    dec ebx
    jne cyc

    popad
    ret

;HELP TEXT
help_text:
       ;0123456789012345678901234567890123456789
    db '        ~~~~~ SYSTEM X-TREE ~~~~~       '
    db '                   HELP                 '
    db '                                        '
    db '                                        '
    db 'F2 - CHANGE SORTMODE (name,ext,size,dat)'
    db 'F3 - VIEW file in tinypad               '

    db 'F5 - COPY FILE to clipboard             '
    db 'F6 - PASTE FILE from clipboard          '
    db 'F11- EDIT PROMPT string                 '
    db 'F12- UPDATE directory sources           '
    db '                                        '

    db 'Enter - input to directory              '
    db 'Backspace - back to previos directory   '
    db 'PageDn/PageUp, Up/Down - move cursor    '

info_text:
    db '        ~~~~~ SYSTEM X-TREE ~~~~~       '
    db '               INFO 84 Ver              '
    db '                                        '
    db '        Create by Pavlushin Evgeni      '
    db 'with ASCL library special for Kolibri OS'
    db ' www.deck4.narod.ru      waptap@mail.ru '


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   ********    FILE BROWSER / DIALOG   *********

;draw_browser_window:

draw_window:

;    mov eax,9
;    mov ebx,procinfo
;    mov ecx,-1
;    int 0x40
;    mov  eax,[procinfo.x_size]
;    cmp  eax,66
;    jg  temp12345
;    ret
; temp12345:
    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  eax,[sc.work_button]
    mov  [b_color],eax

    mov  eax,12        ; function 12:tell os about windowdraw
    mov  ebx,1		   ; 1, start of draw
    int  0x40

;Window

    xor  eax,eax       ; function 0 : define and draw window

    cmp  [browser], 1 ;it's browser
    jne  nob1
    mov  ebx,140*65536+400     ; [x start] *65536 + [x size]
    mov  ecx,160*65536+280     ; [y start] *65536 + [y size]
    jmp  isb1
nob1:
    mov  ebx,140*65536+320     ; [x start] *65536 + [x size]
    mov  ecx,260*65536+240     ; [y start] *65536 + [y size]
isb1:
;    mov  edx,[sc.work]             ; color of work area RRGGBB
    or	     edx,0x03ffffff;000000
    int  0x40

;Get proc info
    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    int 0x40

    mov  eax,[procinfo.x_size]
    cmp  eax,66
    jg	 @f
.ret:
    ret
 @@:
    cmp  [procinfo.y_size], 0x70
    jl  .ret

    cmp  [browser], 1 ;it's browser
    jne  nob9
    mov  [listx],120
;    mov  eax,[procinfo.x_size]
    sub  eax,127;[listx]+7
    cmp  eax,10
    jl   .ret
    mov  [listxsize],eax
    mov  [listy],73
    mov  eax,[procinfo.y_size]
    sub  eax,73+7;[listy]+7
    mov  [listysize],eax
    jmp isb9
nob9:
    mov  [listx],10
;    mov  eax,[procinfo.x_size]
    sub  eax,17 ;[listx]+7
    mov  [listxsize],eax
    mov  [listy],54
    mov  eax,[procinfo.y_size]
    sub  eax,54+34;[listy]+34
    mov  [listysize],eax
isb9:


;Draw only browser components
    cmp  [browser], 1 ;it's browser
    jne  nob2

    mov  eax,[sc.grab_text]	 ; color of text RRGGBB
    or	     eax,0x10000000
    glabel 8,8,'SYSTEM X-TREE FILE BROWSER',eax

;Draw buttons headers
    mov  eax,8
    mov  ebx,8*65536+(6*8-1) ;start pos x
    mov  ecx,23*65536+10      ;start pos y
    mov  edx,22;+1000000000000000000000000000000b  ;spoke butt
    mov  edi,3		  ;draw 13 button's
    mov  esi,0x00339933
    int  0x40
    dec edi
nexthbut:
    add  ebx,(6*8)*65536
    inc  edx
    int  0x40
    dec  edi
    jnz  nexthbut

;File STRING
    glabel 8,25,'  FILE    VIEW    INFO  ',  cl_White ;Black

;BlackLine
    mov eax,[procinfo.x_size]
    sub eax,10
    drawfbox 5,35, eax, 1, cl_Black

;BlackLine2
    mov eax,[procinfo.x_size]
    sub eax,10
    drawfbox 5,68, eax, 1, cl_Black

;BlackLine2 vertical
;    mov eax,[procinfo.y_size]
;    sub eax,69+4
;    drawfbox 115, 69, 1, eax, cl_Black

;Set logo img
    setimg 34,88,logoinfimg
    glabel 20,165,'SYSTEM X-TREE',cl_Black
    add  ebx,10
    glabel ,,'FOR KolibriOS',

    add  ebx,9*65536+20
    glabel ,,'welcome to',cl_Green
    add  ebx,-15*65536+10
    glabel ,,'www.kolibrios.org',cl_Green

;    glabel ,,'Create by',cl_Green
;    add  ebx,10
;    glabel ,,'   Pavlushin',
;    add  ebx,10
;    glabel ,,'       Evgeni',


;Draw head->file buttons
    cmp [drawhf],1
    jne  no_drawhf
    mov  ebx,8*65536+6*12 ;start pos x
    mov  ecx,35*65536+10      ;start pos y
    mov  edx,30  ;spoke butt
    mov  edi,8		  ;draw 4 button's
    mov  esi,cl_Grey
    call menubutton
no_drawhf:

;Draw head->view buttons
    cmp [drawhv],1
    jne  no_drawhv
    mov  ebx,(8+6*8)*65536+6*12 ;start pos x
    mov  ecx,35*65536+10      ;start pos y
    mov  edx,40  ;spoke butt
    mov  edi,4		  ;draw 4 button's
    mov  esi,cl_Grey
    call menubutton
no_drawhv:

;Draw head->info buttons
    cmp [drawhi],1
    jne  no_drawhi
    mov  ebx,(8+12*8)*65536+6*12 ;start pos x
    mov  ecx,35*65536+10      ;start pos y
    mov  edx,50  ;spoke butt
    mov  edi,2		  ;draw 2 button's
    mov  esi,cl_Grey
    call menubutton
no_drawhi:

nob2:

;Draw buttons instruments
    mov  eax,8
    cmp  [browser], 1 ;it's browser
    jne  nob3
    mov  ebx,10*65536+16+5 ;start pos x
    mov  ecx,37*65536+15      ;start pos y
    jmp isb3
nob3:
    mov  ebx,16*65536+16+5 ;start pos x
    mov  ecx,29*65536+15      ;start pos y
isb3:
    mov  edx,8;+1000000000000000000000000000000b  ;spoke butt
    mov  edi,13        ;draw 13 button's
    mov  esi,cl_Grey
    int  0x40
    dec edi
nextbut:
    add  ebx,(16+6)*65536
    inc  edx
    int  0x40
    dec  edi
    jnz  nextbut


    cmp  [browser], 1 ;it's browser
    jne  nob4
;But img browser
    setimg 10,37,butimg
;left logo
    add eax,[procinfo.x_size]
    sub eax,80
    mov [temp],eax
    setimg [temp],37,logoimg
    jmp isb4
nob4:
;But img dialog
    setimg 16,29,butimg
isb4:

    cmp  [browser], 1 ;it's browser
    jne  nob5

    mov [urlx],48
    mov [urly],55
    mov eax,[procinfo.x_size]
    sub eax,48+10
    mov [urlxsize],eax
    mov [urlysize],12

    glabel 20,57,"URL:",cl_Black

;Out view mode info
        movzx   edx, [viewmode]
        shl     edx, 4
    mov eax,4
    mov ebx,180*65536+25
    mov ecx,cl_Black
    add edx,modetext
    mov esi,16
    int 0x40

;List size
    outcount [listsize],294,25,cl_Black,4*65536
    jmp isb5

head_dlg: db 'OPEN FILE'
      db 'SAVE FILE'
but_dlg:  db 'OPEN'
      db 'SAVE'

nob5:

    mov [urlx],10
    mov eax,[procinfo.y_size]
    sub eax,24
    mov [urly],eax
    mov eax,[procinfo.x_size]
    sub eax,80
    mov [urlxsize],eax
    mov [urlysize],12

    cmp byte [dlg_type],'O'    ;if byte O - is Open dialog
    jne no_openh
    mov edx,head_dlg	      ;draw in head OPEN FILE
    jmp out_laby
no_openh:
    cmp byte [dlg_type],'S'    ;if byte S - is Save dialog
    jne no_saveh
    mov edx,head_dlg+9	      ;draw in head SAVE FILE
out_laby:
    mov ebx,8*65536+8
    mov ecx,[sc.grab_text]    ; color of text RRGGBB
    or	    ecx,0x10000000
    mov esi,9
    mov eax,4
    int 0x40
no_saveh:


;Draw OPEN\SAVE button
    mov ebx,0*65536+50
    mov ecx,0*65536+12
    mov eax,[procinfo.x_size]
    sub eax,63
    shl eax,16
    add ebx,eax
    mov eax,[procinfo.y_size]
    sub eax,25
    shl eax,16
    add ecx,eax
    mov  eax,8
    mov  edx,5 ;button ID
    mov  esi,0x006699aa  ;gradient!!!
    int  0x40

; label OPEN or SAVE
    mov ebx,[procinfo.x_size]
    sub ebx,48
    mov eax,[procinfo.y_size]
    sub eax,22
    shl ebx,16
    add ebx,eax

    cmp byte [dlg_type],'O'    ;if byte O - is Open dialog
    jne no_openb
    mov edx,but_dlg	   ;draw in head OPEN FILE
    jmp out_labx
no_openb:
    cmp byte [dlg_type],'S'    ;if byte S - is Save dialog
    jne no_saveb
    mov edx,but_dlg+4	       ;draw in head SAVE FILE
out_labx:
    mov ecx,cl_White
    mov esi,4
    mov eax,4
    int 0x40
no_saveb:

isb5:

anti_flick:

    cmp [flick],2
    je draw_url
    cmp [flick],0
    jne no_flick_url

;***************************
;    DRAW URL LINE
;***************************
draw_url:

;Draw URL focus button
    mov ebx,[urlx]
    shl ebx,16
    add ebx,[urlxsize]
    mov ecx,[urly]
    shl ecx,16
    add ecx,[urlysize]
    dec ebx
    dec ecx
    mov  eax,8
    mov  edx,2 ;button ID
    mov  esi,0x00aaaaaa  ;gradient!!!
    int  0x40

;Draw URL String
    mov eax,13
    mov edx,cl_Black
    int 0x40
    add ebx,1*65536-2+1
    add ecx,1*65536-2+1
    mov edx,cl_White
    int 0x40

;Draw URL Cursor
    mov eax,6
    mul [cursor]
    mov ebx,[urlx]
    add ebx,eax
    shl ebx,16
    add ebx,2*65536+1
    mov ecx,[urly]
    shl ecx,16
    add ecx,[urlysize]
    add ecx,2*65536-4
    mov eax,13
    mov edx,cl_Black
    int 0x40

; OUT TEXT
    mov eax,[urlxsize]	    ;calculating text leight
    sub eax,8
    mov ebx,6
    div ebx
    mov esi,eax

    mov  ebx,[urlx]
    shl  ebx,16
    add  ebx,[urly]
    add  ebx,3*65536+2
    mov  eax,4		   ; function 4 : write text to window
    mov  ecx,0x00000000 ;[sc.grab_text] ; color of text RRGGBB
    mov  edx,path	 ; pointer to text beginning
    int  0x40

    cmp  [flick],2
    jne  no_flick_url
    mov  [flick],0
    jmp  still
no_flick_url:


;***************************
;    DRAW FILE LIST
;***************************

;Draw Scroll Line
    mov eax,13

    mov ebx,[listx]
    add ebx,[listxsize]
    sub ebx,[scrollsize]
    shl ebx,16
    add ebx,dword [scrollsize]
    mov ecx,[listy]
    add ecx,scrollbutsize
    shl ecx,16
    add ecx,[listysize]
    sub ecx,scrollbutsize*2
    mov edx,[scrollcolor] ;0x00006600
    int 0x40

;Draw Scroll Box
    mov  edx,0
    mov  eax,[listysize]
    sub  eax,2
    mov  ebx,dword 10
    div  ebx

    mov ebx,eax
    cmp ebx,[listsize]	    ;filelistsize in ebx
    jae    notusescroll
;usescroll
; calculate scroll size
        mov     eax, [listysize]
        sub     eax, 2*scrollbutsize
        push    eax
        mul     ebx
        div     [listsize]
        cmp     eax, 5
        jae     @f
        mov     al, 5
@@:
        mov     [scroll_height], eax
; calculate scroll pos
        sub     [esp], eax
        mov     eax, [listsize]
        sub     eax, ebx
        mov     ecx, eax
        cmp     eax, [filecursor]
        jb      @f
        mov     eax, [filecursor]
@@:
        mul     dword [esp]
        div     ecx
        mov     [scroll_pos], eax
        pop     ecx
; draw
        lea     ecx, [eax+scrollbutsize]
        add     ecx, [listy]
        shl     ecx, 16
        mov     cx, word [scroll_height]
        mov     eax, 13
        mov     ebx, [listx]
        add     ebx, [listxsize]
        sub     ebx, [scrollsize]
        shl     ebx, 16
        mov     bx, word [scrollsize]
        mov     edx, [scrollboxcol]
        int     0x40
notusescroll:


;Draw list button for get file name
    mov  ebx,[listx]
    shl  ebx,16
    add  ebx,[listxsize]
    sub  ebx,15      ;right free zone
    sub  ebx,[scrollsize]
    mov  ecx,[listy]
    shl  ecx,16
    add  ecx,[listysize]

    mov  eax,8
    mov  edx,4+1000000000000000000000000000000b  ;spoke butt
    int  0x40

    add  ebx,15
    mov  eax,13
    mov  edx,[listcolor] ;ffffff
    int  0x40

;Draw up/down buttons
    mov  ebx,[listx]
    add  ebx,[listxsize]
    sub  ebx,[scrollsize]
    shl  ebx,16
    add  ebx,[scrollsize]
    mov  ecx,[listy]
    shl  ecx,16
    add  ecx,scrollbutsize-1
    mov  eax,8
    mov  edx,6+1000000000000000000000000000000b  ;spoke butt
    int  0x40

    inc  ecx
    mov  eax,13
    mov  edx,[scrollbutcol] ;ffffff
    int  0x40

; Draw image on up button
    pushad
    shr  ebx,16
    mov  edx,ebx
    shl  edx,16
    shr  ecx,16
    add  edx,ecx
    mov  ecx,8*65536+9
    mov  ebx,upsb+8
    mov  eax,7
    int  0x40
    popad


    dec  ecx	 ;correction
    mov  edx,7+1000000000000000000000000000000b  ;spoke butt
    mov  eax,[listysize]
    sub  eax,scrollbutsize
    shl  eax,16
    add  ecx,eax

    mov  eax,8
    int  0x40

    inc  ecx
    mov  eax,13
    mov  edx,[scrollbutcol] ;ffffff
    int  0x40

; Draw image on down button
    pushad
    shr  ebx,16
    mov  edx,ebx
    shl  edx,16
    shr  ecx,16
    add  edx,ecx
    mov  ecx,8*65536+9
    mov  ebx,dnsb+8
    mov  eax,7
    int  0x40
    popad


; Draw text in file list

;    mov  eax,[listxsize]
;    sub  eax,40*6  ;leight of string
;    shr  eax,1
;    add  eax,[listx]
        mov     eax, [listx]
        add     eax, 12+4 ; for icons
    shl  eax,16
    add  eax,[listy]
    add  eax,2

    mov  [filelistxy],eax ;dword 19*65536+58

    mov  edx,0
    mov  eax,[listysize]
    sub  eax,2
    mov  ebx,dword 10
    div  ebx
    mov  [filelistsize],eax  ;dword 40

; OUT FILE DATA
    mov  eax,[filecursor]     ;calc cursor position
    mov  ebx,27
    mul  ebx

;OUT TEXT
    mov  ebp,4096 ; 16             ;out strings process
    sub  ebp,[filecursor]
    mov  edx,convinfo ;fileinfo+11
    add  edx,eax
    mov  ebx,dword [filelistxy]
loo:
        cmp     dword [edx], -1
        jz      noout
    mov  ecx,0x00888888        ;for another file's color white
    cmp  [edx+15],dword 'FOL ' ;folder yellow
    jne  nb
    mov  ecx,0x00006666
    jmp cset1
nb:
        mov     eax, [edx]
        xor     edi, edi
; find extension and set color
@@:
        inc     eax
        cmp     byte [eax-1], 0
        jz      @f
        cmp     byte [eax-1], '.'
        jnz     @b
        mov     edi, eax
        jmp     @b
@@:
        test    edi, edi
        jz      @f
        mov     edi, [edi]
        and     edi, not 0x20202020     ; toupper
@@:
;Color set
; text's blue
        mov     ecx, 0x00446666
        cmp     edi, 'TXT'
        jz      cset
        cmp     edi, 'INC'
        jz      cset
        cmp     edi, 'ASM'
        jz      cset
        cmp     edi, 'RTF'
        jz      cset
; picture's pure
        mov     ecx, 0x00226688
        cmp     edi, 'BMP'
        jz      cset
        cmp     edi, 'JPG'
        jz      cset
        cmp     edi, 'JPEG'
        jz      cset
        cmp     edi, 'GIF'
        jz      cset
        cmp     edi, 'PNG'
        jz      cset
        cmp     edi, 'WAV'
        jz      cset
        cmp     edi, 'MID'
        jz      cset
; execute's green
        mov     ecx, 0x00008866
        test    edi, edi
        jz      cset
; for another file's color white
        mov     ecx, 0x00888888
cset:

cset1:
    push edx
    mov  edx,[edx]
        push    ebx edx
        mov     eax, [listxsize]
        sub     eax, [scrollsize]
        sub     eax, 12+4
        xor     edx, edx
        mov     ebx, 6
        div     ebx
        pop     edx ebx
        sub     eax, 25
        xor     esi, esi
@@:
        cmp     byte [edx+esi], 0
        jz      @f
        inc     esi
        cmp     esi, eax
        jbe     @b
        dec     esi
@@:
        push    eax
    mov  eax,4
    int  0x40
        cmp     byte [edx+esi], 0
        jz      @f
        pushad
        mov     edx, more_char
        mov     eax, esi
        imul    eax, 6*65536
        add     ebx, eax
        mov     esi, 1
        mov     eax, 4
        int     0x40
        popad
@@:
        pop     eax
    pop  edx
    push ebx edx
        inc     eax
        imul    eax, 6*65536
        add     ebx, eax
    add  edx,4
    mov  esi,23
    mov  eax,4
    int  0x40
    pop  edx ebx

    pushad
    cmp  [edx+15],dword 'Fvol' ;volume label
    jne  no_volico
    push hdico+8
    jmp out_ico
no_volico:
    cmp  [edx+15],dword 'FOL '
    jne  no_folico
    cmp  [edx+4],dword 'HARD'
    jne  no_hdico
    push hdico+8
    jmp out_ico
no_hdico:
    cmp  [edx+4],dword 'RAMD'
    jne  no_rdico
    push rdico+8
    jmp out_ico
no_rdico:
    cmp  [edx+4],dword 'FLOP'
    jne  no_fdico
    push rdico+8
    jmp out_ico
no_fdico:
    cmp  [edx+4],dword 'CD-R'
    jne  no_cdico
    push cdico+8
    jmp out_ico
no_cdico:
    push folico+8
    jmp out_ico
no_folico:
    cmp  edi,dword 'BMP'
    je	     is_imgico
    cmp  edi,dword 'JPG'
    je	     is_imgico
    cmp  edi,dword 'JPEG'
    je	     is_imgico
    cmp  edi,dword 'GIF'
    je	     is_imgico
; //// Willow
    cmp  edi,dword 'PNG'
    je	 is_imgico
; //// Willow
    cmp  edi,dword 'WAV'
    je	     is_imgico
    cmp  edi,dword 'MID'
    je	     is_imgico
    jmp  no_imgico
is_imgico:
    push imgico+8
    jmp out_ico
no_imgico:
    cmp  edi,dword 'ASM'
    je	     is_asmincico
    cmp  edi,dword 'INC'
    je	     is_asmincico
    jmp  no_asmincico
is_asmincico:
    push asmincico+8
    jmp out_ico
no_asmincico:
    cmp  edi,dword 'RTF'
    je	 @f
    cmp  edi,dword 'TXT'
    jne  no_txtico
 @@:
    push txtico+8
    jmp out_ico
no_txtico:
    test edi,edi
    jne  no_execico
    push execico+8
    jmp out_ico
no_execico:
    cmp  [edx+15],dword 'DAT '
    jne  no_datico
    push datico+8
    jmp out_ico
no_datico:
    cmp  [edx+15],dword 'DEL '
    jne  no_out_ico
    push delico+8
    jmp out_ico
out_ico:

    mov  edx,ebx
    sub  edx,14*65536+1
    mov  ecx,12*65536+9
;    mov  ebx,upsb+8
    pop  ebx
    mov  eax,7
    int  0x40
no_out_ico:
    popad


    add  ebx,10
noout:
    add  edx,27

    dec  [filelistsize]
    jz	     extloo

    dec  ebp
    jnz  loo
dext:
    mov [downstop],1

extloo:

    cmp  [browser], 1 ;it's browser
    jne  nob8

;Draw text for head->file buttons
    cmp [drawhf],1
    jne  no_drawhftext
    drawfbox 8,35,(6*12)+1,11*8,0x00000000
    drawfbox 9,36,(6*12)-1,(11*8)-2,0x00ffffff

    mov edi,8
    mov ebx,9*65536+37
    mov ecx,cl_Black
    mov edx,file_text_label
    call drawmenu
    jmp no_drawhftext

file_text_label:
    db	     '    Open    '
    db	     '    Copy    '
    db	     '    Paste   '
    db	     '   Delete   '
    db	     '   Tinypad  '
    db	     '    Edit    '
    db	     '            '
    db	     '    Exit    '

no_drawhftext:

;Draw text for head->view buttons
    cmp [drawhv],1
    jne  no_drawhvtext
    drawfbox (8+6*8),35,(6*12)+1,(11*4),0x00000000
    drawfbox (9+6*8),36,(6*12)-1,(11*4)-2,0x00ffffff

    mov edi,4
    mov ebx,(9+6*8)*65536+37
    mov ecx,cl_Black
    mov edx,view_text_label
    call drawmenu
    jmp no_drawhvtext

view_text_label:
    db	     '  Name sort '
    db	     '  Ext. sort '
    db	     '  Size sort '
    db	     '  Date sort '

no_drawhvtext:

;Draw text for head->info buttons
    cmp [drawhi],1
    jne  no_drawhitext
    drawfbox (8+12*8),35,(6*12)+1,(11*2),0x00000000
    drawfbox (9+12*8),36,(6*12)-1,(11*2)-2,0x00ffffff

    mov edi,2
    mov eax,4
    mov ebx,(9+12*8)*65536+37
    mov ecx,cl_Black
    mov edx,info_text_label
    call drawmenu
    jmp no_drawhitext

info_text_label:
    db	     '   Help     '
    db	     '   About    '

no_drawhitext:

nob8:
    cmp  [flick],1
    jne  no_flick
    mov  [flick],0
    jmp  still
no_flick:

    mov  eax,12        ; function 12:tell os about windowdraw
    mov  ebx,2		   ; 2, end of draw
    int  0x40

    ret

;FILE LIST PARAMETRS

listx	    dd 15
listy	    dd 72
listxsize dd 350
listysize dd 41

filelistxy    dd 0
filelistsize  dd 0
scrollsize    dd 8
listcolor     dd 0xffffff ;0xeeeeee
scrollcolor   dd 0x778877
scrollboxcol  dd 0x335533
scrollbutcol  dd 0x224422
scrollbutsize = 9

;URL LINE PARAMETRS

urlx	 dd 10
urly	 dd 20
urlxsize dd 350
urlysize dd 12


drawmenu:
    mov eax,4
    mov esi,12
menuloo:
    int 0x40
    add ebx,11
    add edx,12
    dec edi
    jnz menuloo
    ret

menubutton:
    mov  eax,8
nextmenubut:
    int  0x40
    add  ecx,11*65536
    inc  edx
    dec  edi
    jnz  nextmenubut
    ret

;*****************************
; READ FILE STRUCTURE PROC
;*****************************

read_directory:

;STEP 1 CLEAR CONVINFO
    mov  edi,convinfo
    mov  al,255
    mov  ecx,4096*27
    cld
    push edi
    rep  stosb
    pop  edi

;STEP 2 TEST ON HD OR PARTITION
    cmp [path],byte '/'
    je nstep
; copy info on RD and FD
        mov     esi, hdimg
        mov     ecx, 2*27
        rep     movsb
; hard disks
        mov     eax, 4
.1:
        mov     ecx, 27
        cmp     dword [esi], 0
        jz      .2
        rep     movsb
        jmp     .3
.2:
        add     esi, ecx
.3:
        dec     eax
        jnz     .1
        mov     [listsize], 0
        ret
nstep:
    cmp [path+3],byte '/'
    je nstep2
    cmp [path+4],byte '/'
    je nstep2
    mov ecx,1
    cmp [path+1],word 'RD'
    jz  loxpt
    cmp [path+1],word 'CD'
    jz  loxpt
    mov ecx,2
    cmp [path+1],word 'FD'
    jz  loxpt
    cmp [path+1],dword 'HD0'
    jne nostep_HD0
    movzx ecx,byte [table_area+2]
    jmp loxpt
nostep_HD0:
    cmp [path+1],dword 'HD1'
    jne nostep_HD1
    movzx ecx,byte [table_area+3]
    jmp loxpt
nostep_HD1:
    cmp [path+1],dword 'HD2'
    jne nostep_HD2
    movzx ecx,byte [table_area+4]
    jmp loxpt
nostep_HD2:
    cmp [path+1],dword 'HD3'
    jne nostep_HD3
    movzx ecx,byte [table_area+5]
    jmp loxpt
nostep_HD3:

    mov ecx,2

loxpt:
        cmp     ecx, 20
        jbe     @f
        mov     ecx, 20
@@:
        mov     eax, a1
        mov     esi, ptimg
@@:
        stosd
        add     eax, 3
        push    ecx esi
        mov     ecx, 23
        rep     movsb
        pop     esi ecx
        loop    @b
    mov [listsize],0
    ret
nstep2:

;STEP 3 was deleted

;STEP 4 READ DATA FROM HD

loorhd:
    mov  eax,70
    mov  ebx,fileinfoblock
    int  0x40

    cmp eax,6
    je	    end_of_dir

;  It's erorr's test is poor code

    cmp eax,1
    jne no_inv_part
    glabel 10,10,'Invalid partition or hd base',cl_Red+font_Big
    jmp end_of_dir
no_inv_part:
    cmp eax,3
    jne no_unk_fs
    glabel 10,10,'Unknown file system',cl_Red+font_Big
    jmp end_of_dir
no_unk_fs:

end_of_dir:
; -1 -> 0
        cmp     ebx, -1
        sbb     ebx, -1
    mov  [dirlen],ebx

    ; command successful

        mov     esi, outinfo+32
        and     [listsize], 0
        test    ebx, ebx
        jz      nofiles

  newlineb:
        push    ebx

        mov     edi, fileinfo
        push    edi
        mov     al, ' '
        mov     ecx, 27
        rep     stosb
        pop     edi
        push    esi
        add     esi, 40
        mov     [edi], esi
        pop     esi

        mov     eax, 'DAT '     ; data or ... ?
        mov     cl, [esi]
        test    cl, 0x10
        jz      @f
        mov     eax, 'FOL '
        mov     [edi+15], eax
        jmp     nosize
@@:
        test    cl, 8
        jz      novol
        mov     eax, 'Fvol'
        mov     [edi+15], eax
        jmp     nosize
novol:
        mov     [edi+15], eax

; size
        mov     eax, [esi+32]
        push    esi
        lea     esi, [edi+26]
        mov     ebx, 10
        mov     ecx, 8
newnum:
        xor     edx, edx
        div     ebx
        add     dl, '0'
        mov     [esi], dl
        test    eax, eax
        jz      zernum
        dec     esi
        loop    newnum
zernum:
        pop     esi
nosize:

; date
        mov     dword [edi+6], '.  .'
; year
        movzx   eax, word [esi+28+2]
        mov     ecx, 4
newdel1:
        xor     edx, edx
        mov     ebx, 10
        div     ebx
        add     edx, '0'
        mov     [edi+ecx+9], dl
        loop    newdel1
; month
        movzx   eax, byte [esi+28+1]
        xor     edx, edx
        div     ebx
        add     al, '0'
        add     dl, '0'
        mov     [edi+7], al
        mov     [edi+8], dl
; day
        movzx   eax, byte [esi+28]
        xor     edx, edx
        div     ebx
        add     al, '0'
        add     dl, '0'
        mov     [edi+4], al
        mov     [edi+5], dl

;STEP 5 Test on WRITE OR NOTWRITE
        cmp     word [esi+40], '.'
        jz      nextfl
        cmp     word [esi+40], '..'
        jnz     @f
        cmp     byte [esi+42], 0
        jz      nextfl
@@:

; STEP 6 SORT ALGORYTHM

        push    esi
        xor     esi, esi

; At first Fold after Dat

ftestname:
        cmp     dword [convinfo+esi], -1
        jz      writefile
    cmp byte [fileinfo+15],'F'
    je	    F
    cmp byte [fileinfo+15],'D'
    jne add_element
D:  cmp byte [convinfo+esi+15],'D'
    je	    add_element
    cmp byte [convinfo+esi+15],'F'
    je	    skipfile
    jmp add_element
F:  cmp byte [convinfo+esi+15],'D'
    je writenow
;    cmp byte [convinfo+esi+15],'F'
;    je  add_element
;    jmp add_element

add_element:
; compare items
        movzx   eax, [viewmode]
        call    [compare+eax*4]
        jb      writenow
skipfile:
    add esi,27
    jmp ftestname

writenow:
    mov ecx,4096*27-1
    sub ecx,esi
ldloop:
    mov al,[convinfo+ecx+esi]
    mov [convinfo+ecx+esi+27],al
    dec ecx
    jns ldloop

writefile:
    mov ecx,26
wfloop:
    mov al,[fileinfo+ecx]
    mov [convinfo+ecx+esi],al
    dec ecx
    jns wfloop
        inc     [listsize]
        pop     esi

nextfl:
        add     esi, 304
        pop     ebx
        dec     ebx
        jnz     newlineb

nofiles:
        ret

toupper:
        cmp     al, 'a'
        jb      .ret
        cmp     al, 'z'
        ja      @f
.sub20:
        sub     al, 0x20
.ret:
        ret
@@:
        cmp     al, 0xA0
        jb      .ret
        cmp     al, 0xB0
        jb      .sub20
        cmp     al, 0xE0
        jb      .ret
        cmp     al, 0xF0
        jae     @f
        sub     al, 0xE0-0x90
        ret
@@:
        cmp     al, 0xF1
        jnz     .ret
        dec     eax
        ret

compare_date:
        pushad
        mov     al, [fileinfo+10]
        cmp     al, [convinfo+esi+10]
        jnz     .ret
        mov     al, [fileinfo+11]
        cmp     al, [convinfo+esi+11]
        jnz     .ret
        mov     al, [fileinfo+12]
        cmp     al, [convinfo+esi+12]
        jnz     .ret
        mov     al, [fileinfo+13]
        cmp     al, [convinfo+esi+13]
        jnz     .ret
        mov     al, [fileinfo+7]
        cmp     al, [convinfo+esi+7]
        jnz     .ret
        mov     al, [fileinfo+8]
        cmp     al, [convinfo+esi+8]
        jnz     .ret
        mov     al, [fileinfo+4]
        cmp     al, [convinfo+esi+4]
        jnz     .ret
        mov     al, [fileinfo+5]
        cmp     al, [convinfo+esi+5]
        jz      compare_name.1
.ret:
        popad
        ret
compare_name:
        pushad
.1:
        mov     edi, dword [convinfo+esi]
        mov     esi, dword [fileinfo]
        call    strcmpi
        popad
        ret
compare_ext:
        pushad
        mov     esi, dword [convinfo+esi]
        mov     edi, dword [fileinfo]
        call    find_ext
        xchg    esi, edi
        call    find_ext
        call    strcmpi
        popad
        jz      compare_name
        ret
compare_size:
        pushad
        lea     edi, [convinfo+esi+19]
        lea     esi, [fileinfo+19]
        mov     ecx, 8
        repz    cmpsb
        popad
        jz      compare_name
        ret

strcmpi:
        lodsb
        call    toupper
        push    eax
        mov     al, [edi]
        inc     edi
        call    toupper
        cmp     [esp], al
        pop     eax
        jnz     @f
        test    al, al
        jnz     strcmpi
@@:
        ret

find_ext:
        lodsb
        test    al, al
        jz      .noext
        cmp     al, '.'
        jnz     find_ext
        ret
.noext:
        dec     esi
        ret

;******************************************************************************

; DATA AREA
pmenu_draw dd 0  ;if poup menu is draw,update all window
flick	     dd 0  ;anti flick on ?
drawhf	       dd 0  ;draw file menu?
drawhv	       dd 0  ;draw view menu?
drawhi	       dd 0  ;draw info menu?
cursor	       dd 0  ;cursor in prompt line
focus	     dd 0  ;prompt edit or window?
downstop   dd 0
filecursor dd 0
mousex	       dd 0
mousey	       dd 0
blocksread dd 0
listsize   dd 0  ;num of files in directory
temp	   dd 0
readblock  dd 1
dlg_type   db 0 ;S-save O-open
browser    db 0  ;0-dialog, 1-browser
viewmode   db 0  ;folder sort

compare dd      compare_name
        dd      compare_ext
        dd      compare_size
        dd      compare_date

aRD     db      'RD',0
aFD     db      'FD',0
aHD0    db      'HD0',0
aHD1    db      'HD1',0
aHD2    db      'HD2',0
aHD3    db      'HD3',0
aCD0    db      'CD0',0
aCD1    db      'CD1',0
aCD2    db      'CD2',0
aCD3    db      'CD3',0
a1      db      '1',0,0
a2      db      '2',0,0
a3      db      '3',0,0
a4      db      '4',0,0
a5      db      '5',0,0
a6      db      '6',0,0
a7      db      '7',0,0
a8      db      '8',0,0
a9      db      '9',0,0
a10     db      '10',0
a11     db      '11',0
a12     db      '12',0
a13     db      '13',0
a14     db      '14',0
a15     db      '15',0
a16     db      '16',0
a17     db      '17',0
a18     db      '18',0
a19     db      '19',0
a20     db      '20',0

hdimg:
        dd      aRD
        db      'RAMDISK    FOL         '
        dd      aFD
        db      'FLOPPYDISK FOL         '
hdimg1  dd      0
        db      'HARDDISK   FOL         '
hdimg2  dd      0
        db      'HARDDISK   FOL         '
hdimg3  dd      0
        db      'HARDDISK   FOL         '
hdimg4  dd      0
        db      'HARDDISK   FOL         '
ptimg   db      'PARTITION  FOL         '

modetext:
      ;0123456789012345
   db 'SORT BY NAME   0'
   db 'SORT BY EXT.   1'
   db 'SORT BY SIZE   2' ;type sort
   db 'SORT BY DATE   3'
   db 'DEL SORT NAME  4'
   db 'DEL SORT EXT.  5'
   db 'DEL SORT SIZE  6' ;type sort
   db 'DEL SORT DATE  7'

dirlen	      dd     0x1
b_color   dd   0x6677cc

; //// Willow
;MRUfile:
;   dd 0x0
;   dd 0x0
;   dd 0x1
;   dd path
;   dd tempzone
;   db '/RD/1/MRU.LST',0
; //// Willow

;Name of programs
editor	      db '/RD/1/TINYPAD',0
bmpview   db '/RD/1/MV',0
jpgview   db '/RD/1/JPEGVIEW',0
gifview   db '/RD/1/GIFVIEW',0
ac97wav   db '/RD/1/AC97WAV',0
rtfread   db '/RD/1/RTFREAD',0
; //// Willow
pngview   db '/RD/1/@RCHER',0
; //// Willow
midamp	  db '/RD/1/MIDAMP',0

more_char db 10h

fileinfo_start:
        dd      7
        dd      0
fileinfo_params:
        dd      0
        dd      0
        dd      0
        db      0
fileinfo_name:
        dd      0

clipfile_info:
        dd      ?
        dd      0
        dd      0
        dd      ?
        dd      paramtest
        db      '/RD/1/CLIPFILE.TXT',0
copyr_run:
        dd      7
        dd      0
        dd      copyr_param
        dd      0
        dd      0
        db      '/RD/1/COPYR',0

fileinfoblock:
   dd 0x1    ; read folder
   dd 0x0    ; first block
   dd 0x0    ; flags (ANSI names)
   dd 4095   ; number of blocks to read
   dd outinfo	  ; ret offset
path:
times 1024 db 0	    ;path

table_area:
rb 10

;rb 256
but_file:
file 'systr12.GIF'

I_END:

scroll_height dd ?
scroll_pos dd ?

butimg:
rb 400*16*3+8	  ;buttons (left pice of picture)
logoimg:
rb 70*16*3+8	;logo (right pice of picture)
logoinfimg:
rb 60*60*3+8	;logoinfo (right pice of picture)
upsb:
rb 8*9*3+8    ;up scroll button
dnsb:
rb 8*9*3+8    ;down scroll button

;ICONS RESERVE AREA
hdico:
rb 12*9*3+8
rdico:
rb 12*9*3+8
folico:
rb 12*9*3+8
datico:
rb 12*9*3+8
delico:
rb 12*9*3+8
imgico:
rb 12*9*3+8
txtico:
rb 12*9*3+8
asmincico:
rb 12*9*3+8
execico:
rb 12*9*3+8
cdico:
rb 12*9*3+8

tempimg:    ;reserve ram for images
rb 400*100*3+8	     ;for picture
rb 8000

gif_hash:
rd 4096

MYPID:
rd 8

param_area:
rb 256
paramtest:
rb 1024
filedir:
rb 1024

procinfo process_information
sc system_colors

fileinfo:
rb 200 ;4096*62+1024

copyr_param     rb      256

outinfo:
rb 32+4096*304

convinfo:
rb 4096*27
; stack
        align 4
        rb      1024
RAM_END:
