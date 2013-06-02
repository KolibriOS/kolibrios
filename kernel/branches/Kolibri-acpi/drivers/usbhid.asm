; standard driver stuff
format MS COFF

DEBUG = 1

; this is for DEBUGF macro from 'fdo.inc'
__DEBUG__ = 1
__DEBUG_LEVEL__ = 1

include 'proc32.inc'
include 'imports.inc'
include 'fdo.inc'

public START
public version

; USB constants
DEVICE_DESCR_TYPE = 1
CONFIG_DESCR_TYPE = 2
STRING_DESCR_TYPE = 3
INTERFACE_DESCR_TYPE = 4
ENDPOINT_DESCR_TYPE = 5
DEVICE_QUALIFIER_DESCR_TYPE = 6

CONTROL_PIPE = 0
ISOCHRONOUS_PIPE = 1
BULK_PIPE = 2
INTERRUPT_PIPE = 3

; USB structures
virtual at 0
config_descr:
.bLength                db      ?
.bDescriptorType        db      ?
.wTotalLength           dw      ?
.bNumInterfaces         db      ?
.bConfigurationValue    db      ?
.iConfiguration         db      ?
.bmAttributes           db      ?
.bMaxPower              db      ?
.sizeof:
end virtual

virtual at 0
interface_descr:
.bLength                db      ?
.bDescriptorType        db      ?
.bInterfaceNumber       db      ?
.bAlternateSetting      db      ?
.bNumEndpoints          db      ?
.bInterfaceClass        db      ?
.bInterfaceSubClass     db      ?
.bInterfaceProtocol     db      ?
.iInterface             db      ?
.sizeof:
end virtual

virtual at 0
endpoint_descr:
.bLength                db      ?
.bDescriptorType        db      ?
.bEndpointAddress       db      ?
.bmAttributes           db      ?
.wMaxPacketSize         dw      ?
.bInterval              db      ?
.sizeof:
end virtual

; Driver data for all devices
virtual at 0
device_data:
.type           dd      ?       ; 1 = keyboard, 2 = mouse
.intpipe        dd      ?       ; interrupt pipe handle
.packetsize     dd      ?
.packet         rb      8       ; packet with data from device
.control        rb      8       ; control packet to device
.sizeof:
end virtual

; Driver data for mouse
virtual at device_data.sizeof
mouse_data:
; no additional data
.sizeof:
end virtual

; Driver data for keyboard
virtual at device_data.sizeof
keyboard_data:
.handle         dd      ?       ; keyboard handle from RegKeyboard
.configpipe     dd      ?       ; config pipe handle
.prevpacket     rb      8       ; previous packet with data from device
.timer          dd      ?       ; auto-repeat timer handle
.repeatkey      db      ?       ; auto-repeat key code
.ledstate       db      ?       ; state of LEDs
                align 4
.sizeof:
end virtual

section '.flat' code readable align 16
; The start procedure.
START:
; 1. Test whether the procedure is called with the argument DRV_ENTRY.
; If not, return 0.
        xor     eax, eax        ; initialize return value
        cmp     dword [esp+4], 1        ; compare the argument
        jnz     .nothing
; 2. Register self as a USB driver.
; The name is my_driver = 'usbhid'; IOCTL interface is not supported;
; usb_functions is an offset of a structure with callback functions.
        stdcall RegUSBDriver, my_driver, eax, usb_functions
; 3. Return the returned value of RegUSBDriver.
.nothing:
        ret     4

; This procedure is called when new HID device is detected.
; It initializes the device.
AddDevice:
; Arguments are addressed through esp. In this point of the function,
; [esp+4] = a handle of the config pipe, [esp+8] points to config_descr
; structure, [esp+12] points to interface_descr structure.
; 1. Check device type. Currently only mice and keyboards with
; boot protocol are supported.
; 1a. Get the subclass and the protocol. Since bInterfaceSubClass and
; bInterfaceProtocol are subsequent in interface_descr, just one
; memory reference is used for both.
        mov     edx, [esp+12]
        push    ebx     ; save used register to be stdcall
        mov     cx, word [edx+interface_descr.bInterfaceSubClass]
; 1b. For boot protocol, subclass must be 1 and protocol must be either 1 for
; a keyboard or 2 for a mouse. Check.
        cmp     cx, 0x0101
        jz      .keyboard
        cmp     cx, 0x0201
        jz      .mouse
; 1c. If the device is neither a keyboard nor a mouse, print a message and
; go to 6c.
        DEBUGF 1,'K : unknown HID device\n'
        jmp     .nothing
; 1d. If the device is a keyboard or a mouse, print a message and continue
; configuring.
.keyboard:
        DEBUGF 1,'K : USB keyboard detected\n'
        push    keyboard_data.sizeof
        jmp     .common
.mouse:
        DEBUGF 1,'K : USB mouse detected\n'
        push    mouse_data.sizeof
.common:
; 2. Allocate memory for device data.
        pop     eax     ; get size of device data
; 2a. Call the kernel, saving and restoring register edx.
        push    edx
        call    Kmalloc
        pop     edx
; 2b. Check result. If failed, say a message and go to 6c.
        test    eax, eax
        jnz     @f
        DEBUGF 1,'K : no memory\n'
        jmp     .nothing
@@:
        xchg    eax, ebx
; HID devices use one IN interrupt endpoint for polling the device
; and an optional OUT interrupt endpoint. We do not use the later,
; but must locate the first. Look for the IN interrupt endpoint.
; 3. Get the upper bound of all descriptors' data.
        mov     eax, [esp+8+4]  ; configuration descriptor
        movzx   ecx, [eax+config_descr.wTotalLength]
        add     eax, ecx
; 4. Loop over all descriptors until
; either end-of-data reached - this is fail
; or interface descriptor found - this is fail, all further data
;    correspond to that interface
; or endpoint descriptor found.
; 4a. Loop start: eax points to the interface descriptor.
.lookep:
; 4b. Get next descriptor.
        movzx   ecx, byte [edx] ; the first byte of all descriptors is length
        add     edx, ecx
; 4c. Check that at least two bytes are readable. The opposite is an error.
        inc     edx
        cmp     edx, eax
        jae     .errorep
        dec     edx
; 4d. Check that this descriptor is not interface descriptor. The opposite is
; an error.
        cmp     byte [edx+endpoint_descr.bDescriptorType], INTERFACE_DESCR_TYPE
        jz      .errorep
; 4e. Test whether this descriptor is an endpoint descriptor. If not, continue
; the loop.
        cmp     byte [edx+endpoint_descr.bDescriptorType], ENDPOINT_DESCR_TYPE
        jnz     .lookep
; 5. Check that the descriptor contains all required data and all data are
; readable. If so, proceed to 7.
        cmp     byte [edx+endpoint_descr.bLength], endpoint_descr.sizeof
        jb      .errorep
        sub     eax, endpoint_descr.sizeof
        cmp     edx, eax
        jbe     @f
; 6. An error occured during processing endpoint descriptor.
.errorep:
; 6a. Print a message.
        DEBUGF 1,'K : error: invalid endpoint descriptor\n'
; 6b. Free memory allocated for device data.
.free:
        xchg    eax, ebx
        call    Kfree
.nothing:
; 6c. Return an error.
        xor     eax, eax
        pop     ebx
        ret     12
@@:
; 7. Check that the endpoint is IN interrupt endpoint. If not, go to 6.
        test    [edx+endpoint_descr.bEndpointAddress], 80h
        jz      .errorep
        mov     cl, [edx+endpoint_descr.bmAttributes]
        and     cl, 3
        cmp     cl, INTERRUPT_PIPE
        jnz     .errorep
; 8. Open pipe for the endpoint.
; 8a. Load parameters from the descriptor.
        movzx   ecx, [edx+endpoint_descr.bEndpointAddress]
        movzx   eax, [edx+endpoint_descr.bInterval]
        movzx   edx, [edx+endpoint_descr.wMaxPacketSize]
; 8b. Call the kernel, saving and restoring edx.
        push    edx
        stdcall USBOpenPipe, [esp+4+24], ecx, edx, INTERRUPT_PIPE, eax
        pop     edx
; 8c. Check result. If failed, go to 6b.
        test    eax, eax
        jz      .free
; We use 12 bytes for device type, interrupt pipe and interrupt packet size,
; 8 bytes for a packet and 8 bytes for previous packet, used by a keyboard.
; 9. Initialize device data.
        mov     [ebx+device_data.intpipe], eax
        push    8
        pop     ecx
        cmp     edx, ecx
        jb      @f
        mov     edx, ecx
@@:
        xor     eax, eax
        mov     [ebx+device_data.packetsize], edx
        mov     dword [ebx+device_data.packet], eax
        mov     dword [ebx+device_data.packet+4], eax
        mov     edx, [esp+12+4] ; interface descriptor
        movzx   ecx, [edx+interface_descr.bInterfaceProtocol]
        mov     [ebx+device_data.type], ecx
        cmp     ecx, 1
        jnz     @f
        mov     [ebx+keyboard_data.handle], eax
        mov     [ebx+keyboard_data.timer], eax
        mov     [ebx+keyboard_data.repeatkey], al
        mov     dword [ebx+keyboard_data.prevpacket], eax
        mov     dword [ebx+keyboard_data.prevpacket+4], eax
        mov     eax, [esp+4+4]
        mov     [ebx+keyboard_data.configpipe], eax
@@:
; 10. Send the control packet SET_PROTOCOL(Boot Protocol) to the interface.
        lea     eax, [ebx+device_data.control]
        mov     dword [eax], 21h + (0Bh shl 8) + (0 shl 16)     ; class request to interface + SET_PROTOCOL + Boot protocol
        and     dword [eax+4], 0
        mov     dl, [edx+interface_descr.bInterfaceNumber]
        mov     [eax+4], dl
; Callback function is mouse_configured for mice and keyboard_configured1 for keyboards.
        mov     edx, keyboard_configured1
        cmp     ecx, 1
        jz      @f
        mov     edx, mouse_configured
@@:
        stdcall USBControlTransferAsync, [esp+4+28], eax, 0, 0, edx, ebx, 0
; 11. Return with pointer to device data as returned value.
        xchg    eax, ebx
        pop     ebx
        ret     12

; This function is called when SET_PROTOCOL command for keyboard is done,
; either successful or unsuccessful.
keyboard_configured1:
        xor     edx, edx
; 1. Check the status of the transfer.
; If the transfer was failed, go to the common error handler.
        cmp     dword [esp+8], edx      ; status is zero?
        jnz     keyboard_data_ready.error
; 2. Send the control packet SET_IDLE(infinity). HID auto-repeat is not useful.
        mov     eax, [esp+20]
        push    edx     ; flags for USBControlTransferAsync
        push    eax     ; userdata for USBControlTransferAsync
        add     eax, device_data.control
        mov     dword [eax], 21h + (0Ah shl 8) + (0 shl 24)     ; class request to interface + SET_IDLE + no autorepeat
        stdcall USBControlTransferAsync, dword [eax+keyboard_data.configpipe-device_data.control], \
                eax, edx, edx, keyboard_configured2; , <userdata>, <flags>
; 3. Return.
        ret     20

; This function is called when SET_IDLE command for keyboard is done,
; either successful or unsuccessful.
keyboard_configured2:
; Check the status of the transfer and go to the corresponding label
; in the main handler.
        cmp     dword [esp+8], 0
        jnz     keyboard_data_ready.error
        mov     edx, [esp+20]
        push    edx
        stdcall RegKeyboard, usbkbd_functions, edx
        pop     edx
        mov     [edx+keyboard_data.handle], eax
        jmp     keyboard_data_ready.next

; This function is called when another interrupt packet arrives,
; processed either successfully or unsuccessfully.
; It should parse the packet and initiate another transfer with
; the same callback function.
keyboard_data_ready:
; 1. Check the status of the transfer.
        mov     eax, [esp+8]
        test    eax, eax
        jnz     .error
; Parse the packet, comparing with the previous packet.
; For boot protocol, USB keyboard packet consists of the first byte
; with status keys that are currently pressed. The second byte should
; be ignored, and other 5 bytes denote keys that are currently pressed.
        push    esi ebx         ; save used registers to be stdcall
; 2. Process control keys.
; 2a. Initialize before loop for control keys. edx = mask for control bits
; that were changed.
        mov     ebx, [esp+20+8]
        movzx   edx, byte [ebx+device_data.packet]      ; get state of control keys
        xor     dl, byte [ebx+keyboard_data.prevpacket] ; compare with previous state
; 2b. If state of control keys has not changed, advance to 3.
        jz      .nocontrol
; 2c. Otherwise, loop over control keys; esi = bit number.
        xor     esi, esi
.controlloop:
; 2d. Skip bits that have not changed.
        bt      edx, esi
        jnc     .controlnext
        push    edx     ; save register which is possibly modified by API
; The state of the current control key has changed.
; 2e. For extended control keys, send the prefix 0xE0.
        mov     al, [control_keys+esi]
        test    al, al
        jns     @f
        push    eax
        mov     ecx, 0xE0
        call    SetKeyboardData
        pop     eax
        and     al, 0x7F
@@:
; 2f. If the current state of the control key is "pressed", send normal
; scancode. Otherwise, the key is released, so set the high bit in scancode.
        movzx   ecx, al
        bt      dword [ebx+device_data.packet], esi
        jc      @f
        or      cl, 0x80
@@:
        call    SetKeyboardData
        pop     edx     ; restore register which was possibly modified by API
.controlnext:
; 2g. We have 8 control keys.
        inc     esi
        cmp     esi, 8
        jb      .controlloop
.nocontrol:
; 3. Initialize before loop for normal keys. esi = index.
        push    2
        pop     esi
.normalloop:
; 4. Process one key which was pressed in the previous packet.
; 4a. Get the next pressed key from the previous packet.
        movzx   eax, byte [ebx+esi+keyboard_data.prevpacket]
; 4b. Ignore special codes.
        cmp     al, 3
        jbe     .normalnext1
; 4c. Ignore keys that are still pressed in the current packet.
        lea     ecx, [ebx+device_data.packet]
        call    haskey
        jz      .normalnext1
; 4d. Say warning about keys with strange codes.
        cmp     eax, normal_keys_number
        jae     .badkey1
        movzx   ecx, [normal_keys+eax]
        jecxz   .badkey1
; 4e. For extended keys, send the prefix 0xE0.
        push    ecx     ; save keycode
        test    cl, cl
        jns     @f
        push    ecx
        mov     ecx, 0xE0
        call    SetKeyboardData
        pop     ecx
@@:
; 4f. Send the release event.
        or      cl, 0x80
        call    SetKeyboardData
; 4g. If this key is autorepeating, stop the timer.
        pop     ecx     ; restore keycode
        cmp     cl, [ebx+keyboard_data.repeatkey]
        jnz     .normalnext1
        mov     eax, [ebx+keyboard_data.timer]
        test    eax, eax
        jz      .normalnext1
        stdcall CancelTimerHS, eax
        and     [ebx+keyboard_data.timer], 0
        jmp     .normalnext1
.badkey1:
        DEBUGF 1,'K : unknown keycode: %x\n',al
.normalnext1:
; 5. Process one key which is pressed in the current packet.
; 5a. Get the next pressed key from the current packet.
        movzx   eax, byte [ebx+esi+device_data.packet]
; 5b. Ignore special codes.
        cmp     al, 3
        jbe     .normalnext2
; 5c. Ignore keys that were already pressed in the previous packet.
        lea     ecx, [ebx+keyboard_data.prevpacket]
        call    haskey
        jz      .normalnext2
; 5d. Say warning about keys with strange codes.
        cmp     eax, normal_keys_number
        jae     .badkey2
        movzx   ecx, [normal_keys+eax]
        jecxz   .badkey2
; 5e. For extended keys, send the prefix 0xE0.
        push    ecx     ; save keycode
        test    cl, cl
        jns     @f
        push    ecx
        mov     ecx, 0xE0
        call    SetKeyboardData
        pop     ecx
@@:
; 5f. Send the press event.
        and     cl, not 0x80
        call    SetKeyboardData
; 5g. Stop the current auto-repeat timer, if present.
        mov     eax, [ebx+keyboard_data.timer]
        test    eax, eax
        jz      @f
        stdcall CancelTimerHS, eax
@@:
; 5h. Start the auto-repeat timer.
        pop     ecx     ; restore keycode
        mov     [ebx+keyboard_data.repeatkey], cl
        stdcall TimerHS, 25, 5, autorepeat_timer, ebx
        mov     [ebx+keyboard_data.timer], eax
        jmp     .normalnext2
.badkey2:
        DEBUGF 1,'K : unknown keycode: %x\n',al
.normalnext2:
; 6. Advance to next key.
        inc     esi
        cmp     esi, 8
        jb      .normalloop
; 7. Save the packet data for future reference.
        mov     eax, dword [ebx+device_data.packet]
        mov     dword [ebx+keyboard_data.prevpacket], eax
        mov     eax, dword [ebx+device_data.packet+4]
        mov     dword [ebx+keyboard_data.prevpacket+4], eax
        pop     ebx esi ; restore registers to be stdcall
.next:
; 8. Initiate transfer on the interrupt pipe.
        mov     eax, [esp+20]
        push    1       ; flags for USBNormalTransferAsync
        push    eax     ; userdata for USBNormalTransferAsync
        add     eax, device_data.packet
        stdcall USBNormalTransferAsync, dword [eax+device_data.intpipe-device_data.packet], \
                eax, dword [eax+device_data.packetsize-device_data.packet], \
                keyboard_data_ready;, <userdata>, <flags>
; 9. Return.
.nothing:
        ret     20
.error:
; An error has occured.
; 10. If an error is caused by the disconnect, do nothing, it is handled
; in DeviceDisconnected. Otherwise, say a message.
        cmp     eax, 16
        jz      @f
        push    esi
        mov     esi, errormsgkbd
        call    SysMsgBoardStr
        pop     esi
@@:
        ret     20

; Auxiliary procedure for keyboard_data_ready.
haskey:
        push    2
        pop     edx
@@:
        cmp     byte [ecx+edx], al
        jz      @f
        inc     edx
        cmp     edx, 7
        jbe     @b
@@:
        ret

; Timer function for auto-repeat.
autorepeat_timer:
        mov     eax, [esp+4]
        movzx   ecx, [eax+keyboard_data.repeatkey]
        test    cl, cl
        jns     @f
        push    ecx
        mov     ecx, 0xE0
        call    SetKeyboardData
        pop     ecx
        and     cl, not 0x80
@@:
        call    SetKeyboardData
        ret     4

; This function is called to update LED state on the keyboard.
SetKeyboardLights:
        mov     eax, [esp+4]
        add     eax, device_data.control
        mov     dword [eax], 21h + (9 shl 8) + (2 shl 24)
                ; class request to interface + SET_REPORT + Output zero report
        mov     byte [eax+6], 1
        mov     edx, [esp+8]
        shr     dl, 1
        jnc     @f
        or      dl, 4
@@:
        lea     ecx, [eax+keyboard_data.ledstate-device_data.control]
        mov     [ecx], dl
        stdcall USBControlTransferAsync, dword [eax+keyboard_data.configpipe-device_data.control], \
                eax, ecx, 1, keyboard_data_ready.nothing, 0, 0
        ret     8

; This function is called when it is safe to free keyboard data.
CloseKeyboard:
        mov     eax, [esp+4]
        push    ebx
        call    Kfree
        pop     ebx
        ret     4

; This function is called when SET_PROTOCOL command for mouse is done,
; either successful or unsuccessful.
mouse_configured:
; Check the status of the transfer and go to the corresponding label
; in the main handler.
        cmp     dword [esp+8], 0
        jnz     mouse_data_ready.error
        mov     eax, [esp+20]
        add     eax, device_data.packet
        jmp     mouse_data_ready.next

; This function is called when another interrupt packet arrives,
; processed either successfully or unsuccessfully.
; It should parse the packet and initiate another transfer with
; the same callback function.
mouse_data_ready:
; 1. Check the status of the transfer.
        mov     eax, [esp+8]
        test    eax, eax
        jnz     .error
        mov     edx, [esp+16]
; 2. Parse the packet.
; For boot protocol, USB mouse packet consists of at least 3 bytes.
; The first byte is state of mouse buttons, the next two bytes are
; x and y movements.
; Normal mice do not distinguish between boot protocol and report protocol;
; in this case, scroll data are also present. Advanced mice, however,
; support two different protocols, boot protocol is used for compatibility
; and does not contain extended buttons or scroll data.
        mov     eax, [esp+12]   ; buffer
        push    eax
        xor     ecx, ecx
        cmp     edx, 4
        jbe     @f
        movsx   ecx, byte [eax+4]
@@:
        push    ecx
        xor     ecx, ecx
        cmp     edx, 3
        jbe     @f
        movsx   ecx, byte [eax+3]
        neg     ecx
@@:
        push    ecx
        xor     ecx, ecx
        cmp     edx, 2
        jbe     @f
        movsx   ecx, byte [eax+2]
        neg     ecx
@@:
        push    ecx
        movsx   ecx, byte [eax+1]
        push    ecx
        movzx   ecx, byte [eax]
        push    ecx
        call    SetMouseData
        pop     eax
.next:
; 3. Initiate transfer on the interrupt pipe.
        stdcall USBNormalTransferAsync, dword [eax+device_data.intpipe-device_data.packet], \
                eax, dword [eax+device_data.packetsize-device_data.packet], mouse_data_ready, eax, 1
; 4. Return.
        ret     20
.error:
; An error has occured.
; 5. If an error is caused by the disconnect, do nothing, it is handled
; in DeviceDisconnected. Otherwise, say a message.
        cmp     eax, 16
        jz      @f
        push    esi
        mov     esi, errormsgmouse
        call    SysMsgBoardStr
        pop     esi
@@:
        ret     20

; This function is called when the device is disconnected.
DeviceDisconnected:
        push    ebx     ; save used register to be stdcall
; 1. Say a message. Use different messages for keyboards and mice.
        mov     ebx, [esp+4+4]
        push    esi
        mov     esi, disconnectmsgk
        cmp     byte [ebx+device_data.type], 1
        jz      @f
        mov     esi, disconnectmsgm
@@:
        stdcall SysMsgBoardStr
        pop     esi
; 2. If device is keyboard, then we must unregister it as a keyboard and
; possibly stop the auto-repeat timer.
        cmp     byte [ebx+device_data.type], 1
        jnz     .nokbd
        mov     eax, [ebx+keyboard_data.timer]
        test    eax, eax
        jz      @f
        stdcall CancelTimerHS, eax
@@:
        mov     ecx, [ebx+keyboard_data.handle]
        jecxz   .nokbd
        stdcall DelKeyboard, ecx
; If keyboard is registered, then we should free data in CloseKeyboard, not here.
        jmp     .nothing
.nokbd:
; 3. Free the device data.
        xchg    eax, ebx
        call    Kfree
; 4. Return.
.nothing:
        pop     ebx     ; restore used register to be stdcall
        ret     4       ; purge one dword argument to be stdcall

; strings
my_driver       db      'usbhid',0
errormsgmouse   db      'K : USB transfer error, disabling mouse',10,0
errormsgkbd     db      'K : USB transfer error, disabling keyboard',10,0
disconnectmsgm  db      'K : USB mouse disconnected',10,0
disconnectmsgk  db      'K : USB keyboard disconnected',10,0

; data for keyboard: correspondence between HID usage keys and PS/2 scancodes.
EX = 80h
label control_keys byte
        db      1Dh, 2Ah, 38h, 5Bh+EX, 1Dh+EX, 36h, 38h+EX, 5Ch+EX
label normal_keys byte
        db      00h, 00h, 00h, 00h, 1Eh, 30h, 2Eh, 20h, 12h, 21h, 22h, 23h, 17h, 24h, 25h, 26h  ; 0x
        db      32h, 31h, 18h, 19h, 10h, 13h, 1Fh, 14h, 16h, 2Fh, 11h, 2Dh, 15h, 2Ch, 02h, 03h  ; 1x
        db      04h, 05h, 06h, 07h, 08h, 09h, 0Ah, 0Bh, 1Ch, 01h, 0Eh, 0Fh, 39h, 0Ch, 0Dh, 1Ah  ; 2x
        db      1Bh, 2Bh, 2Bh, 27h, 28h, 29h, 33h, 34h, 35h, 3Ah, 3Bh, 3Ch, 3Dh, 3Eh, 3Fh, 40h  ; 3x
        db      41h, 42h, 43h, 44h, 57h, 58h,37h+EX,46h,0,52h+EX,47h+EX,49h+EX,53h+EX,4Fh+EX,51h+EX,4Dh+EX      ; 4x
        db      4Bh+EX,50h+EX,48h+EX,45h,35h+EX,37h,4Ah,4Eh,1Ch+EX,4Fh,50h,51h,4Bh,4Ch,4Dh,47h  ; 5x
        db      48h, 49h, 52h, 53h, 56h,5Dh+EX,5Eh+EX,59h,64h,65h,66h, 67h, 68h, 69h, 6Ah, 6Bh  ; 6x
        db      6Ch, 6Dh, 6Eh, 76h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h, 00h  ; 7x
        db      00h, 00h, 00h, 00h, 00h, 7Eh, 00h, 73h, 70h, 7Dh, 79h, 7Bh, 5Ch, 00h, 00h, 00h  ; 8x
        db      0F2h,0F1h,78h, 77h, 76h
normal_keys_number = $ - normal_keys

; Exported variable: kernel API version.
align 4
version dd      50005h
; Structure with callback functions.
usb_functions:
        dd      12
        dd      AddDevice
        dd      DeviceDisconnected

; Structure with callback functions for keyboards.
usbkbd_functions:
        dd      12
        dd      CloseKeyboard
        dd      SetKeyboardLights

; for DEBUGF macro
include_debug_strings

; for uninitialized data
section '.data' data readable writable align 16
