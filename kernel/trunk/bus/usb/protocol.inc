; Implementation of the USB protocol for device enumeration.
; Manage a USB device when it becomes ready for USB commands:
; configure, enumerate, load the corresponding driver(s),
; pass device information to the driver.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; USB standard request codes
USB_GET_STATUS        = 0
USB_CLEAR_FEATURE     = 1
USB_SET_FEATURE       = 3
USB_SET_ADDRESS       = 5
USB_GET_DESCRIPTOR    = 6
USB_SET_DESCRIPTOR    = 7
USB_GET_CONFIGURATION = 8
USB_SET_CONFIGURATION = 9
USB_GET_INTERFACE     = 10
USB_SET_INTERFACE     = 11
USB_SYNCH_FRAME       = 12

; USB standard descriptor types
USB_DEVICE_DESCR             = 1
USB_CONFIG_DESCR             = 2
USB_STRING_DESCR             = 3
USB_INTERFACE_DESCR          = 4
USB_ENDPOINT_DESCR           = 5
USB_DEVICE_QUALIFIER_DESCR   = 6
USB_OTHER_SPEED_CONFIG_DESCR = 7
USB_INTERFACE_POWER_DESCR    = 8

; Possible speeds of USB devices
USB_SPEED_FS = 0 ; full-speed
USB_SPEED_LS = 1 ; low-speed
USB_SPEED_HS = 2 ; high-speed

; Compile-time setting. If set, the code will dump all descriptors as they are
; read to the debug board.
USB_DUMP_DESCRIPTORS = 1

; =============================================================================
; ================================ Structures =================================
; =============================================================================
; USB descriptors. See USB specification for detailed explanations.
; First two bytes of every descriptor have the same meaning.
struct usb_descr
bLength                 db      ?
; Size of this descriptor in bytes
bDescriptorType         db      ?
; One of USB_*_DESCR constants.
ends

; USB device descriptor
struct usb_device_descr usb_descr
bcdUSB                  dw      ?
; USB Specification Release number in BCD, e.g. 110h = USB 1.1
bDeviceClass            db      ?
; USB Device Class Code
bDeviceSubClass         db      ?
; USB Device Subclass Code
bDeviceProtocol         db      ?
; USB Device Protocol Code
bMaxPacketSize0         db      ?
; Maximum packet size for zero endpoint
idVendor                dw      ?
; Vendor ID
idProduct               dw      ?
; Product ID
bcdDevice               dw      ?
; Device release number in BCD
iManufacturer           db      ?
; Index of string descriptor describing manufacturer
iProduct                db      ?
; Index of string descriptor describing product
iSerialNumber           db      ?
; Index of string descriptor describing serial number
bNumConfigurations      db      ?
; Number of possible configurations
ends

; USB configuration descriptor
struct usb_config_descr usb_descr
wTotalLength            dw      ?
; Total length of data returned for this configuration
bNumInterfaces          db      ?
; Number of interfaces in this configuration
bConfigurationValue     db      ?
; Value for SET_CONFIGURATION control request
iConfiguration          db      ?
; Index of string descriptor describing this configuration
bmAttributes            db      ?
; Bit 6 is SelfPowered, bit 5 is RemoteWakeupSupported,
; bit 7 must be 1, other bits must be 0
bMaxPower               db      ?
; Maximum power consumption from the bus in 2mA units
ends

; USB interface descriptor
struct usb_interface_descr usb_descr
; The following two fields work in pair. Sometimes one interface can work
; in different modes; e.g. videostream from web-cameras requires different
; bandwidth depending on resolution/quality/compression settings.
; Each mode of each interface has its own descriptor with its own endpoints
; following; all descriptors for one interface have the same bInterfaceNumber,
; and different bAlternateSetting.
; By default, any interface operates in mode with bAlternateSetting = 0.
; Often this is the only mode. If there are another modes, the active mode
; is selected by SET_INTERFACE(bAlternateSetting) control request.
bInterfaceNumber        db      ?
bAlternateSetting       db      ?
bNumEndpoints           db      ?
; Number of endpoints used by this interface, excluding zero endpoint
bInterfaceClass         db      ?
; USB Interface Class Code
bInterfaceSubClass      db      ?
; USB Interface Subclass Code
bInterfaceProtocol      db      ?
; USB Interface Protocol Code
iInterface              db      ?
; Index of string descriptor describing this interface
ends

; USB endpoint descriptor
struct usb_endpoint_descr usb_descr
bEndpointAddress        db      ?
; Lower 4 bits form endpoint number,
; upper bit is 0 for OUT endpoints and 1 for IN endpoints,
; other bits must be zero
bmAttributes            db      ?
; Lower 2 bits form transfer type, one of *_PIPE,
; other bits must be zero for non-isochronous endpoints;
; refer to the USB specification for meaning in isochronous case
wMaxPacketSize          dw      ?
; Lower 11 bits form maximum packet size,
; next two bits specify the number of additional transactions per microframe
; for high-speed periodic endpoints, other bits must be zero.
bInterval               db      ?
; Interval for polling endpoint for data transfers.
; Isochronous and high-speed interrupt endpoints: poll every 2^(bInterval-1)
; (micro)frames
; Full/low-speed interrupt endpoints: poll every bInterval frames
; High-speed bulk/control OUT endpoints: maximum NAK rate
ends

; =============================================================================
; =================================== Code ====================================
; =============================================================================

; When a new device is ready to be configured, a controller-specific code
; calls usb_new_device.
; The sequence of further actions:
; * open pipe for the zero endpoint (usb_new_device);
;   maximum packet size is not known yet, but it must be at least 8 bytes,
;   so it is safe to send packets with <= 8 bytes
; * issue SET_ADDRESS control request (usb_new_device)
; * set the new device address in the pipe (usb_set_address_callback)
; * notify a controller-specific code that initialization of other ports
;   can be started (usb_set_address_callback)
; * issue GET_DESCRIPTOR control request for first 8 bytes of device descriptor
;   (usb_after_set_address)
; * first 8 bytes of device descriptor contain the true packet size for zero
;   endpoint, so set the true packet size (usb_get_descr8_callback)
; * first 8 bytes of a descriptor contain the full size of this descriptor,
;   issue GET_DESCRIPTOR control request for the full device descriptor
;   (usb_after_set_endpoint_size)
; * issue GET_DESCRIPTOR control request for first 8 bytes of configuration
;   descriptor (usb_get_descr_callback)
; * issue GET_DESCRIPTOR control request for full configuration descriptor
;   (usb_know_length_callback)
; * issue SET_CONFIGURATION control request (usb_set_config_callback)
; * parse configuration descriptor, load the corresponding driver(s),
;   pass the configuration descriptor to the driver and let the driver do
;   the further work (usb_got_config_callback)

; This function is called from controller-specific part
; when a new device is ready to be configured.
; in: ecx -> pseudo-pipe, part of usb_pipe
; in: esi -> usb_controller
; in: [esi+usb_controller.ResettingHub] is the pointer to usb_hub for device,
;     NULL if the device is connected to the root hub
; in: [esi+usb_controller.ResettingPort] is the port for the device, zero-based
; in: [esi+usb_controller.ResettingSpeed] is the speed of the device, one of
;     USB_SPEED_xx.
; out: eax = 0 <=> failed, the caller should disable the port.
proc usb_new_device
        push    ebx edi         ; save used registers to be stdcall
; 1. Allocate resources. Any device uses the following resources:
; - device address in the bus
; - memory for device data
; - pipe for zero endpoint
; If some allocation fails, we must undo our actions. Closing the pipe
; is a hard task, so we avoid it and open the pipe as the last resource.
; The order for other two allocations is quite arbitrary.
; 1a. Allocate a bus address.
        push    ecx
        call    usb_set_address_request
        pop     ecx
; 1b. If failed, just return zero.
        test    eax, eax
        jz      .nothing
; 1c. Allocate memory for device data.
; For now, we need sizeof.usb_device_data and extra 8 bytes for GET_DESCRIPTOR
; input and output, see usb_after_set_address. Later we will reallocate it
; to actual size needed for descriptors.
        push    sizeof.usb_device_data + 8
        pop     eax
        push    ecx
        call    malloc
        pop     ecx
; 1d. If failed, free the bus address and return zero.
        test    eax, eax
        jz      .nomemory
; 1e. Open pipe for endpoint zero.
; For now, we do not know the actual maximum packet size;
; for full-speed devices it can be any of 8, 16, 32, 64 bytes,
; low-speed devices must have 8 bytes, high-speed devices must have 64 bytes.
; Thus, we must use some fake "maximum packet size" until the actual size
; will be known. However, the maximum packet size must be at least 8, and
; initial stages of the configuration process involves only packets of <= 8
; bytes, they will be transferred correctly as long as
; the fake "maximum packet size" is also at least 8.
; Thus, any number >= 8 is suitable for actual hardware.
; However, software emulation of EHCI in VirtualBox assumes that high-speed
; control transfers are those originating from pipes with max packet size = 64,
; even on early stages of the configuration process. This is incorrect,
; but we have no specific preferences, so let VirtualBox be happy and use 64
; as the fake "maximum packet size".
        push    eax
; We will need many zeroes.
; "push edi" is one byte, "push 0" is two bytes; save space, use edi.
        xor     edi, edi
        stdcall usb_open_pipe, ecx, edi, 64, edi, edi
; Put pointer to pipe into ebx. "xchg eax,reg" is one byte, mov is two bytes.
        xchg    eax, ebx
        pop     eax
; 1f. If failed, free the memory, the bus address and return zero.
        test    ebx, ebx
        jz      .freememory
; 2. Store pointer to device data in the pipe structure.
        mov     [ebx+usb_pipe.DeviceData], eax
; 3. Init device data, using usb_controller.Resetting* variables.
        mov     [eax+usb_device_data.NumPipes], 1
        mov     [eax+usb_device_data.ConfigDataSize], edi
        mov     [eax+usb_device_data.Interfaces], edi
        movzx   ecx, [esi+usb_controller.ResettingPort]
; Note: the following write zeroes
; usb_device_data.DeviceDescrSize, usb_device_data.NumInterfaces,
; usb_device_data.Speed.
        mov     dword [eax+usb_device_data.Port], ecx
        mov     dl, [esi+usb_controller.ResettingSpeed]
        mov     [eax+usb_device_data.Speed], dl
        mov     edx, [esi+usb_controller.ResettingHub]
        mov     [eax+usb_device_data.Hub], edx
; 4. Store pointer to the config pipe in the hub data.
; Config pipe serves as device identifier.
; Root hubs use the array inside usb_controller structure,
; non-root hubs use the array immediately after usb_hub structure.
        test    edx, edx
        jz      .roothub
        mov     edx, [edx+usb_hub.ConnectedDevicesPtr]
        mov     [edx+ecx*4], ebx
        jmp     @f
.roothub:
        mov     [esi+usb_controller.DevicesByPort+ecx*4], ebx
@@:
        call    usb_reinit_pipe_list
; 5. Issue SET_ADDRESS control request, using buffer filled in step 1a.
; Use the return value from usb_control_async as our return value;
; if it is zero, then something has failed.
        lea     eax, [esi+usb_controller.SetAddressBuffer]
        stdcall usb_control_async, ebx, eax, edi, edi, usb_set_address_callback, edi, edi
.nothing:
; 6. Return.
        pop     edi ebx         ; restore used registers to be stdcall
        ret
; Handlers of failures in steps 1b, 1d, 1f.
.freememory:
        call    free
        jmp     .freeaddr
.nomemory:
        dbgstr 'No memory for device data'
.freeaddr:
        mov     ecx, dword [esi+usb_controller.SetAddressBuffer+2]
        bts     [esi+usb_controller.ExistingAddresses], ecx
        xor     eax, eax
        jmp     .nothing
endp

; Helper procedure for usb_new_device.
; Allocates a new USB address and fills usb_controller.SetAddressBuffer
; with data for SET_ADDRESS(allocated_address) request.
; out: eax = 0 <=> failed
; Destroys edi.
proc usb_set_address_request
; There are 128 bits, one for each possible address.
; Note: only the USB thread works with usb_controller.ExistingAddresses,
; so there is no need for synchronization.
; We must find a bit set to 1 and clear it.
; 1. Find the first dword which has a nonzero bit = which is nonzero.
        mov     ecx, 128/32
        lea     edi, [esi+usb_controller.ExistingAddresses]
        xor     eax, eax
        repz scasd
; 2. If all dwords are zero, return an error.
        jz      .error
; 3. The dword at [edi-4] is nonzero. Find the lowest nonzero bit.
        bsf     eax, [edi-4]
; Now eax = bit number inside the dword at [edi-4].
; 4. Clear the bit.
        btr     [edi-4], eax
; 5. Generate the address by edi = memory address and eax = bit inside dword.
; Address = eax + 8 * (edi-4 - (esi+usb_controller.ExistingAddress)).
        sub     edi, esi
        lea     edi, [eax+(edi-4-usb_controller.ExistingAddresses)*8]
; 6. Store the allocated address in SetAddressBuffer and fill remaining fields.
; Note that usb_controller is zeroed at allocation, so only command byte needs
; to be filled.
        mov     byte [esi+usb_controller.SetAddressBuffer+1], USB_SET_ADDRESS
        mov     dword [esi+usb_controller.SetAddressBuffer+2], edi
; 7. Return non-zero value in eax.
        inc     eax
.nothing:
        ret
.error:
        dbgstr 'cannot allocate USB address'
        xor     eax, eax
        jmp     .nothing
endp

; This procedure is called by USB stack when SET_ADDRESS request initiated by
; usb_new_device is completed, either successfully or unsuccessfully.
; Note that USB stack uses esi = pointer to usb_controller.
proc usb_set_address_callback stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
        push    ebx     ; save ebx to be stdcall
; Load data to registers for further references.
        mov     ebx, [pipe]
        mov     ecx, dword [esi+usb_controller.SetAddressBuffer+2]
        mov     eax, [esi+usb_controller.HardwareFunc]
; 1. Check whether the device has accepted new address. If so, proceed to 2.
; Otherwise, go to 3.
        cmp     [status], 0
        jnz     .error
; 2. Address accepted.
; 2a. The controller-specific structure for the control pipe still uses
; zero address. Call the controller-specific function to change it to
; the actual address.
; Note that the hardware could cache the controller-specific structure,
; so setting the address could take some time until the cache is evicted.
; Thus, the call is asynchronous; meet us in usb_after_set_address when it will
; be safe to continue.
        dbgstr 'address set in device'
        call    [eax+usb_hardware_func.SetDeviceAddress]
; 2b. If the port is in non-root hub, clear 'reset in progress' flag.
; In any case, proceed to 4.
        mov     eax, [esi+usb_controller.ResettingHub]
        test    eax, eax
        jz      .return
        and     [eax+usb_hub.Actions], not HUB_RESET_IN_PROGRESS
.return:
; 4. Address configuration done, we can proceed with other ports.
; Call the worker function for that.
        call    usb_test_pending_port
.nothing:
        pop     ebx     ; restore ebx to be stdcall
        ret
.error:
; 3. Device error: device not responding, disconnect etc.
        DEBUGF 1,'K : error %d in SET_ADDRESS, USB device disabled\n',[status]
; 3a. The address has not been accepted. Mark it as free.
        bts     dword [esi+usb_controller.ExistingAddresses], ecx
; 3b. Disable the port with bad device.
; For the root hub, call the controller-specific function and go to 6.
; For non-root hubs, let the hub code do its work and return (the request
; could take some time, the hub code is responsible for proceeding).
        cmp     [esi+usb_controller.ResettingHub], 0
        jz      .roothub
        mov     eax, [esi+usb_controller.ResettingHub]
        call    usb_hub_disable_resetting_port
        jmp     .nothing
.roothub:
        movzx   ecx, [esi+usb_controller.ResettingPort]
        call    [eax+usb_hardware_func.PortDisable]
        jmp     .return
endp

; This procedure is called from usb_subscription_done when the hardware cache
; is cleared after request from usb_set_address_callback.
; in: ebx -> usb_pipe
proc usb_after_set_address
        dbgstr 'address set for controller'
; Issue control transfer GET_DESCRIPTOR(DEVICE_DESCR) for first 8 bytes.
; Remember, we still do not know the actual packet size;
; 8-bytes-request is safe.
; usb_new_device has allocated 8 extra bytes besides sizeof.usb_device_data;
; use them for both input and output.
        mov     eax, [ebx+usb_pipe.DeviceData]
        add     eax, usb_device_data.DeviceDescriptor
        mov     dword [eax], \
                80h + \         ; device-to-host, standard, device-wide
                (USB_GET_DESCRIPTOR shl 8) + \  ; request
                (0 shl 16) + \  ; descriptor index: there is only one
                (USB_DEVICE_DESCR shl 24)       ; descriptor type
        mov     dword [eax+4], 8 shl 16         ; data length
        stdcall usb_control_async, ebx, eax, eax, 8, usb_get_descr8_callback, eax, 0
        ret
endp

; This procedure is called by USB stack when GET_DESCRIPTOR(DEVICE_DESCR)
; request initiated by usb_after_set_address is completed, either successfully
; or unsuccessfully.
; Note that USB stack uses esi = pointer to usb_controller.
proc usb_get_descr8_callback stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
;       mov     eax, [buffer]
;       DEBUGF 1,'K : descr8: l=%x; %x %x %x %x %x %x %x %x\n',[length],\
;               [eax]:2,[eax+1]:2,[eax+2]:2,[eax+3]:2,[eax+4]:2,[eax+5]:2,[eax+6]:2,[eax+7]:2
        push    edi ebx         ; save used registers to be stdcall
        mov     ebx, [pipe]
; 1. Check whether the operation was successful.
; If not, say something to the debug board and stop the initialization.
        cmp     [status], 0
        jnz     .error
; 2. Length of descriptor must be at least sizeof.usb_device_descr bytes.
; If not, say something to the debug board and stop the initialization.
        mov     eax, [ebx+usb_pipe.DeviceData]
        cmp     [eax+usb_device_data.DeviceDescriptor+usb_device_descr.bLength], sizeof.usb_device_descr
        jb      .error
; 3. Now first 8 bytes of device descriptor are known;
; set DeviceDescrSize accordingly.
        mov     [eax+usb_device_data.DeviceDescrSize], 8
; 4. The controller-specific structure for the control pipe still uses
; the fake "maximum packet size". Call the controller-specific function to
; change it to the actual packet size from the device.
; Note that the hardware could cache the controller-specific structure,
; so changing it could take some time until the cache is evicted.
; Thus, the call is asynchronous; meet us in usb_after_set_endpoint_size
; when it will be safe to continue.
        movzx   ecx, [eax+usb_device_data.DeviceDescriptor+usb_device_descr.bMaxPacketSize0]
        mov     eax, [esi+usb_controller.HardwareFunc]
        call    [eax+usb_hardware_func.SetEndpointPacketSize]
.nothing:
; 5. Return.
        pop     ebx edi         ; restore used registers to be stdcall
        ret
.error:
        dbgstr 'error with USB device descriptor'
        jmp     .nothing
endp

; This procedure is called from usb_subscription_done when the hardware cache
; is cleared after request from usb_get_descr8_callback.
; in: ebx -> usb_pipe
proc usb_after_set_endpoint_size
; 1. Reallocate memory for device data:
; add memory for now-known size of device descriptor and extra 8 bytes
; for further actions.
; 1a. Allocate new memory.
        mov     eax, [ebx+usb_pipe.DeviceData]
        movzx   eax, [eax+usb_device_data.DeviceDescriptor+usb_device_descr.bLength]
; save length for step 2
        push    eax
        add     eax, sizeof.usb_device_data + 8
; Note that malloc destroys ebx.
        push    ebx
        call    malloc
        pop     ebx
; 1b. If failed, say something to the debug board and stop the initialization.
        test    eax, eax
        jz      .nomemory
; 1c. Copy data from old memory to new memory and switch the pointer in usb_pipe.
        push    eax
        push    esi edi
        mov     esi, [ebx+usb_pipe.DeviceData]
        mov     [ebx+usb_pipe.DeviceData], eax
        mov     edi, eax
        mov     eax, esi
repeat sizeof.usb_device_data / 4
        movsd
end repeat
        pop     edi esi
        call    usb_reinit_pipe_list
; 1d. Free the old memory.
; Note that free destroys ebx.
        push    ebx
        call    free
        pop     ebx
        pop     eax
; 2. Issue control transfer GET_DESCRIPTOR(DEVICE) for full descriptor.
; restore length saved in step 1a
        pop     edx
        add     eax, sizeof.usb_device_data
        mov     dword [eax], \
                80h + \         ; device-to-host, standard, device-wide
                (USB_GET_DESCRIPTOR shl 8) + \  ; request
                (0 shl 16) + \  ; descriptor index: there is only one
                (USB_DEVICE_DESCR shl 24)       ; descriptor type
        and     dword [eax+4], 0
        mov     [eax+6], dl     ; data length
        stdcall usb_control_async, ebx, eax, eax, edx, usb_get_descr_callback, eax, 0
; 3. Return.
        ret
.nomemory:
        dbgstr 'No memory for device data'
        ret
endp

; This procedure is called by USB stack when GET_DESCRIPTOR(DEVICE)
; request initiated by usb_after_set_endpoint_size is completed,
; either successfully or unsuccessfully.
proc usb_get_descr_callback stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; Note: the prolog is the same as in usb_get_descr8_callback.
        push    edi ebx         ; save used registers to be stdcall
; 1. Check whether the operation was successful.
; If not, say something to the debug board and stop the initialization.
        cmp     [status], 0
        jnz     usb_get_descr8_callback.error
; The full descriptor is known, dump it if specified by compile-time option.
if USB_DUMP_DESCRIPTORS
        mov     eax, [buffer]
        mov     ecx, [length]
        sub     ecx, 8
        jbe     .skipdebug
        DEBUGF 1,'K : device descriptor:'
@@:
        DEBUGF 1,' %x',[eax]:2
        inc     eax
        dec     ecx
        jnz     @b
        DEBUGF 1,'\n'
.skipdebug:
end if
; 2. Check that bLength is the same as was in the previous request.
; If not, say something to the debug board and stop the initialization.
; It is important, because usb_after_set_endpoint_size has allocated memory
; according to the old bLength. Note that [length] for control transfers
; includes 8 bytes of setup packet, so data length = [length] - 8.
        mov     eax, [buffer]
        movzx   ecx, [eax+usb_device_descr.bLength]
        add     ecx, 8
        cmp     [length], ecx
        jnz     usb_get_descr8_callback.error
; Amuse the user if she is watching the debug board.
        mov     cl, [eax+usb_device_descr.bNumConfigurations]
        DEBUGF 1,'K : found USB device with ID %x:%x, %d configuration(s)\n',\
                [eax+usb_device_descr.idVendor]:4,\
                [eax+usb_device_descr.idProduct]:4,\
                cl
; 3. If there are no configurations, stop the initialization.
        cmp     [eax+usb_device_descr.bNumConfigurations], 0
        jz      .nothing
; 4. Copy length of device descriptor to device data structure.
        movzx   edx, [eax+usb_device_descr.bLength]
        mov     [eax+usb_device_data.DeviceDescrSize-usb_device_data.DeviceDescriptor], dl
; 5. Issue control transfer GET_DESCRIPTOR(CONFIGURATION). We do not know
; the full length of that descriptor, so start with first 8 bytes, they contain
; the full length.
; usb_after_set_endpoint_size has allocated 8 extra bytes after the
; device descriptor, use them for both input and output.
        add     eax, edx
        mov     dword [eax], \
                80h + \         ; device-to-host, standard, device-wide
                (USB_GET_DESCRIPTOR shl 8) + \  ; request
                (0 shl 16) + \  ; descriptor index: there is only one
                (USB_CONFIG_DESCR shl 24)       ; descriptor type
        mov     dword [eax+4], 8 shl 16         ; data length
        stdcall usb_control_async, [pipe], eax, eax, 8, usb_know_length_callback, eax, 0
.nothing:
; 6. Return.
        pop     ebx edi         ; restore used registers to be stdcall
        ret
endp

; This procedure is called by USB stack when GET_DESCRIPTOR(CONFIGURATION)
; request initiated by usb_get_descr_callback is completed,
; either successfully or unsuccessfully.
proc usb_know_length_callback stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
        push    ebx             ; save used registers to be stdcall
; 1. Check whether the operation was successful.
; If not, say something to the debug board and stop the initialization.
        cmp     [status], 0
        jnz     .error
; 2. Get the total length of data associated with config descriptor and store
; it in device data structure. The total length must be at least
; sizeof.usb_config_descr bytes; if not, say something to the debug board and
; stop the initialization.
        mov     eax, [buffer]
        mov     edx, [pipe]
        movzx   ecx, [eax+usb_config_descr.wTotalLength]
        mov     eax, [edx+usb_pipe.DeviceData]
        cmp     ecx, sizeof.usb_config_descr
        jb      .error
        mov     [eax+usb_device_data.ConfigDataSize], ecx
; 3. Reallocate memory for device data:
; include usb_device_data structure, device descriptor,
; config descriptor with all associated data, and extra bytes
; sufficient for 8 bytes control packet and for one usb_interface_data struc.
; Align extra bytes to dword boundary.
if sizeof.usb_interface_data > 8
.extra_size = sizeof.usb_interface_data
else
.extra_size = 8
end if
; 3a. Allocate new memory.
        movzx   edx, [eax+usb_device_data.DeviceDescrSize]
        lea     eax, [ecx+edx+sizeof.usb_device_data+.extra_size+3]
        and     eax, not 3
        push    eax
        call    malloc
        pop     edx
; 3b. If failed, say something to the debug board and stop the initialization.
        test    eax, eax
        jz      .nomemory
; 3c. Copy data from old memory to new memory and switch the pointer in usb_pipe.
        push    eax
        mov     ebx, [pipe]
        push    esi edi
        mov     esi, [ebx+usb_pipe.DeviceData]
        mov     edi, eax
        mov     [ebx+usb_pipe.DeviceData], eax
        mov     eax, esi
        movzx   ecx, [esi+usb_device_data.DeviceDescrSize]
        sub     edx, .extra_size
        mov     [esi+usb_device_data.Interfaces], edx
        add     ecx, sizeof.usb_device_data + 8
        mov     edx, ecx
        shr     ecx, 2
        and     edx, 3
        rep movsd
        mov     ecx, edx
        rep movsb
        pop     edi esi
        call    usb_reinit_pipe_list
; 3d. Free old memory.
        call    free
        pop     eax
; 4. Issue control transfer GET_DESCRIPTOR(DEVICE) for full descriptor.
        movzx   ecx, [eax+usb_device_data.DeviceDescrSize]
        mov     edx, [eax+usb_device_data.ConfigDataSize]
        lea     eax, [eax+ecx+sizeof.usb_device_data]
        mov     dword [eax], \
                80h + \         ; device-to-host, standard, device-wide
                (USB_GET_DESCRIPTOR shl 8) + \  ; request
                (0 shl 16) + \  ; descriptor index: there is only one
                (USB_CONFIG_DESCR shl 24)       ; descriptor type
        and     dword [eax+4], 0
        mov     word [eax+6], dx        ; data length
        stdcall usb_control_async, [pipe], eax, eax, edx, usb_set_config_callback, eax, 0
.nothing:
; 5. Return.
        pop     ebx             ; restore used registers to be stdcall
        ret
.error:
        dbgstr 'error with USB configuration descriptor'
        jmp     .nothing
.nomemory:
        dbgstr 'No memory for device data'
        jmp     .nothing
endp

; This procedure is called by USB stack when GET_DESCRIPTOR(CONFIGURATION)
; request initiated by usb_know_length_callback is completed,
; either successfully or unsuccessfully.
proc usb_set_config_callback stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; Note that the prolog is the same as in usb_know_length_callback.
        push    ebx             ; save used registers to be stdcall
; 1. Check whether the operation was successful.
; If not, say something to the debug board and stop the initialization.
        xor     ecx, ecx
        mov     ebx, [pipe]
        cmp     [status], ecx
        jnz     usb_know_length_callback.error
; The full descriptor is known, dump it if specified by compile-time option.
if USB_DUMP_DESCRIPTORS
        mov     eax, [buffer]
        mov     ecx, [length]
        sub     ecx, 8
        jbe     .skip_debug
        DEBUGF 1,'K : config descriptor:'
@@:
        DEBUGF 1,' %x',[eax]:2
        inc     eax
        dec     ecx
        jnz     @b
        DEBUGF 1,'\n'
.skip_debug:
        xor     ecx, ecx
end if
; 2. Issue control transfer SET_CONFIGURATION to activate this configuration.
; Usually this is the only configuration.
; Use extra bytes allocated by usb_know_length_callback;
; offset from device data start is stored in Interfaces.
        mov     eax, [ebx+usb_pipe.DeviceData]
        mov     edx, [buffer]
        add     eax, [eax+usb_device_data.Interfaces]
        mov     dl, [edx+usb_config_descr.bConfigurationValue]
        mov     dword [eax], USB_SET_CONFIGURATION shl 8
        mov     dword [eax+4], ecx
        mov     byte [eax+2], dl
        stdcall usb_control_async, [pipe], eax, ecx, ecx, usb_got_config_callback, [buffer], ecx
        pop     ebx             ; restore used registers to be stdcall
        ret
endp

; This procedure is called by USB stack when SET_CONFIGURATION
; request initiated by usb_set_config_callback is completed,
; either successfully or unsuccessfully.
; If successfully, the device is configured and ready to work,
; pass the device to the corresponding driver(s).
proc usb_got_config_callback stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
locals
InterfacesData  dd      ?
NumInterfaces   dd      ?
driver          dd      ?
endl
; 1. If there was an error, say something to the debug board and stop the
; initialization.
        cmp     [status], 0
        jz      @f
        dbgstr 'USB error in SET_CONFIGURATION'
        ret
@@:
        push    ebx edi         ; save used registers to be stdcall
; 2. Sanity checks: the total length must be the same as before (because we
; have allocated memory assuming the old value), length of config descriptor
; must be at least sizeof.usb_config_descr (we use fields from it),
; there must be at least one interface.
        mov     ebx, [pipe]
        mov     ebx, [ebx+usb_pipe.DeviceData]
        mov     eax, [calldata]
        mov     edx, [ebx+usb_device_data.ConfigDataSize]
        cmp     [eax+usb_config_descr.wTotalLength], dx
        jnz     .invalid
        cmp     [eax+usb_config_descr.bLength], 9
        jb      .invalid
        movzx   edx, [eax+usb_config_descr.bNumInterfaces]
        test    edx, edx
        jnz     @f
.invalid:
        dbgstr 'error: invalid configuration descriptor'
        jmp     .nothing
@@:
; 3. Store the number of interfaces in device data structure.
        mov     [ebx+usb_device_data.NumInterfaces], dl
; 4. If there is only one interface (which happens quite often),
; the memory allocated in usb_know_length_callback is sufficient.
; Otherwise (which also happens quite often), reallocate device data.
; 4a. Check whether there is only one interface. If so, skip this step.
        cmp     edx, 1
        jz      .has_memory
; 4b. Allocate new memory.
        mov     eax, [ebx+usb_device_data.Interfaces]
        lea     eax, [eax+edx*sizeof.usb_interface_data]
        call    malloc
; 4c. If failed, say something to the debug board and
; stop the initialization.
        test    eax, eax
        jnz     @f
        dbgstr 'No memory for device data'
        jmp     .nothing
@@:
; 4d. Copy data from old memory to new memory and switch the pointer in usb_pipe.
        push    eax
        push    esi
        mov     ebx, [pipe]
        mov     edi, eax
        mov     esi, [ebx+usb_pipe.DeviceData]
        mov     [ebx+usb_pipe.DeviceData], eax
        mov     eax, esi
        mov     ecx, [esi+usb_device_data.Interfaces]
        shr     ecx, 2
        rep movsd
        pop     esi
        call    usb_reinit_pipe_list
; 4e. Free old memory.
        call    free
        pop     ebx
.has_memory:
; 5. Initialize interfaces table: zero all contents.
        mov     edi, [ebx+usb_device_data.Interfaces]
        add     edi, ebx
        mov     [InterfacesData], edi
        movzx   ecx, [ebx+usb_device_data.NumInterfaces]
if sizeof.usb_interface_data <> 8
You have changed sizeof.usb_interface_data? Modify this place too.
end if
        add     ecx, ecx
        xor     eax, eax
        rep stosd
; No interfaces are found yet.
        mov     [NumInterfaces], eax
; 6. Get the pointer to config descriptor data.
; Note: if there was reallocation, [buffer] is not valid anymore,
; so calculate value based on usb_device_data.
        movzx   eax, [ebx+usb_device_data.DeviceDescrSize]
        lea     eax, [eax+ebx+sizeof.usb_device_data]
        mov     [calldata], eax
        mov     ecx, [ebx+usb_device_data.ConfigDataSize]
; 7. Loop over all descriptors,
; scan for interface descriptors with bAlternateSetting = 0,
; load the corresponding driver, call its AddDevice function.
.descriptor_loop:
; While in loop: eax points to the current descriptor,
; ecx = number of bytes left, the iteration starts only if ecx is nonzero,
; edx = size of the current descriptor.
; 7a. The first byte is always accessible; it contains the length of
; the current descriptor. Validate that the length is at least 2 bytes,
; and the entire descriptor is readable (the length is at most number of
; bytes left).
        movzx   edx, [eax+usb_descr.bLength]
        cmp     edx, sizeof.usb_descr
        jb      .invalid
        cmp     ecx, edx
        jb      .invalid
; 7b. Check descriptor type. Ignore all non-INTERFACE descriptor.
        cmp     byte [eax+usb_descr.bDescriptorType], USB_INTERFACE_DESCR
        jz      .interface
.next_descriptor:
; 7c. Advance pointer, decrease length left, if there is still something left,
; continue the loop.
        add     eax, edx
        sub     ecx, edx
        jnz     .descriptor_loop
.done:
.nothing:
        pop     edi ebx         ; restore used registers to be stdcall
        ret
.interface:
; 7d. Validate the descriptor length.
        cmp     edx, sizeof.usb_interface_descr
        jb      .next_descriptor
; 7e. If bAlternateSetting is nonzero, this descriptor actually describes
; another mode of already known interface and belongs to the already loaded
; driver; amuse the user and continue to 7c.
        cmp     byte [eax+usb_interface_descr.bAlternateSetting], 0
        jz      @f
        DEBUGF 1,'K : note: alternate setting with %x/%x/%x\n',\
                [eax+usb_interface_descr.bInterfaceClass]:2,\
                [eax+usb_interface_descr.bInterfaceSubClass]:2,\
                [eax+usb_interface_descr.bInterfaceProtocol]:2
        jmp     .next_descriptor
@@:
; 7f. Check that the new interface does not overflow allocated table.
        mov     edx, [NumInterfaces]
        inc     dl
        jz      .invalid
        cmp     dl, [ebx+usb_device_data.NumInterfaces]
        ja      .invalid
; 7g. We have found a new interface. Advance bookkeeping vars.
        mov     [NumInterfaces], edx
        add     [InterfacesData], sizeof.usb_interface_data
; 7h. Save length left and pointer to the current interface descriptor.
        push    ecx eax
; Amuse the user if she is watching the debug board.
        DEBUGF 1,'K : USB interface class/subclass/protocol = %x/%x/%x\n',\
                [eax+usb_interface_descr.bInterfaceClass]:2,\
                [eax+usb_interface_descr.bInterfaceSubClass]:2,\
                [eax+usb_interface_descr.bInterfaceProtocol]:2
; 7i. Select the correct driver based on interface class.
; For hubs, go to 7j. Otherwise, go to 7k.
; Note: this should be rewritten as table-based lookup when more drivers will
; be available.
        cmp     byte [eax+usb_interface_descr.bInterfaceClass], 9
        jz      .found_hub
        mov     edx, usb_hid_name
        cmp     byte [eax+usb_interface_descr.bInterfaceClass], 3
        jz      .load_driver
        mov     edx, usb_print_name
        cmp     byte [eax+usb_interface_descr.bInterfaceClass], 7
        jz      .load_driver
        mov     edx, usb_stor_name
        cmp     byte [eax+usb_interface_descr.bInterfaceClass], 8
        jz      .load_driver
        mov     edx, usb_other_name
        jmp     .load_driver
.found_hub:
; 7j. Hubs are a part of USB stack, thus, integrated into the kernel.
; Use the pointer to hub callbacks and go to 7m.
        mov     eax, usb_hub_pseudosrv - USBSRV.usb_func
        jmp     .driver_loaded
.load_driver:
; 7k. Load the corresponding driver.
        push    ebx esi edi
        stdcall get_service, edx
        pop     edi esi ebx
; 7l. If failed, say something to the debug board and go to 7p.
        test    eax, eax
        jnz     .driver_loaded
        dbgstr 'failed to load class driver'
        jmp     .next_descriptor2
.driver_loaded:
; 7m. Call AddDevice function of the driver.
; Note that top of stack contains a pointer to the current interface,
; saved by step 7h.
        mov     [driver], eax
        mov     eax, [eax+USBSRV.usb_func]
        pop     edx
        push    edx
; Note: usb_hub_init assumes that edx points to usb_interface_descr,
; ecx = length rest; if you change the code, modify usb_hub_init also.
        stdcall [eax+USBFUNC.add_device], [pipe], [calldata], edx
; 7n. If failed, say something to the debug board and go to 7p.
        test    eax, eax
        jnz     .store_data
        dbgstr 'USB device initialization failed'
        jmp     .next_descriptor2
.store_data:
; 7o. Store the returned value and the driver handle to InterfacesData.
; Note that step 7g has already advanced InterfacesData.
        mov     edx, [InterfacesData]
        mov     [edx+usb_interface_data.DriverData-sizeof.usb_interface_data], eax
        mov     eax, [driver]
        mov     [edx+usb_interface_data.DriverFunc-sizeof.usb_interface_data], eax
.next_descriptor2:
; 7p. Restore registers saved in step 7h, get the descriptor length and
; continue to 7c.
        pop     eax ecx
        movzx   edx, byte [eax+usb_descr.bLength]
        jmp     .next_descriptor
endp

; Driver names, see step 7i of usb_got_config_callback.
iglobal
usb_hid_name    db      'usbhid',0
usb_stor_name   db      'usbstor',0
usb_print_name  db      'usbprint',0
usb_other_name  db      'usbother',0
endg
