; Processings of PE format.
; Works in conjunction with modules.inc for non-PE-specific code.

; PE-specific part of init_module_struct.
; Fills fields of MODULE struct from PE image.
macro init_module_struct_pe_specific
{
; We need a module timestamp for bound imports.
; In a full PE, there are two timestamps, one in the header
; and one in the export table; existing tools use the first one.
; A stripped PE header has no timestamp, so read the export table;
; the stripper should write the correct value there.
        cmp     byte [esi], 'M'
        jz      .parse_mz
        cmp     [esi+STRIPPED_PE_HEADER.NumberOfRvaAndSizes], SPE_DIRECTORY_EXPORT
        jbe     @f
        mov     edx, [esi+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_EXPORT*sizeof.IMAGE_DATA_DIRECTORY+IMAGE_DATA_DIRECTORY.VirtualAddress]
        mov     edx, [esi+edx+IMAGE_EXPORT_DIRECTORY.TimeDateStamp]
@@:
        mov     [eax+MODULE.timestamp], edx
        mov     edx, esi
        sub     edx, [esi+STRIPPED_PE_HEADER.ImageBase]
        mov     [eax+MODULE.basedelta], edx
        mov     edx, [esi+STRIPPED_PE_HEADER.SizeOfImage]
        mov     [eax+MODULE.size], edx
        ret
.parse_mz:
        mov     ecx, [esi+3Ch]
        add     ecx, esi
        mov     edx, [ecx+IMAGE_NT_HEADERS.FileHeader.TimeDateStamp]
        mov     [eax+MODULE.timestamp], edx
        mov     edx, esi
        sub     edx, [ecx+IMAGE_NT_HEADERS.OptionalHeader.ImageBase]
        mov     [eax+MODULE.basedelta], edx
        mov     edx, [ecx+IMAGE_NT_HEADERS.OptionalHeader.SizeOfImage]
        mov     [eax+MODULE.size], edx
        ret
}

; Check whether PE module has been loaded at preferred address.
; If not, relocate the module.
;
; in: esi = PE base address
; in: [esp+4] = module name for debug print
; out: CF=1 - fail
proc fixup_pe_relocations c uses ebp, modulename
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
        cmp     byte [esi], 'M'
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
        ccall   loader_say_error, msg_noreloc1, [modulename], msg_noreloc2, 0
        stc
        ret
.parse_mz:
        mov     eax, [esi+3Ch]
        add     eax, esi
        sub     ebp, [eax+IMAGE_NT_HEADERS.OptionalHeader.ImageBase]
        jz      .nothing
        mov     dl, byte [eax+IMAGE_NT_HEADERS.FileHeader.Characteristics]
        cmp     [eax+IMAGE_NT_HEADERS.OptionalHeader.NumberOfDirectories], IMAGE_DIRECTORY_ENTRY_BASERELOC
        jbe     .norelocs
        add     eax, IMAGE_NT_HEADERS.OptionalHeader.DataDirectory+IMAGE_DIRECTORY_ENTRY_BASERELOC*sizeof.IMAGE_DATA_DIRECTORY
.common:
        cmp     [eax+IMAGE_DATA_DIRECTORY.isize], 0
        jz      .norelocs
        mov     edi, [eax+IMAGE_DATA_DIRECTORY.VirtualAddress]
        push    -1
        push    -1
        push    [eax+IMAGE_DATA_DIRECTORY.isize]
virtual at esp
.sizeleft       dd      ?
.next_page_original_access      dd      ?
.next_page_addr dd      ?
end virtual
        add     edi, esi
; 2. We need to relocate and we have the relocation table.
; esi = PE base address
; edi = pointer to current data of relocation table
; 2a. Relocation table is organized into blocks describing every page.
; End of table is defined from table size fetched from the header.
; Loop 2b-2i over all blocks until no more data is left.
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
        push    esi
        fpo_delta = fpo_delta + 4
; 2c. Check whether we have mprotect-ed the current page at the previous step.
; If so, go to 2e.
        add     edx, esi
        cmp     [.next_page_addr+fpo_delta], edx
        jz      .mprotected_earlier
; 2d. We are going to modify data, so mprotect the current page to be writable.
; Save the old protection, we will restore it after the block is processed.
; Ignore any error.
; Go to 2f after.
PROT_READ = 1
PROT_WRITE = 2
PROT_EXEC = 4
        push    ecx
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, PROT_READ+PROT_WRITE
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
        pop     ecx
        jmp     .mprotected
; 2e. We have already mprotect-ed the current page,
; move corresponding variables.
.mprotected_earlier:
        mov     [.next_page_addr+fpo_delta], -1
        mov     eax, [.next_page_original_access+fpo_delta]
.mprotected:
        push    eax
        fpo_delta = fpo_delta + 4
; 2g. Block data is an array of word values. Repeat 2h for every of those.
.relocloop:
        sub     ecx, 2
        jb      .relocdone
; 2h. Every value consists of a 4-bit type and 12-bit offset in the page.
; x86 uses two types: 0 = no data (used for padding), 3 = 32-bit relative.
        movzx   eax, word [edi]
        add     edi, 2
        mov     ebx, eax
        and     ebx, 0xFFF
        shr     eax, 12
        jz      .relocloop
        cmp     al, IMAGE_REL_BASED_HIGHLOW
        jnz     .badreloc
; If the target dword intersects page boundary,
; we need to mprotect the next page too.
        cmp     ebx, 0xFFC
        jbe     .no_mprotect_next
        push    ebx ecx edx
        fpo_delta = fpo_delta + 12
        lea     eax, [.next_page_original_access+fpo_delta]
        call    .restore_old_access
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, PROT_READ+PROT_WRITE
        mov     esi, 0x1000
        add     edx, esi
        call    FS_SYSCALL_PTR
        mov     [.next_page_original_access+fpo_delta], eax
        mov     [.next_page_addr+fpo_delta], edx
        pop     edx ecx ebx
        fpo_delta = fpo_delta - 12
.no_mprotect_next:
        add     [edx+ebx], ebp
        jmp     .relocloop
.relocdone:
; 2i. Restore memory protection changed in 2d.
        pop     ecx
        fpo_delta = fpo_delta - 4
        mov     eax, 68
        mov     ebx, 30
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
        pop     esi
        fpo_delta = fpo_delta - 4
.pagedone:
        cmp     [.sizeleft+fpo_delta], 0
        jnz     .pageloop
        lea     eax, [.next_page_original_access+fpo_delta]
        call    .restore_old_access
        add     esp, 12
; 3. For performance reasons, relocation should be avoided
; by choosing an appropriate preferred address.
; If we have actually relocated something, yell to the debug board,
; so the programmer can notice that.
; It's a warning, not an error, so don't call loader_say_error.
        mov     ecx, msg_relocated1
        call    sys_msg_board_str
        mov     ecx, [modulename]
        call    sys_msg_board_str
        mov     ecx, msg_relocated2
        call    sys_msg_board_str
        clc
        ret
.badreloc:
        pop     ecx
        pop     esi
        add     esp, 12
        ccall   loader_say_error, msg_bad_relocation, [modulename], 0
        stc
        ret

.restore_old_access:
        cmp     dword [eax+4], -1
        jz      @f
        mov     ecx, [eax]
        mov     edx, [eax+4]
        mov     eax, 68
        mov     ebx, 30
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
@@:
        retn
endp

; Resolves static dependencies in the given PE module.
; Recursively loads and initializes all dependencies.
; in: esi -> MODULE struct
; out: eax=0 - success, eax=-1 - error
; modules_mutex should be locked
proc resolve_pe_imports
locals
export_base             dd      ?
export_ptr              dd      ?
export_size             dd      ?
import_module           dd      ?
import_descriptor       dd      ?
next_forwarder          dd      ?
bound_import_descriptor dd      ?
bound_import_dir        dd      ?
bound_modules_count     dd      ?
bound_modules_ptr       dd      ?
bound_module            dd      ?
bound_modules_left      dd      ?
cur_page                dd      -0x1000 ; the page at 0xFFFFF000 is never allocated
cur_page_old_access     dd      ?
next_page               dd      -1
next_page_old_access    dd      ?
endl

; General case of resolving imports against one module that is already loaded:
; binding either does not exist or has mismatched timestamp,
; so we need to walk through all imported symbols and resolve each one.
; in: ebp -> IMAGE_IMPORT_DESCRIPTOR
macro resolve_import_from_module fail_action
{
local .label1, .loop, .done
; common preparation that doesn't need to be repeated per each symbol
        mov     eax, [import_module]
        mov     eax, [eax+MODULE.base]
        call    prepare_import_from_module
; There are two arrays of dwords pointed to by FirstThunk and OriginalFirstThunk.
; Target array is FirstThunk: addresses of imported symbols should be written
; there, that is where the program expects to find them.
; Source array can be either FirstThunk or OriginalFirstThunk.
; Normally, FirstThunk and OriginalFirstThunk in a just-compiled binary
; point to two identical copies of the same array.
; Binding of the binary rewrites FirstThunk array with actual addresses,
; but keeps OriginalFirstThunk as is.
; If OriginalFirstThunk and FirstThunk are both present, use OriginalFirstThunk
; as source array.
; However, a compiler is allowed to generate a binary without OriginalFirstThunk;
; it is impossible to bind such a binary, but it is still valid.
; If OriginalFirstThunk is absent, use FirstThunk as source array.
        mov     ebx, [ebp+IMAGE_IMPORT_DESCRIPTOR.OriginalFirstThunk]
        mov     ebp, [ebp+IMAGE_IMPORT_DESCRIPTOR.FirstThunk]
        test    ebx, ebx
        jnz     @f
        mov     ebx, ebp
@@:
; FirstThunk and OriginalFirstThunk are RVAs.
        add     ebx, [esi+MODULE.base]
        add     ebp, [esi+MODULE.base]
; Source array is terminated with zero dword.
.loop:
        cmp     dword [ebx], 0
        jz      .done
        mov     ecx, [ebx]
        get_address_for_thunk ; should preserve esi,edi,ebp
        test    eax, eax
        jz      fail_action
        mov     edi, eax
        mov     edx, ebp
        call    .ensure_writable ; should preserve edx,ebx,esi,ebp
        mov     [edx], edi
        add     ebx, 4
        add     ebp, 4
        jmp     .loop
.done:
}

; Resolve one imported symbol.
; in: ecx = ordinal or RVA of thunk
; out: eax = address of exported function
macro get_address_for_thunk
{
local .ordinal, .common
; Ordinal imports have bit 31 set, name imports have bit 31 clear.
        btr     ecx, 31
        jc      .ordinal
; Thunk for name import is RVA of IMAGE_IMPORT_BY_NAME structure.
        add     ecx, [esi+MODULE.base]
        movzx   edx, [ecx+IMAGE_IMPORT_BY_NAME.Hint]
        add     ecx, IMAGE_IMPORT_BY_NAME.Name
        call    get_exported_function_by_name
        jmp     .common
.ordinal:
; Thunk for ordinal import is just an ordinal,
; bit 31 has been cleared by btr instruction.
        call    get_exported_function_by_ordinal
.common:
}

; We have four main variants:
; normal unbound import, old-style bound import, new-style bound import,
; no import.
; * Normal unbound import:
;   we have an array of import descriptors, one per imported module,
;   pointed to by import directory.
;   We should loop over all descriptors and apply resolve_import_from_module
;   for each one.
; * Old-style bound import:
;   we have the same array of import descriptors, but timestamp field is set up.
;   We should do the same loop, but we can do a lightweight processing
;   of modules with correct timestamp. In the best case, "lightweight processing"
;   means just skipping them, but corrections arise for relocated modules
;   and forwarded exports.
; * New-style bound import:
;   we have two parallel arrays of import descriptors and bound descriptors,
;   pointed to by two directories. Timestamp field has a special value -1
;   in import descriptors, real timestamps are in bound descriptors.
; * No import: not really different from normal import with no descriptors.
; Forwarded exports are the part where binding goes really interesting.
; The address of forwarded export can change with any library in the chain.
; In old-style bound import, a descriptor can list only the library itself,
; so it is impossible to bind forwarded exports at all; thunks that point to
; forwarded exports are linked in the list starting from ForwarderChain in import descriptor.
; New-style bound import exists exactly to address this problem; it allows to
; list several related libraries for one imported module.
; However, even with new-style bound import, some forwarded exports can
; still remain unbound: binding tool can fail to load dependent libraries
; during binding time; the tool from MS SDK for unknown reason just refuses to
; bind forwarded exports except for built-in white list if subsystem version is
; < 6.0. So even with new-style bound import, ForwarderChain still can be non-empty.
; Thus, we always need to look at old-style import descriptor.
; 1. Fetch addresses of two directories. We are not interested in their sizes.
; ebp = import RVA
; ebx = bound import RVA
        xor     ebx, ebx
        xor     ebp, ebp
; PE and stripped PE have different places for directories.
        mov     eax, [esi+MODULE.base]
        cmp     byte [eax], 'M'
        jz      .parse_mz
        cmp     [eax+STRIPPED_PE_HEADER.NumberOfRvaAndSizes], SPE_DIRECTORY_IMPORT
        jbe     .common
        mov     ebp, [eax+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_IMPORT*sizeof.IMAGE_DATA_DIRECTORY]
        cmp     [eax+STRIPPED_PE_HEADER.NumberOfRvaAndSizes], SPE_DIRECTORY_BOUND_IMPORT
        jbe     .common
        mov     ebx, [eax+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_BOUND_IMPORT*sizeof.IMAGE_DATA_DIRECTORY]
        jmp     .common
.parse_mz:
        add     eax, [eax+3Ch]
        cmp     [eax+IMAGE_NT_HEADERS.OptionalHeader.NumberOfDirectories], IMAGE_DIRECTORY_ENTRY_IMPORT
        jbe     .common
        mov     ebp, [eax+IMAGE_NT_HEADERS.OptionalHeader.DataDirectory+IMAGE_DIRECTORY_ENTRY_IMPORT*sizeof.IMAGE_DATA_DIRECTORY]
        cmp     [eax+IMAGE_NT_HEADERS.OptionalHeader.NumberOfDirectories], IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT
        jbe     .common
        mov     ebx, [eax+IMAGE_NT_HEADERS.OptionalHeader.DataDirectory+IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT*sizeof.IMAGE_DATA_DIRECTORY]
.common:
; If import directory is not present, no import - nothing to do.
; Ignore bound import directory in this case.
        test    ebp, ebp
        jz      .done
        add     ebp, [esi+MODULE.base] ; directories contain RVA
        add     ebx, [esi+MODULE.base] ; directories contain RVA
        mov     [bound_import_dir], ebx
        mov     [bound_import_descriptor], ebx
; Repeat remaining steps for all descriptors in the directory.
.descriptor_loop:
; 2. Check whether this descriptor is an end mark with zero fields.
; Look at Name field.
        mov     edi, [ebp+IMAGE_IMPORT_DESCRIPTOR.Name]
        test    edi, edi
        jz      .done
; 3. Load the target module.
        add     edi, [esi+MODULE.base] ; Name field is RVA
        call    load_imported_module ; should preserve esi,ebp
        test    eax, eax
        jz      .failed
        mov     [import_module], eax
; 4. Check whether the descriptor has a non-stale old-style binding.
; Zero timestamp means "not bound".
; Timestamp 0xFFFFFFFF means "new-style binding".
; Mismatched timestamp means "stale binding".
; In first and third cases, go to 9.
; In second case, go to 10 for further checks.
        mov     edx, [ebp+IMAGE_IMPORT_DESCRIPTOR.TimeDateStamp]
        test    edx, edx
        jz      .resolve_generic
        cmp     edx, -1
        jz      .new_binding
        cmp     edx, [eax+MODULE.timestamp]
        jnz     .resolve_generic
; 5. The descriptor has a non-stale old-style binding.
; There are two cases when we still need to do something:
; * if the target module has been relocated, we need to add
;   relocation delta to all addresses;
; * if some exports are forwarded, we need to resolve them.
;   Thunks for forwarded exports contain index of next forwarded export
;   instead of target address, making a single-linked list terminated by -1.
;   ForwarderChain is the head of the list.
; Check for the first problem first; the corresponding code can handle both.
; If the target module is relocated, go to 8.
; If the target module is not relocated, but has forwarded exports, go to 7.
; Otherwise, advance to 6.
.old_binding:
        cmp     [eax+MODULE.basedelta], 0
        jnz     .old_binding_relocate
.check_forwarder_chain:
        cmp     [ebp+IMAGE_IMPORT_DESCRIPTOR.ForwarderChain], -1
        jnz     .resolve_forward_chain
.next_descriptor:
; 6. Advance to next descriptor and continue the loop.
        add     ebp, sizeof.IMAGE_IMPORT_DESCRIPTOR
        jmp     .descriptor_loop
.resolve_forward_chain:
; 7. Resolve all thunks from ForwarderChain list.
        mov     eax, [import_module]
        mov     eax, [eax+MODULE.base]
        call    prepare_import_from_module
        mov     edi, [ebp+IMAGE_IMPORT_DESCRIPTOR.ForwarderChain]
.resolve_forward_chain_loop:
        mov     ebx, [ebp+IMAGE_IMPORT_DESCRIPTOR.OriginalFirstThunk]
        add     ebx, [esi+MODULE.base]
        mov     ecx, [ebx+edi*4]
        get_address_for_thunk ; should preserve esi,edi,ebp
        test    eax, eax
        jz      .failed
        mov     ebx, eax
        mov     edx, [ebp+IMAGE_IMPORT_DESCRIPTOR.FirstThunk]
        add     edx, [esi+MODULE.base]
        lea     edx, [edx+edi*4]
        call    .ensure_writable ; should preserve edx,ebx,esi,ebp
        mov     edi, [edx] ; next forwarded export
        mov     [edx], ebx ; store the address
        cmp     edi, -1
        jnz     .resolve_forward_chain_loop
; After resolving, we are done with this import descriptor, go to 6.
        jmp     .next_descriptor
.done:
        call    .restore_protection
        xor     eax, eax
        ret
.old_binding_relocate:
; 8. The descriptor has a non-stale old-style binding,
; but the target module is relocated, so we need to add MODULE.basedelta to
; all addresses. Also, there can be forwarded exports.
; For consistency with generic-case resolve_import_from_module,
; check for end of thunks by looking at OriginalFirstThunk array.
; Note: we assume here that ForwarderChain list is ordered.
; After that, go to 6.
        mov     edi, [eax+MODULE.basedelta]
        cmp     [ebp+IMAGE_IMPORT_DESCRIPTOR.ForwarderChain], -1
        jz      @f
        mov     eax, [import_module]
        mov     eax, [eax+MODULE.base]
        call    prepare_import_from_module
@@:
        mov     edx, [ebp+IMAGE_IMPORT_DESCRIPTOR.FirstThunk]
        add     edx, [esi+MODULE.base]
        mov     ebx, [ebp+IMAGE_IMPORT_DESCRIPTOR.OriginalFirstThunk]
        add     ebx, [esi+MODULE.base]
        mov     eax, [ebp+IMAGE_IMPORT_DESCRIPTOR.ForwarderChain]
        lea     eax, [edx+eax*4]
        mov     [next_forwarder], eax
.old_binding_relocate_loop:
        cmp     dword [ebx], 0
        jz      .next_descriptor
        call    .ensure_writable ; should preserve esi,edi,ebp,ebx,edx
        cmp     edx, [next_forwarder]
        jz      .old_binding_relocate_forwarder
        add     dword [edx], edi
.old_binding_relocate_next:
        add     edx, 4
        add     ebx, 4
        jmp     .old_binding_relocate_loop
.old_binding_relocate_forwarder:
        mov     ecx, [ebx]
        get_address_for_thunk
        test    eax, eax
        jz      .failed
        mov     edx, [next_forwarder]
        mov     ecx, [edx]
        mov     [edx], eax
        mov     eax, [ebp+IMAGE_IMPORT_DESCRIPTOR.FirstThunk]
        add     eax, [esi+MODULE.base]
        lea     eax, [eax+ecx*4]
        mov     [next_forwarder], eax
        jmp     .old_binding_relocate_next
.resolve_generic:
; 9. Run generic-case resolver.
        mov     [import_descriptor], ebp
        resolve_import_from_module .failed ; should preserve esi
        mov     ebp, [import_descriptor]
; After that, go to 6.
        jmp     .next_descriptor
.new_binding:
; New-style bound import.
; 10. Locate the new-style descriptor corresponding to the current
; import descriptor.
; 10a. Check the current new-style descriptor: the one that follows
; previous one, if there was the previous one, or the first one.
; In most cases, new-style descriptors are in the same order as
; import descriptors, so full loop in 10b can be avoided.
        mov     edx, [ebp+IMAGE_IMPORT_DESCRIPTOR.Name]
        add     edx, [esi+MODULE.base]
        mov     ebx, [bound_import_descriptor]
        movzx   edi, [ebx+IMAGE_BOUND_IMPORT_DESCRIPTOR.OffsetModuleName]
        test    edi, edi
        jz      .look_new_binding_hard
        add     edi, [bound_import_dir]
        xor     ecx, ecx
@@:
        mov     al, [edx+ecx]
        cmp     al, [edi+ecx]
        jnz     .look_new_binding_hard
        test    al, al
        jz      .new_binding_found
        inc     ecx
        jmp     @b
.look_new_binding_hard:
; 10b. We are out of luck with the current new-style descriptor,
; so loop over all of them looking for the matching one.
        mov     ebx, [bound_import_dir]
.look_new_binding_loop:
        movzx   edi, [ebx+IMAGE_BOUND_IMPORT_DESCRIPTOR.OffsetModuleName]
        add     edi, [bound_import_dir]
        xor     ecx, ecx
@@:
        mov     al, [edx+ecx]
        cmp     al, [edi+ecx]
        jnz     .look_new_binding_next
        test    al, al
        jz      .new_binding_found
        inc     ecx
        jmp     @b
.look_new_binding_next:
        movzx   ecx, [ebx+IMAGE_BOUND_IMPORT_DESCRIPTOR.NumberOfModuleForwarderRefs]
        lea     ebx, [ebx+(ecx+1)*sizeof.IMAGE_BOUND_IMPORT_DESCRIPTOR]
        jmp     .look_new_binding_loop
.new_binding_found:
; 10c. Store the next descriptor for subsequent scans.
        movzx   ecx, [ebx+IMAGE_BOUND_IMPORT_DESCRIPTOR.NumberOfModuleForwarderRefs]
        lea     eax, [ebx+(ecx+1)*sizeof.IMAGE_BOUND_IMPORT_DESCRIPTOR]
        mov     [bound_import_descriptor], eax
; Bound import descriptors come in groups.
; The first descriptor in each group corresponds to the main imported module.
; If some exports from the module are forwarded, additional descriptors
; are created for modules where those exports are forwarded to.
; Number of additional descriptors is given by one field in the first descriptor.
; 11. We have already loaded the main module, validate its timestamp.
; If timestamp does not match, go to 9 to run generic-case resolver.
        mov     eax, [import_module]
        mov     edx, [eax+MODULE.timestamp]
        cmp     edx, [ebx+IMAGE_BOUND_IMPORT_DESCRIPTOR.TimeDateStamp]
        jnz     .resolve_generic
; 12. If there are no additional libraries,
; the situation is exactly same as old-style binding, so go to 5.
        test    ecx, ecx
        jz      .old_binding
; 13. Load additional libraries and validate their timestamps.
; If at least one timestamp is invalid, resort to generic-case resolving.
; 13a. Allocate memory for all bound modules, including the main module.
        lea     ecx, [(ecx+1)*4]
        stdcall malloc, ecx
        test    eax, eax
        jz      .failed
        mov     [bound_modules_ptr], eax
; 13b. Store the main module.
        mov     edx, [import_module]
        mov     [eax], edx
        xor     ecx, ecx
; 13c. Loop over all additional descriptors.
.newstyle_load_loop:
        inc     ecx
        mov     [bound_modules_count], ecx
        movzx   edi, [ebx+ecx*sizeof.IMAGE_BOUND_IMPORT_DESCRIPTOR+IMAGE_BOUND_IMPORT_DESCRIPTOR.OffsetModuleName]
        add     edi, [bound_import_dir]
        call    load_imported_module ; should preserve ebx,esi,ebp
        test    eax, eax
        jz      .newstyle_failed
        mov     ecx, [bound_modules_count]
        mov     edx, [ebx+ecx*sizeof.IMAGE_BOUND_IMPORT_DESCRIPTOR+IMAGE_BOUND_IMPORT_DESCRIPTOR.TimeDateStamp]
        cmp     [eax+MODULE.timestamp], edx
        jnz     .newstyle_stale
        mov     edx, [bound_modules_ptr]
        mov     [edx+ecx*4], eax
        cmp     cx, [ebx+IMAGE_BOUND_IMPORT_DESCRIPTOR.NumberOfModuleForwarderRefs]
        jb      .newstyle_load_loop
        inc     ecx
        mov     [bound_modules_count], ecx
; New-style binding has correct timestamp.
; There still can be same two problems as in step 5 with old-style binding.
; 14. Check whether at least one module is relocated. If so, go to 16.
.newstyle_check_reloc:
        mov     eax, [edx]
        cmp     [eax+MODULE.basedelta], 0
        jnz     .newstyle_need_reloc
        add     edx, 4
        dec     ecx
        jnz     .newstyle_check_reloc
; 15. Bound modules are not relocated.
; The only remaining problem could be unbound forwarders.
; Free memory allocated at 13a and let steps 6 and 7 do their work.
        stdcall free, [bound_modules_ptr]
        jmp     .check_forwarder_chain
.newstyle_stale:
        stdcall free, [bound_modules_ptr]
        jmp     .resolve_generic
; 16. The descriptor has a non-stale new-style binding,
; but at least one of target modules is relocated, so we need to add
; MODULE.basedelta to addresses from relocated modules.
; Also, there can be forwarded exports.
; For consistency with generic-case resolve_import_from_module,
; check for end of thunks by looking at OriginalFirstThunk array.
; Note: we assume here that ForwarderChain list is ordered.
; After that, go to 6.
.newstyle_need_reloc:
        mov     eax, [import_module]
        mov     eax, [eax+MODULE.base]
        call    prepare_import_from_module
        mov     edx, [ebp+IMAGE_IMPORT_DESCRIPTOR.FirstThunk]
        add     edx, [esi+MODULE.base]
        mov     ebx, [ebp+IMAGE_IMPORT_DESCRIPTOR.OriginalFirstThunk]
        add     ebx, [esi+MODULE.base]
        mov     eax, [ebp+IMAGE_IMPORT_DESCRIPTOR.ForwarderChain]
        lea     eax, [edx+eax*4]
        mov     [next_forwarder], eax
.new_binding_relocate_loop:
        cmp     dword [ebx], 0
        jz      .new_binding_relocate_done
        cmp     edx, [next_forwarder]
        jz      .new_binding_resolve_thunk
        mov     [bound_module], 0
        mov     eax, [bound_modules_count]
        mov     [bound_modules_left], eax
        mov     edi, [bound_modules_ptr]
; There should be at least one module containing address [edx].
; There can be more than one if preferred address ranges for two modules intersect;
; in this case, we are forced to resolve address from scratch.
.new_binding_lookup_module:
        mov     ecx, [edx]
        mov     eax, [edi]
        sub     ecx, [eax+MODULE.base]
        add     ecx, [eax+MODULE.basedelta]
        cmp     ecx, [eax+MODULE.size]
        jae     @f
        cmp     [bound_module], 0
        jnz     .new_binding_resolve_thunk
        mov     [bound_module], eax
@@:
        add     edi, 4
        dec     [bound_modules_left]
        jnz     .new_binding_lookup_module
        mov     edi, [bound_module]
        mov     edi, [edi+MODULE.basedelta]
        test    edi, edi
        jz      .new_binding_relocate_next
        call    .ensure_writable ; should preserve esi,edi,ebp,ebx,edx
        add     dword [edx], edi
.new_binding_relocate_next:
        add     edx, 4
        add     ebx, 4
        jmp     .new_binding_relocate_loop
.new_binding_resolve_thunk:
        mov     [bound_modules_left], edx
        call    .ensure_writable
        mov     ecx, [ebx]
        get_address_for_thunk
        test    eax, eax
        jz      .newstyle_failed
        mov     edx, [bound_modules_left]
        mov     ecx, [edx]
        mov     [edx], eax
        cmp     edx, [next_forwarder]
        jnz     .new_binding_relocate_next
        mov     eax, [ebp+IMAGE_IMPORT_DESCRIPTOR.FirstThunk]
        add     eax, [esi+MODULE.base]
        lea     eax, [eax+ecx*4]
        mov     [next_forwarder], eax
        jmp     .new_binding_relocate_next
.new_binding_relocate_done:
        stdcall free, [bound_modules_ptr]
        jmp     .next_descriptor
.newstyle_failed:
        stdcall free, [bound_modules_ptr]
.failed:
        call    .restore_protection
        xor     eax, eax
        dec     eax
        ret

; Local helper functions.
        fpo_delta = fpo_delta + 4
; Import table may reside in read-only pages.
; We should mprotect any page where we are going to write to.
; Things get interesting when one thunk spans two pages.
; in: edx = address of dword to make writable
.ensure_writable:
; 1. Fast path: if we have already mprotect-ed one page and
; the requested dword is in the same page, do nothing.
        mov     eax, edx
        sub     eax, [cur_page]
        cmp     eax, 0x1000 - 4
        ja      .cur_page_not_sufficient
.ensure_writable.nothing:
        retn
.cur_page_not_sufficient:
; 2. If the requested dword begins in the current page
; and ends in the next page, mprotect the next page and return.
        push    ebx esi edx
        fpo_delta = fpo_delta + 12
        cmp     eax, 0x1000
        jae     .wrong_cur_page
        cmp     [next_page], -1
        jnz     @f
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, PROT_READ+PROT_WRITE
        mov     edx, [cur_page]
        mov     esi, 0x1000
        add     edx, esi
        mov     [next_page], edx
        call    FS_SYSCALL_PTR
        mov     [next_page_old_access], eax
@@:
        pop     edx esi ebx
        retn
.wrong_cur_page:
; The requested dword does not intersect with the current page.
; 3. Restore the protection of the current page,
; it is unlikely to be used again.
        cmp     [cur_page], -0x1000
        jz      @f
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, [cur_page_old_access]
        mov     edx, [cur_page]
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
@@:
; 4. If the next page has been mprotect-ed too,
; switch to it as the current page and restart the function.
        cmp     [next_page], -1
        jz      @f
        mov     eax, [next_page]
        mov     [cur_page], eax
        mov     eax, [next_page_old_access]
        mov     [cur_page_old_access], eax
        mov     [next_page], -1
        pop     edx esi ebx
        jmp     .ensure_writable
@@:
; 5. This is the entirely new page to mprotect.
        mov     edx, [esp]
        and     edx, not 0xFFF
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, PROT_READ+PROT_WRITE
        mov     [cur_page], edx
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
        mov     [cur_page_old_access], eax
        pop     edx esi ebx
        fpo_delta = fpo_delta - 12
        retn

; Called at end of processing,
; restores protection of pages that we have mprotect-ed for write.
.restore_protection:
        push    esi
        fpo_delta = fpo_delta + 4
        cmp     [next_page], -1
        jz      @f
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, [next_page_old_access]
        mov     edx, [next_page]
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
@@:
        cmp     [cur_page], -0x1000
        jz      @f
        mov     eax, 68
        mov     ebx, 30
        mov     ecx, [cur_page_old_access]
        mov     edx, [cur_page]
        mov     esi, 0x1000
        call    FS_SYSCALL_PTR
@@:
        pop     esi
        fpo_delta = fpo_delta - 4
        retn
endp

; Part of resolving symbol from a module that is the same for all symbols.
; resolve_pe_imports calls it only once per module.
; Fetches export directory from the module.
; Non-standard calling convention: saves results to first 2 dwords on the stack.
; in: eax = module base
proc prepare_import_from_module c, export_base, export_ptr, export_size
; The implementation is straightforward.
        mov     [export_base], eax
        cmp     byte [eax], 'M'
        jz      .parse_mz
        cmp     [eax+STRIPPED_PE_HEADER.NumberOfRvaAndSizes], SPE_DIRECTORY_EXPORT
        jbe     .noexport
        mov     edx, [eax+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_EXPORT*sizeof.IMAGE_DATA_DIRECTORY+IMAGE_DATA_DIRECTORY.VirtualAddress]
        test    edx, edx
        jz      .noexport
        add     edx, eax
        mov     [export_ptr], edx
        mov     edx, [eax+sizeof.STRIPPED_PE_HEADER+SPE_DIRECTORY_EXPORT*sizeof.IMAGE_DATA_DIRECTORY+IMAGE_DATA_DIRECTORY.isize]
        mov     [export_size], edx
        ret
.parse_mz:
        mov     ecx, [eax+3Ch]
        add     ecx, eax
        cmp     [ecx+IMAGE_NT_HEADERS.OptionalHeader.NumberOfDirectories], IMAGE_DIRECTORY_ENTRY_EXPORT
        jbe     .noexport
        mov     edx, [ecx+IMAGE_NT_HEADERS.OptionalHeader.DataDirectory.VirtualAddress+IMAGE_DIRECTORY_ENTRY_EXPORT*sizeof.IMAGE_DATA_DIRECTORY]
        test    edx, edx
        jz      .noexport
        add     edx, eax
        mov     [export_ptr], edx
        mov     edx, [ecx+IMAGE_NT_HEADERS.OptionalHeader.DataDirectory.isize+IMAGE_DIRECTORY_ENTRY_EXPORT*sizeof.IMAGE_DATA_DIRECTORY]
        mov     [export_size], edx
        ret
.noexport:
        mov     [export_ptr], 0
        mov     [export_size], 0
        ret
endp

; PE format supports export by name and by ordinal.
; Any exported symbol always have an ordinal.
; It may have a name, it may have no name.
; A symbol can even have multiple names, usually this happens
; when several functions with the same body like 'ret' are merged.
;
; Addresses of all exported symbols are contained in one array AddressOfFunctions.
; Ordinal of a symbol is an index in this array + Base.
; Base is defined in export directory, usually it equals 1.
;
; Export by name is more complicated. There are two parallel arrays
; AddressOfNames and AddressOfNameOrdinals with the same length.
; This length can be less or greater than length of AddressOfFunctions.
; AddressOfNames is a sorted array with all exported names.
; AddressOfNameOrdinals, contrary to the title, gives index in AddressOfFunctions.
; Looking up a name means
; * scanning AddressOfNames array to find the index of the corresponding name
; * looking in AddressOfNameOrdinals at the index found above to get another index;
;   index in AddressOfNames/AddressOfNameOrdinals has no other meaning
; * finally, looking in AddressOfFunctions with that second index.

; Resolve symbol from a module by name.
; prepare_import_from_module should be called beforehand.
; in: ecx -> name, edx = hint for lookup in name table
; out: eax = exported address or NULL
; if [module] is zero, modules_mutex should be unlocked
; if [module] is nonzero, modules_mutex should be locked
proc get_exported_function_by_name c uses ebx esi edi, export_base, export_ptr, export_size, module
locals
forward_export_base     dd      ?
forward_export_ptr      dd      ?
forward_export_size     dd      ?
forward_export_module   dd      ?
endl
; 1. Find length of the name, including terminating zero.
        mov     esi, ecx
@@:
        inc     ecx
        cmp     byte [ecx-1], 0
        jnz     @b
        sub     ecx, esi
; 2. Validate that export directory is present at all.
        mov     eax, [export_ptr]
        test    eax, eax
        jz      .export_name_not_found
; 3. Check whether the hint is correct.
; The hint is a zero-based index in name table.
; Theoretically, zero is a valid hint.
; Unfortunately, in practice everyone uses zero if the hint is unknown,
; which is a quite typical situation, so treating zero as a valid hint
; would waste processor cycles much more often than save.
; So only check the hint if it is between 1 and NumberOfNames-1 inclusive.
; 3a. Validate the hint.
        mov     ebx, [eax+IMAGE_EXPORT_DIRECTORY.AddressOfNames]
        add     ebx, [export_base]
        cmp     edx, [eax+IMAGE_EXPORT_DIRECTORY.NumberOfNames]
        jae     .ignore_hint
        test    edx, edx
        jz      .ignore_hint
; 3b. Check the hinted name.
; If it matches, go to 5. If not, we're out of luck, use normal lookup.
        mov     edi, [ebx+edx*4]
        add     edi, [export_base]
        push    ecx esi
        repz cmpsb
        pop     esi ecx
        jz      .hint_ok
.ignore_hint:
; 4. Binary search over name table.
; Export names are sorted with respect to repz cmpsb.
; edi <= (the target index) < edx
        xor     edi, edi
        mov     edx, [eax+IMAGE_EXPORT_DIRECTORY.NumberOfNames]
.export_name_search.loop:
; if there are no indexes between edi and edx, name is invalid
        cmp     edi, edx
        jae     .export_name_not_found
; try the index in the middle of current range
        lea     eax, [edi+edx]
        shr     eax, 1
; compare
        push    ecx esi edi
        fpo_delta = fpo_delta + 12
        mov     edi, [ebx+eax*4]
        add     edi, [export_base]
        repz cmpsb
        pop     edi esi ecx
        fpo_delta = fpo_delta - 12
; exact match -> found, go to 5
; string at esi = target, string at edi = current attempt
; (string at esi) < (string at edi) -> current index is too high, update upper range
; (string at esi) > (string at edi) -> current index is too low, update lower range
        jz      .found
        jb      @f
        lea     edi, [eax+1]
        jmp     .export_name_search.loop
@@:
        mov     edx, eax
        jmp     .export_name_search.loop
; Generic error handler.
.export_name_not_found:
        mov     ebx, esi
        call    .get_module_name
        ccall   loader_say_error, msg_export_name_not_found, ebx, msg_export_not_found, eax, 0
.return0:
        xor     eax, eax
        ret
.hint_ok:
        mov     eax, edx
.found:
; 5. We have found an index in AddressOfNames/AddressOfNameOrdinals arrays,
; convert it to index in AddressOfFunctions array.
        mov     edx, [export_ptr]
        mov     ebx, [edx+IMAGE_EXPORT_DIRECTORY.AddressOfNameOrdinals]
        add     ebx, [export_base]
        movzx   eax, word [ebx+eax*2]
; 6. Fetch the exported address from AddressOfFunctions array.
        cmp     eax, [edx+IMAGE_EXPORT_DIRECTORY.NumberOfFunctions]
        jae     .export_name_not_found
        mov     ebx, [edx+IMAGE_EXPORT_DIRECTORY.AddressOfFunctions]
        add     ebx, [export_base]
        mov     eax, [ebx+eax*4]
        test    eax, eax
        jz      .export_name_not_found
.check_forwarded:
; This part of code is also used by get_exported_function_by_ordinal.
; 7. Check whether the address is inside the export directory.
; If not, we are done.
        add     eax, [export_base]
        mov     ebx, eax
        sub     ebx, edx
        cmp     ebx, [export_size]
        jb      .export_is_forwarded
        ret
.export_is_forwarded:
; The export is forwarded to another module.
; The address we have got points to the string "<module>.<function>"
; 8. Get the target module name. It is everything before the first dot,
; minus DLL extension.
; 8a. Find the dot.
        mov     ebx, eax
@@:
        inc     eax
        cmp     byte [eax-1], '.'
        jz      .dot_found
        cmp     byte [eax-1], 0
        jnz     @b
        call    .get_module_name
        ccall   loader_say_error, msg_invalid_forwarder, eax, 0
        xor     eax, eax
        ret
.dot_found:
; 8b. Allocate the memory.
        sub     eax, ebx
        mov     edi, eax
        add     eax, 4 ; dll + terminating zero
        stdcall malloc, eax
        test    eax, eax
        jz      .return0
; 8c. Copy module name.
        mov     esi, ebx
        mov     ecx, edi
        mov     edi, eax
        rep movsb
        mov     dword [edi], 'dll'
        mov     ebx, esi ; save pointer to <function>
        mov     edi, eax ; module name
; 9. Load the target module.
; 9a. Get the pointer to MODULE struct for ourselves.
        mov     esi, [module]
        test    esi, esi
        jnz     @f
        mutex_lock modules_mutex
        mov     ecx, [export_base]
        call    find_module_by_addr
        test    esi, esi
        jz      .load_forwarded_failed
@@:
; 9b. Call the worker.
        call    load_imported_module
        test    eax, eax
        jz      .load_forwarded_failed
        mov     esi, eax
; 9c. We don't need module name anymore, free the memory allocated at 8b.
        stdcall free, edi
; 10. Resolve the forwarded export recursively.
; 10a. Prepare for importing.
        mov     [forward_export_module], esi
        mov     eax, [esi+MODULE.base]
        call    prepare_import_from_module
; 10b. Check whether we are importing by ordinal or by name.
; Forwarded export by ordinal has ebx -> "#<ordinal>".
        cmp     byte [ebx], '#'
        jnz     .no_ordinal
        lea     edx, [ebx+1]
        xor     ecx, ecx ; ordinal
@@:
        movzx   eax, byte [edx]
        sub     eax, '0'
        cmp     eax, 10
        jae     .no_ordinal
        lea     ecx, [ecx*5]
        lea     ecx, [ecx*2+eax]
        inc     edx
        cmp     byte [edx], 0
        jnz     @b
; 10c. We are importing by ordinal. Call the worker.
        call    get_exported_function_by_ordinal
        jmp     @f
        ret
.no_ordinal:
; 10d. We are importing by name. Call the worker.
        mov     ecx, ebx
        or      edx, -1
        call    get_exported_function_by_name
@@:
        cmp     [module], 0
        jnz     @f
        push    eax
        mutex_unlock modules_mutex
        pop     eax
@@:
        ret
.load_forwarded_failed:
        cmp     [module], 0
        jnz     @f
        mutex_unlock modules_mutex
@@:
        stdcall free, edi
        xor     eax, eax
        ret

        fpo_delta = fpo_delta + 4
.get_module_name:
        mov     esi, [module]
        test    esi, esi
        jnz     @f
        mutex_lock modules_mutex
        mov     ecx, [export_base]
        call    find_module_by_addr
        mutex_unlock modules_mutex
@@:
        mov     eax, msg_unknown
        test    esi, esi
        jz      @f
        mov     eax, [esi+MODULE.filename]
@@:
        retn
endp

; Resolve symbol from a module by name.
; prepare_import_from_module should be called beforehand.
; in: ecx = ordinal
; out: eax = exported address or NULL
; if [module] is zero, modules_mutex should be unlocked
; if [module] is nonzero, modules_mutex should be locked
proc get_exported_function_by_ordinal c uses ebx esi edi, export_base, export_ptr, export_size, module
locals
forward_export_base     dd      ?
forward_export_ptr      dd      ?
forward_export_size     dd      ?
forward_export_module   dd      ?
endl
; 1. Validate that export directory is present at all.
        mov     edx, [export_ptr]
        test    edx, edx
        jz      .export_ordinal_not_found
; 2. Convert ordinal to index in AddressOfFunctions array.
        mov     eax, ecx ; keep ecx for error message
        sub     eax, [edx+IMAGE_EXPORT_DIRECTORY.Base]
; 3. Validate the index.
        cmp     eax, [edx+IMAGE_EXPORT_DIRECTORY.NumberOfFunctions]
        jae     .export_ordinal_not_found
; 4. Fetch the exported address from AddressOfFunctions array.
; On success, continue to check for forwarded exports in get_exported_function_by_name.
        mov     ebx, [edx+IMAGE_EXPORT_DIRECTORY.AddressOfFunctions]
        add     ebx, [export_base]
        mov     eax, [ebx+eax*4]
        test    eax, eax
        jnz     get_exported_function_by_name.check_forwarded
; Generic error handler.
.export_ordinal_not_found:
        sub     esp, 16
        fpo_delta = fpo_delta + 16
; Convert ordinal to string.
        lea     edi, [esp+15]
        mov     byte [edi], 0
@@:
        mov     eax, 0xCCCCCCCD
        mul     ecx
        shr     edx, 3 ; edx = quotient of ecx / 10
        lea     eax, [edx*5]
        add     eax, eax
        sub     ecx, eax ; ecx = remainder of ecx % 10
        add     cl, '0'
        dec     edi
        mov     byte [edi], cl
        mov     ecx, edx
        test    edx, edx
        jnz     @b
; Get module name.
        mov     esi, [module]
        test    esi, esi
        jnz     @f
        mutex_lock modules_mutex
        mov     ecx, [export_base]
        call    find_module_by_addr
        mutex_unlock modules_mutex
@@:
        mov     eax, msg_unknown
        test    esi, esi
        jz      @f
        mov     eax, [esi+MODULE.filename]
@@:
        ccall   loader_say_error, msg_export_ordinal_not_found, edi, msg_export_not_found, eax, 0
        add     esp, 16
        fpo_delta = fpo_delta - 16
        xor     eax, eax
        ret
endp
