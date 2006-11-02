;
;   Save Ramdisk to HD and FD
;   Mario79 2005
;   Compile with FASM for Menuet
;

include 'lang.inc'
include 'macros.inc'

appname equ 'RDsave '
version equ '1.1'

  use32
  org     0x0

  db     'MENUET01'     ; 8 byte id
  dd     0x01           ; header version
  dd     START          ; start of code
  dd     I_END          ; size of image
  dd     0x1000         ; memory for app
  dd     0x1000         ; esp
  dd     0x0 , 0x0      ; I_Param , I_Icon


;******************************************************************************


START:                ; start of execution
    xor  eax,eax
    mov  edi,bootpath
    mov  ecx,128
    rep  stosd

    mcall 6,filename,0,-1,bootpath

    mov esi,bootpath+1
    mov cx,512
  start_search:
    lodsb
    cmp al,"'"
    jz    set_end_path
    dec cx
    cmp cx,0
    ja    start_search
  set_end_path:
    mov [esi-1],byte 0

    mov  eax,40
    mov  ebx,101b
    int  0x40

red:
    call draw_window

still:
    mov  eax, 10         ; wait here for event
    int  0x40

    dec  eax              ; redraw request ?
    je     red
;    dec  eax
;    dec  eax                  ; button in buffer ?
;    je   button


  button:               ; button
    mov  eax,17            ; get id
    int  0x40
    cmp  ah,2
    jne  ah_3
    mcall 18,6,1
    jmp  red
  ah_3:
    cmp  ah,3
    jne  ah_4
    mcall 18,6,2
    jmp  red
  ah_4:
    cmp  ah,4
    jne  ah_5
    mcall 18,6,3,bootpath+1
    jmp  red
  ah_5:
    cmp  ah,5
    jne  ah_6
    mcall 16,1
    jmp  red
  ah_6:
    cmp  ah,6
    jne  ah_1
    mcall 16,2
    jmp  red

  ah_1:
    cmp  ah,1
    je     exit
    jmp  still

  exit:
    or     eax,-1            ; close this program
    int  0x40


;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************

draw_window:

    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    int  0x40

    mov  eax, 12                ; function 12:tell os about windowdraw
    mov  ebx, 1                 ; 1, start of draw
    int  0x40
                ; DRAW WINDOW
    mov  eax, 0                 ; function 0 : define and draw window
    mov  ebx, 200*65536+230     ; [x start] *65536 + [x size]
    mov  ecx, 200*65536+260     ; [y start] *65536 + [y size]
    mov  edx, [sc.work]         ; color of work area RRGGBB,8->color gl
    or   edx,0x33000000
    mov  edi,header             ; WINDOW LABEL
    int  0x40

    mcall 8,<11,17>,<26,17>,2,[sc.work_button]
    inc  edx
    mcall  , ,<56,17>, ,
    inc  edx
    mcall  , ,<86,17>, ,
    inc  edx
    mcall  , ,<166,17>, ,
    inc  edx
    mcall  , ,<196,17>, ,

    mov    ecx,[sc.work_button_text]
    or     ecx,0x10000000
    mcall 4,<17,31>, ,text_123,1
    add ebx,30
    add edx,1
    mcall
    add ebx,30
    add edx,1
    mcall
    add ebx,80
    add edx,1
    mcall
    add ebx,30
    add edx,1
    mcall

    mov    ecx,[sc.work_text]
    or     ecx,0x80000000
    mcall  ,<40,31>, ,text_1,
    add    ebx,30
    mcall  , , ,text_2,
    add    ebx,30
    mcall  , , ,text_3,
    mcall  ,<15,115>, ,text_4,
    mcall  ,<35,125>, ,text_5,

    mcall  ,<35,41>, ,text_6,
    mcall  ,<35,101>, ,text_6,
    mcall  ,<40,171>, ,text_7,
    mcall  ,<40,201>, ,text_8,
    mcall  ,<25,10>, ,text_9,
    mcall  ,<25,150>, ,text_10,

    mov  eax,12           ; function 12:tell os about windowdraw
    mov  ebx,2            ; 2, end of draw
    int  0x40
ret


header   db appname,version,0

text_123 db '12345'

if lang eq ru

text_1  db 'В папку С:\KOLIBRI',0
text_2  db 'В корень диска С',0
text_3  db 'Путь в файле RD2HD.TXT',0
text_4  db 'п.3 для резервного сохранения',0
text_5  db 'т.к. в ядре его нет.',0
text_6  db '(папка должна присутствовать)',0
text_7  db 'На /FD/1',0
text_8  db 'На /FD/2',0
text_9  db 'Сохранение на жесткий диск:',0
text_10 db 'Сохранение на дискету:',0

else

text_1  db 'To the folder C:\KOLIBRI',0
text_2  db 'To the root of C',0
text_3  db 'To path in the file RD2HD.TXT',0
text_4  db 'p.3 for backup, as the kernel',0
text_5  db 'can not boot from there.',0
text_6  db '(folder must exist)',0
text_7  db 'To /FD/1',0
text_8  db 'To /FD/2',0
text_9  db 'Save to hard disk:',0
text_10 db 'Save to floppy:',0

end if

filename db 'RD2HD   TXT'
I_END:
sc  system_colors
bootpath:
