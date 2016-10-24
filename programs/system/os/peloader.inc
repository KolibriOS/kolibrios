; Check whether PE module has been loaded at preferred address.
; If not, relocate the module.
;
; in: esi = PE base address
; in: [esp+4] = module name for debug print
; out: CF=1 - fail
proc fixup_pe_relocations uses edi ebp
; 1. Fetch some data from PE header or stripped PE header.
; We need:
; * ImageBase - preferred address, compare with esi = actual load address;
;   ebp will keep the delta
; * RVA and size of fixups directory
; * flag IMAGE_FILE_RELOCS_STRIPPED from Characteristics
; If the actual address equals the preferred address, do nothing.
; If fixups directory is present, proceed to 2.
; If there is no fixups directory, there are two options:
; * either the directory has not been created
; * or the module has no fixups (data-only module, for example).
; In the first case, IMAGE_FILE_RELOCS_STRIPPED is set, and this is an error.
; In the second case, IMAGE_FILE_RELOCS_STRIPPED is not set; do nothing.
        mov     ebp, esi
        cmp     word [esi], 'MZ'
        jz      .parse_mz
        sub     ebp, [esi+STRIPPED_PE_HEADER.ImageBase]
        jnz     @f
.nothing:
        ret
@@:
        mov     dl, byte [esi+STRIPPED_PE_HEADER.Characteristics]
        lea     eax, [esi+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_BASERELOC*sizeof.IMAGE_DATA_DIRECTORY]
        cmp     [esi+STRIPPED_PE_HEADER.NumberOfRvaAndSizes], SPE_DIRECTORY_BASERELOC
        ja      .common
.norelocs:
        test    dl, IMAGE_FILE_RELOCS_STRIPPED
        jz      .nothing
        stc
        ret
.parse_mz:
        mov     eax, [esi+3Ch]
        add     eax, esi
        sub     ebp, [eax+IMAGE_NT_HEADERS.OptionalHeader.ImageBase]
        jz      .nothing
        mov     dl, byte [esi+IMAGE_NT_HEADERS.FileHeader.Characteristics]
        cmp     [eax+IMAGE_NT_HEADERS.OptionalHeader.NumberOfDirectories], IMAGE_DIRECTORY_ENTRY_BASERELOC
        jbe     .norelocs
        add     eax, IMAGE_NT_HEADERS.OptionalHeader.DataDirectory+IMAGE_DIRECTORY_ENTRY_BASERELOC*sizeof.IMAGE_DATA_DIRECTORY
.common:
        mov     edi, [eax+IMAGE_DATA_DIRECTORY.VirtualAddress]
        push    [eax+IMAGE_DATA_DIRECTORY.isize]
virtual at esp
.sizeleft       dd      ?
end virtual
        add     edi, esi
        cmp     [.sizeleft], 0
        jz      .norelocs
; 2. We need to relocate and we have the relocation table.
; esi = PE base address
; edi = pointer to current data of relocation table
; 2a. Relocation table is organized into blocks describing every page.
; End of table is defined from table size fetched from the header.
; Loop 2b-2g over all blocks until no more data is left.
.pageloop:
; 2b. Load the header of the current block: address and size.
; Advance total size.
; Size in the block includes size of the header, subtract it.
; If there is no data in this block, go to 2g.
        mov     edx, [edi+IMAGE_BASE_RELOCATION.VirtualAddress]
        mov     ecx, [edi+IMAGE_BASE_RELOCATION.SizeOfBlock]
        sub     [.sizeleft], ecx
        add     edi, sizeof.IMAGE_BASE_RELOCATION
        sub     ecx, sizeof.IMAGE_BASE_RELOCATION
        jbe     .pagedone
; 2c. We are going to modify data, so mprotect the current page to be writable.
; Save the old protection, we will restore it after the block is processed.
; Ignore any error.
PROT_READ = 1
PROT_WRITE = 2
PROT_EXEC = 4
        push    esi ecx
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, PROT_READ+PROT_WRITE
        add     edx, esi
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
        pop     ecx
        push    eax
; 2d. Block data is an array of word values. Repeat 2e for every of those.
.relocloop:
        sub     ecx, 2
        jb      .relocdone
; 2e. Every value consists of a 4-bit type and 12-bit offset in the page.
; x86 uses two types: 0 = no data (used for padding), 3 = 32-bit relative.
        movzx   eax, word [edi]
        add     edi, 2
        mov     ebx, eax
        and     ebx, 0xFFF
        shr     eax, 12
        jz      .relocloop
        cmp     al, IMAGE_REL_BASED_HIGHLOW
        jnz     .badreloc
        add     [edx+ebx], ebp
        jmp     .relocloop
.relocdone:
; 2f. Restore memory protection changed in 2c.
        pop     ecx
        cmp     ecx, -1
        jz      @f
        mov     eax, 68
        mov     ebx, 30
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
@@:
        pop     esi
.pagedone:
        cmp     [.sizeleft], 0
        jnz     .pageloop
        pop     eax ; pop .sizeleft
; 3. For performance reasons, relocation should be avoided
; by choosing an appropriate preferred address.
; If we have actually relocated something, yell to the debug board,
; so the programmer can notice that.
        mov     ecx, msg_relocated1
        call    sys_msg_board_str
        mov     ecx, [esp+4]
        call    sys_msg_board_str
        mov     ecx, msg_relocated2
        call    sys_msg_board_str
        clc
        ret
.badreloc:
        pop     eax
        mov     ecx, msg_bad_relocation1
        call    sys_msg_board_str
        mov     ecx, [esp+4]
        call    sys_msg_board_str
        mov     ecx, msg_newline
        call    sys_msg_board_str
        stc
        ret
endp
