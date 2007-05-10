;
;   Memory Blocks for Menuet v0.1
;   Crown Soft (c) crown_s@rambler.ru
;
;   Compile with FASM for Menuet
;

use32

               org    0x0

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     0x4000                  ; memory for app
               dd     0x4000                  ; esp
               dd     0x0 , 0x0               ; I_Param , I_Icon

include '..\..\..\macros.inc'

title  db   'Memory Blocks for Menuet v0.1  Crown Soft (c)',0


START:                          ; start of execution
    call initpict

still:
    mov  eax,10                 ; wait here for event
    mcall

    cmp  eax,1                  ; redraw request ?
    je   red
    cmp  eax,2                  ; key in buffer ?
    je   key
    cmp  eax,3                  ; button in buffer ?
    je   button
jmp  still

  red:                          ; redraw
    call draw_window
  jmp  still

  key:                          ; key
    mov  eax,2                  ; just read it and ignore
    mcall
  jmp  still

  button:                       ; button
    mov  eax,17                 ; get id
    mcall

    cmp  ah,1                   ; button id=1 ?
    jz   close

    cmp  ah,100                 ; button id=100 ?
    jz   init

    movzx ebx,ah

    dec  bl
    dec  bl                     ; bl -number bitton
    mov  al,[bitstat+ebx*1]
    cmp  al,1                   ; is pictures on bitton?
    jz   still

    inc  [nkeydown]

    cmp  [firstbit],0xff
    jz   tofirst

    cmp  [secondbit],0xff
    jz   tosecond


    movzx  eax,[firstbit]
    mov  [bitstat+eax*1],0

    mov  al,[secondbit]
    mov  [bitstat+eax*1],0

    mov  [secondbit],0xff


  tofirst:
    mov  [firstbit],bl
    mov  [bitstat+ebx*1],1
    call draw_window
  jmp  still

  tosecond:
    mov  [secondbit],bl
    mov  [bitstat+ebx*1],1

    mov  al,[bitpict+ebx*1]
    mov  bl,[firstbit]          ; comply pictures on first bitton
    cmp  [bitpict+ebx*1],al     ;    with pictures on second
    jnz  tosm1
      mov [firstbit] ,0xff
      mov [secondbit],0xff
    tosm1:

    call draw_window
  jmp  still

  init:
    call initpict
  jmp still

  close:
    mov  eax,-1                 ; close program
    mcall



;======================================================================
;===============  WINDOW DEFINITIONS AND DRAW  ========================
;======================================================================
draw_window:
    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,1                     ; 1, start of draw
    mcall

                                   ; DRAW WINDOW
    mov  eax,0                     ; function 0 : define and draw window
    mov  ebx,100*65536+413         ; [x start] *65536 + [x size]
    mov  ecx,100*65536+295         ; [y start] *65536 + [y size]
    mov  edx,0x93000000            ; color of work area RRGGBB,8->color gl
    mov  edi,title                 ; WINDOW LABEL
    mcall

                                   
                                   ; init BUTTON
    mov  eax,8                     ; function 8 : define and draw button
    mov  ebx,10*65536+55           ; [x start] *65536 + [x size]
    mov  ecx,270*65536+12          ; [y start] *65536 + [y size]
    mov  edx,100                   ; button id
    mov  esi,0x6688dd              ; button color RRGGBB
    mcall
                                   ; NEW GAME LABEL
    mov  eax,4                     ; function 4 : write text to window
    mov  ebx,15*65536+273          ; [x start] *65536 + [y start]
    mov  ecx,0x00ddeeff            ; color of text RRGGBB
    mov  edx,labnew                ; pointer to text beginning
    mov  esi,labnewlen-labnew      ; text length
    mcall

    mov  ebx,40001h
    mov  ecx,nkeydown
    mov  edx,135*65536+273
    mov  esi,0x00ffffff
    mov  eax,47
    mcall


    ;--- draw buttons ---
    mov  [bitid],2
    mov  ecx,10
    drm1:
      mov ebx,ecx
      mov ecx,6
      drm2:
        pushad
        ; BUTTON
        imul bx,bx,40
        sub  bx,30
        shl  ebx,16
        mov  bx,34                 ; ebx=[x start]*65536+[x size]

        imul cx,cx,40
        sub  cx,13
        shl  ecx,16
        mov  cx,34                 ; ecx=[y start]*65536+[y size]

        movzx  edx,[bitid]         ; button id

        mov  esi,[coltbl+16*4]     ; button color RRGGBB
        mov  eax,8                 ; function 8 : define and draw button
        mcall

        ; PICTURE
        dec  edx
        dec  edx
                                   ; edx - number of bitton
        mov  al,[bitstat+edx*1]
        cmp  al,0
        jz   drm3
          movzx  eax,[bitpict+edx*1]  ; al - number of picture
          call unpack

          mov  edx,ebx
          shr  ecx,16
          mov  dx,cx               ; ecx=[x size]*65536+[y size]
          add  edx,10001h

          mov  ebx,mas
          mov  ecx,32*65536+32     ; image size
          mov  eax,7               ; function 7 : putimage
          mcall
        drm3:

        inc [bitid]
        popad
      loop drm2
      mov ecx,ebx
    loop drm1

    mov  eax,12                    ; function 12:tell os about windowdraw
    mov  ebx,2                     ; 2, end of draw
    mcall

    ret


;========== put pictures number eax to mas ==================================
unpack:
     pushad
     cld
     mov esi,[pict+eax*4]

     mov  edi, mas

     lodsb               ; al - mask color
     mov  dl,al          ; dl - mask color

     xor  ecx,ecx
     m2:
       xor  eax,eax
       lodsb
       mov  cl,al
       and  cl,0Fh
       shr  al,4

       cmp  al,dl        ; is color mask?
       jnz  m5
         mov al,16
       m5:

       cmp  cl,ch        ; ch=00h
       jz   m1
       mov  eax,[coltbl+eax*4]
       m3:
         stosd
         dec edi
       loop m3
     jmp m2

     m1:

     mov eax,[coltbl+16*4]
     m7:
     cmp edi,1024*3+mas
     jnb m6
       stosd
       dec edi
     jmp m7

     m6:
     popad
ret
;============================================================================

;============================= initpict =====================================
initpict:
  pushad
  mov edi,nkeydown

  mov ecx,60+4
  xor al,al
  rep stosb           ; nkeydown=0   bitstat=0

  mov ecx,60+2
  mov al,0ffh
  rep stosb           ; bitpict=0ffh   firstbit=0ffh   secondbit=0ffh

  ; Initialize RND
  mov   eax,3
  mcall          ; eax=00SSMMHH
  rol   eax,16        ; eax=MMHH00SS - fist random number

  mov   ebx,8088405h

  mov  ecx,30
  ck1:
    ck2:
      mul   ebx       ; compute next random number
      inc   eax       ; new := 8088405h * old + 1

      movzx  edx,al
      and  dl,3fh

      cmp  dl,59
      ja   ck2

      cmp  [bitpict+edx*1],0ffh
    jnz  ck2
    dec  cl
    mov  [bitpict+edx*1],cl
    inc  cl

    mul   ebx       ; compute next random number
    inc   eax       ; new := 8088405h * old + 1

    ck3:
      mul   ebx       ; compute next random number
      inc   eax       ; new := 8088405h * old + 1

      movzx  edx,al
      and  dl,3fh

      cmp  dl,59
      ja   ck3

      cmp  [bitpict+edx*1],0ffh
    jnz  ck3
    dec  cl
    mov  [bitpict+edx*1],cl
    inc cl

  loop  ck1

  call draw_window
  popad
ret

;======================================================================
;======================== DATA AREA ===================================
;======================================================================



;----------------------- Compressed pictures --------------------------

key1 db 1 ; this color will background
db 01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,01Dh ; each byte is 0CNh
db 005h,01Fh,01Ah,002h,0F1h,071h,0F1h,071h ;   there C - color
db 0F1h,002h,01Fh,017h,001h,071h,0F1h,071h ;         N - number pixels
db 0F1h,071h,0F1h,071h,0F1h,071h,001h,01Fh ;
db 011h,002h,012h,001h,071h,0F1h,071h,0F1h ; byte 0X0h - end of picture
db 071h,0F1h,071h,0F1h,071h,0F1h,071h,001h
db 01Fh,001h,0F1h,001h,011h,001h,0F1h,071h
db 0F1h,071h,0F1h,071h,0F1h,071h,0F1h,071h
db 0F1h,001h,012h,00Eh,071h,002h,072h,0F1h
db 071h,0F1h,071h,0F1h,071h,003h,071h,0F1h
db 002h,0F1h,071h,0F1h,071h,0F1h,071h,0F1h
db 071h,0F1h,071h,0F1h,071h,0F1h,077h,0F1h
db 071h,0F1h,071h,001h,083h,001h,071h,002h
db 07Fh,074h,0F1h,071h,0F1h,071h,0F1h,001h
db 013h,001h,0F1h,001h,011h,001h,07Fh,074h
db 0F1h,071h,0F1h,071h,001h,013h,001h,071h
db 001h,012h,00Fh,001h,072h,0F1h,071h,0F1h
db 071h,0F1h,071h,003h,071h,0F1h,001h,013h
db 001h,082h,003h,084h,002h,081h,001h,011h
db 001h,072h,0F1h,071h,0F1h,071h,0F1h,071h
db 0F1h,071h,0F1h,001h,015h,002h,013h,004h
db 011h,002h,012h,001h,071h,0F1h,071h,0F1h
db 071h,0F1h,071h,0F1h,071h,0F1h,071h,001h
db 01Fh,015h,001h,071h,0F1h,071h,0F1h,071h
db 0F1h,071h,0F1h,071h,001h,01Fh,016h,081h
db 002h,0F1h,071h,0F1h,071h,0F1h,002h,081h
db 01Fh,011h,081h,013h,081h,011h,081h,011h
db 005h,011h,081h,011h,081h,01Fh,011h,081h
db 013h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,014h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,012h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,013h,081h,011h,082h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 015h,081h,012h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,013h,081h,011h,081h
db 012h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,014h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,013h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,011h,081h
db 016h,081h,015h,081h,011h,081h,011h,081h
db 013h,081h,011h,081h,011h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,01Fh,015h
db 081h,011h,081h,011h,081h,011h,081h,011h
db 081h,011h,081h,01Fh,017h,081h,011h,081h
db 011h,081h,011h,081h,011h,081h,01Fh,01Bh
db 081h,011h,081h,000h

globe db 1
db 01Ch,008h,01Fh,016h,003h,0C2h,0A1h,0C5h
db 003h,01Fh,011h,002h,0A4h,0C5h,0A2h,0C3h
db 002h,01Dh,001h,0A5h,0C4h,0A2h,0C1h,0A1h
db 0C4h,0A1h,001h,01Bh,001h,0A6h,0C4h,0A2h
db 0C1h,0A1h,0C4h,0A2h,001h,019h,001h,0A8h
db 0C1h,0A1h,0C1h,0A4h,0C4h,0A3h,001h,017h
db 001h,0AFh,0C6h,0A3h,001h,015h,001h,0C2h
db 0AEh,0C6h,0A4h,001h,014h,001h,0C2h,0AEh
db 0C7h,0A1h,0C2h,001h,013h,001h,0C3h,0ADh
db 0CBh,0A1h,001h,012h,001h,0C3h,0ADh,0CAh
db 0A2h,001h,012h,001h,0C3h,0A7h,0C1h,0A5h
db 0C9h,0A3h,001h,011h,001h,0C5h,0A6h,0C1h
db 0A2h,0C1h,0A2h,0C9h,0A4h,002h,0C6h,0A3h
db 0C7h,0A2h,0C8h,0A4h,002h,0C6h,0A4h,0C7h
db 0A1h,0C9h,0A3h,002h,0C7h,0A3h,0CFh,0C3h
db 0A2h,002h,0C9h,0A2h,0CFh,0C3h,0A1h,002h
db 0CBh,0A3h,0CFh,0A1h,002h,0CBh,0A3h,0CFh
db 0C1h,002h,0CDh,0A2h,0C1h,0A3h,0CBh,001h
db 011h,001h,0CCh,0AAh,0C6h,001h,012h,001h
db 0CCh,0ABh,0C5h,001h,012h,001h,0CCh,0ABh
db 0C5h,001h,013h,001h,0CBh,0ABh,0C4h,001h
db 014h,001h,0CDh,0A7h,0C6h,001h,015h,001h
db 0CCh,0A7h,0C5h,001h,017h,001h,0CDh,0A4h
db 0C5h,001h,019h,001h,0CCh,0A3h,0C5h,001h
db 01Bh,001h,0CBh,0A3h,0C4h,001h,01Dh,002h
db 0CAh,0A2h,0C2h,002h,01Fh,011h,003h,0C8h
db 003h,01Fh,016h,008h,000h

wa db 1
db 01Ch,008h,01Fh,016h,00Eh,01Fh,011h,004h
db 031h,0B8h,031h,004h,01Dh,003h,0BEh,003h
db 01Bh,002h,031h,0B3h,031h,008h,031h,0B3h
db 031h,002h,019h,002h,0B5h,00Ah,0B5h,002h
db 017h,002h,0B6h,00Ah,0B6h,002h,015h,002h
db 031h,0B7h,008h,0B7h,031h,002h,014h,002h
db 0B8h,008h,0B8h,002h,013h,002h,0BAh,006h
db 0BAh,002h,012h,002h,0BAh,006h,0BAh,002h
db 012h,001h,031h,0BBh,004h,0BBh,031h,001h
db 011h,002h,0BFh,0BDh,004h,0BCh,031h,002h
db 031h,0BCh,004h,0BBh,031h,004h,031h,0BBh
db 004h,0BBh,006h,0BBh,004h,0B2h,007h,031h
db 0B1h,006h,0B1h,031h,007h,0B2h,004h,0B2h
db 008h,0B1h,031h,004h,031h,0B1h,008h,0B2h
db 004h,0B2h,008h,031h,0B1h,031h,002h,031h
db 0B1h,031h,008h,0B2h,004h,0B2h,009h,0B6h
db 009h,0B2h,002h,011h,001h,031h,0B2h,007h
db 0B8h,007h,0B2h,031h,001h,012h,002h,0B2h
db 007h,0B8h,007h,0B2h,002h,012h,002h,0B3h
db 005h,0BAh,005h,0B3h,002h,013h,002h,0B3h
db 004h,0BAh,004h,0B3h,002h,014h,002h,031h
db 0B3h,002h,0BCh,002h,0B3h,031h,002h,015h
db 002h,0BFh,0B7h,002h,017h,002h,0BFh,0B5h
db 002h,019h,002h,031h,0BFh,0B1h,031h,002h
db 01Bh,003h,0BEh,003h,01Dh,004h,031h,0B8h
db 031h,004h,01Fh,011h,00Eh,01Fh,016h,008h
db 000h

sword db 2
db 02Fh,02Fh,023h,003h,02Fh,02Eh,001h,072h
db 001h,02Fh,02Dh,001h,072h,0F1h,001h,02Fh
db 02Dh,001h,0F1h,071h,0F1h,001h,02Fh,02Dh
db 001h,0F3h,001h,02Fh,02Dh,001h,0F3h,001h
db 02Fh,02Dh,001h,0F3h,001h,02Fh,02Dh,001h
db 0F3h,001h,02Fh,02Dh,001h,0F2h,071h,001h
db 02Fh,02Dh,001h,073h,001h,02Fh,02Dh,001h
db 073h,001h,02Fh,02Dh,001h,073h,001h,02Fh
db 02Dh,001h,073h,001h,02Fh,02Dh,001h,073h
db 001h,02Fh,02Dh,001h,072h,081h,001h,02Fh
db 02Dh,001h,083h,001h,02Fh,02Dh,001h,083h
db 001h,02Fh,02Dh,001h,083h,001h,02Fh,02Dh
db 001h,083h,001h,024h,001h,02Fh,028h,001h
db 083h,001h,022h,001h,02Fh,02Ah,001h,083h
db 002h,02Fh,02Ch,001h,082h,002h,02Fh,02Dh
db 002h,032h,001h,02Fh,02Ch,002h,031h,011h
db 0B1h,001h,02Fh,02Ah,001h,022h,001h,0B1h
db 031h,071h,001h,02Fh,028h,001h,024h,001h
db 071h,031h,0F1h,001h,02Fh,02Dh,001h,0F3h
db 001h,02Fh,02Dh,001h,0F2h,001h,02Fh,02Eh
db 002h,000h

cow db 1
db 018h,004h,01Fh,01Ch,004h,01Ch,003h,01Ch
db 001h,0B2h,001h,01Eh,002h,01Bh,002h,0B1h
db 001h,01Fh,003h,01Ah,001h,0B2h,001h,01Fh
db 001h,0B1h,001h,01Ah,001h,0B2h,001h,015h
db 004h,016h,001h,0B1h,001h,01Ah,001h,0B2h
db 002h,012h,003h,0F2h,002h,014h,002h,0B1h
db 001h,01Ah,002h,0B3h,003h,0F3h,071h,0F1h
db 006h,0B2h,001h,01Bh,001h,0B4h,001h,0F1h
db 071h,0F1h,071h,0F1h,071h,0F2h,002h,0B4h
db 001h,01Bh,002h,0B3h,0F1h,071h,0F1h,071h
db 0F1h,071h,0F1h,071h,0F2h,001h,0B4h,001h
db 01Ch,001h,0B1h,004h,071h,0F1h,071h,004h
db 071h,0F1h,001h,0B2h,002h,018h,006h,0E2h
db 005h,0E2h,003h,071h,001h,0B1h,002h,017h
db 002h,0F5h,001h,0E2h,003h,0F1h,001h,0E2h
db 003h,071h,005h,016h,001h,0F3h,002h,0F2h
db 004h,0F1h,071h,0F1h,004h,072h,001h,074h
db 001h,015h,003h,0F2h,003h,071h,0F1h,071h
db 0F1h,071h,0F1h,071h,0F1h,071h,0F1h,075h
db 002h,071h,002h,017h,003h,0F1h,001h,0F1h
db 071h,0F1h,071h,0F1h,071h,0F1h,071h,0F1h
db 074h,002h,073h,002h,01Ah,001h,0F1h,071h
db 0F1h,071h,0F1h,071h,0F1h,071h,0F1h,075h
db 006h,01Ah,002h,071h,0F1h,071h,0F1h,071h
db 0F1h,071h,0F1h,071h,0F1h,074h,002h,01Eh
db 001h,071h,0F1h,071h,0F1h,071h,0F1h,071h
db 0F1h,071h,0F1h,076h,001h,01Dh,001h,071h
db 0F1h,071h,0F1h,071h,0F1h,071h,0F1h,071h
db 0F1h,071h,0F1h,074h,003h,01Ch,001h,0F1h
db 071h,0F1h,071h,0F1h,071h,0F1h,071h,0F1h
db 071h,0F1h,075h,001h,0F1h,001h,01Ch,001h
db 071h,0F1h,071h,0F1h,071h,0F1h,071h,0F1h
db 071h,0F1h,076h,001h,0F1h,001h,01Ch,001h
db 072h,0F1h,071h,0F1h,071h,0F1h,071h,0F1h
db 077h,003h,01Ch,001h,07Fh,071h,001h,0F1h
db 001h,01Ch,001h,073h,003h,072h,003h,0F1h
db 074h,001h,0F1h,001h,01Ch,002h,072h,003h
db 072h,003h,075h,003h,01Dh,002h,07Dh,001h
db 0F2h,001h,01Eh,002h,07Bh,003h,0F1h,001h
db 01Fh,003h,077h,003h,0F2h,001h,01Fh,013h
db 003h,073h,003h,0F2h,002h,01Fh,016h,009h
db 000h

mace db 1
db 01Fh,01Fh,01Bh,081h,01Fh,01Fh,011h,082h
db 01Ah,081h,01Fh,014h,081h,0F1h,081h,018h
db 082h,01Fh,014h,081h,0F1h,071h,085h,012h
db 081h,0F1h,081h,01Fh,013h,082h,0F1h,071h
db 081h,004h,082h,0F1h,071h,081h,01Fh,012h
db 081h,002h,082h,006h,081h,072h,081h,01Fh
db 011h,081h,00Ch,082h,01Fh,011h,081h,00Fh
db 001h,01Bh,085h,008h,082h,007h,01Bh,081h
db 0F1h,072h,081h,006h,081h,0F1h,071h,081h
db 006h,01Ch,081h,0F1h,071h,081h,006h,081h
db 072h,081h,006h,085h,017h,082h,0F1h,008h
db 082h,006h,081h,0F3h,081h,018h,081h,00Fh
db 003h,081h,072h,081h,019h,081h,005h,082h
db 00Ch,082h,01Ah,081h,004h,081h,0F1h,071h
db 081h,003h,082h,007h,01Bh,081h,004h,081h
db 072h,081h,002h,081h,072h,081h,006h,01Bh
db 082h,004h,082h,003h,081h,0F1h,071h,081h
db 006h,013h,082h,015h,081h,0F1h,071h,081h
db 009h,081h,0F1h,081h,005h,011h,083h,0B1h
db 081h,001h,013h,081h,0F1h,072h,081h,00Ah
db 082h,002h,082h,001h,081h,0B1h,004h,0B1h
db 001h,011h,086h,00Bh,081h,001h,081h,072h
db 081h,003h,013h,082h,001h,016h,004h,082h
db 007h,081h,0F1h,071h,081h,016h,081h,0B1h
db 001h,017h,002h,081h,0F1h,071h,081h,007h
db 081h,0F1h,081h,015h,082h,002h,018h,001h
db 081h,0F1h,071h,081h,006h,012h,082h,015h
db 081h,0B1h,001h,01Ah,081h,0F1h,081h,005h
db 015h,081h,013h,082h,003h,01Ah,082h,01Fh
db 081h,0B1h,001h,01Ch,081h,01Fh,081h,003h
db 01Fh,01Ch,081h,0B1h,001h,01Fh,01Eh,081h
db 002h,01Fh,01Eh,082h,001h,01Fh,01Eh,081h
db 0B1h,001h,01Fh,01Fh,001h,000h

cube db 1
db 01Dh,00Ah,01Fh,016h,001h,0C8h,002h,01Fh
db 015h,001h,0C8h,001h,0A1h,001h,01Fh,014h
db 001h,0C8h,001h,0A2h,001h,01Fh,013h,00Ah
db 0A3h,001h,01Fh,013h,001h,098h,001h,0A3h
db 005h,01Eh,001h,098h,001h,0A3h,001h,0C2h
db 002h,01Eh,001h,098h,001h,0A3h,001h,0C1h
db 001h,0A1h,001h,01Eh,001h,098h,001h,0A3h
db 002h,0A2h,001h,019h,006h,098h,001h,0A3h
db 00Ah,013h,001h,0C5h,001h,098h,001h,0A2h
db 001h,0C8h,002h,012h,001h,0C6h,001h,098h
db 001h,0A1h,001h,0C8h,001h,0A1h,001h,011h
db 001h,0C7h,001h,098h,002h,0C8h,001h,0A2h
db 00Fh,00Eh,0A3h,002h,097h,001h,0C8h,002h
db 098h,001h,0A3h,002h,096h,001h,0C8h,001h
db 0A1h,001h,098h,001h,0A3h,002h,095h,001h
db 0C8h,001h,0A2h,001h,098h,001h,0A3h,002h
db 094h,00Ah,0A3h,001h,098h,001h,0A3h,002h
db 094h,001h,098h,001h,0A3h,001h,098h,001h
db 0A3h,002h,094h,001h,098h,001h,0A3h,001h
db 098h,001h,0A2h,001h,011h,001h,094h,001h
db 098h,001h,0A3h,001h,098h,001h,0A1h,001h
db 012h,001h,094h,001h,098h,001h,0A3h,001h
db 098h,002h,013h,006h,098h,001h,0A3h,00Ah
db 019h,001h,098h,001h,0A2h,002h,0A3h,001h
db 01Eh,001h,098h,001h,0A1h,001h,091h,001h
db 0A3h,001h,01Eh,001h,098h,002h,092h,001h
db 0A3h,001h,01Eh,00Ah,093h,001h,0A3h,001h
db 01Fh,013h,001h,098h,001h,0A3h,001h,01Fh
db 013h,001h,098h,001h,0A2h,001h,01Fh,014h
db 001h,098h,001h,0A1h,001h,01Fh,015h,001h
db 098h,002h,01Fh,016h,00Ah,000h

ball db 1
db 01Fh,01Fh,01Eh,082h,004h,081h,01Fh,018h
db 083h,008h,01Fh,014h,082h,0F2h,071h,0F1h
db 071h,081h,001h,084h,002h,01Fh,011h,082h
db 0F4h,071h,0F1h,071h,0F1h,071h,0F1h,071h
db 083h,001h,01Eh,081h,001h,0F6h,071h,0F1h
db 071h,0F1h,071h,0F1h,071h,083h,001h,01Ch
db 081h,002h,0F7h,071h,0F1h,071h,0F1h,073h
db 083h,001h,01Bh,002h,081h,0F5h,071h,081h
db 0F1h,071h,0F1h,073h,081h,071h,081h,002h
db 01Ah,081h,001h,081h,0F5h,081h,002h,081h
db 0F1h,071h,0F1h,073h,082h,003h,019h,081h
db 001h,0F4h,071h,081h,004h,081h,0F1h,073h
db 081h,071h,004h,019h,081h,071h,0F4h,081h
db 006h,081h,072h,081h,071h,081h,005h,017h
db 081h,071h,0F5h,071h,006h,081h,073h,082h
db 005h,017h,081h,0F1h,071h,0F3h,071h,0F1h
db 006h,081h,072h,081h,071h,081h,005h,017h
db 081h,071h,0F3h,071h,0F1h,071h,081h,004h
db 081h,072h,081h,071h,083h,004h,017h,081h
db 0F1h,071h,0F1h,071h,0F1h,071h,0F1h,072h
db 083h,074h,085h,001h,081h,001h,017h,082h
db 0F1h,071h,0F1h,071h,0F1h,079h,081h,071h
db 086h,001h,017h,083h,072h,0F1h,077h,081h
db 071h,081h,071h,087h,001h,018h,081h,003h
db 079h,081h,071h,087h,001h,019h,081h,003h
db 076h,081h,071h,081h,071h,081h,071h,086h
db 001h,01Ah,004h,074h,081h,071h,081h,071h
db 004h,084h,001h,01Bh,081h,003h,071h,081h
db 071h,081h,071h,081h,071h,006h,083h,001h
db 01Ch,081h,002h,081h,071h,081h,071h,081h
db 071h,007h,082h,001h,01Eh,001h,081h,071h
db 081h,071h,081h,071h,081h,006h,082h,001h
db 01Fh,011h,002h,071h,081h,071h,082h,005h
db 081h,002h,021h,0A1h,01Dh,0A4h,002h,087h
db 002h,024h,0A1h,01Ah,0A6h,022h,007h,025h
db 0A1h,021h,0A1h,018h,0A7h,021h,0A1h,02Bh
db 081h,021h,0A2h,018h,0A8h,021h,0A1h,021h
db 0A1h,021h,081h,021h,081h,021h,081h,021h
db 0A1h,021h,0A1h,021h,0A1h,019h,0A8h,021h
db 0A1h,021h,0A1h,021h,0A1h,021h,0A1h,021h
db 0A1h,021h,0A1h,021h,0A1h,01Ch,0A9h,021h
db 0A1h,021h,0A1h,021h,0A1h,021h,0A1h,021h
db 0A1h,01Fh,011h,0AEh,000h

dish db 1
db 019h,091h,01Fh,01Ch,091h,013h,091h,01Dh
db 084h,01Bh,091h,012h,091h,01Bh,082h,0F3h
db 071h,001h,018h,091h,012h,091h,01Bh,082h
db 0F2h,083h,0F1h,001h,019h,091h,013h,082h
db 017h,081h,0F2h,085h,0F1h,001h,01Ah,091h
db 011h,082h,0F1h,001h,015h,081h,0F1h,086h
db 0F1h,002h,017h,092h,013h,083h,0F1h,001h
db 013h,081h,0F1h,087h,0F1h,002h,01Ah,091h
db 012h,005h,011h,081h,0F1h,087h,0F1h,001h
db 081h,001h,019h,091h,017h,002h,0F1h,088h
db 0F1h,001h,081h,001h,018h,091h,018h,003h
db 087h,0F1h,001h,082h,001h,01Fh,011h,081h
db 0F1h,081h,002h,086h,0F1h,001h,081h,001h
db 01Fh,011h,081h,0F1h,083h,002h,084h,0F1h
db 001h,082h,001h,01Fh,081h,0F1h,085h,002h
db 082h,0F1h,001h,083h,001h,01Eh,081h,0F1h
db 087h,002h,0F1h,001h,081h,071h,081h,001h
db 01Eh,081h,0F1h,089h,0F1h,001h,081h,072h
db 081h,001h,01Dh,081h,0F1h,089h,0F1h,001h
db 081h,072h,082h,001h,01Dh,081h,0F1h,088h
db 0F1h,001h,081h,073h,081h,001h,01Dh,081h
db 0F1h,088h,0F1h,001h,081h,074h,081h,003h
db 01Ah,081h,0F1h,088h,0F1h,001h,081h,074h
db 081h,002h,071h,081h,001h,019h,081h,0F1h
db 087h,0F1h,001h,081h,074h,081h,002h,072h
db 081h,001h,018h,081h,0F1h,087h,0F1h,001h
db 081h,074h,081h,002h,072h,082h,001h,018h
db 081h,0F1h,085h,0F2h,001h,081h,073h,081h
db 003h,073h,081h,001h,01Ah,081h,0F5h,002h
db 072h,081h,004h,074h,081h,001h,01Ch,00Ah
db 012h,001h,073h,082h,001h,01Fh,019h,001h
db 072h,083h,001h,01Fh,019h,001h,085h,001h
db 01Fh,019h,001h,085h,001h,01Fh,014h,085h
db 00Ch,01Eh,081h,0F1h,07Fh,081h,001h,01Ch
db 001h,08Fh,084h,001h,01Ch,00Fh,004h,000h

flag db 1
db 01Fh,01Fh,01Fh,01Fh,01Fh,012h,006h,01Fh
db 019h,009h,01Fh,016h,007h,0F2h,004h,01Fh
db 013h,008h,0F3h,003h,016h,002h,018h,003h
db 0F1h,006h,0F5h,004h,013h,002h,015h,004h
db 0F3h,006h,0F6h,008h,015h,002h,0F5h,006h
db 0F6h,008h,015h,007h,0F6h,006h,0F6h,002h
db 015h,007h,0F6h,006h,0F6h,002h,015h,007h
db 0F6h,006h,0F6h,002h,015h,007h,0F6h,006h
db 0F6h,002h,015h,007h,0F6h,006h,0F6h,002h
db 015h,007h,0F6h,006h,0F6h,002h,015h,002h
db 0F5h,006h,0F6h,008h,015h,002h,0F5h,006h
db 0F6h,008h,015h,002h,0F5h,009h,0F3h,008h
db 015h,002h,0F3h,00Dh,0F1h,008h,015h,002h
db 0F2h,006h,016h,00Bh,015h,006h,01Bh,004h
db 0F4h,002h,015h,004h,01Eh,009h,015h,003h
db 01Fh,012h,007h,01Fh,01Fh,002h,01Fh,01Fh
db 002h,01Fh,01Fh,002h,01Fh,01Fh,002h,01Fh
db 01Fh,002h,01Fh,01Fh,002h,01Fh,01Fh,002h
db 000h

apple db 4
db 04Fh,04Fh,04Fh,04Fh,04Ah,024h,04Fh,04Ah
db 023h,0A1h,021h,082h,001h,04Fh,04Ah,021h
db 0A3h,021h,081h,021h,001h,04Fh,04Ah,023h
db 0A2h,022h,001h,04Fh,04Ah,022h,0A1h,021h
db 082h,001h,043h,031h,001h,04Fh,047h,021h
db 0A1h,082h,021h,001h,041h,031h,0B1h,001h
db 04Fh,048h,022h,0A1h,021h,002h,0B1h,001h
db 04Fh,04Bh,023h,031h,071h,001h,015h,04Fh
db 044h,012h,092h,011h,031h,071h,001h,093h
db 014h,04Fh,041h,011h,091h,0F1h,071h,091h
db 012h,002h,0D2h,071h,092h,012h,001h,04Eh
db 011h,091h,0F1h,073h,0D7h,072h,091h,012h
db 001h,04Ch,011h,091h,0F3h,092h,077h,094h
db 012h,001h,04Bh,011h,091h,0F1h,091h,0F1h
db 071h,09Dh,011h,001h,04Bh,011h,091h,0F1h
db 092h,073h,09Bh,011h,001h,04Ch,013h,09Fh
db 012h,001h,04Bh,031h,071h,0F1h,012h,09Dh
db 012h,001h,04Ch,031h,071h,0F1h,0B1h,011h
db 09Ch,012h,001h,04Ch,031h,071h,0B1h,071h
db 011h,09Ch,012h,001h,04Dh,031h,071h,0F1h
db 071h,011h,09Bh,012h,001h,04Dh,031h,071h
db 0B1h,071h,011h,09Bh,012h,001h,04Dh,031h
db 071h,0F1h,071h,011h,09Bh,012h,001h,04Dh
db 031h,071h,0B1h,071h,011h,09Bh,011h,001h
db 04Dh,031h,071h,0F1h,071h,011h,09Ch,011h
db 001h,04Ch,031h,073h,011h,09Ch,011h,001h
db 04Dh,014h,09Dh,011h,001h,04Eh,011h,09Eh
db 011h,001h,04Fh,041h,011h,09Bh,012h,001h
db 04Fh,043h,012h,098h,011h,002h,04Fh,046h
db 009h,000h

ok db 1
db 01Fh,01Fh,01Eh,007h,01Fh,017h,003h,027h
db 003h,01Fh,012h,002h,02Dh,002h,01Eh,001h
db 02Fh,022h,001h,01Ch,001h,02Fh,024h,001h
db 01Ah,001h,02Fh,026h,001h,018h,001h,02Fh
db 028h,001h,017h,001h,02Fh,028h,001h,081h
db 015h,001h,02Fh,021h,0F4h,025h,001h,081h
db 014h,001h,02Fh,071h,0F3h,071h,025h,001h
db 081h,014h,001h,02Fh,0F4h,026h,001h,082h
db 012h,001h,02Fh,071h,0F3h,071h,027h,001h
db 081h,012h,001h,02Fh,0F4h,028h,001h,081h
db 012h,001h,02Eh,071h,0F3h,071h,028h,001h
db 082h,011h,001h,02Eh,0F4h,029h,001h,082h
db 011h,001h,027h,0F3h,071h,022h,071h,0F3h
db 071h,029h,001h,082h,011h,001h,027h,071h
db 0F3h,071h,021h,0F4h,02Ah,001h,082h,011h
db 001h,028h,071h,0F3h,071h,0F3h,071h,02Ah
db 001h,082h,012h,001h,028h,071h,0F6h,02Ah
db 001h,083h,012h,001h,029h,071h,0F4h,071h
db 02Ah,001h,083h,012h,001h,02Ah,071h,0F3h
db 02Bh,001h,082h,014h,001h,02Ah,071h,0F1h
db 071h,02Ah,001h,083h,014h,001h,02Fh,028h
db 001h,083h,015h,001h,02Fh,026h,001h,083h
db 017h,001h,02Fh,024h,001h,084h,018h,001h
db 02Fh,022h,001h,084h,01Ah,002h,02Dh,002h
db 084h,01Ch,081h,003h,027h,003h,085h,01Eh
db 083h,007h,087h,01Fh,012h,08Dh,01Fh,017h
db 087h,000h

speaker db 1
db 016h,08Fh,084h,01Ch,08Fh,086h,01Ah,081h
db 07Fh,076h,081h,019h,081h,071h,008h,083h
db 009h,081h,019h,081h,071h,007h,081h,003h
db 081h,008h,081h,019h,081h,071h,006h,081h
db 005h,081h,007h,081h,019h,081h,071h,006h
db 081h,001h,081h,001h,081h,001h,081h,007h
db 081h,019h,081h,071h,006h,081h,001h,081h
db 071h,081h,001h,081h,007h,081h,019h,081h
db 071h,007h,081h,003h,081h,008h,081h,019h
db 081h,071h,008h,083h,009h,081h,019h,081h
db 071h,00Fh,005h,081h,019h,081h,071h,007h
db 081h,003h,081h,008h,081h,019h,081h,071h
db 006h,081h,005h,081h,007h,081h,019h,081h
db 071h,006h,081h,001h,081h,001h,081h,001h
db 081h,007h,081h,019h,081h,071h,006h,081h
db 001h,081h,071h,081h,001h,081h,007h,081h
db 019h,081h,071h,007h,081h,003h,081h,008h
db 081h,019h,081h,071h,008h,083h,009h,081h
db 019h,081h,071h,00Fh,005h,081h,019h,081h
db 071h,005h,089h,006h,081h,019h,081h,071h
db 004h,081h,009h,081h,005h,081h,019h,081h
db 071h,003h,081h,00Bh,081h,004h,081h,019h
db 081h,071h,002h,081h,005h,081h,071h,081h
db 002h,081h,002h,081h,003h,081h,019h,081h
db 071h,002h,081h,002h,081h,001h,081h,003h
db 081h,002h,081h,001h,081h,003h,081h,019h
db 081h,071h,002h,081h,001h,081h,008h,081h
db 002h,081h,003h,081h,019h,081h,071h,002h
db 081h,002h,083h,003h,082h,001h,081h,001h
db 081h,003h,081h,019h,081h,071h,002h,081h
db 001h,082h,072h,081h,002h,072h,081h,002h
db 081h,003h,081h,019h,081h,071h,002h,081h
db 001h,081h,071h,0F1h,071h,003h,081h,0F1h
db 071h,081h,001h,081h,003h,081h,019h,081h
db 071h,003h,081h,001h,0F1h,071h,081h,004h
db 071h,0F1h,001h,081h,004h,081h,019h,081h
db 071h,004h,081h,009h,081h,005h,081h,019h
db 081h,071h,005h,089h,006h,081h,019h,081h
db 071h,00Fh,005h,081h,019h,08Fh,088h,000h

print db 1
db 01Fh,01Fh,01Fh,01Fh,01Dh,00Fh,001h,01Fh
db 011h,001h,0FEh,001h,01Fh,082h,0FDh,082h
db 01Fh,001h,0F2h,0C3h,0F9h,001h,01Fh,011h
db 001h,0FEh,001h,01Fh,011h,001h,0F2h,0C9h
db 0F3h,001h,01Ch,005h,0FEh,005h,014h,002h
db 011h,001h,074h,001h,0F2h,0CAh,0F2h,001h
db 081h,073h,001h,011h,003h,0F1h,002h,0F4h
db 001h,0FEh,001h,071h,0F3h,002h,0F1h,002h
db 0F1h,081h,001h,082h,005h,0F3h,084h,0F3h
db 005h,082h,001h,081h,0F1h,002h,081h,002h
db 081h,001h,0F1h,0B1h,0F1h,0B1h,0F1h,001h
db 0F1h,081h,0F4h,081h,0F1h,001h,0B1h,0F1h
db 0B1h,0F1h,031h,001h,081h,002h,081h,003h
db 011h,002h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,001h,076h,001h,0B1h,0F1h,0B1h,0F1h
db 0B1h,0F1h,031h,002h,011h,002h,013h,002h
db 0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h
db 006h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 031h,002h,017h,001h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,031h,001h,018h,001h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,031h,001h,018h,001h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,031h,001h,018h,001h,0B1h
db 0F1h,001h,0F2h,001h,0F2h,001h,0F2h,001h
db 0F2h,001h,0F2h,001h,0F2h,001h,031h,001h
db 017h,001h,0B1h,00Fh,006h,0B1h,031h,001h
db 015h,001h,0B1h,0F2h,001h,0F2h,001h,0F2h
db 001h,0F2h,001h,0F2h,001h,0F2h,001h,0F2h
db 001h,0F2h,001h,031h,001h,013h,001h,0B1h
db 0F1h,00Fh,009h,0B1h,031h,001h,011h,001h
db 0B1h,0F3h,001h,0F2h,001h,0F2h,001h,0F2h
db 001h,0F2h,001h,0F2h,001h,0F2h,001h,0F2h
db 001h,0F2h,001h,031h,002h,0F1h,0B1h,00Fh
db 00Ch,031h,002h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 031h,002h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,0B1h,0F1h,031h
db 002h,03Fh,03Fh,001h,011h,00Fh,00Fh,012h
db 001h,0F1h,082h,001h,01Fh,013h,001h,0F1h
db 082h,001h,015h,003h,01Fh,015h,003h,000h

light db 1
db 01Dh,086h,01Fh,019h,082h,0F5h,071h,081h
db 001h,01Fh,016h,081h,071h,0B3h,0F4h,071h
db 081h,001h,01Fh,014h,081h,071h,0B2h,0F7h
db 071h,081h,001h,01Fh,012h,081h,071h,0B1h
db 0F9h,071h,082h,001h,01Fh,081h,071h,0B1h
db 0F1h,0B2h,0F7h,0B1h,071h,082h,001h,01Eh
db 081h,071h,0B1h,0F1h,0B2h,0F8h,0B1h,071h
db 081h,001h,081h,01Ch,081h,071h,0F1h,0B1h
db 0F1h,0B6h,0F4h,0B1h,071h,082h,001h,01Ch
db 081h,071h,0B3h,0F1h,0B1h,0F8h,0B1h,071h
db 082h,001h,01Ch,081h,071h,0B1h,0F1h,0B1h
db 0FAh,0B1h,071h,082h,001h,01Ch,081h,071h
db 0B1h,0F1h,0B1h,0FBh,071h,082h,001h,01Ch
db 081h,071h,0B1h,0F1h,0B1h,0FBh,071h,082h
db 001h,01Ch,081h,071h,0F2h,0B1h,0FBh,071h
db 082h,001h,01Ch,081h,071h,0F2h,0B1h,072h
db 081h,0F5h,081h,0F2h,071h,082h,001h,01Dh
db 081h,071h,0F1h,0B1h,081h,0F2h,081h,0F3h
db 082h,0F1h,071h,082h,001h,01Fh,081h,071h
db 0F1h,081h,0F1h,0B1h,081h,0F3h,082h,0F1h
db 071h,081h,001h,081h,01Fh,081h,071h,0F1h
db 081h,0F1h,0B1h,081h,0F3h,071h,081h,071h
db 082h,001h,01Fh,012h,081h,072h,082h,0F1h
db 071h,081h,071h,081h,0F1h,071h,081h,001h
db 01Fh,014h,081h,071h,0F3h,081h,0F1h,081h
db 0F1h,071h,081h,001h,01Fh,016h,081h,071h
db 0F1h,082h,0F2h,071h,081h,001h,01Fh,017h
db 081h,071h,0F2h,071h,081h,0F1h,071h,081h
db 001h,01Fh,018h,081h,071h,0F1h,081h,0F1h
db 071h,081h,001h,01Fh,019h,081h,071h,0F1h
db 081h,0F1h,071h,081h,001h,01Fh,019h,081h
db 071h,0F1h,081h,0F1h,071h,081h,001h,01Fh
db 019h,081h,071h,0F1h,081h,0F1h,071h,081h
db 001h,01Fh,019h,031h,071h,0F1h,081h,0F1h
db 071h,081h,001h,01Fh,019h,033h,0F2h,071h
db 002h,01Fh,019h,031h,0F1h,0B1h,003h,031h
db 001h,01Fh,019h,031h,0B1h,031h,0B1h,071h
db 031h,002h,01Fh,019h,031h,0F1h,0B1h,031h
db 002h,031h,001h,01Fh,01Ah,001h,031h,0B1h
db 071h,031h,001h,01Fh,01Ch,004h,000h

foto db 1
db 01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,01Fh
db 01Fh,017h,004h,01Fh,01Ch,001h,0F3h,081h
db 001h,01Fh,014h,002h,081h,013h,001h,074h
db 001h,081h,001h,012h,004h,081h,019h,002h
db 0F1h,081h,004h,076h,004h,0F3h,081h,003h
db 016h,001h,071h,003h,081h,072h,001h,086h
db 001h,081h,071h,005h,081h,072h,001h,014h
db 001h,0FFh,0F9h,071h,081h,001h,013h,001h
db 0F1h,07Fh,079h,082h,001h,013h,001h,0F1h
db 079h,005h,07Ah,082h,001h,013h,001h,0F1h
db 077h,002h,074h,0F1h,002h,078h,081h,002h
db 013h,009h,081h,0F5h,073h,00Bh,013h,001h
db 082h,001h,081h,001h,081h,001h,081h,075h
db 0F2h,071h,082h,001h,081h,001h,081h,001h
db 081h,002h,081h,001h,013h,001h,081h,001h
db 081h,001h,081h,001h,072h,005h,072h,082h
db 001h,081h,001h,081h,001h,081h,001h,081h
db 003h,013h,001h,082h,001h,081h,001h,071h
db 081h,001h,085h,001h,081h,071h,001h,081h
db 003h,081h,001h,081h,002h,081h,001h,013h
db 001h,081h,001h,081h,001h,081h,071h,001h
db 082h,071h,0F1h,071h,082h,001h,071h,081h
db 003h,081h,001h,081h,001h,081h,003h,013h
db 001h,082h,002h,071h,001h,082h,071h,0F1h
db 085h,001h,071h,004h,081h,001h,081h,002h
db 081h,001h,013h,001h,081h,001h,081h,001h
db 071h,001h,082h,0F1h,071h,085h,001h,071h
db 003h,081h,001h,081h,001h,081h,003h,013h
db 001h,082h,002h,071h,001h,082h,0F1h,086h
db 001h,071h,004h,081h,001h,081h,002h,081h
db 001h,013h,001h,081h,001h,081h,001h,071h
db 001h,082h,071h,086h,001h,071h,003h,081h
db 001h,081h,001h,081h,003h,013h,001h,082h
db 002h,071h,001h,089h,001h,071h,002h,081h
db 001h,081h,001h,081h,002h,081h,001h,013h
db 001h,081h,001h,081h,001h,081h,071h,001h
db 087h,001h,071h,081h,001h,081h,001h,081h
db 001h,081h,001h,081h,003h,013h,001h,082h
db 001h,081h,001h,071h,081h,001h,085h,001h
db 081h,071h,001h,081h,001h,081h,001h,081h
db 001h,081h,003h,015h,002h,081h,001h,081h
db 001h,072h,005h,072h,001h,081h,001h,081h
db 001h,081h,001h,081h,001h,081h,001h,017h
db 006h,081h,075h,081h,00Bh,01Eh,081h,005h
db 081h,000h

flop db 1
db 011h,00Fh,00Dh,013h,001h,073h,001h,0FFh
db 0F5h,001h,073h,001h,012h,001h,073h,001h
db 0FFh,0F5h,001h,073h,001h,081h,011h,001h
db 073h,001h,0FFh,0F5h,001h,071h,001h,071h
db 001h,082h,001h,073h,001h,0FFh,0F5h,001h
db 071h,001h,071h,001h,082h,001h,073h,001h
db 0FFh,0F5h,001h,073h,001h,082h,001h,073h
db 001h,0FFh,0F5h,001h,073h,001h,082h,001h
db 073h,001h,0FFh,0F5h,001h,073h,001h,082h
db 001h,073h,001h,0FFh,0F5h,001h,073h,001h
db 082h,001h,073h,001h,0FFh,0F5h,001h,073h
db 001h,082h,001h,073h,001h,0FFh,0F5h,001h
db 073h,001h,082h,001h,073h,001h,0FFh,0F5h
db 001h,073h,001h,082h,001h,073h,001h,0FFh
db 0F5h,001h,073h,001h,082h,001h,073h,001h
db 0FFh,0F5h,001h,073h,001h,082h,001h,073h
db 001h,0FFh,0F5h,001h,073h,001h,082h,001h
db 073h,001h,0FFh,0F5h,001h,073h,001h,082h
db 001h,073h,00Fh,007h,073h,001h,082h,001h
db 07Fh,07Dh,001h,082h,001h,07Fh,07Dh,001h
db 082h,001h,07Fh,07Dh,001h,082h,001h,077h
db 00Fh,003h,073h,001h,082h,001h,077h,001h
db 08Ch,001h,073h,001h,073h,001h,082h,001h
db 077h,001h,082h,004h,086h,001h,073h,001h
db 073h,001h,082h,001h,077h,001h,082h,001h
db 072h,001h,086h,001h,073h,001h,073h,001h
db 082h,001h,077h,001h,082h,001h,072h,001h
db 086h,001h,073h,001h,073h,001h,082h,001h
db 077h,001h,082h,001h,072h,001h,086h,001h
db 073h,001h,073h,001h,082h,001h,077h,001h
db 082h,001h,072h,001h,086h,001h,073h,001h
db 073h,001h,082h,001h,077h,001h,082h,004h
db 086h,001h,073h,001h,073h,001h,082h,011h
db 001h,076h,001h,08Ch,001h,073h,001h,073h
db 001h,082h,012h,00Fh,00Ch,083h,012h,08Fh
db 08Fh,013h,08Fh,08Dh,000h

pillar db 1
db 016h,08Fh,084h,01Bh,082h,0FFh,0F4h,082h
db 018h,081h,0F2h,073h,0F1h,071h,0F1h,071h
db 0F1h,071h,0F1h,071h,0F1h,071h,0F1h,071h
db 0F1h,075h,001h,017h,081h,0F1h,071h,083h
db 07Dh,083h,0F1h,071h,001h,016h,081h,0F1h
db 071h,081h,0F3h,001h,071h,081h,071h,081h
db 071h,081h,071h,081h,071h,081h,071h,001h
db 0F3h,081h,0F1h,071h,001h,015h,081h,0F1h
db 071h,081h,073h,0F1h,001h,089h,001h,071h
db 0F1h,072h,081h,0F1h,071h,001h,015h,081h
db 0F1h,071h,081h,071h,0F1h,081h,0F1h,00Bh
db 0F1h,081h,072h,081h,0F1h,071h,001h,016h
db 081h,0F1h,071h,082h,071h,0F1h,001h,0F7h
db 072h,001h,071h,0F1h,082h,0F1h,071h,001h
db 017h,081h,0F1h,071h,0F2h,071h,0F1h,001h
db 071h,081h,0F1h,071h,081h,0F1h,071h,081h
db 071h,001h,072h,0F2h,072h,001h,018h,001h
db 0F1h,073h,001h,0F1h,071h,081h,0F1h,071h
db 081h,0F1h,071h,081h,071h,081h,001h,074h
db 001h,01Ah,004h,081h,0F1h,071h,081h,0F1h
db 071h,081h,0F1h,071h,081h,071h,081h,005h
db 01Fh,081h,0F1h,071h,081h,0F1h,071h,081h
db 0F1h,071h,081h,071h,081h,001h,01Fh,014h
db 081h,0F1h,071h,081h,0F1h,071h,081h,0F1h
db 071h,081h,071h,081h,001h,01Fh,014h,081h
db 0F1h,071h,081h,0F1h,071h,081h,0F1h,071h
db 081h,071h,081h,001h,01Fh,014h,081h,0F1h
db 071h,081h,0F1h,071h,081h,0F1h,071h,081h
db 071h,081h,001h,01Fh,014h,081h,0F1h,071h
db 081h,0F1h,071h,081h,0F1h,071h,081h,071h
db 081h,001h,01Fh,014h,081h,0F1h,071h,081h
db 0F1h,071h,081h,0F1h,071h,081h,071h,081h
db 001h,01Fh,014h,081h,0F1h,071h,081h,0F1h
db 071h,081h,0F1h,071h,081h,071h,081h,001h
db 01Fh,014h,081h,0F1h,071h,081h,0F1h,071h
db 081h,0F1h,071h,081h,071h,081h,001h,01Fh
db 014h,081h,0F1h,071h,081h,0F1h,071h,081h
db 0F1h,071h,081h,071h,081h,001h,01Fh,014h
db 081h,0F1h,071h,081h,0F1h,071h,081h,0F1h
db 071h,081h,071h,081h,001h,01Fh,014h,081h
db 0F1h,071h,081h,0F1h,071h,081h,0F1h,071h
db 081h,071h,081h,001h,01Fh,014h,081h,0F1h
db 071h,081h,0F1h,071h,081h,0F1h,071h,081h
db 071h,081h,001h,01Fh,014h,081h,0F1h,071h
db 081h,0F1h,071h,081h,0F1h,071h,081h,071h
db 081h,001h,01Fh,014h,081h,0F1h,071h,081h
db 0F1h,071h,081h,0F1h,071h,081h,071h,081h
db 001h,01Fh,014h,081h,0F1h,071h,081h,0F1h
db 071h,081h,0F1h,071h,081h,071h,081h,001h
db 01Fh,014h,081h,0F1h,071h,081h,0F1h,071h
db 081h,0F1h,071h,081h,071h,081h,001h,01Fh
db 012h,08Fh,081h,001h,01Eh,081h,0F3h,07Eh
db 001h,01Ch,08Fh,084h,002h,01Ah,081h,0F2h
db 07Fh,073h,081h,001h,019h,00Fh,008h,000h

newspaper db 1
db 01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,013h
db 003h,01Fh,01Dh,001h,0F3h,002h,01Fh,01Ah
db 001h,0F2h,001h,0F3h,002h,01Fh,017h,001h
db 0F2h,002h,0F1h,001h,0F3h,002h,01Fh,014h
db 001h,0F2h,001h,0F1h,002h,0F2h,001h,081h
db 0F2h,002h,01Fh,011h,001h,0F2h,001h,0F2h
db 001h,0F2h,001h,0F1h,081h,001h,0F3h,002h
db 01Dh,001h,0F1h,071h,0F3h,001h,0F2h,001h
db 071h,001h,0F2h,001h,0F4h,002h,01Ah,001h
db 0F2h,081h,001h,081h,0F3h,001h,081h,0F3h
db 001h,0F1h,081h,0F1h,081h,0F3h,002h,017h
db 001h,0F1h,071h,0F3h,081h,001h,071h,0F2h
db 071h,001h,0F1h,001h,0F1h,082h,0F1h,001h
db 0F1h,001h,081h,0F2h,002h,014h,001h,0F2h
db 081h,001h,081h,0F3h,071h,001h,081h,0F3h
db 003h,0F1h,001h,0F1h,001h,0F1h,081h,001h
db 0F3h,001h,012h,001h,0F1h,071h,0F3h,081h
db 001h,071h,0F3h,081h,001h,081h,0F3h,002h
db 0F3h,001h,0F4h,001h,071h,011h,001h,0F2h
db 081h,001h,081h,0F3h,071h,001h,081h,0F3h
db 071h,0F1h,001h,0F3h,001h,081h,0F1h,001h
db 0F3h,001h,071h,002h,081h,071h,0F3h,081h
db 001h,071h,0F3h,081h,001h,081h,0F2h,001h
db 0F1h,002h,0F2h,081h,001h,0F3h,001h,071h
db 001h,071h,001h,0F1h,081h,001h,081h,0F3h
db 071h,001h,081h,0F3h,071h,0F1h,001h,071h
db 0F3h,002h,0F4h,001h,071h,001h,071h,002h
db 0F3h,081h,001h,071h,0F3h,081h,001h,081h
db 0F2h,001h,074h,0F3h,001h,0F2h,001h,071h
db 001h,071h,001h,071h,001h,071h,001h,081h
db 0F2h,071h,001h,081h,0F3h,071h,0F1h,001h
db 083h,074h,001h,0F2h,001h,071h,001h,071h
db 001h,071h,002h,081h,072h,001h,081h,0F2h
db 081h,001h,081h,0F3h,002h,084h,071h,001h
db 0F2h,001h,071h,001h,071h,001h,071h,001h
db 012h,002h,081h,073h,081h,0F2h,081h,072h
db 0F3h,002h,082h,001h,0F2h,001h,071h,001h
db 071h,001h,071h,001h,015h,002h,081h,072h
db 001h,081h,0F2h,071h,001h,081h,0F3h,002h
db 0F2h,001h,071h,001h,071h,001h,071h,001h
db 018h,002h,081h,072h,001h,081h,0F2h,081h
db 001h,081h,0F4h,001h,071h,001h,071h,001h
db 071h,001h,01Bh,002h,081h,071h,0F1h,071h
db 081h,0F2h,081h,001h,0F2h,001h,071h,001h
db 071h,001h,071h,001h,01Eh,002h,081h,071h
db 081h,001h,081h,0F3h,001h,071h,001h,071h
db 001h,071h,001h,01Fh,012h,002h,081h,072h
db 001h,0F1h,001h,071h,001h,071h,001h,071h
db 001h,01Fh,015h,002h,081h,072h,001h,071h
db 002h,071h,001h,01Fh,018h,002h,081h,001h
db 081h,072h,001h,01Fh,01Bh,005h,000h

umbrella db 1
db 01Fh,01Fh,01Fh,036h,01Fh,018h,033h,0F6h
db 032h,01Fh,014h,032h,0F3h,0B1h,0F1h,0B1h
db 0F1h,0B1h,0F1h,0B1h,0F1h,032h,015h,082h
db 019h,031h,0F2h,0B1h,0F1h,0B1h,0F1h,0B1h
db 0F1h,0B3h,032h,0B2h,032h,012h,081h,001h
db 081h,018h,038h,0F1h,0B2h,032h,0B6h,032h
db 001h,01Ah,03Bh,0B7h,031h,0B1h,071h,001h
db 01Ah,03Dh,0B4h,031h,0B3h,071h,001h,019h
db 03Eh,0B2h,031h,0B4h,071h,001h,019h,034h
db 0B1h,071h,0B1h,071h,0B1h,071h,0B1h,035h
db 0B6h,071h,001h,018h,032h,071h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,031h,001h,0B6h,071h,001h,019h,031h
db 0B1h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,071h,0B1h,001h,081h,001h,031h,0B6h
db 031h,001h,018h,031h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,071h,0B1h,001h,081h
db 001h,0B1h,032h,0B4h,031h,071h,001h,019h
db 031h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,001h,081h,001h,0B1h,071h,0B1h,032h
db 0B2h,031h,0B2h,071h,001h,018h,031h,0B1h
db 071h,0B1h,071h,0B1h,071h,0B1h,001h,081h
db 001h,0B1h,071h,0B1h,071h,034h,0B3h,071h
db 001h,019h,031h,0B1h,071h,0B1h,071h,0B1h
db 001h,081h,001h,0B1h,071h,0B1h,071h,0B1h
db 033h,0B4h,071h,001h,01Ah,031h,0B1h,071h
db 0B1h,001h,081h,001h,0B1h,071h,0B1h,071h
db 0B1h,071h,034h,0B3h,071h,001h,01Bh,031h
db 0B1h,001h,081h,001h,0B1h,071h,0B1h,071h
db 0B1h,071h,0B1h,034h,0B3h,071h,001h,01Ch
db 001h,081h,001h,0B1h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,035h,0B2h,071h,001h,01Bh
db 081h,071h,001h,0B1h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,035h,0B2h,071h,001h
db 01Ah,081h,071h,001h,011h,001h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,071h,035h,0B1h
db 071h,001h,01Ah,081h,071h,001h,013h,001h
db 0B1h,071h,0B1h,071h,0B1h,071h,037h,071h
db 001h,019h,081h,071h,001h,015h,002h,071h
db 0B1h,071h,038h,001h,019h,081h,071h,001h
db 018h,002h,038h,001h,015h,082h,012h,081h
db 071h,001h,01Bh,008h,015h,081h,071h,001h
db 011h,081h,071h,001h,01Fh,019h,081h,071h
db 001h,011h,081h,071h,001h,01Fh,01Ah,001h
db 071h,011h,081h,071h,001h,01Fh,01Bh,002h
db 081h,071h,001h,01Fh,01Dh,003h,000h

books db 0
db 00Fh,00Fh,006h,061h,021h,003h,022h,00Fh
db 009h,061h,0A1h,024h,0A1h,021h,00Eh,052h
db 003h,052h,002h,061h,0A1h,021h,071h,0F1h
db 071h,0A1h,022h,00Dh,051h,0D1h,054h,0D1h
db 051h,001h,061h,0A1h,021h,071h,0F1h,071h
db 0A1h,023h,012h,003h,012h,005h,051h,0D1h
db 051h,071h,0F1h,071h,0D1h,052h,061h,022h
db 071h,0F1h,071h,024h,011h,091h,014h,091h
db 011h,004h,051h,0D1h,051h,071h,0F1h,071h
db 0D1h,053h,061h,0A1h,027h,011h,091h,011h
db 071h,0F1h,071h,091h,012h,042h,001h,041h
db 052h,071h,0F1h,071h,055h,061h,0A1h,063h
db 023h,011h,091h,011h,071h,0F1h,071h,091h
db 011h,091h,041h,071h,043h,0D1h,059h,061h
db 0A1h,062h,023h,013h,071h,0F1h,071h,012h
db 091h,041h,071h,041h,071h,0F1h,041h,0D1h
db 059h,061h,0A1h,034h,021h,011h,081h,014h
db 091h,011h,041h,071h,041h,071h,0F1h,071h
db 041h,0D1h,059h,061h,0A1h,062h,023h,011h
db 081h,095h,011h,042h,071h,0F1h,071h,042h
db 0D1h,059h,061h,0A1h,063h,022h,011h,081h
db 095h,011h,071h,046h,0D1h,059h,061h,0A1h
db 034h,021h,011h,081h,034h,091h,011h,071h
db 046h,0D1h,059h,061h,0A1h,063h,022h,011h
db 081h,095h,011h,071h,046h,0D1h,059h,061h
db 0A1h,062h,023h,011h,081h,095h,011h,071h
db 0C4h,042h,0D1h,059h,061h,0A1h,063h,022h
db 011h,081h,034h,091h,011h,071h,046h,0D1h
db 059h,061h,0A1h,062h,023h,011h,081h,095h
db 011h,071h,0C4h,042h,0D1h,059h,061h,0A1h
db 063h,022h,011h,081h,095h,011h,071h,046h
db 0D1h,059h,061h,0A1h,062h,023h,011h,081h
db 095h,011h,071h,046h,0D1h,059h,061h,0A1h
db 063h,022h,011h,081h,095h,011h,071h,046h
db 0D1h,059h,061h,0A1h,062h,023h,011h,081h
db 095h,011h,071h,046h,0D1h,059h,061h,0A1h
db 063h,022h,011h,081h,095h,011h,071h,046h
db 0D1h,059h,061h,0A1h,062h,023h,011h,081h
db 095h,011h,071h,046h,0D1h,059h,061h,0A1h
db 063h,022h,011h,081h,095h,011h,071h,046h
db 0D1h,059h,061h,0A1h,034h,021h,011h,081h
db 034h,091h,011h,071h,046h,0D1h,059h,061h
db 0A1h,063h,022h,011h,081h,095h,011h,071h
db 0C4h,042h,0D1h,059h,061h,0A1h,062h,023h
db 011h,081h,095h,011h,071h,046h,0D1h,059h
db 061h,0A1h,034h,021h,011h,081h,034h,091h
db 011h,071h,0C4h,042h,0D1h,059h,061h,0A1h
db 063h,022h,011h,081h,095h,011h,071h,046h
db 0D1h,058h,001h,061h,0A1h,062h,023h,011h
db 081h,095h,011h,071h,046h,0D1h,057h,003h
db 026h,001h,016h,001h,046h,001h,056h,000h

paint db 6
db 06Fh,06Fh,069h,008h,06Fh,067h,002h,071h
db 0F6h,071h,002h,06Fh,064h,001h,0F3h,071h
db 0B1h,071h,0B1h,071h,0B1h,072h,031h,001h
db 06Fh,062h,001h,0F1h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,071h,0B1h,071h,0B1h
db 031h,001h,06Bh,001h,063h,001h,0F1h,071h
db 0B1h,071h,0B1h,0D6h,071h,0B1h,071h,0B1h
db 031h,001h,06Ah,002h,062h,001h,0F1h,0B1h
db 071h,0B1h,071h,0B1h,0D6h,051h,0B1h,071h
db 031h,001h,06Ah,002h,061h,001h,071h,0F1h
db 071h,0B1h,071h,0B1h,0D7h,051h,071h,0B1h
db 071h,031h,001h,068h,001h,0F1h,001h,061h
db 001h,0F1h,071h,0B1h,071h,0B1h,071h,0B1h
db 051h,0D6h,051h,071h,0B1h,031h,001h,067h
db 001h,0F1h,002h,061h,001h,0F1h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,052h,0D3h,052h
db 0B1h,071h,0B1h,031h,001h,066h,004h,061h
db 001h,0F1h,071h,0B1h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,054h,0B1h,071h,0B1h,071h
db 031h,001h,065h,004h,061h,001h,0F1h,071h
db 0B1h,071h,0A1h,021h,0A1h,071h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,071h,031h,001h,063h,001h,0F1h,001h
db 063h,001h,0F1h,0B1h,071h,0A1h,021h,0A1h
db 021h,0A1h,021h,0B1h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,071h,0B1h,071h,031h
db 001h,061h,001h,0F1h,001h,064h,001h,0F1h
db 071h,0B1h,021h,0A1h,021h,0A1h,021h,0A1h
db 021h,0B1h,071h,0B1h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,071h,031h,001h,0F1h
db 001h,065h,001h,0F1h,0B1h,021h,0A1h,021h
db 0A1h,021h,0A1h,022h,071h,0B1h,071h,0B1h
db 071h,0B1h,072h,033h,071h,001h,0C1h,002h
db 065h,001h,0F1h,071h,022h,0A1h,021h,0A1h
db 022h,071h,0B1h,071h,0B1h,071h,0B1h,072h
db 031h,004h,0C1h,001h,082h,001h,064h,001h
db 0F1h,0B1h,071h,025h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,031h,001h,062h,001h
db 0C1h,002h,081h,071h,031h,001h,063h,001h
db 0F1h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 031h,001h,061h,001h,0C1h,001h,061h,001h
db 073h,031h,001h,062h,001h,0F1h,0B1h,071h
db 0B1h,071h,0B1h,071h,093h,071h,0B1h,071h
db 0B1h,071h,0B1h,031h,002h,0C1h,001h,062h
db 001h,071h,0B1h,071h,031h,001h,062h,001h
db 0F1h,071h,0B1h,071h,0B1h,071h,094h,011h
db 071h,0B1h,071h,0B1h,071h,0B1h,031h,002h
db 062h,001h,071h,0B1h,071h,0B1h,071h,031h
db 001h,062h,001h,0B1h,071h,0B1h,071h,095h
db 011h,0B1h,071h,0B1h,071h,0B1h,071h,0B1h
db 031h,003h,071h,0B1h,071h,0B1h,071h,0B1h
db 031h,001h,062h,001h,071h,0B1h,071h,0B1h
db 096h,011h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,071h,031h,001h,062h,001h,0B1h,071h
db 0B1h,071h,011h,096h,011h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,031h,001h,063h,001h
db 0B1h,071h,0B1h,071h,011h,093h,012h,0B1h
db 071h,0B1h,071h,0C3h,071h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,071h,031h,001h,063h
db 001h,0F1h,0B1h,071h,0B1h,071h,013h,071h
db 0B1h,071h,0B1h,0C6h,041h,0B1h,071h,0B1h
db 071h,0B1h,071h,0B1h,031h,001h,064h,001h
db 0F1h,0B1h,071h,0B1h,071h,0B1h,071h,0B1h
db 071h,0B1h,0C8h,041h,0B1h,071h,0B1h,071h
db 0B1h,032h,001h,065h,001h,0F1h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,071h,0C8h,041h
db 071h,0B1h,071h,0B1h,071h,031h,001h,067h
db 001h,0F1h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,041h,0C5h,042h,071h,0B1h,071h,0B1h
db 071h,032h,001h,068h,001h,081h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,045h,0B1h,071h
db 0B1h,071h,0B1h,071h,032h,001h,06Ah,002h
db 031h,071h,0B1h,071h,0B1h,071h,0B1h,071h
db 0B1h,071h,0B1h,071h,0B1h,071h,033h,002h
db 06Bh,005h,03Bh,003h,06Ch,002h,064h,00Bh
db 000h

molecule db 1
db 01Eh,084h,01Fh,01Bh,082h,064h,002h,01Fh
db 018h,081h,068h,001h,01Fh,017h,081h,061h
db 0F1h,071h,065h,001h,01Fh,013h,033h,082h
db 061h,072h,066h,001h,033h,01Ch,032h,093h
db 002h,062h,034h,063h,001h,093h,002h,019h
db 031h,097h,032h,094h,002h,061h,001h,094h
db 031h,001h,018h,031h,091h,0F1h,071h,093h
db 031h,097h,031h,001h,0F1h,071h,093h,031h
db 001h,017h,031h,092h,072h,093h,031h,091h
db 0F1h,071h,094h,031h,001h,072h,093h,032h
db 001h,016h,031h,096h,031h,092h,072h,094h
db 032h,001h,094h,032h,001h,016h,031h,096h
db 031h,097h,033h,001h,093h,033h,001h,017h
db 001h,095h,031h,097h,033h,001h,082h,091h
db 032h,001h,016h,082h,001h,093h,083h,001h
db 094h,034h,001h,063h,002h,031h,003h,013h
db 081h,063h,001h,082h,063h,001h,038h,001h
db 064h,081h,001h,063h,001h,012h,081h,061h
db 0F1h,071h,081h,066h,002h,034h,002h,065h
db 081h,001h,063h,001h,011h,081h,062h,072h
db 081h,061h,0F1h,071h,064h,081h,004h,021h
db 0F1h,071h,064h,082h,001h,062h,081h,001h
db 081h,063h,081h,062h,072h,064h,081h,034h
db 021h,072h,063h,083h,001h,062h,081h,001h
db 081h,063h,081h,067h,032h,094h,002h,064h
db 083h,001h,061h,082h,002h,063h,081h,066h
db 031h,097h,031h,001h,063h,083h,001h,061h
db 082h,001h,011h,001h,063h,001h,065h,031h
db 091h,0F1h,071h,094h,031h,001h,085h,001h
db 031h,082h,001h,012h,001h,082h,031h,001h
db 084h,031h,092h,072h,094h,032h,001h,084h
db 001h,031h,082h,001h,013h,001h,031h,091h
db 071h,002h,082h,031h,097h,033h,001h,082h
db 002h,091h,031h,002h,015h,031h,094h,002h
db 031h,097h,033h,003h,093h,031h,001h,016h
db 031h,097h,001h,094h,034h,001h,095h,032h
db 001h,016h,031h,097h,001h,038h,001h,095h
db 032h,001h,017h,001h,094h,033h,002h,034h
db 002h,094h,033h,001h,018h,001h,037h,001h
db 061h,004h,061h,001h,037h,001h,019h,002h
db 033h,002h,066h,082h,002h,033h,002h,01Ch
db 004h,067h,083h,004h,01Fh,013h,001h,088h
db 001h,01Fh,018h,002h,084h,002h,01Fh,01Bh
db 004h,000h

cd db 1
db 01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,01Fh,014h
db 086h,01Fh,018h,083h,0B3h,073h,083h,01Fh
db 013h,083h,0E2h,0B3h,076h,002h,01Fh,081h
db 0A2h,0E3h,0B3h,078h,001h,01Dh,081h,0A4h
db 0E3h,0B2h,079h,001h,01Bh,081h,072h,0A3h
db 0E3h,0B2h,07Ah,001h,01Ah,081h,0E1h,072h
db 0A3h,0E2h,0B2h,07Ah,001h,019h,081h,0E3h
db 072h,0A2h,0E2h,0B2h,07Bh,001h,018h,081h
db 072h,0E2h,072h,0A2h,0E1h,0B2h,07Bh,001h
db 017h,081h,075h,0E1h,072h,0A2h,084h,07Ah
db 001h,016h,081h,076h,0E1h,072h,081h,005h
db 071h,0F1h,077h,001h,016h,081h,077h,0E1h
db 081h,001h,014h,001h,081h,078h,001h,016h
db 081h,078h,081h,001h,014h,001h,081h,078h
db 001h,016h,082h,071h,081h,071h,081h,071h
db 081h,071h,081h,001h,014h,001h,081h,078h
db 001h,016h,081h,071h,081h,071h,081h,071h
db 081h,071h,082h,001h,014h,001h,081h,078h
db 001h,016h,082h,071h,081h,071h,081h,071h
db 081h,0F1h,071h,081h,004h,081h,079h,001h
db 016h,081h,071h,081h,071h,081h,071h,081h
db 0F1h,071h,0F1h,071h,084h,0E1h,079h,001h
db 017h,081h,071h,081h,071h,081h,0F1h,071h
db 0F1h,074h,0B1h,071h,0E2h,077h,001h,018h
db 082h,071h,081h,0F1h,071h,0F1h,074h,0B2h
db 0A1h,071h,0E2h,076h,001h,019h,082h,0F1h
db 071h,0F1h,076h,0B1h,0A1h,071h,0E3h,074h
db 001h,01Ah,081h,0F1h,071h,0F1h,076h,0B2h
db 0A2h,071h,0E3h,073h,001h,01Bh,081h,0F1h
db 078h,0B1h,0A2h,071h,0E4h,071h,001h,01Dh
db 081h,077h,0B3h,0A2h,071h,0E3h,001h,01Fh
db 082h,076h,0B2h,0A2h,071h,081h,002h,01Fh
db 013h,082h,073h,0B3h,0A1h,003h,01Fh,017h
db 007h,000h

trumpet db 1
db 01Ch,03Dh,001h,01Fh,014h,031h,0F1h,0B2h
db 036h,071h,001h,01Fh,016h,031h,0F1h,0B1h
db 035h,071h,001h,01Fh,018h,031h,0F1h,0B1h
db 033h,071h,001h,01Fh,013h,034h,012h,031h
db 0F1h,0B1h,033h,071h,001h,01Fh,012h,031h
db 0B2h,072h,032h,011h,031h,0B1h,032h,071h
db 001h,01Fh,012h,031h,0B1h,071h,003h,071h
db 0B1h,032h,0B1h,032h,071h,001h,01Fh,011h
db 031h,0F1h,071h,001h,013h,001h,071h,081h
db 031h,0B1h,032h,071h,001h,01Fh,011h,031h
db 0F1h,001h,014h,031h,001h,0B1h,031h,0B1h
db 032h,071h,001h,01Fh,011h,031h,0B1h,001h
db 014h,031h,001h,0B1h,031h,0B1h,032h,071h
db 001h,01Fh,011h,031h,0F1h,001h,012h,031h
db 011h,031h,001h,0B1h,031h,0B1h,032h,071h
db 001h,01Fh,011h,031h,0B1h,001h,011h,034h
db 001h,0B1h,031h,0B1h,032h,071h,001h,01Fh
db 011h,031h,0F1h,001h,031h,0F1h,0B2h,081h
db 001h,0B1h,031h,0B1h,032h,071h,001h,01Fh
db 011h,031h,0B1h,001h,031h,0B1h,081h,071h
db 081h,001h,0B1h,031h,0F5h,032h,011h,031h
db 001h,01Bh,031h,071h,031h,0F1h,0B1h,081h
db 001h,0B1h,001h,0F2h,035h,002h,032h,001h
db 01Bh,031h,0B1h,032h,003h,0B1h,0F1h,032h
db 0B1h,032h,071h,001h,012h,001h,031h,001h
db 01Bh,031h,071h,031h,0B2h,031h,001h,0B1h
db 031h,0B1h,031h,0B1h,032h,071h,001h,013h
db 002h,01Bh,031h,071h,032h,003h,0B1h,001h
db 0B1h,031h,0B1h,032h,071h,001h,01Fh,011h
db 031h,071h,031h,0B2h,031h,001h,0B1h,001h
db 0B1h,031h,0B1h,032h,071h,001h,01Fh,011h
db 031h,071h,032h,003h,0B1h,001h,0B1h,031h
db 0B1h,032h,071h,001h,01Fh,011h,031h,071h
db 031h,0B2h,001h,0B1h,031h,001h,0B1h,031h
db 0B1h,032h,071h,001h,01Fh,011h,031h,071h
db 031h,0B1h,081h,001h,0B1h,081h,001h,0B1h
db 031h,0B1h,032h,071h,001h,01Fh,011h,031h
db 071h,031h,0B1h,081h,001h,0B1h,081h,001h
db 0B1h,031h,0B1h,032h,071h,001h,01Fh,011h
db 031h,071h,031h,0B1h,081h,001h,0B1h,031h
db 001h,0B1h,031h,0B1h,032h,071h,001h,01Fh
db 011h,031h,0B1h,031h,0B1h,081h,001h,032h
db 001h,0B1h,031h,0B1h,032h,071h,001h,01Fh
db 011h,031h,0F1h,031h,0B1h,001h,031h,001h
db 031h,001h,0B1h,031h,0B1h,032h,071h,001h
db 01Fh,011h,031h,0B1h,032h,0B1h,001h,031h
db 002h,0B1h,031h,0B1h,032h,071h,001h,01Fh
db 012h,031h,0B1h,001h,034h,001h,0B1h,031h
db 0B1h,031h,071h,001h,01Fh,013h,031h,071h
db 031h,004h,032h,0B1h,001h,031h,071h,001h
db 01Fh,014h,031h,071h,0B1h,001h,0B2h,003h
db 072h,001h,01Fh,016h,002h,036h,002h,01Fh
db 019h,006h,000h

liquid db 1
db 019h,08Dh,01Fh,084h,06Dh,084h,018h,083h
db 06Fh,066h,083h,014h,081h,071h,06Dh,071h
db 0F3h,071h,067h,071h,081h,013h,081h,071h
db 06Ch,071h,0F5h,071h,066h,071h,081h,013h
db 081h,071h,06Eh,073h,068h,071h,081h,014h
db 081h,073h,06Fh,064h,073h,001h,015h,081h
db 0F1h,062h,073h,06Dh,073h,062h,081h,001h
db 016h,061h,0F1h,064h,07Dh,064h,081h,001h
db 017h,061h,0F1h,06Fh,066h,081h,001h,017h
db 061h,0F1h,06Fh,066h,081h,001h,017h,061h
db 0F1h,061h,0F1h,073h,06Fh,061h,081h,001h
db 017h,061h,0F1h,061h,0F1h,071h,063h,0AAh
db 065h,081h,001h,017h,061h,0F1h,061h,0F1h
db 071h,0AFh,0A1h,062h,081h,001h,017h,061h
db 0F1h,0A1h,0F1h,071h,0AFh,0A3h,081h,001h
db 017h,061h,0F1h,0A1h,0F1h,073h,0ABh,0F1h
db 0A4h,081h,001h,017h,061h,0F1h,0A1h,0F1h
db 071h,0AEh,0F1h,0A3h,081h,001h,017h,061h
db 0F3h,071h,0A3h,0F3h,0A6h,0F2h,0A4h,081h
db 001h,017h,061h,0F1h,0A1h,0F1h,071h,0A6h
db 0F6h,0A5h,0F1h,081h,001h,017h,061h,0F1h
db 0A1h,0F1h,073h,0AEh,0F1h,0A1h,081h,001h
db 017h,061h,0F1h,0A1h,0F1h,071h,0A1h,0F2h
db 0AAh,0F3h,0A2h,081h,001h,017h,061h,0F1h
db 0A1h,0F1h,071h,0A3h,0FAh,0A5h,081h,001h
db 017h,061h,0F1h,0A1h,0F1h,071h,0A3h,08Ah
db 0A5h,081h,001h,017h,061h,0F1h,0A1h,0F1h
db 073h,081h,0A5h,071h,0F2h,071h,0A1h,083h
db 0A2h,081h,001h,017h,061h,0F1h,0A1h,0F1h
db 071h,0A8h,071h,0F2h,071h,0A4h,083h,001h
db 017h,061h,0F1h,0A1h,0F1h,071h,0A9h,0F1h
db 071h,0A7h,081h,001h,017h,061h,0F1h,0A1h
db 0F1h,071h,0A9h,0F1h,071h,0A7h,081h,001h
db 018h,001h,081h,0A2h,072h,0A7h,0F2h,071h
db 0A5h,081h,001h,019h,001h,081h,0AAh,071h
db 0F2h,071h,0A5h,081h,001h,01Ah,001h,083h
db 0A7h,071h,0F2h,071h,0A2h,083h,001h,01Ch
db 003h,088h,072h,083h,003h,01Fh,011h,00Dh
db 000h

telephone db 1
db 01Fh,01Fh,01Fh,01Fh,01Dh,006h,01Fh,019h
db 002h,0F1h,0B5h,003h,01Fh,015h,001h,0B3h
db 0F5h,0B3h,003h,01Fh,011h,001h,0B4h,032h
db 002h,0B1h,0F3h,0B3h,002h,01Dh,001h,0B4h
db 032h,001h,0F1h,0B1h,001h,0B3h,0F2h,0B3h
db 002h,01Bh,002h,0B2h,033h,001h,0B1h,071h
db 031h,001h,0B4h,0F2h,0B3h,002h,019h,001h
db 0B1h,002h,031h,003h,0B1h,071h,031h,001h
db 031h,0B2h,002h,031h,0F2h,0B3h,001h,018h
db 001h,0B2h,031h,001h,071h,0B3h,071h,031h
db 001h,032h,001h,0F1h,0B1h,001h,031h,0B1h
db 0F1h,0B3h,001h,018h,001h,031h,001h,071h
db 0B7h,003h,0B1h,071h,031h,001h,031h,0B1h
db 0F1h,0B3h,001h,018h,001h,071h,0B2h,0F2h
db 031h,0B5h,001h,0B1h,071h,031h,001h,031h
db 0B2h,0F1h,0B3h,001h,016h,001h,071h,0B3h
db 031h,001h,0F2h,031h,0B5h,071h,031h,001h
db 031h,0B2h,0F1h,0B3h,0F1h,001h,014h,001h
db 071h,0B2h,0F2h,031h,0B1h,031h,001h,0F2h
db 031h,0B5h,001h,031h,0B3h,0F3h,031h,001h
db 013h,001h,071h,0B3h,031h,001h,0F2h,031h
db 0B1h,031h,001h,0B5h,071h,001h,031h,0B3h
db 071h,033h,001h,012h,001h,071h,0B2h,0F2h
db 031h,0B1h,031h,001h,0F2h,031h,0B5h,071h
db 031h,001h,031h,0B3h,071h,033h,001h,011h
db 001h,071h,0B3h,031h,001h,0F2h,031h,0B1h
db 031h,001h,0B5h,071h,032h,002h,0B3h,071h
db 033h,002h,071h,0F2h,0B4h,031h,001h,0F2h
db 031h,0B5h,071h,033h,001h,032h,0B2h,071h
db 032h,003h,071h,0F4h,0B4h,031h,001h,0B5h
db 071h,034h,001h,031h,0B1h,032h,003h,031h
db 002h,071h,0B2h,0F4h,0B8h,071h,036h,001h
db 0B3h,071h,033h,002h,071h,0B4h,0F4h,0B5h
db 071h,037h,002h,0B2h,033h,001h,011h,001h
db 071h,0B6h,0F4h,0B2h,071h,039h,006h,012h
db 002h,072h,0B6h,0F3h,071h,03Bh,002h,017h
db 002h,072h,0B6h,0F1h,03Ah,002h,01Bh,002h
db 072h,0B5h,038h,002h,01Fh,002h,072h,0B3h
db 036h,002h,01Fh,014h,002h,072h,0B1h,034h
db 002h,01Fh,018h,002h,071h,032h,002h,01Fh
db 01Ch,003h,000h

smile db 1
db 01Ch,008h,01Fh,016h,003h,0B8h,003h,01Fh
db 011h,002h,0BEh,002h,01Dh,002h,0BFh,0B1h
db 002h,01Bh,001h,0BFh,0B5h,001h,019h,001h
db 0BFh,0B7h,001h,017h,001h,0BFh,0B9h,001h
db 015h,002h,0BFh,0B9h,002h,014h,001h,0B8h
db 002h,0B6h,002h,0B8h,001h,013h,001h,0B8h
db 004h,0B4h,004h,0B8h,001h,012h,001h,0B8h
db 004h,0B4h,004h,0B8h,001h,012h,001h,0B8h
db 004h,0B4h,004h,0B8h,001h,011h,001h,0B9h
db 004h,0B4h,004h,0B9h,002h,0B9h,004h,0B4h
db 004h,0B9h,002h,0BAh,002h,0B6h,002h,0BAh
db 002h,0B7h,001h,0BEh,001h,0B7h,002h,0B7h
db 001h,0BEh,001h,0B7h,002h,0B6h,001h,0BFh
db 0B1h,001h,0B6h,002h,0B3h,004h,0BFh,0B1h
db 004h,0B3h,002h,0B7h,001h,0BEh,001h,0B7h
db 001h,011h,001h,0B7h,001h,0BCh,001h,0B7h
db 001h,012h,001h,0B7h,002h,0BAh,002h,0B7h
db 001h,012h,001h,0B8h,003h,0B6h,003h,0B8h
db 001h,013h,001h,0B8h,001h,091h,006h,091h
db 001h,0B8h,001h,014h,002h,0B8h,001h,096h
db 001h,0B8h,002h,015h,001h,0B9h,001h,094h
db 001h,0B9h,001h,017h,001h,0B9h,004h,0B9h
db 001h,019h,001h,0BFh,0B5h,001h,01Bh,002h
db 0BFh,0B1h,002h,01Dh,002h,0BEh,002h,01Fh
db 011h,003h,0B8h,003h,01Fh,016h,008h,000h

banana db 1
db 01Fh,01Fh,01Fh,01Fh,01Ah,002h,01Fh,01Eh
db 003h,01Fh,01Eh,003h,01Fh,01Eh,003h,01Fh
db 01Eh,003h,01Fh,01Dh,005h,01Fh,01Ch,001h
db 0B1h,001h,0B1h,001h,01Fh,01Bh,002h,0B1h
db 001h,0B2h,001h,01Fh,01Ah,001h,0B2h,001h
db 0B2h,001h,01Fh,01Ah,001h,0B2h,001h,0B3h
db 001h,01Fh,019h,001h,0B3h,001h,0B2h,001h
db 01Fh,019h,001h,0B3h,001h,0B3h,001h,01Fh
db 018h,001h,0B4h,001h,0B3h,001h,01Fh,017h
db 001h,0B4h,001h,0B4h,001h,01Fh,017h,001h
db 0B4h,001h,0B4h,001h,01Fh,016h,001h,0B5h
db 001h,0B4h,001h,01Fh,016h,001h,0B5h,001h
db 0B4h,002h,01Fh,015h,001h,0B5h,001h,0B5h
db 001h,01Fh,014h,001h,0B6h,001h,0B5h,002h
db 01Fh,013h,001h,0B6h,001h,0B6h,002h,01Fh
db 012h,001h,0B6h,002h,0B6h,002h,01Fh,011h
db 001h,0B7h,002h,0B6h,002h,01Fh,002h,0B7h
db 003h,0B5h,002h,01Fh,001h,0B9h,008h,01Fh
db 002h,0BCh,002h,01Fh,013h,004h,0B6h,003h
db 01Fh,018h,006h,000h

bolt db 1
db 01Fh,01Fh,01Fh,006h,01Fh,01Ah,001h,085h
db 002h,01Fh,018h,001h,078h,001h,01Fh,017h
db 081h,002h,071h,004h,081h,001h,01Fh,017h
db 001h,083h,073h,002h,081h,01Fh,017h,081h
db 001h,072h,085h,001h,01Fh,016h,002h,083h
db 073h,002h,081h,001h,01Fh,013h,002h,071h
db 081h,001h,072h,085h,001h,071h,002h,01Eh
db 002h,073h,001h,083h,073h,002h,081h,073h
db 002h,01Bh,001h,075h,081h,001h,087h,001h
db 075h,001h,01Ah,001h,0F2h,074h,081h,006h
db 081h,074h,003h,01Ah,001h,072h,0F2h,07Ch
db 002h,082h,001h,01Ah,001h,074h,0FBh,001h
db 084h,001h,01Ah,001h,075h,0F1h,078h,001h
db 085h,001h,01Ah,001h,075h,0F1h,078h,001h
db 085h,001h,01Bh,002h,073h,0F1h,078h,001h
db 083h,002h,01Eh,002h,071h,0F1h,078h,001h
db 081h,002h,01Fh,013h,00Ch,01Fh,016h,081h
db 001h,072h,085h,001h,01Fh,016h,002h,083h
db 073h,002h,081h,001h,01Fh,013h,002h,071h
db 081h,001h,072h,085h,001h,071h,002h,01Eh
db 002h,073h,001h,083h,073h,002h,081h,073h
db 002h,01Bh,001h,075h,081h,001h,072h,085h
db 001h,075h,001h,01Ah,001h,0F2h,074h,081h
db 006h,081h,074h,003h,01Ah,001h,072h,0F2h
db 07Ch,002h,082h,001h,01Ah,001h,074h,0FBh
db 001h,084h,001h,01Ah,001h,075h,0F1h,078h
db 001h,085h,001h,01Ah,001h,075h,0F1h,078h
db 001h,085h,001h,01Bh,002h,073h,0F1h,078h
db 001h,083h,002h,01Eh,002h,071h,0F1h,078h
db 001h,081h,002h,01Fh,013h,00Ch,000h


coltbl dd 000000h,800000h,008000h,808000h
       dd 000080h,800080h,008080h,0C0C0C0h
       dd 808080h,0FF0000h,00FF00h,0FFFF00h
       dd 0000FFh,0FF00FFh,00FFFFh,0FFFFFFh
       dd 0006688ddh  ; bitton color


pict   dd globe
       dd wa
       dd sword
       dd cow
       dd mace
       dd cube
       dd ball
       dd dish
       dd apple
       dd ok
       dd speaker
       dd print
       dd light
       dd key1
       dd foto
       dd flop
       dd pillar
       dd newspaper
       dd umbrella
       dd books
       dd flag
       dd paint
       dd molecule
       dd cd
       dd trumpet
       dd liquid
       dd telephone
       dd smile
       dd banana
       dd bolt

labnew  db   'New game    Clicks:'
labnewlen:


nkeydown dd ?
bitstat db ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
        db ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
        db ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
bitpict db ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
        db ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
        db ?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?
firstbit  db ?
secondbit db ?


bitid   db ?


mas:  ;  mas db (32*32)*3+1 dup (?)

I_END=mas+(32*32)*3+1



