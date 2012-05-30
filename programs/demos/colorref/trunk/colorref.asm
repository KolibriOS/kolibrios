;  COLORREF.ASM - COLOR REFERENCE
;
;  Compile with FASM for Menuet
;

use32
         org  0x0

         db  'MENUET01'            ; 8 byte id
         dd   0x01                 ; header version
         dd   start                ; start of code
         dd   finis                ; size of image
         dd   0x1000               ; memory for app
         dd   0x1000               ; esp
         dd   0x0,0x0              ; I_Param , I_Icon

include 'lang.inc'
include '..\..\..\macros.inc'
wide:    dd   0                    ; screen pixels width
mouse:   dd   0                    ; 1=right,2=left [mouse click]


start:

    mov  eax,14                    ; get screen size
    mcall
    shr  eax,16                    ; get width into AX
    inc  eax                       ; not 0 based
    mov  [wide],eax

    call draw_window

still:
    mov  eax,23                    ; wait for event w/timeout
    mov  ebx,5                     ; delay in hundredths
    mcall

    cmp  eax,1                     ; redraw request ?
    jne  s1
    jmp  red
s1: cmp  eax,2                     ; key in buffer ?
    jne  s2
    jmp  key
s2: cmp  eax,3                     ; button in buffer ?
    jne  s3
    jmp   button
s3: mov  eax,9                     ; process info function
    mov  ebx,stat_table            ; return data table
    mov  ecx,-1                    ; who am i
    mcall
    cmp  ax,[stat_table+4]         ; are we active?
    je   active                    ; yep
    jmp  still

  active:
    mov  eax,37                    ; mouse info function
    mov  ebx,2                     ; get buttons
    mcall
    cmp  eax,0                     ; mouse click?
    jne  click
    jmp  still
  click:
    mov  [mouse],eax               ; save mouse click
    mov  eax,37                    ; mouse info
    xor  ebx,ebx                   ; get screen pos for mouse
    mcall                      ; into EAX
    xor  ebx,ebx
    mov  bx,ax                     ; BX=y screen position
    shr  eax,16                    ; AX=x screen position
    xchg eax,ebx                   ; EAX=y, EBX=x
    dec  eax                       ; don't calc mouse scanline
    mov  ecx,[wide]                ; get pixels wide
    mul  ecx
    add  ebx,eax                   ; add x
    mov  eax,35                    ; get mouse pos pixel
    mcall                      ; EAX=mouse pixel color
    mov  ebx,eax                   ; EBX has color
    mov  esi,colors                ; color table
    mov  ecx,72                    ; total colors
    xor  edx,edx                   ; init a counter
  check:
    lodsd
    inc  edx                       ; update counter
    cmp  ebx,eax                   ; color match?
    je   _match                     ; yep
    loop check                     ; check all colors
    jmp still                      ; no match
  _match:
    cmp  [mouse],dword 1           ; right click?
    je   right                     ; yep
  left:
    cmp  [picks],edx               ; changed left color yet?
    jne  l1                        ; no, do it
    jmp  still
l1: mov  [picks],edx               ; update left pick color
    call clear                     ; erase old text
    call draw_picks                ; redraw colors and text
    jmp  still
  right:
    cmp  [picks+4],edx             ; changed right color yet?
    jne  r1                        ; no, do it
    jmp  still
r1: mov  [picks+4],edx             ; update right pick color
    call clear                     ; erase old text
    call draw_picks                ; redraw colors and text
    jmp  still

  red:                             ; redraw
    call draw_window
    jmp  still

  key:                             ; key
    mov  eax,2                     ; just read it and ignore
    mcall
    cmp  al,0                      ; key in buffer?
    je   k1                        ; yep
    jmp  still
k1: cmp  ah,'H'                    ; cap H ?
    je   k2                        ; yep
    cmp  ah,'h'                    ; locase h ?
    je   k2                        ; yep
    jmp  still
k2: call help                      ; show help screen
    jmp  still

  button:                          ; button
    mov  eax,17                    ; get id
    mcall
    cmp  ah,1                      ; button id=1 ?
    je   close
    jmp  still

  close:
    mov  eax,-1                    ; close this program
    mcall


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mcall 12,1

    mov  eax,0                     ; DRAW WINDOW
    mov  ebx,1*65536+200           ; [x start] *65536 + [x size]
    mov  ecx,200*65536+240         ; [y start] *65536 + [y size]
    mov  edx,0x14000000            ; work area color (type II)
    mov  edi,title                ; frame color
    mcall
   
    call palette                   ; display color palette

    mcall 12, 2

    ret


;   *********************************************
;   *******  COLOR PALETTE LAYOUT ROUTINES ******
;   *********************************************


palette:
    mov  ebx,15*65536+18           ; LAYOUT: start x and width
    mov  edx,32*65536+18           ; LAYOUT: start y and depth
    mov  ecx,8                     ; 8 rows
    mov  ebp,colors                ; color table
p1: push ecx
    mov  ecx,9                     ; 9 columns
p2: push ecx
    push edx
    mov  ecx,edx                   ; y coord
    mov  edx,[ebp]                 ; color
    mov  eax,13                    ; draw bar function
    mcall
    pop  edx
    pop  ecx
    add  ebx,19*65536              ; next column
    add  ebp,4                     ; next color
p3: loop p2
    pop  ecx
    mov  ebx,15*65536+18           ; restart x
    add  edx,19*65536              ; next row
    loop p1
    call draw_picks

    ret

draw_picks:
    mov  ebx,64*65536+24           ; draw x and width
    mov  ecx,188*65536+42          ; draw y and depth
    mov  edx,0xc0c0c0              ; color grey
    mov  eax,13                    ; draw bar function
    mcall
    mov  eax,[picks]               ; first picked color
    mov  esi,22*65536+196          ; print at x and y
    call do_hex                    ; print color number
    mov  eax,[picks+4]             ; second picked color
    mov  esi,22*65536+215          ; print at x and y
    call do_hex                    ; print color number
    mov  eax,[picks]               ; first picked color
    mov  ebx,67*65536+18           ; x and width
    mov  esi,191*65536+18          ; y and depth
    call do_color                  ; paint color 1 square
    mov  eax,[picks+4]             ; second picked color
    mov  ebx,67*65536+18           ; x and width
    mov  esi,209*65536+18          ; y and depth
    call do_color                  ; paint color 2 square
    mov  eax,[picks]               ; first picked color
    mov  ebx,96*65536+196          ; x and y
    call do_name                   ; print color's name
    mov  eax,[picks+4]             ; second picked color
    mov  ebx,96*65536+215          ; x and y
    call do_name                   ; print color's name

    ret

do_hex:
    dec  eax                       ; use 0 base
    mov  ecx,4                     ; dword length
    mul  ecx                       ; calc pointer
    mov  edi,colors                ; color table
    add  edi,eax                   ; add offset
    mov  ecx,[edi]                 ; save color 1
    mov  ebx,0x60100               ; print 6 hex digits
    mov  edx,esi                   ; copy color
    mov  esi,0xe1e1e1              ; use white
    mov  eax,47                    ; print number function
    mcall

    ret

do_color:
    dec  eax                       ; use 0 base
    mov  ecx,4                     ; dword length
    mul  ecx                       ; calc pointer
    mov  edi,colors                ; color table
    add  edi,eax                   ; add offset
    mov  edx,[edi]                 ; color
    mov  ecx,esi                   ; recover y an depth
    mov  eax,13                    ; draw bar function
    mcall

    ret

do_name:
    dec  eax                       ; use 0 base
    mov  ecx,15                    ; string length
    mul  ecx                       ; calc pointer
    mov  edx,names                 ; color table
    add  edx,eax                   ; add offset
    mov  ecx,0xe1e1e1              ; color
    mov  esi,15
    mov  eax,4                     ; print text function
    mcall

    ret

clear:
    mov  ebx,22*65536+36           ; x and width
    mov  ecx,196*65536+26          ; y and depth
    mov  edx,0x000000              ; color
    mov  eax,13                    ; draw bar funx
    mcall
    mov  ebx,96*65536+90           ; x and width
    mov  ecx,196*65536+26          ; y and depth
    mov  edx,0x000000              ; color
    mov  eax,13                    ; draw bar funx
    mcall

    ret

help:
	mcall 48,4
	mov ecx, eax
	shl ecx, 16
	add ecx, 236
	sub ecx, eax

	
    mov  ebx,5*65536+191           ; x and width
    mov  edx,0x465e8f              ; dark denim color
    mov  eax,13                    ; write text funx
    mcall
    mov  ebx,20*65536+40           ; starting x and y
    mov  edx,text                  ; start of text
    mov  esi,27                    ; width of text
    mov  ecx,14                    ; 14 text lines to do
    mov  eax,4                     ; write text funx
h1: push ecx
    sub  ebx,65537                 ; drop shadow x and y
    mov  ecx,0x000000              ; black shadow
    mcall
    add  ebx,65537                 ; original x and y
    mov  ecx,0xefefef              ; white text
    mcall
    add  edx,27                    ; next line of text
    add  bx,12                     ; next row
    pop  ecx
    loop h1
    mov  eax,10                    ; wait on event
    mcall
    cmp  eax,2                     ; got a key?
    jne  h2                        ; nope
    mov  eax,2                     ; yep, burn it
    mcall
h2: 
	call draw_window

    ret


;   *********************************************
;   **********  DATA DEFINITIONS AREA ***********
;   *********************************************

title    db   'COLOR REFERENCE H>HELP',0

picks:
    dd   31,2           ; selected top/bot colors

colors:
    dd   0xe0e0e0       ; white
    dd   0xe7e6a0       ; pale yellow
    dd   0xe7e05a       ; lemon yellow
    dd   0xe7c750       ; mustard
    dd   0xe7b850       ; cadium yellow
    dd   0xbfa461       ; yellow ocre
    dd   0xe0c090       ; cream
    dd   0xe0b27b       ; peach
    dd   0xe2986d       ; dark peach
    dd   0xebb2c0       ; pink
    dd   0xe0b0a0       ; flesh
    dd   0xc79790       ; artificial arm
    dd   0xb88688       ; deep blush
    dd   0xc4a077       ; washed khaki
    dd   0xb69269       ; khaki
    dd   0xa8845b       ; dark khaki
    dd   0xab937a       ; beige
    dd   0xa39370       ; poupon
    dd   0x988c00       ; camouflage
    dd   0x98a024       ; pale olive
    dd   0x838b00       ; olive
    dd   0x6d7600       ; dark olive
    dd   0x5b6200       ; black olive
    dd   0x94946a       ; washed army
    dd   0x74744a       ; army
    dd   0x66a696       ; pale teal
    dd   0x409b90       ; faded teal
    dd   0x008d8d       ; pastel teal
    dd   0x007c7c       ; teal
    dd   0x006464       ; dark teal
    dd   0x00b8ca       ; light turquoise
    dd   0x00a0b2       ; turquoise
    dd   0x00889a       ; dark turquoise
    dd   0x575f8c       ; medium cobalt
    dd   0x4e4e7c       ; cobalt
    dd   0x00459a       ; ultramarine
    dd   0x400088       ; navy blue
    dd   0x4e00e7       ; true blue
    dd   0x508cec       ; sky blue
    dd   0x6a73d0       ; mountain blue
    dd   0x677ab0       ; faded jeans
    dd   0x576fa0       ; denim
    dd   0xd048c8       ; fuschia
    dd   0xb800e7       ; lavendar
    dd   0xa800a8       ; light violet
    dd   0x780078       ; violet
    dd   0x520064       ; purple
    dd   0xb800b8       ; magenta
    dd   0xa4307a       ; rose
    dd   0x90207f       ; mauve
    dd   0xe76e83       ; salmon
    dd   0xea7a7d       ; pastel orange
    dd   0xe26830       ; orange
    dd   0xac5800       ; burnt sienna
    dd   0xcc0000       ; red orange
    dd   0xac0000       ; cadium red
    dd   0x880040       ; brick red
    dd   0x780000       ; rust
    dd   0x683020       ; terra cotta
    dd   0x7f4658       ; light maroon
    dd   0x702050       ; maroon
    dd   0x7a5b5f       ; umber blush
    dd   0x584838       ; burnt umber
    dd   0x8a5d1a       ; cigar brown
    dd   0x64504a       ; ice brown
    dd   0x564242       ; dark chocolate
    dd   0x00aa66       ; celery stalk
    dd   0x107a30       ; forest green
    dd   0x365800       ; hooker's green
    dd   0x8beb88       ; pastel lime
    dd   0x7bbb64       ; lime
    dd   0x4ba010       ; dark lime

names:
    db   'WHITE          '
    db   'PALE YELLOW    '
    db   'LEMON YELLOW   '
    db   'MUSTARD        '
    db   'CADIUM YELLOW  '
    db   'YELLOW OCRE    '
    db   'CREAM          '
    db   'PEACH          '
    db   'DARK PEACH     '
    db   'PINK           '
    db   'FLESH          '
    db   'ARTIFICIAL ARM '
    db   'DEEP BLUSH     '
    db   'WASHED KHAKI   '
    db   'KHAKI          '
    db   'DARK KHAKI     '
    db   'BEIGE          '
    db   'POUPON         '
    db   'CAMOUFLAGE     '
    db   'PALE OLIVE     '
    db   'OLIVE          '
    db   'DARK OLIVE     '
    db   'BLACK OLIVE    '
    db   'WASHED ARMY    '
    db   'ARMY           '
    db   'PALE TEAL      '
    db   'FADED TEAL     '
    db   'PASTEL TEAL    '
    db   'TEAL           '
    db   'DARK TEAL      '
    db   'LIGHT TURQUOISE'
    db   'TURQUOISE      '
    db   'DARK TURQUOISE '
    db   'MEDIUM COBALT  '
    db   'COBALT         '
    db   'ULTRAMARINE    '
    db   'NAVY BLUE      '
    db   'TRUE BLUE      '
    db   'SKY BLUE       '
    db   'MOUNTAIN BLUE  '
    db   'FADED JEANS    '
    db   'DENIM          '
    db   'FUSHIA         '
    db   'LAVENDAR       '
    db   'LIGHT VIOLET   '
    db   'VIOLET         '
    db   'PURPLE         '
    db   'MAGENTA        '
    db   'ROSE           '
    db   'MAUVE          '
    db   'SALMON         '
    db   'PASTEL ORANGE  '
    db   'ORANGE         '
    db   'BURNT SIENNA   '
    db   'RED ORANGE     '
    db   'CADIUM RED     '
    db   'BRICK RED      '
    db   'RUST           '
    db   'TERRA COTTA    '
    db   'LIGHT MAROON   '
    db   'MAROON         '
    db   'UMBER BLUSH    '
    db   'BURNT UMBER    '
    db   'CIGAR BROWN    '
    db   'ICE BROWN      '
    db   'DARK CHOCOLATE '
    db   'CELERY STALK   '
    db   'FOREST GREEN   '
    db   "HOOKER'S GREEN "
    db   'PASTEL LIME    '
    db   'LIME           '
    db   'DARK LIME      '


text:
    db   'TO SEE HOW COLORS COMPARE  '
    db   'TO ONE ANOTHER, LEFT CLICK '
    db   'THE FIRST COLOR AND RIGHT  '
    db   'CLICK THE SECOND. TO GET   '
    db   "A SENSE OF A COLOR'S TRUE  "
    db   'HUE, RIGHT AND LEFT CLICK  '
    db   'THE SAME COLOR TO SEE IT   '
    db   'ON THE NEUTRAL BACKGROUND. '
    db   'TO USE A LIGHTER OR DARKER '
    db   'VALUE OF A COLOR, ADD OR   '
    db   'SUBTRACT 0x10 OR 0x20 FROM '
    db   'EACH BYTE OF ITS HEX VALUE.'
    db   '                           '
    db   '       ANY KEY ...         '

stat_table:


finis: