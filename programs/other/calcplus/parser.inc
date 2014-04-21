 macro test_err
 {  local   ..not_error
    cmp     [error_n], 0
    je	    ..not_error
    ret
 ..not_error:
 }

 macro set_err err
 {
    mov     [error_n], err
    ret
 }

 proc convert_to_str uses ebx ecx edx esi edi, _num, _str
    mov     eax, [_num]
    mov     esi, [_str]
    mov     edi, 0
    mov     ecx, eax
    and     ecx, 1 shl 31
    cmp     ecx, 0
    je	    @f
    mov     [esi], byte "-"
    inc     esi
    inc     edi
    not     eax
    inc     eax
  @@:
    mov     ebx, 10
    xor     ecx, ecx
  @@:
    xor     edx, edx
    div     ebx
    push    edx
    inc     ecx
    inc     edi
    cmp     eax, 0
    jne     @b
  @@:
    pop     eax
    add     al, "0"
    mov     [esi], al
    inc     esi
    loop    @b
    mov     [esi], byte 0
    mov     eax, edi
    ret
 endp

 ; ---------------------------

 proc parse
    mov     [exp_pos], 0
    mov     [error_n], 0
    stdcall parse_lvl0
    ret
 endp

 ; ---------------------------

 proc parse_lvl0 uses ebx ecx
    test_err
    stdcall parse_lvl1
    test_err
  @@:
    mov     ebx, exp
    add     ebx, [exp_pos]
    cmp     [ebx], byte 0
    je	    .end
    cmp     [ebx], byte ")"
    je	    .end
    inc     [exp_pos]
    cmp     [ebx], byte "+"
    jne     .not_add
    mov     ecx, eax
    stdcall parse_lvl1
    test_err
    add     eax, ecx
    jmp     @b
 .not_add:
    cmp     [ebx], byte "-"
    jne     @b
    mov     ecx, eax
    stdcall parse_lvl1
    test_err
    sub     ecx, eax
    mov     eax, ecx
    jmp     @b
 .end:
    ret
 endp

 ; ---------------------------

 proc parse_lvl1 uses ebx ecx edx
    test_err
    stdcall parse_lvl2
    test_err
  @@:
    mov     ebx, exp
    add     ebx, [exp_pos]
    cmp     [ebx], byte 0
    je	    .end
    inc     [exp_pos]
    cmp     [ebx], byte "*"
    jne     .not_mul
    mov     ecx, eax
    stdcall parse_lvl2
    test_err
    imul    ecx, eax
    mov     eax, ecx
    jmp     @b
 .not_mul:
    cmp     [ebx], byte "/"
    je	    .div_or_mod
    cmp     [ebx], byte "%"
    je	    .div_or_mod
    jmp     .end
 .div_or_mod:
    mov     ecx, eax
    stdcall parse_lvl2
    test_err
    cmp     eax, 0
    jne     .not_null
    set_err 1
 .not_null:
    xchg    ecx, eax
    cdq
    div     ecx
    cmp     [ebx], byte "%"
    je	    .mod
    jmp     @b
 .mod:
    mov     eax, edx
    jmp     @b
 .end:
    dec     [exp_pos]
    ret
 endp

 ; ---------------------------

 proc parse_lvl2 uses ebx
    test_err
    stdcall skip_spaces
    mov     ebx, exp
    add     ebx, [exp_pos]
    cmp     [ebx], byte 48
    jl	    @f
    cmp     [ebx], byte 57
    jg	    @f
    stdcall parse_lvl3
    jmp     .end
  @@:
    inc     [exp_pos]
    cmp     [ebx], byte "("
    jne     @f
    stdcall parse_lvl0
    test_err
    inc     [exp_pos]
    jmp     .end
  @@:
    cmp     [ebx], byte "+"
    jne     @f
    stdcall parse_lvl2
    test_err
    jmp     .end
  @@:
    cmp     [ebx], byte "-"
    jne     .end
    stdcall parse_lvl2
    test_err
    neg     eax
 .end:
    stdcall skip_spaces
    ret
 endp

 ; ---------------------------

 proc parse_lvl3 uses ebx ecx
    sub     eax, eax
    sub     ecx, ecx
    mov     ebx, exp
    add     ebx, [exp_pos]
  @@:
    cmp     [ebx], byte 0
    je	    @f
    cmp     [ebx], byte 48
    jl	    @f
    cmp     [ebx], byte 57
    jg	    @f
    imul    eax, 10
    mov     cl, [ebx]
    add     eax, ecx
    sub     eax, 48
    inc     ebx
    inc     [exp_pos]
    jmp     @b
  @@:
    ret
 endp

 ; ---------------------------

 proc skip_spaces uses ebx
    mov     ebx, exp
    add     ebx, [exp_pos]
  @@:
    cmp     [ebx], byte " "
    jne     @f
    inc     ebx
    inc     [exp_pos]
    jmp     @b
  @@:
    ret
 endp