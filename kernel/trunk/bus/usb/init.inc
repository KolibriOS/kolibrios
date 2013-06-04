; Initialization of the USB subsystem.
; Provides usb_init procedure, includes all needed files.

; General notes:
; * There is one entry point for external kernel code: usb_init is called
;   from initialization code and initializes USB subsystem.
; * There are several entry points for API; see the docs for description.
; * There are several functions which are called from controller-specific
;   parts of USB subsystem. The most important is usb_new_device,
;   which is called when a new device has been connected (over some time),
;   has been reset and is ready to start configuring.
; * IRQ handlers are very restricted. They can not take any locks,
;   since otherwise a deadlock is possible: imagine that a code has taken the
;   lock and was interrupted by IRQ handler. Now IRQ handler would wait for
;   releasing the lock, and a lock owner would wait for exiting IRQ handler
;   to get the control.
; * Thus, there is the special USB thread which processes almost all activity.
;   IRQ handlers do the minimal processing and wake this thread.
; * Also the USB thread wakes occasionally to process tasks which can be
;   predicted without interrupts. These include e.g. a periodic roothub
;   scanning in UHCI and initializing in USB_CONNECT_DELAY ticks
;   after connecting a new device.
; * The main procedure of USB thread, usb_thread_proc, does all its work
;   by querying usb_hardware_func.ProcessDeferred for every controller
;   and usb_hub_process_deferred for every hub.
;   ProcessDeferred does controller-specific actions and calculates the time
;   when it should be invoked again, possibly infinite.
;   usb_thread_proc selects the minimum from all times returned by
;   ProcessDeferred and sleeps until this moment is reached or the thread
;   is awakened by IRQ handler.

; Initializes the USB subsystem.
proc usb_init
; 1. Initialize all locks.
        mov     ecx, usb_controllers_list_mutex
        call    mutex_init
        mov     ecx, usb1_ep_mutex
        call    mutex_init
        mov     ecx, usb_gtd_mutex
        call    mutex_init
        mov     ecx, ehci_ep_mutex
        call    mutex_init
        mov     ecx, ehci_gtd_mutex
        call    mutex_init
; 2. Kick off BIOS from all USB controllers, calling the corresponding function
; *hci_kickoff_bios. Also count USB controllers for the next step.
; Note: USB1 companion(s) must go before the corresponding EHCI controller,
; otherwise BIOS could see a device moving from EHCI to a companion;
; first, this always wastes time;
; second, some BIOSes are buggy, do not expect that move and try to refer to
; previously-assigned controller instead of actual; sometimes that leads to
; hangoff.
; Thus, process controllers in PCI order.
        mov     esi, pcidev_list
        push    0
.kickoff:
        mov     esi, [esi+PCIDEV.fd]
        cmp     esi, pcidev_list
        jz      .done_kickoff
        cmp     word [esi+PCIDEV.class+1], 0x0C03
        jnz     .kickoff
        mov     eax, uhci_kickoff_bios
        cmp     byte [esi+PCIDEV.class], 0x00
        jz      .do_kickoff
        mov     eax, ohci_kickoff_bios
        cmp     byte [esi+PCIDEV.class], 0x10
        jz      .do_kickoff
        mov     eax, ehci_kickoff_bios
        cmp     byte [esi+PCIDEV.class], 0x20
        jnz     .kickoff
.do_kickoff:
        inc     dword [esp]
        call    eax
        jmp     .kickoff
.done_kickoff:
        pop     eax
; 3. If no controllers were found, exit.
; Otherwise, run the USB thread.
        test    eax, eax
        jz      .nothing
        call    create_usb_thread
        jz      .nothing
; 4. Initialize all USB controllers, calling usb_init_controller for each.
; Note: USB1 companion(s) should go before the corresponding EHCI controller,
; although this is not strictly necessary (this way, a companion would not try
; to initialize high-speed device only to see a disconnect when EHCI takes
; control).
; Thus, process all EHCI controllers in the first loop, all USB1 controllers
; in the second loop. (One loop in reversed PCI order could also be used,
; but seems less natural.)
; 4a. Loop over all PCI devices, call usb_init_controller
; for all EHCI controllers.
        mov     eax, pcidev_list
.scan_ehci:
        mov     eax, [eax+PCIDEV.fd]
        cmp     eax, pcidev_list
        jz      .done_ehci
        cmp     [eax+PCIDEV.class], 0x0C0320
        jnz     .scan_ehci
        mov     edi, ehci_hardware_func
        call    usb_init_controller
        jmp     .scan_ehci
.done_ehci:
; 4b. Loop over all PCI devices, call usb_init_controller
; for all UHCI and OHCI controllers.
        mov     eax, pcidev_list
.scan_usb1:
        mov     eax, [eax+PCIDEV.fd]
        cmp     eax, pcidev_list
        jz      .done_usb1
        mov     edi, uhci_hardware_func
        cmp     [eax+PCIDEV.class], 0x0C0300
        jz      @f
        mov     edi, ohci_hardware_func
        cmp     [eax+PCIDEV.class], 0x0C0310
        jnz     .scan_usb1
@@:
        call    usb_init_controller
        jmp     .scan_usb1
.done_usb1:
.nothing:
        ret
endp

uglobal
align 4
usb_event       dd      ?
endg

; Helper function for usb_init. Creates and initializes the USB thread.
proc create_usb_thread
; 1. Create the thread.
        push    edi
        movi    ebx, 1
        mov     ecx, usb_thread_proc
        xor     edx, edx
        call    new_sys_threads
        pop     edi
; If failed, say something to the debug board and return with ZF set.
        test    eax, eax
        jns     @f
        DEBUGF 1,'K : cannot create kernel thread for USB, error %d\n',eax
.clear:
        xor     eax, eax
        jmp     .nothing
@@:
; 2. Wait while the USB thread initializes itself.
@@:
        call    change_task
        cmp     [usb_event], 0
        jz      @b
; 3. If initialization failed, the USB thread sets [usb_event] to -1.
; Return with ZF set or cleared corresponding to the result.
        cmp     [usb_event], -1
        jz      .clear
.nothing:
        ret
endp

; Helper function for IRQ handlers. Wakes the USB thread if ebx is nonzero.
proc usb_wakeup_if_needed
        test    ebx, ebx
        jz      usb_wakeup.nothing
usb_wakeup:
        xor     edx, edx
        mov     eax, [usb_event]
        mov     ebx, [eax+EVENT.id]
        xor     esi, esi
        call    raise_event
.nothing:
        ret
endp

; Main loop of the USB thread.
proc usb_thread_proc
; 1. Initialize: create event to allow wakeup by interrupt handlers.
        xor     esi, esi
        mov     ecx, MANUAL_DESTROY
        call    create_event
        test    eax, eax
        jnz     @f
; If failed, set [usb_event] to -1 and terminate myself.
        dbgstr 'cannot create event for USB thread'
        or      [usb_event], -1
        jmp     sys_end
@@:
        mov     [usb_event], eax
        push    -1              ; initial timeout: infinite
usb_thread_wait:
; 2. Main loop: wait for either wakeup event or timeout.
        pop     ecx             ; get timeout
        mov     eax, [usb_event]
        mov     ebx, [eax+EVENT.id]
        call    wait_event_timeout
        push    -1              ; default timeout: infinite
; 3. Main loop: call worker functions of all controllers;
; if some function schedules wakeup in timeout less than the current value,
; replace that value with the returned timeout.
        mov     esi, usb_controllers_list
@@:
        mov     esi, [esi+usb_controller.Next]
        cmp     esi, usb_controllers_list
        jz      .controllers_done
        mov     eax, [esi+usb_controller.HardwareFunc]
        call    [eax+usb_hardware_func.ProcessDeferred]
        cmp     [esp], eax
        jb      @b
        mov     [esp], eax
        jmp     @b
.controllers_done:
; 4. Main loop: call hub worker function for all hubs,
; similarly calculating minimum of all returned timeouts.
; When done, continue to 2.
        mov     esi, usb_hubs_list
@@:
        mov     esi, [esi+usb_hub.Next]
        cmp     esi, usb_hubs_list
        jz      usb_thread_wait
        call    usb_hub_process_deferred
        cmp     [esp], eax
        jb      @b
        mov     [esp], eax
        jmp     @b
endp

iglobal
align 4
usb_controllers_list:
        dd      usb_controllers_list
        dd      usb_controllers_list
usb_hubs_list:
        dd      usb_hubs_list
        dd      usb_hubs_list
endg
uglobal
align 4
usb_controllers_list_mutex      MUTEX
endg

include "memory.inc"
include "hccommon.inc"
include "pipe.inc"
include "ohci.inc"
include "uhci.inc"
include "ehci.inc"
include "protocol.inc"
include "hub.inc"
include "scheduler.inc"
