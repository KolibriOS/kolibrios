; USB Host Controller support code: hardware-independent part,
; common for all controller types.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; USB device must have at least 100ms of stable power before initializing can
; proceed; one timer tick is 10ms, so enforce delay in 10 ticks
USB_CONNECT_DELAY = 10
; USB requires at least 10 ms for reset signalling. Normally, this is one timer
; tick. However, it is possible that we start reset signalling in the end of
; interval between timer ticks and then we test time in the start of the next
; interval; in this case, the delta between [timer_ticks] is 1, but the real
; time passed is significantly less than 10 ms. To avoid this, we add an extra
; tick; this guarantees that at least 10 ms have passed.
USB_RESET_TIME = 2
; USB requires at least 10 ms of reset recovery, a delay between reset
; signalling and any commands to device. Add an extra tick for the same reasons
; as with the previous constant.
USB_RESET_RECOVERY_TIME = 2

; =============================================================================
; ================================ Structures =================================
; =============================================================================
; Controller descriptor.
; This structure represents the common (controller-independent) part
; of a controller for the USB code. The corresponding controller-dependent
; part *hci_controller is located immediately before usb_controller.
struct usb_controller
; Two following fields organize all controllers in the global linked list.
Next            dd      ?
Prev            dd      ?
HardwareFunc    dd      ?
; Pointer to usb_hardware_func structure with controller-specific functions.
NumPorts        dd      ?
; Number of ports in the root hub.
SetAddressBuffer        rb      8
; Buffer for USB control command SET_ADDRESS.
ExistingAddresses       rd      128/32
; Bitmask for 128 bits; bit i is cleared <=> address i is free for allocating
; for new devices. Bit 0 is always set.
;
; The hardware is allowed to cache some data from hardware structures.
; Regular operations are designed considering this,
; but sometimes it is required to wait for synchronization of hardware cache
; with modified structures in memory.
; The code keeps two queues of pipes waiting for synchronization,
; one for asynchronous (bulk/control) pipes, one for periodic pipes, hardware
; cache is invalidated under different conditions for those types.
; Both queues are organized in the same way, as single-linked lists.
; There are three special positions: the head of list (new pipes are added
; here), the first pipe to be synchronized at the current iteration,
; the tail of list (all pipes starting from here are synchronized).
WaitPipeListAsync       dd      ?
WaitPipeListPeriodic    dd      ?
; List heads.
WaitPipeRequestAsync    dd      ?
WaitPipeRequestPeriodic dd      ?
; Pending request to hardware to refresh cache for items from WaitPipeList*.
; (Pointers to some items in WaitPipeList* or NULLs).
ReadyPipeHeadAsync      dd      ?
ReadyPipeHeadPeriodic   dd      ?
; Items of RemovingList* which were released by hardware and are ready
; for further processing.
; (Pointers to some items in WaitPipeList* or NULLs).
NewConnected    dd      ?
; bit mask of recently connected ports of the root hub,
; bit set = a device was recently connected to the corresponding port;
; after USB_CONNECT_DELAY ticks of stable status these ports are moved to
; PendingPorts
NewDisconnected dd      ?
; bit mask of disconnected ports of the root hub,
; bit set = a device in the corresponding port was disconnected,
; disconnect processing is required.
PendingPorts    dd      ?
; bit mask of ports which are ready to be initialized
ControlLock     MUTEX   ?
; mutex which guards all operations with control queue
BulkLock        MUTEX   ?
; mutex which guards all operations with bulk queue
PeriodicLock    MUTEX   ?
; mutex which guards all operations with periodic queues
WaitSpinlock:
; spinlock guarding WaitPipeRequest/ReadyPipeHead (but not WaitPipeList)
StartWaitFrame  dd      ?
; USB frame number when WaitPipeRequest* was registered.
ResettingHub    dd      ?
; Pointer to usb_hub responsible for the currently resetting port, if any.
; NULL for the root hub.
ResettingPort   db      ?
; Port that is currently resetting, 0-based.
ResettingSpeed  db      ?
; Speed of currently resetting device.
ResettingStatus db      ?
; Status of port reset. 0 = no port is resetting, -1 = reset failed,
; 1 = reset in progress, 2 = reset recovery in progress.
                rb      1       ; alignment
ResetTime       dd      ?
; Time when reset signalling or reset recovery has been started.
ConnectedTime   rd      16
; Time, in timer ticks, when the port i has signalled the connect event.
; Valid only if bit i in NewConnected is set.
DevicesByPort   rd      16
; Pointer to usb_pipe for zero endpoint (which serves as device handle)
; for each port.
ends

; Interface-specific data. Several interfaces of one device can operate
; independently, each is controlled by some driver and is identified by
; some driver-specific data passed as is to the driver.
struct usb_interface_data
DriverData      dd      ?
; Passed as is to the driver.
DriverFunc      dd      ?
; Pointer to USBSRV structure for the driver.
ends

; Device-specific data.
struct usb_device_data
PipeListLock    MUTEX
; Lock guarding OpenedPipeList. Must be the first item of the structure,
; the code passes pointer to usb_device_data as is to mutex_lock/unlock.
OpenedPipeList  rd      2
; List of all opened pipes for the device.
; Used when the device is disconnected, so all pipes should be closed.
ClosedPipeList  rd      2
; List of all closed, but still valid pipes for the device.
; A pipe closed with USBClosePipe is just deallocated,
; but a pipe closed due to disconnect must remain valid until driver-provided
; disconnect handler returns; this list links all such pipes to deallocate them
; after disconnect processing.
NumPipes        dd      ?
; Number of not-yet-closed pipes.
Hub             dd      ?
; NULL if connected to the root hub, pointer to usb_hub otherwise.
TTHub           dd      ?
; Pointer to usb_hub for (the) hub with Transaction Translator for the device,
; NULL if the device operates in the same speed as the controller.
Port            db      ?
; Port on the hub, zero-based.
TTPort          db      ?
; Port on the TTHub, zero-based.
DeviceDescrSize db      ?
; Size of device descriptor.
Speed           db      ?
; Device speed, one of USB_SPEED_*.
NumInterfaces   dd      ?
; Number of interfaces.
ConfigDataSize  dd      ?
; Total size of data associated with the configuration descriptor
; (including the configuration descriptor itself).
Interfaces      dd      ?
; Offset from the beginning of this structure to Interfaces field.
; Variable-length fields:
; DeviceDescriptor:
;  device descriptor starts here
; ConfigDescriptor = DeviceDescriptor + DeviceDescrSize
;  configuration descriptor with all associated data
; Interfaces = ALIGN_UP(ConfigDescriptor + ConfigDataSize, 4)
;  array of NumInterfaces elements of type usb_interface_data
ends

usb_device_data.DeviceDescriptor = sizeof.usb_device_data

; Description of controller-specific data and functions.
struct usb_hardware_func
ID              dd      ?       ; '*HCI'
DataSize        dd      ?       ; sizeof(*hci_controller)
Init            dd      ?
; Initialize controller-specific part of controller data.
; in: eax -> *hci_controller to initialize, [ebp-4] = (bus shl 8) + devfn
; out: eax = 0 <=> failed, otherwise eax -> usb_controller
ProcessDeferred dd      ?
; Called regularly from the main loop of USB thread
; (either due to timeout from a previous call, or due to explicit wakeup).
; in: esi -> usb_controller
; out: eax = maximum timeout for next call (-1 = infinity)
SetDeviceAddress        dd      ?
; in: esi -> usb_controller, ebx -> usb_pipe, cl = address
GetDeviceAddress        dd      ?
; in: esi -> usb_controller, ebx -> usb_pipe
; out: eax = address
PortDisable             dd      ?
; Disable the given port in the root hub.
; in: esi -> usb_controller, ecx = port (zero-based)
InitiateReset           dd      ?
; Start reset signalling on the given port.
; in: esi -> usb_controller, ecx = port (zero-based)
SetEndpointPacketSize   dd      ?
; in: esi -> usb_controller, ebx -> usb_pipe, ecx = packet size
AllocPipe               dd      ?
; out: eax = pointer to allocated usb_pipe
FreePipe                dd      ?
; void stdcall with one argument = pointer to previously allocated usb_pipe
InitPipe                dd      ?
; in: edi -> usb_pipe for target, ecx -> usb_pipe for config pipe,
; esi -> usb_controller, eax -> usb_gtd for the first TD,
; [ebp+12] = endpoint, [ebp+16] = maxpacket, [ebp+20] = type
UnlinkPipe              dd      ?
; esi -> usb_controller, ebx -> usb_pipe
AllocTD                 dd      ?
; out: eax = pointer to allocated usb_gtd
FreeTD                  dd      ?
; void stdcall with one argument = pointer to previously allocated usb_gtd
AllocTransfer           dd      ?
; Allocate and initialize one stage of a transfer.
; ebx -> usb_pipe, other parameters are passed through the stack:
; buffer,size = data to transfer
; flags = same as in usb_open_pipe:
;   bit 0 = allow short transfer, other bits reserved
; td = pointer to the current end-of-queue descriptor
; direction =
;   0000b for normal transfers,
;   1000b for control SETUP transfer,
;   1101b for control OUT transfer,
;   1110b for control IN transfer
; returns eax = pointer to the new end-of-queue descriptor
; (not included in the queue itself) or 0 on error
InsertTransfer          dd      ?
; Activate previously initialized transfer (maybe with multiple stages).
; esi -> usb_controller, ebx -> usb_pipe,
; [esp+4] -> first usb_gtd for the transfer,
; ecx -> last descriptor for the transfer
NewDevice               dd      ?
; Initiate configuration of a new device (create pseudo-pipe describing that
; device and call usb_new_device).
; esi -> usb_controller, eax = speed (one of USB_SPEED_* constants).
ends

; =============================================================================
; =================================== Code ====================================
; =============================================================================

; Initializes one controller, called by usb_init for every controller.
; edi -> usb_hardware_func, eax -> PCIDEV structure for the device.
proc usb_init_controller
        push    ebp
        mov     ebp, esp
; 1. Store in the stack PCI coordinates and save pointer to PCIDEV:
; make [ebp-4] = (bus shl 8) + devfn, used by controller-specific Init funcs.
        push    dword [eax+PCIDEV.devfn]
        push    eax
; 2. Allocate *hci_controller + usb_controller.
        mov     ebx, [edi+usb_hardware_func.DataSize]
        add     ebx, sizeof.usb_controller
        stdcall kernel_alloc, ebx
        test    eax, eax
        jz      .nothing
; 3. Zero-initialize both structures.
        push    edi eax
        mov     ecx, ebx
        shr     ecx, 2
        xchg    edi, eax
        xor     eax, eax
        rep stosd
; 4. Initialize usb_controller structure,
; except data known only to controller-specific code (like NumPorts)
; and link fields
; (this structure will be inserted to the overall list at step 6).
        dec     eax
        mov     [edi+usb_controller.ExistingAddresses+4-sizeof.usb_controller], eax
        mov     [edi+usb_controller.ExistingAddresses+8-sizeof.usb_controller], eax
        mov     [edi+usb_controller.ExistingAddresses+12-sizeof.usb_controller], eax
        mov     [edi+usb_controller.ResettingPort-sizeof.usb_controller], al    ; no resetting port
        dec     eax     ; don't allocate zero address
        mov     [edi+usb_controller.ExistingAddresses-sizeof.usb_controller], eax
        lea     ecx, [edi+usb_controller.PeriodicLock-sizeof.usb_controller]
        call    mutex_init
        add     ecx, usb_controller.ControlLock - usb_controller.PeriodicLock
        call    mutex_init
        add     ecx, usb_controller.BulkLock - usb_controller.ControlLock
        call    mutex_init
        pop     eax edi
        mov     [eax+ebx-sizeof.usb_controller+usb_controller.HardwareFunc], edi
        push    eax
; 5. Call controller-specific initialization.
; If failed, free memory allocated in step 2 and return.
        call    [edi+usb_hardware_func.Init]
        test    eax, eax
        jz      .fail
        pop     ecx
; 6. Insert the controller to the global list.
        xchg    eax, ebx
        mov     ecx, usb_controllers_list_mutex
        call    mutex_lock
        mov     edx, usb_controllers_list
        mov     eax, [edx+usb_controller.Prev]
        mov     [ebx+usb_controller.Next], edx
        mov     [ebx+usb_controller.Prev], eax
        mov     [edx+usb_controller.Prev], ebx
        mov     [eax+usb_controller.Next], ebx
        call    mutex_unlock
; 7. Wakeup USB thread to call ProcessDeferred.
        call    usb_wakeup
.nothing:
; 8. Restore pointer to PCIDEV saved in step 1 and return.
        pop     eax
        leave
        ret
.fail:
        call    kernel_free
        jmp     .nothing
endp

; Helper function, calculates physical address including offset in page.
proc get_phys_addr
        push    ecx
        mov     ecx, eax
        and     ecx, 0xFFF
        call    get_pg_addr
        add     eax, ecx
        pop     ecx
        ret
endp

; Put the given control pipe in the wait list;
; called when the pipe structure is changed and a possible hardware cache
; needs to be synchronized. When it will be known that the cache is updated,
; usb_subscription_done procedure will be called.
proc usb_subscribe_control
        cmp     [ebx+usb_pipe.NextWait], -1
        jnz     @f
        mov     eax, [esi+usb_controller.WaitPipeListAsync]
        mov     [ebx+usb_pipe.NextWait], eax
        mov     [esi+usb_controller.WaitPipeListAsync], ebx
@@:
        ret
endp

; Called after synchronization of hardware cache with software changes.
; Continues process of device enumeration based on when it was delayed
; due to call to usb_subscribe_control.
proc usb_subscription_done
        mov     eax, [ebx+usb_pipe.DeviceData]
        cmp     [eax+usb_device_data.DeviceDescrSize], 0
        jz      usb_after_set_address
        jmp     usb_after_set_endpoint_size
endp

; This function is called when a new device has either passed
; or failed first stages of configuration, so the next device
; can enter configuration process.
proc usb_test_pending_port
        mov     [esi+usb_controller.ResettingPort], -1
        cmp     [esi+usb_controller.PendingPorts], 0
        jz      .nothing
        bsf     ecx, [esi+usb_controller.PendingPorts]
        btr     [esi+usb_controller.PendingPorts], ecx
        mov     eax, [esi+usb_controller.HardwareFunc]
        jmp     [eax+usb_hardware_func.InitiateReset]
.nothing:
        ret
endp

; This procedure is regularly called from controller-specific ProcessDeferred,
; it checks whether there are disconnected events and if so, process them.
proc usb_disconnect_stage2
        bsf     ecx, [esi+usb_controller.NewDisconnected]
        jz      .nothing
        lock btr [esi+usb_controller.NewDisconnected], ecx
        btr     [esi+usb_controller.PendingPorts], ecx
        xor     ebx, ebx
        xchg    ebx, [esi+usb_controller.DevicesByPort+ecx*4]
        test    ebx, ebx
        jz      usb_disconnect_stage2
        call    usb_device_disconnected
        jmp     usb_disconnect_stage2
.nothing:
        ret
endp

; Initial stage of disconnect processing: called when device is disconnected.
proc usb_device_disconnected
; Loop over all pipes, close everything, wait until hardware reacts.
; The final handling is done in usb_pipe_closed.
        push    ebx
        mov     ecx, [ebx+usb_pipe.DeviceData]
        call    mutex_lock
        lea     eax, [ecx+usb_device_data.OpenedPipeList-usb_pipe.NextSibling]
        push    eax
        mov     ebx, [eax+usb_pipe.NextSibling]
.pipe_loop:
        call    usb_close_pipe_nolock
        mov     ebx, [ebx+usb_pipe.NextSibling]
        cmp     ebx, [esp]
        jnz     .pipe_loop
        pop     eax
        pop     ebx
        mov     ecx, [ebx+usb_pipe.DeviceData]
        call    mutex_unlock
        ret
endp

; Called from controller-specific ProcessDeferred,
; processes wait-pipe-done notifications,
; returns whether there are more items in wait queues.
; in: esi -> usb_controller
; out: eax = bitmask of pipe types with non-empty wait queue
proc usb_process_wait_lists
        xor     edx, edx
        push    edx
        call    usb_process_one_wait_list
        jnc     @f
        or      byte [esp], 1 shl CONTROL_PIPE
@@:
        movi    edx, 4
        call    usb_process_one_wait_list
        jnc     @f
        or      byte [esp], 1 shl INTERRUPT_PIPE
@@:
        xor     edx, edx
        call    usb_process_one_wait_list
        jnc     @f
        or      byte [esp], 1 shl CONTROL_PIPE
@@:
        pop     eax
        ret
endp

; Helper procedure for usb_process_wait_lists;
; does the same for one wait queue.
; in: esi -> usb_controller,
; edx=0 for *Async, edx=4 for *Periodic list
; out: CF = issue new request
proc usb_process_one_wait_list
; 1. Check whether there is a pending request. If so, do nothing.
        mov     ebx, [esi+usb_controller.WaitPipeRequestAsync+edx]
        cmp     ebx, [esi+usb_controller.ReadyPipeHeadAsync+edx]
        clc
        jnz     .nothing
; 2. Check whether there are new data. If so, issue a new request.
        cmp     ebx, [esi+usb_controller.WaitPipeListAsync+edx]
        stc
        jnz     .nothing
        test    ebx, ebx
        jz      .nothing
; 3. Clear all lists.
        xor     ecx, ecx
        mov     [esi+usb_controller.WaitPipeListAsync+edx], ecx
        mov     [esi+usb_controller.WaitPipeRequestAsync+edx], ecx
        mov     [esi+usb_controller.ReadyPipeHeadAsync+edx], ecx
; 4. Loop over all pipes from the wait list.
.pipe_loop:
; For every pipe:
; 5. Save edx and next pipe in the list.
        push    edx
        push    [ebx+usb_pipe.NextWait]
; 6. If USB_FLAG_EXTRA_WAIT is set, reinsert the pipe to the list and continue.
        test    [ebx+usb_pipe.Flags], USB_FLAG_EXTRA_WAIT
        jz      .process
        mov     eax, [esi+usb_controller.WaitPipeListAsync+edx]
        mov     [ebx+usb_pipe.NextWait], eax
        mov     [esi+usb_controller.WaitPipeListAsync+edx], ebx
        jmp     .continue
.process:
; 7. Call the handler depending on USB_FLAG_CLOSED.
        or      [ebx+usb_pipe.NextWait], -1
        test    [ebx+usb_pipe.Flags], USB_FLAG_CLOSED
        jz      .nodisconnect
        call    usb_pipe_closed
        jmp     .continue
.nodisconnect:
        call    usb_subscription_done
.continue:
; 8. Restore edx and next pipe saved in step 5 and continue the loop.
        pop     ebx
        pop     edx
        test    ebx, ebx
        jnz     .pipe_loop
.check_new_work:
; 9. Set CF depending on whether WaitPipeList* is nonzero.
        cmp     [esi+usb_controller.WaitPipeListAsync+edx], 1
        cmc
.nothing:
        ret
endp
