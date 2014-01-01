; Constants and structures that are shared between different parts of
; USB subsystem and *HCI drivers.

; =============================================================================
; ================================= Constants =================================
; =============================================================================
; Version of all structures related to host controllers.
; Must be the same in kernel and *hci-drivers.
USBHC_VERSION = 1

; USB device must have at least 100ms of stable power before initializing can
; proceed; one timer tick is 10ms, so enforce delay in 10 ticks
USB_CONNECT_DELAY = 10
; USB requires at least 10 ms for reset signalling. Normally, this is one timer
; tick. However, it is possible that we start reset signalling in the end of
; interval between timer ticks and then we test time in the start of the next
; interval; in this case, the delta between [timer_ticks] is 1, but the real
; time passed is significantly less than 10 ms. To avoid this, we add an extra
; tick; this guarantees that at least 10 ms have passed.
USB_RESET_TIME = 2
; USB requires at least 10 ms of reset recovery, a delay between reset
; signalling and any commands to device. Add an extra tick for the same reasons
; as with the previous constant.
USB_RESET_RECOVERY_TIME = 2

; USB pipe types
CONTROL_PIPE = 0
ISOCHRONOUS_PIPE = 1
BULK_PIPE = 2
INTERRUPT_PIPE = 3

; Status codes for transfer callbacks.
; Taken from OHCI as most verbose controller in this sense.
USB_STATUS_OK           = 0     ; no error
USB_STATUS_CRC          = 1     ; CRC error
USB_STATUS_BITSTUFF     = 2     ; bit stuffing violation
USB_STATUS_TOGGLE       = 3     ; data toggle mismatch
USB_STATUS_STALL        = 4     ; device returned STALL
USB_STATUS_NORESPONSE   = 5     ; device not responding
USB_STATUS_PIDCHECK     = 6     ; invalid PID check bits
USB_STATUS_WRONGPID     = 7     ; unexpected PID value
USB_STATUS_OVERRUN      = 8     ; too many data from endpoint
USB_STATUS_UNDERRUN     = 9     ; too few data from endpoint
USB_STATUS_BUFOVERRUN   = 12    ; overflow of internal controller buffer
USB_STATUS_BUFUNDERRUN  = 13    ; underflow of internal controller buffer
USB_STATUS_CLOSED       = 16    ; pipe closed
                                ; either explicitly with USBClosePipe
                                ; or implicitly due to device disconnect

; Possible speeds of USB devices
USB_SPEED_FS = 0 ; full-speed
USB_SPEED_LS = 1 ; low-speed
USB_SPEED_HS = 2 ; high-speed

; flags for usb_pipe.Flags
USB_FLAG_CLOSED     = 1         ; pipe is closed, no new transfers
; pipe is closed, return error instead of submitting any new transfer
USB_FLAG_CAN_FREE   = 2
; pipe is closed via explicit call to USBClosePipe, so it can be freed without
; any driver notification; if this flag is not set, then the pipe is closed due
; to device disconnect, so it must remain valid until return from disconnect
; callback provided by the driver
USB_FLAG_EXTRA_WAIT = 4
; The pipe was in wait list, while another event occured;
; when the first wait will be done, reinsert the pipe to wait list
USB_FLAG_CLOSED_BIT = 0 ; USB_FLAG_CLOSED = 1 shl USB_FLAG_CLOSED_BIT

; =============================================================================
; ================================ Structures =================================
; =============================================================================

; Description of controller-specific data and functions.
struct usb_hardware_func
Version         dd      ?       ; must be USBHC_VERSION
ID              dd      ?       ; '*HCI'
DataSize        dd      ?       ; sizeof(*hci_controller)
BeforeInit      dd      ?
; Early initialization: take ownership from BIOS.
; in: [ebp-4] = (bus shl 8) + devfn
Init            dd      ?
; Initialize controller-specific part of controller data.
; in: eax -> *hci_controller to initialize, [ebp-4] = (bus shl 8) + devfn
; out: eax = 0 <=> failed, otherwise eax -> usb_controller
ProcessDeferred dd      ?
; Called regularly from the main loop of USB thread
; (either due to timeout from a previous call, or due to explicit wakeup).
; in: esi -> usb_controller
; out: eax = maximum timeout for next call (-1 = infinity)
SetDeviceAddress        dd      ?
; in: esi -> usb_controller, ebx -> usb_pipe, cl = address
GetDeviceAddress        dd      ?
; in: esi -> usb_controller, ebx -> usb_pipe
; out: eax = address
PortDisable             dd      ?
; Disable the given port in the root hub.
; in: esi -> usb_controller, ecx = port (zero-based)
InitiateReset           dd      ?
; Start reset signalling on the given port.
; in: esi -> usb_controller, ecx = port (zero-based)
SetEndpointPacketSize   dd      ?
; in: esi -> usb_controller, ebx -> usb_pipe, ecx = packet size
AllocPipe               dd      ?
; out: eax = pointer to allocated usb_pipe
FreePipe                dd      ?
; void stdcall with one argument = pointer to previously allocated usb_pipe
InitPipe                dd      ?
; in: edi -> usb_pipe for target, ecx -> usb_pipe for config pipe,
; esi -> usb_controller, eax -> usb_gtd for the first TD,
; [ebp+12] = endpoint, [ebp+16] = maxpacket, [ebp+20] = type
UnlinkPipe              dd      ?
; esi -> usb_controller, ebx -> usb_pipe
AllocTD                 dd      ?
; out: eax = pointer to allocated usb_gtd
FreeTD                  dd      ?
; void stdcall with one argument = pointer to previously allocated usb_gtd
AllocTransfer           dd      ?
; Allocate and initialize one stage of a transfer.
; ebx -> usb_pipe, other parameters are passed through the stack:
; buffer,size = data to transfer
; flags = same as in usb_open_pipe:
;   bit 0 = allow short transfer, other bits reserved
; td = pointer to the current end-of-queue descriptor
; direction =
;   0000b for normal transfers,
;   1000b for control SETUP transfer,
;   1101b for control OUT transfer,
;   1110b for control IN transfer
; returns eax = pointer to the new end-of-queue descriptor
; (not included in the queue itself) or 0 on error
InsertTransfer          dd      ?
; Activate previously initialized transfer (maybe with multiple stages).
; esi -> usb_controller, ebx -> usb_pipe,
; [esp+4] -> first usb_gtd for the transfer,
; ecx -> last descriptor for the transfer
NewDevice               dd      ?
; Initiate configuration of a new device (create pseudo-pipe describing that
; device and call usb_new_device).
; esi -> usb_controller, eax = speed (one of USB_SPEED_* constants).
ends

; pointers to kernel API functions that are called from *HCI-drivers
struct usbhc_func
usb_process_gtd                 dd      ?
usb_init_static_endpoint        dd      ?
usb_wakeup_if_needed            dd      ?
usb_subscribe_control           dd      ?
usb_subscription_done           dd      ?
usb_allocate_common             dd      ?
usb_free_common                 dd      ?
usb_td_to_virt                  dd      ?
usb_init_transfer               dd      ?
usb_undo_tds                    dd      ?
usb_test_pending_port           dd      ?
usb_get_tt                      dd      ?
usb_get_tt_think_time           dd      ?
usb_new_device                  dd      ?
usb_disconnect_stage2           dd      ?
usb_process_wait_lists          dd      ?
usb_unlink_td                   dd      ?
usb_is_final_packet             dd      ?
usb_find_ehci_companion         dd      ?
ends

; Controller descriptor.
; This structure represents the common (controller-independent) part
; of a controller for the USB code. The corresponding controller-dependent
; part *hci_controller is located immediately before usb_controller.
struct usb_controller
; Two following fields organize all controllers in the global linked list.
Next            dd      ?
Prev            dd      ?
HardwareFunc    dd      ?
; Pointer to usb_hardware_func structure with controller-specific functions.
NumPorts        dd      ?
; Number of ports in the root hub.
PCICoordinates  dd      ?
; Device:function and bus number from PCI.
;
; The hardware is allowed to cache some data from hardware structures.
; Regular operations are designed considering this,
; but sometimes it is required to wait for synchronization of hardware cache
; with modified structures in memory.
; The code keeps two queues of pipes waiting for synchronization,
; one for asynchronous (bulk/control) pipes, one for periodic pipes, hardware
; cache is invalidated under different conditions for those types.
; Both queues are organized in the same way, as single-linked lists.
; There are three special positions: the head of list (new pipes are added
; here), the first pipe to be synchronized at the current iteration,
; the tail of list (all pipes starting from here are synchronized).
WaitPipeListAsync       dd      ?
WaitPipeListPeriodic    dd      ?
; List heads.
WaitPipeRequestAsync    dd      ?
WaitPipeRequestPeriodic dd      ?
; Pending request to hardware to refresh cache for items from WaitPipeList*.
; (Pointers to some items in WaitPipeList* or NULLs).
ReadyPipeHeadAsync      dd      ?
ReadyPipeHeadPeriodic   dd      ?
; Items of RemovingList* which were released by hardware and are ready
; for further processing.
; (Pointers to some items in WaitPipeList* or NULLs).
NewConnected    dd      ?
; bit mask of recently connected ports of the root hub,
; bit set = a device was recently connected to the corresponding port;
; after USB_CONNECT_DELAY ticks of stable status these ports are moved to
; PendingPorts
NewDisconnected dd      ?
; bit mask of disconnected ports of the root hub,
; bit set = a device in the corresponding port was disconnected,
; disconnect processing is required.
PendingPorts    dd      ?
; bit mask of ports which are ready to be initialized
ControlLock     MUTEX   ?
; mutex which guards all operations with control queue
BulkLock        MUTEX   ?
; mutex which guards all operations with bulk queue
PeriodicLock    MUTEX   ?
; mutex which guards all operations with periodic queues
WaitSpinlock:
; spinlock guarding WaitPipeRequest/ReadyPipeHead (but not WaitPipeList)
StartWaitFrame  dd      ?
; USB frame number when WaitPipeRequest* was registered.
ResettingHub    dd      ?
; Pointer to usb_hub responsible for the currently resetting port, if any.
; NULL for the root hub.
ResettingPort   db      ?
; Port that is currently resetting, 0-based.
ResettingSpeed  db      ?
; Speed of currently resetting device.
ResettingStatus db      ?
; Status of port reset. 0 = no port is resetting, -1 = reset failed,
; 1 = reset in progress, 2 = reset recovery in progress.
                rb      1       ; alignment
ResetTime       dd      ?
; Time when reset signalling or reset recovery has been started.
SetAddressBuffer        rb      8
; Buffer for USB control command SET_ADDRESS.
ExistingAddresses       rd      128/32
; Bitmask for 128 bits; bit i is cleared <=> address i is free for allocating
; for new devices. Bit 0 is always set.
ConnectedTime   rd      16
; Time, in timer ticks, when the port i has signalled the connect event.
; Valid only if bit i in NewConnected is set.
DevicesByPort   rd      16
; Pointer to usb_pipe for zero endpoint (which serves as device handle)
; for each port.
ends

; Pipe descriptor.
; * An USB pipe is described by two structures, for hardware and for software.
; * This is the software part. The hardware part is defined in a driver
;   of the corresponding controller.
; * The hardware part is located immediately before usb_pipe,
;   both are allocated at once by controller-specific code
;   (it knows the total length, which depends on the hardware part).
struct usb_pipe
Controller      dd      ?
; Pointer to usb_controller structure corresponding to this pipe.
; Must be the first dword after hardware part, see *hci_new_device.
;
; Every endpoint is included into one of processing lists:
; * Bulk list contains all Bulk endpoints.
; * Control list contains all Control endpoints.
; * Several Periodic lists serve Interrupt endpoints with different interval.
;   - There are N=2^n "leaf" periodic lists for N ms interval, one is processed
;     in the frames 0,N,2N,..., another is processed in the frames
;     1,1+N,1+2N,... and so on. The hardware starts processing of periodic
;     endpoints in every frame from the list identified by lower n bits of the
;     frame number; the addresses of these N lists are written to the
;     controller data area during the initialization.
;   - We assume that n=5, N=32 to simplify the code and compact the data.
;     OHCI works in this way. UHCI and EHCI actually have n=10, N=1024,
;     but this is an overkill for interrupt endpoints; the large value of N is
;     useful only for isochronous transfers in UHCI and EHCI. UHCI/EHCI code
;     initializes "leaf" lists k,k+32,k+64,...,k+(1024-32) to the same value,
;     giving essentially N=32.
;     This restriction means that the actual maximum interval of polling any
;     interrupt endpoint is 32ms, which seems to be a reasonable value.
;   - Similarly, there are 16 lists for 16-ms interval, 8 lists for 8-ms
;     interval and so on. Finally, there is one list for 1ms interval. Their
;     addresses are not directly known to the controller.
;   - The hardware serves endpoints following a physical link from the hardware
;     part.
;   - The hardware links are organized as follows. If the list item is not the
;     last, it's hardware link points to the next item. The hardware link of
;     the last item points to the first item of the "next" list.
;   - The "next" list for k-th and (k+M)-th periodic lists for interval 2M ms
;     is the k-th periodic list for interval M ms, M >= 1. In this scheme,
;     if two "previous" lists are served in the frames k,k+2M,k+4M,...
;     and k+M,k+3M,k+5M,... correspondingly, the "next" list is served in
;     the frames k,k+M,k+2M,k+3M,k+4M,k+5M,..., which is exactly what we want.
;   - The links between Periodic, Control, Bulk lists and the processing of
;     Isochronous endpoints are controller-specific.
; * The head of every processing list is a static entry which does not
;   correspond to any real pipe. It is described by usb_static_ep
;   structure, not usb_pipe. For OHCI and UHCI, sizeof.usb_static_ep plus
;   sizeof hardware part is 20h, the total number of lists is
;   32+16+8+4+2+1+1+1 = 65, so all these structures fit in one page,
;   leaving space for other data. This is another reason for 32ms limit.
; * Static endpoint descriptors are kept in *hci_controller structure.
; * All items in every processing list, including the static head, are
;   organized in a double-linked list using .NextVirt and .PrevVirt fields.
; * [[item.NextVirt].PrevVirt] = [[item.PrevVirt].NextVirt] for all items.
NextVirt        dd      ?
; Next endpoint in the processing list.
; See also PrevVirt field and the description before NextVirt field.
PrevVirt        dd      ?
; Previous endpoint in the processing list.
; See also NextVirt field and the description before NextVirt field.
;
; Every pipe has the associated transfer queue, that is, the double-linked
; list of Transfer Descriptors aka TD. For Control, Bulk and Interrupt
; endpoints this list consists of usb_gtd structures
; (GTD = General Transfer Descriptors), for Isochronous endpoints
; this list consists of usb_itd structures, which are not developed yet.
; The pipe needs to know only the last TD; the first TD can be
; obtained as [[pipe.LastTD].NextVirt].
LastTD          dd      ?
; Last TD in the transfer queue.
;
; All opened pipes corresponding to the same physical device are organized in
; the double-linked list using .NextSibling and .PrevSibling fields.
; The head of this list is kept in usb_device_data structure (OpenedPipeList).
; This list is used when the device is disconnected and all pipes for the
; device should be closed.
; Also, all pipes closed due to disconnect must remain valid at least until
; driver-provided disconnect function returns; all should-be-freed-but-not-now
; pipes for one device are organized in another double-linked list with
; the head in usb_device_data.ClosedPipeList; this list uses the same link
; fields, one pipe can never be in both lists.
NextSibling     dd      ?
; Next pipe for the physical device.
PrevSibling     dd      ?
; Previous pipe for the physical device.
;
; When hardware part of pipe is changed, some time is needed before further
; actions so that hardware reacts on this change. During that time,
; all changed pipes are organized in single-linked list with the head
; usb_controller.WaitPipeList* and link field NextWait.
; Currently there are two possible reasons to change:
; change of address/packet size in initial configuration,
; close of the pipe. They are distinguished by USB_FLAG_CLOSED.
NextWait        dd      ?
Lock            MUTEX
; Mutex that guards operations with transfer queue for this pipe.
Type            db      ?
; Type of pipe, one of {CONTROL,ISOCHRONOUS,BULK,INTERRUPT}_PIPE.
Flags           db      ?
; Combination of flags, USB_FLAG_*.
                rb      2       ; dword alignment
DeviceData      dd      ?
; Pointer to usb_device_data, common for all pipes for one device.
ends

; This structure describes the static head of every list of pipes.
struct usb_static_ep
; software fields
Bandwidth       dd      ?
; valid only for interrupt/isochronous USB1 lists
; The offsets of the following two fields must be the same in this structure
; and in usb_pipe.
NextVirt        dd      ?
PrevVirt        dd      ?
ends

; This structure represents one transfer descriptor
; ('g' stands for "general" as opposed to isochronous usb_itd).
; Note that one transfer can have several descriptors:
; a control transfer has three stages.
; Additionally, every controller has a limit on transfer length with
; one descriptor (packet size for UHCI, 1K for OHCI, 4K for EHCI),
; large transfers must be split into individual packets according to that limit.
struct usb_gtd
Callback        dd      ?
; Zero for intermediate descriptors, pointer to callback function
; for final descriptor. See the docs for description of the callback.
UserData        dd      ?
; Dword which is passed to Callback as is, not used by USB code itself.
; Two following fields organize all descriptors for one pipe in
; the linked list.
NextVirt        dd      ?
PrevVirt        dd      ?
Pipe            dd      ?
; Pointer to the parent usb_pipe.
Buffer          dd      ?
; Pointer to data for this descriptor.
Length          dd      ?
; Length of data for this descriptor.
ends

; Interface-specific data. Several interfaces of one device can operate
; independently, each is controlled by some driver and is identified by
; some driver-specific data passed as is to the driver.
struct usb_interface_data
DriverData      dd      ?
; Passed as is to the driver.
DriverFunc      dd      ?
; Pointer to USBSRV structure for the driver.
ends

; Device-specific data.
struct usb_device_data
PipeListLock    MUTEX
; Lock guarding OpenedPipeList. Must be the first item of the structure,
; the code passes pointer to usb_device_data as is to mutex_lock/unlock.
OpenedPipeList  rd      2
; List of all opened pipes for the device.
; Used when the device is disconnected, so all pipes should be closed.
ClosedPipeList  rd      2
; List of all closed, but still valid pipes for the device.
; A pipe closed with USBClosePipe is just deallocated,
; but a pipe closed due to disconnect must remain valid until driver-provided
; disconnect handler returns; this list links all such pipes to deallocate them
; after disconnect processing.
NumPipes        dd      ?
; Number of not-yet-closed pipes.
Hub             dd      ?
; NULL if connected to the root hub, pointer to usb_hub otherwise.
TTHub           dd      ?
; Pointer to usb_hub for (the) hub with Transaction Translator for the device,
; NULL if the device operates in the same speed as the controller.
Port            db      ?
; Port on the hub, zero-based.
TTPort          db      ?
; Port on the TTHub, zero-based.
DeviceDescrSize db      ?
; Size of device descriptor.
Speed           db      ?
; Device speed, one of USB_SPEED_*.
NumInterfaces   dd      ?
; Number of interfaces.
ConfigDataSize  dd      ?
; Total size of data associated with the configuration descriptor
; (including the configuration descriptor itself).
Interfaces      dd      ?
; Offset from the beginning of this structure to Interfaces field.
; Variable-length fields:
; DeviceDescriptor:
;  device descriptor starts here
; ConfigDescriptor = DeviceDescriptor + DeviceDescrSize
;  configuration descriptor with all associated data
; Interfaces = ALIGN_UP(ConfigDescriptor + ConfigDataSize, 4)
;  array of NumInterfaces elements of type usb_interface_data
ends

usb_device_data.DeviceDescriptor = sizeof.usb_device_data
