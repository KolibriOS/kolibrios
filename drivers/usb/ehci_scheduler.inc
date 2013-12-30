; Implementation of periodic transaction scheduler for USB.
; Bandwidth dedicated to periodic transactions is limited, so
; different pipes should be scheduled as uniformly as possible.

; USB2 scheduler.
; There are two parts: high-speed pipes and split-transaction pipes.
;
; High-speed scheduler uses the same algorithm as USB1 scheduler:
; when adding a pipe, optimize the following quantity:
;  * for every microframe, take all bandwidth scheduled to periodic transfers,
;  * calculate maximum over all microframes,
;  * select a variant which minimizes that maximum;
;  * if there are several such variants,
;    prefer those that are closer to end of frame
;    to minimize collisions with split transactions;
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
        jb      .update
        cmp     ecx, [.targetsmask]
        jb      @f
.update:
        mov     [.bandwidth], edi
        mov     [.target], edx
        mov     [.targetsmask], ecx
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
; 5. Calculate bandwidth for the new pipe.
        mov     eax, [.maxpacket]
        call    calc_hs_bandwidth
        mov     ecx, [.maxpacket]
        shr     ecx, 11
        inc     ecx
        and     ecx, 3
        imul    eax, ecx
; 6. Get the pointer to the best list.
        pop     edx             ; restore value from step 3
        pop     edx             ; get delta calculated in step 3
        add     edx, [.target]
; 7. Check that bandwidth for the new pipe plus old bandwidth
; still fits to maximum allowed by the core specification
; current [.bandwidth] + new bandwidth <= limit;
; USB2 specification allows maximum 60000*80% bit times for periodic microframe
        mov     ecx, [.bandwidth]
        add     ecx, eax
        cmp     ecx, 48000
        ja      .no_bandwidth
; 8. Convert {o|u}hci_static_ep to usb_static_ep, update bandwidth and return.
        mov     ecx, [.targetsmask]
        add     [edx+ehci_static_ep.Bandwidths+ecx*2], ax
        add     edx, ehci_static_ep.SoftwarePart
        movi    eax, 1
        shl     eax, cl
        pop     edi ebx         ; restore used registers to be stdcall
        ret
.no_bandwidth:
        dbgstr 'Periodic bandwidth limit reached'
        xor     eax, eax
        xor     edx, edx
        pop     edi ebx
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
; while they fit in the lower byte (8 microframes per frame)
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
; 13. Calculate bandwidth on the bus.
        mov     eax, [.maxpacket]
        call    calc_hs_bandwidth
        mov     ecx, [.maxpacket]
        shr     ecx, 11
        inc     ecx
        and     ecx, 3
        imul    eax, ecx
; 14. Check that current [.bandwidth] + new bandwidth <= limit;
; USB2 specification allows maximum 60000*80% bit times for periodic microframe.
        mov     ecx, [.bandwidth]
        add     ecx, eax
        cmp     ecx, 48000
        ja      .no_bandwidth
; 15. Update bandwidths including the new pipe.
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
; 16. Return target list and target S-Mask.
        mov     eax, [.targetsmask]
        pop     edi ebx         ; restore used registers to be stdcall
        ret
endp

; Pipe is removing, update the corresponding lists.
; We do not reorder anything, so just update book-keeping variable
; in the list header.
proc ehci_hs_interrupt_list_unlink
        movzx   eax, word [ebx+ehci_pipe.Token-sizeof.ehci_pipe+2]
; calculate bandwidth
        call    calc_hs_bandwidth
        mov     ecx, [ebx+ehci_pipe.Flags-sizeof.ehci_pipe]
        shr     ecx, 30
        imul    eax, ecx
        movzx   ecx, byte [ebx+ehci_pipe.Flags-sizeof.ehci_pipe]
; get target list
        mov     edx, [ebx+ehci_pipe.BaseList-sizeof.ehci_pipe]
; update bandwidth
.dec_bandwidth:
        shr     ecx, 1
        jnc     @f
        sub     word [edx+ehci_static_ep.Bandwidths - ehci_static_ep.SoftwarePart], ax
@@:
        add     edx, 2
        test    ecx, ecx
        jnz     .dec_bandwidth
; return
        ret
endp

; Helper procedure for USB2 scheduler: calculate bandwidth on the bus.
; in: low 11 bits of eax = payload size in bytes
; out: eax = maximal bandwidth in HS-bits
proc calc_hs_bandwidth
        and     eax, (1 shl 11) - 1     ; get payload for one transaction
        add     eax, 3  ; add 3 bytes for other fields in data packet, PID+CRC16
; Multiply by 8 for bytes -> bits and then by 7/6 to accomodate bit stuffing;
; total 28/3 = 9+1/3
        mov     edx, 55555556h
        lea     ecx, [eax*9]
        mul     edx
; Add 989 extra bits: 68 bits for Token packet (32 for SYNC, 24 for token+address,
; 4 extra bits for possible bit stuffing in token+address, 8 for EOP),
; 736 bits for bus turn-around, 40 bits for SYNC+EOP in Data packet,
; 8 bits for inter-packet delay, 49 bits for Handshake packet,
; 88 bits for another inter-packet delay.
        lea     eax, [ecx+edx+989]
        ret
endp

; Split-transaction scheduler (aka TT scheduler, TT stands for Transaction
; Translator, section 11.14 of the core spec) needs to schedule three event
; types on two buses: Start-Split and Complete-Split on HS bus and normal
; transaction on FS/LS bus.
; Assume that FS/LS bus is more restricted and more important to be scheduled
; uniformly, so select the variant which minimizes maximal used bandwidth
; on FS/LS bus and does not overflow HS bus.
; If there are several such variants, prefer variants which is closest to
; start of frame, and within the same microframe consider HS bandwidth
; utilization as a last criteria.

; The procedure ehci_select_tt_interrupt_list has been splitted into several
; macro, each representing a logical step of the procedure,
; to simplify understanding what is going on. Consider all the following macro
; as logical parts of one procedure, they are meaningless outside the context.

; Given a frame, calculate bandwidth occupied by already opened pipes
; in every microframe.
; Look for both HS and FS/LS buses: there are 16 words of information,
; 8 for HS bus, 8 for FS/LS bus, for every microframe.
; Since we count already opened pipes, the total bandwidth in every microframe
; is less than 60000 bits (and even 60000*80% bits), otherwise the scheduler
; would not allow to open those pipes.
; edi -> first list for the frame
macro tt_calc_bandwidth_in_frame
{
local .lists, .pipes, .pipes_done, .carry
; 1. Zero everything.
        xor     eax, eax
        mov     edx, edi
repeat 4
        mov     dword [.budget+(%-1)*4], eax
end repeat
repeat 4
        mov     dword [.hs_bandwidth+(%-1)*4], eax
end repeat
        mov     [.total_budget], ax
; Loop over all lists for the given frame.
.lists:
; 2. Total HS bandwidth for all pipes in one list is kept inside list header,
; add it. Note that overflow is impossible, so we may add entire dwords.
        mov     ebx, [edx+ehci_static_ep.SoftwarePart+usb_static_ep.NextVirt]
repeat 4
        mov     eax, dword [edx+ehci_static_ep.Bandwidths+(%-1)*4]
        add     dword [.hs_bandwidth+(%-1)*4], eax
end repeat
; Loop over all pipes in the given list.
        add     edx, ehci_static_ep.SoftwarePart
.pipes:
        cmp     ebx, edx
        jz      .pipes_done
; 3. For every pipe in every list for the given frame:
; 3a. Check whether the pipe resides on the same FS/LS bus as the new pipe.
; If not, skip this pipe.
        mov     eax, [ebx+usb_pipe.DeviceData]
        mov     eax, [eax+usb_device_data.TTHub]
        cmp     eax, [.tthub]
        jnz     @f
; 3b. Calculate FS/LS budget for the opened pipe.
; Note that eax = TTHub after 3a.
        call    tt_calc_budget
; 3c. Update total budget: add the value from 3b
; to the budget of the first microframe scheduled for this pipe.
        bsf     ecx, [ebx+ehci_pipe.Flags-sizeof.ehci_pipe]
        add     [.budget+ecx*2], ax
@@:
        mov     ebx, [ebx+usb_pipe.NextVirt]
        jmp     .pipes
.pipes_done:
        mov     edx, [edx+ehci_static_ep.NextList-ehci_static_ep.SoftwarePart]
        test    edx, edx
        jnz     .lists
; 4. If the budget for some microframe is exceeded, carry it to the following
; microframe(s). The actual size of one microframe is 187.5 raw bytes;
; the core spec says that 188 bytes should be scheduled in every microframe.
        xor     eax, eax
        xor     ecx, ecx
.carry:
        xor     edx, edx
        add     ax, [.budget+ecx*2]
        cmp     ax, 188
        jbe     @f
        mov     dx, ax
        mov     ax, 188
        sub     dx, ax
@@:
        mov     [.budget+ecx*2], ax
        add     [.total_budget], ax
        mov     ax, dx
        inc     ecx
        cmp     ecx, 8
        jb      .carry
}

; Checks whether the new pipe fits in the existing FS budget
; starting from the given microframe. If not, mark the microframe
; as impossible for scheduling.
; in: ecx = microframe
macro tt_exclude_microframe_if_no_budget
{
local .loop, .good, .bad
; 1. If the new budget plus the current budget does not exceed 188 bytes,
; the variant is possible.
        mov     ax, [.budget+ecx*2]
        mov     edx, ecx
        add     ax, [.new_budget]
        sub     ax, 188
        jbe     .good
; 2. Otherwise,
; a) nothing should be scheduled in some following microframes,
; b) after adding the new budget everything should fit in first 6 microframes,
;    this guarantees that even in the worst case 90% limit is satisfied.
.loop:
        cmp     edx, 5
        jae     .bad
        cmp     [.budget+(edx+1)*2], 0
        jnz     .bad
        inc     edx
        sub     ax, 188
        ja      .loop
.bad:
        btr     [.possible_microframes], ecx
.good:
}

; Calculate data corresponding to the particular scheduling variant for the new pipe.
; Data describe the current scheduling state collected over all frames touched
; by the given variant: maximal HS bandwidth, maximal FS/LS budget,
; which microframes fit in the current FS/LS budget for all frames.
macro tt_calc_statistics_for_one_variant
{
local .frames, .microframes
; 1. Initialize: zero maximal bandwidth,
; first 6 microframes are possible for scheduling.
        xor     eax, eax
repeat 4
        mov     dword [.max_hs_bandwidth+(%-1)*4], eax
end repeat
        mov     [.max_fs_bandwidth], ax
        mov     [.possible_microframes], 0x3F
; Loop over all frames starting with [.variant] advancing by [.variant_delta].
        mov     edi, [.variant]
.frames:
; 2. Calculate statistics for one frame.
        tt_calc_bandwidth_in_frame
; 3. Update maximal FS budget.
        mov     ax, [.total_budget]
        cmp     ax, [.max_fs_bandwidth]
        jb      @f
        mov     [.max_fs_bandwidth], ax
@@:
; 4. For every microframe, update maximal HS bandwidth
; and check whether the microframe is allowed for scheduling.
        xor     ecx, ecx
.microframes:
        mov     ax, [.hs_bandwidth+ecx*2]
        cmp     ax, [.max_hs_bandwidth+ecx*2]
        jb      @f
        mov     [.max_hs_bandwidth+ecx*2], ax
@@:
        tt_exclude_microframe_if_no_budget
        inc     ecx
        cmp     ecx, 8
        jb      .microframes
; Stop loop when outside of first descriptor group.
        lea     eax, [esi+ehci_controller.IntEDs+32*sizeof.ehci_static_ep-sizeof.ehci_controller]
        add     edi, [.variant_delta]
        cmp     edi, eax
        jb      .frames
}

struct usb_split_info
microframe_mask         dd      ?       ; lower byte is S-mask, second byte is C-mask
ssplit_bandwidth        dd      ?
csplit_bandwidth        dd      ?
ends

; Check whether the current variant and the current microframe are allowed
; for scheduling. If so, check whether they are better than the previously
; selected variant+microframe, if any. If so, update the previously selected
; variant+microframe to current ones.
; ecx = microframe, [.variant] = variant
macro tt_check_variant_microframe
{
local .nothing, .update, .ssplit, .csplit, .csplit_done
; 1. If the current microframe does not fit in existing FS budget, do nothing.
        bt      [.possible_microframes], ecx
        jnc     .nothing
; 2. Calculate maximal HS bandwidth over all affected microframes.
; 2a. Start-split phase: one or more microframes starting with ecx,
; coded in lower byte of .info.microframe_mask.
        xor     ebx, ebx
        xor     edx, edx
.ssplit:
        lea     eax, [ecx+edx]
        movzx   eax, [.max_hs_bandwidth+eax*2]
        add     eax, [.info.ssplit_bandwidth]
        cmp     ebx, eax
        ja      @f
        mov     ebx, eax
@@:
        inc     edx
        bt      [.info.microframe_mask], edx
        jc      .ssplit
; 2b. Complete-split phase: zero or more microframes starting with
; ecx+(last start-split microframe)+2,
; coded in second byte of .info.microframe_mask.
        add     edx, 8
.csplit:
        inc     edx
        bt      [.info.microframe_mask], edx
        jnc     .csplit_done
        lea     eax, [ecx+edx]
        cmp     eax, 8
        jae     .csplit_done
        movzx   eax, [.max_hs_bandwidth+(eax-8)*2]
        add     eax, [.info.csplit_bandwidth]
        cmp     ebx, eax
        ja      .csplit
        mov     ebx, eax
        jmp     .csplit
.csplit_done:
; 3. Check that current HS bandwidth + new bandwidth <= limit;
; USB2 specification allows maximum 60000*80% bit times for periodic microframe.
        cmp     ebx, 48000
        ja      .nothing
; 4. This variant is possible for scheduling.
; Check whether it is better than the currently selected one.
; 4a. The primary criteria: FS/LS bandwidth.
        mov     ax, [.max_fs_bandwidth]
        cmp     ax, [.best_fs_bandwidth]
        ja      .nothing
        jb      .update
; 4b. The secondary criteria: prefer microframes which are closer to start of frame.
        cmp     ecx, [.targetsmask]
        ja      .nothing
        jb      .update
; 4c. The last criteria: HS bandwidth.
        cmp     ebx, [.bandwidth]
        ja      .nothing
.update:
; 5. This variant is better than the previously selected.
; Update the best variant with current data.
        mov     [.best_fs_bandwidth], ax
        mov     [.bandwidth], ebx
        mov     [.targetsmask], ecx
        mov     eax, [.variant]
        mov     [.target], eax
.nothing:
}

; TT scheduler: add new pipe.
; in: esi -> usb_controller, edi -> usb_pipe
; out: edx -> usb_static_ep, eax = S-Mask
proc ehci_select_tt_interrupt_list
virtual at ebp-12-.local_vars_size
.local_vars_start:
.info                   usb_split_info
.new_budget             dw      ?
.total_budget           dw      ?
.possible_microframes   dd      ?
.tthub                  dd      ?
.budget                 rw      8
.hs_bandwidth           rw      8
.max_hs_bandwidth       rw      8
.max_fs_bandwidth       dw      ?
.best_fs_bandwidth      dw      ?
.variant                dd      ?
.variant_delta          dd      ?
.target_delta           dd      ?
.local_vars_size = $ - .local_vars_start
if .local_vars_size > 24*4
err Modify stack frame size in 
end if

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
        mov     eax, [edi+ehci_pipe.Token-sizeof.ehci_pipe]
        shr     eax, 16
        and     eax, (1 shl 11) - 1
        push    ebx edi
; 1. Compute the real interval. FS/LS devices encode the interval as
; number of milliseconds. Use the maximal power of two that is not greater than
; the given interval and EHCI scheduling area = 32 frames.
        cmp     [.interval], 1
        adc     [.interval], 0
        mov     ecx, 64
        mov     eax, 64 * sizeof.ehci_static_ep
@@:
        shr     ecx, 1
        cmp     [.interval], ecx
        jb      @b
        mov     [.interval], ecx
; 2. Compute variables for further calculations.
; 2a. [.variant_delta] is delta between two lists from the first group
; that correspond to the same variant.
        imul    ecx, sizeof.ehci_static_ep
        mov     [.variant_delta], ecx
; 2b. [.target_delta] is delta between the final answer from the group
; corresponding to [.interval] and the item from the first group.
        sub     eax, ecx
        sub     eax, ecx
        mov     [.target_delta], eax
; 2c. [.variant] is the first list from the first group that corresponds
; to the current variant.
        lea     eax, [esi+ehci_controller.IntEDs-sizeof.ehci_controller]
        mov     [.variant], eax
; 2d. [.tthub] identifies TT hub for new pipe, [.new_budget] is FS budget
; for new pipe.
        mov     eax, [edi+usb_pipe.DeviceData]
        mov     eax, [eax+usb_device_data.TTHub]
        mov     ebx, edi
        mov     [.tthub], eax
        call    tt_calc_budget
        mov     [.new_budget], ax
; 2e. [.usb_split_info] describes bandwidth used by new pipe on HS bus.
        lea     edi, [.info]
        call    tt_fill_split_info
        test    eax, eax
        jz      .no_bandwidth
; 2f. There is no best variant yet, put maximal possible values,
; so any variant would be better than the "current".
        or      [.best_fs_bandwidth], -1
        or      [.target], -1
        or      [.bandwidth], -1
        or      [.targetsmask], -1
; 3. Loop over all variants, for every variant decide whether it is acceptable,
; select the best variant from all acceptable variants.
.check_variants:
        tt_calc_statistics_for_one_variant
        xor     ecx, ecx
.check_microframes:
        tt_check_variant_microframe
        inc     ecx
        cmp     ecx, 6
        jb      .check_microframes
        add     [.variant], sizeof.ehci_static_ep
        dec     [.interval]
        jnz     .check_variants
; 4. If there is no acceptable variants, return error.
        mov     ecx, [.targetsmask]
        mov     edx, [.target]
        cmp     ecx, -1
        jz      .no_bandwidth
; 5. Calculate the answer: edx -> selected list, eax = S-Mask and C-Mask.
        mov     eax, [.info.microframe_mask]
        add     edx, [.target_delta]
        shl     eax, cl
        and     eax, 0xFFFF
; 6. Update HS bandwidths in the selected list.
        xor     ecx, ecx
        mov     ebx, [.info.ssplit_bandwidth]
.update_ssplit:
        bt      eax, ecx
        jnc     @f
        add     [edx+ehci_static_ep.Bandwidths+ecx*2], bx
@@:
        inc     ecx
        cmp     ecx, 8
        jb      .update_ssplit
        mov     ebx, [.info.csplit_bandwidth]
.update_csplit:
        bt      eax, ecx
        jnc     @f
        add     [edx+ehci_static_ep.Bandwidths+(ecx-8)*2], bx
@@:
        inc     ecx
        cmp     ecx, 16
        jb      .update_csplit
; 7. Return.
        add     edx, ehci_static_ep.SoftwarePart
        pop     edi ebx
        ret
.no_bandwidth:
        dbgstr 'Periodic bandwidth limit reached'
        xor     eax, eax
        xor     edx, edx
        pop     edi ebx
        ret
endp

; Pipe is removing, update the corresponding lists.
; We do not reorder anything, so just update book-keeping variable
; in the list header.
proc ehci_fs_interrupt_list_unlink
; calculate bandwidth
        push    edi
        sub     esp, sizeof.usb_split_info
        mov     edi, esp
        call    tt_fill_split_info
; get target list
        mov     edx, [ebx+ehci_pipe.BaseList-sizeof.ehci_pipe]
; update bandwidth for Start-Split
        mov     eax, [edi+usb_split_info.ssplit_bandwidth]
        xor     ecx, ecx
.dec_bandwidth_1:
        bt      [ebx+ehci_pipe.Flags-sizeof.ehci_pipe], ecx
        jnc     @f
        sub     word [edx+ecx*2+ehci_static_ep.Bandwidths - ehci_static_ep.SoftwarePart], ax
@@:
        inc     ecx
        cmp     ecx, 8
        jb      .dec_bandwidth_1
; update bandwidth for Complete-Split
        mov     eax, [edi+usb_split_info.csplit_bandwidth]
.dec_bandwidth_2:
        bt      [ebx+ehci_pipe.Flags-sizeof.ehci_pipe], ecx
        jnc     @f
        sub     word [edx+(ecx-8)*2+ehci_static_ep.Bandwidths - ehci_static_ep.SoftwarePart], ax
@@:
        inc     ecx
        cmp     ecx, 16
        jb      .dec_bandwidth_2
        add     esp, sizeof.usb_split_info
        pop     edi
        ret
endp

; Helper procedure for ehci_select_tt_interrupt_list.
; Calculates "best-case budget" according to the core spec,
; that is, number of bytes (not bits) corresponding to "optimistic" transaction
; time, including inter-packet delays/bus turn-around time,
; but without bit stuffing and timers drift.
; One extra TT-specific delay is added: TT think time from the hub descriptor.
; Similar to calc_usb1_bandwidth with corresponding changes.
; eax -> usb_hub with TT, ebx -> usb_pipe
proc tt_calc_budget
        invoke  usbhc_api.usb_get_tt_think_time ; ecx = TT think time in FS-bytes
        mov     eax, [ebx+ehci_pipe.Token-sizeof.ehci_pipe]
        shr     eax, 16
        and     eax, (1 shl 11) - 1     ; get data length
        bt      [ebx+ehci_pipe.Token-sizeof.ehci_pipe], 12
        jc      .low_speed
; Full-speed interrupt IN/OUT:
; 33 bits for Token packet (8 for SYNC, 24 for token+address, 3 for EOP),
; 18 bits for bus turn-around, 11 bits for SYNC+EOP in Data packet,
; 2 bits for inter-packet delay, 19 bits for Handshake packet,
; 2 bits for another inter-packet delay. 85 bits total, pad to 11 bytes.
        lea     eax, [eax+11+ecx]
; 1 byte is minimal TT think time in addition to ecx.
        ret
.low_speed:
; Low-speed interrupt IN/OUT:
; multiply by 8 for LS -> FS,
; add 85 bytes as in full-speed interrupt and extra 5 bytes for two PRE packets
; and two hub delays.
; 1 byte is minimal TT think time in addition to ecx.
        lea     eax, [eax*8+90+ecx]
        ret
endp

; Helper procedure for TT scheduler.
; Calculates Start-Split/Complete-Split masks and HS bandwidths.
; ebx -> usb_pipe, edi -> usb_split_info
proc tt_fill_split_info
; Interrupt endpoints.
; The core spec says in 5.7.3 "Interrupt Transfer Packet Size Constraints" that:
; The maximum allowable interrupt data payload size is 64 bytes or less for full-speed.
; Low-speed devices are limited to eight bytes or less maximum data payload size.
; This is important for scheduling, it guarantees that in any case transaction fits
; in two microframes (usually one, two if transaction has started too late in the first
; microframe), so check it.
        mov     eax, [ebx+ehci_pipe.Token-sizeof.ehci_pipe]
        mov     ecx, 8
        bt      eax, 12
        jc      @f
        mov     ecx, 64
@@:
        shr     eax, 16
        and     eax, (1 shl 11) - 1     ; get data length
        cmp     eax, ecx
        ja      .error
        add     eax, 3  ; add 3 bytes for other fields in data packet, PID+CRC16
; Multiply by 8 for bytes -> bits and then by 7/6 to accomodate bit stuffing;
; total 28/3 = 9+1/3
        mov     edx, 55555556h
        lea     ecx, [eax*9]
        mul     edx
; One start-split, three complete-splits (unless the last is too far,
; but this is handled by the caller).
        mov     eax, [ebx+usb_pipe.LastTD]
        mov     [edi+usb_split_info.microframe_mask], 0x1C01
; Structure and HS bandwidth of packets depends on the direction.
        bt      [eax+ehci_gtd.Token-sizeof.ehci_gtd], 8
        jc      .interrupt_in
.interrupt_out:
; Start-Split phase:
; 77 bits for SPLIT packet (32 for SYNC, 8 for EOP, 32 for data, 5 for bit stuffing),
; 88 bits for inter-packet delay, 68 bits for Token packet,
; 88 bits for inter-packet delay, 40 bits for SYNC+EOP in Data packet,
; 88 bits for last inter-packet delay, total 449 bits.
        lea     eax, [edx+ecx+449]
        mov     [edi+usb_split_info.ssplit_bandwidth], eax
; Complete-Split phase:
; 77 bits for SPLIT packet,
; 88 bits for inter-packet delay, 68 bits for Token packet,
; 736 bits for bus turn-around, 49 bits for Handshake packet,
; 8 bits for inter-packet delay, total 1026 bits.
        mov     [edi+usb_split_info.csplit_bandwidth], 1026
        ret
.interrupt_in:
; Start-Split phase:
; 77 bits for SPLIT packet, 88 bits for inter-packet delay,
; 68 bits for Token packet, 88 bits for another inter-packet delay,
; total 321 bits.
        mov     [edi+usb_split_info.ssplit_bandwidth], 321
; Complete-Split phase:
; 77 bits for SPLIT packet, 88 bits for inter-packet delay,
; 68 bits for Token packet, 736 bits for bus turn-around,
; 40 bits for SYNC+EOP in Data packet, 8 bits for inter-packet delay,
; total 1017 bits.
        lea     eax, [edx+ecx+1017]
        mov     [edi+usb_split_info.csplit_bandwidth], eax
        ret
.error:
        xor     eax, eax
        ret
endp
