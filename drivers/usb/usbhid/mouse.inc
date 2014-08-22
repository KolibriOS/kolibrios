; HID mouse driver, part of USBHID driver.

; Global constants.
; They are assembled in a macro to separate code and data;
; the code is located at the point of "include 'mouse.inc'",
; the data are collected when workers_globals is instantiated.
macro workers_globals
{
; include global constants from previous workers
        workers_globals
align 4
; Callbacks for HID layer.
mouse_driver:
        dd      mouse_driver_add_device
        dd      mouse_driver_disconnect
        dd      mouse_driver_begin_packet
        dd      mouse_driver_array_overflow?
        dd      mouse_driver_input_field
        dd      mouse_driver_end_packet
}

; Data that are specific for one mouse device.
struct mouse_device_data
buttons         dd      ?       ; buttons that are currently pressed
dx              dd      ?       ; current x moving
dy              dd      ?       ; current y moving
wheel           dd      ?       ; current wheel moving
hwheel          dd      ?
ends

; This procedure is called when HID layer detects a new mouse.
; in: ebx -> device_data from USB layer, edi -> collection
; out: eax = device-specific data or NULL on error
proc mouse_driver_add_device
; Just allocate memory; no initialization needed.
        movi    eax, sizeof.mouse_device_data
        invoke  Kmalloc
        ret
endp

; This procedure is called when HID layer detects disconnect of a previously
; connected mouse.
; in: edi -> mouse_device_data (pointer returned from mouse_driver_add_device)
proc mouse_driver_disconnect
; Free the allocated memory.
        mov     eax, edi
        invoke  Kfree
        ret
endp

; This procedure is called when HID layer starts processing a new input packet
; from a mouse.
; in: edi -> mouse_device_data (pointer returned from mouse_driver_add_device)
proc mouse_driver_begin_packet
; Zero all variables describing the current state.
        mov     [edi+mouse_device_data.buttons], 0
        mov     [edi+mouse_device_data.dx], 0
        mov     [edi+mouse_device_data.dy], 0
        mov     [edi+mouse_device_data.wheel], 0
        mov     [edi+mouse_device_data.hwheel], 0
        ret
endp

; This procedure is called when HID layer processes every non-empty array field group.
; in: edi -> mouse_device_data (pointer returned from mouse_driver_add_device)
; in: ecx = fields count (always nonzero), edx = pointer to fields values
; in: esi -> report_field_group
; out: CF set => array is ok, CF cleared => array should be ignored
proc mouse_driver_array_overflow?
; no array fields, no overflows
        stc
        ret
endp

; This procedure is called from HID layer for every field.
; in: edi -> mouse_device_data (pointer returned from mouse_driver_add_device)
; in: ecx = field usage, edx = value, esi -> report_field_group
proc mouse_driver_input_field
; 1. Determine the handler. We process x/y moving, wheel and up to 32 buttons.
; Pass other fields to the default handler - default_driver_input_field if
; HID_DUMP_UNCLAIMED is enabled, just ignore otherwise.
        cmp     ecx, USAGE_GD_X
        jz      .x
        cmp     ecx, USAGE_GD_Y
        jz      .y
        cmp     ecx, USAGE_GD_WHEEL
        jz      .wheel
        cmp     ecx, 0xC0238
        jz      .hwheel
        sub     ecx, USAGE_BUTTON_PAGE + 1
        jb      .unclaimed
        cmp     ecx, 32
        jae     .unclaimed
; 2. This is a button.
; If a button is pressed, set the corresponding bit in the state.
; If a button is not pressed, do nothing.
        test    edx, edx
        jz      @f
        bts     [edi+mouse_device_data.buttons], ecx
@@:
if ~HID_DUMP_UNCLAIMED
.unclaimed:
end if
        ret
if HID_DUMP_UNCLAIMED
.unclaimed:
        add     ecx, USAGE_BUTTON_PAGE + 1
        jmp     default_driver_input_field
end if
.x:
; 3. This is x moving. For relative fields, store the value in the state.
; Pass absolute field to the default handler.
        test    byte [esi+report_field_group.flags], HID_FIELD_RELATIVE
        jz      .absolute_x
        mov     [edi+mouse_device_data.dx], edx
        ret
.y:
; 4. This is y moving. For relative fields, store the value in the state,
; changing the sign: HID uses "mathematics" scheme with Y axis increasing from
; bottom to top, the kernel expects "programming" PS/2-style with Y axis
; increasing from top to bottom.
; Pass absolute fields to the default handler.
        test    byte [esi+report_field_group.flags], HID_FIELD_RELATIVE
        jz      .absolute_y
        neg     edx
        mov     [edi+mouse_device_data.dy], edx
        ret
.wheel:
; 5. This is wheel event. For relative fields, store the value in the state,
; changing the sign. Pass absolute fields to the default handler.
        test    byte [esi+report_field_group.flags], HID_FIELD_RELATIVE
        jz      .unclaimed
        neg     edx
        mov     [edi+mouse_device_data.wheel], edx
        ret
.hwheel:
        test    byte [esi+report_field_group.flags], HID_FIELD_RELATIVE
        jz      .unclaimed
        mov     [edi+mouse_device_data.hwheel], edx
        ret
.absolute_x:
        mov     [edi+mouse_device_data.dx], edx
        or      [edi+mouse_device_data.buttons], 0x80000000
        ret
.absolute_y:
        mov     [edi+mouse_device_data.dy], edx
        or      [edi+mouse_device_data.buttons], 0x40000000
        ret
endp

; This procedure is called when HID layer ends processing a new input packet
; from a mouse.
; in: edi -> mouse_device_data (pointer returned from mouse_driver_add_device)
proc mouse_driver_end_packet
; Call the kernel, passing collected state.
        invoke  SetMouseData, \
                [edi+mouse_device_data.buttons], \
                [edi+mouse_device_data.dx], \
                [edi+mouse_device_data.dy], \
                [edi+mouse_device_data.wheel], \
                [edi+mouse_device_data.hwheel]
        ret
endp
