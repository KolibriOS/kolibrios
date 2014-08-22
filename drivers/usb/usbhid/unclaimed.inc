; HID default driver, part of USBHID driver.
; Present only if compile-time setting HID_DUMP_UNCLAIMED is on.
; Active for those devices when we do not have a specialized driver.
; Just dumps everything to the debug board.

if HID_DUMP_UNCLAIMED
; Global constants.
; They are assembled in a macro to separate code and data;
; the code is located at the point of "include 'unclaimed.inc'",
; the data are collected when workers_globals is instantiated.
macro workers_globals
{
; include global constants from previous workers
        workers_globals
align 4
; Callbacks for HID layer.
default_driver:
        dd      default_driver_add_device
        dd      default_driver_disconnect
        dd      default_driver_begin_packet
        dd      default_driver_array_overflow?
        dd      default_driver_input_field
        dd      default_driver_end_packet
}
; This procedure is called when HID layer detects a new driverless device.
; in: ebx -> usb_device_data, edi -> collection
; out: eax = device-specific data or NULL on error
default_driver_add_device:
; just return something nonzero, no matter what
        xor     eax, eax
        inc     eax
        ret
; This procedure is called when HID layer processes every non-empty array field group.
; in: edi -> keyboard_device_data (pointer returned from keyboard_driver_add_device)
; in: ecx = fields count (always nonzero), edx = pointer to fields values
; in: esi -> report_field_group
; out: CF set => group is ok, CF cleared => group should be ignored
default_driver_array_overflow?:
; parse everything
        stc
        ret
; This procedure is called from HID layer for every field.
; in: ecx = field usage, edx = value, esi -> report_field_group
default_driver_input_field:
; Do not dump zero values in Variable fields,
; they are present even if the corresponding control is inactive.
        test    edx, edx
        jnz     @f
        test    byte [esi+report_field_group.flags], HID_FIELD_VARIABLE
        jnz     .nodump
@@:
        DEBUGF 1,'K : unclaimed HID input: usage=%x, value=%x\n',ecx,edx
.nodump:
; pass through
; Three nothing-to-do procedures.
default_driver_disconnect:
default_driver_begin_packet:
default_driver_end_packet:
        ret
end if
