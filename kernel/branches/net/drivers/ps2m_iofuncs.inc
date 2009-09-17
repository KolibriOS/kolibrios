kbd_read:
    push    ecx edx

    mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
  kr_loop:
    in      al,0x64
    test    al,1
    jnz     kr_ready
    loop    kr_loop
    mov     ah,1
    jmp     kr_exit
  kr_ready:
    push    ecx
    mov     ecx,32
  kr_delay:
    loop    kr_delay
    pop     ecx
    in      al,0x60
    xor     ah,ah
  kr_exit:
    pop     edx ecx
    ret


kbd_write:

    push    ecx edx

    mov     dl,al
    in      al,0x60
    mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
  kw_loop:
    in      al,0x64
    test    al,2
    jz      kw_ok
    loop    kw_loop
    mov     ah,1
    jmp     kw_exit
  kw_ok:
    mov     al,dl
    out     0x60,al
    mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
  kw_loop3:
    in      al,0x64
    test    al,2
    jz      kw_ok3
    loop    kw_loop3
    mov     ah,1
    jmp     kw_exit
  kw_ok3:
    mov     ah,8
  kw_loop4:
    mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
  kw_loop5:
    in      al,0x64
    test    al,1
    jnz     kw_ok4
    loop    kw_loop5
    dec     ah
    jnz     kw_loop4
  kw_ok4:
    xor     ah,ah
  kw_exit:
    pop     edx ecx
    ret


kbd_cmd:

    mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
  c_wait:
    in      al,0x64
    test    al,2
    jz      c_send
    loop    c_wait
    jmp     c_error
  c_send:
    mov     al,bl
    out     0x64,al
    mov     ecx,0x1ffff ; last 0xffff, new value in view of fast CPU's
  c_accept:
    in      al,0x64
    test    al,2
    jz      c_ok
    loop    c_accept
  c_error:
    mov     ah,1
    jmp     c_exit
  c_ok:
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
 
    call mouse_read
 
    cmp  al, 0xFA
    jne  .noack
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
    call Sleep
    dec  [mouse_nr_tries]
    jnz  .repeat

    stc
    ret

  .fin:
    clc
    ret
