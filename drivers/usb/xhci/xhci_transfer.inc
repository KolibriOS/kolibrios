; Transfer rings, transfers and deferred processing for the xHCI driver.
; This file is included from xhci.asm.

; =============================================================================
; ========================= Transfer descriptors ==============================
; =============================================================================

proc xhci_alloc_td
        push    ebx ecx edi
        mov     ebx, xhci_gtd_mutex
        invoke  usbhc_api.usb_allocate_common, sizeof.xhci_gtd + sizeof.usb_gtd
        test    eax, eax
        jz      @f
        mov     edi, eax
        push    eax
        xor     eax, eax
        movi    ecx, (sizeof.xhci_gtd + sizeof.usb_gtd + 3)/4
        rep stosd
        pop     eax
        add     eax, sizeof.xhci_gtd
@@:
        pop     edi ecx ebx
        ret
endp

proc xhci_free_td
        sub     dword [esp+4], sizeof.xhci_gtd
        jmp     [usbhc_api.usb_free_common]
endp

; =============================================================================
; ============================== Ring writing =================================
; =============================================================================

; Writes one TRB at the current enqueue position of a transfer ring and
; advances the position.
; in: ebx -> xhci_pipe. out: eax = address of the written TRB.
proc xhci_ring_put stdcall uses ecx edx edi, \
        d0:dword, d1:dword, d2:dword, d3:dword
        mov     edi, [ebx+xhci_pipe.Enqueue]
        shl     edi, 4
        add     edi, [ebx+xhci_pipe.Ring]
        mov     eax, [d0]
        mov     [edi], eax
        mov     eax, [d1]
        mov     [edi+4], eax
        mov     eax, [d2]
        mov     [edi+8], eax
        mov     eax, [d3]
        or      eax, [ebx+xhci_pipe.Cycle]
; The very first TRB of a transfer is written with an inverted Cycle bit and
; corrected once the whole transfer is in place, so that the controller cannot
; start executing a chain that is still being built.
        cmp     [ebx+xhci_pipe.FirstFix], 1
        jnz     @f
        xor     eax, XHCI_TRB_C
        mov     [ebx+xhci_pipe.FirstFix], edi
@@:
        mov     [edi+12], eax
; Advance, refreshing the Link TRB when the ring wraps around.
        mov     eax, [ebx+xhci_pipe.Enqueue]
        inc     eax
        cmp     eax, XHCI_RING_TRBS-1
        jb      .stored
        push    edi
        mov     edi, [ebx+xhci_pipe.Ring]
        add     edi, (XHCI_RING_TRBS-1)*16
        mov     eax, [ebx+xhci_pipe.Cycle]
        or      eax, (XHCI_TRB_LINK shl 10) + XHCI_TRB_TC
        mov     [edi+12], eax
        pop     edi
        xor     [ebx+xhci_pipe.Cycle], XHCI_TRB_C
        xor     eax, eax
.stored:
        mov     [ebx+xhci_pipe.Enqueue], eax
        mov     eax, edi
        ret
endp

; Computes the control dword of a data TRB.
; in: esi -> usb_gtd, ebp = bytes still to come after this fragment.
; out: edx = control dword without the Cycle bit.
proc xhci_data_ctl
        push    eax
        mov     edx, XHCI_TRB_NORMAL shl 10
        mov     eax, [esi+xhci_gtd.Dir-sizeof.xhci_gtd]
        test    al, 8
        jz      @f
        mov     edx, XHCI_TRB_DATA shl 10
        and     al, 3
        cmp     al, 2
        jnz     @f
        or      edx, XHCI_TRB_DIR_IN
@@:
; Chain every fragment except the last one; ask for an interrupt on the last
; one and on any short packet.
        or      edx, XHCI_TRB_ISP
        test    ebp, ebp
        jz      .last
        or      edx, XHCI_TRB_CH
        jmp     .nothing
.last:
        or      edx, XHCI_TRB_IOC
.nothing:
        pop     eax
        ret
endp

; Walks the buffer of a descriptor the way xhci_emit_data lays it out into
; TRBs: physically contiguous pages are merged into one TRB of up to 64K, a
; page that is not contiguous with the previous one starts a new TRB.
; in: edi = virtual address, ebp = length.
; out: eax = number of TRBs needed (at least one). Preserves edi, ebp.
proc xhci_count_trbs
        push    ebx ecx edx esi edi ebp
        xor     esi, esi                ; TRB count
        test    ebp, ebp
        jnz     .loop
        inc     esi
        jmp     .done
.loop:
        call    xhci_next_run           ; eax = phys, ecx = length of the run
        inc     esi
        add     edi, ecx
        sub     ebp, ecx
        jnz     .loop
.done:
        mov     eax, esi
        pop     ebp edi esi edx ecx ebx
        ret
endp

; Finds the longest physically contiguous run starting at the current
; position of a buffer, limited to what fits into one TRB (64K).
; in: edi = virtual address, ebp = bytes left.
; out: eax = physical address of the run, ecx = its length.
proc xhci_next_run
        push    ebx edx esi
        mov     eax, edi
        push    ecx
        invoke  GetPhysAddr
        pop     ecx
        mov     esi, eax                ; esi = physical start
; the first fragment ends at the page boundary or at the end of the buffer
        mov     ecx, edi
        and     ecx, 0FFFh
        mov     edx, 1000h
        sub     edx, ecx                ; edx = bytes to the end of the page
        cmp     edx, ebp
        jbe     @f
        mov     edx, ebp
@@:
        mov     ecx, edx                ; ecx = run length so far
; extend the run over following pages while they are physically contiguous
; and the run stays below 64K
.extend:
        mov     edx, ebp
        sub     edx, ecx
        jz      .done                   ; whole buffer covered
        mov     eax, ecx
        add     eax, 1000h
        cmp     eax, 10000h
        ja      .done                   ; another page would exceed 64K
        lea     eax, [edi+ecx]          ; virtual address of the next page
        push    ecx edx
        invoke  GetPhysAddr
        pop     edx ecx
        mov     ebx, esi
        add     ebx, ecx                ; expected physical address
        cmp     eax, ebx
        jnz     .done                   ; not contiguous
        cmp     edx, 1000h
        jbe     @f
        mov     edx, 1000h
@@:
        add     ecx, edx
        jmp     .extend
.done:
        mov     eax, esi
        pop     esi edx ebx
        ret
endp

; Writes the data TRBs of one descriptor. Physically contiguous pages of the
; buffer are merged into one TRB, so a typical kernel buffer of a few
; megabytes takes a few dozen TRBs instead of one per page.
; in: ebx -> xhci_pipe, esi -> usb_gtd.
proc xhci_emit_data
        push    ebp edi
        mov     ebp, [esi+xhci_gtd.Len-sizeof.xhci_gtd]
        mov     edi, [esi+xhci_gtd.Buf-sizeof.xhci_gtd]
        test    ebp, ebp
        jnz     .loop
; A zero-length transfer still needs one TRB.
        call    xhci_data_ctl
        stdcall xhci_ring_put, 0, 0, 0, edx
        jmp     .done
.loop:
        call    xhci_next_run           ; eax = phys, ecx = length
        push    eax                     ; physical address of the run
        push    ecx                     ; its length
        add     edi, ecx
        sub     ebp, ecx
; The TD Size field tells how many packets are still to come after this TRB.
        mov     ecx, [ebx+xhci_pipe.MaxPacket]
        and     ecx, 7FFh
        jnz     @f
        movi    ecx, 64
@@:
        mov     eax, ebp
        add     eax, ecx
        dec     eax
        xor     edx, edx
        div     ecx
        cmp     eax, 31
        jbe     @f
        movi    eax, 31
@@:
        shl     eax, 17
        pop     ecx                     ; the length of the run
        or      ecx, eax
        call    xhci_data_ctl
        pop     eax                     ; the physical address
        stdcall xhci_ring_put, eax, 0, ecx, edx
        test    ebp, ebp
        jnz     .loop
.done:
        pop     edi ebp
        ret
endp

; Writes the TRBs of one descriptor.
; in: ebx -> xhci_pipe, esi -> usb_gtd, edi -> last usb_gtd of the transfer.
proc xhci_emit_td
        push    eax ecx edx
; Remember where the descriptor starts, so that every TRB of it can be entered
; into the map afterwards.
        mov     eax, [ebx+xhci_pipe.Enqueue]
        mov     [esi+xhci_gtd.FirstTrb-sizeof.xhci_gtd], eax
        mov     eax, [esi+xhci_gtd.Dir-sizeof.xhci_gtd]
        test    al, 8
        jz      .data                   ; a bulk or interrupt transfer
        and     al, 3
        jz      .setup
        cmp     [esi+xhci_gtd.Len-sizeof.xhci_gtd], 0
        jnz     .data
        jmp     .status
; ---------------------------------------------------------------- Setup Stage
.setup:
; The Transfer Type field depends on the stage that follows.
        xor     ecx, ecx                ; no data stage
        cmp     esi, edi
        jz      .have_trt
        mov     eax, [esi+usb_gtd.NextVirt]
        cmp     [eax+xhci_gtd.Len-sizeof.xhci_gtd], 0
        jz      .have_trt
        movi    ecx, 2                  ; an OUT data stage
        mov     eax, [eax+xhci_gtd.Dir-sizeof.xhci_gtd]
        and     al, 3
        cmp     al, 2
        jnz     .have_trt
        movi    ecx, 3                  ; an IN data stage
.have_trt:
        shl     ecx, 16
        or      ecx, (XHCI_TRB_SETUP shl 10) + XHCI_TRB_IDT + XHCI_TRB_IOC
        mov     eax, [esi+xhci_gtd.Buf-sizeof.xhci_gtd]
        mov     edx, [eax+4]
        mov     eax, [eax]
        stdcall xhci_ring_put, eax, edx, 8, ecx
        jmp     .finish
; --------------------------------------------------------------- Status Stage
.status:
        mov     edx, (XHCI_TRB_STATUS shl 10) + XHCI_TRB_IOC
        mov     eax, [esi+xhci_gtd.Dir-sizeof.xhci_gtd]
        and     al, 3
        cmp     al, 2
        jnz     @f
        or      edx, XHCI_TRB_DIR_IN
@@:
        stdcall xhci_ring_put, 0, 0, 0, edx
        jmp     .finish
; ---------------------------------------------- Data Stage or normal transfer
.data:
        call    xhci_emit_data
.finish:
; Enter every TRB of the descriptor into the map, so that an event reported
; for any of them - the controller reports errors and short packets on the TRB
; where they happened, not on the last one - finds the right owner. Then
; remember where the ring continues after the descriptor.
        mov     ecx, [esi+xhci_gtd.FirstTrb-sizeof.xhci_gtd]
        mov     edx, [ebx+xhci_pipe.RingMap]
.map_loop:
        cmp     ecx, [ebx+xhci_pipe.Enqueue]
        jz      .mapped
        mov     [edx+ecx*4], esi
        inc     ecx
        cmp     ecx, XHCI_RING_TRBS-1
        jb      .map_loop
        xor     ecx, ecx
        jmp     .map_loop
.mapped:
        mov     eax, [ebx+xhci_pipe.Enqueue]
        shl     eax, 4
        add     eax, [ebx+xhci_pipe.RingPhys]
        mov     [esi+xhci_gtd.NextTrb-sizeof.xhci_gtd], eax
        mov     eax, [ebx+xhci_pipe.Cycle]
        mov     [esi+xhci_gtd.NextCycle-sizeof.xhci_gtd], eax
        pop     edx ecx eax
        ret
endp

; =============================================================================
; ============================= Building transfers ============================
; =============================================================================

; Allocates the descriptors needed for one stage of a transfer.
; ebx = pipe, the other parameters come through the stack.
; out: eax = the new end-of-queue descriptor, or 0 on error.
proc xhci_alloc_transfer stdcall uses edi, \
        buffer:dword, size:dword, flags:dword, td:dword, direction:dword
locals
origTD          dd      ?
packetSize      dd      ?       ; must be last, see usb_init_transfer
endl
        mov     eax, [td]
        mov     [origTD], eax
; A transfer always starts at the current tail of the queue; that is the moment
; to reset the reservation counter of the ring.
        cmp     eax, [ebx+usb_pipe.LastTD]
        jnz     .no_reset
        and     [ebx+xhci_pipe.Reserved-sizeof.xhci_pipe], 0
.no_reset:
        cmp     [direction], 8
        jnz     .check_hubdata
        mov     eax, [buffer]
        cmp     byte [eax], 0                   ; host to device, standard
        jnz     .check_hubdescr
        cmp     byte [eax+1], 5                 ; SET_ADDRESS
        jnz     .split
; The address has already been assigned by the Address Device command, so the
; request must not reach the device; it is answered locally instead.
        or      [ebx+xhci_pipe.Flags-sizeof.xhci_pipe], XHCI_PIPE_SETADDR
        jmp     .split
.check_hubdescr:
; A request for the hub descriptor tells that the device is a hub, which the
; controller has to know about; the answer carries the number of ports.
        cmp     byte [eax], 0A0h
        jnz     .split
        cmp     byte [eax+1], 6                 ; GET_DESCRIPTOR
        jnz     .split
        cmp     byte [eax+3], 29h               ; the hub descriptor
        jnz     .split
        test    [ebx+xhci_pipe.Flags-sizeof.xhci_pipe], XHCI_PIPE_ISHUB
        jnz     .split
        or      [ebx+xhci_pipe.Flags-sizeof.xhci_pipe], XHCI_PIPE_HUBPEND
        and     [ebx+xhci_pipe.HubBuf-sizeof.xhci_pipe], 0
        jmp     .split
.check_hubdata:
        test    [ebx+xhci_pipe.Flags-sizeof.xhci_pipe], XHCI_PIPE_HUBPEND
        jz      .split
        cmp     [direction], 0Eh                ; the data stage of that request
        jnz     .split
        mov     eax, [buffer]
        mov     [ebx+xhci_pipe.HubBuf-sizeof.xhci_pipe], eax
.split:
; Split the transfer if it does not fit into a single descriptor.
        cmp     [size], XHCI_MAX_TD
        jbe     .lastpacket
        mov     ecx, [ebx+xhci_pipe.MaxPacket-sizeof.xhci_pipe]
        and     ecx, 7FFh
        jnz     @f
        movi    ecx, 64
@@:
        mov     eax, XHCI_MAX_TD
        xor     edx, edx
        mov     edi, eax
        div     ecx
        sub     edi, edx
        mov     [packetSize], edi
.fullpackets:
        call    xhci_alloc_packet
        test    eax, eax
        jz      .fail
        mov     [td], eax
        add     [buffer], edi
        sub     [size], edi
        cmp     [size], XHCI_MAX_TD
        ja      .fullpackets
.lastpacket:
        mov     eax, [size]
        mov     [packetSize], eax
        call    xhci_alloc_packet
        test    eax, eax
        jz      .fail
        ret
.fail:
        mov     edi, xhci_hardware_func
        mov     eax, [td]
        invoke  usbhc_api.usb_undo_tds, [origTD]
        xor     eax, eax
        ret
endp

; Helper for xhci_alloc_transfer: allocates and fills in one descriptor.
; ebx = pipe; fills the current last descriptor and returns eax = the next one.
proc xhci_alloc_packet
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
        push    ebx ecx edx edi
; 1. Work out how many TRBs the descriptor needs. A TRB cannot cross a page
; boundary, so one TRB per touched page is needed in the worst case.
        push    ebp edi
        mov     edi, [.buffer]
        mov     ebp, [.packetSize]
        call    xhci_count_trbs
        pop     edi ebp
        mov     edi, eax
; 2. Refuse the transfer if the ring cannot hold it: the TRBs already queued
; plus those reserved by the earlier stages of this transfer must leave room.
; The ring holds XHCI_RING_TRBS entries, the last of which is the Link TRB, so
; positions run from 0 to XHCI_RING_TRBS-2 and wrap over that many slots.
        mov     eax, [ebx+xhci_pipe.Enqueue-sizeof.xhci_pipe]
        sub     eax, [ebx+xhci_pipe.Dequeue-sizeof.xhci_pipe]
        jns     @f
        add     eax, XHCI_RING_TRBS-1
@@:
        add     eax, [ebx+xhci_pipe.Reserved-sizeof.xhci_pipe]
        add     eax, edi
        cmp     eax, XHCI_RING_TRBS-2
        jb      @f
; The ring is full. This is always worth a line: the caller sees a plain
; failure and a large transfer that keeps being refused looks like a hang.
        DEBUGF 1,'K : XHCI: ring full, transfer of %d bytes refused (need %d TRBs, %d used)\n',[.transferSize],edi,eax
        jmp     .fail
@@:
        add     [ebx+xhci_pipe.Reserved-sizeof.xhci_pipe], edi
; 3. Allocate the next descriptor and let the common code link the two.
        call    xhci_alloc_td
        test    eax, eax
        jz      .fail
        push    eax
        invoke  usbhc_api.usb_init_transfer
; 4. Fill in the descriptor that has just been closed; ecx points to it.
        mov     edx, [.buffer]
        mov     [ecx+xhci_gtd.Buf-sizeof.xhci_gtd], edx
        mov     edx, [.packetSize]
        mov     [ecx+xhci_gtd.Len-sizeof.xhci_gtd], edx
        mov     edx, [.direction]
        mov     [ecx+xhci_gtd.Dir-sizeof.xhci_gtd], edx
        mov     edx, [.Flags]
        mov     [ecx+xhci_gtd.Flags-sizeof.xhci_gtd], edx
        mov     [ecx+xhci_gtd.TrbCount-sizeof.xhci_gtd], edi
        and     [ecx+xhci_gtd.Status-sizeof.xhci_gtd], 0
        and     [ecx+xhci_gtd.Residual-sizeof.xhci_gtd], 0
        and     [ecx+xhci_gtd.Done-sizeof.xhci_gtd], 0
        pop     eax
        pop     edi edx ecx ebx
        ret
.fail:
        xor     eax, eax
        pop     edi edx ecx ebx
        ret
endp

; Activates a transfer previously built by xhci_alloc_transfer.
; in: esi -> usb_controller, ebx -> usb_pipe,
;     [esp+4] -> first usb_gtd, ecx -> last descriptor.
proc xhci_insert_transfer
        push    ebx ecx edx esi edi ebp
virtual at esp
        rd      6       ; saved registers
        dd      ?       ; return address
.firstTD        dd      ?
end virtual
        mov     edi, ecx                ; edi -> last descriptor
        mov     esi, [.firstTD]         ; esi -> current descriptor
        sub     ebx, sizeof.xhci_pipe
        test    [ebx+xhci_pipe.Flags], XHCI_PIPE_SETADDR
        jnz     .fake_set_address
        and     [ebx+xhci_pipe.Reserved], 0
        mov     [ebx+xhci_pipe.FirstFix], 1
if XHCI_TIMING_TRACE
        and     [xhci_tt_bytes], 0
        and     [xhci_tt_trbs], 0
        and     [xhci_tt_tds], 0
end if
.td_loop:
if XHCI_TIMING_TRACE
        mov     eax, [esi+xhci_gtd.Len-sizeof.xhci_gtd]
        add     [xhci_tt_bytes], eax
        mov     eax, [esi+xhci_gtd.TrbCount-sizeof.xhci_gtd]
        add     [xhci_tt_trbs], eax
        inc     [xhci_tt_tds]
end if
        call    xhci_emit_td
        cmp     esi, edi
        jz      .td_done
        mov     esi, [esi+usb_gtd.NextVirt]
        jmp     .td_loop
.td_done:
; Make the first TRB visible, then ring the doorbell.
        mov     eax, [ebx+xhci_pipe.FirstFix]
        cmp     eax, 1
        jbe     @f
        xor     dword [eax+12], XHCI_TRB_C
@@:
        and     [ebx+xhci_pipe.FirstFix], 0
        mov     esi, [ebx+sizeof.xhci_pipe+usb_pipe.Controller]
        mov     eax, [ebx+xhci_pipe.SlotId]
        mov     edx, [ebx+xhci_pipe.DCI]
; Trace only the first transfers of the pipe; see xhci_handle_transfer_event.
        cmp     [ebx+xhci_pipe.TraceCount], 8
        jae     @f
        inc     [ebx+xhci_pipe.TraceCount]
        xhci_trace 'K : XHCI ring: slot %d dci %d\n', eax, edx
@@:
if XHCI_TIMING_TRACE
; Account the transfer instead of tracing it; see the counters in xhci.asm.
        pushad
        mov     esi, [ebx+xhci_pipe.EpType]
        and     esi, not 4              ; bulk out is 2, bulk in is 6
        cmp     esi, 2
        jnz     .st_done
        rdtsc
        cmp     [xhci_st_inflight], 0
        jnz     .st_running
        mov     [xhci_st_busy_from], eax
        cmp     [xhci_st_done], 0
        jz      .st_nogap
        push    eax
        sub     eax, [xhci_st_done]
; A gap this long is either genuine idling between two unrelated bursts or a
; wrap of the 32-bit stamp; either way it says nothing about the transfer
; that follows, so drop it.
        cmp     eax, 40000000h
        jae     .st_charged
        cmp     [xhci_tt_bytes], 31
        jnz     .st_stage
        add     [xhci_st_gapcmd], eax
        jmp     .st_charged
.st_stage:
        add     [xhci_st_gapstg], eax
.st_charged:
        pop     eax
.st_nogap:
        cmp     [xhci_st_tick], 0
        jnz     .st_running
        invoke  GetTimerTicks
        mov     [xhci_st_tick], eax
.st_running:
        mov     eax, [xhci_tt_tds]
        add     [xhci_st_inflight], eax
        add     [xhci_st_tds], eax
        mov     eax, [xhci_tt_bytes]
        add     [xhci_st_bytes], eax
; A command block wrapper is 31 bytes and starts one SCSI command.
        cmp     eax, 31
        jnz     .st_done
        inc     [xhci_st_cmds]
.st_done:
        popad
end if
        call    xhci_doorbell
; Transfers are queued from the context of whoever asked for them, while the
; USB thread may be asleep with a long timeout computed when nothing was in
; flight. Wake it so that it recomputes the timeout (see ProcessDeferred) and
; polls while this transfer is pending; otherwise a completion whose interrupt
; is late or missed would wait for the two-second safety net.
        movi    ebx, 1                  ; the wakeup only happens when ebx is set
        invoke  usbhc_api.usb_wakeup_if_needed
        pop     ebp edi esi edx ecx ebx
        ret
.fake_set_address:
; SET_ADDRESS is answered without touching the ring: the controller has already
; given the device an address of its own choosing.
        and     [ebx+xhci_pipe.Flags], not XHCI_PIPE_SETADDR
        and     [ebx+xhci_pipe.Reserved], 0
.fake_loop:
        mov     [esi+xhci_gtd.Status-sizeof.xhci_gtd], XHCI_CC_SUCCESS
        and     [esi+xhci_gtd.Residual-sizeof.xhci_gtd], 0
        mov     [esi+xhci_gtd.Done-sizeof.xhci_gtd], 1
        cmp     esi, edi
        jz      @f
        mov     esi, [esi+usb_gtd.NextVirt]
        jmp     .fake_loop
@@:
        mov     esi, [ebx+sizeof.xhci_pipe+usb_pipe.Controller]
        mov     [esi+xhci_controller.NeedScan-XCD], 1
        movi    ebx, 1                  ; the wakeup only happens when ebx is set
        invoke  usbhc_api.usb_wakeup_if_needed
        pop     ebp edi esi edx ecx ebx
        ret
endp

; =============================================================================
; ============================== Root hub ports ===============================
; =============================================================================

; Disables the given port of the root hub.
; in: esi -> usb_controller, ecx = port index.
proc xhci_port_disable
        push    eax edx
        call    xhci_port_reg
        mov     dword [edx], XHCI_PORT_PP + XHCI_PORT_PED   ; one disables the port
        pop     edx eax
        ret
endp

; Called when a device has been connected for at least USB_CONNECT_DELAY ticks.
; in: esi -> usb_controller, ecx = port index.
proc xhci_new_port
        push    edx
        call    xhci_port_reg
        mov     eax, [edx]
        DEBUGF 1,'K : XHCI %x port %d state is %x\n',esi,ecx,eax
if XHCI_SUPERSPEED
; SuperSpeed ports get their own connect path: the link trains itself and
; there is nothing to reset the USB2 way. The single-port-at-a-time
; serialization is shared with the USB2 path below.
        bt      [esi+xhci_controller.SSPortMask-XCD], ecx
        jnc     .usb2_port
        bts     [esi+usb_controller.PendingPorts], ecx
        cmp     [esi+usb_controller.ResettingPort], -1
        jnz     .nothing
        btr     [esi+usb_controller.PendingPorts], ecx
        call    xhci_ss_start_port
        jmp     .nothing
.usb2_port:
end if
; A SuperSpeed port reports its speed before any reset and enables itself on
; connect. This path serves USB2 only, so leave such a device alone instead
; of resetting it and failing later.
        push    eax
        shr     eax, 10
        and     eax, 15
        cmp     eax, XHCI_PSI_SS
        pop     eax
        jb      @f
        DEBUGF 1,'K : XHCI: port %d has a SuperSpeed device, not supported\n',ecx
        jmp     .nothing
@@:
; Only one port at a time can go through the first stages of configuration.
        bts     [esi+usb_controller.PendingPorts], ecx
        cmp     [esi+usb_controller.ResettingPort], -1
        jnz     .nothing
        btr     [esi+usb_controller.PendingPorts], ecx
        pop     edx
; Fall through to the reset entry point.

; Starts reset signalling on the given port.
; in: esi -> usb_controller, ecx = port index.
.reset:
        push    edx
        call    xhci_port_reg
        and     [esi+usb_controller.ResettingHub], 0
        mov     [esi+usb_controller.ResettingPort], cl
        mov     dword [edx], XHCI_PORT_PP + XHCI_PORT_PR
        invoke  GetTimerTicks
        mov     [esi+usb_controller.ResetTime], eax
        mov     [esi+xhci_controller.ResetStart-XCD], eax
        mov     [esi+usb_controller.ResettingStatus], 1
        DEBUGF 1,'K : XHCI %x: resetting port %d\n',esi,ecx
.nothing:
        pop     edx
        ret
endp

; Called USB_RESET_TIME ticks after reset signalling was started. On xHCI the
; controller itself decides when reset signalling ends and clears the Port
; Reset bit; on real hardware that takes around 50 ms, well beyond
; USB_RESET_TIME, so as long as the bit is still set, just keep waiting
; (an emulator typically completes the reset instantly). Only when the
; controller is done, acknowledge the change bits and move on to the reset
; recovery period.
proc xhci_port_reset_done
        push    ecx edx
        movzx   ecx, [esi+usb_controller.ResettingPort]
        call    xhci_port_reg
        mov     eax, [edx]
        test    al, XHCI_PORT_PR
        jz      .reset_over
; Still resetting. Come back in USB_RESET_TIME more ticks, unless a second has
; passed since the start: a port unable to finish a reset in that time is
; broken, give up on it then.
        invoke  GetTimerTicks
        mov     [esi+usb_controller.ResetTime], eax
        sub     eax, [esi+xhci_controller.ResetStart-XCD]
        cmp     eax, 100
        jbe     .nothing        ; stay in ResettingStatus = 1
        DEBUGF 1,'K : XHCI: reset of port %d timed out\n',ecx
        mov     [esi+usb_controller.ResettingStatus], -1
        jmp     .nothing        ; the recovery timer will report the failure
.reset_over:
if XHCI_SUPERSPEED
; A completed warm reset raises WRC as well; acknowledging it on a USB2 port
; is harmless, the bit is never set there.
        mov     dword [edx], XHCI_PORT_PP + XHCI_PORT_PRC + XHCI_PORT_PEC + XHCI_PORT_WRC
else
        mov     dword [edx], XHCI_PORT_PP + XHCI_PORT_PRC + XHCI_PORT_PEC
end if
        invoke  GetTimerTicks
        mov     [esi+usb_controller.ResetTime], eax
        mov     [esi+usb_controller.ResettingStatus], 2
.nothing:
        pop     edx ecx
        ret
endp

; Called when a new device has been reset, has recovered and can be configured.
proc xhci_port_init
        push    ecx edx
        xor     eax, eax
        xchg    al, [esi+usb_controller.ResettingStatus]
        test    al, al
        js      .fail
        movzx   ecx, [esi+usb_controller.ResettingPort]
        call    xhci_port_reg
        mov     eax, [edx]
        DEBUGF 1,'K : XHCI %x status of port %d is %x\n',esi,ecx,eax
        test    al, XHCI_PORT_PED
        jz      .fail
; Bits 13:10 of PORTSC hold the speed identifier assigned by the controller.
        mov     edx, eax
        shr     edx, 10
        and     edx, 15
if XHCI_SUPERSPEED
        and     [esi+xhci_controller.ResettingPSIV-XCD], 0
        cmp     edx, XHCI_PSI_SS
        jb      .usb2_speed
        bt      [esi+xhci_controller.SSPortMask-XCD], ecx
        jnc     .usb2_speed
; The kernel knows nothing about SuperSpeed, so the device is reported as a
; high-speed one; the raw identifier reaches the Slot Context via
; ResettingPSIV and the PSIV field of the pipe.
        mov     [esi+xhci_controller.ResettingPSIV-XCD], edx
        DEBUGF 1,'K : XHCI: port %d device is SuperSpeed, PSIV %d\n',ecx,edx
        movi    eax, USB_SPEED_HS
        jmp     .known_speed
.usb2_speed:
end if
        cmp     edx, XHCI_PSI_FS
        jz      .full_speed
        cmp     edx, XHCI_PSI_LS
        jz      .low_speed
        cmp     edx, XHCI_PSI_HS
        jz      .high_speed
        DEBUGF 1,'K : XHCI: unsupported port speed %d, device ignored\n',edx
        jmp     .fail
.full_speed:
        movi    eax, USB_SPEED_FS
        jmp     .known_speed
.low_speed:
        movi    eax, USB_SPEED_LS
        jmp     .known_speed
.high_speed:
        movi    eax, USB_SPEED_HS
.known_speed:
        call    xhci_new_device
        test    eax, eax
        jz      .disable
        pop     edx ecx
        ret
.disable:
        movzx   ecx, [esi+usb_controller.ResettingPort]
        call    xhci_port_disable
.fail:
        pop     edx ecx
        jmp     [usbhc_api.usb_test_pending_port]
endp

; Scans the root hub ports for status changes.
; in: esi -> usb_controller.
proc xhci_scan_ports
        push    ebx ecx edx
        xor     ecx, ecx
.loop:
        cmp     ecx, [esi+usb_controller.NumPorts]
        jae     .done
        call    xhci_port_reg
        mov     eax, [edx]
        test    eax, XHCI_PORT_CHANGES
        jz      .next
; Acknowledge the change bits, then read the status again. Only Port Power
; and the observed change bits are written; echoing anything else back makes
; some controllers restart port detection over and over.
        mov     ebx, eax
        and     eax, XHCI_PORT_CHANGES
        or      eax, XHCI_PORT_PP
        mov     [edx], eax
        mov     eax, [edx]
        DEBUGF 1,'K : XHCI %x: status of port %d changed to %x\n',esi,ecx,ebx
        test    ebx, XHCI_PORT_CSC
        jz      .next
; A device has been connected or disconnected.
        cmp     [esi+usb_controller.ResettingHub], 0
        jnz     .noreset
        cmp     cl, [esi+usb_controller.ResettingPort]
        jnz     .noreset
        cmp     [esi+usb_controller.ResettingStatus], 2
        jnz     @f
        test    al, XHCI_PORT_CCS
        jnz     .next
@@:
        mov     [esi+usb_controller.ResettingStatus], -1
.noreset:
        bts     [esi+usb_controller.NewDisconnected], ecx
        test    al, XHCI_PORT_CCS
        jz      .disconnect
        push    eax ecx
        invoke  GetTimerTicks
        pop     ecx
; If a device on this port has already dropped off during the debounce
; interval once, do not wait it out again: pretend the connect happened long
; ago, so that the reset is started by the very same pass.
        bt      [esi+xhci_controller.BounceMask-XCD], ecx
        jnc     @f
        sub     eax, USB_CONNECT_DELAY+1
        DEBUGF 1,'K : XHCI: port %d bounced before, resetting at once\n',ecx
@@:
        mov     [esi+usb_controller.ConnectedTime+ecx*4], eax
        pop     eax
        bts     [esi+usb_controller.NewConnected], ecx
        jmp     .next
.disconnect:
; The device disappeared. If that happened while the connect debounce was
; still running, remember the port as bouncing.
        btr     [esi+usb_controller.NewConnected], ecx
        jnc     .next
        bts     [esi+xhci_controller.BounceMask-XCD], ecx
.next:
        inc     ecx
        jmp     .loop
.done:
        pop     edx ecx ebx
        ret
endp

; =============================================================================
; =========================== Deferred processing =============================
; =============================================================================

; Called regularly from the USB thread.
; in: esi -> usb_controller. out: eax = maximum timeout before the next call.
proc xhci_process_deferred
        push    ebx edi
if XHCI_TIMING_TRACE
; Once a second, report what the bus actually did. window is in timer ticks of
; 10 ms, busy is in TSC ticks - divide it by the cpu frequency the kernel
; prints at boot to get seconds. bytes/window is the throughput the system
; sees, bytes/busy is what the wire manages, and the difference between the
; two is time nobody spent transferring anything.
        pushad
        cmp     [xhci_st_bytes], 0
        jz      .st_quiet
        invoke  GetTimerTicks
        mov     ebx, eax
        sub     eax, [xhci_st_tick]
        cmp     eax, 100
        jb      .st_quiet
        DEBUGF 1,'K : XT sum window=%u busy=%u bytes=%u tds=%u cmds=%u gapcmd=%u gapstg=%u\n',eax,[xhci_st_busy],[xhci_st_bytes],[xhci_st_tds],[xhci_st_cmds],[xhci_st_gapcmd],[xhci_st_gapstg]
        mov     [xhci_st_tick], ebx
        and     [xhci_st_busy], 0
        and     [xhci_st_bytes], 0
        and     [xhci_st_tds], 0
        and     [xhci_st_cmds], 0
        and     [xhci_st_gapcmd], 0
        and     [xhci_st_gapstg], 0
        and     [xhci_st_done], 0
.st_quiet:
        popad
end if
; 1. Collect the events the interrupt handler has noticed.
        xor     eax, eax
        xchg    eax, [esi+xhci_controller.DeferredActions-XCD]
        push    eax
        push    -1                      ; the return value
; A Host System Error or Host Controller Error means the controller is dying;
; nothing here can revive it, but dying silently would leave nothing to debug
; from. The live register is checked rather than the collected bits, so this
; works in polling mode too, and the note is made only once.
        cmp     [esi+xhci_controller.FatalNoted-XCD], 0
        jnz     @f
        mov     eax, [esi+xhci_controller.OpBase-XCD]
        mov     eax, [eax+XhciStatusReg]
        test    eax, XHCI_STS_HSE + XHCI_STS_HCE
        jz      @f
        mov     [esi+xhci_controller.FatalNoted-XCD], 1
        DEBUGF 1,'K : XHCI: host controller error, USBSTS %x\n',eax
@@:
; 2. Drain the event ring. This is done unconditionally: some controllers set
; the status bits without actually raising the interrupt.
        call    xhci_process_event_ring
; 3. Handle root hub status changes.
        mov     eax, [esp+4]
        test    eax, XHCI_STS_PCD
        jnz     .scan_ports
        cmp     [esi+xhci_controller.PortChanged-XCD], 0
        jz      .skip_roothub
.scan_ports:
        and     [esi+xhci_controller.PortChanged-XCD], 0
        call    xhci_scan_ports
.skip_roothub:
; 4. Process the disconnect events.
        invoke  usbhc_api.usb_disconnect_stage2
; 5. Configure the devices that have been connected long enough.
        xor     ecx, ecx
        cmp     [esi+usb_controller.NewConnected], ecx
        jz      .skip_newconnected
.portloop:
        bt      [esi+usb_controller.NewConnected], ecx
        jnc     .nextport
        push    ecx
        invoke  GetTimerTicks
        pop     ecx
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
        call    xhci_new_port
        jmp     .portloop
.nextport:
        inc     ecx
        cmp     ecx, [esi+usb_controller.NumPorts]
        jb      .portloop
.skip_newconnected:
; 6. Every change this driver makes to a device or endpoint context is already
; visible to the controller by the time the corresponding command completes, so
; a pending synchronization request can always be satisfied at once.
        mov     eax, [esi+usb_controller.WaitPipeRequestAsync]
        mov     [esi+usb_controller.ReadyPipeHeadAsync], eax
        mov     eax, [esi+usb_controller.WaitPipeRequestPeriodic]
        mov     [esi+usb_controller.ReadyPipeHeadPeriodic], eax
; 7. Finalize the transfers that have been completed.
        xor     eax, eax
        xchg    eax, [esi+xhci_controller.NeedScan-XCD]
        test    eax, eax
        jz      @f
        call    xhci_process_updated_schedule
@@:
; 8. Advance the port reset state machine.
.test_reset:
        cmp     [esi+usb_controller.ResettingStatus], 1
        jnz     .no_reset_in_progress
        invoke  GetTimerTicks
        sub     eax, [esi+usb_controller.ResetTime]
        sub     eax, USB_RESET_TIME
        jge     .reset_done
        neg     eax
        cmp     [esp], eax
        jb      .skip_reset
        mov     [esp], eax
        jmp     .skip_reset
.reset_done:
        call    xhci_port_reset_done
.no_reset_in_progress:
        cmp     [esi+usb_controller.ResettingStatus], 0
        jz      .skip_reset
        invoke  GetTimerTicks
        sub     eax, [esi+usb_controller.ResetTime]
        sub     eax, USB_RESET_RECOVERY_TIME
        jge     .reset_recovery_done
        neg     eax
        cmp     [esp], eax
        jb      .skip_reset
        mov     [esp], eax
        jmp     .skip_reset
.reset_recovery_done:
        call    xhci_port_init
        jmp     .test_reset
.skip_reset:
; 9. Process the wait lists.
        invoke  usbhc_api.usb_process_wait_lists
        test    al, (1 shl CONTROL_PIPE) + (1 shl INTERRUPT_PIPE)
        jz      @f
        mov     edx, [esi+usb_controller.WaitPipeListAsync]
        mov     [esi+usb_controller.WaitPipeRequestAsync], edx
        mov     edx, [esi+usb_controller.WaitPipeListPeriodic]
        mov     [esi+usb_controller.WaitPipeRequestPeriodic], edx
        mov     dword [esp], 1          ; come back on the next tick
@@:
; 10. If a descriptor has been completed while this procedure was running, make
; sure it gets processed soon.
        cmp     [esi+xhci_controller.NeedScan-XCD], 0
        jz      @f
        mov     dword [esp], 1
@@:
; 11. Never ask for an unbounded sleep. Without an interrupt line nothing else
; would ever wake the thread, and even with one a missed interrupt would
; otherwise stall the controller forever. When polling is all there is, wake
; on every tick: interrupt endpoints (mice, keyboards) are only served as fast
; as the event ring is drained.
        movi    edx, 200                ; two seconds
        cmp     [esi+xhci_controller.NoIrq-XCD], 0
        jz      @f
        movi    edx, 1
@@:
        cmp     [esp], edx
        jbe     @f
        mov     [esp], edx
@@:
; While a control or bulk transfer is in flight, poll on the next tick anyway:
; a missed or late interrupt then costs 10 ms instead of two seconds. Interrupt
; endpoints are left out on purpose - a mouse always has a transfer queued, so
; polling for it would busy the thread for no gain.
        call    xhci_has_inflight
        test    eax, eax
        jz      @f
        cmp     dword [esp], 1
        jbe     @f
        mov     dword [esp], 1
@@:
; Report the enumeration state, but only when it changes: with per-tick
; polling an unconditional print would flood the log.
        mov     eax, [esi+usb_controller.NewConnected]
        movzx   ecx, [esi+usb_controller.ResettingStatus]
        mov     edx, ecx
        shl     edx, 16
        or      edx, eax
        cmp     edx, [esi+xhci_controller.PassLast-XCD]
        jz      @f
        mov     [esi+xhci_controller.PassLast-XCD], edx
        xhci_trace 'K : XHCI pass: connected=%x state=%x\n', eax, ecx
@@:
        pop     eax
        pop     ecx
        pop     edi ebx
        ret
endp

; Scans all pipes for finished descriptors.
; in: esi -> usb_controller.
proc xhci_process_updated_schedule
        push    ebx edi
        lea     edi, [esi+xhci_controller.ControlED-XCD]
        call    xhci_process_updated_list
        lea     edi, [esi+xhci_controller.BulkED-XCD]
        call    xhci_process_updated_list
        lea     edi, [esi+xhci_controller.IntED-XCD]
        call    xhci_process_updated_list
        pop     edi ebx
        ret
endp

; Tells whether any control or bulk pipe has TRBs the controller has not yet
; reported on, that is, whether a non-periodic transfer is in flight.
; in: esi -> usb_controller. out: eax = 1 if so, 0 otherwise.
proc xhci_has_inflight
        push    ebx edi
        lea     edi, [esi+xhci_controller.ControlED-XCD]
        call    .list
        jnz     .yes
        lea     edi, [esi+xhci_controller.BulkED-XCD]
        call    .list
        jnz     .yes
        xor     eax, eax
        pop     edi ebx
        ret
.yes:
        movi    eax, 1
        pop     edi ebx
        ret
; in: edi -> usb_static_ep. out: ZF clear if some pipe of the list is busy.
.list:
        mov     ebx, [edi+usb_static_ep.NextVirt]
.loop:
        cmp     ebx, edi
        jz      .idle
        mov     eax, [ebx+xhci_pipe.Enqueue-sizeof.xhci_pipe]
        cmp     eax, [ebx+xhci_pipe.Dequeue-sizeof.xhci_pipe]
        jnz     .busy
        mov     ebx, [ebx+usb_pipe.NextVirt]
        jmp     .loop
.idle:
        xor     eax, eax                ; ZF set
.busy:
        ret
endp

; Processes one list of pipes.
; in: esi -> usb_controller, edi -> usb_static_ep.
proc xhci_process_updated_list
        push    ebx
        mov     ebx, [edi+usb_static_ep.NextVirt]
.loop:
        cmp     ebx, edi
        jz      .done
        push    [ebx+usb_static_ep.NextVirt]
        lea     ecx, [ebx+usb_pipe.Lock]
        invoke  MutexLock
        mov     ebx, [ebx+usb_pipe.LastTD]
        push    ebx
        mov     ebx, [ebx+usb_gtd.NextVirt]
.tdloop:
        cmp     ebx, [esp]
        jz      .tddone
        cmp     [ebx+xhci_gtd.Done-sizeof.xhci_gtd], 0
        jz      .tddone
; The callback can enqueue new transfers, so the queue lock has to be released
; while it runs.
        push    ecx
        invoke  MutexUnlock
        call    xhci_process_updated_td
        pop     ecx
        invoke  MutexLock
        jmp     .tdloop
.tddone:
        invoke  MutexUnlock
        pop     ebx                     ; the tail descriptor
        pop     ebx                     ; the next pipe
        jmp     .loop
.done:
        pop     ebx
        ret
endp

; Processes one finished descriptor.
; in: ebx -> usb_gtd. out: ebx -> the next usb_gtd.
proc xhci_process_updated_td
; 1. Remove the descriptor from the queue of its pipe.
        invoke  usbhc_api.usb_unlink_td
; 2. Work out how many bytes have actually been transferred. usb_gtd.Length is
; not simply the size of this descriptor: usb_process_gtd adds the amount
; transferred by every intermediate descriptor of the same stage to the length
; of the following one, so the difference below is the running total over the
; whole stage, which is exactly what the check in step 3 and the callback
; expect.
        mov     edx, [ebx+usb_gtd.Length]
        sub     edx, [ebx+xhci_gtd.Residual-sizeof.xhci_gtd]
        jns     @f
        xor     edx, edx
@@:
; 3. Decide whether anything beyond notifying the driver has to happen.
; A short packet needs special treatment when it is not the last descriptor of
; the stage, or when the caller considers short transfers to be an error.
        xor     ecx, ecx
        mov     eax, [ebx+xhci_gtd.Status-sizeof.xhci_gtd]
        cmp     eax, XHCI_CC_SUCCESS
        jz      .no_error
        cmp     eax, XHCI_CC_SHORT
        jnz     .error
.no_error:
        test    byte [ebx+xhci_gtd.Flags-sizeof.xhci_gtd], 1
        jnz     .notify
        cmp     edx, [ebx+usb_gtd.Length]
        jnz     .special
.notify:
; 4. Report the result and release the descriptor.
        call    xhci_check_hub_descriptor
        invoke  usbhc_api.usb_process_gtd
        push    [ebx+usb_gtd.NextVirt]
        stdcall xhci_free_td, ebx
        pop     ebx
        ret
.error:
        DEBUGF 1,'K : XHCI: transfer failed, completion code %d\n',eax
.special:
; 5. Abort the rest of the transfer: release the intermediate descriptors and
; stop at the one that carries the callback.
        push    edx                     ; the transferred length
        push    eax                     ; the completion code
.look_final:
        invoke  usbhc_api.usb_is_final_packet
        jnc     .found_final
        push    [ebx+usb_gtd.NextVirt]
        stdcall xhci_free_td, ebx
        pop     ebx
        invoke  usbhc_api.usb_unlink_td
        jmp     .look_final
.found_final:
        pop     eax
        cmp     eax, XHCI_CC_SUCCESS
        jz      .short_packet
        cmp     eax, XHCI_CC_SHORT
        jz      .short_packet
        call    xhci_status_from_cc
        mov     ecx, eax
        jmp     .have_status
.short_packet:
; A short packet is an error only when the caller forbids it.
        movi    ecx, USB_STATUS_UNDERRUN
        test    byte [ebx+xhci_gtd.Flags-sizeof.xhci_gtd], 1
        jz      .have_status
        xor     ecx, ecx
        pop     edx
        jmp     .notify
.have_status:
; 6. A control transfer has more than one stage: the descriptor reached here
; can be the last one of the data stage, in which case the status stage
; descriptor has to be dropped as well.
        cmp     [ebx+usb_gtd.Callback], 0
        jnz     .normal
        push    ecx
        push    [ebx+usb_gtd.NextVirt]
        stdcall xhci_free_td, ebx
        pop     ebx
        invoke  usbhc_api.usb_unlink_td
        pop     ecx
.normal:
; 7. Now ebx is the last descriptor of the aborted transfer, so the ring
; must continue right after it: the TRBs of the descriptors just released
; are still in the ring and must be skipped. Also take the endpoint out of the
; Halted state it is left in after an error, and clear the ring positions of
; the released descriptors, whose events will never come.
        push    ecx
        call    xhci_recover_endpoint
        pop     ecx
        pop     edx
        jmp     .notify
endp

; Restarts a halted endpoint after a failed transfer.
; in: ebx -> usb_gtd of the failed descriptor.
proc xhci_recover_endpoint
        pushad
        mov     ebp, ebx                ; ebp -> last usb_gtd of the transfer
        mov     ebx, [ebx+usb_gtd.Pipe]
        mov     esi, [ebx+usb_pipe.Controller]
        sub     ebx, sizeof.xhci_pipe
        cmp     [ebx+xhci_pipe.SlotId], 0
        jz      .nothing
; 1. Work out where the ring has to continue: right after the last descriptor
; of the aborted transfer.
        mov     edi, [ebp+xhci_gtd.NextTrb-sizeof.xhci_gtd]
        sub     edi, [ebx+xhci_pipe.RingPhys]
        shr     edi, 4
        cmp     edi, XHCI_RING_TRBS-1
        jb      @f
        xor     edi, edi
@@:
; 2. Clear the map entries of every TRB that is being skipped: the events for
; those descriptors will never arrive, and the descriptors themselves are
; already released by the caller.
        mov     ecx, [ebx+xhci_pipe.Dequeue]
        mov     edx, [ebx+xhci_pipe.RingMap]
.clear:
        cmp     ecx, edi
        jz      .cleared
        and     dword [edx+ecx*4], 0
        inc     ecx
        cmp     ecx, XHCI_RING_TRBS-1
        jb      .clear
        xor     ecx, ecx
        jmp     .clear
.cleared:
; 3. Reset Endpoint takes the endpoint out of the Halted state that a STALL
; or a transaction error leaves it in. After a plain short packet the
; endpoint is not halted and the command fails with Context State Error,
; which is harmless; a stopped ring is restarted by the doorbell below.
        mov     ecx, [ebx+xhci_pipe.DCI]
        shl     ecx, 16
        mov     edx, [ebx+xhci_pipe.SlotId]
        shl     edx, 24
        or      edx, ecx
        or      edx, XHCI_TRB_RESET_EP shl 10
        xor     eax, eax
        push    ebx
        xor     ebx, ebx
        xor     ecx, ecx
        call    xhci_cmd_sync
        pop     ebx
; 4. Move the dequeue pointer past the aborted transfer, both in software and
; in the Endpoint Context.
        mov     [ebx+xhci_pipe.Dequeue], edi
        push    [ebx+xhci_pipe.Cycle]
        mov     eax, [ebp+xhci_gtd.NextCycle-sizeof.xhci_gtd]
        mov     [ebx+xhci_pipe.Cycle], eax
        call    xhci_set_dequeue
        pop     [ebx+xhci_pipe.Cycle]
; 5. Restart the endpoint, in case further transfers are already queued.
        mov     eax, [ebx+xhci_pipe.SlotId]
        mov     edx, [ebx+xhci_pipe.DCI]
        call    xhci_doorbell
.nothing:
        popad
        ret
endp
