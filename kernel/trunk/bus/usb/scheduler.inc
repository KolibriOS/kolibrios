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

; sanity check: structures in UHCI and OHCI should be the same
if (sizeof.ohci_static_ep=sizeof.uhci_static_ep)&(ohci_static_ep.SoftwarePart=uhci_static_ep.SoftwarePart)&(ohci_static_ep.NextList=uhci_static_ep.NextList)
; Select a list for a new pipe.
; in: esi -> usb_controller, maxpacket, type, interval can be found in the stack
; in: ecx = 2 * maximal interval = total number of periodic lists + 1
; in: edx -> {u|o}hci_static_ep for the first list
; in: eax -> byte past {u|o}hci_static_ep for the last list in the first group
; out: edx -> usb_static_ep for the selected list or zero if failed
proc usb1_select_interrupt_list
; inherit some variables from usb_open_pipe
virtual at ebp-8
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
        imul    eax, sizeof.ohci_static_ep
        push    eax
        imul    ebx, ecx, sizeof.ohci_static_ep
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
        add     eax, [edx+ohci_static_ep.SoftwarePart+usb_static_ep.Bandwidth]
        mov     edx, [edx+ohci_static_ep.NextList]
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
        add     edx, sizeof.ohci_static_ep
        dec     ecx
        jnz     .varloop
; 4. Get the pointer to the best list.
        pop     edx             ; restore value from step 2
        pop     eax             ; purge stack var from prolog
        add     edx, [.target]
; 5. Calculate bandwidth for the new pipe.
        mov     eax, [.maxpacket]       ; TODO: calculate real bandwidth
        and     eax, (1 shl 11) - 1
; 6. TODO: check that bandwidth for the new pipe plus old bandwidth
; still fits to maximum allowed by the core specification.
; 7. Convert {o|u}hci_static_ep to usb_static_ep, update bandwidth and return.
        add     edx, ohci_static_ep.SoftwarePart
        add     [edx+usb_static_ep.Bandwidth], eax
        pop     edi ebx         ; restore used registers to be stdcall
        ret
endp
; sanity check, part 2
else
.err select_interrupt_list must be different for UHCI and OHCI
end if

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
; find list header
        mov     edx, ebx
@@:
        mov     edx, [edx+usb_pipe.NextVirt]
        cmp     [edx+usb_pipe.Controller], esi
        jnz     @b
; subtract pipe bandwidth
; TODO: calculate real bandwidth
        mov     eax, [.maxpacket]
        and     eax, (1 shl 11) - 1
        sub     [edx+usb_static_ep.Bandwidth], eax
        ret     8
endp

; USB2 scheduler.
; There are two parts: high-speed pipes and split-transaction pipes.
; Split-transaction scheduler is currently a stub.
; High-speed scheduler uses the same algorithm as USB1 scheduler:
; when adding a pipe, optimize the following quantity:
;  * for every microframe, take all bandwidth scheduled to periodic transfers,
;  * calculate maximum over all microframe,
;  * select a variant which minimizes that maximum;
; when removing a pipe, do nothing (except for bookkeeping).
; in: esi -> usb_controller
; out: edx -> usb_static_ep, eax = S-Mask
proc ehci_select_hs_interrupt_list
; inherit some variables from usb_open_pipe
virtual at ebp-12
.targetsmask    dd      ?
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
; prolog, initialize local vars
        or      [.bandwidth], -1
        or      [.target], -1
        or      [.targetsmask], -1
        push    ebx edi         ; save used registers to be stdcall
; 1. In EHCI, every list describes one millisecond = 8 microframes.
; Thus, there are two significantly different branches:
; for pipes with interval >= 8 microframes, advance to 2,
; for pipes which should be planned in every frame (one or more microframes),
; go to 9.
; Note: the actual interval for high-speed devices is 2^([.interval]-1),
; (the core specification forbids [.interval] == 0)
        mov     ecx, [.interval]
        dec     ecx
        cmp     ecx, 3
        jb      .every_frame
; 2. Determine the actual interval in milliseconds.
        sub     ecx, 3
        cmp     ecx, 5  ; maximum 32ms
        jbe     @f
        movi    ecx, 5
@@:
; There are four nested loops,
; * Loop #4 (the innermost one) calculates the total periodic bandwidth
;   scheduled in the given microframe of the given millisecond.
; * Loop #3 calculates the maximum over all milliseconds
;   in the given variant, that is the quantity we're trying to minimize.
; * Loops #1 and #2 check all variants;
;   loop #1 is responsible for the target millisecond,
;   loop #2 is responsible for the microframe within millisecond.
; 3. Prepare for loops.
; ebx = number of iterations of loop #1
; [esp] = delta of counter for loop #3, in bytes
; [esp+4] = delta between the first group and the target group, in bytes
        movi    ebx, 1
        movi    edx, sizeof.ehci_static_ep
        shl     ebx, cl
        shl     edx, cl
        mov     eax, 64*sizeof.ehci_static_ep
        sub     eax, edx
        sub     eax, edx
        push    eax
        push    edx
; 4. Select the best variant.
; 4a. Loop #1: initialize counter = pointer to ehci_static_ep for
; the target millisecond in the first group.
        lea     edx, [esi+ehci_controller.IntEDs-sizeof.ehci_controller]
.varloop0:
; 4b. Loop #2: initialize counter = microframe within the target millisecond.
        xor     ecx, ecx
.varloop:
; 4c. Loop #3: save counter of loop #1,
; initialize counter with the value of loop #1 counter,
; initialize maximal bandwidth = zero.
        xor     edi, edi
        push    edx
virtual at esp
.saved_counter1         dd      ?       ; step 4c
.loop3_delta            dd      ?       ; step 3
.target_delta           dd      ?       ; step 3
end virtual
.calc_max_bandwidth:
; 4d. Loop #4: initialize counter with the value of loop #3 counter,
; initialize total bandwidth = zero.
        xor     eax, eax
        push    edx
.calc_bandwidth:
; 4e. Loop #4: add the bandwidth from the current list
; and advance to the next list, while there is one.
        add     ax, [edx+ehci_static_ep.Bandwidths+ecx*2]
        mov     edx, [edx+ehci_static_ep.NextList]
        test    edx, edx
        jnz     .calc_bandwidth
; 4f. Loop #4 end: restore counter of loop #3.
        pop     edx
; 4g. Loop #3: update maximal bandwidth.
        cmp     eax, edi
        jb      @f
        mov     edi, eax
@@:
; 4h. Loop #3: advance the counter and repeat while within the first group.
        lea     eax, [esi+ehci_controller.IntEDs+32*sizeof.ehci_static_ep-sizeof.ehci_controller]
        add     edx, [.loop3_delta]
        cmp     edx, eax
        jb      .calc_max_bandwidth
; 4i. Loop #3 end: restore counter of loop #1.
        pop     edx
; 4j. Loop #2: if the current variant is better (maybe not strictly)
; then the previous optimum, update the optimal bandwidth and the target.
        cmp     edi, [.bandwidth]
        ja      @f
        mov     [.bandwidth], edi
        mov     [.target], edx
        movi    eax, 1
        shl     eax, cl
        mov     [.targetsmask], eax
@@:
; 4k. Loop #2: continue 8 times for every microframe.
        inc     ecx
        cmp     ecx, 8
        jb      .varloop
; 4l. Loop #1: advance counter and repeat ebx times,
; ebx was calculated in step 3.
        add     edx, sizeof.ehci_static_ep
        dec     ebx
        jnz     .varloop0
; 5. Get the pointer to the best list.
        pop     edx             ; restore value from step 3
        pop     edx             ; get delta calculated in step 3
        add     edx, [.target]
; 6. Calculate bandwidth for the new pipe.
; TODO1: calculate real bandwidth
        mov     eax, [.maxpacket]
        mov     ecx, eax
        and     eax, (1 shl 11) - 1
        shr     ecx, 11
        inc     ecx
        and     ecx, 3
        imul    eax, ecx
; 7. TODO2: check that bandwidth for the new pipe plus old bandwidth
; still fits to maximum allowed by the core specification
; current [.bandwidth] + new bandwidth <= limit;
; USB2 specification allows maximum 60000*80% bit times for periodic microframe
; 8. Convert {o|u}hci_static_ep to usb_static_ep, update bandwidth and return.
        mov     ecx, [.targetsmask]
        add     [edx+ehci_static_ep.Bandwidths+ecx*2], ax
        add     edx, ehci_static_ep.SoftwarePart
        movi    eax, 1
        shl     eax, cl
        pop     edi ebx         ; restore used registers to be stdcall
        ret
.every_frame:
; The pipe should be scheduled every frame in two or more microframes.
; 9. Calculate maximal bandwidth for every microframe: three nested loops.
; 9a. The outermost loop: ebx = microframe to calculate.
        xor     ebx, ebx
.calc_all_bandwidths:
; 9b. The intermediate loop:
; edx = pointer to ehci_static_ep in the first group, [esp] = counter,
; edi = maximal bandwidth
        lea     edx, [esi+ehci_controller.IntEDs-sizeof.ehci_controller]
        xor     edi, edi
        push    32
.calc_max_bandwidth2:
; 9c. The innermost loop: calculate bandwidth for the given microframe
; in the given frame.
        xor     eax, eax
        push    edx
.calc_bandwidth2:
        add     ax, [edx+ehci_static_ep.Bandwidths+ebx*2]
        mov     edx, [edx+ehci_static_ep.NextList]
        test    edx, edx
        jnz     .calc_bandwidth2
        pop     edx
; 9d. The intermediate loop continued: update maximal bandwidth.
        cmp     eax, edi
        jb      @f
        mov     edi, eax
@@:
        add     edx, sizeof.ehci_static_ep
        dec     dword [esp]
        jnz     .calc_max_bandwidth2
        pop     eax
; 9e. Push the calculated maximal bandwidth and continue the outermost loop.
        push    edi
        inc     ebx
        cmp     ebx, 8
        jb      .calc_all_bandwidths
virtual at esp
.bandwidth7     dd      ?
.bandwidth6     dd      ?
.bandwidth5     dd      ?
.bandwidth4     dd      ?
.bandwidth3     dd      ?
.bandwidth2     dd      ?
.bandwidth1     dd      ?
.bandwidth0     dd      ?
end virtual
; 10. Select the best variant.
; edx = S-Mask = bitmask of scheduled microframes
        movi    edx, 0x11
        cmp     ecx, 1
        ja      @f
        mov     dl, 0x55
        jz      @f
        mov     dl, 0xFF
@@:
; try all variants edx, edx shl 1, edx shl 2, ...
; until they fit in the lower byte (8 microframes per frame)
.select_best_mframe:
        xor     edi, edi
        mov     ecx, edx
        mov     eax, esp
.calc_mframe:
        add     cl, cl
        jnc     @f
        cmp     edi, [eax]
        jae     @f
        mov     edi, [eax]
@@:
        add     eax, 4
        test    cl, cl
        jnz     .calc_mframe
        cmp     [.bandwidth], edi
        jb      @f
        mov     [.bandwidth], edi
        mov     [.targetsmask], edx
@@:
        add     dl, dl
        jnc     .select_best_mframe
; 11. Restore stack after step 9.
        add     esp, 8*4
; 12. Get the pointer to the target list (responsible for every microframe).
        lea     edx, [esi+ehci_controller.IntEDs.SoftwarePart+62*sizeof.ehci_static_ep-sizeof.ehci_controller]
; 13. TODO1: calculate real bandwidth.
        mov     eax, [.maxpacket]
        mov     ecx, eax
        and     eax, (1 shl 11) - 1
        shr     ecx, 11
        inc     ecx
        and     ecx, 3
        imul    eax, ecx
; 14. TODO2: check that current [.bandwidth] + new bandwidth <= limit;
; USB2 specification allows maximum 60000*80% bit times for periodic microframe.
; Update bandwidths including the new pipe.
        mov     ecx, [.targetsmask]
        lea     edi, [edx+ehci_static_ep.Bandwidths-ehci_static_ep.SoftwarePart]
.update_bandwidths:
        shr     ecx, 1
        jnc     @f
        add     [edi], ax
@@:
        add     edi, 2
        test    ecx, ecx
        jnz     .update_bandwidths
; 15. Return target list and target S-Mask.
        mov     eax, [.targetsmask]
        pop     edi ebx         ; restore used registers to be stdcall
        ret
endp

; Pipe is removing, update the corresponding lists.
; We do not reorder anything, so just update book-keeping variable
; in the list header.
proc ehci_hs_interrupt_list_unlink
; get target list
        mov     edx, [ebx+ehci_pipe.BaseList-ehci_pipe.SoftwarePart]
; TODO: calculate real bandwidth
        movzx   eax, word [ebx+ehci_pipe.Token-ehci_pipe.SoftwarePart+2]
        mov     ecx, [ebx+ehci_pipe.Flags-ehci_pipe.SoftwarePart]
        and     eax, (1 shl 11) - 1
        shr     ecx, 30
        imul    eax, ecx
        movzx   ecx, byte [ebx+ehci_pipe.Flags-ehci_pipe.SoftwarePart]
        add     edx, ehci_static_ep.Bandwidths - ehci_static_ep.SoftwarePart
; update bandwidth
.dec_bandwidth:
        shr     ecx, 1
        jnc     @f
        sub     [edx], ax
@@:
        add     edx, 2
        test    ecx, ecx
        jnz     .dec_bandwidth
; return
        ret
endp

uglobal
ehci_last_fs_alloc      dd      ?
endg

; This needs to be rewritten. Seriously.
; It schedules everything to the first microframe of some frame,
; frame is spinned out of thin air.
; This works while you have one keyboard and one mouse...
; maybe even ten keyboards and ten mice... but give any serious stress,
; and this would break.
proc ehci_select_fs_interrupt_list
virtual at ebp-12
.targetsmask    dd      ?
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
        cmp     [.interval], 1
        adc     [.interval], 0
        mov     ecx, 64
        mov     eax, ecx
@@:
        shr     ecx, 1
        cmp     [.interval], ecx
        jb      @b
        sub     eax, ecx
        sub     eax, ecx
        dec     ecx
        and     ecx, [ehci_last_fs_alloc]
        inc     [ehci_last_fs_alloc]
        add     eax, ecx
        imul    eax, sizeof.ehci_static_ep
        lea     edx, [esi+ehci_controller.IntEDs.SoftwarePart+eax-sizeof.ehci_controller]
        mov     ax, 1C01h
        ret
endp

proc ehci_fs_interrupt_list_unlink
        ret
endp
