; CMD - Command line interpreter
; copyleft Chemist dmitry_gt@tut.by
;
; Compile with FASM for Menuet
;
;

use32

 org 0x0

 db 'MENUET01'
 dd 0x01
 dd START
 dd I_END
 dd 0x300000
 dd 0x7fff0
 dd 0x0
 dd 0x0

include "macros.inc"
include "lang.inc"
START:

 call cmdexist
 call draw
 call fill
 call clearcmd
 call autoexec
 call ppr
 call cursor

still:

 mov eax,10
 int 0x40

 cmp eax,1
 je re
 cmp eax,2
 je key
 cmp eax,3
 je button

 jmp still

button:
 mov eax,17
 int 0x40

 cmp ah,1
 jne noclose

 jmp exit

noclose:
 jmp still

re:
 call draw
 call red
 jmp still

red:
 call cls1

 mov dword [xpos],24

 mov dword [linev],2000

 mov eax,dword [ypos]
 push eax

 mov dword [ypos],6

 mov ecx,dword [lpress]
loop1:
 push ecx
 mov eax,4
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov edx,tic_table
 add edx,dword [linev]
 mov esi,81
 int 0x40
 add dword [xpos],10
 add dword [linev],81
 pop ecx
 loop loop1

 sub dword [xpos],10

no_line:
 pop eax
 mov dword [ypos],eax

no_red:
 ret

key:
 mov eax,2
 int 0x40

 cmp ah,27
 jne no_escape

 mov dword [ypos],42
 call clearstr
 call ppr
 mov dword [count],0
 call cursor
 jmp still

no_escape:
 cmp ah,178
 jne no_aup
 cmp dword [count],0
 jne no_aup
 cmp dword [oldcount],0
 je no_aup

 call clearsum
 sub dword [ypos],6

 cld
 mov ecx,81
 mov edi,tic_table+600
 mov esi,tic_table+900
 rep movsb

 cld
 mov ecx,dword [oldcount]
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+600
 rep movsb

 call red

 mov eax,dword [oldcount]
 mov dword [count],eax

 add dword [linen],eax
 add dword [linel],eax

 mov ebx,6
 imul ebx,eax

 add dword [ypos],ebx

 call cursor

 jmp still

no_aup:
 cmp ah,8
 jne no_backspace
 cmp dword [count],0
 je still

 cmp dword [count],0
 je no_backspace

 call clearsum
 sub dword [ypos],12
 call clearsum
 sub dword [ypos],6
 call cursor
 dec dword [count]
 jmp still

no_backspace:
 cmp ah,13
 jne no_enter

 cmp dword [count],0
 jne enter_ok
 call clearsum
 jmp ecmd2

enter_ok:
 call checkcmd

 cmp byte [tic_table+600],'/'
 jne no_script
 cmp dword [count],1
 je nparam5

 jmp command

no_script:

 call oldcmd

 call checkprg

 jmp ecmd2

no_enter:

 cmp ah,176    ; Arrow keys, HOME, END, and DEL are unsupported now
 je still
 cmp ah,179
 je still
 cmp ah,177
 je still
 cmp ah,178
 je still
 cmp ah,182
 je still
 cmp ah,180
 je still
 cmp ah,181
 je still

 cmp dword [count],74
 je still

 mov byte [smb],ah

 mov edi,tic_table+600
 add edi,dword [count]
 mov esi,smb
 movsb

 inc dword [count]

 call clearsum
 sub dword [ypos],6
 call print
 add dword [ypos],6
 call cursor

 jmp still

clearstr:
 mov dword [ypos],6
 mov dword [clr],480
 call clear

 mov eax,dword [linel]
 sub dword [linen],eax
 mov dword [linel],0

 mov ecx,eax
 push eax
loop4:
 push ecx
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,fill1
 movsb

 inc dword [linen]
 pop ecx
 loop loop4

 pop eax
 sub dword [linen],eax

 mov dword [ypos],42

 ret

clearsum:
 mov dword [clr],6
 call clear

 dec dword [linen]
 dec dword [linel]

 mov edi,tic_table
 add edi,dword [linen]
 mov esi,fill1
 movsb

 ret

clear:
 mov eax,13
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[clr]
 mov ecx,[xpos]
 shl ecx,16
 add ecx,9
 mov edx,0
 int 0x40
 add dword [ypos],6
 ret

draw:

 mov eax,12
 mov ebx,1
 int 0x40

 mov eax,0
 mov ebx,100*65536+492
 mov ecx,100*65536+280
 mov edx,0x3000000
 mov esi,0x805080d0
 mov edi,0x005080d0
 int 0x40

 mov eax,4
 mov ebx,8*65536+8
 mov ecx,0x10ddeeff
 mov edx,title
 mov esi,title_end-title
 int 0x40

; mov eax,8
; mov ebx,(492-19)*65536+12
; mov ecx,5*65536+12
; mov edx,1
; mov esi,0x6688dd
; int 0x40

 mov eax,12
 mov ebx,2
 int 0x40

 ret

print:
 mov eax,4
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov edx,smb
 mov esi,1
 int 0x40

 mov edi,tic_table
 add edi,dword [linen]
 mov esi,smb
 movsb
 inc dword [linen]
 inc dword [linel]

 ret

cursor:
 mov eax,4
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov edx,smb_cursor
 mov esi,1
 int 0x40

 mov edi,tic_table
 mov esi,smb_cursor
 add edi,dword [linen]
 movsb
 inc dword [linen]
 inc dword [linel]

 ret

ppr:
 mov eax,4
 mov ebx,6
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov edx,prompt
 mov esi,5
 int 0x40
 mov dword [ypos],42

 cld
 mov ecx,5
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,prompt
 rep movsb

 add dword [linen],6
 add dword [linel],6

 ret

help:
 cmp byte [callp],1
 je gonext8
 call clearsum
gonext8:
 call newline
 call newline
 mov edx,h1
 call printf
 call newline
 mov edx,h2
 call printf
 call newline
 call newline
 mov edx,h3
 call printf
 call newline
 call newline
 mov edx,h4
 call printf
 call newline
 mov edx,h5
 call printf
 call newline
 mov edx,h6
 call printf
 call newline
 mov edx,h7
 call printf
 call newline
 call newline
 mov edx,h8
 call printf
 call newline
 mov edx,h9
 call printf
 call newline
 mov edx,h10
 call printf
 call newline
 mov edx,h11
 call printf
 call newline
 mov edx,h12
 call printf
 call newline
 mov edx,h13
 call printf
 call newline
 call newline
 mov edx,h14
 call printf
 call newline
 call newline
 mov edx,h15
 call printf
 call newline
 mov edx,h16
 call printf
 call newline

 cmp byte [callp],1
 je go

 jmp ecmd

ver:
 cmp byte [callp],1
 je gonext7
 call clearsum
gonext7:
 call newline
 mov edx,about
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

pause1:
 cmp byte [callp],1
 je gonext1
 call clearsum
gonext1:
 call pause2

 cmp byte [callp],1
 je go

 jmp ecmd

ls:
 call oldcmd
 call cls2

loopls:
 inc dword [blockcnt]

 mov eax,34
 mov ebx,0
 mov ecx,0
 mov edx,0
 add edx,dword [blockcnt]
 mov esi,1
 mov edi,tic_table+7000
 int 0x40

 mov ecx,16
loop40:
 push ecx

 cld
 mov ecx,8
 mov edi,filename
 mov esi,tic_table
 add esi,dword [lscnt]
 rep movsb

 add dword [lscnt],8

 mov edi,filename+8
 mov esi,ddot
 movsb

 cld
 mov ecx,3
 mov edi,filename+9
 mov esi,tic_table
 add esi,dword [lscnt]
 rep movsb

 cmp byte [filename+10],0
 jne no_fn_space1

 mov edi,filename+10
 mov esi,dzero
 movsb

no_fn_space1:
 cmp byte [filename],0xe5  ; deleted file
 je no_newline
 cmp byte [filename],0xf   ; long fat32 filename
 je no_newline
 cmp byte [filename],0x10  ; folder
 je no_newline

 cmp word [filename],'AK'
 jne filename_ok
 cmp byte [filename+3],'e'
 jne filename_ok
 cmp byte [filename+5],'y'
 jne filename_ok
 cmp byte [filename+7],'a'
 jne filename_ok
 cmp byte [filename+10],'s'
 jne filename_ok

 jmp no_newline

filename_ok:
 mov eax,6
 mov ebx,filename
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 jne ls_print

 jmp no_newline

ls_print_done:
 inc byte [lscntf]

 add dword [ypos],96

 cmp byte [lscntf],5
 jne no_newline

 mov byte [lscntf],0
 inc byte [lscntx]

 cmp byte [lscntx],23
 je pause2n

 mov dword [ypos],6
 call newline

no_newline:
 add dword [lscnt],24

 pop ecx
 dec ecx
 cmp ecx,0
 jne loop40

 cmp dword [blockcnt],16
 je ls_end

 jmp no_pause2

pause2n:
 mov byte [lscntx],0

 call newline
 call pause2
 call cls2
 mov dword [lscnt],8024
 jmp loopls

no_pause2:
 mov dword [lscnt],8024

 jmp loopls

ls_end:
 mov dword [blockcnt],0
 mov dword [lscnt],8024
 mov byte [lscntf],0
 mov byte [lscntx],0

 cmp byte [callp],1
 je go

 jmp ecmd2

ls_print:
 mov edi,filename+8
 mov esi,fill1
 movsb

 mov eax,4
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[xpos]
 mov edx,filename
 mov ecx,0x00ddeeff
 mov esi,12
 int 0x40

 cld
 mov ecx,12
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,filename
 rep movsb

 add dword [linen],16
 add dword [linel],16

 jmp ls_print_done

lscheck:
 cmp byte [callp],1
 je gonext9
 call clearsum
gonext9:
 sub dword [count],3

 cld
 mov ecx,dword [count]
 mov edi,tic_table+400
 mov esi,tic_table+600
 add esi,3
 rep movsb

 mov ebx,tic_table+400
 push esi edi ecx eax
 mov esi,ebx

 mov edi,tic_table+400
 mov ecx,70
strup2:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup2
 pop eax ecx edi esi

 mov ecx,dword [count]

looplsc:
 cmp byte [tic_table+400+ecx],'.'
 je chdot

 loop looplsc

 jmp chnodot

chdot:
 mov ebx,dword [count]

 cld
 mov ecx,5
 mov edi,tic_table+400
 add edi,ebx
 mov esi,dzero
 rep movsb

 mov eax,6
 mov ebx,tic_table+400
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 je nosuchfile

 jmp lschok

chnodot:
 mov ebx,dword [count]

 mov edi,tic_table+400
 add edi,ebx
 mov esi,ddot
 movsb

 inc ebx

 cld
 mov ecx,3
 mov edi,tic_table+400
 add edi,ebx
 mov esi,fill3
 rep movsb

 mov eax,6
 mov ebx,tic_table+400
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 je nosuchfile

 mov edi,tic_table+400
 add edi,dword [count]
 mov esi,fill1
 movsb

lschok:
 call newline

 mov eax,4
 mov ebx,6
 shl ebx,16
 add ebx,[xpos]
 mov edx,tic_table+400
 mov esi,12
 mov ecx,0x00ddeeff
 int 0x40

 cld
 mov ecx,12
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+400
 rep movsb

 add dword [linen],12
 add dword [linel],12

 add dword [count],3

 cmp byte [callp],1
 je go

 jmp ecmd

ps:
 call oldcmd
 call cls2
 call newline
 mov edx,proc_head
 call printf
 call newline
 mov edx,proc_hd11
 call printf
 call newline
 call newline

 mov eax,9
 mov ebx,tic_table
 mov ecx,1
 int 0x40

 mov dword [count2],eax

ll1:
 inc dword [pn]
 mov eax,9
 mov ebx,tic_table
 mov ecx,[pn]
 int 0x40

 mov ebx,[tic_table+30]
 mov dword [fnumb],4
 mov dword [ypos],6
 call decnumb

 cld
 mov ecx,4
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+200
 rep movsb

 add dword [linen],5
 add dword [linel],5

 cld
 mov ecx,11
 mov esi,tic_table+10
 mov edi,pname
 rep movsb

 mov dword [ypos],36
 mov edx,pname
 mov eax,4
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov esi,12
 int 0x40

 cld
 mov ecx,11
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,pname
 rep movsb

 add dword [linen],10
 add dword [linel],10

 mov dword [ypos],94
 mov ebx,[tic_table+22]
 mov dword [fnumb],8
 call decnumb

 cld
 mov ecx,8
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+200
 rep movsb

 add dword [linen],10
 add dword [linel],10

 mov dword [ypos],154
 mov ebx,[tic_table+26]
 mov dword [fnumb],8
 call decnumb

 cld
 mov ecx,8
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+200
 rep movsb

 add dword [linen],12
 add dword [linel],12

 mov dword [ypos],228
 mov ebx,[pn]
 mov dword [fnumb],4
 call decnumb

 cld
 mov ecx,4
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+200
 rep movsb

 add dword [linel],4
 add dword [linen],4

 call newline

 mov dword [ypos],60

 cmp dword [xpos],254
 jne nscreen

 call pause2
 call cls2

 mov dword [xpos],24
 call newline
 mov dword [ypos],60

 mov edx,proc_head
 call printf
 call newline
 mov edx,proc_hd11
 call printf
 call newline
 call newline

nscreen:
 mov eax, dword [count2]
 cmp dword [pn],eax
 jne ll1
 mov dword [pn],0

 cmp byte [callp],1
 je go

 jmp ecmd2

printn:
 mov eax,47
 shl ebx,16
 mov edx,[ypos]
 shl edx,16
 add edx,[xpos]
 mov esi,0x00ddeeff
 int 0x40
 ret

pause2:
 call newline
 mov edx,mess1
 call printf

 mov eax,10
 int 0x40

red3:
 cmp eax,2
 je back
 cmp eax,3
 je exit

 call draw
 call red

 mov eax,10
 int 0x40
 jmp red3

back:
 mov eax,2
 int 0x40
 cmp ah,27
 je checmd
 ret

exit:
 mov eax,-1
 int 0x40

checmd:
 cmp byte [callp],1
 je ecmd3

 jmp ecmd

err:
 call clearsum

 call newline

 call ppr
 cmp dword [count],0
 je ecmd1
 mov edx,err1
 mov eax,4
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov esi,33
 int 0x40

 cld
 mov ecx,27
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,err1
 rep movsb

 add dword [linen],27
 add dword [linel],27

 call clearcmd

 jmp cmd_ok2

nparam:
 cmp byte [callp],1
 je gonext4
 call clearsum
gonext4:
 call newline
 mov edx,mess2
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

nparam2:
 cmp byte [callp],1
 je gonext3
 call clearsum
gonext3:
 call newline
 mov edx,mess5
 call printf

 cmp byte [callp],0
 je ecmd

 jmp go

cp:
 cmp byte [callp],1
 je gonext10
 call clearsum
gonext10:
 sub dword [count],3

 cld
 mov ecx,dword [count]
 mov edi,tic_table+400
 mov esi,tic_table+600
 add esi,3
 rep movsb

 mov ecx,12
loop50:
 cmp byte [tic_table+400+ecx],'+'
 je file2

 loop loop50

 add dword [count],3
 jmp nparam3

file2:
 mov dword [f1len],ecx

 inc ecx
 mov ebx,ecx
 cld
 mov edi,tic_table+9000
 mov esi,tic_table+400
 rep movsb

 mov ecx,12
 mov edi,tic_table+9100
 mov esi,tic_table+400
 add esi,ebx
 rep movsb

 mov ebx,tic_table+9000
 push esi edi ecx eax
 mov esi,ebx

 mov edi,tic_table+9000
 mov ecx,12
strup3:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup3
 pop eax ecx edi esi

 mov ecx,dword [f1len]

loopcp:
 cmp byte [tic_table+9000+ecx],'.'
 je chdotcp

 loop loopcp

 jmp chnodotcp

chdotcp:
 mov ebx,dword [f1len]

 cld
 mov ecx,4
 mov edi,tic_table+9000
 add edi,ebx
 mov esi,dzero
 rep movsb

 jmp gocp1

chnodotcp:
 mov ebx,dword [f1len]

 mov edi,tic_table+9000
 add edi,ebx
 mov esi,ddot
 movsb

 inc ebx

 cld
 mov ecx,3
 mov edi,tic_table+9000
 add edi,ebx
 mov esi,fill3
 rep movsb

gocp1:
 mov eax,6
 mov ebx,tic_table+9000
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 je nosuchfile

 mov dword [filesize],eax

 mov ebx,tic_table+9100
 push esi edi ecx eax
 mov esi,ebx

 mov edi,tic_table+9100
 mov ecx,12
strup4:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup4
 pop eax ecx edi esi

 mov ebx,dword [f1len]
 mov ecx,dword [count]
 sub ecx,ebx

 mov dword [f2len],ecx

loopcp2:
 cmp byte [tic_table+9100+ecx],'.'
 je chdotcp2

 loop loopcp2

 jmp chnodotcp2

chdotcp2:
 mov ebx,dword [f2len]

 cld
 mov ecx,4
 mov edi,tic_table+9100
 add edi,ebx
 mov esi,dzero
 rep movsb

 jmp gocp2

chnodotcp2:
 mov ebx,dword [f2len]

 mov edi,tic_table+9100
 add edi,ebx
 mov esi,ddot
 movsb

 inc ebx

 cld
 mov ecx,3
 mov edi,tic_table+9100
 add edi,ebx
 mov esi,fill3
 rep movsb

gocp2:
 mov eax,6
 mov ebx,tic_table+9100
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 jne alreadyexist

 mov eax,33
 mov ebx,tic_table+9100
 mov ecx,tic_table+25000
 mov edx,dword [filesize]
 mov esi,0
 int 0x40

 cmp eax,0
 jne no_ok

 mov eax,6
 mov ebx,tic_table+9100
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 je no_ok

 call newline
 mov edx,mess11
 call printf
 jmp cp_end

no_ok:
 call newline
 mov edx,mess12
 call printf

cp_end:
 add dword [count],3

 cmp byte [callp],1
 je go

 jmp ecmd

alreadyexist:
 add dword [count],3
 call newline
 mov edx,mess13
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

rn:
 cmp byte [callp],1
 je gonext11
 call clearsum
gonext11:
 sub dword [count],3

 cld
 mov ecx,dword [count]
 mov edi,tic_table+400
 mov esi,tic_table+600
 add esi,3
 rep movsb

 mov ecx,12
loop51:
 push ecx
 cmp byte [tic_table+400+ecx],'+'
 je file3

 pop ecx
 loop loop51

 add dword [count],3
 jmp nparam4

file3:
 mov dword [f1len],ecx

 inc ecx
 mov ebx,ecx
 cld
 mov edi,tic_table+9000
 mov esi,tic_table+400
 rep movsb

 mov ecx,12
 mov edi,tic_table+9100
 mov esi,tic_table+400
 add esi,ebx
 rep movsb

 mov ebx,tic_table+9000
 push esi edi ecx eax
 mov esi,ebx

 mov edi,tic_table+9000
 mov ecx,12
strup5:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup5
 pop eax ecx edi esi

 mov ecx,dword [f1len]

looprn:
 cmp byte [tic_table+9000+ecx],'.'
 je chdotrn

 loop looprn

 jmp chnodotrn

chdotrn:
 mov ebx,dword [f1len]

 cld
 mov ecx,4
 mov edi,tic_table+9000
 add edi,ebx
 mov esi,dzero
 rep movsb

 jmp gorn1

chnodotrn:
 mov ebx,dword [f1len]

 mov edi,tic_table+9000
 add edi,ebx
 mov esi,ddot
 movsb

 inc ebx

 cld
 mov ecx,3
 mov edi,tic_table+9000
 add edi,ebx
 mov esi,fill3
 rep movsb

gorn1:
 mov eax,6
 mov ebx,tic_table+9000
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 je nosuchfile

 mov ebx,tic_table+9100
 push esi edi ecx eax
 mov esi,ebx

 mov edi,tic_table+9100
 mov ecx,12
strup6:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup6
 pop eax ecx edi esi

 mov ebx,dword [f1len]
 mov ecx,dword [count]
 sub ecx,ebx

 mov dword [f2len],ecx

looprn2:
 cmp byte [tic_table+9100+ecx],'.'
 je chdotrn2

 loop looprn2

 jmp chnodotrn2

chdotrn2:
 mov ebx,dword [f2len]

 cld
 mov ecx,4
 mov edi,tic_table+9100
 add edi,ebx
 mov esi,dzero
 rep movsb

 jmp gorn2

chnodotrn2:
 mov ebx,dword [f2len]

 mov edi,tic_table+9100
 add edi,ebx
 mov esi,ddot
 movsb

 inc ebx

 cld
 mov ecx,3
 mov edi,tic_table+9100
 add edi,ebx
 mov esi,fill3
 rep movsb

gorn2:
 mov eax,6
 mov ebx,tic_table+9100
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 jne alreadyexist

 mov eax,6
 mov ebx,tic_table+9000
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 mov dword [filesize],eax

 mov eax,33
 mov ebx,tic_table+9100
 mov ecx,tic_table+25000
 mov edx,dword [filesize]
 mov esi,0
 int 0x40

 cmp eax,0
 jne no_ok1

 mov eax,6
 mov ebx,tic_table+9100
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 je no_ok1

 mov eax,32
 mov ebx,tic_table+9000
 int 0x40

 call newline
 mov edx,mess14
 call printf
 jmp rn_end

no_ok1:
 call newline
 mov edx,mess15
 call printf

rn_end:
 add dword [count],3

 cmp byte [callp],1
 je go

 jmp ecmd

del:
 cmp byte [callp],1
 je gonext12
 call clearsum
gonext12:
 sub dword [count],4

 cld
 mov ecx,dword [count]
 mov edi,tic_table+400
 mov esi,tic_table+600
 add esi,4
 rep movsb

 mov ebx,tic_table+400
 push esi edi ecx eax
 mov esi,ebx

 mov edi,tic_table+400
 mov ecx,70
strup1:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup1
 pop eax ecx edi esi

 mov ecx,dword [count]

loopdel:
 cmp byte [tic_table+400+ecx],'.'
 je chdotdel

 loop loopdel

 jmp chnodotdel

chdotdel:
 mov ebx,dword [count]

 cld
 mov ecx,4
 mov edi,tic_table+400
 add edi,ebx
 mov esi,dzero
 rep movsb

 jmp godel

chnodotdel:
 mov ebx,dword [count]

 mov edi,tic_table+400
 add edi,ebx
 mov esi,ddot
 movsb

 inc ebx

 cld
 mov ecx,3
 mov edi,tic_table+400
 add edi,ebx
 mov esi,fill3
 rep movsb

godel:
 mov eax,6
 mov ebx,tic_table+400
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+25000
 int 0x40

 cmp eax,4294967295
 je nosuchfile2

 mov eax,32
 mov ebx,tic_table+400
 int 0x40

 call newline
 mov edx,mess6
 call printf

 add dword [count],4

del_end:
 add dword [count],4

 cmp byte [callp],1
 je go

 jmp ecmd

nosuchfile:
 add dword [count],3
 call newline
 mov edx,mess7
 call printf

 cmp byte [callp],0
 je ecmd

 jmp go

nosuchfile2:
 add dword [count],4
 call newline
 mov edx,mess7
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

nosuchfile3:
 inc dword [count]
 call newline
 mov edx,mess7
 call printf
 jmp ecmd

autoexec:
 mov eax,6
 mov ebx,autoexfile
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+15000
 int 0x40

 cmp eax,4294967295
 je noaxfile

 sub dword [linen],81
 sub dword [xpos],10
 dec dword [lpress]

 jmp autolabel

noaxfile:
 ret

command:
 call clearsum
 dec dword [count]

 cld
 mov ecx,dword [count]
 mov edi,filename
 mov esi,tic_table+601
 rep movsb

 mov ebx,filename
 push esi edi ecx eax
 mov esi,ebx

 mov edi,filename
 mov ecx,12
strup7:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup7
 pop eax ecx edi esi

 cld
 mov ecx,4
 mov edi,filename+8
 mov esi,dcmd
 rep movsb

 mov eax,6
 mov ebx,filename
 mov ecx,0
 mov edx,-1
 mov esi,tic_table+15000
 int 0x40

 cmp eax,4294967295
 je nosuchfile3

autolabel:
 mov dword [filesize2],eax
 mov byte [callp],1

go:
 call clearcmd

gonext:
 cmp dword [filesize2],0
 je ecmd3

 mov ebx,tic_table
 add ebx,dword [readcnt]
 cmp byte [ebx],13
 je read

 inc dword [readcnt]
 inc dword [readcnt2]
 dec dword [filesize2]

 jmp gonext

read:
 mov ecx,dword [readcnt2]
 mov ebx,0
 mov edx,dword [readcnt]
 sub edx,dword [readcnt2]
loop60:
 push ecx
 mov edi,tic_table+600
 add edi,ebx
 mov esi,tic_table
 add esi,edx
 movsb

 inc ebx
 inc edx

 pop ecx
 loop loop60

 mov eax,dword [readcnt2]
 mov dword [count],eax

 mov dword [readcnt2],0
 add dword [readcnt],2

 jmp checkcmd

decnumb:
 cmp dword [fnumb],4
 je go1

 call divide
 mov dword [n8],edx
 mov ebx,eax
 call divide
 mov dword [n7],edx
 mov ebx,eax
 call divide
 mov dword [n6],edx
 mov ebx,eax
 call divide
 mov dword [n5],edx
 mov ebx,eax

go1:
 call divide
 mov dword [n4],edx
 mov ebx,eax
 call divide
 mov dword [n3],edx
 mov ebx,eax
 call divide
 mov dword [n2],edx
 mov ebx,eax
 call divide
 mov dword [n1],edx

 add dword [n1],48
 add dword [n2],48
 add dword [n3],48
 add dword [n4],48

 cmp dword [fnumb],4
 je go2

 add dword [n5],48
 add dword [n6],48
 add dword [n7],48
 add dword [n8],48

go2:
 mov edi,tic_table+200
 mov esi,n1
 movsb
 mov edi,tic_table+201
 mov esi,n2
 movsb
 mov edi,tic_table+202
 mov esi,n3
 movsb
 mov edi,tic_table+203
 mov esi,n4
 movsb

 cmp dword [fnumb],4
 je go4

 mov edi,tic_table+204
 mov esi,n5
 movsb
 mov edi,tic_table+205
 mov esi,n6
 movsb
 mov edi,tic_table+206
 mov esi,n7
 movsb
 mov edi,tic_table+207
 mov esi,n8
 movsb

go4:
 mov eax,4
 mov ebx,[ypos]
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov edx,tic_table+200
 mov esi,dword [fnumb]
 int 0x40

 ret

divide:
 mov eax,ebx
 xor edx,edx
 mov ecx,10
 div ecx
 ret

kill:
 mov eax,0
 mov ebx,0
 mov ecx,0
 mov edx,0

 mov al, byte [tic_table+608]
 mov bl, byte [tic_table+607]
 mov cl, byte [tic_table+606]
 mov dl, byte [tic_table+605]

 sub al,48
 sub bl,48
 sub cl,48
 sub dl,48

 imul ebx,10
 imul ecx,100
 imul edx,1000

 mov esi,0
 add esi,eax
 add esi,ebx
 add esi,ecx
 add esi,edx
 mov ecx,esi

 mov eax,9
 mov ebx,tic_table
 int 0x40

 cmp eax,ecx
 jb nosuchprocess

 mov eax,18
 mov ebx,2
 mov edx,0
 int 0x40

 call clearsum
 call newline
 mov edx,mess3
 call printf
 call newline
 mov edx,mess4
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

ecmd1:
 sub dword [xpos],10
 jmp ecmd

nosuchprocess:
 call clearsum
 call newline
 mov edx,mess8
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

echoeol:
 cmp byte [callp],1
 je gonext15
 call clearsum
gonext15:
 call newline

 cmp byte [callp],1
 je go

 jmp ecmd

echo:
 cmp byte [callp],1
 je gonext13
 call clearsum
gonext13:
 sub dword [count],5

 cld
 mov ecx,dword [count]
 mov edi,tic_table+300
 mov esi,tic_table+600
 add esi,5
 rep movsb

 call newline

 mov eax,4
 mov ebx,6
 shl ebx,16
 add ebx,[xpos]
 mov edx,tic_table+300
 mov ecx,0x00ddeeff
 mov esi,dword [count]
 int 0x40

 cld
 mov ecx,dword [count]
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+300
 rep movsb

 mov eax,dword [count]
 add dword [linen],eax
 add dword [linel],eax

 add dword [count],5

 cmp byte [callp],1
 je go

 jmp ecmd

printf:
 mov eax,4
 mov ebx,6
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov esi,45
 int 0x40

 cld
 mov ecx,45
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,edx
 rep movsb

 add dword [linen],46
 add dword [linel],46

 add dword [ypos],6

 ret

printipc:
 mov eax,4
 mov ebx,6
 shl ebx,16
 add ebx,[xpos]
 mov ecx,0x00ddeeff
 mov esi,79
 int 0x40

 cld
 mov ecx,79
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,edx
 rep movsb

 add dword [linen],80
 add dword [linel],80

 mov dword [ypos],480

 ret

clearcmd:
 cld
 mov ecx,80
 mov edi,tic_table+600
 mov esi,tic_table+500
 rep movsb

 mov ecx,80
 mov edi,tic_table+400
 mov esi,tic_table+500
 rep movsb

 mov ecx,81
 mov edi,tic_table+800
 mov [esi],byte 'x'
 rep movsb

 mov ecx,12
 mov edi,filename
 mov esi,pname
 rep movsb

 mov dword [count],0
 mov dword [pn],0
 mov dword [blockcnt],0
 mov dword [lscnt],8024
 ret

oldcmd:
 mov eax,dword [count]
 mov dword [oldcount],eax

 cld
 mov ecx,81
 mov edi,tic_table+900
 mov esi,tic_table+500
 rep movsb

 cld
 mov ecx,81
 mov edi,tic_table+900
 mov esi,tic_table+600
 rep movsb

 ret

ecmd:
 call oldcmd
 call clearcmd
 call newline

 call ppr
 call cursor

 jmp still

ecmd2:
 call clearcmd
 call newline

 call ppr
 call cursor
 jmp still

ecmd3:
 call clearcmd
 call newline

 call ppr
 call cursor

 mov dword [readcnt],15000
 mov dword [readcnt2],0
 mov byte [callp],0
 mov dword [filesize2],0

 jmp still

chparam:
 pop ecx
 mov eax,ecx
 mov edx,eax
 push edx
 inc eax

 cld
 mov edi,tic_table+400
 mov esi,tic_table+600
 rep movsb

 mov ecx,dword [count]
 sub ecx,eax

 cld
 mov edi,tic_table+800
 mov esi,tic_table+600
 add esi,eax
 rep movsb

 pop edx

 mov dword [ipccount],edx

 cld
 mov ecx,11
 mov edi,tic_table+400
 add edi,edx
 mov esi,qspace
 rep movsb

 mov eax,19
 mov ebx,tic_table+400
 mov ecx,tic_table+800
 int 0x40

 cmp eax,0xfffffff0
 jb cmd_ok

 jmp err

cls:
 call oldcmd
 call cls2

 cmp byte [callp],0
 jne needret

 call clearcmd
 call ppr
 call cursor

 jmp still

needret:
 sub dword [linen],81
 dec dword [lpress]
 sub dword [xpos],10

 jmp gonext

cls1:
 mov eax,13
 mov ebx,6*65536+486
 mov ecx,24*65536+250
 mov edx,0
 int 0x40
 ret

cls2:
 mov dword [linen],2000
 mov ecx,2025
loop2:
 push ecx
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,fill1
 movsb

 inc dword [linen]
 pop ecx
 loop loop2

 mov dword [linel],0
 mov dword [linen],2000
 mov dword [linev],2000
 mov dword [xpos],24
 mov dword [ypos],6
 mov dword [lpress],1
 mov dword [stnum],1

 call cls1

 ret

chscr:
 cmp dword [xpos],264
 jne no_scr

 mov dword [cnt_loop1],4500
 mov dword [cnt_loop2],2000

 mov ecx,2025
loop10:
 push ecx
 mov edi,tic_table
 add edi,dword [cnt_loop1]
 mov esi,tic_table
 add esi,dword [cnt_loop2]
 movsb

 inc dword [cnt_loop1]
 inc dword [cnt_loop2]

 pop ecx
 loop loop10

 mov dword [cnt_loop1],2000

 mov ecx,2025
loop11:
 push ecx
 mov edi,tic_table
 add edi,dword [cnt_loop1]
 mov esi,fill1
 movsb

 inc dword [cnt_loop1]

 pop ecx
 loop loop11

 mov dword [cnt_loop1],2000
 mov dword [cnt_loop2],4581

 mov ecx,1944
loop12:
 push ecx
 mov edi,tic_table
 add edi,dword [cnt_loop1]
 mov esi,tic_table
 add esi,dword [cnt_loop2]
 movsb

 inc dword [cnt_loop1]
 inc dword [cnt_loop2]

 pop ecx
 loop loop12

 dec dword [lpress]
 sub dword [linen],81

 mov dword [xpos],264
 mov dword [ypos],6
 mov dword [clr],480
 call clear

 call red

no_scr:
 ret

newline:
 call chscr
 push edx
 mov edx,81
 sub edx,dword [linel]
 add dword [linen],edx
 pop edx

 inc dword [lpress]

 mov dword [linel],0
 add dword [xpos],10
 ret

fill:
 cld
 mov ecx,81
 mov edi,tic_table+500
 mov esi,fill1
 rep movsb
 ret

nparam3:
 cmp byte [callp],1
 je gonext5
 call clearsum
gonext5:
 call newline
 mov edx,mess9
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

nparam4:
 cmp byte [callp],1
 je gonext6
 call clearsum
gonext6:
 call newline
 mov edx,mess0
 call printf

 cmp byte [callp],1
 je go

 jmp ecmd

nparam5:
 cmp byte [callp],1
 je gonext14
 call clearsum
gonext14:
 call newline
 mov edx,mess16
 call printf
 jmp ecmd

endscr:
 cmp byte [callp],1
 je ecmd3
 call clearsum
 call newline
 mov edx,mess17
 call printf
 jmp ecmd

checkcmd:
 cmp dword [tic_table+600],'help'
 jne no_help
 cmp dword [count],4
 jne no_help

 jmp help

no_help:
 cmp dword [tic_table+600],'exit'
 jne no_exit
 cmp dword [count],4
 jne no_exit

 jmp exit

no_exit:
 cmp word [tic_table+600],'ve'
 jne no_ver
 cmp byte [tic_table+602],'r'
 jne no_ver
 cmp dword [count],3
 jne no_ver

 jmp ver

no_ver:
 cmp word [tic_table+600],'cl'
 jne no_cls
 cmp byte [tic_table+602],'s'
 jne no_cls
 cmp dword [count],3
 jne no_cls

 jmp cls

no_cls:
 cmp dword [tic_table+600],'shut'
 jne no_shutdown
 cmp dword [tic_table+604],'down'
 jne no_shutdown
 cmp dword [count],8
 jne no_shutdown

 mov eax,18
 mov ebx,1
 int 0x40

 mov eax,5
 mov ebx,200
 int 0x40

no_shutdown:
 cmp word [tic_table+600],'ps'
 jne no_ps
 cmp dword [count],2
 je ps

no_ps:
 cmp dword [tic_table+600],'kill'
 jne no_kill
 cmp byte [tic_table+604],' '
 jne no_kill
 cmp dword [count],9
 je kill

 jmp nparam

no_kill:
 cmp dword [tic_table+600],'paus'
 jne no_pause
 cmp byte [tic_table+604],'e'
 jne no_pause
 cmp dword [count],5
 je pause1

no_pause:
 cmp dword [tic_table+600],'echo'
 jne no_echo
 cmp dword [count],4
 je echoeol
 cmp byte [tic_table+604],' '
 je echo

no_echo:
 cmp word [tic_table+600],'de'
 jne no_del
 cmp byte [tic_table+602],'l'
 jne no_del
 cmp byte [tic_table+603],' '
 jne no_del
 cmp dword [count],3
 je nparam2
 cmp dword [count],4
 je nparam2

 jmp del

no_del:
 cmp word [tic_table+600],'ls'
 jne no_ls
 cmp dword [count],2
 je ls
 cmp dword [count],3
 je ls

 jmp lscheck

no_ls:
 cmp word [tic_table+600],'cp'
 jne no_cp
 cmp dword [count],2
 je nparam3
 cmp dword [count],3
 je nparam3
 cmp dword [count],4
 je nparam3
 cmp dword [count],5
 je nparam3
 cmp byte [tic_table+602],' '
 jne no_cp

 jmp cp

no_cp:
 cmp word [tic_table+600],'rn'
 jne no_rn
 cmp dword [count],2
 je nparam4
 cmp dword [count],3
 je nparam4
 cmp dword [count],4
 je nparam4
 cmp dword [count],5
 je nparam4
 cmp byte [tic_table+602],' '
 jne no_rn

 jmp rn

no_rn:
 cmp dword [tic_table+600],'ends'
 jne no_end
 cmp dword [count],4
 je endscr

no_end:
 cmp byte [callp],1
 je checkprg

 ret

checkprg:
 mov ebx,tic_table+600
 push esi edi ecx eax
 mov esi,ebx

 mov edi,tic_table+600
 mov ecx,74
strup:
 mov al,[esi]
 cmp al,'A'
 jb @f
 cmp al,'z'
 ja @f
 cmp al,'a'
 jb @f
 add al,-0x20
@@:
 mov [edi],al
 inc esi
 inc edi
 dec ecx
 jnz strup
 pop eax ecx edi esi

 mov ecx,dword [count]
loop20:
 push ecx
 cmp byte [tic_table+600+ecx],'&'
 je chparam

 pop ecx
 loop loop20

 cld
 mov ecx,11
 mov edi,tic_table+600
 add edi,dword [count]
 mov esi,qspace
 rep movsb

 mov eax,19
 mov ebx,tic_table+600
 mov ecx,0
 int 0x40

 cmp eax,0xfffffff0
 jb cmd_ok_1

 jmp err

cmd_ok_1:
 mov eax,dword [count]
 mov dword [ipccount],eax

cmd_ok:
 mov eax,60
 mov ebx,1
 mov ecx,ipcb
 mov edx,118
 int 0x40

 call clearipc

 mov eax,40
 mov ebx,01000111b
 int 0x40

 mov eax,23
 mov ebx,10
 int 0x40

 cmp eax,7
 jne noipc

 cmp byte [callp],1
 je printipcprgname

 call clearsum

ipccontinue:
 mov eax,9
 mov ebx,tic_table+100000
 mov ecx,-1
 int 0x40

 mov ecx,eax
loopfindipc:
 push ecx

 mov eax,9
 mov ebx,tic_table+100000
 int 0x40

 mov bl,[tic_table+600]
 cmp byte [tic_table+100000+10],bl
 jne goonipc
 cmp dword [ipccount],1
 je ipcfinished

 mov bl,[tic_table+601]
 cmp byte [tic_table+100000+11],bl
 jne goonipc
 cmp dword [ipccount],2
 je ipcfinished

 mov bl,[tic_table+602]
 cmp byte [tic_table+100000+12],bl
 jne goonipc
 cmp dword [ipccount],3
 je ipcfinished

 mov bl,[tic_table+603]
 cmp byte [tic_table+100000+13],bl
 jne goonipc
 cmp dword [ipccount],4
 je ipcfinished

 mov bl,[tic_table+604]
 cmp byte [tic_table+100000+14],bl
 jne goonipc
 cmp dword [ipccount],5
 je ipcfinished

 mov bl,[tic_table+605]
 cmp byte [tic_table+100000+15],bl
 jne goonipc
 cmp dword [ipccount],6
 je ipcfinished

 mov bl,[tic_table+606]
 cmp byte [tic_table+100000+16],bl
 jne goonipc
 cmp dword [ipccount],7
 je ipcfinished

 mov bl,[tic_table+607]
 cmp byte [tic_table+100000+17],bl
 jne goonipc

goonipc:
 pop ecx
 dec ecx
 jnz loopfindipc

ipcfinished:
 mov ebx,[tic_table+100000+30]
 mov dword [ipcpid],ebx

 mov eax,60
 mov ebx,2
 mov ecx,dword [ipcpid]
 mov edx,ddot
 mov esi,1
 int 0x40

 call newline
 call clearipc

 jmp waitipc

printipcprgname:
 call newline
 call printcmd
 jmp ipccontinue

noipc:
 cmp byte [callp],1
 je prtcmd

 call clearsum

 jmp cmd_ok2

printcmd:
 mov eax,4
 mov ebx,6
 shl ebx,16
 add ebx,[xpos]
 mov edx,tic_table+600
 mov ecx,0x00ddeeff
 mov esi,dword [count]
 int 0x40

 cld
 mov ecx,dword [count]
 mov edi,tic_table
 add edi,dword [linen]
 mov esi,tic_table+600
 rep movsb

 mov eax,dword [count]
 add dword [linen],eax
 add dword [linel],eax

 ret

prtcmd:
 call newline
 call printcmd
 jmp go

cmd_ok2:
 cmp byte [callp],1
 je go

 ret

waitipc:
 mov eax,40
 mov ebx,01000111b
 int 0x40

 mov eax,10
 int 0x40

 cmp eax,7
 je ipcok
 cmp eax,1
 je reipc
 cmp eax,3
 je exit
 cmp eax,2
 je keyipc

 jmp waitipc

keyipc:
 mov eax,2
 int 0x40

 jmp waitipc

reipc:
 call draw
 call red

 jmp waitipc

clearipc:
 cld
 mov ecx,118
 mov edi,ipcb
 mov esi,ipcc
 rep movsb

 ret

ipcok:
 cmp dword [ipcb+16],'~ccc'
 je ipccls
 cmp dword [ipcb+16],'~eee'
 je endipc
 cmp dword [ipcb+16],'~lll'
 je ipcline
 cmp dword [ipcb+16],'~ppp'
 je ipcprint
 cmp dword [ipcb+16],'~kkk'
 je ipckey

 jmp waitipc

reipc2:
 call draw
 call red

ipckey:
 mov eax,10
 int 0x40

 cmp eax,1
 je reipc2
 cmp eax,3
 je exit
 cmp eax,2
 je ipckeypressed

 jmp ipckey

ipckeypressed:
 mov eax,2
 int 0x40

 mov byte [ipckeyvalue],ah

 mov eax,60
 mov ebx,2
 mov ecx,dword [ipcpid]
 mov edx,ipckeyvalue
 mov esi,1
 int 0x40

 call clearipc
 jmp waitipc

ipccls:
 call cls2
 call clearipc
 jmp waitipc

ipcline:
 call newline
 call clearipc
 jmp waitipc

ipcprint:
 mov edx,ipcb+20
 call printipc
 call clearipc
 jmp waitipc

endipc:
 cmp byte [callp],1
 je go

 call clearsum

 jmp ecmd

cmdexist:
 mov eax,9
 mov ebx,tic_table
 mov ecx,-1
 int 0x40

 mov ecx,eax
loopex:
 push ecx

 mov eax,9
 mov ebx,I_END
 int 0x40

 cmp word [I_END+10],'CM'
 jne no_cmd
 cmp byte [I_END+12],'D'
 jne no_cmd

 inc byte [cmd_ex]

 cmp byte [cmd_ex],2
 je exit

no_cmd:
 pop ecx
 loop loopex

 ret

title:
 db 'CMD - Command line interpreter'
title_end:

smb_cursor db '|'

prompt db 'CMD>>'
if lang eq de
h1  db '  CMD - Command line interpreter version 0.26 '
h2  db '        copyleft Chemist - dmitry_gt@tut.by   '
h3  db '  Verfuegbare Kommandos:                      '
h4  db '  HELP - Zeoigt diesen Text LS - Zeigt Dateien'
h5  db '  EXIT - Programmende       CP - Kopiert Datei'
h6  db '  CLS  - Loescht Bildschirm PS - Processinfo  '
h7  db '  KILL - Process beenden    RN - File umnennen'
h8  db '        VER  - Zeigt Programmversion          '
h9  db '        DEL  - Loescht Datei von Ramdisk      '
h10 db '        SHUTDOWN - KolibriOS beenden          '
h11 db '        PAUSE    - Auf Taste warten           '
h12 db '        ECHO     - Schreibt Text auf Ausgabe  '
h13 db '        ENDS     - Scriptende                 '
h14 db '        /[filename] - Script starten          '
h15 db '        Parameter mit "&" angeben:            '
h16 db '        tinypad&cmd.asm - oeffnet cmd.asm     '

about db 'Command Line Interpreter version 0.26         '

err1 db 'Kommando oder Dateiname unbekannt'

proc_head db  ' PID Name      Start     Laenge   Proc_NUMB  '
proc_hd11 db  '-------------------------------------------- '

mess1 db 'Taste druecken fuer weiter (ESC - abbrechen)..'
mess2 db 'Bitte 4 Byte Prozessnummer angeben (nicht PID)'
mess3 db 'Prozess mit angegebenere Nummer erfolgreich   '
mess4 db 'beendet.                                      '
mess5 db 'Verwendung: del [Dateiname]                   '
mess6 db 'Angegebene Datei erfolgreich geloescht.       '
mess7 db 'Datei nicht gefunden!                         '
mess8 db 'Prozess nicht gefunden!                       '
mess9 db 'Verwendung: cp [Quelle+Ziel]                  '
mess0 db 'Verwendung: rn [Quelle+Ziel]                  '

mess11 db 'Datei erfolgreich kopiert                     '
mess12 db 'ERROR: Kann Datei nicht kopieren!             '
mess13 db 'ERROR: Datei existiert bereits!               '
mess14 db 'Datei erfolgreich umbenannt                   '
mess15 db 'ERROR: Kann Datei nicht umbenennen!           '
mess16 db 'Scriptname erwartet!                          '
mess17 db 'Dieses Kommando ist nur in Scripts zulaessig! '
else
h1  db '  CMD - Command line interpreter version 0.26 '
h2  db '        copyleft Chemist - dmitry_gt@tut.by   '
h3  db '  Available commands:                         '
h4  db '  HELP - Display this text  LS - List files   '
h5  db '  EXIT - Exit Programm      CP - Copy file    '
h6  db '  CLS  - Clear Screen       PS - Process info '
h7  db '  KILL - Kill Process       RN - Rename File  '
h8  db '        VER  - Display programm version       '
h9  db '        DEL  - Delete file from ramdisk       '
h10 db '        SHUTDOWN - Quit Menuet                '
h11 db '        PAUSE    - Wait for keypress          '
h12 db '        ECHO     - Print text to screen       '
h13 db '        ENDS     - End script                 '
h14 db '        /[filename] - Execute script          '
h15 db '        Use & symbol to enter params:         '
h16 db '        tinypad&cmd.asm - open cmd.asm        '

about db 'Command Line Interpreter version 0.26         '

err1 db 'Unknown command or filename      '

proc_head db  ' PID Name      Start     Length   Proc_NUMB  '
proc_hd11 db  '-------------------------------------------- '

mess1 db 'Press any key to continue (ESC - Cancel)...   '
mess2 db 'Plz specify a 4 byte process number (not PID) '
mess3 db 'Process with number you specified has been    '
mess4 db 'terminated.                                   '
mess5 db 'Usage: del [filename]                         '
mess6 db 'Filename you specified has been deleted.      '
mess7 db 'No such file!                                 '
mess8 db 'No such process!                              '
mess9 db 'Usage: cp [source_file+destination_file]      '
mess0 db 'Usage: rn [source_file+destination_file]      '

mess11 db 'File successfully copied                      '
mess12 db 'ERROR: Can not copy file!                     '
mess13 db 'ERROR: Output file alredy exist!              '
mess14 db 'File successfully renamed                     '
mess15 db 'ERROR: Can not rename file!                   '
mess16 db 'You must specify a command script filename!   '
mess17 db 'This command is available only in scripts!    '
end if

linen dd 2000
lpress dd 1
linel dd 0
linev dd 2000
stnum dd 1
rdstat dd 0

ypos   dd 6
xpos   dd 24
count  dd 0
clr    dd 0
smb    db 0
pn     dd 0
count2  dd 0
count3  dd 9
oldcount dd 0
oldcount1 dd 0

fnumb dd 0
n1 dd 0
n2 dd 0
n3 dd 0
n4 dd 0
n5 dd 0
n6 dd 0
n7 dd 0
n8 dd 0

ddot db '.'
dcmd db '.CMD',0

cnt_loop1 dd 0
cnt_loop2 dd 0

lscnt dd 8024
blockcnt dd 0
lscntf db 0
lscntx db 0

filesize dd 0
filesize2 dd 0

readcnt dd 15000
readcnt2 dd 0

callp db 0
callp2 db 0

fill1 db ' '
fill3 db '   ',0

pname      db '            ',0
autoexfile db 'AUTOEXEC.CMD',0
filename   db '            ',0

dzero db 0,0,0,0
qspace db '           '

f1len dd 0
f2len dd 0

ipcpid dd 0

ipckeyvalue db 0

ipccount dd 0

cmd_ex db 0

ipcb:
 db 0
 db 0,0,0
 dd 8
times 110 db 0

ipcc:
 db 0
 db 0,0,0
 dd 8
times 110 db 0

I_END:

tic_table: