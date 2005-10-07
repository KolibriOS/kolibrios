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

; eax  x
; ebx  y
; ecx  color
; edx  letter
; esi  shl size
; edi  force

        pushad
        call  [disable_mouse]
        xor   eax, eax
        xor   ebx, ebx  ; 0x37000+eax+ebx*8
        inc   esi
        align 4
      chc:

        push  eax
        push  ebx

        mov   edx,ebx
        shl   edx,3
        add   edx,eax
        add   edx,0x37000+8
        mov   ecx,[esp+32-12+8]
        imul  ecx,8*10
        add   edx,ecx
        cmp   [edx],byte 'o'
        jnz   .nopix
        mov   eax,[esp+4]
        mov   ebx,[esp+0]
        add   eax,[esp+32+2*4-4]
        add   ebx,[esp+32+2*4-16]
        mov   ecx,[esp+32+2*4-8]
        ;;;call  [disable_mouse]
        call  [putpixel]
      .nopix:
        pop   ebx
        pop   eax

        inc   eax
        cmp   eax,5 ; ebp
        jne   chc

        xor   eax, eax

        inc   ebx
        cmp   ebx,9 ; ebp
        jne   chc

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

        push   eax
        push   ecx
        push   edx
        movzx  ebx,ax
        shr    eax,16
        movzx  edx,byte [ecx]
        mov    ecx,[esp+3*4+32-16]
        call   drawletter2
        pop    edx
        pop    ecx
        pop    eax

        push   edx
        movzx  edx,byte [ecx]
        imul   edx,10*10
        add    edx,0x30000
        cmp    [edx+6],byte ' '
        jne    nocharadd8
        add    eax,8*65536
        jmp    charaddok
      nocharadd8:
        movzx  edx,byte [edx+6]
        sub    edx,47
        shl    edx,16
        add    eax,edx
      charaddok:
        pop    edx

        inc    ecx
        dec    edx
        jnz    letnew2

        popad
        ret

align 4
drawletter2:

; eax  x
; ebx  y
; ecx  color
; edx  letter
; esi  shl size
; edi  force

        pushad
        call  [disable_mouse]
        xor   eax, eax
        xor   ebx, ebx  ; +eax+ebx*8
        inc   esi

        align 4
      chc2:

        push  eax
        push  ebx

;        cmp   esi,1
;        je    noldiv
;        xor   edx,edx
;        div   esi
;        push  eax
;        xor   edx,edx
;        mov   eax,ebx
;        div   esi
;        mov   ebx,eax
;        pop   eax
;      noldiv:

        mov   edx,ebx
        ;shl   edx,3
        imul  edx,10
        add   edx,eax
        add   edx,0x30000+8+2
        mov   ecx,[esp+32-12+8]
        ;shl   ecx,6
        imul  ecx,10*10
        add   edx,ecx
        cmp   [edx],byte 'o'
        jnz   nopix2
        mov   eax,[esp+4]
        mov   ebx,[esp+0]
        add   eax,[esp+32+2*4-4]
        add   ebx,[esp+32+2*4-16]
        mov   ecx,[esp+32+2*4-8]
        ;;;call  [disable_mouse]
        call  [putpixel]
      nopix2:
        pop   ebx
        pop   eax

        ;mov   ebp,7
        ;imul  ebp,esi

        inc   eax
        cmp   eax,7 ;ebp
        jnz   chc2

        xor   eax, eax

        ;mov   ebp,9
        ;imul  ebp,esi

        inc   ebx
        cmp   ebx,9 ; ebp
        jnz   chc2

        popad
        ret
