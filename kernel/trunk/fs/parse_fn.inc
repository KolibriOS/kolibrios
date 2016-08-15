;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$


iglobal
; pointer to memory for path replace table,
; size of one record is 128 bytes: 64 bytes for search pattern + 64 bytes for replace string

; start with one entry: sys -> <sysdir>
full_file_name_table dd sysdir_name
.size           dd      1

tmp_file_name_size dd   1
endg

uglobal
; Parser_params will initialize: sysdir_name = "sys", sysdir_path = <sysdir>
sysdir_name     rb      64
sysdir_path     rb      64
sysdir_name1    rb      64
sysdir_path1    rb      64

; for example:
;dir_name1      db 'KolibriOS',0
;               rb 64-8
;dir_path1      db 'HD0/1',0
;               rb 64-6
endg

uglobal
tmp_file_name_table dd  ?
endg

; use bx_from_load and init system directory /sys
proc Parser_params
locals
  buff db 4 dup(?)              ; for test cd
endl
        mov     eax, [OS_BASE+0x10000+bx_from_load]
        mov     ecx, sysdir_path
        mov     [ecx-64], dword 'sys'
        cmp     al, 'r'; if ram disk
        jnz     @f
        mov     [ecx], dword 'RD/?'
        mov     [ecx+3], byte ah
        mov     [ecx+4], byte 0
        ret
@@:
        cmp     al, 'm'; if ram disk
        jnz     @f
        mov     [ecx], dword 'CD?/'; if cd disk {m}
        mov     [ecx+4], byte '1'
        mov     [ecx+5], dword '/KOL'
        mov     [ecx+9], dword 'IBRI'
        mov     [ecx+13], byte 0
.next_cd:
        mov     [ecx+2], byte ah
        inc     ah
        cmp     ah, '5'
        je      .not_found_cd
        lea     edx, [buff]
        pushad
        stdcall read_file, read_firstapp, edx, 0, 4
        popad
        cmp     [edx], dword 'MENU'
        jne     .next_cd
        jmp     .ok

@@:
        sub     al, 49
        mov     [ecx], dword 'HD?/'; if hard disk
        mov     [ecx+2], byte al
        mov     [ecx+4], byte ah
        mov     [ecx+5], dword '/KOL'
        mov     [ecx+9], dword 'IBRI'
        mov     [ecx+13], byte 0
.ok:
.not_found_cd:
        ret
endp

proc load_file_parse_table
        stdcall kernel_alloc, 0x1000
        mov     [tmp_file_name_table], eax
        mov     edi, eax
        mov     esi, sysdir_name
        mov     ecx, 128/4
        rep movsd

        invoke  ini.enum_keys, conf_fname, conf_path_sect, get_every_key

        mov     eax, [tmp_file_name_table]
        mov     [full_file_name_table], eax
        mov     eax, [tmp_file_name_size]
        mov     [full_file_name_table.size], eax
        ret
endp

uglobal
def_val_1 db 0
endg

proc get_every_key stdcall, f_name, sec_name, key_name
        mov     esi, [key_name]
        mov     ecx, esi
        cmp     byte [esi], '/'
        jnz     @f
        inc     esi
@@:
        mov     edi, [tmp_file_name_size]
        shl     edi, 7
        cmp     edi, 0x1000
        jae     .stop_parse
        add     edi, [tmp_file_name_table]
        lea     ebx, [edi+64]
@@:
        cmp     edi, ebx
        jae     .skip_this_key
        lodsb
        test    al, al
        jz      @f
        or      al, 20h
        stosb
        jmp     @b
@@:
        stosb

        invoke  ini.get_str, [f_name], [sec_name], ecx, ebx, 64, def_val_1

        cmp     byte [ebx], '/'
        jnz     @f
        lea     esi, [ebx+1]
        mov     edi, ebx
        mov     ecx, 63
        rep movsb
@@:
        push    ebp
        mov     ebp, [tmp_file_name_table]
        mov     ecx, [tmp_file_name_size]
        jecxz   .noreplace
        mov     eax, ecx
        dec     eax
        shl     eax, 7
        add     ebp, eax
.replace_loop:
        mov     edi, ebx
        mov     esi, ebp
@@:
        lodsb
        test    al, al
        jz      .doreplace
        mov     dl, [edi]
        inc     edi
        test    dl, dl
        jz      .replace_loop_cont
        or      dl, 20h
        cmp     al, dl
        jz      @b
        jmp     .replace_loop_cont
.doreplace:
        cmp     byte [edi], 0
        jz      @f
        cmp     byte [edi], '/'
        jnz     .replace_loop_cont
@@:
        lea     esi, [ebp+64]
        call    .replace
        jc      .skip_this_key2
.replace_loop_cont:
        sub     ebp, 128
        loop    .replace_loop
.noreplace:
        pop     ebp

        inc     [tmp_file_name_size]
.skip_this_key:
        xor     eax, eax
        inc     eax
        ret
.skip_this_key2:
        pop     ebp
        jmp     .skip_this_key
.stop_parse:
        xor     eax, eax
        ret
endp

proc get_every_key.replace
; in: ebx->destination, esi->first part of name, edi->second part of name
; maximum length is 64 bytes
; out: CF=1 <=> overflow
; 1) allocate temporary buffer in stack
        sub     esp, 64
; 2) save second part of name to temporary buffer
        push    esi
        lea     esi, [esp+4]    ; esi->tmp buffer
        xchg    esi, edi        ; edi->tmp buffer, esi->source
@@:
        lodsb
        stosb
        test    al, al
        jnz     @b
; 3) copy first part of name to destination
        pop     esi
        mov     edi, ebx
@@:
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b
@@:
; 4) restore second part of name from temporary buffer to destination
; (may cause overflow)
        lea     edx, [ebx+64]   ; limit of destination
        mov     esi, esp
@@:
        cmp     edi, edx
        jae     .overflow
        lodsb
        stosb
        test    al, al
        jnz     @b
; all is OK
        add     esp, 64         ; CF is cleared
        ret
.overflow:
; name is too long
        add     esp, 64
        stc
        ret
endp

cp866toUpper:
; convert cp866 character in al to uppercase
        cmp     al, 'a'
        jb      .ret
        cmp     al, 'z'
        jbe     @f
        cmp     al, 0xA0
        jb      .ret
        cmp     al, 0xB0
        jb      @f
        cmp     al, 0xE0
        jb      .ret
        cmp     al, 0xF0
        jb      .rus
        cmp     al, 0xF7
        ja      .ret
        and     eax, -2
.ret:
        ret

@@:
        sub     eax, 32
        ret

.rus:
        sub     eax, 0xE0-0x90
        ret

utf16toUpper:
; convert UTF-16 character in ax to uppercase
        cmp     ax, 'a'
        jb      .ret
        cmp     ax, 'z'
        jbe     @f
        cmp     ax, 430h
        jb      .ret
        cmp     ax, 450h
        jb      @f
        cmp     ax, 460h
        jnc     .ret
        sub     eax, 80
.ret:
        ret

@@:
        sub     eax, 32
        ret

uni2ansi_char:
; convert UNICODE character in ax to ANSI character in al using cp866 encoding
        cmp     ax, 0x80
        jb      .ret
        cmp     ax, 0xB6
        jz      .B6
        cmp     ax, 0x400
        jb      .unk
        cmp     ax, 0x410
        jb      @f
        cmp     ax, 0x440
        jb      .rus1
        cmp     ax, 0x450
        jb      .rus2
        cmp     ax, 0x460
        jb      @f
.unk:
        mov     al, '_'
.ret:
        ret

.B6:
        mov     al, 20
        ret

.rus1:  ; 0x410-0x43F -> 0x80-0xAF
        add     al, 0x70
        ret

.rus2:  ; 0x440-0x44F -> 0xE0-0xEF
        add     al, 0xA0
        ret

@@:
        push    ecx edi
        mov     ecx, 8
        mov     edi, .table
        repnz scasb
        mov     ah, cl
        pop     edi ecx
        jnz     .unk
        mov     al, 0xF7
        sub     al, ah
        ret

.table  db  1, 51h, 4, 54h, 7, 57h, 0Eh, 5Eh

ansi2uni_char:
; convert ANSI character in al to UNICODE character in ax, using cp866 encoding
        movzx   eax, al
        cmp     al, 0x80
        jb      @f      ; 0x00-0x7F - trivial map
        cmp     al, 0xB0
        jb      .rus    ; 0x80-0xAF -> 0x410-0x43F
        cmp     al, 0xE0
        jb      .unk
        cmp     al, 0xF0
        jb      .rus2   ; 0xE0-0xEF -> 0x440-0x44F
        cmp     al, 0xF8
        jnc     .unk
        mov     al, [eax+uni2ansi_char.table-0xF0]
        add     ax, 400h
        ret

@@:
        cmp     al, 20
        jnz     .ret
        mov     al, 0xB6
.ret:
        ret

.rus:
        add     ax, 0x410-0x80
        ret

.rus2:
        add     ax, 0x440-0xE0
        ret

.unk:
        mov     al, '_'
        ret

cp866toUTF8_string:
;   in:
; esi -> cp866 string (could be zero terminated)
; edi -> buffer for UTF-8 string
; ecx = buffer size (signed)
        lodsb
        call    ansi2uni_char
        push    eax
        call    UTF16to8
        pop     eax
        js      @f
        test    eax, eax
        jnz     cp866toUTF8_string
@@:
        ret

; SF=1 -> counter
; ZF=1 -> zero char

UTF16to8_string:
;   in:
; esi -> UTF-16 string (could be zero terminated)
; edi -> buffer for UTF-8 string
; ecx = buffer size (signed)
        xor     eax, eax
@@:
        lodsw
        push    eax
        call    UTF16to8
        pop     eax
        js      @f
        test    eax, eax
        jnz     @b
@@:
        ret

UTF16to8:
;   in:
; eax = UTF-16 char
; edi -> buffer for UTF-8 char (increasing)
; ecx = byte counter (decreasing)
        dec     ecx
        js      .ret
        cmp     eax, 80h
        jnc     @f
        stosb
        test    eax, eax    ; SF=0
.ret:
        ret

@@:
        dec     ecx
        js      .ret
        cmp     eax, 800h
        jnc     @f
        shl     eax, 2
        shr     al, 2
        or      eax, 1100000010000000b
        xchg    al, ah
        stosw
        ret

@@:
        dec     ecx
        js      .ret
        shl     eax, 4
        shr     ax, 2
        shr     al, 2
        or      eax, 111000001000000010000000b
        bswap   eax
        shr     eax, 8
        stosb
        shr     eax, 8
        stosw
        ret

utf8to16:
; in: esi -> UTF-8 char (increasing)
; out: ax = UTF-16 char
        lodsb
        test    al, al
        jns     .got
        shl     al, 2
        jnc     utf8to16
@@:
        shl     ax, 8
        lodsb
        test    al, al
        jns     .got
        shl     al, 2
        jc      @b
        shr     ah, 2
        shl     ax, 3
        jnc     @f
        shl     eax, 3
        lodsb
        test    al, al
        jns     .got
        shl     al, 2
        jc      @b
        shr     eax, 2
        ret

@@:
        shr     ax, 5
        ret

.got:
        xor     ah, ah
        ret

strlen:
; in: esi -> source
; out: ecx = length
        push    edi eax
        or      ecx, -1
        mov     edi, esi
        xor     eax, eax
        repnz scasb
        inc     ecx
        not     ecx
        pop     eax edi
        ret
