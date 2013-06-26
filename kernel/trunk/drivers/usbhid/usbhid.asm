; standard driver stuff
format MS COFF

DEBUG = 1

; this is for DEBUGF macro from 'fdo.inc'
__DEBUG__ = 1
__DEBUG_LEVEL__ = 1

include '../proc32.inc'
include '../imports.inc'
include '../fdo.inc'
include '../../struct.inc'

public START
public version

; Compile-time settings.
; If set, the code will dump all descriptors as they are read to the debug board.
USB_DUMP_DESCRIPTORS = 1
; If set, the code will dump any unclaimed input to the debug board.
HID_DUMP_UNCLAIMED = 1

; USB constants
DEVICE_DESCR_TYPE           = 1
CONFIG_DESCR_TYPE           = 2
STRING_DESCR_TYPE           = 3
INTERFACE_DESCR_TYPE        = 4
ENDPOINT_DESCR_TYPE         = 5
DEVICE_QUALIFIER_DESCR_TYPE = 6

CONTROL_PIPE     = 0
ISOCHRONOUS_PIPE = 1
BULK_PIPE        = 2
INTERRUPT_PIPE   = 3

; USB HID constants
HID_DESCR_TYPE      = 21h
REPORT_DESCR_TYPE   = 22h
PHYSICAL_DESCR_TYPE = 23h

; USB structures
struct config_descr
bLength                 db      ?
bDescriptorType         db      ?
wTotalLength            dw      ?
bNumInterfaces          db      ?
bConfigurationValue     db      ?
iConfiguration          db      ?
bmAttributes            db      ?
bMaxPower               db      ?
ends

struct interface_descr
bLength                 db      ?
bDescriptorType         db      ?
bInterfaceNumber        db      ?
bAlternateSetting       db      ?
bNumEndpoints           db      ?
bInterfaceClass         db      ?
bInterfaceSubClass      db      ?
bInterfaceProtocol      db      ?
iInterface              db      ?
ends

struct endpoint_descr
bLength                 db      ?
bDescriptorType         db      ?
bEndpointAddress        db      ?
bmAttributes            db      ?
wMaxPacketSize          dw      ?
bInterval               db      ?
ends

; USB HID structures
struct hid_descr
bLength			db	?
bDescriptorType		db	?
bcdHID			dw	?
bCountryCode		db	?
bNumDescriptors		db	?
base_sizeof	rb	0
; now two fields are repeated .bNumDescriptors times:
subDescriptorType	db	?
subDescriptorLength	dw	?
ends

; Include macro for parsing report descriptors/data.
macro workers_globals
{}
include 'report.inc'

; Driver data for all devices
struct usb_device_data
hid			hid_data	; data of HID layer
epdescr			dd	?	; endpoint descriptor
hiddescr		dd	?	; HID descriptor
interface_number	dd	?	; copy of interface_descr.bInterfaceNumber
configpipe		dd	?	; config pipe handle
intpipe			dd	?	; interrupt pipe handle
input_transfer_size	dd	?	; input transfer size
input_buffer		dd	?	; buffer for input transfers
control			rb	8	; control packet to device
ends

section '.flat' code readable align 16
; The start procedure.
proc START
virtual at esp
	dd	?	; return address
.reason	dd	?
end virtual
; 1. Test whether the procedure is called with the argument DRV_ENTRY.
; If not, return 0.
        xor     eax, eax        ; initialize return value
        cmp     [.reason], 1    ; compare the argument
        jnz     .nothing
; 2. Register self as a USB driver.
; The name is my_driver = 'usbhid'; IOCTL interface is not supported;
; usb_functions is an offset of a structure with callback functions.
        stdcall RegUSBDriver, my_driver, eax, usb_functions
; 3. Return the returned value of RegUSBDriver.
.nothing:
        ret     4
endp

; This procedure is called when new HID device is detected.
; It initializes the device.
proc AddDevice
	push	ebx esi edi	; save used registers to be stdcall
virtual at esp
		rd	3	; saved registers
		dd	?	; return address
.config_pipe	dd	?
.config_descr	dd	?
.interface	dd	?
end virtual
        DEBUGF 1,'K : USB HID device detected\n'
; 1. Allocate memory for device data.
	movi	eax, sizeof.usb_device_data
        call    Kmalloc
        test    eax, eax
        jnz	@f
        mov	esi, nomemory_msg
        call	SysMsgBoardStr
        jmp	.return0
@@:
; zero-initialize it
	mov	edi, eax
	xchg	eax, ebx
	xor	eax, eax
	movi	ecx, sizeof.usb_device_data / 4
	rep	stosd
        mov     edx, [.interface]
; HID devices use one IN interrupt endpoint for polling the device
; and an optional OUT interrupt endpoint. We do not use the later,
; but must locate the first. Look for the IN interrupt endpoint.
; Also, look for the HID descriptor; according to HID spec, it must be
; located before endpoint descriptors.
; 2. Get the upper bound of all descriptors' data.
        mov     eax, [.config_descr]
        movzx   ecx, [eax+config_descr.wTotalLength]
        add     eax, ecx
; 3. Loop over all descriptors until
; either end-of-data reached - this is fail
; or interface descriptor found - this is fail, all further data
;    correspond to that interface
; or endpoint descriptor for IN endpoint is found
; (HID descriptor must be located before the endpoint descriptor).
; 3a. Loop start: edx points to the interface descriptor.
.lookep:
; 3b. Get next descriptor.
        movzx   ecx, byte [edx] ; the first byte of all descriptors is length
        test	ecx, ecx
        jz	.cfgerror
        add     edx, ecx
; 3c. Check that at least two bytes are readable. The opposite is an error.
        inc     edx
        cmp     edx, eax
        jae     .cfgerror
        dec     edx
; 3d. Check that this descriptor is not interface descriptor. The opposite is
; an error.
        cmp     [edx+endpoint_descr.bDescriptorType], INTERFACE_DESCR_TYPE
        jz      .cfgerror
; 3e. For HID descriptor, proceed to 4.
; For endpoint descriptor, go to 5.
; For other descriptors, continue the loop.
; Note: bDescriptorType is in the same place in all descriptors.
        cmp     [edx+endpoint_descr.bDescriptorType], ENDPOINT_DESCR_TYPE
        jz	.foundep
        cmp	[edx+endpoint_descr.bDescriptorType], HID_DESCR_TYPE
        jnz	.lookep
; 4a. Check that the descriptor contains all required data and all data are
; readable. The opposite is an error.
	movzx	ecx, [edx+hid_descr.bLength]
	cmp	ecx, hid_descr.base_sizeof + 3
	jb	.cfgerror
	add	ecx, edx
	cmp	ecx, eax
	ja	.cfgerror
; 4b. Store the pointer in usb_device_data structure for further references.
	mov	[ebx+usb_device_data.hiddescr], edx
; 4c. Continue the loop.
	jmp	.lookep
.foundep:
; 5a. Check that the descriptor contains all required data and all data are
; readable. The opposite is an error.
        cmp     byte [edx+endpoint_descr.bLength], sizeof.endpoint_descr
        jb      .cfgerror
        lea	ecx, [edx+sizeof.endpoint_descr]
        cmp	ecx, eax
        jbe	@f
; 6. An error occured during processing endpoint descriptor.
.cfgerror:
; 6a. Print a message.
	mov	esi, invalid_config_descr_msg
	call	SysMsgBoardStr
; 6b. Free memory allocated for device data.
.free:
        xchg    eax, ebx
        call    Kfree
.return0:
; 6c. Return an error.
        xor     eax, eax
.nothing:
	pop	edi esi ebx	; restore used registers to be stdcall
        ret	12
@@:
; 5b. If this is not IN interrupt endpoint, ignore it and continue the loop.
	test	[edx+endpoint_descr.bEndpointAddress], 80h
	jz	.lookep
	mov	cl, [edx+endpoint_descr.bmAttributes]
	and	cl, 3
	cmp	cl, INTERRUPT_PIPE
	jnz	.lookep
; 5c. Store the pointer in usb_device_data structure for futher references.
	mov	[ebx+usb_device_data.epdescr], edx
; 5d. Check that HID descriptor was found. If not, go to 6.
	cmp	[ebx+usb_device_data.hiddescr], 0
	jz	.cfgerror
.descriptors_found:
; 6. Configuration descriptor seems to be ok.
; Send SET_IDLE command disabling auto-repeat feature (it is quite useless)
; and continue configuring in SET_IDLE callback.
	lea	edx, [ebx+usb_device_data.control]
	mov	eax, [.interface]
	mov	dword [edx], 21h + \	; Class-specific request to Interface
		(0Ah shl 8) + \		; SET_IDLE
		(0 shl 16) + \		; apply to all input reports
		(0 shl 24)		; disable auto-repeat
	movzx	eax, [eax+interface_descr.bInterfaceNumber]
	mov	[ebx+usb_device_data.interface_number], eax
	mov	[edx+4], eax		; set interface number, zero length
	mov	eax, [.config_pipe]
	mov	[ebx+usb_device_data.configpipe], eax
	xor	ecx, ecx
	stdcall	USBControlTransferAsync, eax, edx, ecx, ecx, idle_set, ebx, ecx
; 7. Return pointer to usb_device_data.
	xchg	eax, ebx
	jmp	.nothing
endp

; This procedure is called by USB stack when SET_IDLE request initiated by
; AddDevice is completed, either successfully or unsuccessfully.
proc idle_set
	push	ebx esi		; save used registers to be stdcall
virtual at esp
		rd	2	; saved registers
		dd	?	; return address
.pipe		dd	?
.status		dd	?
.buffer		dd	?
.length		dd	?
.calldata	dd	?
end virtual
; Ignore status. Support for SET_IDLE is optional, so the device is free to
; STALL the request; config pipe should remain functional without explicit cleanup.
	mov	ebx, [.calldata]
; 1. HID descriptor contains length of Report descriptor. Parse it.
	mov	esi, [ebx+usb_device_data.hiddescr]
	movzx	ecx, [esi+hid_descr.bNumDescriptors]
	lea	eax, [hid_descr.base_sizeof+ecx*3]
	cmp	eax, 100h
	jae	.cfgerror
	cmp	al, [esi+hid_descr.bLength]
	jb	.cfgerror
.look_report:
	dec	ecx
	js	.cfgerror
	cmp	[esi+hid_descr.subDescriptorType], REPORT_DESCR_TYPE
	jz	.found_report
	add	esi, 3
	jmp	.look_report
.cfgerror:
	mov	esi, invalid_config_descr_msg
.abort_with_msg:
	call	SysMsgBoardStr
	jmp	.nothing
.found_report:
; 2. Send request for the Report descriptor.
; 2a. Allocate memory.
	movzx	eax, [esi+hid_descr.subDescriptorLength]
	test	eax, eax
	jz	.cfgerror
	push	eax
	call	Kmalloc
	pop	ecx
; If failed, say a message and stop initialization.
	mov	esi, nomemory_msg
	test	eax, eax
	jz	.abort_with_msg
; 2b. Submit the request.
	xchg	eax, esi
	lea	edx, [ebx+usb_device_data.control]
	mov	eax, [ebx+usb_device_data.interface_number]
	mov	dword [edx], 81h + \	; Standard request to Interface
		(6 shl 8) + \		; GET_DESCRIPTOR
		(0 shl 16) + \		; descriptor index: there is only one report descriptor
		(REPORT_DESCR_TYPE shl 24); descriptor type
	mov	[edx+4], ax		; Interface number
	mov	[edx+6], cx		; descriptor length
	stdcall	USBControlTransferAsync, [ebx+usb_device_data.configpipe], \
		edx, esi, ecx, got_report, ebx, 0
; 2c. If failed, free the buffer and stop initialization.
	test	eax, eax
	jnz	.nothing
	xchg	eax, esi
	call	Kfree
.nothing:
	pop	esi ebx		; restore used registers to be stdcall
	ret	20
endp

; This procedure is called by USB stack when the report descriptor queried
; by idle_set is completed, either successfully or unsuccessfully.
proc got_report stdcall uses ebx esi edi, pipe, status, buffer, length, calldata
locals
parse_descr_locals
if ~HID_DUMP_UNCLAIMED
has_driver	db	?
		rb	3
end if
endl
; 1. Check the status; if the request has failed, say something to the debug board
; and stop initialization.
	cmp	[status], 0
	jnz	.generic_fail
; 2. Subtract size of setup packet from the total length;
; the rest is length of the descriptor, and it must be nonzero.
	sub	[length], 8
	ja	.has_something
.generic_fail:
	push	esi
	mov	esi, reportfail
	call	SysMsgBoardStr
	pop	esi
	jmp	.exit
.has_something:
; 3. Process descriptor.
; 3a. Dump it to the debug board, if enabled in compile-time setting.
if USB_DUMP_DESCRIPTORS
	mov	eax, [buffer]
	mov	ecx, [length]
	DEBUGF 1,'K : report descriptor:'
@@:
	DEBUGF 1,' %x',[eax]:2
	inc	eax
	dec	ecx
	jnz	@b
	DEBUGF 1,'\n'
end if
; 3b. Call the HID layer.
	parse_descr
	cmp	[report_ok], 0
	jz	got_report.exit
	mov	ebx, [calldata]
	postprocess_descr
; 4. Stop initialization if no driver is assigned.
if ~HID_DUMP_UNCLAIMED
	cmp	[has_driver], 0
	jz	got_report.exit
end if
; 5. Open interrupt IN pipe. If failed, stop initialization.
	mov	edx, [ebx+usb_device_data.epdescr]
        movzx   ecx, [edx+endpoint_descr.bEndpointAddress]
        movzx   eax, [edx+endpoint_descr.bInterval]
        movzx   edx, [edx+endpoint_descr.wMaxPacketSize]
	stdcall	USBOpenPipe, [ebx+usb_device_data.configpipe], ecx, edx, INTERRUPT_PIPE, eax
	test	eax, eax
	jz	got_report.exit
	mov	[ebx+usb_device_data.intpipe], eax
; 6. Initialize buffer for input packet.
; 6a. Find the length of input packet.
; This is the maximal length of all input reports.
	mov	edx, [ebx+usb_device_data.hid.input.first_report]
	xor	eax, eax
.find_input_size:
	test	edx, edx
	jz	.found_input_size
	cmp	eax, [edx+report.size]
	jae	@f
	mov	eax, [edx+report.size]
@@:
	mov	edx, [edx+report.next]
	jmp	.find_input_size
.found_input_size:
; report.size is in bits, transform it to bytes
	add	eax, 7
	shr	eax, 3
; if reports are numbered, the first byte is report ID, include it
	cmp	[ebx+usb_device_data.hid.input.numbered], 0
	jz	@f
	inc	eax
@@:
	mov	[ebx+usb_device_data.input_transfer_size], eax
; 6b. Allocate memory for input packet: dword-align and add additional dword
; for extract_field_value.
	add	eax, 4+3
	and	eax, not 3
	call	Kmalloc
	test	eax, eax
	jnz	@f
	mov	esi, nomemory_msg
	call	SysMsgBoardStr
	jmp	got_report.exit
@@:
	mov	[ebx+usb_device_data.input_buffer], eax
; 7. Submit a request for input packet and wait for input.
	call	ask_for_input
got_report.exit:
	mov	eax, [buffer]
	call	Kfree
	ret
endp

; Helper procedure for got_report and got_input.
; Submits a request for the next input packet.
proc ask_for_input
; just call USBNormalTransferAsync with correct parameters,
; allow short packets
	stdcall	USBNormalTransferAsync, \
		[ebx+usb_device_data.intpipe], \
		[ebx+usb_device_data.input_buffer], \
		[ebx+usb_device_data.input_transfer_size], \
		got_input, ebx, \
		1
	ret
endp

; This procedure is called by USB stack when a HID device responds with input
; data packet.
proc got_input stdcall uses ebx esi edi, pipe, status, buffer, length, calldata
locals
parse_input_locals
endl
; 1. Validate parameters: fail on error, ignore zero-length transfers.
	mov	ebx, [calldata]
	cmp	[status], 0
	jnz	.fail
	cmp	[length], 0
	jz	.done
; 2. Get pointer to report in esi.
; 2a. If there are no report IDs, use hid.input.data.
	mov	eax, [buffer]
	mov	esi, [ebx+usb_device_data.hid.input.data]
	cmp	[ebx+usb_device_data.hid.input.numbered], 0
	jz	.report_found
; 2b. Otherwise, the first byte of report is report ID;
; locate the report by its ID, advance buffer+length to one byte.
	movzx	eax, byte [eax]
	mov	esi, [esi+eax*4]
	inc	[buffer]
	dec	[length]
.report_found:
; 3. Validate: ignore transfers with unregistered report IDs
; and transfers which are too short for the corresponding report.
	test	esi, esi
	jz	.done
	mov	eax, [esi+report.size]
	add	eax, 7
	shr	eax, 3
	cmp	eax, [length]
	ja	.done
; 4. Pass everything to HID layer.
	parse_input
.done:
; 5. Query the next input.
	mov	ebx, [calldata]
	call	ask_for_input
.nothing:
	ret
.fail:
	mov	esi, transfer_error_msg
	call	SysMsgBoardStr
	jmp	.nothing
endp

; This function is called by the USB subsystem when a device is disconnected.
proc DeviceDisconnected
        push    ebx esi edi	; save used registers to be stdcall
virtual at esp
		rd	3	; saved registers
		dd	?	; return address
.device_data	dd	?
end virtual
; 1. Say a message.
        mov     ebx, [.device_data]
        mov     esi, disconnectmsg
        stdcall SysMsgBoardStr
; 2. Ask HID layer to release all HID-related resources.
	hid_cleanup
; 3. Free the device data.
        xchg    eax, ebx
        call    Kfree
; 4. Return.
.nothing:
        pop     edi esi ebx	; restore used registers to be stdcall
        ret     4       ; purge one dword argument to be stdcall
endp

include 'sort.inc'
include 'unclaimed.inc'
include 'mouse.inc'
include 'keyboard.inc'

; strings
my_driver       db      'usbhid',0
nomemory_msg	db	'K : no memory',13,10,0
invalid_config_descr_msg db 'K : invalid config descriptor',13,10,0
reportfail	db	'K : failed to read report descriptor',13,10,0
transfer_error_msg db	'K : USB transfer error, disabling HID device',13,10,0
disconnectmsg	db	'K : USB HID device disconnected',13,10,0
invalid_report_msg db	'K : report descriptor is invalid',13,10,0
delimiter_note	db	'K : note: alternate usage ignored',13,10,0

; Exported variable: kernel API version.
align 4
version dd      50005h
; Structure with callback functions.
usb_functions:
        dd      12
        dd      AddDevice
        dd      DeviceDisconnected

; for DEBUGF macro
include_debug_strings

; Workers data
workers_globals

; for uninitialized data
;section '.data' data readable writable align 16
