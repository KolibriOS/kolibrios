 proc string.length uses ebx, _str
    mov     eax, 0
    mov     ebx, [_str]
  @@:
    cmp     [ebx], byte 0
    je	    @f
    inc     eax
    inc     ebx
    jmp     @b
  @@:
    ret
 endp

 proc string.copy uses eax ebx ecx, _src, _dst
    mov     eax, [_src]
    mov     ebx, [_dst]
  @@:
    mov     cl, [eax]
    mov     [ebx], cl
    cmp     cl, 0
    je	    @f
    inc     eax
    inc     ebx
    jmp     @b
  @@:
    ret
 endp

 proc string.concatenate uses eax, _src, _dst
    stdcall string.length, [_dst]
    add     eax, [_dst]
    stdcall string.copy, [_src], eax
    ret
 endp

 proc string.cmp uses ecx esi edi, _str1, _str2, _n
    mov     ecx, [_n]
    test    ecx, ecx	     ; Max length is zero?
    je	    .done

    mov     esi, [_str1]	; esi = string s1
    mov     edi, [_str2]	; edi = string s2
    cld
 .compare:
    cmpsb		     ; Compare two bytes
    jne     .done
    cmp     byte [esi-1], 0  ; End of string?
    je	    .done
    dec     ecx 	     ; Length limit reached?
    jne     .compare
 .done:
    seta    al		     ; al = (s1 > s2)
    setb    ah		     ; ah = (s1 < s2)
    sub     al, ah
    movsx   eax, al	     ; eax = (s1 > s2) - (s1 < s2), i.e. -1, 0, 1
    ret
 endp

 proc string.to_lower_case uses eax, _str
    mov     eax, [_str]
  @@:
    cmp     [eax], byte 0
    je	    @f
    cmp     [eax], byte 65
    jl	    .next
    cmp     [eax], byte 90
    jg	    .next
    add     [eax], byte 97 - 65
 .next:
    inc     eax
    jmp     @b
  @@:
    ret
 endp

 proc string.to_upper_case uses eax, _str
    mov     eax, [_str]
  @@:
    cmp     [eax], byte 0
    je	    @f
    cmp     [eax], byte 97
    jl	    .next
    cmp     [eax], byte 122
    jg	    .next
    sub     [eax], byte 97 - 65
 .next:
    inc     eax
    jmp     @b
  @@:
    ret
 endp

 proc string.match uses ebx ecx edx, _str1, _str2
    mov     ebx, [_str1]
    mov     ecx, [_str2]
  @@:
    cmp     [ebx], byte 0
    je	    @f
    cmp     [ecx], byte 0
    je	    @f

    mov     dl, [ebx]
    cmp     [ecx], byte '?'
    je	    .next
    cmp     [ecx], byte '*'
    je	    .next_ebx
    cmp     [ecx], dl
    je	    .next

    cmp     [ecx - 1], byte '*'
    je	    .next_ecx

    jmp     @f

 .next_ecx:
    dec     ecx
    jmp     .next
 .next_ebx:
    dec     ebx
 .next:
    inc     ebx
    inc     ecx
    jmp     @b
  @@:

  @@:
    cmp     [ecx], byte 0
    je	    @f
    cmp     [ecx], byte '*'
    jne     @f
    inc     ecx
    jmp     @b
  @@:

    cmp     [ecx], byte 0
    je	    @f
    mov     eax, 0
    ret
  @@:
    mov     eax, 1
    ret
 endp

 proc string.trim_last uses eax, _str
    stdcall string.length, [_str]
    add     eax, [_str]
    dec     eax
  @@:
    cmp     [eax], byte ' '
    jne     @f
    mov     [eax], byte 0
    dec     eax
    jmp     @b
  @@:
    ret
 endp

 proc string.trim_first, _str
    mov     eax, [_str]
  @@:
    cmp     [eax], byte ' '
    jne     @f
    inc     eax
    jmp     @b
  @@:
    ret
 endp

 proc string.index_of uses ebx ecx, _str, _char, _num
    mov     ebx, [_char]
    mov     ecx, [_str]
    mov     eax, 0
  @@:
    cmp     [ecx], byte 0
    je	    @f
    cmp     [ecx], bl
    jne     .after_check
    dec     [_num]
    jz	    .finded
 .after_check:
    inc     ecx
    inc     eax
    jmp     @b
  @@:
    mov     eax, -1
 .finded:
    ret
 endp

 proc string.last_index_of uses ebx ecx, _str, _char, _num
    stdcall string.length, [_str]
    mov     ecx, [_str]
    add     ecx, eax
    mov     ebx, [_char]
  @@:
    cmp     eax, 0
    je	    @f
    cmp     [ecx], bl
    jne     .after_check
    dec     [_num]
    jz	    .finded
 .after_check:
    dec     ecx
    dec     eax
    jmp     @b
  @@:
    mov     eax, -2
 .finded:
    inc     eax
    ret
 endp