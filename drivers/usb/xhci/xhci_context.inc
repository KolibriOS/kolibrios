; Device slots, contexts and pipes for the xHCI driver.
; This file is included from xhci.asm.

; =============================================================================
; ============================ Context helpers ================================
; =============================================================================

; Returns the address of one context inside a context structure.
; in: eax = base of the structure, ecx = index of the context,
;     esi -> usb_controller.
; out: eax = address of the context.
proc xhci_context
        push    ecx edx
        mov     edx, [esi+xhci_controller.ContextSize-XCD]
        imul    edx, ecx
        add     eax, edx
        pop     edx ecx
        ret
endp

; Zeroes the Input Context. The caller must hold CmdLock.
; in: esi -> usb_controller.
proc xhci_clear_input_ctx
        push    eax ecx edi
        mov     edi, [esi+xhci_controller.InputCtx-XCD]
        xor     eax, eax
        mov     ecx, 33*64/4
        rep stosd
        pop     edi ecx eax
        ret
endp

; Returns the pipe of the default control endpoint of the same device, which
; represents the device as a whole.
; in: ebx -> xhci_pipe. out: eax -> xhci_pipe or 0.
proc xhci_ctrl_pipe
        mov     eax, [ebx+xhci_pipe.DevPage]
        test    eax, eax
        jz      .nothing
        mov     eax, [eax+XHCI_PIPES_OFS+1*4]
        test    eax, eax
        jz      .nothing
        sub     eax, sizeof.xhci_pipe
.nothing:
        ret
endp

; Fills the Slot Context of the Input Context from the device description kept
; in a pipe. The caller must hold CmdLock.
; in: esi -> usb_controller, ebx -> xhci_pipe describing the device,
;     ecx = value for the Context Entries field.
proc xhci_fill_slot_ctx
        push    eax ecx edx edi
        mov     eax, [esi+xhci_controller.InputCtx-XCD]
        push    ecx
        movi    ecx, 1
        call    xhci_context
        pop     ecx
        mov     edi, eax
; dword 0: Route String, Speed and Context Entries.
        mov     eax, [ebx+xhci_pipe.Route]
        and     eax, 0FFFFFh
if XHCI_SUPERSPEED
; A SuperSpeed root-port device carries the raw PORTSC speed identifier;
; its kernel-visible Speed pretends high-speed and must not get here.
        mov     edx, [ebx+xhci_pipe.PSIV]
        test    edx, edx
        jnz     .havespeed
end if
        mov     edx, [ebx+xhci_pipe.Speed]
        cmp     edx, USB_SPEED_FS
        jz      .fs
        cmp     edx, USB_SPEED_LS
        jz      .ls
        movi    edx, XHCI_PSI_HS
        jmp     .havespeed
.fs:
        movi    edx, XHCI_PSI_FS
        jmp     .havespeed
.ls:
        movi    edx, XHCI_PSI_LS
.havespeed:
        shl     edx, 20
        or      eax, edx
        shl     ecx, 27
        or      eax, ecx
        mov     [edi], eax
; dword 1: Root Hub Port Number.
        mov     eax, [ebx+xhci_pipe.RootPort]
        shl     eax, 16
        mov     [edi+4], eax
; dword 2: the Transaction Translator, if the device is served by one.
        mov     eax, [ebx+xhci_pipe.TTSlot]
        test    eax, eax
        jz      @f
        mov     edx, [ebx+xhci_pipe.TTPort]
        shl     edx, 8
        or      eax, edx
@@:
        mov     [edi+8], eax
        and     dword [edi+12], 0
        pop     edi edx ecx eax
        ret
endp

; Fills the Endpoint Context of the Input Context for the given pipe.
; The caller must hold CmdLock.
; in: esi -> usb_controller, ebx -> xhci_pipe.
proc xhci_fill_ep_ctx
        push    eax ecx edx edi
        mov     eax, [esi+xhci_controller.InputCtx-XCD]
        mov     ecx, [ebx+xhci_pipe.DCI]
        inc     ecx
        call    xhci_context
        mov     edi, eax
; dword 0: Interval.
        mov     eax, [ebx+xhci_pipe.Interval]
        shl     eax, 16
        mov     [edi], eax
; dword 1: Error Count, Endpoint Type, Max Burst Size and Max Packet Size.
        mov     eax, [ebx+xhci_pipe.EpType]
        shl     eax, 3
        or      eax, 3 shl 1            ; three retries before giving up
        mov     edx, [ebx+xhci_pipe.MaxPacket]
        mov     ecx, edx
        shr     ecx, 11
        and     ecx, 3                  ; extra transactions per microframe
        shl     ecx, 8
        or      eax, ecx
if XHCI_SUPERSPEED
; A SuperSpeed bulk endpoint may allow bursts of several packets; the field
; shares its bits with the extra-transactions ones, which are zero then.
        mov     ecx, [ebx+xhci_pipe.MaxBurst]
        shl     ecx, 8
        or      eax, ecx
end if
        and     edx, 7FFh
        shl     edx, 16
        or      eax, edx
        mov     [edi+4], eax
; dwords 2 and 3: the TR Dequeue Pointer with the Dequeue Cycle State.
        mov     eax, [ebx+xhci_pipe.Enqueue]
        shl     eax, 4
        add     eax, [ebx+xhci_pipe.RingPhys]
        or      eax, [ebx+xhci_pipe.Cycle]
        mov     [edi+8], eax
        and     dword [edi+12], 0
; dword 4: Average TRB Length, only a hint for the bandwidth scheduler.
        mov     eax, [ebx+xhci_pipe.MaxPacket]
        and     eax, 7FFh
        cmp     [ebx+xhci_pipe.EpType], XHCI_EP_CONTROL
        jnz     @f
        movi    eax, 8
@@:
        mov     [edi+16], eax
        and     dword [edi+20], 0
        pop     edi edx ecx eax
        ret
endp

; =============================================================================
; ============================== Slot commands ================================
; =============================================================================

; Issues the Address Device command for a newly created default control pipe.
; in: esi -> usb_controller, ebx -> xhci_pipe. out: eax = 0 on failure.
proc xhci_address_device
        push    ebx ecx edx edi
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexLock
        call    xhci_clear_input_ctx
        mov     edx, [esi+xhci_controller.InputCtx-XCD]
        mov     dword [edx+4], 3        ; add the Slot Context and endpoint zero
        movi    ecx, 1
        call    xhci_fill_slot_ctx
        call    xhci_fill_ep_ctx
        mov     [ebx+xhci_pipe.MaxDCI], 1
; Point the DCBAA entry of the slot at the Device Context.
        mov     eax, [esi+xhci_controller.CommonPage-XCD]
        mov     ecx, [ebx+xhci_pipe.SlotId]
        mov     edx, [ebx+xhci_pipe.DevPagePhys]
        mov     [eax+ecx*8], edx
        and     dword [eax+ecx*8+4], 0
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, XHCI_TRB_ADDRESS_DEV shl 10
        mov     eax, [esi+xhci_controller.InputCtxPhys-XCD]
        push    ebx
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_submit
        pop     ebx
        push    eax
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexUnlock
        pop     eax
        cmp     eax, XHCI_CC_SUCCESS
        jz      .ok
        DEBUGF 1,'K : XHCI: Address Device failed, code %d\n',eax
        xor     eax, eax
        jmp     .nothing
.ok:
        movi    eax, 1
.nothing:
        pop     edi edx ecx ebx
        ret
endp

; Issues a Configure Endpoint command that adds the given endpoint.
; in: esi -> usb_controller, ebx -> xhci_pipe of the endpoint,
;     edi -> xhci_pipe of the default control endpoint of the same device.
; out: eax = completion code.
proc xhci_configure_endpoint
        push    ebx ecx edx esi edi
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexLock
        call    xhci_clear_input_ctx
; Drop the endpoint and add it again, so that reopening an endpoint that has
; been closed before also works.
        mov     ecx, [ebx+xhci_pipe.DCI]
        movi    eax, 1
        shl     eax, cl
        mov     edx, [esi+xhci_controller.InputCtx-XCD]
        mov     [edx], eax
        or      eax, 1                  ; the Slot Context is always added
        mov     [edx+4], eax
        call    xhci_fill_ep_ctx
; The Context Entries field must cover every endpoint in use by the device.
        mov     ecx, [ebx+xhci_pipe.DCI]
        cmp     ecx, [edi+xhci_pipe.MaxDCI]
        jae     @f
        mov     ecx, [edi+xhci_pipe.MaxDCI]
@@:
        mov     [edi+xhci_pipe.MaxDCI], ecx
        push    ebx
        mov     ebx, edi
        call    xhci_fill_slot_ctx
        pop     ebx
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, XHCI_TRB_CONFIG_EP shl 10
        mov     eax, [esi+xhci_controller.InputCtxPhys-XCD]
        push    ebx
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_submit
        pop     ebx
        push    eax
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexUnlock
        pop     eax
        pop     edi esi edx ecx ebx
        ret
endp

; Tells the controller where the endpoint has to continue from.
; in: esi -> usb_controller, ebx -> xhci_pipe.
proc xhci_set_dequeue
        pushad
        cmp     [ebx+xhci_pipe.SlotId], 0
        jz      .nothing
        mov     eax, [ebx+xhci_pipe.Dequeue]
        shl     eax, 4
        add     eax, [ebx+xhci_pipe.RingPhys]
        or      eax, [ebx+xhci_pipe.Cycle]
        mov     ecx, [ebx+xhci_pipe.DCI]
        shl     ecx, 16
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, ecx
        or      edx, XHCI_TRB_SET_DEQ shl 10
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_sync
        cmp     eax, XHCI_CC_SUCCESS
        jz      .nothing
        DEBUGF 1,'K : XHCI: Set TR Dequeue Pointer failed, code %d\n',eax
.nothing:
        popad
        ret
endp

; =============================================================================
; =============================== New device ==================================
; =============================================================================

; Called from xhci_port_init and from the hub support code when a new device
; has been connected and reset. Enables a device slot for it and hands it over
; to the protocol layer.
; in: esi -> usb_controller, eax = speed. out: eax = 0 on failure.
proc xhci_new_device
        push    ebx ecx edx edi
        mov     [esi+usb_controller.ResettingSpeed], al
; 1. Reserve room for the pseudo-pipe that describes the new device. Only the
; fields read by xhci_init_pipe are filled in; the layout matches a real pipe,
; so the result can be passed to usb_new_device as a usb_pipe pointer.
        sub     esp, sizeof.xhci_pipe + 4
        mov     ebx, esp
        push    ebx
        mov     edi, ebx
        xor     eax, eax
        mov     ecx, (sizeof.xhci_pipe + 4)/4
        rep stosd
        pop     ebx
        mov     [ebx+sizeof.xhci_pipe], esi     ; usb_pipe.Controller
        movzx   eax, [esi+usb_controller.ResettingSpeed]
        mov     [ebx+xhci_pipe.Speed], eax
; 2. Compute the root hub port, the route string and the tier of the device.
        movzx   edx, [esi+usb_controller.ResettingPort]
        mov     edi, [esi+usb_controller.ResettingHub]
        test    edi, edi
        jnz     .behind_hub
        mov     eax, [esi+xhci_controller.PortMap-XCD+edx*4]
        mov     [ebx+xhci_pipe.RootPort], eax
if XHCI_SUPERSPEED
        mov     eax, [esi+xhci_controller.ResettingPSIV-XCD]
        mov     [ebx+xhci_pipe.PSIV], eax
end if
        jmp     .have_topology
.behind_hub:
; The parent hub is a device too, and its own pipe carries the topology data.
        mov     eax, [edi+USB_HUB_CONFIGPIPE]
        sub     eax, sizeof.xhci_pipe
        mov     ecx, [eax+xhci_pipe.RootPort]
        mov     [ebx+xhci_pipe.RootPort], ecx
        mov     ecx, [eax+xhci_pipe.Depth]
        mov     eax, [eax+xhci_pipe.Route]
; The route string holds one nibble per tier, at most five of them.
        cmp     ecx, 5
        jae     .have_route
        push    ecx edx
        shl     ecx, 2
        inc     edx                     ; the route uses 1-based port numbers
        cmp     edx, 15
        jbe     @f
        movi    edx, 15
@@:
        shl     edx, cl
        or      eax, edx
        pop     edx ecx
.have_route:
        inc     ecx
        mov     [ebx+xhci_pipe.Depth], ecx
        mov     [ebx+xhci_pipe.Route], eax
; 3. Low- and full-speed devices behind a high-speed hub need the address of
; the Transaction Translator that serves them.
        cmp     [ebx+xhci_pipe.Speed], USB_SPEED_HS
        jz      .have_topology
        push    ebx
        mov     ecx, edx
        mov     edx, edi
        invoke  usbhc_api.usb_get_tt
        pop     ebx
        test    edx, edx
        jz      .have_topology
        sub     edx, sizeof.xhci_pipe
        mov     eax, [edx+xhci_pipe.SlotId]
        mov     [ebx+xhci_pipe.TTSlot], eax
        inc     ecx
        mov     [ebx+xhci_pipe.TTPort], ecx
.have_topology:
; 4. Ask the controller for a device slot.
        xor     eax, eax
        push    ebx
        xor     ebx, ebx
        xor     ecx, ecx
        mov     edx, XHCI_TRB_ENABLE_SLOT shl 10
        call    xhci_cmd_sync
        pop     ebx
        cmp     eax, XHCI_CC_SUCCESS
        jnz     .fail
        test    edx, edx
        jz      .fail
        mov     [ebx+xhci_pipe.SlotId], edx
        DEBUGF 1,'K : XHCI new device: slot %d, root port %d, route %x, speed %d\n',edx,[ebx+xhci_pipe.RootPort],[ebx+xhci_pipe.Route],[ebx+xhci_pipe.Speed]
; 5. Hand the device over to the protocol layer. It opens the pipe of the
; default control endpoint, and that is where the slot is really addressed.
        lea     ecx, [ebx+sizeof.xhci_pipe]
        invoke  usbhc_api.usb_new_device
        test    eax, eax
        jnz     .done
; 6. The protocol layer has failed; release the slot again.
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, XHCI_TRB_DISABLE_SLOT shl 10
        xor     eax, eax
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_sync
        xor     eax, eax
        jmp     .done
.fail:
        dbgstr 'XHCI: Enable Slot failed'
        xor     eax, eax
.done:
        add     esp, sizeof.xhci_pipe + 4
        pop     edi edx ecx ebx
        ret
endp

; =============================================================================
; ================================= Pipes =====================================
; =============================================================================

proc xhci_alloc_pipe
        push    ebx ecx edi
        mov     ebx, xhci_ep_mutex
        invoke  usbhc_api.usb_allocate_common, sizeof.xhci_pipe + sizeof.usb_pipe
        test    eax, eax
        jz      @f
; Zero the whole block: the kernel may call FreePipe for a pipe whose InitPipe
; has never run, and that path must not act on garbage.
        mov     edi, eax
        push    eax
        xor     eax, eax
        movi    ecx, (sizeof.xhci_pipe + sizeof.usb_pipe + 3)/4
        rep stosd
        pop     eax
        add     eax, sizeof.xhci_pipe
@@:
        pop     edi ecx ebx
        ret
endp

proc xhci_free_pipe stdcall, ptr:dword
        push    ebx esi edi
        mov     ebx, [ptr]
        sub     ebx, sizeof.xhci_pipe
        mov     esi, [ebx+sizeof.xhci_pipe+usb_pipe.Controller]
        test    esi, esi
        jz      .free
        cmp     [ebx+xhci_pipe.SlotId], 0
        jz      .drop_pipe
        cmp     [ebx+xhci_pipe.DCI], 1
        jnz     .drop_pipe
; This is the default control endpoint, that is, the handle of the whole
; device: release the slot together with the per-device page.
        mov     ecx, [ebx+xhci_pipe.SlotId]
        mov     edx, ecx
        shl     edx, 24
        or      edx, XHCI_TRB_DISABLE_SLOT shl 10
        xor     eax, eax
        push    ebx
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_sync
        pop     ebx
; The controller may access the Device Context until the command above
; completes, so only now may the DCBAA entry and the slot table forget it.
        mov     ecx, [ebx+xhci_pipe.SlotId]
        mov     edi, [esi+xhci_controller.CommonPage-XCD]
        and     dword [edi+ecx*8], 0
        and     dword [edi+ecx*8+4], 0
        and     [esi+xhci_controller.SlotPages-XCD+ecx*4], 0
        mov     eax, [ebx+xhci_pipe.DevPage]
        test    eax, eax
        jz      .drop_ring
        invoke  KernelFree, eax
        and     [ebx+xhci_pipe.DevPage], 0
        jmp     .drop_ring
.drop_pipe:
; Remove the pipe from the per-device table, if it is still listed there.
; On disconnect the kernel releases the pipes of a device in arbitrary order,
; and the pipe of the default control endpoint - the owner of the per-device
; page - may well be freed before this one, taking the page with it. The
; authoritative pointer is the one in SlotPages: it is cleared when the slot
; is released, so it can never point to freed memory.
        mov     ecx, [ebx+xhci_pipe.SlotId]
        test    ecx, ecx
        jz      .drop_ring
        mov     eax, [esi+xhci_controller.SlotPages-XCD+ecx*4]
        test    eax, eax
        jz      .drop_ring
        mov     ecx, [ebx+xhci_pipe.DCI]
        cmp     ecx, 32
        jae     .drop_ring
        and     dword [eax+XHCI_PIPES_OFS+ecx*4], 0
.drop_ring:
        mov     eax, [ebx+xhci_pipe.Ring]
        test    eax, eax
        jz      @f
        invoke  KernelFree, eax
        and     [ebx+xhci_pipe.Ring], 0
@@:
        mov     eax, [ebx+xhci_pipe.RingMap]
        test    eax, eax
        jz      .free
        invoke  KernelFree, eax
        and     [ebx+xhci_pipe.RingMap], 0
.free:
        pop     edi esi ebx
        mov     eax, [ptr]
        sub     eax, sizeof.xhci_pipe
        invoke  usbhc_api.usb_free_common, eax
        ret
endp

; Allocates the transfer ring of a pipe.
; in: ebx -> xhci_pipe. out: eax = 0 on failure.
proc xhci_alloc_ring
        push    ecx edx
        call    xhci_alloc_page
        test    eax, eax
        jz      .nothing
        mov     [ebx+xhci_pipe.RingMap], eax
        call    xhci_alloc_page
        test    eax, eax
        jnz     @f
        invoke  KernelFree, [ebx+xhci_pipe.RingMap]
        and     [ebx+xhci_pipe.RingMap], 0
        xor     eax, eax
        jmp     .nothing
@@:
        mov     [ebx+xhci_pipe.Ring], eax
        mov     [ebx+xhci_pipe.RingPhys], edx
; The last entry returns to the beginning of the ring and toggles the cycle.
        lea     ecx, [eax+(XHCI_RING_TRBS-1)*16]
        mov     [ecx], edx
        mov     dword [ecx+12], (XHCI_TRB_LINK shl 10) + XHCI_TRB_TC
        and     [ebx+xhci_pipe.Enqueue], 0
        and     [ebx+xhci_pipe.Dequeue], 0
        and     [ebx+xhci_pipe.Reserved], 0
        and     [ebx+xhci_pipe.FirstFix], 0
        mov     [ebx+xhci_pipe.Cycle], XHCI_TRB_C
        movi    eax, 1
.nothing:
        pop     edx ecx
        ret
endp

; Stores the pipe in the per-device table used by the event ring handler.
; in: ebx -> xhci_pipe.
proc xhci_register_pipe
        push    eax ecx edx
        mov     eax, [ebx+xhci_pipe.DevPage]
        test    eax, eax
        jz      @f
        mov     ecx, [ebx+xhci_pipe.DCI]
        cmp     ecx, 32
        jae     @f
        lea     edx, [ebx+sizeof.xhci_pipe]
        mov     [eax+XHCI_PIPES_OFS+ecx*4], edx
@@:
        pop     edx ecx eax
        ret
endp

; Computes the Interval field of the Endpoint Context of an interrupt endpoint.
; The kernel passes bInterval from the endpoint descriptor: for high-speed
; devices it is an exponent of 125 us periods, for low- and full-speed devices
; it is a number of frames.
; in: ebx -> xhci_pipe, eax = bInterval.
proc xhci_calc_interval
        push    eax ecx
        cmp     [ebx+xhci_pipe.Speed], USB_SPEED_HS
        jnz     .low_speed
        test    eax, eax
        jz      @f
        dec     eax
@@:
        jmp     .clamp
.low_speed:
; One frame equals eight periods of 125 us, hence the offset of three.
        test    eax, eax
        jnz     @f
        movi    eax, 1
@@:
        bsr     ecx, eax
        lea     eax, [ecx+3]
.clamp:
        cmp     eax, 15
        jbe     @f
        movi    eax, 15
@@:
        mov     [ebx+xhci_pipe.Interval], eax
        pop     ecx eax
        ret
endp

; Called from usb_open_pipe.
; in: edi -> usb_pipe for the target, ecx -> usb_pipe of the config pipe,
;     esi -> usb_controller, eax -> usb_gtd of the first descriptor,
;     [ebp+12] = endpoint, [ebp+16] = maxpacket, [ebp+20] = type,
;     [ebp+24] = interval.
; out: eax = 0 on failure.
proc xhci_init_pipe
virtual at ebp+8
.config_pipe    dd      ?
.endpoint       dd      ?
.maxpacket      dd      ?
.type           dd      ?
.interval       dd      ?
end virtual
        push    ebx
        sub     edi, sizeof.xhci_pipe
        sub     ecx, sizeof.xhci_pipe
        mov     ebx, edi                ; ebx -> xhci_pipe being initialized
; 1. Copy the description of the device from the config pipe.
        mov     eax, [ecx+xhci_pipe.SlotId]
        mov     [ebx+xhci_pipe.SlotId], eax
        mov     eax, [ecx+xhci_pipe.DevPage]
        mov     [ebx+xhci_pipe.DevPage], eax
        mov     eax, [ecx+xhci_pipe.DevPagePhys]
        mov     [ebx+xhci_pipe.DevPagePhys], eax
        mov     eax, [ecx+xhci_pipe.Speed]
        mov     [ebx+xhci_pipe.Speed], eax
        mov     eax, [ecx+xhci_pipe.RootPort]
        mov     [ebx+xhci_pipe.RootPort], eax
        mov     eax, [ecx+xhci_pipe.Route]
        mov     [ebx+xhci_pipe.Route], eax
        mov     eax, [ecx+xhci_pipe.Depth]
        mov     [ebx+xhci_pipe.Depth], eax
        mov     eax, [ecx+xhci_pipe.TTSlot]
        mov     [ebx+xhci_pipe.TTSlot], eax
        mov     eax, [ecx+xhci_pipe.TTPort]
        mov     [ebx+xhci_pipe.TTPort], eax
if XHCI_SUPERSPEED
        mov     eax, [ecx+xhci_pipe.PSIV]
        mov     [ebx+xhci_pipe.PSIV], eax
end if
; 2. Work out the Device Context Index and the endpoint type.
        mov     eax, [.endpoint]
        test    eax, eax
        jz      .control_ep
        mov     ecx, eax
        and     ecx, 15
        add     ecx, ecx
        test    al, 80h
        jz      @f
        inc     ecx
        or      [ebx+xhci_pipe.Flags], XHCI_PIPE_IN
@@:
        mov     [ebx+xhci_pipe.DCI], ecx
        mov     eax, [.type]
        cmp     eax, BULK_PIPE
        jz      .bulk
        cmp     eax, INTERRUPT_PIPE
        jz      .interrupt
; A control endpoint other than endpoint zero is unusual but legal.
        mov     [ebx+xhci_pipe.EpType], XHCI_EP_CONTROL
        jmp     .have_type
.bulk:
        movi    eax, XHCI_EP_BULK_OUT
        test    [ebx+xhci_pipe.Flags], XHCI_PIPE_IN
        jz      @f
        movi    eax, XHCI_EP_BULK_IN
@@:
        mov     [ebx+xhci_pipe.EpType], eax
        jmp     .have_type
.interrupt:
        movi    eax, XHCI_EP_INT_OUT
        test    [ebx+xhci_pipe.Flags], XHCI_PIPE_IN
        jz      @f
        movi    eax, XHCI_EP_INT_IN
@@:
        mov     [ebx+xhci_pipe.EpType], eax
        mov     eax, [.interval]
        call    xhci_calc_interval
.have_type:
        mov     eax, [.maxpacket]
        mov     [ebx+xhci_pipe.MaxPacket], eax
if XHCI_SUPERSPEED
        call    xhci_ss_get_burst
end if
        jmp     .alloc_ring
.control_ep:
; The default control endpoint always has index one. Until the real packet size
; is known from the device descriptor, use the value the specification requires
; for the speed of the device: the kernel asks for 64 regardless of the speed,
; which is wrong for low- and full-speed devices.
        mov     [ebx+xhci_pipe.DCI], 1
        mov     [ebx+xhci_pipe.EpType], XHCI_EP_CONTROL
        movi    eax, 64
        cmp     [ebx+xhci_pipe.Speed], USB_SPEED_HS
        jz      @f
        movi    eax, 8
@@:
if XHCI_SUPERSPEED
; A SuperSpeed device always uses 512 bytes on the default control endpoint.
        cmp     [ebx+xhci_pipe.PSIV], XHCI_PSI_SS
        jb      @f
        mov     eax, 512
@@:
end if
        mov     [ebx+xhci_pipe.MaxPacket], eax
.alloc_ring:
; 3. Allocate the transfer ring.
        call    xhci_alloc_ring
        test    eax, eax
        jz      .fail
; 4. Choose the software list the pipe belongs to and insert it there. The
; hardware knows nothing about these lists; they exist only so that the kernel
; can walk all pipes and reinsert an aborted one.
        lea     edx, [esi+xhci_controller.ControlED-XCD]
        cmp     [.type], BULK_PIPE
        jb      @f
        lea     edx, [esi+xhci_controller.BulkED-XCD]
        jz      @f
        lea     edx, [esi+xhci_controller.IntED-XCD]
@@:
        lea     edi, [ebx+sizeof.xhci_pipe]
        mov     [edi+usb_pipe.BaseList], edx
        mov     ecx, [edx+usb_pipe.NextVirt]
        mov     [edi+usb_pipe.NextVirt], ecx
        mov     [edi+usb_pipe.PrevVirt], edx
        mov     [ecx+usb_pipe.PrevVirt], edi
        mov     [edx+usb_pipe.NextVirt], edi
; 5. Now do the hardware part.
        cmp     [.endpoint], 0
        jz      .address_device
        call    xhci_register_pipe
        mov     edi, [.config_pipe]
        sub     edi, sizeof.xhci_pipe
        call    xhci_configure_endpoint
        xhci_trace 'K : XHCI open pipe: slot %d dci %d\n', [ebx+xhci_pipe.SlotId], [ebx+xhci_pipe.DCI]
        cmp     eax, XHCI_CC_SUCCESS
        jz      .success
        DEBUGF 1,'K : XHCI: Configure Endpoint failed, code %d\n',eax
        jmp     .unlink_fail
.address_device:
; 6. This is a new device: allocate its Device Context, publish the slot and
; let the controller give the device a bus address.
        call    xhci_alloc_page
        test    eax, eax
        jz      .unlink_fail
        mov     [ebx+xhci_pipe.DevPage], eax
        mov     [ebx+xhci_pipe.DevPagePhys], edx
        mov     ecx, [ebx+xhci_pipe.SlotId]
        mov     [esi+xhci_controller.SlotPages-XCD+ecx*4], eax
        call    xhci_register_pipe
        call    xhci_address_device
        test    eax, eax
        jz      .free_devpage
.success:
; usb_open_pipe keeps the pointer to the new pipe in edi across this call, so
; it has to be handed back untouched; the branch above has overwritten it with
; the pipe of the default control endpoint.
        lea     edi, [ebx+sizeof.xhci_pipe]
        pop     ebx
        movi    eax, 1
        ret
.free_devpage:
        mov     ecx, [ebx+xhci_pipe.SlotId]
        and     [esi+xhci_controller.SlotPages-XCD+ecx*4], 0
        mov     eax, [ebx+xhci_pipe.DevPage]
        invoke  KernelFree, eax
        and     [ebx+xhci_pipe.DevPage], 0
.unlink_fail:
; Undo the insertion into the software list before failing.
        lea     edi, [ebx+sizeof.xhci_pipe]
        mov     eax, [edi+usb_pipe.NextVirt]
        mov     edx, [edi+usb_pipe.PrevVirt]
        mov     [eax+usb_pipe.PrevVirt], edx
        mov     [edx+usb_pipe.NextVirt], eax
.fail:
        lea     edi, [ebx+sizeof.xhci_pipe]
        pop     ebx
        xor     eax, eax
        ret
endp

; Removes the pipe from the hardware configuration of the device.
; in: esi -> usb_controller, ebx -> usb_pipe.
proc xhci_unlink_pipe
        pushad
        sub     ebx, sizeof.xhci_pipe
        cmp     [ebx+xhci_pipe.SlotId], 0
        jz      .nothing
        cmp     [ebx+xhci_pipe.DCI], 1
        jz      .nothing        ; the slot itself is released by FreePipe
        call    xhci_ctrl_pipe
        test    eax, eax
        jz      .nothing
        mov     edi, eax                ; edi -> pipe of endpoint zero
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexLock
        call    xhci_clear_input_ctx
        mov     ecx, [ebx+xhci_pipe.DCI]
        movi    eax, 1
        shl     eax, cl
        mov     edx, [esi+xhci_controller.InputCtx-XCD]
        mov     [edx], eax              ; drop this endpoint
        mov     dword [edx+4], 1        ; keep the Slot Context
        mov     ecx, [edi+xhci_pipe.MaxDCI]
        test    ecx, ecx
        jnz     @f
        movi    ecx, 1
@@:
        push    ebx
        mov     ebx, edi
        call    xhci_fill_slot_ctx
        pop     ebx
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, XHCI_TRB_CONFIG_EP shl 10
        mov     eax, [esi+xhci_controller.InputCtxPhys-XCD]
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_submit
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexUnlock
.nothing:
        popad
        ret
endp

; Temporarily removes the pipe from the hardware schedule.
; in: esi -> usb_controller, ebx -> usb_pipe.
proc xhci_disable_pipe
        pushad
        sub     ebx, sizeof.xhci_pipe
        cmp     [ebx+xhci_pipe.SlotId], 0
        jz      .nothing
        mov     ecx, [ebx+xhci_pipe.DCI]
        shl     ecx, 16
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, ecx
        or      edx, XHCI_TRB_STOP_EP shl 10
        xor     eax, eax
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_sync
.nothing:
        popad
        ret
endp

; Reinserts the pipe into the hardware schedule after xhci_disable_pipe, with
; an empty transfer queue.
; in: esi -> usb_controller, ebx -> usb_pipe,
;     edx -> current descriptor, eax -> new last descriptor.
proc xhci_enable_pipe
        pushad
        sub     ebx, sizeof.xhci_pipe
        mov     eax, [ebx+xhci_pipe.Ring]
        test    eax, eax
        jz      .nothing
; The queue is empty now, so the ring can be restarted from its beginning.
        mov     edi, eax
        xor     eax, eax
        mov     ecx, 0x1000/4
        rep stosd
        mov     edi, [ebx+xhci_pipe.RingMap]
        mov     ecx, 0x1000/4
        rep stosd
        mov     edi, [ebx+xhci_pipe.Ring]
        add     edi, (XHCI_RING_TRBS-1)*16
        mov     eax, [ebx+xhci_pipe.RingPhys]
        mov     [edi], eax
        mov     dword [edi+12], (XHCI_TRB_LINK shl 10) + XHCI_TRB_TC
        and     [ebx+xhci_pipe.Enqueue], 0
        and     [ebx+xhci_pipe.Dequeue], 0
        and     [ebx+xhci_pipe.Reserved], 0
        and     [ebx+xhci_pipe.FirstFix], 0
        mov     [ebx+xhci_pipe.Cycle], XHCI_TRB_C
        call    xhci_set_dequeue
.nothing:
        popad
        ret
endp

; =============================================================================
; ========================== Address and packet size ==========================
; =============================================================================

; Called from usb_set_address_callback. The bus address is chosen by the
; controller itself while executing the Address Device command, so the only
; thing left to do is to remember the number the kernel has reserved: it asks
; for it back on disconnect in order to mark it free again.
; in: esi -> usb_controller, ebx -> usb_pipe, cl = address.
proc xhci_set_device_address
        movzx   eax, cl
        mov     [ebx+xhci_pipe.Address-sizeof.xhci_pipe], eax
        jmp     [usbhc_api.usb_subscribe_control]
endp

; in: esi -> usb_controller, ebx -> usb_pipe. out: eax = address.
proc xhci_get_device_address
        mov     eax, [ebx+xhci_pipe.Address-sizeof.xhci_pipe]
        ret
endp

; Called from usb_get_descr8_callback once the real packet size of the default
; control endpoint is known. On xHCI that value lives in the Endpoint Context,
; so an Evaluate Context command is needed to update it.
; in: esi -> usb_controller, ebx -> usb_pipe, ecx = packet size.
proc xhci_set_endpoint_packet_size
        push    ebx ecx edx edi
        sub     ebx, sizeof.xhci_pipe
if XHCI_SUPERSPEED
; For a SuperSpeed device bMaxPacketSize0 is an exponent, and the only value
; the specification allows is 9: use 512 directly instead of the raw byte.
        cmp     [ebx+xhci_pipe.PSIV], XHCI_PSI_SS
        jb      @f
        mov     ecx, 512
@@:
end if
        mov     [ebx+xhci_pipe.MaxPacket], ecx
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexLock
        call    xhci_clear_input_ctx
        mov     edx, [esi+xhci_controller.InputCtx-XCD]
        mov     dword [edx+4], 2        ; evaluate the default control endpoint
        call    xhci_fill_ep_ctx
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, XHCI_TRB_EVAL_CTX shl 10
        mov     eax, [esi+xhci_controller.InputCtxPhys-XCD]
        push    ebx
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_submit
        pop     ebx
        cmp     eax, XHCI_CC_SUCCESS
        jz      @f
        DEBUGF 1,'K : XHCI: Evaluate Context failed, code %d\n',eax
@@:
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexUnlock
        pop     edi edx ecx ebx
        jmp     [usbhc_api.usb_subscribe_control]
endp

; If the descriptor that has just completed is the answer to a request for the
; hub descriptor, tell the controller that the device is a hub: it has to know
; that in order to serve the devices behind it.
; in: ebx -> usb_gtd.
proc xhci_check_hub_descriptor
        pushad
        cmp     [ebx+usb_gtd.Callback], 0
        jz      .nothing
        mov     eax, [ebx+usb_gtd.Pipe]
        mov     esi, [eax+usb_pipe.Controller]
        sub     eax, sizeof.xhci_pipe
        test    [eax+xhci_pipe.Flags], XHCI_PIPE_HUBPEND
        jz      .nothing
        and     [eax+xhci_pipe.Flags], not XHCI_PIPE_HUBPEND
        cmp     [ebx+xhci_gtd.Status-sizeof.xhci_gtd], XHCI_CC_SUCCESS
        jnz     .nothing
        mov     edx, [eax+xhci_pipe.HubBuf]
        test    edx, edx
        jz      .nothing
        cmp     byte [edx+1], 29h       ; bDescriptorType
        jnz     .nothing
        or      [eax+xhci_pipe.Flags], XHCI_PIPE_ISHUB
        mov     ebx, eax                ; ebx -> xhci_pipe of the hub
        movzx   edi, byte [edx+2]       ; bNbrPorts
        movzx   ebp, byte [edx+3]       ; low byte of wHubCharacteristics
        shr     ebp, 5
        and     ebp, 3                  ; Transaction Translator Think Time
        DEBUGF 1,'K : XHCI: device on slot %d is a hub with %d ports\n',[ebx+xhci_pipe.SlotId],edi
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexLock
        call    xhci_clear_input_ctx
        mov     eax, [esi+xhci_controller.InputCtx-XCD]
        mov     dword [eax+4], 1        ; only the Slot Context is evaluated
        mov     ecx, [ebx+xhci_pipe.MaxDCI]
        test    ecx, ecx
        jnz     @f
        movi    ecx, 1
@@:
        call    xhci_fill_slot_ctx
; Patch the Slot Context with the fields that describe a hub. Only Configure
; Endpoint takes them into account, Evaluate Context would ignore them.
        mov     eax, [esi+xhci_controller.InputCtx-XCD]
        movi    ecx, 1
        call    xhci_context
        or      dword [eax], 1 shl 26   ; this device is a hub
        cmp     [ebx+xhci_pipe.Speed], USB_SPEED_HS
        jnz     @f
        shl     ebp, 16
        or      [eax+8], ebp
@@:
        shl     edi, 24
        or      [eax+4], edi            ; Number of Ports
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, XHCI_TRB_CONFIG_EP shl 10
        mov     eax, [esi+xhci_controller.InputCtxPhys-XCD]
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_submit
        cmp     eax, XHCI_CC_SUCCESS
        jz      @f
        DEBUGF 1,'K : XHCI: cannot mark the device as a hub, code %d\n',eax
@@:
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexUnlock
.nothing:
        popad
        ret
endp
