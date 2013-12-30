; Implementation of periodic transaction scheduler for USB.
; Bandwidth dedicated to periodic transactions is limited, so
; different pipes should be scheduled as uniformly as possible.

; USB1 scheduler.
; Algorithm is simple:
; when adding a pipe, optimize the following quantity:
;  * for every millisecond, take all bandwidth scheduled to periodic transfers,
;  * calculate maximum over all milliseconds,
;  * select a variant which minimizes that maximum;
; when removing a pipe, do nothing (except for bookkeeping).

; The caller must provide CONTROLLER_NAME define.
macro define_controller_name name
{
_hci_static_ep.SoftwarePart = name # _static_ep.SoftwarePart
_hci_static_ep.NextList = name # _static_ep.NextList
sizeof._hci_static_ep = sizeof. # name # _static_ep
}

; Select a list for a new pipe.
; in: esi -> usb_controller, maxpacket, type, interval can be found in the stack
; in: ecx = 2 * maximal interval = total number of periodic lists + 1
; in: edx -> {u|o}hci_static_ep for the first list
; in: eax -> byte past {u|o}hci_static_ep for the last list in the first group
; out: edx -> usb_static_ep for the selected list or zero if failed
proc usb1_select_interrupt_list
; inherit some variables from usb_open_pipe
virtual at ebp-12
.speed          db      ?
                rb      3
.bandwidth      dd      ?
.target         dd      ?
                dd      ?
                dd      ?
.config_pipe    dd      ?
.endpoint       dd      ?
.maxpacket      dd      ?
.type           dd      ?
.interval       dd      ?
end virtual
        push    ebx edi         ; save used registers to be stdcall
        push    eax             ; save eax for checks in step 3
; 1. Only intervals 2^k ms can be supported.
; The core specification says that the real interval should not be greater
; than the interval given by the endpoint descriptor, but can be less.
; Determine the actual interval as 2^k ms.
        mov     eax, ecx
; 1a. Set [.interval] to 1 if it was zero; leave it as is otherwise
        cmp     [.interval], 1
        adc     [.interval], 0
; 1b. Divide ecx by two while it is strictly greater than [.interval].
@@:
        shr     ecx, 1
        cmp     [.interval], ecx
        jb      @b
; ecx = the actual interval
;
; For example, let ecx = 8, eax = 64.
; The scheduler space is 32 milliseconds,
; we need to schedule something every 8 ms;
; there are 8 variants: schedule at times 0,8,16,24,
; schedule at times 1,9,17,25,..., schedule at times 7,15,23,31.
; Now concentrate: there are three nested loops,
; * the innermost loop calculates the total periodic bandwidth scheduled
;   in the given millisecond,
; * the intermediate loop calculates the maximum over all milliseconds
;   in the given variant, that is the quantity we're trying to minimize,
; * the outermost loop checks all variants.
; 2. Calculate offset between the first list and the first list for the
; selected interval, in bytes; save in the stack for step 4.
        sub     eax, ecx
        sub     eax, ecx
        imul    eax, sizeof._hci_static_ep
        push    eax
        imul    ebx, ecx, sizeof._hci_static_ep
; 3. Select the best variant.
; 3a. The outermost loop.
; Prepare for the loop: set the current optimal bandwidth to maximum
; possible value (so that any variant will pass the first comparison),
; calculate delta for the intermediate loop.
        or      [.bandwidth], -1
.varloop:
; 3b. The intermediate loop.
; Prepare for the loop: set the maximum to be calculated to zero,
; save counter of the outermost loop.
        xor     edi, edi
        push    edx
virtual at esp
.cur_variant    dd      ?       ; step 3b
.result_delta   dd      ?       ; step 2
.group1_limit   dd      ?       ; function prolog
end virtual
.calc_max_bandwidth:
; 3c. The innermost loop. Sum over all lists.
        xor     eax, eax
        push    edx
.calc_bandwidth:
        add     eax, [edx+_hci_static_ep.SoftwarePart+usb_static_ep.Bandwidth]
        mov     edx, [edx+_hci_static_ep.NextList]
        test    edx, edx
        jnz     .calc_bandwidth
        pop     edx
; 3d. The intermediate loop continued: update maximum.
        cmp     eax, edi
        jb      @f
        mov     edi, eax
@@:
; 3e. The intermediate loop continued: advance counter.
        add     edx, ebx
        cmp     edx, [.group1_limit]
        jb      .calc_max_bandwidth
; 3e. The intermediate loop done: restore counter of the outermost loop.
        pop     edx
; 3f. The outermost loop continued: if the current variant is
; better (maybe not strictly) then the previous optimum, update
; the optimal bandwidth and resulting list.
        cmp     edi, [.bandwidth]
        ja      @f
        mov     [.bandwidth], edi
        mov     [.target], edx
@@:
; 3g. The outermost loop continued: advance counter.
        add     edx, sizeof._hci_static_ep
        dec     ecx
        jnz     .varloop
; 4. Calculate bandwidth for the new pipe.
        mov     eax, [.maxpacket]
        mov     cl, [.speed]
        mov     ch, byte [.endpoint]
        and     ch, 80h
        call    calc_usb1_bandwidth
; 5. Get the pointer to the best list.
        pop     edx             ; restore value from step 2
        pop     ecx             ; purge stack var from prolog
        add     edx, [.target]
; 6. Check that bandwidth for the new pipe plus old bandwidth
; still fits to maximum allowed by the core specification, 90% of 12000 bits.
        mov     ecx, eax
        add     ecx, [.bandwidth]
        cmp     ecx, 10800
        ja      .no_bandwidth
; 7. Convert {o|u}hci_static_ep to usb_static_ep, update bandwidth and return.
        add     edx, _hci_static_ep.SoftwarePart
        add     [edx+usb_static_ep.Bandwidth], eax
        pop     edi ebx         ; restore used registers to be stdcall
        ret
.no_bandwidth:
        dbgstr 'Periodic bandwidth limit reached'
        xor     edx, edx
        pop     edi ebx
        ret
endp

; Pipe is removing, update the corresponding lists.
; We do not reorder anything, so just update book-keeping variable
; in the list header.
proc usb1_interrupt_list_unlink
virtual at esp
                dd      ?       ; return address
.maxpacket      dd      ?
.lowspeed       db      ?
.direction      db      ?
                rb      2
end virtual
; calculate bandwidth on the bus
        mov     eax, [.maxpacket]
        mov     ecx, dword [.lowspeed]
        call    calc_usb1_bandwidth
; find list header
        mov     edx, ebx
@@:
        mov     edx, [edx+usb_pipe.NextVirt]
        cmp     [edx+usb_pipe.Controller], esi
        jz      @b
; subtract pipe bandwidth
        sub     [edx+usb_static_ep.Bandwidth], eax
        ret     8
endp

; Helper procedure for USB1 scheduler: calculate bandwidth on the bus.
; in: low 11 bits of eax = payload size in bytes
; in: cl = 0 - full-speed, nonzero - high-speed
; in: ch = 0 - OUT, nonzero - IN
; out: eax = maximal bandwidth in FS-bits
proc calc_usb1_bandwidth
        and     eax, (1 shl 11) - 1     ; get payload for one transaction
        add     eax, 3  ; add 3 bytes for other fields in data packet, PID+CRC16
        test    cl, cl
        jnz     .low_speed
; Multiply by 8 for bytes -> bits, by 7/6 to accomodate bit stuffing
; and by 401/400 for IN transfers to accomodate timers difference
; 9+107/300 for IN transfers, 9+1/3 for OUT transfers
; For 0 <= eax < 09249355h, floor(eax * 107/300) = floor(eax * 5B4E81B5h / 2^32).
; For 0 <= eax < 80000000h, floor(eax / 3) = floor(eax * 55555556h / 2^32).
        mov     edx, 55555556h
        test    ch, ch
        jz      @f
        mov     edx, 5B4E81B5h
@@:
        lea     ecx, [eax*9]
        mul     edx
; Add 93 extra bits: 39 bits for Token packet (8 for SYNC, 24 for token+address,
; 4 extra bits for possible bit stuffing in token+address, 3 for EOP),
; 18 bits for bus turn-around, 11 bits for SYNC+EOP in Data packet plus 1 bit
; for possible timers difference, 2 bits for inter-packet delay, 20 bits for
; Handshake packet, 2 bits for another inter-packet delay.
        lea     eax, [ecx+edx+93]
        ret
.low_speed:
; Multiply by 8 for bytes -> bits, by 7/6 to accomodate bit stuffing,
; by 8 for LS -> FS and by 406/50 for IN transfers to accomodate timers difference.
; 75+59/75 for IN transfers, 74+2/3 for OUT transfers.
        mov     edx, 0AAAAAABh
        test    ch, ch
        mov     ecx, 74
        jz      @f
        mov     edx, 0C962FC97h
        inc     ecx
@@:
        imul    ecx, eax
        mul     edx
; Add 778 extra bits:
; 16 bits for PRE packet, 4 bits for hub delay, 8*39 bits for Token packet
; 8*18 bits for bus turn-around
; (406/50)*11 bits for SYNC+EOP in Data packet,
; 8*2 bits for inter-packet delay,
; 16 bits for PRE packet, 4 bits for hub delay, 8*20 bits for Handshake packet,
; 8*2 bits for another inter-packet delay.
        lea     eax, [ecx+edx+778]
        ret
endp
