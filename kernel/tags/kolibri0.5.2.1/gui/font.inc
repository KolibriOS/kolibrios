align 4
dtext:
        ; eax x & y
        ; ebx font ( 0xX0000000 ) & color ( 0x00RRGGBB )
        ; ecx start of text
        ; edx length
        ; edi 1 force

        test   ebx,0x10000000
        jnz    dtext2

        pushad

        mov    esi, edx
        and    esi, 0xff
        test   esi, esi    ; zero length ?
        jnz    @f
        popad
        ret
      @@:
        align 4
      .letnew:

        push   eax ecx edx
        movzx  ebx,ax
        shr    eax,16
        movzx  edx,byte [ecx]
        mov    ecx,[esp+3*4+32-16]
        call   drawletter
        pop    edx ecx eax

        add    eax,6*65536

        inc    ecx
        dec    edx
        jnz    .letnew

        popad
        ret

align 4
drawletter:
;eax - x
;ebx - y
;ecx - color
;edx - ascii code
    pushad
    call  [disable_mouse]
    mov   esi,9
    lea   ebp,[0x3F600+8*edx+edx]
.symloop:
    push  esi
    mov   dl,byte [ebp]
    mov   esi,8
.pixloop:
    test  dl,1
    jz    .nopix
    call  [putpixel]
.nopix:
    shr   dl,1    
    inc   eax
    dec   esi
    jnz   .pixloop
    sub   eax,8
    inc   ebx
    inc   ebp
    pop   esi
    dec   esi
    jnz   .symloop
    popad
    ret  
        


dtext2:

        ; eax x & y
        ; ebx color
        ; ecx start of text
        ; edx length
        ; edi 1 force

        pushad

        mov    esi,edx
        and    esi,0xff
        test   esi,esi    ; zero length ?
        jnz    @f
        popad
        ret
      @@:

      align 4
      letnew2:

        push   ecx
        push   edx
        movzx  ebx,ax
        shr    eax,16
        movzx  edx,byte [ecx]
        mov    ecx,[esp+2*4+32-16]
        call   drawletter2
        shl    eax,16
        add    eax,ebx
        pop    edx
        pop    ecx

        inc    ecx
        dec    edx
        jnz    letnew2

        popad
        ret

align 4
drawletter2:
;eax - x
;ebx - y
;ecx - color
;edx - symbol
;edi - force?
;result - eax=eax+sym_size
    pushad
    call  [disable_mouse]
    shl   edx,1
    mov   esi,9
    lea   ebp,[0x3EC00+4*edx+edx+1]
.symloop:
    push  esi
    mov   dl,byte [ebp]
    xor   esi,esi
.pixloop:
    test  dl,1
    jz    .nopix
    call  [putpixel]
.nopix:
    shr   dl,1
    inc   esi
    inc   eax
    cmp   esi,8
    jl    .pixloop
    sub   eax,8
    inc   ebx
    pop   esi
    inc   ebp
    dec   esi
    jnz   .symloop
    movzx edx,byte [ebp-10]
    add   [esp+32-4],edx
    popad
    ret