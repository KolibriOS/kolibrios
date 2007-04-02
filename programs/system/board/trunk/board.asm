;
;   DEBUG BOARD for APPLICATIONS and KERNEL DEVELOPMENT
;
;   See f63
;
;   Compile with FASM for Menuet
;
LMARGIN equ (15+5)
TMARGIN equ (35+5)
HSPACE  equ 16		
VSPACE  equ 12
IPC_BUF equ 160
DR_GRID equ 0;1

FL_KRNL equ 1

include 'lang.inc'

   use32
   org    0x0
   db     'MENUET01'              ; 8 byte id
   dd     0x01                    ; header version
   dd     START                   ; start of code
   dd     I_END                   ; size of image
   dd     i_end+0x2000                  ; memory for app (4 Kb)
   dd     i_end+0x2000                  ; esp
   dd     0x0 , 0x0               ; I_Param , I_Icon
include 'MACROS.INC'
include 'debug.inc'
purge newline
MAXSTRINGS = 16
TMP = 80*(MAXSTRINGS+1)

START:                          ; start of execution

     mcall 60,1,ipcbuff,IPC_BUF+20
     mcall 40,1000111b
     mov  [ipcbuff+4],8
     mov  ecx,4096
    flush:
     mov  eax,63
     mov  ebx,2
     mcall
     loop flush

     mov  ecx, TMP
     xor  eax, eax
     mov  edi, [targ]
     rep  stosb

     mov  [tmp1],'x'
     mov  [tmp2],'x'

     mov  eax,14
     mcall
     and  eax,0xffff0000
     sub  eax,399 shl 16
     add  eax,399
     mov  [xstart],eax

     mov  eax,48
     mov  ebx,3
     mov  ecx,sc
     mov  edx,sizeof.system_colors
     mcall

  red:
     call draw_window

still:

    mov  eax,23                 ; wait here for event
    mov  ebx,1
    mcall

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button
    cmp  eax,7
    je   ipc

    mov  eax,63
    mov  ebx,2
    mcall

    cmp  ebx,1
    jne  still
		
  new_data:
    mov  ebp,[targ]
  .no4:
    cmp  al,13
    jne  no13
    and  dword[ebp-8],0
    jmp  new_check
   no13:
    cmp  al,10
    jne  no10
    inc  dword[ebp-4]
    cmp  dword[ebp-4],MAXSTRINGS
    jbe  .noypos
    mov  dword[ebp-4],MAXSTRINGS
    lea  esi,[ebp+80]
    mov  edi,ebp
    mov  ecx,80*(MAXSTRINGS)
    cld
    rep  movsb

    mov  esi,[ebp-4]
    imul esi,80
    add  esi,[ebp-8]
    add  esi,ebp
    mov  ecx,80
    xor  al,al
    rep  stosb
  .noypos:
    mov  [targ],text2
    and  [krnl_cnt],0
    jmp  new_check
  no10:
    cmp  ebp,text1
    je   add2
  		mov  ecx,[krnl_cnt]
		  cmp  al,[krnl_msg+ecx]
  		jne  .noknl
		  inc  [krnl_cnt]
  		cmp  [krnl_cnt],4
  		jne  new_check
    mov  [targ],text1
  	.noknl:
    mov  ebp,[targ]
	   jecxz .add
    push eax
    mov  esi,krnl_msg
   .l1:
    lodsb
    call add_char
    loop .l1
    pop  eax
   .add:
    and  [krnl_cnt],0
  add2:
    call add_char

  new_check:

    mov  eax,63
    mov  ebx,2
    mcall

    cmp  ebx,1
    je   new_data

    cmp  [vmode],2
    je   still
    call draw_window

    jmp  still

  ipc:
    mov  [vmode],2
    mov  eax,ipcbuff
    mov  esi,[eax+8]
    mov  byte[eax],1
    push dword[eax+12]
    pop  [dump_len]
    mcall 9,work,-1
    mov  ecx,eax
   .lp:
    mcall 9
    cmp  [ebx+30],esi
    je   .ok
    loop .lp
    and  [dump_len],0
    jmp  red
  .ok:
    mov  [pid],esi
    lea  esi,[ebx+10]
    mov  edi,dump_title+10
    mov  ecx,12
    rep  movsb
    jmp  red
  key:                          ; key
    mov  al,2                  ; just read it and ignore
    mcall
    cmp  ah,' '
    je   button.no_krnl_flt
    cmp  [vmode],2
    jne  still
    cmp  ah,176 ;left
    jb   still
    cmp  ah,179 ;right
    ja  still
    mov  ecx,[offs]
    shr  eax,8
    sub  eax,176
    add  ecx,[arrows+eax*4]
    shl  ecx,12
    shr  cx,12
    jmp  button.check_sel
  .nol:
    jmp  still

arrows dd -1,16,-16,1

  button:                       ; button
    mov  al,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jne  .noclose

    or   eax,-1                 ; close this program
    mcall
  .noclose:
   	shr  eax,8
  		cmp  eax,10
  		jb   .nodump
		  lea  edi,[eax-10]
  		mcall 37,1
		  sub  eax,[edi*4+dump_cell_marg]
  		sub  eax,TMARGIN+VSPACE
  		push eax
    and  eax,0xffff
		  xor  edx,edx
  		div  word[edi*4+dump_cell_size+2]
		  mov  ecx,eax
    shl  ecx,16
  		xor  edx,edx
		  pop  eax
  		shr  eax,16
  		div  word[edi*4+dump_cell_size]
  		mov  cx,ax
  .check_sel:
  		mov  eax,ecx
    shl  ax,12
    shr  eax,12
    inc  eax
    cmp  eax,[dump_len]
    ja   still;.nosel
    mov  dword[sel_byte],ecx
    dec  eax
    mov  [offs],eax
    jmp  red

  .nodump:
    cmp  eax,2
    jne  .no_krnl_flt
    xor  [flag],FL_KRNL
    jmp  still
  .no_krnl_flt:
    mov  [ipcbuff+4],8
    and  byte[ipcbuff],0
    inc  [vmode]
    cmp  [vmode],3
    jb   .vmok
    and  [vmode],0
  .vmok:
    jmp  red

add_char:
    push esi
    mov  esi,[ebp-4]
    imul esi,80
    add  esi,[ebp-8]
    mov  [ebp+esi],al
    inc  dword[ebp-8]
    cmp  dword[ebp-8],80
    jb   .ok
    mov  dword[ebp-8],79
  .ok:
    pop  esi
    ret

;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    xor  eax,eax                     ; function 0 : define and draw window
;   mov  ebx,50*65536+400          ; [x start] *65536 + [x size]
    mov  ebx,[xstart]
    mov  ecx,MAXSTRINGS*10+45      ; [y start] *65536 + [y size]
    mov  edx,[sc.work]             ; color of work area RRGGBB,8->color gl
    or   edx,0x13000000
    mov  edi,header                ; WINDOW LABEL
    mcall
    
    mov  ecx,4
    mov  esi,[sc.work]
    mov  ebx,296 shl 16+5*6
    mov  edx,3;+1 shl 30
    mcall 8,,<5,12>
    mov  edx,[vmode]
    lea  edx,[edx*4+duk]
    mcall 4,<300,8>,,,4

    cmp  [vmode],2
    je   no_mdbg
    mov  ebx,15*65536+33           ; draw info text with function 4
    mov  ecx,[sc.work_text]
    mov  edx,text1
    cmp  [vmode],0
    je   .kern
    mov  edx,text2
  .kern:
    mov  esi,80
    mov  eax,4
  newline:
    mcall
    add  ebx,10
    add  edx,80
    cmp  [edx],byte 'x'
    jne  newline
    jmp  enddraw
  no_mdbg:
  if DUMP_TEST eq 1
    mov  esi,0
    mov  [dump_len],100;IPC_BUF
  else
    mov  esi,ipcbuff+16
  end if
    mov  ecx,[dump_len]
    call dump_btn
  		call draw_dump
		enddraw:
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret

if DR_GRID eq 1
draw_grid:
  mov  ecx,11
  mov  edi,(TMARGIN+VSPACE)shl 16+TMARGIN+VSPACE
 .l1:
  push ecx
  mov  ebx,LMARGIN shl 16+LMARGIN+16*HSPACE
  mcall 38,,edi,0
  add  edi,VSPACE shl 16+VSPACE
  pop  ecx
  loop .l1
  mov  ecx,17
  mov  edi,(TMARGIN+VSPACE)shl 16+TMARGIN+VSPACE*10
  mov  ebx,LMARGIN shl 16+LMARGIN
 .l2:
  push ecx
  mcall 38,,edi,0
  add  ebx,HSPACE shl 16+HSPACE
  pop  ecx
  loop .l2
  ret
end if

draw_numbers:
  mcall 4,(LMARGIN+2) shl 16+180,0,numb,numb_len-numb
  mov  eax,dword[sel_byte]
  shl  ax,12
  shr  eax,12
  mov  edi,eax
if ~ DUMP_TEST eq 1
  add  edi,ipcbuff+16
end if
  mov  edx,(LMARGIN+2+6*6)shl 16+180
  mov  ebx,0x30000
  movzx ecx,byte[edi]
  mcall 47,,,,0x4e00e7
  add  ebx,0x20000
  add  edx,(6*10)shl 16
  movzx ecx,word[edi]
  mcall
  add  ebx,0x50000
  add  edx,(6*13)shl 16
  mov  ecx,[edi]
  mcall
  mov  ebx,0x80100
  add  edx,(6*19)shl 16
  mcall
.ex:
  ret

draw_dump:
; esi - data ptr, ecx - length
  jecxz draw_numbers.ex
  pusha
  call draw_numbers
  mcall 4,(LMARGIN+2) shl 16+27,0,dump_title,dump_t_len-dump_title
  mcall 47,0x30101,ipcbuff+8,(LMARGIN+2+6*29)shl 16+27
  add   edx,(6*27) shl 16
  mov   ecx,offs
  mcall
  sub   edx,(5*6)shl 16
  mcall ,0x30001
  mov  ecx,16
  mov  edi,HSPACE shl 16
  mov  ebx,(LMARGIN+5)shl 16+42
  call draw_marks
  mov  ecx,[esp+24]
  dec  ecx
  shr  ecx,4
  inc  ecx
  mov  ebx,(LMARGIN-10)shl 16+TMARGIN+2+VSPACE
  mov  edi,VSPACE
  call draw_marks
  popa
		mov  edx,TMARGIN+2
		mov  edi,ecx
	.lp:	
		add  edx,(LMARGIN+2) shl 16+VSPACE
		mov  ecx,16
		cmp  edi,ecx
		jae  .less
		mov  ecx,edi
	.less:	
		sub  edi,ecx
		push esi ecx
		mov  ebx,0x20100
	.lp1:	
		push ecx esi
		movzx ecx,byte[esi]
		mcall 47,,,,0
		add  edx,HSPACE shl 16
		pop  esi ecx
		inc  esi
		loop .lp1
		pusha
		mov  ebx,edx
		and  ebx,0xffff
		add  ebx,(LMARGIN+16*HSPACE+15)shl 16
		mov  edx,[esp+36]
		mov  esi,[esp+32]
		mcall 4,,0
		popa
		add  esp,8
		and  edx,0xffff
		test edi,edi
		jnz  .lp
.ex:
		ret

draw_marks:
; ebx -xy, edi-addition, ecx -cycles
		pusha
  mov  edx,__hexdigits
  mov  eax,4
  mov  esi,1
.tt:
  push ecx
  mcall ,,0xffffff
  add  ebx,edi
  inc  edx
  pop  ecx
  loop .tt
  popa
  ret

dump_btn: ; ecx-length
  jecxz draw_dump.ex
		pusha
		test ecx,0xffff
		je   .even
		add  ecx,16
	.even:	
		shr  ecx,4
		imul ecx,VSPACE
		add  ecx,(TMARGIN+VSPACE)shl 16-5
		mcall 8,LMARGIN shl 16+16*HSPACE-5,,10+3 shl 29,[sc.work]
		inc  edx
		mcall ,(LMARGIN+16*HSPACE+15)shl 16+6*16
		mov  edx,0xff0000
		mov  esi,dump_cell_size
		xor  eax,eax
		movzx ebx,[sel_byte]
		lodsw
		imul bx,ax
		shl  ebx,16
		lea  ebx,[ebx+eax+LMARGIN shl 16]
		movzx ecx,[sel_byte+2]
		lodsw
		imul cx,ax
		shl  ecx,16
		lea  ecx,[ecx+eax+(TMARGIN+VSPACE) shl 16]
		mcall 13
		movzx ebx,[sel_byte]
		lodsw
		imul bx,ax
		shl  ebx,16
		lea  ebx,[ebx+eax+(LMARGIN+16*HSPACE+15)shl 16]
  mcall 13
		popa
.ex:
		ret		

krnl_msg db 'K : '
duk db 'KernUserDump'
numb db 'Byte:     Word:       Dword:               Hex:'
numb_len:
dump_title db 'Dump from              (pid=    h)         Offset:     (   h)'
dump_t_len:

; DATA AREA

dump_cell_marg dd LMARGIN shl 16,(LMARGIN+16*HSPACE+15)shl 16
dump_cell_size dw HSPACE,VSPACE,6,VSPACE
; 11,11 > 0,-1
; 5,11  > 0,-1
if lang eq ru
   header    db   'Доска отладки и сообщений',0
else if lang eq en
   header    db   'General debug & message board',0
else
   header    db   'Allgemeines debug- & nachrichtenboard',0
end if
   krnl_cnt dd 0
   vmode dd 0
   targ  dd text2
I_END:
     offs dd ?
     flag rb 1
     ipcbuff rb IPC_BUF+20
     rd 2
;     x1pos  dd ?
;     y1pos  dd ?
     text1 rb 80*(MAXSTRINGS+1)
     tmp1  db ?
     rd 2
;     x2pos  dd ?
;     y2pos  dd ?
     text2 rb 80*(MAXSTRINGS+1)
     tmp2  db ?
     work rb 4096
     sel_byte  dw ?,?
     pid  dd ?
     xstart dd ?
     dump_len dd ?
     sc system_colors
i_end:
