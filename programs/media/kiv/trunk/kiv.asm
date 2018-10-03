format binary as ""

use32
org 0x0

db 'MENUET01'
dd 0x01, START, I_END, F_END, stacktop, @PARAMS, sys_path

;-----------------------------------------------------------------------------

include '../../../config.inc'
include '../../../proc32.inc'
include '../../../macros.inc'
include '../../../dll.inc'
include '../../../KOSfuncs.inc'
;include '../../../debug.inc'

include '../../../develop/libraries/libs-dev/libio/libio.inc'
include '../../../develop/libraries/libs-dev/libimg/libimg.inc'

;include '../../../develop/libraries/box_lib/asm/trunk/opendial.mac'
;use_OpenDialog
;-----------------------------------------------------------------------------

START:
    mcall   SF_SYS_MISC, SSF_HEAP_INIT

    stdcall dll.Load, @IMPORT
    or  eax, eax
    jnz exit

    invoke  sort.START, 1

    mov ecx, 1  ; for 15.4: 1 = tile
    cmp word [@PARAMS], '\T'
    jz  set_bgr
    inc ecx ; for 15.4: 2 = stretch
    cmp word [@PARAMS], '\S'
    jz  set_bgr

    cmp byte [@PARAMS], 0
    jz @f
    mov esi, @PARAMS
    mov edi, path
    mov ecx, 4096/4
    rep movsd
    mov byte [edi-1], 0
@@:
; OpenDialog initialisation
    push    dword OpenDialog_data
    call    [OpenDialog_Init]

; initialize keyboard handling
    invoke  ini_get_shortcut, inifilename, aShortcuts, aNext, -1, next_mod
    mov [next_key], eax
    invoke  ini_get_shortcut, inifilename, aShortcuts, aPrev, -1, prev_mod
    mov [prev_key], eax
    invoke  ini_get_shortcut, inifilename, aShortcuts, aSlide, -1, slide_mod
    mov [slide_key], eax
    invoke  ini_get_shortcut, inifilename, aShortcuts, aTglbar, -1, tglbar_mod
    mov [tglbar_key], eax
    mcall   SF_KEYBOARD, SSF_SET_INPUT_MODE, 1    ; set kbd mode to scancodes

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
    mcall   SF_BACKGROUND_SET, SSF_MODE_BG
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
    push    SF_WAIT_EVENT
    pop eax
    jz  @f
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    mov edx, [cur_frame]
    mov ebx, [cur_frame_time]
    add ebx, [edx + Image.Delay]
    sub ebx, eax
    cmp ebx, [edx + Image.Delay]
    ja  red_update_frame
    test    ebx, ebx
    jz  red_update_frame
    push    SF_WAIT_EVENT_TIMEOUT
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
    mcall   SF_GET_KEY
    test    al, al
    jnz keyloopdone
    shr eax, 8
    mov ecx, eax
    mcall   SF_KEYBOARD, SSF_GET_CONTROL_KEYS
    mov edx, next_mod
    call    check_shortcut
    jz  .next
    add edx, prev_mod - next_mod
    call    check_shortcut
    jz  .prev
    add edx, slide_mod - prev_mod
    call    check_shortcut
    jz  .slide
    add edx, tglbar_mod - slide_mod
    call    check_shortcut
    jz  .tglbar
    cmp cl, 1 ; Esc
    jz  .esc
    jmp keyloop
.esc:
    test byte [bSlideShow], 1
    jnz .slide
    jmp keyloop
.tglbar:
    mov byte [bTglbar], 1
    test byte[bSlideShow], 1
    jnz @f
    xor [toolbar_height], 31
@@:
    jmp keyloop
.slide:
    call slide_show
    jmp keyloop
.prev:
    dec esi
    jmp keyloop
.next:
    inc esi
    jmp keyloop
keyloopdone:
    test esi, esi
    jnz next_or_prev_handler
    test byte [bSlideShow], 2
    jnz red
    test byte [bTglbar], 1
    jnz red
    jmp still
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
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    mov [cur_frame_time], eax
    mcall   SF_THREAD_INFO, procinfo, -1
    call    draw_cur_frame
    jmp still

button:
    mcall   SF_GET_BUTTON
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
    cmp eax, 'sld'
    jne @f

    call    slide_show
    jmp red

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
    mcall   SF_TERMINATE_PROCESS

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

    invoke  img.decode, [img_data], [img_data_len], 0
    or  eax, eax
    jz  .error
    cmp [image], 0
    pushf
    mov [image], eax
    call    img_resize_to_screen
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

align 4
proc img_resize_to_screen uses eax ebx ecx edx
	mov ebx, [image]
	cmp	[ebx+Image.Type],Image.bpp24
	jne .end_f
	test [ebx+Image.Flags],Image.IsAnimated
	jnz .end_f
	mov eax, [ebx+Image.Data]	
	mov [buf_0],eax
	mov eax, [ebx+Image.Width]
	mov [buf_0.w],eax
	mov eax, [ebx+Image.Height]
	mov [buf_0.h],eax

	mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
	mov edx, [image_padding]
	shl edx, 1
	add edx, eax
	mcall SF_GET_SCREEN_SIZE
	mov ecx, eax
	shr ecx, 17

	mov ebx, [image]
	movzx eax,ax
	sub eax, edx
	sub eax, [toolbar_height]
	sub eax, 5-1 ;5 px = border
	cmp eax, 1
	jle .end0
	cmp eax, [ebx+Image.Height]
	jl .end1
	.end0:
		xor eax,eax
		jmp .end2
	.end1:
		mov [ebx+Image.Height],eax
	.end2:
	sub ecx, [image_padding]
	shl ecx, 1
	sub ecx, 10-1 ;10 px = 2 borders
	cmp ecx, 1
	jle .end3
	cmp ecx, [ebx+Image.Width]
	jl .end4
	.end3:
		xor ecx,ecx
		jmp .end5
	.end4:
		mov [ebx+Image.Width],ecx
	.end5:
	cmp eax,ecx
	jne @f
		test eax,eax
		jz .end_f
	@@:
	stdcall [buf2d_resize], buf_0, ecx, eax, 2
.end_f:
	ret
endp

align 4
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
    test    [eax + Image.Flags], Image.IsAnimated
    jnz .not_in_row
    push eax
@@: cmp [eax + Image.Next], 0
    jz  @f
    mov eax, [eax + Image.Next]
    add edx, [eax + Image.Width]
    inc edx
    jmp @b
@@: pop eax
.not_in_row:
    mov [draw_width], edx
    add edx, 19
    cmp edx, 50 + 25*numimages
    jae @f
    mov edx, 50 + 25*numimages
@@:
;    dec edx
    mov [wnd_width], edx
    mov esi, [eax + Image.Height]
    test    [eax + Image.Flags], Image.IsAnimated
    jnz .max_equals_first
    push eax
@@: cmp [eax + Image.Next], 0
    jz  @f
    mov eax, [eax + Image.Next]
    cmp esi, [eax + Image.Height]
    jae @b
    mov esi, [eax + Image.Height]
    jmp @b
@@: pop eax
.max_equals_first:
    mov [draw_height], esi
    add esi, [toolbar_height]
    add esi, [image_padding]
    add esi, [image_padding]
    add esi, 5  ; window bottom frame height
    dec esi
    mov [wnd_height], esi
    popf
    jz  .no_resize
    test [wnd_style], 1 SHL 25
    jz .no_resize
    mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    add esi, eax
    mcall   SF_CHANGE_WINDOW,-1,-1
.no_resize:
    ret

set_as_bgr:
    mov esi, [image]
    mov ecx, [esi + Image.Width]
    mov edx, [esi + Image.Height]
    mcall   SF_BACKGROUND_SET, SSF_SIZE_BG

    mcall   SF_BACKGROUND_SET, SSF_MAP_BG
    test    eax, eax
    jz  @f

    push    eax
    invoke  img.to_rgb2, esi, eax
    pop ecx
    mcall   SF_BACKGROUND_SET, SSF_UNMAP_BG

@@:
    mcall   SF_BACKGROUND_SET, SSF_REDRAW_BG

	;save to file eskin.ini
	xor al,al
	mov ecx,1024
	mov edi,sys_path+2
	repne scasb
	sub edi,sys_path+3
	invoke  ini_set_str, inifileeskin, amain, aprogram, sys_path+2, edi
	;add param '\S__'
	cmp word[@PARAMS],'\S'
	je @f
	mov esi, @PARAMS+4096-8
    mov edi, @PARAMS+4096-4
    mov ecx, 4096/4-1
	std
    rep movsd
	cld
	mov dword[@PARAMS],'\S__'
	@@:
	;
	xor al,al
	mov ecx,4096
	mov edi,@PARAMS
	repne scasb
	sub edi,@PARAMS+1
	invoke  ini_set_str, inifileeskin, amain, aparam, @PARAMS, edi
    ret

slide_show:
    or  byte [bSlideShow], 2
    xor byte [bSlideShow], 1
    btc dword [wnd_style], 25
    jc  @f
    mov eax, [toolbar_height_old]
    mov [toolbar_height], eax
    mov [image_padding], 5
    jmp .toolbar_height_done
@@:
    mov eax, [toolbar_height]
    mov [toolbar_height_old], eax
    mov [toolbar_height], 0
    mov [image_padding], 0
.toolbar_height_done:
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
    mcall   SF_SYS_MISC, SSF_MEM_ALLOC, 0x1000
    test    eax, eax
    jz  .ret
    mov ebx, readdir_fileinfo
    mov dword [ebx+12], (0x1000 - 32) / 304
    mov dword [ebx+16], eax
    mcall   SF_FILE
    cmp eax, 6
    jz  .dirok
    test    eax, eax
    jnz free_directory
    mov edx, [directory_ptr]
    mov ecx, [edx+8]
    mov [readblocks], ecx
    imul    ecx, 304
    add ecx, 32
    mcall   SF_SYS_MISC, SSF_MEM_REALLOC
    test    eax, eax
    jz  free_directory
    mov [directory_ptr], eax
    mcall   SF_FILE, readdir_fileinfo
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
    cmp byte[esi+3], 0
    jne .not_3
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
    cmp ecx, 'tga'
    jz  .copy
    cmp ecx, 'pcx'
    jz  .copy
    cmp ecx, 'xcf'
    jz  .copy
    cmp ecx, 'pbm'
    jz  .copy
    cmp ecx, 'pgm'
    jz  .copy
    cmp ecx, 'pnm'
    jz  .copy
    cmp ecx, 'tif'
    jz  .copy
  .not_3:
    cmp byte[esi+4], 0
    jne .nocopy
    or  ecx, 0x20202020
    cmp ecx, 'tiff'
    jz  @f
    cmp ecx, 'wbmp'
    jz  @f
    cmp ecx, 'jpeg'
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
    mcall   SF_SYS_MISC, SSF_MEM_FREE, [directory_ptr]
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
    mcall   SF_SYSTEM_GET, SSF_TIME_COUNT
    pop ebx
    mov [cur_frame_time], eax
@@:
    pop eax
    ret

draw_window:
    btr  word [bSlideShow], 1  ; mode changed
    jc .mode_changed
    test byte [bTglbar], 1
    jz .mode_not_changed
.mode_changed:
    test [wnd_style], 1 SHL 25
    jz .mode_slide
    mov [bg_color], 0x00ffffff
    mov eax, [image]
    cmp eax, eax
    call update_image_sizes
    mcall SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    mov esi, [wnd_height]
    add esi, eax
    test byte [bTglbar], 1
    jz @f
    mcall SF_CHANGE_WINDOW, -1, -1, [wnd_width], 
    jmp .mode_not_changed
@@:
    mcall SF_CHANGE_WINDOW, [wnd_x], [wnd_y], [wnd_width], 
    jmp .mode_not_changed
.mode_slide:
    mov [bg_color], 0x00000000
    mov eax, [procinfo.box.left]
    mov [wnd_x], eax
    mov eax, [procinfo.box.top]
    mov [wnd_y], eax
    mcall SF_GET_SCREEN_SIZE
    mov edx, eax
    shr edx, 16
    movzx eax, ax
    mov esi, eax
    mcall SF_CHANGE_WINDOW, 0, 0, ,
    jmp .posok.slide_show

.mode_not_changed:
    cmp [bFirstDraw], 0
    jz  .posok
    or  ecx, -1
    mcall   SF_THREAD_INFO, procinfo

    test byte [procinfo.wnd_state], 0x04
    jnz .posok

    mov edx, ecx
    mov esi, ecx
    cmp dword [procinfo.box.width], 50 + 25 * numimages
    jae @f
    mov edx, 50 + 25 * numimages
@@:
    cmp dword [procinfo.box.height], 70
    jae @f
    mov esi, 70
@@:
    mov eax, edx
    and eax, esi
    cmp eax, -1
    jz  @f
    mov ebx, ecx
    mcall   SF_CHANGE_WINDOW
@@:

.posok:
    test [wnd_style], 1 SHL 25
    jz .posok.slide_show
    mcall   SF_REDRAW, SSF_BEGIN_DRAW
    mcall   SF_STYLE_SETTINGS, SSF_GET_SKIN_HEIGHT
    mov ebp, eax    ; save skin height
    add eax, [wnd_height]
    mov ebx, [wnd_x]
    shl ebx, 16
    add ebx, [wnd_width]
    mov ecx, [wnd_y]
    shl ecx, 16
    add ecx, eax
    mcall   SF_CREATE_WINDOW, , , [wnd_style], 0, real_header
    jmp .posok.common
.posok.slide_show:
    mcall   SF_REDRAW, SSF_BEGIN_DRAW
    mcall SF_GET_SCREEN_SIZE
    mov ebx, eax
    shr ebx, 16
    movzx eax, ax
    mov ecx, eax
    mcall   SF_CREATE_WINDOW, , , [wnd_style], 0, real_header
.posok.common:
    mcall   SF_THREAD_INFO, procinfo, -1
    mov eax, [procinfo.client_box.width]
    sub eax, [image_padding]
    sub eax, [image_padding]
    sub eax, [draw_width]
    sar eax, 1
    test eax, eax
    jns @f
    mov eax, 0
@@:
    add eax, [image_padding]
    mov [draw_x], eax
    mov eax, [procinfo.client_box.height]
    sub eax, [toolbar_height]
    sub eax, [image_padding]
    sub eax, [image_padding]
    sub eax, [draw_height]
    sar eax, 1
    test eax, eax
    jns @f
    mov eax, 0
@@:
    add eax, [toolbar_height]
    add eax, [image_padding]
    mov [draw_y], eax
    mov [bFirstDraw], 1
    cmp dword [procinfo.client_box.height], 0
    jle .nodraw
    mov ebx, [procinfo.client_box.width]
    inc ebx
    mov ecx, [draw_y]
    mcall   SF_DRAW_RECT, , , [bg_color]
    mov ecx, [procinfo.client_box.height]
    inc ecx
    mov esi, [cur_frame]
    mov esi, [esi + Image.Height]
    add esi, [draw_y]
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
    mov ebx, [draw_y]
    sub ecx, ebx
    shl ebx, 16
    add ecx, ebx
    mov ebx, [draw_x]
    mcall
    mov esi, [cur_frame]
    mov esi, [esi + Image.Width]
    add esi, ebx
    mov ebx, [procinfo.client_box.width]
    inc ebx
    sub ebx, esi
    jbe @f
    shl esi, 16
    add ebx, esi
    mcall
@@:

    test [wnd_style], 1 SHL 25
    jz .slide_show_mode
    mov byte [bTglbar], 0
    cmp byte [toolbar_height], 0
    je .decorations_done
    mov ebx, [procinfo.client_box.width]
    push    ebx
    mcall   SF_DRAW_LINE, , <30, 30>, 0x007F7F7F
    mcall   , <5 + 25 * 1, 5 + 25 * 1>, <0, 30>
    mcall   , <10 + 25 * 3, 10 + 25 * 3>
    mcall   , <15 + 25 * 5, 15 + 25 * 5>
    pop ebx
    sub ebx, 25 * 5 + 10 
    push    ebx
    imul    ebx, 10001h
    mcall

    mcall   SF_DEFINE_BUTTON, <5 + 25 * 0, 20>, <5, 20>, 'opn'+40000000h
    mcall   , <10 + 25 * 1, 20>, , 'bck'+40000000h
    mcall   , <10 + 25 * 2, 20>, , 'fwd'+40000000h
    mcall   , <15 + 25 * 3, 20>, , 'bgr'+40000000h
    mcall   , <15 + 25 * 4, 20>, , 'sld'+40000000h
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

    mcall   SF_PUT_IMAGE_EXT, buttons+openbtn*20, <20, 20>, <5 + 25 * 0, 5>, 8, palette
    mcall   , buttons+backbtn*20,    , <10 + 25 * 1, 5>
    mcall   , buttons+forwardbtn*20, , <10 + 25 * 2, 5>
    mcall   , buttons+bgrbtn*20,     , <15 + 25 * 3, 5>
    mcall   , buttons+slidebtn*20,   , <15 + 25 * 4, 5>
    mov edx, [procinfo.client_box.width]
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
    jmp .decorations_done

.slide_show_mode:

.decorations_done:
    call    draw_cur_frame

.nodraw:
    mcall   SF_REDRAW, SSF_END_DRAW

    ret

draw_cur_frame:
    push    0   ; ypos
    push    0   ; xpos
    mov eax, [procinfo.client_box.height]
    sub eax, [toolbar_height]
    sub eax, [image_padding]
    inc eax
    push    eax ; max height
    mov eax, [procinfo.client_box.width]
    sub eax, [image_padding]
    inc eax
    push    eax ; max width
    push [draw_y]
    push [draw_x]
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
    mov ebx, [procinfo.client_box.width]
    sub ebx, [image_padding]
    inc ebx
    jbe .done
    mov esi, [draw_x]
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
    mov ecx, [procinfo.client_box.height]
    sub ecx, [toolbar_height]
    sub ecx, [image_padding]
    inc ecx
;    inc ebx
    push    ecx ; max height
    push    ebx ; max width
    push    [draw_y]  ; y
    push    esi ; x
    push    eax ; image
    call    [img.draw]
    pop eax
    cmp [eax + Image.Next], 0
    jnz .afloop
    ret


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
wnd_style        dd 0x73FFFFFF
wnd_x            dd 100
wnd_y            dd 100
image_padding    dd 5
toolbar_height   dd 31
bg_color         dd 0x00ffffff

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
    proc_lib ,'proc_lib.obj',\
	libbuf2d, 'buf2d.obj'


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
    ini_get_shortcut, 'ini_get_shortcut',\
	ini_set_str, 'ini_set_str'

import  sort, sort.START, 'START', SortDir, 'SortDir', strcmpi, 'strcmpi'

import  proc_lib, \
    OpenDialog_Init, 'OpenDialog_init', \
    OpenDialog_Start,'OpenDialog_start'

import  libbuf2d, \
	buf2d_init, 'lib_init', \
	buf2d_resize, 'buf2d_resize'

align 4
buf_0: dd 0
	dw 0,0
.w: dd 0
.h: dd 0,0
	db 24 ;+20 bit in pixel

bFirstDraw  db  0
bSlideShow  db  0
bTglbar     db  0
;-----------------------------------------------------------------------------

virtual at 0
file 'kivicons.bmp':0xA,4
load offbits dword from 0
end virtual
numimages = 10
openbtn = 0
backbtn = 1
forwardbtn = 2
bgrbtn = 3
fliphorzbtn = 4
flipvertbtn = 5
rotcwbtn = 6
rotccwbtn = 7
rot180btn = 8
slidebtn = 9

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

inifilename db  '/sys/settings/app.ini',0
aShortcuts  db  'Kiv',0
aNext       db  'Next',0
aPrev       db  'Prev',0
aSlide      db  'SlideShow',0
aTglbar     db  'ToggleBar',0

inifileeskin db '/sys/settings/eskin.ini',0
amain       db 'main',0
aprogram    db 'program',0
aparam      db 'param',0

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
.x_size                 dw 420 ;+48 ; Window X size
.x_start                dw 10 ;+50 ; Window X position
.y:
.y_size                 dw 320 ;+52 ; Window y size
.y_start                dw 10 ;+54 ; Window Y position

communication_area_name:
    db 'FFFFFFFF_open_dialog',0

open_dialog_path:
if __nightbuild eq yes
    db '/sys/MANAGERS/opendial',0
else
    db '/sys/File Managers/opendial',0
end if
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
db 'TGA',0
db 'PCX',0
db 'XCF',0
db 'PBM',0
db 'PGM',0
db 'PNM',0
db 'TIF',0
db 'TIFF',0
db 'WBMP',0
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
wnd_width   dd  100
wnd_height  dd  100
draw_x      dd  ?
draw_y      dd  ?
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
slide_mod   dd  ?
slide_key   dd  ?
tglbar_mod  dd  ?
tglbar_key  dd  ?

toolbar_height_old   rd 1

procinfo    process_information
align 16
path:       rb  4096  ;1024+16
real_header rb  256
@PARAMS rb 4096  ;512
;---------------------------------------------------------------------
sys_path rb 1024
temp_dir_pach:
        rb 4096
;---------------------------------------------------------------------
    rb 4096
stacktop:
;---------------------------------------------------------------------
F_END:
