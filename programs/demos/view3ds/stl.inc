; STL file support (ASCII and binary) for view3ds.
;
; STL format handling (ascii/binary detection, chunk layout) is based on
; convert_stl_3ds.inc by IgorA <IgorA@kolibrios.org>, originally written
; for programs/develop/info3ds. Rewritten here without proc32/mem.Alloc
; dependencies to fit view3ds's raw-asm style, loading geometry directly
; into points_ptr / triangles_ptr instead of building an intermediate
; .3ds chunk buffer.
;
; STL has no shared vertex list (every triangle owns 3 unique
; vertices), so triangle i simply uses vertex indices 3*i,3*i+1,3*i+2.

STL_MAX_TRI equ 0FFFFFh   ; sanity cap on triangle count

;-------------------------------------------------------------------------
; in:  [fptr],[fsize]
; out: eax = 1 if the file looks like STL (ascii or binary), else 0
;
; NOTE: binary STL has no reliable magic - its free-form 80 byte header
; is often filled by exporters with a text comment that itself starts
; with "solid " (a well-known STL gotcha). The binary size check
; (face_count*50+84 == fsize) is exact/unambiguous, so it MUST be
; tried first; the "solid " prefix is only a fallback for genuine
; ascii files, where this arithmetic essentially never coincides.
;-------------------------------------------------------------------------
is_stl_file:
        mov     esi,[fptr]
        mov     ecx,[fsize]
        cmp     ecx,84
        jb      .chk_ascii
        mov     eax,[esi+80]
        imul    eax,50
        add     eax,84
        cmp     eax,ecx
        jne     .chk_ascii
        mov     eax,1
        ret
    .chk_ascii:
        cmp     ecx,15
        jb      .no
        cmp     dword[esi],'soli'
        jne     .no
        cmp     word[esi+4],'d '
        jne     .no
        mov     eax,1
        ret
    .no:
        xor     eax,eax
ret

;-------------------------------------------------------------------------
; in:  [fptr],[fsize] (already checked with is_stl_file)
; out: points_ptr / triangles_ptr filled,
;      points_count_var / triangles_count_var set
; dispatch order must match is_stl_file (binary check first, see note above)
;-------------------------------------------------------------------------
read_stl:
        mov     esi,[fptr]
        mov     ecx,[fsize]
        cmp     ecx,84
        jb      .ascii
        mov     eax,[esi+80]
        imul    eax,50
        add     eax,84
        cmp     eax,ecx
        jne     .ascii
        call    read_stl_binary
        ret
    .ascii:
        call    read_stl_ascii
ret

;-------------------------------------------------------------------------
read_stl_binary:
        cld
        mov     esi,[fptr]
        mov     eax,[esi+80]           ; face (triangle) count
        cmp     eax,STL_MAX_TRI
        jbe     @f
        mov     eax,STL_MAX_TRI
    @@:
        mov     [triangles_count_var],eax
        lea     ebx,[eax+eax*2]        ; *3
        mov     [points_count_var],ebx

        xor     bl,bl
        call    alloc_mem_for_tp

        mov     esi,[fptr]
        add     esi,84
        mov     edi,[points_ptr]
        mov     ebx,[triangles_ptr]
        xor     edx,edx                ; running vertex index
        mov     ecx,[triangles_count_var]
        or      ecx,ecx
        jz      .done
    .loop:
        add     esi,12                 ; skip normal vector
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        movsd
        add     esi,2                  ; skip attribute byte count

        mov     eax,edx
        mov     [ebx],eax
        inc     eax
        mov     [ebx+4],eax
        inc     eax
        mov     [ebx+8],eax
        add     ebx,12
        add     edx,3

        dec     ecx
        jnz     .loop
    .done:
        mov     dword[edi],-1
        mov     dword[ebx],-1
ret

;-------------------------------------------------------------------------
read_stl_ascii:
        cld
        ; pass 1: count 'vertex' occurrences
        mov     esi,[fptr]
        mov     ecx,[fsize]
        xor     ebx,ebx
    .cnt_loop:
        cmp     ecx,6
        jb      .cnt_done
        cmp     byte[esi],'v'
        jne     .cnt_next
        cmp     dword[esi+1],'erte'
        jne     .cnt_next
        cmp     byte[esi+5],'x'
        jne     .cnt_next
        inc     ebx
        add     esi,6
        sub     ecx,6
        jmp     .cnt_loop
    .cnt_next:
        inc     esi
        dec     ecx
        jmp     .cnt_loop
    .cnt_done:

        mov     eax,ebx
        cmp     eax,STL_MAX_TRI*3
        jbe     @f
        mov     eax,STL_MAX_TRI*3
    @@:
        xor     edx,edx
        mov     ecx,3
        div     ecx                    ; eax = triangle count (truncated)
        mov     [triangles_count_var],eax
        lea     eax,[eax+eax*2]
        mov     [points_count_var],eax

        xor     bl,bl
        call    alloc_mem_for_tp

        ; pass 2: parse floats and fill arrays
        mov     esi,[fptr]
        mov     edx,esi
        add     edx,[fsize]            ; edx = end of buffer
        mov     edi,[points_ptr]
        mov     ebx,[triangles_ptr]
        xor     ecx,ecx                ; ecx = vertex index counter
    .fill_loop:
        cmp     ecx,[points_count_var]
        jge     .fill_done
        cmp     esi,edx
        jae     .fill_done
        cmp     byte[esi],'v'
        jne     .fill_next
        cmp     dword[esi+1],'erte'
        jne     .fill_next
        cmp     byte[esi+5],'x'
        jne     .fill_next
        add     esi,6

        call    stl_read_float
        fstp    dword[edi]
        call    stl_read_float
        fstp    dword[edi+4]
        call    stl_read_float
        fstp    dword[edi+8]
        add     edi,12

        mov     [ebx],ecx
        add     ebx,4
        inc     ecx
        jmp     .fill_loop
    .fill_next:
        inc     esi
        jmp     .fill_loop
    .fill_done:
        mov     dword[edi],-1
        mov     dword[ebx],-1
ret

;-------------------------------------------------------------------------
; small self-contained ascii->float parser (handles sign, fraction,
; and 'e'/'E' exponent, since STL coordinates are often in
; scientific notation)
; in:  esi -> text pointer (leading whitespace allowed)
; out: esi advanced past the parsed number, value pushed on FPU stack
;      eax,ebx,ecx,edx preserved
;-------------------------------------------------------------------------
align 4
stl_read_float:
        push    eax ebx ecx edx
    .skip_ws:
        mov     al,[esi]
        cmp     al,' '
        je      .adv_ws
        cmp     al,9
        je      .adv_ws
        cmp     al,13
        je      .adv_ws
        cmp     al,10
        je      .adv_ws
        jmp     .chk_sign
    .adv_ws:
        inc     esi
        jmp     .skip_ws
    .chk_sign:
        xor     edx,edx
        cmp     byte[esi],'-'
        jne     .chk_plus
        mov     edx,1
        inc     esi
        jmp     .int_part
    .chk_plus:
        cmp     byte[esi],'+'
        jne     .int_part
        inc     esi
    .int_part:
        fldz
    .int_loop:
        movzx   eax,byte[esi]
        cmp     al,'0'
        jb      .frac_chk
        cmp     al,'9'
        ja      .frac_chk
        sub     al,'0'
        fimul   dword[stl_ten]
        push    eax
        fiadd   dword[esp]
        pop     eax
        inc     esi
        jmp     .int_loop
    .frac_chk:
        xor     ecx,ecx
        cmp     byte[esi],'.'
        jne     .exp_chk
        inc     esi
    .frac_loop:
        movzx   eax,byte[esi]
        cmp     al,'0'
        jb      .frac_apply
        cmp     al,'9'
        ja      .frac_apply
        sub     al,'0'
        fimul   dword[stl_ten]
        push    eax
        fiadd   dword[esp]
        pop     eax
        inc     ecx
        inc     esi
        jmp     .frac_loop
    .frac_apply:
        jecxz   .exp_chk
    .frac_div:
        fidiv   dword[stl_ten]
        dec     ecx
        jnz     .frac_div
    .exp_chk:
        mov     al,[esi]
        cmp     al,'e'
        je      .exp_start
        cmp     al,'E'
        jne     .apply_sign
    .exp_start:
        inc     esi
        xor     ebx,ebx                ; exponent sign: 0=+, 1=-
        cmp     byte[esi],'-'
        jne     .exp_plus
        mov     ebx,1
        inc     esi
        jmp     .exp_digits
    .exp_plus:
        cmp     byte[esi],'+'
        jne     .exp_digits
        inc     esi
    .exp_digits:
        xor     ecx,ecx
    .exp_loop:
        movzx   eax,byte[esi]
        cmp     al,'0'
        jb      .exp_apply
        cmp     al,'9'
        ja      .exp_apply
        sub     al,'0'
        imul    ecx,10
        add     ecx,eax
        inc     esi
        jmp     .exp_loop
    .exp_apply:
        jecxz   .apply_sign
    .exp_apply_loop:
        cmp     ebx,1
        je      .exp_neg
        fimul   dword[stl_ten]
        jmp     .exp_next
    .exp_neg:
        fidiv   dword[stl_ten]
    .exp_next:
        dec     ecx
        jnz     .exp_apply_loop
    .apply_sign:
        cmp     edx,1
        jne     .done
        fchs
    .done:
        pop     edx ecx ebx eax
ret

stl_ten dd 10
