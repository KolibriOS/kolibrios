; Functions for USB pipe manipulation: opening/closing, sending data etc.
;
; =============================================================================
; ================================= Constants =================================
; =============================================================================
; USB pipe types
CONTROL_PIPE = 0
ISOCHRONOUS_PIPE = 1
BULK_PIPE = 2
INTERRUPT_PIPE = 3

; Status codes for transfer callbacks.
; Taken from OHCI as most verbose controller in this sense.
USB_STATUS_OK           = 0     ; no error
USB_STATUS_CRC          = 1     ; CRC error
USB_STATUS_BITSTUFF     = 2     ; bit stuffing violation
USB_STATUS_TOGGLE       = 3     ; data toggle mismatch
USB_STATUS_STALL        = 4     ; device returned STALL
USB_STATUS_NORESPONSE   = 5     ; device not responding
USB_STATUS_PIDCHECK     = 6     ; invalid PID check bits
USB_STATUS_WRONGPID     = 7     ; unexpected PID value
USB_STATUS_OVERRUN      = 8     ; too many data from endpoint
USB_STATUS_UNDERRUN     = 9     ; too few data from endpoint
USB_STATUS_BUFOVERRUN   = 12    ; overflow of internal controller buffer
USB_STATUS_BUFUNDERRUN  = 13    ; underflow of internal controller buffer
USB_STATUS_CLOSED       = 16    ; pipe closed
                                ; either explicitly with USBClosePipe
                                ; or implicitly due to device disconnect

; flags for usb_pipe.Flags
USB_FLAG_CLOSED     = 1         ; pipe is closed, no new transfers
; pipe is closed, return error instead of submitting any new transfer
USB_FLAG_CAN_FREE   = 2
; pipe is closed via explicit call to USBClosePipe, so it can be freed without
; any driver notification; if this flag is not set, then the pipe is closed due
; to device disconnect, so it must remain valid until return from disconnect
; callback provided by the driver
USB_FLAG_EXTRA_WAIT = 4
; The pipe was in wait list, while another event occured;
; when the first wait will be done, reinsert the pipe to wait list
USB_FLAG_CLOSED_BIT = 0 ; USB_FLAG_CLOSED = 1 shl USB_FLAG_CLOSED_BIT

; =============================================================================
; ================================ Structures =================================
; =============================================================================

; Pipe descriptor.
; * An USB pipe is described by two structures, for hardware and for software.
; * This is the software part. The hardware part is defined in a driver
;   of the corresponding controller.
; * The hardware part is located immediately before usb_pipe,
;   both are allocated at once by controller-specific code
;   (it knows the total length, which depends on the hardware part).
struct usb_pipe
Controller      dd      ?
; Pointer to usb_controller structure corresponding to this pipe.
; Must be the first dword after hardware part, see *hci_new_device.
;
; Every endpoint is included into one of processing lists:
; * Bulk list contains all Bulk endpoints.
; * Control list contains all Control endpoints.
; * Several Periodic lists serve Interrupt endpoints with different interval.
;   - There are N=2^n "leaf" periodic lists for N ms interval, one is processed
;     in the frames 0,N,2N,..., another is processed in the frames
;     1,1+N,1+2N,... and so on. The hardware starts processing of periodic
;     endpoints in every frame from the list identified by lower n bits of the
;     frame number; the addresses of these N lists are written to the
;     controller data area during the initialization.
;   - We assume that n=5, N=32 to simplify the code and compact the data.
;     OHCI works in this way. UHCI and EHCI actually have n=10, N=1024,
;     but this is an overkill for interrupt endpoints; the large value of N is
;     useful only for isochronous transfers in UHCI and EHCI. UHCI/EHCI code
;     initializes "leaf" lists k,k+32,k+64,...,k+(1024-32) to the same value,
;     giving essentially N=32.
;     This restriction means that the actual maximum interval of polling any
;     interrupt endpoint is 32ms, which seems to be a reasonable value.
;   - Similarly, there are 16 lists for 16-ms interval, 8 lists for 8-ms
;     interval and so on. Finally, there is one list for 1ms interval. Their
;     addresses are not directly known to the controller.
;   - The hardware serves endpoints following a physical link from the hardware
;     part.
;   - The hardware links are organized as follows. If the list item is not the
;     last, it's hardware link points to the next item. The hardware link of
;     the last item points to the first item of the "next" list.
;   - The "next" list for k-th and (k+M)-th periodic lists for interval 2M ms
;     is the k-th periodic list for interval M ms, M >= 1. In this scheme,
;     if two "previous" lists are served in the frames k,k+2M,k+4M,...
;     and k+M,k+3M,k+5M,... correspondingly, the "next" list is served in
;     the frames k,k+M,k+2M,k+3M,k+4M,k+5M,..., which is exactly what we want.
;   - The links between Periodic, Control, Bulk lists and the processing of
;     Isochronous endpoints are controller-specific.
; * The head of every processing list is a static entry which does not
;   correspond to any real pipe. It is described by usb_static_ep
;   structure, not usb_pipe. For OHCI and UHCI, sizeof.usb_static_ep plus
;   sizeof hardware part is 20h, the total number of lists is
;   32+16+8+4+2+1+1+1 = 65, so all these structures fit in one page,
;   leaving space for other data. This is another reason for 32ms limit.
; * Static endpoint descriptors are kept in *hci_controller structure.
; * All items in every processing list, including the static head, are
;   organized in a double-linked list using .NextVirt and .PrevVirt fields.
; * [[item.NextVirt].PrevVirt] = [[item.PrevVirt].NextVirt] for all items.
NextVirt        dd      ?
; Next endpoint in the processing list.
; See also PrevVirt field and the description before NextVirt field.
PrevVirt        dd      ?
; Previous endpoint in the processing list.
; See also NextVirt field and the description before NextVirt field.
;
; Every pipe has the associated transfer queue, that is, the double-linked
; list of Transfer Descriptors aka TD. For Control, Bulk and Interrupt
; endpoints this list consists of usb_gtd structures
; (GTD = General Transfer Descriptors), for Isochronous endpoints
; this list consists of usb_itd structures, which are not developed yet.
; The pipe needs to know only the last TD; the first TD can be
; obtained as [[pipe.LastTD].NextVirt].
LastTD          dd      ?
; Last TD in the transfer queue.
;
; All opened pipes corresponding to the same physical device are organized in
; the double-linked list using .NextSibling and .PrevSibling fields.
; The head of this list is kept in usb_device_data structure (OpenedPipeList).
; This list is used when the device is disconnected and all pipes for the
; device should be closed.
; Also, all pipes closed due to disconnect must remain valid at least until
; driver-provided disconnect function returns; all should-be-freed-but-not-now
; pipes for one device are organized in another double-linked list with
; the head in usb_device_data.ClosedPipeList; this list uses the same link
; fields, one pipe can never be in both lists.
NextSibling     dd      ?
; Next pipe for the physical device.
PrevSibling     dd      ?
; Previous pipe for the physical device.
;
; When hardware part of pipe is changed, some time is needed before further
; actions so that hardware reacts on this change. During that time,
; all changed pipes are organized in single-linked list with the head
; usb_controller.WaitPipeList* and link field NextWait.
; Currently there are two possible reasons to change:
; change of address/packet size in initial configuration,
; close of the pipe. They are distinguished by USB_FLAG_CLOSED.
NextWait        dd      ?
Lock            MUTEX
; Mutex that guards operations with transfer queue for this pipe.
Type            db      ?
; Type of pipe, one of {CONTROL,ISOCHRONOUS,BULK,INTERRUPT}_PIPE.
Flags           db      ?
; Combination of flags, USB_FLAG_*.
                rb      2       ; dword alignment
DeviceData      dd      ?
; Pointer to usb_device_data, common for all pipes for one device.
ends

; This structure describes the static head of every list of pipes.
struct usb_static_ep
; software fields
Bandwidth       dd      ?
; valid only for interrupt/isochronous USB1 lists
; The offsets of the following two fields must be the same in this structure
; and in usb_pipe.
NextVirt        dd      ?
PrevVirt        dd      ?
ends

; This structure represents one transfer descriptor
; ('g' stands for "general" as opposed to isochronous usb_itd).
; Note that one transfer can have several descriptors:
; a control transfer has three stages.
; Additionally, every controller has a limit on transfer length with
; one descriptor (packet size for UHCI, 1K for OHCI, 4K for EHCI),
; large transfers must be split into individual packets according to that limit.
struct usb_gtd
Callback        dd      ?
; Zero for intermediate descriptors, pointer to callback function
; for final descriptor. See the docs for description of the callback.
UserData        dd      ?
; Dword which is passed to Callback as is, not used by USB code itself.
; Two following fields organize all descriptors for one pipe in
; the linked list.
NextVirt        dd      ?
PrevVirt        dd      ?
Pipe            dd      ?
; Pointer to the parent usb_pipe.
Buffer          dd      ?
; Pointer to data for this descriptor.
Length          dd      ?
; Length of data for this descriptor.
ends

; =============================================================================
; =================================== Code ====================================
; =============================================================================

USB_STDCALL_VERIFY = 1
macro stdcall_verify [arg]
{
common
if USB_STDCALL_VERIFY
        pushad
        stdcall arg
        call    verify_regs
        popad
else
        stdcall arg
end if
}

; Initialization of usb_static_ep structure,
; called from controller-specific initialization; edi -> usb_static_ep
proc usb_init_static_endpoint
        mov     [edi+usb_static_ep.NextVirt], edi
        mov     [edi+usb_static_ep.PrevVirt], edi
        ret
endp

; Part of API for drivers, see documentation for USBOpenPipe.
proc usb_open_pipe stdcall uses ebx esi edi,\
 config_pipe:dword, endpoint:dword, maxpacket:dword, type:dword, interval:dword
locals
tt_vars         rd      (ehci_select_tt_interrupt_list.local_vars_size + 3) / 4
targetsmask     dd      ?       ; S-Mask for USB2
bandwidth       dd      ?
target          dd      ?
endl
; 1. Verify type of pipe: it must be one of *_PIPE constants.
; Isochronous pipes are not supported yet.
        mov     eax, [type]
        cmp     eax, INTERRUPT_PIPE
        ja      .badtype
        cmp     al, ISOCHRONOUS_PIPE
        jnz     .goodtype
.badtype:
        dbgstr 'unsupported type of USB pipe'
        jmp     .return0
.goodtype:
; 2. Allocate memory for pipe and transfer queue.
; Empty transfer queue consists of one inactive TD.
        mov     ebx, [config_pipe]
        mov     esi, [ebx+usb_pipe.Controller]
        mov     edx, [esi+usb_controller.HardwareFunc]
        call    [edx+usb_hardware_func.AllocPipe]
        test    eax, eax
        jz      .nothing
        mov     edi, eax
        mov     edx, [esi+usb_controller.HardwareFunc]
        call    [edx+usb_hardware_func.AllocTD]
        test    eax, eax
        jz      .free_and_return0
; 3. Initialize transfer queue: pointer to transfer descriptor,
; pointers in transfer descriptor, queue lock.
        mov     [edi+usb_pipe.LastTD], eax
        mov     [eax+usb_gtd.NextVirt], eax
        mov     [eax+usb_gtd.PrevVirt], eax
        mov     [eax+usb_gtd.Pipe], edi
        lea     ecx, [edi+usb_pipe.Lock]
        call    mutex_init
; 4. Initialize software part of pipe structure, except device-related fields.
        mov     al, byte [type]
        mov     [edi+usb_pipe.Type], al
        xor     eax, eax
        mov     [edi+usb_pipe.Flags], al
        mov     [edi+usb_pipe.DeviceData], eax
        mov     [edi+usb_pipe.Controller], esi
        or      [edi+usb_pipe.NextWait], -1
; 5. Initialize device-related fields:
; for zero endpoint, set .NextSibling = .PrevSibling = this;
; for other endpoins, copy device data, take the lock guarding pipe list
; for the device and verify that disconnect processing has not yet started
; for the device. (Since disconnect processing also takes that lock,
; either it has completed or it will not start until we release the lock.)
; Note: usb_device_disconnected should not see the new pipe until
; initialization is complete, so that lock will be held during next steps
; (disconnect processing should either not see it at all, or see fully
; initialized pipe).
        cmp     [endpoint], eax
        jz      .zero_endpoint
        mov     ecx, [ebx+usb_pipe.DeviceData]
        mov     [edi+usb_pipe.DeviceData], ecx
        call    mutex_lock
        test    [ebx+usb_pipe.Flags], USB_FLAG_CLOSED
        jz      .common
.fail:
; If disconnect processing has completed, unlock the mutex, free memory
; allocated in step 2 and return zero.
        call    mutex_unlock
        mov     edx, [esi+usb_controller.HardwareFunc]
        stdcall [edx+usb_hardware_func.FreeTD], [edi+usb_pipe.LastTD]
.free_and_return0:
        mov     edx, [esi+usb_controller.HardwareFunc]
        stdcall [edx+usb_hardware_func.FreePipe], edi
.return0:
        xor     eax, eax
        jmp     .nothing
.zero_endpoint:
        mov     [edi+usb_pipe.NextSibling], edi
        mov     [edi+usb_pipe.PrevSibling], edi
.common:
; 6. Initialize hardware part of pipe structure.
; 6a. Acquire the corresponding mutex.
        lea     ecx, [esi+usb_controller.ControlLock]
        cmp     [type], BULK_PIPE
        jb      @f      ; control pipe
        lea     ecx, [esi+usb_controller.BulkLock]
        jz      @f      ; bulk pipe
        lea     ecx, [esi+usb_controller.PeriodicLock]
@@:
        call    mutex_lock
; 6b. Let the controller-specific code do its job.
        push    ecx
        mov     edx, [esi+usb_controller.HardwareFunc]
        mov     eax, [edi+usb_pipe.LastTD]
        mov     ecx, [config_pipe]
        call    [edx+usb_hardware_func.InitPipe]
        pop     ecx
; 6c. Release the mutex.
        push    eax
        call    mutex_unlock
        pop     eax
; 6d. If controller-specific code indicates failure,
; release the lock taken in step 5, free memory allocated in step 2
; and return zero.
        test    eax, eax
        jz      .fail
; 7. The pipe is initialized. If this is not the first pipe for the device,
; insert it to the tail of pipe list for the device,
; increment number of pipes,
; release the lock taken at step 5.
        mov     ecx, [edi+usb_pipe.DeviceData]
        test    ecx, ecx
        jz      @f
        mov     eax, [ebx+usb_pipe.PrevSibling]
        mov     [edi+usb_pipe.NextSibling], ebx
        mov     [edi+usb_pipe.PrevSibling], eax
        mov     [ebx+usb_pipe.PrevSibling], edi
        mov     [eax+usb_pipe.NextSibling], edi
        inc     [ecx+usb_device_data.NumPipes]
        call    mutex_unlock
@@:
; 8. Return pointer to usb_pipe.
        mov     eax, edi
.nothing:
        ret
endp

; This procedure is called several times during initial device configuration,
; when usb_device_data structure is reallocated.
; It (re)initializes all pointers in usb_device_data.
; ebx -> usb_pipe
proc usb_reinit_pipe_list
        push    eax
; 1. (Re)initialize the lock guarding pipe list.
        mov     ecx, [ebx+usb_pipe.DeviceData]
        call    mutex_init
; 2. Initialize list of opened pipes: two entries, the head and ebx.
        add     ecx, usb_device_data.OpenedPipeList - usb_pipe.NextSibling
        mov     [ecx+usb_pipe.NextSibling], ebx
        mov     [ecx+usb_pipe.PrevSibling], ebx
        mov     [ebx+usb_pipe.NextSibling], ecx
        mov     [ebx+usb_pipe.PrevSibling], ecx
; 3. Initialize list of closed pipes: empty list, only the head is present.
        add     ecx, usb_device_data.ClosedPipeList - usb_device_data.OpenedPipeList
        mov     [ecx+usb_pipe.NextSibling], ecx
        mov     [ecx+usb_pipe.PrevSibling], ecx
        pop     eax
        ret
endp

; Part of API for drivers, see documentation for USBClosePipe.
proc usb_close_pipe
        push    ebx esi ; save used registers to be stdcall
virtual at esp
        rd      2       ; saved registers
        dd      ?       ; return address
.pipe   dd      ?
end virtual
; 1. Lock the pipe list for the device.
        mov     ebx, [.pipe]
        mov     esi, [ebx+usb_pipe.Controller]
        mov     ecx, [ebx+usb_pipe.DeviceData]
        call    mutex_lock
; 2. Set the flag "the driver has abandoned this pipe, free it at any time".
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_lock
        or      [ebx+usb_pipe.Flags], USB_FLAG_CAN_FREE
        call    mutex_unlock
; 3. Call the worker function.
        call    usb_close_pipe_nolock
; 4. Unlock the pipe list for the device.
        mov     ecx, [ebx+usb_pipe.DeviceData]
        call    mutex_unlock
; 5. Wakeup the USB thread so that it can proceed with releasing that pipe.
        push    edi
        call    usb_wakeup
        pop     edi
; 6. Return.
        pop     esi ebx ; restore used registers to be stdcall
        retn    4
endp

; Worker function for pipe closing. Called by usb_close_pipe API and
; from disconnect processing.
; The lock guarding pipe list for the device should be held by the caller.
; ebx -> usb_pipe, esi -> usb_controller
proc usb_close_pipe_nolock
; 1. Set the flag "pipe is closed, ignore new transfers".
; If it was already set, do nothing.
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_lock
        bts     dword [ebx+usb_pipe.Flags], USB_FLAG_CLOSED_BIT
        jc      .closed
        call    mutex_unlock
; 2. Remove the pipe from the list of opened pipes.
        mov     eax, [ebx+usb_pipe.NextSibling]
        mov     edx, [ebx+usb_pipe.PrevSibling]
        mov     [eax+usb_pipe.PrevSibling], edx
        mov     [edx+usb_pipe.NextSibling], eax
; 3. Unlink the pipe from hardware structures.
; 3a. Acquire the corresponding lock.
        lea     edx, [esi+usb_controller.WaitPipeListAsync]
        lea     ecx, [esi+usb_controller.ControlLock]
        cmp     [ebx+usb_pipe.Type], BULK_PIPE
        jb      @f      ; control pipe
        lea     ecx, [esi+usb_controller.BulkLock]
        jz      @f      ; bulk pipe
        add     edx, usb_controller.WaitPipeListPeriodic - usb_controller.WaitPipeListAsync
        lea     ecx, [esi+usb_controller.PeriodicLock]
@@:
        push    edx
        call    mutex_lock
        push    ecx
; 3b. Let the controller-specific code do its job.
        mov     eax, [esi+usb_controller.HardwareFunc]
        call    [eax+usb_hardware_func.UnlinkPipe]
; 3c. Release the corresponding lock.
        pop     ecx
        call    mutex_unlock
; 4. Put the pipe into wait queue.
        pop     edx
        cmp     [ebx+usb_pipe.NextWait], -1
        jz      .insert_new
        or      [ebx+usb_pipe.Flags], USB_FLAG_EXTRA_WAIT
        jmp     .inserted
.insert_new:
        mov     eax, [edx]
        mov     [ebx+usb_pipe.NextWait], eax
        mov     [edx], ebx
.inserted:
; 5. Return.
        ret
.closed:
        call    mutex_unlock
        xor     eax, eax
        ret
endp

; This procedure is called when a pipe with USB_FLAG_CLOSED is removed from the
; corresponding wait list. It means that the hardware has fully forgot about it.
; ebx -> usb_pipe, esi -> usb_controller
proc usb_pipe_closed
        push    edi
        mov     edi, [esi+usb_controller.HardwareFunc]
; 1. Loop over all transfers, calling the driver with USB_STATUS_CLOSED
; and freeing all descriptors.
        mov     edx, [ebx+usb_pipe.LastTD]
        test    edx, edx
        jz      .no_transfer
        mov     edx, [edx+usb_gtd.NextVirt]
.transfer_loop:
        cmp     edx, [ebx+usb_pipe.LastTD]
        jz      .transfer_done
        mov     ecx, [edx+usb_gtd.Callback]
        test    ecx, ecx
        jz      .no_callback
        push    edx
        stdcall_verify ecx, ebx, USB_STATUS_CLOSED, \
                [edx+usb_gtd.Buffer], 0, [edx+usb_gtd.UserData]
        pop     edx
.no_callback:
        push    [edx+usb_gtd.NextVirt]
        stdcall [edi+usb_hardware_func.FreeTD], edx
        pop     edx
        jmp     .transfer_loop
.transfer_done:
        stdcall [edi+usb_hardware_func.FreeTD], edx
.no_transfer:
; 2. Decrement number of pipes for the device.
; If this pipe is the last pipe, go to 5.
        mov     ecx, [ebx+usb_pipe.DeviceData]
        call    mutex_lock
        dec     [ecx+usb_device_data.NumPipes]
        jz      .last_pipe
        call    mutex_unlock
; 3. If the flag "the driver has abandoned this pipe" is set,
; free memory and return.
        test    [ebx+usb_pipe.Flags], USB_FLAG_CAN_FREE
        jz      .nofree
        stdcall [edi+usb_hardware_func.FreePipe], ebx
        pop     edi
        ret
; 4. Otherwise, add it to the list of closed pipes and return.
.nofree:
        add     ecx, usb_device_data.ClosedPipeList - usb_pipe.NextSibling
        mov     edx, [ecx+usb_pipe.PrevSibling]
        mov     [ebx+usb_pipe.NextSibling], ecx
        mov     [ebx+usb_pipe.PrevSibling], edx
        mov     [ecx+usb_pipe.PrevSibling], ebx
        mov     [edx+usb_pipe.NextSibling], ebx
        pop     edi
        ret
.last_pipe:
; That was the last pipe for the device.
; 5. Notify device driver(s) about disconnect.
        call    mutex_unlock
        mov     eax, [ecx+usb_device_data.NumInterfaces]
        test    eax, eax
        jz      .notify_done
        add     ecx, [ecx+usb_device_data.Interfaces]
.notify_loop:
        mov     edx, [ecx+usb_interface_data.DriverFunc]
        test    edx, edx
        jz      @f
        mov     edx, [edx+USBSRV.usb_func]
        cmp     [edx+USBFUNC.strucsize], USBFUNC.device_disconnect + 4
        jb      @f
        mov     edx, [edx+USBFUNC.device_disconnect]
        test    edx, edx
        jz      @f
        push    eax ecx
        stdcall_verify edx, [ecx+usb_interface_data.DriverData]
        pop     ecx eax
@@:
        add     ecx, sizeof.usb_interface_data
        dec     eax
        jnz     .notify_loop
.notify_done:
; 6. Bus address, if assigned, can now be reused.
        call    [edi+usb_hardware_func.GetDeviceAddress]
        test    eax, eax
        jz      @f
        bts     [esi+usb_controller.ExistingAddresses], eax
@@:
        dbgstr 'USB device disconnected'
; 7. All drivers have returned from disconnect callback,
; so all drivers should not use any device-related pipes.
; Free the remaining pipes.
        mov     eax, [ebx+usb_pipe.DeviceData]
        add     eax, usb_device_data.ClosedPipeList - usb_pipe.NextSibling
        push    eax
        mov     eax, [eax+usb_pipe.NextSibling]
.free_loop:
        cmp     eax, [esp]
        jz      .free_done
        push    [eax+usb_pipe.NextSibling]
        stdcall [edi+usb_hardware_func.FreePipe], eax
        pop     eax
        jmp     .free_loop
.free_done:
        stdcall [edi+usb_hardware_func.FreePipe], ebx
        pop     eax
; 8. Free the usb_device_data structure.
        sub     eax, usb_device_data.ClosedPipeList - usb_pipe.NextSibling
        call    free
; 9. Return.
.nothing:
        pop     edi
        ret
endp

; Part of API for drivers, see documentation for USBNormalTransferAsync.
proc usb_normal_transfer_async stdcall uses ebx edi,\
 pipe:dword, buffer:dword, size:dword, callback:dword, calldata:dword, flags:dword
; 1. Sanity check: callback must be nonzero.
; (It is important for other parts of code.)
        xor     eax, eax
        cmp     [callback], eax
        jz      .nothing
; 2. Lock the transfer queue.
        mov     ebx, [pipe]
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_lock
; 3. If the pipe has already been closed (presumably due to device disconnect),
; release the lock taken in step 2 and return zero.
        xor     eax, eax
        test    [ebx+usb_pipe.Flags], USB_FLAG_CLOSED
        jnz     .unlock
; 4. Allocate and initialize TDs for the transfer.
        mov     edx, [ebx+usb_pipe.Controller]
        mov     edi, [edx+usb_controller.HardwareFunc]
        stdcall [edi+usb_hardware_func.AllocTransfer], [buffer], [size], [flags], [ebx+usb_pipe.LastTD], 0
; If failed, release the lock taken in step 2 and return zero.
        test    eax, eax
        jz      .unlock
; 5. Store callback and its parameters in the last descriptor for this transfer.
        mov     ecx, [eax+usb_gtd.PrevVirt]
        mov     edx, [callback]
        mov     [ecx+usb_gtd.Callback], edx
        mov     edx, [calldata]
        mov     [ecx+usb_gtd.UserData], edx
        mov     edx, [buffer]
        mov     [ecx+usb_gtd.Buffer], edx
; 6. Advance LastTD pointer and activate transfer.
        push    [ebx+usb_pipe.LastTD]
        mov     [ebx+usb_pipe.LastTD], eax
        call    [edi+usb_hardware_func.InsertTransfer]
        pop     eax
; 7. Release the lock taken in step 2 and
; return pointer to the first descriptor for the new transfer.
.unlock:
        push    eax
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_unlock
        pop     eax
.nothing:
        ret
endp

; Part of API for drivers, see documentation for USBControlTransferAsync.
proc usb_control_async stdcall uses ebx edi,\
 pipe:dword, config:dword, buffer:dword, size:dword, callback:dword, calldata:dword, flags:dword
locals
last_td         dd      ?
endl
; 1. Sanity check: callback must be nonzero.
; (It is important for other parts of code.)
        cmp     [callback], 0
        jz      .return0
; 2. Lock the transfer queue.
        mov     ebx, [pipe]
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_lock
; 3. If the pipe has already been closed (presumably due to device disconnect),
; release the lock taken in step 2 and return zero.
        test    [ebx+usb_pipe.Flags], USB_FLAG_CLOSED
        jnz     .unlock_return0
; A control transfer contains two or three stages:
; Setup stage, optional Data stage, Status stage.
; 4. Allocate and initialize TDs for the Setup stage.
; Payload is 8 bytes from [config].
        mov     edx, [ebx+usb_pipe.Controller]
        mov     edi, [edx+usb_controller.HardwareFunc]
        stdcall [edi+usb_hardware_func.AllocTransfer], [config], 8, 0, [ebx+usb_pipe.LastTD], (2 shl 2) + 0
                ; short transfer is an error, direction is DATA0, token is SETUP
        mov     [last_td], eax
        test    eax, eax
        jz      .fail
; 5. Allocate and initialize TDs for the Data stage, if [size] is nonzero.
; Payload is [size] bytes from [buffer].
        mov     edx, [config]
        mov     ecx, (3 shl 2) + 1      ; DATA1, token is OUT
        cmp     byte [edx], 0
        jns     @f
        cmp     [size], 0
        jz      @f
        inc     ecx     ; token is IN
@@:
        cmp     [size], 0
        jz      .nodata
        push    ecx
        stdcall [edi+usb_hardware_func.AllocTransfer], [buffer], [size], [flags], eax, ecx
        pop     ecx
        test    eax, eax
        jz      .fail
        mov     [last_td], eax
.nodata:
; 6. Allocate and initialize TDs for the Status stage.
; No payload.
        xor     ecx, 3  ; IN becomes OUT, OUT becomes IN
        stdcall [edi+usb_hardware_func.AllocTransfer], 0, 0, 0, eax, ecx
        test    eax, eax
        jz      .fail
; 7. Store callback and its parameters in the last descriptor for this transfer.
        mov     ecx, [eax+usb_gtd.PrevVirt]
        mov     edx, [callback]
        mov     [ecx+usb_gtd.Callback], edx
        mov     edx, [calldata]
        mov     [ecx+usb_gtd.UserData], edx
        mov     edx, [buffer]
        mov     [ecx+usb_gtd.Buffer], edx
; 8. Advance LastTD pointer and activate transfer.
        push    [ebx+usb_pipe.LastTD]
        mov     [ebx+usb_pipe.LastTD], eax
        call    [edi+usb_hardware_func.InsertTransfer]
; 9. Release the lock taken in step 2 and
; return pointer to the first descriptor for the new transfer.
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_unlock
        pop     eax
        ret
.fail:
        mov     eax, [last_td]
        test    eax, eax
        jz      .unlock_return0
        stdcall usb_undo_tds, [ebx+usb_pipe.LastTD]
.unlock_return0:
        lea     ecx, [ebx+usb_pipe.Lock]
        call    mutex_unlock
.return0:
        xor     eax, eax
        ret
endp

; Part of API for drivers, see documentation for USBGetParam.
proc usb_get_param
virtual at esp
                dd      ?       ; return address
.pipe           dd      ?
.param          dd      ?
end virtual
        mov     edx, [.param]
        mov     ecx, [.pipe]
        mov     eax, [ecx+usb_pipe.DeviceData]
        test    edx, edx
        jz      .get_device_descriptor
        dec     edx
        jz      .get_config_descriptor
        dec     edx
        jz      .get_speed
        or      eax, -1
        ret     8
.get_device_descriptor:
        add     eax, usb_device_data.DeviceDescriptor
        ret     8
.get_config_descriptor:
        movzx   ecx, [eax+usb_device_data.DeviceDescrSize]
        lea     eax, [eax+ecx+usb_device_data.DeviceDescriptor]
        ret     8
.get_speed:
        movzx   eax, [eax+usb_device_data.Speed]
        ret     8
endp

; Initialize software part of usb_gtd. Called from controller-specific code
; somewhere in AllocTransfer with eax -> next (inactive) usb_gtd,
; ebx -> usb_pipe, ebp frame from call to AllocTransfer with [.td] ->
; current (initializing) usb_gtd.
; Returns ecx = [.td].
proc usb_init_transfer
virtual at ebp-4
.Size   dd      ?
        rd      2
.Buffer dd      ?
        dd      ?
.Flags  dd      ?
.td     dd      ?
end virtual
        mov     [eax+usb_gtd.Pipe], ebx
        mov     ecx, [.td]
        mov     [eax+usb_gtd.PrevVirt], ecx
        mov     edx, [ecx+usb_gtd.NextVirt]
        mov     [ecx+usb_gtd.NextVirt], eax
        mov     [eax+usb_gtd.NextVirt], edx
        mov     [edx+usb_gtd.PrevVirt], eax
        mov     edx, [.Size]
        mov     [ecx+usb_gtd.Length], edx
        xor     edx, edx
        mov     [ecx+usb_gtd.Callback], edx
        mov     [ecx+usb_gtd.UserData], edx
        ret
endp

; Free all TDs for the current transfer if something has failed
; during initialization (e.g. no memory for the next TD).
; Stdcall with one stack argument = first TD for the transfer
; and eax = last initialized TD for the transfer.
proc usb_undo_tds
        push    [eax+usb_gtd.NextVirt]
@@:
        cmp     eax, [esp+8]
        jz      @f
        push    [eax+usb_gtd.PrevVirt]
        stdcall [edi+usb_hardware_func.FreeTD], eax
        pop     eax
        jmp     @b
@@:
        pop     ecx
        mov     [eax+usb_gtd.NextVirt], ecx
        mov     [ecx+usb_gtd.PrevVirt], eax
        ret     4
endp

; Helper procedure for handling short packets in controller-specific code.
; Returns with CF cleared if this is the final packet in some stage:
; for control transfers that means one of Data and Status stages,
; for other transfers - the final packet in the only stage.
proc usb_is_final_packet
        cmp     [ebx+usb_gtd.Callback], 0
        jnz     .nothing
        mov     eax, [ebx+usb_gtd.NextVirt]
        cmp     [eax+usb_gtd.Callback], 0
        jz      .stc
        mov     eax, [ebx+usb_gtd.Pipe]
        cmp     [eax+usb_pipe.Type], CONTROL_PIPE
        jz      .nothing
.stc:
        stc
.nothing:
        ret
endp

; Helper procedure for controller-specific code:
; removes one TD from the transfer queue, ebx -> usb_gtd to remove.
proc usb_unlink_td
        mov     ecx, [ebx+usb_gtd.Pipe]
        add     ecx, usb_pipe.Lock
        call    mutex_lock
        mov     eax, [ebx+usb_gtd.PrevVirt]
        mov     edx, [ebx+usb_gtd.NextVirt]
        mov     [edx+usb_gtd.PrevVirt], eax
        mov     [eax+usb_gtd.NextVirt], edx
        call    mutex_unlock
        ret
endp

if USB_STDCALL_VERIFY
proc verify_regs
virtual at esp
        dd      ?       ; return address
.edi    dd      ?
.esi    dd      ?
.ebp    dd      ?
.esp    dd      ?
.ebx    dd      ?
.edx    dd      ?
.ecx    dd      ?
.eax    dd      ?
end virtual
        cmp     ebx, [.ebx]
        jz      @f
        dbgstr 'ERROR!!! ebx changed'
@@:
        cmp     esi, [.esi]
        jz      @f
        dbgstr 'ERROR!!! esi changed'
@@:
        cmp     edi, [.edi]
        jz      @f
        dbgstr 'ERROR!!! edi changed'
@@:
        cmp     ebp, [.ebp]
        jz      @f
        dbgstr 'ERROR!!! ebp changed'
@@:
        ret
endp
end if
