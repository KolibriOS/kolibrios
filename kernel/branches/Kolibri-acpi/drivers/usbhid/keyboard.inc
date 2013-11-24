; HID keyboard driver, part of USBHID driver.

; Global constants.
; They are assembled in a macro to separate code and data;
; the code is located at the point of "include 'keyboard.inc'",
; the data are collected when workers_globals is instantiated.
macro workers_globals
{
; include global constants from previous workers
        workers_globals
align 4
; Callbacks for HID layer.
keyboard_driver:
        dd      keyboard_driver_add_device
        dd      keyboard_driver_disconnect
        dd      keyboard_driver_begin_packet
        dd      keyboard_driver_array_overflow?
        dd      keyboard_driver_input_field
        dd      keyboard_driver_end_packet
; Callbacks for keyboard layer.
kbd_functions:
        dd      12
        dd      CloseKeyboard
        dd      SetKeyboardLights
; Kernel keyboard layer takes input in form of PS/2 scancodes.
; data for keyboard: correspondence between HID usage keys and PS/2 scancodes.
EX = 80h        ; if set, precede the scancode with special scancode 0xE0
label control_keys byte
; Usages 700E0h ... 700E7h: LCtrl, LShift, LAlt, LWin, RCtrl, RShift, RAlt, RWin
        db      1Dh, 2Ah, 38h, 5Bh+EX, 1Dh+EX, 36h, 38h+EX, 5Ch+EX
; Usages 70004h ... 70004h + normal_keys_number - 1
label normal_keys byte
        db      1Eh, 30h, 2Eh, 20h, 12h, 21h, 22h, 23h, 17h, 24h, 25h, 26h, 32h, 31h, 18h, 19h
        db      10h, 13h, 1Fh, 14h, 16h, 2Fh, 11h, 2Dh, 15h, 2Ch, 02h, 03h, 04h, 05h, 06h, 07h
        db      08h, 09h, 0Ah, 0Bh, 1Ch, 01h, 0Eh, 0Fh, 39h, 0Ch, 0Dh, 1Ah, 1Bh, 2Bh,   0, 27h
        db      28h, 29h, 33h, 34h, 35h, 3Ah, 3Bh, 3Ch, 3Dh, 3Eh, 3Fh, 40h, 41h, 42h, 43h, 44h
        db      57h, 58h,37h+EX,46h,0,52h+EX,47h+EX,49h+EX,53h+EX,4Fh+EX,51h+EX,4Dh+EX,4Bh+EX,50h+EX,48h+EX,45h
        db      35h+EX,37h,4Ah,4Eh,1Ch+EX,4Fh,50h, 51h, 4Bh, 4Ch, 4Dh, 47h, 48h, 49h, 52h, 53h
        db        0,5Dh+EX,5Eh+EX
normal_keys_number = $ - normal_keys
}

; Data that are specific for one keyboard device.
struct keyboard_device_data
handle          dd      ?       ; keyboard handle from RegKeyboard
timer           dd      ?       ; auto-repeat timer handle
repeatkey       db      ?       ; auto-repeat key code
                rb      3       ; padding
usbdev          dd      ?       ; pointer to device_data of USB and HID layers
modifiers       dd      ?       ; state of LCtrl ... RWin
led_report      dd      ?       ; output report for LEDs state
numlock_bit     dd      ?       ; position of NumLock bit in LED output report
capslock_bit    dd      ?
scrolllock_bit  dd      ?       ; guess what
ends

; This procedure is called when HID layer detects a new keyboard.
; in: ebx -> usb_device_data, edi -> collection
; out: eax = device-specific data or NULL on error
proc keyboard_driver_add_device
; 1. Allocate memory for keyboard_device_data. If failed, return NULL.
        movi    eax, sizeof.keyboard_device_data
        call    Kmalloc
        test    eax, eax
        jz      .nothing
; 2. Initialize keyboard_device_data: store pointer to USB layer data,
; zero some fields, initialize bit positions to -1.
        mov     [eax+keyboard_device_data.usbdev], ebx
        xor     ecx, ecx
        mov     [eax+keyboard_device_data.timer], ecx
        mov     [eax+keyboard_device_data.repeatkey], cl
        mov     [eax+keyboard_device_data.modifiers], ecx
        mov     [eax+keyboard_device_data.led_report], ecx
        dec     ecx
        mov     [eax+keyboard_device_data.numlock_bit], ecx
        mov     [eax+keyboard_device_data.capslock_bit], ecx
        mov     [eax+keyboard_device_data.scrolllock_bit], ecx
; 3. Look for LED report and bits corresponding to indicators.
; For now, assume that all LEDs are set by the same report.
; 3a. Save registers.
        push    ebx esi
; 3b. Prepare for loop over output reports: get the first output report.
; If there are no output records, skip step 3;
; default values of led_report and *_bit were set in step 2.
        mov     edx, [edi+collection.output.first_report]
        test    edx, edx
        jz      .led_report_set
.scan_led_report:
; Process one output report.
; 3c. Prepare for loop over field groups in the current report:
; get the first field group.
        mov     ecx, [edx+report.first_field]
.scan_led_field:
; Process one field group.
; 3d. If there are no more field groups, exit the loop over field groups.
        test    ecx, ecx
        jz      .next_led_report
; For now, assume that all LEDs are plain variable fields, not arrays.
; 3e. Ignore array field groups.
        test    byte [ecx+report_field_group.flags], HID_FIELD_VARIABLE
        jz      .next_led_field
; 3f. Loop over all fields in the current group.
        push    [ecx+report_field_group.count]
; esi = pointer to usage of the current field
        lea     esi, [ecx+report_field_group.common_sizeof]
; ebx = bit position of the current field
        mov     ebx, [ecx+report_field_group.offset]
; if report is numbered, add extra byte in the start of report
        cmp     [edx+report.id], 0
        jz      .scan_led_usage
        add     ebx, 8
.scan_led_usage:
; for USAGE_LED_*LOCK, store the current bit position in the corresponding field
; and store the current report as the LED report
        cmp     dword [esi], USAGE_LED_NUMLOCK
        jz      .numlock
        cmp     dword [esi], USAGE_LED_CAPSLOCK
        jz      .capslock
        cmp     dword [esi], USAGE_LED_SCROLLLOCK
        jnz     .next_field
.scrolllock:
        mov     [eax+keyboard_device_data.scrolllock_bit], ebx
        jmp     @f
.capslock:
        mov     [eax+keyboard_device_data.capslock_bit], ebx
        jmp     @f
.numlock:
        mov     [eax+keyboard_device_data.numlock_bit], ebx
@@:
        mov     [eax+keyboard_device_data.led_report], edx
.next_field:
        add     esi, 4
        add     ebx, [ecx+report_field_group.size]
        dec     dword [esp]
        jnz     .scan_led_usage
        pop     ebx
.next_led_field:
; 3g. Continue loop over field groups: get next field group.
        mov     ecx, [ecx+report_field_group.next]
        jmp     .scan_led_field
.next_led_report:
; 3h. If the LED report has been set, break from the loop over reports.
; Otherwise, get the next report and continue if the current report is not
; the last for this collection.
        cmp     [eax+keyboard_device_data.led_report], 0
        jnz     .led_report_set
        cmp     edx, [edi+collection.output.last_report]
        mov     edx, [edx+report.next]
        jnz     .scan_led_report
.led_report_set:
; 3i. Restore registers.
        pop     esi ebx
; 4. Register keyboard in the kernel.
; store pointer to keyboard_device_data in the stack
        push    eax
; call kernel API
        stdcall RegKeyboard, kbd_functions, eax
; restore pointer to keyboard_device_data from the stack,
; putting keyboard handle from API to the stack
        xchg    eax, [esp]
; put keyboard handle from API from the stack to keyboard_device_data field
        pop     [eax+keyboard_device_data.handle]
; If failed, free keyboard_device_data and return NULL.
        cmp     [eax+keyboard_device_data.handle], 0
        jz      .fail_free
; 5. Return pointer to keyboard_device_data.
.nothing:
        ret
.fail_free:
        call    Kfree
        xor     eax, eax
        ret
endp

; This procedure is called when HID layer detects disconnect of a previously
; connected keyboard.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
proc keyboard_driver_disconnect
; 1. If an autorepeat timer is active, stop it.
        cmp     [edi+keyboard_device_data.timer], 0
        jz      @f
        stdcall CancelTimerHS, [edi+keyboard_device_data.timer]
@@:
; 2. Unregister keyboard in the kernel.
        stdcall DelKeyboard, [edi+keyboard_device_data.handle]
; We should free data in CloseKeyboard, not here.
        ret
endp

; This procedure is called when HID layer starts processing a new input packet
; from a keyboard.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
proc keyboard_driver_begin_packet
; Nothing to do.
        ret
endp

; This procedure is called when HID layer processes every non-empty array field group.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
; in: ecx = fields count (always nonzero), edx = pointer to fields values
; in: esi -> report_field_group
; out: CF set => group is ok, CF cleared => group should be ignored
proc keyboard_driver_array_overflow?
; The keyboard signals array overflow by filling the entire array with
; USAGE_KBD_ROLLOVER codes.
        mov     eax, [edx]      ; eax = first field in the array
        sub     eax, USAGE_KBD_ROLLOVER ; eax = 0 if overflow, nonzero otherwise
        neg     eax     ; CF cleared if eax was zero, CF set if eax was nonzero
        ret
endp

; This procedure is called from HID layer for every field.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
; in: ecx = field usage, edx = value, esi -> report_field_group
proc keyboard_driver_input_field
if HID_DUMP_UNCLAIMED
.unclaimed = default_driver_input_field
end if
; 1. Process normal keys:
; from USAGE_KBD_FIRST_KEY to USAGE_KBD_FIRST_KEY + normal_keys_number - 1,
; excluding zeroes in [normal_keys].
; 1a. Test whether usage is in the range.
        lea     eax, [ecx-USAGE_KBD_FIRST_KEY]
        cmp     eax, normal_keys_number
        jae     .not_normal_key
; 1b. If the corresponding entry in [normal_keys] is zero,
; pass this field to the default handler - if HID_DUMP_UNCLAIMED is enabled,
; default handler is default_driver_input_field, otherwise just ignore the field.
        cmp     [normal_keys + eax], 0
        jz      .unclaimed
; 1c. Get the scancode.
        movzx   ecx, [normal_keys + eax]
; 1d. Further actions are slightly different for key press and key release.
; Decide what to do.
        test    edx, edx
        jz      .normal_key_released
.normal_key_pressed:
; The key is pressed.
; 1e. Store the last pressed key for autorepeat.
        mov     [edi+keyboard_device_data.repeatkey], cl
; 1f. Copy bit 7 to CF and send scancode with bit 7 cleared.
        btr     ecx, 7
        call    .send_key
; 1g. Stop the previous autorepeat timer, if any.
        mov     eax, [edi+keyboard_device_data.timer]
        test    eax, eax
        jz      @f
        stdcall CancelTimerHS, eax
@@:
; 1h. Start the new autorepeat timer with 250 ms initial delay
; and 50 ms subsequent delays.
        stdcall TimerHS, 25, 5, autorepeat_timer, edi
        mov     [edi+keyboard_device_data.timer], eax
if ~HID_DUMP_UNCLAIMED
.unclaimed:
end if
        ret
.normal_key_released:
; The key is released.
; 1i. Stop the autorepeat timer if it is autorepeating the released key.
        cmp     [edi+keyboard_device_data.repeatkey], cl
        jnz     .no_stop_timer
        push    ecx
        mov     [edi+keyboard_device_data.repeatkey], 0
        mov     eax, [edi+keyboard_device_data.timer]
        test    eax, eax
        jz      @f
        stdcall CancelTimerHS, eax
        mov     [edi+keyboard_device_data.timer], 0
@@:
        pop     ecx
.no_stop_timer:
; 1j. Copy bit 7 to CF and send scancode with bit 7 set.
        bts     ecx, 7
        call    .send_key
        ret
.not_normal_key:
; 2. USAGE_KBD_NOEVENT is simply a filler for free array fields,
; ignore it.
        cmp     ecx, USAGE_KBD_NOEVENT
        jz      .nothing
; 3. Process modifiers: 8 keys starting at USAGE_KBD_LCTRL.
; 3a. Test whether usage is in range.
; If not, we don't know what this field means, so pass it to the default handler.
        lea     eax, [ecx-USAGE_KBD_LCTRL]
        cmp     eax, 8
        jae     .unclaimed
; 3b. Further actions are slightly different for modifier press
; and modifier release. Decide what to do.
        test    edx, edx
        jz      .modifier_not_pressed
.modifier_pressed:
; The modifier is pressed.
; 3c. Set the corresponding status bit.
; If it was not set, send the corresponding scancode to the kernel
; with bit 7 cleared.
        bts     [edi+keyboard_device_data.modifiers], eax
        jc      @f
        movzx   ecx, [control_keys+eax]
        btr     ecx, 7
        call    .send_key
@@:
.nothing:
        ret
.modifier_not_pressed:
; The modifier is not pressed.
; 3d. Clear the correspodning status bit.
; If it was set, send the corresponding scancode to the kernel
; with bit 7 set.
        btr     [edi+keyboard_device_data.modifiers], eax
        jnc     @f
        movzx   ecx, [control_keys+eax]
        bts     ecx, 7
        call    .send_key
@@:
        ret

; Helper procedure. Sends scancode from cl to the kernel.
; If CF is set, precede it with special code 0xE0.
.send_key:
        jnc     @f
        push    ecx
        mov     ecx, 0xE0
        call    SetKeyboardData
        pop     ecx
@@:
        call    SetKeyboardData
        ret
endp

; This procedure is called when HID layer ends processing a new input packet
; from a keyboard.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
proc keyboard_driver_end_packet
; Nothing to do.
        ret
endp

; Timer callback for SetTimerHS.
proc autorepeat_timer
virtual at esp
                dd      ?       ; return address
.data           dd      ?
end virtual
; Just resend the last pressed key.
        mov     eax, [.data]
        movzx   ecx, [eax+keyboard_device_data.repeatkey]
; Copy bit 7 to CF and send scancode with bit 7 cleared.
        btr     ecx, 7
        call    keyboard_driver_input_field.send_key
        ret     4
endp

; This function is called from the keyboard layer
; when it is safe to free keyboard data.
proc CloseKeyboard
virtual at esp
                dd      ?       ; return address
.device_data    dd      ?
end virtual
        mov     eax, [.device_data]
        call    Kfree
        ret     4
endp

; This function is called from the keyboard layer
; to update LED state on the keyboard.
proc SetKeyboardLights stdcall uses ebx esi edi, device_data, led_state
locals
size    dd      ?
endl
; 1. Get the pointer to the LED report.
; If there is no LED report, exit from the function.
        mov     ebx, [device_data]
        mov     esi, [ebx+keyboard_device_data.led_report]
        test    esi, esi
        jz      .nothing
; 2. Get report size in bytes.
; report.size is size in bits without possible report ID;
; if an ID is assigned, the size is one byte greater.
        mov     eax, [esi+report.size]
        add     eax, 7
        shr     eax, 3
        cmp     [esi+report.id], 0
        jz      @f
        inc     eax
@@:
        mov     [size], eax
; 3. Allocate memory for report + 8 bytes for setup packet.
; Dword-align size for subsequent rep stosd and bts.
; If failed, exit from the function.
        add     eax, 8 + 3
        and     eax, not 3
        push    eax
        call    Kmalloc
        pop     ecx
        test    eax, eax
        jz      .nothing
; 4. Zero-initialize output report.
        push    eax
        mov     edi, eax
        shr     ecx, 2
        xor     eax, eax
        rep stosd
        pop     edi
        add     edi, 8
; 5. Store report ID, if assigned. If not assigned, that would just write zero
; over zeroes.
        mov     edx, [esi+report.id]
        mov     [edi], edx
; 6. Set report bits corresponding to active indicators.
        mov     eax, [led_state]
        test    al, 1           ; PS/2 Scroll Lock
        jz      @f
        mov     ecx, [ebx+keyboard_device_data.scrolllock_bit]
        test    ecx, ecx
        js      @f
        bts     [edi], ecx
@@:
        test    al, 2           ; PS/2 Num Lock
        jz      @f
        mov     ecx, [ebx+keyboard_device_data.numlock_bit]
        test    ecx, ecx
        js      @f
        bts     [edi], ecx
@@:
        test    al, 4           ; PS/2 Caps Lock
        jz      @f
        mov     ecx, [ebx+keyboard_device_data.capslock_bit]
        test    ecx, ecx
        js      @f
        bts     [edi], ecx
@@:
; 7. Fill setup packet.
        shl     edx, 16         ; move Report ID to byte 2
        or      edx, 21h + \    ; Class-specific request to Interface
                (9 shl 8) + \   ; SET_REPORT
                (2 shl 24)      ; Report Type = Output
        lea     eax, [edi-8]
        mov     ebx, [ebx+keyboard_device_data.usbdev]
        mov     dword [eax], edx
        mov     edx, [size]
        shl     edx, 16         ; move Size to last word
        or      edx, [ebx+usb_device_data.interface_number]
        mov     [eax+4], edx
; 8. Submit output control request.
        stdcall USBControlTransferAsync, [ebx+usb_device_data.configpipe], \
                eax, edi, [size], after_set_keyboard_lights, ebx, 0
; If failed, free the buffer now.
; If succeeded, the callback will free the buffer.
        test    eax, eax
        jnz     .nothing
        lea     eax, [edi-8]
        call    Kfree
.nothing:
        ret
endp

; This procedure is called from the USB subsystem when the request initiated by
; SetKeyboardLights is completed, either successfully or unsuccessfully.
proc after_set_keyboard_lights
virtual at esp
                dd      ?       ; return address
.pipe           dd      ?
.status         dd      ?
.buffer         dd      ?
.length         dd      ?
.calldata       dd      ?
end virtual
; Ignore status, just free the buffer allocated by SetKeyboardLights.
        mov     eax, [.buffer]
        sub     eax, 8
        call    Kfree
        ret     20
endp
