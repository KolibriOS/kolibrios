align 4
dtext:  ; Text String Output (rw by Johnny_B[john@kolibrios.org])
        ; eax x & y
        ; ebx font ( 0xX0000000 ) & color ( 0x00RRGGBB )
        ; ecx start of text
        ; edx length
        ; edi 1 force

        pushad

        mov    esi,edx    ;esi=length
        mov    ebp,ecx    ;ebp=ptr to text
        mov    ecx,ebx    ;ecx=color
        movzx  ebx,ax     ;ebx=y
        shr    eax,16     ;eax=x
        and    esi, 0xFF  ;limit of text = 255 symbols

dtext.lnew:
        test   esi, esi    ; zero length ?
        jnz    @f
        jmp    dtext.output_end
      @@:

        movzx  edx,byte [ebp] ;edx=ascii code
        test   edx,edx
        jz     dtext.output_end
        test   ecx,0x10000000
        jnz    dtext.letnew2

        align 4
      .letnew:

drawletter:  ;output char of type 1(monotype)
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

        add    eax,6

        inc    ebp  ;ptr to text
        dec    esi  ;length
        jnz    dtext.lnew

        jmp dtext.output_end


      dtext.letnew2:

align 4
drawletter2:       ;output char of type 2(proportional)
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


        inc    ebp   ;ptr to text
        dec    esi   ;length
        jnz    dtext.lnew


dtext.output_end:
        popad
        ret