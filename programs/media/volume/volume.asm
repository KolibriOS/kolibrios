; 1.0.1

 DEBUG equ 0
 WIN_SIZE equ 120
 TIMER equ 60

    use32
    org     0
    db	    'MENUET01'
    dd	    1, daemon_entry, @end, @memory, @stack, @params, 0

    include "../../macros.inc"
    include "../../proc32.inc"
    include "../../dll.inc"
if DEBUG eq 1
    include "../../debug.inc"
end if
;=====================================================================

 daemon_entry:
    mcall   68, 11
    mcall   68, 22, shm_name, 512, 5
    mov     [shm], eax

    mcall   9, buffer, -1
    mov     esi, dword[buffer + 30]
    mov     edi, eax

  @@:
    mcall   9, buffer, edi
    cmpe    dword[buffer + 30], esi, .next
    cmpe    dword[buffer + 10], dword "@Vol", open_1
    cmpe    dword[buffer + 10], dword "@VOL", open_1
    cmpe    dword[buffer + 10], dword "@vol", open_1
if DEBUG eq 1
    cmpe    dword[buffer + 10], dword "Volu", open_1
    cmpe    dword[buffer + 10], dword "VOLu", open_1
    cmpe    dword[buffer + 10], dword "volu", open_1
end if
 .next:
    dec     edi
    jnz     @b

    mov     eax, [shm]
    mov     [eax], dword 0
    mcall   40, 10b
    mcall   66, 1, 1
    mcall   66, 4, 77, 0x110
    mcall   66, 4, 75, 0x110
    mcall   66, 4, 80, 0x110
if DEBUG eq 1
    mcall   66, 4, 16, 0x110
end if

    stdcall dll.Load, @imports
    stdcall dword[img.decode], icons, icons.size, 0
    mov     dword[image.data], eax
    stdcall dword[img.to_rgb], dword[image.data], image
    stdcall dword[img.destroy], dword[image.data]

 ; load driver
    mcall   68, 16, snd_driver.name
    mov     [snd_driver], eax
    cmpe    eax, 0, exit

    mcall   70, is_load
    cmpe    eax, 0, @f
    mov     dword[volume], 5
  @@:
    mcall   70, is_save
    call    set_sound_proc

 update:
    mcall   23, 5
    cmpe    al, EV_KEY, key
    mov     eax, [shm]
    cmpne   [eax], dword 0, open_2
    jmp     update

 open_1:
    mov     ebx, 1
    cmpne   [@params], byte '+', @f
    mov     ebx, 2
  @@:
    cmpne   [@params], byte '-', @f
    mov     ebx, 3
  @@:
    cmpne   [@params], byte 'm', @f
    mov     ebx, 4
  @@:
    mov     eax, [shm]
    mov     [eax], ebx
    jmp     exit

 open_2:
    mov     eax, [shm]
    mov     ebx, [eax]
    mov     [command], ebx
    mov     [eax], dword 0
    cmpne   [win.pid], dword 0, update
    mcall   51, 1, _entry, _stack
    jmp     update

 key:
    mcall   2
if DEBUG
    cmpe    ah, 16, exit
end if
    mov     edx, [shm]
    cmpne   ah, 77, @f
 .cm_1:
    cmpne   [win.pid], 0, .else_1
    mov     [edx], dword 2
    jmp     open_2
 .else_1:
    mov     [command], 2
  @@:
    cmpne   ah, 75, @f
 .cm_2:
    cmpne   [win.pid], 0, .else_2
    mov     [edx], dword 3
    jmp     open_2
 .else_2:
    mov     [command], 3
  @@:
    cmpne   ah, 80, @f
 .cm_3:
    mov     [edx], dword 4
    jmp     open_2
  @@:
    jmp     update

 exit:
if DEBUG eq 1
    dps     "EXIT"
end if
    mcall   -1

;=====================================================================

 _entry:
    mcall   40, 100111b

    mcall   9, buffer, -1
    mov     ecx, eax
    mov     edx, dword [buffer + 30]
    mov     [win.pid], edx

    mcall   14
    movzx   ebx, ax
    shr     eax, 17
    shr     ebx, 1
    sub     eax, WIN_SIZE / 2
    sub     ebx, WIN_SIZE / 2
    mov     [win.x], eax
    mov     [win.y], ebx

    mcall   70, is_load
    cmpe    eax, 0, @f
    mov     dword[volume], 5
  @@:
    mov     dword[timer], TIMER

    jmp     set_sound

 ;----------------------------

 _update:
    mcall   23, 5
    cmpe    al, EV_REDRAW, _redraw
    cmpe    al, EV_KEY, _key
    cmpe    al, EV_BUTTON, _button
    cmpe    al, EV_MOUSE, _mouse

    cmpne   [command], 2, @f
    mov     [mute], 0
    cmpe    [volume], dword 10, @f
    inc     dword[volume]
    jmp     .apply
  @@:
    cmpne   [command], 3, @f
    mov     [mute], 0
    cmpe    [volume], dword 0, @f
    dec     dword[volume]
    jmp     .apply
  @@:
    cmpne   [command], 4, @f
    mov     eax, 1
    sub     eax, [mute]
    mov     [mute], eax
    jmp     .apply

 .apply:
    mov     [command], 0
    jmp     set_sound
  @@:

    mcall   18, 7
    mov     ecx, eax
    mcall   9, buffer
    mov     eax, dword[buffer + 30]
    cmpne   [win.pid], eax, _exit

    dec     dword[timer]
    jnz     _update

 ;----------------------------

 _exit:
if DEBUG eq 1
    dps     "CLOSE"
end if
    mov     [win.pid], 0
    mcall   70, is_save
    mcall   -1

 ;----------------------------

 _button:
    mcall   17
    cmpe    ah, 1, _exit
    cmpe    ah, 3, toggle_mute
    jmp     _update

 ;----------------------------

 _key:
    mcall   2
    cmpe    ah, 027, _exit
    cmpe    ah, 176, dec_volume ; <-
    cmpe    ah, 183, dec_volume ; PgDown
    cmpe    ah, 179, inc_volume ; ->
    cmpe    ah, 184, inc_volume ; PgUp
    cmpe    ah, 178, unmute_volume ; ^
    cmpe    ah, 180, unmute_volume ; Home
    cmpe    ah, 177, toggle_mute ; v
    cmpe    ah, 181, toggle_mute ; End

    jmp     _update

 ;----------------------------

 _mouse:
    mcall   37, 7
    cmpe    eax, 0, _update

    mov     esi, eax
    mcall   37, 1
    movzx   ebx, ax
    shr     eax, 16
    cmpg    eax, WIN_SIZE, _update
    cmpg    ebx, WIN_SIZE, _update

    cmpe    si, 1,  dec_volume
    jne     inc_volume

 ;----------------------------

 dec_volume:
    cmpe    dword[volume], 0, _update
    dec     dword[volume]
    jmp     unmute_volume

 inc_volume:
    cmpe    dword[volume], 10, _update
    inc     dword[volume]
    jmp     unmute_volume

 toggle_mute:
    mov     eax, 1
    sub     eax, [mute]
    mov     [mute], eax
    jmp     @f

 unmute_volume:
    mov     dword[mute], 0

 set_sound:
  @@:
    call    set_sound_proc
    mov     dword[timer], TIMER
    call    draw_icon
    call    draw_bar
    jmp     _update

 set_sound_proc:
    mov     [snd_driver.command], 6
    mov     [snd_driver.inputsz], 4
    mov     [snd_driver.output], 0
    mov     [snd_driver.outputsz], 0
    mov     [snd_driver.input], buffer
    mov     edi, 10
    cmpe    [mute], 1, .set_sound
    sub     edi, [volume]
  .set_sound:
    imul    edi, 479
    neg     edi
    mov     dword[buffer], edi
    mcall   68, 17, snd_driver
    ret

 ;----------------------------

 _redraw:
    call    draw_window
    call    draw_icon
    call    draw_bar

    jmp     _update

 ;----------------------------

 draw_window:
    mcall   0, <[win.x], WIN_SIZE + 1>, <[win.y], WIN_SIZE + 1>, 0x61000000

    mcall   13, <0, WIN_SIZE>, <1, WIN_SIZE - 2>, 0x3D3D3D
    mcall     , <1, WIN_SIZE - 2>, <0, 1>
    mcall     , 		 , <WIN_SIZE - 1, 1>

    mcall   8, <0, WIN_SIZE>, <0, WIN_SIZE>, 0x60000002
    mcall    , <20, 80>, <22, 60>, 0x60000003

    ret

 ;----------------------------

 draw_icon:
    mcall   7, image, <30, 45>, <31, 30>

    cmpe    dword[mute], 0, @f
    mov     ebx, 30 * 45 * 3 * 5 + image
    jmp     .draw
  @@:
    cmpne   dword[volume], 0, @f
    mov     ebx, 30 * 45 * 3 * 1 + image
    jmp     .draw
  @@:
    cmpge  dword[volume], 5, @f
    mov     ebx, 30 * 45 * 3 * 2 + image
    jmp     .draw
  @@:
    cmpge  dword[volume], 9, @f
    mov     ebx, 30 * 45 * 3 * 3 + image
    jmp     .draw
  @@:
    mov     ebx, 30 * 45 * 3 * 4 + image
 .draw:
    mcall    ,	    ,	      , <61, 30>

    ret

 ;----------------------------

 draw_bar:
 ;; draw shadow
    mov     eax, 13
    mov     ebx, 11 shl 16 + 9
    mov     ecx, 100 shl 16 + 1
    mov     edx, 0x252525
    mov     esi, 10
  @@:
    cmpe    esi, 0, @f
    mcall
    add     ebx, 10 shl 16
    dec     esi
    jmp     @b
  @@:

 ;; draw active
    mov     ebx, 11 shl 16 + 9
    mov     ecx, 96 shl 16 + 4
    mov     edx, 0xE5E5E5
    mov     esi, [volume]
  @@:
    cmpe    esi, 0, @f
    mcall
    add     ebx, 10 shl 16
    dec     esi
    jmp     @b
  @@:

 ;; draw inactive
    mov     ebx, 11 shl 16 + 9
    mov     edx, 0x737373
    mov     esi, 10
    sub     esi, [volume]
    imul    edi, [volume], 10
    shl     edi, 16
    add     ebx, edi
  @@:
    cmpe    esi, 0, @f
    mcall
    add     ebx, 10 shl 16
    dec     esi
    jmp     @b
  @@:

    ret

 ;----------------------------

 @imports:
    library img, "libimg.obj"
    import  img, \
	img.init,    "lib_init", \
	img.to_rgb,  "img_to_rgb2", \
	img.decode,  "img_decode", \
	img.destroy, "img_destroy"

 ;----------------------------

 volume_dat db "/sys/settings/volume.dat", 0

 is_save:
	    dd 2
	    dd 0
	    dd 0
	    dd 8
	    dd volume
	    db 0
	    dd volume_dat

 is_load:
	    dd 0
	    dd 0
	    dd 0
	    dd 8
	    dd volume
	    db 0
	    dd volume_dat

 icons	    file "icon.png"
  .size     = $-icons
 snd_driver.name:
	    db "SOUND", 0
 shm_name   db "volume-man", 0

 @end:

;=====================================================================

 win:
  .x	    rd 1
  .y	    rd 1
  .pid	    rd 1

 image	    rb 45 * 30 * 6 * 3
  .data     rd 1

 shm	    rd 1
 volume     rd 1
 timer	    rd 1
 mute	    rd 1
 command    rd 1
 buffer     rb 1024
 snd_driver rd 1
  .command  rd 1
  .input    rd 1
  .inputsz  rd 1
  .output   rd 1
  .outputsz rd 1

;=====================================================================
	    rb 2048
 _stack:
	    rb 2048
 @stack:
 @params    rb 256

 @memory:
