; USB Host Controller support code: hardware-independent part,
; common for all controller types.

iglobal
; USB HC support: some functions interesting only for *HCI-drivers.
align 4
usb_hc_func:
        dd      usb_process_gtd
        dd      usb_init_static_endpoint
        dd      usb_wakeup_if_needed
        dd      usb_subscribe_control
        dd      usb_subscription_done
        dd      usb_allocate_common
        dd      usb_free_common
        dd      usb_td_to_virt
        dd      usb_init_transfer
        dd      usb_undo_tds
        dd      usb_test_pending_port
        dd      usb_get_tt
        dd      usb_get_tt_think_time
        dd      usb_new_device
        dd      usb_disconnect_stage2
        dd      usb_process_wait_lists
        dd      usb_unlink_td
        dd      usb_is_final_packet
        dd      usb_find_ehci_companion
endg

; Initializes one controller, called by usb_init for every controller.
; eax -> PCIDEV structure for the device.
proc usb_init_controller
        push    ebp
        mov     ebp, esp
; 1. Store in the stack PCI coordinates and save pointer to PCIDEV:
; make [ebp-4] = (bus shl 8) + devfn, used by controller-specific Init funcs.
        push    dword [eax+PCIDEV.devfn]
        push    eax
        mov     edi, [eax+PCIDEV.owner]
        test    edi, edi
        jz      .nothing
        mov     edi, [edi+USBSRV.usb_func]
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
        mov     eax, [ebp-4]
        mov     [edi+usb_controller.PCICoordinates-sizeof.usb_controller], eax
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

; Put the given control/bulk pipe in the wait list;
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

; Same as usb_subscribe_control, but for interrupt/isochronous pipe.
proc usb_subscribe_periodic
        cmp     [ebx+usb_pipe.NextWait], -1
        jnz     @f
        mov     eax, [esi+usb_controller.WaitPipeListPeriodic]
        mov     [ebx+usb_pipe.NextWait], eax
        mov     [esi+usb_controller.WaitPipeListPeriodic], ebx
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
; 7. Call the handler depending on USB_FLAG_CLOSED and USB_FLAG_DISABLED.
        or      [ebx+usb_pipe.NextWait], -1
        test    [ebx+usb_pipe.Flags], USB_FLAG_CLOSED
        jz      .nodisconnect
        call    usb_pipe_closed
        jmp     .continue
.nodisconnect:
        test    [ebx+usb_pipe.Flags], USB_FLAG_DISABLED
        jz      .nodisabled
        call    usb_pipe_disabled
        jmp     .continue
.nodisabled:
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

; Called from USB1 controller-specific initialization.
; Finds EHCI companion controller for given USB1 controller.
; in: bl = PCI device:function for USB1 controller, bh = PCI bus
; out: eax -> usb_controller for EHCI companion
proc usb_find_ehci_companion
; 1. Loop over all registered controllers.
        mov     eax, usb_controllers_list
.next:
        mov     eax, [eax+usb_controller.Next]
        cmp     eax, usb_controllers_list
        jz      .notfound
; 2. For every controller, check the type, ignore everything that is not EHCI.
        mov     edx, [eax+usb_controller.HardwareFunc]
        cmp     [edx+usb_hardware_func.ID], 'EHCI'
        jnz     .next
; 3. For EHCI controller, compare PCI coordinates with input data:
; bus and device must be the same, function can be different.
        mov     edx, [eax+usb_controller.PCICoordinates]
        xor     edx, ebx
        cmp     dx, 8
        jae     .next
        ret
.notfound:
        xor     eax, eax
        ret
endp

; Find Transaction Translator hub and port for the given device.
; in: edx = parent hub for the device, ecx = port for the device
; out: edx = TT hub for the device, ecx = TT port for the device.
proc usb_get_tt
; If the parent hub is high-speed, it is TT for the device.
; Otherwise, the parent hub itself is behind TT, and the device
; has the same TT hub+port as the parent hub.
        mov     eax, [edx+usb_hub.ConfigPipe]
        mov     eax, [eax+usb_pipe.DeviceData]
        cmp     [eax+usb_device_data.Speed], USB_SPEED_HS
        jz      @f
        movzx   ecx, [eax+usb_device_data.TTPort]
        mov     edx, [eax+usb_device_data.TTHub]
@@:
        mov     edx, [edx+usb_hub.ConfigPipe]
        ret
endp
