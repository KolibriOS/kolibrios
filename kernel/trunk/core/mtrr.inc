;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2014. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; Initializes MTRRs.
proc init_mtrr

        cmp     [BOOT_VARS+BOOT_MTRR], byte 2
        je      .exit

        bt      [cpu_caps], CAPS_MTRR
        jnc     .exit

        call    mtrr_reconfigure
        stdcall set_mtrr, [LFBAddress], 0x1000000, MEM_WC

.exit:
        ret
endp

; Helper procedure for mtrr_reconfigure and set_mtrr,
; called before changes in MTRRs.
proc mtrr_begin_change
        mov     eax, cr0
        or      eax, 0x60000000 ;disable caching
        mov     cr0, eax
        wbinvd                  ;invalidate cache
        ret
endp

; Helper procedure for mtrr_reconfigure and set_mtrr,
; called after changes in MTRRs.
proc mtrr_end_change
        wbinvd                  ;again invalidate
        mov     eax, cr0
        and     eax, not 0x60000000
        mov     cr0, eax        ; enable caching
        ret
endp

; Some limits to number of structures located in the stack.
MAX_USEFUL_MTRRS = 16
MAX_RANGES = 16

; mtrr_reconfigure keeps a list of MEM_WB ranges.
; This structure describes one item in the list.
struct mtrr_range
next            dd      ?       ; next item
start           dq      ?       ; first byte
length          dq      ?       ; length in bytes
ends

uglobal
align 4
num_variable_mtrrs      dd      0       ; number of variable-range MTRRs
endg

; Helper procedure for MTRR initialization.
; Takes MTRR configured by BIOS and tries to recongifure them
; in order to allow non-UC data at top of 4G memory.
; Example: if low part of physical memory is 3.5G = 0xE0000000 bytes wide,
; BIOS can configure two MTRRs so that the first MTRR describes [0, 4G) as WB
; and the second MTRR describes [3.5G, 4G) as UC;
; WB+UC=UC, so the resulting memory map would be as needed,
; but in this configuration our attempts to map LFB at (say) 0xE8000000 as WC
; would be ignored, WB+UC+WC is still UC.
; So we must keep top of 4G memory not covered by MTRRs,
; using three WB MTRRs [0,2G) + [2G,3G) + [3G,3.5G),
; this gives the same memory map, but allows to add further entries.
; See mtrrtest.asm for detailed input/output from real hardware+BIOS.
proc mtrr_reconfigure
        push    ebp     ; we're called from init_LFB, and it feels hurt when ebp is destroyed
; 1. Prepare local variables.
; 1a. Create list of MAX_RANGES free (aka not yet allocated) ranges.
        xor     eax, eax
        lea     ecx, [eax+MAX_RANGES]
.init_ranges:
        sub     esp, sizeof.mtrr_range - 4
        push    eax
        mov     eax, esp
        dec     ecx
        jnz     .init_ranges
        mov     eax, esp
; 1b. Fill individual local variables.
        xor     edx, edx
        sub     esp, MAX_USEFUL_MTRRS * 16      ; .mtrrs
        push    edx             ; .mtrrs_end
        push    edx             ; .num_used_mtrrs
        push    eax             ; .first_free_range
        push    edx             ; .first_range: no ranges yet
        mov     cl, [cpu_phys_addr_width]
        or      eax, -1
        shl     eax, cl ; note: this uses cl&31 = cl-32, not the entire cl
        push    eax     ; .phys_reserved_mask
virtual at esp
.phys_reserved_mask     dd      ?
.first_range            dd      ?
.first_free_range       dd      ?
.num_used_mtrrs         dd      ?
.mtrrs_end              dd      ?
.mtrrs          rq      MAX_USEFUL_MTRRS * 2
.local_vars_size = $ - esp
end virtual

; 2. Get the number of variable-range MTRRs from MTRRCAP register.
; Abort if zero.
        mov     ecx, 0xFE
        rdmsr
        test    al, al
        jz      .abort
        mov     byte [num_variable_mtrrs], al
; 3. Validate MTRR_DEF_TYPE register.
        mov     ecx, 0x2FF
        rdmsr
; If BIOS has not initialized variable-range MTRRs, fallback to step 7.
        test    ah, 8
        jz      .fill_ranges_from_memory_map
; If the default memory type (not covered by MTRRs) is not UC,
; then probably BIOS did something strange, so it is better to exit immediately
; hoping for the best.
        cmp     al, MEM_UC
        jnz     .abort
; 4. Validate all variable-range MTRRs
; and copy configured MTRRs to the local array [.mtrrs].
; 4a. Prepare for the loop over existing variable-range MTRRs.
        mov     ecx, 0x200
        lea     edi, [.mtrrs]
.get_used_mtrrs_loop:
; 4b. For every MTRR, read PHYSBASEn and PHYSMASKn.
; In PHYSBASEn, clear upper bits and copy to ebp:ebx.
        rdmsr
        or      edx, [.phys_reserved_mask]
        xor     edx, [.phys_reserved_mask]
        mov     ebp, edx
        mov     ebx, eax
        inc     ecx
; If PHYSMASKn is not active, ignore this MTRR.
        rdmsr
        inc     ecx
        test    ah, 8
        jz      .get_used_mtrrs_next
; 4c. For every active MTRR, check that number of local entries is not too large.
        inc     [.num_used_mtrrs]
        cmp     [.num_used_mtrrs], MAX_USEFUL_MTRRS
        ja      .abort
; 4d. For every active MTRR, store PHYSBASEn with upper bits cleared.
; This contains the MTRR base and the memory type in low byte.
        mov     [edi], ebx
        mov     [edi+4], ebp
; 4e. For every active MTRR, check that the range is continuous:
; PHYSMASKn with upper bits set must be negated power of two, and
; low bits of PHYSBASEn must be zeroes:
; PHYSMASKn = 1...10...0,
; PHYSBASEn = x...x0...0,
; this defines a continuous range from x...x0...0 to x...x1...1,
; length = 10...0 = negated PHYSMASKn.
; Store length in the local array.
        and     eax, not 0xFFF
        or      edx, [.phys_reserved_mask]
        mov     dword [edi+8], 0
        mov     dword [edi+12], 0
        sub     [edi+8], eax
        sbb     [edi+12], edx
; (x and -x) is the maximum power of two that divides x.
; Condition for powers of two: (x and -x) equals x.
        and     eax, [edi+8]
        and     edx, [edi+12]
        cmp     eax, [edi+8]
        jnz     .abort
        cmp     edx, [edi+12]
        jnz     .abort
        sub     eax, 1
        sbb     edx, 0
        and     eax, not 0xFFF
        and     eax, ebx
        jnz     .abort
        and     edx, ebp
        jnz     .abort
; 4f. For every active MTRR, validate memory type: it must be either WB or UC.
        add     edi, 16
        cmp     bl, MEM_UC
        jz      .get_used_mtrrs_next
        cmp     bl, MEM_WB
        jnz     .abort
.get_used_mtrrs_next:
; 4g. Repeat the loop at 4b-4f for all [num_variable_mtrrs] entries.
        mov     eax, [num_variable_mtrrs]
        lea     eax, [0x200+eax*2]
        cmp     ecx, eax
        jb      .get_used_mtrrs_loop
; 4h. If no active MTRRs were detected, fallback to step 7.
        cmp     [.num_used_mtrrs], 0
        jz      .fill_ranges_from_memory_map
        mov     [.mtrrs_end], edi
; 5. Generate sorted list of ranges marked as WB.
; 5a. Prepare for the loop over configured MTRRs filled at step 4.
        lea     ecx, [.mtrrs]
.fill_wb_ranges:
; 5b. Ignore non-WB MTRRs.
        mov     ebx, [ecx]
        cmp     bl, MEM_WB
        jnz     .next_wb_range
        mov     ebp, [ecx+4]
        and     ebx, not 0xFFF  ; clear memory type and reserved bits
; ebp:ebx = start of the range described by the current MTRR.
; 5c. Find the first existing range containing a point greater than ebp:ebx.
        lea     esi, [.first_range]
.find_range_wb:
; If there is no next range or start of the next range is greater than ebp:ebx,
; exit the loop to 5d.
        mov     edi, [esi]
        test    edi, edi
        jz      .found_place_wb
        mov     eax, ebx
        mov     edx, ebp
        sub     eax, dword [edi+mtrr_range.start]
        sbb     edx, dword [edi+mtrr_range.start+4]
        jb      .found_place_wb
; Otherwise, if end of the next range is greater than or equal to ebp:ebx,
; exit the loop to 5e.
        mov     esi, edi
        sub     eax, dword [edi+mtrr_range.length]
        sbb     edx, dword [edi+mtrr_range.length+4]
        jb      .expand_wb
        or      eax, edx
        jnz     .find_range_wb
        jmp     .expand_wb
.found_place_wb:
; 5d. ebp:ebx is not within any existing range.
; Insert a new range between esi and edi.
; (Later, during 5e, it can be merged with the following ranges.)
        mov     eax, [.first_free_range]
        test    eax, eax
        jz      .abort
        mov     [esi], eax
        mov     edx, [eax+mtrr_range.next]
        mov     [.first_free_range], edx
        mov     dword [eax+mtrr_range.start], ebx
        mov     dword [eax+mtrr_range.start+4], ebp
; Don't fill [eax+mtrr_range.next] and [eax+mtrr_range.length] yet,
; they will be calculated including merges at step 5e.
        mov     esi, edi
        mov     edi, eax
.expand_wb:
; 5e. The range at edi contains ebp:ebx, and esi points to the first range
; to be checked for merge: esi=edi if ebp:ebx was found in an existing range,
; esi is next after edi if a new range with ebp:ebx was created.
; Merge it with following ranges while start of the next range is not greater
; than the end of the new range.
        add     ebx, [ecx+8]
        adc     ebp, [ecx+12]
; ebp:ebx = end of the range described by the current MTRR.
.expand_wb_loop:
; If there is no next range or start of the next range is greater than ebp:ebx,
; exit the loop to 5g.
        test    esi, esi
        jz      .expand_wb_done
        mov     eax, ebx
        mov     edx, ebp
        sub     eax, dword [esi+mtrr_range.start]
        sbb     edx, dword [esi+mtrr_range.start+4]
        jb      .expand_wb_done
; Otherwise, if end of the next range is greater than or equal to ebp:ebx,
; exit the loop to 5f.
        sub     eax, dword [esi+mtrr_range.length]
        sbb     edx, dword [esi+mtrr_range.length+4]
        jb      .expand_wb_last
; Otherwise, the current range is completely within the new range.
; Free it and continue the loop.
        mov     edx, [esi+mtrr_range.next]
        cmp     esi, edi
        jz      @f
        mov     eax, [.first_free_range]
        mov     [esi+mtrr_range.next], eax
        mov     [.first_free_range], esi
@@:
        mov     esi, edx
        jmp     .expand_wb_loop
.expand_wb_last:
; 5f. Start of the new range is inside range described by esi,
; end of the new range is inside range described by edi.
; If esi is equal to edi, the new range is completely within
; an existing range, so proceed to the next range.
        cmp     esi, edi
        jz      .next_wb_range
; Otherwise, set end of interval at esi to end of interval at edi
; and free range described by edi.
        mov     ebx, dword [esi+mtrr_range.start]
        mov     ebp, dword [esi+mtrr_range.start+4]
        add     ebx, dword [esi+mtrr_range.length]
        adc     ebp, dword [esi+mtrr_range.length+4]
        mov     edx, [esi+mtrr_range.next]
        mov     eax, [.first_free_range]
        mov     [esi+mtrr_range.next], eax
        mov     [.first_free_range], esi
        mov     esi, edx
.expand_wb_done:
; 5g. We have found the next range (maybe 0) after merging and
; the new end of range (maybe ebp:ebx from the new range
; or end of another existing interval calculated at step 5f).
; Write them to range at edi.
        mov     [edi+mtrr_range.next], esi
        sub     ebx, dword [edi+mtrr_range.start]
        sbb     ebp, dword [edi+mtrr_range.start+4]
        mov     dword [edi+mtrr_range.length], ebx
        mov     dword [edi+mtrr_range.length+4], ebp
.next_wb_range:
; 5h. Continue the loop 5b-5g over all configured MTRRs.
        add     ecx, 16
        cmp     ecx, [.mtrrs_end]
        jb      .fill_wb_ranges
; 6. Exclude all ranges marked as UC.
; 6a. Prepare for the loop over configured MTRRs filled at step 4.
        lea     ecx, [.mtrrs]
.fill_uc_ranges:
; 6b. Ignore non-UC MTRRs.
        mov     ebx, [ecx]
        cmp     bl, MEM_UC
        jnz     .next_uc_range
        mov     ebp, [ecx+4]
        and     ebx, not 0xFFF  ; clear memory type and reserved bits
; ebp:ebx = start of the range described by the current MTRR.
        lea     esi, [.first_range]
; 6c. Find the first existing range containing a point greater than ebp:ebx.
.find_range_uc:
; If there is no next range, ignore this MTRR,
; exit the loop and continue to next MTRR.
        mov     edi, [esi]
        test    edi, edi
        jz      .next_uc_range
; If start of the next range is greater than or equal to ebp:ebx,
; exit the loop to 6e.
        mov     eax, dword [edi+mtrr_range.start]
        mov     edx, dword [edi+mtrr_range.start+4]
        sub     eax, ebx
        sbb     edx, ebp
        jnb     .truncate_uc
; Otherwise, continue the loop if end of the next range is less than ebp:ebx,
; exit the loop to 6d otherwise.
        mov     esi, edi
        add     eax, dword [edi+mtrr_range.length]
        adc     edx, dword [edi+mtrr_range.length+4]
        jnb     .find_range_uc
; 6d. ebp:ebx is inside (or at end of) an existing range.
; Split the range. (The second range, maybe containing completely within UC-range,
; maybe of zero length, can be removed at step 6e, if needed.)
        mov     edi, [.first_free_range]
        test    edi, edi
        jz      .abort
        mov     dword [edi+mtrr_range.start], ebx
        mov     dword [edi+mtrr_range.start+4], ebp
        mov     dword [edi+mtrr_range.length], eax
        mov     dword [edi+mtrr_range.length+4], edx
        mov     eax, [edi+mtrr_range.next]
        mov     [.first_free_range], eax
        mov     eax, [esi+mtrr_range.next]
        mov     [edi+mtrr_range.next], eax
; don't change [esi+mtrr_range.next] yet, it will be filled at step 6e
        mov     eax, ebx
        mov     edx, ebp
        sub     eax, dword [esi+mtrr_range.start]
        sbb     edx, dword [esi+mtrr_range.start+4]
        mov     dword [esi+mtrr_range.length], eax
        mov     dword [esi+mtrr_range.length+4], edx
.truncate_uc:
; 6e. edi is the first range after ebp:ebx, check it and next ranges
; for intersection with the new range, truncate heads.
        add     ebx, [ecx+8]
        adc     ebp, [ecx+12]
; ebp:ebx = end of the range described by the current MTRR.
.truncate_uc_loop:
; If start of the next range is greater than ebp:ebx,
; exit the loop to 6g.
        mov     eax, ebx
        mov     edx, ebp
        sub     eax, dword [edi+mtrr_range.start]
        sbb     edx, dword [edi+mtrr_range.start+4]
        jb      .truncate_uc_done
; Otherwise, if end of the next range is greater than ebp:ebx,
; exit the loop to 6f.
        sub     eax, dword [edi+mtrr_range.length]
        sbb     edx, dword [edi+mtrr_range.length+4]
        jb      .truncate_uc_last
; Otherwise, the current range is completely within the new range.
; Free it and continue the loop if there is a next range.
; If that was a last range, exit the loop to 6g.
        mov     edx, [edi+mtrr_range.next]
        mov     eax, [.first_free_range]
        mov     [.first_free_range], edi
        mov     [edi+mtrr_range.next], eax
        mov     edi, edx
        test    edi, edi
        jnz     .truncate_uc_loop
        jmp     .truncate_uc_done
.truncate_uc_last:
; 6f. The range at edi partially intersects with the UC-range described by MTRR.
; Truncate it from the head.
        mov     dword [edi+mtrr_range.start], ebx
        mov     dword [edi+mtrr_range.start+4], ebp
        neg     eax
        adc     edx, 0
        neg     edx
        mov     dword [edi+mtrr_range.length], eax
        mov     dword [edi+mtrr_range.length+4], edx
.truncate_uc_done:
; 6g. We have found the next range (maybe 0) after intersection.
; Write it to [esi+mtrr_range.next].
        mov     [esi+mtrr_range.next], edi
.next_uc_range:
; 6h. Continue the loop 6b-6g over all configured MTRRs.
        add     ecx, 16
        cmp     ecx, [.mtrrs_end]
        jb      .fill_uc_ranges
; Sanity check: if there are no ranges after steps 5-6,
; fallback to step 7. Otherwise, go to 8.
        cmp     [.first_range], 0
        jnz     .ranges_ok
.fill_ranges_from_memory_map:
; 7. BIOS has not configured variable-range MTRRs.
; Create one range from 0 to [MEM_AMOUNT].
        mov     eax, [.first_free_range]
        mov     edx, [eax+mtrr_range.next]
        mov     [.first_free_range], edx
        mov     [.first_range], eax
        xor     edx, edx
        mov     [eax+mtrr_range.next], edx
        mov     dword [eax+mtrr_range.start], edx
        mov     dword [eax+mtrr_range.start+4], edx
        mov     ecx, [MEM_AMOUNT]
        mov     dword [eax+mtrr_range.length], ecx
        mov     dword [eax+mtrr_range.length+4], edx
.ranges_ok:
; 8. We have calculated list of WB-ranges.
; Now we should calculate a list of MTRRs so that
; * every MTRR describes a range with length = power of 2 and start that is aligned,
; * every MTRR can be WB or UC
; * (sum of all WB ranges) minus (sum of all UC ranges) equals the calculated list
; * top of 4G memory must not be covered by any ranges
; Example: range [0,0xBC000000) can be converted to
; [0,0x80000000)+[0x80000000,0xC0000000)-[0xBC000000,0xC0000000)
; WB            +WB                     -UC
; but not to [0,0x100000000)-[0xC0000000,0x100000000)-[0xBC000000,0xC0000000).
; 8a. Check that list of ranges is [0,something) plus, optionally, [4G,something).
; This holds in practice (see mtrrtest.asm for real-life examples)
; and significantly simplifies the code: ranges are independent, start of range
; is almost always aligned (the only exception >4G upper memory can be easily covered),
; there is no need to consider adding holes before start of range, only
; append them to end of range.
        xor     eax, eax
        mov     edi, [.first_range]
        cmp     dword [edi+mtrr_range.start], eax
        jnz     .abort
        cmp     dword [edi+mtrr_range.start+4], eax
        jnz     .abort
        cmp     dword [edi+mtrr_range.length+4], eax
        jnz     .abort
        mov     edx, [edi+mtrr_range.next]
        test    edx, edx
        jz      @f
        cmp     dword [edx+mtrr_range.start], eax
        jnz     .abort
        cmp     dword [edx+mtrr_range.start+4], 1
        jnz     .abort
        cmp     [edx+mtrr_range.next], eax
        jnz     .abort
@@:
; 8b. Initialize: no MTRRs filled.
        mov     [.num_used_mtrrs], eax
        lea     esi, [.mtrrs]
.range2mtrr_loop:
; 8c. If we are dealing with upper-memory range (after 4G)
; with length > start, create one WB MTRR with [start,2*start),
; reset start to 2*start and return to this step.
; Example: [4G,24G) -> [4G,8G) {returning} + [8G,16G) {returning}
; + [16G,24G) {advancing to ?}.
        mov     eax, dword [edi+mtrr_range.length+4]
        test    eax, eax
        jz      .less4G
        mov     edx, dword [edi+mtrr_range.start+4]
        cmp     eax, edx
        jb      .start_aligned
        inc     [.num_used_mtrrs]
        cmp     [.num_used_mtrrs], MAX_USEFUL_MTRRS
        ja      .abort
        mov     dword [esi], MEM_WB
        mov     dword [esi+4], edx
        mov     dword [esi+8], 0
        mov     dword [esi+12], edx
        add     esi, 16
        add     dword [edi+mtrr_range.start+4], edx
        sub     dword [edi+mtrr_range.length+4], edx
        jnz     .range2mtrr_loop
        cmp     dword [edi+mtrr_range.length], 0
        jz      .range2mtrr_next
.less4G:
; 8d. If we are dealing with low-memory range (before 4G)
; and appending a maximal-size hole would create a range covering top of 4G,
; create a maximal-size WB range and return to this step.
; Example: for [0,0xBC000000) the following steps would consider
; variants [0,0x80000000)+(another range to be splitted) and
; [0,0x100000000)-(another range to be splitted); we forbid the last variant,
; so the first variant must be used.
        bsr     ecx, dword [edi+mtrr_range.length]
        xor     edx, edx
        inc     edx
        shl     edx, cl
        lea     eax, [edx*2]
        add     eax, dword [edi+mtrr_range.start]
        jnz     .start_aligned
        inc     [.num_used_mtrrs]
        cmp     [.num_used_mtrrs], MAX_USEFUL_MTRRS
        ja      .abort
        mov     eax, dword [edi+mtrr_range.start]
        mov     dword [esi], eax
        or      dword [esi], MEM_WB
        mov     dword [esi+4], 0
        mov     dword [esi+8], edx
        mov     dword [esi+12], 0
        add     esi, 16
        add     dword [edi+mtrr_range.start], edx
        sub     dword [edi+mtrr_range.length], edx
        jnz     .less4G
        jmp     .range2mtrr_next
.start_aligned:
; Start is aligned for any allowed length, maximum-size hole is allowed.
; Select the best MTRR configuration for one range.
; length=...101101
; Without hole at the end, we need one WB MTRR for every 1-bit in length:
; length=...100000 + ...001000 + ...000100 + ...000001
; We can also append one hole at the end so that one 0-bit (selected by us)
; becomes 1 and all lower bits become 0 for WB-range:
; length=...110000 - (...00010 + ...00001)
; In this way, we need one WB MTRR for every 1-bit higher than the selected bit,
; one WB MTRR for the selected bit, one UC MTRR for every 0-bit between
; the selected bit and lowest 1-bit (they become 1-bits after negation)
; and one UC MTRR for lowest 1-bit.
; So we need to select 0-bit with the maximal difference
; (number of 0-bits) - (number of 1-bits) between selected and lowest 1-bit,
; this equals the gain from using a hole. If the difference is negative for
; all 0-bits, don't append hole.
; Note that lowest 1-bit is not included when counting, but selected 0-bit is.
; 8e. Find the optimal bit position for hole.
; eax = current difference, ebx = best difference,
; ecx = hole bit position, edx = current bit position.
        xor     eax, eax
        xor     ebx, ebx
        xor     ecx, ecx
        bsf     edx, dword [edi+mtrr_range.length]
        jnz     @f
        bsf     edx, dword [edi+mtrr_range.length+4]
        add     edx, 32
@@:
        push    edx     ; save position of lowest 1-bit for step 8f
.calc_stat:
        inc     edx
        cmp     edx, 64
        jae     .stat_done
        inc     eax     ; increment difference in hope for 1-bit
; Note: bt conveniently works with both .length and .length+4,
; depending on whether edx>=32.
        bt      dword [edi+mtrr_range.length], edx
        jc      .calc_stat
        dec     eax     ; hope was wrong, decrement difference to correct 'inc'
        dec     eax     ; and again, now getting the real difference
        cmp     eax, ebx
        jle     .calc_stat
        mov     ebx, eax
        mov     ecx, edx
        jmp     .calc_stat
.stat_done:
; 8f. If we decided to create a hole, flip all bits between lowest and selected.
        pop     edx     ; restore position of lowest 1-bit saved at step 8e
        test    ecx, ecx
        jz      .fill_hi_init
@@:
        inc     edx
        cmp     edx, ecx
        ja      .fill_hi_init
        btc     dword [edi+mtrr_range.length], edx
        jmp     @b
.fill_hi_init:
; 8g. Create MTRR ranges corresponding to upper 32 bits.
        sub     ecx, 32
.fill_hi_loop:
        bsr     edx, dword [edi+mtrr_range.length+4]
        jz      .fill_hi_done
        inc     [.num_used_mtrrs]
        cmp     [.num_used_mtrrs], MAX_USEFUL_MTRRS
        ja      .abort
        mov     eax, dword [edi+mtrr_range.start]
        mov     [esi], eax
        mov     eax, dword [edi+mtrr_range.start+4]
        mov     [esi+4], eax
        xor     eax, eax
        mov     [esi+8], eax
        bts     eax, edx
        mov     [esi+12], eax
        cmp     edx, ecx
        jl      .fill_hi_uc
        or      dword [esi], MEM_WB
        add     dword [edi+mtrr_range.start+4], eax
        jmp     @f
.fill_hi_uc:
        sub     dword [esi+4], eax
        sub     dword [edi+mtrr_range.start+4], eax
@@:
        add     esi, 16
        sub     dword [edi+mtrr_range.length], eax
        jmp     .fill_hi_loop
.fill_hi_done:
; 8h. Create MTRR ranges corresponding to lower 32 bits.
        add     ecx, 32
.fill_lo_loop:
        bsr     edx, dword [edi+mtrr_range.length]
        jz      .range2mtrr_next
        inc     [.num_used_mtrrs]
        cmp     [.num_used_mtrrs], MAX_USEFUL_MTRRS
        ja      .abort
        mov     eax, dword [edi+mtrr_range.start]
        mov     [esi], eax
        mov     eax, dword [edi+mtrr_range.start+4]
        mov     [esi+4], eax
        xor     eax, eax
        mov     [esi+12], eax
        bts     eax, edx
        mov     [esi+8], eax
        cmp     edx, ecx
        jl      .fill_lo_uc
        or      dword [esi], MEM_WB
        add     dword [edi+mtrr_range.start], eax
        jmp     @f
.fill_lo_uc:
        sub     dword [esi], eax
        sub     dword [edi+mtrr_range.start], eax
@@:
        add     esi, 16
        sub     dword [edi+mtrr_range.length], eax
        jmp     .fill_lo_loop
.range2mtrr_next:
; 8i. Repeat the loop at 8c-8h for all ranges.
        mov     edi, [edi+mtrr_range.next]
        test    edi, edi
        jnz     .range2mtrr_loop
; 9. We have calculated needed MTRRs, now setup them in the CPU.
; 9a. Abort if number of MTRRs is too large.
        mov     eax, [num_variable_mtrrs]
        cmp     [.num_used_mtrrs], eax
        ja      .abort

; 9b. Prepare for changes.
        call    mtrr_begin_change

; 9c. Prepare for loop over MTRRs.
        lea     esi, [.mtrrs]
        mov     ecx, 0x200
@@:
; 9d. For every MTRR, copy PHYSBASEn as is: step 8 has configured
; start value and type bits as needed.
        mov     eax, [esi]
        mov     edx, [esi+4]
        wrmsr
        inc     ecx
; 9e. For every MTRR, calculate PHYSMASKn = -(length) or 0x800
; with upper bits cleared, 0x800 = MTRR is valid.
        xor     eax, eax
        xor     edx, edx
        sub     eax, [esi+8]
        sbb     edx, [esi+12]
        or      eax, 0x800
        or      edx, [.phys_reserved_mask]
        xor     edx, [.phys_reserved_mask]
        wrmsr
        inc     ecx
; 9f. Continue steps 9d and 9e for all MTRRs calculated at step 8.
        add     esi, 16
        dec     [.num_used_mtrrs]
        jnz     @b
; 9g. Zero other MTRRs.
        xor     eax, eax
        xor     edx, edx
        mov     ebx, [num_variable_mtrrs]
        lea     ebx, [0x200+ebx*2]
@@:
        cmp     ecx, ebx
        jae     @f
        wrmsr
        inc     ecx
        wrmsr
        inc     ecx
        jmp     @b
@@:

; 9i. Configure MTRR_DEF_TYPE.
        mov     ecx, 0x2FF
        rdmsr
        or      ah, 8   ; enable variable-ranges MTRR
        and     al, 0xF0; default memtype = UC
        wrmsr

; 9j. Changes are done.
        call    mtrr_end_change

.abort:
        add     esp, .local_vars_size + MAX_RANGES * sizeof.mtrr_range
        pop     ebp
        ret
endp

; Allocate&set one MTRR for given range.
; size must be power of 2 that divides base.
proc set_mtrr stdcall, base:dword,size:dword,mem_type:dword
; find unused register
        mov     ecx, 0x201
.scan:
        rdmsr
        dec     ecx
        test    ah, 8
        jz      .found
        rdmsr
        test    edx, edx
        jnz     @f
        and     eax, not 0xFFF  ; clear reserved bits
        cmp     eax, [base]
        jz      .ret
@@:
        add     ecx, 3
        mov     eax, [num_variable_mtrrs]
        lea     eax, [0x200+eax*2]
        cmp     ecx, eax
        jb      .scan
; no free registers, ignore the call
.ret:
        ret
.found:
; found, write values
        call    mtrr_begin_change
        xor     edx, edx
        mov     eax, [base]
        or      eax, [mem_type]
        wrmsr

        mov     al, [cpu_phys_addr_width]
        xor     edx, edx
        bts     edx, eax
        xor     eax, eax
        sub     eax, [size]
        sbb     edx, 0
        or      eax, 0x800
        inc     ecx
        wrmsr
        call    mtrr_end_change
        ret
endp

; Helper procedure for mtrr_validate.
; Calculates memory type for given address according to variable-range MTRRs.
; Assumes that MTRRs are enabled.
; in: ebx = 32-bit physical address
; out: eax = memory type for ebx
proc mtrr_get_real_type
; 1. Initialize: we have not yet found any MTRRs covering ebx.
        push    0
        mov     ecx, 0x201
.mtrr_loop:
; 2. For every MTRR, check whether it is valid; if not, continue to the next MTRR.
        rdmsr
        dec     ecx
        test    ah, 8
        jz      .next
; 3. For every valid MTRR, check whether (ebx and PHYSMASKn) == PHYSBASEn,
; excluding low 12 bits.
        and     eax, ebx
        push    eax
        rdmsr
        test    edx, edx
        pop     edx
        jnz     .next
        xor     edx, eax
        and     edx, not 0xFFF
        jnz     .next
; 4. If so, set the bit corresponding to memory type defined by this MTRR.
        and     eax, 7
        bts     [esp], eax
.next:
; 5. Continue loop at 2-4 for all variable-range MTRRs.
        add     ecx, 3
        mov     eax, [num_variable_mtrrs]
        lea     eax, [0x200+eax*2]
        cmp     ecx, eax
        jb      .mtrr_loop
; 6. If no MTRRs cover address in ebx, use default MTRR type from MTRR_DEF_CAP.
        pop     edx
        test    edx, edx
        jz      .default
; 7. Find&clear 1-bit in edx.
        bsf     eax, edx
        btr     edx, eax
; 8. If there was only one 1-bit, then all MTRRs are consistent, return that bit.
        test    edx, edx
        jz      .nothing
; Otherwise, return MEM_UC (e.g. WB+UC is UC).
        xor     eax, eax
.nothing:
        ret
.default:
        mov     ecx, 0x2FF
        rdmsr
        movzx   eax, al
        ret
endp

; If MTRRs are configured improperly, this is not obvious to the user;
; everything works, but the performance can be horrible.
; Try to detect this and let the user know that the low performance
; is caused by some problem and is not a global property of the system.
; Let's hope he would report it to developers...
proc mtrr_validate
; 1. If MTRRs are not supported, they cannot be configured improperly.
        bt      [cpu_caps], CAPS_MTRR
        jnc     .exit
; 2. If variable-range MTRRs are not configured, this is a problem.
        mov     ecx, 0x2FF
        rdmsr
        test    ah, 8
        jz      .fail
; 3. Get the memory type for address somewhere inside working memory.
; It must be write-back.
        mov     ebx, 0x27FFFF
        call    mtrr_get_real_type
        cmp     al, MEM_WB
        jnz     .fail
; 4. If we're using a mode with LFB,
; get the memory type for last pixel of the framebuffer.
; It must be write-combined.
        test    word [SCR_MODE], 0x4000
        jz      .exit
        mov     eax, [_display.pitch]
        mul     [_display.height]
        dec     eax
; LFB is mapped to virtual address LFB_BASE,
; it uses global pages if supported by CPU.
        mov     ebx, [sys_pgdir+(LFB_BASE shr 20)]
        test    ebx, PG_LARGE
        jnz     @f
        mov     ebx, [page_tabs+(LFB_BASE shr 10)]
@@:
        and     ebx, not 0xFFF
        add     ebx, eax
        call    mtrr_get_real_type
        cmp     al, MEM_WC
        jz      .exit
; 5. The check at step 4 fails on Bochs:
; Bochs BIOS configures MTRRs in a strange way not respecting [cpu_phys_addr_width],
; so mtrr_reconfigure avoids to touch anything.
; However, Bochs core ignores MTRRs (keeping them only for rdmsr/wrmsr),
; so we don't care about proper setting for Bochs.
; Use northbridge PCI id to detect Bochs: it emulates either i440fx or i430fx
; depending on configuration file.
        mov     eax, [pcidev_list.fd]
        cmp     eax, pcidev_list        ; sanity check: fail if no PCI devices
        jz      .fail
        cmp     [eax+PCIDEV.vendor_device_id], 0x12378086
        jz      .exit
        cmp     [eax+PCIDEV.vendor_device_id], 0x01228086
        jnz     .fail
.exit:
        ret
.fail:
        mov     ebx, mtrr_user_message
        mov     ebp, notifyapp
        call    fs_execute_from_sysdir_param
        ret
endp
