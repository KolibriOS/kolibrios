
;
;   Compile with flat assembler
;   Программка позволяющая проигрывать плейлист
;   Разработал:   ДедОк  :)

use32

    org    0x0

	    db	   'MENUET01'	    ; 8 byte id
	    dd	   0x01 	    ; header version
	    dd	   START	    ; start of code
	    dd	   I_END	    ; size of image
	    dd	   0x25000	     ; memory for app
	    dd	   0x20000	     ; esp
	    dd	   playlist,  0x0

include '../../../macros.inc'

START:			   ; start of execution
or   [trig_sys],40h
call scandr
jmp  mmm.seach
still:
    not   [trig_sys]
    or	 [trig_sys],400h
    not   [trig_sys]
    or	 [trig_sys],200h
    call enproc
    call delproc
    mov  eax,23 		 ; wait here for event
    mov  ebx,20
    mcall
    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,3			; button in buffer ?
    je	button
    test [trig_sys],200h
    jz	 next_song
    jmp  still
next_song:
    add  [poz_butt],1
    movzx eax,[poz_song]
    add  eax,1
    mov  [poz_song],al
    mov  dx,255
    mul  dx
    mov  [song_poz], eax
    movzx eax,[poz_song]
    sub  eax,1
    cmp  [song_count],eax
    ja	 .dgo
    mov  [poz_song],1
    mov  [song_poz],255
    mov  [poz_butt],6
.dgo:
    call  load_song
    call procskan
    call start
    or	 [trig_sys],200h
    call draw_window
    jmp  still
red:
    call draw_window
    jmp  still
button: 		      ; button
    mov   eax,17
    mcall
    cmp ah,1
    je	close
    cmp ah,2
    je	mmm
    cmp ah,3
    je	mma
    cmp ah,4
    je	mmd
    cmp ah,5
    je	mmf
    cmp ah,38
    jb	mma
    ret
close:
    mov  eax,-1
    mcall
mma:
    mov [poz_butt],ah
    mov [poz_song],ah
    sub [poz_song],5
    movzx eax,[poz_song]
    add  eax,[sme]
    mov  [poz_song],al
    mov  dx,255
    mul  dx
    mov  [song_poz], eax
    call  load_song
    call procskan
    call start
    or	 [trig_sys],200h
    call draw_window
    jmp still
mmw:
    mov  ebx,file_info2
    mov  ecx,[razm_str]
    add  dword [ebx+4],ecx
    or	 [trig_sys],40h
    jmp  mmm.seach
    ret
mmm:
.seach:
    call getstring
    call get_drive
    call put_db
    call draw_window
    test [trig_sys],04h
    jz	 mmw
    jmp  still
mmd:
    cmp  [poz_db3],510
    jb	 .emd
    sub  [poz_db3],255
    add  [poz_butt],1
    sub  [sme],1
.emd:
    call draw_window
    jmp  still
mmf:
    add  [poz_db3],255
    sub  [poz_butt],1
    add  [sme],1
    call draw_window
    jmp  still
procskan:
    mov  [num_proc],255
qqqqq:
    mov  eax,9
    mov  ebx,pib
    mov  ecx, [num_proc]
    mcall
    cld
    lea si,[pib.process_name]
    lea di,[minus]
    mov cx,7
    rep cmps byte [si],[di]
    jcxz mat
    sub  [num_proc],1
    cmp  [num_proc],1
    jne  qqqqq
    ret
mat:
    mov eax,18
    mov ebx,2
    mov ecx,[num_proc]
    mcall
    jmp  procskan
scandr:
    mov  eax,18
    mov  ebx,11
    mov  ecx,1
    mov  edx,scan_rez
    mcall
    ret
mountpl:
    mov  cx,255
    cld
    lea  di,[fl]
    mov  al,00h
    repne stos byte [di]
    cld
    mov  cx,6
    lea  si,[play_list]
    lea  di,[fl]
    rep movs byte [di],[si]
    mov  cx,0
    ret
get_drive:
    mov  al,byte[scan_rez+1]
    mov  [my_draw],al
    mov  [ns],48
    cmp  [my_draw],0
    jne  .c0
    or	 [trig_sys],01h
.c0:
    cmp  [my_draw],0x80
    jb	 .h0
    sub  [my_draw],0x80
    mov  [n_razd],1
    mov  [nd],49
    mov  al,[cd_0]
    mov  [td],al
    call  folscan
.h0:
    cmp  [trig_scan],1
    je	 ex1
    cmp  [my_draw],0x40
    jb	 .c1
    sub  [my_draw],0x40
    mov  al,byte[scan_rez+2]
    mov  [n_razd],al
    add  al,48
    mov  [nd],al
    mov  al,[hd_0]
    mov  [td],al
    call  folscan
.c1:
    cmp  [trig_scan],1
    je	 ex1
    cmp  [my_draw],0x20
    jb	 .h1
    sub  [my_draw],0x20
    mov  [n_razd],1
    mov  [nd],49
    mov  al,[cd_0]
    mov  [ns],19
    call  folscan
.h1:
    cmp  [trig_scan],1
    je	 ex1
    cmp  [my_draw],0x10
    jb	 .c2
    sub  [my_draw],0x10
    mov  al,byte[scan_rez+3]
    mov  [n_razd],al
    add  al,48
    mov  [nd],al
    mov  al,[hd_0]
    mov  [td],al
    mov  [ns],49
    call  folscan
.c2:
    cmp  [trig_scan],1
    je	 ex1
    cmp  [my_draw],0x08
    jb	 .h2
    sub  [my_draw],0x08
    mov  [n_razd],1
    mov  [nd],49
    mov  al,[cd_0]
    mov  [td],al
    mov  [ns],50
    call  folscan
.h2:
    cmp  [trig_scan],1
    je	 ex1
    cmp  [my_draw],0x04
    jb	 .c3
    sub  [my_draw],0x04
    mov  al,byte[scan_rez+4]
    mov  [n_razd],al
    add  al,48
    mov  [nd],al
    mov  al,[hd_0]
    mov  [td],al
    mov  [ns],50
    call folscan
.c3:
    cmp  [trig_scan],1
     je  ex1
    cmp  [my_draw],0x02
    jb	 .h3
    sub  [my_draw],0x02
    mov  [n_razd],1
    mov  [nd],49
    mov  al,[cd_0]
    mov  [td],al
    mov  [ns],51
    call folscan
.h3:
    cmp  [trig_scan],1
    je	 ex1
    cmp  [my_draw],0x01
    jb	 ex1
    sub  [my_draw],0x01
    mov  al,byte[scan_rez+5]
    mov  [n_razd],al
    add  al,48
    mov  [nd],al
    mov  al,[hd_0]
    mov  [td],al
    mov  [ns],51
    call folscan
ex1:
    cmp  [fold],05h
    jne  ext2
    or	 [trig_sys],04h
ext2:
    cmp  [fold],0Ah
    jne  ext3
    or	 [trig_sys],08h
    jmp  eext
ext3:
    mov  [trig_scan],0
    not  [trig_sys]
    or	 [trig_sys],0Eh
    not  [trig_sys]
eext:
    ret
folscan:
    cmp  [trig_scan],1
    je	 .out2
    mov  eax,70
    mov  ebx,file_info
    mcall
    mov  [fold],eax
    cmp  [fold],0
    jne  .out
    mov  al,1
    mov  [trig_scan],al
    jmp  .out2
.out:
     mov  al,1
    sub  [n_razd],al
    mov  al,1
    sub  [nd],al
    cmp  [n_razd],0
    jne  folscan
.out2:
    ret
get_play:
    mov  cx,255
    lea  si,[floc]
    lea  di,[playlist]
    rep movs byte [di],[si]
    mov  cx,0
ret
getstring:
    mov  eax,70
    mov  ebx,file_info2
    mov  dword [ebx+12], 255
    mov  dword [ebx+16],string1
    mov  dword [ebx+21],playlist
    mcall
    cmp eax,6
    jne .pro
    or	 [trig_sys],04h
    call draw_window
    jmp  still
.pro:

    cmp  ebx,0xFFFFFFFF
    je	.exit
    call rus_m
    cld
    lea  di,[string1]
    mov  cx,255
    mov  al,0ah
    repne  scas byte  [es:di]
    je	 .calc
    mov [razm_str],0
    mov  ebx,file_info2
    add  dword [ebx+4], 255
    or	 [trig_sys],40h
    jmp mmm.seach
.calc:
     mov  [razm_str],255
     sub  [razm_str],ecx
.kon:
    mov  [razm_path],0
    cld
    lea  di,[string1]
    mov  ecx,0
.ppr:
    cmp byte [es:di],70h
    je	.s1
    cmp byte [es:di],50h
    je	.s1
    add  ecx,1
    add  di,1
    cmp  ecx,[razm_str]
    jb	.ppr
    jmp .next
.s1:
    add  di,1
    cmp  byte [es:di],33h
    je	 .s2
    sub  di,1
    jmp .next
    ret
.s2:

    or	 [trig_sys],80h
    mov  [razm_path],ecx
    add  [razm_path],2

;    mov  [ss2],0
    cld
    lea  di,[string1]
    mov  cx,0
    mov  [zzz],cx
.ppre:
    cmp byte [es:di],3Ah
    je	.se1
    add  cx,1
    add  di,1
    cmp  ecx,[razm_path]
    jne  .ppre
    jmp  .mount
.se1:
    add  di,1
    cmp  byte [es:di],5Ch
    je	 .se2
    sub  di,1
    jmp  .mount
.se2:
    cmp  ecx,250
    jb	 .se3
    mov  ecx,0
.se3:
    add  cx,2
;    mov  [ss2],cx
    mov  [zzz],cx
.mount:
    movzx  ecx,[zzz]
    cmp  ecx, [razm_path]
    jb	.mount1
    or	 [trig_sys],04h
    jmp  still
.mount1:
    mov  ecx,255
    cld
    lea  di,[fl]
    mov  al,00h
    repne stos byte [di]
    cld
    mov  ecx,[razm_path]
    sub  cx,[zzz]
    lea  si,[string1]
    add  si,[zzz]
    lea  di,[fl]
    rep movs byte [di],[si]
    mov  ecx,0
.sl_m:
    mov  ecx,0
    lea  di,[fl]
.cikl_mx:
    mov  al,byte[di]
    mov  [char_con],al
    cmp  al,5Ch
    je	 .con_sl
    jmp  .ext6
.con_sl:
    mov al,2Fh
.ext6:
    stos byte [di]
    add  ecx,1
    cmp  ecx,[razm_path]
    jb	.cikl_mx

.exit:
    ret
.next:
    not  [trig_sys]
    or	 [trig_sys],80h
    not  [trig_sys]
    mov  ebx,file_info2
    mov  ecx,[razm_str]
    add  dword [ebx+4],ecx
    or	 [trig_sys],40h
    jmp mmm.seach
rus_m:
    test  [trig_sys],40h
    jnz   ddff
    ret
ddff:
    mov  ecx,0
    lea  di,[string1]
cicl_m:
    mov  al,byte[di]
    mov  [char_con],al
    shr  al,4
    cmp  al,0Eh
    je	 min_sor
    cmp  al,0Ch
    je	 min_sor
    cmp  al,0Dh
    je	 min_sor
    cmp  al,0Fh
    je	 min_des
    shl  al,4
    mov  al,0
    jmp  ext5
min_sor:
    sub  [char_con],40h
    jmp  ext5
min_des:
     sub  [char_con],10h
    jmp  ext5
ext5:
    mov  al,[char_con]
    stos byte [di]
    add  ecx,1
    cmp  ecx,255
    jb	cicl_m
    not  [trig_sys]
    or	 [trig_sys],40h
    not  [trig_sys]
    ret

put_db:
    test [trig_sys],80h
    jz	.exit
    add  [song_count],1
    add  [poz_db],255
    mov  ecx,255
    cld
    mov  ebx,[poz_db]
    lea  di,[memlist+ebx]
    mov  al,00h
    repne stos byte [di]
    cld
    mov  ecx,255
    lea  si,[floc]
    mov  ebx,[poz_db]
    lea  di,[memlist+ebx]
    rep movs byte [di],[si]
    mov  ecx,0
.exit:
    ret
load_song:
    cld
    mov  ecx,255
    lea  di,[song_path]
    mov  ebx,[song_poz]
    lea  si,[memlist+ebx]
    rep movs byte [di],[si]
    mov  ecx,0
    ret
start:
    mov  eax,70
    mov  ebx,folder_inf
    mcall
    ret
delproc:
    test [trig_sys],400h
    jnz   .sdf
    ret
.sdf:
       mov  [num_proc],255
.fgf:
    mov  eax,9
    mov  ebx,pib
    mov  ecx, [num_proc]
    mcall
    cld
    lea si,[pib.process_name]
    lea di,[minus]
    mov cx,7
    rep cmps byte [si],[di]
    jcxz .qwer
    sub  [num_proc],1
    cmp  [num_proc],1
    jne  .fgf
     test [trig_sys],200h
    ret
.qwer:
    mov  eax,[pib+process_information.cpu_usage]
    mov  [q_takt],eax
    cmp  [q_takt],200000
    jb	 .asdf
    ret
.asdf:
    add   [count_err],1
    cmp   [count_err],6
    ja	  .djbn
    ret
.djbn:
    mov   [count_err],0
    not   [trig_sys]
    or	 [trig_sys],200h
    not   [trig_sys]
    ret
enproc:
    mov  [num_proc],255
.fgf:
    mov  eax,9
    mov  ebx,pib
    mov  ecx, [num_proc]
    mcall
    cld
    lea si,[pib.process_name]
    lea di,[minus]
    mov cx,7
    rep cmps byte [si],[di]
    jcxz .qwer
    sub  [num_proc],1
    cmp  [num_proc],1
    jne  .fgf
    ret
.qwer:
    or	 [trig_sys],400h
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:
    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    mcall
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,1			   ; 1, start of draw
    mcall
				   ; DRAW WINDOW
    xor  eax,eax	    ; function 0 : define and draw window
    mov  ebx,100*65536+550	   ; [x start] *65536 + [x size]
    mov  ecx,100*65536+420	    ; [y start] *65536 + [y size]
    mov  edx,[sc.work]		   ; color of work area RRGGBB,8->color gl
    or	 edx,0x33000000
    mov  edi,title
    mcall
    mov  eax,4
    mov  ebx,5*65536+5
    mov  ecx,0xC0000000
    mov  edx,label1
    mov  edi,[sc.work]
    mcall
    mov  eax,4
    mov  ebx,300*65536+5
    mov  ecx,0xC0000000
    mov  edx,label5
    mov  edi,[sc.work]
    mcall
    mov  eax,4
    mov  ebx,300*65536+15
    mov  ecx,0xC0000000
    mov  edx,label6
    mov  edi,[sc.work]
    mcall
    mov  eax,4
    mov  ebx,25*65536+35
    mov  ecx,0xC00000dd
    mov  edx,song_path
    mov  edi,[sc.work]
    mcall
    mov  eax,4
    mov  ebx,100*65536+5
    mov  ecx,0xC0000000
    mov  edx,playlist
    mov  edi,[sc.work]
    mcall
    test [trig_sys],4h
    jnz   .zam1
    mov  eax,4
    mov  ebx,5*65536+15
    mov  ecx,0xC0000000
    mov  edx,label3
    mov  edi,[sc.work]
    mcall
    jmp  .zam2
.zam1:
    mov  eax,4
    mov  ebx,5*65536+15
    mov  ecx,0xC0000000
    mov  edx,label2
    mov  edi,[sc.work]
    mcall
    mov  eax,4
    mov  ebx,5*65536+25
    mov  ecx,0xC0000000
    mov  edx,label4
    mov  edi,[sc.work]
    mcall
.zam2:

;    mov  eax,47
;    mov  esi,0x50000000
;    mov  ecx, [trig_sys]
;    mov  edi,[sc.work]
;    mov  ebx,0x00080100
;    mov  edx,385*65536+25
;    mcall
    mov  eax,47
    mov  esi,0x50000000
    mov  ecx, [song_count]
    mov  edi,[sc.work]
    mov  ebx,0x00030000
    mov  edx,500*65536+5
    mcall
    mov  eax,47
    mov  esi,0x50000000
    movzx  ecx, [poz_song]
    mov  edi,[sc.work]
    mov  ebx,0x00030000
    mov  edx,500*65536+15
    mcall
    mov  [bbut],32
    mov  [poz_but],48*65536+10
    mov  edx,6
butt1:
    mov  ebx,0*65536+20
    mov  ecx,[poz_but]
    mov  esi,[color1]
    movzx  eax,[poz_butt]
    cmp  edx,eax
    jne  .uuu
    mov  esi,[color2]
.uuu:
    mov  eax,8
    mcall
    add  edx,1
    add  [poz_but],10*65536
    sub  [bbut],1
    cmp  [bbut],0
    jne   butt1
    mov  eax,8
    mov  ebx,525*65536+15
    mov  ecx,45*65536+15
    mov  esi,0x00aaaadd
    mov  edx,4
    mcall
    mov  eax,8
    mov  ebx,525*65536+15
    mov  ecx,365*65536+15
    mov  esi,0x00aaaadd
    mov  edx,5
    mcall
    mov  [ai],32
    mov  [list_poz],25*65536+50
    mov  ebx,[poz_db3]
    mov  [poz_db2],ebx
list:
    mov  eax,4
    mov  ecx,0xC0000000
    mov  ebx,[poz_db2]
    lea  edx,[memlist+ebx]
    mov  ebx, [list_poz]
    mov  edi,[sc.work]
    mcall
    add  [list_poz],10
    add  [poz_db2],255
    sub  [ai],1
    cmp  [ai],0
    ja	list
    mov  eax,12 		   ; function 12:tell os about windowdraw
    mov  ebx,2			   ; 2, end of draw
    mcall
    ret



; DATA AREA


title:         db   'Проигрываем плейлист v 0.19 :)',0
play_list:     db   '1.kpl',0
cd_0:	       db   'c',0
hd_0:	       db   'h',0
label1:        db   'Файл плейлиста:',0
label2:        db   'сканирование закончено... :)            ',0
label3:        db   'Подождите, пожалуйста, идёт сканирование',0
label4:        db   'Воспроизводимый файл:',0
label5:        db   'Всего файлов в плейлисте найдено:',0
label6:        db   'Номер воспроизводимого файла:',0
minus:	       db   'ac97snd',0
player:        db   '/rd/1/ac97snd' ,0
my_draw        db   0
n_razd	       db   0
trig_scan      db   0
char_con       db   0
poz_butt       db   0
poz_song       db   0
count_err      dd   0
num_proc       dd   0
q_takt	       dd   0
razm_str       dd   0
fold	       dd   0
razm_path      dd   0
trig_sys       dd   0
poz_db	       dd   0
poz_db2        dd   0
poz_db3        dd   255
song_poz       dd   0
list_poz       dd   5*65536+50
song_count     dd   0
ai	       dd   0
st_r	       dd   0
bbut	       dd   0
poz_but        dd   0
sme	       dd   0
color1	       dd   0x006666dd
color2	       dd   0x00dddddd
ss2	       dd   0
zzz	       dw   22
scan_rez:
	       db   0
	       db   0
	       db   0
	       db   0
	       db   0
	       db   0
	       db   0
	       db   0
	       db   0
	       db   0
floc:
pc:	       db   '/'
td:	       db   'h'
dr:	       db   'd'
ns	       db   48
vc:	       db   '/'
nd	       db   48
ks:	       db   '/'
fl:
	       rb   255
playlist       rb   255
song_path      rb   255
folder_inf:
	       dd   7
	       dd   0
	       dd   song_path
	       dd   0
	       dd   0
	       db   0
	       dd   player
file_info:
	       dd   0
	       dd   0
	       dd   0
	       dd   0
	       dd   0
	       db   0
	       dd   floc
file_info2:
	       dd   0
	       dd   0
	       dd   0
	       dd   0
	       dd   0
	       db   0
	       dd   0
string1        rb   255

I_END:



temp dd ?
sc system_colors
pib process_information
memlist        rb   65536

