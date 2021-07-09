;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2016. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Checks whether the given file is already loaded as PE.
; If so, returns pointer to PEDESCR which can be used for map_pe_usermode.
;   out: eax = 0, ebx undefined
;   out: esi -> PEDESCR
; If not, loads the given file; if it is PE, creates PEDESCR;
;   out: eax -> file data, ebx = file size
;   out: esi -> PEDESCR
; if it is not PE, returns loaded file as is.
;   out: eax -> file data, ebx = file size
;   out: esi = 0
; On error:
;   out: eax = negative error code, ebx = 0
;   out: esi = 0
proc load_file_maybe_pe stdcall uses edi, file_name:dword
    locals
      fileinfo rb 40
      filedata dd ?
      defaultbase dd ?
      entry dd ?
      stacksize dd ?
    endl
        and     [filedata], 0
; 1. Lock the common mutex for PE list.
        mov     ecx, pe_list_mutex
        call    mutex_lock
; 2. Check whether this PE file is already mapped somewhere.
; 2a. Get the timestamp. If failed, pass filesystem error to the caller.
        lea     eax, [fileinfo]
        stdcall get_fileinfo, [file_name], eax
        mov     edi, eax
        neg     edi
        jnz     .fail_unlock
; 2b. Scan the list of PE files, for each file compare timestamp and name.
; If all parameters match, go to 6. Otherwide, proceed to 3.
        mov     esi, [pe_list.fd]
.scan_existing:
        cmp     esi, pe_list
        jz      .load_new
        mov     eax, dword [fileinfo+24]; last modified time
        mov     edx, dword [fileinfo+28]; last modified date
        cmp     dword [esi+PEDESCR.timestamp], eax
        jnz     .continue_scan
        cmp     dword [esi+PEDESCR.timestamp+4], edx
        jnz     .continue_scan
        stdcall strncmp, [esi+PEDESCR.name], [file_name], -1
        test    eax, eax
        jz      .already_loaded
.continue_scan:
        mov     esi, [esi+PEDESCR.fd]
        jmp     .scan_existing

; Either this file is not PE, or it has not been mapped yet.
.load_new:
; 3. Load and unpack file data.
; If failed, return -5 "file not found".
        stdcall load_file, [file_name]
        movi    edi, -5
        test    eax, eax
        jz      .fail_unlock
        mov     [filedata], eax
        mov     dword [fileinfo+32], ebx
; 4. Check that the file is valid PE, has image data and is not too large.
; If not, pass the loaded file to the caller as is.
        cmp     ebx, 40h
        jb      .not_pe
        cmp     word [eax], 'MZ'
        jz      .check_mz
        cmp     [eax+STRIPPED_PE_HEADER.Signature], STRIPPED_PE_SIGNATURE
        jnz     .not_pe
        mov     ecx, [eax+STRIPPED_PE_HEADER.SizeOfStackReserve]
        mov     [stacksize], ecx
        mov     ecx, [eax+STRIPPED_PE_HEADER.ImageBase]
        mov     esi, [eax+STRIPPED_PE_HEADER.SizeOfImage]
        mov     edi, [eax+STRIPPED_PE_HEADER.AddressOfEntryPoint]
        jmp     .pe
.check_mz:
        mov     ecx, [eax+3Ch]
        add     eax, ecx
        add     ecx, IMAGE_NT_HEADERS.OptionalHeader
        jc      .not_pe
        cmp     ecx, ebx
        ja      .not_pe
        cmp     [eax+IMAGE_NT_HEADERS.Signature], 'PE'
        jnz     .not_pe
        movzx   edx, [eax+IMAGE_NT_HEADERS.FileHeader.SizeOfOptionalHeader]
        cmp     edx, IMAGE_OPTIONAL_HEADER32.DataDirectory
        jb      .not_pe
        add     ecx, edx
        jc      .not_pe
        cmp     ecx, ebx
        ja      .not_pe
        cmp     [eax+IMAGE_NT_HEADERS.OptionalHeader.Magic], 10Bh
        jnz     .not_pe
        mov     ecx, [eax+IMAGE_NT_HEADERS.OptionalHeader.SizeOfStackReserve]
        mov     [stacksize], ecx
        mov     ecx, [eax+IMAGE_NT_HEADERS.OptionalHeader.ImageBase]
        mov     esi, [eax+IMAGE_NT_HEADERS.OptionalHeader.SizeOfImage]
        mov     edi, [eax+IMAGE_NT_HEADERS.OptionalHeader.AddressOfEntryPoint]
.pe:
        test    esi, esi
        jz      .not_pe
        cmp     esi, 16*1024*1024
        ja      .not_pe
        mov     [defaultbase], ecx
        mov     [entry], edi
        add     esi, 0xFFF
        shr     esi, 12
; 5. Allocate and initialize PEDESCR structure.
; 5a. Calculate structure size: sizeof.PEDESCR + dword per image page + size of PE name.
        mov     edi, [file_name]
        mov     ecx, -1
        xor     eax, eax
        repnz scasb
        not     ecx
        lea     eax, [ecx+esi*4+sizeof.PEDESCR]
; 5b. Allocate memory.
; If failed, return -30 "no memory".
        push    ecx
        call    malloc
        pop     ecx
        movi    edi, -30
        test    eax, eax
        jz      .fail_and_free_data
; 5c. Initialize PEDESCR structure.
        mov     [eax+PEDESCR.size], esi
        lea     edi, [eax+esi*4+sizeof.PEDESCR]
        mov     esi, [file_name]
        mov     [eax+PEDESCR.name], edi
        rep movsb
        mov     esi, eax
        lea     edi, [eax+sizeof.PEDESCR]
        mov     ecx, [eax+PEDESCR.size]
        xor     eax, eax
        rep stosd
        mov     eax, dword [fileinfo+24]
        mov     dword [esi+PEDESCR.timestamp], eax
        mov     eax, dword [fileinfo+28]
        mov     dword [esi+PEDESCR.timestamp+4], eax
        mov     eax, [defaultbase]
        mov     [esi+PEDESCR.defaultbase], eax
        mov     eax, [entry]
        mov     [esi+PEDESCR.entry], eax
        mov     eax, [stacksize]
        mov     [esi+PEDESCR.stacksize], eax
        and     dword [esi+PEDESCR.refcount], 0; no SMAPs yet; later it will be incremented
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_init
; 5d. Insert PEDESCR structure in tail of the common list.
        mov     [esi+PEDESCR.fd], pe_list
        mov     eax, [pe_list.bk]
        mov     [pe_list.bk], esi
        mov     [esi+PEDESCR.bk], eax
        mov     [eax+PEDESCR.fd], esi
.already_loaded:
; We have got the PEDESCR structure,
; either already-existed from step 2 or created at step 5.
; In the last case we have also got the file data,
; in the first case [filedata] is still zero.
; 6. Increment reference counter in PEDESCR structure.
        inc     [esi+PEDESCR.refcount]
; 7. Release the common mutex for PE list.
; We have got a new reference to our PEDESCR, it will not go away unexpectedly.
        mov     ecx, pe_list_mutex
        call    mutex_unlock
        mov     eax, [filedata]
        mov     ebx, dword [fileinfo+32]
        ret
.fail_and_free_data:
        stdcall kernel_free, [filedata]
.fail_unlock:
        mov     ecx, pe_list_mutex
        call    mutex_unlock
.fail:
        mov     eax, edi
        xor     ebx, ebx
        xor     esi, esi
        ret
.not_pe:
        mov     ecx, pe_list_mutex
        call    mutex_unlock
        mov     eax, [filedata]
        xor     esi, esi
        ret
endp

proc map_pe_usermode stdcall uses ebx esi edi, descr:dword, filedata:dword, filesize:dword
           locals
             img_base     dd ?
             peheader     dd ?
             header_size  dd ?
             num_sections dd ?
             sections     dd ?
             section_idx  dd ?
             page_index   dd ?
             page_offset  dd ?
             cur_page     dd ?
             cur_access   db ?
                          rb 3
             pages        dd ?
             num_allocated_pages dd ?
           endl

; 1. Basic preparations.
; 1a. Check that the process heap has been initialized.
; Return -30 "no memory" if not.
        mov     esi, [descr]
        movi    edi, -30
        mov     eax, [current_process]
        cmp     [eax+PROC.heap_top], 0
        jz      .fail_dereference
; 1b. If filedata is passed, fill the required fields from header now.
        mov     eax, [filedata]
        mov     ebx, [filesize]
        dec     edi ; -30 -> -31
        test    eax, eax
        jz      @f
        call    .validate_header
        jc      .fail_dereference
@@:

; 2. Generate array of pages for mapping in address space in the process.
; It is possible to join this step with step 13, avoiding temporary allocation
; and putting the result directly in the page table, but that would require
; doing potentially long operations like loading/unpacking the file
; while holding address space lock, which could block other threads
; that just want to lazy-allocate their zero-only pages not related to PE.
; So, keep generating and mapping separate.
; 2a. Allocate memory.
        inc     edi ; -31 -> -30
        mov     eax, [esi+PEDESCR.size]
        shl     eax, 2
        call    malloc
        test    eax, eax
        jz      .fail_dereference
        mov     [pages], eax
; 2b. Acquire the lock.
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_lock
; 2c. Prepare for loop over pages: set page index and page offset to zero.
        xor     ecx, ecx
        mov     [page_index], ecx
        mov     [page_offset], ecx
        mov     [num_allocated_pages], ecx
.fill_pages:
; 2d. For each page, test whether we need to regenerate it.
; Pages that need to be regenerated are marked as zero in pages array.
        mov     eax, [esi+sizeof.PEDESCR+ecx*4]
        test    eax, eax
        jz      .create_page
; 2e. For each page that we do not need to regenerate,
; increment reference counter if it is less than 0xFF
; and go to 2t.
        lea     edx, [eax+1]
        test    edx, 0xFF
        jz      .page_created
        mov     [esi+sizeof.PEDESCR+ecx*4], edx
        jmp     .page_created
.create_page:
; 2f. If the file has not been already loaded/unpacked,
; do it now, validating the content.
        cmp     [filedata], 0
        jnz     @f
        stdcall load_file, [esi+PEDESCR.name]
        test    eax, eax
        jz      .fail_free_pages
        call    .validate_header
        jc      .fail_free_pages
@@:
; 2h. Initialize for generating a page:
; do not allocate a page until we will be sure that data are present,
; there are no access rights yet.
        and     [cur_page], 0
        mov     [cur_access], 0
; 2i. Check whether the page overlaps file header.
; If not, go to 2m.
        mov     eax, [page_offset]
        cmp     eax, [header_size]
        jae     .no_header
; 2j. Allocate the page data.
        stdcall kernel_alloc, 0x1000
        test    eax, eax
        jz      .fail_free_pages
        mov     [cur_page], eax
; 2k. Set access rights for header: readonly; copy header data.
        mov     [cur_access], IMAGE_SCN_MEM_READ shr 28
        mov     esi, [filedata]
        mov     edi, eax
        add     esi, [page_offset]
        mov     ecx, [header_size]
        sub     ecx, [page_offset]
        cmp     ecx, 0x1000
        jb      @f
        mov     ecx, 0x1000
@@:
        mov     edx, ecx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
; 2l. Fill the rest of the page with zeroes.
        mov     ecx, 0x1000
        sub     ecx, edx
        mov     edx, ecx
        shr     ecx, 2
        and     edx, 3
        xor     eax, eax
        rep stosd
        mov     ecx, edx
        rep stosb
.no_header:
; 2m. Prepare for loop over sections.
; Skip the loop if there are no sections.
        mov     eax, [num_sections]
        mov     ebx, [sections]
        test    eax, eax
        jz      .no_sections
        mov     [section_idx], eax
.sections_loop:
; 2n. For every section, check whether it has data overlapped with
; the current page; if so, allocate the page if not yet, copy the data
; and fill rest of page with zeroes.
; If data are present, there can be two cases:
; - the current page has data from the beginning,
; - first byte of the current page is not covered by the section.
; The first case is typical, the second case is rare.
; If the page has not been allocated yet, we can optimize by storing zeroes
; only in areas that are not covered by the current section.
; However, this becomes twisted in the second case,
; so don't bother to optimize the rare case.
        cmp     [ebx+COFF_SECTION.SizeOfRawData], 0
        jz      .section_data_done
        mov     esi, [page_offset]
        sub     esi, [ebx+COFF_SECTION.VirtualAddress]
        cmp     esi, [ebx+COFF_SECTION.SizeOfRawData]
        jb      .beginning_inside
        add     esi, 1000h
        jnc     .section_data_done
        jz      .section_data_done
; Rare case: there is an overlap, but the first byte is not covered.
; If the page has not been allocated, allocate it now and store 4K zeroes.
        cmp     [cur_page], 0
        jnz     @f
        stdcall kernel_alloc, 0x1000
        test    eax, eax
        jz      .fail_free_pages
        mov     [cur_page], eax
        mov     edi, eax
        xor     eax, eax
        mov     ecx, 0x1000/4
        rep stosd
@@:
        mov     edi, [ebx+COFF_SECTION.VirtualAddress]
        and     edi, 0xFFF
        xor     esi, esi
.copy_data:
        mov     eax, [ebx+COFF_SECTION.SizeOfRawData]
        sub     eax, esi
        mov     ecx, 0x1000
        sub     ecx, edi
        add     edi, [cur_page]
        cmp     ecx, eax
        jb      @f
        mov     ecx, eax
@@:
        add     esi, [filedata]
        add     esi, [ebx+COFF_SECTION.PtrRawData]
        mov     edx, ecx
        shr     ecx, 2
        and     edx, 3
        rep movsd
        mov     ecx, edx
        rep movsb
        jmp     .section_data_done
.beginning_inside:
; Normal case: do not store zeroes which will be immediately overwritten.
        xor     edi, edi
        cmp     [cur_page], edi
        jnz     .copy_data
        stdcall kernel_alloc, 0x1000
        test    eax, eax
        jz      .fail_free_pages
        mov     [cur_page], eax
        mov     edi, eax
        mov     ecx, [ebx+COFF_SECTION.SizeOfRawData]
        sub     ecx, esi
        cmp     ecx, 0x1000
        jb      @f
        mov     ecx, 0x1000
@@:
        add     esi, [filedata]
        add     esi, [ebx+COFF_SECTION.PtrRawData]
        mov     edx, ecx
        shr     ecx, 2
        rep movsd
        mov     ecx, edx
        and     ecx, 3
        rep movsb
        mov     ecx, 0x1000
        sub     ecx, edx
        mov     edx, ecx
        shr     ecx, 2
        and     edx, 3
        xor     eax, eax
        rep stosd
        mov     ecx, edx
        rep stosb
.section_data_done:
; 2o. Get size of the section header.
; Characteristics is the last dword in both
; COFF_SECTION and STRIPPED_PE_SECTION, so this helps to access it.
        movi    ecx, sizeof.STRIPPED_PE_SECTION
        cmp     [peheader], 0
        jz      @f
        mov     cl, sizeof.COFF_SECTION
@@:
; 2p. If the current page intersects virtual address range of the section,
; update access rights using section access rights.
        cmp     [ebx+COFF_SECTION.VirtualSize], 0
        jz      .section_access_done
        mov     esi, [page_offset]
        sub     esi, [ebx+COFF_SECTION.VirtualAddress]
        cmp     esi, [ebx+COFF_SECTION.VirtualSize]
        jb      @f
        add     esi, 0x1000
        jnc     .section_access_done
        jz      .section_access_done
@@:
        mov     eax, [ebx+ecx-4]
        shr     eax, 28
        or      [cur_access], al
.section_access_done:
; 2q. Advance to the next section, while there are sections left.
        add     ebx, ecx
        dec     [section_idx]
        jnz     .sections_loop
.no_sections:
; 2r. Shareable pages can not be lazy-allocated
; even if they only contain uninitialized data.
; If the page is shareable and has not been allocated yet, do it now.
        test    [cur_access], IMAGE_SCN_MEM_SHARED shr 28
        jz      @f
        cmp     [cur_page], 0
        jnz     @f
        stdcall kernel_alloc, 0x1000
        test    eax, eax
        jz      .fail_free_pages
        mov     [cur_page], eax
        mov     edi, eax
        xor     eax, eax
        mov     ecx, 0x1000/4
        rep stosd
@@:
; 2s. Get and store the item for page array: 0xFF for pages with zeroes,
; physical address of the page plus 1 for reference counter otherwise,
; with access rights in bits 8-11 in both cases.
        mov     edi, 0xFF
        mov     eax, [cur_page]
        test    eax, eax
        jz      @f
        call    get_pg_addr
        lea     edi, [eax+1]
        stdcall free_kernel_space, [cur_page]
@@:
        movzx   eax, [cur_access]
        shl     eax, 8
        or      eax, edi
        mov     ecx, [page_index]
        mov     esi, [descr]
        mov     [esi+sizeof.PEDESCR+ecx*4], eax
.page_created:
; 2t. Transform the item from page array to page table entry:
; - drop reference counter,
; - map zero-only page to LAZY_ALLOC_PAGE
;   with optional flags LAZY_ALLOC_{UNREADABLE,UNWRITABLE},
;   PF handler will lazy-allocate it;
; - for pages with data,
;   map readable and executable to user bit,
;   for shareable pages map writable to writable bit,
;   for non-shareable pages ignore writable to support copy-on-write.
        mov     edx, eax
        and     eax, not 0xFFF
        jz      .page_set_zero
        inc     [num_allocated_pages]
        or      eax, PG_READ+PG_SHARED
        test    dh, (IMAGE_SCN_MEM_READ+IMAGE_SCN_MEM_EXECUTE) shr 28
        jz      @f
        or      al, PG_USER
@@:
        test    dh, IMAGE_SCN_MEM_SHARED shr 28
        jz      @f
        test    dh, IMAGE_SCN_MEM_WRITE shr 28
        jz      @f
        or      al, PG_WRITE
@@:
        jmp     .pte_generated
.page_set_zero:
        mov     al, LAZY_ALLOC_PAGE
        test    dh, (IMAGE_SCN_MEM_READ+IMAGE_SCN_MEM_EXECUTE) shr 28
        jnz     @f
        or      al, LAZY_ALLOC_UNREADABLE
@@:
        test    dh, IMAGE_SCN_MEM_WRITE shr 28
        jnz     @f
        or      al, LAZY_ALLOC_UNWRITABLE
@@:
.pte_generated:
        mov     edi, [pages]
        mov     [edi+ecx*4], eax
; 2u. Advance to the next page, until PEDESCR.size is reached.
        inc     ecx
        inc     [page_index]
        add     [page_offset], 0x1000
        cmp     ecx, [esi+PEDESCR.size]
        jb      .fill_pages
; 2v. Release the lock.
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_unlock
; 3. Allocate a new SMAP.
        movi    eax, sizeof.SMAP
        call    malloc
        test    eax, eax
        jz      .fail_free_pages_unlocked
        mov     ebx, eax
; 4. Lock the address space so that a random PF from other thread
; between end of step 5 and beginning of step 7 would not trash anything.
        mov     ecx, [current_process]
        add     ecx, PROC.heap_lock
        call    mutex_lock
; 5. Allocate space in the address space.
; Prefer PEDESCR.defaultbase, but allow address anywhere else
; if allocation at PEDESCR.defaultbase is not possible.
        mov     edi, [esi+PEDESCR.size]
        shl     edi, 12
        stdcall user_alloc_at_nolock, [esi+PEDESCR.defaultbase], edi
        test    eax, eax
        jnz     @f
        stdcall user_alloc_nolock, edi
        test    eax, eax
        jz      .user_alloc_failed
@@:
        mov     [img_base], eax
; 6. Fill SMAP with values and insert it to the list of SMAPs.
        mov     [ebx+SMAP.type], SMAP_TYPE_PE
        mov     ecx, [current_process]
        add     ecx, PROC.smap_list
        mov     edx, [ecx+SMAP.fd]
        mov     [ebx+SMAP.fd], edx
        mov     [ebx+SMAP.bk], ecx
        mov     [ecx+SMAP.fd], ebx
        mov     [edx+SMAP.bk], ebx
        mov     [ebx+SMAP.base], eax
        mov     [ebx+SMAP.size], edi
        mov     [ebx+SMAP.parent], esi
; 7. Copy page table entries prepared at step 2 to the page table.
        mov     edx, eax
        shr     edx, 12
        mov     ecx, [esi+PEDESCR.size]
        mov     esi, [pages]
        lea     edi, [page_tabs+edx*4]
        rep movsd
        mov     eax, [num_allocated_pages]
        shl     eax, 12
; 8. Release the address space lock.
        mov     ecx, [current_process]
        add     [ecx+PROC.mem_used], eax
        add     ecx, PROC.heap_lock
        call    mutex_unlock
; 9. Cleanup and return allocated address.
        mov     eax, [pages]
        call    free
        cmp     [filedata], 0
        jz      @f
        stdcall kernel_free, [filedata]
@@:
        mov     eax, [img_base]
        ret
.fail_and_free_data:
        stdcall kernel_free, [filedata]
.fail:
        mov     eax, edi
        ret
.user_alloc_failed:
        mov     ecx, [current_process]
        add     ecx, PROC.heap_lock
        call    mutex_unlock
        mov     eax, ebx
        call    free
.fail_free_pages_unlocked:
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_lock
.fail_free_pages:
        mov     ecx, [page_index]
        test    ecx, ecx
        jz      .fail_free_pages_array
        mov     eax, [esi+sizeof.PEDESCR+(ecx-1)*4]
        mov     edx, eax
        and     eax, not 0xFFF
        jz      .fail_free_next
        and     edx, 0xFF
        cmp     edx, 0xFF
        jz      .fail_free_next
        dec     dword [esi+sizeof.PEDESCR+(ecx-1)*4]
        dec     edx
        jnz     .fail_free_next
        mov     [esi+sizeof.PEDESCR+(ecx-1)*4], edx
        call    free_page
.fail_free_next:
        dec     [page_index]
        jmp     .fail_free_pages
.fail_free_pages_array:
        lea     ecx, [esi+PEDESCR.page_array_lock]
        call    mutex_unlock
        mov     eax, [pages]
        call    free
        movi    edi, -30
.fail_dereference:
        cmp     [filedata], 0
        jz      @f
        stdcall kernel_free, [filedata]
@@:
        call    dereference_pe
        mov     eax, edi
        ret

.validate_header:
        mov     [filedata], eax
        cmp     ebx, 40h
        jb      .validate_header.error
        mov     [peheader], 0
        cmp     word [eax], STRIPPED_PE_SIGNATURE
        jz      .validate_header.stripped
        cmp     word [eax], 'MZ'
        jnz     .validate_header.error
        mov     ecx, [eax+3Ch]
        add     eax, ecx
        add     ecx, IMAGE_NT_HEADERS.OptionalHeader
        jc      .validate_header.error
        cmp     ecx, ebx
        ja      .validate_header.error
        cmp     [eax+IMAGE_NT_HEADERS.Signature], 'PE'
        jnz     .validate_header.error
        mov     [peheader], eax
        movzx   edx, [eax+IMAGE_NT_HEADERS.FileHeader.SizeOfOptionalHeader]
        cmp     edx, IMAGE_OPTIONAL_HEADER32.DataDirectory
        jb      .validate_header.error
        add     ecx, edx
        jc      .validate_header.error
        cmp     ecx, ebx
        ja      .validate_header.error
        lea     edx, [eax+IMAGE_NT_HEADERS.OptionalHeader+edx]
        mov     [sections], edx
        movzx   edx, [eax+IMAGE_NT_HEADERS.FileHeader.NumberOfSections]
        mov     [num_sections], edx
        imul    edx, sizeof.COFF_SECTION
        add     ecx, edx
        jc      .validate_header.error
        cmp     ecx, ebx
        ja      .validate_header.error
        mov     edx, [eax+IMAGE_NT_HEADERS.OptionalHeader.SizeOfHeaders]
        mov     [header_size], edx
        cmp     edx, ebx
        ja      .validate_header.error
        mov     edx, [num_sections]
        mov     ecx, [sections]
        test    edx, edx
        jz      .validate_header.sections_ok
@@:
        mov     eax, [ecx+COFF_SECTION.PtrRawData]
        add     eax, [ecx+COFF_SECTION.SizeOfRawData]
        jc      .validate_header.error
        cmp     eax, ebx
        ja      .validate_header.error
        add     ecx, sizeof.COFF_SECTION
        dec     edx
        jnz     @b
.validate_header.sections_ok:
.validate_header.ok:
        clc
        retn
.validate_header.stripped:
        movzx   ecx, [eax+STRIPPED_PE_HEADER.NumberOfRvaAndSizes]
        lea     ecx, [sizeof.STRIPPED_PE_HEADER+ecx*8]
        movzx   edx, [eax+STRIPPED_PE_HEADER.NumberOfSections]
        mov     [num_sections], edx
        imul    edx, sizeof.STRIPPED_PE_SECTION
        add     edx, ecx
        cmp     edx, ebx
        ja      .validate_header.error
        mov     edx, [eax+STRIPPED_PE_HEADER.SizeOfHeaders]
        mov     [header_size], edx
        cmp     edx, ebx
        ja      .validate_header.error
        add     ecx, eax
        mov     [sections], ecx
        mov     edx, [num_sections]
        test    edx, edx
        jz      .validate_header.stripped.sections_ok
@@:
        mov     eax, [ecx+STRIPPED_PE_SECTION.PtrRawData]
        add     eax, [ecx+STRIPPED_PE_SECTION.SizeOfRawData]
        jc      .validate_header.error
        cmp     eax, ebx
        ja      .validate_header.error
        add     ecx, sizeof.STRIPPED_PE_SECTION
        dec     edx
        jnz     @b
.validate_header.stripped.sections_ok:
        clc
        retn
.validate_header.error:
        stc
        retn
endp

; in: edi -> SMAP
; in: address space lock must be held
proc release_pemap stdcall uses ebx esi, process:dword
    locals
        num_released_pages dd 0
        mapped_pagedir dd -1
    endl
        mov     esi, [edi+SMAP.base]
        mov     ebx, [edi+SMAP.parent]
        shr     esi, 12
        dec     esi
        add     ebx, sizeof.PEDESCR
        call    .get_page_tab_entry
        mov     ecx, [eax]
        and     ecx, not MEM_BLOCK_DONT_FREE
        mov     [eax],  ecx
        shr     ecx, 12
        dec     ecx
        jz      .released
.release:
        inc     esi
        call    .get_page_tab_entry
        mov     edx, [eax]
        test    dl, 1
        jz      .next
        test    edx, PG_SHARED
        jz      .next
        mov     dword [eax], 0
        inc     [num_released_pages]
        xor     edx, [ebx]
        test    edx, not 0xFFF
        jnz     .next
        mov     edx, [ebx]
        mov     eax, edx
        and     edx, 0xFF
        cmp     edx, 0xFF
        jz      .next
        dec     eax
        test    dword [ebx], IMAGE_SCN_MEM_SHARED shr 20
        jnz     @f
        test    eax, 0xFF
        jnz     @f
        call    free_page
        xor     eax, eax
@@:
        mov     [ebx], eax
.next:
        add     ebx, 4
        dec     ecx
        jnz     .release
        mov     eax, [num_released_pages]
        shl     eax, 12
        mov     edx, [process]
        sub     [edx+PROC.mem_used], eax
        cmp     [mapped_pagedir], -1
        jz      .released
        stdcall map_page, [tmp_task_ptab], 0, PG_UNMAP
.released:
        ret

.get_page_tab_entry:
        mov     eax, [process]
        cmp     eax, [current_process]
        jnz     @f
        lea     eax, [page_tabs+esi*4]
        retn
@@:
        push    edx
        mov     edx, esi
        shr     edx, 10
        cmp     edx, [mapped_pagedir]
        jz      @f
        mov     [mapped_pagedir], edx
        mov     eax, [eax+PROC.pdt_0+edx*4]
        and     eax, not 0xFFF
        stdcall map_page, [tmp_task_ptab], eax, PG_SWR
@@:
        mov     eax, [tmp_task_ptab]
        mov     edx, esi
        and     edx, 0x3FF
        lea     eax, [eax+edx*4]
        pop     edx
        retn
endp

proc unmap_pe_usermode stdcall uses ebx esi edi, address:dword
        mov     ecx, [current_process]
        lea     edi, [ecx+PROC.smap_list]
        add     ecx, PROC.heap_lock
        call    mutex_lock
        mov     esi, edi
        mov     eax, [address]
.scan:
        mov     edi, [edi+SMAP.fd]
        cmp     edi, esi
        jz      .notfound
        cmp     [edi+SMAP.base], eax
        jnz     .scan
        mov     eax, [edi+SMAP.fd]
        mov     edx, [edi+SMAP.bk]
        mov     [eax+SMAP.bk], edx
        mov     [edx+SMAP.fd], eax
        call    mutex_unlock
        stdcall destroy_smap, [current_process]
        xor     eax, eax
        ret
.notfound:
        call    mutex_unlock
        or      eax, -1
        ret
endp
