; tinyfrac.asm
;
; teeny program displays the Mandelbrot set.
;
; written on Sun  03-26-1995  by Ed Beroset (Fidonet 1:3641/1.250)
;
; This program was based on a program by Frank Hommers, later optimized
; for size by Mikko Hyvarinen and posted in Fidonet's 80XXX echo.
;
; This new version has many new features and was based on my own
; optimization of Hyvarinen's version.  Some features:
;
; pan     using the arrow keys, one can navigate the fractal.
;
;               Home  Up  PgUp
;               Left      Right   correspond to 8 obvious directions
;               End   Dn  PgDn
;
; zoom    there are now ten levels of magnification available.  If the
;         program is assembled with FEATURES defined, the number
;         corresponding to the zoom level (0-9, zero is most zoomed in)
;         is displayed in the upper left hand corner of the screen just
;         before each new fractal is drawn.  The gray '+' key zooms out,
;         the gray '-' key zooms in.
;
; beep    the program will beep at the completion of each fractal
;         drawing or if the user attempts to zoom past either limit.
;
; mode    if the program is assembled with MODECHANGE defined, the
;         will change to the next video mode if the 'v' key is pressed.
;         This is handy because drawing fractals at high resolution can
;         be very timeconsuming.  The user can find an interesting spot
;         in a low res mode and then change to a high res mode to see it
;         more fully rendered.
;
; size    this whole project was started off as a size optimization
;         exercise, so there have been some rather ugly tradeoffs to
;         sacrifice speed for size.
;
; 8086    yes, it runs on an 8086 although only if you leave out either
;         the FEATURES option or the MODECHANGE option and it would be
;         slower and more painful than oral surgery.
;
; cost    there IS such a thing as a free lunch!  This code is hereby
;         released to the public domain by the author.
;
;
; to assemble &amp; link:
;   TASM /m2 tinyfrac       (assemble using two pass mode if required)
;   TLINK /Tdc tinyfrac     (link Target platform is DOS, COM file)
;
;

PIXWIDTH    equ 511
PIXHEIGHT   equ 255

ZOOMLIMIT   equ  13       ; can change to up to 13 for extended zoom in

; feel free to experiment with the following constants:

DELTA       equ 200       ; the unit of pan movement in pixels
THRESHOLD   equ  7        ; must be in the range of (0,255)
STARTSCALE  equ  5        ; a number from 0 to ZOOMLIMIT, inclusive
CHAR_COLOR  equ 0fh       ; white on black background (for PRINTZOOM feature)



; ************************************************************
;
;   Menuet header


use32

                  org     0x0

                  db      'MENUET01'
                  dd      0x01
                  dd      START
                  dd      I_END
                  dd      0x62000
                  dd      0x1000
                  dd      0,0

include 'lang.inc'
include '..\..\..\macros.inc'

STARTX  dd  200
STARTY  dd  120

scaleaddy dd 120
scaleaddx dd 200

START:

red:
        call    draw_window
        call    draw_fractal

still:

        mov  eax,10
        mcall

        dec  eax
        jz   red
        dec  eax
        jz   key

      button:
        mov  al,17
        mcall

        cmp  ah,1
        jne  no_close
        or   eax,-1
        mcall
      no_close:

        cmp  ah,2
        jne  no_bgr


        mov  eax,15   ; bgr 512 x 256
        mov  ebx,1
        mov  ecx,512
        mov  edx,256
        mcall

        mov  eax,15
        mov  ebx,5
        mov  ecx,0x1000
        mov  edx,0
        mov  esi,512*3*256
        mcall

        mov  eax,15
        mov  ebx,3
        mcall

        jmp  still

      no_bgr:

        cmp  ah,3
        jb   no_color
        cmp  ah,5
        jg   no_color
        shr  eax,8
        sub  eax,3
        imul eax,8
        add  eax,8
        not  eax
        and  eax,11000b
        mov  [shlc],al
        call draw_fractal
        jmp  still

      no_color:

        jmp  still


      key:
        mov  al,2
        mcall

        cmp  ah,'e'
        je   cycle
        cmp  ah,'r'
        je   cycle
        jmp  no_cycle
      cycle:
        call color_cycle
        jmp  still
      no_cycle:

        cmp  ah,'q'
        jne  no_in
        inc  byte [scale]
        mov  ebx,[STARTX]
        imul ebx,2
        sub  ebx,[scaleaddx]
        mov  [STARTX],ebx
        mov  ebx,[STARTY]
        imul ebx,2
        sub  ebx,[scaleaddy]
        mov  [STARTY],ebx
      no_in:

        cmp  ah,'w'
        jne  no_out
        dec  byte [scale]
        mov  ebx,[STARTX]
        add  ebx,[scaleaddx]
        shr  ebx,1
        mov  [STARTX],ebx
        mov  ebx,[STARTY]
        add  ebx,[scaleaddy]
        shr  ebx,1
        mov  [STARTY],ebx
      no_out:

        cmp  ah,130+48
        jne  no_up
        sub  [STARTY],100
      no_up:

        cmp  ah,129+48
        jne  no_down
        add  [STARTY],100
      no_down:

        cmp  ah,128+48
        jne  no_left
        sub  [STARTX],100
      no_left:

        cmp  ah,131+48
        jne  no_right
        add  [STARTX],100
      no_right:

        call draw_fractal
        jmp  still

color_cycle:

     pusha
     mov  ecx,0x08080808
     mov  esi,(256/8)*5
     cmp  ah,'e'
     je   f_out
     mov  ecx,-0x08080808
     mov  esi,(256/8)*5-1
   f_out:

   newcycle:
     mov  edi,0x1000
   newpix:
     mov  eax,[edi]
     add  eax,ecx
     mov  [edi],eax
     add  edi,4
     cmp  edi,0x1000+512*256*3
     jb   newpix
     call put_image
     mov  eax,5
     mov  ebx,1
     mcall
     dec  esi
     jnz  newcycle

     mov  eax,0
     mov  edi,0x1000
     mov  ecx,512*256*3 / 4 +50
     cld
     rep  stosd

     popa

     call draw_fractal

     ret





; **********************************************************************
;
;    Tinyfrac
;


draw_fractal:

        pusha
        mov     eax,4
        mov     ebx,10*65536+30
        mov     ecx,0x80ffffff
        mov     edx,calc
        mcall
        popa
        pusha

        movzx   ebp,word [STARTX]
        movzx   edi,word [STARTY]


;       This routine is the fractal drawing engine.  It has been
;       optimized for size, sacrificing speed.

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

        call    put_pixel

        inc     bp
        loop    @@CalcPixel
        inc     di
        pop     cx
        loop    @@CalcRow

        call    put_image

        popa

        ret


put_image:

        pusha

        mov  eax,7
        mov  ebx,0x1000
        mov  ecx,512*65536+256
        mov  edx,4*65536+21
        mcall

        popa

        ret


shlc db 0

put_pixel:

        pusha
        sub     edi,[STARTY]
        sub     ebp,[STARTX]
        and     edi,0xff
        and     ebp,0x1ff
        shl     edi,9
        mov     ebx,edi ; * 3 - Y
        add     edi,ebx
        add     edi,ebx
        mov     ebx,ebp
        add     ebp,ebx
        add     ebp,ebx
        add     edi,ebp
        mov     cl,[shlc]
        mov     ebx,0xff
        shl     ebx,cl
        add     cl,3
        shl     eax,cl
        and     eax,ebx
        mov     [0x1000+edi],eax
        popa

        ret


;****************************************************************************
 ;
 ;       This routine multiplies AX by DX and shifts the result (in
;       DX:AX) to the right by scale bits (or scale+1 bits if CY is
;       set).  The resulting value is left in AX.  DX is destroyed.
;
;****************************************************************************

Shifty:
        push    cx              ; save middle bits (i*i - j*j)
        db      0b1h            ; code for mov cl,immed8
scale   db      STARTSCALE
        adc     cl,0            ; adjust per CY flag
        imul    dx              ; do the multiply

        xchg    ax,dx           ;
        shl     eax,16          ; put hi part in hi 16 bits
        xchg    ax,dx
        shr     eax,cl          ;

        pop     cx              ;
        ret                     ;



; **********************************************************************
;
;                   WINDOW DEFINITIONS AND DRAW
;
; **********************************************************************



draw_window:

      pusha

      mov  eax,12
      mov  ebx,1
      mcall

      xor  eax,eax
      mov  ebx,50*65536+PIXWIDTH+8
      mov  ecx,100*65536+PIXHEIGHT+25
      mov  edx,0x14334455
      mov  edi,title
      mcall

      mov  eax,8
      mov  ebx,290*65536+112
      mov  ecx,5*65536+12
      mov  edx,2
      mov  esi,0x808080
      mcall

      mov  ebx,420*65536+12
      mov  ecx,5*65536+12
      mov  edx,3
      mov  esi,0xa00000
      mov  edi,3
      ;mov  eax,8
    newcolor:
      mcall
      add  ebx,13*65536
      shr  esi,8
      inc  edx
      dec  edi
      jnz  newcolor

      mov  eax,4
      mov  ebx,300*65536+8
      mov  ecx,0x80ffffff
      mov  edx,button_txt
      mcall

      mov  eax,12
      mov  ebx,2
      mcall

      popa
      ret


; ***************************************************************
;
;     DATA AREA
;


title      db 'Tinyfrac - MOVE: ARROWS, ZOOM Q/W, CYCLE: E/R',0
button_txt  db 'Set as wallpaper',0

calc        db 'CALCULATING',0

I_END: