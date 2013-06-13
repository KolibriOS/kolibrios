; Code for OHCI controllers.
; Note: it should be moved to an external driver,
; it was convenient to have this code compiled into the kernel during initial
; development, but there are no reasons to keep it here.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; OHCI register declarations
; All of the registers should be read and written as Dwords.
; Partition 1. Control and Status registers.
OhciRevisionReg         = 0
OhciControlReg          = 4
OhciCommandStatusReg    = 8
OhciInterruptStatusReg  = 0Ch
OhciInterruptEnableReg  = 10h
OhciInterruptDisableReg = 14h
; Partition 2. Memory Pointer registers.
OhciHCCAReg             = 18h
OhciPeriodCurrentEDReg  = 1Ch
OhciControlHeadEDReg    = 20h
OhciControlCurrentEDReg = 24h
OhciBulkHeadEDReg       = 28h
OhciBulkCurrentEDReg    = 2Ch
OhciDoneHeadReg         = 30h
; Partition 3. Frame Counter registers.
OhciFmIntervalReg       = 34h
OhciFmRemainingReg      = 38h
OhciFmNumberReg         = 3Ch
OhciPeriodicStartReg    = 40h
OhciLSThresholdReg      = 44h
; Partition 4. Root Hub registers.
OhciRhDescriptorAReg    = 48h
OhciRhDescriptorBReg    = 4Ch
OhciRhStatusReg         = 50h
OhciRhPortStatusReg     = 54h

; =============================================================================
; ================================ Structures =================================
; =============================================================================

; OHCI-specific part of a pipe descriptor.
; * This structure corresponds to the Endpoint Descriptor aka ED from the OHCI
;   specification.
; * The hardware requires 16-bytes alignment of the hardware part.
;   Since the allocator (usb_allocate_common) allocates memory sequentially
;   from page start (aligned on 0x1000 bytes), block size for the allocator
;   must be divisible by 16; usb1_allocate_endpoint ensures this.
struct ohci_pipe
; All addresses are physical.
Flags           dd      ?
; 1. Lower 7 bits (bits 0-6) are FunctionAddress. This is the USB address of
;    the function containing the endpoint that this ED controls.
; 2. Next 4 bits (bits 7-10) are EndpointNumber. This is the USB address of
;    the endpoint within the function.
; 3. Next 2 bits (bits 11-12) are Direction. This 2-bit field indicates the
;    direction of data flow: 1 = IN, 2 = OUT. If neither IN nor OUT is
;    specified, then the direction is determined from the PID field of the TD.
;    For CONTROL endpoints, the transfer direction is different
;    for different transfers, so the value of this field is 0
;    (3 would have the same effect) and the actual direction
;    of one transfer is encoded in the Transfer Descriptor.
; 4. Next bit (bit 13) is Speed bit. It indicates the speed of the endpoint:
;    full-speed (S = 0) or low-speed (S = 1).
; 5. Next bit (bit 14) is sKip bit. When this bit is set, the hardware
;    continues on to the next ED on the list without attempting access
;    to the TD queue or issuing any USB token for the endpoint.
;    Always cleared.
; 6. Next bit (bit 15) is Format bit. It must be 0 for Control, Bulk and
;    Interrupt endpoints and 1 for Isochronous endpoints.
; 7. Next 11 bits (bits 16-26) are MaximumPacketSize. This field indicates
;    the maximum number of bytes that can be sent to or received from the
;    endpoint in a single data packet.
TailP           dd      ?
; Physical address of the tail descriptor in the TD queue.
; The descriptor itself is not in the queue. See also HeadP.
HeadP           dd      ?
; 1. First bit (bit 0) is Halted bit. This bit is set by the hardware to
;    indicate that processing of the TD queue on the endpoint is halted.
; 2. Second bit (bit 1) is toggleCarry bit. Whenever a TD is retired, this
;    bit is written to contain the last data toggle value from the retired TD.
; 3. Next two bits (bits 2-3) are reserved and always zero.
; 4. With masked 4 lower bits, this is HeadP itself: physical address of the
;    head descriptor in the TD queue, that is, next TD to be processed for this
;    endpoint. Note that a TD must be 16-bytes aligned.
;    Empty queue is characterized by the condition HeadP == TailP.
NextED          dd      ?
; If nonzero, then this entry is a physical address of the next ED to be
; processed. See also the description before NextVirt field of the usb_pipe
; structure. Additionally to that description, the following is specific for
; the OHCI controller:
; * n=5, N=32, there are 32 "leaf" periodic lists.
; * The 1ms periodic list also serves Isochronous endpoints, which should be
;   in the end of the list.
; * There is no "next" list for Bulk and Control lists, they are processed
;   separately from others.
; * There is no "next" list for Periodic list for 1ms interval.
ends

; This structure describes the static head of every list of pipes.
; The hardware requires 16-bytes alignment of this structure.
; All instances of this structure are located sequentially in uhci_controller,
; uhci_controller is page-aligned, so it is sufficient to make this structure
; 16-bytes aligned and verify that the first instance is 16-bytes aligned
; inside uhci_controller.
struct ohci_static_ep
Flags           dd      ?
; Same as ohci_pipe.Flags.
; sKip bit is set, so the hardware ignores other fields except NextED.
                dd      ?
; Corresponds to ohci_pipe.TailP. Not used.
NextList        dd      ?
; Virtual address of the next list.
NextED          dd      ?
; Same as ohci_pipe.NextED.
SoftwarePart    rd      sizeof.usb_static_ep/4
; Software part, common for all controllers.
                dd      ?
; Padding for 16-bytes alignment.
ends

if sizeof.ohci_static_ep mod 16
.err ohci_static_ep must be 16-bytes aligned
end if

; OHCI-specific part of controller data.
; * The structure describes the memory area used for controller data,
;   additionally to the registers of the controller.
; * The structure includes two parts, the hardware part and the software part.
; * The hardware part consists of first 256 bytes and corresponds to
;   the HCCA from OHCI specification.
; * The hardware requires 256-bytes alignment of the hardware part, so
;   the entire descriptor must be 256-bytes aligned.
;   This structure is allocated with kernel_alloc (see usb_init_controller),
;   this gives page-aligned data.
; * The controller is described by both ohci_controller and usb_controller
;   structures, for each controller there is one ohci_controller and one
;   usb_controller structure. These structures are located sequentially
;   in the memory: beginning from some page start, there is ohci_controller
;   structure - this enforces hardware alignment requirements - and then
;   usb_controller structure.
; * The code keeps pointer to usb_controller structure. The ohci_controller
;   structure is addressed as [ptr + ohci_controller.field - sizeof.ohci_controller].
struct ohci_controller
; ------------------------------ hardware fields ------------------------------
InterruptTable  rd      32
; Pointers to interrupt EDs. The hardware starts processing of periodic lists
; within the frame N from the ED pointed to by [InterruptTable+(N and 31)*4].
; See also the description of periodic lists inside ohci_pipe structure.
FrameNumber     dw      ?
; The current frame number. This field is written by hardware only.
; This field is read by ohci_process_deferred and ohci_irq to
; communicate when control/bulk processing needs to be temporarily
; stopped/restarted.
                dw      ?
; Padding. Written as zero at every update of FrameNumber.
DoneHead        dd      ?
; Physical pointer to the start of Done Queue.
; When the hardware updates this field, it sets bit 0 to one if there is
; unmasked interrupt pending.
                rb      120
; Reserved for the hardware.
; ------------------------------ software fields ------------------------------
IntEDs          ohci_static_ep
                rb      62 * sizeof.ohci_static_ep
; Heads of 63 Periodic lists, see the description in usb_pipe.
ControlED       ohci_static_ep
; Head of Control list, see the description in usb_pipe.
BulkED          ohci_static_ep
; Head of Bulk list, see the description in usb_pipe.
MMIOBase        dd      ?
; Virtual address of memory-mapped area with OHCI registers OhciXxxReg.
PoweredUp       db      ?
; 1 in normal work, 0 during early phases of the initialization.
; This field is initialized to zero during memory allocation
; (see usb_init_controller), set to one by ohci_init when ports of the root hub
; are powered up, so connect/disconnect events can be handled.
                rb      3 ; alignment
DoneList        dd      ?
; List of descriptors which were processed by the controller and now need
; to be finalized.
DoneListEndPtr  dd      ?
; Pointer to dword which should receive a pointer to the next item in DoneList.
; If DoneList is empty, this is a pointer to DoneList itself;
; otherwise, this is a pointer to NextTD field of the last item in DoneList.
ends

if ohci_controller.IntEDs mod 16
.err Static endpoint descriptors must be 16-bytes aligned inside ohci_controller
end if

; OHCI general transfer descriptor.
; * The structure describes transfers to be performed on Control, Bulk or
;   Interrupt endpoints.
; * The structure includes two parts, the hardware part and the software part.
; * The hardware part consists of first 16 bytes and corresponds to
;   the General Transfer Descriptor aka general TD from OHCI specification.
; * The hardware requires 16-bytes alignment of the hardware part, so
;   the entire descriptor must be 16-bytes aligned. Since the allocator
;   (usb_allocate_common) allocates memory sequentially from page start
;   (aligned on 0x1000 bytes), block size for the allocator must be
;   divisible by 16; usb1_allocate_generic_td ensures this.
struct ohci_gtd
; ------------------------------ hardware fields ------------------------------
; All addresses in this part are physical.
Flags           dd      ?
; 1. Lower 18 bits (bits 0-17) are ignored and not modified by the hardware.
; 2. Next bit (bit 18) is bufferRounding bit. If this bit is 0, then the last
;    data packet must exactly fill the defined data buffer. If this bit is 1,
;    then the last data packet may be smaller than the defined buffer without
;    causing an error condition on the TD.
; 3. Next 2 bits (bits 19-20) are Direction field. This field indicates the
;    direction of data flow. If the Direction field in the ED is OUT or IN,
;    this field is ignored and the direction from the ED is used instead.
;    Otherwise, 0 = SETUP, 1 = OUT, 2 = IN, 3 is reserved.
; 4. Next 3 bits (bits 21-23) are DelayInterrupt field. This field contains
;    the interrupt delay count for this TD. When a TD is complete, the hardware
;    may wait up to DelayInterrupt frames before generating an interrupt.
;    If DelayInterrupt is 7 (maximum possible), then there is no interrupt
;    associated with completion of this TD.
; 5. Next 2 bits (bits 24-25) are DataToggle field. This field is used to
;    generate/compare the data PID value (DATA0 or DATA1). It is updated after
;    each successful transmission/reception of a data packet. The bit 25
;    is 0 when the data toggle value is acquired from the toggleCarry field in
;    the ED and 1 when the data toggle value is taken from the bit 24.
; 6. Next 2 bits (bits 26-27) are ErrorCount field. For each transmission
;    error, this value is incremented. If ErrorCount is 2 and another error
;    occurs, the TD is retired with error. When a transaction completes without
;    error, ErrorCount is reset to 0.
; 7. Upper 4 bits (bits 28-31) are ConditionCode field. This field contains
;    the status of the last attempted transaction, one of USB_STATUS_* values.
CurBufPtr       dd      ?
; Physical address of the next memory location that will be accessed for
; transfer to/from the endpoint. 0 means zero-length data packet or that all
; bytes have been transferred.
NextTD          dd      ?
; This field has different meanings depending on the status of the descriptor.
; When the descriptor is queued for processing, but not yet processed:
;   Physical address of the next TD for the endpoint.
; When the descriptor is processed by hardware, but not yet by software:
;   Physical address of the previous processed TD.
; When the descriptor is processed by the IRQ handler, but not yet completed:
;   Virtual pointer to the next processed TD.
BufEnd          dd      ?
; Physical address of the last byte in the buffer for this TD.
		dd	?	; padding to align with uhci_gtd
ends

; OHCI isochronous transfer descriptor.
; * The structure describes transfers to be performed on Isochronous endpoints.
; * The structure includes two parts, the hardware part and the software part.
; * The hardware part consists of first 32 bytes and corresponds to
;   the Isochronous Transfer Descriptor aka isochronous TD from OHCI
;   specification.
; * The hardware requires 32-bytes alignment of the hardware part, so
;   the entire descriptor must be 32-bytes aligned.
; * The isochronous endpoints are not supported yet, so only hardware part is
;   defined at the moment.
struct ohci_itd
StartingFrame   dw      ?
; This field contains the low order 16 bits of the frame number in which the
; first data packet of the Isochronous TD is to be sent.
Flags           dw      ?
; 1. Lower 5 bits (bits 0-4) are ignored and not modified by the hardware.
; 2. Next 3 bits (bits 5-7) are DelayInterrupt field.
; 3. Next 3 bits (bits 8-10) are FrameCount field. The TD describes
;    FrameCount+1 data packets.
; 4. Next bit (bit 11) is ignored and not modified by the hardware.
; 5. Upper 4 bits (bits 12-15) are ConditionCode field. This field contains
;    the completion code, one of USB_STATUS_* values, when the TD is moved to
;    the Done Queue.
BufPage0        dd      ?
; Lower 12 bits are ignored and not modified by the hardware.
; With masked 12 bits this field is the physical page containing all buffers.
NextTD          dd      ?
; Physical address of the next TD in the transfer queue.
BufEnd          dd      ?
; Physical address of the last byte in the buffer.
OffsetArray     rw      8
; Initialized by software, read by hardware: Offset for packet 0..7.
; Used to determine size and starting address of an isochronous data packet.
; Written by hardware, read by software: PacketStatusWord for packet 0..7.
; Contains completion code and, if applicable, size received for an isochronous
; data packet.
ends

; Description of OHCI-specific data and functions for
; controller-independent code.
; Implements the structure usb_hardware_func from hccommon.inc for OHCI.
iglobal
align 4
ohci_hardware_func:
        dd      'OHCI'
        dd      sizeof.ohci_controller
        dd      ohci_init
        dd      ohci_process_deferred
        dd      ohci_set_device_address
        dd      ohci_get_device_address
        dd      ohci_port_disable
        dd      ohci_new_port.reset
        dd      ohci_set_endpoint_packet_size
        dd      usb1_allocate_endpoint
        dd      usb1_free_endpoint
        dd      ohci_init_pipe
        dd      ohci_unlink_pipe
        dd      usb1_allocate_general_td
        dd      usb1_free_general_td
        dd      ohci_alloc_transfer
        dd      ohci_insert_transfer
        dd      ohci_new_device
endg

; =============================================================================
; =================================== Code ====================================
; =============================================================================

; Controller-specific initialization function.
; Called from usb_init_controller. Initializes the hardware and
; OHCI-specific parts of software structures.
; eax = pointer to ohci_controller to be initialized
; [ebp-4] = pcidevice
proc ohci_init
; inherit some variables from the parent (usb_init_controller)
.devfn   equ ebp - 4
.bus     equ ebp - 3
; 1. Store pointer to ohci_controller for further use.
        push    eax
        mov     edi, eax
; 2. Initialize hardware fields of ohci_controller.
; Namely, InterruptTable needs to be initialized with
; physical addresses of heads of first 32 Periodic lists.
; Note that all static heads fit in one page, so one call
; to get_pg_addr is sufficient.
if (ohci_controller.IntEDs / 0x1000) <> (ohci_controller.BulkED / 0x1000)
.err assertion failed
end if
if ohci_controller.IntEDs >= 0x1000
.err assertion failed
end if
        lea     esi, [eax+ohci_controller.IntEDs+32*sizeof.ohci_static_ep]
        call    get_pg_addr
        add     eax, ohci_controller.IntEDs
        movi    ecx, 32
        mov     edx, ecx
@@:
        stosd
        add     eax, sizeof.ohci_static_ep
        loop    @b
; 3. Initialize static heads ohci_controller.IntEDs, .ControlED, .BulkED.
; Use the loop over groups: first group consists of first 32 Periodic
; descriptors, next group consists of next 16 Periodic descriptors,
; ..., last group consists of the last Periodic descriptor.
; 3a. Prepare for the loop.
; make edi point to start of ohci_controller.IntEDs,
; other registers are already set.
; -128 fits in one byte, +128 does not fit.
        sub     edi, -128
; 3b. Loop over groups. On every iteration:
; edx = size of group, edi = pointer to the current group,
; esi = pointer to the next group, eax = physical address of the next group.
.init_static_eds:
; 3c. Get the size of the next group.
        shr     edx, 1
; 3d. Exit the loop if there is no next group.
        jz      .init_static_eds_done
; 3e. Initialize the first half of the current group.
; Advance edi to the second half.
        push    eax esi
        call    ohci_init_static_ep_group
        pop     esi eax
; 3f. Initialize the second half of the current group
; with the same values.
; Advance edi to the next group, esi/eax to the next of the next group.
        call    ohci_init_static_ep_group
        jmp     .init_static_eds
.init_static_eds_done:
; 3g. Initialize the head of the last Periodic list.
        xor     eax, eax
        xor     esi, esi
        call    ohci_init_static_endpoint
; 3i. Initialize the heads of Control and Bulk lists.
        call    ohci_init_static_endpoint
        call    ohci_init_static_endpoint
; 4. Create a virtual memory area to talk with the controller.
; 4a. Enable memory & bus master access.
        mov     ch, [.bus]
        mov     cl, 0
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
        and     al, not 0Fh
; 4c. Create mapping for physical memory. 256 bytes are sufficient.
        stdcall map_io_mem, eax, 100h, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .fail
        stosd   ; fill ohci_controller.MMIOBase
        xchg    eax, edi
; now edi = MMIOBase
; 5. Reset the controller if needed.
; 5a. Check operational state.
; 0 = reset, 1 = resume, 2 = operational, 3 = suspended
        mov     eax, [edi+OhciControlReg]
        and     al, 3 shl 6
        cmp     al, 2 shl 6
        jz      .operational
; 5b. State is not operational, reset is needed.
.reset:
; 5c. Save FmInterval register.
        pushd   [edi+OhciFmIntervalReg]
; 5d. Issue software reset and wait up to 10ms, checking status every 1 ms.
        movi    ecx, 1
        movi    edx, 10
        mov     [edi+OhciCommandStatusReg], ecx
@@:
        mov     esi, ecx
        call    delay_ms
        test    [edi+OhciCommandStatusReg], ecx
        jz      .resetdone
        dec     edx
        jnz     @b
        pop     eax
        dbgstr 'controller reset timeout'
        jmp     .fail_unmap
.resetdone:
; 5e. Restore FmInterval register.
        pop     eax
        mov     edx, eax
        and     edx, 3FFFh
        jz      .setfminterval
        cmp     dx, 2EDFh       ; default value?
        jnz     @f              ; assume that BIOS has configured the value
.setfminterval:
        mov     eax, 27792EDFh  ; default value
@@:
        mov     [edi+OhciFmIntervalReg], eax
; 5f. Set PeriodicStart to 90% of FmInterval.
        movzx   eax, ax
; Two following lines are equivalent to eax = floor(eax * 0.9)
; for any 0 <= eax < 1C71C71Dh, which of course is far from maximum 0FFFFh.
        mov     edx, 0E6666667h
        mul     edx
        mov     [edi+OhciPeriodicStartReg], edx
.operational:
; 6. Setup controller registers.
        pop     esi     ; restore pointer to ohci_controller saved in step 1
; 6a. Physical address of HCCA.
        mov     eax, esi
        call    get_pg_addr
        mov     [edi+OhciHCCAReg], eax
; 6b. Transition to operational state and clear all Enable bits.
        mov     cl, 2 shl 6
        mov     [edi+OhciControlReg], ecx
; 6c. Physical addresses of head of Control and Bulk lists.
if ohci_controller.BulkED >= 0x1000
.err assertion failed
end if
        add     eax, ohci_controller.ControlED
        mov     [edi+OhciControlHeadEDReg], eax
        add     eax, ohci_controller.BulkED - ohci_controller.ControlED
        mov     [edi+OhciBulkHeadEDReg], eax
; 6d. Zero Head registers: there are no active Control and Bulk descriptors yet.
        xor     eax, eax
;       mov     [edi+OhciPeriodCurrentEDReg], eax
        mov     [edi+OhciControlCurrentEDReg], eax
        mov     [edi+OhciBulkCurrentEDReg], eax
;       mov     [edi+OhciDoneHeadReg], eax
; 6e. Enable processing of all lists with control:bulk ratio = 1:1.
        mov     dword [edi+OhciControlReg], 10111100b
; 7. Get number of ports.
        add     esi, sizeof.ohci_controller
        mov     eax, [edi+OhciRhDescriptorAReg]
        and     eax, 0xF
        mov     [esi+usb_controller.NumPorts], eax
; 8. Initialize DoneListEndPtr to point to DoneList.
        lea     eax, [esi+ohci_controller.DoneList-sizeof.ohci_controller]
        mov     [esi+ohci_controller.DoneListEndPtr-sizeof.ohci_controller], eax
; 9. Hook interrupt.
        mov     ah, [.bus]
        mov     al, 0
        mov     bh, [.devfn]
        mov     bl, 3Ch
        call    pci_read_reg
; al = IRQ
        movzx   eax, al
        stdcall attach_int_handler, eax, ohci_irq, esi
; 10. Enable controller interrupt on HcDoneHead writeback and RootHubStatusChange.
        mov     dword [edi+OhciInterruptEnableReg], 80000042h
        DEBUGF 1,'K : OHCI controller at %x:%x with %d ports initialized\n',[.bus]:2,[.devfn]:2,[esi+usb_controller.NumPorts]
; 11. Initialize ports of the controller.
; 11a. Initiate power up, disable all ports, clear all "changed" bits.
        mov     dword [edi+OhciRhStatusReg], 10000h     ; SetGlobalPower
        xor     ecx, ecx
@@:
        mov     dword [edi+OhciRhPortStatusReg+ecx*4], 1F0101h  ; SetPortPower+ClearPortEnable+clear "changed" bits
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      @b
; 11b. Wait for power up.
; VirtualBox has AReg == 0, delay_ms doesn't like zero value; ignore zero delay
        push    esi
        mov     esi, [edi+OhciRhDescriptorAReg]
        shr     esi, 24
        add     esi, esi
        jz      @f
        call    delay_ms
@@:
        pop     esi
; 11c. Ports are powered up; now it is ok to process connect/disconnect events.
        mov     [esi+ohci_controller.PoweredUp-sizeof.ohci_controller], 1
                ; IRQ handler doesn't accept connect/disconnect events before this point
; 11d. We could miss some events while waiting for powering up;
; scan all ports manually and check for connected devices.
        xor     ecx, ecx
.port_loop:
        test    dword [edi+OhciRhPortStatusReg+ecx*4], 1
        jz      .next_port
; There is a connected device; mark the port as 'connected'
; and save the connected time.
; Note that ConnectedTime must be set before 'connected' mark,
; otherwise the code in ohci_process_deferred could use incorrect time.
        mov     eax, [timer_ticks]
        mov     [esi+usb_controller.ConnectedTime+ecx*4], eax
        lock bts [esi+usb_controller.NewConnected], ecx
.next_port:
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .port_loop
; 12. Return pointer to usb_controller.
        xchg    eax, esi
        ret
.fail_unmap:
; On error after step 4, release the virtual memory area.
        stdcall free_kernel_space, edi
.fail:
; On error, free the ohci_controller structure and return zero.
; Note that the pointer was placed in the stack at step 1.
; Note also that there can be no errors after step 8,
; where that pointer is popped from the stack.
        pop     ecx
.nothing:
        xor     eax, eax
        ret
endp

; Helper procedure for step 3 of ohci_init.
; Initializes the static head of one list.
; eax = physical address of the "next" list, esi = pointer to the "next" list,
; edi = pointer to head to initialize.
; Advances edi to the next head, keeps eax/esi.
proc ohci_init_static_endpoint
        mov     byte [edi+ohci_static_ep.Flags+1], 1 shl (14 - 8)       ; sKip this endpoint
        mov     [edi+ohci_static_ep.NextED], eax
        mov     [edi+ohci_static_ep.NextList], esi
        add     edi, ohci_static_ep.SoftwarePart
        call    usb_init_static_endpoint
        add     edi, sizeof.ohci_static_ep - ohci_static_ep.SoftwarePart
        ret
endp

; Helper procedure for step 3 of ohci_init.
; Initializes one half of group of static heads.
; edx = size of the next group = half of size of the group,
; edi = pointer to the group, eax = physical address of the next group,
; esi = pointer to the next group.
; Advances eax, esi, edi to next group, keeps edx.
proc ohci_init_static_ep_group
        push    edx
@@:
        call    ohci_init_static_endpoint
        add     eax, sizeof.ohci_static_ep
        add     esi, sizeof.ohci_static_ep
        dec     edx
        jnz     @b
        pop     edx
        ret
endp

; Controller-specific pre-initialization function: take ownership from BIOS.
; Some BIOSes, although not all of them, provide legacy emulation
; for USB keyboard and/or mice as PS/2-devices. In this case,
; we must notify the BIOS that we don't need that emulation and know how to
; deal with USB devices.
proc ohci_kickoff_bios
; 1. Get the physical address of MMIO registers.
        mov     ah, [esi+PCIDEV.bus]
        mov     bh, [esi+PCIDEV.devfn]
        mov     al, 2
        mov     bl, 10h
        call    pci_read_reg
        and     al, not 0Fh
; 2. Create mapping for physical memory. 256 bytes are sufficient.
        stdcall map_io_mem, eax, 100h, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .nothing
; 3. Some BIOSes enable controller interrupts as a result of giving
; controller away. At this point the system knows nothing about how to serve
; OHCI interrupts, so such an interrupt will send the system into an infinite
; loop handling the same IRQ again and again. Thus, we need to block OHCI
; interrupts. We can't do this at the controller level until step 5,
; because the controller is currently owned by BIOS, so we block all hardware
; interrupts on this processor until step 5.
        pushf
        cli
; 4. Take the ownership over the controller.
; 4a. Check whether BIOS handles this controller at all.
        mov     edx, 100h
        test    dword [eax+OhciControlReg], edx
        jz      .has_ownership
; 4b. Send "take ownership" command to the BIOS.
; (This should generate SMI, BIOS should release its ownership in SMI handler.)
        mov     dword [eax+OhciCommandStatusReg], 8
; 4c. Wait for result no more than 50 ms, checking for status every 1 ms.
        movi    ecx, 50
@@:
        test    dword [eax+OhciControlReg], edx
        jz      .has_ownership
        push    esi
        movi    esi, 1
        call    delay_ms
        pop     esi
        loop    @b
        dbgstr 'warning: taking OHCI ownership from BIOS timeout'
.has_ownership:
; 5. Disable all controller interrupts until the system will be ready to
; process them.
        mov     dword [eax+OhciInterruptDisableReg], 0C000007Fh
; 6. Now we can unblock interrupts in the processor.
        popf
; 7. Release memory mapping created in step 2 and return.
        stdcall free_kernel_space, eax
.nothing:
        ret
endp

; IRQ handler for OHCI controllers.
ohci_irq.noint:
; Not our interrupt: restore registers and return zero.
        xor     eax, eax
        pop     edi esi ebx
        ret

proc ohci_irq
        push    ebx esi edi     ; save used registers to be cdecl
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
        mov     edi, [esi+ohci_controller.MMIOBase-sizeof.ohci_controller]
        mov     eax, [edi+OhciInterruptStatusReg]
; 3. Check whether that interrupt has been generated by our controller.
; (One IRQ can be shared by several devices.)
        and     eax, [edi+OhciInterruptEnableReg]
        jz      .noint
; 4. Get the physical pointer to the last processed descriptor.
; All processed descriptors form single-linked list from last to first
; with the help of NextTD field. The list is restarted every time when
; the controller writes to DoneHead, so grab the pointer now (before the next
; step) or it could be lost (the controller could write new value to DoneHead
; any time after WorkDone bit is cleared in OhciInterruptStatusReg).
        mov     ecx, [esi+ohci_controller.DoneHead-sizeof.ohci_controller]
        and     ecx, not 1
; 5. Clear the events we know of.
; Note that this should be done before processing of events:
; new events could arise while we are processing those, this way we won't lose
; them (the controller would generate another interrupt
; after completion of this one).
        mov     [edi+OhciInterruptStatusReg], eax
; 6. Save the mask of events for further reference.
        push    eax
; 7. Handle 'transfer is done' events.
; 7a. Test whether there are such events.
        test    al, 2
        jz      .skip_donehead
; There are some 'transfer is done' events, processed descriptors are linked
; through physical addresses in the reverse order.
; We can't do much in an interrupt handler, since callbacks could require
; waiting for locks and that can't be done in an interrupt handler.
; However, we can't also just defer all work to the USB thread, since
; it is possible that previous lists are not yet processed and it is hard
; to store unlimited number of list heads. Thus, we reverse the current list,
; append it to end of the previous list (if there is one) and defer other
; processing to the USB thread; this way there always is no more than one list
; (possibly joined from several controller-reported lists).
; The list traversal requires converting physical addresses to virtual pointers,
; so we may as well store pointers instead of physical addresses.
; 7b. Prepare for the reversing loop.
        push    ebx
        xor     ebx, ebx
        test    ecx, ecx
        jz      .tddone
        call    usb_td_to_virt
        test    eax, eax
        jz      .tddone
        lea     edx, [eax+ohci_gtd.NextTD]
; 7c. Reverse the list, converting physical to virtual. On every iteration:
; ecx = physical address of the current item
; eax = virtual pointer to the current item
; edx = virtual pointer to the last item.NextTD (first in the reverse list)
; ebx = virtual pointer to the next item (previous in the reverse list)
.tdloop:
        mov     ecx, [eax+ohci_gtd.NextTD]
        mov     [eax+ohci_gtd.NextTD], ebx
        lea     ebx, [eax+sizeof.ohci_gtd]
        test    ecx, ecx
        jz      .tddone
        call    usb_td_to_virt
        test    eax, eax
        jnz     .tdloop
.tddone:
        mov     ecx, ebx
        pop     ebx
; 7d. The list is reversed,
; ecx = pointer to the first item, edx = pointer to the last item.NextTD.
; If the list is empty (unusual case), step 7 is done.
        test    ecx, ecx
        jz      .skip_donehead
; 7e. Otherwise, append this list to the end of previous one.
; Note that in theory the interrupt handler and the USB thread
; could execute in parallel.
.append_restart:
; Atomically get DoneListEndPtr in eax and set it to edx.
        mov     eax, [esi+ohci_controller.DoneListEndPtr-sizeof.ohci_controller]
        lock cmpxchg [esi+ohci_controller.DoneListEndPtr-sizeof.ohci_controller], edx
        jnz     .append_restart
; Store pointer to the new list.
; Note: we cannot perform any operations with [DoneListEndPtr]
; until we switch DoneListEndPtr to a new descriptor:
; it is possible that after first line of .append_restart loop
; ohci_process_deferred obtains the control, finishes processing
; of the old list, sets DoneListEndPtr to address of DoneList,
; frees all old descriptors, so eax would point to invalid location.
; This way, .append_restart loop would detect that DoneListEndPtr
; has changed, so eax needs to be re-read.
        mov     [eax], ecx
; 7f. Notify the USB thread that there is new work.
        inc     ebx
.skip_donehead:
; 8. Handle start-of-frame events.
; 8a. Test whether there are such events.
        test    byte [esp], 4
        jz      .skip_sof
; We enable SOF interrupt only when some pipes are waiting after changes.
        spin_lock_irqsave [esi+usb_controller.WaitSpinlock]
; 8b. Make sure that there was at least one frame update
; since the request. If not, wait for the next SOF.
        movzx   eax, [esi+ohci_controller.FrameNumber-sizeof.ohci_controller]
        cmp     eax, [esi+usb_controller.StartWaitFrame]
        jz      .sof_unlock
; 8c. Copy WaitPipeRequest* to ReadyPipeHead*.
        mov     eax, [esi+usb_controller.WaitPipeRequestAsync]
        mov     [esi+usb_controller.ReadyPipeHeadAsync], eax
        mov     eax, [esi+usb_controller.WaitPipeRequestPeriodic]
        mov     [esi+usb_controller.ReadyPipeHeadPeriodic], eax
; 8d. It is possible that pipe change is due to removal and
; Control/BulkCurrentED registers still point to one of pipes to be removed.
; The code responsible for disconnect events has temporarily stopped
; Control/Bulk processing, so it is safe to clear Control/BulkCurrentED.
; After that, restart processing.
        xor     edx, edx
        mov     [edi+OhciControlCurrentEDReg], edx
        mov     [edi+OhciBulkCurrentEDReg], edx
        mov     dword [edi+OhciCommandStatusReg], 6
        or      dword [edi+OhciControlReg], 30h
; 8e. Disable further interrupts on SOF.
; Note: OhciInterruptEnableReg/OhciInterruptDisableReg have unusual semantics.
        mov     dword [edi+OhciInterruptDisableReg], 4
; Notify the USB thread that there is new work (with pipes from ReadyPipeHead*).
        inc     ebx
.sof_unlock:
        spin_unlock_irqrestore [esi+usb_controller.RemoveSpinlock]
.skip_sof:
; Handle roothub events.
; 9. Test whether there are such events.
        test    byte [esp], 40h
        jz      .skip_roothub
; 10. Check the status of the roothub itself.
; 10a. Global overcurrent?
        test    dword [edi+OhciRhStatusReg], 2
        jz      @f
; Note: this needs work.
        dbgstr 'global overcurrent'
@@:
; 10b. Clear roothub events.
        mov     dword [edi+OhciRhStatusReg], 80020000h
; 11. Check the status of individual ports.
; Look for connect/disconnect and reset events.
; 11a. Prepare for the loop: start from port 0.
        xor     ecx, ecx
.portloop:
; 11b. Get the port status and changes of it.
; Accumulate change information.
; Look to "11.12.3 Port Change Information Processing" of the USB2 spec.
        xor     eax, eax
.accloop:
        mov     edx, [edi+OhciRhPortStatusReg+ecx*4]
        xor     ax, ax
        or      eax, edx
        test    edx, 1F0000h
        jz      .accdone
        mov     dword [edi+OhciRhPortStatusReg+ecx*4], 1F0000h
        jmp     .accloop
.accdone:
; debugging output, not needed for work
;       test    eax, 1F0000h
;       jz      @f
;       DEBUGF 1,'K : ohci irq [%d] status of port %d is %x\n',[timer_ticks],ecx,eax
;@@:
; 11c. Ignore any events until all ports are powered up.
; They will be processed by ohci_init.
        cmp     [esi+ohci_controller.PoweredUp-sizeof.ohci_controller], 0
        jz      .nextport
; Handle changing of connection status.
        test    eax, 10000h
        jz      .nocsc
; There was a connect or disconnect event at this port.
; 11d. Disconnect the old device on this port, if any.
; if the port was resetting, indicate fail and signal
        cmp     cl, [esi+usb_controller.ResettingPort]
        jnz     @f
        mov     [esi+usb_controller.ResettingStatus], -1
        inc     ebx
@@:
        lock bts [esi+usb_controller.NewDisconnected], ecx
; notify the USB thread that new work is waiting
        inc     ebx
; 11e. Change connected status. For the connection event, also
; store the connection time; any further processing is permitted only
; after USB_CONNECT_DELAY ticks.
        test    al, 1
        jz      .disconnect
; Note: ConnectedTime must be stored before setting the 'connected' bit,
; otherwise ohci_process_deferred could use an old time.
        mov     eax, [timer_ticks]
        mov     [esi+usb_controller.ConnectedTime+ecx*4], eax
        lock bts [esi+usb_controller.NewConnected], ecx
        jmp     .nextport
.disconnect:
        lock btr [esi+usb_controller.NewConnected], ecx
        jmp     .nextport
.nocsc:
; 11f. Process 'reset done' events.
        test    eax, 100000h
        jz      .nextport
        test    al, 10h
        jnz     .nextport
        mov     edx, [timer_ticks]
        mov     [esi+usb_controller.ResetTime], edx
        mov     [esi+usb_controller.ResettingStatus], 2
        inc     ebx
.nextport:
; 11g. Continue the loop for the next port.
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .portloop
.skip_roothub:
; 12. Restore the stack after step 6.
        pop     eax
; 13. Notify the USB thread if some deferred processing is required.
        call    usb_wakeup_if_needed
; 14. Interrupt processed; return something non-zero.
        mov     al, 1
        pop     edi esi ebx     ; restore used registers to be stdcall
        ret
endp

; This procedure is called from usb_set_address_callback
; and stores USB device address in the ohci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe, cl = address
proc ohci_set_device_address
        mov     byte [ebx+ohci_pipe.Flags-sizeof.ohci_pipe], cl
; Wait until the hardware will forget the old value.
        call    usb_subscribe_control
        ret
endp

; This procedure returns USB device address from the usb_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe
; out: eax = endpoint address
proc ohci_get_device_address
        mov     eax, [ebx+ohci_pipe.Flags-sizeof.ohci_pipe]
        and     eax, 7Fh
        ret
endp

; This procedure is called from usb_set_address_callback
; if the device does not accept SET_ADDRESS command and needs
; to be disabled at the port level.
; in: esi -> usb_controller, ecx = port
proc ohci_port_disable
        mov     edx, [esi+ohci_controller.MMIOBase-sizeof.ohci_controller]
        mov     dword [edx+OhciRhPortStatusReg+ecx*4], 1
        ret
endp

; This procedure is called from usb_get_descr8_callback when
; the packet size for zero endpoint becomes known and
; stores the packet size in ohci_pipe structure.
; in: esi -> usb_controller, ebx -> usb_pipe, ecx = packet size
proc ohci_set_endpoint_packet_size
        mov     byte [ebx+ohci_pipe.Flags+2-sizeof.ohci_pipe], cl
; Wait until the hardware will forget the old value.
        call    usb_subscribe_control
        ret
endp

; This procedure is called from API usb_open_pipe and processes
; the controller-specific part of this API. See docs.
; in: edi -> usb_pipe for target, ecx -> usb_pipe for config pipe,
; esi -> usb_controller, eax -> usb_gtd for the first TD,
; [ebp+12] = endpoint, [ebp+16] = maxpacket, [ebp+20] = type
proc ohci_init_pipe
virtual at ebp+8
.config_pipe    dd      ?
.endpoint       dd      ?
.maxpacket      dd      ?
.type           dd      ?
.interval       dd      ?
end virtual
; 1. Initialize the queue of transfer descriptors: empty.
        sub     eax, sizeof.ohci_gtd
        call    get_phys_addr
        mov     [edi+ohci_pipe.TailP-sizeof.ohci_pipe], eax
        mov     [edi+ohci_pipe.HeadP-sizeof.ohci_pipe], eax
; 2. Generate ohci_pipe.Flags, see the description in ohci_pipe.
        mov     eax, [ecx+ohci_pipe.Flags-sizeof.ohci_pipe]
        and     eax, 0x207F     ; keep Speed bit and FunctionAddress
        mov     edx, [.endpoint]
        and     edx, 15
        shl     edx, 7
        or      eax, edx
        mov     [edi+ohci_pipe.Flags-sizeof.ohci_pipe], eax
        mov     eax, [.maxpacket]
        mov     word [edi+ohci_pipe.Flags+2-sizeof.ohci_pipe], ax
        cmp     [.type], CONTROL_PIPE
        jz      @f
        test    byte [.endpoint], 80h
        setnz   al
        inc     eax
        shl     al, 3
        or      byte [edi+ohci_pipe.Flags+1-sizeof.ohci_pipe], al
@@:
; 3. Insert the new pipe to the corresponding list of endpoints.
; 3a. Use Control list for control pipes, Bulk list for bulk pipes.
        lea     edx, [esi+ohci_controller.ControlED.SoftwarePart-sizeof.ohci_controller]
        cmp     [.type], BULK_PIPE
        jb      .insert ; control pipe
        lea     edx, [esi+ohci_controller.BulkED.SoftwarePart-sizeof.ohci_controller]
        jz      .insert ; bulk pipe
.interrupt_pipe:
; 3b. For interrupt pipes, let the scheduler select the appropriate list
; based on the current bandwidth distribution and the requested bandwidth.
; This could fail if the requested bandwidth is not available;
; if so, return an error.
        lea     edx, [esi + ohci_controller.IntEDs - sizeof.ohci_controller]
        lea     eax, [esi + ohci_controller.IntEDs + 32*sizeof.ohci_static_ep - sizeof.ohci_controller]
        movi    ecx, 64
        call    usb1_select_interrupt_list
        test    edx, edx
        jz      .return0
; 3c. Insert endpoint at edi to the head of list in edx.
; Inserting to tail would work as well,
; but let's be consistent with other controllers.
.insert:
        mov     ecx, [edx+usb_pipe.NextVirt]
        mov     [edi+usb_pipe.NextVirt], ecx
        mov     [edi+usb_pipe.PrevVirt], edx
        mov     [ecx+usb_pipe.PrevVirt], edi
        mov     [edx+usb_pipe.NextVirt], edi
        mov     ecx, [edx+ohci_pipe.NextED-sizeof.ohci_pipe]
        mov     [edi+ohci_pipe.NextED-sizeof.ohci_pipe], ecx
        lea     eax, [edi-sizeof.ohci_pipe]
        call    get_phys_addr
        mov     [edx+ohci_pipe.NextED-sizeof.ohci_pipe], eax
; 4. Return something non-zero.
        ret
.return0:
        xor     eax, eax
        ret
endp

; This function is called from ohci_process_deferred when
; a new device was connected at least USB_CONNECT_DELAY ticks
; and therefore is ready to be configured.
; ecx = port, esi -> usb_controller
proc ohci_new_port
; test whether we are configuring another port
; if so, postpone configuring and return
        bts     [esi+usb_controller.PendingPorts], ecx
        cmp     [esi+usb_controller.ResettingPort], -1
        jnz     .nothing
        btr     [esi+usb_controller.PendingPorts], ecx
; fall through to ohci_new_port.reset

; This function is called from usb_test_pending_port.
; It starts reset signalling for the port. Note that in USB first stages
; of configuration can not be done for several ports in parallel.
.reset:
; reset port
        and     [esi+usb_controller.ResettingHub], 0
        mov     [esi+usb_controller.ResettingPort], cl
; Note: setting status must be the last action:
; it is possible that the device has been disconnected
; after timeout of USB_CONNECT_DELAY but before call to ohci_new_port.
; In this case, ohci_irq would not set reset status to 'failed',
; because ohci_irq would not know that this port is to be reset.
; However, the hardware would generate another interrupt
; in a response to reset a disconnected port, and this time
; ohci_irq knows that it needs to generate 'reset failed' event
; (because ResettingPort is now filled).
        push    edi
        mov     edi, [esi+ohci_controller.MMIOBase-sizeof.ohci_controller]
        mov     dword [edi+OhciRhPortStatusReg+ecx*4], 10h
        pop     edi
.nothing:
        ret
endp

; This procedure is called from the several places in main USB code
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
proc ohci_alloc_transfer stdcall uses edi, \
        buffer:dword, size:dword, flags:dword, td:dword, direction:dword
locals
origTD          dd      ?
packetSize      dd      ?       ; must be the last variable, see usb_init_transfer
endl
; 1. Save original value of td:
; it will be useful for rollback if something would fail.
        mov     eax, [td]
        mov     [origTD], eax
; One transfer descriptor can describe up to two pages.
; In the worst case (when the buffer is something*1000h+0FFFh)
; this corresponds to 1001h bytes. If the requested size is
; greater, we should split the transfer into several descriptors.
; Boundaries to split must be multiples of endpoint transfer size
; to avoid short packets except in the end of the transfer.
        cmp     [size], 1001h
        jbe     .lastpacket
; 2. While the remaining data cannot fit in one packet,
; allocate full-sized descriptors.
; 2a. Calculate size of one descriptor: must be a multiple of transfer size
; and must be not greater than 1001h.
        movzx   ecx, word [ebx+ohci_pipe.Flags+2-sizeof.ohci_pipe]
        mov     eax, 1001h
        xor     edx, edx
        mov     edi, eax
        div     ecx
        sub     edi, edx
; 2b. Allocate in loop.
        mov     [packetSize], edi
.fullpackets:
        call    ohci_alloc_packet
        test    eax, eax
        jz      .fail
        mov     [td], eax
        add     [buffer], edi
        sub     [size], edi
        cmp     [size], 1001h
        ja      .fullpackets
; 3. The remaining data can fit in one descriptor;
; allocate the last descriptor with size = size of remaining data.
.lastpacket:
        mov     eax, [size]
        mov     [packetSize], eax
        call    ohci_alloc_packet
        test    eax, eax
        jz      .fail
; 4. Enable an immediate interrupt on completion of the last packet.
        and     byte [ecx+ohci_gtd.Flags+2-sizeof.ohci_gtd], not (7 shl (21-16))
; 5. If a short transfer is ok for a caller, set the corresponding bit in
; the last descriptor, but not in others.
; Note: even if the caller says that short transfers are ok,
; all packets except the last one are marked as 'must be complete':
; if one of them will be short, the software intervention is needed
; to skip remaining packets; ohci_process_finalized_td will handle this
; transparently to the caller.
        test    [flags], 1
        jz      @f
        or      byte [ecx+ohci_gtd.Flags+2-sizeof.ohci_gtd], 1 shl (18-16)
@@:
        ret
.fail:
        mov     edi, ohci_hardware_func
        mov     eax, [td]
        stdcall usb_undo_tds, [origTD]
        xor     eax, eax
        ret
endp

; Helper procedure for ohci_alloc_transfer.
; Allocates and initializes one transfer descriptor.
; ebx = pipe, other parameters are passed through the stack;
; fills the current last descriptor and
; returns eax = next descriptor (not filled).
proc ohci_alloc_packet
; inherit some variables from the parent ohci_alloc_transfer
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
        call    usb1_allocate_general_td
        test    eax, eax
        jz      .nothing
; 2. Initialize controller-independent parts of both TDs.
        push    eax
        call    usb_init_transfer
        pop     eax
; 3. Save the returned value (next descriptor).
        push    eax
; 4. Store the physical address of the next descriptor.
        sub     eax, sizeof.ohci_gtd
        call    get_phys_addr
        mov     [ecx+ohci_gtd.NextTD-sizeof.ohci_gtd], eax
; 5. For zero-length transfers, store zero in both fields for buffer addresses.
; Otherwise, fill them with real values.
        xor     eax, eax
        mov     [ecx+ohci_gtd.CurBufPtr-sizeof.ohci_gtd], eax
        mov     [ecx+ohci_gtd.BufEnd-sizeof.ohci_gtd], eax
        cmp     [.packetSize], eax
        jz      @f
        mov     eax, [.buffer]
        call    get_phys_addr
        mov     [ecx+ohci_gtd.CurBufPtr-sizeof.ohci_gtd], eax
        mov     eax, [.buffer]
        add     eax, [.packetSize]
        dec     eax
        call    get_phys_addr
        mov     [ecx+ohci_gtd.BufEnd-sizeof.ohci_gtd], eax
@@:
; 6. Generate Flags field:
; - set bufferRounding (bit 18) to zero = disallow short transfers;
;   for the last transfer in a row, ohci_alloc_transfer would set the real value;
; - set Direction (bits 19-20) to lower 2 bits of [.direction];
; - set DelayInterrupt (bits 21-23) to 7 = do not generate interrupt;
;   for the last transfer in a row, ohci_alloc_transfer would set the real value;
; - set DataToggle (bits 24-25) to next 2 bits of [.direction];
; - set ConditionCode (bits 28-31) to 1111b as a indicator that there was no
;   attempts to perform this transfer yet;
; - zero all other bits.
        mov     eax, [.direction]
        mov     edx, eax
        and     eax, 3
        shl     eax, 19
        and     edx, (3 shl 2)
        shl     edx, 24 - 2
        lea     eax, [eax + edx + (7 shl 21) + (15 shl 28)]
        mov     [ecx+ohci_gtd.Flags-sizeof.ohci_gtd], eax
; 7. Restore the returned value saved in step 3.
        pop     eax
.nothing:
        ret
endp

; This procedure is called from the several places in main USB code
; and activates the transfer which was previously allocated by
; ohci_alloc_transfer.
; ecx -> last descriptor for the transfer, ebx -> usb_pipe
proc ohci_insert_transfer
; 1. Advance the queue of transfer descriptors.
        mov     eax, [ecx+ohci_gtd.NextTD-sizeof.ohci_gtd]
        mov     [ebx+ohci_pipe.TailP-sizeof.ohci_pipe], eax
; 2. For control and bulk pipes, notify the controller that
; there is new work in control/bulk queue respectively.
ohci_notify_new_work:
        mov     edx, [ebx+usb_pipe.Controller]
        mov     edx, [edx+ohci_controller.MMIOBase-sizeof.ohci_controller]
        cmp     [ebx+usb_pipe.Type], CONTROL_PIPE
        jz      .control
        cmp     [ebx+usb_pipe.Type], BULK_PIPE
        jnz     .nothing
.bulk:
        mov     dword [edx+OhciCommandStatusReg], 4
        jmp     .nothing
.control:
        mov     dword [edx+OhciCommandStatusReg], 2
.nothing:
        ret
endp

; This function is called from ohci_process_deferred when
; a new device has been reset and needs to be configured.
proc ohci_port_after_reset
; 1. Get the status.
; If reset has been failed (device disconnected during reset),
; continue to next device (if there is one).
        xor     eax, eax
        xchg    al, [esi+usb_controller.ResettingStatus]
        test    al, al
        js      usb_test_pending_port
; If the controller has disabled the port (e.g. overcurrent),
; continue to next device (if there is one).
        movzx   ecx, [esi+usb_controller.ResettingPort]
        mov     eax, [edi+OhciRhPortStatusReg+ecx*4]
        test    al, 2
        jnz     @f
        DEBUGF 1,'K : USB port disabled after reset, status=%x\n',eax
        jmp     usb_test_pending_port
@@:
        push    ecx
; 2. Get LowSpeed bit to bit 0 of eax and call the worker procedure
; to notify the protocol layer about new OHCI device.
        mov     eax, [edi+OhciRhPortStatusReg+ecx*4]
        DEBUGF 1,'K : port_after_reset [%d] status of port %d is %x\n',[timer_ticks],ecx,eax
        shr     eax, 9
        call    ohci_new_device
        pop     ecx
; 3. If something at the protocol layer has failed
; (no memory, no bus address), disable the port and stop the initialization.
        test    eax, eax
        jnz     .nothing
.disable_exit:
        mov     dword [edi+OhciRhPortStatusReg+ecx*4], 1
        jmp     usb_test_pending_port
.nothing:
        ret
endp

; This procedure is called from uhci_port_init and from hub support code
; when a new device is connected and has been reset.
; It calls usb_new_device at the protocol layer with correct parameters.
; in: esi -> usb_controller, eax = speed;
; OHCI is USB1 device, so only low bit of eax (LowSpeed) is used.
proc ohci_new_device
; 1. Clear all bits of speed except bit 0.
        and     eax, 1
; 2. Store the speed for the protocol layer.
        mov     [esi+usb_controller.ResettingSpeed], al
; 3. Create pseudo-pipe in the stack.
; See ohci_init_pipe: only .Controller and .Flags fields are used.
        shl     eax, 13
        push    esi     ; .Controller
        mov     ecx, esp
        sub     esp, 12 ; ignored fields
        push    eax     ; .Flags
; 4. Notify the protocol layer.
        call    usb_new_device
; 5. Cleanup the stack after step 3 and return.
        add     esp, 20
        ret
endp

; This procedure is called in the USB thread from usb_thread_proc,
; processes regular actions and those actions which can't be safely done
; from interrupt handler.
; Returns maximal time delta before the next call.
proc ohci_process_deferred
        push    ebx edi         ; save used registers to be stdcall
; 1. Initialize the return value.
        push    -1
; 2. Process disconnect events.
        call    usb_disconnect_stage2
; 3. Check for connected devices.
; If there is a connected device which was connected less than
; USB_CONNECT_DELAY ticks ago, plan to wake up when the delay will be over.
; Otherwise, call ohci_new_port.
        mov     edi, [esi+ohci_controller.MMIOBase-sizeof.ohci_controller]
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
        lock btr [esi+usb_controller.NewConnected], ecx
        jnc     .nextport
        call    ohci_new_port
.noconnect:
.nextport:
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .portloop
.skip_newconnected:
; 4. Check for end of reset signalling. If so, call ohci_port_after_reset.
        cmp     [esi+usb_controller.ResettingStatus], 2
        jnz     .no_reset_recovery
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_controller.ResetTime]
        sub     eax, USB_RESET_RECOVERY_TIME
        jge     .reset_done
        neg     eax
        cmp     [esp], eax
        jb      .skip_roothub
        mov     [esp], eax
        jmp     .skip_roothub
.no_reset_recovery:
        cmp     [esi+usb_controller.ResettingStatus], 0
        jz      .skip_roothub
.reset_done:
        call    ohci_port_after_reset
.skip_roothub:
; 5. Finalize transfers processed by hardware.
; It is better to perform this step after processing disconnect events,
; although not strictly obligatory. This way, an active transfer aborted
; due to disconnect would be handled with more specific USB_STATUS_CLOSED,
; not USB_STATUS_NORESPONSE.
; Loop over all items in DoneList, call ohci_process_finalized_td for each.
        xor     ebx, ebx
        xchg    ebx, [esi+ohci_controller.DoneList-sizeof.ohci_controller]
.tdloop:
        test    ebx, ebx
        jz      .tddone
        call    ohci_process_finalized_td
        jmp     .tdloop
.tddone:
; 6. Process wait-done notifications, test for new wait requests.
; Note: that must be done after steps 2 and 5 which could create new requests.
; 6a. Call the worker function from main USB code.
        call    usb_process_wait_lists
; 6b. If no new requests, skip the rest of this step.
        test    eax, eax
        jz      @f
; 6c. OHCI is not allowed to cache anything; we don't know what is
; processed right now, but we can be sure that the controller will not
; use any removed structure starting from the next frame.
; Schedule SOF event.
        spin_lock_irq [esi+usb_controller.RemoveSpinlock]
        mov     eax, [esi+usb_controller.WaitPipeListAsync]
        mov     [esi+usb_controller.WaitPipeRequestAsync], eax
        mov     eax, [esi+usb_controller.WaitPipeListPeriodic]
        mov     [esi+usb_controller.WaitPipeRequestPeriodic], eax
; temporarily stop bulk and interrupt processing;
; this is required for handler of SOF event
        and     dword [edi+OhciControlReg], not 30h
; remember the frame number when processing has been stopped
; (needs to be done after stopping)
        movzx   eax, [esi+ohci_controller.FrameNumber-sizeof.ohci_controller]
        mov     [esi+usb_controller.StartWaitFrame], eax
; make sure that the next SOF will happen after the request
        mov     dword [edi+OhciInterruptStatusReg], 4
; enable interrupt on SOF
; Note: OhciInterruptEnableReg/OhciInterruptDisableReg have unusual semantics,
; so there should be 'mov' here, not 'or'
        mov     dword [edi+OhciInterruptEnableReg], 4
        spin_unlock_irq [esi+usb_controller.RemoveSpinlock]
@@:
; 7. Restore the return value and return.
        pop     eax
        pop     edi ebx         ; restore used registers to be stdcall
        ret
endp

; Helper procedure for ohci_process_deferred. Processes one completed TD.
; in: esi -> usb_controller, ebx -> usb_gtd, out: ebx -> next usb_gtd.
proc ohci_process_finalized_td
;       DEBUGF 1,'K : processing %x\n',ebx
; 1. Check whether the pipe has been closed, either due to API call or due to
; disconnect; if so, the callback will be called by usb_pipe_closed with
; correct status, so go to step 6 with ebx = 0 (do not free the TD).
        mov     edx, [ebx+usb_gtd.Pipe]
        test    [edx+usb_pipe.Flags], USB_FLAG_CLOSED
        jz      @f
        lea     eax, [ebx+ohci_gtd.NextTD-sizeof.ohci_gtd]
        xor     ebx, ebx
        jmp     .next_td2
@@:
; 2. Remove the descriptor from the descriptors queue.
        call    usb_unlink_td
; 3. Get number of bytes that remain to be transferred.
; If CurBufPtr is zero, everything was transferred.
        xor     edx, edx
        cmp     [ebx+ohci_gtd.CurBufPtr-sizeof.ohci_gtd], edx
        jz      .gotlen
; Otherwise, the remaining length is
; (BufEnd and 0xFFF) - (CurBufPtr and 0xFFF) + 1,
; plus 0x1000 if BufEnd and CurBufPtr are in different pages.
        mov     edx, [ebx+ohci_gtd.BufEnd-sizeof.ohci_gtd]
        mov     eax, [ebx+ohci_gtd.CurBufPtr-sizeof.ohci_gtd]
        mov     ecx, edx
        and     edx, 0xFFF
        inc     edx
        xor     ecx, eax
        and     ecx, -0x1000
        jz      @f
        add     edx, 0x1000
@@:
        and     eax, 0xFFF
        sub     edx, eax
.gotlen:
; The actual length is Length - (remaining length).
        sub     edx, [ebx+usb_gtd.Length]
        neg     edx
; 4. Check for error. If so, go to 7.
        push    ebx
        mov     eax, [ebx+ohci_gtd.Flags-sizeof.ohci_gtd]
        shr     eax, 28
        jnz     .error
.notify:
; 5. Successful completion.
; 5a. Check whether this descriptor has an associated callback.
        mov     ecx, [ebx+usb_gtd.Callback]
        test    ecx, ecx
        jz      .ok_nocallback
; 5b. If so, call the callback.
        stdcall_verify ecx, [ebx+usb_gtd.Pipe], eax, \
                [ebx+usb_gtd.Buffer], edx, [ebx+usb_gtd.UserData]
        jmp     .next_td
.ok_nocallback:
; 5c. Otherwise, add length of the current descriptor to the next descriptor.
        mov     eax, [ebx+usb_gtd.NextVirt]
        add     [eax+usb_gtd.Length], edx
.next_td:
; 6. Free the current descriptor and advance to the next item.
; If the current item is the last in the list,
; set DoneListEndPtr to pointer to DoneList.
        cmp     ebx, [esp]
        jz      @f
        stdcall usb1_free_general_td, ebx
@@:
        pop     ebx
        lea     eax, [ebx+ohci_gtd.NextTD-sizeof.ohci_gtd]
.next_td2:
        push    ebx
        mov     ebx, eax
        lea     edx, [esi+ohci_controller.DoneList-sizeof.ohci_controller]
        xor     ecx, ecx        ; no next item
        lock cmpxchg [esi+ohci_controller.DoneListEndPtr-sizeof.ohci_controller], edx
        jz      .last
; The current item is not the last.
; It is possible, although very rare, that ohci_irq has already advanced
; DoneListEndPtr, but not yet written NextTD. Wait until NextTD is nonzero.
@@:
        mov     ecx, [ebx]
        test    ecx, ecx
        jz      @b
.last:
        pop     ebx
; ecx = the next item
        push    ecx
; Free the current item, set ebx to the next item, continue to 5a.
        test    ebx, ebx
        jz      @f
        stdcall usb1_free_general_td, ebx
@@:
        pop     ebx
        ret
.error:
; 7. There was an error while processing this descriptor.
; The hardware has stopped processing the queue.
; 7a. Save status and length.
        push    eax
        push    edx
;       DEBUGF 1,'K : TD failed:\n'
;       DEBUGF 1,'K : %x %x %x %x\n',[ebx-sizeof.ohci_gtd],[ebx-sizeof.ohci_gtd+4],[ebx-sizeof.ohci_gtd+8],[ebx-sizeof.ohci_gtd+12]
;       DEBUGF 1,'K : %x %x %x %x\n',[ebx-sizeof.ohci_gtd+16],[ebx-sizeof.ohci_gtd+20],[ebx-sizeof.ohci_gtd+24],[ebx-sizeof.ohci_gtd+28]
;       mov     eax, [ebx+usb_gtd.Pipe]
;       DEBUGF 1,'K : pipe: %x %x %x %x\n',[eax-sizeof.ohci_pipe],[eax-sizeof.ohci_pipe+4],[eax-sizeof.ohci_pipe+8],[eax-sizeof.ohci_pipe+12]
; 7b. Traverse the list of descriptors looking for the final packet
; for this transfer.
; Free and unlink non-final descriptors, except the current one.
; Final descriptor will be freed in step 6.
        call    usb_is_final_packet
        jnc     .found_final
        mov     ebx, [ebx+usb_gtd.NextVirt]
virtual at esp
.length         dd      ?
.error_code     dd      ?
.current_item   dd      ?
end virtual
.look_final:
        call    usb_unlink_td
        call    usb_is_final_packet
        jnc     .found_final
        push    [ebx+usb_gtd.NextVirt]
        stdcall usb1_free_general_td, ebx
        pop     ebx
        jmp     .look_final
.found_final:
; 7c. If error code is USB_STATUS_UNDERRUN and the last TD allows short packets,
; it is not an error.
; Note: all TDs except the last one in any transfer stage are marked
; as short-packet-is-error to stop controller from further processing
; of that stage; we need to restart processing from a TD following the last.
; After that, go to step 5 with eax = 0 (no error).
        cmp     dword [.error_code], USB_STATUS_UNDERRUN
        jnz     .no_underrun
        test    byte [ebx+ohci_gtd.Flags+2-sizeof.ohci_gtd], 1 shl (18-16)
        jz      .no_underrun
        and     dword [.error_code], 0
        mov     ecx, [ebx+usb_gtd.Pipe]
        mov     edx, [ecx+ohci_pipe.HeadP-sizeof.ohci_pipe]
        and     edx, 2
.advance_queue:
        mov     eax, [ebx+usb_gtd.NextVirt]
        sub     eax, sizeof.ohci_gtd
        call    get_phys_addr
        or      eax, edx
        mov     [ecx+ohci_pipe.HeadP-sizeof.ohci_pipe], eax
        push    ebx
        mov     ebx, ecx
        call    ohci_notify_new_work
        pop     ebx
        pop     edx eax
        jmp     .notify
; 7d. Abort the entire transfer.
; There are two cases: either there is only one transfer stage
; (everything except control transfers), then ebx points to the last TD and
; all previous TD were unlinked and dismissed (if possible),
; or there are several stages (a control transfer) and ebx points to the last
; TD of Data or Status stage (usb_is_final_packet does not stop in Setup stage,
; because Setup stage can not produce short packets); for Data stage, we need
; to unlink and free (if possible) one more TD and advance ebx to the next one.
.no_underrun:
        cmp     [ebx+usb_gtd.Callback], 0
        jnz     .halted
        cmp     ebx, [.current_item]
        push    [ebx+usb_gtd.NextVirt]
        jz      @f
        stdcall usb1_free_general_td, ebx
@@:
        pop     ebx
        call    usb_unlink_td
.halted:
; 7e. For bulk/interrupt transfers we have no choice but halt the queue,
; the driver should intercede (through some API which is not written yet).
; Control pipes normally recover at the next SETUP transaction (first stage
; of any control transfer), so we hope on the best and just advance the queue
; to the next transfer. (According to the standard, "A control pipe may also
; support functional stall as well, but this is not recommended.").
; Advance the transfer queue to the next descriptor.
        mov     ecx, [ebx+usb_gtd.Pipe]
        mov     edx, [ecx+ohci_pipe.HeadP-sizeof.ohci_pipe]
        and     edx, 2  ; keep toggleCarry bit
        cmp     [ecx+usb_pipe.Type], CONTROL_PIPE
        jnz     @f
        inc     edx     ; set Halted bit
@@:
        jmp     .advance_queue
endp

; This procedure is called when a pipe is closing (either due to API call
; or due to disconnect); it unlinks the pipe from the corresponding list.
; esi -> usb_controller, ebx -> usb_pipe
proc ohci_unlink_pipe
        cmp     [ebx+usb_pipe.Type], INTERRUPT_PIPE
        jnz     @f
        mov     eax, [ebx+ohci_pipe.Flags-sizeof.ohci_pipe]
        bt      eax, 13
        setc    cl
        bt      eax, 11
        setc    ch
        shr     eax, 16
        stdcall usb1_interrupt_list_unlink, eax, ecx
@@:
        mov     edx, [ebx+usb_pipe.NextVirt]
        mov     eax, [ebx+usb_pipe.PrevVirt]
        mov     [edx+usb_pipe.PrevVirt], eax
        mov     [eax+usb_pipe.NextVirt], edx
        mov     edx, [ebx+ohci_pipe.NextED-sizeof.ohci_pipe]
        mov     [eax+ohci_pipe.NextED-sizeof.ohci_pipe], edx
        ret
endp
