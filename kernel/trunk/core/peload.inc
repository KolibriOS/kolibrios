;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

include 'export.inc'

align 4

proc load_PE stdcall, file_name:dword
           locals
             image  dd ?
             entry  dd ?
             base   dd ?
           endl

        stdcall load_file, [file_name]
        test    eax, eax
        jz      .fail

        mov     [image], eax

        mov     edx, [eax+STRIPPED_PE_HEADER.SizeOfImage]
;        mov     cl, [eax+STRIPPED_PE_HEADER.Subsystem]
        cmp     word [eax], STRIPPED_PE_SIGNATURE
        jz      @f

        mov     edx, [eax+60]
;        mov     cl, [eax+5Ch+edx]
        mov     edx, [eax+80+edx]

@@:
        mov     [entry], 0
;        cmp     cl, 1
;        jnz     .cleanup
        stdcall kernel_alloc, edx
        test    eax, eax
        jz      .cleanup

        mov     [base], eax
        DEBUGF 1,'K : driver %s mapped to %x\n',[file_name],[base]

        push    ebx ebp
        mov     ebx, [image]
        mov     ebp, eax
        call    map_PE
        pop     ebp ebx

        mov     [entry], eax
        test    eax, eax
        jnz     .cleanup

        stdcall kernel_free, [base]
.cleanup:
        stdcall kernel_free, [image]
        mov     eax, [entry]
        ret
.fail:
        xor     eax, eax
        ret
endp

map_PE:                    ;ebp=base:dword, ebx=image:dword
        push    edi
        push    esi
        sub     esp, .locals_size
virtual at esp
.numsections    dd      ?
.import_names   dd      ?
.import_targets dd      ?
.peheader       dd      ?
.bad_import     dd      ?
.import_idx     dd      ?
.import_descr   dd      ?
.relocs_rva     dd      ?
.relocs_size    dd      ?
.section_header_size dd ?
.AddressOfEntryPoint    dd ?
.ImageBase      dd      ?
.locals_size = $ - esp
end virtual
        cmp     word [ebx], STRIPPED_PE_SIGNATURE
        jz      .stripped

        mov     edx, ebx
        add     edx, [ebx+60]
        movzx   eax, word [edx+6]
        mov     [.numsections], eax
        mov     eax, [edx+40]
        mov     [.AddressOfEntryPoint], eax
        mov     eax, [edx+52]
        mov     [.ImageBase], eax
        mov     ecx, [edx+84]
        mov     [.section_header_size], 40
        mov     eax, [edx+128]
        mov     [.import_descr], eax
        mov     eax, [edx+160]
        mov     [.relocs_rva], eax
        mov     eax, [edx+164]
        mov     [.relocs_size], eax
        add     edx, 256

        jmp     .common
.stripped:
        mov     eax, [ebx+STRIPPED_PE_HEADER.AddressOfEntryPoint]
        mov     [.AddressOfEntryPoint], eax
        mov     eax, [ebx+STRIPPED_PE_HEADER.ImageBase]
        mov     [.ImageBase], eax
        movzx   eax, [ebx+STRIPPED_PE_HEADER.NumberOfSections]
        mov     [.numsections], eax
        movzx   ecx, [ebx+STRIPPED_PE_HEADER.NumberOfRvaAndSizes]
        xor     eax, eax
        mov     [.relocs_rva], eax
        mov     [.relocs_size], eax
        test    ecx, ecx
        jz      @f
        mov     eax, [ebx+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_IMPORT*8]
@@:
        mov     [.import_descr], eax
        cmp     ecx, SPE_DIRECTORY_BASERELOC
        jbe     @f
        mov     eax, [ebx+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_BASERELOC*8]
        mov     [.relocs_rva], eax
        mov     eax, [ebx+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_BASERELOC*8+4]
        mov     [.relocs_size], eax
@@:
        mov     [.section_header_size], 28
        lea     edx, [ebx+ecx*8+sizeof.STRIPPED_PE_HEADER+8]
        mov     ecx, [ebx+STRIPPED_PE_HEADER.SizeOfHeaders]

.common:
        mov     esi, ebx
        mov     edi, ebp
        shr     ecx, 2
        rep movsd

        cmp     [.numsections], 0
        jz      .nosections
.copy_sections:
        mov     eax, [edx+8]
        test    eax, eax
        je      .no_section_data
        mov     esi, ebx
        mov     edi, ebp
        add     esi, [edx+12]
        mov     ecx, eax
        add     edi, [edx+4]

        add     ecx, 3
        shr     ecx, 2
        rep movsd

.no_section_data:
        mov     ecx, [edx]
        cmp     ecx, eax
        jbe     .no_section_fill
        sub     ecx, eax
        add     eax, [edx+4]
        lea     edi, [eax+ebp]

        xor     eax, eax
        rep stosb

.no_section_fill:
        add     edx, [.section_header_size]
        dec     [.numsections]
        jnz     .copy_sections
.nosections:
        cmp     [.relocs_size], 0
        je      .no_relocations
        mov     esi, ebp
        mov     ecx, ebp
        sub     esi, [.ImageBase]
        add     ecx, [.relocs_rva]
.relocs_block:
        mov     edi, [ecx]
        add     edi, ebp
        mov     ebx, [ecx+4]
        add     ecx, 8
        sub     [.relocs_size], ebx
        sub     ebx, 8
        shr     ebx, 1
        jz      .relocs_next_block
.one_reloc:
        movzx   eax, word [ecx]
        add     ecx, 2
        mov     edx, eax
        shr     eax, 12
        and     edx, 4095
        cmp     eax, 3
        jne     @f
        add     [edx+edi], esi
@@:
        dec     ebx
        jnz     .one_reloc
.relocs_next_block:
        cmp     [.relocs_size], 0
        jg      .relocs_block
.no_relocations:
        cmp     [.import_descr], 0
        je      .no_imports
        add     [.import_descr], ebp
        mov     [.bad_import], 0
.import_block:
        mov     ecx, [.import_descr]
        cmp     dword [ecx+4], 0
        jne     @f
        cmp     dword [ecx+12], 0
        je      .done_imports
@@:
        mov     edx, dword [ecx]
        mov     ecx, dword [ecx+16]
        test    edx, edx
        jnz     @f
        mov     edx, ecx
@@:
        mov     [.import_idx], 0
        add     ecx, ebp
        add     edx, ebp
        mov     [.import_names], edx
        mov     [.import_targets], ecx
.import_func:
        mov     esi, [.import_idx]
        mov     edi, [.import_names]
        mov     eax, [edi+esi*4]
        test    eax, eax
        je      .next_import_block
        js      .next_import_block
        lea     edi, [ebp+eax]
        mov     eax, [.import_targets]
        mov     dword [eax+esi*4], 0
        lea     esi, [edi+2]
        movzx   ebx, word [edi]
        push    32
        mov     ecx, [__exports+32]
        mov     eax, [ecx+OS_BASE+ebx*4]
        add     eax, OS_BASE
        push    eax
        push    esi
        call    strncmp
        test    eax, eax
        jz      .import_func_found
        xor     ebx, ebx
.import_func_candidate:
        push    32
        mov     ecx, [__exports+32]
        mov     eax, [ecx+OS_BASE+ebx*4]
        add     eax, OS_BASE
        push    eax
        push    esi
        call    strncmp
        test    eax, eax
        je      .import_func_found
        inc     ebx
        cmp     ebx, [__exports+24]
        jb      .import_func_candidate

        mov     esi, msg_unresolved
        call    sys_msg_board_str
        lea     esi, [edi+2]
        call    sys_msg_board_str
        mov     esi, msg_CR
        call    sys_msg_board_str

        mov     [.bad_import], 1
        jmp     .next_import_func
.import_func_found:
        mov     esi, [__exports+28]
        mov     edx, [.import_idx]
        mov     ecx, [.import_targets]
        mov     eax, [esi+OS_BASE+ebx*4]
        add     eax, OS_BASE
        mov     [ecx+edx*4], eax
.next_import_func:
        inc     [.import_idx]
        jmp     .import_func
.next_import_block:
        add     [.import_descr], 20
        jmp     .import_block
.done_imports:
        xor     eax, eax
        cmp     [.bad_import], 0
        jne     @f
.no_imports:
        mov     eax, ebp
        add     eax, [.AddressOfEntryPoint]
@@:
        add     esp, .locals_size
        pop     esi
        pop     edi
        ret
