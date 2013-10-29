; standard driver stuff
format MS COFF

DEBUG = 1
DUMP_PACKETS = 0

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

; Mass storage protocol constants, USB layer
REQUEST_GETMAXLUN = 0xFE        ; get max lun
REQUEST_BORESET = 0xFF          ; bulk-only reset

; Mass storage protocol structures, USB layer
; Sent from host to device in the first stage of an operation.
struc command_block_wrapper
{
.Signature      dd      ?       ; the constant 'USBC'
.Tag            dd      ?       ; identifies response with request
.Length         dd      ?       ; length of data-transport phase
.Flags          db      ?       ; one of CBW_FLAG_*
CBW_FLAG_OUT = 0
CBW_FLAG_IN = 80h
.LUN            db      ?       ; addressed unit
.CommandLength  db      ?       ; the length of the following field
.Command        rb      16
.sizeof:
}
virtual at 0
command_block_wrapper   command_block_wrapper
end virtual

; Sent from device to host in the last stage of an operation.
struc command_status_wrapper
{
.Signature      dd      ?       ; the constant 'USBS'
.Tag            dd      ?       ; identifies response with request
.LengthRest     dd      ?       ; .Length - (size of data which were transferred)
.Status         db      ?       ; one of CSW_STATUS_*
CSW_STATUS_OK = 0
CSW_STATUS_FAIL = 1
CSW_STATUS_FATAL = 2
.sizeof:
}
virtual at 0
command_status_wrapper  command_status_wrapper
end virtual

; Constants of SCSI layer
SCSI_REQUEST_SENSE = 3
SCSI_INQUIRY = 12h
SCSI_READ_CAPACITY = 25h
SCSI_READ10 = 28h
SCSI_WRITE10 = 2Ah

; Result of SCSI REQUEST SENSE command.
SENSE_UNKNOWN = 0
SENSE_RECOVERED_ERROR = 1
SENSE_NOT_READY = 2
SENSE_MEDIUM_ERROR = 3
SENSE_HARDWARE_ERROR = 4
SENSE_ILLEGAL_REQUEST = 5
SENSE_UNIT_ATTENTION = 6
SENSE_DATA_PROTECT = 7
SENSE_BLANK_CHECK = 8
; 9 is vendor-specific
SENSE_COPY_ABORTED = 10
SENSE_ABORTED_COMMAND = 11
SENSE_EQUAL = 12
SENSE_VOLUME_OVERFLOW = 13
SENSE_MISCOMPARE = 14
; 15 is reserved

; Structures of SCSI layer
; Result of SCSI INQUIRY request.
struc inquiry_data
{
.PeripheralDevice       db      ?       ; lower 5 bits are PeripheralDeviceType
                                        ; upper 3 bits are PeripheralQualifier
.RemovableMedium        db      ?       ; upper bit is RemovableMedium
                                        ; other bits are for compatibility
.Version                db      ?       ; lower 3 bits are ANSI-Approved version
                                        ; next 3 bits are ECMA version
                                        ; upper 2 bits are ISO version
.ResponseDataFormat     db      ?       ; lower 4 bits are ResponseDataFormat
                                        ; bit 6 is TrmIOP
                                        ; bit 7 is AENC
.AdditionalLength       db      ?
                        dw      ?       ; reserved
.Flags                  db      ?
.VendorID               rb      8       ; vendor ID, big-endian
.ProductID              rb      16      ; product ID, big-endian
.ProductRevBE           dd      ?       ; product revision, big-endian
.sizeof:
}
virtual at 0
inquiry_data inquiry_data
end virtual

struc sense_data
{
.ErrorCode              db      ?       ; lower 7 bits are error code:
                                        ; 70h = current error,
                                        ; 71h = deferred error
                                        ; upper bit is InformationValid
.SegmentNumber          db      ?       ; number of segment descriptor
                                        ; for commands COPY [+VERIFY], COMPARE
.SenseKey               db      ?       ; bits 0-3 are one of SENSE_*
                                        ; bit 4 is reserved
                                        ; bit 5 is IncorrectLengthIndicator
                                        ; bits 6 and 7 are used by
                                        ; sequential-access devices
.Information            dd      ?       ; command-specific
.AdditionalLength       db      ?       ; length of data starting here
.CommandInformation     dd      ?       ; command-specific
.AdditionalSenseCode    db      ?       ; \ more detailed error code
.AdditionalSenseQual    db      ?       ; / standard has a large table of them
.FRUCode                db      ?       ; which part of device has failed
                                        ; (device-specific, not regulated)
.SenseKeySpecific       rb      3       ; depends on SenseKey
.sizeof:
}
virtual at 0
sense_data sense_data
end virtual

; Device data
; USB Mass storage device has one or more logical units, identified by LUN,
; logical unit number. The highest value of LUN, that is, number of units
; minus 1, can be obtained via control request Get Max LUN.
virtual at 0
usb_device_data:
.ConfigPipe             dd      ?       ; configuration pipe
.OutPipe                dd      ?       ; pipe for OUT bulk endpoint
.InPipe                 dd      ?       ; pipe for IN bulk endpoint
.MaxLUN                 dd      ?       ; maximum Logical Unit Number
.LogicalDevices         dd      ?       ; pointer to array of usb_unit_data
; 1 for a connected USB device, 1 for each disk device
; the structure can be freed when .NumReferences decreases to zero
.NumReferences          dd      ?       ; number of references
.ConfigRequest          rb      8       ; buffer for configuration requests
.LengthRest             dd      ?       ; Length - (size of data which were transferred)
; All requests to a given device are serialized,
; only one request to a given device can be processed at a time.
; The current request and all pending requests are organized in the following
; queue, the head being the current request.
; NB: the queue must be device-wide due to the protocol:
; data stage is not tagged (unlike command_*_wrapper), so the only way to know
; what request the data are associated with is to guarantee that only one
; request is processing at the time.
.RequestsQueue          rd      2
.QueueLock              rd      3       ; protects .RequestsQueue
.InquiryData            inquiry_data    ; information about device
; data for the current request
.Command                command_block_wrapper
.DeviceDisconnected     db      ?
.Status                 command_status_wrapper
.Sense                  sense_data
.sizeof:
end virtual

; Information about one logical device.
virtual at 0
usb_unit_data:
.Parent         dd      ?       ; pointer to parent usb_device_data
.LUN            db      ?       ; index in usb_device_data.LogicalDevices array
.DiskIndex      db      ?       ; for name "usbhd<index>"
.MediaPresent   db      ?
                db      ?       ; alignment
.DiskDevice     dd      ?       ; handle of disk device or NULL
.SectorSize     dd      ?       ; sector size
; For some devices, the first request to the medium fails with 'unit not ready'.
; When the code sees this status, it retries the command several times.
; Two following variables track the retry count and total time for those;
; total time is currently used only for debug output.
.UnitReadyAttempts      dd      ?
.TimerTicks             dd      ?
.sizeof:
end virtual

; This is the structure for items in the queue usb_device_data.RequestsQueue.
virtual at 0
request_queue_item:
.Next           dd      ?       ; next item in the queue
.Prev           dd      ?       ; prev item in the queue
.ReqBuilder     dd      ?       ; procedure to fill command_block_wrapper
.Buffer         dd      ?       ; input or output data
                                ; (length is command_block_wrapper.Length)
.Callback       dd      ?       ; procedure to call in the end of transfer
.UserData       dd      ?       ; passed as-is to .Callback
; There are 3 possible stages of any request, one of them optional:
; command stage (host sends command_block_wrapper to device),
; optional data stage,
; status stage (device sends command_status_wrapper to host).
; Also, if a request fails, the code queues additional request
; SCSI_REQUEST_SENSE; sense_data from SCSI_REQUEST_SENSE
; contains some information about the error.
.Stage          db      ?
.sizeof:
end virtual

section '.flat' code readable align 16
; The start procedure.
proc START
virtual at esp
        dd      ?       ; return address
.reason dd      ?       ; DRV_ENTRY or DRV_EXIT
end virtual
; 1. Test whether the procedure is called with the argument DRV_ENTRY.
; If not, return 0.
        xor     eax, eax        ; initialize return value
        cmp     [.reason], 1    ; compare the argument
        jnz     .nothing
; 2. Initialize: we have one global mutex.
        mov     ecx, free_numbers_lock
        call    MutexInit
; 3. Register self as a USB driver.
; The name is my_driver = 'usbstor'; IOCTL interface is not supported;
; usb_functions is an offset of a structure with callback functions.
        stdcall RegUSBDriver, my_driver, 0, usb_functions
; 4. Return the returned value of RegUSBDriver.
.nothing:
        ret     4
endp

; Helper procedures to work with requests queue.

; Add a request to the queue. Stdcall with 5 arguments.
proc queue_request
        push    ebx esi
virtual at esp
                rd      2       ; saved registers
                dd      ?       ; return address
.device         dd      ?       ; pointer to usb_device_data
.ReqBuilder     dd      ?       ; request_queue_item.ReqBuilder
.Buffer         dd      ?       ; request_queue_item.Buffer
.Callback       dd      ?       ; request_queue_item.Callback
.UserData       dd      ?       ; request_queue_item.UserData
end virtual
; 1. Allocate the memory for the request description.
        movi    eax, request_queue_item.sizeof
        call    Kmalloc
        test    eax, eax
        jnz     @f
        mov     esi, nomemory
        call    SysMsgBoardStr
        pop     esi ebx
        ret     20
@@:
; 2. Fill user-provided parts of the request description.
        push    edi
        xchg    eax, ebx
        lea     esi, [.ReqBuilder+4]
        lea     edi, [ebx+request_queue_item.ReqBuilder]
        movsd   ; ReqBuilder
        movsd   ; Buffer
        movsd   ; Callback
        movsd   ; UserData
        pop     edi
; 3. Set stage to zero: not started.
        mov     [ebx+request_queue_item.Stage], 0
; 4. Lock the queue.
        mov     esi, [.device]
        lea     ecx, [esi+usb_device_data.QueueLock]
        call    MutexLock
; 5. Insert the request to the tail of the queue.
        add     esi, usb_device_data.RequestsQueue
        mov     edx, [esi+request_queue_item.Prev]
        mov     [ebx+request_queue_item.Next], esi
        mov     [ebx+request_queue_item.Prev], edx
        mov     [edx+request_queue_item.Next], ebx
        mov     [esi+request_queue_item.Prev], ebx
; 6. Test whether the queue was empty
; and the request should be started immediately.
        cmp     [esi+request_queue_item.Next], ebx
        jnz     .unlock
; 8. If the step 6 shows that the request is the first in the queue,
; start it.
        sub     esi, usb_device_data.RequestsQueue
        call    setup_request
        jmp     .nothing
.unlock:
        call    MutexUnlock
; 9. Return.
.nothing:
        pop     esi ebx
        ret     20
endp

; The current request is completed. Call the callback,
; remove the request from the queue, start the next
; request if there is one.
; esi points to usb_device_data
proc complete_request
; 1. Print common debug messages on fails.
if DEBUG
        cmp     [esi+usb_device_data.Status.Status], CSW_STATUS_FAIL
        jb      .normal
        jz      .fail
        DEBUGF 1, 'K : Fatal error during execution of command %x\n', [esi+usb_device_data.Command.Command]:2
        jmp     .normal
.fail:
        DEBUGF 1, 'K : Command %x failed\n', [esi+usb_device_data.Command.Command]:2
.normal:
end if
; 2. Get the current request.
        mov     ebx, [esi+usb_device_data.RequestsQueue+request_queue_item.Next]
; 3. Call the callback.
        stdcall [ebx+request_queue_item.Callback], esi, [ebx+request_queue_item.UserData]
; 4. Lock the queue.
        lea     ecx, [esi+usb_device_data.QueueLock]
        call    MutexLock
; 5. Remove the request.
        lea     edx, [esi+usb_device_data.RequestsQueue]
        mov     eax, [ebx+request_queue_item.Next]
        mov     [eax+request_queue_item.Prev], edx
        mov     [edx+request_queue_item.Next], eax
; 6. Free the request memory.
        push    eax edx
        xchg    eax, ebx
        call    Kfree
        pop     edx ebx
; 7. If there is a next request, start processing.
        cmp     ebx, edx
        jnz     setup_request
; 8. Unlock the queue and return.
        lea     ecx, [esi+usb_device_data.QueueLock]
        call    MutexUnlock
        ret
endp

; Start processing the request. Called either by queue_request
; or when the previous request has been processed.
; Do not call directly, use queue_request.
; Must be called when queue is locked; unlocks the queue when returns.
proc setup_request
        xor     eax, eax
; 1. If DeviceDisconnected has been run, then all handles of pipes
; are invalid, so we must fail immediately.
; (That is why this function needs the locked queue: this
; guarantee that either DeviceDisconnected has been already run, or
; DeviceDisconnected will not return before the queue is unlocked.)
        cmp     [esi+usb_device_data.DeviceDisconnected], al
        jnz     .fatal
; 2. If the previous command has encountered a fatal error,
; perform reset recovery.
        cmp     [esi+usb_device_data.Status.Status], CSW_STATUS_FATAL
        jb      .norecovery
; 2a. Send Bulk-Only Mass Storage Reset command to config pipe.
        lea     edx, [esi+usb_device_data.ConfigRequest]
        mov     word [edx], (REQUEST_BORESET shl 8) + 21h       ; class request
        mov     word [edx+6], ax        ; length = 0
        stdcall USBControlTransferAsync, [esi+usb_device_data.ConfigPipe], edx, eax, eax, recovery_callback1, esi, eax
; 2b. Fail here = fatal error.
        test    eax, eax
        jz      .fatal
; 2c. Otherwise, unlock the queue and return. recovery_callback1 will continue processing.
.unlock_return:
        lea     ecx, [esi+usb_device_data.QueueLock]
        call    MutexUnlock
        ret
.norecovery:
; 3. Send the command. Fail (no memory or device disconnected) = fatal error.
; Otherwise, go to 2c.
        call    request_stage1
        test    eax, eax
        jnz     .unlock_return
.fatal:
; 4. Fatal error. Set status = FATAL, unlock the queue, complete the request.
        mov     [esi+usb_device_data.Status.Status], CSW_STATUS_FATAL
        lea     ecx, [esi+usb_device_data.QueueLock]
        call    MutexUnlock
        jmp     complete_request
endp

; Initiate USB transfer for the first stage of a request (send command).
proc request_stage1
        mov     ebx, [esi+usb_device_data.RequestsQueue+request_queue_item.Next]
; 1. Set the stage to 1 = command stage.
        inc     [ebx+request_queue_item.Stage]
; 2. Generate the command. Zero-initialize and use the caller-provided proc.
        lea     edx, [esi+usb_device_data.Command]
        xor     eax, eax
        mov     [edx+command_block_wrapper.CommandLength], 12
        mov     dword [edx+command_block_wrapper.Command], eax
        mov     dword [edx+command_block_wrapper.Command+4], eax
        mov     dword [edx+command_block_wrapper.Command+8], eax
        mov     dword [edx+command_block_wrapper.Command+12], eax
        inc     [edx+command_block_wrapper.Tag]
        stdcall [ebx+request_queue_item.ReqBuilder], edx, [ebx+request_queue_item.UserData]
; 4. Initiate USB transfer.
        lea     edx, [esi+usb_device_data.Command]
if DUMP_PACKETS
        DEBUGF 1,'K : USBSTOR out:'
        mov     eax, edx
        mov     ecx, command_block_wrapper.sizeof
        call    debug_dump
        DEBUGF 1,'\n'
end if
        stdcall USBNormalTransferAsync, [esi+usb_device_data.OutPipe], edx, command_block_wrapper.sizeof, request_callback1, esi, 0
        ret
endp

if DUMP_PACKETS
proc debug_dump
        test    ecx, ecx
        jz      .done
.loop:
        test    ecx, 0Fh
        jnz     @f
        DEBUGF 1,'\nK :'
@@:
        DEBUGF 1,' %x',[eax]:2
        inc     eax
        dec     ecx
        jnz     .loop
.done:
        ret
endp
end if

; Called when the Reset command is completed,
; either successfully or not.
proc recovery_callback1
virtual at esp
                dd      ?       ; return address
.pipe           dd      ?
.status         dd      ?
.buffer         dd      ?
.length         dd      ?
.calldata       dd      ?
end virtual
        cmp     [.status], 0
        jnz     .error
; todo: reset pipes
        push    ebx esi
        mov     esi, [.calldata+8]
        call    request_stage1
        pop     esi ebx
        test    eax, eax
        jz      .error
        ret     20
.error:
        DEBUGF 1, 'K : error %d while resetting', [.status]
        jmp     request_callback1.common_error
endp

; Called when the first stage of request is completed,
; either successfully or not.
proc request_callback1
virtual at esp
                dd      ?       ; return address
.pipe           dd      ?
.status         dd      ?
.buffer         dd      ?
.length         dd      ?
.calldata       dd      ?
end virtual
; 1. Initialize.
        mov     ecx, [.calldata]
        mov     eax, [.status]
; 2. Test for error.
        test    eax, eax
        jnz     .error
; No error.
; 3. Increment the stage.
        mov     edx, [ecx+usb_device_data.RequestsQueue+request_queue_item.Next]
        inc     [edx+request_queue_item.Stage]
; 4. If there is no data, skip this stage.
        cmp     [ecx+usb_device_data.Command.Length], 0
        jz      ..request_get_status
; 5. Initiate USB transfer. If this fails, go to the error handler.
        mov     eax, [ecx+usb_device_data.InPipe]
        cmp     [ecx+usb_device_data.Command.Flags], 0
        js      @f
        mov     eax, [ecx+usb_device_data.OutPipe]
if DUMP_PACKETS
        DEBUGF 1,'K : USBSTOR out:'
        push    eax ecx
        mov     eax, [edx+request_queue_item.Buffer]
        mov     ecx, [ecx+usb_device_data.Command.Length]
        call    debug_dump
        pop     ecx eax
        DEBUGF 1,'\n'
end if
@@:
        stdcall USBNormalTransferAsync, eax, [edx+request_queue_item.Buffer], [ecx+usb_device_data.Command.Length], request_callback2, ecx, 0
        test    eax, eax
        jz      .error
; 6. Return.
        ret     20
.error:
; Error.
; 7. Print debug message and complete the request as failed.
        DEBUGF 1,'K : error %d after %d bytes in request stage\n',eax,[.length]
.common_error:
; TODO: add recovery after STALL
        mov     ecx, [.calldata]
        mov     [ecx+usb_device_data.Status.Status], CSW_STATUS_FATAL
        push    ebx esi
        mov     esi, ecx
        call    complete_request
        pop     esi ebx
        ret     20
endp

; Called when the second stage of request is completed,
; either successfully or not.
proc request_callback2
virtual at esp
                dd      ?       ; return address
.pipe           dd      ?
.status         dd      ?
.buffer         dd      ?
.length         dd      ?
.calldata       dd      ?
end virtual
if DUMP_PACKETS
        mov     eax, [.calldata]
        mov     eax, [eax+usb_device_data.InPipe]
        cmp     [.pipe], eax
        jnz     @f
        DEBUGF 1,'K : USBSTOR in:'
        push    eax ecx
        mov     eax, [.buffer+8]
        mov     ecx, [.length+8]
        call    debug_dump
        pop     ecx eax
        DEBUGF 1,'\n'
@@:
end if
; 1. Initialize.
        mov     ecx, [.calldata]
        mov     eax, [.status]
; 2. Test for error.
        test    eax, eax
        jnz     .error
; No error.
..request_get_status:
; 3. Increment the stage.
        mov     edx, [ecx+usb_device_data.RequestsQueue+request_queue_item.Next]
        inc     [edx+request_queue_item.Stage]
; 4. Initiate USB transfer. If this fails, go to the error handler.
        lea     edx, [ecx+usb_device_data.Status]
        stdcall USBNormalTransferAsync, [ecx+usb_device_data.InPipe], edx, command_status_wrapper.sizeof, request_callback3, ecx, 0
        test    eax, eax
        jz      .error
        ret     20
.error:
; Error.
; 7. Print debug message and complete the request as failed.
        DEBUGF 1,'K : error %d after %d bytes in data stage\n',eax,[.length]
        jmp     request_callback1.common_error
endp

; Called when the third stage of request is completed,
; either successfully or not.
proc request_callback3
virtual at esp
                dd      ?       ; return address
.pipe           dd      ?
.status         dd      ?
.buffer         dd      ?
.length         dd      ?
.calldata       dd      ?
end virtual
if DUMP_PACKETS
        DEBUGF 1,'K : USBSTOR in:'
        mov     eax, [.buffer]
        mov     ecx, [.length]
        call    debug_dump
        DEBUGF 1,'\n'
end if
; 1. Initialize.
        mov     eax, [.status]
; 2. Test for error.
        test    eax, eax
        jnz     .transfer_error
; Transfer is OK.
; 3. Validate the status. Invalid status = fatal error.
        push    ebx esi
        mov     esi, [.calldata+8]
        mov     ebx, [esi+usb_device_data.RequestsQueue+request_queue_item.Next]
        cmp     [esi+usb_device_data.Status.Signature], 'USBS'
        jnz     .invalid
        mov     eax, [esi+usb_device_data.Command.Tag]
        cmp     [esi+usb_device_data.Status.Tag], eax
        jnz     .invalid
        cmp     [esi+usb_device_data.Status.Status], CSW_STATUS_FATAL
        ja      .invalid
; 4. The status block is valid. Check the status code.
        jz      .complete
; 5. If this command was not REQUEST_SENSE, copy status data to safe place.
; Otherwise, the original command has failed, so restore the fail status.
        cmp     byte [esi+usb_device_data.Command.Command], SCSI_REQUEST_SENSE
        jz      .request_sense
        mov     eax, [esi+usb_device_data.Status.LengthRest]
        mov     [esi+usb_device_data.LengthRest], eax
        cmp     [esi+usb_device_data.Status.Status], CSW_STATUS_FAIL
        jz      .fail
.complete:
        call    complete_request
.nothing:
        pop     esi ebx
        ret     20
.request_sense:
        mov     [esi+usb_device_data.Status.Status], CSW_STATUS_FAIL
        jmp     .complete
.invalid:
; 6. Invalid status block. Say error, set status to fatal and complete request.
        push    esi
        mov     esi, invresponse
        call    SysMsgBoardStr
        pop     esi
        mov     [esi+usb_device_data.Status.Status], CSW_STATUS_FATAL
        jmp     .complete
.fail:
; 7. The command has failed. 
; If this command was not REQUEST_SENSE, schedule the REQUEST_SENSE command
; to determine the reason of fail. Otherwise, assume that there is no error data.
        cmp     [esi+usb_device_data.Command.Command], SCSI_REQUEST_SENSE
        jz      .fail_request_sense
        mov     [ebx+request_queue_item.ReqBuilder], request_sense_req
        lea     eax, [esi+usb_device_data.Sense]
        mov     [ebx+request_queue_item.Buffer], eax
        call    request_stage1
        test    eax, eax
        jnz     .nothing
.fail_request_sense:
        DEBUGF 1,'K : fail during REQUEST SENSE\n'
        mov     byte [esi+usb_device_data.Sense], 0
        jmp     .complete
.transfer_error:
; TODO: add recovery after STALL
        DEBUGF 1,'K : error %d after %d bytes in status stage\n',eax,[.length]
        jmp     request_callback1.common_error
endp

; Builder for SCSI_REQUEST_SENSE request.
; edx = first argument = pointer to usb_device_data.Command,
; second argument = custom data given to queue_request (ignored).
proc request_sense_req
        mov     [edx+command_block_wrapper.Length], sense_data.sizeof
        mov     [edx+command_block_wrapper.Flags], CBW_FLAG_IN
        mov     byte [edx+command_block_wrapper.Command+0], SCSI_REQUEST_SENSE
        mov     byte [edx+command_block_wrapper.Command+4], sense_data.sizeof
        ret     8
endp

; This procedure is called when new mass-storage device is detected.
; It initializes the device.
; Technically, initialization implies sending several USB queries,
; so it is split in several procedures. The first is AddDevice,
; other are callbacks which will be called at some time in the future,
; when the device will respond.
; The general scheme:
; * AddDevice parses descriptors, opens pipes; if everything is ok,
;   AddDevice sends REQUEST_GETMAXLUN with callback known_lun_callback;
; * known_lun_callback allocates memory for LogicalDevices and sends
;   SCSI_TEST_UNIT_READY to all logical devices with test_unit_ready_callback;
; * test_unit_ready_callback checks whether the unit is ready;
;   if not, it repeats the same request several times;
;   if ok or there were too many attempts, it sends SCSI_INQUIRY with
;   callback inquiry_callback;
; * inquiry_callback checks that a logical device is a block device
;   and the unit was ready; if so, it notifies the kernel about new disk device.
proc AddDevice
        push    ebx esi
virtual at esp
                rd      2       ; saved registers ebx, esi
                dd      ?       ; return address
.pipe0          dd      ?       ; handle of the config pipe
.config         dd      ?       ; pointer to config_descr
.interface      dd      ?       ; pointer to interface_descr
end virtual
; 1. Check device type. Currently only SCSI-command-set Bulk-only devices
; are supported.
; 1a. Get the subclass and the protocol. Since bInterfaceSubClass and
; bInterfaceProtocol are subsequent in interface_descr, just one
; memory reference is used for both.
        mov     esi, [.interface]
        xor     ebx, ebx
        mov     cx, word [esi+interface_descr.bInterfaceSubClass]
; 1b. For Mass-storage SCSI-command-set Bulk-only devices subclass must be 6
; and protocol must be 50h. Check.
        cmp     cx, 0x5006
        jz      .known
; There are devices with subclass 5 which use the same protocol 50h.
; The difference is not important for the code except for this test,
; so allow them to proceed also.
        cmp     cx, 0x5005
        jz      .known
; 1c. If the device is unknown, print a message and go to 11c.
        mov     esi, unkdevice
        call    SysMsgBoardStr
        jmp     .nothing
; 1d. If the device uses known command set, print a message and continue
; configuring.
.known:
        push    esi
        mov     esi, okdevice
        call    SysMsgBoardStr
        pop     esi
; 2. Allocate memory for internal device data.
; 2a. Call the kernel.
        mov     eax, usb_device_data.sizeof
        call    Kmalloc
; 2b. Check return value.
        test    eax, eax
        jnz     @f
; 2c. If failed, say a message and go to 11c.
        mov     esi, nomemory
        call    SysMsgBoardStr
        jmp     .nothing
@@:
; 2d. If succeeded, zero the contents and continue configuring.
        xchg    ebx, eax        ; ebx will point to usb_device_data
        xor     eax, eax
        mov     [ebx+usb_device_data.OutPipe], eax
        mov     [ebx+usb_device_data.InPipe], eax
        mov     [ebx+usb_device_data.MaxLUN], eax
        mov     [ebx+usb_device_data.LogicalDevices], eax
        mov     dword [ebx+usb_device_data.ConfigRequest], eax
        mov     dword [ebx+usb_device_data.ConfigRequest+4], eax
        mov     [ebx+usb_device_data.Status.Status], al
        mov     [ebx+usb_device_data.DeviceDisconnected], al
; 2e. There is one reference: a connected USB device.
        inc     eax
        mov     [ebx+usb_device_data.NumReferences], eax
; 2f. Save handle of configuration pipe for reset recovery.
        mov     eax, [.pipe0]
        mov     [ebx+usb_device_data.ConfigPipe], eax
; 2g. Save the interface number for configuration requests.
        mov     al, [esi+interface_descr.bInterfaceNumber]
        mov     [ebx+usb_device_data.ConfigRequest+4], al
; 2h. Initialize common fields in command wrapper.
        mov     [ebx+usb_device_data.Command.Signature], 'USBC'
        mov     [ebx+usb_device_data.Command.Tag], 'xxxx'
; 2i. Initialize requests queue.
        lea     eax, [ebx+usb_device_data.RequestsQueue]
        mov     [eax+request_queue_item.Next], eax
        mov     [eax+request_queue_item.Prev], eax
        lea     ecx, [ebx+usb_device_data.QueueLock]
        call    MutexInit
; Bulk-only mass storage devices use one OUT bulk endpoint for sending
; command/data and one IN bulk endpoint for receiving data/status.
; Look for those endpoints.
; 3. Get the upper bound of all descriptors' data.
        mov     edx, [.config]  ; configuration descriptor
        movzx   ecx, [edx+config_descr.wTotalLength]
        add     edx, ecx
; 4. Loop over all descriptors until
; either end-of-data reached - this is fail
; or interface descriptor found - this is fail, all further data
;    correspond to that interface
; or both endpoint descriptors found.
; 4a. Loop start: esi points to the interface descriptor,
.lookep:
; 4b. Get next descriptor.
        movzx   ecx, byte [esi] ; the first byte of all descriptors is length
        add     esi, ecx
; 4c. Check that at least two bytes are readable. The opposite is an error.
        inc     esi
        cmp     esi, edx
        jae     .errorep
        dec     esi
; 4d. Check that this descriptor is not interface descriptor. The opposite is
; an error.
        cmp     byte [esi+endpoint_descr.bDescriptorType], INTERFACE_DESCR_TYPE
        jz      .errorep
; 4e. Test whether this descriptor is an endpoint descriptor. If not, continue
; the loop.
        cmp     byte [esi+endpoint_descr.bDescriptorType], ENDPOINT_DESCR_TYPE
        jnz     .lookep
; 5. Check that the descriptor contains all required data and all data are
; readable. The opposite is an error.
        cmp     byte [esi+endpoint_descr.bLength], endpoint_descr.sizeof
        jb      .errorep
        lea     ecx, [esi+endpoint_descr.sizeof]
        cmp     ecx, edx
        ja      .errorep
; 6. Check that the endpoint is bulk endpoint. The opposite is an error.
        mov     cl, [esi+endpoint_descr.bmAttributes]
        and     cl, 3
        cmp     cl, BULK_PIPE
        jnz     .errorep
; 7. Get the direction of this endpoint.
        movzx   ecx, [esi+endpoint_descr.bEndpointAddress]
        shr     ecx, 7
; 8. Test whether a pipe for this direction is already opened. If so, continue
; the loop.
        cmp     [ebx+usb_device_data.OutPipe+ecx*4], 0
        jnz     .lookep
; 9. Open pipe for this endpoint.
; 9a. Save registers.
        push    ecx edx
; 9b. Load parameters from the descriptor.
        movzx   ecx, [esi+endpoint_descr.bEndpointAddress]
        movzx   edx, [esi+endpoint_descr.wMaxPacketSize]
        movzx   eax, [esi+endpoint_descr.bInterval]     ; not used for USB1, may be important for USB2
; 9c. Call the kernel.
        stdcall USBOpenPipe, [ebx+usb_device_data.ConfigPipe], ecx, edx, BULK_PIPE, eax
; 9d. Restore registers.
        pop     edx ecx
; 9e. Check result. If failed, go to 11b.
        test    eax, eax
        jz      .free
; 9f. Save result.
        mov     [ebx+usb_device_data.OutPipe+ecx*4], eax
; 10. Test whether the second pipe is already opened. If not, continue loop.
        xor     ecx, 1
        cmp     [ebx+usb_device_data.OutPipe+ecx*4], 0
        jz      .lookep
        jmp     .created
; 11. An error occured during processing endpoint descriptor.
.errorep:
; 11a. Print a message.
        DEBUGF 1,'K : error: invalid endpoint descriptor\n'
.free:
; 11b. Free the allocated usb_device_data.
        xchg    eax, ebx
        call    Kfree
.nothing:
; 11c. Return an error.
        xor     eax, eax
        jmp     .return
.created:
; 12. Pipes are opened. Send GetMaxLUN control request.
        lea     eax, [ebx+usb_device_data.ConfigRequest]
        mov     byte [eax], 0A1h        ; class request from interface
        mov     byte [eax+1], REQUEST_GETMAXLUN
        mov     byte [eax+6], 1         ; transfer 1 byte
        lea     ecx, [ebx+usb_device_data.MaxLUN]
if DUMP_PACKETS
        DEBUGF 1,'K : GETMAXLUN: %x %x %x %x %x %x %x %x\n',[eax]:2,[eax+1]:2,[eax+2]:2,[eax+3]:2,[eax+4]:2,[eax+5]:2,[eax+6]:2,[eax+7]:2
end if
        stdcall USBControlTransferAsync, [ebx+usb_device_data.ConfigPipe], eax, ecx, 1, known_lun_callback, ebx, 0
; 13. Return with pointer to device data as returned value.
        xchg    eax, ebx
.return:
        pop     esi ebx
        ret     12
endp

; This function is called when REQUEST_GETMAXLUN is done,
; either successful or unsuccessful.
proc known_lun_callback
        push    ebx esi
virtual at esp
                rd      2       ; saved registers
                dd      ?       ; return address
.pipe           dd      ?
.status         dd      ?
.buffer         dd      ?
.length         dd      ?
.calldata       dd      ?
end virtual
; 1. Check the status. If the request failed, assume that MaxLUN is zero.
        mov     ebx, [.calldata]
        mov     eax, [.status]
        test    eax, eax
        jz      @f
        DEBUGF 1, 'K : GETMAXLUN failed with status %d, assuming zero\n', eax
        mov     [ebx+usb_device_data.MaxLUN], 0
@@:
; 2. Allocate the memory for logical devices.
        mov     eax, [ebx+usb_device_data.MaxLUN]
        inc     eax
        DEBUGF 1,'K : %d logical unit(s)\n',eax
        imul    eax, usb_unit_data.sizeof
        push    ebx
        call    Kmalloc
        pop     ebx
; If failed, print a message and do nothing.
        test    eax, eax
        jnz     @f
        mov     esi, nomemory
        call    SysMsgBoardStr
        pop     esi ebx
        ret     20
@@:
        mov     [ebx+usb_device_data.LogicalDevices], eax
; 3. Initialize logical devices and initiate TEST_UNIT_READY request.
        xchg    esi, eax
        xor     ecx, ecx
.looplun:
        mov     [esi+usb_unit_data.Parent], ebx
        mov     [esi+usb_unit_data.LUN], cl
        xor     eax, eax
        mov     [esi+usb_unit_data.MediaPresent], al
        mov     [esi+usb_unit_data.DiskDevice], eax
        mov     [esi+usb_unit_data.SectorSize], eax
        mov     [esi+usb_unit_data.UnitReadyAttempts], eax
        push    ecx
        call    GetTimerTicks
        mov     [esi+usb_unit_data.TimerTicks], eax
        stdcall queue_request, ebx, test_unit_ready_req, 0, test_unit_ready_callback, esi
        pop     ecx
        inc     ecx
        add     esi, usb_unit_data.sizeof
        cmp     ecx, [ebx+usb_device_data.MaxLUN]
        jbe     .looplun
; 4. Return.
        pop     esi ebx
        ret     20
endp

; Builder for SCSI INQUIRY request.
; edx = first argument = pointer to usb_device_data.Command,
; second argument = custom data given to queue_request.
proc inquiry_req
        mov     eax, [esp+8]
        mov     al, [eax+usb_unit_data.LUN]
        mov     [edx+command_block_wrapper.Length], inquiry_data.sizeof
        mov     [edx+command_block_wrapper.Flags], CBW_FLAG_IN
        mov     [edx+command_block_wrapper.LUN], al
        mov     byte [edx+command_block_wrapper.Command+0], SCSI_INQUIRY
        mov     byte [edx+command_block_wrapper.Command+4], inquiry_data.sizeof
        ret     8
endp

; Called when SCSI INQUIRY request is completed.
proc inquiry_callback
; 1. Check the status.
        mov     ecx, [esp+4]
        cmp     [ecx+usb_device_data.Status.Status], CSW_STATUS_OK
        jnz     .fail
; 2. The command has completed successfully.
; Print a message showing device type, ignore anything but block devices.
        mov     al, [ecx+usb_device_data.InquiryData.PeripheralDevice]
        and     al, 1Fh
        DEBUGF 1,'K : peripheral device type is %x\n',al
        test    al, al
        jnz     .nothing
        DEBUGF 1,'K : direct-access mass storage device detected\n'
; 3. We have found a new disk device. Increment number of references.
        lock inc [ecx+usb_device_data.NumReferences]
; Unfortunately, we are now in the context of the USB thread,
; so we can't notify the kernel immediately: it would try to do something
; with a new disk, those actions would be synchronous and would require
; waiting for results of USB requests, but we need to exit this callback
; to allow the USB thread to continue working and handling those requests.
; 4. Thus, create a temporary kernel thread which would do it.
        mov     edx, [esp+8]
        push    ebx ecx esi edi
        movi    ebx, 1
        mov     ecx, new_disk_thread
        ; edx = parameter
        call    CreateThread
        pop     edi esi ecx ebx
        cmp     eax, -1
        jnz     .nothing
; on error, reverse step 3
        lock dec [ecx+usb_device_data.NumReferences]
.nothing:
        ret     8
.fail:
; 4. The command has failed. Print a message and do nothing.
        push    esi
        mov     esi, inquiry_fail
        call    SysMsgBoardStr
        pop     esi
        ret     8
endp

; Builder for SCSI TEST_UNIT_READY request.
; edx = first argument = pointer to usb_device_data.Command,
; second argument = custom data given to queue_request.
proc test_unit_ready_req
        mov     eax, [esp+8]
        mov     al, [eax+usb_unit_data.LUN]
        mov     [edx+command_block_wrapper.Length], 0
        mov     [edx+command_block_wrapper.Flags], CBW_FLAG_IN
        mov     [edx+command_block_wrapper.LUN], al
        ret     8
endp

; Called when SCSI TEST_UNIT_READY request is completed.
proc test_unit_ready_callback
virtual at esp
                dd      ?       ; return address
.device         dd      ?
.calldata       dd      ?
end virtual
; 1. Check the status.
        mov     ecx, [.device]
        mov     edx, [.calldata]
        cmp     [ecx+usb_device_data.Status.Status], CSW_STATUS_OK
        jnz     .fail
; 2. The command has completed successfully,
; possibly after some repetitions. Print a debug message showing
; number and time of those. Remember that media is ready and go to 4.
        DEBUGF 1,'K : media is ready\n'
        call    GetTimerTicks
        sub     eax, [edx+usb_unit_data.TimerTicks]
        DEBUGF 1,'K : %d attempts, %d ticks\n',[edx+usb_unit_data.UnitReadyAttempts],eax
        inc     [edx+usb_unit_data.MediaPresent]
        jmp     .inquiry
.fail:
; 3. The command has failed.
; Retry the same request up to 3 times with 10ms delay;
; if limit of retries is not reached, exit from the function.
; Otherwise, go to 4.
        inc     [edx+usb_unit_data.UnitReadyAttempts]
        cmp     [edx+usb_unit_data.UnitReadyAttempts], 3
        jz      @f
        push    ecx edx esi
        movi    esi, 10
        call    Sleep
        pop     esi edx ecx
        stdcall queue_request, ecx, test_unit_ready_req, 0, test_unit_ready_callback, edx
        ret     8
@@:
        DEBUGF 1,'K : media not ready\n'
.inquiry:
; 4. initiate INQUIRY request.
        lea     eax, [ecx+usb_device_data.InquiryData]
        stdcall queue_request, ecx, inquiry_req, eax, inquiry_callback, edx
        ret     8
endp

; Temporary thread for initial actions with a new disk device.
proc new_disk_thread
        sub     esp, 32
virtual at esp
.name   rb      32      ; device name
.param  dd      ?       ; contents of edx at the moment of int 0x40/eax=51
        dd      ?       ; stack segment
end virtual
; We are ready to notify the kernel about a new disk device.
        mov     esi, [.param]
; 1. Generate name.
; 1a. Find a free index.
        mov     ecx, free_numbers_lock
        call    MutexLock
        xor     eax, eax
@@:
        bsf     edx, [free_numbers+eax]
        jnz     @f
        add     eax, 4
        cmp     eax, 4*4
        jnz     @b
        call    MutexUnlock
        push    esi
        mov     esi, noindex
        call    SysMsgBoardStr
        pop     esi
        jmp     .drop_reference
@@:
; 1b. Mark the index as busy.
        btr     [free_numbers+eax], edx
        lea     eax, [eax*8+edx]
        push    eax
        call    MutexUnlock
        pop     eax
; 1c. Generate a name of the form "usbhd<index>" in the stack.
        mov     dword [esp], 'usbh'
        lea     edi, [esp+5]
        mov     byte [edi-1], 'd'
        push    eax
        push    -'0'
        movi    ecx, 10
@@:
        cdq
        div     ecx
        push    edx
        test    eax, eax
        jnz     @b
@@:
        pop     eax
        add     al, '0'
        stosb
        jnz     @b
        pop     ecx
        mov     edx, esp
; 3d. Store the index in usb_unit_data to free it later.
        mov     [esi+usb_unit_data.DiskIndex], cl
; 4. Notify the kernel about a new disk.
; 4a. Add a disk.
;       stdcall queue_request, ecx, read_capacity_req, eax, read_capacity_callback, eax
        stdcall DiskAdd, disk_functions, edx, esi, 0
        mov     ebx, eax
; 4b. If it failed, release the index and do nothing.
        test    eax, eax
        jz      .free_index
; 4c. Notify the kernel that a media is present.
        stdcall DiskMediaChanged, eax, 1
; 5. Lock the requests queue, check that device is not disconnected,
; store the disk handle, unlock the requests queue.
        mov     ecx, [esi+usb_unit_data.Parent]
        add     ecx, usb_device_data.QueueLock
        call    MutexLock
        cmp     byte [ecx+usb_device_data.DeviceDisconnected-usb_device_data.QueueLock], 0
        jnz     .disconnected
        mov     [esi+usb_unit_data.DiskDevice], ebx
        call    MutexUnlock
        jmp     .exit
.disconnected:
        call    MutexUnlock
        stdcall disk_close, ebx
        jmp     .exit
.free_index:
        mov     ecx, free_numbers_lock
        call    MutexLock
        movzx   eax, [esi+usb_unit_data.DiskIndex]
        bts     [free_numbers], eax
        call    MutexUnlock
.drop_reference:
        mov     esi, [esi+usb_unit_data.Parent]
        lock dec [esi+usb_device_data.NumReferences]
        jnz     .exit
        mov     eax, [esi+usb_device_data.LogicalDevices]
        call    Kfree
        xchg    eax, esi
        call    Kfree
.exit:
        or      eax, -1
        int     0x40
endp

; This function is called when the device is disconnected.
proc DeviceDisconnected
        push    ebx esi
virtual at esp
        rd      2       ; saved registers
        dd      ?       ; return address
.device dd      ?
end virtual
; 1. Say a message.
        mov     esi, disconnectmsg
        call    SysMsgBoardStr
; 2. Lock the requests queue, set .DeviceDisconnected to 1,
; unlock the requests queue.
; Locking is required for synchronization with queue_request:
; all USB callbacks are executed in the same thread and are
; synchronized automatically, but queue_request can be running
; from any thread which wants to do something with a filesystem.
; Without locking, it would be possible that queue_request has
; been started, has checked that device is not yet disconnected,
; then DeviceDisconnected completes and all handles become invalid,
; then queue_request tries to use them.
        mov     esi, [.device]
        lea     ecx, [esi+usb_device_data.QueueLock]
        call    MutexLock
        mov     [esi+usb_device_data.DeviceDisconnected], 1
        call    MutexUnlock
; 3. Drop one reference to the structure and check whether
; that was the last reference.
        lock dec [esi+usb_device_data.NumReferences]
        jz      .free
; 4. If not, there are some additional references due to disk devices;
; notify the kernel that those disks are deleted.
; Note that new disks cannot be added while we are looping here,
; because new_disk_thread checks for .DeviceDisconnected.
        mov     ebx, [esi+usb_device_data.MaxLUN]
        mov     esi, [esi+usb_device_data.LogicalDevices]
        inc     ebx
.diskdel:
        mov     eax, [esi+usb_unit_data.DiskDevice]
        test    eax, eax
        jz      @f
        stdcall DiskDel, eax
@@:
        add     esi, usb_unit_data.sizeof
        dec     ebx
        jnz     .diskdel
; In this case, some operations with those disks are still possible,
; so we can't do anything more now. disk_close will take care of the rest.
.return:
        pop     esi ebx
        ret     4
; 5. If there are no disk devices, free all resources which were allocated.
.free:
        mov     eax, [esi+usb_device_data.LogicalDevices]
        test    eax, eax
        jz      @f
        call    Kfree
@@:
        xchg    eax, esi
        call    Kfree
        jmp     .return
endp

; Disk functions.
DISK_STATUS_OK              = 0 ; success
DISK_STATUS_GENERAL_ERROR   = -1; if no other code is suitable
DISK_STATUS_INVALID_CALL    = 1 ; invalid input parameters
DISK_STATUS_NO_MEDIA        = 2 ; no media present
DISK_STATUS_END_OF_MEDIA    = 3 ; end of media while reading/writing data

; Called when all operations with the given disk are done.
proc disk_close
        push    ebx esi
virtual at esp
        rd      2       ; saved registers
        dd      ?       ; return address
.userdata       dd      ?
end virtual
        mov     esi, [.userdata]
        mov     ecx, free_numbers_lock
        call    MutexLock
        movzx   eax, [esi+usb_unit_data.DiskIndex]
        bts     [free_numbers], eax
        call    MutexUnlock
        mov     esi, [esi+usb_unit_data.Parent]
        lock dec [esi+usb_device_data.NumReferences]
        jnz     .nothing
        mov     eax, [esi+usb_device_data.LogicalDevices]
        call    Kfree
        xchg    eax, esi
        call    Kfree
.nothing:
        pop     esi ebx
        ret     4
endp

; Returns sector size, capacity and flags of the media.
proc disk_querymedia stdcall uses ebx esi edi, \
        userdata:dword, mediainfo:dword
; 1. Create event for waiting.
        xor     esi, esi
        xor     ecx, ecx
        call    CreateEvent
        test    eax, eax
        jz      .generic_fail
        push    eax
        push    edx
        push    ecx
        push    0
        push    0
virtual at ebp-.localsize
.locals:
; two following dwords are the output of READ_CAPACITY
.LastLBABE      dd      ?
.SectorSizeBE   dd      ?
.Status         dd      ?
; two following dwords identify an event
.event_code     dd      ?
.event          dd      ?
                rd      3       ; saved registers
.localsize = $ - .locals
                dd      ?       ; saved ebp
                dd      ?       ; return address
.userdata       dd      ?
.mediainfo      dd      ?
end virtual
; 2. Initiate SCSI READ_CAPACITY request.
        mov     eax, [userdata]
        mov     ecx, [eax+usb_unit_data.Parent]
        mov     edx, esp
        stdcall queue_request, ecx, read_capacity_req, edx, read_capacity_callback, edx
; 3. Wait for event. This destroys it.
        mov     eax, [.event]
        mov     ebx, [.event_code]
        call    WaitEvent
; 4. Get the status and results.
        pop     ecx
        bswap   ecx     ; .LastLBA
        pop     edx
        bswap   edx     ; .SectorSize
        pop     eax     ; .Status
; 5. If the request has completed successfully, store results.
        test    eax, eax
        jnz     @f
        DEBUGF 1,'K : sector size is %d, last sector is %d\n',edx,ecx
        mov     ebx, [mediainfo]
        mov     [ebx], eax      ; flags = 0
        mov     [ebx+4], edx    ; sectorsize
        add     ecx, 1
        adc     eax, 0
        mov     [ebx+8], ecx
        mov     [ebx+12], eax   ; capacity
        mov     eax, [userdata]
        mov     [eax+usb_unit_data.SectorSize], edx
        xor     eax, eax
@@:
; 6. Restore the stack and return.
        pop     ecx
        pop     ecx
        ret
.generic_fail:
        or      eax, -1
        ret
endp

; Builder for SCSI READ_CAPACITY request.
; edx = first argument = pointer to usb_device_data.Command,
; second argument = custom data given to queue_request,
; pointer to disk_querymedia.locals.
proc read_capacity_req
        mov     eax, [esp+8]
        mov     eax, [eax+disk_querymedia.userdata-disk_querymedia.locals]
        mov     al, [eax+usb_unit_data.LUN]
        mov     [edx+command_block_wrapper.Length], 8
        mov     [edx+command_block_wrapper.Flags], CBW_FLAG_IN
        mov     [edx+command_block_wrapper.LUN], al
        mov     byte [edx+command_block_wrapper.Command+0], SCSI_READ_CAPACITY
        ret     8
endp

; Called when SCSI READ_CAPACITY request is completed.
proc read_capacity_callback
; Transform the status to return value of disk_querymedia
; and set the event.
        mov     ecx, [esp+4]
        xor     eax, eax
        cmp     [ecx+usb_device_data.Status.Status], al
        jz      @f
        or      eax, -1
@@:
        mov     ecx, [esp+8]
        mov     [ecx+disk_querymedia.Status-disk_querymedia.locals], eax
        push    ebx esi edi
        mov     eax, [ecx+disk_querymedia.event-disk_querymedia.locals]
        mov     ebx, [ecx+disk_querymedia.event_code-disk_querymedia.locals]
        xor     edx, edx
        xor     esi, esi
        call    RaiseEvent
        pop     edi esi ebx
        ret     8
endp

disk_write:
        mov     al, SCSI_WRITE10
        jmp     disk_read_write

disk_read:
        mov     al, SCSI_READ10

; Reads from the device or writes to the device.
proc disk_read_write stdcall uses ebx esi edi, \
        userdata:dword, buffer:dword, startsector:qword, numsectors:dword
; 1. Initialize.
        push    eax     ; .command
        mov     eax, [userdata]
        mov     eax, [eax+usb_unit_data.SectorSize]
        push    eax     ; .SectorSize
        push    0       ; .processed
        mov     eax, [numsectors]
        mov     eax, [eax]
; 2. The transfer length for SCSI_{READ,WRITE}10 commands can not be greater
; than 0xFFFF, so split the request to slices with <= 0xFFFF sectors.
max_sectors_at_time = 0xFFFF
.split:
        push    eax     ; .length_rest
        cmp     eax, max_sectors_at_time
        jb      @f
        mov     eax, max_sectors_at_time
@@:
        sub     [esp], eax
        push    eax     ; .length_cur
; 3. startsector must fit in 32 bits, otherwise abort the request.
        cmp     dword [startsector+4], 0
        jnz     .generic_fail
; 4. Create event for waiting.
        xor     esi, esi
        xor     ecx, ecx
        call    CreateEvent
        test    eax, eax
        jz      .generic_fail
        push    eax     ; .event
        push    edx     ; .event_code
        push    ecx     ; .status
virtual at ebp-.localsize
.locals:
.status         dd      ?
.event_code     dd      ?
.event          dd      ?
.length_cur     dd      ?
.length_rest    dd      ?
.processed      dd      ?
.SectorSize     dd      ?
.command        db      ?
                rb      3
                rd      3       ; saved registers
.localsize = $ - .locals
                dd      ?       ; saved ebp
                dd      ?       ; return address
.userdata       dd      ?
.buffer         dd      ?
.startsector    dq      ?
.numsectors     dd      ?
end virtual
; 5. Initiate SCSI READ10 or WRITE10 request.
        mov     eax, [userdata]
        mov     ecx, [eax+usb_unit_data.Parent]
        stdcall queue_request, ecx, read_write_req, [buffer], read_write_callback, esp
; 6. Wait for event. This destroys it.
        mov     eax, [.event]
        mov     ebx, [.event_code]
        call    WaitEvent
; 7. Get the status. If the operation has failed, abort.
        pop     eax     ; .status
        pop     ecx ecx ; cleanup .event_code, .event
        pop     ecx     ; .length_cur
        test    eax, eax
        jnz     .return
; 8. Otherwise, continue the loop started at step 2.
        add     dword [startsector], ecx
        adc     dword [startsector+4], eax
        imul    ecx, [.SectorSize]
        add     [buffer], ecx
        pop     eax
        test    eax, eax
        jnz     .split
        push    eax
.return:
; 9. Restore the stack, store .processed to [numsectors], return.
        pop     ecx     ; .length_rest
        pop     ecx     ; .processed
        mov     edx, [numsectors]
        mov     [edx], ecx
        pop     ecx     ; .SectorSize
        pop     ecx     ; .command
        ret
.generic_fail:
        or      eax, -1
        pop     ecx     ; .length_cur
        jmp     .return
endp

; Builder for SCSI READ10 or WRITE10 request.
; edx = first argument = pointer to usb_device_data.Command,
; second argument = custom data given to queue_request,
; pointer to disk_read_write.locals.
proc read_write_req
        mov     eax, [esp+8]
        mov     ecx, [eax+disk_read_write.userdata-disk_read_write.locals]
        mov     cl, [ecx+usb_unit_data.LUN]
        mov     [edx+command_block_wrapper.LUN], cl
        mov     ecx, [eax+disk_read_write.length_cur-disk_read_write.locals]
        imul    ecx, [eax+disk_read_write.SectorSize-disk_read_write.locals]
        mov     [edx+command_block_wrapper.Length], ecx
        mov     cl, [eax+disk_read_write.command-disk_read_write.locals]
        mov     [edx+command_block_wrapper.Flags], CBW_FLAG_OUT
        cmp     cl, SCSI_READ10
        jnz     @f
        mov     [edx+command_block_wrapper.Flags], CBW_FLAG_IN
@@:
        mov     byte [edx+command_block_wrapper.Command], cl
        mov     ecx, dword [eax+disk_read_write.startsector-disk_read_write.locals]
        bswap   ecx
        mov     dword [edx+command_block_wrapper.Command+2], ecx
        mov     ecx, [eax+disk_read_write.length_cur-disk_read_write.locals]
        xchg    cl, ch
        mov     word [edx+command_block_wrapper.Command+7], cx
        ret     8
endp

; Called when SCSI READ10 or WRITE10 request is completed.
proc read_write_callback
; 1. Initialize.
        push    ebx esi edi
virtual at esp
        rd      3       ; saved registers
        dd      ?       ; return address
.device         dd      ?
.calldata       dd      ?
end virtual
        mov     ecx, [.device]
        mov     esi, [.calldata]
; 2. Get the number of sectors which were read.
; If the status is OK or FAIL, the field .LengthRest is valid.
; Otherwise, it is invalid, so assume zero sectors.
        xor     eax, eax
        cmp     [ecx+usb_device_data.Status.Status], CSW_STATUS_FAIL
        ja      .sectors_calculated
        mov     eax, [ecx+usb_device_data.LengthRest]
        xor     edx, edx
        div     [esi+disk_read_write.SectorSize-disk_read_write.locals]
        test    edx, edx
        jz      @f
        inc     eax
@@:
        mov     edx, eax
        mov     eax, [esi+disk_read_write.length_cur-disk_read_write.locals]
        sub     eax, edx
        jae     .sectors_calculated
        xor     eax, eax
.sectors_calculated:
; 3. Increase the total number of processed sectors.
        add     [esi+disk_read_write.processed-disk_read_write.locals], eax
; 4. Set status to OK if all sectors were read, to ERROR otherwise.
        cmp     eax, [esi+disk_read_write.length_cur-disk_read_write.locals]
        setz    al
        movzx   eax, al
        dec     eax
        mov     [esi+disk_read_write.status-disk_read_write.locals], eax
; 5. Set the event.
        mov     eax, [esi+disk_read_write.event-disk_read_write.locals]
        mov     ebx, [esi+disk_read_write.event_code-disk_read_write.locals]
        xor     edx, edx
        xor     esi, esi
        call    RaiseEvent
; 6. Return.
        pop     edi esi ebx
        ret     8
endp

; strings
my_driver       db      'usbstor',0
disconnectmsg   db      'K : USB mass storage device disconnected',13,10,0
nomemory        db      'K : no memory',13,10,0
unkdevice       db      'K : unknown mass storage device',13,10,0
okdevice        db      'K : USB mass storage device detected',13,10,0
transfererror   db      'K : USB transfer error, disabling mass storage',13,10,0
invresponse     db      'K : invalid response from mass storage device',13,10,0
fatalerr        db      'K : mass storage device reports fatal error',13,10,0
inquiry_fail    db      'K : INQUIRY command failed',13,10,0
;read_capacity_fail db  'K : READ CAPACITY command failed',13,10,0
;read_fail      db      'K : READ command failed',13,10,0
noindex         db      'K : failed to generate disk name',13,10,0

; Exported variable: kernel API version.
align 4
version dd      50005h
; Structure with callback functions.
usb_functions:
        dd      usb_functions_end - usb_functions
        dd      AddDevice
        dd      DeviceDisconnected
usb_functions_end:

disk_functions:
        dd      disk_functions_end - disk_functions
        dd      disk_close
        dd      0       ; closemedia
        dd      disk_querymedia
        dd      disk_read
        dd      disk_write
        dd      0       ; flush
        dd      0       ; adjust_cache_size: use default cache
disk_functions_end:

free_numbers_lock       rd      3
; 128 devices should be enough for everybody
free_numbers    dd      -1, -1, -1, -1

; for DEBUGF macro
include_debug_strings

; for uninitialized data
section '.data' data readable writable align 16
