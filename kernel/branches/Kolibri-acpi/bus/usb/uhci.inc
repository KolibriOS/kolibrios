; Code for UHCI controllers.
; Note: it should be moved to an external driver,
; it was convenient to have this code compiled into the kernel during initial
; development, but there are no reasons to keep it here.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; UHCI register declarations
UhciCommandReg     = 0
UhciStatusReg      = 2
UhciInterruptReg   = 4
UhciFrameNumberReg = 6
UhciBaseAddressReg = 8
UhciSOFModifyReg   = 0Ch
UhciPort1StatusReg = 10h
; possible PIDs for USB data transfers
USB_PID_SETUP = 2Dh
USB_PID_IN    = 69h
USB_PID_OUT   = 0E1h
; UHCI does not support an interrupt on root hub status change. We must poll
; the controller periodically. This is the period in timer ticks (10ms).
; We use the value 100 ms: it is valid value for USB hub poll rate (1-255 ms),
; small enough to be responsible to connect events and large enough to not
; load CPU too often.
UHCI_POLL_INTERVAL = 100
; the following constant is an invalid encoding for length fields in
; uhci_gtd; it is used to check whether an inactive TD has been
; completed (actual length of the transfer is valid) or not processed at all
; (actual length of the transfer is UHCI_INVALID_LENGTH).
; Valid values are 0-4FFh and 7FFh. We use 700h as an invalid value.
UHCI_INVALID_LENGTH = 700h

; =============================================================================
; ================================ Structures =================================
; =============================================================================

; UHCI-specific part of a pipe descriptor.
; * The structure corresponds to the Queue Head aka QH from the UHCI
;   specification with some additional fields.
; * The hardware uses first two fields (8 bytes). Next two fields are used for
;   software book-keeping.
; * The hardware requires 16-bytes alignment of the hardware part.
;   Since the allocator (usb_allocate_common) allocates memory sequentially
;   from page start (aligned on 0x1000 bytes), size of the structure must be
;   divisible by 16.
struct uhci_pipe
NextQH          dd      ?
; 1. First bit (bit 0) is Terminate bit. 1 = there is no next QH.
; 2. Next bit (bit 1) is QH/TD select bit. 1 = NextQH points to QH.
; 3. Next two bits (bits 2-3) are reserved.
; 4. With masked 4 lower bits, this is the physical address of the next QH in
;    the QH list.
; See also the description before NextVirt field of the usb_pipe
; structure. Additionally to that description, the following is specific for
; the UHCI controller:
; * n=10, N=1024. However, this number is quite large.
; * 1024 lists are used only for individual transfer descriptors for
;   Isochronous endpoints. This means that the software can sleep up to 1024 ms
;   before initiating the next portion of a large isochronous transfer, which
;   is a sufficiently large value.
; * We use the 32ms upper limit for interrupt endpoint polling interval.
;   This seems to be a reasonable value.
; * The "next" list for last Periodic list is the Control list.
; * The "next" list for Control list is Bulk list and the "next"
;   list for Bulk list is Control list. This loop is used for bandwidth
;   reclamation: the hardware traverses lists until end-of-frame.
HeadTD          dd      ?
; 1. First bit (bit 0) is Terminate bit. 1 = there is no TDs in this QH.
; 2. Next bit (bit 1) is QH/TD select bit. 1 = HeadTD points to QH.
; 3. Next two bits (bits 2-3) are reserved.
; 4. With masked 4 lower bits, this is the physical address of the first TD in
;    the TD queue for this QH.
Token           dd      ?
; This field is a template for uhci_gtd.Token field in transfer
; descriptors. The meaning of individual bits is the same as for
; uhci_gtd.Token, except that PID bitfield is always
; USB_PID_SETUP/IN/OUT for control/in/out pipes,
; the MaximumLength bitfield encodes maximum packet size,
; the Reserved bit 20 is LowSpeedDevice bit.
ErrorTD         dd      ?
; Usually NULL. If nonzero, it is a pointer to descriptor which was error'd
; and should be freed sometime in the future (the hardware could still use it).
SoftwarePart    rd      sizeof.usb_pipe/4
; Common part for all controllers, described by usb_pipe structure.
ends

if sizeof.uhci_pipe mod 16
.err uhci_pipe must be 16-bytes aligned
end if

; This structure describes the static head of every list of pipes.
; The hardware requires 16-bytes alignment of this structure.
; All instances of this structure are located sequentially in uhci_controller,
; uhci_controller is page-aligned, so it is sufficient to make this structure
; 16-bytes aligned and verify that the first instance is 16-bytes aligned
; inside uhci_controller.
struct uhci_static_ep
NextQH          dd      ?
; Same as uhci_pipe.NextQH.
HeadTD          dd      ?
; Same as uhci_pipe.HeadTD.
NextList        dd      ?
; Virtual address of the next list.
                dd      ?
; Not used.
SoftwarePart    rd      sizeof.usb_static_ep/4
; Common part for all controllers, described by usb_static_ep structure.
                dd      ?
; Padding for 16-byte alignment.
ends

if sizeof.uhci_static_ep mod 16
.err uhci_static_ep must be 16-bytes aligned
end if

; UHCI-specific part of controller data.
; * The structure includes two parts, the hardware part and the software part.
; * The hardware part consists of first 4096 bytes and corresponds to
;   the Frame List from UHCI specification.
; * The hardware requires page-alignment of the hardware part, so
;   the entire descriptor must be page-aligned.
;   This structure is allocated with kernel_alloc (see usb_init_controller),
;   this gives page-aligned data.
struct uhci_controller
; ------------------------------ hardware fields ------------------------------
FrameList       rd      1024
; Entry n corresponds to the head of the frame list to be executed in
; the frames n,n+1024,n+2048,n+3072,...
; The first bit of each entry is Terminate bit, 1 = the frame is empty.
; The second bit of each entry is QH/TD select bit, 1 = the entry points to
; QH, 0 = to TD.
; With masked 2 lower bits, the entry is a physical address of the first QH/TD
; to be executed.
; ------------------------------ software fields ------------------------------
; Every list has the static head, which is an always empty QH.
; The following fields are static heads, one per list:
; 32+16+8+4+2+1 = 63 for Periodic lists, 1 for Control list and 1 for Bulk list.
IntEDs          uhci_static_ep
                rb      62 * sizeof.uhci_static_ep
ControlED       uhci_static_ep
BulkED          uhci_static_ep
IOBase          dd      ?
; Base port in I/O space for UHCI controller.
; UHCI register UhciXxx is addressed as in/out to IOBase + UhciXxx,
; see declarations in the beginning of this source.
DeferredActions dd      ?
; Bitmask of bits from UhciStatusReg which need to be processed
; by uhci_process_deferred. Bit 0 = a transaction with IOC bit
; has completed. Bit 1 = a transaction has failed. Set by uhci_irq,
; cleared by uhci_process_deferred.
LastPollTime    dd      ?
; See the comment before UHCI_POLL_INTERVAL. This variable keeps the
; last time, in timer ticks, when the polling was done.
ends

if uhci_controller.IntEDs mod 16
.err Static endpoint descriptors must be 16-bytes aligned inside uhci_controller
end if

; UHCI general transfer descriptor.
; * The structure describes non-Isochronous data transfers
;   for the UHCI controller.
; * The structure includes two parts, the hardware part and the software part.
; * The hardware part consists of first 16 bytes and corresponds to the
;   Transfer Descriptor aka TD from UHCI specification.
; * The hardware requires 16-bytes alignment of the hardware part, so
;   the entire descriptor must be 16-bytes aligned. Since the allocator
;   (uhci_allocate_common) allocates memory sequentially from page start
;   (aligned on 0x1000 bytes), size of the structure must be divisible by 16.
struct uhci_gtd
NextTD          dd      ?
; 1. First bit (bit 0) is Terminate bit. 1 = there is no next TD.
; 2. Next bit (bit 1) is QH/TD select bit. 1 = NextTD points to QH.
;    This bit is always set to 0 in the implementation.
; 3. Next bit (bit 2) is Depth/Breadth select bit. 1 = the controller should
;    proceed to the NextTD after this TD is complete. 0 = the controller
;    should proceed to the next endpoint after this TD is complete.
;    The implementation sets this bit to 0 for final stages of all transactions
;    and to 1 for other stages.
; 4. Next bit (bit 3) is reserved and must be zero.
; 5. With masked 4 lower bits, this is the physical address of the next TD
;    in the TD list.
ControlStatus   dd      ?
; 1. Lower 11 bits (bits 0-10) are ActLen. This is written by the controller
;    at the conclusion of a USB transaction to indicate the actual number of
;    bytes that were transferred minus 1.
; 2. Next 6 bits (bits 11-16) are reserved.
; 3. Next bit (bit 17) signals Bitstuff error.
; 4. Next bit (bit 18) signals CRC/Timeout error.
; 5. Next bit (bit 19) signals NAK receive.
; 6. Next bit (bit 20) signals Babble error.
; 7. Next bit (bit 21) signals Data Buffer error.
; 8. Next bit (bit 22) signals Stall error.
; 9. Next bit (bit 23) is Active field. 1 = this TD should be processed.
; 10. Next bit (bit 24) is InterruptOnComplete bit. 1 = the controller should
;     issue an interrupt on completion of the frame in which this TD is
;     executed.
; 11. Next bit (bit 25) is IsochronousSelect bit. 1 = this TD is isochronous.
; 12. Next bit (bit 26) is LowSpeedDevice bit. 1 = this TD is for low-speed.
; 13. Next two bits (bits 27-28) are ErrorCounter field. This field is
;     decremented by the controller on every non-fatal error with this TD.
;     Babble and Stall are considered fatal errors and immediately deactivate
;     the TD without decrementing this field. 0 = no error limit,
;     n = deactivate the TD after n errors.
; 14. Next bit (bit 29) is ShortPacketDetect bit. 1 = short packet is an error.
;     Note: the specification defines this bit as input for the controller,
;     but does not specify the value written by controller.
;     Some controllers (e.g. Intel) keep the value, some controllers (e.g. VIA)
;     set the value to whether a short packet was actually detected
;     (or something like that).
;     Thus, we duplicate this bit as bit 0 of OrigBufferInfo.
; 15. Upper two bits (bits 30-31) are reserved.
Token           dd      ?
; 1. Lower 8 bits (bits 0-7) are PID, one of USB_PID_*.
; 2. Next 7 bits (bits 8-14) are DeviceAddress field. This is the address of
;    the target device on the USB bus.
; 3. Next 4 bits (bits 15-18) are Endpoint field. This is the target endpoint
;    number.
; 4. Next bit (bit 19) is DataToggle bit. n = issue/expect DATAn token.
; 5. Next bit (bit 20) is reserved.
; 6. Upper 11 bits (bits 21-31) are MaximumLength field. This field specifies
;    the maximum number of data bytes for the transfer minus 1 byte. Null data
;    packet is encoded as 0x7FF, maximum possible non-null data packet is 1280
;    bytes, encoded as 0x4FF.
Buffer          dd      ?
; Physical address of the data buffer for this TD.
OrigBufferInfo  dd      ?
; Usually NULL. If the original buffer crosses a page boundary, this is a
; pointer to the structure uhci_original_buffer for this request.
; bit 0: 1 = short packet is NOT allowed
; (before the TD is processed, it is the copy of bit 29 of ControlStatus;
;  some controllers modify that bit, so we need a copy in a safe place)
SoftwarePart    rd      sizeof.usb_gtd/4
; Software part, common for all controllers.
ends

if sizeof.uhci_gtd mod 16
.err uhci_gtd must be 16-bytes aligned
end if

; UHCI requires that the entire transfer buffer should be on one page.
; If the actual buffer crosses page boundary, uhci_alloc_packet
; allocates additional memory for buffer for hardware.
; This structure describes correspondence between two buffers.
struct uhci_original_buffer
OrigBuffer      dd      ?
UsedBuffer      dd      ?
ends

; Description of UHCI-specific data and functions for
; controller-independent code.
; Implements the structure usb_hardware_func from hccommon.inc for UHCI.
iglobal
align 4
uhci_hardware_func:
        dd      'UHCI'
        dd      sizeof.uhci_controller
        dd      uhci_init
        dd      uhci_process_deferred
        dd      uhci_set_device_address
        dd      uhci_get_device_address
        dd      uhci_port_disable
        dd      uhci_new_port.reset
        dd      uhci_set_endpoint_packet_size
        dd      usb1_allocate_endpoint
        dd      uhci_free_pipe
        dd      uhci_init_pipe
        dd      uhci_unlink_pipe
        dd      usb1_allocate_general_td
        dd      uhci_free_td
        dd      uhci_alloc_transfer
        dd      uhci_insert_transfer
        dd      uhci_new_device
endg

; =============================================================================
; =================================== Code ====================================
; =============================================================================

; Controller-specific initialization function.
; Called from usb_init_controller. Initializes the hardware and
; UHCI-specific parts of software structures.
; eax = pointer to uhci_controller to be initialized
; [ebp-4] = pcidevice
proc uhci_init
; inherit some variables from the parent (usb_init_controller)
.devfn   equ ebp - 4
.bus     equ ebp - 3
; 1. Store pointer to uhci_controller for further use.
        push    eax
        mov     edi, eax
        mov     esi, eax
; 2. Initialize uhci_controller.FrameList.
; Note that FrameList is located in the beginning of uhci_controller,
; so esi and edi now point to uhci_controller.FrameList.
; First 32 entries of FrameList contain physical addresses
; of first 32 Periodic static heads, further entries duplicate these.
; See the description of structures for full info.
; Note that all static heads fit in one page, so one call to
; get_phys_addr is sufficient.
if (uhci_controller.IntEDs / 0x1000) <> (uhci_controller.BulkED / 0x1000)
.err assertion failed
end if
; 2a. Get physical address of first static head.
; Note that 1) it is located in the beginning of a page
; and 2) all other static heads fit in the same page,
; so one call to get_phys_addr without correction of lower 12 bits
; is sufficient.
if (uhci_controller.IntEDs mod 0x1000) <> 0
.err assertion failed
end if
        add     eax, uhci_controller.IntEDs
        call    get_phys_addr
; 2b. Fill first 32 entries.
        inc     eax
        inc     eax     ; set QH bit for uhci_pipe.NextQH
        push    32
        pop     ecx
        mov     edx, ecx
@@:
        stosd
        add     eax, sizeof.uhci_static_ep
        loop    @b
; 2c. Fill the rest entries.
        mov     ecx, 1024 - 32
        rep movsd
; 3. Initialize static heads uhci_controller.*ED.
; Use the loop over groups: first group consists of first 32 Periodic
; descriptors, next group consists of next 16 Periodic descriptors,
; ..., last group consists of the last Periodic descriptor.
; 3a. Prepare for the loop.
; make esi point to the second group, other registers are already set.
        add     esi, 32*4 + 32*sizeof.uhci_static_ep
; 3b. Loop over groups. On every iteration:
; edx = size of group, edi = pointer to the current group,
; esi = pointer to the next group, eax = physical address of the next group.
.init_static_eds:
; 3c. Get the size of next group.
        shr     edx, 1
; 3d. Exit the loop if there is no next group.
        jz      .init_static_eds_done
; 3e. Initialize the first half of the current group.
; Advance edi to the second half.
        push    eax esi
        call    uhci_init_static_ep_group
        pop     esi eax
; 3f. Initialize the second half of the current group
; with the same values.
; Advance edi to the next group, esi/eax to the next of the next group.
        call    uhci_init_static_ep_group
        jmp     .init_static_eds
.init_static_eds_done:
; 3g. Initialize the last static head.
        xor     esi, esi
        call    uhci_init_static_endpoint
; 3i. Initialize the head of Control list.
        add     eax, sizeof.uhci_static_ep
        call    uhci_init_static_endpoint
; 3j. Initialize the head of Bulk list.
        sub     eax, sizeof.uhci_static_ep
        call    uhci_init_static_endpoint
; 4. Get I/O base address and size from PCI bus.
; 4a. Read&save PCI command state.
        mov     bh, [.devfn]
        mov     ch, [.bus]
        mov     cl, 1
        mov     eax, ecx
        mov     bl, 4
        call    pci_read_reg
        push    eax
; 4b. Disable IO access.
        and     al, not 1
        push    ecx
        xchg    eax, ecx
        call    pci_write_reg
        pop     ecx
; 4c. Read&save IO base address.
        mov     eax, ecx
        mov     bl, 20h
        call    pci_read_reg
        and     al, not 3
        xchg    eax, edi
; now edi = IO base
; 4d. Write 0xffff to IO base address.
        push    ecx
        xchg    eax, ecx
        or      ecx, -1
        call    pci_write_reg
        pop     ecx
; 4e. Read IO base address.
        mov     eax, ecx
        call    pci_read_reg
        and     al, not 3
        cwde
        not     eax
        inc     eax
        xchg    eax, esi
; now esi = IO size
; 4f. Restore IO base address.
        xchg    eax, ecx
        mov     ecx, edi
        push    eax
        call    pci_write_reg
        pop     eax
; 4g. Restore PCI command state and enable io & bus master access.
        pop     ecx
        or      ecx, 5
        mov     bl, 4
        push    eax
        call    pci_write_reg
        pop     eax
; 5. Reset the controller.
; 5e. Host reset.
        mov     edx, edi
        mov     ax, 2
        out     dx, ax
; 5f. Wait up to 10ms.
        push    10
        pop     ecx
@@:
        push    esi
        push    1
        pop     esi
        call    delay_ms
        pop     esi
        in      ax, dx
        test    al, 2
        loopnz  @b
        jz      @f
        dbgstr 'UHCI controller reset timeout'
        jmp     .fail
@@:
if 0
; emergency variant for tests - always wait 10 ms
; wait 10 ms
        push    esi
        push    10
        pop     esi
        call    delay_ms
        pop     esi
; clear reset signal
        xor     eax, eax
        out     dx, ax
end if
.resetok:
; 6. Get number of ports & disable all ports.
        add     esi, edi
        lea     edx, [edi+UhciPort1StatusReg]
.scanports:
        cmp     edx, esi
        jae     .doneports
        in      ax, dx
        cmp     ax, 0xFFFF
        jz      .doneports
        test    al, al
        jns     .doneports
        xor     eax, eax
        out     dx, ax
        inc     edx
        inc     edx
        jmp     .scanports
.doneports:
        lea     esi, [edx-UhciPort1StatusReg]
        sub     esi, edi
        shr     esi, 1  ; esi = number of ports
        jnz     @f
        dbgstr 'error: no ports on UHCI controller'
        jmp     .fail
@@:
; 7. Setup the rest of uhci_controller.
        xchg    esi, [esp]      ; restore the pointer to uhci_controller from the step 1
        add     esi, sizeof.uhci_controller
        pop     [esi+usb_controller.NumPorts]
        DEBUGF 1,'K : UHCI controller at %x:%x with %d ports initialized\n',[.bus]:2,[.devfn]:2,[esi+usb_controller.NumPorts]
        mov     [esi+uhci_controller.IOBase-sizeof.uhci_controller], edi
        mov     eax, [timer_ticks]
        mov     [esi+uhci_controller.LastPollTime-sizeof.uhci_controller], eax
; 8. Hook interrupt.
        mov     ah, [.bus]
        mov     al, 0
        mov     bh, [.devfn]
        mov     bl, 3Ch
        call    pci_read_reg
; al = IRQ
;       DEBUGF 1,'K : UHCI %x: io=%x, irq=%x\n',esi,edi,al
        movzx   eax, al
        stdcall attach_int_handler, eax, uhci_irq, esi
; 9. Setup controller registers.
        xor     eax, eax
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
; 9a. UhciStatusReg := 3Fh: clear all status bits
; (for this register 1 clears the corresponding bit, 0 does not change it).
        inc     edx
        inc     edx     ; UhciStatusReg == 2
        mov     al, 3Fh
        out     dx, ax
; 9b. UhciInterruptReg := 0Dh.
        inc     edx
        inc     edx     ; UhciInterruptReg == 4
        mov     al, 0Dh
        out     dx, ax
; 9c. UhciFrameNumberReg := 0.
        inc     edx
        inc     edx     ; UhciFrameNumberReg == 6
        mov     al, 0
        out     dx, ax
; 9d. UhciBaseAddressReg := physical address of uhci_controller.
        inc     edx
        inc     edx     ; UhciBaseAddressReg == 8
        lea     eax, [esi-sizeof.uhci_controller]
        call    get_phys_addr
        out     dx, eax
; 9e. UhciCommandReg := Run + Configured + (MaxPacket is 64 bytes)
        sub     edx, UhciBaseAddressReg ; UhciCommandReg == 0
        mov     ax, 0C1h        ; Run, Configured, MaxPacket = 64b
        out     dx, ax
; 10. Do initial scan of existing devices.
        call    uhci_poll_roothub
; 11. Return pointer to usb_controller.
        xchg    eax, esi
        ret
.fail:
; On error, pop the pointer saved at step 1 and return zero.
; Note that the main code branch restores the stack at step 7 and never fails
; after step 7.
        pop     ecx
        xor     eax, eax
        ret
endp

; Controller-specific pre-initialization function: take ownership from BIOS.
; UHCI has no mechanism to ask the owner politely to release ownership,
; so do it in inpolite way, preventing controller from any SMI activity.
proc uhci_kickoff_bios
; 1. Get the I/O address.
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 1
        mov     bh, [esi+PCIDEV.devfn]
        mov     bl, 20h
        call    pci_read_reg
        and     eax, 0xFFFC
        xchg    eax, edx
; 2. Stop the controller and disable all interrupts.
        in      ax, dx
        and     al, not 1
        out     dx, ax
        add     edx, UhciInterruptReg
        xor     eax, eax
        out     dx, ax
; 3. Disable all bits for SMI routing, clear SMI routing status,
; enable master interrupt bit.
        mov     ah, [esi+PCIDEV.bus]
        mov     al, 1
        mov     bl, 0xC0
        mov     ecx, 0AF00h
        call    pci_write_reg
        ret
endp

; Helper procedure for step 3 of uhci_init.
; Initializes the static head of one list.
; eax = physical address of the "next" list, esi = pointer to the "next" list,
; edi = pointer to head to initialize.
; Advances edi to the next head, keeps eax/esi.
proc uhci_init_static_endpoint
        mov     [edi+uhci_static_ep.NextQH], eax
        mov     byte [edi+uhci_static_ep.HeadTD], 1
        mov     [edi+uhci_static_ep.NextList], esi
        add     edi, uhci_static_ep.SoftwarePart
        call    usb_init_static_endpoint
        add     edi, sizeof.uhci_static_ep - uhci_static_ep.SoftwarePart
        ret
endp

; Helper procedure for step 3 of uhci_init, see comments there.
; Initializes one half of group of static heads.
; edx = size of the next group = half of size of the group,
; edi = pointer to the group, eax = physical address of the next group,
; esi = pointer to the next group.
; Advances eax, esi, edi to next group, keeps edx.
proc uhci_init_static_ep_group
        push    edx
@@:
        call    uhci_init_static_endpoint
        add     eax, sizeof.uhci_static_ep
        add     esi, sizeof.uhci_static_ep
        dec     edx
        jnz     @b
        pop     edx
        ret
endp

; IRQ handler for UHCI controllers.
uhci_irq.noint:
; Not our interrupt: restore esi and return zero.
        pop     esi
        xor     eax, eax
        ret
proc uhci_irq
        push    esi     ; save used register to be cdecl
virtual at esp
                dd      ?       ; saved esi
                dd      ?       ; return address
.controller     dd      ?
end virtual
        mov     esi, [.controller]
; 1. Read UhciStatusReg.
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        inc     edx
        inc     edx     ; UhciStatusReg == 2
        in      ax, dx
; 2. Test whether it is our interrupt; if so, at least one status bit is set.
        test    al, 0x1F
        jz      .noint
; 3. Clear all status bits.
        out     dx, ax
; 4. Sanity check.
        test    al, 0x3C
        jz      @f
        DEBUGF 1,'K : something terrible happened with UHCI (%x)\n',al
@@:
; 5. We can't do too much from an interrupt handler, e.g. we can't take
; any mutex locks since our code could be called when another code holds the
; lock and has no chance to release it. Thus, only inform the processing thread
; that it should scan the queue and wake it if needed.
        lock or byte [esi+uhci_controller.DeferredActions-sizeof.uhci_controller], al
        push    ebx
        xor     ebx, ebx
        inc     ebx
        call    usb_wakeup_if_needed
        pop     ebx
; 6. This is our interrupt; return 1.
        mov     al, 1
        pop     esi     ; restore used register to be stdcall
        ret
endp

; This procedure is called in the USB thread from usb_thread_proc,
; processes regular actions and those actions which can't be safely done
; from interrupt handler.
; Returns maximal time delta before the next call.
proc uhci_process_deferred
        push    ebx edi         ; save used registers to be stdcall
; 1. Initialize the return value.
        push    -1
; 2. Poll the root hub every UHCI_POLL_INTERVAL ticks.
; Also force polling if some transaction has completed with errors;
; the error can be caused by disconnect, try to detect it.
        test    byte [esi+uhci_controller.DeferredActions-sizeof.uhci_controller], 2
        jnz     .force_poll
        mov     eax, [timer_ticks]
        sub     eax, [esi+uhci_controller.LastPollTime-sizeof.uhci_controller]
        sub     eax, UHCI_POLL_INTERVAL
        jl      .nopoll
.force_poll:
        mov     eax, [timer_ticks]
        mov     [esi+uhci_controller.LastPollTime-sizeof.uhci_controller], eax
        call    uhci_poll_roothub
        mov     eax, -UHCI_POLL_INTERVAL
.nopoll:
        neg     eax
        cmp     [esp], eax
        jb      @f
        mov     [esp], eax
@@:
; 3. Process wait lists.
; 3a. Test whether there is a wait request.
        mov     eax, [esi+usb_controller.WaitPipeRequestAsync]
        cmp     eax, [esi+usb_controller.ReadyPipeHeadAsync]
        jnz     .check_removed
        mov     eax, [esi+usb_controller.WaitPipeRequestPeriodic]
        cmp     eax, [esi+usb_controller.ReadyPipeHeadPeriodic]
        jz      @f
.check_removed:
; 3b. Yep. Find frame and compare it with the saved one.
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        add     edx, UhciFrameNumberReg
        in      ax, dx
        cmp     word [esi+usb_controller.StartWaitFrame], ax
        jnz     .removed
; 3c. The same frame; wake up in 0.01 sec.
        mov     dword [esp], 1
        jmp     @f
.removed:
; 3d. The frame is changed, old contents is guaranteed to be forgotten.
        mov     eax, [esi+usb_controller.WaitPipeRequestAsync]
        mov     [esi+usb_controller.ReadyPipeHeadAsync], eax
        mov     eax, [esi+usb_controller.WaitPipeRequestPeriodic]
        mov     [esi+usb_controller.ReadyPipeHeadPeriodic], eax
@@:
; 4. Process disconnect events. This should be done after step 2
; (which includes the first stage of disconnect processing).
        call    usb_disconnect_stage2
; 5. Test whether USB_CONNECT_DELAY for a connected device is over.
; Call uhci_new_port for all such devices.
        xor     ecx, ecx
        cmp     [esi+usb_controller.NewConnected], ecx
        jz      .skip_newconnected
.portloop:
        bt      [esi+usb_controller.NewConnected], ecx
        jnc     .noconnect
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_controller.ConnectedTime+ecx*4]
        sub     eax, USB_CONNECT_DELAY
        jge     .connected
        neg     eax
        cmp     [esp], eax
        jb      .nextport
        mov     [esp], eax
        jmp     .nextport
.connected:
        btr     [esi+usb_controller.NewConnected], ecx
        call    uhci_new_port
.noconnect:
.nextport:
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .portloop
.skip_newconnected:
; 6. Test for processed packets.
; This should be done after step 4, so transfers which were failed due
; to disconnect are marked with the exact reason, not just
; 'device not responding'.
        xor     eax, eax
        xchg    byte [esi+uhci_controller.DeferredActions-sizeof.uhci_controller], al
        test    al, 3
        jz      .noioc
        call    uhci_process_updated_schedule
.noioc:
; 7. Test whether reset signalling has been started. If so, 
; either should be stopped now (if time is over) or schedule wakeup (otherwise).
; This should be done after step 6, because a completed SET_ADDRESS command
; could result in reset of a new port.
.test_reset:
; 7a. Test whether reset signalling is active.
        cmp     [esi+usb_controller.ResettingStatus], 1
        jnz     .no_reset_in_progress
; 7b. Yep. Test whether it should be stopped.
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_controller.ResetTime]
        sub     eax, USB_RESET_TIME
        jge     .reset_done
; 7c. Not yet, but initiate wakeup in -eax ticks and exit this step.
        neg     eax
        cmp     [esp], eax
        jb      .skip_reset
        mov     [esp], eax
        jmp     .skip_reset
.reset_done:
; 7d. Yep, call the worker function and proceed to 7e.
        call    uhci_port_reset_done
.no_reset_in_progress:
; 7e. Test whether reset process is done, either successful or failed.
        cmp     [esi+usb_controller.ResettingStatus], 0
        jz      .skip_reset
; 7f. Yep. Test whether it should be stopped.
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_controller.ResetTime]
        sub     eax, USB_RESET_RECOVERY_TIME
        jge     .reset_recovery_done
; 7g. Not yet, but initiate wakeup in -eax ticks and exit this step.
        neg     eax
        cmp     [esp], eax
        jb      .skip_reset
        mov     [esp], eax
        jmp     .skip_reset
.reset_recovery_done:
; 7h. Yep, call the worker function. This could initiate another reset,
; so return to the beginning of this step.
        call    uhci_port_init
        jmp     .test_reset
.skip_reset:
; 8. Process wait-done notifications, test for new wait requests.
; Note: that must be done after steps 4 and 6 which could create new requests.
; 8a. Call the worker function.
        call    usb_process_wait_lists
; 8b. If no new requests, skip the rest of this step.
        test    eax, eax
        jz      @f
; 8c. UHCI is not allowed to cache anything; we don't know what is
; processed right now, but we can be sure that the controller will not
; use any removed structure starting from the next frame.
; Request removal of everything disconnected until now,
; schedule wakeup in 0.01 sec.
        mov     eax, [esi+usb_controller.WaitPipeListAsync]
        mov     [esi+usb_controller.WaitPipeRequestAsync], eax
        mov     eax, [esi+usb_controller.WaitPipeListPeriodic]
        mov     [esi+usb_controller.WaitPipeRequestPeriodic], eax
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        add     edx, UhciFrameNumberReg
        in      ax, dx
        mov     word [esi+usb_controller.StartWaitFrame], ax
        mov     dword [esp], 1
@@:
; 9. Return the value from the top of stack.
        pop     eax
        pop     edi ebx         ; restore used registers to be stdcall.
        ret
endp

; This procedure is called in the USB thread from uhci_process_deferred
; when UHCI IRQ handler has signalled that new IOC-packet was processed.
; It scans all lists for completed packets and calls uhci_process_finalized_td
; for those packets.
; in: esi -> usb_controller
proc uhci_process_updated_schedule
; Important note: we cannot hold the list lock during callbacks,
; because callbacks sometimes open and/or close pipes and thus acquire/release
; the corresponding lock itself.
; Fortunately, pipes can be finally freed only by another step of
; uhci_process_deferred, so all pipes existing at the start of this function
; will be valid while this function is running. Some pipes can be removed
; from the corresponding list, some pipes can be inserted; insert/remove
; functions guarantee that traversing one list yields all pipes that were in
; that list at the beginning of the traversing (possibly with some new pipes,
; possibly without some new pipes, that doesn't matter).
; 1. Process all Periodic lists.
        lea     edi, [esi+uhci_controller.IntEDs.SoftwarePart-sizeof.uhci_controller]
        lea     ebx, [esi+uhci_controller.IntEDs.SoftwarePart+63*sizeof.uhci_static_ep-sizeof.uhci_controller]
@@:
        call    uhci_process_updated_list
        cmp     edi, ebx
        jnz     @b
; 2. Process the Control list.
        call    uhci_process_updated_list
; 3. Process the Bulk list.
        call    uhci_process_updated_list
; 4. Return.
        ret
endp

; This procedure is called from uhci_process_updated_schedule,
; see comments there.
; It processes one list, esi -> usb_controller, edi -> usb_static_ep,
; and advances edi to the next head.
proc uhci_process_updated_list
        push    ebx             ; save used register to be stdcall
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
        mov     eax, [ebx+uhci_gtd.ControlStatus-uhci_gtd.SoftwarePart]
        test    eax, 1 shl 23   ; active?
        jnz     .tddone
; Release the queue lock while processing one descriptor:
; callback function could (and often would) schedule another transfer.
        push    ecx
        call    mutex_unlock
        call    uhci_process_finalized_td
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
        pop     ebx             ; restore used register to be stdcall
        add     edi, sizeof.uhci_static_ep
        ret
endp

; This procedure is called from uhci_process_updated_list, which is itself
; called from uhci_process_updated_schedule, see comments there.
; It processes one completed descriptor.
; in: esi -> usb_controller, ebx -> usb_gtd, out: ebx -> next usb_gtd.
proc uhci_process_finalized_td
; 1. Remove this descriptor from the list of descriptors for this pipe.
        call    usb_unlink_td
;       DEBUGF 1,'K : finalized TD:\n'
;       DEBUGF 1,'K : %x %x %x %x\n',[ebx-20],[ebx-16],[ebx-12],[ebx-8]
;       DEBUGF 1,'K : %x %x %x %x\n',[ebx-4],[ebx],[ebx+4],[ebx+8]
; 2. If this is IN transfer into special buffer, copy the data
; to target location.
        mov     edx, [ebx+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart]
        and     edx, not 1      ; clear lsb (used for another goal)
        jz      .nocopy
        cmp     byte [ebx+uhci_gtd.Token-uhci_gtd.SoftwarePart], USB_PID_IN
        jnz     .nocopy
; Note: we assume that pointer to buffer is valid in the memory space of
; the USB thread. This means that buffer must reside in kernel memory
; (shared by all processes).
        push    esi edi
        mov     esi, [edx+uhci_original_buffer.UsedBuffer]
        mov     edi, [edx+uhci_original_buffer.OrigBuffer]
        mov     ecx, [ebx+uhci_gtd.ControlStatus-uhci_gtd.SoftwarePart]
        inc     ecx
        and     ecx, 7FFh
        mov     edx, ecx
        shr     ecx, 2
        and     edx, 3
        rep movsd
        mov     ecx, edx
        rep movsb
        pop     edi esi
.nocopy:
; 3. Calculate actual number of bytes transferred.
; 3a. Read the state.
        mov     eax, [ebx+uhci_gtd.ControlStatus-uhci_gtd.SoftwarePart]
        mov     ecx, [ebx+uhci_gtd.Token-uhci_gtd.SoftwarePart]
; 3b. Get number of bytes processed.
        lea     edx, [eax+1]
        and     edx, 7FFh
; 3c. Subtract number of bytes in this packet.
        add     ecx, 1 shl 21
        shr     ecx, 21
        sub     edx, ecx
; 3d. Add total length transferred so far.
        add     edx, [ebx+usb_gtd.Length]
; Actions on error and on success are slightly different.
; 4. Test for error. On error, proceed to step 5, otherwise go to step 6
; with ecx = 0 (no error).
; USB transaction error is always considered as such.
; If short packets are not allowed, UHCI controllers do not set an error bit,
; but stop (clear Active bit and do not advance) the queue.
; Short packet is considered as an error if the packet is actually short
; (actual length is less than maximal one) and the code creating the packet
; requested that behaviour (so bit 0 of OrigBufferInfo is set; this could be
; because the caller disallowed short packets or because the packet is not
; the last one in the corresponding transfer).
        xor     ecx, ecx
        test    eax, 1 shl 22
        jnz     .error
        test    byte [ebx+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart], 1
        jz      .notify
        cmp     edx, [ebx+usb_gtd.Length]
        jz      .notify
.error:
; 5. There was an error while processing this packet.
; The hardware has stopped processing the queue.
        DEBUGF 1,'K : TD failed:\n'
if uhci_gtd.SoftwarePart <> 20
.err modify offsets for debug output
end if
        DEBUGF 1,'K : %x %x %x %x\n',[ebx-20],[ebx-16],[ebx-12],[ebx-8]
        DEBUGF 1,'K : %x %x %x %x\n',[ebx-4],[ebx],[ebx+4],[ebx+8]
; 5a. Save the status and length.
        push    edx
        push    eax
        mov     eax, [ebx+usb_gtd.Pipe]
        DEBUGF 1,'K : pipe: %x %x\n',[eax+0-uhci_pipe.SoftwarePart],[eax+4-uhci_pipe.SoftwarePart]
; 5b. Store the current TD as an error packet.
; If an error packet is already stored for this pipe,
; it is definitely not used already, so free the old packet.
        mov     eax, [eax+uhci_pipe.ErrorTD-uhci_pipe.SoftwarePart]
        test    eax, eax
        jz      @f
        stdcall uhci_free_td, eax
@@:
        mov     eax, [ebx+usb_gtd.Pipe]
        mov     [eax+uhci_pipe.ErrorTD-uhci_pipe.SoftwarePart], ebx
; 5c. Traverse the list of descriptors looking for the final packet
; for this transfer.
; Free and unlink non-final descriptors, except the current one.
; Final descriptor will be freed in step 7.
        call    usb_is_final_packet
        jnc     .found_final
        mov     ebx, [ebx+usb_gtd.NextVirt]
.look_final:
        call    usb_unlink_td
        call    usb_is_final_packet
        jnc     .found_final
        push    [ebx+usb_gtd.NextVirt]
        stdcall uhci_free_td, ebx
        pop     ebx
        jmp     .look_final
.found_final:
; 5d. Restore the status saved in 5a and transform it to the error code.
        pop     eax     ; error code
        shr     eax, 16
; Notes:
; * any USB transaction error results in Stalled bit; if it is not set,
;   but we are here, it must be due to short packet;
; * babble is considered a fatal USB transaction error,
;   other errors just lead to retrying the transaction;
;   if babble is detected, return the corresponding error;
; * if several non-fatal errors have occured during transaction retries,
;   all corresponding bits are set. In this case, return some error code,
;   the order is quite arbitrary.
        push    USB_STATUS_UNDERRUN
        pop     ecx
        test    al, 1 shl (22-16)       ; not Stalled?
        jz      .know_error
        mov     cl, USB_STATUS_OVERRUN
        test    al, 1 shl (20-16)       ; Babble detected?
        jnz     .know_error
        mov     cl, USB_STATUS_BITSTUFF
        test    al, 1 shl (17-16)       ; Bitstuff error?
        jnz     .know_error
        mov     cl, USB_STATUS_NORESPONSE
        test    al, 1 shl (18-16)       ; CRC/TimeOut error?
        jnz     .know_error
        mov     cl, USB_STATUS_BUFOVERRUN
        test    al, 1 shl (21-16)       ; Data Buffer error?
        jnz     .know_error
        mov     cl, USB_STATUS_STALL
.know_error:
; 5e. If error code is USB_STATUS_UNDERRUN
; and the last TD allows short packets, it is not an error.
; Note: all TDs except the last one in any transfer stage are marked
; as short-packet-is-error to stop controller from further processing
; of that stage; we need to restart processing from a TD following the last.
; After that, go to step 6 with ecx = 0 (no error).
        cmp     ecx, USB_STATUS_UNDERRUN
        jnz     @f
        test    byte [ebx+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart], 1
        jnz     @f
; The controller has stopped this queue on the error packet.
; Update uhci_pipe.HeadTD to point to the next packet in the queue.
        call    uhci_fix_toggle
        xor     ecx, ecx
.control:
        mov     eax, [ebx+uhci_gtd.NextTD-uhci_gtd.SoftwarePart]
        and     al, not 0xF
        mov     edx, [ebx+usb_gtd.Pipe]
        mov     [edx+uhci_pipe.HeadTD-uhci_pipe.SoftwarePart], eax
        pop     edx     ; length
        jmp     .notify
@@:
; 5f. Abort the entire transfer.
; There are two cases: either there is only one transfer stage
; (everything except control transfers), then ebx points to the last TD and
; all previous TD were unlinked and dismissed (if possible),
; or there are several stages (a control transfer) and ebx points to the last
; TD of Data or Status stage (usb_is_final_packet does not stop in Setup stage,
; because Setup stage can not produce short packets); for Data stage, we need
; to unlink and free (if possible) one more TD and advance ebx to the next one.
        cmp     [ebx+usb_gtd.Callback], 0
        jnz     .normal
; We cannot free ErrorTD yet, it could still be used by the hardware.
        push    ecx
        mov     eax, [ebx+usb_gtd.Pipe]
        push    [ebx+usb_gtd.NextVirt]
        cmp     ebx, [eax+uhci_pipe.ErrorTD-uhci_pipe.SoftwarePart]
        jz      @f
        stdcall uhci_free_td, ebx
@@:
        pop     ebx
        call    usb_unlink_td
        pop     ecx
.normal:
; 5g. For bulk/interrupt transfers we have no choice but halt the queue,
; the driver should intercede (through some API which is not written yet).
; Control pipes normally recover at the next SETUP transaction (first stage
; of any control transfer), so we hope on the best and just advance the queue
; to the next transfer. (According to the standard, "A control pipe may also
; support functional stall as well, but this is not recommended.").
        mov     edx, [ebx+usb_gtd.Pipe]
        cmp     [edx+usb_pipe.Type], CONTROL_PIPE
        jz      .control
; Bulk/interrupt transfer; halt the queue.
        mov     eax, [ebx+uhci_gtd.NextTD-uhci_gtd.SoftwarePart]
        and     al, not 0xF
        inc     eax     ; set Halted bit
        mov     [edx+uhci_pipe.HeadTD-uhci_pipe.SoftwarePart], eax
        pop     edx     ; restore length saved in step 5a
.notify:
; 6. Either the descriptor in ebx was processed without errors,
; or all necessary error actions were taken and ebx points to the last
; related descriptor.
; 6a. Test whether it is the last packet in the transfer
; <=> it has an associated callback.
        mov     eax, [ebx+usb_gtd.Callback]
        test    eax, eax
        jz      .nocallback
; 6b. It has an associated callback; call it with corresponding parameters.
        stdcall_verify eax, [ebx+usb_gtd.Pipe], ecx, \
                [ebx+usb_gtd.Buffer], edx, [ebx+usb_gtd.UserData]
        jmp     .callback
.nocallback:
; 6c. It is an intermediate packet. Add its length to the length
; in the following packet.
        mov     eax, [ebx+usb_gtd.NextVirt]
        add     [eax+usb_gtd.Length], edx
.callback:
; 7. Free the current descriptor (if allowed) and return the next one.
; 7a. Save pointer to the next descriptor.
        push    [ebx+usb_gtd.NextVirt]
; 7b. Free the descriptor, unless it is saved as ErrorTD.
        mov     eax, [ebx+usb_gtd.Pipe]
        cmp     [eax+uhci_pipe.ErrorTD-uhci_pipe.SoftwarePart], ebx
        jz      @f
        stdcall uhci_free_td, ebx
@@:
; 7c. Restore pointer to the next descriptor and return.
        pop     ebx
        ret
endp

; Helper procedure for restarting transfer queue.
; When transfers are queued, their toggle bit is filled assuming that
; everything will go without errors. On error, some packets needs to be
; skipped, so toggle bits may become incorrect.
; This procedure fixes toggle bits.
; in: ebx -> last packet to be skipped, ErrorTD -> last processed packet
proc uhci_fix_toggle
; 1. Nothing to do for control pipes: in that case,
; toggle bits for different transfer stages are independent.
        mov     ecx, [ebx+usb_gtd.Pipe]
        cmp     [ecx+usb_pipe.Type], CONTROL_PIPE
        jz      .nothing
; 2. The hardware expects next packet with toggle = (ErrorTD.toggle xor 1),
; the current value in next packet is (ebx.toggle xor 1).
; Nothing to do if ErrorTD.toggle == ebx.toggle.
        mov     eax, [ecx+uhci_pipe.ErrorTD-uhci_pipe.SoftwarePart]
        mov     eax, [eax+uhci_gtd.Token-uhci_gtd.SoftwarePart]
        xor     eax, [ebx+uhci_gtd.Token-uhci_gtd.SoftwarePart]
        test    eax, 1 shl 19
        jz      .nothing
; 3. Lock the transfer queue.
        add     ecx, usb_pipe.Lock
        call    mutex_lock
; 4. Flip the toggle bit in all packets from ebx.NextVirt to ecx.LastTD
; (inclusive).
        mov     eax, [ebx+usb_gtd.NextVirt]
.loop:
        xor     byte [eax+uhci_gtd.Token-uhci_gtd.SoftwarePart+2], 1 shl (19-16)
        cmp     eax, [ecx+usb_pipe.LastTD-usb_pipe.Lock]
        mov     eax, [eax+usb_gtd.NextVirt]
        jnz     .loop
; 5. Flip the toggle bit in uhci_pipe structure.
        xor     byte [ecx+uhci_pipe.Token-uhci_pipe.SoftwarePart-usb_pipe.Lock+2], 1 shl (19-16)
        or      dword [ecx+uhci_pipe.Token-uhci_pipe.SoftwarePart-usb_pipe.Lock], eax
; 6. Unlock the transfer queue.
        call    mutex_unlock
.nothing:
        ret
endp

; This procedure is called in the USB thread from uhci_process_deferred
; every UHCI_POLL_INTERVAL ticks. It polls the controller for
; connect/disconnect events.
; in: esi -> usb_controller
proc uhci_poll_roothub
        push    ebx     ; save used register to be stdcall
; 1. Prepare for the loop for every port.
        xor     ecx, ecx
.portloop:
; 2. Some implementations of UHCI set ConnectStatusChange bit in a response to
; PortReset. Thus, we must ignore this change for port which is resetting.
        cmp     cl, [esi+usb_controller.ResettingPort]
        jz      .nextport
; 3. Read port status.
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        lea     edx, [edx+ecx*2+UhciPort1StatusReg]
        in      ax, dx
; 4. If no change bits are set, continue to the next port.
        test    al, 0Ah
        jz      .nextport
; 5. Clear change bits and read the status again.
; (It is possible, although quite unlikely, that some event occurs between
; the first read and the clearing, invalidating the old status. If an event
; occurs after the clearing, we will not miss it, looking in the next scan.
        out     dx, ax
        mov     ebx, eax
        in      ax, dx
; 6. Process connect change notifications.
; Note: if connect status has changed, ignore enable status change;
; it is normal to disable a port at disconnect event.
; Some controllers set enable status change bit, some don't.
        test    bl, 2
        jz      .noconnectchange
        DEBUGF 1,'K : [%d] UHCI %x connect status changed, %x/%x\n',[timer_ticks],esi,bx,ax
; yep. Regardless of the current status, note disconnect event;
; if there is something connected, store the connect time and note connect event.
; In any way, do not process 
        bts     [esi+usb_controller.NewDisconnected], ecx
        test    al, 1
        jz      .disconnect
        mov     eax, [timer_ticks]
        mov     [esi+usb_controller.ConnectedTime+ecx*4], eax
        bts     [esi+usb_controller.NewConnected], ecx
        jmp     .nextport
.disconnect:
        btr     [esi+usb_controller.NewConnected], ecx
        jmp     .nextport
.noconnectchange:
; 7. Process enable change notifications.
; Note: that needs work.
        test    bl, 8
        jz      .nextport
        test    al, 4
        jnz     .nextport
        dbgstr 'Port disabled'
.nextport:
; 8. Continue the loop for every port.
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .portloop
        pop     ebx     ; restore used register to be stdcall
        ret
endp

; This procedure is called from uhci_process_deferred when
; a new device was connected at least USB_CONNECT_DELAY ticks
; and therefore is ready to be configured.
; in: esi -> usb_controller, ecx = port (zero-based)
proc uhci_new_port
; test whether we are configuring another port
; if so, postpone configuring and return
        bts     [esi+usb_controller.PendingPorts], ecx
        cmp     [esi+usb_controller.ResettingPort], -1
        jnz     .nothing
        btr     [esi+usb_controller.PendingPorts], ecx
; fall through to uhci_new_port.reset

; This function is called from uhci_new_port and uhci_test_pending_port.
; It starts reset signalling for the port. Note that in USB first stages
; of configuration can not be done for several ports in parallel.
.reset:
; 1. Store information about resetting hub (roothub) and port.
        and     [esi+usb_controller.ResettingHub], 0
        mov     [esi+usb_controller.ResettingPort], cl
; 2. Initiate reset signalling.
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        lea     edx, [edx+ecx*2+UhciPort1StatusReg]
        in      ax, dx
        or      ah, 2
        out     dx, ax
; 3. Store the current time and set status to 1 = reset signalling active.
        mov     eax, [timer_ticks]
        mov     [esi+usb_controller.ResetTime], eax
        mov     [esi+usb_controller.ResettingStatus], 1
.nothing:
        ret
endp

; This procedure is called from uhci_process_deferred when
; reset signalling for a port needs to be finished.
proc uhci_port_reset_done
; 1. Stop reset signalling.
        movzx   ecx, [esi+usb_controller.ResettingPort]
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        lea     edx, [edx+ecx*2+UhciPort1StatusReg]
        in      ax, dx
        DEBUGF 1,'K : [%d] UHCI %x status %x/',[timer_ticks],esi,ax
        and     ah, not 2
        out     dx, ax
; 2. Status bits in UHCI are invalid during reset signalling.
; Wait a millisecond while status bits become valid again.
        push    esi
        push    1
        pop     esi
        call    delay_ms
        pop     esi
; 3. ConnectStatus bit is zero during reset and becomes 1 during step 2;
; some controllers interpret this as a (fake) connect event.
; Enable port and clear status change notification.
        in      ax, dx
        DEBUGF 1,'%x\n',ax
        or      al, 6   ; enable port, clear status change
        out     dx, ax
; 4. Store the current time and set status to 2 = reset recovery active.
        mov     eax, [timer_ticks]
        DEBUGF 1,'K : reset done at %d\n',[timer_ticks]
        mov     [esi+usb_controller.ResetTime], eax
        mov     [esi+usb_controller.ResettingStatus], 2
        ret
endp

; This procedure is called from uhci_process_deferred when
; a new device has been reset, recovered after reset and
; needs to be configured.
; in: esi -> usb_controller
proc uhci_port_init
; 1. Read port status.
        mov     [esi+usb_controller.ResettingStatus], 0
        movzx   ecx, [esi+usb_controller.ResettingPort]
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        lea     edx, [edx+ecx*2+UhciPort1StatusReg]
        in      ax, dx
        DEBUGF 1,'K : [%d] UHCI %x status %x\n',[timer_ticks],esi,ax
; 2. If the device has been disconnected, stop the initialization.
        test    al, 1
        jnz     @f
        dbgstr 'USB port disabled after reset'
        jmp     usb_test_pending_port
@@:
; 3. Copy LowSpeed bit to bit 0 of eax and call the worker procedure
; to notify the protocol layer about new UHCI device.
        push    edx
        mov     al, ah
        call    uhci_new_device
        pop     edx
        test    eax, eax
        jnz     .nothing
; 4. If something at the protocol layer has failed
; (no memory, no bus address), disable the port and stop the initialization.
.disable_exit:
        in      ax, dx
        and     al, not 4
        out     dx, ax  ; disable the port
        jmp     usb_test_pending_port
.nothing:
        ret
endp

; This procedure is called from uhci_port_init and from hub support code
; when a new device is connected and has been reset.
; It calls usb_new_device at the protocol layer with correct parameters.
; in: esi -> usb_controller, eax = speed;
; UHCI is USB1 device, so only low bit of eax (LowSpeed) is used.
proc uhci_new_device
; 1. Clear all bits of speed except bit 0.
        and     eax, 1
; 2. Store the speed for the protocol layer.
        mov     [esi+usb_controller.ResettingSpeed], al
; 3. Create pseudo-pipe in the stack.
; See uhci_init_pipe: only .Controller and .Token fields are used.
        push    esi     ; fill .Controller field
        mov     ecx, esp
        shl     eax, 20 ; bit 20 = LowSpeedDevice
        push    eax     ; ignored (ErrorTD)
        push    eax     ; .Token field: DeviceAddress is zero, bit 20 = LowSpeedDevice
; 4. Notify the protocol layer.
        call    usb_new_device
; 5. Cleanup the stack after step 3 and return.
        add     esp, 12
        ret
endp

; This procedure is called from usb_set_address_callback
; and stores USB device address in the uhci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe, cl = address
proc uhci_set_device_address
        mov     byte [ebx+uhci_pipe.Token+1-uhci_pipe.SoftwarePart], cl
        call    usb_subscription_done
        ret
endp

; This procedure returns USB device address from the uhci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe
; out: eax = endpoint address
proc uhci_get_device_address
        mov     al, byte [ebx+uhci_pipe.Token+1-uhci_pipe.SoftwarePart]
        and     eax, 7Fh
        ret
endp

; This procedure is called from usb_set_address_callback
; if the device does not accept SET_ADDRESS command and needs
; to be disabled at the port level.
; in: esi -> usb_controller, ecx = port (zero-based)
proc uhci_port_disable
        mov     edx, [esi+uhci_controller.IOBase-sizeof.uhci_controller]
        lea     edx, [edx+UhciPort1StatusReg+ecx*2]
        in      ax, dx
        and     al, not 4
        out     dx, ax
        ret
endp

; This procedure is called from usb_get_descr8_callback when
; the packet size for zero endpoint becomes known and
; stores the packet size in uhci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe, ecx = packet size
proc uhci_set_endpoint_packet_size
        dec     ecx
        shl     ecx, 21
        and     [ebx+uhci_pipe.Token-uhci_pipe.SoftwarePart], (1 shl 21) - 1
        or      [ebx+uhci_pipe.Token-uhci_pipe.SoftwarePart], ecx
; uhci_pipe.Token field is purely for software bookkeeping and does not affect
; the hardware; thus, we can continue initialization immediately.
        call    usb_subscription_done
        ret
endp

; This procedure is called from API usb_open_pipe and processes
; the controller-specific part of this API. See docs.
; in: edi -> usb_pipe for target, ecx -> usb_pipe for config pipe,
; esi -> usb_controller, eax -> usb_gtd for the first TD,
; [ebp+12] = endpoint, [ebp+16] = maxpacket, [ebp+20] = type
proc uhci_init_pipe
; inherit some variables from the parent usb_open_pipe
virtual at ebp+8
.config_pipe    dd      ?
.endpoint       dd      ?
.maxpacket      dd      ?
.type           dd      ?
.interval       dd      ?
end virtual
; 1. Initialize ErrorTD to zero.
        and     [edi+uhci_pipe.ErrorTD-uhci_pipe.SoftwarePart], 0
; 2. Initialize HeadTD to the physical address of the first TD.
        push    eax     ; store pointer to the first TD for step ?
        sub     eax, uhci_gtd.SoftwarePart
        call    get_phys_addr
        mov     [edi+uhci_pipe.HeadTD-uhci_pipe.SoftwarePart], eax
; 3. Initialize Token field:
; take DeviceAddress and LowSpeedDevice from the parent pipe,
; take Endpoint and MaximumLength fields from API arguments,
; set PID depending on pipe type and provided pipe direction,
; set DataToggle to zero.
        mov     eax, [ecx+uhci_pipe.Token-uhci_pipe.SoftwarePart]
        and     eax, 0x107F00   ; keep DeviceAddress and LowSpeedDevice
        mov     edx, [.endpoint]
        and     edx, 15
        shl     edx, 15
        or      eax, edx
        mov     edx, [.maxpacket]
        dec     edx
        shl     edx, 21
        or      eax, edx
        mov     al, USB_PID_SETUP
        cmp     [.type], CONTROL_PIPE
        jz      @f
        mov     al, USB_PID_OUT
        test    byte [.endpoint], 80h
        jz      @f
        mov     al, USB_PID_IN
@@:
        mov     [edi+uhci_pipe.Token-uhci_pipe.SoftwarePart], eax
; 4. Initialize the first TD:
; copy Token from uhci_pipe.Token zeroing reserved bit 20,
; set ControlStatus for future transfers, bit make it inactive,
; set bit 0 in NextTD = "no next TD".
        pop     edx     ; restore pointer saved in step 2
        mov     [edx+uhci_gtd.Token-uhci_gtd.SoftwarePart], eax
        and     byte [edx+uhci_gtd.Token+2-uhci_gtd.SoftwarePart], not (1 shl (20-16))
        and     eax, 1 shl 20
        shl     eax, 6
        or      eax, UHCI_INVALID_LENGTH + (3 shl 27)
                ; not processed, inactive, allow 3 errors
        mov     [edx+uhci_gtd.ControlStatus-uhci_gtd.SoftwarePart], eax
        mov     [edx+uhci_gtd.NextTD-uhci_gtd.SoftwarePart], 1
; 5. Select the corresponding list and insert to the list.
; 5a. Use Control list for control pipes, Bulk list for bulk pipes.
        lea     edx, [esi+uhci_controller.ControlED.SoftwarePart-sizeof.uhci_controller]
        cmp     [.type], BULK_PIPE
        jb      .insert ; control pipe
        lea     edx, [esi+uhci_controller.BulkED.SoftwarePart-sizeof.uhci_controller]
        jz      .insert ; bulk pipe
.interrupt_pipe:
; 5b. For interrupt pipes, let the scheduler select the appropriate list
; based on the current bandwidth distribution and the requested bandwidth.
; This could fail if the requested bandwidth is not available;
; if so, return an error.
        lea     edx, [esi + uhci_controller.IntEDs - sizeof.uhci_controller]
        lea     eax, [esi + uhci_controller.IntEDs + 32*sizeof.uhci_static_ep - sizeof.uhci_controller]
        push    64
        pop     ecx
        call    usb1_select_interrupt_list
        test    edx, edx
        jz      .return0
.insert:
; Insert to the head of the corresponding list.
; Note: inserting to the head guarantees that the list traverse in
; uhci_process_updated_schedule, once started, will not interact with new pipes.
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
        mov     ecx, [edx+uhci_static_ep.NextQH-uhci_static_ep.SoftwarePart]
        mov     [edi+uhci_pipe.NextQH-uhci_pipe.SoftwarePart], ecx
        lea     eax, [edi-uhci_pipe.SoftwarePart]
        call    get_phys_addr
        inc     eax
        inc     eax
        mov     [edx+uhci_static_ep.NextQH-uhci_static_ep.SoftwarePart], eax
; 6. Return with nonzero eax.
        ret
.return0:
        xor     eax, eax
        ret
endp

; This procedure is called when a pipe is closing (either due to API call
; or due to disconnect); it unlinks a pipe from the corresponding list.
if uhci_static_ep.SoftwarePart <> uhci_pipe.SoftwarePart
.err uhci_unlink_pipe assumes that uhci_static_ep.SoftwarePart == uhci_pipe.SoftwarePart
end if
proc uhci_unlink_pipe
        cmp     [ebx+usb_pipe.Type], INTERRUPT_PIPE
        jnz     @f
        mov     eax, [ebx+uhci_pipe.Token-uhci_pipe.SoftwarePart]
        cmp     al, USB_PID_IN
        setz    ch
        bt      eax, 20
        setc    cl
        add     eax, 1 shl 21
        shr     eax, 21
        stdcall usb1_interrupt_list_unlink, eax, ecx
@@:
; Note: we need to ensure that NextVirt field of the pipe is not modified;
; this procedure can be called while uhci_process_updated_schedule processes
; the same pipe, and it needs a correct NextVirt field to continue.
        mov     edx, [ebx+usb_pipe.NextVirt]
        mov     eax, [ebx+usb_pipe.PrevVirt]
        mov     [edx+usb_pipe.PrevVirt], eax
        mov     [eax+usb_pipe.NextVirt], edx
; Note: eax could be either usb_pipe or usb_static_ep;
; fortunately, NextQH and SoftwarePart have same offsets in both.
        mov     edx, [ebx+uhci_pipe.NextQH-uhci_pipe.SoftwarePart]
        mov     [eax+uhci_pipe.NextQH-uhci_pipe.SoftwarePart], edx
        ret
endp

; Free memory associated with pipe.
; For UHCI, this includes usb_pipe structure and ErrorTD, if present.
proc uhci_free_pipe
        mov     eax, [esp+4]
        mov     eax, [eax+uhci_pipe.ErrorTD-uhci_pipe.SoftwarePart]
        test    eax, eax
        jz      @f
        stdcall uhci_free_td, eax
@@:
        jmp     usb1_free_endpoint
endp

; This procedure is called from the several places in main USB code
; and allocates required packets for the given transfer stage.
; ebx = pipe, other parameters are passed through the stack
proc uhci_alloc_transfer stdcall uses edi, buffer:dword, size:dword, flags:dword, td:dword, direction:dword
locals
token           dd      ?
origTD          dd      ?
packetSize      dd      ?       ; must be the last variable, see usb_init_transfer
endl
; 1. [td] will be the first packet in the transfer.
; Save it to allow unrolling if something will fail.
        mov     eax, [td]
        mov     [origTD], eax
; In UHCI one TD describes one packet, transfers should be split into parts
; with size <= endpoint max packet size.
; 2. Get the maximum packet size for endpoint from uhci_pipe.Token
; and generate Token field for TDs.
        mov     edi, [ebx+uhci_pipe.Token-uhci_pipe.SoftwarePart]
        mov     eax, edi
        shr     edi, 21
        inc     edi
; zero packet size (it will be set for every packet individually),
; zero reserved bit 20,
        and     eax, (1 shl 20) - 1
        mov     [packetSize], edi
; set the correct PID if it is different from the pipe-wide PID
; (Data and Status stages of control transfers),
        mov     ecx, [direction]
        and     ecx, 3
        jz      @f
        mov     al, USB_PID_OUT
        dec     ecx
        jz      @f
        mov     al, USB_PID_IN
@@:
; set the toggle bit for control transfers,
        mov     ecx, [direction]
        test    cl, 1 shl 3
        jz      @f
        and     ecx, 1 shl 2
        and     eax, not (1 shl 19)
        shl     ecx, 19-2
        or      eax, ecx
@@:
; store the resulting Token in the stack variable.
        mov     [token], eax
; 3. While the remaining data cannot fit in one packet,
; allocate full packets (of maximal possible size).
.fullpackets:
        cmp     [size], edi
        jbe     .lastpacket
        call    uhci_alloc_packet
        test    eax, eax
        jz      .fail
        mov     [td], eax
        add     [buffer], edi
        sub     [size], edi
        jmp     .fullpackets
.lastpacket:
; 4. The remaining data can fit in one packet;
; allocate the last packet with size = size of remaining data.
        mov     eax, [size]
        mov     [packetSize], eax
        call    uhci_alloc_packet
        test    eax, eax
        jz      .fail
; 5. Clear 'short packets are not allowed' bit for the last packet,
; if the caller requested this.
; Note: even if the caller says that short transfers are ok,
; all packets except the last one are marked as 'must be complete':
; if one of them will be short, the software intervention is needed
; to skip remaining packets; uhci_process_finalized_td will handle this
; transparently to the caller.
        test    [flags], 1
        jz      @f
        and     byte [ecx+uhci_gtd.ControlStatus+3-uhci_gtd.SoftwarePart], not (1 shl (29-24))
        and     byte [ecx+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart], not 1
@@:
; 6. Update toggle bit in uhci_pipe structure from current value of [token].
        mov     edx, [token]
        xor     edx, [ebx+uhci_pipe.Token-uhci_pipe.SoftwarePart]
        and     edx, 1 shl 19
        xor     [ebx+uhci_pipe.Token-uhci_pipe.SoftwarePart], edx
.nothing:
        ret
.fail:
        mov     edi, uhci_hardware_func
        mov     eax, [td]
        stdcall usb_undo_tds, [origTD]
        xor     eax, eax
        jmp     .nothing
endp

; Helper procedure for uhci_alloc_transfer. Allocates one packet.
proc uhci_alloc_packet
; inherit some variables from the parent uhci_alloc_transfer
virtual at ebp-12
.token          dd      ?
.origTD         dd      ?
.packetSize     dd      ?
                rd      2
.buffer         dd      ?
.transferSize   dd      ?
.Flags          dd      ?
.td             dd      ?
.direction      dd      ?
end virtual
; 1. In UHCI all data for one packet must be on the same page.
; Thus, if the given buffer splits page boundary, we need a temporary buffer
; and code that transfers data between the given buffer and the temporary one.
; 1a. There is no buffer for zero-length packets.
        xor     eax, eax
        cmp     [.packetSize], eax
        jz      .notempbuf
; 1b. A temporary buffer is not required if the first and the last bytes
; of the given buffer are the same except lower 12 bits.
        mov     edx, [.buffer]
        add     edx, [.packetSize]
        dec     edx
        xor     edx, [.buffer]
        test    edx, -0x1000
        jz      .notempbuf
; 1c. We need a temporary buffer. Allocate [packetSize]*2 bytes, so that
; there must be [packetSize] bytes on one page,
; plus space for a header uhci_original_buffer.
        push    ebx
        mov     eax, [.packetSize]
        add     eax, eax
        add     eax, sizeof.uhci_original_buffer
        call    malloc
        pop     ebx
; 1d. If failed, return zero.
        test    eax, eax
        jz      .nothing
; 1e. Test whether [.packetSize] bytes starting from
; eax + sizeof.uhci_original_buffer are in the same page.
; If so, use eax + sizeof.uhci_original_buffer as a temporary buffer.
; Otherwise, use the beginning of the next page as a temporary buffer
; (since we have overallocated, sufficient space must remain).
        lea     ecx, [eax+sizeof.uhci_original_buffer]
        mov     edx, ecx
        add     edx, [.packetSize]
        dec     edx
        xor     edx, ecx
        test    edx, -0x1000
        jz      @f
        mov     ecx, eax
        or      ecx, 0xFFF
        inc     ecx
@@:
        mov     [eax+uhci_original_buffer.UsedBuffer], ecx
        mov     ecx, [.buffer]
        mov     [eax+uhci_original_buffer.OrigBuffer], ecx
; 1f. For SETUP and OUT packets, copy data from the given buffer
; to the temporary buffer now. For IN packets, data go in other direction
; when the transaction completes.
        cmp     byte [.token], USB_PID_IN
        jz      .nocopy
        push    esi edi
        mov     esi, ecx
        mov     edi, [eax+uhci_original_buffer.UsedBuffer]
        mov     ecx, [.packetSize]
        mov     edx, ecx
        shr     ecx, 2
        and     edx, 3
        rep movsd
        mov     ecx, edx
        rep movsb
        pop     edi esi
.nocopy:
.notempbuf:
; 2. Allocate the next TD.
        push    eax
        call    usb1_allocate_general_td
        pop     edx
; If failed, free the temporary buffer (if it was allocated) and return zero.
        test    eax, eax
        jz      .fail
; 3. Initialize controller-independent parts of both TDs.
        push    edx
        call    usb_init_transfer
; 4. Initialize the next TD:
; mark it as last one (this will be changed when further packets will be
; allocated), copy Token field from uhci_pipe.Token zeroing bit 20,
; generate ControlStatus field, mark as Active
; (for last descriptor, this will be changed by uhci_insert_transfer).
        mov     [eax+uhci_gtd.NextTD-uhci_gtd.SoftwarePart], 1  ; no next TD
        mov     edx, [ebx+uhci_pipe.Token-uhci_pipe.SoftwarePart]
        mov     [eax+uhci_gtd.Token-uhci_gtd.SoftwarePart], edx
        and     byte [eax+uhci_gtd.Token+2-uhci_gtd.SoftwarePart], not (1 shl (20-16))
        and     edx, 1 shl 20
        shl     edx, 6
        or      edx, UHCI_INVALID_LENGTH + (1 shl 23) + (3 shl 27)
                ; not processed, active, allow 3 errors
        mov     [eax+uhci_gtd.ControlStatus-uhci_gtd.SoftwarePart], edx
; 5. Initialize remaining fields of the current TD.
; 5a. Store pointer to the buffer allocated in step 1 (or zero).
        pop     [ecx+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart]
; 5b. Store physical address of the next TD.
        push    eax
        sub     eax, uhci_gtd.SoftwarePart
        call    get_phys_addr
; use Depth traversal unless this is the first TD in the transfer stage;
; uhci_insert_transfer will set Depth traversal for the first TD and clear
; it in the last TD
        cmp     ecx, [ebx+usb_pipe.LastTD]
        jz      @f
        or      eax, 4
@@:
        mov     [ecx+uhci_gtd.NextTD-uhci_gtd.SoftwarePart], eax
; 5c. Store physical address of the buffer: zero if no data present,
; the temporary buffer if it was allocated, the given buffer otherwise.
        xor     eax, eax
        cmp     [.packetSize], eax
        jz      .hasphysbuf
        mov     eax, [.buffer]
        mov     edx, [ecx+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart]
        test    edx, edx
        jz      @f
        mov     eax, [edx+uhci_original_buffer.UsedBuffer]
@@:
        call    get_phys_addr
.hasphysbuf:
        mov     [ecx+uhci_gtd.Buffer-uhci_gtd.SoftwarePart], eax
; 5d. For IN transfers, disallow short packets.
; This will be overridden, if needed, by uhci_alloc_transfer.
        mov     eax, [.token]
        mov     edx, [.packetSize]
        dec     edx
        cmp     al, USB_PID_IN
        jnz     @f
        or      byte [ecx+uhci_gtd.ControlStatus+3-uhci_gtd.SoftwarePart], 1 shl (29-24)        ; disallow short packets
        or      byte [ecx+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart], 1
@@:
; 5e. Get Token field: combine [.token] with [.packetSize].
        shl     edx, 21
        or      edx, eax
        mov     [ecx+uhci_gtd.Token-uhci_gtd.SoftwarePart], edx
; 6. Flip toggle bit in [.token].
        xor     eax, 1 shl 19
        mov     [.token], eax
; 7. Return pointer to the next TD.
        pop     eax
.nothing:
        ret
.fail:
        xchg    eax, edx
        call    free
        xor     eax, eax
        ret
endp

; This procedure is called from the several places in main USB code
; and activates the transfer which was previously allocated by
; uhci_alloc_transfer.
; ecx -> last descriptor for the transfer, ebx -> usb_pipe
proc uhci_insert_transfer
;       DEBUGF 1,'K : uhci_insert_transfer: eax=%x, ecx=%x, [esp+4]=%x\n',eax,ecx,[esp+4]
        and     byte [eax+uhci_gtd.ControlStatus+2-uhci_gtd.SoftwarePart], not (1 shl (23-16))  ; clear Active bit
        or      byte [ecx+uhci_gtd.ControlStatus+3-uhci_gtd.SoftwarePart], 1 shl (24-24)        ; set InterruptOnComplete bit
        mov     eax, [esp+4]
        or      byte [eax+uhci_gtd.ControlStatus+2-uhci_gtd.SoftwarePart], 1 shl (23-16)        ; set Active bit
        or      byte [eax+uhci_gtd.NextTD-uhci_gtd.SoftwarePart], 4     ; set Depth bit
        ret
endp

; Free all memory associated with one TD.
; For UHCI, this includes memory for uhci_gtd itself
; and the temporary buffer, if present.
proc uhci_free_td
        mov     eax, [esp+4]
        mov     eax, [eax+uhci_gtd.OrigBufferInfo-uhci_gtd.SoftwarePart]
        and     eax, not 1
        jz      .nobuf
        push    ebx
        call    free
        pop     ebx
.nobuf:
        sub     dword [esp+4], uhci_gtd.SoftwarePart
        jmp     usb_free_common
endp
