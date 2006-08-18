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
;
               memsize=20000h
               org 0
 PARAMS     =    memsize - 1024

use32

               db     'MENUET01'              ; 8 byte id
               dd     0x01                    ; header version
               dd     START                   ; start of code
               dd     I_END                   ; size of image
               dd     memsize                 ; memory for app
               dd     memsize - 1024           ; esp
               dd     PARAMS , 0x0               ; I_Param , I_Icon

include 'lang.inc'
stack_size=4096 + 1024

include 'macros.inc'

START:                          ; start of execution

    cmp     [PARAMS], byte 0
    jne     check_parameters

    ; Calculate the 'free' memory available
    ; to the application, and create the malloc block from it
  .l1:
    mov     ecx,memsize-fin-stack_size
    mov     edi,fin
    call    add_mem

    ; Get some memory
    mov     ecx,16384
    call    malloc
    mov     [work_area],edi
    call    colorprecalc ;inicializa tablas usadas para pasar de ybr a bgr
    call    draw_window
    call    read_string.rs_done

still:
    push still
    mov ebx,100                ;1 second
    mov  eax,23                 ; wait here for event
    int  0x40
    cmp  eax,1                  ; redraw request ?
    je   draw_window
    cmp  eax,2                  ; key in buffer ?
    je   read_string
    cmp  eax,3                  ; button in buffer ?
    je   button
    jmp display_next

button:                       ; BUTTON
    mov  eax,17
    int  0x40
    cmp ah,3
    je set_as_bgr2
    cmp ah,2
    je slideshow
    cmp  ah,1                   ; CLOSE PROGRAM
    jne  close_program.exit
close_program:
    mov  eax,-1
    int  0x40
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

    add     eax,5  ; offset for boarder
    add     ebx,20 ; offset for title bar
    push    ax ; pox
    push    bx ; pos
    push    cx ; size
    push    dx ; size
    pop     ecx
    pop     edx
    mov     ebx,edi
    mov     eax,7

    int     40h                         ; Put image function
.l1:
    popad
    ret



;******************************************************************************

check_parameters:
    cmp     [PARAMS], dword "BOOT" ; received BOOT parameter -> goto handler
    je      boot_set_background

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

    mov     ecx,memsize-fin-stack_size  ; size
    mov     edi,fin                     ; pointer
    call    add_mem             ; mark memory from fin to 0x100000-1024 as free
    ; Get some memory
    mov     ecx,16384           ; get 16 Kb of memory
    call    malloc              ; returns pointer in edi
    mov     [work_area],edi     ; save it
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
;******************************************************************************

set_as_bgr2:
    mov ebp,dword[jpeg_st]
    test    ebp,ebp
    jz      .end

    mov     dword [ebp+draw_ptr],put_chunk_to_bgr
    call    jpeg_display
    mov     eax, 15
    mov     ebx, 1
    mov     ecx, [ebp + x_size]
    mov     edx, [ebp + y_size]
    int     0x40

    ; Stretch the image to fit
    mov     eax, 15
    mov     ebx, 4
    mov     ecx, 2
    int     0x40

    mov     eax, 15
    mov     ebx, 3
    int     0x40


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
    int     0x40
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

    mov  eax,12
    mov  ebx,1
    int  0x40

    ; Draw the window to the appropriate size - it may have
    ; been resized by the user
    mov     eax, 0
    cmp     [winxs], ax
    jne     dw_001

    ; Give the screen some inital defaults
    mov     ax, 400
    mov     [winxs], ax
    mov     ax, 300
    mov     [winys], ax
    mov     ax, 100
    mov     [winxo], ax
    mov     [winyo], ax
    jmp     dw_002

dw_001:
    mov     eax, 9
    mov ebx, memsize - 1024
    mov ecx, -1
    int     0x40
    mov     eax, [ebx + 34]
    mov     [winxo], ax
    mov     eax, [ebx + 38]
    mov     [winyo], ax
    mov     eax, [ebx + 42]
    mov     [winxs], ax
    mov     eax, [ebx + 46]
    mov     [winys], ax

dw_002:
    mov     bx, [winxo]
    shl     ebx, 16
    mov     bx, [winxs]

    mov     cx, [winyo]
    shl     ecx, 16
    mov     cx, [winys]


    mov  eax,0                     ; DRAW WINDOW
    mov  edx,[wcolor]
    add  edx,0x02000000
    mov  esi,0x80557799
    mov  edi,0x00557799
    int  0x40

    mov  eax,4                     ; WINDOW LABEL
    mov  ebx,8*65536+8
    mov  ecx,0x00ffffff
    mov  edx,labelt
    mov  esi,labellen-labelt
    int  0x40


    mov  eax,8                     ; CLOSE BUTTON

    mov  bx, [winxs]
    sub  bx, 19
    shl  ebx, 16
    add  ebx, 12

    mov  ecx,5*65536+12
    mov  edx,1
    mov  esi,0x557799
    int  0x40

    ; draw status bar
    mov     eax, 13
    movzx     ebx, word [winxs]
    sub     ebx, 5
    add     ebx, 4*65536
    mov     cx, [winys]
    sub     ecx, 19
    shl     ecx, 16
    add     ecx, 3
    mov     edx, 0x00557799
    int     0x40

    mov  eax,8                     ; BUTTON 2: filename
    mov  ebx,4*65536+55
    mov  cx, [winys]
    sub  cx, 16
    shl  ecx, 16
    add  ecx, 12
    mov  esi, 0x00557799
    mov  edx,2
    int  0x40

    mov  eax,4                     ; Button text
    movzx ebx, word [winys]
    sub   ebx, 13
    add   ebx, 6*65536
    mov  ecx,0x00ffffff
    mov  edx,setname
    mov  esi,setnamelen-setname
    int  0x40


    mov  eax,8                     ; BUTTON 3: set as background
    mov  bx, [winxs]
    sub  bx, 60
    shl  ebx, 16
    mov  bx,55
    mov  cx, [winys]
    sub  cx, 16
    shl  ecx, 16
    add  ecx, 12
    mov  esi, 0x00557799
    mov  edx,3
    int  0x40

    mov  eax,4                     ; Button text
    movzx ebx, word [winxs]
    sub   ebx, 60
    shl   ebx,16
    mov   bx, word [winys]
    sub   bx,13
    mov  ecx,0x00ffffff
    mov  edx,setbgr
    mov  esi,setbgrlen-setbgr
    int  0x40
    call    print_strings
    call    load_image
    mov     eax,12                    ; function 12:tell os about windowdraw
    mov     ebx,2                     ; 2, end of draw
    int     0x40

    ret



    ; Read in the image file name.
read_string:
    movzx edi,byte[name_string.cursor]
    add     edi,name_string
    mov     eax,2
    int     0x40                        ; Get the key value
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
    sub     ebx, 7
    add     ebx, 4 * 65536
    movzx    ecx, word [winys]
    sub     ecx, 39
    add     ecx, 20 * 65536

    mov     edx,0
    int     0x40
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
    sub     ebx, 64+58
    add     ebx, 60*65536
    mov     cx, [winys]
    sub     cx, 16
    shl     ecx, 16
    add     ecx, 12
    mov     edx,0xffffff
    int     0x40

    mov     eax,4               ;
    movzx   ebx, word [winys]
    sub     ebx, 14
    add     ebx, 60*65536
    mov     ecx,0x000000
    mov     edx,name_string
    mov     esi,60
    int     0x40
    popa
    ret

slideshow:
    test dword[file_dir],-1
    jnz .exit
    test dword[jpeg_st],-1
    jz .exit
    mov esi,name_string
    movzx ecx,byte[name_string.cursor]
   .l1:
    cmp byte[esi+ecx],'/'
    je .l2
    loop .l1
  .exit:
    ret
  .l2:
    mov byte[esi+ecx],0
    call open
    mov byte[esi+ecx],'/'
    test eax,eax
    jz .exit

    mov dword[eax+file_handler.size],-1 ;directory size is always 0
    mov [file_dir],eax
    inc cl
    mov [name_string.cursor],cl

display_next:
    mov eax,[file_dir]
    test eax,eax
    jnz .l1
    ret
   .l1:
    mov ecx,32
    sub esp,ecx
    mov edi,esp
    call read
    cmp ecx,32
    jnc .l11
   .l10:
    add esp,32
    mov eax,dword[file_dir]
    mov dword[file_dir],0
    jmp close
   .l11:
    mov esi,esp
    movzx edi,byte[name_string.cursor]
    add edi,name_string
    lodsb
    cmp al,0
    je .l10
    cmp al,229
    jne .l0
    add esp,32
    jmp display_next
   .l0:
    stosb
    mov cl,7
   .l2:
    lodsb
    cmp al,32
    jna .l3
    stosb
    loop .l2
   .l3:
    lea esi,[esp+8]
    mov al,'.'
    stosb
    mov cl,3
   .l4:
    lodsb
    cmp al,32
    jna .l5
    stosb
    loop .l4
   .l5:
    mov al,0
    stosb
    cmp edi,name_string.end
    jc .l5
    add esp,32
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
labelt          db  'Jpegview v0.14'
labellen:
setname          db  'SLIDESHOW'
setnamelen:

setbgr           db  '   BGR  '
setbgrlen:

x_pointer       dd  0
x_offset        dd  0
x_numofbytes    dd  0
x_numofb2       dd  0
x_counter       dd  0
winxo:          dw  0
winyo:          dw  0
winxs:          dw  0
winys:          dw  0
jpeg_st:        dd  0
file_dir:       dd  0
work_area:      dd  0
tcolor          dd  0x000000
btcolor         dd  0x224466+0x808080
name_string:    db '/rd/1/jpegview.jpg',0

rb 100
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



