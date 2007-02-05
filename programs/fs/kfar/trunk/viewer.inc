virtual at 0
viewer_data:
        .buf_start      dq      ?
        .cur_pos        dq      ?
        .filesize       dq      ?
        .buf_pos        dd      ?
        .buf_size       dd      ?
        .col            dq      ?
        .encoding       db      ?
        .bEofReached    db      ?
        .flags          db      ?       ; & 1: hex mode
                                        ; & 2: unwrap
                        rb      1
        .filename       rb      1024
        .buf            rb      16384
        .size = $
end virtual

view_file:
        mov     eax, [ebp + panel1_files - panel1_data]
        mov     ecx, [eax+ecx*4]
        test    byte [ecx], 10h
        jz      .file
        ret
.file:
        lea     esi, [ebp + panel1_dir - panel1_data]
        mov     eax, viewer_data.size
        mov     edx, viewer_vtable
        call    new_screen
        test    eax, eax
        jnz     @f
        ret
@@:
        mov     [ebp+viewer_data.encoding], encodings.cp866
        mov     [ebp+viewer_data.flags], 0
        and     dword [ebp+viewer_data.col], 0
        and     dword [ebp+viewer_data.col+4], 0
        lea     edi, [ebp+viewer_data.filename]
        push    edi
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
        lea     esi, [ecx+40]
        mov     al, '/'
        stosb
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     eax
        push    eax
        mov     ebx, attrinfo
        mov     [ebx+21], eax
.attr_retry:
        push    70
        pop     eax
        int     40h
        test    eax, eax
        jz      @f
        lea     ebx, [ebp+viewer_data.filename]
        push    ebx
        push    aCannotReadFile
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    RetryOrCancelBtn
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        mov     ebx, attrinfo
        test    eax, eax
        jz      .attr_retry
        jmp     delete_active_screen
@@:
        mov     eax, dword [attrinfo.attr+32]
        mov     dword [ebp+viewer_data.filesize], eax
        mov     eax, dword [attrinfo.attr+36]
        mov     dword [ebp+viewer_data.filesize+4], eax
        mov     ebx, readinfo
        xor     eax, eax
        mov     dword [ebx+readinfo.first-readinfo], eax
        mov     dword [ebx+readinfo.first+4-readinfo], eax
        mov     dword [ebp+viewer_data.buf_start], eax
        mov     dword [ebp+viewer_data.buf_start+4], eax
        mov     dword [ebp+viewer_data.cur_pos], eax
        mov     dword [ebp+viewer_data.cur_pos+4], eax
        mov     [ebx+readinfo.size-readinfo], 16384
        lea     eax, [ebp+viewer_data.buf]
        mov     [readinfo.data], eax
        mov     [ebp+viewer_data.buf_pos], eax
        pop     dword [readinfo.name]
.retry:
        push    70
        pop     eax
        int     40h
        mov     [ebp+viewer_data.buf_size], ebx
        test    eax, eax
        jz      .readok
        cmp     eax, 6
        jz      .readok
        lea     ebx, [ebp+viewer_data.filename]
        push    ebx
        push    aCannotReadFile
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    RetryOrCancelBtn
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        mov     ebx, readinfo
        test    eax, eax
        jz      .attr_retry
        jmp     delete_active_screen
.readok:
        call    viewer_set_keybar
        call    viewer_draw_text
        ret

viewer_get_next_char:
        pusha
        mov     eax, [ebp+viewer_data.buf_pos]
        lea     ecx, [ebp+viewer_data.buf]
        add     ecx, [ebp+viewer_data.buf_size]
        cmp     eax, ecx
        jb      .buffered
        mov     al, ' '
        sub     ecx, ebp
        cmp     ecx, viewer_data.buf + 16384
        jb      .err
        mov     eax, dword [ebp+viewer_data.buf_start]
        add     eax, 8192
        mov     dword [ebp+viewer_data.buf_start], eax
        mov     edx, dword [ebp+viewer_data.buf_start+4]
        adc     edx, 0
        mov     dword [ebp+viewer_data.buf_start+4], edx
        add     eax, 16384-8192
        adc     edx, 0
        mov     dword [readinfo.first], eax
        mov     dword [readinfo.first+4], edx
        mov     dword [readinfo.size], 8192
        lea     edi, [ebp+viewer_data.buf]
        lea     esi, [edi+8192]
        mov     ecx, (16384-8192)/4
        rep     movsd
        mov     [readinfo.data], edi
        lea     eax, [ebp+viewer_data.filename]
        mov     [readinfo.name], eax
.readretry:
        mov     ebx, readinfo
        push    70
        pop     eax
        int     40h
        test    eax, eax
        jz      .readok
        cmp     eax, 6
        jz      .readok
        call    ask_retry_ignore
        jz      .readretry
.readok:
        sub     [ebp+viewer_data.buf_pos], 8192
        add     ebx, 16384-8192
        mov     [ebp+viewer_data.buf_size], ebx
        mov     eax, [ebp+viewer_data.buf_pos]
        cmp     ecx, 16384-8192
        jnz     .buffered
.err:
        stc
        popa
        ret
.buffered:
        mov     al, [eax]
.done:
        mov     [esp+28], al
        inc     [ebp+viewer_data.buf_pos]
        clc
        popa
        ret

viewer_get_prev_char:
        pusha
        lea     ecx, [ebp+viewer_data.buf]
        cmp     [ebp+viewer_data.buf_pos], ecx
        ja      .buffered
        mov     eax, dword [ebp+viewer_data.buf_start]
        mov     edx, dword [ebp+viewer_data.buf_start+4]
        test    eax, eax
        jnz     @f
        test    edx, edx
        jnz     @f
        stc
        jmp     .ret
@@:
        sub     eax, 8192
        sbb     edx, 0
        jnc     @f
        xor     eax, eax
        xor     edx, edx
@@:
        call    viewer_seek
        add     [ebp+viewer_data.buf_pos], 8192
.buffered:
        mov     eax, [ebp+viewer_data.buf_pos]
        dec     eax
        mov     [ebp+viewer_data.buf_pos], eax
        mov     al, [eax]
        mov     [esp+28], al
        clc
.ret:
        popa
        ret

fld_uint64:
        fild    qword [eax]
        test    byte [eax+7], 80h
        jz      .ret
        fadd    [@f]
.ret:
        ret
@@      dq      18446744073709551616.0 ;0x10000000000000000

viewer_draw_text:
        call    viewer_seek_to_current
        xor     eax, eax
        xor     edx, edx
        call    get_console_ptr
        mov     ah, [view_status_color]
        lea     esi, [ebp+viewer_data.filename]
        mov     dl, 3
        mov     ecx, [cur_width]
        sub     ecx, 44
        cmp     ecx, 20
        jae     .5
        mov     cl, 20
.5:
        lodsb
        cmp     al, '/'
        jnz     @f
        dec     edx
        jz      .6
@@:
        stosw
        loop    .5
.6:
        dec     ecx
        stosw
        push    esi
@@:
        inc     esi
        cmp     byte [esi-1], 0
        jnz     @b
        sub     esi, [esp]
        dec     esi
        cmp     ecx, esi
        jae     .7
        mov     al, '.'
        stosw
        stosw
        stosw
        sub     ecx, 3
        sub     esi, ecx
        add     [esp], esi
.7:
        pop     esi
@@:
        lodsb
        test    al, al
        jz      @f
        dec     ecx
        stosw
        jmp     @b
@@:
        mov     al, ' '
        add     ecx, 8
        rep     stosw
        movzx   esi, [ebp+viewer_data.encoding]
        lea     esi, [encodings.names+esi*8]
        mov     cl, 8
@@:
        lodsb
        stosw
        loop    @b
        mov     al, ' '
        mov     cl, 8
        rep     stosw
        std
        push    edi
        mov     edx, dword [ebp+viewer_data.filesize+4]
        mov     eax, dword [ebp+viewer_data.filesize]
        mov     cl, 10
@@:
        push    eax
        mov     eax, edx
        xor     edx, edx
        div     ecx
        xchg    eax, [esp]
        div     ecx
        xchg    eax, edx
        add     al, '0'
        mov     ah, [view_status_color]
        stosw
        xchg    eax, edx
        pop     edx
        test    eax, eax
        jnz     @b
        test    edx, edx
        jnz     @b
        mov     al, ' '
        mov     ah, [view_status_color]
        stosw
        cld
        pop     edi
        inc     edi
        inc     edi
        mov     al, ' '
        mov     cl, 5
        rep     stosw
if lang eq ru
        mov     al, 'К'
        stosw
        mov     al, 'о'
        stosw
        mov     al, 'л'
        stosw
else
        mov     al, 'C'
        stosw
        mov     al, 'o'
        stosw
        mov     al, 'l'
        stosw
end if
        mov     al, ' '
        stosw
        push    edi
        mov     eax, dword [ebp+viewer_data.col]
        mov     edx, dword [ebp+viewer_data.col+4]
        test    edx, edx
        jnz     .col_big
        cmp     eax, 100000
        jae     .col_big
        mov     cl, 10
        push    -'0'
@@:
        div     ecx
        push    edx
        xor     edx, edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     eax, '0'
        jz      .col_done
        mov     ah, [view_status_color]
        stosw
        jmp     @b
.col_big:
        push    eax
        mov     al, '.'
        mov     ah, [view_status_color]
        stosw
        stosw
        stosw
        xor     eax, eax
        xchg    eax, edx
        mov     cl, 100
        div     ecx
        pop     eax
        div     ecx
        mov     cl, 10
        xor     eax, eax
        xchg    eax, edx
        div     ecx
        add     al, '0'
        mov     ah, [view_status_color]
        stosw
        mov     al, dl
        add     al, '0'
        stosw
.col_done:
        pop     ecx
        add     ecx, 10*2
        sub     ecx, edi
        shr     ecx, 1
        mov     al, ' '
        mov     ah, [view_status_color]
        rep     stosw
        finit
        fldcw   [fpu_cw]
        mov     [tmp], 100
        lea     eax, [ebp+viewer_data.filesize]
        cmp     dword [eax], 0
        jnz     @f
        cmp     dword [eax+4], 0
        jz      .size_zero
@@:
        call    fld_uint64
        lea     eax, [ebp+viewer_data.cur_pos]
        call    fld_uint64
        fdivrp
        fimul   [_100d]
        fistp   [tmp]
.size_zero:
        mov     byte [edi-2], '%'
        sub     edi, 4
        mov     eax, [tmp]
        mov     cl, 10
@@:
        xor     edx, edx
        div     ecx
        xchg    eax, edx
        add     al, '0'
        mov     [edi], al
        sub     edi, 2
        xchg    eax, edx
        test    eax, eax
        jnz     @b
        xor     eax, eax
        push    1
        pop     edx
        call    get_console_ptr
        test    [ebp+viewer_data.flags], 1
        jnz     .DrawHex
        push    2
        pop     edx
        xor     ecx, ecx
        mov     dword [scrpos], ecx
        mov     dword [scrpos+4], ecx
        mov     dword [viewer_right_side], ecx
        mov     dword [viewer_right_side+4], ecx
.1:
        call    viewer_get_next_char
        jc      .done
.2:
        cmp     al, 0xD
        jz      .newline
        cmp     al, 0xA
        jz      .newline
        cmp     ecx, [cur_width]
        jb      .no_newline
        test    [ebp+viewer_data.flags], 2
        jnz     .no_newline
.newline:
        push    eax
        mov     ecx, dword [scrpos]
        mov     eax, dword [scrpos+4]
        sub     ecx, dword [ebp+viewer_data.col]
        sbb     eax, dword [ebp+viewer_data.col+4]
        ja      .line_filled
        jb      .line_full
        cmp     ecx, [cur_width]
        ja      .line_filled
        jmp     @f
.line_full:
        xor     ecx, ecx
@@:
        test    ecx, ecx
        jnz     @f
        mov     eax, dword [scrpos]
        or      eax, dword [scrpos+4]
        jz      @f
        inc     ecx
@@:
        sub     ecx, [cur_width]
        neg     ecx
        mov     al, ' '
        mov     ah, [view_normal_color]
        rep     stosw
.line_filled:
        mov     eax, dword [scrpos]
        sub     eax, dword [viewer_right_side]
        mov     eax, dword [scrpos+4]
        sbb     eax, dword [viewer_right_side+4]
        jb      @f
        mov     eax, dword [scrpos]
        mov     dword [viewer_right_side], eax
        mov     eax, dword [scrpos+4]
        mov     dword [viewer_right_side+4], eax
@@:
        xor     ecx, ecx
        mov     dword [scrpos], ecx
        mov     dword [scrpos+4], ecx
        inc     edx
        pop     eax
        cmp     edx, [cur_height]
        jae     .done
.no_newline:
        cmp     al, 0xD
        jz      .3
        cmp     al, 0xA
        jz      .3
.4:
        test    al, al
        jns     @f
        movzx   esi, [ebp+viewer_data.encoding]
        shl     esi, 7
        movzx   eax, al
        mov     al, [encodings.tables-80h+esi+eax]
@@:
        cmp     al, 9
        jz      .tab
        call    .write_char
        inc     ecx
        jns     .1
        push    edx
        mov     eax, ecx
        xor     edx, edx
        div     [tabsize]
        mov     ecx, edx
        pop     edx
        jmp     .1
.3:
        mov     bl, al
        call    viewer_get_next_char
        jc      .done
        xor     bl, al
        cmp     bl, 0xA xor 0xD
        jz      .1
        jmp     .2
.tab:
        push    edx
        push    ecx
        mov     eax, ecx
        mov     ecx, [tabsize]
        xor     edx, edx
        div     ecx
        pop     eax
        sub     ecx, edx
        add     eax, ecx
        test    [ebp+viewer_data.flags], 2
        jnz     @f
        cmp     eax, [cur_width]
        jbe     @f
        sub     eax, [cur_width]
        sub     ecx, eax
        mov     eax, [cur_width]
@@:
        push    eax
        mov     al, ' '
@@:
        call    .write_char
        loop    @b
        pop     ecx
        pop     edx
        jmp     .1
.done:
        setc    [ebp+viewer_data.bEofReached]
.done2:
        inc     edx
        cmp     edx, [cur_height]
        jnc     @f
        mov     eax, dword [ebp+viewer_data.cur_pos]
        or      eax, dword [ebp+viewer_data.cur_pos+4]
        jz      @f
        call    viewer_seek_to_current
        call    viewer_prev_newline
        jmp     viewer_draw_text
@@:
        push    edi
        xor     eax, eax
        mov     edx, [cur_height]
        dec     edx
        call    get_console_ptr
        mov     ecx, edi
        pop     edi
        sub     ecx, edi
        shr     ecx, 1
        mov     al, ' '
        mov     ah, [view_normal_color]
        rep     stosw
        cmp     [ebp+viewer_data.bEofReached], 0
        jz      @f
        xor     edx, edx
        mov     eax, [cur_width]
        sub     eax, 4
        call    get_console_ptr
        mov     ah, [view_status_color]
        mov     al, '1'
        stosw
        mov     al, '0'
        stosw
        stosw
@@:
        call    draw_image
        ret
.DrawHex:
        xor     esi, esi
        mov     [ebp+viewer_data.bEofReached], 0
.line:
        mov     al, ' '
        mov     ah, [view_normal_color]
        push    edi
        mov     ecx, [cur_width]
        rep     stosw
        mov     ebx, edi
        pop     edi
        call    viewer_get_next_char
        jc      .hexdone
        push    eax
        push    edi
        add     edi, 36*2
        mov     al, 0xB3
        call    .safechar
        pop     edi
        mov     eax, dword [ebp+viewer_data.cur_pos]
        mov     edx, dword [ebp+viewer_data.cur_pos+4]
        push    esi
        shl     esi, 4
        add     eax, esi
        adc     edx, 0
        pop     esi
        push    eax
        mov     al, dl
        shr     eax, 4
        and     al, 0xF
        call    .hex_digit
        mov     al, dl
        and     al, 0xF
        call    .hex_digit
        pop     eax
        push    8
        pop     ecx
        add     edi, 7*2
        std
@@:
        push    eax
        and     al, 0xF
        call    .hex_digit
        pop     eax
        shr     eax, 4
        loop    @b
        cld
        add     edi, 9*2
        mov     al, ':'
        mov     ah, [view_normal_color]
        stosw
        mov     al, ' '
        stosw
        xor     ecx, ecx
        pop     eax
        jmp     @f
.hexchar:
        call    viewer_get_next_char
        jc      .hexdone
@@:
        push    eax
        shr     al, 4
        call    .hex_digit
        pop     eax
        push    eax
        and     al, 0xF
        call    .hex_digit
        mov     al, ' '
        call    .safechar
        pop     eax
        push    edi
        add     edi, 48*2
        push    ecx
        shl     ecx, 2
        sub     edi, ecx
        pop     ecx
        cmp     ecx, 8
        jb      @f
        sub     edi, 4
@@:
        test    al, al
        jns     @f
        movzx   edx, [ebp+viewer_data.encoding]
        shl     edx, 7
        movzx   eax, al
        mov     al, [encodings.tables+eax+edx-80h]
        mov     ah, [view_normal_color]
@@:
        call    .safechar
        pop     edi
        inc     ecx
        cmp     ecx, 8
        jnz     @f
        mov     al, 0xB3
        call    .safechar
        mov     al, ' '
        call    .safechar
@@:
        cmp     ecx, 16
        jnz     .hexchar
        mov     edi, ebx
        add     esi, 3
        mov     edx, esi
        cmp     esi, [cur_height]
        jae     .done2
        dec     esi
        dec     esi
        jmp     .line
.hexdone:
        mov     [ebp+viewer_data.bEofReached], 1
        mov     edi, ebx
        lea     edx, [esi+2]
        jmp     .done2

.hex_digit:
        push    eax
        cmp     al, 10
        sbb     al, 69h
        das
        mov     ah, [view_normal_color]
        call    .safechar
        pop     eax
        ret
.safechar:
        cmp     edi, ebx
        jae     @f
        stosw
@@:
        ret
.write_char:
        push    ecx eax
        mov     ecx, dword [scrpos]
        mov     eax, dword [scrpos+4]
        sub     ecx, dword [ebp+viewer_data.col]
        sbb     eax, dword [ebp+viewer_data.col+4]
        jb      .left
        ja      .skip
        test    ecx, ecx
        jnz     @f
        mov     eax, dword [scrpos]
        or      eax, dword [scrpos+4]
        jnz     .skip
@@:
        cmp     ecx, [cur_width]
        ja      .skip
        jb      .do
        mov     al, '>'
        mov     [edi-2], al
        mov     al, [view_arrows_color]
        mov     [edi-1], al
        jmp     .skip
.left:
        mov     eax, dword [scrpos]
        or      eax, dword [scrpos+4]
        jnz     .skip
        mov     al, '<'
        mov     ah, [view_arrows_color]
        stosw
        jmp     .skip
.do:
        mov     al, [esp]
        mov     ah, [view_normal_color]
        stosw
.skip:
        pop     eax ecx
        add     dword [scrpos], 1
        adc     dword [scrpos+4], 0
        ret

viewer_seek_to_current:
        mov     eax, dword [ebp+viewer_data.cur_pos]
        mov     edx, dword [ebp+viewer_data.cur_pos+4]
viewer_seek:
        push    eax edx
        sub     eax, dword [ebp+viewer_data.buf_start]
        sbb     edx, dword [ebp+viewer_data.buf_start+4]
        jb      .prev
        ja      .next
        cmp     eax, 16384
        jae     .next
        lea     eax, [ebp+viewer_data.buf+eax]
        mov     [ebp+viewer_data.buf_pos], eax
        pop     edx eax
        ret
.prev:
        inc     edx
        jnz     .read_full
        and     eax, not 511
        cmp     eax, -16384
        jbe     .read_full
        lea     edi, [ebp+viewer_data.buf+16384-4]
        lea     esi, [edi+eax]
        lea     ecx, [eax+16384]
        push    ecx
        shr     ecx, 2
        std
        rep     movsd
        cld
        pop     ecx
        add     dword [ebp+viewer_data.buf_start], eax
        adc     dword [ebp+viewer_data.buf_start+4], -1
        neg     eax
        cmp     dword [ebp+viewer_data.buf_size], ecx
        jb      .doread
        mov     dword [ebp+viewer_data.buf_size], ecx
        jmp     .doread
.read_full:
        pop     edx eax
        push    eax edx
        and     eax, not 511
        sub     eax, 8192-512
        sbb     edx, 0
        jnc     @f
        xor     eax, eax
        xor     edx, edx
@@:
        mov     dword [ebp+viewer_data.buf_start], eax
        mov     dword [ebp+viewer_data.buf_start+4], edx
        and     dword [ebp+viewer_data.buf_size], 0
        mov     eax, 16384
.doread:
        mov     ebx, readinfo
        push    dword [ebp+viewer_data.buf_start]
        pop     dword [ebx+4]
        push    dword [ebp+viewer_data.buf_start+4]
        pop     dword [ebx+8]
        mov     [ebx+12], eax
        lea     eax, [ebp+viewer_data.buf]
        mov     [ebx+16], eax
        lea     eax, [ebp+viewer_data.filename]
        mov     [ebx+21], eax
        push    70
        pop     eax
        int     40h
        test    eax, eax
        jz      .readok
        cmp     eax, 6
        jz      .readok
        call    ask_retry_ignore
        jz      .doread
.readok:
        cmp     ebx, [readinfo.size]
        jnz     @f
        add     ebx, [ebp+viewer_data.buf_size]
@@:
.ret:
        mov     [ebp+viewer_data.buf_size], ebx
        pop     edx eax
        sub     eax, dword [ebp+viewer_data.buf_start]
        lea     eax, [ebp+eax+viewer_data.buf]
        mov     [ebp+viewer_data.buf_pos], eax
        ret
.next:
        or      eax, 511
        sub     eax, 16384-1
        sbb     edx, 0
        jnz     .read_full
        cmp     eax, 16384
        jae     .read_full
        cmp     [ebp+viewer_data.buf_size], 16384
        jb      .read_full
        lea     edi, [ebp+viewer_data.buf]
        lea     esi, [edi+eax]
        mov     ecx, 16384
        sub     ecx, eax
        shr     ecx, 2
        rep     movsd
        add     dword [ebp+viewer_data.buf_start], eax
        adc     dword [ebp+viewer_data.buf_start+4], 0
        mov     ebx, readinfo
        mov     [ebx+16], edi
        sub     edi, ebp
        sub     edi, viewer_data.buf
        mov     edx, dword [ebp+viewer_data.buf_start]
        add     edx, edi
        mov     [ebx+4], edx
        mov     edx, dword [ebp+viewer_data.buf_start+4]
        adc     edx, 0
        mov     [ebx+8], edx
        mov     [ebx+12], eax
        lea     eax, [ebp+viewer_data.filename]
        mov     [ebx+21], eax
@@:
        push    70
        pop     eax
        int     40h
        test    eax, eax
        jz      @f
        cmp     eax, 6
        jz      @f
        call    ask_retry_ignore
        jnz     @f
        mov     ebx, readinfo
        jmp     @b
@@:
        sub     ebx, [readinfo.size]
        add     ebx, 16384
        jmp     .ret

ask_retry_ignore:
        push    esi
        lea     esi, [ebp+viewer_data.filename]
        push    esi
        push    aCannotReadFile
        call    get_error_msg
        push    eax
        mov     eax, esp
        push    RetryOrIgnoreBtn
        push    2
        push    eax
        push    3
        push    -1
        push    -1
        push    aError
        call    SayErr
        add     esp, 3*4
        pop     esi
        test    eax, eax
        ret

viewer_set_curpos:
        mov     eax, [ebp+viewer_data.buf_pos]
        sub     eax, ebp
        sub     eax, viewer_data.buf
        xor     edx, edx
        add     eax, dword [ebp+viewer_data.buf_start]
        adc     edx, dword [ebp+viewer_data.buf_start+4]
        mov     dword [ebp+viewer_data.cur_pos], eax
        mov     dword [ebp+viewer_data.cur_pos+4], edx
        ret

viewer_next_newline:
        test    [ebp+viewer_data.flags], 1
        jz      .nohex
        push    10h
        pop     ecx
@@:
        call    viewer_get_next_char
        jc      @f
        loop    @b
        call    viewer_set_curpos
        clc
@@:     ret
.nohex:
        xor     ecx, ecx
@@:
        call    viewer_get_next_char
        jc      .ret
        cmp     al, 0xD
        jz      .1
        cmp     al, 0xA
        jz      .1
        cmp     al, 9
        jz      .tab
        inc     ecx
.next:
        test    [ebp+viewer_data.flags], 2
        jnz     @b
        cmp     ecx, [cur_width]
        jbe     @b
        call    viewer_get_prev_char
        jmp     .2
.tab:
        push    ecx
        mov     eax, ecx
        mov     ecx, [tabsize]
        xor     edx, edx
        div     ecx
        sub     ecx, edx
        add     [esp], ecx
        pop     ecx
        jmp     .next
.1:
        mov     cl, al
        call    viewer_get_next_char
        jc      .ret
        xor     cl, al
        cmp     cl, 0xD xor 0xA
        jz      .2
        dec     [ebp+viewer_data.buf_pos]
.2:
.ret:
        call    viewer_set_curpos
        ret

viewer_prev_newline:
        test    [ebp+viewer_data.flags], 1
        jz      .nohex
        push    10h
        pop     ecx
@@:
        call    viewer_get_prev_char
        jc      @f
        loop    @b
@@:     call    viewer_set_curpos
        ret
.nohex:
        mov     eax, 8192
        push    eax
        cdq
        div     [cur_width]
        pop     eax
        sub     eax, edx
        mov     edx, eax
        call    viewer_get_prev_char
        jc      .0
        dec     edx
        cmp     al, 0xD
        jz      .2
        cmp     al, 0xA
        jnz     .3
.2:
        mov     cl, al
        call    viewer_get_prev_char
        jc      .0
        dec     edx
        xor     cl, al
        cmp     cl, 0xD xor 0xA
        jnz     .3
@@:
        call    viewer_get_prev_char
        jc      .0
        dec     edx
        jz      .0
.3:
        cmp     al, 0xD
        jz      .1
        cmp     al, 0xA
        jnz     @b
.1:
        call    viewer_get_next_char
.0:
        push    dword [ebp+viewer_data.cur_pos+4]
        push    dword [ebp+viewer_data.cur_pos]
        call    viewer_set_curpos
@@:
        push    dword [ebp+viewer_data.cur_pos+4]
        push    dword [ebp+viewer_data.cur_pos]
        call    viewer_next_newline
        jc      .ret3
        mov     eax, dword [ebp+viewer_data.cur_pos]
        mov     edx, dword [ebp+viewer_data.cur_pos+4]
        sub     eax, [esp+8]
        sbb     edx, [esp+12]
        jae     .ret3
        pop     eax
        pop     eax
        jmp     @b
.ret3:
        pop     dword [ebp+viewer_data.cur_pos]
        pop     dword [ebp+viewer_data.cur_pos+4]
.ret:
        pop     eax
        pop     eax
        jmp     viewer_seek_to_current

viewer_set_keybar:
        mov     eax, keybar_viewer
if lang eq ru
        test    [ebp+viewer_data.flags], 1
        jnz     .hex
        mov     dword [eax+(4-1)*6], 'Код '
        mov     word [eax+(4-1)*6+4], '  '
        jmp     @f
.hex:
        mov     dword [eax+(4-1)*6], 'Текс'
        mov     word [eax+(4-1)*6+4], 'т '
@@:
        test    [ebp+viewer_data.flags], 2
        jnz     .unwrap
        mov     dword [eax+6], 'Разв'
        mov     word [eax+6+4], 'ер'
        jmp     @f
.unwrap:
        mov     dword [eax+6], 'Свер'
        mov     word [eax+6+4], 'н '
@@:
else
        test    [ebp+viewer_data.flags], 1
        jnz     .hex
        mov     dword [eax+(4-1)*6], 'Hex '
;        mov     word [eax+(4-1)*6+4], '  '
        jmp     @f
.hex:
        mov     dword [eax+(4-1)*6], 'Text'
;        mov     word [eax+(4-1)*6+4], '  '
@@:
        test    [ebp+viewer_data.flags], 2
        jnz     .unwrap
        mov     dword [eax+6], 'Unwr'
        mov     word [eax+6+4], 'ap'
        jmp     @f
.unwrap:
        mov     dword [eax+6], 'Wrap'
        mov     word [eax+6+4], '  '
@@:
end if
        movzx   esi, [ebp+viewer_data.encoding]
        xor     esi, 1
        lea     esi, [encodings.names+esi*8]
        lea     edi, [eax+keybar_cp-keybar_viewer]
        movsd
        movsw
        jmp     draw_keybar

viewer_OnRedraw:
        call    viewer_set_keybar
        call    viewer_draw_text
        ret

viewer_OnKey:
        mov     esi, viewer_ctrlkeys
        jmp     process_ctrl_keys
.exit:
        call    delete_active_screen
        ret
.down:
        cmp     [ebp+viewer_data.bEofReached], 0
        jnz     .ret
        call    viewer_seek_to_current
        call    viewer_next_newline
        jc      .ret
        call    viewer_draw_text
        ret
.pgdn:
        cmp     [ebp+viewer_data.bEofReached], 0
        jnz     .ret
        call    viewer_seek_to_current
        mov     ebx, [cur_height]
        sub     ebx, 3
@@:
        call    viewer_next_newline
        jc      .ret
        dec     ebx
        jnz     @b
        call    viewer_draw_text
        ret
.up:
        call    viewer_seek_to_current
        call    viewer_prev_newline
        call    viewer_draw_text
        ret
.pgup:
        call    viewer_seek_to_current
        mov     ebx, [cur_height]
        sub     ebx, 3
@@:
        call    viewer_prev_newline
        dec     ebx
        jnz     @b
.done_redraw:
        call    viewer_draw_text
.ret:
        ret
.f8:
        movzx   esi, [ebp+viewer_data.encoding]
        xor     [ebp+viewer_data.encoding], 1
        call    viewer_set_keybar
        jmp     .done_redraw
.f2:
        xor     [ebp+viewer_data.flags], 2
        call    viewer_set_keybar
        jmp     .done_redraw
.f4:
        xor     [ebp+viewer_data.flags], 1
        pushf
        call    viewer_set_keybar
        popf
        jnz     .tohex
        add     dword [ebp+viewer_data.cur_pos], 1
        adc     dword [ebp+viewer_data.cur_pos+4], 0
        call    viewer_seek_to_current
        call    viewer_prev_newline
        jmp     .done_redraw
.tohex:
        and     byte [ebp+viewer_data.cur_pos], 0xF0
        jmp     .done_redraw
.home:
        xor     eax, eax
        mov     dword [ebp+viewer_data.cur_pos], eax
        mov     dword [ebp+viewer_data.cur_pos+4], eax
        mov     dword [ebp+viewer_data.col], eax
        mov     dword [ebp+viewer_data.col+4], eax
        jmp     .done_redraw
.end:
        mov     eax, dword [ebp+viewer_data.filesize]
        mov     edx, dword [ebp+viewer_data.filesize+4]
        and     al, 0xF0
        mov     dword [ebp+viewer_data.cur_pos], eax
        mov     dword [ebp+viewer_data.cur_pos+4], edx
        and     dword [ebp+viewer_data.col], 0
        and     dword [ebp+viewer_data.col+4], 0
        jmp     .done_redraw
.right:
        add     dword [ebp+viewer_data.col], 1
        adc     dword [ebp+viewer_data.col+4], 0
        jnc     .done_redraw
        sub     dword [ebp+viewer_data.col], 1
        sbb     dword [ebp+viewer_data.col+4], 0
        ret
.left:
        sub     dword [ebp+viewer_data.col], 1
        sbb     dword [ebp+viewer_data.col+4], 0
        jnc     .done_redraw
        add     dword [ebp+viewer_data.col], 1
        adc     dword [ebp+viewer_data.col+4], 0
        ret
.ctrl_left:
        test    [ebp+viewer_data.flags], 1
        jnz     .ctrl_left_hex
        sub     dword [ebp+viewer_data.col], 20
        sbb     dword [ebp+viewer_data.col+4], 0
        jnc     .done_redraw
.ctrl_shift_left:
        and     dword [ebp+viewer_data.col], 0
        and     dword [ebp+viewer_data.col+4], 0
        jmp     .done_redraw
.ctrl_left_hex:
        call    viewer_seek_to_current
        call    viewer_get_prev_char
        jc      @f
        call    viewer_set_curpos
        jmp     .done_redraw
.ctrl_right:
        test    [ebp+viewer_data.flags], 1
        jnz     .ctrl_right_hex
        add     dword [ebp+viewer_data.col], 20
        adc     dword [ebp+viewer_data.col+4], 0
        jnc     .done_redraw
        sub     dword [ebp+viewer_data.col], 20
        sbb     dword [ebp+viewer_data.col+4], 0
@@:     ret
.ctrl_right_hex:
        call    viewer_seek_to_current
        call    viewer_get_next_char
        call    viewer_set_curpos
        jmp     .done_redraw
.ctrl_shift_right:
        test    [ebp+viewer_data.flags], 1
        jnz     @b
        mov     eax, dword [viewer_right_side]
        mov     edx, dword [viewer_right_side+4]
        sub     eax, [cur_width]
        sbb     edx, 0
        jnc     @f
        xor     eax, eax
        xor     edx, edx
@@:
        mov     dword [ebp+viewer_data.col], eax
        mov     dword [ebp+viewer_data.col+4], edx
        jmp     .done_redraw

viewer_getname:
if lang eq ru
        mov     eax, 'Прос'
        stosd
        mov     eax, 'мотр'
        stosd
        mov     eax, '    '
        stosd
        stosb
else
        mov     eax, 'View'
        stosd
        mov     eax, '    '
        stosd
        stosd
        stosb
end if
        sub     ecx, 13
        lea     esi, [ebp+viewer_data.filename]
        push    3
        pop     edx
@@:
        lodsb
        stosb
        dec     ecx
        cmp     al, '/'
        jnz     @b
        dec     edx
        jnz     @b
        push    esi
@@:
        lodsb
        test    al, al
        jnz     @b
        sub     esi, [esp]
        dec     esi
        cmp     esi, ecx
        jbe     @f
        mov     al, '.'
        stosb
        stosb
        stosb
        sub     ecx, 3
        sub     esi, ecx
        add     [esp], esi
@@:
        pop     esi
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
        ret
