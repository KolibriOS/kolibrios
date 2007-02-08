;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;
;    TERMINAL
  
use32
 org	0x0
 db	'MENUET01'    ; header
 dd	0x01	      ; header version
 dd	START	      ; entry point
 dd	I_END	      ; image size
 dd	I_END+0x10000 ; required memory
 dd	I_END+0x10000 ; esp
 dd	0x0 , 0x0     ; I_Param , I_Path
   
include 'lang.inc'
include 'macros.inc'
   
   
START:				; start of execution
   
    call draw_window
   
    call set_variables
   
still:
   
    mov  eax,23 		; wait here for event
    mov  ebx,20
    int  0x40
   
    cmp  eax,1			; redraw request ?
    je	 red
    cmp  eax,2			; key in buffer ?
    je	 key
    cmp  eax,3			; button in buffer ?
    je	 button
    cmp  eax,16+4
    je	 read_input
   
    jmp  still
   
   
read_input:
   
    push ecx
    mov  eax,42
    mov  ebx,4
    int  0x40
    pop  ecx
   
    cmp  bl,27				; ESCAPE COMMAND
    jne  no_esc
    call esc_command
    jmp  newdata
  no_esc:
   
    cmp  bl,13				; BEGINNING OF LINE
    jne  nobol
    mov  ecx,[pos]
    add  ecx,1
  boll1:
    sub  ecx,1
    mov  eax,ecx
    xor  edx,edx
    mov  ebx,80
    div  ebx
    cmp  edx,0
    jne  boll1
    mov  [pos],ecx
    jmp  newdata
  nobol:
   
    cmp  bl,10				  ; LINE DOWN
    jne  nolf
   addx1:
    add  [pos],dword 1
    mov  eax,[pos]
    xor  edx,edx
    mov  ecx,80
    div  ecx
    cmp  edx,0
    jnz  addx1
    mov  eax,[pos]
    jmp  cm1
  nolf:
   
    cmp  bl,8				 ; BACKSPACE
    jne  nobasp
    mov  eax,[pos]
    dec  eax
    mov  [pos],eax
    mov  [eax+text],byte 32
    mov  [eax+text+60*80],byte 0
    jmp  newdata
   nobasp:
   
    cmp  bl,15				 ; CHARACTER
    jbe  newdata
    mov  eax,[pos]
    call draw_data
    mov  eax,[pos]
    add  eax,1
  cm1:
    mov  ebx,[scroll+4]
    imul ebx,80
    cmp  eax,ebx
    jb	 noeaxz
    mov  esi,text+80
    mov  edi,text
    mov  ecx,ebx
    cld
    rep  movsb
    mov  esi,text+80+60*80
    mov  edi,text+60*80
    mov  ecx,ebx
    cld
    rep  movsb
    mov  eax,ebx
    sub  eax,80
  noeaxz:
    mov  [pos],eax
  newdata:
    mov  eax,11
    int  0x40
    cmp  eax,16+4
    je	 read_input
    call draw_text
    jmp  still
   
   
  red:				; REDRAW WINDOW
    call draw_window
    jmp  still
   
  key:				; KEY
    mov  eax,2			; send to modem
    int  0x40
    shr  eax,8
    cmp  eax,178		; ARROW KEYS
    jne  noaup
    mov  al,'A'
    call arrow
    jmp  still
  noaup:
    cmp  eax,177
    jne  noadown
    mov  al,'B'
    call arrow
    jmp  still
  noadown:
    cmp  eax,179
    jne  noaright
    mov  al,'C'
    call arrow
    jmp  still
  noaright:
    cmp  eax,176
    jne  noaleft
    mov  al,'D'
    call arrow
    jmp  still
  noaleft:
  modem_out:
    mov  ecx,0x3f8
    mov  bl,al
    mov  eax,43
    int  0x40
    jmp  still
   
  button:			; BUTTON
    mov  eax,17
    int  0x40
    cmp  ah,1			; CLOSE PROGRAM
    jne  noclose
    mov  eax,45 		; FREE IRQ
    mov  ebx,1
    mov  ecx,4
    int  0x40
    mov  eax,46
    mov  ebx,1
    mov  ecx,0x3f0
    mov  edx,0x3ff
    int  0x40
     mov  eax,-1
     int  0x40
  noclose:
   
    jmp  still
   
   
arrow:
   
    push eax
    mov  al,27
    call to_modem
    mov  al,'['
    call to_modem
    pop  eax
    call to_modem
   
    ret
   
   
to_modem:
   
    pusha
   
    mov  ecx,0x3f8
    mov  ebx,eax
    mov  eax,43
    int  0x40
    mov  eax,5
    mov  ebx,5
    int  0x40
   
    popa
    ret
   
   
draw_data:
   
    pusha
   
    cmp  bl,0xe4   ; Á
    jne  noe4
    mov  bl,0xc1
  noe4:
    cmp  bl,0xc4   ; É
    jne  noc4
    mov  bl,0xc9
  noc4:
    mov  [eax+text],bl
    mov  bl,byte [attribute]
    mov  [eax+text+60*80],bl
   
    popa
    ret
   
   
irqtable:
   
    dd	0x3f8  + 0x01000000  ; read port 0x3f8, byte
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
    dd	0
   
   
   
   
set_variables:
   
    pusha
   
    mov  eax,46
    mov  ebx,0
    mov  ecx,0x3f0
    mov  edx,0x3ff
    int  0x40
   
    mov  eax,45 	 ; reserve irq 4
    mov  ebx,0
    mov  ecx,4
    int  0x40
   
    mov  eax,44
    mov  ebx,irqtable
    mov  ecx,4
    int  0x40
   
;    jmp  noportint
   
    mov  cx,0x3f8+3
    mov  bl,0x80
    mov  eax,43
    int  0x40
   
    mov  cx,0x3f8+1
    mov  bl,0
    mov  eax,43
    int  0x40
   
    mov  cx,0x3f8+0
    mov  bl,0x30 / 16
    mov  eax,43
    int  0x40
   
    mov  cx,0x3f8+3
    mov  bl,3
    mov  eax,43
    int  0x40
   
    mov  cx,0x3f8+4
    mov  bl,0xB
    mov  eax,43
    int  0x40
   
    mov  cx,0x3f8+1
    mov  bl,1
    mov  eax,43
    int  0x40
   
  noportint:
   
     mov  eax,40
     mov  ebx,0000000000010000b shl 16 + 111b
    int  0x40
   
    popa
   
    ret
   
   
   
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
   
   
draw_window:
   
    pusha
   
    mov  eax,12
    mov  ebx,1
    int  0x40
   
    mov  eax,0			   ; DRAW WINDOW
    mov  ebx,100*65536+491
    mov  ecx,100*65536+270
    mov  edx,0x13000000
    mov  edi,labelt
    int  0x40
   
    xor  eax,eax
    mov  edi,text+80*30
    mov  ecx,80*30 /4
    cld
    rep  stosd
   
    call draw_text
   
    mov  eax,12
    mov  ebx,2
    int  0x40
   
    popa
   
    ret
   
   
bgc  dd  0x000000
     dd  0x000000
     dd  0x00ff00
     dd  0x0000ff
     dd  0x005500
     dd  0xff00ff
     dd  0x00ffff
     dd  0x770077
   
tc   dd  0xffffff
     dd  0xff00ff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
     dd  0xffffff
   
   
draw_text:
   
    pusha
   
    mov  esi,text
    mov  eax,0
    mov  ebx,0
  newletter:
    mov  cl,[esi]
    mov  dl,[esi+60*80]
    cmp  cl,[esi+30*80]
    jne  yesletter
    cmp  dl,[esi+90*80]
    jne  yesletter
    jmp  noletter
  yesletter:
    mov  [esi+30*80],cl
    mov  [esi+90*80],dl
   
    pusha
    and  edx,0xff
    shl  edx,2
    add  edx,bgc
    mov  edx,[edx]
    mov  ecx,ebx
    add  ecx,26
    shl  ecx,16
    mov  cx,9
    mov  ebx,eax
    add  ebx,6
    shl  ebx,16
    mov  bx,6
    mov  eax,13
    int  0x40
    popa
   
    pusha
    and  edx,0xff
    shl  edx,2
    add  edx,tc
    mov  ecx,[edx]
    push bx
    mov  ebx,eax
    add  ebx,6
    shl  ebx,16
    pop  bx
    add  bx,26
    mov  eax,4
    mov  edx,esi
    mov  esi,1
    int  0x40
    popa
   
  noletter:
   
    add  esi,1
    add  eax,6
    cmp  eax,80*6
    jb	 newletter
    mov  eax,0
    add  ebx,10
    cmp  ebx,24*10
    jb	 newletter
   
    popa
    ret
   
   
esc_command:
   
     mov   eax,32
     mov   edi,esccmd
     mov   ecx,10
     cld
     rep   stosb
     mov   edi,esccmd
   newescc:
     mov   eax,42
     mov   ebx,4
     int   0x40
     cmp   ecx,0
     je    escok
     mov   eax,5
     mov   ebx,1
     int   0x40
     jmp   newescc
   escok:
     mov   [edi],bl
     add   edi,1
     cmp   edi,esccmd+20
     je    dontunderstand
     mov   esi,escend
   nec:
     cmp   bl,[esi]
     jz    com_ok
     add   esi,1
     cmp   [esi],byte 0
     je    newescc
     jmp   nec
   com_ok:
   
     call  get_numbers
   
     cmp   bl,'H'		      ; SET CURSOR POSITION
     jne   no_cursor_position
     cmp   [escnumbers],0
     jne   ncp1
     mov   [pos],dword 0
     jmp   cmd_done
    ncp1:
     mov    eax,[escnumbers]
     dec    eax
     imul   eax,80
     add    eax,[escnumbers+4]
     dec    eax
     mov    [pos],eax
     jmp    cmd_done
   no_cursor_position:
   
     cmp    bl,'K'			; ERASE LINE
     jne    no_erase_end_of_line
     cmp    [escnumbers],0
     jne    no_end_line
     mov    ecx,[pos]
   eeol:
     mov    [ecx+text],byte ' '
     mov    [ecx+text+60*80],byte 0
     add    ecx,1
     xor    edx,edx
     mov    eax,ecx
     mov    ebx,80
     div    ebx
     cmp    edx,0
     jne    eeol
     jmp    cmd_done
    no_end_line:
     cmp    [escnumbers],1		; BEGINNING OF LINE
     jne    no_beg_line
     mov    ecx,[pos]
   ebol:
     mov    [ecx+text],byte ' '
     mov    [ecx+text+60*80],byte 0
     sub    ecx,1
     xor    edx,edx
     mov    eax,ecx
     mov    ebx,80
     div    ebx
     cmp    edx,0
     jne    ebol
     mov    [pos],ecx
     jmp    cmd_done
    no_beg_line:
   no_erase_end_of_line:
   
     cmp    bl,'J'			    ; ERASE TO END OF SCREEN
     jne    no_erase_to_end_of_screen
     cmp    [escnumbers],dword 0
     jne    no_erase_to_end_of_screen
     mov    ecx,[pos]
   eteos:
     mov    [ecx+text],byte ' '
     mov    [ecx+text+60*80],byte 0
     add    ecx,1
     cmp    ecx,80*24+1
     jb     eteos
     jmp    cmd_done
   no_erase_to_end_of_screen:
   
     cmp    bl,'r'			     ; SET SCROLL REGION
     jne    no_scroll_region
     mov    eax,[escnumbers]
     dec    eax
     mov    [scroll+0],eax
     mov    eax,[escnumbers+4]
     mov    [scroll+4],eax
     jmp    cmd_done
   no_scroll_region:
   
     cmp    bl,'A'			      ; CURSOR UP
     jne    no_cursor_up
     mov    eax,[pos]
     sub    eax,80
     mov    [pos],eax
     jmp    cmd_done
   no_cursor_up:
   
     cmp    bl,'C'			      ; CURSOR LEFT
     jne    no_cursor_left
     mov    eax,[pos]
     mov    ebx,[escnumbers]
     sub    eax,ebx
     mov    [pos],eax
     call   cmd_done
   no_cursor_left:
   
     cmp    bl,'m'			     ; CHARACTER ATTRIBUTE
     jne    no_char_attribute
     mov    eax,[escnumbers]
     mov    [attribute],eax
     jmp    cmd_done
   no_char_attribute:
   
     cmp    bl,'Z'			      ; TERMINAL TYPE
     jne    no_terminal_type
     mov    al,27
     call   to_modem
     mov    al,'?'
     call   to_modem
     mov    al,'1'
     call   to_modem
     mov    al,';'
     call   to_modem
     mov    al,'0'
     call   to_modem
     mov    al,'c'
     call   to_modem
     jmp    cmd_done
   no_terminal_type:
   
   dontunderstand:
   
   cmd_done:
   
     ret
   
   
draw_numbers:
   
     pusha
   
     mov  eax,13
     mov  ebx,250*65536+100
     mov  ecx,8*65536+8
     mov  edx,0x000000
     int  0x40
   
     mov  eax,[escnumbers]
     xor  edx,edx
     mov  ebx,10
     div  ebx
     add  eax,48
     add  edx,48
     mov  byte [numtext+0],al
     mov  byte [numtext+1],dl
   
     mov  eax,[escnumbers+4]
     xor  edx,edx
     mov  ebx,10
     div  ebx
     add  eax,48
     add  edx,48
     mov  [numtext+3],al
     mov  [numtext+4],dl
   
     mov  eax,4
     mov  ebx,250*65536+8
     mov  ecx,0xffffff
     mov  edx,numtext
     mov  esi,10
     int  0x40
   
     popa
   
     ret
   
draw_event:
   
     pusha
   
     mov  eax,13
     mov  ebx,150*65536+100
     mov  ecx,8*65536+8
     mov  edx,0xffffff
     int  0x40
   
     mov  eax,4
     mov  ebx,150*65536+8
     mov  ecx,0x000000
     mov  edx,esccmd
     mov  esi,20
     int  0x40
   
     popa
     ret
   
   
get_numbers:
   
     pusha
   
     mov   [escnumbers+0],0
     mov   [escnumbers+4],0
     mov   [escnumbers+8],0
     mov   ecx,esccmd
     cmp   [ecx+1],byte '0'
     jb    gn_over
     cmp   [ecx+1],byte '9'
     jg    gn_over
     mov   edi,escnumbers
   gn_new:
     add   ecx,1
     movzx eax,byte [ecx]
     sub   eax,48
     add   ecx,1
     cmp   [ecx],byte '0'
     jb    gnl1
     cmp   [ecx],byte '9'
     jg    gnl1
     mov   ebx,10
     xor   edx,edx
     mul   ebx
     movzx ebx,byte[ecx]
     add   eax,ebx
     sub   eax,48
     add   ecx,1
   gnl1:
     mov   [edi],eax
     add   edi,4
     cmp   [ecx],byte ';'
     je    gn_new
  gn_over:
     popa
     ret
   
   
   
   
; DATA AREA
   
   
pos	    dd	80*10
irc_data    dd	0x0
print	    db	0x0
attribute   dd	0
scroll	    dd	1
	    dd	24
numtext     db	'                     '
esccmd	    dd	0,0,0,0,0,0,0,0,0,0,0,0,0
escend	    db	'ZrhlABCDHfDME=>NmKJgincoyq',0
escnumbers  dd	0,0,0,0,0
wcolor	    dd	0x000000
labelt	    db	'TERMINAL FOR MODEM IN COM1  0.03',0
   
text:
db '                                                                   '
db '             '
db '*** A TELNET APPLICATION FOR HAYES COMPATIBLE MODEMS IN COM1       '
db '             '
db '*** USE HAYES COMMANDS TO CONNECT TO A SERVER                      '
db '             '
db '*** ATDT (PHONENUMBER)                                             '
db '             '
db '                                                                   '
db '             '
   
I_END:
   
   
   
   
   