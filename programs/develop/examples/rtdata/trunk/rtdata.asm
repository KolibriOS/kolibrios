;
;   COMMUNICATING WITH MODEM: PORTS & IRQ
;
;   Compile with FASM for Menuet
;

include "lang.inc"
include "macros.inc"

  use32
  org    0x0

  db     'MENUET01'  ; 8 byte id
  dd     0x01        ; header version
  dd     START       ; start of code
  dd     I_END       ; size of image
  dd     0x1000      ; memory for app
  dd     0x1000      ; esp
  dd     0x0 , 0x0   ; I_Param , I_Icon


START:                          ; start of execution


    mov  eax,45                 ; reserve irq 4
    mov  ebx,0
    mov  ecx,4
    int  0x40

    mov  eax,46                 ; reserve ports 0x3f8-0x3ff
    mov  ebx,0
    mov  ecx,0x3f8
    mov  edx,0x3ff
    int  0x40

    mov  eax,44                 ; read these ports at interrupt/irq 4
    mov  ebx,irqtable
    mov  ecx,4
    int  0x40

    mov  eax,40                 ; enable event for interrupt/irq 4
    mov  ebx,10000b shl 16 + 111b
    int  0x40

    call program_com1

    call draw_window

still:

    mov  eax,10                 ; wait here for event
    int  0x40

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button
    cmp  eax,16+4               ; data read by interrupt ?
    je   irq4

    jmp  still

  red:                          ; redraw
    call draw_window
    jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    int  0x40

    mov  al,ah
    mov  dx,0x3f8
    out  dx,al

    jmp  still

  button:                       ; button
    or   eax,-1                 ; close this program
    int  0x40


  irq4:

    mov  eax,42
    mov  ebx,4
    int  0x40

    ; eax = number of bytes left
    ; ecx = 0 success, =1 fail
    ; bl  = byte

    inc   [pos]
    and   [pos],31
    mov   eax,[pos]

    mov   [string+eax], bl
    call  draw_string

    jmp  still


baudrate_9600   equ 12
baudrate_57600  equ  2

program_com1:

    mov  dx,0x3f8+3
    mov  al,0x80
    out  dx,al

    mov  dx,0x3f8+1
    mov  al,0x00
    out  dx,al

    mov  dx,0x3f8+0
    mov  al,baudrate_9600
    out  dx,al

    mov  dx,0x3f8+3
    mov  al,0x3
    out  dx,al

    mov  dx,0x3f8+4
    mov  al,0xb
    out  dx,al

    mov  dx,0x3f8+1
    mov  al,0x1
    out  dx,al

    ret



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax, 48
    mov  ebx, 3
    mov  ecx, sc
    mov  edx, sizeof.system_colors
    int  0x40

    mov  eax, 12                   ; function 12:tell os about windowdraw
    mov  ebx, 1                    ; 1, start of draw
    int  0x40

                                   ; DRAW WINDOW
    mov  eax, 0                    ; function 0 : define and draw window
    mov  ebx, 100*65536+250        ; [x start] *65536 + [x size]
    mov  ecx, 100*65536+85         ; [y start] *65536 + [y size]
    mov  edx, [sc.work]
    or   edx, 0x03000000           ; color of work area RRGGBB,8->color gl
    int  0x40

                                   ; WINDOW LABEL
    mov  eax, 4                    ; function 4 : write text to window
    mov  ebx, 8*65536+8            ; [x start] *65536 + [y start]
    mov  ecx, [sc.grab_text]
    or   ecx, 0x10000000           ; font 1 & color ( 0xF0RRGGBB )
    mov  edx, header               ; pointer to text beginning
    mov  esi, header.len           ; text length
    int  0x40

    mov  eax, 4                    ; draw text
    mov  ebx, 20*65536+33
    mov  ecx, [sc.work_text]
    mov  edx, text+4
  .nextstr:
    mov  esi, [edx-4]
    test esi, 0xFF000000
    jnz  .finstr
    int  0x40
    add  edx, esi
    add  edx, 4
    add  ebx, 10
    jmp  .nextstr
  .finstr:

    call draw_string

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    int  0x40

    ret


draw_string:
    mov  eax, 4
    mov  ebx, 20*65536+65
    mov  ecx, [sc.work_text]
    mov  edx, string
    mov  esi, 32
    int  0x40
ret


; DATA AREA


if lang eq ru
   text mstr "‚‚„ˆŒ›… ‘ˆŒ‚‹› ……„€’‘Ÿ Œ„…Œ“.",\
             "„€›… ’ Œ„…Œ€ ‘—ˆ’›‚€’‘Ÿ ",\
             "…›‚€ˆ IRQ4 ˆ ’€†€’‘Ÿ ˆ†…."
   header:
        db   'Œ„…Œ € COM1'
    .len = $ - header
else
   text mstr "TYPED CHARACTERS ARE SENT TO MODEM.",\
             "DATA FROM MODEM IS READ BY IRQ4",\
             "INTERRUPT AND DISPLAYED BELOW."
   header:
        db   'MODEM AT COM1'
    .len = $ - header
end if

pos  dd  0x0

irqtable:
       ; port    ; 1=byte, 2=word
  dd   0x3f8 +0x01000000   ; read byte from port 0x3f8 at interrupt/irq 4
  dd   0x0                 ; no more ports ( max 15 ) to read


I_END:

string rb 32
sc system_colors
