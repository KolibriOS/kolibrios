;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;; DISASSEMBLER ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; TODO: prepare to work independently from debugger

;-----------------------------------------------------------------------------
;                      Read next byte for disassembly
;
; out: AL = byte
disasm_get_byte:
        push    ecx
        mov     ecx, [disasm_cur_pos]
        sub     ecx, [disasm_start_pos]
        cmp     ecx, [disasm_buf_size]
        jae     disasm_err
        mov     al, [disasm_buffer+ecx]
        pop     ecx
        inc     [disasm_cur_pos]
        ret

;-----------------------------------------------------------------------------
;                      Read next word for disassembly
;
; out: AX = word
disasm_get_word:
        push    ecx
        mov     ecx, [disasm_cur_pos]
        sub     ecx, [disasm_start_pos]
        inc     ecx
        cmp     ecx, [disasm_buf_size]
        jae     disasm_err
        mov     ax, word [disasm_buffer-1+ecx]
        pop     ecx
        add     [disasm_cur_pos], 2
        ret

;-----------------------------------------------------------------------------
;                     Read next dword for disassembly
;
; out: EAX = dword
disasm_get_dword:
        push    ecx
        mov     ecx, [disasm_cur_pos]
        sub     ecx, [disasm_start_pos]
        add     ecx, 3
        cmp     ecx, [disasm_buf_size]
        jae     disasm_err
        mov     eax, dword [disasm_buffer-3+ecx]
        pop     ecx
        add     [disasm_cur_pos], 4
        ret

;-----------------------------------------------------------------------------

disasm_err:
        mov     esp, ebp

; TODO: make it local?
stc_ret:
        stc
        ret

;-----------------------------------------------------------------------------
;                       Exit from disassembly loop

disasm_ret:
        mov     esp, ebp
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------
;                       Disassembly one instruction
;
; Read data, in loop, to read multibyte instruction opcodes

disasm_instr:
        mov     ebp, esp
        cmp     [debuggee_pid], 0
        jz      stc_ret
        mov     edi, disasm_string
        xor     ecx, ecx

; TODO: make it local?
; ecx=flags (IN or OUT?)
disasm_loop1:
        xor     eax, eax
        call    disasm_get_byte
        jmp     dword [disasm_table_1 + eax*4]

;-----------------------------------------------------------------------------

cop0:
clock:
csegcs:
csegds:
cseges:
csegss:
csegfs:
cseggs:
        mov     esi, cmd1

iglobal
cmd1:
        db      0x2E,3,'cs:'
        db      0x36,3,'ss:'
        db      0x3E,3,'ds:'
        db      0x26,3,'es:'
        db      0x64,3,'fs:'
        db      0x65,3,'gs:'
        db      0x06,10,'push    es'
        db      0x07,10,'pop     es'
        db      0x0E,10,'push    cs'
        db      0x16,10,'push    ss'
        db      0x17,10,'pop     ss'
        db      0x1E,10,'push    ds'
        db      0x1F,10,'pop     ds'
        db      0x27,3,'daa'
        db      0x2F,3,'das'
        db      0x37,3,'aaa'
        db      0x3F,3,'aas'
        db      0x60,6,0,'pusha'
        db      0x61,5,0,'popa'
        db      0x90,3,'nop'
        db      0x9B,5,'fwait'
        db      0x9C,6,0,'pushf'
        db      0x9D,5,0,'popf'
        db      0x9E,4,'sahf'
        db      0x9F,4,'lahf'
        db      0xA4,5,'movsb'
        db      0xA5,5,0,'movs'
        db      0xA6,5,'cmpsb'
        db      0xA7,5,0,'cmps'
        db      0xAA,5,'stosb'
        db      0xAB,5,0,'stos'
        db      0xAC,5,'lodsb'
        db      0xAD,5,0,'lods'
        db      0xAE,5,'scasb'
        db      0xAF,5,0,'scas'
        db      0xC3,3,'ret'
        db      0xC9,5,'leave'
        db      0xCC,4,'int3'
        db      0xF0,4,'lock'
        db      0xF5,3,'cmc'
        db      0xF8,3,'clc'
        db      0xF9,3,'stc'
        db      0xFA,3,'cli'
        db      0xFB,3,'sti'
        db      0xFC,3,'cld'
        db      0xFD,3,'std'

cmd2:
        db      0x05,7,'syscall'
        db      0x06,4,'clts'
        db      0x31,5,'rdtsc'
        db      0x34,8,'sysenter'
        db      0xA2,5,'cpuid'
        db      0x77,4,'emms'

endg
        jmp     @f

;-----------------------------------------------------------------------------

ccpuid:
crdtsc:
cemms:
cop0_F:
        mov     esi, cmd2

    @@:
        cmp     al, [esi]
        jz      .found
        inc     esi
        movzx   edx, byte [esi]
        inc     esi
        add     esi, edx
        jmp     @b

    .found:
        inc     esi
        lodsb
        cmp     byte [esi], 0
        jz      @f
        movzx   ecx, al

    disasm_1:
        rep movsb
        and     byte [edi], 0
        ret

    @@:
        mov     dl, ch
        movzx   ecx, al
        dec     ecx
        inc     esi
        rep movsb
        test    dl, 1
        mov     al, 'w'
        jnz     @f
        mov     al, 'd'

    @@:
        stosb
        and     byte [edi], 0
        ret

    c67:
        or      ch, 2
        jmp     disasm_loop1

    c66:
        or      ch, 1
        jmp     disasm_loop1

;-----------------------------------------------------------------------------

cxlat:
cunk:
cerr:
        mov     eax, '???'
        stosd
        clc
        ret

cF:
        call    disasm_get_byte
        jmp     dword [disasm_table_2 + eax*4]

;-----------------------------------------------------------------------------
;                        Parse operand prefixes

crep:
        push    [disasm_cur_pos]
        call    disasm_get_byte
        cmp     al, 0x0F
        jz      .sse
        mov     dl, al
        mov     eax, 'rep '
        stosd
        mov     al, dl

    @@:
        and     eax, not 1
        cmp     al, 0x66
        jnz     @f
        call    disasm_get_byte
        mov     dl, al
        jmp     @b

    @@:
        cmp     al, 0xA6
        jz      .repz
        cmp     al, 0xAE
        jz      .repz
        cmp     al, 0xA4
        jz      .prefix
        cmp     al, 0xAA
        jz      .prefix
        cmp     al, 0xAC
        jz      .prefix
        cmp     al, 0x6C
        jz      .prefix
        cmp     al, 0x6E
        jz      .prefix

        pop     [disasm_cur_pos]
    .noprefix:
        and     byte [edi-1], 0
        ret

    .repz:
        mov     byte [edi-1], 'z'
        mov     al, ' '
        stosb

    .prefix:
        pop     [disasm_cur_pos]
        jmp     disasm_loop1

    .sse:
        pop     eax
        call    disasm_get_byte

;-----------------------------------------------------------------------------

iglobal
rep_sse_cmds:
        db      0x58,3,'add'
        db      0xC2,3,'cmp'
        db      0,0
endg
        mov     esi, rep_sse_cmds+1

    @@:
        movzx   edx, byte [esi]
        cmp     al, [esi-1]
        jz      @f
        lea     esi, [esi+edx+2]
        cmp     byte [esi], 0
        jnz     @b
        sub     [disasm_cur_pos], 2
        mov     eax, 'rep'
        stosd
        ret

    @@:
        push    ecx
        mov     ecx, edx
        inc     esi
        rep movsb
        pop     ecx
        mov     al, 's'
        stosb
        jmp     rep_sse_final

;-----------------------------------------------------------------------------

crepnz:
        call    disasm_get_byte
        cmp     al, 0x0F
        jz      .sse
        mov     dl, al
        mov     eax, 'repn'
        stosd
        mov     al, 'z'
        stosb
        mov     al, ' '
        stosb
        movzx   eax, dl
        cmp     al, 66h
        je      c66
        cmp     al, 67h
        je      c67
        cmp     al, 0x6C
        jb      crep.noprefix
        cmp     al, 0x6F
        jbe     .prefix
        cmp     al, 0xA4
        jb      crep.noprefix
        cmp     al, 0xA7
        jbe     .prefix
        cmp     al, 0xAA
        jb      crep.noprefix
        cmp     al, 0xAF
        ja      crep.noprefix

    .prefix:
        jmp     cop0

    .sse:
        call    disasm_get_byte
        mov     esi, rep_sse_cmds+1

    @@:
        movzx   edx, byte [esi]
        cmp     al, [esi-1]
        jz      .found0
        lea     esi, [esi+edx+2]
        cmp     byte [esi], 0
        jnz     @b
        mov     esi, sse_cmds2+1

    @@:
        movzx   edx, byte [esi]
        cmp     al, [esi-1]
        jz      .found1
        lea     esi, [esi+edx+2]
        cmp     byte [esi], 0
        jnz     @b
        sub     [disasm_cur_pos], 2
        mov     eax, 'repn'
        stosd
        mov     al, 'z'
        stosb
        and     byte [edi], 0
        ret

    .found0:
        push    ecx
        mov     ecx, edx
        inc     esi
        rep movsb
        pop     ecx
        mov     al, 's'
        stosb
        mov     al, 'd'
        jmp     rep_sse_final

    .found1:
        push    ecx
        mov     ecx, edx
        inc     esi
        rep movsb
        pop     ecx
        mov     al, 'p'
        stosb
        mov     al, 's'

    rep_sse_final:
        stosb
        push    ecx
        push    5
        pop     ecx
        sub     ecx, edx
        adc     ecx, 1
        mov     al, ' '
        rep stosb
        pop     ecx
        or      ch, 1
        jmp     disasm_mmx1

;-----------------------------------------------------------------------------

macro disasm_set_modew
{
        test    al, 1
        jz      @f
        or      ch, 80h
    @@:
}

;-----------------------------------------------------------------------------

cmov2:
        disasm_set_modew
    ; mov r/m,i
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        test    al, 00111000b
        jnz     cunk
        mov     eax, 'mov '
        stosd
        mov     eax, '    '
        stosd
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        xor     eax, eax
        test    ch, 80h
        jnz     .1
        call    disasm_get_byte
        jmp     .3

    .1:
        test    ch, 1
        jnz     .2
        call    disasm_get_dword
        jmp     .3

    .2:
        call    disasm_get_word

    .3:
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cret2:
        mov     eax, 'ret '
        stosd
        mov     eax, '    '
        stosd
        xor     eax, eax
        jmp     cmov2.2

;-----------------------------------------------------------------------------

disasm_write_num:
        push    esi
        cmp     eax, 0x80
        jl      .nosymb
        lea     esi, [eax-1]
        test    eax, esi
        jz      .nosymb
        call    find_symbol
        jc      .nosymb

    @@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b

    @@:
        pop     esi
        ret

    .nosymb:
        pop     esi
        push    ecx eax
        inc     edi

    @@:
        mov     ecx, eax
        shr     eax, 4
        jz      @f
        inc     edi
        jmp     @b

    @@:
        pop     eax
        cmp     ecx, 10
        jb      @f
        inc     edi

    @@:
        push    edi eax

    @@:
        mov     ecx, eax
        and     al, 0xF
        cmp     al, 10
        sbb     al, 69h
        das
        dec     edi
        mov     [edi], al
        mov     eax, ecx
        shr     eax, 4
        jnz     @b
        cmp     ecx, 10
        jb      @f
        mov     byte [edi-1], '0'

    @@:
        pop     eax edi ecx
        cmp     eax, 10
        jb      @f
        mov     byte [edi], 'h'
        inc     edi

    @@:
        ret

;-----------------------------------------------------------------------------

iglobal
label disasm_regs32 dword
label disasm_regs dword
        db      'eax',0
        db      'ecx',0
        db      'edx',0
        db      'ebx',0
        db      'esp',0
        db      'ebp',0
        db      'esi',0
        db      'edi',0

disasm_regs16   dw      'ax','cx','dx','bx','sp','bp','si','di'
disasm_regs8    dw      'al','cl','dl','bl','ah','ch','dh','bh'
disasm_scale    db      '1248'
endg

;-----------------------------------------------------------------------------

disasm_readrmop:
        call    disasm_get_byte
        test    ch, 40h
        jnz     .skip_size
        push    eax
        and     al, 0xC0
        cmp     al, 0xC0
        pop     eax
        jz      .skip_size
        test    ch, 80h
        jz      .byte
        test    ch, 1
        jnz     .word
        mov     dword [edi], 'dwor'
        mov     byte [edi+4], 'd'
        inc     edi
        jmp     @f

    .byte:
        test    ch, 20h
        jz      .qb
        mov     byte [edi], 't'
        inc     edi

    .qb:
        mov     dword [edi], 'byte'
        jmp     @f

    .word:
        test    ch, 20h
        jz      .qw
        mov     byte [edi], 'q'
        inc     edi

    .qw:
        mov     dword [edi], 'word'

    @@:
        mov     byte [edi+4], ' '
        add     edi, 5

    .skip_size:
        test    ch, 2
        jnz     disasm_readrmop16
        push    ecx
        movzx   ecx, al
        and     eax, 7
        shr     ecx, 6
        jz      .vmod0
        jp      .vmod3
        mov     byte [edi], '['
        inc     edi
        cmp     al, 4
        jz      .sib1
        mov     eax, [disasm_regs+eax*4]
        stosd
        dec     edi
        jmp     @f

    .sib1:
        call    .parse_sib

    @@:
        mov     al, '+'
        stosb
        dec     ecx
        jz      .vmod1
        call    disasm_get_dword
        jmp     @f

    .vmod1:
        call    disasm_get_byte
        movsx   eax, al

    @@:
        test    eax, eax
        jns     .2
        neg     eax
        mov     byte [edi-1], '-'

    .2:
        call    disasm_write_num

    .2a:
        mov     al, ']'
        stosb
        pop     ecx
        ret

    .vmod3:
        pop     ecx
        test    ch, 10h
        jnz     .vmod3_mmi
        test    ch, 80h
        jz      .vmod3_byte
        test    ch, 1
        jnz     .vmod3_word
        test    ch, 20h
        jnz     .vmod3_sti
        mov     eax, [disasm_regs32+eax*4]
        stosd
        dec     edi
        ret

    .vmod3_byte:
        mov     ax, [disasm_regs8+eax*2]

    @@:
        stosw
        ret

    .vmod3_word:
        mov     ax, [disasm_regs16+eax*2]
        jmp     @b

    .vmod3_sti:
        mov     word [edi], 'st'
        add     al, '0'
        mov     byte [edi+2], al
        add     edi, 3
        ret

    .vmod3_mmi:
    
    disasm_write_mmreg = $

        test    ch, 1
        jz      @f
        mov     byte [edi], 'x'
        inc     edi

    @@:
        mov     word [edi], 'mm'
        add     al, '0'
        mov     byte [edi+2], al
        add     edi, 3
        ret

    .vmod0:
        mov     byte [edi], '['
        inc     edi
        cmp     al, 4
        jz      .sib2
        cmp     al, 5
        jz      .ofs32
        mov     eax, [disasm_regs+eax*4]
        stosd
        mov     byte [edi-1], ']'
        pop     ecx
        ret

    .ofs32:
        call    disasm_get_dword
        jmp     .2

    .sib2:
        call    .parse_sib
        mov     al, ']'
        stosb
        pop     ecx
        ret

    .parse_sib:
        call    disasm_get_byte
        push    edx
        mov     dl, al
        mov     dh, 0
        and     eax, 7
        cmp     al, 5
        jnz     @f
        jecxz   .sib0

    @@:
        mov     eax, [disasm_regs+eax*4]
        stosd
        dec     edi
        mov     dh, 1

    .sib0:
        mov     al, dl
        shr     eax, 3
        and     eax, 7
        cmp     al, 4
        jz      .sibret
        test    dh, dh
        jz      @f
        mov     byte [edi], '+'
        inc     edi

    @@:
        mov     eax, [disasm_regs+eax*4]
        stosd
        dec     edi
        shr     dl, 6
        jz      @f
        mov     al, '*'
        stosb
        movzx   eax, dl
        mov     al, [disasm_scale+eax]
        stosb

    @@:
    .sibret:
        test    dh, dh
        jnz     .sibret2
        call    disasm_get_dword
        cmp     byte [edi-1], '['
        jz      @f
        mov     byte [edi], '+'
        test    eax, eax
        jns     .sibns
        neg     eax
        mov     byte [edi], '-'

    .sibns:
        inc     edi

    @@:
        call    disasm_write_num

    .sibret2:
        pop     edx
        ret

;-----------------------------------------------------------------------------

iglobal
disasm_rm16_1   dd      'bxsi','bxdi','bpsi','bpdi'
disasm_rm16_2   dw      'si','di','bp','bx'
endg

;-----------------------------------------------------------------------------

disasm_readrmop16:
        push    ecx
        movzx   ecx, al
        and     eax, 7
        shr     ecx, 6
        jz      .vmod0
        jp      disasm_readrmop.vmod3   ; mod=3 is the same in 16- and 32-bit code
    ; 1 or 2
        mov     byte [edi], '['
        inc     edi
        cmp     al, 4
        jae     @f
        mov     eax, [disasm_rm16_1+eax*4]
        stosw
        mov     al, '+'
        stosb
        shr     eax, 16
        jmp     .1

    @@:
        mov     eax, dword [disasm_rm16_2+eax*2-4*2]

    .1:
        stosw
        mov     al, '+'
        stosb
        xor     eax, eax
        dec     ecx
        jnz     .2
        call    disasm_get_byte
        cbw
        jmp     @f

    .2:
        call    disasm_get_word

    @@:
        test    ax, ax
        jns     @f
        mov     byte [edi-1], '-'
        neg     ax

    @@:
        call    disasm_write_num

    .done1:
        mov     al, ']'
        stosb
        pop     ecx
        ret

    .vmod0:
        mov     byte [edi], '['
        inc     edi
        cmp     al, 6
        jz      .ofs16
        cmp     al, 4
        jae     @f
        mov     eax, [disasm_rm16_1+eax*4]
        stosw
        mov     al, '+'
        stosb
        shr     eax, 16
        jmp     .3

    @@:
        mov     eax, dword [disasm_rm16_2+eax*2-4*2]

    .3:
        stosw
        jmp     .done1

    .ofs16:
        xor     eax, eax
        call    disasm_get_word
        call    disasm_write_num
        jmp     .done1

;-----------------------------------------------------------------------------

cpush21:
        mov     eax, 'push'
        stosd
        mov     eax, '    '
        stosd

disasm_i32:
        call    disasm_get_dword
        call    disasm_write_num
        and     byte [edi], 0
        ret

cpush22:
        mov     eax, 'push'
        stosd
        mov     eax, '    '
        stosd
        call    disasm_get_byte
        movsx   eax, al

    @@:
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

center:
        mov     eax, 'ente'
        stosd
        mov     eax, 'r   '
        stosd
        xor     eax, eax
        call    disasm_get_word
        call    disasm_write_num
        mov     al, ','
        stosb
        mov     al, ' '
        stosb
        xor     eax, eax
        call    disasm_get_byte
        jmp     @b

;-----------------------------------------------------------------------------

cinc1:
; inc reg32
cdec1:
; dec reg32
cpush1:
; push reg32
cpop1:
; pop reg32
cbswap:
; bswap reg32
        mov     edx, eax
        and     edx, 7
        shr     eax, 3
        sub     al, 8
        mov     esi, 'inc '
        jz      @f
        mov     esi, 'dec '
        dec     al
        jz      @f
        mov     esi, 'push'
        dec     al
        jz      @f
        mov     esi, 'pop '
        dec     al
        jz      @f
        mov     esi, 'bswa'

    @@:
        xchg    eax, esi
        stosd
        mov     eax, '    '
        jz      @f
        mov     al, 'p'

    @@:
        stosd
        xchg    eax, edx
        call    disasm_write_reg1632
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cxchg1:
; xchg eax,reg32
        and     eax, 7
        xchg    eax, edx
        mov     eax, 'xchg'
        stosd
        mov     eax, '    '
        stosd
        xor     eax, eax
        call    disasm_write_reg1632
        mov     ax, ', '
        stosw
        xchg    eax, edx
        call    disasm_write_reg1632
        and     byte [edi], 0
        ret

cint:
        mov     eax, 'int '
        stosd
        mov     eax, '    '
        stosd

disasm_i8u:
        xor     eax, eax
        call    disasm_get_byte
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cmov11:
; mov r8,i8
        mov     ecx, eax
        mov     eax, 'mov '
        stosd
        mov     eax, '    '
        stosd
        and     ecx, 7
        mov     ax, [disasm_regs8+ecx*2]
        stosw
        mov     ax, ', '
        stosw
        jmp     disasm_i8u

;-----------------------------------------------------------------------------

cmov12:
; mov r32,i32
        xchg    eax, edx
        mov     eax, 'mov '
        stosd
        mov     eax, '    '
        stosd
        xchg    eax, edx
        and     eax, 7
        call    disasm_write_reg1632
        mov     ax, ', '
        stosw
        jmp     cmov2.1

;-----------------------------------------------------------------------------

iglobal
disasm_shifts   dd      'rol ','ror ','rcl ','rcr ','shl ','shr ','sal ','sar '
endg

;-----------------------------------------------------------------------------

cshift2:
; shift r/m,1 = D0/D1
cshift3:
; shift r/m,cl = D2/D3
        disasm_set_modew
        mov     dl, al
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        mov     eax, [disasm_shifts+eax*4]
        stosd
        mov     eax, '    '
        stosd
        call    disasm_readrmop
        cmp     dl, 0xD2
        jb      .s1
        mov     eax, ', cl'
        stosd
        and     byte [edi], 0
        ret

    .s1:
        mov     eax, ', 1'
        stosd
        clc
        ret

;-----------------------------------------------------------------------------

cshift1:
; shift r/m,i8 = C0/C1
        disasm_set_modew
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        mov     eax, [disasm_shifts+eax*4]
        stosd
        mov     eax, '    '
        stosd
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        jmp     disasm_i8u

;-----------------------------------------------------------------------------

caam:
        mov     eax, 'aam '
        jmp     @f

caad:
        mov     eax, 'aad '

    @@:
        stosd
        mov     eax, '    '
        stosd
        xor     eax, eax
        call    disasm_get_byte
        cmp     al, 10
        jz      @f
        call    disasm_write_num

    @@:
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cmov3:
; A0: mov al,[ofs32]
; A1: mov ax/eax,[ofs32]
; A2: mov [ofs32],al
; A3: mov [ofs32],ax/eax
        mov     edx, 'mov '
        xchg    eax, edx
        stosd
        mov     eax, '    '
        stosd
        test    dl, 2
        jnz     .1
        call    .write_acc
        mov     ax, ', '
        stosw
        call    .write_ofs32
        jmp     .2

    .1:
        call    .write_ofs32
        mov     ax, ', '
        stosw
        call    .write_acc

    .2:
        and     byte [edi], 0
        ret

    .write_acc:
        test    dl, 1
        jz      .8bit
        test    ch, 1
        jnz     .16bit
        mov     eax, 'eax'
        stosd
        dec     edi
        ret

    .16bit:
        mov     ax, 'ax'
        stosw
        ret

    .8bit:
        mov     ax, 'al'
        stosw
        ret

    .write_ofs32:
        mov     al, '['
        stosb
        call    disasm_get_dword
        call    disasm_write_num
        mov     al, ']'
        stosb
        ret

;-----------------------------------------------------------------------------

disasm_write_reg:
        test    ch, 80h
        jnz     disasm_write_reg1632
        mov     ax, [disasm_regs8+eax*2]
        stosw
        ret

;-----------------------------------------------------------------------------

disasm_write_reg1632:
        test    ch, 1
        jnz     @f
        mov     eax, [disasm_regs32+eax*4]
        stosd
        dec     edi
        ret

    @@:
        mov     ax, [disasm_regs16+eax*2]
        stosw
        ret

;-----------------------------------------------------------------------------

; 0F B6/B7
cmovzx:
; 0F BE/BF
cmovsx:
        mov     edx, eax
        disasm_set_modew
        mov     eax, 'movz'
        cmp     dl, 0xB8
        jb      @f
        mov     eax, 'movs'

    @@:
        stosd
        mov     eax, 'x   '
        stosd
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        call    disasm_write_reg1632
        mov     ax, ', '
        stosw
        or      ch, 1   ; 2nd operand - 8 or 16 bits
        call    disasm_readrmop
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
disasm_op2cmds  dd 'add ','or  ','adc ','sbb ','and ','sub ','xor ','cmp '
endg

;-----------------------------------------------------------------------------

cop21:
        disasm_set_modew
        mov     esi, 'test'
        cmp     al, 0A8h
        jae     @f
        shr     al, 3
        and     eax, 7
        mov     esi, [disasm_op2cmds+eax*4]

    @@:
        xchg    eax, esi
        stosd
        mov     eax, '    '
        stosd
        test    ch, 80h
        jnz     .1632
        mov     eax, 'al, '
        stosd
        jmp     disasm_i8u

    .1632:
        test    ch, 1
        jnz     .16
        mov     eax, 'eax,'
        stosd
        mov     al, ' '
        stosb
        call    disasm_get_dword
        jmp     .x

    .16:
        mov     eax, 'ax, '
        stosd
        xor     eax, eax
        call    disasm_get_word

    .x:
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

carpl:
        xor     edx, edx
        or      ch, 0C1h
        mov     eax, 'arpl'
        jmp     cop22.d2

;-----------------------------------------------------------------------------

ccmpxchg:
        xor     edx, edx
        disasm_set_modew
        or      ch, 40h
        mov     eax, 'cmpx'
        stosd
        mov     eax, 'chg '
        jmp     cop22.d1

;-----------------------------------------------------------------------------

cbsf:
cbsr:
        or      ch, 80h

;-----------------------------------------------------------------------------

cop22:
        disasm_set_modew
        or      ch, 40h
        mov     edx, eax
        mov     esi, 'lea '
        cmp     al, 8Dh
        jz      @f
        mov     esi, 'imul'
        cmp     al, 0xAF
        jz      @f
        mov     esi, 'bsf '
        cmp     al, 0BCh
        jz      @f
        mov     esi, 'bsr '
        cmp     al, 0BDh
        jz      @f
        mov     esi, 'mov '
        cmp     al, 88h
        jae     @f
        mov     esi, 'xchg'
        cmp     al, 86h
        jae     @f
        mov     esi, 'test'
        cmp     al, 84h
        jae     @f
        shr     al, 3
        and     eax, 7
        mov     esi, [disasm_op2cmds+eax*4]

    @@:
        xchg    eax, esi

    .d2:
        stosd
        mov     eax, '    '

    .d1:
        stosd
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        cmp     dl, 0x8D
        jz      @f
        cmp     dl, 0x86
        jz      @f
        cmp     dl, 0x87
        jz      @f
        cmp     dl, 0xBC
        jz      @f
        cmp     dl, 0xBD
        jz      @f
        test    dl, 2
        jz      .d0

    @@:
        call    disasm_write_reg
        mov     ax, ', '
        stosw
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .d0:
        push    eax
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        pop     eax
        call    disasm_write_reg
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cbound:
        mov     edx, eax
        mov     eax, 'boun'
        stosd
        mov     eax, 'd   '
        or      ch, 0xC0
        jmp     cop22.d1

;-----------------------------------------------------------------------------

cop23:
        disasm_set_modew
        xchg    eax, edx
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     eax, 3
        and     eax, 7
        mov     eax, [disasm_op2cmds+eax*4]

ctest:
        stosd
        mov     eax, '    '
        stosd
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        test    ch, 80h
        jz      .i8
        cmp     dl, 83h
        jz      .i8
        test    ch, 1
        jnz     .i16
        call    disasm_get_dword
        jmp     .ic

    .i8:
        xor     eax, eax
        call    disasm_get_byte
        cmp     dl, 83h
        jnz     .ic
        movsx   eax, al
        jmp     .ic

    .i16:
        xor     eax, eax
        call    disasm_get_word

    .ic:
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cmovcc:
        or      ch, 0C0h
        and     eax, 0xF
        mov     ax, [disasm_jcc_codes + eax*2]
        mov     dword [edi], 'cmov'
        add     edi, 4
        stosw
        mov     ax, '  '
        stosw
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     eax, 3
        and     eax, 7
        call    disasm_write_reg1632
        mov     ax, ', '
        stosw
        call    disasm_readrmop
        and     byte [edi], 0
        ret

; btx r/m,i8 = 0F BA 
cbtx1:
        or      ch, 80h
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        cmp     al, 4
        jb      cunk
        mov     eax, [btx1codes+eax*4-4*4]
        stosd
        mov     eax, '    '
        stosd
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        jmp     disasm_i8u

;-----------------------------------------------------------------------------

iglobal
btx1codes       dd      'bt  ','bts ','btr ','btc '
endg

;-----------------------------------------------------------------------------

; btx r/m,r = 0F 101xx011 (A3,AB,B3,BB)
cbtx2:
        shr     al, 3
        and     eax, 3
        mov     eax, [btx1codes+eax*4]
        stosd
        mov     eax, '    '
        stosd
        or      ch, 0xC0
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        push    eax
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        pop     eax
        call    disasm_write_reg1632
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

csetcc:
        and     eax, 0xF
        mov     ax, [disasm_jcc_codes + eax*2]
        mov     dword [edi], 'setc'
        add     edi, 3
        stosw
        mov     ax, '  '
        stosw
        stosb
        call    disasm_readrmop
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
disasm_jcc_codes dw 'o ','no','b ','ae','z ','nz','be','a ','s ','ns','p ','np','l ','ge','le','g '
endg

;-----------------------------------------------------------------------------

cjcc1:
cjmp2:
        cmp     al, 0xEB
        jz      .1
        and     eax, 0xF
        mov     ax, [disasm_jcc_codes + eax*2]
        jmp     .2

    .1:
        mov     ax, 'mp'

    .2:
        mov     byte [edi], 'j'
        inc     edi
        stosw
        mov     eax, '    '
        stosb
        stosd
        call    disasm_get_byte
        movsx   eax, al

disasm_rva:
        add     eax, [disasm_cur_pos]
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

ccall1:
cjmp1:
cjcc2:
        mov     edx, 'call'
        cmp     al, 0xE8
        jz      @f
        mov     edx, 'jmp '
        cmp     al, 0xE9
        jz      @f
        mov     edx, '    '
        and     eax, 0xF
        mov     dx, [disasm_jcc_codes+eax*2]
        shl     edx, 8
        mov     dl, 'j'

    @@:
        xchg    eax, edx
        stosd
        mov     eax, '    '
        stosd
        test    ch, 1
        jnz     @f
        call    disasm_get_dword
        jmp     disasm_rva

    @@:
        call    disasm_get_word
        add     eax, [disasm_cur_pos]
        and     eax, 0xFFFF
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

ccallf:
        mov     eax, 'call'
        stosd
        mov     eax, '    '
        stosd
        mov     al, 'd'
        test    ch, 1
        jnz     @f
        mov     al, 'p'

    @@:
        stosb
        mov     eax, 'word'
        stosd
        mov     al, ' '
        stosb
        test    ch, 1
        jnz     .1
        call    disasm_get_dword
        jmp     .2

    .1:
        xor     eax, eax
        call    disasm_get_word

    .2:
        push    eax
        xor     eax, eax
        call    disasm_get_word
        call    disasm_write_num
        mov     al, ':'
        stosb
        pop     eax
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
op11codes       dd      'test',0,'not ','neg ','mul ','imul','div ','idiv'
op12codes       dd      'inc ','dec ','call',0,'jmp ',0,'push',0
endg

;-----------------------------------------------------------------------------

cop1:
        disasm_set_modew
        xchg    eax, edx
        call    disasm_get_byte
        movzx   esi, al
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        cmp     dl, 0xFE
        jnz     @f
        cmp     al, 1
        jbe     @f

    .0:
        inc     [disasm_cur_pos]
        jmp     cunk

    @@:
        and     edx, 8
        add     eax, edx
        cmp     al, 11
        jz      .callfar
        cmp     al, 13
        jz      .jmpfar
        mov     eax, [op11codes+eax*4]
        test    eax, eax
        jz      .0
        cmp     eax, 'test'
        jz      ctest

    .2:
        stosd
        mov     eax, '    '
        stosd
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .callfar:
        mov     eax, 'call'

    .1:
        cmp     esi, 0xC0
        jae     .0
        stosd
        mov     eax, '    '
        stosd
        mov     eax, 'far '
        stosd
        mov     al, 'd'
        test    ch, 1
        jnz     @f
        mov     al, 'p'

    @@:
        stosb
        or      ch, 1
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .jmpfar:
        mov     eax, 'jmp '
        jmp     .1

;-----------------------------------------------------------------------------

cpop2:
        or      ch, 80h
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        test    al, 00111000b
        jnz     cunk
        mov     eax, 'pop '
        jmp     cop1.2

;-----------------------------------------------------------------------------

cloopnz:
        mov     eax, 'loop'
        stosd
        mov     eax, 'nz  '
        test    ch, 2
        jz      @f
        mov     ah, 'w'

    @@:
        jmp     cloop.cmn

cloopz:
        mov     eax, 'loop'
        stosd
        mov     eax, 'z   '
        test    ch, 2
        jz      @f
        mov     eax, 'zw  '

    @@:
        jmp     cloop.cmn

cjcxz:
cloop:
        cmp     al, 0xE2
        jz      .loop
        test    ch, 2
        jnz     .jcxz
        mov     eax, 'jecx'
        stosd
        mov     eax, 'z   '
        jmp     .cmn

    .jcxz:
        mov     eax, 'jcxz'
        stosd
        mov     eax, '    '
        jmp     .cmn

    .loop:
        mov     eax, 'loop'
        stosd
        mov     eax, '    '
        test    ch, 2
        jz      .cmn
        mov     al, 'w'

    .cmn:
        stosd
        call    disasm_get_byte
        movsx   eax, al
        add     eax, [disasm_cur_pos]
        test    ch, 1
        jz      @f
        and     eax, 0xFFFF

;    @@:
disasm_write_num_done:
    @@:
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

; imul r,r/m,i
cimul1:
        or      ch, 80h         ; 32bit operation
        xchg    eax, edx
        mov     eax, 'imul'
        stosd
        mov     eax, '    '
        stosd
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        call    disasm_write_reg1632
        mov     ax, ', '
        stosw
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        test    ch, 1
        jnz     .16
        cmp     dl, 0x69
        jz      .op32
        call    disasm_get_byte
        movsx   eax, al
        jmp     disasm_write_num_done

    .op32:
        call    disasm_get_dword
        jmp     disasm_write_num_done

    .16:
        cmp     dl, 0x69
        jz      .op16
        call    disasm_get_byte
        cbw
        jmp     disasm_write_num_done

    .op16:
        xor     eax, eax
        call    disasm_get_word
        jmp     disasm_write_num_done

;-----------------------------------------------------------------------------

cshld:
cshrd:
        mov     edx, 'shld'
        test    al, 8
        jz      @f
        mov     edx, 'shrd'

    @@:
        xchg    eax, edx
        stosd
        mov     eax, '    '
        stosd
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        push    eax
        or      ch, 80h
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        pop     eax
        call    disasm_write_reg1632
        mov     ax, ', '
        stosw
        test    dl, 1
        jz      disasm_i8u
        mov     ax, 'cl'
        stosw
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

ccbw:
        mov     eax, 'cbw '
        test    ch, 1
        jnz     @f
        mov     eax, 'cwde'

    @@:
        stosd
        and     byte [edi], 0
        ret

ccwd:
        mov     eax, 'cwd '
        test    ch, 1
        jnz     @b
        mov     eax, 'cdq '
        jmp     @b

;-----------------------------------------------------------------------------

ccmpxchg8b:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     cerr
        shr     al, 3
        and     al, 7
        cmp     al, 1
        jnz     cerr
        dec     [disasm_cur_pos]
        mov     eax, 'cmpx'
        stosd
        mov     eax, 'chg8'
        stosd
        mov     al, 'b'
        stosb
        mov     al, ' '
        stosb
        or      ch, 40h
        call    disasm_readrmop
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
fpuD8   dd      'add ','mul ','com ','comp','sub ','subr','div ','divr'
endg

;-----------------------------------------------------------------------------

cD8:
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        push    eax
        shr     al, 3
        and     eax, 7
        mov     byte [edi], 'f'
        inc     edi
        xchg    eax, edx
        mov     eax, [fpuD8+edx*4]
        stosd
        mov     ax, '  '
        stosw
        stosb
        pop     eax
        cmp     dl, 2
        jb      .1
        cmp     dl, 3
        jbe     .2

    .1:
        cmp     al, 0xC0
        jb      .2
        mov     eax, 'st0,'
        stosd
        mov     al, ' '
        stosb

    .2:
        or      ch, 80h or 20h
        and     ch, not 1
        call    disasm_readrmop
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
fpuD9_2:
        dq      'fchs    ','fabs    ',0,0,'ftst    ','fxam    ',0,0
        db      'fld1    fldl2t  fldl2e  fldpi   fldlg2  fldln2  fldz    '
        dq      0
        db      'f2xm1   fyl2x   fptan   fpatan  fxtract fprem1  fdecstp fincstp '
        db      'fprem   fyl2xp1 fsqrt   fsincos frndint fscale  fsin    fcos    '
fpuD9_fnop      db      'fnop    '
endg

;-----------------------------------------------------------------------------

cD9:
        call    disasm_get_byte
        sub     al, 0xC0
        jae     .l1
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        cmp     al, 7
        jnz     @f
        mov     eax, 'fnst'
        stosd
        mov     eax, 'cw  '
        jmp     .x1

    @@:
        cmp     al, 5
        jnz     @f
        mov     eax, 'fldc'
        stosd
        mov     eax, 'w   '

    .x1:
        stosd
        or      ch, 0C1h
        jmp     .cmn

    @@:
        mov     edx, 'fld '
        test    al, al
        jz      @f
        mov     edx, 'fst '
        cmp     al, 2
        jz      @f
        mov     edx, 'fstp'
        cmp     al, 3
        jnz     cunk

    @@:
        xchg    eax, edx
        stosd
        mov     eax, '    '
        stosd
        or      ch, 80h
        and     ch, not 1

    .cmn:
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .l1:
        cmp     al, 10h
        jae     .l2
        mov     edx, 'fld '
        cmp     al, 8
        jb      @f
        mov     edx, 'fxch'

    @@:
        xchg    eax, edx
        stosd
        mov     eax, '    '
        stosd
        xchg    eax, edx
        and     al, 7
        add     al, '0'
        shl     eax, 16
        mov     ax, 'st'
        stosd
        clc
        ret

    .l2:
        cmp     al, 0x10
        jnz     @f
        mov     esi, fpuD9_fnop
        jmp     .l3

    @@:
        sub     al, 0x20
        jb      cerr
        lea     esi, [fpuD9_2+eax*8]
        cmp     byte [esi], 0
        jz      cerr

    .l3:
        movsd
        movsd
        and     byte [edi-1], 0
        ret

;-----------------------------------------------------------------------------

cDA:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     cunk
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        mov     word [edi], 'fi'
        inc     edi
        inc     edi
        mov     eax, [fpuD8+eax*4]
        stosd
        mov     ax, '  '
        stosw
        or      ch, 80h
        and     ch, not 1       ; 32-bit operand
        call    disasm_readrmop
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
fpuDB   dd      'ild ',0,'ist ','istp',0,'ld  ',0,'stp '
endg

;-----------------------------------------------------------------------------

cDB:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     .1
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        xchg    eax, edx
        mov     eax, [fpuDB+edx*4]
        test    eax, eax
        jz      cerr
        mov     byte [edi], 'f'
        inc     edi
        stosd
        mov     ax, '  '
        stosw
        stosb
        or      ch, 80h
        and     ch, not 1       ; 32-bit operand
        cmp     dl, 4
        jb      @f
        or      ch, 20h
        and     ch, not 80h     ; 80-bit operand

    @@:
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .1:
        cmp     al, 0xE3
        jnz     cunk
        mov     eax, 'fnin'
        stosd
        mov     eax, 'it'
        stosd
        dec     edi
        ret             ; CF cleared

;-----------------------------------------------------------------------------

iglobal
fpuDC   dd      'add ','mul ',0,0,'subr','sub ','divr','div '
endg

;-----------------------------------------------------------------------------

cDC:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     .1
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        mov     byte [edi], 'f'
        inc     edi
        mov     eax, [fpuD8+eax*4]
        stosd
        mov     ax, '  '
        stosw
        stosb
        or      ch, 0A1h        ; qword
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .1:
        mov     dl, al
        shr     al, 3
        and     eax, 7
        mov     eax, [fpuDC+eax*4]
        test    eax, eax
        jz      cerr
        mov     byte [edi], 'f'
        inc     edi
        stosd
        mov     eax, '   s'
        stosd
        mov     al, 't'
        stosb
        and     edx, 7
        lea     eax, [edx+'0']
        stosb
        mov     eax, ', st'
        stosd
        mov     ax, '0'
        stosw
        ret     ; CF cleared

;-----------------------------------------------------------------------------

iglobal
fpuDD   dd      'fld ',0,'fst ','fstp',0,0,0,0
fpuDD_2 dq      'ffree   ',0,'fst     ','fstp    ','fucom   ','fucomp  ',0,0
endg

;-----------------------------------------------------------------------------

cDD:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     .1
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        xchg    eax, edx
        mov     eax, [fpuDD+edx*4]
        test    eax, eax
        jz      cunk
        stosd
        mov     eax, '    '
        stosd
        or      ch, 0A1h        ; qword operand
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .1:
        push    eax
        shr     al, 3
        and     eax, 7
        xchg    eax, edx
        mov     eax, dword [fpuDD_2+edx*8]
        test    eax, eax
        jz      cerr
        stosd
        mov     eax, dword [fpuDD_2+4+edx*8]
        stosd
        mov     ax, 'st'
        stosw
        pop     eax
        and     al, 7
        add     al, '0'
        stosb
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
fpuDE   dd      'add ','mul ',0,0,'subr','sub ','divr','div '
endg

;-----------------------------------------------------------------------------

cDE:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     .1
        dec     [disasm_cur_pos]
        mov     word [edi], 'fi'
        inc     edi
        inc     edi
        shr     al, 3
        and     eax, 7
        mov     eax, [fpuD8+eax*4]
        stosd
        mov     ax, '  '
        stosw
        or      ch, 81h         ; force 16-bit
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .1:
        push    eax
        shr     al, 3
        and     eax, 7
        xchg    eax, edx
        mov     eax, [fpuDE+edx*4]
        test    eax, eax
        jz      .fcompp
        mov     byte [edi], 'f'
        inc     edi
        stosd
        mov     al, 'p'
        cmp     byte [edi-1], ' '
        jnz     @f
        mov     byte [edi-1], al
        mov     al, ' '

    @@:
        stosb
        mov     eax, '  st'
        stosd
        pop     eax
        and     al, 7
        add     al, '0'
        stosb
        mov     ax, ', '
        stosw
        mov     eax, 'st0'
        stosd
        ret     ; CF cleared

    .fcompp:
        pop     eax
        cmp     al, 0xD9
        jnz     cerr
        mov     eax, 'fcom'
        stosd
        mov     ax, 'pp'
        stosw
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
fpuDF   dd      'ild ',0,'ist ','istp','bld ','ild ','bstp','istp'
endg

;-----------------------------------------------------------------------------

cDF:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     .1
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        xchg    eax, edx
        mov     eax, [fpuDF+edx*4]
        test    eax, eax
        jz      cerr
        mov     byte [edi], 'f'
        inc     edi
        stosd
        mov     ax, '  '
        stosw
        stosb
        or      ch, 81h         ; force 16-bit operand
        cmp     dl, 4
        jb      @f
        or      ch, 20h
        test    dl, 1
        jnz     @f
        or      ch, 40h

    @@:
        call    disasm_readrmop
        and     byte [edi], 0
        ret

    .1:
        cmp     al, 0xE0
        jnz     cunk
        mov     eax, 'fnst'
        stosd
        mov     eax, 'sw  '
        stosd
        mov     ax, 'ax'
        stosw
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cmovd1:
        mov     eax, 'movd'
        stosd
        mov     eax, '    '
        stosd
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        call    disasm_write_mmreg
        mov     ax, ', '
        stosw
        or      ch, 0C0h
        and     ch, not 1
        call    disasm_readrmop
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cmovd2:
        mov     eax, 'movd'
        stosd
        mov     eax, '    '
        stosd
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        push    eax ecx
        or      ch, 0C0h
        and     ch, not 1
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        pop     ecx eax
        call    disasm_write_mmreg
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

cmovq1:
        test    ch, 1
        jz      .mm
        mov     eax, 'movd'
        stosd
        mov     eax, 'qa  '
        stosd
        jmp     disasm_mmx1

    .mm:
        mov     eax, 'movq'
        stosd
        mov     eax, '    '
        stosd
        jmp     disasm_mmx1

;-----------------------------------------------------------------------------

cmovq2:
        test    ch, 1
        jz      .mm
        mov     eax, 'movd'
        stosd
        mov     eax, 'qa  '
        stosd
        jmp     disasm_mmx3

    .mm:
        mov     eax, 'movq'

disasm_mmx2:
        stosd
        mov     eax, '    '
        stosd

disasm_mmx3:
        or      ch, 50h
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        push    eax
        call    disasm_readrmop
        mov     ax, ', '
        stosw
        pop     eax
        shr     al, 3
        and     eax, 7
        call    disasm_write_mmreg
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
mmx_cmds:
        db      0x60,'unpcklbw'
        db      0x61,'unpcklwd'
        db      0x62,'unpckldq'
        db      0x63,'packsswb'
        db      0x64,'pcmpgtb '
        db      0x65,'pcmpgtw '
        db      0x66,'pcmpgtd '
        db      0x67,'packuswb'
        db      0x68,'unpckhbw'
        db      0x69,'unpckhwd'
        db      0x6A,'unpckhdq'
        db      0x6B,'packssdw'
        db      0x74,'pcmpeqb '
        db      0x75,'pcmpeqw '
        db      0x76,'pcmpeqd '
        db      0xD4,'paddq   '
        db      0xD5,'pmullw  '
        db      0xD8,'psubusb '
        db      0xD9,'psubusw '
        db      0xDA,'pminub  '
        db      0xDB,'pand    '
        db      0xDC,'paddusb '
        db      0xDD,'paddusw '
        db      0xDE,'pmaxub  '
        db      0xDF,'pandn   '
        db      0xE0,'pavgb   '
        db      0xE3,'pavgw   '
        db      0xE4,'pmulhuw '
        db      0xE5,'pmulhw  '
        db      0xE8,'psubsb  '
        db      0xE9,'psubsw  '
        db      0xEA,'pminsw  '
        db      0xEB,'por     '
        db      0xEC,'paddsb  '
        db      0xED,'paddsw  '
        db      0xEE,'pmaxsw  '
        db      0xEF,'pxor    '
        db      0xF4,'pmuludq '
        db      0xF5,'pmaddwd '
        db      0xF6,'psadbw  '
        db      0xF8,'psubb   '
        db      0xF9,'psubw   '
        db      0xFA,'psubd   '
        db      0xFB,'psubq   '
        db      0xFC,'paddb   '
        db      0xFD,'paddw   '
        db      0xFE,'paddd   '
endg

;-----------------------------------------------------------------------------

cpcmn:
        mov     esi, mmx_cmds

    @@:
        cmp     al, [esi]
        jz      @f
        add     esi, 9
        jmp     @b

    @@:
        inc     esi
        mov     al, 'p'
        cmp     byte [esi], al
        jz      @f
        stosb

    @@:
        movsd
        movsd
        cmp     byte [edi-1], ' '
        jz      @f
        mov     al, ' '
        stosb

;    @@:

disasm_mmx1:
    @@:
        or      ch, 50h
        call    disasm_get_byte
        dec     [disasm_cur_pos]
        shr     al, 3
        and     eax, 7
        call    disasm_write_mmreg
        mov     ax, ', '
        stosw
        call    disasm_readrmop
        cmp     word [disasm_string], 'cm'
        jz      .cmp
        and     byte [edi], 0
        ret

    .cmp:
        call    disasm_get_byte
        and     eax, 7
        mov     dx, 'eq'
        dec     eax
        js      @f
        mov     dx, 'lt'
        jz      @f
        mov     dh, 'e'
        dec     eax
        jnz     .no2

    @@:
        xchg    dx, word [disasm_string+3]
        mov     word [disasm_string+5], dx
        and     byte [edi], 0
        ret

    .no2:
        dec     eax
        jnz     @f
        add     edi, 2
        push    edi
        lea     esi, [edi-3]
        lea     ecx, [esi-(disasm_string+8)+2]
        std
        rep movsb
        cld
        mov     cx, word [esi-3]
        mov     dword [esi-3], 'unor'
        mov     byte [esi+1], 'd'
        mov     word [esi+2], cx
        pop     edi
        and     byte [edi+1], 0
        ret

    @@:
        mov     edx, 'neq'
        dec     eax
        jz      @f
        mov     edx, 'nlt'
        dec     eax
        jz      @f
        mov     edx, 'nle'
        dec     eax
        jz      @f
        mov     edx, 'ord'

    @@:
        push    edi
        lea     esi, [edi-1]
        lea     ecx, [esi-(disasm_string+8)+2]
        std
        rep movsb
        cld
        mov     cx, word [esi-3]
        mov     dword [esi-3], edx
        mov     word [esi], cx
        pop     edi
        and     byte [edi+1], 0
        ret

;-----------------------------------------------------------------------------

cpsrlw:
        mov     eax, 'psrl'
        jmp     @f

cpsraw:
        mov     eax, 'psra'
        jmp     @f

cpsllw:
        mov     eax, 'psll'

    @@:
        stosd
        mov     eax, 'w   '
        stosd
        jmp     disasm_mmx1

cpsrld:
        mov     eax, 'psrl'
        jmp     @f

cpsrad:
        mov     eax, 'psra'
        jmp     @f

cpslld:
        mov     eax, 'psll'

    @@:
        stosd
        mov     eax, 'd   '
        stosd
        jmp     disasm_mmx1

cpsrlq:
        mov     eax, 'psrl'
        jmp     @f

cpsllq:
        mov     eax, 'psll'

    @@:
        stosd
        mov     eax, 'q   '
        stosd
        jmp     disasm_mmx1

;-----------------------------------------------------------------------------

csse1:
iglobal
sse_cmds1:
        db      0x2F,4,'comi'
        db      0x54,3,'and'
        db      0x55,4,'andn'
        db      0x58,3,'add'
        db      0xC2,3,'cmp'
endg
        mov     esi, sse_cmds1+1

    .1:
    @@:
        movzx   edx, byte [esi]
        cmp     al, [esi-1]
        jz      @f
        lea     esi, [esi+edx+2]
        jmp     @b

    @@:
        push    ecx
        mov     ecx, edx
        inc     esi
        rep movsb
        pop     ecx
        mov     al, 's'
        cmp     byte [edi-1], 'i'
        jz      @f
        mov     al, 'p'

    @@:
        stosb
        mov     al, 'd'
        test    ch, 1
        jnz     @f
        mov     al, 's'

    @@:
        stosb
        push    ecx
        push    5
        pop     ecx
        sub     ecx, edx
        adc     ecx, 1
        mov     al, ' '
        rep stosb
        pop     ecx
        or      ch, 1           ; force XMM reg
        jmp     disasm_mmx1

;-----------------------------------------------------------------------------

csse2:
iglobal
sse_cmds2:
        db      0xD0,6,'addsub'
        db      0,0
endg
        test    ch, 1
        jz      cerr
        mov     esi, sse_cmds2+1
        jmp     csse1.1

cpshift:
        mov     dl, al
        mov     ax, 'ps'
        stosw
        call    disasm_get_byte
        push    eax
        and     al, 0xC0
        cmp     al, 0xC0
        jnz     .pop_cunk
        pop     eax
        push    eax
        shr     al, 3
        and     eax, 7
        cmp     al, 2
        jz      .rl
        cmp     al, 4
        jz      .ra
        cmp     al, 6
        jz      .ll

    .pop_cunk:
        pop     eax
        jmp     cunk

    .ll:
        mov     ax, 'll'
        jmp     @f

    .rl:
        mov     ax, 'rl'
        jmp     @f

    .ra:
        cmp     dl, 0x73
        jz      .pop_cunk
        mov     ax, 'ra'

    @@:
        stosw
        mov     al, 'w'
        cmp     dl, 0x71
        jz      @f
        mov     al, 'd'
        cmp     dl, 0x72
        jz      @f
        mov     al, 'q'

    @@:
        stosb
        mov     ax, '  '
        stosw
        stosb
        pop     eax
        and     eax, 7
        call    disasm_write_mmreg
        mov     ax, ', '
        stosw
        xor     eax, eax
        call    disasm_get_byte
        call    disasm_write_num
        and     byte [edi], 0
        ret

;-----------------------------------------------------------------------------

iglobal
grp15c1 dq      'fxsave  ','fxrstor ','ldmxcsr ','stmxcsr ',0,0,0,'clflush '
endg

;-----------------------------------------------------------------------------

cgrp15:
        call    disasm_get_byte
        cmp     al, 0xC0
        jae     cunk
        shr     al, 3
        and     eax, 7
        mov     edx, eax
        mov     eax, dword [grp15c1+eax*8]
        test    eax, eax
        jz      cerr
        dec     [disasm_cur_pos]
        stosd
        mov     eax, dword [grp15c1+4+edx*8]
        stosd
        or      ch, 40h
        call    disasm_readrmop
        and     byte [edi], 0
        ret

; vim: ft=fasm tabstop=4

