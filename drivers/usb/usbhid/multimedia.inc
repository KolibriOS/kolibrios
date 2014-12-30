; HID multimedia keyboard driver, part of USBHID driver.

; Global constants.
; They are assembled in a macro to separate code and data;
; the code is located at the point of "include 'multimedia.inc'",
; the data are collected when workers_globals is instantiated.
macro workers_globals
{
; include global constants from previous workers
        workers_globals
align 4
; Callbacks for HID layer.
multimedia_driver:
        dd      multimedia_driver_add_device
        dd      multimedia_driver_disconnect
        dd      multimedia_driver_begin_packet
        dd      multimedia_driver_array_overflow?
        dd      multimedia_driver_input_field
        dd      multimedia_driver_end_packet
}

; Data that are specific for one keyboard device.
struct multimedia_device_data
usbdev          dd      ?       ; pointer to device_data of USB and HID layers
last_pressed    dd      ?
ends

; This procedure is called when HID layer detects a new keyboard.
; in: ebx -> usb_device_data, edi -> collection
; out: eax = device-specific data or NULL on error
proc multimedia_driver_add_device
; 1. Allocate memory for keyboard_device_data. If failed, return NULL.
        movi    eax, sizeof.multimedia_device_data
        invoke  Kmalloc
        test    eax, eax
        jz      .nothing
; 2. Initialize keyboard_device_data: store pointer to USB layer data,
; zero some fields, initialize bit positions to -1.
        mov     [eax+multimedia_device_data.usbdev], ebx
        mov     [eax+multimedia_device_data.last_pressed], 0
.nothing:
        ret
endp

; This procedure is called when HID layer detects disconnect of a previously
; connected keyboard.
; in: edi -> multimedia_device_data (pointer returned from multimedia_driver_add_device)
proc multimedia_driver_disconnect
; We should free data in CloseKeyboard, not here.
        ret
endp

; This procedure is called when HID layer starts processing a new input packet
; from a keyboard.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
proc multimedia_driver_begin_packet
; Nothing to do.
        ret
endp

; This procedure is called when HID layer processes every non-empty array field group.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
; in: ecx = fields count (always nonzero), edx = pointer to fields values
; in: esi -> report_field_group
; out: CF set => group is ok, CF cleared => group should be ignored
proc multimedia_driver_array_overflow?
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
proc multimedia_driver_input_field
if HID_DUMP_UNCLAIMED
.unclaimed = default_driver_input_field
end if

        test    edx, edx
        jnz     @f
        cmp     [edi+multimedia_device_data.last_pressed], ecx
        jne     .nothing
@@:
        mov     eax, 0x19
        cmp     ecx, USAGE_CONSUMER + 0xB5      ; Next track
        je      .multimedia_key
        mov     al, 0x10
        cmp     ecx, USAGE_CONSUMER + 0xB6      ; Previous track
        je      .multimedia_key
        mov     al, 0x24
        cmp     ecx, USAGE_CONSUMER + 0xB7      ; Stop
        je      .multimedia_key
        mov     al, 0x22
        cmp     ecx, USAGE_CONSUMER + 0xCD      ; Play/pause
        je      .multimedia_key
        mov     al, 0x20
        cmp     ecx, USAGE_CONSUMER + 0xE2      ; Mute
        je      .multimedia_key
        mov     al, 0x30
        cmp     ecx, USAGE_CONSUMER + 0xE9      ; Volume up
        je      .multimedia_key
        mov     al, 0x2E
        cmp     ecx, USAGE_CONSUMER + 0xEA      ; Volume down
        je      .multimedia_key
        mov     al, 0x6D
        cmp     ecx, USAGE_CONSUMER + 0x183     ; Media select
        je      .multimedia_key
        mov     al, 0x6C
        cmp     ecx, USAGE_CONSUMER + 0x18A     ; E-Mail
        je      .multimedia_key
        mov     al, 0x21
        cmp     ecx, USAGE_CONSUMER + 0x192     ; Calculator
        je      .multimedia_key
        mov     al, 0x6B
        cmp     ecx, USAGE_CONSUMER + 0x194     ; My computer
        je      .multimedia_key
        mov     al, 0x65
        cmp     ecx, USAGE_CONSUMER + 0x221     ; WWW Search
        je      .multimedia_key
        mov     al, 0x32
        cmp     ecx, USAGE_CONSUMER + 0x223     ; WWW Home
        je      .multimedia_key
        mov     al, 0x6a
        cmp     ecx, USAGE_CONSUMER + 0x224     ; WWW Back
        je      .multimedia_key
        mov     al, 0x69
        cmp     ecx, USAGE_CONSUMER + 0x225     ; WWW forward
        je      .multimedia_key
        mov     al, 0x68
        cmp     ecx, USAGE_CONSUMER + 0x226     ; WWW Stop
        je      .multimedia_key
        mov     al, 0x67
        cmp     ecx, USAGE_CONSUMER + 0x227     ; WWW refresh
        je      .multimedia_key
        mov     al, 0x66
        cmp     ecx, USAGE_CONSUMER + 0x22A     ; WWW favorites
        je      .multimedia_key
        jmp     .unclaimed


.multimedia_key:
; 1d. Further actions are slightly different for key press and key release.
; Decide what to do.
        test    edx, edx
        jz      .multimedia_key_released
.multimedia_key_pressed:
        mov     [edi+multimedia_device_data.last_pressed], eax
; The key is pressed.
        push    eax
        mov     ecx, 0xE0
        invoke  SetKeyboardData
        pop     ecx
        invoke  SetKeyboardData
        ret

.multimedia_key_released:
        mov     [edi+multimedia_device_data.last_pressed], 0
; The key is released.
        or      cl, 0x80
        push    eax
        mov     ecx, 0xE0
        invoke  SetKeyboardData
        pop     ecx
        invoke  SetKeyboardData
        ret
.nothing:
        ret

endp

; This procedure is called when HID layer ends processing a new input packet
; from a keyboard.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
proc multimedia_driver_end_packet
; Nothing to do.
        ret
endp