; Support for USB (non-root) hubs:
; powering up/resetting/disabling ports,
; watching for adding/removing devices.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; Hub constants
; USB hub descriptor type
USB_HUB_DESCRIPTOR = 29h

; Features for CLEAR_FEATURE commands to the hub.
C_HUB_LOCAL_POWER   = 0
C_HUB_OVER_CURRENT  = 1

; Bits in result of GET_STATUS command for a port.
; Also suitable for CLEAR_FEATURE/SET_FEATURE commands, where applicable,
; except TEST/INDICATOR.
PORT_CONNECTION     = 0
PORT_ENABLE         = 1
PORT_SUSPEND        = 2
PORT_OVER_CURRENT   = 3
PORT_RESET          = 4
PORT_POWER          = 8
PORT_LOW_SPEED      = 9
PORT_HIGH_SPEED     = 10
PORT_TEST_BIT       = 11
PORT_INDICATOR_BIT  = 12
C_PORT_CONNECTION   = 16
C_PORT_ENABLE       = 17
C_PORT_SUSPEND      = 18
C_PORT_OVER_CURRENT = 19
C_PORT_RESET        = 20
PORT_TEST_FEATURE   = 21
PORT_INDICATOR_FEATURE = 22

; Internal constants
; Bits in usb_hub.Actions
HUB_WAIT_POWERED      = 1
; ports were powered, wait until power is stable
HUB_WAIT_CONNECT      = 2
; some device was connected, wait initial debounce interval
HUB_RESET_IN_PROGRESS = 4
; reset in progress, so buffer for config requests is owned
; by reset process; this includes all stages from initial disconnect test
; to end of setting address (fail on any stage should lead to disabling port,
; which requires a config request)
HUB_RESET_WAITING     = 8
; the port is ready for reset, but another device somewhere on the bus
; is resetting. Implies HUB_RESET_IN_PROGRESS
HUB_RESET_SIGNAL      = 10h
; reset signalling is active for some port in the hub
; Implies HUB_RESET_IN_PROGRESS
HUB_RESET_RECOVERY    = 20h
; reset recovery is active for some port in the hub
; Implies HUB_RESET_IN_PROGRESS

; Well, I think that those 5 flags WAIT_CONNECT and RESET_* require additional
; comments. So that is the overview of what happens with a new device assuming
; no errors.
; * device is connected;
; * hub notifies us about connect event; after some processing
;   usb_hub_port_change finally processes that event, setting the flag
;   HUB_WAIT_CONNECT and storing time when the device was connected;
; * 100 ms delay;
; * usb_hub_process_deferred clears HUB_WAIT_CONNECT,
;   sets HUB_RESET_IN_PROGRESS, stores the port index in ConfigBuffer and asks
;   the hub whether there was a disconnect event for that port during those
;   100 ms (on the hardware level notifications are obtained using polling
;   with some intervals, so it is possible that the corresponding notification
;   has not arrived yet);
; * usb_hub_connect_port_status checks that there was no disconnect event
;   and sets HUB_RESET_WAITING flag (HUB_RESET_IN_PROGRESS is still set,
;   ConfigBuffer still contains the port index);
; * usb_hub_process_deferred checks whether there is another device currently
;   resetting. If so, it waits until reset is done
;   (with HUB_RESET_WAITING and HUB_RESET_IN_PROGRESS bits set);
; * usb_hub_process_deferred clears HUB_RESET_WAITING, sets HUB_RESET_SIGNAL
;   and initiates reset signalling on the port;
; * usb_hub_process_deferred checks the status every tick;
;   when reset signalling is stopped by the hub, usb_hub_resetting_port_status
;   callback clears HUB_RESET_SIGNAL and sets HUB_RESET_RECOVERY;
; * 10 ms (at least) delay;
; * usb_hub_process_deferred clears HUB_RESET_RECOVERY and notifies other code
;   that the new device is ready to be configured;
; * when it is possible to reset another device, the protocol layer
;   clears HUB_RESET_IN_PROGRESS bit.

; =============================================================================
; ================================ Structures =================================
; =============================================================================
; This structure contains all used data for one hub.
struct usb_hub
; All configured hubs are organized in the global usb_hub_list.
; Two following fields give next/prev items in that list.
; While the hub is unconfigured, they point to usb_hub itself.
Next                    dd      ?
Prev                    dd      ?
Controller              dd      ?
; Pointer to usb_controller for the bus.
;
; Handles of two pipes: configuration control pipe for zero endpoint opened by
; the common code and status interrupt pipe opened by us.
ConfigPipe              dd      ?
StatusPipe              dd      ?
NumPorts                dd      ?
; Number of downstream ports; from 1 to 255.
Actions                 dd      ?
; Bitfield with HUB_* constants.
PoweredOnTime           dd      ?
; Time (in ticks) when all downstream ports were powered up.
ResetTime               dd      ?
; Time (in ticks) when the current port was reset;
; when a port is resetting, contains the last tick of status check;
; when reset recovery for a port is active, contains the time when
; reset was completed.
;
; There are two possible reasons for configuration requests:
; synchronous, when certain time is passed after something,
; and asynchronous, when the hub is notifying about some change and
; config request needs to be issued in order to query details.
; Use two different buffers to avoid unnecessary dependencies.
ConfigBuffer            rb      8
; Buffer for configuration requests for synchronous events.
ChangeConfigBuffer      rb      8
; Buffer for configuration requests for status changes.
AccStatusChange         db      ?
; Accumulated status change. See 11.12.3 of USB2 spec or comments in code.
HubCharacteristics      dw      ?
; Copy of usb_hub_descr.wHubCharacteristics.
PowerOnInterval         db      ?
; Copy of usb_hub_descr.bPwrOn2PwrGood.
;
; Two following fields are written at once by GET_STATUS request
; and must remain in this order.
StatusData              dw      ?
; Bitfield with 1 shl PORT_* indicating status of the current port.
StatusChange            dw      ?
; Bitfield with 1 shl PORT_* indicating change in status of the current port.
; Two following fields are written at once by GET_STATUS request
; and must remain in this order.
; The meaning is the same as of StatusData/StatusChange; two following fields
; are used by the synchronous requests to avoid unnecessary interactions with
; the asynchronous handler.
ResetStatusData         dw      ?
ResetStatusChange       dw      ?
StatusChangePtr         dd      ?
; Pointer to StatusChangeBuf.
ConnectedDevicesPtr     dd      ?
; Pointer to ConnectedDevices.
ConnectedTimePtr        dd      ?
; Pointer to ConnectedTime.
;
; Variable-length parts:
; DeviceRemovable rb (NumPorts+8)/8
;  Bit i+1 = device at port i (zero-based) is non-removable.
; StatusChangeBuf rb (NumPorts+8)/8
;  Buffer for status interrupt pipe. Bit 0 = hub status change,
;  other bits = status change of the corresponding ports.
; ConnectedDevices rd NumPorts
;  Pointers to config pipes for connected devices or zero if no device connected.
; ConnectedTime rd NumPorts
;  For initial debounce interval:
;   time (in ticks) when a device was connected at that port.
;  Normally: -1
ends

; Hub descriptor.
struct usb_hub_descr usb_descr
bNbrPorts               db      ?
; Number of downstream ports.
wHubCharacteristics     dw      ?
; Bit 0: 0 = all ports are powered at once, 1 = individual port power switching
; Bit 1: reserved, must be zero
; Bit 2: 1 = the hub is part of a compound device
; Bits 3-4: 00 = global overcurrent protection,
;           01 = individual port overcurrent protection,
;           1x = no overcurrent protection
; Bits 5-6: Transaction Translator Think Time, 8*(value+1) full-speed bit times
; Bit 7: 1 = port indicators supported
; Other bits are reserved.
bPwrOn2PwrGood          db      ?
; Time in 2ms intervals between powering up a port and a port becoming ready.
bHubContrCurrent        db      ?
; Maximum current requirements of the Hub Controller electronics in mA.
; DeviceRemovable - variable length
;  Bit 0 is reserved, bit i+1 = device at port i is non-removable.
; PortPwrCtrlMask - variable length
;  Obsolete, exists for compatibility. We ignore it.
ends

iglobal
align 4
; Implementation of struct USBFUNC for hubs.
usb_hub_callbacks:
        dd      usb_hub_callbacks_end - usb_hub_callbacks
        dd      usb_hub_init
        dd      usb_hub_disconnect
usb_hub_callbacks_end:
usb_hub_pseudosrv       dd      usb_hub_callbacks
endg

; This procedure is called when new hub is detected.
; It initializes the device.
; Technically, initialization implies sending several USB queries,
; so it is split in several procedures. The first is usb_hub_init,
; other are callbacks which will be called at some time in the future,
; when the device will respond.
; edx = usb_interface_descr, ecx = length rest
proc usb_hub_init
        push    ebx esi         ; save used registers to be stdcall
virtual at esp
                rd      2       ; saved registers
                dd      ?       ; return address
.pipe           dd      ?       ; handle of the config pipe
.config         dd      ?       ; pointer to usb_config_descr
.interface      dd      ?       ; pointer to usb_interface_descr
end virtual
; Hubs use one IN interrupt endpoint for polling the device
; 1. Locate the descriptor of the interrupt endpoint.
; Loop over all descriptors owned by this interface.
.lookep:
; 1a. Skip the current descriptor.
        movzx   eax, [edx+usb_descr.bLength]
        add     edx, eax
        sub     ecx, eax
        jb      .errorep
; 1b. Length of data left must be at least sizeof.usb_endpoint_descr.
        cmp     ecx, sizeof.usb_endpoint_descr
        jb      .errorep
; 1c. If we have found another interface descriptor but not found our endpoint,
; this is an error: all subsequent descriptors belong to that interface
; (or further interfaces).
        cmp     [edx+usb_endpoint_descr.bDescriptorType], USB_INTERFACE_DESCR
        jz      .errorep
; 1d. Ignore all interface-related descriptors except endpoint descriptor.
        cmp     [edx+usb_endpoint_descr.bDescriptorType], USB_ENDPOINT_DESCR
        jnz     .lookep
; 1e. Length of endpoint descriptor must be at least sizeof.usb_endpoint_descr.
        cmp     [edx+usb_endpoint_descr.bLength], sizeof.usb_endpoint_descr
        jb      .errorep
; 1f. Ignore all endpoints except for INTERRUPT IN.
        cmp     [edx+usb_endpoint_descr.bEndpointAddress], 0
        jge     .lookep
        mov     al, [edx+usb_endpoint_descr.bmAttributes]
        and     al, 3
        cmp     al, INTERRUPT_PIPE
        jnz     .lookep
; We have located the descriptor for INTERRUPT IN endpoint,
; the pointer is in edx.
; 2. Allocate memory for the hub descriptor.
; Maximum length (assuming 255 downstream ports) is 40 bytes.
; 2a. Save registers.
        push    edx
; 2b. Call the allocator.
        push    40
        pop     eax
        call    malloc
; 2c. Restore registers.
        pop     ecx
; 2d. If failed, say something to the debug board and return error.
        test    eax, eax
        jz      .nomemory
; 2e. Store the pointer in esi. xchg eax,r32 is one byte shorter than mov.
        xchg    esi, eax
; 3. Open a pipe for the status endpoint with descriptor found in step 1.
        mov     ebx, [.pipe]
        movzx   eax, [ecx+usb_endpoint_descr.bEndpointAddress]
        movzx   edx, [ecx+usb_endpoint_descr.bInterval]
        movzx   ecx, [ecx+usb_endpoint_descr.wMaxPacketSize]
        stdcall usb_open_pipe, ebx, eax, ecx, INTERRUPT_PIPE, edx
; If failed, free the memory allocated in step 2,
; say something to the debug board and return error.
        test    eax, eax
        jz      .free
; 4. Send control query for the hub descriptor,
; pass status pipe as a callback parameter,
; allow short packets.
        mov     dword [esi], 0xA0 + \   ; class-specific request
                (USB_GET_DESCRIPTOR shl 8) + \
                (0 shl 16) + \          ; descriptor index 0
                (USB_HUB_DESCRIPTOR shl 24)
        mov     dword [esi+4], 40 shl 16
        stdcall usb_control_async, ebx, esi, esi, 40, usb_hub_got_config, eax, 1
; 5. If failed, free the memory allocated in step 2,
; say something to the debug board and return error.
        test    eax, eax
        jz      .free
; Otherwise, return 1. usb_hub_got_config will overwrite it later.
        xor     eax, eax
        inc     eax
        jmp     .nothing
.free:
        xchg    eax, esi
        call    free
        jmp     .return0
.errorep:
        dbgstr 'Invalid config descriptor for a hub'
        jmp     .return0
.nomemory:
        dbgstr 'No memory for USB hub data'
.return0:
        xor     eax, eax
.nothing:
        pop     esi ebx         ; restore used registers to be stdcall
        retn    12
endp

; This procedure is called when the request for the hub descriptor initiated
; by usb_hub_init is finished, either successfully or unsuccessfully.
proc usb_hub_got_config stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
        push    ebx             ; save used registers to be stdcall
; 1. If failed, say something to the debug board, free the buffer
; and stop the initialization.
        cmp     [status], 0
        jnz     .invalid
; 2. The length must be at least sizeof.usb_hub_descr.
; Note that [length] includes 8 bytes of setup packet.
        cmp     [length], 8 + sizeof.usb_hub_descr
        jb      .invalid
; 3. Sanity checks for the hub descriptor.
        mov     eax, [buffer]
if USB_DUMP_DESCRIPTORS
        mov     ecx, [length]
        sub     ecx, 8
        DEBUGF 1,'K : hub config:'
        push    eax
@@:
        DEBUGF 1,' %x',[eax]:2
        inc     eax
        dec     ecx
        jnz     @b
        DEBUGF 1,'\n'
        pop     eax
end if
        cmp     [eax+usb_hub_descr.bLength], sizeof.usb_hub_descr
        jb      .invalid
        cmp     [eax+usb_hub_descr.bDescriptorType], USB_HUB_DESCRIPTOR
        jnz     .invalid
        movzx   ecx, [eax+usb_hub_descr.bNbrPorts]
        test    ecx, ecx
        jz      .invalid
; 4. We use sizeof.usb_hub_descr bytes plus DeviceRemovable info;
; size of DeviceRemovable is (NumPorts+1) bits, this gives
; floor(NumPorts/8)+1 bytes. Check that all data are present in the
; descriptor and were successfully read.
        mov     edx, ecx
        shr     edx, 3
        add     edx, sizeof.usb_hub_descr + 1
        cmp     [eax+usb_hub_descr.bLength], dl
        jb      .invalid
        sub     [length], 8
        cmp     [length], edx
        jb      .invalid
; 5. Allocate the memory for usb_hub structure.
; Total size of variable-length data is ALIGN_UP(2*(floor(NumPorts/8)+1),4)+8*NumPorts.
        lea     edx, [sizeof.usb_hub+(edx-sizeof.usb_hub_descr)*2+3]
        and     edx, not 3
        lea     eax, [edx+ecx*8]
        push    ecx edx
        call    malloc
        pop     edx ecx
        test    eax, eax
        jz      .nomemory
        xchg    eax, ebx
; 6. Fill usb_hub structure.
        mov     [ebx+usb_hub.NumPorts], ecx
        add     edx, ebx
        mov     [ebx+usb_hub.ConnectedDevicesPtr], edx
        mov     eax, [pipe]
        mov     [ebx+usb_hub.ConfigPipe], eax
        mov     edx, [eax+usb_pipe.Controller]
        mov     [ebx+usb_hub.Controller], edx
        mov     eax, [calldata]
        mov     [ebx+usb_hub.StatusPipe], eax
        push    esi edi
        mov     esi, [buffer]
; The following commands load bNbrPorts, wHubCharacteristics, bPwrOn2PwrGood.
        mov     edx, dword [esi+usb_hub_descr.bNbrPorts]
        mov     dl, 0
; The following command zeroes AccStatusChange and stores
; HubCharacteristics and PowerOnInterval.
        mov     dword [ebx+usb_hub.AccStatusChange], edx
        xor     eax, eax
        mov     [ebx+usb_hub.Actions], eax
; Copy DeviceRemovable data.
        lea     edi, [ebx+sizeof.usb_hub]
        add     esi, sizeof.usb_hub_descr
        mov     edx, ecx
        shr     ecx, 3
        inc     ecx
        rep movsb
        mov     [ebx+usb_hub.StatusChangePtr], edi
; Zero ConnectedDevices.
        mov     edi, [ebx+usb_hub.ConnectedDevicesPtr]
        mov     ecx, edx
        rep stosd
        mov     [ebx+usb_hub.ConnectedTimePtr], edi
; Set ConnectedTime to -1.
        dec     eax
        mov     ecx, edx
        rep stosd
        pop     edi esi
; 7. Replace value of 1 returned from usb_hub_init to the real value.
; Note: hubs are part of the core USB code, so this code can work with
; internals of other parts. Another way, the only possible one for external
; drivers, is to use two memory allocations: one (returned from AddDevice and
; fixed after that) for pointer, another for real data. That would work also,
; but wastes one allocation.
        mov     eax, [pipe]
        mov     eax, [eax+usb_pipe.DeviceData]
        add     eax, [eax+usb_device_data.Interfaces]
.scan:
        cmp     [eax+usb_interface_data.DriverData], 1
        jnz     @f
        cmp     [eax+usb_interface_data.DriverFunc], usb_hub_pseudosrv - USBSRV.usb_func
        jz      .scan_found
@@:
        add     eax, sizeof.usb_interface_data
        jmp     .scan
.scan_found:
        mov     [eax+usb_interface_data.DriverData], ebx
; 8. Insert the hub structure to the tail of the overall list of all hubs.
        mov     ecx, usb_hubs_list
        mov     edx, [ecx+usb_hub.Prev]
        mov     [ecx+usb_hub.Prev], ebx
        mov     [edx+usb_hub.Next], ebx
        mov     [ebx+usb_hub.Prev], edx
        mov     [ebx+usb_hub.Next], ecx
; 9. Start powering up all ports.
        DEBUGF 1,'K : found hub with %d ports\n',[ebx+usb_hub.NumPorts]
        lea     eax, [ebx+usb_hub.ConfigBuffer]
        xor     ecx, ecx
        mov     dword [eax], 23h + \    ; class-specific request to hub port
                (USB_SET_FEATURE shl 8) + \
                (PORT_POWER shl 16)
        mov     edx, [ebx+usb_hub.NumPorts]
        mov     dword [eax+4], edx
        stdcall usb_control_async, [ebx+usb_hub.ConfigPipe], eax, ecx, ecx, usb_hub_port_powered, ebx, ecx
.freebuf:
; 10. Free the buffer for hub descriptor and return.
        mov     eax, [buffer]
        call    free
        pop     ebx             ; restore used registers to be stdcall
        ret
.nomemory:
        dbgstr 'No memory for USB hub data'
        jmp     .freebuf
.invalid:
        dbgstr 'Invalid hub descriptor'
        jmp     .freebuf
endp

; This procedure is called when the request to power up some port is completed,
; either successfully or unsuccessfully.
proc usb_hub_port_powered stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; 1. Check whether the operation was successful.
; If not, say something to the debug board and ssstop the initialization.
        cmp     [status], 0
        jnz     .invalid
; 2. Check whether all ports were powered.
; If so, go to 4. Otherwise, proceed to 3.
        mov     eax, [calldata]
        dec     dword [eax+usb_hub.ConfigBuffer+4]
        jz      .done
; 3. Power up the next port and return.
        lea     edx, [eax+usb_hub.ConfigBuffer]
        xor     ecx, ecx
        stdcall usb_control_async, [eax+usb_hub.ConfigPipe], edx, ecx, ecx, usb_hub_port_powered, eax, ecx
.nothing:
        ret
.done:
; 4. All ports were powered.
; The hub requires some delay until power will be stable, the delay value
; is provided in the hub descriptor; we have copied that value to
; usb_hub.PowerOnInterval. Note the time and set the corresponding flag
; for usb_hub_process_deferred.
        mov     ecx, [timer_ticks]
        mov     [eax+usb_hub.PoweredOnTime], ecx
        or      [eax+usb_hub.Actions], HUB_WAIT_POWERED
        jmp     .nothing
.invalid:
        dbgstr 'Error while powering hub ports'
        jmp     .nothing
endp

; Requests notification about any changes in hub/ports configuration.
; Called when initial configuration is done and when a previous notification
; has been processed.
proc usb_hub_wait_change
        mov     ecx, [eax+usb_hub.NumPorts]
        shr     ecx, 3
        inc     ecx
        stdcall usb_normal_transfer_async, [eax+usb_hub.StatusPipe], \
                [eax+usb_hub.StatusChangePtr], ecx, usb_hub_changed, eax, 1
        ret
endp

; This procedure is called when something has changed on the hub.
proc usb_hub_changed stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
;       DEBUGF 1,'K : [%d] int pipe for hub %x\n',[timer_ticks],[calldata]
; 1. Check whether our request has failed.
; If so, say something to the debug board and stop processing notifications.
        xor     ecx, ecx
        cmp     [status], ecx
        jnz     .failed
; 2. If no data were retrieved, restart waiting.
        mov     eax, [calldata]
        cmp     [length], ecx
        jz      .continue
; 3. If size of data retrieved is less than maximal, pad with zeroes;
; this corresponds to 'state of other ports was not changed'
        mov     ecx, [eax+usb_hub.NumPorts]
        shr     ecx, 3
        inc     ecx
        sub     ecx, [length]
        push    eax edi
        mov     edi, [buffer]
        add     edi, [length]
        xor     eax, eax
        rep stosb
        pop     edi eax
.restart:
; State of some elements of the hub was changed.
; Find the first element that was changed,
; ask the hub about nature of the change,
; clear the corresponding change,
; reask the hub about status+change (it is possible that another change
; occurs between the first ask and clearing the change; we won't see that
; change, so we need to query the status after clearing the change),
; continue two previous steps until nothing changes,
; process all changes which were registered.
; When all changes for one element will be processed, return to here and look
; for other changed elements.
        mov     edx, [eax+usb_hub.StatusChangePtr]
; We keep all observed changes in the special var usb_hub.AccStatusChange;
; it will be logical OR of all observed StatusChange's.
; 4. No observed changes yet, zero usb_hub.AccStatusChange.
        xor     ecx, ecx
        mov     [eax+usb_hub.AccStatusChange], cl
; 5. Test whether there was a change in the hub itself.
; If so, query hub state.
        btr     dword [edx], ecx
        jnc     .no_hub_change
.next_hub_change:
;       DEBUGF 1,'K : [%d] querying status of hub %x\n',[timer_ticks],eax
        lea     edx, [eax+usb_hub.ChangeConfigBuffer]
        lea     ecx, [eax+usb_hub.StatusData]
        mov     dword [edx], 0A0h + \   ; class-specific request from hub itself
                (USB_GET_STATUS shl 8)
        mov     dword [edx+4], 4 shl 16 ; get 4 bytes
        stdcall usb_control_async, [eax+usb_hub.ConfigPipe], edx, ecx, 4, usb_hub_status, eax, 0
        jmp     .nothing
.no_hub_change:
; 6. Find the first port with changed state and clear the corresponding bit
; (so next scan after .restart will not consider this port again).
; If found, go to 8. Otherwise, advance to 7.
        inc     ecx
.test_port_change:
        btr     [edx], ecx
        jc      .found_port_change
        inc     ecx
        cmp     ecx, [eax+usb_hub.NumPorts]
        jbe     .test_port_change
.continue:
; 7. All changes have been processed. Wait for next notification.
        call    usb_hub_wait_change
.nothing:
        ret
.found_port_change:
        mov     dword [eax+usb_hub.ChangeConfigBuffer+4], ecx
.next_port_change:
; 8. Query port state. Continue work in usb_hub_port_status callback.
;       movzx   ecx, [eax+usb_hub.ChangeConfigBuffer+4]
;       dec     ecx
;       DEBUGF 1,'K : [%d] querying status of hub %x port %d\n',[timer_ticks],eax,ecx
        lea     edx, [eax+usb_hub.ChangeConfigBuffer]
        mov     dword [edx], 0A3h + \   ; class-specific request from hub port
                (USB_GET_STATUS shl 8)
        mov     byte [edx+6], 4         ; data length = 4 bytes
        lea     ecx, [eax+usb_hub.StatusData]
        stdcall usb_control_async, [eax+usb_hub.ConfigPipe], edx, ecx, 4, usb_hub_port_status, eax, 0
        jmp     .nothing
.failed:
        cmp     [status], USB_STATUS_CLOSED
        jz      .nothing
        dbgstr 'Querying hub notification failed'
        jmp     .nothing
endp

; This procedure is called when the request of hub status is completed,
; either successfully or unsuccessfully.
proc usb_hub_status stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; 1. Check whether our request has failed.
; If so, say something to the debug board and stop processing notifications.
        cmp     [status], 0
        jnz     .failed
; 2. Accumulate observed changes.
        mov     eax, [calldata]
        mov     dl, byte [eax+usb_hub.StatusChange]
        or      [eax+usb_hub.AccStatusChange], dl
.next_change:
; 3. Find the first change. If found, advance to 4. Otherwise, go to 5.
        mov     cl, C_HUB_OVER_CURRENT
        btr     dword [eax+usb_hub.StatusChange], 1
        jc      .clear_hub_change
        mov     cl, C_HUB_LOCAL_POWER
        btr     dword [eax+usb_hub.StatusChange], 0
        jnc     .final
.clear_hub_change:
; 4. Clear the change and continue in usb_hub_change_cleared callback.
        lea     edx, [eax+usb_hub.ChangeConfigBuffer]
        mov     dword [edx], 20h + \    ; class-specific request to hub itself
                (USB_CLEAR_FEATURE shl 8)
        mov     [edx+2], cl     ; feature selector
        and     dword [edx+4], 0
        stdcall usb_control_async, [eax+usb_hub.ConfigPipe], edx, 0, 0, usb_hub_change_cleared, eax, 0
.nothing:
        ret
.final:
; 5. All changes cleared and accumulated, now process them.
; Note: that needs work.
        DEBUGF 1,'K : hub status %x\n',[eax+usb_hub.AccStatusChange]:2
        test    [eax+usb_hub.AccStatusChange], 1
        jz      .no_local_power
        test    [eax+usb_hub.StatusData], 1
        jz      .local_power_lost
        dbgstr 'Hub local power is now good'
        jmp     .no_local_power
.local_power_lost:
        dbgstr 'Hub local power is now lost'
.no_local_power:
        test    [eax+usb_hub.AccStatusChange], 2
        jz      .no_overcurrent
        test    [eax+usb_hub.StatusData], 2
        jz      .no_overcurrent
        dbgstr 'Hub global overcurrent'
.no_overcurrent:
; 6. Process possible changes for other ports.
        jmp     usb_hub_changed.restart
.failed:
        dbgstr 'Querying hub status failed'
        jmp     .nothing
endp

; This procedure is called when the request to clear hub change is completed,
; either successfully or unsuccessfully.
proc usb_hub_change_cleared stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; 1. Check whether our request has failed.
; If so, say something to the debug board and stop processing notifications.
        cmp     [status], 0
        jnz     .failed
; 2. If there is a change which was observed, but not yet cleared,
; go to the code which clears it.
        mov     eax, [calldata]
        cmp     [eax+usb_hub.StatusChange], 0
        jnz     usb_hub_status.next_change
; 3. Otherwise, go to the code which queries the status.
        jmp     usb_hub_changed.next_hub_change
.failed:
        dbgstr 'Clearing hub change failed'
        ret
endp

; This procedure is called when the request of port status is completed,
; either successfully or unsuccessfully.
proc usb_hub_port_status stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; 1. Check whether our request has failed.
; If so, say something to the debug board and stop processing notifications.
        cmp     [status], 0
        jnz     .failed
; 2. Accumulate observed changes.
        mov     eax, [calldata]
;       movzx   ecx, [eax+usb_hub.ChangeConfigBuffer+4]
;       dec     ecx
;       DEBUGF 1,'K : [%d] hub %x port %d status %x change %x\n',[timer_ticks],eax,ecx,[eax+usb_hub.StatusData]:4,[eax+usb_hub.StatusChange]:4
        mov     dl, byte [eax+usb_hub.StatusChange]
        or      [eax+usb_hub.AccStatusChange], dl
.next_change:
; 3. Find the first change. If found, advance to 4. Otherwise, go to 5.
; Ignore change in reset status; it is cleared by synchronous code
; (usb_hub_process_deferred), so avoid unnecessary interference.
;       mov     cl, C_PORT_RESET
        btr     dword [eax+usb_hub.StatusChange], PORT_RESET
;       jc      .clear_port_change
        mov     cl, C_PORT_OVER_CURRENT
        btr     dword [eax+usb_hub.StatusChange], PORT_OVER_CURRENT
        jc      .clear_port_change
        mov     cl, C_PORT_SUSPEND
        btr     dword [eax+usb_hub.StatusChange], PORT_SUSPEND
        jc      .clear_port_change
        mov     cl, C_PORT_ENABLE
        btr     dword [eax+usb_hub.StatusChange], PORT_ENABLE
        jc      .clear_port_change
        mov     cl, C_PORT_CONNECTION
        btr     dword [eax+usb_hub.StatusChange], PORT_CONNECTION
        jnc     .final
.clear_port_change:
; 4. Clear the change and continue in usb_hub_port_changed callback.
        call    usb_hub_clear_port_change
        jmp     .nothing
.final:
; All changes cleared and accumulated, now process them.
        movzx   ecx, byte [eax+usb_hub.ChangeConfigBuffer+4]
        dec     ecx
        DEBUGF 1,'K : final: hub %x port %d status %x change %x\n',eax,ecx,[eax+usb_hub.StatusData]:4,[eax+usb_hub.AccStatusChange]:2
; 5. Process connect/disconnect events.
; 5a. Test whether there is such event.
        test    byte [eax+usb_hub.AccStatusChange], 1 shl PORT_CONNECTION
        jz      .nodisconnect
; 5b. If there was a connected device, notify the main code about disconnect.
        push    ebx
        mov     edx, [eax+usb_hub.ConnectedDevicesPtr]
        xor     ebx, ebx
        xchg    ebx, [edx+ecx*4]
        test    ebx, ebx
        jz      @f
        push    eax ecx
        call    usb_device_disconnected
        pop     ecx eax
@@:
        pop     ebx
; 5c. If the disconnect event corresponds to the port which is currently
; resetting, then another request from synchronous code could be in the fly,
; so aborting reset immediately would lead to problems with those requests.
; Thus, just set the corresponding status and let the synchronous code process.
        test    byte [eax+usb_hub.Actions], (HUB_RESET_SIGNAL or HUB_RESET_RECOVERY)
        jz      @f
        mov     edx, [eax+usb_hub.Controller]
        cmp     [edx+usb_controller.ResettingPort], cl
        jnz     @f
        mov     [edx+usb_controller.ResettingStatus], -1
@@:
; 5d. If the current status is 'connected', store the current time as connect
; time and set the corresponding bit for usb_hub_process_deferred.
; Otherwise, set connect time to -1.
; If current time is -1, pretend that the event occured one tick later and
; store zero.
        mov     edx, [eax+usb_hub.ConnectedTimePtr]
        test    byte [eax+usb_hub.StatusData], 1 shl PORT_CONNECTION
        jz      .disconnected
        or      [eax+usb_hub.Actions], HUB_WAIT_CONNECT
        push    eax
        call    usb_hub_store_connected_time
        pop     eax
        jmp     @f
.disconnected:
        or      dword [edx+ecx*4], -1
@@:
.nodisconnect:
; 6. Process port disabling.
        test    [eax+usb_hub.AccStatusChange], 1 shl PORT_ENABLE
        jz      .nodisable
        test    byte [eax+usb_hub.StatusData], 1 shl PORT_ENABLE
        jnz     .nodisable
; Note: that needs work.
        dbgstr 'Port disabled'
.nodisable:
; 7. Process port overcurrent.
        test    [eax+usb_hub.AccStatusChange], 1 shl PORT_OVER_CURRENT
        jz      .noovercurrent
        test    byte [eax+usb_hub.StatusData], 1 shl PORT_OVER_CURRENT
        jz      .noovercurrent
; Note: that needs work.
        dbgstr 'Port over-current'
.noovercurrent:
; 8. Process possible changes for other ports.
        jmp     usb_hub_changed.restart
.failed:
        dbgstr 'Querying port status failed'
.nothing:
        ret
endp

; Helper procedure to store current time in ConnectedTime,
; advancing -1 to zero if needed.
proc usb_hub_store_connected_time
        mov     eax, [timer_ticks]
; transform -1 to 0, leave other values as is
        cmp     eax, -1
        sbb     eax, -1
        mov     [edx+ecx*4], eax
        ret
endp

; Helper procedure for several parts of hub code.
; Sends a request to clear the given feature of the port.
; eax -> usb_hub, cl = feature;
; as is should be called from async code, sync code should set
; edx to ConfigBuffer and call usb_hub_clear_port_change.buffer;
; port number (1-based) should be filled in [edx+4] by previous requests.
proc usb_hub_clear_port_change
        lea     edx, [eax+usb_hub.ChangeConfigBuffer]
.buffer:
;       push    edx
;       movzx   edx, byte [edx+4]
;       dec     edx
;       DEBUGF 1,'K : [%d] hub %x port %d clear feature %d\n',[timer_ticks],eax,edx,cl
;       pop     edx
        mov     dword [edx], 23h + \    ; class-specific request to hub port
                (USB_CLEAR_FEATURE shl 8)
        mov     byte [edx+2], cl
        and     dword [edx+4], 0xFF
        stdcall usb_control_async, [eax+usb_hub.ConfigPipe], edx, edx, 0, usb_hub_port_changed, eax, 0
        ret
endp

; This procedure is called when the request to clear port change is completed,
; either successfully or unsuccessfully.
proc usb_hub_port_changed stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; 1. Check whether our request has failed.
; If so, say something to the debug board and stop processing notifications.
        cmp     [status], 0
        jnz     .failed
; 2. If the request was originated by synchronous code, no further processing
; is required.
        mov     eax, [calldata]
        lea     edx, [eax+usb_hub.ConfigBuffer]
        cmp     [buffer], edx
        jz      .nothing
; 3. If there is a change which was observed, but not yet cleared,
; go to the code which clears it.
        cmp     [eax+usb_hub.StatusChange], 0
        jnz     usb_hub_port_status.next_change
; 4. Otherwise, go to the code which queries the status.
        jmp     usb_hub_changed.next_port_change
.failed:
        dbgstr 'Clearing port change failed'
.nothing:
        ret
endp

; This procedure is called in the USB thread from usb_thread_proc,
; contains synchronous code which should be activated at certain time
; (e.g. reset a recently connected device after debounce interval 100ms).
; Returns the number of ticks when it should be called next time.
proc usb_hub_process_deferred
; 1. Top-of-stack will contain return value; initialize to infinite timeout.
        push    -1
; 2. If wait for stable power is active, then
; either reschedule wakeup (if time is not over)
; or start processing notifications.
        test    byte [esi+usb_hub.Actions], HUB_WAIT_POWERED
        jz      .no_powered
        movzx   eax, [esi+usb_hub.PowerOnInterval]
; three following instructions are equivalent to edx = ceil(eax / 5) + 1
; 1 extra tick is added to make sure that the interval is at least as needed
; (it is possible that PoweredOnTime was set just before timer interrupt, and
; this test goes on just after timer interrupt)
        add     eax, 9
; two following instructions are equivalent to edx = floor(eax / 5)
; for any 0 <= eax < 40000000h
        mov     ecx, 33333334h
        mul     ecx
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_hub.PoweredOnTime]
        sub     eax, edx
        jge     .powered_on
        neg     eax
        pop     ecx
        push    eax
        jmp     .no_powered
.powered_on:
        and     [esi+usb_hub.Actions], not HUB_WAIT_POWERED
        mov     eax, esi
        call    usb_hub_wait_change
.no_powered:
; 3. If reset is pending, check whether we can start it and start it, if so.
        test    byte [esi+usb_hub.Actions], HUB_RESET_WAITING
        jz      .no_wait_reset
        mov     eax, [esi+usb_hub.Controller]
        cmp     [eax+usb_controller.ResettingPort], -1
        jnz     .no_wait_reset
        call    usb_hub_initiate_reset
.no_wait_reset:
; 4. If reset signalling is active, wait for end of reset signalling
; and schedule wakeup in 1 tick.
        test    byte [esi+usb_hub.Actions], HUB_RESET_SIGNAL
        jz      .no_resetting_port
; It has no sense to query status several times per tick.
        mov     eax, [timer_ticks]
        cmp     eax, [esi+usb_hub.ResetTime]
        jz      @f
        mov     [esi+usb_hub.ResetTime], eax
        movzx   ecx, byte [esi+usb_hub.ConfigBuffer+4]
        mov     eax, usb_hub_resetting_port_status
        call    usb_hub_query_port_status
@@:
        pop     eax
        push    1
.no_resetting_port:
; 5. If reset recovery is active and time is not over, reschedule wakeup.
        test    byte [esi+usb_hub.Actions], HUB_RESET_RECOVERY
        jz      .no_reset_recovery
        mov     eax, [timer_ticks]
        sub     eax, [esi+usb_hub.ResetTime]
        sub     eax, USB_RESET_RECOVERY_TIME
        jge     .reset_done
        neg     eax
        cmp     [esp], eax
        jb      @f
        mov     [esp], eax
@@:
        jmp     .no_reset_recovery
.reset_done:
; 6. If reset recovery is active and time is over, clear 'reset recovery' flag,
; notify other code about a new device and let it do further steps.
; If that fails, stop reset process for this port and disable that port.
        and     [esi+usb_hub.Actions], not HUB_RESET_RECOVERY
; Bits 9-10 of port status encode port speed.
; If PORT_LOW_SPEED is set, the device is low-speed. Otherwise,
; PORT_HIGH_SPEED bit distinguishes full-speed and high-speed devices.
; This corresponds to values of USB_SPEED_FS=0, USB_SPEED_LS=1, USB_SPEED_HS=2.
        mov     eax, dword [esi+usb_hub.ResetStatusData]
        shr     eax, PORT_LOW_SPEED
        and     eax, 3
        test    al, 1
        jz      @f
        mov     al, 1
@@:
;       movzx   ecx, [esi+usb_hub.ConfigBuffer+4]
;       dec     ecx
;       DEBUGF 1,'K : [%d] hub %x port %d speed %d\n',[timer_ticks],esi,ecx,eax
        push    esi
        mov     esi, [esi+usb_hub.Controller]
        cmp     [esi+usb_controller.ResettingStatus], -1
        jz      .disconnected_while_reset
        mov     edx, [esi+usb_controller.HardwareFunc]
        call    [edx+usb_hardware_func.NewDevice]
        pop     esi
        test    eax, eax
        jnz     .no_reset_recovery
        mov     eax, esi
        call    usb_hub_disable_resetting_port
        jmp     .no_reset_recovery
.disconnected_while_reset:
        pop     esi
        mov     eax, esi
        call    usb_hub_reset_aborted
.no_reset_recovery:
; 7. Handle recent connection events.
; Note: that should be done after step 6, because step 6 can clear
; HUB_RESET_IN_PROGRESS flag.
; 7a. Test whether there is such an event pending. If no, skip this step.
        test    byte [esi+usb_hub.Actions], HUB_WAIT_CONNECT
        jz      .no_wait_connect
; 7b. If we have started reset process for another port in the same hub,
; skip this step: the buffer for config requests can be used for that port.
        test    byte [esi+usb_hub.Actions], HUB_RESET_IN_PROGRESS
        jnz     .no_wait_connect
; 7c. Clear flag 'there are connection events which should be processed'.
; If there are another connection events, this flag will be set again.
        and     [esi+usb_hub.Actions], not HUB_WAIT_CONNECT
; 7d. Prepare for loop over all ports.
        xor     ecx, ecx
.test_wait_connect:
; 7e. For every port test for recent connection event.
; If none, continue the loop for the next port.
        mov     edx, [esi+usb_hub.ConnectedTimePtr]
        mov     eax, [edx+ecx*4]
        cmp     eax, -1
        jz      .next_wait_connect
        or      [esi+usb_hub.Actions], HUB_WAIT_CONNECT
; 7f. Test whether initial delay is over.
        sub     eax, [timer_ticks]
        neg     eax
        sub     eax, USB_CONNECT_DELAY
        jge     .connect_delay_over
; 7g. The initial delay is not over;
; set the corresponding flag again, reschedule wakeup and continue the loop.
        neg     eax
        cmp     [esp], eax
        jb      @f
        mov     [esp], eax
@@:
        jmp     .next_wait_connect
.connect_delay_over:
; The initial delay is over.
; It is possible that there was disconnect event during that delay, probably
; with connect event after that. If so, we should restart the waiting. However,
; on the hardware level connect/disconnect events from hubs are implemented
; using polling with interval selected by the hub, so it is possible that
; we have not yet observed that disconnect event.
; Thus, we query port status+change data before all further processing.
; 7h. Send the request for status+change data.
        push    ecx
; Hub requests expect 1-based port number, not zero-based we operate with.
        inc     ecx
        mov     eax, usb_hub_connect_port_status
        call    usb_hub_query_port_status
        pop     ecx
; 3i. If request has been submitted successfully, set the flag
; 'reset in progress, config buffer is owned by reset process' and break
; from the loop.
        test    eax, eax
        jz      .next_wait_connect
        or      [esi+usb_hub.Actions], HUB_RESET_IN_PROGRESS
        jmp     .no_wait_connect
.next_wait_connect:
; 7j. Continue the loop for next port.
        inc     ecx
        cmp     ecx, [esi+usb_hub.NumPorts]
        jb      .test_wait_connect
.no_wait_connect:
; 8. Pop return value from top-of-stack and return.
        pop     eax
        ret
endp

; Helper procedure for other code. Called when reset process is aborted.
proc usb_hub_reset_aborted
; Clear 'reset in progress' flag and test for other devices which could be
; waiting for reset.
        and     [eax+usb_hub.Actions], not HUB_RESET_IN_PROGRESS
        push    esi
        mov     esi, [eax+usb_hub.Controller]
        call    usb_test_pending_port
        pop     esi
        ret
endp

; Helper procedure for usb_hub_process_deferred.
; Sends a request to query port status.
; esi -> usb_hub, eax = callback, ecx = 1-based port.
proc usb_hub_query_port_status
;       dec     ecx
;       DEBUGF 1,'K : [%d] [main] hub %x port %d query status\n',[timer_ticks],esi,ecx
;       inc     ecx
        add     ecx, 4 shl 16           ; data length = 4
        lea     edx, [esi+usb_hub.ConfigBuffer]
        mov     dword [edx], 0A3h + \   ; class-specific request from hub port
                (USB_GET_STATUS shl 8)
        mov     dword [edx+4], ecx
        lea     ecx, [esi+usb_hub.ResetStatusData]
        stdcall usb_control_async, [esi+usb_hub.ConfigPipe], edx, ecx, 4, eax, esi, 0
        ret
endp

; This procedure is called when the request to query port status
; initiated by usb_hub_process_deferred for testing connection is completed,
; either successfully or unsuccessfully.
proc usb_hub_connect_port_status stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
        push    esi     ; save used register to be stdcall
        mov     eax, [calldata]
        mov     esi, [pipe]
;       movzx   ecx, [eax+usb_hub.ConfigBuffer+4]
;       dec     ecx
;       DEBUGF 1,'K : [%d] [connect test] hub %x port %d status %x change %x\n',[timer_ticks],eax,ecx,[eax+usb_hub.ResetStatusData]:4,[eax+usb_hub.ResetStatusChange]:4
; 1. In any case, clear 'reset in progress' flag.
; If everything is ok, it would be set again.
        and     [eax+usb_hub.Actions], not HUB_RESET_IN_PROGRESS
; 2. If the request has failed, stop reset process.
        cmp     [status], 0
        jnz     .nothing
        mov     edx, [eax+usb_hub.ConnectedTimePtr]
        movzx   ecx, byte [eax+usb_hub.ConfigBuffer+4]
        dec     ecx
; 3. Test whether there was a disconnect event.
        test    byte [eax+usb_hub.ResetStatusChange], 1 shl PORT_CONNECTION
        jz      .reset
; 4. There was a disconnect event.
; There is another handler of connect/disconnect events, usb_hub_port_status.
; However, we do not know whether it has already processed this event
; or it will process it sometime later.
; If ConnectedTime is -1, then another handler has already run,
; there was no connection event, so just leave the value as -1.
; Otherwise, there are two possibilities: either another handler has not yet
; run (which is quite likely), or there was a connection event and the other
; handler has run exactly while our request was processed (otherwise our
; request would not been submitted; this is quite unlikely due to timing
; requirements, but not impossible). In this case, set ConnectedTime to the
; current time: in the likely case it prevents usb_hub_process_deferred from immediate
; issuing of another requests (which would be just waste of time);
; in the unlikely case it is still correct (although slightly increases
; the debounce interval).
        cmp     dword [edx+ecx*4], -1
        jz      .nothing
        call    usb_hub_store_connected_time
        jmp     .nothing
.reset:
; 5. The device remained connected for the entire debounce interval;
; we can proceed with initialization.
; Clear connected time for this port and notify usb_hub_process_deferred that
; the new port is waiting for reset.
        or      dword [edx+ecx*4], -1
        or      [eax+usb_hub.Actions], HUB_RESET_IN_PROGRESS + HUB_RESET_WAITING
.nothing:
        pop     esi     ; restore used register to be stdcall
        ret
endp

; This procedure is called when the request to query port status
; initiated by usb_hub_process_deferred for testing reset status is completed,
; either successfully or unsuccessfully.
proc usb_hub_resetting_port_status stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; 1. If the request has failed, do nothing.
        cmp     [status], 0
        jnz     .nothing
; 2. If reset signalling is still active, do nothing.
        mov     eax, [calldata]
;       movzx   ecx, [eax+usb_hub.ConfigBuffer+4]
;       dec     ecx
;       DEBUGF 1,'K : hub %x port %d ResetStatusData = %x change = %x\n',eax,ecx,[eax+usb_hub.ResetStatusData]:4,[eax+usb_hub.ResetStatusChange]:4
        test    byte [eax+usb_hub.ResetStatusData], 1 shl PORT_RESET
        jnz     .nothing
; 3. Store the current time to start reset recovery interval
; and clear 'reset signalling active' flag.
        mov     edx, [timer_ticks]
        mov     [eax+usb_hub.ResetTime], edx
        and     [eax+usb_hub.Actions], not HUB_RESET_SIGNAL
; 4. If the device has not been disconnected, set 'reset recovery active' bit.
; Otherwise, terminate reset process.
        test    byte [eax+usb_hub.ResetStatusChange], 1 shl PORT_CONNECTION
        jnz     .disconnected
        or      [eax+usb_hub.Actions], HUB_RESET_RECOVERY
.common:
; In any case, clear change of resetting status.
        lea     edx, [eax+usb_hub.ConfigBuffer]
        mov     cl, C_PORT_RESET
        call    usb_hub_clear_port_change.buffer
.nothing:
        ret
.disconnected:
        call    usb_hub_reset_aborted
        jmp     .common
endp

; Helper procedure for usb_hub_process_deferred. Initiates reset signalling
; on the current port (given by 1-based value [ConfigBuffer+4]).
; esi -> usb_hub, eax -> usb_controller
proc usb_hub_initiate_reset
; 1. Store hub+port data in the controller structure.
        movzx   ecx, [esi+usb_hub.ConfigBuffer+4]
        dec     ecx
        mov     [eax+usb_controller.ResettingPort], cl
        mov     [eax+usb_controller.ResettingHub], esi
; 2. Store the current time and set 'reset signalling active' flag.
        mov     eax, [timer_ticks]
        mov     [esi+usb_hub.ResetTime], eax
        and     [esi+usb_hub.Actions], not HUB_RESET_WAITING
        or      [esi+usb_hub.Actions], HUB_RESET_SIGNAL
; 3. Send request to the hub to initiate request signalling.
        lea     edx, [esi+usb_hub.ConfigBuffer]
;       DEBUGF 1,'K : [%d] hub %x port %d initiate reset\n',[timer_ticks],esi,ecx
        mov     dword [edx], 23h + \
                (USB_SET_FEATURE shl 8) + \
                (PORT_RESET shl 16)
        and     dword [edx+4], 0xFF
        stdcall usb_control_async, [esi+usb_hub.ConfigPipe], edx, 0, 0, usb_hub_reset_started, esi, 0
        test    eax, eax
        jnz     @f
        mov     eax, esi
        call    usb_hub_reset_aborted
@@:
        ret
endp

; This procedure is called when the request to start reset signalling initiated
; by usb_hub_initiate_reset is completed, either successfully or unsuccessfully.
proc usb_hub_reset_started stdcall, pipe:dword, status:dword, buffer:dword, length:dword, calldata:dword
; If the request is successful, do nothing.
; Otherwise, clear 'reset signalling' flag and abort reset process.
        mov     eax, [calldata]
;       movzx   ecx, [eax+usb_hub.ConfigBuffer+4]
;       dec     ecx
;       DEBUGF 1,'K : [%d] hub %x port %d reset started\n',[timer_ticks],eax,ecx
        cmp     [status], 0
        jz      .nothing
        and     [eax+usb_hub.Actions], not HUB_RESET_SIGNAL
        dbgstr 'Failed to reset hub port'
        call    usb_hub_reset_aborted
.nothing:
        ret
endp

; This procedure is called by the protocol layer if something has failed during
; initial stages of the configuration process, so the device should be disabled
; at hub level.
proc usb_hub_disable_resetting_port
        and     [eax+usb_hub.Actions], not HUB_RESET_IN_PROGRESS
;       movzx   ecx, [eax+usb_hub.ConfigBuffer+4]
;       dec     ecx
;       DEBUGF 1,'K : [%d] hub %x port %d disable\n',[timer_ticks],eax,ecx
        lea     edx, [eax+usb_hub.ConfigBuffer]
        mov     cl, PORT_ENABLE
        jmp     usb_hub_clear_port_change.buffer
endp

; This procedure is called when the hub is disconnected.
proc usb_hub_disconnect
virtual at esp
                dd      ?       ; return address
.hubdata        dd      ?
end virtual
; 1. If the hub is disconnected during initial configuration,
; 1 is stored as hub data and there is nothing to do.
        mov     eax, [.hubdata]
        cmp     eax, 1
        jz      .nothing
; 2. Remove the hub from the overall list.
        mov     ecx, [eax+usb_hub.Next]
        mov     edx, [eax+usb_hub.Prev]
        mov     [ecx+usb_hub.Prev], edx
        mov     [edx+usb_hub.Next], ecx
; 3. If some child is in reset process, abort reset.
        push    esi
        mov     esi, [eax+usb_hub.Controller]
        cmp     [esi+usb_controller.ResettingHub], eax
        jnz     @f
        cmp     [esi+usb_controller.ResettingPort], -1
        jz      @f
        push    eax
        call    usb_test_pending_port
        pop     eax
@@:
        pop     esi
; 4. Loop over all children and notify other code that they were disconnected.
        push    ebx
        xor     ecx, ecx
.disconnect_children:
        mov     ebx, [eax+usb_hub.ConnectedDevicesPtr]
        mov     ebx, [ebx+ecx*4]
        test    ebx, ebx
        jz      @f
        push    eax ecx
        call    usb_device_disconnected
        pop     ecx eax
@@:
        inc     ecx
        cmp     ecx, [eax+usb_hub.NumPorts]
        jb      .disconnect_children
; 4. Free memory allocated for the hub data.
        call    free
        pop     ebx
.nothing:
        retn    4
endp
