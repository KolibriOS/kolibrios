; Code for EHCI controllers.
; Note: it should be moved to an external driver,
; it was convenient to have this code compiled into the kernel during initial
; development, but there are no reasons to keep it here.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; EHCI register declarations.
; Part 1. Capability registers.
; Base is MMIO from the PCI space.
EhciCapLengthReg    = 0
EhciVersionReg      = 2
EhciStructParamsReg = 4
EhciCapParamsReg    = 8
EhciPortRouteReg    = 0Ch
; Part 2. Operational registers.
; Base is (base for part 1) + (value of EhciCapLengthReg).
EhciCommandReg      = 0
EhciStatusReg       = 4
EhciInterruptReg    = 8
EhciFrameIndexReg   = 0Ch
EhciCtrlDataSegReg  = 10h
EhciPeriodicListReg = 14h
EhciAsyncListReg    = 18h
EhciConfigFlagReg   = 40h
EhciPortsReg        = 44h

; Possible values of ehci_pipe.NextQH.Type bitfield.
EHCI_TYPE_ITD  = 0 ; isochronous transfer descriptor
EHCI_TYPE_QH   = 1 ; queue head
EHCI_TYPE_SITD = 2 ; split-transaction isochronous TD
EHCI_TYPE_FSTN = 3 ; frame span traversal node

; =============================================================================
; ================================ Structures =================================
; =============================================================================

; Hardware part of EHCI general transfer descriptor.
struct ehci_hardware_td
NextTD          dd      ?
; Bit 0 is Terminate bit, 1 = there is no next TD.
; Bits 1-4 must be zero.
; With masked 5 lower bits, this is the physical address of the next TD, if any.
AlternateNextTD dd      ?
; Similar to NextTD, used if the transfer terminates with a short packet.
Token           dd      ?
; 1. Lower byte is Status field:
; bit 0 = ping state for USB2 endpoints, ERR handshake signal for USB1 endpoints
; bit 1 = split transaction state, meaningless for USB2 endpoints
; bit 2 = missed micro-frame
; bit 3 = transaction error
; bit 4 = babble detected
; bit 5 = data buffer error
; bit 6 = halted
; bit 7 = active
; 2. Next two bits (bits 8-9) are PID code, 0 = OUT, 1 = IN, 2 = SETUP.
; 3. Next two bits (bits 10-11) is ErrorCounter. Initialized as 3, decremented
;    on each error; if it goes to zero, transaction is stopped.
; 4. Next 3 bits (bits 12-14) are CurrentPage field.
; 5. Next bit (bit 15) is InterruptOnComplete bit.
; 6. Next 15 bits (bits 16-30) are TransferLength field,
;    number of bytes to transfer.
; 7. Upper bit (bit 31) is DataToggle bit.
BufferPointers  rd      5
; The buffer to be transferred can be spanned on up to 5 physical pages.
; The first item of this array is the physical address of the first byte in
; the buffer, other items are physical addresses of next pages. Lower 12 bits
; in other items must be set to zero; ehci_pipe.Overlay reuses some of them.
BufferPointersHigh      rd      5
; Upper dwords of BufferPointers for controllers with 64-bit memory access.
; Always zero.
ends

; EHCI general transfer descriptor.
; * The structure describes transfers to be performed on Control, Bulk or
;   Interrupt endpoints.
; * The structure includes two parts, the hardware part and the software part.
; * The hardware part consists of first 52 bytes and corresponds to
;   the Queue Element Transfer Descriptor from EHCI specification.
; * The hardware requires 32-bytes alignment of the hardware part, so
;   the entire descriptor must be 32-bytes aligned. Since the allocator
;   (usb_allocate_common) allocates memory sequentially from page start
;   (aligned on 0x1000 bytes), size of the structure must be divisible by 32.
; * The hardware also requires that the hardware part must not cross page
;   boundary; the allocator satisfies this automatically.
struct ehci_gtd ehci_hardware_td
Flags                   dd      ?
; Copy of flags from the call to usb_*_transfer_async.
SoftwarePart            rd      sizeof.usb_gtd/4
; Software part, common for all controllers.
                        rd      3       ; padding
ends

if sizeof.ehci_gtd mod 32
.err ehci_gtd must be 32-bytes aligned
end if

; EHCI-specific part of a pipe descriptor.
; * This structure corresponds to the Queue Head from the EHCI specification.
; * The hardware requires 32-bytes alignment of the hardware part.
;   Since the allocator (usb_allocate_common) allocates memory sequentially
;   from page start (aligned on 0x1000 bytes), size of the structure must be
;   divisible by 32.
; * The hardware requires also that the hardware part must not cross page
;   boundary; the allocator satisfies this automatically.
struct ehci_pipe
NextQH                  dd      ?
; 1. First bit (bit 0) is Terminate bit, 1 = there is no next QH.
; 2. Next two bits (bits 1-2) are Type field of the next QH,
;    one of EHCI_TYPE_* constants.
; 3. Next two bits (bits 3-4) are reserved, must be zero.
; 4. With masked 5 lower bits, this is the physical address of the next object
;    to be processed, usually next QH.
Token                   dd      ?
; 1. Lower 7 bits are DeviceAddress field. This is the address of the
;    target device on the USB bus.
; 2. Next bit (bit 7) is Inactivate-on-next-transaction bit. Can be nonzero
;    only for interrupt/isochronous USB1 endpoints.
; 3. Next 4 bits (bits 8-11) are Endpoint field. This is the target endpoint
;    number.
; 4. Next 2 bits (bits 12-13) are EndpointSpeed field, one of EHCI_SPEED_*.
; 5. Next bit (bit 14) is DataToggleControl bit,
;    0 = use DataToggle bit from QH, 1 = from TD.
; 6. Next bit (bit 15) is Head-of-reclamation-list. The head of Control list
;    has 1 here, all other QHs have zero.
; 7. Next 11 bits (bits 16-26) are MaximumPacketLength field for the target
;    endpoint.
; 8. Next bit (bit 27) is ControlEndpoint bit, must be 1 for USB1 control
;    endpoints and 0 for all others.
; 9. Upper 4 bits (bits 28-31) are NakCountReload field.
;    Zero for USB1 endpoints, zero for periodic endpoints.
;    For control/bulk USB2 endpoints, the code sets it to 4,
;    which is rather arbitrary.
Flags                   dd      ?
; 1. Lower byte is S-mask, each bit corresponds to one microframe per frame;
;    bit is set <=> enable transactions in this microframe.
; 2. Next byte is C-mask, each bit corresponds to one microframe per frame;
;    bit is set <=> enable complete-split transactions in this microframe.
;    Meaningful only for USB1 endpoints.
; 3. Next 14 bits give address of the target device as hub:port, bits 16-22
;    are the USB address of the hub, bits 23-29 are the port number.
;    Meaningful only for USB1 endpoints.
; 4. Upper 2 bits define number of consequetive transactions per micro-frame
;    which host is allowed to permit for this endpoint.
;    For control/bulk endpoints, it must be 1.
;    For periodic endpoints, the value is taken from the endpoint descriptor.
HeadTD                  dd      ?
; The physical address of the first TD for this pipe.
; Lower 5 bits must be zero.
Overlay                 ehci_hardware_td        ?
; Working area for the current TD, if there is any.
; When TD is retired, it is written to that TD and Overlay is loaded
; from the new TD, if any.
BaseList                dd      ?
; Pointer to head of the corresponding pipe list.
SoftwarePart            rd      sizeof.usb_pipe/4
; Software part, common for all controllers.
                        rd      2       ; padding
ends

if sizeof.ehci_pipe mod 32
.err ehci_pipe must be 32-bytes aligned
end if

; This structure describes the static head of every list of pipes.
; The hardware requires 32-bytes alignment of this structure.
; All instances of this structure are located sequentially in ehci_controller,
; ehci_controller is page-aligned, so it is sufficient to make this structure
; 32-bytes aligned and verify that the first instance is 32-bytes aligned
; inside ehci_controller.
; The hardware also requires that 44h bytes (size of 64-bit Queue Head
; Descriptor) starting at the beginning of this structure must not cross page
; boundary. If not, most hardware still behaves correctly (in fact, the last
; dword can have any value and this structure is never written), but on some
; hardware some things just break in mysterious ways.
struct ehci_static_ep
; Hardware fields are the same as in ehci_pipe.
; Only NextQH and Overlay.Token are actually used.
; NB: some emulators ignore Token.Halted bit (probably assuming that it is set
; only when device fails and emulation never fails) and always follow
; [Alternate]NextTD when they see that OverlayToken.Active bit is zero;
; so it is important to also set [Alternate]NextTD to 1.
NextQH          dd      ?
Token           dd      ?
Flags           dd      ?
HeadTD          dd      ?
NextTD          dd      ?
AlternateNextTD dd      ?
OverlayToken    dd      ?
NextList        dd      ?
SoftwarePart    rd      sizeof.usb_static_ep/4
Bandwidths      rw      8
                dd      ?
ends

if sizeof.ehci_static_ep mod 32
.err ehci_static_ep must be 32-bytes aligned
end if

if ehci_static_ep.OverlayToken <> ehci_pipe.Overlay.Token
.err ehci_static_ep.OverlayToken misplaced
end if

; EHCI-specific part of controller data.
; * The structure includes two parts, the hardware part and the software part.
; * The hardware part consists of first 4096 bytes and corresponds to
;   the Periodic Frame List from the EHCI specification.
; * The hardware requires page-alignment of the hardware part, so
;   the entire descriptor must be page-aligned.
;   This structure is allocated with kernel_alloc (see usb_init_controller),
;   this gives page-aligned data.
; * The controller is described by both ehci_controller and usb_controller
;   structures, for each controller there is one ehci_controller and one
;   usb_controller structure. These structures are located sequentially
;   in the memory: beginning from some page start, there is ehci_controller
;   structure - this enforces hardware alignment requirements - and then
;   usb_controller structure.
; * The code keeps pointer to usb_controller structure. The ehci_controller
;   structure is addressed as [ptr + ehci_controller.field - sizeof.ehci_controller].
struct ehci_controller
; ------------------------------ hardware fields ------------------------------
FrameList               rd      1024
; Entry n corresponds to the head of the frame list to be executed in
; the frames n,n+1024,n+2048,n+3096,...
; The first bit of each entry is Terminate bit, 1 = the frame is empty.
; Bits 1-2 are Type field, one of EHCI_TYPE_* constants.
; Bits 3-4 must be zero.
; With masked 5 lower bits, the entry is a physical address of the first QH/TD
; to be executed.
; ------------------------------ software fields ------------------------------
; Every list has the static head, which is an always halted QH.
; The following fields are static heads, one per list:
; 32+16+8+4+2+1 = 63 for Periodic lists, 1 for Control list and 1 for Bulk list.
IntEDs                  ehci_static_ep
                        rb      62 * sizeof.ehci_static_ep
; Beware.
; Two following strings ensure that 44h bytes at any static head
; do not cross page boundary. Without that, the code "works on my machine"...
; but fails on some hardware in seemingly unrelated ways.
; One hardware TD (without any software fields) fit in the rest of the page.
ehci_controller.ControlDelta = 2000h - (ehci_controller.IntEDs + 63 * sizeof.ehci_static_ep)
StopQueueTD             ehci_hardware_td
; Used as AlternateNextTD for transfers when short packet is considered
; as an error; short packet must stop the queue in this case, not advance
; to the next transfer.
                        rb      ehci_controller.ControlDelta - sizeof.ehci_hardware_td
; Padding for page-alignment.
ControlED               ehci_static_ep
BulkED                  ehci_static_ep
MMIOBase1               dd      ?
; Virtual address of memory-mapped area with part 1 of EHCI registers EhciXxxReg.
MMIOBase2               dd      ?
; Pointer inside memory-mapped area MMIOBase1; points to part 2 of EHCI registers.
StructuralParams        dd      ?
; Copy of EhciStructParamsReg value.
CapabilityParams        dd      ?
; Copy of EhciCapParamsReg value.
DeferredActions         dd      ?
; Bitmask of events from EhciStatusReg which were observed by the IRQ handler
; and needs to be processed in the IRQ thread.
ends

if ehci_controller.IntEDs mod 32
.err Static endpoint descriptors must be 32-bytes aligned inside ehci_controller
end if

; Description of #HCI-specific data and functions for
; controller-independent code.
; Implements the structure usb_hardware_func from hccommon.inc for EHCI.
iglobal
align 4
ehci_hardware_func:
        dd      'EHCI'
        dd      sizeof.ehci_controller
        dd      ehci_init
        dd      ehci_process_deferred
        dd      ehci_set_device_address
        dd      ehci_get_device_address
        dd      ehci_port_disable
        dd      ehci_new_port.reset
        dd      ehci_set_endpoint_packet_size
        dd      ehci_alloc_pipe
        dd      ehci_free_pipe
        dd      ehci_init_pipe
        dd      ehci_unlink_pipe
        dd      ehci_alloc_td
        dd      ehci_free_td
        dd      ehci_alloc_transfer
        dd      ehci_insert_transfer
        dd      ehci_new_device
endg

; =============================================================================
; =================================== Code ====================================
; =============================================================================

; Controller-specific initialization function.
; Called from usb_init_controller. Initializes the hardware and
; EHCI-specific parts of software structures.
; eax = pointer to ehci_controller to be initialized
; [ebp-4] = pcidevice
proc ehci_init
; inherit some variables from the parent (usb_init_controller)
.devfn   equ ebp - 4
.bus     equ ebp - 3
; 1. Store pointer to ehci_controller for further use.
        push    eax
        mov     edi, eax
        mov     esi, eax
; 2. Initialize ehci_controller.FrameList.
; Note that FrameList is located in the beginning of ehci_controller,
; so esi and edi now point to ehci_controller.FrameList.
; First 32 entries of FrameList contain physical addresses
; of first 32 Periodic static heads, further entries duplicate these.
; See the description of structures for full info.
; 2a. Get physical address of first static head.
; Note that 1) it is located in the beginning of a page
; and 2) first 32 static heads fit in the same page,
; so one call to get_phys_addr without correction of lower 12 bits
; is sufficient.
if (ehci_controller.IntEDs / 0x1000) <> ((ehci_controller.IntEDs + 32 * sizeof.ehci_static_ep) / 0x1000)
.err assertion failed
end if
if (ehci_controller.IntEDs mod 0x1000) <> 0
.err assertion failed
end if
        add     eax, ehci_controller.IntEDs
        call    get_phys_addr
; 2b. Fill first 32 entries.
        inc     eax
        inc     eax     ; set Type to EHCI_TYPE_QH
        push    32
        pop     ecx
        mov     edx, ecx
@@:
        stosd
        add     eax, sizeof.ehci_static_ep
        loop    @b
; 2c. Fill the rest entries.
        mov     ecx, 1024 - 32
        rep movsd
; 3. Initialize static heads ehci_controller.*ED.
; Use the loop over groups: first group consists of first 32 Periodic
; descriptors, next group consists of next 16 Periodic descriptors,
; ..., last group consists of the last Periodic descriptor.
; 3a. Prepare for the loop.
; make esi point to the second group, other registers are already set.
        add     esi, 32*4 + 32*sizeof.ehci_static_ep
; 3b. Loop over groups. On every iteration:
; edx = size of group, edi = pointer to the current group,
; esi = pointer to the next group.
.init_static_eds:
; 3c. Get the size of next group.
        shr     edx, 1
; 3d. Exit the loop if there is no next group.
        jz      .init_static_eds_done
; 3e. Initialize the first half of the current group.
; Advance edi to the second half.
        push    esi
        call    ehci_init_static_ep_group
        pop     esi
; 3f. Initialize the second half of the current group
; with the same values.
; Advance edi to the next group, esi/eax to the next of the next group.
        call    ehci_init_static_ep_group
        jmp     .init_static_eds
.init_static_eds_done:
; 3g. Initialize the last static head.
        xor     esi, esi
        call    ehci_init_static_endpoint
; While we are here, initialize StopQueueTD.
if (ehci_controller.StopQueueTD <> ehci_controller.IntEDs + 63 * sizeof.ehci_static_ep)
.err assertion failed
end if
        inc     [edi+ehci_hardware_td.NextTD]   ; 0 -> 1
        inc     [edi+ehci_hardware_td.AlternateNextTD]  ; 0 -> 1
; leave other fields as zero, including Active bit
; 3i. Initialize the head of Control list.
        add     edi, ehci_controller.ControlDelta
        lea     esi, [edi+sizeof.ehci_static_ep]
        call    ehci_init_static_endpoint
        or      byte [edi-sizeof.ehci_static_ep+ehci_static_ep.Token+1], 80h
; 3j. Initialize the head of Bulk list.
        sub     esi, sizeof.ehci_static_ep
        call    ehci_init_static_endpoint
; 4. Create a virtual memory area to talk with the controller.
; 4a. Enable memory & bus master access.
        mov     ch, [.bus]
        mov     cl, 1
        mov     eax, ecx
        mov     bh, [.devfn]
        mov     bl, 4
        call    pci_read_reg
        or      al, 6
        xchg    eax, ecx
        call    pci_write_reg
; 4b. Read memory base address.
        mov     ah, [.bus]
        mov     al, 2
        mov     bl, 10h
        call    pci_read_reg
;       DEBUGF 1,'K : phys MMIO %x\n',eax
        and     al, not 0Fh
; 4c. Create mapping for physical memory. 200h bytes are always sufficient.
        stdcall map_io_mem, eax, 200h, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .fail
;       DEBUGF 1,'K : MMIO %x\n',eax
if ehci_controller.MMIOBase1 <> ehci_controller.BulkED + sizeof.ehci_static_ep
.err assertion failed
end if
        stosd   ; fill ehci_controller.MMIOBase1
        movzx   ecx, byte [eax+EhciCapLengthReg]
        mov     edx, [eax+EhciCapParamsReg]
        mov     ebx, [eax+EhciStructParamsReg]
        add     eax, ecx
if ehci_controller.MMIOBase2 <> ehci_controller.MMIOBase1 + 4
.err assertion failed
end if
        stosd   ; fill ehci_controller.MMIOBase2
if ehci_controller.StructuralParams <> ehci_controller.MMIOBase2 + 4
.err assertion failed
end if
if ehci_controller.CapabilityParams <> ehci_controller.StructuralParams + 4
.err assertion failed
end if
        mov     [edi], ebx      ; fill ehci_controller.StructuralParams
        mov     [edi+4], edx    ; fill ehci_controller.CapabilityParams
        DEBUGF 1,'K : HCSPARAMS=%x, HCCPARAMS=%x\n',ebx,edx
        and     ebx, 15
        mov     [edi+usb_controller.NumPorts+sizeof.ehci_controller-ehci_controller.StructuralParams], ebx
        mov     edi, eax
; now edi = MMIOBase2
; 6. Transfer the controller to a known state.
; 6b. Stop the controller if it is running.
        push    10
        pop     ecx
        test    dword [edi+EhciStatusReg], 1 shl 12
        jnz     .stopped
        and     dword [edi+EhciCommandReg], not 1
@@:
        push    1
        pop     esi
        call    delay_ms
        test    dword [edi+EhciStatusReg], 1 shl 12
        jnz     .stopped
        loop    @b
        dbgstr 'Failed to stop EHCI controller'
        jmp     .fail_unmap
.stopped:
; 6c. Reset the controller. Wait up to 50 ms checking status every 1 ms.
        or      dword [edi+EhciCommandReg], 2
        push    50
        pop     ecx
@@:
        push    1
        pop     esi
        call    delay_ms
        test    dword [edi+EhciCommandReg], 2
        jz      .reset_ok
        loop    @b
        dbgstr 'Failed to reset EHCI controller'
        jmp     .fail_unmap
.reset_ok:
; 7. Configure the controller.
        pop     esi     ; restore the pointer saved at step 1
        add     esi, sizeof.ehci_controller
; 7a. If the controller is 64-bit, say to it that all structures are located
; in first 4G.
        test    byte [esi+ehci_controller.CapabilityParams-sizeof.ehci_controller], 1
        jz      @f
        mov     dword [edi+EhciCtrlDataSegReg], 0
@@:
; 7b. Hook interrupt and enable appropriate interrupt sources.
        mov     ah, [.bus]
        mov     al, 0
        mov     bh, [.devfn]
        mov     bl, 3Ch
        call    pci_read_reg
; al = IRQ
        DEBUGF 1,'K : attaching to IRQ %x\n',al
        movzx   eax, al
        stdcall attach_int_handler, eax, ehci_irq, esi
;       mov     dword [edi+EhciStatusReg], 111111b      ; clear status
; disable Frame List Rollover interrupt, enable all other sources
        mov     dword [edi+EhciInterruptReg], 110111b
; 7c. Inform the controller of the address of periodic lists head.
        lea     eax, [esi-sizeof.ehci_controller]
        call    get_phys_addr
        mov     dword [edi+EhciPeriodicListReg], eax
; 7d. Inform the controller of the address of asynchronous lists head.
        lea     eax, [esi+ehci_controller.ControlED-sizeof.ehci_controller]
        call    get_phys_addr
        mov     dword [edi+EhciAsyncListReg], eax
; 7e. Configure operational details and run the controller.
        mov     dword [edi+EhciCommandReg], \
                (1 shl 16) + \ ; interrupt threshold = 1 microframe = 0.125ms
                (0 shl 11) + \ ; disable Async Park Mode
                (0 shl 8) +  \ ; zero Async Park Mode Count
                (1 shl 5) +  \ ; Async Schedule Enable
                (1 shl 4) +  \ ; Periodic Schedule Enable
                (0 shl 2) +  \ ; 1024 elements in FrameList
                1              ; Run
; 7f. Route all ports to this controller, not companion controllers.
        mov     dword [edi+EhciConfigFlagReg], 1
        DEBUGF 1,'K : EHCI controller at %x:%x with %d ports initialized\n',[.bus]:2,[.devfn]:2,[esi+usb_controller.NumPorts]
; 8. Apply port power, if needed, and disable all ports.
        xor     ecx, ecx
@@:
        mov     dword [edi+EhciPortsReg+ecx*4], 1000h   ; Port Power enabled, all other bits disabled
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      @b
        test    byte [esi+ehci_controller.StructuralParams-sizeof.ehci_controller], 10h
        jz      @f
        push    esi
        push    20
        pop     esi
        call    delay_ms
        pop     esi
@@:
        DEBUGF 1,'K : EHCI %x: command = %x, status = %x\n',esi,[edi+EhciCommandReg],[edi+EhciStatusReg]
; 9. Return pointer to usb_controller.
        xchg    eax, esi
        ret
; On error, pop the pointer saved at step 1 and return zero.
; Note that the main code branch restores the stack at step 7 and never fails
; after step 7.
.fail_unmap:
        pop     eax
        push    eax
        stdcall free_kernel_space, [eax+ehci_controller.MMIOBase1]
.fail:
        pop     ecx
        xor     eax, eax
        ret
endp

; Helper procedure for step 3 of ehci_init, see comments there.
; Initializes the static head of one list.
; esi = pointer to the "next" list, edi = pointer to head to initialize.
; Advances edi to the next head, keeps esi.
proc ehci_init_static_endpoint
        xor     eax, eax
        inc     eax     ; set Terminate bit
        mov     [edi+ehci_static_ep.NextTD], eax
        mov     [edi+ehci_static_ep.AlternateNextTD], eax
        test    esi, esi
        jz      @f
        mov     eax, esi
        call    get_phys_addr
        inc     eax
        inc     eax     ; set Type to EHCI_TYPE_QH
@@:
        mov     [edi+ehci_static_ep.NextQH], eax
        mov     [edi+ehci_static_ep.NextList], esi
        mov     byte [edi+ehci_static_ep.OverlayToken], 1 shl 6 ; halted
        add     edi, ehci_static_ep.SoftwarePart
        call    usb_init_static_endpoint
        add     edi, sizeof.ehci_static_ep - ehci_static_ep.SoftwarePart
        ret
endp

; Helper procedure for step 3 of ehci_init, see comments there.
; Initializes one half of group of static heads.
; edx = size of the next group = half of size of the group,
; edi = pointer to the group, esi = pointer to the next group.
; Advances esi, edi to next group, keeps edx.
proc ehci_init_static_ep_group
        push    edx
@@:
        call    ehci_init_static_endpoint
        add     esi, sizeof.ehci_static_ep
        dec     edx
        jnz     @b
        pop     edx
        ret
endp

; Controller-specific pre-initialization function: take ownership from BIOS.
; Some BIOSes, although not all of them, use USB controllers themselves
; to support USB flash drives. In this case,
; we must notify the BIOS that we don't need that emulation and know how to
; deal with USB devices.
proc ehci_kickoff_bios
; 1. Get the physical address of MMIO registers.
        mov     ah, [esi+PCIDEV.bus]
        mov     bh, [esi+PCIDEV.devfn]
        mov     al, 2
        mov     bl, 10h
        call    pci_read_reg
        and     al, not 0Fh
; 2. Create mapping for physical memory. 200h bytes are always sufficient.
        stdcall map_io_mem, eax, 200h, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .nothing
        push    eax     ; push argument for step 8
; 3. Some BIOSes enable controller interrupts as a result of giving
; controller away. At this point the system knows nothing about how to serve
; EHCI interrupts, so such an interrupt will send the system into an infinite
; loop handling the same IRQ again and again. Thus, we need to block EHCI
; interrupts. We can't do this at the controller level until step 5,
; because the controller is currently owned by BIOS, so we block all hardware
; interrupts on this processor until step 5.
        pushf
        cli
; 4. Take the ownership over the controller.
; 4a. Locate take-ownership capability in the PCI configuration space.
; Limit the loop with 100h iterations; since the entire configuration space is
; 100h bytes long, hitting this number of iterations means that something is
; corrupted.
; Use a value from MMIO as a starting point.
        mov     edx, [eax+EhciCapParamsReg]
        DEBUGF 1,'K : edx=%x\n',edx
        movzx   edi, byte [eax+EhciCapLengthReg]
        add     edi, eax
        push    0
        mov     bl, dh          ; get Extended Capabilities Pointer
        test    bl, bl
        jz      .has_ownership2
        cmp     bl, 40h
        jb      .no_capability
.look_bios_handoff:
        test    bl, 3
        jnz     .no_capability
; In each iteration, read the current dword,
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 2
        mov     bh, [esi+PCIDEV.devfn]
        call    pci_read_reg
; check, whether the capability ID is take-ownership ID = 1,
        cmp     al, 1
        jz      .found_bios_handoff
; if not, advance to next-capability link and continue loop.
        dec     byte [esp]
        jz      .no_capability
        mov     bl, ah
        cmp     bl, 40h
        jae     .look_bios_handoff
.no_capability:
        dbgstr 'warning: cannot locate take-ownership capability'
        jmp     .has_ownership2
.found_bios_handoff:
; 4b. Check whether BIOS has ownership.
; Some BIOSes release ownership before loading OS, but forget to unwatch for
; change-ownership requests; they cannot handle ownership request, so
; such a request sends the system into infinite loop of handling the same SMI
; over and over. Avoid this.
        inc     ebx
        inc     ebx
        test    eax, 0x10000
        jz      .has_ownership
; 4c. Request ownership.
        inc     ebx
        mov     cl, 1
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 0
        call    pci_write_reg
; 4d. Some BIOSes set ownership flag, but forget to watch for change-ownership
; requests; if so, there is no sense in waiting.
        inc     ebx
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 2
        call    pci_read_reg
        dec     ebx
        dec     ebx
        test    ah, 20h
        jz      .force_ownership
; 4e. Wait for result no more than 1 s, checking for status every 1 ms.
; If successful, go to 5.
        mov     dword [esp], 1000
@@:
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 0
        call    pci_read_reg
        test    al, 1
        jz      .has_ownership
        push    esi
        push    1
        pop     esi
        call    delay_ms
        pop     esi
        dec     dword [esp]
        jnz     @b
        dbgstr  'warning: taking EHCI ownership from BIOS timeout'
.force_ownership:
; 4f. BIOS has not responded within the timeout.
; Let's just clear BIOS ownership flag and hope that everything will be ok.
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 0
        mov     cl, 0
        call    pci_write_reg
.has_ownership:
; 5. Just in case clear all SMI event sources except change-ownership.
        dbgstr 'has_ownership'
        inc     ebx
        inc     ebx
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 2
        mov     ecx, eax
        call    pci_read_reg
        and     ax, 2000h
        xchg    eax, ecx
        call    pci_write_reg
.has_ownership2:
        pop     ecx
; 6. Disable all controller interrupts until the system will be ready to
; process them.
        mov     dword [edi+EhciInterruptReg], 0
; 7. Now we can unblock interrupts in the processor.
        popf
; 8. Release memory mapping created in step 2 and return.
        call    free_kernel_space
.nothing:
        ret
endp

; IRQ handler for EHCI controllers.
ehci_irq.noint:
        spin_unlock_irqrestore [esi+usb_controller.WaitSpinlock]
; Not our interrupt: restore registers and return zero.
        xor     eax, eax
        pop     edi esi ebx
        ret

proc ehci_irq
        push    ebx esi edi     ; save registers to be cdecl
virtual at esp
        rd      3       ; saved registers
        dd      ?       ; return address
.controller     dd      ?
end virtual
; 1. ebx will hold whether some deferred processing is needed,
; that cannot be done from the interrupt handler. Initialize to zero.
        xor     ebx, ebx
; 2. Get the mask of events which should be processed.
        mov     esi, [.controller]
        mov     edi, [esi+ehci_controller.MMIOBase2-sizeof.ehci_controller]
        spin_lock_irqsave [esi+usb_controller.WaitSpinlock]
        mov     eax, [edi+EhciStatusReg]
        mov     ecx, eax
;       DEBUGF 1,'K : [%d] EHCI status %x\n',[timer_ticks],eax
; 3. Check whether that interrupt has been generated by our controller.
; (One IRQ can be shared by several devices.)
        and     eax, [edi+EhciInterruptReg]
        jz      .noint
; 4. Clear the events we know of.
; Note that this should be done before processing of events:
; new events could arise while we are processing those, this way we won't lose
; them (the controller would generate another interrupt after completion
; of this one).
        DEBUGF 1,'K : EHCI %x interrupt: status = %x, enable = %x\n',esi,ecx,[edi+EhciInterruptReg]
;       DEBUGF 1,'K : EHCI interrupt: status = %x\n',eax
        mov     [edi+EhciStatusReg], eax
; 5. Sanity check.
        test    al, 10h
        jz      @f
        DEBUGF 1,'K : something terrible happened with EHCI %x (%x)\n',esi,al
@@:
; We can't do too much from an interrupt handler. Inform the processing thread
; that it should perform appropriate actions.
        or      [esi+ehci_controller.DeferredActions-sizeof.ehci_controller], eax
        spin_unlock_irqrestore [esi+usb_controller.WaitSpinlock]
        inc     ebx
        call    usb_wakeup_if_needed
; 6. Interrupt processed; return non-zero.
        mov     al, 1
        pop     edi esi ebx     ; restore used registers to be cdecl
        ret
endp

; This procedure is called from usb_set_address_callback
; and stores USB device address in the ehci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe, cl = address
proc ehci_set_device_address
        mov     byte [ebx+ehci_pipe.Token-ehci_pipe.SoftwarePart], cl
        call    usb_subscribe_control
        ret
endp

; This procedure returns USB device address from the ehci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe
; out: eax = endpoint address
proc ehci_get_device_address
        mov     eax, [ebx+ehci_pipe.Token-ehci_pipe.SoftwarePart]
        and     eax, 7Fh
        ret
endp

; This procedure is called from usb_set_address_callback
; if the device does not accept SET_ADDRESS command and needs
; to be disabled at the port level.
; in: esi -> usb_controller, ecx = port (zero-based)
proc ehci_port_disable
        mov     eax, [esi+ehci_controller.MMIOBase2-sizeof.ehci_controller]
        and     dword [eax+EhciPortsReg+ecx*4], not (4 or 2Ah)
        ret
endp

; This procedure is called from usb_get_descr8_callback when
; the packet size for zero endpoint becomes known and
; stores the packet size in ehci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe, ecx = packet size
proc ehci_set_endpoint_packet_size
        mov     eax, [ebx+ehci_pipe.Token-ehci_pipe.SoftwarePart]
        and     eax, not (0x7FF shl 16)
        shl     ecx, 16
        or      eax, ecx
        mov     [ebx+ehci_pipe.Token-ehci_pipe.SoftwarePart], eax
; Wait until hardware cache is evicted.
        call    usb_subscribe_control
        ret
endp

uglobal
align 4
; Data for memory allocator, see memory.inc.
ehci_ep_first_page      dd      ?
ehci_ep_mutex           MUTEX
ehci_gtd_first_page     dd      ?
ehci_gtd_mutex          MUTEX
endg

; This procedure allocates memory for pipe.
; Both hardware+software parts must be allocated, returns pointer to usb_pipe
; (software part).
proc ehci_alloc_pipe
        push    ebx
        mov     ebx, ehci_ep_mutex
        stdcall usb_allocate_common, sizeof.ehci_pipe
        test    eax, eax
        jz      @f
        add     eax, ehci_pipe.SoftwarePart
@@:
        pop     ebx
        ret
endp

; This procedure frees memory for pipe allocated by ehci_alloc_pipe.
; void stdcall with one argument = pointer to usb_pipe.
proc ehci_free_pipe
virtual at esp
        dd      ?       ; return address
.ptr    dd      ?
end virtual
        sub     [.ptr], ehci_pipe.SoftwarePart
        jmp     usb_free_common
endp

; This procedure is called from API usb_open_pipe and processes
; the controller-specific part of this API. See docs.
; in: edi -> usb_pipe for target, ecx -> usb_pipe for config pipe,
; esi -> usb_controller, eax -> usb_gtd for the first TD,
; [ebp+12] = endpoint, [ebp+16] = maxpacket, [ebp+20] = type
proc ehci_init_pipe
virtual at ebp+8
.config_pipe    dd      ?
.endpoint       dd      ?
.maxpacket      dd      ?
.type           dd      ?
.interval       dd      ?
end virtual
; 1. Zero all fields in the hardware part.
        push    eax ecx
        sub     edi, ehci_pipe.SoftwarePart
        xor     eax, eax
        push    ehci_pipe.SoftwarePart/4
        pop     ecx
        rep stosd
        pop     ecx eax
; 2. Setup PID in the first TD and make sure that the it is not active.
        xor     edx, edx
        test    byte [.endpoint], 80h
        setnz   dh
        mov     [eax+ehci_gtd.Token-ehci_gtd.SoftwarePart], edx
        mov     [eax+ehci_gtd.NextTD-ehci_gtd.SoftwarePart], 1
        mov     [eax+ehci_gtd.AlternateNextTD-ehci_gtd.SoftwarePart], 1
; 3. Store physical address of the first TD.
        sub     eax, ehci_gtd.SoftwarePart
        call    get_phys_addr
        mov     [edi+ehci_pipe.Overlay.NextTD-ehci_pipe.SoftwarePart], eax
; 4. Fill ehci_pipe.Flags except for S- and C-masks.
; Copy location from the config pipe.
        mov     eax, [ecx+ehci_pipe.Flags-ehci_pipe.SoftwarePart]
        and     eax, 3FFF0000h
; Use 1 requests per microframe for control/bulk endpoints,
; use value from the endpoint descriptor for periodic endpoints
        push    1
        pop     edx
        test    [.type], 1
        jz      @f
        mov     edx, [.maxpacket]
        shr     edx, 11
        inc     edx
@@:
        shl     edx, 30
        or      eax, edx
        mov     [edi+ehci_pipe.Flags-ehci_pipe.SoftwarePart], eax
; 5. Fill ehci_pipe.Token.
        mov     eax, [ecx+ehci_pipe.Token-ehci_pipe.SoftwarePart]
; copy following fields from the config pipe:
; DeviceAddress, EndpointSpeed, ControlEndpoint if new type is control
        mov     ecx, eax
        and     eax, 307Fh
        and     ecx, 8000000h
        or      ecx, 4000h
        mov     edx, [.endpoint]
        and     edx, 15
        shl     edx, 8
        or      eax, edx
        mov     edx, [.maxpacket]
        shl     edx, 16
        or      eax, edx
; for control endpoints, use DataToggle from TD, otherwise use DataToggle from QH
        cmp     [.type], CONTROL_PIPE
        jnz     @f
        or      eax, ecx
@@:
; for control/bulk USB2 endpoints, set NakCountReload to 4
        test    eax, USB_SPEED_HS shl 12
        jz      .nonak
        cmp     [.type], CONTROL_PIPE
        jz      @f
        cmp     [.type], BULK_PIPE
        jnz     .nonak
@@:
        or      eax, 40000000h
.nonak:
        mov     [edi+ehci_pipe.Token-ehci_pipe.SoftwarePart], eax
; 5. Select the corresponding list and insert to the list.
; 5a. Use Control list for control pipes, Bulk list for bulk pipes.
        lea     edx, [esi+ehci_controller.ControlED.SoftwarePart-sizeof.ehci_controller]
        cmp     [.type], BULK_PIPE
        jb      .insert ; control pipe
        lea     edx, [esi+ehci_controller.BulkED.SoftwarePart-sizeof.ehci_controller]
        jz      .insert ; bulk pipe
.interrupt_pipe:
; 5b. For interrupt pipes, let the scheduler select the appropriate list
; and the appropriate microframe(s) (which goes to S-mask and C-mask)
; based on the current bandwidth distribution and the requested bandwidth.
; There are two schedulers, one for high-speed devices,
; another for split transactions.
; This could fail if the requested bandwidth is not available;
; if so, return an error.
        test    word [edi+ehci_pipe.Flags-ehci_pipe.SoftwarePart+2], 3FFFh
        jnz     .interrupt_fs
        call    ehci_select_hs_interrupt_list
        jmp     .interrupt_common
.interrupt_fs:
        call    ehci_select_fs_interrupt_list
.interrupt_common:
        test    edx, edx
        jz      .return0
        mov     word [edi+ehci_pipe.Flags-ehci_pipe.SoftwarePart], ax
.insert:
        mov     [edi+ehci_pipe.BaseList-ehci_pipe.SoftwarePart], edx
; Insert to the head of the corresponding list.
; Note: inserting to the head guarantees that the list traverse in
; ehci_process_updated_schedule, once started, will not interact with new pipes.
; However, we still need to ensure that links in the new pipe (edi.NextVirt)
; are initialized before links to the new pipe (edx.NextVirt).
; 5c. Insert in the list of virtual addresses.
        mov     ecx, [edx+usb_pipe.NextVirt]
        mov     [edi+usb_pipe.NextVirt], ecx
        mov     [edi+usb_pipe.PrevVirt], edx
        mov     [ecx+usb_pipe.PrevVirt], edi
        mov     [edx+usb_pipe.NextVirt], edi
; 5d. Insert in the hardware list: copy previous NextQH to the new pipe,
; store the physical address of the new pipe to previous NextQH.
        mov     ecx, [edx+ehci_static_ep.NextQH-ehci_static_ep.SoftwarePart]
        mov     [edi+ehci_pipe.NextQH-ehci_pipe.SoftwarePart], ecx
        lea     eax, [edi-ehci_pipe.SoftwarePart]
        call    get_phys_addr
        inc     eax
        inc     eax
        mov     [edx+ehci_static_ep.NextQH-ehci_static_ep.SoftwarePart], eax
; 6. Return with nonzero eax.
        ret
.return0:
        xor     eax, eax
        ret
endp

; This function is called from ehci_process_deferred when
; a new device was connected at least USB_CONNECT_DELAY ticks
; and therefore is ready to be configured.
; ecx = port, esi -> ehci_controller, edi -> EHCI MMIO
proc ehci_new_port
; 1. If the device operates at low-speed, just release it to a companion.
        mov     eax, [edi+EhciPortsReg+ecx*4]
        DEBUGF 1,'K : [%d] EHCI %x port %d state is %x\n',[timer_ticks],esi,ecx,eax
        mov     edx, eax
        and     ah, 0Ch
        cmp     ah, 4
        jz      .low_speed
; 2. Devices operating at full-speed and high-speed must now have ah == 8.
; Some broken hardware asserts both D+ and D- even after initial decoupling;
; if so, stop initialization here, no sense in further actions.
        cmp     ah, 0Ch
        jz      .se1
; 3. If another port is resetting right now, mark this port as 'reset pending'
; and return.
        bts     [esi+usb_controller.PendingPorts], ecx
        cmp     [esi+usb_controller.ResettingPort], -1
        jnz     .nothing
        btr     [esi+usb_controller.PendingPorts], ecx
; Otherwise, fall through to ohci_new_port.reset.

; This function is called from ehci_new_port and usb_test_pending_port.
; It starts reset signalling for the port. Note that in USB first stages
; of configuration can not be done for several ports in parallel.
.reset:
        push    edi
        mov     edi, [esi+ehci_controller.MMIOBase2-sizeof.ehci_controller]
        mov     eax, [edi+EhciPortsReg+ecx*4]
; 1. Store information about resetting hub (roothub) and port.
        and     [esi+usb_controller.ResettingHub], 0
        mov     [esi+usb_controller.ResettingPort], cl
; 2. Initiate reset signalling.
        or      ah, 1
        and     al, not (4 or 2Ah)
        mov     [edi+EhciPortsReg+ecx*4], eax
; 3. Store the current time and set status to 1 = reset signalling active.
        mov     eax, [timer_ticks]
        mov     [esi+usb_controller.ResetTime], eax
        mov     [esi+usb_controller.ResettingStatus], 1
;       dbgstr 'high-speed or full-speed device, resetting'
        DEBUGF 1,'K : [%d] EHCI %x: port %d has HS or FS device, resetting\n',[timer_ticks],esi,ecx
        pop     edi
.nothing:
        ret
.low_speed:
;       dbgstr 'low-speed device, releasing'
        DEBUGF 1,'K : [%d] EHCI %x: port %d has LS device, releasing\n',[timer_ticks],esi,ecx
        or      dh, 20h
        and     dl, not 2Ah
        mov     [edi+EhciPortsReg+ecx*4], edx
        ret
.se1:
        dbgstr 'SE1 after connect debounce. Broken hardware?'
        ret
endp

; This procedure is called from several places in main USB code
; and allocates required packets for the given transfer.
; ebx = pipe, other parameters are passed through the stack:
; buffer,size = data to transfer
; flags = same as in usb_open_pipe: bit 0 = allow short transfer, other bits reserved
; td = pointer to the current end-of-queue descriptor
; direction =
;   0000b for normal transfers,
;   1000b for control SETUP transfer,
;   1101b for control OUT transfer,
;   1110b for control IN transfer
; returns eax = pointer to the new end-of-queue descriptor
; (not included in the queue itself) or 0 on error
proc ehci_alloc_transfer stdcall uses edi, \
        buffer:dword, size:dword, flags:dword, td:dword, direction:dword
locals
origTD          dd      ?
packetSize      dd      ?       ; must be last variable, see usb_init_transfer
endl
; 1. Save original value of td:
; it will be useful for rollback if something would fail.
        mov     eax, [td]
        mov     [origTD], eax
; One transfer descriptor can describe up to 5 pages.
; In the worst case (when the buffer is something*1000h+0FFFh)
; this corresponds to 4001h bytes. If the requested size is
; greater, we should split the transfer into several descriptors.
; Boundaries to split must be multiples of endpoint transfer size
; to avoid short packets except in the end of the transfer,
; 4000h is always a good value.
; 2. While the remaining data cannot fit in one descriptor,
; allocate full descriptors (of maximal possible size).
        mov     edi, 4000h
        mov     [packetSize], edi
.fullpackets:
        cmp     [size], edi
        jbe     .lastpacket
        call    ehci_alloc_packet
        test    eax, eax
        jz      .fail
        mov     [td], eax
        add     [buffer], edi
        sub     [size], edi
        jmp     .fullpackets
; 3. The remaining data can fit in one packet;
; allocate the last descriptor with size = size of remaining data.
.lastpacket:
        mov     eax, [size]
        mov     [packetSize], eax
        call    ehci_alloc_packet
        test    eax, eax
        jz      .fail
; 9. Update flags in the last packet.
        mov     edx, [flags]
        mov     [ecx+ehci_gtd.Flags-ehci_gtd.SoftwarePart], edx
; 10. Fill AlternateNextTD field in all allocated TDs.
; If the caller says that short transfer is ok, the queue must advance to
; the next descriptor, which is in eax.
; Otherwise, the queue should stop, so make AlternateNextTD point to
; always-inactive descriptor StopQueueTD.
        push    eax
        test    dl, 1
        jz      .disable_short
        sub     eax, ehci_gtd.SoftwarePart
        jmp     @f
.disable_short:
        mov     eax, [ebx+usb_pipe.Controller]
        add     eax, ehci_controller.StopQueueTD - sizeof.ehci_controller
@@:
        call    get_phys_addr
        mov     edx, [origTD]
@@:
        cmp     edx, [esp]
        jz      @f
        mov     [edx+ehci_gtd.AlternateNextTD-ehci_gtd.SoftwarePart], eax
        mov     edx, [edx+usb_gtd.NextVirt]
        jmp     @b
@@:
        pop     eax
        ret
.fail:
        mov     edi, ehci_hardware_func
        mov     eax, [td]
        stdcall usb_undo_tds, [origTD]
        xor     eax, eax
        ret
endp

; Helper procedure for ehci_alloc_transfer.
; Allocates and initializes one transfer descriptor.
; ebx = pipe, other parameters are passed through the stack;
; fills the current last descriptor and
; returns eax = next descriptor (not filled).
proc ehci_alloc_packet
; inherit some variables from the parent ehci_alloc_transfer
virtual at ebp-8
.origTD         dd      ?
.packetSize     dd      ?
                rd      2
.buffer         dd      ?
.transferSize   dd      ?
.Flags          dd      ?
.td             dd      ?
.direction      dd      ?
end virtual
; 1. Allocate the next TD.
        call    ehci_alloc_td
        test    eax, eax
        jz      .nothing
; 2. Initialize controller-independent parts of both TDs.
        push    eax
        call    usb_init_transfer
        pop     eax
; 3. Copy PID to the new descriptor.
        mov     edx, [ecx+ehci_gtd.Token-ehci_gtd.SoftwarePart]
        mov     [eax+ehci_gtd.Token-ehci_gtd.SoftwarePart], edx
        mov     [eax+ehci_gtd.NextTD-ehci_gtd.SoftwarePart], 1
        mov     [eax+ehci_gtd.AlternateNextTD-ehci_gtd.SoftwarePart], 1
; 4. Save the returned value (next descriptor).
        push    eax
; 5. Store the physical address of the next descriptor.
        sub     eax, ehci_gtd.SoftwarePart
        call    get_phys_addr
        mov     [ecx+ehci_gtd.NextTD-ehci_gtd.SoftwarePart], eax
; 6. For zero-length transfers, store zero in all fields for buffer addresses.
; Otherwise, fill them with real values.
        xor     eax, eax
        mov     [ecx+ehci_gtd.Flags-ehci_gtd.SoftwarePart], eax
repeat 10
        mov     [ecx+ehci_gtd.BufferPointers-ehci_gtd.SoftwarePart+(%-1)*4], eax
end repeat
        cmp     [.packetSize], eax
        jz      @f
        mov     eax, [.buffer]
        call    get_phys_addr
        mov     [ecx+ehci_gtd.BufferPointers-ehci_gtd.SoftwarePart], eax
        and     eax, 0xFFF
        mov     edx, [.packetSize]
        add     edx, eax
        sub     edx, 0x1000
        jbe     @f
        mov     eax, [.buffer]
        add     eax, 0x1000
        call    get_pg_addr
        mov     [ecx+ehci_gtd.BufferPointers+4-ehci_gtd.SoftwarePart], eax
        sub     edx, 0x1000
        jbe     @f
        mov     eax, [.buffer]
        add     eax, 0x2000
        call    get_pg_addr
        mov     [ecx+ehci_gtd.BufferPointers+8-ehci_gtd.SoftwarePart], eax
        sub     edx, 0x1000
        jbe     @f
        mov     eax, [.buffer]
        add     eax, 0x3000
        call    get_pg_addr
        mov     [ecx+ehci_gtd.BufferPointers+12-ehci_gtd.SoftwarePart], eax
        sub     edx, 0x1000
        jbe     @f
        mov     eax, [.buffer]
        add     eax, 0x4000
        call    get_pg_addr
        mov     [ecx+ehci_gtd.BufferPointers+16-ehci_gtd.SoftwarePart], eax
@@:
; 7. Fill Token field:
; set Status = 0 (inactive, ehci_insert_transfer would mark everything active);
; keep current PID if [.direction] is zero, use two lower bits of [.direction]
; otherwise shifted as (0|1|2) -> (2|0|1);
; set error counter to 3;
; set current page to 0;
; do not interrupt on complete (ehci_insert_transfer sets this bit where needed);
; set DataToggle to bit 2 of [.direction].
        mov     eax, [ecx+ehci_gtd.Token-ehci_gtd.SoftwarePart]
        and     eax, 300h       ; keep PID code
        mov     edx, [.direction]
        test    edx, edx
        jz      .haspid
        and     edx, 3
        dec     edx
        jns     @f
        add     edx, 3
@@:
        mov     ah, dl
        mov     edx, [.direction]
        and     edx, not 3
        shl     edx, 29
        or      eax, edx
.haspid:
        or      eax, 0C00h
        mov     edx, [.packetSize]
        shl     edx, 16
        or      eax, edx
        mov     [ecx+ehci_gtd.Token-ehci_gtd.SoftwarePart], eax
; 4. Restore the returned value saved in step 2.
        pop     eax
.nothing:
        ret
endp

; This procedure is called from several places in main USB code
; and activates the transfer which was previously allocated by
; ehci_alloc_transfer.
; ecx -> last descriptor for the transfer, ebx -> usb_pipe
proc ehci_insert_transfer
        or      byte [ecx+ehci_gtd.Token+1-ehci_gtd.SoftwarePart], 80h  ; set IOC bit
        mov     eax, [esp+4]
.activate:
        or      byte [eax+ehci_gtd.Token-ehci_gtd.SoftwarePart], 80h    ; set Active bit
        cmp     eax, ecx
        mov     eax, [eax+usb_gtd.NextVirt]
        jnz     .activate
        ret
endp

; This function is called from ehci_process_deferred when
; reset signalling for a new device needs to be finished.
proc ehci_port_reset_done
        movzx   ecx, [esi+usb_controller.ResettingPort]
        and     dword [edi+EhciPortsReg+ecx*4], not 12Ah
        mov     eax, [timer_ticks]
        mov     [esi+usb_controller.ResetTime], eax
        mov     [esi+usb_controller.ResettingStatus], 2
        DEBUGF 1,'K : [%d] EHCI %x: reset port %d done\n',[timer_ticks],esi,ecx
        ret
endp

; This function is called from ehci_process_deferred when
; a new device has been reset, recovered after reset and needs to be configured.
proc ehci_port_init
; 1. Get the status and set it to zero.
; If reset has been failed (device disconnected during reset),
; continue to next device (if there is one).
        xor     eax, eax
        xchg    al, [esi+usb_controller.ResettingStatus]
        test    al, al
        js      usb_test_pending_port
; 2. Get the port status. High-speed devices should be now enabled,
; full-speed devices are left disabled;
; if the port is disabled, release it to a companion and continue to
; next device (if there is one).
        movzx   ecx, [esi+usb_controller.ResettingPort]
        mov     eax, [edi+EhciPortsReg+ecx*4]
        DEBUGF 1,'K : [%d] EHCI %x status of port %d is %x\n',[timer_ticks],esi,ecx,eax
        test    al, 4
        jnz     @f
;       DEBUGF 1,'K : USB port disabled after reset, status = %x\n',eax
        dbgstr 'releasing to companion'
        or      ah, 20h
        mov     [edi+EhciPortsReg+ecx*4], eax
        jmp     usb_test_pending_port
@@:
; 3. Call the worker procedure to notify the protocol layer
; about new EHCI device. It is high-speed.
        push    USB_SPEED_HS
        pop     eax
        call    ehci_new_device
        test    eax, eax
        jnz     .nothing
; 4. If something at the protocol layer has failed
; (no memory, no bus address), disable the port and stop the initialization.
.disable_exit:
        and     dword [edi+EhciPortsReg+ecx*4], not (4 or 2Ah)
        jmp     usb_test_pending_port
.nothing:
        ret
endp

; This procedure is called from ehci_port_init and from hub support code
; when a new device is connected and has been reset.
; It calls usb_new_device at the protocol layer with correct parameters.
; in: esi -> usb_controller, eax = speed.
proc ehci_new_device
        push    ebx ecx ; save used registers (ecx is important for ehci_port_init)
; 1. Store the speed for the protocol layer.
        mov     [esi+usb_controller.ResettingSpeed], al
; 2. Shift speed bits to the proper place in ehci_pipe.Token.
        shl     eax, 12
; 3. For high-speed devices, go to step 5 with edx = 0.
        xor     edx, edx
        cmp     ah, USB_SPEED_HS shl (12-8)
        jz      .common
; 4. For low-speed and full-speed devices, fill address:port
; of the last high-speed hub (the closest to the device hub)
; for split transactions, and set ControlEndpoint bit in eax;
; ehci_init_pipe assumes that the parent pipe is a control pipe.
        movzx   ecx, [esi+usb_controller.ResettingPort]
        mov     edx, [esi+usb_controller.ResettingHub]
        push    eax
.find_hs_hub:
        mov     eax, [edx+usb_hub.ConfigPipe]
        mov     eax, [eax+usb_pipe.DeviceData]
        cmp     [eax+usb_device_data.Speed], USB_SPEED_HS
        jz      .found_hs_hub
        movzx   ecx, [eax+usb_device_data.Port]
        mov     edx, [eax+usb_device_data.Hub]
        jmp     .find_hs_hub
.found_hs_hub:
        mov     edx, [edx+usb_hub.ConfigPipe]
        inc     ecx
        mov     edx, [edx+ehci_pipe.Token-ehci_pipe.SoftwarePart]
        shl     ecx, 23
        and     edx, 7Fh
        shl     edx, 16
        or      edx, ecx        ; ehci_pipe.Flags
        pop     eax
        or      eax, 1 shl 27   ; ehci_pipe.Token
.common:
; 5. Create pseudo-pipe in the stack.
; See ehci_init_pipe: only .Controller, .Token, .Flags fields are used.
        push    esi     ; ehci_pipe.SoftwarePart.Controller
        mov     ecx, esp
        sub     esp, ehci_pipe.SoftwarePart - ehci_pipe.Flags - 4
        push    edx     ; ehci_pipe.Flags
        push    eax     ; ehci_pipe.Token
; 6. Notify the protocol layer.
        call    usb_new_device
; 7. Cleanup the stack after step 5 and return.
        add     esp, ehci_pipe.SoftwarePart - ehci_pipe.Flags + 8
        pop     ecx ebx ; restore used registers
        ret
endp

; This procedure is called in the USB thread from usb_thread_proc,
; processes regular actions and those actions which can't be safely done
; from interrupt handler.
; Returns maximal time delta before the next call.
proc ehci_process_deferred
        push    ebx edi         ; save used registers to be stdcall
        mov     edi, [esi+ehci_controller.MMIOBase2-sizeof.ehci_controller]
; 1. Get the mask of events to process.
        xor     eax, eax
        xchg    eax, [esi+ehci_controller.DeferredActions-sizeof.ehci_controller]
        push    eax
; 2. Initialize the return value.
        push    -1
; Handle roothub events.
; 3a. Test whether there are such events.
        test    al, 4
        jz      .skip_roothub
; Status of some port has changed. Loop over all ports.
; 3b. Prepare for the loop: start from port 0.
        xor     ecx, ecx
.portloop:
; 3c. Get the port status and changes of it.
; If there are no changes, just continue to the next port.
        mov     eax, [edi+EhciPortsReg+ecx*4]
        test    al, 2Ah
        jz      .nextport
; 3d. Clear change bits and read the status again.
; (It is possible, although quite unlikely, that some event occurs between
; the first read and the clearing, invalidating the old status. If an event
; occurs after the clearing, we will not miss it, looking in the next scan.
        mov     [edi+EhciPortsReg+ecx*4], eax
        mov     ebx, eax
        mov     eax, [edi+EhciPortsReg+ecx*4]
        DEBUGF 1,'K : [%d] EHCI %x: status of port %d changed to %x\n',[timer_ticks],esi,ecx,ebx
; 3e. Handle overcurrent.
; Note: that needs work.
        test    bl, 20h ; overcurrent change
        jz      .noovercurrent
        test    al, 10h ; overcurrent active
        jz      .noovercurrent
        DEBUGF 1,'K : overcurrent at port %d\n',ecx
.noovercurrent:
; 3f. Handle changing of connection status.
        test    bl, 2
        jz      .nocsc
; There was a connect or disconnect event at this port.
; 3g. Disconnect the old device on this port, if any.
; If the port was resetting, indicate fail; later stages will process it.
        cmp     [esi+usb_controller.ResettingHub], 0
        jnz     @f
        cmp     cl, [esi+usb_controller.ResettingPort]
        jnz     @f
        mov     [esi+usb_controller.ResettingStatus], -1
@@:
        bts     [esi+usb_controller.NewDisconnected], ecx
; 3h. Change connected status. For the connection event, also store
; the connection time; any further processing is permitted only after
; USB_CONNECT_DELAY ticks.
        test    al, 1
        jz      .disconnect
        mov     eax, [timer_ticks]
        mov     [esi+usb_controller.ConnectedTime+ecx*4], eax
        bts     [esi+usb_controller.NewConnected], ecx
        jmp     .nextport
.disconnect:
        btr     [esi+usb_controller.NewConnected], ecx
        jmp     .nextport
.nocsc:
; 3i. Handle port disabling.
; Note: that needs work.
        test    al, 8
        jz      @f
        test    al, 4
        jz      @f
        DEBUGF 1,'K : port %d disabled\n',ecx
@@:
; 3j. Continue the loop for the next port.
.nextport:
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .portloop
.skip_roothub:
; 4. Process disconnect events. This should be done after step 3
; (which includes the first stage of disconnect processing).
        call    usb_disconnect_stage2
; 5. Check for previously connected devices.
; If there is a connected device which was connected less than
; USB_CONNECT_DELAY ticks ago, plan to wake up when the delay will be over.
; Otherwise, call ehci_new_port.
; This should be done after step 3.
        xor     ecx, ecx
        cmp     [esi+usb_controller.NewConnected], ecx
        jz      .skip_newconnected
.portloop2:
        bt      [esi+usb_controller.NewConnected], ecx
        jnc     .noconnect
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_controller.ConnectedTime+ecx*4]
        sub     eax, USB_CONNECT_DELAY
        jge     .connected
        neg     eax
        cmp     [esp], eax
        jb      .nextport2
        mov     [esp], eax
        jmp     .nextport2
.connected:
        btr     [esi+usb_controller.NewConnected], ecx
        call    ehci_new_port
        jmp     .portloop2
.noconnect:
.nextport2:
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .portloop2
.skip_newconnected:
; 6. Process wait lists.
; 6a. Periodic endpoints.
; If a request is pending >8 microframes, satisfy it.
; If a request is pending <=8 microframes, schedule next wakeup in 0.01s.
        mov     eax, [esi+usb_controller.WaitPipeRequestPeriodic]
        cmp     eax, [esi+usb_controller.ReadyPipeHeadPeriodic]
        jz      .noperiodic
        mov     edx, [edi+EhciFrameIndexReg]
        sub     edx, [esi+usb_controller.StartWaitFrame]
        and     edx, 0x3FFF
        cmp     edx, 8
        jbe     @f
        mov     [esi+usb_controller.ReadyPipeHeadPeriodic], eax
        jmp     .noperiodic
@@:
        pop     eax
        push    1               ; wakeup in 0.01 sec for next test
.noperiodic:
; 6b. Asynchronous endpoints.
; Satisfy a request when InterruptOnAsyncAdvance fired.
        test    byte [esp+4], 20h
        jz      @f
        dbgstr 'async advance int'
        mov     eax, [esi+usb_controller.WaitPipeRequestAsync]
        mov     [esi+usb_controller.ReadyPipeHeadAsync], eax
@@:
; Some hardware in some (rarely) conditions set the status bit,
; but just does not generate the corresponding interrupt.
; Force checking the status here.
        mov     eax, [esi+usb_controller.WaitPipeRequestAsync]
        cmp     [esi+usb_controller.ReadyPipeHeadAsync], eax
        jz      .noasync
        spin_lock_irq [esi+usb_controller.WaitSpinlock]
        mov     edx, [edi+EhciStatusReg]
        test    dl, 20h
        jz      @f
        mov     dword [edi+EhciStatusReg], 20h
        and     dword [esi+ehci_controller.DeferredActions-sizeof.ehci_controller], not 20h
        dbgstr 'warning: async advance int missed'
        mov     [esi+usb_controller.ReadyPipeHeadAsync], eax
        jmp     .async_unlock
@@:
        cmp     dword [esp], 100
        jb      .async_unlock
        mov     dword [esp], 100
.async_unlock:
        spin_unlock_irq [esi+usb_controller.WaitSpinlock]
.noasync:
; 7. Finalize transfers processed by hardware.
; It is better to perform this step after step 4 (disconnect events),
; although not strictly obligatory. This way, an active transfer aborted
; due to disconnect would be handled with more specific USB_STATUS_CLOSED,
; not USB_STATUS_NORESPONSE.
        test    byte [esp+4], 3
        jz      @f
        call    ehci_process_updated_schedule
@@:
; 8. Test whether reset signalling has been started and should be stopped now.
; This must be done after step 7, because completion of some transfer could
; result in resetting a new port.
.test_reset:
; 8a. Test whether reset signalling is active.
        cmp     [esi+usb_controller.ResettingStatus], 1
        jnz     .no_reset_in_progress
; 8b. Yep. Test whether it should be stopped.
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_controller.ResetTime]
        sub     eax, USB_RESET_TIME
        jge     .reset_done
; 8c. Not yet, but initiate wakeup in -eax ticks and exit this step.
        neg     eax
        cmp     [esp], eax
        jb      .skip_reset
        mov     [esp], eax
        jmp     .skip_reset
.reset_done:
; 8d. Yep, call the worker function and proceed to 8e.
        call    ehci_port_reset_done
.no_reset_in_progress:
; 8e. Test whether reset process is done, either successful or failed.
        cmp     [esi+usb_controller.ResettingStatus], 0
        jz      .skip_reset
; 8f. Yep. Test whether it should be stopped.
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_controller.ResetTime]
        sub     eax, USB_RESET_RECOVERY_TIME
        jge     .reset_recovery_done
; 8g. Not yet, but initiate wakeup in -eax ticks and exit this step.
        neg     eax
        cmp     [esp], eax
        jb      .skip_reset
        mov     [esp], eax
        jmp     .skip_reset
.reset_recovery_done:
; 8h. Yep, call the worker function. This could initiate another reset,
; so return to the beginning of this step.
        call    ehci_port_init
        jmp     .test_reset
.skip_reset:
; 9. Process wait-done notifications, test for new wait requests.
; Note: that must be done after steps 4 and 7 which could create new requests.
; 9a. Call the worker function.
        call    usb_process_wait_lists
; 9b. If it reports that an asynchronous endpoint should be removed,
; doorbell InterruptOnAsyncAdvance and schedule wakeup in 1s
; (sometimes it just does not fire).
        test    al, 1 shl CONTROL_PIPE
        jz      @f
        mov     edx, [esi+usb_controller.WaitPipeListAsync]
        mov     [esi+usb_controller.WaitPipeRequestAsync], edx
        or      dword [edi+EhciCommandReg], 1 shl 6
        dbgstr 'async advance doorbell'
        cmp     dword [esp], 100
        jb      @f
        mov     dword [esp], 100
@@:
; 9c. If it reports that a periodic endpoint should be removed,
; save the current frame and schedule wakeup in 0.01 sec.
        test    al, 1 shl INTERRUPT_PIPE
        jz      @f
        mov     eax, [esi+usb_controller.WaitPipeListPeriodic]
        mov     [esi+usb_controller.WaitPipeRequestPeriodic], eax
        mov     edx, [edi+EhciFrameIndexReg]
        mov     [esi+usb_controller.StartWaitFrame], edx
        mov     dword [esp], 1  ; wakeup in 0.01 sec for next test
@@:
; 10. Pop the return value, restore the stack after step 1 and return.
        pop     eax
        pop     ecx
        pop     edi ebx ; restore used registers to be stdcall
        ret
endp

; This procedure is called in the USB thread from ehci_process_deferred
; when EHCI IRQ handler has signalled that new IOC-packet was processed.
; It scans all lists for completed packets and calls ehci_process_finalized_td
; for those packets.
proc ehci_process_updated_schedule
; Important note: we cannot hold the list lock during callbacks,
; because callbacks sometimes open and/or close pipes and thus acquire/release
; the corresponding lock itself.
; Fortunately, pipes can be finally freed only by another step of
; ehci_process_deferred, so all pipes existing at the start of this function
; will be valid while this function is running. Some pipes can be removed
; from the corresponding list, some pipes can be inserted; insert/remove
; functions guarantee that traversing one list yields all pipes that were in
; that list at the beginning of the traversing (possibly with some new pipes,
; possibly without some new pipes, that doesn't matter).
        push    edi
; 1. Process all Periodic lists.
        lea     edi, [esi+ehci_controller.IntEDs-sizeof.ehci_controller+ehci_static_ep.SoftwarePart]
        lea     ebx, [esi+ehci_controller.IntEDs+63*sizeof.ehci_static_ep-sizeof.ehci_controller+ehci_static_ep.SoftwarePart]
@@:
        call    ehci_process_updated_list
        cmp     edi, ebx
        jnz     @b
; 2. Process the Control list.
        add     edi, ehci_controller.ControlDelta
        call    ehci_process_updated_list
; 3. Process the Bulk list.
        call    ehci_process_updated_list
; 4. Return.
        pop     edi
        ret
endp

; This procedure is called from ehci_process_updated_schedule, see comments there.
; It processes one list, esi -> usb_controller, edi -> usb_static_ep,
; and advances edi to next head.
proc ehci_process_updated_list
        push    ebx
; 1. Perform the external loop over all pipes.
        mov     ebx, [edi+usb_static_ep.NextVirt]
.loop:
        cmp     ebx, edi
        jz      .done
; store pointer to the next pipe in the stack
        push    [ebx+usb_static_ep.NextVirt]
; 2. For every pipe, perform the internal loop over all descriptors.
; All descriptors are organized in the queue; we process items from the start
; of the queue until a) the last descriptor (not the part of the queue itself)
; or b) an active (not yet processed by the hardware) descriptor is reached.
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_lock
        mov     ebx, [ebx+usb_pipe.LastTD]
        push    ebx
        mov     ebx, [ebx+usb_gtd.NextVirt]
.tdloop:
; 3. For every descriptor, test active flag and check for end-of-queue;
; if either of conditions holds, exit from the internal loop.
        cmp     ebx, [esp]
        jz      .tddone
        cmp     byte [ebx+ehci_gtd.Token-ehci_gtd.SoftwarePart], 0
        js      .tddone
; Release the queue lock while processing one descriptor:
; callback function could (and often would) schedule another transfer.
        push    ecx
        call    mutex_unlock
        call    ehci_process_updated_td
        pop     ecx
        call    mutex_lock
        jmp     .tdloop
.tddone:
        call    mutex_unlock
        pop     ebx
; End of internal loop, restore pointer to the next pipe
; and continue the external loop.
        pop     ebx
        jmp     .loop
.done:
        pop     ebx
        add     edi, sizeof.ehci_static_ep
        ret
endp

; This procedure is called from ehci_process_updated_list, which is itself
; called from ehci_process_updated_schedule, see comments there.
; It processes one completed descriptor.
; in: ebx -> usb_gtd, out: ebx -> next usb_gtd.
proc ehci_process_updated_td
;       mov     eax, [ebx+usb_gtd.Pipe]
;       cmp     [eax+usb_pipe.Type], INTERRUPT_PIPE
;       jnz     @f
;       DEBUGF 1,'K : finalized TD for pipe %x:\n',eax
;       lea     eax, [ebx-ehci_gtd.SoftwarePart]
;       DEBUGF 1,'K : %x %x %x %x\n',[eax],[eax+4],[eax+8],[eax+12]
;       DEBUGF 1,'K : %x %x %x %x\n',[eax+16],[eax+20],[eax+24],[eax+28]
;@@:
; 1. Remove this descriptor from the list of descriptors for this pipe.
        call    usb_unlink_td
; 2. Calculate actual number of bytes transferred.
        mov     eax, [ebx+ehci_gtd.Token-ehci_gtd.SoftwarePart]
        lea     edx, [eax+eax]
        shr     edx, 17
        sub     edx, [ebx+usb_gtd.Length]
        neg     edx
; 3. Check whether we need some special processing beyond notifying the driver.
; Transfer errors require special processing.
; Short packets require special processing if
; a) this is not the last descriptor for transfer stage
; (in this case we need to process subsequent descriptors for the stage too)
; or b) the caller considers short transfers to be an error.
; ehci_alloc_transfer sets bit 0 of ehci_gtd.Flags to 0 if short packet
; in this descriptor requires special processing and to 1 otherwise.
; If special processing is not needed, advance to 4 with ecx = 0.
; Otherwise, go to 6.
        xor     ecx, ecx
        test    al, 40h
        jnz     .error
        test    byte [ebx+ehci_gtd.Flags-ehci_gtd.SoftwarePart], 1
        jnz     .notify
        cmp     edx, [ebx+usb_gtd.Length]
        jnz     .special
.notify:
; 4. Either the descriptor in ebx was processed without errors,
; or all necessary error actions were taken and ebx points to the last
; related descriptor.
; 4a. Test whether it is the last descriptor in the transfer
; <=> it has an associated callback.
        mov     eax, [ebx+usb_gtd.Callback]
        test    eax, eax
        jz      .nocallback
; 4b. It has an associated callback; call it with corresponding parameters.
        stdcall_verify eax, [ebx+usb_gtd.Pipe], ecx, \
                [ebx+usb_gtd.Buffer], edx, [ebx+usb_gtd.UserData]
        jmp     .callback
.nocallback:
; 4c. It is an intermediate descriptor. Add its length to the length
; in the following descriptor.
        mov     eax, [ebx+usb_gtd.NextVirt]
        add     [eax+usb_gtd.Length], edx
.callback:
; 5. Free the current descriptor and return the next one.
        push    [ebx+usb_gtd.NextVirt]
        stdcall ehci_free_td, ebx
        pop     ebx
        ret
.error:
        push    ebx
        sub     ebx, ehci_gtd.SoftwarePart
        DEBUGF 1,'K : TD failed:\n'
        DEBUGF 1,'K : %x %x %x %x\n',[ebx],[ebx+4],[ebx+8],[ebx+12]
        DEBUGF 1,'K : %x %x %x %x\n',[ebx+16],[ebx+20],[ebx+24],[ebx+28]
        pop     ebx
        DEBUGF 1,'K : pipe now:\n'
        mov     ecx, [ebx+usb_gtd.Pipe]
        sub     ecx, ehci_pipe.SoftwarePart
        DEBUGF 1,'K : %x %x %x %x\n',[ecx],[ecx+4],[ecx+8],[ecx+12]
        DEBUGF 1,'K : %x %x %x %x\n',[ecx+16],[ecx+20],[ecx+24],[ecx+28]
        DEBUGF 1,'K : %x %x %x %x\n',[ecx+32],[ecx+36],[ecx+40],[ecx+44]
.special:
; 6. Special processing is needed.
; 6a. Save the status and length.
        push    edx
        push    eax
; 6b. Traverse the list of descriptors looking for the final descriptor
; for this transfer. Free and unlink non-final descriptors.
; Final descriptor will be freed in step 5.
.look_final:
        call    usb_is_final_packet
        jnc     .found_final
        push    [ebx+usb_gtd.NextVirt]
        stdcall ehci_free_td, ebx
        pop     ebx
        call    usb_unlink_td
        jmp     .look_final
.found_final:
; 6c. Restore the status saved in 6a and transform it to the error code.
; Notes:
; * any USB transaction error results in Halted bit; if it is not set,
;   but we are here, it must be due to short packet;
; * babble is considered a fatal USB transaction error,
;   other errors just lead to retrying the transaction;
;   if babble is detected, return the corresponding error;
; * if several non-fatal errors have occured during transaction retries,
;   all corresponding bits are set. In this case, return some error code,
;   the order is quite arbitrary.
        pop     eax     ; status
        push    USB_STATUS_UNDERRUN
        pop     ecx
        test    al, 40h         ; not Halted?
        jz      .know_error
        mov     cl, USB_STATUS_OVERRUN
        test    al, 10h         ; Babble detected?
        jnz     .know_error
        mov     cl, USB_STATUS_BUFOVERRUN
        test    al, 20h         ; Data Buffer error?
        jnz     .know_error
        mov     cl, USB_STATUS_NORESPONSE
        test    al, 8           ; Transaction Error?
        jnz     .know_error
        mov     cl, USB_STATUS_STALL
.know_error:
; 6d. If error code is USB_STATUS_UNDERRUN and the last TD allows short packets,
; it is not an error; in this case, go to 4 with ecx = 0.
        cmp     ecx, USB_STATUS_UNDERRUN
        jnz     @f
        test    byte [ebx+ehci_gtd.Flags-ehci_gtd.SoftwarePart], 1
        jz      @f
        xor     ecx, ecx
        pop     edx     ; length
        jmp     .notify
@@:
; 6e. Abort the entire transfer.
; There are two cases: either there is only one transfer stage
; (everything except control transfers), then ebx points to the last TD and
; all previous TD were unlinked and dismissed (if possible),
; or there are several stages (a control transfer) and ebx points to the last
; TD of Data or Status stage (usb_is_final_packet does not stop in Setup stage,
; because Setup stage can not produce short packets); for Data stage, we need
; to unlink and free (if possible) one more TD and advance ebx to the next one.
        cmp     [ebx+usb_gtd.Callback], 0
        jnz     .normal
        push    ecx
        push    [ebx+usb_gtd.NextVirt]
        stdcall ehci_free_td, ebx
        pop     ebx
        call    usb_unlink_td
        pop     ecx
.normal:
; 6f. For bulk/interrupt transfers we have no choice but halt the queue,
; the driver should intercede (through some API which is not written yet).
; Control pipes normally recover at the next SETUP transaction (first stage
; of any control transfer), so we hope on the best and just advance the queue
; to the next transfer. (According to the standard, "A control pipe may also
; support functional stall as well, but this is not recommended.").
        mov     edx, [ebx+usb_gtd.Pipe]
        mov     eax, [ebx+ehci_gtd.NextTD-ehci_gtd.SoftwarePart]
        or      al, 1
        mov     [edx+ehci_pipe.Overlay.NextTD-ehci_pipe.SoftwarePart], eax
        mov     [edx+ehci_pipe.Overlay.AlternateNextTD-ehci_pipe.SoftwarePart], eax
        cmp     [edx+usb_pipe.Type], CONTROL_PIPE
        jz      .control
; Bulk/interrupt transfer; halt the queue.
        mov     [edx+ehci_pipe.Overlay.Token-ehci_pipe.SoftwarePart], 40h
        pop     edx
        jmp     .notify
; Control transfer.
.control:
        and     [edx+ehci_pipe.Overlay.Token-ehci_pipe.SoftwarePart], 0
        dec     [edx+ehci_pipe.Overlay.NextTD-ehci_pipe.SoftwarePart]
        pop     edx
        jmp     .notify
endp

; This procedure unlinks the pipe from the corresponding pipe list.
; esi -> usb_controller, ebx -> usb_pipe
proc ehci_unlink_pipe
        cmp     [ebx+usb_pipe.Type], INTERRUPT_PIPE
        jnz     @f
        test    word [ebx+ehci_pipe.Flags-ehci_pipe.SoftwarePart+2], 3FFFh
        jnz     .interrupt_fs
        call    ehci_hs_interrupt_list_unlink
        jmp     .interrupt_common
.interrupt_fs:
        call    ehci_fs_interrupt_list_unlink
.interrupt_common:
@@:
        mov     edx, [ebx+usb_pipe.NextVirt]
        mov     eax, [ebx+usb_pipe.PrevVirt]
        mov     [edx+usb_pipe.PrevVirt], eax
        mov     [eax+usb_pipe.NextVirt], edx
        mov     edx, esi
        sub     edx, eax
        cmp     edx, sizeof.ehci_controller
        mov     edx, [ebx+ehci_pipe.NextQH-ehci_pipe.SoftwarePart]
        jb      .prev_is_static
        mov     [eax+ehci_pipe.NextQH-ehci_pipe.SoftwarePart], edx
        ret
.prev_is_static:
        mov     [eax+ehci_static_ep.NextQH-ehci_static_ep.SoftwarePart], edx
        ret
endp

proc ehci_alloc_td
        push    ebx
        mov     ebx, ehci_gtd_mutex
        stdcall usb_allocate_common, sizeof.ehci_gtd
        test    eax, eax
        jz      @f
        add     eax, ehci_gtd.SoftwarePart
@@:
        pop     ebx
        ret
endp

; This procedure is called from several places from main USB code and
; frees all additional data associated with the transfer descriptor.
; EHCI has no additional data, so just free ehci_gtd structure.
proc ehci_free_td
        sub     dword [esp+4], ehci_gtd.SoftwarePart
        jmp     usb_free_common
endp
