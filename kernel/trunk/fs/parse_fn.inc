;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;-------------------------------------------------------------------------
;
;  File path partial substitution (according to configuration)
;
;
;     SPraid
;
;-------------------------------------------------------------------------

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
