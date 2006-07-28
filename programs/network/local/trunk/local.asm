;
;    Remote processing example (local node)                                     
;
;    Compile with FASM for Menuet
;
   
   
use32
   
                org     0x0
   
                db      'MENUET00'              ; 8 byte id
                dd      38                      ; required os
                dd      START                   ; program start
                dd      I_END                   ; program image size
                dd      0x100000                ; required amount of memory
                dd      0x00000000              ; reserved=no extended header
   
include 'lang.inc'
include 'macros.inc'
   
START:                                  ; start of execution
   
    mov  eax,53                ; open socket
    mov  ebx,0
    mov  ecx,0x2000            ; local port
    mov  edx,0x3000            ; remote port
    mov  esi,dword [host_ip]   ; node IP
    int  0x40
   
    mov  [socketNum], eax
   
    call draw_window            ; at first, draw the window
   
still:
   
    mov  eax,23                 ; wait here for event
    mov  ebx,1
    int  0x40
   
    cmp  eax,1                  ; redraw request ?
    jz   red
    cmp  eax,2                  ; key in buffer ?
    jz   key
    cmp  eax,3                  ; button in buffer ?
    jz   button
   
    mov  eax, 53                ; get data
    mov  ebx, 2
    mov  ecx, [socketNum]
    int  0x40
    cmp  eax, 0
    jne  read
   
    jmp  still
   
red:
    call draw_window
    jmp  still
   
key:
    mov  eax,2
    int  0x40
    jmp  still
   
button:
    mov  eax,17
    int  0x40
   
    cmp  ah,1                  ; button id=1 ?
    jnz  noclose
    mov  eax, 53
    mov  ebx, 1
    mov  ecx, [socketNum]
    int  0x40
    mov  eax,-1
    int  0x40
  noclose:
   
    cmp  ah,2                  ; SEND CODE ?
    je   send_xcode
   
    cmp  ah,3                  ; LEFT COORDINATES ?
    jne  no_left
    mov  [picture_position],0
    mov  dword [send_data+15],dword STARTX
    mov  dword [send_data+19],dword 4
    mov  esi,send_data
    mov  edi,I_END
    mov  ecx,23
    cld
    rep  movsb
    mov  [I_END+23],dword -20
    mov  eax,53
    mov  ebx,4
    mov  ecx,[socketNum]
    mov  edx,23 + 4
    mov  esi,I_END
    int  0x40
    jmp  still
  no_left:
   
    cmp  ah,4                  ; RIGHT COORDINATES ?
    jne  no_right
    mov  [picture_position],128
    mov  dword [send_data+15],dword STARTX
    mov  dword [send_data+19],dword 4
    mov  esi,send_data
    mov  edi,I_END
    mov  ecx,23
    cld
    rep  movsb
    mov  [I_END+23],dword -20 + 128
    mov  eax,53
    mov  ebx,4
    mov  ecx,[socketNum]
    mov  edx,23 + 4
    mov  esi,I_END
    int  0x40
    jmp  still
  no_right:
   
    cmp  ah,5                  ; SEND EXECUTE ?
    je   send_execute
   
    jmp  still
   
   
xx  dd  0
yy  dd  0
   
   
   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                              ;;
;;           SEND CODE TO REMOTE                ;;
;;                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
send_xcode:
   
  mov  dword [send_data+15],dword 0x80000
  mov  dword [send_data+19],dword remote_code_end - remote_code_start
   
  mov  esi,send_data              ; header
  mov  edi,I_END
  mov  ecx,23
  cld
  rep  movsb
   
  mov  esi,remote_code  ;  remote_code_start      ; data
  mov  edi,I_END+23
  mov  ecx,remote_code_end - remote_code_start
  cld
  rep  movsb
   
  mov  eax,53                     ; SEND CODE TO REMOTE
  mov  ebx,4
  mov  ecx,[socketNum]
  mov  edx,23 + remote_code_end - remote_code_start
  mov  esi,I_END
  int  0x40
   
  jmp  still
   
   
send_execute:
   
  mov  dword [execute+15],dword draw_fractal
   
  mov  eax,53                     ; START EXECUTE AT REMOTE
  mov  ebx,4
  mov  ecx,[socketNum]
  mov  edx,19
  mov  esi,execute
  int  0x40
   
  mov  edi,3
   
  jmp  still
   
   
   
;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                      ;;
;;       READ           ;;
;;                      ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;
   
   
read:
   
   
cfr007:
   
 mov  eax, 53
 mov  ebx, 3
 mov  ecx, [socketNum]
 int  0x40   ; read byte
   
 shl  edx,8
 mov  dl,bl
   
 dec  edi
 jnz  cok
   
 mov  edi,3
   
 and  edx,0xffffff
 mov  eax,1
 mov  ebx,[xx]
 mov  ecx,[yy]
 add  ebx,15
 add  ecx,35
 add  ebx,[picture_position]
 int  0x40
   
 inc  [xx]
 cmp  [xx],dword 128
 jb   cok
 mov  [xx],0
   
 inc  [yy]
 cmp  [yy],dword 128
 jb   cok
 mov  [yy],0
   
cok:
   
 mov  eax, 53
 mov  ebx, 2
 mov  ecx, [socketNum]
 int  0x40   ; any more data?
   
 cmp  eax, 0
 jne  cfr007  ; yes, so get it
   
 jmp  still
   
   
   
   
;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************
   
   
draw_window:
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    int  0x40
   
                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+286         ; [x start] *65536 + [x size]
    mov  ecx,60*65536+330         ; [y start] *65536 + [y size]
    mov  edx,0x03ffffff            ; color of work area RRGGBB
    mov  esi,0x80aabbcc            ; color of grab bar  RRGGBB,8->color gl
    mov  edi,0x00aabbcc            ; color of frames    RRGGBB
    int  0x40
   
                                   ; WINDOW LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,8*65536+8             ; [x start] *65536 + [y start]
    mov  ecx,0x00ffffff            ; color of text RRGGBB
    mov  edx,labeltext             ; pointer to text beginning
    mov  esi,lte-labeltext         ; text length
    int  0x40
   
    mov  eax,8                     ; SEND CODE
    mov  ebx,60*65536+160
    mov  ecx,181*65536+13
    mov  edx,2
    mov  esi,0x667788
    int  0x40
   
    mov  eax,8                     ; LEFT
    mov  ebx,60*65536+75
    mov  ecx,197*65536+13
    mov  edx,3
    mov  esi,0x667788
    int  0x40
   
    mov  eax,8                     ; RIGHT
    mov  ebx,148*65536+72
    mov  ecx,197*65536+13
    mov  edx,4
    mov  esi,0x667788
    int  0x40
   
    mov  eax,8                     ; SEND EXECUTE
    mov  ebx,60*65536+160
    mov  ecx,213*65536+13
    mov  edx,5
    mov  esi,0x667788
    int  0x40
   
   
    cld
    mov  ebx,25*65536+185           ; draw info text with function 4
    mov  ecx,0x000000
    mov  edx,text
    mov  esi,40
  newline:
    mov  eax,4
    int  0x40
    add  ebx,16
    add  edx,40
    cmp  [edx],byte 'x'
    jnz  newline
   
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40
   
    ret
   
   
; DATA AREA
   
   
text:
    db ' 1)            SEND CODE                '
    db ' 2)       LEFT          RIGHT           '
    db " 3)         SEND 'EXECUTE'              "
    db '                                        '
    db ' LOCAL   : 192.168.1.26                 '
    db ' REMOTE  : 192.168.1.22                 '
    db ' REMOTE CODE AT THE END OF THIS FILE    '
    db 'x <- END MARKER, DONT DELETE            '
   
   
labeltext:  db  'CLUSTER LOCAL'  ;
lte:
   
socketNum   dd  0x0
   
host_ip  db  192,168,1,22
   
picture_position dd 0x0
   
send_data   db  'MenuetRemote00'  ; 00  header      ; -> remote port 0x3000
            db  1                 ; 14  send
            dd  0x0               ; 15  position
            dd  0x0               ; 19  size
                                  ; 23
   
execute     db  'MenuetRemote00'  ; 00  header      ; -> remote port 0x3000
            db  2                 ; 14  execute
            dd  0x0               ; 15  position
                                  ; 19
   
   
   
   
   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                             ;;
;;                       REMOTE CODE                           ;;
;;                                                             ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
   
   
remote_code:
   
   
org 0x80000
   
remote_code_start:
   
   
PIXWIDTH    equ   129
PIXHEIGHT   equ   129
ZOOMLIMIT   equ    13
DELTA       equ   200
THRESHOLD   equ     7
STARTSCALE  equ     6
CHAR_COLOR  equ   0fh
   
STARTX       dd   -20
STARTY       dd    10
scaleaddy    dd    60
scaleaddx    dd   100
   
   
   
draw_fractal:
   
        pusha
   
        movzx   ebp,word [STARTX]
        movzx   edi,word [STARTY]
        mov     cx, PIXHEIGHT   ; height of screen in pixels
   
        sub     di,cx           ; adjust our Y offset
@@CalcRow:
   
        push    cx
        mov     cx, PIXWIDTH -1  ; width of screen in pixels
   
        sub     bp,cx           ;
@@CalcPixel:
        push    cx              ; save the column counter on stack
        xor     cx, cx          ; clear out color loop counter
        xor     bx, bx          ; zero i coefficient
        xor     dx, dx          ; zero j coefficient
@@CycleColors:
        push    dx              ; save j value for later
        mov     ax, bx          ; ax = i
        sub     ax, dx          ; ax = i - j
        add     dx, bx          ; dx = i + j
        stc                     ; one additional shift, please
        call    Shifty          ; ax = ((i+j)*(i-j)) shifted right
        pop     dx              ; retrieve our saved value for j
        add     ax,bp           ; account for base offset...
        cmp     ah,THRESHOLD    ; Q: is i &gt; THRESHOLD * 256?
        xchg    bx,ax           ; now swap new i with old i
        jg      @@draw          ; Y: draw this pixel
        clc                     ; no additional shifts here, please
        call    Shifty          ; now dx:ax = old i * j
        xchg    dx,ax           ;
        add     dx,di           ; account for base offset...
        inc     cl              ; increment color
        jnz     @@CycleColors   ; keep going until we're done
@@draw:
        xchg    ax, cx          ; mov color into al
        pop     cx              ; retrieve our column counter
        pop     dx              ; fetch row (column already in cx)
        push    dx              ; must leave a copy on the stack
        xor     bx,bx           ; write to video page zero
   
        call    store_pixel
   
        inc     bp
        loop    @@CalcPixel
        inc     di
        pop     cx
        loop    @@CalcRow
   
        call   return_data
   
        popa
   
        ret
   
   
Shifty:
   
        push    cx
        db      0b1h
scale   db      STARTSCALE
        adc     cl,0
        imul    dx
   
        xchg    ax,dx
        shl     eax,16
        xchg    ax,dx
        shr     eax,cl
   
        pop     cx
        ret
   
   
pixel_pos: dd  data_area
   
store_pixel:
   
      pusha
   
      mov  ebx,[pixel_pos]
      shl  eax,3
      and  eax,0xff
      mov  [ebx],eax
      add  dword [pixel_pos],dword 3
   
      popa
      ret
   
   
return_data:
   
      mov  ecx,128 * 128/16
      mov  esi,data_area
   
    sd:
   
      pusha
   
      mov  eax,53              ; use the socket provided by host
      mov  ebx,4
      mov  ecx,[0]
      mov  edx,3*16
      int  0x40
   
      mov  eax,5
      mov  ebx,1
      int  0x40
   
      popa
   
      add  esi,3*16
   
      loop sd
   
      ret
   
   
data_area:
   
remote_code_end:
   
   
I_END:
   
   
   
   
   
   
   