;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; External kernel dependencies (libraries) loading.
; The code currently does not work, requires correcting dll.inc.

$Revision$

if 0
iglobal
tmp_file_name_size  dd  1
endg

uglobal
tmp_file_name_table dd  ?
s_libname   rb  64
def_val_1   db  ?
endg

macro library [name,fname]
{
  forward
    dd __#name#_library_table__,__#name#_library_name__
  common
    dd 0
  forward
    __#name#_library_name__ db fname,0
}

macro import lname,[name,sname]
{
  common
    align 4
    __#lname#_library_table__:
  forward
    name dd __#name#_import_name__
  common
    dd 0
  forward
    __#name#_import_name__ db sname,0
}

macro export [name,sname]
{
align 4
  forward
    dd __#name#_export_name__,name
  common
    dd 0
  forward
    __#name#_export_name__ db sname,0
}



align 4            ; loading library (use kernel functions)
proc load_k_library stdcall, file_name:dword
           locals
             coff      dd ?
             sym       dd ?
             strings   dd ?
             img_size  dd ?
             img_base  dd ?
             exports   dd ?
           endl

        cli

        stdcall load_file, [file_name]
        test    eax, eax
        jz      .fail

        mov     [coff], eax
        movzx   ecx, [eax+CFH.nSections]
        xor     ebx, ebx

        lea     edx, [eax+20]
@@:
        add     ebx, [edx+CFS.SizeOfRawData]
        add     ebx, 15
        and     ebx, not 15
        add     edx, COFF_SECTION_SIZE
        dec     ecx
        jnz     @B
        mov     [img_size], ebx

        stdcall kernel_alloc, [img_size]

        test    eax, eax
        jz      .fail
        mov     [img_base], eax

        mov     edx, [coff]
        movzx   ebx, [edx+CFH.nSections]
        mov     edi, [img_base]
        lea     eax, [edx+20]
@@:
        mov     [eax+CFS.VirtualAddress], edi
        mov     esi, [eax+CFS.PtrRawData]
        test    esi, esi
        jnz     .copy
        add     edi, [eax+CFS.SizeOfRawData]
        jmp     .next
.copy:
        add     esi, edx
        mov     ecx, [eax+CFS.SizeOfRawData]
        cld
        rep movsb
.next:
        add     edi, 15
        and     edi, not 15
        add     eax, COFF_SECTION_SIZE
        dec     ebx
        jnz     @B

        mov     ebx, [edx+CFH.pSymTable]
        add     ebx, edx
        mov     [sym], ebx
        mov     ecx, [edx+CFH.nSymbols]
        add     ecx, ecx
        lea     ecx, [ecx+ecx*8];ecx*=18 = nSymbols*CSYM_SIZE
        add     ecx, [sym]
        mov     [strings], ecx

        lea     eax, [edx+20]

        stdcall fix_coff_symbols, eax, [sym], [edx+CFH.nSymbols], \
                [strings], dword 0
        test    eax, eax
        jnz     @F

@@:
        mov     edx, [coff]
        movzx   ebx, [edx+CFH.nSections]
        mov     edi, 0
        lea     eax, [edx+20]
@@:
        add     [eax+CFS.VirtualAddress], edi ;patch user space offset
        add     eax, COFF_SECTION_SIZE
        dec     ebx
        jnz     @B

        add     edx, 20
        stdcall fix_coff_relocs, [coff], edx, [sym]

        mov     ebx, [coff]
        stdcall get_coff_sym, [sym], [ebx+CFH.nSymbols], szEXPORTS
        mov     [exports], eax

        stdcall kernel_free, [coff]

        mov     eax, [exports]
        ret
.fail:
        xor     eax, eax
        ret
endp


proc dll.Load, import_table:dword
        mov     esi, [import_table]
  .next_lib:
        mov     edx, [esi]
        or      edx, edx
        jz      .exit
        push    esi

        mov     edi, s_libname

        mov     al, '/'
        stosb
        mov     esi, sysdir_path
            @@:
        lodsb
        stosb
        or      al, al
        jnz     @b
        dec     edi
        mov     [edi], dword '/lib'
        mov     [edi+4], byte '/'
        add     edi, 5
        pop     esi
        push    esi
        mov     esi, [esi+4]
            @@:
        lodsb
        stosb
        or      al, al
        jnz     @b

        pushad
        stdcall load_k_library, s_libname
        mov     [esp+28], eax
        popad
        or      eax, eax
        jz      .fail
        stdcall dll.Link, eax, edx
        stdcall dll.Init, [eax+4]
        pop     esi
        add     esi, 8
        jmp     .next_lib
  .exit:
        xor     eax, eax
        ret
  .fail:
        add     esp, 4
        xor     eax, eax
        inc     eax
        ret
endp

proc dll.Link, exp:dword,imp:dword
        push    eax
        mov     esi, [imp]
        test    esi, esi
        jz      .done
  .next:
        lodsd
        test    eax, eax
        jz      .done
        stdcall dll.GetProcAddress, [exp], eax
        or      eax, eax
        jz      @f
        mov     [esi-4], eax
        jmp     .next
            @@:
        mov     dword[esp], 0
  .done:
        pop     eax
        ret
endp

proc dll.Init, dllentry:dword
        pushad
        mov     eax, mem.Alloc
        mov     ebx, mem.Free
        mov     ecx, mem.ReAlloc
        mov     edx, dll.Load
        stdcall [dllentry]
        popad
        ret
endp

proc dll.GetProcAddress, exp:dword,sz_name:dword
        mov     edx, [exp]
  .next:
        test    edx, edx
        jz      .end
        stdcall strncmp, [edx], [sz_name], dword -1
        test    eax, eax
        jz      .ok
        add     edx, 8
        jmp     .next
  .ok:
        mov     eax, [edx+4]
  .end:
        ret
endp

;-----------------------------------------------------------------------------
proc mem.Alloc size ;/////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
        push    ebx ecx
;       mov     eax,[size]
;       lea     ecx,[eax+4+4095]
;       and     ecx,not 4095
;       stdcall kernel_alloc, ecx
;       add     ecx,-4
;       mov     [eax],ecx
;       add     eax,4

        stdcall kernel_alloc, [size]

        pop     ecx ebx
        ret
endp

;-----------------------------------------------------------------------------
proc mem.ReAlloc mptr,size;///////////////////////////////////////////////////
;-----------------------------------------------------------------------------
        push    ebx ecx esi edi eax
        mov     eax, [mptr]
        mov     ebx, [size]
        or      eax, eax
        jz      @f
        lea     ecx, [ebx+4+4095]
        and     ecx, not 4095
        add     ecx, -4
        cmp     ecx, [eax-4]
        je      .exit
    @@:
        mov     eax, ebx
        call    mem.Alloc
        xchg    eax, [esp]
        or      eax, eax
        jz      .exit
        mov     esi, eax
        xchg    eax, [esp]
        mov     edi, eax
        mov     ecx, [esi-4]
        cmp     ecx, [edi-4]
        jbe     @f
        mov     ecx, [edi-4]
    @@:
        add     ecx, 3
        shr     ecx, 2
        cld
        rep movsd
        xchg    eax, [esp]
        call    mem.Free
  .exit:
        pop     eax edi esi ecx ebx
        ret
endp

;-----------------------------------------------------------------------------
proc mem.Free mptr ;//////////////////////////////////////////////////////////
;-----------------------------------------------------------------------------
;       mov     eax,[mptr]
;       or      eax,eax
;       jz      @f
;       push    ebx ecx
;       lea     ecx,[eax-4]
;       stdcall kernel_free, ecx
;       pop     ecx ebx
;    @@: ret
        stdcall kernel_free, [mptr]
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

.stop_parse:
        xor     eax, eax
        ret

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
endp

proc get_every_key.replace
; in: ebx->destination, esi->first part of name, edi->second part of name
; maximum length is 64 bytes
; out: CF=1 <=> overflow
        sub     esp, 64 ; allocate temporary buffer in stack
        push    esi
        lea     esi, [esp+4]    ; esi->tmp buffer
        xchg    esi, edi        ; edi->tmp buffer, esi->source
@@: ; save second part of name to temporary buffer
        lodsb
        stosb
        test    al, al
        jnz     @b
        pop     esi
        mov     edi, ebx
@@: ; copy first part of name to destination
        lodsb
        test    al, al
        jz      @f
        stosb
        jmp     @b

@@: ; restore second part of name from temporary buffer to destination
        lea     edx, [ebx+64]   ; limit of destination
        mov     esi, esp
@@:
        cmp     edi, edx
        jae     .overflow
        lodsb
        stosb
        test    al, al
        jnz     @b
        add     esp, 64 ; CF is cleared
        ret

.overflow:  ; name is too long
        add     esp, 64
        stc
        ret
endp
end if
