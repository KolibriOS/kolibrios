; SuperSpeed support for the xHCI driver, included from xhci.asm when
; XHCI_SUPERSPEED is enabled. The pieces too small to live here - the port
; map, the Slot Context speed, the 512-byte default control endpoint, the
; Max Burst field of the Endpoint Context - sit in the other files under
; "if XHCI_SUPERSPEED" guards; this file keeps the self-contained procedures.
;
; The kernel USB stack knows nothing about SuperSpeed, so it never learns
; about it: a SuperSpeed device is announced as a high-speed one, which also
; keeps the Transaction Translator logic away from it. The raw PORTSC speed
; identifier travels in xhci_pipe.PSIV instead and reaches everything that
; really needs it. SuperSpeed hubs are not supported: the kernel only speaks
; the USB2 hub protocol, so a device behind a USB3 hub is served by the USB2
; half of that hub at high speed.

; =============================================================================
; ============================ SuperSpeed ports ===============================
; =============================================================================

; Called from xhci_new_port for a SuperSpeed root port instead of the USB2
; reset entry point: a SuperSpeed link trains itself and the port enables
; itself on connect, reporting the speed with no reset at all. When the link
; is already up, skip straight to the reset recovery stage, after which
; xhci_port_init picks the device up; otherwise try a warm reset.
; in: esi -> usb_controller, ecx = port index.
proc xhci_ss_start_port
        push    eax edx
        and     [esi+usb_controller.ResettingHub], 0
        mov     [esi+usb_controller.ResettingPort], cl
        push    ecx
        invoke  GetTimerTicks
        pop     ecx
        mov     [esi+usb_controller.ResetTime], eax
        mov     [esi+xhci_controller.ResetStart-XCD], eax
        call    xhci_port_reg
        mov     eax, [edx]
        test    al, XHCI_PORT_PED
        jz      .warm_reset
        mov     [esi+usb_controller.ResettingStatus], 2
        DEBUGF 1,'K : XHCI %x: SS port %d link is up, PORTSC=%x\n',esi,ecx,eax
        jmp     .nothing
.warm_reset:
; Connected, but the link has not come up. The Port Reset bit is set while a
; warm reset is running, so the ordinary reset-wait machinery in
; xhci_port_reset_done serves this case unchanged; the extra WRC change bit is
; acknowledged there together with the usual ones.
        mov     dword [edx], XHCI_PORT_PP + XHCI_PORT_WPR
        mov     [esi+usb_controller.ResettingStatus], 1
        DEBUGF 1,'K : XHCI %x: warm-resetting port %d, PORTSC=%x\n',esi,ecx,eax
.nothing:
        pop     edx eax
        ret
endp

; The kernel restarts pending root ports through the NewPortReset entry of
; usb_hardware_func directly, bypassing xhci_new_port; SuperSpeed ports must
; not get the USB2 reset on that path either, so the entry points here.
; in: esi -> usb_controller, ecx = port index.
proc xhci_ss_port_reset
        bt      [esi+xhci_controller.SSPortMask-XCD], ecx
        jnc     xhci_new_port.reset
        call    xhci_ss_start_port
        ret
endp

; =============================================================================
; ===================== Endpoint Companion descriptors ========================
; =============================================================================

; Reads the SuperSpeed Endpoint Companion descriptor of a bulk endpoint and
; keeps its bMaxBurst for the Endpoint Context, allowing bursts of up to
; bMaxBurst+1 packets per transaction. Zero (no bursts) is always legal, so
; any failure just leaves the field alone. Only bulk endpoints are considered:
; a nonzero burst on a periodic endpoint would also require the Max ESIT
; Payload fields, and real interrupt endpoints do not burst anyway.
; Relies on the stack frame of usb_open_pipe exactly like xhci_init_pipe does:
; [ebp+8] is the config pipe of the device.
; in: ebx -> xhci_pipe of a non-control endpoint being opened.
proc xhci_ss_get_burst
        push    eax ecx edx edi
; 1. Only SuperSpeed bulk endpoints are of interest.
        cmp     [ebx+xhci_pipe.PSIV], XHCI_PSI_SS
        jb      .nothing
        mov     eax, [ebx+xhci_pipe.EpType]
        and     al, 3
        cmp     al, 2                   ; bulk endpoint types are 2 and 6
        jnz     .nothing
; 2. Find the configuration data kept by the kernel behind the device
; descriptor; it holds the endpoint descriptors with their companions.
        mov     eax, [ebp+8]
        mov     eax, [eax+usb_pipe.DeviceData]
        test    eax, eax
        jz      .nothing
        movzx   edx, [eax+usb_device_data.DeviceDescrSize]
        mov     edi, [eax+usb_device_data.ConfigDataSize]
        lea     ecx, [eax+edx+usb_device_data.DeviceDescriptor]
        add     edi, ecx                ; edi -> end of the configuration data
; 3. Reconstruct bEndpointAddress from the Device Context Index.
        mov     edx, [ebx+xhci_pipe.DCI]
        shr     edx, 1
        test    [ebx+xhci_pipe.Flags], XHCI_PIPE_IN
        jz      @f
        or      dl, 80h
@@:
; 4. Walk the descriptors to the one of this endpoint.
.walk:
        lea     eax, [ecx+2]
        cmp     eax, edi
        ja      .nothing
        movzx   eax, byte [ecx]         ; bLength
        test    eax, eax
        jz      .nothing                ; malformed data, stop
        cmp     byte [ecx+1], 5         ; an endpoint descriptor?
        jnz     .advance
        cmp     [ecx+2], dl             ; bEndpointAddress
        jnz     .advance
; 5. The companion descriptor (type 30h) immediately follows its endpoint.
        add     ecx, eax
        lea     eax, [ecx+3]
        cmp     eax, edi
        ja      .nothing
        cmp     byte [ecx+1], 30h       ; SuperSpeed Endpoint Companion
        jnz     .nothing
        movzx   eax, byte [ecx+2]       ; bMaxBurst
        cmp     eax, 15
        ja      .nothing
        mov     [ebx+xhci_pipe.MaxBurst], eax
        DEBUGF 1,'K : XHCI: slot %d dci %d bursts %d+1 packets\n',[ebx+xhci_pipe.SlotId],[ebx+xhci_pipe.DCI],eax
        jmp     .nothing
.advance:
        add     ecx, eax
        jmp     .walk
.nothing:
        pop     edi edx ecx eax
        ret
endp
