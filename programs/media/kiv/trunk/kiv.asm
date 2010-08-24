use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, F_END, stacktop, @PARAMS, 0x0

;-----------------------------------------------------------------------------

FALSE = 0
TRUE  = 1

include '../../../proc32.inc'
include '../../../macros.inc'
include 'dll.inc'

include '../../../develop/libraries/libs-dev/libio/libio.inc'
include '../../../develop/libraries/libs-dev/libimg/libimg.inc'

;include '../../../develop/libraries/box_lib/asm/trunk/opendial.mac'
;use_OpenDialog
;-----------------------------------------------------------------------------

START:
    mcall   68, 11

    stdcall dll.Load, @IMPORT
    or  eax, eax
    jnz exit

    invoke  sort.START, 1

; OpenDialog initialisation
    push    dword OpenDialog_data
    call    [OpenDialog_Init]

    mov ecx, 1  ; for 15.4: 1 = tile
    cmp word [@PARAMS], '\T'
    jz  set_bgr
    inc ecx ; for 15.4: 2 = stretch
    cmp word [@PARAMS], '\S'
    jz  set_bgr

; initialize keyboard handling
    invoke  ini_get_shortcut, inifilename, aShortcuts, aNext, -1, next_mod
    mov [next_key], eax
    invoke  ini_get_shortcut, inifilename, aShortcuts, aPrev, -1, prev_mod
    mov [prev_key], eax
    mcall   66, 1, 1    ; set kbd mode to scancodes

    cmp byte [@PARAMS], 0
    jnz params_given

    mov [OpenDialog_data.draw_window],draw_window_fake
    
; OpenDialog Open
    push    dword OpenDialog_data
    call    [OpenDialog_Start]

    cmp [OpenDialog_data.status],1
    jne exit

    mov [OpenDialog_data.draw_window],draw_window

    mov esi, path
    mov edi, @PARAMS
    mov ecx, 4096/4
    rep movsd
    mov byte [edi-1], 0
    jmp params_given

set_bgr:
    mcall   15, 4
    mov eax, @PARAMS + 4
    call    load_image
    jc  exit

    call    set_as_bgr
    jmp exit

params_given:

    mov esi, @PARAMS
    push    esi
    call    find_last_name_component

    pop eax
    call    load_image
    jc  exit
    call    generate_header

;-----------------------------------------------------------------------------

red:
    call    draw_window

still:
    mov eax, [image]
    test    byte [eax + Image.Flags], Image.IsAnimated
    push    10
    pop eax
    jz  @f
    mcall   26, 9
    mov edx, [cur_frame]
    mov ebx, [cur_frame_time]
    add ebx, [edx + Image.Delay]
    sub ebx, eax
    cmp ebx, [edx + Image.Delay]
    ja  red_update_frame
    test    ebx, ebx
    jz  red_update_frame
    push    23
    pop eax
  @@:
    mcall
    dec eax
    js  red_update_frame
    jz  red
    dec eax
    jnz button

key:
    xor esi, esi
keyloop:
    mcall   2
    test    al, al
    jnz keyloopdone
    shr eax, 8
    mov ecx, eax
    mcall   66, 3
    mov edx, next_mod
    call    check_shortcut
    jz  .next
    add edx, prev_mod - next_mod
    call    check_shortcut
    jnz keyloop
.prev:
    dec esi
    jmp keyloop
.next:
    inc esi
    jmp keyloop
keyloopdone:
    test    esi, esi
    jz  still
next_or_prev_handler:
    call    next_or_prev_image
    jmp red

red_update_frame:
    mov eax, [cur_frame]
    mov eax, [eax + Image.Next]
    test    eax, eax
    jnz @f
    mov eax, [image]
  @@:
    mov [cur_frame], eax
    mcall   26, 9
    mov [cur_frame_time], eax
    mcall   9, procinfo, -1
    call    draw_cur_frame
    jmp still

button:
    mcall   17
    shr eax, 8

    ; flip horizontally
    cmp eax, 'flh'
    jne @f

    invoke  img.flip, [image], FLIP_HORIZONTAL
    jmp redraw_image

    ; flip vertically
    @@: cmp eax, 'flv'
    jne @f

    invoke  img.flip, [image], FLIP_VERTICAL
    jmp redraw_image

    ; flip both horizontally and vertically
    @@: cmp eax, 'flb'
    jne @f

    invoke  img.flip, [image], FLIP_BOTH
    jmp redraw_image

    ; rotate left
    @@: cmp eax, 'rtl'
    jne @f

    push    ROTATE_90_CCW
.rotate_common:
    invoke  img.rotate, [image]
    mov eax, [image]
    test    eax, eax    ; clear ZF flag
    call    update_image_sizes
    jmp redraw_image

    ; rotate right
    @@: cmp eax, 'rtr'
    jne @f

    push    ROTATE_90_CW
    jmp .rotate_common

    ; open new file
    @@: cmp eax, 'opn'
    jne @f
    
; OpenDialog Open
    push    dword OpenDialog_data
    call    [OpenDialog_Start]
    
    cmp [OpenDialog_data.status],1
    jne still
    
    mov esi, path
    mov edi, @PARAMS
    push    edi
    mov ecx, 4096/4
    rep movsd
    mov byte [edi-1], 0
    
    pop esi
    push    esi
    call    find_last_name_component

    pop eax 
    push    [image]
    call    load_image
    jc  .restore_old
    call    generate_header
    
    invoke  img.destroy
    call    free_directory
    jmp red
    
    .restore_old:
    pop [image]
    call    init_frame
    jmp still

    ; set background
    @@:
    cmp eax, 'bgr'
    jne @f

    call    set_as_bgr
    jmp still

    @@:

    or  esi, -1
    cmp eax, 'bck'
    jz  next_or_prev_handler
    neg esi
    cmp eax, 'fwd'
    jz  next_or_prev_handler

    cmp eax, 1
    jne still

  exit:
    mcall   -1

  redraw_image = red

load_image:
    and [img_data], 0
    push    eax
    invoke  file.open, eax, O_READ
    or  eax, eax
    jz  .error_pop
    mov [fh], eax
    invoke  file.size
    mov [img_data_len], ebx
    stdcall mem.Alloc, ebx
    test    eax, eax
    jz  .error_close
    mov [img_data], eax
    invoke  file.read, [fh], eax, [img_data_len]
    cmp eax, -1
    jz  .error_close
    cmp eax, [img_data_len]
    jnz .error_close
    invoke  file.close, [fh]
    inc eax
    jz  .error

; img.decode checks for img.is_img
;   invoke  img.is_img, [img_data], [img_data_len]
;   or  eax, eax
;   jz  exit
    invoke  img.decode, [img_data], [img_data_len], 0
    or  eax, eax
    jz  .error
    cmp [image], 0
    pushf
    mov [image], eax
    call    init_frame
    popf
    call    update_image_sizes
    call    free_img_data
    clc
    ret

.error_free:
    invoke  img.destroy, [image]
    jmp .error

.error_pop:
    pop eax
    jmp .error
.error_close:
    invoke  file.close, [fh]
.error:
    call    free_img_data
    stc
    ret

free_img_data:
    mov eax, [img_data]
    test    eax, eax
    jz  @f
    stdcall mem.Free, eax
@@:
    ret

update_image_sizes:
    pushf
    mov edx, [eax + Image.Width]
    mov [draw_width], edx
    add edx, 19
    cmp edx, 40 + 25*9
    jae @f
    mov edx, 40 + 25*9
@@:
    mov [wnd_width], edx
    mov esi, [eax + Image.Height]
    mov [draw_height], esi
    add esi, 44
    mov [wnd_height], esi
    popf
    jz  .no_resize
    mcall   48, 4
    add esi, eax
    mcall   67,-1,-1
.no_resize:
    ret

set_as_bgr:
    mov esi, [image]
    mov ecx, [esi + Image.Width]
    mov edx, [esi + Image.Height]
    mcall   15, 1

    mcall   15, 6
    test    eax, eax
    jz  @f

    push    eax
    invoke  img.to_rgb2, esi, eax
    pop ecx
    mcall   15, 7

@@:
    mcall   15, 3
    ret

; seek to ESI image files
; esi>0 means next file, esi<0 - prev file
next_or_prev_image:
    push    esi
    call    load_directory
    pop esi
    mov ebx, [directory_ptr]
    test    ebx, ebx
    jz  .ret
    cmp dword[ebx+4], 0
    jz  .ret
    mov eax, [cur_file_idx]
    cmp eax, -1
    jnz @f
    test    esi, esi
    jns @f
    mov eax, [ebx+4]
@@:
    push    [image]
    add eax, esi
@@:
    test    eax, eax
    jns @f
    add eax, [ebx+4]
    jmp @b
@@:
    cmp eax, [ebx+4]
    jb  @f
    sub eax, [ebx+4]
    jmp @b
@@:
    push    eax
.scanloop:
    push    eax ebx esi
    imul    esi, eax, 304
    add esi, [directory_ptr]
    add esi, 32 + 40
    mov edi, curdir
@@:
    inc edi
    cmp byte [edi-1], 0
    jnz @b
    mov byte [edi-1], '/'
@@:
    lodsb
    stosb
    test    al, al
    jnz @b
    mov eax, curdir
    call    load_image
    pushf
    mov esi, curdir
    push    esi
    mov edi, @PARAMS
    mov ecx, 4096/4
    rep movsd
    mov byte [edi-1], 0
    pop esi
@@:
    lodsb
    test    al, al
    jnz @b
@@:
    dec esi
    cmp byte [esi], '/'
    jnz @b
    mov byte [esi], 0
    popf
    pop esi ebx eax
    jnc .loadedok
    test    esi, esi
    js  .try_prev
.try_next:
    inc eax
    cmp eax, [ebx+4]
    jb  @f
    xor eax, eax
@@:
.try_common:
    cmp eax, [esp]
    jz  .notfound
    jmp .scanloop
.try_prev:
    dec eax
    jns @f
    mov eax, [ebx+4]
    dec eax
@@:
    jmp .try_common
.loadedok:
    mov [cur_file_idx], eax
    pop eax
    invoke  img.destroy
    call    generate_header
.ret:
    ret
.notfound:
    pop eax
    pop [image]
    call    init_frame
    ret

load_directory:
    cmp [directory_ptr], 0
    jnz .ret
    mov esi, @PARAMS
    mov edi, curdir
    mov ecx, [last_name_component]
    sub ecx, esi
    dec ecx
    js  @f
    rep movsb
@@:
    mov byte [edi], 0
    mcall   68, 12, 0x1000
    test    eax, eax
    jz  .ret
    mov ebx, readdir_fileinfo
    mov dword [ebx+12], (0x1000 - 32) / 304
    mov dword [ebx+16], eax
    mcall   70
    cmp eax, 6
    jz  .dirok
    test    eax, eax
    jnz free_directory
    mov edx, [directory_ptr]
    mov ecx, [edx+8]
    mov [readblocks], ecx
    imul    ecx, 304
    add ecx, 32
    mcall   68, 20
    test    eax, eax
    jz  free_directory
    mov [directory_ptr], eax
    mcall   70, readdir_fileinfo
.dirok:
    cmp ebx, 0
    jle free_directory
    mov eax, [directory_ptr]
    add eax, 32
    mov edi, eax
    push    0
.dirskip:
    push    eax
    test    byte [eax], 18h
    jnz .nocopy
    lea esi, [eax+40]
    mov ecx, esi
@@:
    lodsb
    test    al, al
    jnz @b
@@:
    dec esi
    cmp esi, ecx
    jb  .noext
    cmp byte [esi], '.'
    jnz @b
    inc esi
    mov ecx, [esi]
    or  ecx, 0x202020
    cmp ecx, 'jpg'
    jz  .copy
    cmp ecx, 'bmp'
    jz  .copy
    cmp ecx, 'gif'
    jz  .copy
    cmp ecx, 'png'
    jz  .copy
    cmp ecx, 'jpe'
    jz  .copy
    cmp ecx, 'ico'
    jz  .copy
    cmp ecx, 'cur'
    jz  .copy
; dunkaist [
    cmp ecx, 'pcx'
    jz  .copy
; dunkaist ]
    cmp ecx, 'jpeg'
    jz  @f
    cmp ecx, 'jpeG'
    jnz .nocopy
@@:
    cmp byte [esi+4], 0
    jnz .nocopy
.copy:
    mov esi, [esp]
    mov ecx, 304 / 4
    rep movsd
    inc dword [esp+4]
.nocopy:
.noext:
    pop eax
    add eax, 304
    dec ebx
    jnz .dirskip
    mov eax, [directory_ptr]
    pop ebx
    mov [eax+4], ebx
    test    ebx, ebx
    jz  free_directory
    push    0   ; sort mode
    push    ebx
    add eax, 32
    push    eax
    call    [SortDir]
    xor eax, eax
    mov edi, [directory_ptr]
    add edi, 32 + 40
.scan:
    mov esi, [last_name_component]
    push    edi
    invoke  strcmpi
    pop edi
    jz  .found
    inc eax
    add edi, 304
    dec ebx
    jnz .scan
    or  eax, -1
.found:
    mov [cur_file_idx], eax
.ret:
    ret

free_directory:
    mcall   68, 13, [directory_ptr]
    and [directory_ptr], 0
    ret

; in: esi->full name (e.g. /path/to/file.png)
; out: [last_name_component]->last component (e.g. file.png)
find_last_name_component:
    mov ecx, esi
@@:
    lodsb
    test    al, al
    jnz @b
@@:
    dec esi
    cmp esi, ecx
    jb  @f
    cmp byte [esi], '/'
    jnz @b
@@:
    inc esi
    mov [last_name_component], esi
    ret

init_frame:
    push    eax
    mov eax, [image]
    mov [cur_frame], eax
    test    byte [eax + Image.Flags], Image.IsAnimated
    jz  @f
    push    ebx
    mcall   26, 9
    pop ebx
    mov [cur_frame_time], eax
@@:
    pop eax
    ret

draw_window:
    cmp [bFirstDraw], 0
    jz  .posok
    or  ecx, -1
    mcall   9, procinfo

    cmp dword [ebx + 66], 0
    jle .posok

    mov edx, ecx
    mov esi, ecx
    cmp dword [ebx + 42], 40 + 25 * 9
    jae @f
    mov edx, 40 + 25 * 9
@@:
    cmp dword [ebx + 46], 70
    jae @f
    mov esi, 70
@@:
    mov eax, edx
    and eax, esi
    cmp eax, -1
    jz  @f
    mov ebx, ecx
    mcall   67
@@:

.posok:
    mcall   12, 1
    mcall   48, 4
    mov ebp, eax    ; save skin height
    add eax, [wnd_height]
    __mov   ebx, 100, 0
    add ebx, [wnd_width]
    lea ecx, [100*65536 + eax]
    mcall   0, , , 0x73FFFFFF, 0, real_header

    mcall   9, procinfo, -1
    mov [bFirstDraw], 1
    cmp dword [ebx + 66], 0
    jle .nodraw
    mov ebx, [ebx + 62]
    inc ebx
    mcall   13, , <0, 35>, 0xFFFFFF
    mov ecx, [procinfo + 66]
    inc ecx
    mov esi, [draw_height]
    add esi, 35
    sub ecx, esi
    jbe @f
    push    esi
    shl esi, 16
    add ecx, esi
    pop esi
    mcall
    xor ecx, ecx
@@:
    add ecx, esi
    add ecx, 35*10000h - 35
    __mov   ebx, 0, 5
    mcall
    mov esi, [draw_width]
    add esi, ebx
    mov ebx, [procinfo+62]
    inc ebx
    sub ebx, esi
    jbe @f
    shl esi, 16
    add ebx, esi
    mcall
@@:

    mov ebx, [procinfo + 62]
    push    ebx
    mcall   38, , <30, 30>, 0x007F7F7F
    mcall   , <5 + 25 * 1, 5 + 25 * 1>, <0, 30>
    mcall   , <10 + 25 * 3, 10 + 25 * 3>
    mcall   , <15 + 25 * 4, 15 + 25 * 4>
    pop ebx
    sub ebx, 25 * 5 + 10
    push    ebx
    imul    ebx, 10001h
    mcall

    mcall   8, <5 + 25 * 0, 20>, <5, 20>, 'opn'+40000000h
    mcall   , <10 + 25 * 1, 20>, , 'bck'+40000000h
    mcall   , <10 + 25 * 2, 20>, , 'fwd'+40000000h
    mcall   , <15 + 25 * 3, 20>, , 'bgr'+40000000h
    pop ebx
    add ebx, 5
    shl ebx, 16
    mov bl, 20
    mcall   , , , 'flh'+40000000h
    add ebx, 25 * 65536
    mcall   , , , 'flv'+40000000h
    add ebx, 30 * 65536
    mcall   , , , 'rtr'+40000000h
    add ebx, 25 * 65536
    mcall   , , , 'rtl'+40000000h
    add ebx, 25 * 65536
    mcall   , , , 'flb'+40000000h

    mov ebp, (numimages-1)*20

    mcall   65, buttons+openbtn*20, <20, 20>, <5 + 25 * 0, 5>, 8, palette
    mcall   , buttons+backbtn*20, , <10 + 25 * 1, 5>
    mcall   , buttons+forwardbtn*20, , <10 + 25 * 2, 5>
    mcall   , buttons+bgrbtn*20, , <15 + 25 * 3, 5>
    mov edx, [procinfo + 62]
    sub edx, 25 * 5 + 4
    shl edx, 16
    mov dl, 5
    mcall   , buttons+fliphorzbtn*20
    add edx, 25 * 65536
    mcall   , buttons+flipvertbtn*20
    add edx, 30 * 65536
    mcall   , buttons+rotcwbtn*20
    add edx, 25 * 65536
    mcall   , buttons+rotccwbtn*20
    add edx, 25 * 65536
    mcall   , buttons+rot180btn*20

    call    draw_cur_frame

.nodraw:
    mcall   12, 2

    ret

draw_cur_frame:
    push    0   ; ypos
    push    0   ; xpos
    mov eax, [procinfo+66]
    sub eax, 34
    push    eax ; max height
    mov eax, [procinfo+62]
    sub eax, 4
    push    eax ; max width
    push    35  ; y
    push    5   ; x
    push    [cur_frame]
    call    [img.draw]
    mov eax, [image]
    test    [eax + Image.Flags], Image.IsAnimated
    jnz .done
    cmp [eax + Image.Next], 0
    jnz .additional_frames
.done:
    ret
.additional_frames:
    mov ebx, [procinfo+62]
    sub ebx, 4
    jbe .done
    push    5
    pop esi
.afloop:
    sub ebx, [eax + Image.Width]
    jbe .done
    dec ebx
    jz  .done
    add esi, [eax + Image.Width]
    mov eax, [eax + Image.Next]
    push    eax
    inc esi
    push    0   ; ypos
    push    0   ; xpos
    mov ecx, [procinfo+66]
    sub ecx, 34
    push    ecx ; max height
    push    ebx ; max width
    push    35  ; y
    push    esi ; x
    push    eax ; image
    call    [img.draw]
    pop eax
    cmp [eax + Image.Next], 0
    jnz .afloop
    ret

; void* __stdcall mem.Alloc(unsigned size);
mem.Alloc:
    push    ebx ecx
    mov ecx, [esp+12]
    mcall   68, 12
    pop ecx ebx
    ret 4

; void* __stdcall mem.ReAlloc(void* mptr, unsigned size);
mem.ReAlloc:
    push    ebx ecx edx
    mov edx, [esp+16]
    mov ecx, [esp+20]
    mcall   68, 20
    pop edx ecx ebx
    ret 8

; void __stdcall mem.Free(void* mptr);
mem.Free:
    push    ebx ecx
    mov ecx, [esp+12]
    mcall   68, 13
    pop ecx ebx
    ret 4

check_shortcut:
; in:   cl = scancode (from sysfn 2),
;   eax = state of modifiers (from sysfn 66.3),
;   edx -> shortcut descriptor
; out:  ZF set <=> fail
    cmp cl, [edx+4]
    jnz .not
    push    eax
    mov esi, [edx]
    and esi, 0xF
    and al, 3
    call    dword [check_modifier_table+esi*4]
    test    al, al
    pop eax
    jnz .not
    push    eax
    mov esi, [edx]
    shr esi, 4
    and esi, 0xF
    shr al, 2
    and al, 3
    call    dword [check_modifier_table+esi*4]
    test    al, al
    pop eax
    jnz .not
    push    eax
    mov esi, [edx]
    shr esi, 8
    and esi, 0xF
    shr al, 4
    and al, 3
    call    dword [check_modifier_table+esi*4]
    test    al, al
    pop eax
;   jnz .not
.not:
    ret

check_modifier_0:
    setnz   al
    ret
check_modifier_1:
    setp    al
    ret
check_modifier_2:
    cmp al, 3
    setnz   al
    ret
check_modifier_3:
    cmp al, 1
    setnz   al
    ret
check_modifier_4:
    cmp al, 2
    setnz   al
    ret

; fills real_header with window title
; window title is generated as '<filename> - Kolibri Image Viewer'
generate_header:
    push    eax
    mov esi, [last_name_component]
    mov edi, real_header
@@:
    lodsb
    test    al, al
    jz  @f
    stosb
    cmp edi, real_header+256
    jb  @b
.overflow:
    mov dword [edi-4], '...'
.ret:
    pop eax
    ret
@@:
    mov esi, s_header
@@:
    lodsb
    stosb
    test    al, al
    jz  .ret
    cmp edi, real_header+256
    jb  @b
    jmp .overflow
;-----------------------------------------------------------------------------

s_header db ' - Kolibri Image Viewer', 0

;-----------------------------------------------------------------------------
;-----------------------------------------------------------------------------

align 4
@IMPORT:

library             \
    libio  , 'libio.obj'  , \
    libgfx , 'libgfx.obj' , \
    libimg , 'libimg.obj' , \
    libini , 'libini.obj' , \
    sort   , 'sort.obj'   , \
    proc_lib ,'proc_lib.obj'


import  libio             , \
    libio.init , 'lib_init'   , \
    file.size  , 'file_size'  , \
    file.open  , 'file_open'  , \
    file.read  , 'file_read'  , \
    file.close , 'file_close'

import  libgfx              , \
    libgfx.init   , 'lib_init'  , \
    gfx.open      , 'gfx_open'  , \
    gfx.close     , 'gfx_close' , \
    gfx.pen.color , 'gfx_pen_color' , \
    gfx.line      , 'gfx_line'

import  libimg             , \
    libimg.init , 'lib_init'   , \
    img.is_img  , 'img_is_img' , \
    img.to_rgb2 , 'img_to_rgb2', \
    img.decode  , 'img_decode' , \
    img.flip    , 'img_flip'   , \
    img.rotate  , 'img_rotate' , \
    img.destroy , 'img_destroy', \
    img.draw    , 'img_draw'

import  libini, \
    ini_get_shortcut, 'ini_get_shortcut'

import  sort, sort.START, 'START', SortDir, 'SortDir', strcmpi, 'strcmpi'

import  proc_lib, \
    OpenDialog_Init, 'OpenDialog_init', \
    OpenDialog_Start,'OpenDialog_start'

bFirstDraw  db  0
;-----------------------------------------------------------------------------

virtual at 0
file 'kivicons.bmp':0xA,4
load offbits dword from 0
end virtual
numimages = 9
openbtn = 0
backbtn = 1
forwardbtn = 2
bgrbtn = 3
fliphorzbtn = 4
flipvertbtn = 5
rotcwbtn = 6
rotccwbtn = 7
rot180btn = 8

palette:
    file 'kivicons.bmp':0x36,offbits-0x36
buttons:
    file 'kivicons.bmp':offbits
repeat 10
y = % - 1
z = 20 - %
repeat numimages*5
load a dword from $ - numimages*20*20 + numimages*20*y + (%-1)*4
load b dword from $ - numimages*20*20 + numimages*20*z + (%-1)*4
store dword a at $ - numimages*20*20 + numimages*20*z + (%-1)*4
store dword b at $ - numimages*20*20 + numimages*20*y + (%-1)*4
end repeat
end repeat

inifilename db  '/sys/media/kiv.ini',0
aShortcuts  db  'Shortcuts',0
aNext       db  'Next',0
aPrev       db  'Prev',0

align 4
check_modifier_table:
    dd  check_modifier_0
    dd  check_modifier_1
    dd  check_modifier_2
    dd  check_modifier_3
    dd  check_modifier_4

;---------------------------------------------------------------------
align 4
OpenDialog_data:
.type           dd 0
.procinfo       dd procinfo ;+4
.com_area_name      dd communication_area_name ;+8
.com_area       dd 0 ;+12
.opendir_pach       dd temp_dir_pach ;+16
.dir_default_pach   dd communication_area_default_pach ;+20
.start_path     dd open_dialog_path ;+24
.draw_window        dd draw_window ;+28
.status         dd 0 ;+32
.openfile_pach      dd path  ;openfile_pach ;+36
.filename_area      dd 0    ;+40
.filter_area        dd Filter
.x:
.x_size			dw 420 ;+48 ; Window X size
.x_start		dw 10 ;+50 ; Window X position
.y:
.y_size			dw 320 ;+52 ; Window y size
.y_start		dw 10 ;+54 ; Window Y position

communication_area_name:
    db 'FFFFFFFF_open_dialog',0
open_dialog_path:
    db '/sys/File Managers/opendial',0
communication_area_default_pach:
    db '/rd/1',0

Filter:
dd Filter.end - Filter
.1:
db 'BMP',0
db 'GIF',0
db 'JPG',0
db 'JPEG',0
db 'JPE',0
db 'PNG',0
db 'ICO',0
db 'CUR',0
; dunkaist [
db 'PCX',0
; dunkaist ]
.end:
db 0

draw_window_fake:
    ret
;------------------------------------------------------------------------------
readdir_fileinfo:
    dd  1
    dd  0
    dd  0
readblocks dd   0
directory_ptr   dd  0
;------------------------------------------------------------------------------
I_END:
curdir      rb  1024

align 4
img_data     dd ?
img_data_len dd ?
fh       dd ?
image        dd ?
wnd_width   dd  ?
wnd_height  dd  ?
draw_width  dd  ?
draw_height dd  ?
last_name_component dd  ?
cur_file_idx    dd  ?
cur_frame_time  dd  ?
cur_frame   dd  ?

next_mod    dd  ?
next_key    dd  ?
prev_mod    dd  ?
prev_key    dd  ?

procinfo:   rb  1024
path:       rb  4096  ;1024+16
real_header rb  256
@PARAMS rb 4096  ;512
;---------------------------------------------------------------------
temp_dir_pach:
        rb 4096
;---------------------------------------------------------------------
    rb 4096
stacktop:
;---------------------------------------------------------------------
F_END: