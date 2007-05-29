;    IMGVIEW.ASM
;
;    This program displays jpeg images. The window can be resized.
;
;    Version 0.0    END OF 2003
;                   Octavio Vega
;    Version 0.1    7th March 2004
;                   Mike Hibbett ( very small part! )
;    Version 0.11   7th April 2004
;                   Ville Turjanmaa ( 'set_as_bgr' function )
;    Version 0.12   29th May 2004
;                   Ivan Poddubny (correct "set_as_bgr"+parameters+boot+...)
;    Version 0.12   30 de mayo 2004
;                   Octavio Vega
;                   bugs correction and slideshow
;    version 0.13   3 de junio 2004
;                   Octavio Vega
;                   unos retoques
;    version 0.14   10th August 2004
;                   Mike Hibbett Added setting default colours
;    version 0.15   24th August 2006
;                   diamond (rewritten to function 70)
;    version 0.16   19th May 2007
;                   Mario79
;                   1) correction for changed function 15,
;                   2) use monochrome background if free memory there are less than 2 MB
;                   3) use COL0 - COL9 boot parameter
;                   0=black,1=white,2=green,3=lilas,4=grey
;                   5=light-blue,6=blue,7=salad,8=pink,9=yellow
                  
               memsize=20000h
               org 0
 PARAMS     =    memsize - 1024

appname equ 'Jpegview '
version equ '0.15'

use32

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     memsize                 ; memory for app
               dd     memsize - 1024           ; esp
               dd     PARAMS , 0x0               ; I_Param , I_Icon

stack_size=4096 + 1024

include '..\..\..\macros.inc'

START:                          ; start of execution

    cmp     [PARAMS], byte 0
    jne     check_parameters

    ; Calculate the 'free' memory available
    ; to the application, and create the malloc block from it
  .l1:
    mov     ecx,memsize-fin-stack_size
    mov     edi,fin
    call    add_mem

    call    colorprecalc ;inicializa tablas usadas para pasar de ybr a bgr
    call    draw_window
    call    read_string.rs_done

still:
    push still
    mov ebx,100                ;1 second
    mov  eax,23                 ; wait here for event
    mcall
    cmp  eax,1                  ; redraw request ?
    je   draw_window
    cmp  eax,2                  ; key in buffer ?
    je   read_string
    cmp  eax,3                  ; button in buffer ?
    je   button
    jmp display_next

button:                       ; BUTTON
    mov  eax,17
    mcall
    cmp ah,3
    je set_as_bgr2
    cmp ah,2
    je slideshow
    cmp  ah,1                   ; CLOSE PROGRAM
    jne  close_program.exit
close_program:
    mov  eax,-1
    mcall
  .exit:
    ret

   ; Put a 'chunk' of the image on the window
put_image:
    pushad

    lea ebp,[edx+eax+7]
    cmp  [winxs],bp
    jc     .l1
    lea ebp,[ecx+ebx+20+2+17]
    cmp [winys],bp
    jc     .l1

    add     eax,2  ; offset for boarder
    add     ebx,2 ; offset for title bar
    push    ax ; pox
    push    bx ; pos
    push    cx ; size
    push    dx ; size
    pop     ecx
    pop     edx
    mov     ebx,edi
    mov     eax,7

    mcall                         ; Put image function
.l1:
    popad
    ret



;******************************************************************************

check_parameters:
    cmp     [PARAMS], dword "BOOT" ; received BOOT parameter -> goto handler
    je      boot_set_background
    cmp     [PARAMS], word "CO"
    jne     @f
    cmp     [PARAMS+2], byte "L"
    je      boot_set_background    
@@:
    mov     edi, name_string       ; clear string with file name
    mov     al,  0
    mov     ecx, 100
    rep     stosb

    mov     ecx, 100               ; calculate length of parameter string
    mov     edi, PARAMS
    repne   scasb
    sub     edi, PARAMS
    mov     ecx, edi

    mov     esi, PARAMS            ; copy parameters to file name
    mov     edi, name_string
    cld
    rep     movsb

    jmp     START.l1       ; return to beggining of the progra

;******************************************************************************
boot_set_background:
    mcall 18,16
    cmp   eax,1024*2
    jb    set_mono
    mov     ecx,memsize-fin-stack_size  ; size
    mov     edi,fin                     ; pointer
    call    add_mem             ; mark memory from fin to 0x100000-1024 as free
    call    colorprecalc        ; calculate colors
    mov     esi,name_string
    call    open
    test    eax,eax
    jz      close_program
    call    jpeg_info
    mov dword [jpeg_st],ebp
    call    set_as_bgr2         ; set wallpaper
    jmp     close_program       ; close the program right now

;******************************************************************************
set_mono:
    mov     eax, 15
    mov     ebx, 1
    mov     ecx, 1
    mov     edx, 1
    mcall

    cmp     [PARAMS], dword "BOOT" ; received BOOT parameter -> goto handler
    jne     @f
.green:
    mov     ecx,mono+6
    jmp     .set
@@:
;    cmp     [PARAMS], word "CO" ; received BOOT parameter -> goto handler
;    jne     .green
    xor     ecx,ecx
    mov     cl,[PARAMS+3]
    sub     cl,0x30
    cmp     ecx,0
    jb      .green
    cmp     ecx,9     
    ja      .green
    imul    ecx,3
    add     ecx,mono
.set:
    mcall   15,5, ,0,3

    ; Stretch the image to fit
    mov     eax, 15
    mov     ebx, 4
    mov     ecx, 1
    mcall

    mov     eax, 15
    mov     ebx, 3
    mcall
    jmp     close_program
    
mono:
    db 0,0,0       ; black
    db 255,255,255 ; white
    db 128,128,0   ; green
    db 240,202,166 ; lilas
    db 192,192,192 ; grey
    db 255,255,0   ; light-blue
    db 255,0,0     ; blue
    db 192,220,192 ; salad
    db 255,0,255   ; pink
    db 0,255,255   ; yellow
;******************************************************************************

set_as_bgr2:
    mov ebp,dword[jpeg_st]
    test    ebp,ebp
    jz      .end
    
    mov     eax, 15
    mov     ebx, 1
    mov     ecx, [ebp + x_size]
    mov     edx, [ebp + y_size]
    mcall

    mov     dword [ebp+draw_ptr],put_chunk_to_bgr
    call    jpeg_display

    ; Stretch the image to fit
    mov     eax, 15
    mov     ebx, 4
    mov     ecx, 2
    mcall

    mov     eax, 15
    mov     ebx, 3
    mcall


 .end:
    ret

;******************************************************************************

put_chunk_to_bgr:
    pushad

    mov     [x_pointer], edi
    mov     esi, ecx
    imul    esi, 3
    mov     [x_numofbytes], esi
    mov     ecx, [ebp + x_size]
    imul    ecx, ebx
    add     ecx, eax
    imul    ecx, 3
    mov     [x_offset], ecx
    mov     [x_counter], edx
    mov     eax, [ebp + x_size]
    imul    eax, 3
    mov     [x_numofb2], eax
 .new_string:
    mov     eax, 15
    mov     ebx, 5
    mov     ecx, [x_pointer]
    mov     edx, [x_offset]
    mov     esi, [x_numofbytes]
    mcall
    mov     eax, [x_numofbytes]
    add     [x_pointer], eax
    mov     eax, [x_numofb2]
    add     [x_offset], eax
    dec     [x_counter]
    jnz     .new_string

    popad
    ret

;******************************************************************************



;   *********************************************
;   *******  WINDOW DEFINITIONS AND DRAW ********
;   *********************************************


draw_window:

    mov  eax,48
    mov  ebx,3
    mov  ecx,sc
    mov  edx,sizeof.system_colors
    mcall

    mov  eax,12
    mov  ebx,1
    mcall

    ; Draw the window to the appropriate size - it may have
    ; been resized by the user
    cmp     [winxs], 0
    jne     dw_001

    ; Give the screen some inital defaults
    mov     [winxs], 400
    mov     [winys], 300
    mov     ax, 100
    mov     [winxo], ax
    mov     [winyo], ax
    jmp     dw_002

dw_001:
    mov     eax, 9
    mov ebx, memsize - 1024
    mov ecx, -1
    mcall
    mov     eax, [ebx + 34]
    mov     [winxo], ax
    mov     eax, [ebx + 38]
    mov     [winyo], ax
    mov     eax, [ebx + 42]
    mov     [winxs], ax
    mov     eax, [ebx + 46]
    mov     [winys], ax

dw_002:
        mov     ebx, dword [winxo-2]
        mov     bx, [winxs]
        mov     ecx, dword [winyo-2]
        mov     cx, [winys]

    xor  eax,eax                   ; DRAW WINDOW
    mov  edx,[sc.work]
    or   edx,0x33000000
    mov  edi,title                ; WINDOW LABEL
    mcall


    mov  eax,8                     ; BUTTON 2: slideshow
    mov  ebx,57
    mov  cx, [winys]
    sub  cx, 39
    shl  ecx, 16
    add  ecx, 12
    mov  esi, [sc.work_button]
    mov  edx,2
    mcall

    mov  eax,4                     ; Button text
    movzx ebx, word [winys]
    add   ebx, 3 shl 16 - 36
    mov  ecx,[sc.work_button_text]
    mov  edx,setname
    mov  esi,setnamelen-setname
    mcall


    mov  eax,8                     ; BUTTON 3: set as background
    mov  bx, [winxs]
    sub  bx, 65
    shl  ebx, 16
    mov  bx,55
    mov  cx, [winys]
    sub  cx, 39
    shl  ecx, 16
    add  ecx, 12
    mov  esi, [sc.work_button]
    mov  edx,3
    mcall

    mov  eax,4                     ; Button text
    movzx ebx, word [winxs]
    sub   ebx, 63
    shl   ebx,16
    mov   bx, word [winys]
    sub   bx,36
    mov  ecx,[sc.work_button_text]
    mov  edx,setbgr
    mov  esi,setbgrlen-setbgr
    mcall
    call    print_strings
    call    load_image
    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,2                     ; 2, end of draw
    mcall

    ret



    ; Read in the image file name.
read_string:
    movzx edi,byte[name_string.cursor]
    add     edi,name_string
    mov     eax,2
    mcall                        ; Get the key value
    shr     eax,8
    cmp     eax,13                      ; Return key ends input
    je      .rs_done
    cmp     eax,8
    jnz     .nobsl
    cmp     edi,name_string
    je      .exit
    dec     edi
    mov     [edi],byte 0;'_'
    dec     byte[name_string.cursor]
    jmp     print_strings
.exit:   ret
.nobsl:
    cmp     eax,31
    jbe     .exit
    cmp     eax,97
    jb      .keyok
    sub     eax,32
.keyok:
    mov ah,0
    stosw
    cmp edi,name_string.end
    jnc print_strings
    inc byte[name_string.cursor]
    jmp    print_strings
.rs_done:
    call   print_strings
    mov     esi,name_string
    call    open
    test    eax,eax
    jz      .exit
    call    jpeg_info
    test    ebp,ebp
    jz      close
    xchg    [jpeg_st],ebp
    call jpeg_close

load_image:

    mov     eax,13              ; clear picture area
    movzx    ebx, word [winxs]
    add     ebx, 1  shl 16 -10
    movzx    ecx, word [winys]
    sub     ecx, 40
    add     ecx, 1  shl 16

    mov     edx,[sc.work]
    mcall
    mov    ebp,[jpeg_st]
    test    ebp,ebp
    jz      .exit
    mov     dword [ebp+draw_ptr],put_image
    jmp    jpeg_display
 .exit: ret

print_strings:
    pusha
    mov     eax,13              ; clear text area
    movzx   ebx, word [winxs]
    add     ebx, 59 shl 16 -125
    mov     cx, [winys]
    sub     cx, 39
    shl     ecx, 16
    add     ecx, 12
    mov     edx,0xffffff
    mcall

    mov     eax,4               ;
    movzx   ebx, word [winys]
    add     ebx, 61 shl 16 - 37
    mov     ecx,0x000000
    mov     edx,name_string
    mov     esi,60
    mcall
    popa
    ret

slideshow:
        cmp     [file_dir], 0
        jnz     .exit
        cmp     [jpeg_st], 0
        jz      .exit
        mov     esi, name_string
        movzx   ecx, byte [name_string.cursor]
.l1:
        cmp     byte [esi+ecx], '/'
        jz      .l2
        loop    .l1
.exit:
        ret
.l2:
        mov     byte [esi+ecx], 0
        call    open
        mov     byte [esi+ecx], '/'
        test    eax, eax
        jz      .exit
        test    byte [fileattr], 0x10
        jz      .exit
        mov     [file_dir], eax
        inc     ecx
        mov     [name_string.cursor], cl
display_next:
        mov     ebx, [file_dir]
        test    ebx, ebx
        jnz     @f
        ret
@@:
        mov     byte [ebx], 1
        mov     byte [ebx+12], 1
        mov     dword [ebx+16], dirinfo
        mov     eax, 70
        mcall
        mov     eax, [file_dir]
        inc     dword [eax+4]
        cmp     ebx, 1
        jz      @f
        mov     eax, [file_dir]
        and     [file_dir], 0
        jmp     close
@@:
        movzx   edi, byte [name_string.cursor]
        add     edi, name_string
        lea     esi, [dirinfo+32+40]
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        mov     ecx, name_string.end
        sub     ecx, edi
        rep     stosb
    call   print_strings
    mov     esi,name_string
    call    open
    test    eax,eax
    jz      display_next
    call    jpeg_info
    test    ebp,ebp
    jnz     .l6
    call close
    jmp display_next
   .l6:
    mov dword[ebp+draw_ptr],put_image
    push ebp
    xchg [jpeg_st],ebp
    call jpeg_close
    pop ebp
    jmp jpeg_display


include 'filelib.asm'
include 'memlib.asm'
include 'jpeglib.asm'


; DATA AREA

wcolor          dd  0x000000
title          db  appname,version,0
setname          db  'SLIDESHOW'
setnamelen:

setbgr           db  '   BGR  '
setbgrlen:

x_pointer       dd  0
x_offset        dd  0
x_numofbytes    dd  0
x_numofb2       dd  0
x_counter       dd  0
winxo           dw  0
winyo           dw  0
winxs           dw  0
winys           dw  0
jpeg_st         dd  0
file_dir        dd  0
name_string:    db '/rd/1/jpegview.jpg',0
rb 256
    .end:
    .cursor: db 19
    .cursor2: db 0

align 4

rgb16:          db 0,4,8,13,17,21,25,29,34,38,42,46,50,55,59,63
rgb4:           db 0,21,42,63

include 'jpegdat.asm'

align 4

iniciomemoria:
              dd -(iniciomemoria+4),-(iniciomemoria+4),(iniciomemoria+4),.l1,0
.l1           dd 0

fin:
I_END:
sc     system_colors
fileattr: rb 40
dirinfo: rb 32+304
