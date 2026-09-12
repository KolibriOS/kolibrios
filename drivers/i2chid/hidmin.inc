; Minimal HID report descriptor parser.
;
; The goal is not a full HID implementation: the parser walks the descriptor
; once and remembers the bit layout of the first input report that contains
; relative X and relative Y (Generic Desktop / X, Y) - i.e. the mouse
; compatibility collection that every I2C mouse and every Windows Precision
; Touchpad exposes. Buttons (Usage Page 9) and a relative wheel found in the
; same report are picked up too. Absolute (digitizer) reports are advanced
; over but otherwise ignored.
;
; Report IDs are honoured: bit positions are counted separately per report ID
; and the chosen ID is checked in every incoming packet.
;
; Alongside the mouse report, the parser also maps the touchpad (Windows
; Precision Touchpad) report: the report that carries Tip Switch usages.
; Each Tip Switch opens a new finger block; the absolute X and Y that follow
; it belong to that finger; Contact Count and the Button-page fields of the
; same report are recorded too. Once the pad is switched into touchpad mode
; those offsets are what the frame decoder reads - every vendor lays the
; frame out differently, and only the descriptor knows how.
HID_MAX_FINGERS = 5

HID_MAX_USAGES  = 16
HID_MAX_REPIDS  = 16

; Returns the pointer to the input bit counter of the current report ID,
; allocating a new slot on first use.
; out: ebx -> dword bit counter. Clobbers eax, ecx.
proc hid_get_bits_slot
        mov     eax, [p_report_id]
        xor     ecx, ecx
.scan:
        cmp     ecx, [p_idcnt]
        jae     .add
        cmp     eax, [p_ids+ecx*4]
        jz      .found
        inc     ecx
        jmp     .scan
.add:
        cmp     ecx, HID_MAX_REPIDS
        jb      @f
        dec     ecx                     ; table full: reuse the last slot
        jmp     .found
@@:
        mov     [p_ids+ecx*4], eax
        mov     dword [p_bits+ecx*4], 0
        mov     dword [p_fbits+ecx*4], 0
        inc     [p_idcnt]
.found:
        lea     ebx, [p_bits+ecx*4]
        ret
endp

; Handles one Input main item. v_flags = item data (bit 2 = relative).
proc hid_process_input
        push    eax ebx ecx edx
        call    hid_get_bits_slot
        mov     eax, [ebx]
        mov     [v_base], eax
; Advance the counter over this field right away.
        mov     eax, [p_report_size]
        imul    eax, [p_report_count]
        add     [ebx], eax
; Remember the largest logical maximum ever attached to Generic Desktop X
; and Y. The mouse collection declares a tiny relative range there while
; the touchpad collection declares the real surface extents, so the
; maximum over the whole descriptor is the size of the pad - which is what
; the button zones are measured against, and it differs per model.
        cmp     [p_usage_cnt], 0
        je      .no_extent
        xor     ecx, ecx
.ext_loop:
        cmp     ecx, [p_usage_cnt]
        jae     .no_extent
        mov     eax, [p_usages+ecx*4]
        cmp     eax, 0x00010030
        jne     .ext_y
        mov     eax, [p_log_max]
        cmp     eax, [t_padmaxx]
        jbe     .ext_next
        mov     [t_padmaxx], eax
        jmp     .ext_next
.ext_y:
        cmp     eax, 0x00010031
        jne     .ext_next
        mov     eax, [p_log_max]
        cmp     eax, [t_padmaxy]
        jbe     .ext_next
        mov     [t_padmaxy], eax
.ext_next:
        inc     ecx
        jmp     .ext_loop
.no_extent:
; Button-style field: Usage Minimum/Maximum on the Button page.
        cmp     [p_usage_min], -1
        je      .not_buttons
        cmp     [p_usage_max], -1
        je      .not_buttons
        cmp     [p_usage_page], 9
        jne     .not_buttons
        cmp     [p_report_size], 1      ; buttons are always 1-bit fields
        jne     .done
        mov     eax, [v_base]
        mov     ecx, [p_report_count]
        call    hid_note_buttons
        jmp     .done
.not_buttons:
        cmp     [p_usage_cnt], 0
        je      .done
        call    hid_note_touchpad
; Usage-list field: look for GD/X, GD/Y, GD/Wheel among the usages.
        test    [v_flags], 4            ; only relative fields are interesting
        jz      .done
        xor     ecx, ecx
.usage_loop:
        cmp     ecx, [p_report_count]
        jae     .done
; usage index = min(field index, usage count - 1): a short usage list
; applies its last usage to the remaining fields.
        mov     edx, [p_usage_cnt]
        dec     edx
        mov     eax, ecx
        cmp     eax, edx
        jbe     @f
        mov     eax, edx
@@:
        mov     eax, [p_usages+eax*4]
        cmp     eax, 0x00010030
        je      .is_x
        cmp     eax, 0x00010031
        je      .is_y
        cmp     eax, 0x00010038
        je      .is_wheel
.next_usage:
        inc     ecx
        jmp     .usage_loop
.is_x:
        test    [t_have], 1
        jnz     .next_usage
        call    hid_claim_report_id
        jc      .next_usage
        call    hid_field_offset
        mov     [t_xoff], eax
        mov     eax, [p_report_size]
        mov     [t_xsize], eax
        or      [t_have], 1
        jmp     .next_usage
.is_y:
        test    [t_have], 2
        jnz     .next_usage
        call    hid_claim_report_id
        jc      .next_usage
        call    hid_field_offset
        mov     [t_yoff], eax
        mov     eax, [p_report_size]
        mov     [t_ysize], eax
        or      [t_have], 2
        jmp     .next_usage
.is_wheel:
        test    [t_have], 8
        jnz     .next_usage
        mov     eax, [t_id]
        cmp     eax, -1                 ; wheel never claims a report on its own
        je      .next_usage
        cmp     eax, [p_report_id]
        jne     .next_usage
        call    hid_field_offset
        mov     [t_woff], eax
        mov     eax, [p_report_size]
        mov     [t_wsize], eax
        or      [t_have], 8
        jmp     .next_usage
.done:
        pop     edx ecx ebx eax
        ret
endp

; Records a run of 1-bit Button-page fields starting at bit eax, ecx of them.
; The mouse decoder wants the buttons of ITS report: an item from some other
; report is kept only until a better one turns up, and .eof drops it if it
; still does not match. The touchpad report keeps its own copy.
proc hid_note_buttons
        push    eax ecx edx
        cmp     ecx, 8
        jbe     @f
        mov     ecx, 8
@@:
        mov     edx, [p_report_id]
        cmp     edx, [t_ptp_id]
        jne     .mouse
        cmp     [t_pbtn_cnt], 0
        jne     .mouse
        mov     [t_pbtn_off], eax
        mov     [t_pbtn_cnt], ecx
.mouse:
        cmp     [t_btncnt], 0
        je      .take
        mov     edx, [t_btnid]
        cmp     edx, [t_id]             ; already the chosen report's: keep
        je      .done
        mov     edx, [p_report_id]
        cmp     edx, [t_btnid]          ; a second run in the same report: keep the first
        je      .done
.take:
        mov     [t_btnoff], eax
        mov     [t_btncnt], ecx
        mov     eax, [p_report_id]
        mov     [t_btnid], eax
.done:
        pop     edx ecx eax
        ret
endp

; Walks the usages of the current input item looking for the touchpad
; fields. Field k takes usage min(k, count-1), as in the mouse search.
proc hid_note_touchpad
        push    eax ebx ecx edx
        xor     ecx, ecx
.field:
        cmp     ecx, [p_report_count]
        jae     .done
        mov     edx, [p_usage_cnt]
        dec     edx
        mov     eax, ecx
        cmp     eax, edx
        jbe     @f
        mov     eax, edx
@@:
        mov     ebx, [p_usages+eax*4]   ; ebx = usage of field ecx
        call    hid_field_offset        ; eax = its bit offset
        cmp     ebx, 0x000D0042         ; Tip Switch: a new finger
        je      .tip
        cmp     ebx, 0x000D0054         ; Contact Count
        je      .count
        cmp     ebx, 0x000D0051         ; Contact Identifier
        je      .cid
        cmp     ebx, 0x00010030
        je      .x
        cmp     ebx, 0x00010031
        je      .y
        shr     ebx, 16
        cmp     ebx, 9                  ; buttons written as a usage list
        jne     .next
        push    ecx
        neg     ecx
        add     ecx, [p_report_count]   ; the fields from here to the end
        call    hid_note_buttons
        pop     ecx
        jmp     .done                   ; one run per item is enough
.tip:
        cmp     [t_ptp_id], -1
        jne     @f
        mov     edx, [p_report_id]
        mov     [t_ptp_id], edx         ; the first report with fingers is the one
@@:
        call    hid_in_ptp
        jne     .next
        mov     edx, [t_fcount]
        cmp     edx, HID_MAX_FINGERS
        jae     .next
        mov     [t_ftip+edx*4], eax
        mov     dword [t_fxs+edx*4], 0
        mov     dword [t_fys+edx*4], 0
        mov     dword [t_fcids+edx*4], 0
        inc     [t_fcount]
        jmp     .next
.cid:
        call    hid_in_ptp
        jne     .next
        mov     edx, [t_fcount]
        test    edx, edx
        jz      .next
        dec     edx
        cmp     dword [t_fcids+edx*4], 0
        jne     .next
        mov     [t_fcid+edx*4], eax
        mov     eax, [p_report_size]
        mov     [t_fcids+edx*4], eax
        jmp     .next
.count:
        call    hid_in_ptp
        jne     .next
        mov     [t_ccnt_off], eax
        mov     eax, [p_report_size]
        mov     [t_ccnt_size], eax
        jmp     .next
.x:
        test    [v_flags], 4            ; absolute only; relative is the mouse
        jnz     .next
        call    hid_in_ptp
        jne     .next
        mov     edx, [t_fcount]
        test    edx, edx
        jz      .next
        dec     edx
        cmp     dword [t_fxs+edx*4], 0
        jne     .next
        mov     [t_fx+edx*4], eax
        mov     eax, [p_report_size]
        mov     [t_fxs+edx*4], eax
        jmp     .next
.y:
        test    [v_flags], 4
        jnz     .next
        call    hid_in_ptp
        jne     .next
        mov     edx, [t_fcount]
        test    edx, edx
        jz      .next
        dec     edx
        cmp     dword [t_fys+edx*4], 0
        jne     .next
        mov     [t_fy+edx*4], eax
        mov     eax, [p_report_size]
        mov     [t_fys+edx*4], eax
.next:
        inc     ecx
        jmp     .field
.done:
        pop     edx ecx ebx eax
        ret
endp

; ZF set when the current item belongs to the touchpad report.
proc hid_in_ptp
        push    eax
        mov     eax, [p_report_id]
        cmp     eax, [t_ptp_id]
        pop     eax
        ret
endp

; Size in bytes of the feature report with id eax; 0 when the descriptor
; never mentioned one.
proc hid_feature_bytes
        push    ecx
        xor     ecx, ecx
@@:
        cmp     ecx, [p_idcnt]
        jae     .none
        cmp     eax, [p_ids+ecx*4]
        je      .found
        inc     ecx
        jmp     @b
.found:
        mov     eax, [p_fbits+ecx*4]
        add     eax, 7
        shr     eax, 3
        pop     ecx
        ret
.none:
        xor     eax, eax
        pop     ecx
        ret
endp

; Locks the candidate report ID to the current one, or fails (CF=1) when a
; different report already holds X/Y.
proc hid_claim_report_id
        mov     eax, [t_id]
        cmp     eax, -1
        jne     @f
        mov     eax, [p_report_id]
        mov     [t_id], eax
        clc
        ret
@@:
        cmp     eax, [p_report_id]
        jne     .taken
        clc
        ret
.taken:
        stc
        ret
endp

; Bit offset of field #ecx of the current item.
; in: ecx = field index. out: eax = v_base + ecx * report_size.
proc hid_field_offset
        mov     eax, [p_report_size]
        imul    eax, ecx
        add     eax, [v_base]
        ret
endp

; Parses the report descriptor.
; in: esi -> descriptor, ecx = length.
; out: eax = 1 and f_* variables filled if a usable report was found.
proc hid_parse_report_desc uses ebx esi edi
        lea     edi, [esi+ecx]
        xor     eax, eax
        mov     [p_usage_page], eax
        mov     [p_report_size], eax
        mov     [p_report_count], eax
        mov     [p_report_id], eax
        mov     [p_uses_repid], eax
        mov     [p_usage_cnt], eax
        mov     [p_idcnt], eax
        mov     [p_log_max], eax
        mov     [t_padmaxx], eax
        mov     [t_padmaxy], eax
        mov     [t_have], eax
        mov     [t_btncnt], eax
        mov     [t_wsize], eax
        mov     [p_usage_min], -1
        mov     [p_usage_max], -1
        mov     [t_id], -1
        mov     [t_btnid], -2
        mov     [t_ptp_id], -1
        mov     [t_fcount], eax
        mov     [t_ccnt_size], eax
        mov     [t_pbtn_cnt], eax
        mov     [t_ptp_bytes], eax
.loop:
        cmp     esi, edi
        jae     .eof
        movzx   eax, byte [esi]
        inc     esi
        cmp     al, 0xFE
        je      .long_item
        mov     ecx, eax
        and     ecx, 3
        cmp     ecx, 3
        jne     @f
        inc     ecx                     ; size code 3 means 4 bytes
@@:
        lea     ebx, [esi+ecx]
        cmp     ebx, edi
        ja      .eof
        xor     edx, edx
        jecxz   .no_data
        cmp     ecx, 1
        jne     @f
        movzx   edx, byte [esi]
        jmp     .have_data
@@:
        cmp     ecx, 2
        jne     @f
        movzx   edx, word [esi]
        jmp     .have_data
@@:
        mov     edx, [esi]
.have_data:
        add     esi, ecx
.no_data:
        and     eax, 0xFC
; Global items
        cmp     al, 0x04
        je      .g_usage_page
        cmp     al, 0x84
        je      .g_report_id
        cmp     al, 0x74
        je      .g_report_size
        cmp     al, 0x94
        je      .g_report_count
        cmp     al, 0x24
        je      .g_log_max
; Local items
        cmp     al, 0x08
        je      .l_usage
        cmp     al, 0x18
        je      .l_usage_min
        cmp     al, 0x28
        je      .l_usage_max
; Main items
        cmp     al, 0x80
        je      .m_input
        cmp     al, 0x90
        je      .m_clear_locals
        cmp     al, 0xB0
        je      .m_feature
        cmp     al, 0xA0
        je      .m_clear_locals
        cmp     al, 0xC0
        je      .m_clear_locals
        jmp     .loop
.g_usage_page:
        and     edx, 0xFFFF
        mov     [p_usage_page], edx
        jmp     .loop
.g_report_id:
        mov     [p_report_id], edx
        mov     [p_uses_repid], 1
        jmp     .loop
.g_report_size:
        mov     [p_report_size], edx
        jmp     .loop
.g_report_count:
        mov     [p_report_count], edx
        jmp     .loop
.g_log_max:
        mov     [p_log_max], edx
        jmp     .loop
.l_usage:
        mov     ebx, [p_usage_cnt]
        cmp     ebx, HID_MAX_USAGES
        jae     .loop
        cmp     ecx, 4                  ; 4-byte usage carries its own page
        je      @f
        mov     eax, [p_usage_page]
        shl     eax, 16
        or      edx, eax
@@:
        mov     [p_usages+ebx*4], edx
        inc     [p_usage_cnt]
        jmp     .loop
.l_usage_min:
        mov     [p_usage_min], edx
        jmp     .loop
.l_usage_max:
        mov     [p_usage_max], edx
        jmp     .loop
.m_input:
        mov     [v_flags], edx
        call    hid_process_input
.m_feature:
; Feature reports are only ever written, never decoded, but writing one
; needs its exact size: a SET_REPORT shorter than the report is ignored by
; some firmwares (the Elan Input Mode report has two bytes, mode and the
; selective-reporting switches - one byte does nothing there).
        call    hid_get_bits_slot
        mov     eax, [p_report_size]
        imul    eax, [p_report_count]
        add     [p_fbits+ecx*4], eax
.m_clear_locals:
        xor     eax, eax
        mov     [p_usage_cnt], eax
        mov     [p_usage_min], -1
        mov     [p_usage_max], -1
        jmp     .loop
.long_item:
        cmp     esi, edi
        jae     .eof
        movzx   ecx, byte [esi]
        lea     esi, [esi+ecx+2]
        jmp     .loop
.eof:
; Size of the touchpad report, when there is one.
        mov     eax, [t_ptp_id]
        cmp     eax, -1
        je      .no_ptp
        xor     ecx, ecx
@@:
        cmp     ecx, [p_idcnt]
        jae     .no_ptp
        cmp     eax, [p_ids+ecx*4]
        je      @f
        inc     ecx
        jmp     @b
@@:
        mov     eax, [p_bits+ecx*4]
        add     eax, 7
        shr     eax, 3
        mov     [t_ptp_bytes], eax
.no_ptp:
; Success needs relative X and Y in the same report - or, failing that, a
; touchpad report with at least one complete finger, which is then the only
; thing the device will be driven through.
        mov     eax, [t_have]
        and     eax, 3
        cmp     eax, 3
        je      .have_mouse
        cmp     [t_fcount], 0
        je      .fail
        cmp     dword [t_fxs], 0
        je      .fail
        cmp     dword [t_fys], 0
        je      .fail
        mov     [f_repid], -1           ; no mouse report to match
        mov     [f_bytes], 0
        mov     [t_btncnt], 0
        movi    eax, 1
        ret
.have_mouse:
; Keep buttons/wheel only when they live in the chosen report.
        mov     eax, [t_btnid]
        cmp     eax, [t_id]
        je      @f
        mov     [t_btncnt], 0
@@:
; Final layout.
        mov     eax, [t_id]
        mov     [f_repid], eax
        xor     ecx, ecx
@@:
        cmp     ecx, [p_idcnt]
        jae     .no_bits
        cmp     eax, [p_ids+ecx*4]
        je      .got_bits
        inc     ecx
        jmp     @b
.got_bits:
        mov     eax, [p_bits+ecx*4]
        add     eax, 7
        shr     eax, 3
        mov     [f_bytes], eax
        movi    eax, 1
        ret
.no_bits:
.fail:
        xor     eax, eax
        ret
endp

; Extracts an unsigned field from an input report.
; in: esi -> report data, eax = bit offset, ecx = bit count (1..24).
; out: eax = value.
proc hid_extract_u
        push    ebx ecx edx
        mov     edx, eax
        shr     edx, 3
        mov     ebx, [esi+edx]          ; buffer has spare bytes at the end
        and     eax, 7
        push    ecx
        mov     ecx, eax
        shr     ebx, cl
        pop     ecx
        neg     ecx
        add     ecx, 32
        shl     ebx, cl
        shr     ebx, cl
        mov     eax, ebx
        pop     edx ecx ebx
        ret
endp

; Same, sign-extended.
proc hid_extract_s
        push    ebx ecx edx
        mov     edx, eax
        shr     edx, 3
        mov     ebx, [esi+edx]
        and     eax, 7
        push    ecx
        mov     ecx, eax
        shr     ebx, cl
        pop     ecx
        neg     ecx
        add     ecx, 32
        shl     ebx, cl
        sar     ebx, cl
        mov     eax, ebx
        pop     edx ecx ebx
        ret
endp
