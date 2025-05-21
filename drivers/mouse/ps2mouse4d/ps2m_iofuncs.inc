kbd_read:
    push    ecx edx

    mov     ecx,0xfffff ; last 0xffff, new value in view of fast CPU's
  kr_loop:
    in      al,0x64
    test    al,1
    jnz     kr_ready
    loop    kr_loop
    mov     ah,1
    jmp     kr_exit
  kr_ready:
    in      al,0x60
    xor     ah,ah
  kr_exit:
    pop     edx ecx
    ret


kbd_write:

    push    ecx edx

    mov     dl,al
    mov     ecx,0x1ffff
@@:
    in      al,0x64
    test    al,2
    jz      @f
    loop    @b
    mov     ah,1
    jmp     .nothing
@@:
    mov     al,dl
    out     0x60,al
    mov     ah,0
.nothing:
    pop     edx ecx
    ret


kbd_cmd:

    mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
  c_wait:
    in      al,0x64
    test    al,2
    jz      c_send
    loop    c_wait
    mov     ah,1
    jmp     c_exit
  c_send:
    mov     al,bl
    out     0x64,al
    xor     ah,ah
  c_exit:
    ret

mouse_cmd:
    mov  [mouse_cmd_byte], al
    mov  [mouse_nr_resends], 5
 .resend:
    mov  bl, 0xd4
    call kbd_cmd
    cmp  ah,1
    je   .fail

    mov  al, [mouse_cmd_byte]
    call kbd_write
    cmp  ah, 1
    je   .fail

    cmp  [mouse_cmd_byte], 0xF5
    je .ok
    call mouse_read

    cmp  al, 0xFA
    jne  .noack
.ok:
    clc
    ret
  .noack:
    cmp  al, 0xFE	; resend
    jne  .noresend
    dec  [mouse_nr_resends]
    jnz  .resend
  .noresend:
  .fail:
    stc
    ret


mouse_read:
    mov  [mouse_nr_tries], 100
  .repeat:
    call kbd_read
	  cmp  ah, 1
	  jne  .fin
	  mov  esi, 10
    invoke Sleep
    dec  [mouse_nr_tries]
    jnz  .repeat

    stc
    ret

  .fin:
    clc
    ret
