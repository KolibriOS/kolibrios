; Memory management for USB structures.
; Protocol layer uses the common kernel heap malloc/free.
; Hardware layer has special requirements:
; * memory blocks should be properly aligned
; * memory blocks should not cross page boundary
; Hardware layer allocates fixed-size blocks.
; Thus, the specific allocator is quite easy to write:
; allocate one page, split into blocks, maintain the single-linked
; list of all free blocks in each page.

; Note: size must be a multiple of required alignment.

; Data for one pool: dd pointer to the first page, MUTEX lock.

uglobal
; Structures in UHCI and OHCI have equal sizes.
; Thus, functions and data for allocating/freeing can be shared;
; we keep them here rather than in controller-specific files.
align 4
; Data for UHCI and OHCI endpoints pool.
usb1_ep_first_page      dd      ?
usb1_ep_mutex           MUTEX
; Data for UHCI and OHCI general transfer descriptors pool.
usb_gtd_first_page      dd      ?
usb_gtd_mutex           MUTEX
endg

; sanity check: structures in UHCI and OHCI should be the same for allocation
if (sizeof.ohci_pipe=sizeof.uhci_pipe)&(ohci_pipe.SoftwarePart=uhci_pipe.SoftwarePart)

; Allocates one endpoint structure for UHCI/OHCI.
; Returns pointer to software part (usb_pipe) in eax.
proc usb1_allocate_endpoint
        push    ebx
        mov     ebx, usb1_ep_mutex
        stdcall usb_allocate_common, sizeof.ohci_pipe
        test    eax, eax
        jz      @f
        add     eax, ohci_pipe.SoftwarePart
@@:
        pop     ebx
        ret
endp

; Free one endpoint structure for UHCI/OHCI.
; Stdcall with one argument, pointer to software part (usb_pipe).
proc usb1_free_endpoint
        sub     dword [esp+4], ohci_pipe.SoftwarePart
        jmp     usb_free_common
endp

else
; sanity check continued
.err allocate_endpoint/free_endpoint must be different for OHCI and UHCI
end if

; sanity check: structures in UHCI and OHCI should be the same for allocation
if (sizeof.ohci_gtd=sizeof.uhci_gtd)&(ohci_gtd.SoftwarePart=uhci_gtd.SoftwarePart)

; Allocates one general transfer descriptor structure for UHCI/OHCI.
; Returns pointer to software part (usb_gtd) in eax.
proc usb1_allocate_general_td
        push    ebx
        mov     ebx, usb_gtd_mutex
        stdcall usb_allocate_common, sizeof.ohci_gtd
        test    eax, eax
        jz      @f
        add     eax, ohci_gtd.SoftwarePart
@@:
        pop     ebx
        ret
endp

; Free one general transfer descriptor structure for UHCI/OHCI.
; Stdcall with one argument, pointer to software part (usb_gtd).
proc usb1_free_general_td
        sub     dword [esp+4], ohci_gtd.SoftwarePart
        jmp     usb_free_common
endp

else
; sanity check continued
.err allocate_general_td/free_general_td must be different for OHCI and UHCI
end if

; Allocator for fixed-size blocks: allocate a block.
; [ebx-4] = pointer to the first page, ebx = pointer to MUTEX structure.
proc usb_allocate_common
        push    edi     ; save used register to be stdcall
virtual at esp
        dd      ?       ; saved edi
        dd      ?       ; return address
.size   dd      ?
end virtual
; 1. Take the lock.
        mov     ecx, ebx
        call    mutex_lock
; 2. Find the first allocated page with a free block, if any.
; 2a. Initialize for the loop.
        mov     edx, ebx
.pageloop:
; 2b. Get the next page, keeping the current in eax.
        mov     eax, edx
        mov     edx, [edx-4]
; 2c. If there is no next page, we're out of luck; go to 4.
        test    edx, edx
        jz      .newpage
        add     edx, 0x1000
@@:
; 2d. Get the pointer to the first free block on this page.
; If there is no free block, continue to 2b.
        mov     eax, [edx-8]
        test    eax, eax
        jz      .pageloop
; 2e. Get the pointer to the next free block.
        mov     ecx, [eax]
; 2f. Update the pointer to the first free block from eax to ecx.
; Normally [edx-8] still contains eax, if so, atomically set it to ecx
; and proceed to 3.
; However, the price of simplicity of usb_free_common (in particular, it
; doesn't take the lock) is that [edx-8] could (rarely) be changed while
; we processed steps 2d+2e. If so, return to 2d and retry.
        lock cmpxchg [edx-8], ecx
        jnz     @b
.return:
; 3. Release the lock taken in step 1 and return.
        push    eax
        mov     ecx, ebx
        call    mutex_unlock
        pop     eax
        pop     edi     ; restore used register to be stdcall
        ret     4
.newpage:
; 4. Allocate a new page.
        push    eax
        stdcall kernel_alloc, 0x1000
        pop     edx
; If failed, say something to the debug board and return zero.
        test    eax, eax
        jz      .nomemory
; 5. Add the new page to the tail of list of allocated pages.
        mov     [edx-4], eax
; 6. Initialize two service dwords in the end of page:
; first free block is (start of page) + (block size)
; (we will return first block at (start of page), so consider it allocated),
; no next page.
        mov     edx, eax
        lea     edi, [eax+0x1000-8]
        add     edx, [.size]
        mov     [edi], edx
        and     dword [edi+4], 0
; 7. All blocks starting from edx are free; join them in a single-linked list.
@@:
        mov     ecx, edx
        add     edx, [.size]
        mov     [ecx], edx
        cmp     edx, edi
        jbe     @b
        sub     ecx, [.size]
        and     dword [ecx], 0
; 8. Return (start of page).
        jmp     .return
.nomemory:
        dbgstr 'no memory for USB descriptor'
        xor     eax, eax
        jmp     .return
endp

; Allocator for fixed-size blocks: free a block.
proc usb_free_common
        push    ecx edx
virtual at esp
        rd      2       ; saved registers
        dd      ?       ; return address
.block  dd      ?
end virtual
; Insert the given block to the head of free blocks in this page.
        mov     ecx, [.block]
        mov     edx, ecx
        or      edx, 0xFFF
@@:
        mov     eax, [edx+1-8]
        mov     [ecx], eax
        lock cmpxchg [edx+1-8], ecx
        jnz     @b
        pop     edx ecx
        ret     4
endp

; Helper procedure for OHCI: translate physical address in ecx
; of some transfer descriptor to linear address.
proc usb_td_to_virt
; Traverse all pages used for transfer descriptors, looking for the one
; with physical address as in ecx.
        mov     eax, [usb_gtd_first_page]
@@:
        test    eax, eax
        jz      .zero
        push    eax
        call    get_pg_addr
        sub     eax, ecx
        jz      .found
        cmp     eax, -0x1000
        ja      .found
        pop     eax
        mov     eax, [eax+0x1000-4]
        jmp     @b
.found:
; When found, combine page address from eax with page offset from ecx.
        pop     eax
        and     ecx, 0xFFF
        add     eax, ecx
.zero:
        ret
endp
