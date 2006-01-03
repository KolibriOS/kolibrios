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
;600000 bytes memory!

;******************************************************************************
  use32
  org    0x0
  db     'MENUET01'   ; 8 byte id
  dd     0x01         ; header version
  dd     START        ; program start
  dd     I_END        ; program image size
  dd     RAM_END      ; memory
  dd     RAM_END      ; stack
  dd     param_area ,0x0   ; param,icon
;  dd 0,0

;******************************************************************************
include 'lang.inc'
include 'macros.inc'
include 'ascl.inc'
include 'ascgl.inc'
    gif_hash_offset = gif_hash

START:                          ; start of execution
; //// Willow
    mov eax,58
    mov ebx,MRUfile
    int 0x40
; //// Willow

    mov eax,40
    mov ebx,0100111b
    int 0x40

    cmp byte [param_area],0 ;test parameters line
    jne no_brow             ;it's dialog
    mov [browser],dword 1   ;it's browser
no_brow:

    cmp [browser],dword 1
    je  no_dlg

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
    mov ebx,0     ;y
    mov esi,286     ;xs
    mov edi,16      ;ys
    mov ecx,tempimg ;src
    mov edx,butimg   ;dest
    call getimgproc
    mov eax,288
    mov esi,60
    mov edx,logoimg  ;dest
    call getimgproc
    mov eax,0  ;x
    mov ebx,16     ;y
    mov esi,51     ;xs
    mov edi,esi      ;ys
    mov edx,logoinfimg   ;dest
    call getimgproc
    mov eax,51  ;x
    mov esi,8     ;xs
    mov edi,9      ;ys
    mov edx,upsb   ;dest
    call getimgproc
    mov eax,51+8  ;x
    mov edx,dnsb   ;dest
    call getimgproc

    mov eax,51+16  ;x
    mov ebx,16     ;y
    mov esi,12     ;xs
    mov edi,9      ;ys

    mov ecx,tempimg ;src
    mov edx,hdico   ;dest
    mov ebp,9
loogetimg:
    call getimgproc
    add edx,9*12*3+8
    add eax,12
    dec ebp
    jnz loogetimg
    jmp endgip

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

    mov ecx,esi ;xs
    mov ebx,edi ;ys

    mov ebp,esi

    pop edi
    lea esi,[eax+8+edi]

    pop edi
    add edi,8

    cld
cyc:
    movsw
    movsb
    dec ecx
    jne cyc
    add esi,edx
    mov ecx,ebp ;xs
    sub esi,ecx
    sub esi,ecx
    sub esi,ecx
    dec ebx
    jne cyc

    popad
    ret

endgip:

    call read_directory
;    call convertation
red:
    call draw_window            ; at first, draw the window

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
    cmp eax,1
    jne still

scrl:
    mov eax,37
    mov ebx,1
    int 0x40
    mov ebx,eax
    shr eax,16       ;x
    and ebx,0xffff   ;y

    mov ebp,eax
    sub ebp,[listx] ;[procinfo.x_size]
    sub ebp,[listxsize]
    add ebp,[scrollsize]
    cmp ebp,dword [scrollsize] ;8
    ja menu_test ; still

    mov ebp,ebx
    sub ebp,[listy] ;76    ;up scroll
    sub ebp,[scrollbutsize]
    cmp ebp,0
    jl  menu_test
    mov [mousey],ebp

    push eax ebx edx
    mov  edx,0
    mov  eax,[listysize]
    sub  eax,2
    mov  ebx,dword 10
    div  ebx

    mov  [filelistsize],eax
    mov  ebx,eax
    cmp  ebx,[listsize]  ;filelistsize in ebx
    ja   notusescrl

    mov edx,0
    mov eax,[listysize]
    sub eax,[scrollbutsize]
    sub eax,[scrollbutsize]

    shl eax,16+6
    div dword [listsize]
    mul ebx
    shr eax,16+6

    mov ebp,eax    ; in ebp ysize of scroll

    mov edx,0
    mov eax,[listsize]

    mov ebx,[listysize]
    sub ebx,[scrollbutsize]
    sub ebx,[scrollbutsize]
    shl eax,16
    div ebx ;dword [listsize]
    mul [mousey]
    shr eax,16

    mov ebx,[listsize]
    sub ebx,[filelistsize]
    cmp eax,ebx
    jnae no_cor
    mov eax,[listsize]      ;correction for full dirs (1000 files)
    sub eax,[filelistsize]
no_cor:
    mov [filecursor],eax

    jmp  usescrl
notusescrl:
    mov [filecursor],0 ;ebp
usescrl:

    pop  edx ebx eax

    mov esi,[listy];[procinfo.y_size]
    add esi,[listysize]
    sub esi,[scrollbutsize]

    cmp ebx,esi
    jna oks ;menu_test ;still

    sub esi,ebp
    inc esi ;correction
    cmp ebx,esi
    ja  menu_test ;still
oks:

    mov [flick],1
    jmp anti_flick ;red

menu_test:
    cmp [pmenu_draw],1 ;if menu is show, update all
    jne still
    mov [pmenu_draw],0
    jmp red            ;update all window

;this function not use in dialog when poup menu's is not used
;in dialog's

;===================
; Test keyboard
;===================
key:                          ; key
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
    jne  no_k_ent
    jmp  kfad
no_k_ent:
    cmp  ah,key_Bspace
    je   back
    cmp  ah,key_F2
    je   viewset
    cmp  ah,key_F3
    je   textopen
    cmp  ah,key_F5
    je   copy_to_clip
    cmp  ah,key_F6
    je   paste_from_clip
    cmp  ah,key_F11
    je   edit_prompt
    cmp  ah,key_F12
    je   update
    jmp  still

; test input string
con_edit:
    cmp  ah,key_Enter
    jne  no_con_ent
    not  [focus]
    jmp  savetest
    jmp  update
no_con_ent:
    cmp  ah,key_Left
    jne  no_con_left
    dec  [cursor]
    mov  [flick],2
    jmp  anti_flick ;red
no_con_left:
    cmp  ah,key_Right
    jne  no_con_right
    inc  [cursor]
    mov  [flick],2
    jmp  anti_flick ;red
no_con_right:
    cmp  ah,key_Bspace
    jne  no_con_bspace

    mov ecx,[cursor]
    cmp ecx,0
    je  still
    dec ecx
    mov  ebp,[cursor]
lobsp:
    mov  bl,byte [path+ebp]
    mov  byte [path+ebp-1],bl
    inc  ebp
    cmp  ebp,100
    jne  lobsp
    dec  [cursor]
    mov  [flick],2
    jmp  anti_flick ;red
no_con_bspace:

    mov  ecx,[cursor]
    dec  ecx
    mov  ebp,100 ;[cursor]
losx:
    mov  bl,byte [path+ebp]
    mov  byte [path+ebp+1],bl
    dec  ebp
    cmp  ebp,ecx ;100
    jne  losx

    mov  ebp, [cursor]
    cmp  ebp,100
    ja   still

    mov  byte [path+ebp],ah
    inc  dword [cursor]

    mov  [flick],2
    jmp  anti_flick


;----------------------------
;Test on mouse button
;-----------------------------

  button:                       ; button
    mov eax,17
    int 0x40

    cmp ah,2           ;Edit prompt line?
    je  edit_prompt

    cmp ah,4
    jne no_filelist

mousetest:
    mov eax,37
    mov ebx,1
    int 0x40
    mov ebx,eax
    shr eax,16       ;x
    and ebx,0xffff   ;y

    sub ebx,[listy] ;80
    mov [mousey],ebx

    mov ecx,[listx]
    cmp eax,ecx
    jl  still
    add ecx,[listxsize]
    cmp eax,ecx
    jg  still

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
    mov ebx,62
    mul ebx
    mov ebp,eax

    mov edi,paramtest ;clear param string
    mov ecx,256
    mov al,0
    rep stosb


    mov  esi,0
lll2:
    mov al,byte [path+esi]
    cmp al,byte 0 ;' '; '/'
    je  fis2
    cmp al,byte ' '; '/'
    je  fis2
    inc esi
    cmp esi,100
    jna lll2
    jmp fis2

fis2:
    mov edi,ebp
    cmp [convinfo+edi+26],dword 'FOL '
    jne openf
    mov [path+esi],byte '/'
    inc esi
    mov ebp,8

los:                              ;directory extension bug fix
    mov al,[convinfo+edi]
    cmp al,' '
    jz  skip_name_space
    mov [path+esi],al
    inc esi
  skip_name_space:
    inc edi
    dec ebp
    jnz los

    cmp byte [convinfo+edi],'.'
    jnz dir_name_end
    cmp byte [convinfo+edi+1],' '
    jz  dir_name_end
    mov ebp,4
  dir_ext_char:
    mov al,[convinfo+edi]
    cmp al,' '
    jz  dir_name_end
    mov [path+esi],al
    inc esi
    inc edi
    dec ebp
    jnz dir_ext_char
  dir_name_end:
    mov [filecursor],0

;los:
;    mov al,[convinfo+edi]
;    mov [path+esi],al
;    inc esi
;    inc edi
;    dec ebp
;    jnz los
;    mov [filecursor],0
;    cmp byte [dlg_type],'S'
;    je  savetest
;no_save:

    call read_directory
;;    call convertation
    call draw_window
;    jmp still

;Savetest
savetest:
    cmp byte [dlg_type],'S'
    je  savetest_yes
    jmp still
savetest_yes:
    mov ecx,100
savetestloop:
    cmp [path+ecx],byte 0
    je  next_byte
    cmp [path+ecx],byte 32
    je  next_byte
    cmp [path+ecx],byte '.'
    je  openf  ;it's file
;    cmp [path+ecx],byte '/'
;    je  no_save  ;it's dir
next_byte:
    dec ecx
    jnz savetestloop
    jmp still

;Open/Run file

openf:
    mov ecx,100
lopt:
    mov al,[path+ecx]
    mov [paramtest+ecx],al
    dec ecx
    jns lopt

;    mov ebp,100
;loow:
;    cmp [paramtest+ebp],byte '.'
;    je  file_set
;    dec ebp
;    jnz loow   ;is file set not file add

    cmp dword [focus],0 ;if prompt line with focus no add file name from frame
    jne file_set

    mov [paramtest+esi],byte '/'
    inc esi
    mov ebp,8+4
    mov edx,edi
losf:
    mov al,[convinfo+edi]
    mov [paramtest+esi],al
    inc esi
    inc edi
    dec ebp
    jnz losf
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
    mov esi,100
    int 0x40

    jmp exit

is_brow:

;    cmp [convinfo+edi+26],dword 'Fvol'
;    je  edit

    mov eax,dword [convinfo+edx+8]
    cmp eax,'.   '
    jne noexecute
    mov ebx,0
    jmp execute
noexecute:

    cmp eax,'.JPG'
    jne nojv
jpg_jpe:
    mov ebx,jpgview
    jmp run
nojv:
    cmp eax,'.JPE'
    je  jpg_jpe
    cmp eax,'.GIF'
    jne nojv1
    mov ebx,gifview
    jmp run
nojv1:
    cmp eax,'.WAV'
    jne nojv12
    mov ebx,ac97wav
    jmp run
nojv12:
    cmp eax,'.BMP'
    jne nobv
    mov ebx,bmpview
    jmp run
nobv:
; //// Willow
    cmp eax,'.PNG'
    jne nopngv
    mov ebx,pngview
    jmp run
nopngv:
; //// Willow
    cmp eax,'.ASM'
    je edit
    cmp eax,'.TXT'
    je edit
    cmp eax,'.INC'
    je edit
    cmp eax,'.DAT'
    je edit
    jmp still
edit:
    mov ebx,editor
    jmp run

execute:
    mov ecx,0 ;200
loexe:
    mov al,[paramtest+ecx]
;    cmp al,0
;    je setzr
;    cmp al,' '
;    je setzr
;    je  badl
    cmp al,'.'
    je setzr
;    je  badl
;    jmp okl
;badl:
;    mov al,0
okl:
    mov [open_path+ecx],al
    inc ecx
    cmp ecx,200
    jnae loexe

setzr:
;    add ecx,3
;    mov al,0
    mov [open_path+ecx],byte 0 ;al

    mov eax,58
    mov ebx,fileinfo_start
    int 0x40

    jmp still

run:
    mov ecx,paramtest
    mov eax,19
    int 0x40
    jmp still

no_filelist:

    cmp ah,5    ;OPEN/SAVE button
    je  kfad

    cmp ah,6    ;Scroll up
    jne no_scrlup
    mov ebx,1
    jmp up
no_scrlup:

    cmp ah,7    ;Scroll down
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
    mov  esi,0
xlll2:
    mov al,byte [path+esi]
    cmp al,byte '/'
    jne  xfis2
    inc esi
    cmp esi,12*20
    jnae xlll2
    jmp still
xfis2:
    mov al,byte [path+esi]
    cmp al,byte ' '
    je  xaa2
    inc esi
    cmp esi,12*20
    jnae xfis2
    jmp still
xaa2:
    mov eax,[filecursor]
    mov ebx,62
    mul ebx
    mov edi,eax
    cmp [convinfo+edi+26],dword 'FOL '
    je  still
    mov ecx,12*20
xlopt:
    mov al,[path+ecx]
    mov [paramtest+ecx],al
    dec ecx
    jns xlopt
    mov [paramtest+esi],byte '/'
    inc esi
    mov ebp,8+4
    mov edx,edi
xlosf:
    mov al,[convinfo+edi]
    mov [paramtest+esi],al
    inc esi
    inc edi
    dec ebp
    jnz xlosf
    mov [paramtest+esi],byte 0
    mov ebx,editor
    mov ecx,paramtest
    mov eax,19
    int 0x40
    jmp red ;still

no_textopen:

    cmp  ah,11
    jne  no_view
viewset:
;    not  dword [delflag]
    inc dword [viewmode]
    cmp dword [viewmode],8
    jnae not_cm
    mov [viewmode],0
not_cm:
    call read_directory
;    call convertation
    mov [filecursor],0
    call draw_window
    jmp still
no_view:

    cmp  ah,12        ;move back
    jne  no_back
back:
    mov  esi,12*20
lll:
    mov al,byte [path+esi]
    cmp al,byte ' '
    jne  findsl
    dec esi
    jnz lll
    jmp still
findsl:
    dec esi
fis:
    mov al,byte [path+esi]
    cmp al,byte '/'
    je  aa
    mov [path+esi],byte 0 ;' '
    dec esi
    jnz fis
aa:
    mov [path+esi],byte 0 ;' '

    mov [filecursor],0
    call read_directory
;    call convertation
    call draw_window
    jmp still

no_back:
    cmp  ah,13        ;string up?
    jne  no_up
    mov  ebx,1        ;step
up:
    mov  [downstop],0
    sub  [filecursor],ebx
    cmp  [filecursor],0
    jnl  cr_ok
    mov  [filecursor],0
cr_ok:
    jmp  draw_wd
no_up:
    cmp  ah,14       ;string down?
    jne  no_dn
    mov  ebx,1       ;step
down:
    cmp  [downstop],1
    je   no_dn
    add  [filecursor],ebx
    jmp  draw_wd
no_dn:

    cmp  ah,15
    jne  no_copyclip    ;copy to clipboard
copy_to_clip:
    mov  ebx,param_area ;clipfilp
    mov  eax,32
    int  0x40
    mov  esi,0
wlll2:
    mov al,byte [path+esi]
    cmp al,byte '/'
    jne wfis2
    inc esi
    cmp esi,12*20
    jnae wlll2
    jmp still
wfis2:
    mov al,byte [path+esi]
    cmp al,byte ' '
    je  waa2
    inc esi
    cmp esi,12*20
    jnae wfis2
    jmp still
waa2:
    mov eax,[filecursor]
    mov ebx,62
    mul ebx
    mov edi,eax
    cmp [convinfo+edi+26],dword 'FOL '
    je  still
    mov ecx,12*20
wlopt:
    mov al,[path+ecx]
    mov [paramtest+ecx],al
    dec ecx
    jns wlopt
    mov [paramtest+esi],byte '/'
    inc esi
    mov ebp,8+4
    mov edx,edi
wlosf:
    mov al,[convinfo+edi]
    mov [paramtest+esi],al
    inc esi
    inc edi
    dec ebp
    jnz wlosf
    mov [paramtest+esi],byte 0
    mov ebx,param_area ;clipfile
    mov ecx,paramtest
    mov edx,100
    mov esi,0
    mov eax,33
    int 0x40
    jmp still
no_copyclip:

    cmp ah,16
    jne no_clippaste
paste_from_clip:
    mov ebx,param_area ;clipfile
    mov ecx,0
    mov edx,-1
    mov esi,sourcepath
    mov eax,6
    int 0x40

    mov ecx,99
cdestp:
    mov al,[path+ecx]
    mov [destpath+ecx],al
    dec ecx
    jns cdestp

    mov esi,0
zlll2:
    mov al,byte [destpath+esi]
    cmp al,byte '/'
    jne zfis2
    inc esi
    cmp esi,100
    jnae zlll2
    jmp still
zfis2:
    mov al,byte [destpath+esi]
    cmp al,byte ' '
    je  zaa2
    inc esi
    cmp esi,100
    jnae zfis2
    jmp still
zaa2:
    mov byte [destpath+esi],'/'
    inc esi

    mov edi,0
qlll2:
    mov al,byte [sourcepath+edi]
    cmp al,byte '.'
    je  qfis2
    inc edi
    cmp edi,100
    jnae qlll2
    jmp still
qfis2:
    sub edi,8  ;.-8=start of file name

    mov ecx,11 ;11 sybols
cfname:
    mov al,[sourcepath+edi]
    cmp al,byte ' '
    je  dar
    mov [destpath+esi],al
    inc esi
dar:
    inc edi
    dec ecx
    jns cfname

;    mov [destpath+esi],byte 0

    mov ecx,199
cdestjp:
    mov al,[sourcepath+ecx]
    cmp al,byte 0
    jne nor
    mov al,byte 32
nor:
    mov [sourcepath+ecx],al
    dec ecx
    jns cdestjp

    cmp [browser],dword 1
    jne no_outpath
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,10*65536+67             ; [x start] *65536 + [y start]
    mov  ecx,0x00000000 ;[sc.grab_text] ; color of text RRGGBB
    mov  edx,sourcepath                  ; pointer to text beginning
    mov  esi,100 ;12*20             ; text length
    int  0x40
    mov  ebx,250*65536+67             ; [x start] *65536 + [y start]
    mov  ecx,0x00000000 ;[sc.grab_text] ; color of text RRGGBB
    mov  edx,destpath                  ; pointer to text beginning
    mov  esi,100 ;12*20             ; text length
    int  0x40
no_outpath:

    mov ebx,copyrfile
    mov ecx,sourcepath
    mov eax,19
    int 0x40
    delay 50   ;wait recoed file
    jmp update ;still
no_clippaste:

    cmp ah,19         ;Delete from floppy
delete_file:
    jne no_delt
    cmp dword [path],'/RD/'
    jne no_delt
    cmp byte [path+4],'1'
    jne no_delt

    mov eax,[filecursor]
    mov ebx,62
    mul ebx
    mov edi,eax
    add edi,convinfo
    mov ebp,edi
    mov eax,dword [edi]
    mov dword [paramtest],eax
    mov eax,dword [edi+4]
    mov dword [paramtest+4],eax
    mov eax,dword [edi+4+4+1]
    mov dword [paramtest+4+4],eax

    mov ebx,paramtest
    mov eax,32
    int 0x40
    jmp update
no_delt:

    cmp ah,20         ;I - Help
    je  help_scr

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
    je  copy_to_clip    ;Copy

    cmp ah,32
    je  paste_from_clip ;Paste

    cmp ah,33
    je  delete_file     ;Delte

    cmp ah,34
    je  textopen        ;Edit in Tinypad

    cmp ah,37
    je  exit

;VIEW MENU
    cmp ah,40         ;Sort by name show del
    jne no_sn
;    mov dword [viewmode],0
    and dword [viewmode],100b
    jmp update
no_sn:

    cmp ah,41         ;Sort by extension show del
    jne no_se
    and dword [viewmode],1101b
    or  dword [viewmode],0001b
    jmp update
no_se:

    cmp ah,42         ;Sort by size show del
    jne no_ss
    and dword [viewmode],1110b
    or  dword [viewmode],0010b
    jmp update
no_ss:

    cmp ah,43         ;Sort by date show del
    jne no_sd
    or  dword [viewmode],0011b
    jmp update
no_sd:

    cmp ah,44         ;Show del files
    jne no_ds
    or  dword [viewmode],0100b
    jmp update
no_ds:

    cmp ah,45         ;Fade del files
    jne no_df
    and dword [viewmode],11111011b
    jmp update
no_df:

;HELP MENU
    cmp ah,50         ;Help?
    je  help_scr

    cmp ah,51         ;Info?
    je  info_scr

    cmp ah,83
    ja  no_hd_part
    cmp ah,80
    jb  no_hd_part
    mov ecx,0
    sub ah,80
    inc ah
    mov cl,ah
    mov eax,21
    mov ebx,8 ;7
    int 0x40

no_hd_part:

    cmp ah,1                   ; test on exit button
    je  exit

    jmp still

exit:
; //// Willow
    mov eax,58
    mov ebx,MRUfile
    mov dword[ebx+8],255
    inc dword[ebx]
    int 0x40
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
    cmp [browser],dword 1 ;it's browser?
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

outlab:         ;out labels
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
    db '               INFO 81 Ver              '
    db '                                        '
    db '        Create by Pavlushin Evgeni      '
    db ' with ASCL libary special for Menuet OS '
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

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40

;Window

    xor  eax,eax                   ; function 0 : define and draw window

    cmp  [browser],dword 1 ;it's browser
    jne  nob1
    mov  ebx,140*65536+400         ; [x start] *65536 + [x size]
    mov  ecx,160*65536+280         ; [y start] *65536 + [y size]
    jmp  isb1
nob1:
    mov  ebx,140*65536+320         ; [x start] *65536 + [x size]
    mov  ecx,260*65536+240         ; [y start] *65536 + [y size]
isb1:
;    mov  edx,[sc.work]             ; color of work area RRGGBB
    or   edx,0x03ffffff;000000
    int  0x40

;Get proc info
    mov eax,9
    mov ebx,procinfo
    mov ecx,-1
    int 0x40

    mov  eax,[procinfo.x_size]
    cmp  eax,66
    jg  temp12345
    ret
 temp12345:

    cmp  [browser],dword 1 ;it's browser
    jne  nob9
    mov  [listx],120
    mov  eax,[procinfo.x_size]
    sub  eax,[listx]
    sub  eax,7
    mov  [listxsize],eax
    mov  [listy],73
    mov  eax,[procinfo.y_size]
    sub  eax,[listy]
    sub  eax,7
    mov  [listysize],eax
    jmp isb9
nob9:
    mov  [listx],10
    mov  eax,[procinfo.x_size]
    sub  eax,[listx]
    sub  eax,7
    mov  [listxsize],eax
    mov  [listy],54
    mov  eax,[procinfo.y_size]
    sub  eax,[listy]
    sub  eax,34
    mov  [listysize],eax
isb9:


;Draw only browser components
    cmp  [browser],dword 1 ;it's browser
    jne  nob2

    mov  eax,[sc.grab_text]        ; color of text RRGGBB
    or   eax,0x10000000
    label 8,8,'SYSTEM X-TREE FILE BROWSER',eax

;Draw buttons headers
    mov  eax,8
    mov  ebx,8*65536+(6*8-1) ;start pos x
    mov  ecx,23*65536+10      ;start pos y
    mov  edx,22;+1000000000000000000000000000000b  ;spoke butt
    mov  edi,3                ;draw 13 button's
    mov  esi,0x00339933
    int  0x40
    dec edi
nexthbut:
    add  ebx,(6*8)*65536
    inc  edx
    int  0x40
    dec  edi
    jnz  nexthbut

;DRAW PARTITION BUTTONS
    mov  eax,8
    mov  ebx,340*65536+5 ;start pos x
    mov  ecx,24*65536+8      ;start pos y
    mov  edx,80;+1000000000000000000000000000000b  ;spoke butt
    mov  edi,4                ;draw 13 button's
    mov  esi,0x00339933
    int  0x40
    dec edi
nextpbut:
    add  ebx,6*65536
    inc  edx
    int  0x40
    dec  edi
    jnz  nextpbut

;DRAW PARTITON TEXT
    label 341,25,'1234',cl_White;Black

;File STRING
    label 8,25,'  FILE    VIEW    INFO  ',  ;cl_White ;Black

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
    label 20,165,'SYSTEM X-TREE',cl_Black
    add  ebx,10
    label ,,'FOR  MENUETOS',

    add  ebx,9*65536+20
    label ,,'welcome to',cl_Green
    add  ebx,-15*65536+10
    label ,,'www.menuetos.org',cl_Green

;    label ,,'Create by',cl_Green
;    add  ebx,10
;    label ,,'   Pavlushin',
;    add  ebx,10
;    label ,,'       Evgeni',


;Draw head->file buttons
    cmp [drawhf],1
    jne  no_drawhf
    mov  ebx,8*65536+6*12 ;start pos x
    mov  ecx,35*65536+10      ;start pos y
    mov  edx,30  ;spoke butt
    mov  edi,8                ;draw 4 button's
    mov  esi,cl_Grey
    call menubutton
no_drawhf:

;Draw head->view buttons
    cmp [drawhv],1
    jne  no_drawhv
    mov  ebx,(8+6*8)*65536+6*12 ;start pos x
    mov  ecx,35*65536+10      ;start pos y
    mov  edx,40  ;spoke butt
    mov  edi,6                ;draw 4 button's
    mov  esi,cl_Grey
    call menubutton
no_drawhv:

;Draw head->info buttons
    cmp [drawhi],1
    jne  no_drawhi
    mov  ebx,(8+12*8)*65536+6*12 ;start pos x
    mov  ecx,35*65536+10      ;start pos y
    mov  edx,50  ;spoke butt
    mov  edi,2                ;draw 2 button's
    mov  esi,cl_Grey
    call menubutton
no_drawhi:

nob2:

;Draw buttons instruments
    mov  eax,8
    cmp  [browser],dword 1 ;it's browser
    jne  nob3
    mov  ebx,10*65536+16+5 ;start pos x
    mov  ecx,37*65536+15      ;start pos y
    jmp isb3
nob3:
    mov  ebx,16*65536+16+5 ;start pos x
    mov  ecx,29*65536+15      ;start pos y
isb3:
    mov  edx,8;+1000000000000000000000000000000b  ;spoke butt
    mov  edi,13                ;draw 13 button's
    mov  esi,cl_Grey
    int  0x40
    dec edi
nextbut:
    add  ebx,(16+6)*65536
    inc  edx
    int  0x40
    dec  edi
    jnz  nextbut


    cmp  [browser],dword 1 ;it's browser
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

    cmp  [browser],dword 1 ;it's browser
    jne  nob5

    mov [urlx],48
    mov [urly],55
    mov eax,[procinfo.x_size]
    sub eax,48+10
    mov [urlxsize],eax
    mov [urlysize],12

    label 20,57,"URL:",cl_Black

;Out view mode info
    mov eax,[viewmode]
    mov ebx,16
    mul ebx
    mov edx,eax
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

    cmp byte [dlg_type],'O'     ;if byte O - is Open dialog
    jne no_openh
    mov edx,head_dlg          ;draw in head OPEN FILE
    jmp out_laby
no_openh:
    cmp byte [dlg_type],'S'     ;if byte S - is Save dialog
    jne no_saveh
    mov edx,head_dlg+9        ;draw in head SAVE FILE
out_laby:
    mov ebx,8*65536+8
    mov ecx,[sc.grab_text]        ; color of text RRGGBB
    or  ecx,0x10000000
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

    cmp byte [dlg_type],'O'     ;if byte O - is Open dialog
    jne no_openb
    mov edx,but_dlg          ;draw in head OPEN FILE
    jmp out_labx
no_openb:
    cmp byte [dlg_type],'S'     ;if byte S - is Save dialog
    jne no_saveb
    mov edx,but_dlg+4        ;draw in head SAVE FILE
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
    mov eax,[urlxsize]  ;calculating text leight
    sub eax,8
    mov ebx,6
    div ebx
    mov esi,eax

    mov  ebx,[urlx]
    shl  ebx,16
    add  ebx,[urly]
    add  ebx,3*65536+2
    mov  eax,4                     ; function 4 : write text to window
    mov  ecx,0x00000000 ;[sc.grab_text] ; color of text RRGGBB
    mov  edx,path                  ; pointer to text beginning
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
    add ecx,[scrollbutsize]
    shl ecx,16
    add ecx,[listysize]
    sub ecx,[scrollbutsize]
    sub ecx,[scrollbutsize]
    mov edx,[scrollcolor] ;0x00006600
    int 0x40

;Draw Scroll Box
    mov  edx,0
    mov  eax,[listysize]
    sub  eax,2
    mov  ebx,dword 10
    div  ebx

    mov edx,0
    mov ebx,eax
    cmp ebx,[listsize]  ;filelistsize in ebx
    ja  notusescroll
;usescroll
    mov eax,[listysize]
    sub eax,[scrollbutsize]
    sub eax,[scrollbutsize]
    shl eax,16
    div dword [listsize]
    mul ebx
    shr eax,16
    mov esi,[mousey]
    shl esi,16
    add esi,eax

    mov eax,13
    mov ebx,[listx]
    add ebx,[listxsize]
    sub ebx,[scrollsize]
    shl ebx,16
    add ebx,dword [scrollsize]
    mov ecx,[listy]
    add ecx,[scrollbutsize]
    shl ecx,16
    add ecx,esi
    mov edx,[scrollboxcol]
    int 0x40
notusescroll:


;Draw list button for get file name
    mov  ebx,[listx]
    shl  ebx,16
    add  ebx,[listxsize]
    sub  ebx,15              ;right free zone
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
    add  ecx,[scrollbutsize]
    dec  ecx     ;correction
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


    dec  ecx     ;correction
    mov  edx,7+1000000000000000000000000000000b  ;spoke butt
    mov  eax,[listysize]
    sub  eax,[scrollbutsize]
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

    mov  eax,[listxsize]
    sub  eax,40*6  ;leight of string
    shr  eax,1
    add  eax,[listx]
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
    mov  eax,[filecursor]           ;calc cursor position
    mov  ebx,62
    mul  ebx

;OUT TEXT
    mov  ebp,4096 ; 16             ;out strings process
    sub  ebp,[filecursor]
    mov  edx,convinfo ;fileinfo+11
    add  edx,eax
    mov  ebx,dword [filelistxy]
loo:
    mov  ecx,0x00888888        ;for another file's color white
    cmp  [edx+26],dword 'FOL ' ;folder yellow
    jne  nb
    mov  ecx,0x00006666
    jmp cset1
nb:
    mov eax,[edx+8]
;Color set
    cmp  eax,dword '.TXT'  ;text's blue
    je  itx
    cmp  eax,dword '.INC'
    je  itx
    cmp  eax,dword '.ASM'
    je  itx
    jmp nt
itx:
    mov  ecx,0x00446666
    jmp cset
nt:
    cmp  eax,dword '.BMP'  ;picture's pure
    je  ipic
    cmp  eax,dword '.JPG'
    je  ipic
    cmp  eax,dword '.JPE'
    je  ipic
    cmp  eax,dword '.GIF'
    je  ipic
; //// Willow
    cmp  eax,dword '.PNG'
    je  ipic
; //// Willow
    cmp  eax,dword '.WAV'
    je  ipic
    jmp np
ipic:
    mov  ecx,0x00226688
    jmp cset
np:
    cmp  eax,dword '.   '  ;execute's green
    jne  nexec
    mov  ecx,0x00008866
    jmp cset
nexec:
cset:

cset1:
    mov  esi,40 ;symbols out 62 ;32
    mov  eax,4
    pushad
    int  0x40
;    popad

;    pushad
    cmp  [edx+26],dword 'Fvol' ;volume label
    jne  no_volico
    push hdico+8
    jmp out_ico
no_volico:
    cmp  [edx+26],dword 'FOL '
    jne  no_folico
    cmp  [edx+9],dword 'HARD'
    jne  no_hdico
    push hdico+8
    jmp out_ico
no_hdico:
    cmp  [edx+9],dword 'RAMD'
    jne  no_rdico
    push rdico+8
    jmp out_ico
no_rdico:
    push folico+8
    jmp out_ico
no_folico:
    cmp  [edx+8],dword '.BMP'
    je   is_imgico
    cmp  [edx+8],dword '.JPG'
    je   is_imgico
    cmp  [edx+8],dword '.JPE'
    je   is_imgico
    cmp  [edx+8],dword '.GIF'
    je   is_imgico
; //// Willow
    cmp  [edx+8],dword '.PNG'
    je   is_imgico
; //// Willow
    cmp  [edx+8],dword '.WAV'
    je   is_imgico
    jmp  no_imgico
is_imgico:
    push imgico+8
    jmp out_ico
no_imgico:
    cmp  [edx+8],dword '.ASM'
    je   is_asmincico
    cmp  [edx+8],dword '.INC'
    je   is_asmincico
    jmp  no_asmincico
is_asmincico:
    push asmincico+8
    jmp out_ico
no_asmincico:
    cmp  [edx+8],dword '.TXT'
    jne  no_txtico
    push txtico+8
    jmp out_ico
no_txtico:
    cmp  [edx+8],dword '.   '
    jne  no_execico
    push execico+8
    jmp out_ico
no_execico:
    cmp  [edx+26],dword 'DAT '
    jne  no_datico
    push datico+8
    jmp out_ico
no_datico:
    cmp  [edx+26],dword 'DEL '
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
    add  edx,62

    dec  [filelistsize]
    cmp  [filelistsize],dword 0
    je   extloo

    dec  ebp
    jnz  loo
dext:
    mov [downstop],1

extloo:

    cmp  [browser],dword 1 ;it's browser
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
    db   '    Open    '
    db   '    Copy    '
    db   '    Paste   '
    db   '   Delete   '
    db   '   Tinypad  '
    db   '    Edit    '
    db   '            '
    db   '    Exit    '

no_drawhftext:

;Draw text for head->view buttons
    cmp [drawhv],1
    jne  no_drawhvtext
    drawfbox (8+6*8),35,(6*12)+1,(11*6),0x00000000
    drawfbox (9+6*8),36,(6*12)-1,(11*6)-2,0x00ffffff

    mov edi,6 ;4
    mov ebx,(9+6*8)*65536+37
    mov ecx,cl_Black
    mov edx,view_text_label
    call drawmenu
    jmp no_drawhvtext

view_text_label:
    db   '  Name sort '
    db   '  Ext. sort '
    db   '  Size sort '
    db   '  Date sort '
    db   '  Show DEL  '
    db   '  Fade DEL  '

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
    db   '   Help     '
    db   '   About    '

no_drawhitext:

nob8:
    cmp  [flick],1
    jne  no_flick
    mov  [flick],0
    jmp  still
no_flick:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret

;FILE LIST PARAMETRS

listx     dd 15
listy     dd 72
listxsize dd 350
listysize dd 41

filelistxy    dd 0
filelistsize  dd 0
scrollsize    dd 8
listcolor     dd 0xffffff ;0xeeeeee
scrollcolor   dd 0x778877
scrollboxcol  dd 0x335533
scrollbutcol  dd 0x224422
scrollbutsize dd 9
usescroll     dd 1

;URL LINE PARAMETRS

urlx     dd 10
urly     dd 20
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

; STEP 0 SEt TYPE OF SORT

    mov eax,[viewmode]
;with no show del files
    and eax,0FFFFFFFBh
    cmp eax,0
    jnz no_sort_by_name
    mov [start],0
    mov [x0],12
    mov [x1],99
    mov [x2],99
    mov [x3],99
    jmp sortset

no_sort_by_name:
    dec eax
    jnz  no_sort_by_ext
    mov [start],9
    mov [x0],9
    mov [x1],99
    mov [x2],99
    mov [x3],12
    jmp sortset

no_sort_by_ext:
    dec eax
    jnz  no_sort_by_size
    mov [start],30
    mov [x0],12
    mov [x1],99
    mov [x2],99
    mov [x3],38
    jmp sortset

no_sort_by_size:
    dec eax
    mov [start],21
    mov [x0],12
    mov [x1],17
    mov [x2],20
    mov [x3],26
    jmp sortset  ;sort by date

;SORT VARILE
start dd 0
x0 dd 0
x1 dd 0
x2 dd 0
x3 dd 0

sortset:

;STEP 1 CLEAR CONVINFO
    mov  edi,convinfo
    mov  al,255
    mov  ecx,4096*62  ;512
    cld
    rep  stosb

;STEP 2 TEST ON HD OR PARTITION
    cmp [path],byte '/'
    je nstep
    mov ecx,61+62
loxhd:
    mov al,[hdimg+ecx]
    mov [convinfo+ecx],al
    dec ecx
    jns loxhd
    mov [listsize],0
    ret
nstep:
    cmp [path+3],byte '/'
    je nstep2
    mov ecx,61+62
loxpt:
    mov al,[ptimg+ecx]
    mov [convinfo+ecx],al
    dec ecx
    jns loxpt
    mov [listsize],0
    ret
nstep2:

;STEP 3 CLEAR OUTINFO
    mov  edi,outinfo ;0x14000 ;0x20000
    xor  eax,eax
    mov  ecx,4096*32  ;512
    cld
    rep  stosb

;STEP 4 READ DATA FROM HD
    mov  dword [farea],outinfo
    mov  dword [readblock],0

loorhd:
    mov  eax,[readblock]
    mov  [fileinfoblock+4],eax
    mov  eax,58
    mov  ebx,fileinfoblock
    int  0x40
    cmp  eax,0
    jne  hd_err
    add  dword [farea],512
    inc  dword [readblock]
    cmp  dword [readblock],4096/16
    jna  loorhd

hd_err:
    mov ebx,dword [readblock]
    shl ebx,4
    mov dword [blocksread],ebx ; for quick resorting

    cmp eax,5
    je  end_of_dir

;  It's erorr's test is poor code

    cmp eax,1
    jne no_inv_part
    label 10,10,'Invalid partition or hd base',cl_Red+font_Big
    jmp end_of_dir
no_inv_part:
    cmp eax,3
    jne no_unk_fs
    label 10,10,'Unknow file system',cl_Red+font_Big
    jmp end_of_dir
no_unk_fs:

end_of_dir:
    mov  [dirlen],ebx

    ; command succesful

    mov  esi,outinfo ;data_area+1024
;    mov  edi,fileinfo+11
    mov  edx,0 ;4096  ;16

  newlineb:

    mov  edi,fileinfo+11

    pushad               ; clear
    mov  al,32
    mov  ecx,58
    sub  edi,11
    cld
    rep  stosb
    popad

    mov  cl,[esi]       ; end of entries ?
    cmp  cl,6
    jnz  noib0

    mov  [edi+26],dword 'EOE '
    add  esi,32
;    add  edi,62
    jmp  inf

  noib0:

    mov  cl,[esi+0]
    cmp  cl,0xe5
    je   yesdelfil

    mov  cl,[esi+11]    ; long fat32 name ?
    cmp  cl,0xf
    jnz  noib1

    mov  [edi+26],dword 'F32 '
    add  esi,32
;    add  edi,62
    jmp  inf

  noib1:

    mov  eax,'DAT '     ; data or .. ?

    mov  cl,[esi+0]     ; deleted file
    cmp  cl,0xe5
    je   yesdelfil
    cmp  cl,0x0
    je   yesdelfil
    jmp  nodelfil
   yesdelfil:
    mov  eax,'DEL '
    jmp  ffile

  nodelfil:

    mov  cl,[esi+11]    ; folder
    and  cl,0x10
    jz   ffile
    mov  eax,'FOL '
    mov  [edi+26],eax
    jmp  nosize
  ffile:

; Asko patch for v79
    mov  cl,[esi+11]    ; fold
    and  cl,0xf
    cmp  cl,0xf         ; skip long filename
    jz   novol
    test cl,0x8         ; is it fold label?
    jz   novol         ; no
    mov  eax,'Fvol'
    mov  [edi+26],eax
    jmp  nosize
  novol:

    mov  [edi+26],eax

    pusha               ; size
    mov  eax,[esi+28]
    mov  esi,edi
    add  esi,37
    mov  ebx,10
    mov  ecx,8
  newnum:
    xor  edx,edx
    div  ebx
    add  dl,48
    mov  [esi],dl
    test eax,eax
    jz   zernum
    dec  esi
    loop newnum
  zernum:
    popa
  nosize:

    pusha                    ; date
    mov  [edi+17],dword '.  .'

    movzx eax,word [esi+24]
    shr  eax,9         ; year
    add  eax,1980
    mov  ecx,4
  newdel1:
    dec  ecx
    xor  edx,edx
    mov  ebx,10
    div  ebx
    add  edx,48
    mov  [edi+ecx+21],dl
    test ecx,ecx
    jnz  newdel1

    movzx eax,word [esi+24]
    shr  eax,5    ; month
    and  eax,0x0f
    mov  ecx,2
  newdel2:
    dec  ecx
    xor  edx,edx
    mov  ebx,10
    div  ebx
    add  edx,48
    mov  [edi+ecx+18],dl
    test ecx,ecx
    jnz  newdel2

    movzx eax,word [esi+24]
    and  eax,0x1f ; day
    mov  ecx,2
  newdel3:
    dec  ecx
    xor  edx,edx
    mov  ebx,10
    div  ebx
    add  edx,48
    mov  [edi+ecx+15],dl
    test ecx,ecx
    jnz  newdel3

    popa


    pusha                    ; number
    mov  eax,edx
    sub  eax,4096
    neg  eax

    xor  edx,edx
    mov  ebx,10
    div  ebx
    add  dl,48
    mov  [edi+43],dl          ;0001
    xor  edx,edx
    div  ebx
    add  dl,48
    mov  [edi+42],dl          ;0010
    xor  edx,edx
    div  ebx
    add  al,48
    add  dl,48
    mov  [edi+41],dl          ;0100
    mov  [edi+40],byte 0      ;1000
    popa

    mov  ecx,8          ; first 8
    cld
    rep  movsb
    mov  [edi],byte '.'
    inc  edi
    mov  ecx,3          ; last 3
    cld
    rep  movsb

    add  esi,(32-11)
;    add  edi,(60-12+2)

  inf:

    pushad

;STEP 5 Test on WRITE OR NOTWRITE
    mov  edx,fileinfo+11
looo:

;   Delete del, eoe, f32 and another head-names
    cmp  [viewmode],3  ;0-3 no outdel
    ja   del_out
    cmp  [edx+26],dword 'DEL '
    je   nextfl
del_out:
    cmp  [edx+26],dword 'DEL '
    jne  no_del
    cmp  [edx],dword 0 ;431 ;435 ;484 +10% speed
    je   nextfl
no_del:
    cmp  [edx+26],dword 'EOE '
    je   nextfl
    cmp  [edx+26],dword 'F32 '  ;F32 not useles
    je   nextfl
    cmp  [edx],dword '.   '
    je   nextfl
    cmp  [edx],dword '..  '
    je   nextfl
    cmp  [edx],dword 'FIRS'
    je   nextfl

; ---------_______-------_______ --------_________-----------
; SORT by name and del deletet files or f32 headers from list
; _________-------_______ --------_______---------___________

; STEP 6 UNIVERSAL SORT ALGORYTHM

xxx:
    mov esi,0 ;[tekfilename] ;0
    mov ebx,[start] ; 0

; At first Fold after Dat and Del

ftestname:
    cmp byte [fileinfo+11+26],'F'
    je  F
    cmp byte [fileinfo+11+26],'D'
    je  D
    jmp add_element
D:  cmp byte [convinfo+esi+26],'D'
    je  add_element
    cmp byte [convinfo+esi+26],'F'
    je  skipfile
    jmp add_element
F:  cmp byte [convinfo+esi+26],'D'
    je writenow
;    cmp byte [convinfo+esi+26],'F'
;    je  add_element
;    jmp add_element

add_element:
    mov al,[fileinfo+ebx+11]
    cmp al,[convinfo+esi+ebx]
    je  equal
    jb  writenow
skipfile:
    add esi,62
    mov ebx,[start]  ;0
    jmp ftestname

equal:
    inc ebx
    cmp ebx,[x0]
    je  writefile
    cmp ebx,[x1]
    je  x1p
    cmp ebx,[x2]
    je  x2p
    cmp ebx,[x3]
    jae x3p
    jmp add_element

x1p:
    mov ebx,18
    jmp add_element
x2p:
    mov ebx,15
    jmp add_element
x3p:
    mov ebx,0
    jmp add_element

writenow:
    mov ecx,4096*62
    sub ecx,esi
ldloop:
    mov al,[convinfo+ecx+esi]
    mov [convinfo+ecx+esi+62],al
    dec ecx
    jns ldloop


writefile:
    mov ecx,61
wfloop:
    mov al,[fileinfo+ecx+11]
    mov [convinfo+ecx+esi],al
    dec ecx
    jns wfloop

nextfile:

nextfl:
;    popad
;    pushad
    mov eax,edx
    shl eax,26
    cmp eax,0
    jne no_outcnt
    push edx
    drawfbox 294,25,(4*6),10,cl_White
    pop  ebp
    outcount ebp,294,25,cl_Black,4*65536
no_outcnt:
    popad

    inc edx
    cmp edx,4096
    jnae newlineb


;STEP 8 GET SIZE OF RESORTING LIST
    mov ecx,0
    mov edi,0
    mov eax,[blocksread]
    mov ebx,62
    mul ebx
    mov edx,eax
loogs:
    mov eax,dword [convinfo+edi+26]
    cmp eax,dword 0xffffffff
    je endgs
    add edi,62
    inc ecx
    cmp edi,edx ;4096*62
    jnae loogs
endgs:
    mov [listsize],ecx
    ret

;******************************************************************************

; DATA AREA
pmenu_draw dd 0  ;if poup menu is draw,update all window
flick      dd 0  ;anti flick on ?
drawhf     dd 0  ;draw file menu?
drawhv     dd 0  ;draw view menu?
drawhi     dd 0  ;draw info menu?
browser    dd 0  ;0-dialog, 1-browser
cursor     dd 0  ;cursor in prompt line
focus      dd 0  ;prompt edit or window?
viewmode   dd 0  ;folder sort & not del
downstop   dd 0
filecursor dd 0
mousex     dd 0
mousey     dd 0
blocksread dd 0
listsize   dd 0  ;num of files in directory
temp       dd 0
readblock  dd 1
dlg_type   db 0 ;S-save O-open

          ;01234567890123456789012345678901234567890123456789012345678912
hdimg  db 'HD       HARDDISK         FOL                                 '
       db 'RD       RAMDISK          FOL                                 '

ptimg  db '1        FIRST  PARTITION FOL                                 '
       db '2        SECOND PARTITION FOL                                 '

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

dirlen    dd   0x1
b_color   dd   0x6677cc

; //// Willow
MRUfile:
   dd 0x0
   dd 0x0
   dd 0x1
   dd path
   dd tempzone
   db '/RD/1/MRU.LST',0
; //// Willow

;Name of programs
editor    db 'TINYPAD    '
bmpview   db 'MV         '
jpgview   db 'JPEGVIEW   '
gifview   db 'GIFVIEW    '
ac97wav   db 'AC97WAV    '
copyrfile db 'COPYR      '
; //// Willow
pngview   db '@RCHER     '
; //// Willow

fileinfo_start:
dd 16
dd 0
dd 0 ;tempzone+1000;
dd 0
dd tempzone ;0x10000
open_path:
times 256 db 0  ;run app path

fileinfoblock:
   dd 0x0       ; read
   dd 0x0       ; first block
   dd 0x1       ; number of blocks to read
farea:
   dd outinfo   ; ret offset
   dd tempzone    ; work size of sytemram
path:
times 256 db 0  ;path
;rb 256
but_file:
file 'systr12.GIF'
butimg:
rb 400*16*3+8   ;buttons (left pice of picture)
logoimg:
rb 70*16*3+8    ;logo (right pice of picture)
logoinfimg:
rb 60*60*3+8    ;logoinfo (right pice of picture)
upsb:
rb 8*9*3+8      ;up scroll button
dnsb:
rb 8*9*3+8      ;down scroll button

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

tempimg:        ;reserve ram for images
rb 400*100*3+8   ;for picture
rb 8000

gif_hash:
rd 4096
tempzone:   ;temp zone for 58 function
rb 4000

sourcepath rb 100
destpath   rb 100

MYPID:
rd 8

I_END:

param_area:
rb 256
paramtest:
rb 256
filedir:
rb 256

procinfo process_information
sc system_colors

fileinfo:
rb 200 ;4096*62+1024

outinfo:
rb 4096*34+1024

convinfo:
rb 4096*62+1024

RAM_END:
