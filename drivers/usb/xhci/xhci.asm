; Code for xHCI controllers.
;
; With XHCI_SUPERSPEED enabled (the default) the driver serves Low-, Full- and
; High-speed devices on USB2 root hub ports and SuperSpeed devices on USB3
; root hub ports. The kernel USB stack only knows the three USB2 speeds, so a
; SuperSpeed device is reported to it as a high-speed one; everything that
; really depends on the speed - the Slot Context, the 512-byte default control
; endpoint, the burst size from the SuperSpeed Endpoint Companion descriptor -
; is handled privately by the driver, see xhci_ss.inc. SuperSpeed hubs are not
; supported: a device behind a USB3 hub is served by the USB2 half of that hub
; at high speed.
;
; Building with XHCI_SUPERSPEED = 0 compiles all of that out
; and returns to the pure USB2 driver: SuperSpeed ports are recognized through
; the Supported Protocol extended capability and skipped, so a device plugged
; into a USB3 port is simply not seen by the system.
;
; The kernel USB stack was designed around UHCI/OHCI/EHCI, where a pipe is a
; hardware queue head and a transfer descriptor is a hardware structure whose
; status is written back by the controller. xHCI works differently: transfers
; are described by TRBs on a per-endpoint ring and the completion status is
; reported only through the Event Ring. To fit into the existing interface,
; every usb_gtd gets a private xhci_gtd with Status/Residual/Done fields; the
; event ring handler fills them in and the usual "scan all pipes for finished
; descriptors" loop then works exactly like it does for EHCI.
;
; Commands (Enable Slot, Address Device, Configure Endpoint, ...) are issued
; synchronously: the caller submits a command TRB and polls the event ring
; until the corresponding Command Completion Event arrives. Polling never
; invokes any callback, it only records state, so it is safe to do from any
; context that is allowed to sleep.

; Standard driver stuff
format PE DLL native
entry start
__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1
section '.reloc' data readable discardable fixups
section '.text' code readable executable
include '../../proc32.inc'
include '../../struct.inc'
include '../../macros.inc'
include '../../fdo.inc'
include '../../../kernel/trunk/bus/usb/common.inc'

; Two build-time switches, both edited here.
;
; XHCI_SUPERSPEED zero compiles out every SuperSpeed addition and yields a
; driver that behaves exactly like the older USB2-only one: SuperSpeed ports
; are skipped and a device plugged into a USB3 port is not seen at all. Build
; it that way when the SuperSpeed support is suspected of causing trouble.
XHCI_SUPERSPEED equ 1
;
; XHCI_TIMING_TRACE one adds throughput accounting: bulk transfers are counted
; rather than traced, and one summary line per second reports how much of the
; time the bus actually carried data. Printing a line per transfer would cost
; about a millisecond apiece on real hardware - more than the transfers being
; measured - which is why this counts instead. Diagnostic builds only.
XHCI_TIMING_TRACE equ 0

; =============================================================================
; ================================= Constants =================================
; =============================================================================

; ------------------------- Capability registers ------------------------------
; Base is the MMIO area from the PCI space.
XhciCapLengthReg    = 0     ; byte
XhciVersionReg      = 2     ; word
XhciStructParams1   = 4
XhciStructParams2   = 8
XhciStructParams3   = 0Ch
XhciCapParams1      = 10h
XhciDoorbellOffReg  = 14h
XhciRuntimeOffReg   = 18h
XhciCapParams2      = 1Ch

; ------------------------- Operational registers -----------------------------
; Base is (capability register base) + (value of XhciCapLengthReg).
XhciCommandReg      = 0
XhciStatusReg       = 4
XhciPageSizeReg     = 8
XhciDevNotifyReg    = 14h
XhciCmdRingReg      = 18h   ; 64-bit
XhciDcbaapReg       = 30h   ; 64-bit
XhciConfigReg       = 38h
XhciPortsReg        = 400h  ; PORTSC of port n (1-based) is at 400h+10h*(n-1)

; USBCMD bits
XHCI_CMD_RUN        = 1 shl 0
XHCI_CMD_HCRST      = 1 shl 1
XHCI_CMD_INTE       = 1 shl 2
XHCI_CMD_HSEE       = 1 shl 3

; USBSTS bits
XHCI_STS_HCH        = 1 shl 0   ; HCHalted
XHCI_STS_HSE        = 1 shl 2   ; Host System Error
XHCI_STS_EINT       = 1 shl 3   ; Event Interrupt
XHCI_STS_PCD        = 1 shl 4   ; Port Change Detect
XHCI_STS_CNR        = 1 shl 11  ; Controller Not Ready
XHCI_STS_HCE        = 1 shl 12  ; Host Controller Error

; PORTSC bits
XHCI_PORT_CCS       = 1 shl 0   ; Current Connect Status, read-only
XHCI_PORT_PED       = 1 shl 1   ; Port Enabled/Disabled, write one to disable
XHCI_PORT_OCA       = 1 shl 3
XHCI_PORT_PR        = 1 shl 4   ; Port Reset
XHCI_PORT_PP        = 1 shl 9   ; Port Power
XHCI_PORT_CSC       = 1 shl 17
XHCI_PORT_PEC       = 1 shl 18
XHCI_PORT_WRC       = 1 shl 19
XHCI_PORT_OCC       = 1 shl 20
XHCI_PORT_PRC       = 1 shl 21
XHCI_PORT_PLC       = 1 shl 22
XHCI_PORT_CEC       = 1 shl 23
XHCI_PORT_WPR       = 1 shl 31  ; Warm Port Reset, SuperSpeed ports only
; All change bits together.
XHCI_PORT_CHANGES   = 0FE0000h
; PORTSC is never written with a read-modify-write cycle: some controllers
; take the echoed read-only and link-state fields at face value and restart
; port detection, which shows up as an endless connect/disconnect loop.
; Every write consists of Port Power, which must stay set, plus only the
; bits the write is actually meant to assert.

; --------------------------- Runtime registers -------------------------------
; Base is (capability register base) + (value of XhciRuntimeOffReg).
XhciMfIndexReg      = 0
XhciIman0Reg        = 20h
XhciImod0Reg        = 24h
XhciErstSize0Reg    = 28h
XhciErstBase0Reg    = 30h   ; 64-bit
XhciErdp0Reg        = 38h   ; 64-bit

; ------------------------------- TRB types -----------------------------------
XHCI_TRB_NORMAL       = 1
XHCI_TRB_SETUP        = 2
XHCI_TRB_DATA         = 3
XHCI_TRB_STATUS       = 4
XHCI_TRB_LINK         = 6
XHCI_TRB_ENABLE_SLOT  = 9
XHCI_TRB_DISABLE_SLOT = 10
XHCI_TRB_ADDRESS_DEV  = 11
XHCI_TRB_CONFIG_EP    = 12
XHCI_TRB_EVAL_CTX     = 13
XHCI_TRB_RESET_EP     = 14
XHCI_TRB_STOP_EP      = 15
XHCI_TRB_SET_DEQ      = 16
XHCI_TRB_RESET_DEV    = 17
XHCI_TRB_NOOP_CMD     = 23
XHCI_TRB_TRANSFER_EV  = 32
XHCI_TRB_CMDCOMPL_EV  = 33
XHCI_TRB_PORTCHG_EV   = 34

; TRB control flags (dword 3)
XHCI_TRB_C          = 1 shl 0   ; Cycle
XHCI_TRB_TC         = 1 shl 1   ; Toggle Cycle, Link TRB only
XHCI_TRB_ISP        = 1 shl 2   ; Interrupt on Short Packet
XHCI_TRB_CH         = 1 shl 4   ; Chain
XHCI_TRB_IOC        = 1 shl 5   ; Interrupt On Completion
XHCI_TRB_IDT        = 1 shl 6   ; Immediate Data
XHCI_TRB_BSR        = 1 shl 9   ; Block Set Address Request
XHCI_TRB_DIR_IN     = 1 shl 16  ; Direction of a Data or Status Stage TRB

; Completion codes
XHCI_CC_SUCCESS     = 1
XHCI_CC_DATA_BUF    = 2
XHCI_CC_BABBLE      = 3
XHCI_CC_USB_TRANS   = 4
XHCI_CC_TRB_ERROR   = 5
XHCI_CC_STALL       = 6
XHCI_CC_SHORT       = 13
XHCI_CC_CTX_STATE   = 19

; Endpoint types in the Endpoint Context
XHCI_EP_ISOCH_OUT   = 1
XHCI_EP_BULK_OUT    = 2
XHCI_EP_INT_OUT     = 3
XHCI_EP_CONTROL     = 4
XHCI_EP_ISOCH_IN    = 5
XHCI_EP_BULK_IN     = 6
XHCI_EP_INT_IN      = 7

; Port speed identifiers, default mapping of xHCI 7.2.2.1.1. Anything at
; XHCI_PSI_SS and above (5 is SuperSpeedPlus) counts as SuperSpeed and is
; passed to the Slot Context as is.
XHCI_PSI_FS         = 1
XHCI_PSI_LS         = 2
XHCI_PSI_HS         = 3
XHCI_PSI_SS         = 4

; ------------------------------ Memory layout --------------------------------
; A transfer ring occupies a whole page (256 TRBs, the last of them a Link TRB
; pointing back to the start); a second page keeps the mapping from a TRB
; index to the owning usb_gtd, used by the event ring handler.
XHCI_RING_TRBS      = 256
; The command ring uses the same layout, without the map.
XHCI_CMD_TRBS       = 128
; The event ring is a single segment occupying a whole page.
XHCI_EVENT_TRBS     = 256

; Layout of the "common" page: the Device Context Base Address Array, the
; single Event Ring Segment Table entry and the Scratchpad Buffer Array.
XHCI_DCBAA_OFS      = 0         ; 256 entries * 8 bytes
XHCI_ERST_OFS       = 2048      ; 16 bytes, 64-byte aligned
XHCI_SCRATCH_OFS    = 2112      ; 64-byte aligned
XHCI_MAX_SCRATCH    = (4096 - XHCI_SCRATCH_OFS) / 8

; Layout of the per-device page: the Device Context and the array translating a
; Device Context Index into the corresponding usb_pipe.
XHCI_DEVCTX_OFS     = 0         ; at most 32 * 64 bytes
XHCI_PIPES_OFS      = 2048      ; 32 dwords

; Maximum amount of data described by one usb_gtd. Larger transfers are split;
; the boundaries are aligned down to a multiple of the packet size, so that a
; short packet can only happen at the very end of the transfer.
XHCI_MAX_TD         = 10000h

; Offset of the ConfigPipe field inside usb_hub. That structure is private to
; the kernel, but the pointer to the config pipe of the hub itself is needed to
; find out where in the tree a device behind that hub sits. The fields before
; it are Next, Prev and Controller.
USB_HUB_CONFIGPIPE  = 12

; Maximum number of root hub ports visible to the kernel. usb_controller keeps
; per-port data in arrays of 16 dwords, so this cannot be raised without
; changing the kernel.
XHCI_MAX_PORTS      = 16

; Timeout of a synchronous command, in timer ticks of 10 ms each.
XHCI_CMD_TIMEOUT    = 100

; xhci_pipe.Flags
XHCI_PIPE_IN        = 1     ; the endpoint direction is device-to-host
XHCI_PIPE_SETADDR   = 2     ; the transfer being submitted is SET_ADDRESS
XHCI_PIPE_HUBPEND   = 4     ; a GET_DESCRIPTOR(HUB) answer is expected
XHCI_PIPE_ISHUB     = 8     ; the device has already been announced as a hub

; =============================================================================
; ================================ Structures =================================
; =============================================================================

; xHCI-specific part of a pipe descriptor.
; Unlike other host controllers, xHCI keeps no per-pipe structure that the
; hardware reads directly: an endpoint is described by an Endpoint Context
; inside the Device Context and by its Transfer Ring. Therefore this structure
; has no alignment requirements at all.
struct xhci_pipe
Ring            dd      ?
; Virtual address of the page with the transfer ring, zero if not allocated.
RingPhys        dd      ?
; Physical address of the same page.
RingMap         dd      ?
; Virtual address of the page with the TRB index -> usb_gtd map.
Enqueue         dd      ?
; Index of the next TRB to be written by the driver.
Dequeue         dd      ?
; Index of the first TRB not yet known to be consumed by the controller.
Cycle           dd      ?
; Producer Cycle State at the enqueue position.
FirstFix        dd      ?
; While a transfer is being written to the ring: 1 before the first TRB is
; written, then the address of that TRB, whose Cycle bit is left inverted until
; the whole transfer is in place. Zero at all other times.
Reserved        dd      ?
; Number of TRBs reserved by descriptors allocated but not yet inserted.
SlotId          dd      ?
; Device Slot ID assigned by the controller, zero if none.
DCI             dd      ?
; Device Context Index of the endpoint: 1 for the default control endpoint,
; 2*number+(direction is in ? 1 : 0) for the others.
MaxDCI          dd      ?
; Highest index in use by the device. Meaningful in the pipe of the default
; control endpoint, which represents the device as a whole.
DevPage         dd      ?
; Virtual address of the per-device page (Device Context and pipes by index).
DevPagePhys     dd      ?
; Physical address of the same page, i.e. of the Device Context.
Speed           dd      ?
; Device speed, one of USB_SPEED_*.
PSIV            dd      ?
; Raw PORTSC speed identifier of a SuperSpeed device on a root port, zero
; otherwise. The kernel knows nothing about SuperSpeed, so the Speed of such a
; device claims high-speed; this field keeps what the Slot Context really
; needs. Used only by the SuperSpeed code.
MaxBurst        dd      ?
; Max Burst Size for the Endpoint Context, taken from the SuperSpeed Endpoint
; Companion descriptor; zero for USB2 pipes. Used only by the SuperSpeed code.
RootPort        dd      ?
; Root hub port the device hangs on, 1-based.
Route           dd      ?
; Route String for the Slot Context.
Depth           dd      ?
; Number of hub tiers between the root hub and the device.
TTSlot          dd      ?
; Slot ID of the hub whose Transaction Translator serves the device, or zero.
TTPort          dd      ?
; Port on that hub, 1-based.
Address         dd      ?
; USB address allocated by the kernel. The controller assigns the real bus
; address itself; this copy exists only because the kernel asks for it back on
; disconnect in order to reuse the number.
MaxPacket       dd      ?
; Maximum packet size of the endpoint.
EpType          dd      ?
; Endpoint Type value for the Endpoint Context.
Interval        dd      ?
; Interval value for the Endpoint Context.
Flags           dd      ?
; Combination of XHCI_PIPE_* flags.
HubBuf          dd      ?
; Buffer of the pending GET_DESCRIPTOR(HUB) request, see XHCI_PIPE_HUBPEND.
TraceCount      dd      ?
; Number of debug lines already printed for this pipe. The first transfers of
; every pipe are traced to show that it came alive; after that the pipe goes
; quiet, so that a working mouse does not flood the log.
ends

; xHCI-specific part of a transfer descriptor.
; The controller never writes to the TRBs, so the completion status taken from
; the event ring is stored here instead.
struct xhci_gtd
Buf             dd      ?
; Virtual address of the data of this descriptor.
Len             dd      ?
; Length of the data.
Dir             dd      ?
; Direction code as passed to AllocTransfer.
Flags           dd      ?
; Flags as passed to AllocTransfer; bit 0 = a short transfer is allowed.
TrbCount        dd      ?
; Number of TRBs the descriptor needs.
Status          dd      ?
; Completion code taken from the transfer event.
Residual        dd      ?
; Number of bytes not transferred, taken from the transfer event.
Done            dd      ?
; Nonzero once the transfer event for this descriptor has been received.
NextTrb         dd      ?
; Physical address of the first TRB after this descriptor, used to restart the
; ring after an error.
NextCycle       dd      ?
; Cycle state at NextTrb.
FirstTrb        dd      ?
; Index of the first TRB of this descriptor in the ring.
ends

; xHCI-specific part of controller data.
; Nothing here is read by the hardware, so this structure has no alignment or
; contiguity requirements; everything the controller reads is allocated
; separately as whole pages.
struct xhci_controller
; Static heads of the three pipe lists. The hardware knows nothing about them;
; they exist because the kernel keeps every pipe in a doubly-linked list with a
; static head and reinserts a pipe there after usb_abort_pipe.
ControlED       usb_static_ep
BulkED          usb_static_ep
IntED           usb_static_ep
MMIOBase        dd      ?
; Virtual address of the memory-mapped capability registers.
MMIOSize        dd      ?
; Size of that mapping, in bytes.
OpBase          dd      ?
; Virtual address of the operational registers.
RtBase          dd      ?
; Virtual address of the runtime registers.
DbBase          dd      ?
; Virtual address of the doorbell array.
CapParams1      dd      ?
; Copy of HCCPARAMS1.
ContextSize     dd      ?
; Size of one context structure, 32 or 64 bytes.
MaxSlots        dd      ?
; Number of device slots enabled on the controller.
NumRealPorts    dd      ?
; Total number of root hub ports, SuperSpeed ones included.
DeferredActions dd      ?
; Bitmask of events noticed by the interrupt handler, processed in the thread.
NoIrq           dd      ?
; Nonzero when the firmware has assigned no interrupt line to the controller,
; so that everything has to be driven by polling.
ResetStart      dd      ?
; Time when reset signalling was started on the currently resetting port.
; Unlike usb_controller.ResetTime, which is pushed forward while the reset is
; still running, this keeps the initial moment, bounding the total duration.
PassLast        dd      ?
; Last reported combination of the enumeration state, see xhci_process_deferred.
BounceMask      dd      ?
; Ports whose device dropped off while waiting out the connect debounce.
; Some ports lose the device unless it is reset promptly after the connect,
; so on the next connect the debounce is skipped and reset starts at once.
SSPortMask      dd      ?
; Which entries of PortMap are SuperSpeed ports, as a bitmask over the port
; indices used by the kernel. Used only by the SuperSpeed code.
ResettingPSIV   dd      ?
; Raw PORTSC speed identifier of the root-port device being enumerated, zero
; when it is a USB2 one. Parallels usb_controller.ResettingSpeed and exists
; for the same reason. Used only by the SuperSpeed code.
CmdRing         dd      ?
CmdRingPhys     dd      ?
CmdEnqueue      dd      ?
CmdCycle        dd      ?
; The command ring.
EventRing       dd      ?
EventRingPhys   dd      ?
EventDequeue    dd      ?
EventCycle      dd      ?
; The event ring.
CommonPage      dd      ?
CommonPhys      dd      ?
; The page holding the DCBAA, the ERST and the scratchpad buffer array.
InputCtx        dd      ?
InputCtxPhys    dd      ?
; The Input Context, shared by all commands and guarded by CmdLock.
CmdStatus       dd      ?
; Completion code of the command that has completed last.
CmdSlotId       dd      ?
; Slot ID reported by that command.
CmdDone         dd      ?
; Set by the event handler when the awaited command has completed.
CmdWaitPhys     dd      ?
; Physical address of the command TRB being awaited, zero if none.
NeedScan        dd      ?
; Nonzero when at least one descriptor has been completed and the pipe lists
; have to be scanned.
PortChanged     dd      ?
; Nonzero when a Port Status Change Event has been seen.
FatalNoted      dd      ?
; Nonzero once a Host System Error or Host Controller Error has been reported,
; so that a dead controller produces one log line rather than a stream.
CmdLock         MUTEX
; Serializes command submission and the use of the Input Context.
EventLock       MUTEX
; Serializes event ring processing.
PortMap         rd      XHCI_MAX_PORTS
; Real (1-based) port number for every port index known to the kernel.
SlotPages       rd      256
; Virtual address of the per-device page of every slot, zero when unused.
ends

; Description of xHCI-specific data and functions for controller-independent
; code. Implements the structure usb_hardware_func from common.inc.
iglobal
align 4
xhci_hardware_func:
        dd      USBHC_VERSION
        dd      'XHCI'
        dd      sizeof.xhci_controller
        dd      xhci_kickoff_bios
        dd      xhci_init
        dd      xhci_process_deferred
        dd      xhci_set_device_address
        dd      xhci_get_device_address
        dd      xhci_port_disable
if XHCI_SUPERSPEED
        dd      xhci_ss_port_reset
else
        dd      xhci_new_port.reset
end if
        dd      xhci_set_endpoint_packet_size
        dd      xhci_alloc_pipe
        dd      xhci_free_pipe
        dd      xhci_init_pipe
        dd      xhci_unlink_pipe
        dd      xhci_alloc_td
        dd      xhci_free_td
        dd      xhci_alloc_transfer
        dd      xhci_insert_transfer
        dd      xhci_new_device
        dd      xhci_disable_pipe
        dd      xhci_enable_pipe
xhci_name db    'XHCI',0
endg

; The xhci_controller structure is located immediately before usb_controller,
; so its fields are addressed as [esi+xhci_controller.field-XCD].
XCD = sizeof.xhci_controller

uglobal
align 4
; Data for the slab allocator, see memory.inc in the kernel. The pointer to the
; first page must immediately precede the mutex.
xhci_ep_first_page      dd      ?
xhci_ep_mutex           MUTEX
xhci_gtd_first_page     dd      ?
xhci_gtd_mutex          MUTEX
; Counter limiting the amount of tracing produced while bringing the driver up
; on new hardware; see the xhci_trace macro.
xhci_trace_count        dd      ?
if XHCI_TIMING_TRACE
; Totals of the transfer being inserted, refilled on every call.
xhci_tt_bytes           dd      ?
xhci_tt_trbs            dd      ?
xhci_tt_tds             dd      ?
; Throughput counters. Printing one line per transfer costs about a
; millisecond on real hardware - more than the transfer itself - so the
; timing build counts instead and prints one summary per second.
xhci_st_tick            dd      ?      ; GetTimerTicks at the window start
xhci_st_bytes           dd      ?      ; bytes handed to bulk endpoints
xhci_st_tds             dd      ?      ; descriptors queued
xhci_st_cmds            dd      ?      ; command blocks, i.e. SCSI commands
xhci_st_inflight        dd      ?      ; bulk descriptors not yet completed
xhci_st_busy_from       dd      ?      ; TSC when the count last left zero
xhci_st_busy            dd      ?      ; TSC ticks with at least one in flight
xhci_st_done            dd      ?      ; TSC when the count last reached zero
; The idle time is split by what ended it: a command block wrapper means the
; layer above took that long to ask for the next SCSI command, anything else
; means one command stalled between its own stages.
xhci_st_gapcmd          dd      ?
xhci_st_gapstg          dd      ?
end if
; Handle of the timer that wakes the USB thread when no controller has an
; interrupt line; zero if not created, -1 if creation failed.
xhci_heartbeat_timer    dd      ?
endg

XHCI_TRACE_LIMIT = 400

; Emits a debug line for the first XHCI_TRACE_LIMIT events only, so that a
; per-iteration trace cannot flood the debug board.
macro xhci_trace fmt, arg1, arg2
{
local .skip
        cmp     [xhci_trace_count], XHCI_TRACE_LIMIT
        jae     .skip
        inc     [xhci_trace_count]
        DEBUGF 1,fmt,arg1,arg2
.skip:
}

; =============================================================================
; =================================== Code ====================================
; =============================================================================

; Called once when the driver is loading and once at shutdown.
proc start
virtual at esp
                dd      ? ; return address
.reason         dd      ? ; DRV_ENTRY or DRV_EXIT
.cmdline        dd      ? ; normally NULL
end virtual
        cmp     [.reason], DRV_ENTRY
        jnz     .nothing
        mov     ecx, xhci_ep_mutex
        invoke  MutexInit
        mov     ecx, xhci_gtd_mutex
        invoke  MutexInit
        push    esi edi
        mov     esi, [USBHCFunc]
        mov     edi, usbhc_api
        movi    ecx, sizeof.usbhc_func/4
        rep movsd
        pop     edi esi
        invoke  RegUSBDriver, xhci_name, 0, xhci_hardware_func
.nothing:
        ret
endp

; =============================================================================
; ============================ Low-level helpers ==============================
; =============================================================================

; Allocates one zero-filled page.
; out: eax = virtual address or 0, edx = physical address.
proc xhci_alloc_page
        push    ecx edi
        invoke  KernelAlloc, 0x1000
        test    eax, eax
        jz      .nothing
        mov     edi, eax
        push    eax
        xor     eax, eax
        mov     ecx, 0x1000/4
        rep stosd
        pop     eax
        push    eax
        invoke  GetPhysAddr
        mov     edx, eax
        pop     eax
.nothing:
        pop     edi ecx
        ret
endp

; Writes a value into a 64-bit register; the upper half is always zero, since
; the kernel is 32-bit and every structure lives below 4G.
; in: edi = register address, eax = value.
proc xhci_write64
        mov     [edi], eax
        mov     dword [edi+4], 0
        ret
endp

; Rings a doorbell.
; in: esi -> usb_controller, eax = slot (zero for the command ring),
;     edx = doorbell target.
proc xhci_doorbell
        push    ecx edx
        mov     ecx, [esi+xhci_controller.DbBase-XCD]
        mov     [ecx+eax*4], edx
; Read the register back so that the write cannot be postponed.
        mov     edx, [ecx+eax*4]
        pop     edx ecx
        ret
endp

; Returns the address of the PORTSC register of a port.
; in: esi -> usb_controller, ecx = port index as used by the kernel.
; out: edx = address of PORTSC.
proc xhci_port_reg
        push    eax
        mov     eax, [esi+xhci_controller.PortMap-XCD+ecx*4]
        dec     eax
        shl     eax, 4
        mov     edx, [esi+xhci_controller.OpBase-XCD]
        lea     edx, [edx+eax+XhciPortsReg]
        pop     eax
        ret
endp

; Translates an xHCI completion code into one of the USB_STATUS_* constants.
; in: eax = completion code. out: eax = status.
proc xhci_status_from_cc
        cmp     eax, XHCI_CC_STALL
        jz      .stall
        cmp     eax, XHCI_CC_BABBLE
        jz      .babble
        cmp     eax, XHCI_CC_DATA_BUF
        jz      .buffer
        movi    eax, USB_STATUS_NORESPONSE
        ret
.stall:
        movi    eax, USB_STATUS_STALL
        ret
.babble:
        movi    eax, USB_STATUS_OVERRUN
        ret
.buffer:
        movi    eax, USB_STATUS_BUFOVERRUN
        ret
endp

; =============================================================================
; =============================== Event ring ==================================
; =============================================================================

; Handles one Transfer Event.
; in: esi -> usb_controller, edi -> event TRB.
proc xhci_handle_transfer_event
        push    ebx ecx edx
; 1. Locate the pipe: the event carries the Slot ID and the Device Context
; Index, and the per-device page keeps a pipe pointer for every index.
        mov     eax, [edi+12]
        shr     eax, 24                 ; Slot ID
        jz      .nothing
        mov     ebx, [esi+xhci_controller.SlotPages-XCD+eax*4]
        test    ebx, ebx
        jz      .nothing
        mov     ecx, [edi+12]
        shr     ecx, 16
        and     ecx, 31                 ; Endpoint ID equals the index
        jz      .nothing
        mov     ebx, [ebx+XHCI_PIPES_OFS+ecx*4]
        test    ebx, ebx
        jz      .nothing
        sub     ebx, sizeof.xhci_pipe
; 2. Convert the reported TRB address into an index in the transfer ring.
; Events with the Event Data bit set carry no TRB pointer; this driver never
; produces such events, so they are ignored.
        test    byte [edi+12], 4
        jnz     .nothing
        mov     eax, [edi]
        sub     eax, [ebx+xhci_pipe.RingPhys]
        cmp     eax, XHCI_RING_TRBS*16
        jae     .nothing
        shr     eax, 4
        cmp     eax, XHCI_RING_TRBS-1
        jae     .nothing                ; the Link TRB never generates events
; 3. Find the descriptor owning that TRB: every TRB of a descriptor is entered
; into the map, because the controller reports errors and short packets on the
; TRB where they happened, not on the last one of the descriptor.
        mov     edx, [ebx+xhci_pipe.RingMap]
        mov     ecx, eax                ; ecx = index of the reported TRB
        mov     eax, [edx+ecx*4]
        test    eax, eax
        jz      .orphan
; A short packet in the middle of a descriptor makes the controller skip the
; rest of it and then still report the last TRB with a success code; that
; second event must not overwrite the result of the first one.
        cmp     [eax+xhci_gtd.Done-sizeof.xhci_gtd], 0
        jnz     .nothing
; 4. Record the result in the descriptor. The residual in the event refers to
; the reported TRB only; the fragments of the descriptor that follow it were
; not transferred either, so their lengths are added to get the amount not
; transferred for the descriptor as a whole.
        push    eax                     ; the descriptor
        mov     edx, [edi+8]
        and     edx, 0FFFFFFh           ; residual of the reported TRB
        push    ecx
.sum_rest:
        inc     ecx
        cmp     ecx, XHCI_RING_TRBS-1
        jb      @f
        xor     ecx, ecx
@@:
        push    edx
        mov     edx, [ebx+xhci_pipe.RingMap]
        cmp     [edx+ecx*4], eax
        pop     edx
        jnz     .summed                 ; the next TRB belongs to someone else
        push    eax
        mov     eax, ecx
        shl     eax, 4
        add     eax, [ebx+xhci_pipe.Ring]
        mov     eax, [eax+8]
        and     eax, 1FFFFh             ; TRB Transfer Length
        add     edx, eax
        pop     eax
        jmp     .sum_rest
.summed:
        pop     ecx                     ; index of the reported TRB
        pop     eax                     ; the descriptor
        mov     [eax+xhci_gtd.Residual-sizeof.xhci_gtd], edx
        mov     edx, [edi+8]
        shr     edx, 24                 ; completion code
        mov     [eax+xhci_gtd.Status-sizeof.xhci_gtd], edx
        mov     [eax+xhci_gtd.Done-sizeof.xhci_gtd], 1
if XHCI_TIMING_TRACE
; The bus counts as busy while any bulk descriptor is outstanding; the window
; closes when the last one completes. Only bulk endpoints are counted, an
; interrupt endpoint polled every millisecond would keep it busy forever.
        pushad
        mov     esi, [ebx+xhci_pipe.EpType]
        and     esi, not 4              ; bulk out is 2, bulk in is 6
        cmp     esi, 2
        jnz     .st_done
        cmp     [xhci_st_inflight], 0
        jz      .st_done
        dec     [xhci_st_inflight]
        jnz     .st_done
        rdtsc
        mov     ecx, eax                ; keep the timestamp itself
        sub     eax, [xhci_st_busy_from]
        add     [xhci_st_busy], eax
        mov     [xhci_st_done], ecx
.st_done:
        popad
end if
; Trace the first few completions of every pipe, then go quiet: a steady
; stream of events from a working device is noise, not information. Real
; errors are always reported, but a short packet is not one: an interrupt
; endpoint that returns less than its packet size - a mouse, say - reports one
; on every poll, and tracing those floods the debug board and burns the whole
; trace budget before anything interesting happens.
        cmp     edx, XHCI_CC_SUCCESS
        jz      .limited
        cmp     edx, XHCI_CC_SHORT
        jnz     .trace
.limited:
        cmp     [ebx+xhci_pipe.TraceCount], 8
        jae     .traced
        inc     [ebx+xhci_pipe.TraceCount]
.trace:
        xhci_trace 'K : XHCI event: td %x cc %x\n', eax, [edi+8]
.traced:
; 5. Advance the software copy of the dequeue pointer past the descriptor,
; releasing its map entries: the controller has either executed or skipped
; every TRB up to the end of the descriptor.
        push    eax
        mov     edx, [ebx+xhci_pipe.RingMap]
        mov     ecx, [eax+xhci_gtd.FirstTrb-sizeof.xhci_gtd]
.release:
        cmp     [edx+ecx*4], eax
        jnz     .released
        and     dword [edx+ecx*4], 0
        inc     ecx
        cmp     ecx, XHCI_RING_TRBS-1
        jb      .release
        xor     ecx, ecx
        jmp     .release
.released:
        pop     eax
        mov     [ebx+xhci_pipe.Dequeue], ecx
        mov     [esi+xhci_controller.NeedScan-XCD], 1
        jmp     .nothing
.orphan:
; An event pointing into the ring, but no descriptor owns that TRB. Happens
; legitimately for the second event of a descriptor whose entries were already
; released; anything else is worth a line in the log, so trace it - the
; per-pipe limit keeps it from flooding.
        cmp     [ebx+xhci_pipe.TraceCount], 8
        jae     .nothing
        inc     [ebx+xhci_pipe.TraceCount]
        xhci_trace 'K : XHCI orphan event: %x %x\n', [edi], [edi+8]
.nothing:
        pop     edx ecx ebx
        ret
endp

; Handles one Command Completion Event.
; in: esi -> usb_controller, edi -> event TRB.
proc xhci_handle_command_event
        mov     eax, [esi+xhci_controller.CmdWaitPhys-XCD]
        test    eax, eax
        jz      .nothing
        cmp     eax, [edi]
        jnz     .nothing
        mov     eax, [edi+8]
        shr     eax, 24
        mov     [esi+xhci_controller.CmdStatus-XCD], eax
        mov     eax, [edi+12]
        shr     eax, 24
        mov     [esi+xhci_controller.CmdSlotId-XCD], eax
        mov     [esi+xhci_controller.CmdDone-XCD], 1
.nothing:
        ret
endp

; Processes all pending events. Never calls any callback: it only records the
; results, so it is safe to call while waiting for a command to complete.
; in: esi -> usb_controller.
proc xhci_process_event_ring
        push    ebx ecx edx edi
        lea     ecx, [esi+xhci_controller.EventLock-XCD]
        invoke  MutexLock
        xor     ebx, ebx                ; number of events processed
.loop:
        mov     edi, [esi+xhci_controller.EventDequeue-XCD]
        shl     edi, 4
        add     edi, [esi+xhci_controller.EventRing-XCD]
; The Cycle bit of an entry differs from the consumer cycle state as long as
; the entry has not been written by the controller.
        mov     eax, [edi+12]
        and     eax, XHCI_TRB_C
        cmp     eax, [esi+xhci_controller.EventCycle-XCD]
        jnz     .done
        inc     ebx
        mov     eax, [edi+12]
        shr     eax, 10
        and     eax, 3Fh
        cmp     eax, XHCI_TRB_TRANSFER_EV
        jz      .transfer
        cmp     eax, XHCI_TRB_CMDCOMPL_EV
        jz      .command
        cmp     eax, XHCI_TRB_PORTCHG_EV
        jz      .portchange
; Anything else is unexpected here: a Host Controller Event (a dying
; controller, an overflowing event ring) or an event type this driver never
; enables. There is nothing to do about them, but skipping silently would
; leave nothing to debug from, so leave a trace.
        xhci_trace 'K : XHCI event type %d ignored, dw2 %x\n', eax, [edi+8]
        jmp     .next
.transfer:
        call    xhci_handle_transfer_event
        jmp     .next
.command:
        call    xhci_handle_command_event
        jmp     .next
.portchange:
        mov     [esi+xhci_controller.PortChanged-XCD], 1
.next:
        mov     eax, [esi+xhci_controller.EventDequeue-XCD]
        inc     eax
        cmp     eax, XHCI_EVENT_TRBS
        jb      @f
        xor     eax, eax
        xor     [esi+xhci_controller.EventCycle-XCD], XHCI_TRB_C
@@:
        mov     [esi+xhci_controller.EventDequeue-XCD], eax
        jmp     .loop
.done:
; Publish the new dequeue pointer, clearing the Event Handler Busy bit.
        test    ebx, ebx
        jz      @f
        mov     eax, [esi+xhci_controller.EventDequeue-XCD]
        shl     eax, 4
        add     eax, [esi+xhci_controller.EventRingPhys-XCD]
        or      eax, 8                  ; Event Handler Busy
        mov     edi, [esi+xhci_controller.RtBase-XCD]
        add     edi, XhciErdp0Reg
        call    xhci_write64
@@:
        lea     ecx, [esi+xhci_controller.EventLock-XCD]
        invoke  MutexUnlock
        pop     edi edx ecx ebx
        ret
endp

; =============================================================================
; ============================== Command ring =================================
; =============================================================================

; Submits a command TRB and waits for its Command Completion Event.
; The caller must hold CmdLock.
; in: esi -> usb_controller, eax/ebx/ecx/edx = the four dwords of the TRB;
;     the Cycle bit in edx is set here.
; out: eax = completion code, zero if the command timed out;
;      edx = Slot ID reported by the controller.
proc xhci_cmd_submit
        push    ebx ecx edi
; 1. Write the TRB at the current enqueue position.
        mov     edi, [esi+xhci_controller.CmdEnqueue-XCD]
        shl     edi, 4
        add     edi, [esi+xhci_controller.CmdRing-XCD]
        mov     [edi], eax
        mov     [edi+4], ebx
        mov     [edi+8], ecx
        or      edx, [esi+xhci_controller.CmdCycle-XCD]
        mov     [edi+12], edx
; 2. Remember which event is being awaited.
        mov     eax, [esi+xhci_controller.CmdEnqueue-XCD]
        shl     eax, 4
        add     eax, [esi+xhci_controller.CmdRingPhys-XCD]
        mov     [esi+xhci_controller.CmdWaitPhys-XCD], eax
        and     [esi+xhci_controller.CmdDone-XCD], 0
        and     [esi+xhci_controller.CmdStatus-XCD], 0
        and     [esi+xhci_controller.CmdSlotId-XCD], 0
; 3. Advance the enqueue pointer, refreshing the Link TRB when the ring wraps.
        mov     eax, [esi+xhci_controller.CmdEnqueue-XCD]
        inc     eax
        cmp     eax, XHCI_CMD_TRBS-1
        jb      .stored
        mov     edi, [esi+xhci_controller.CmdRing-XCD]
        add     edi, (XHCI_CMD_TRBS-1)*16
        mov     eax, [esi+xhci_controller.CmdCycle-XCD]
        or      eax, (XHCI_TRB_LINK shl 10) + XHCI_TRB_TC
        mov     [edi+12], eax
        xor     [esi+xhci_controller.CmdCycle-XCD], XHCI_TRB_C
        xor     eax, eax
.stored:
        mov     [esi+xhci_controller.CmdEnqueue-XCD], eax
; 4. Ring the command doorbell.
        xor     eax, eax
        xor     edx, edx
        call    xhci_doorbell
; 5. Wait for the completion event.
        movi    ecx, XHCI_CMD_TIMEOUT
.wait:
        call    xhci_process_event_ring
        cmp     [esi+xhci_controller.CmdDone-XCD], 0
        jnz     .complete
        push    ecx esi
        movi    esi, 1
        invoke  Sleep
        pop     esi ecx
        dec     ecx
        jnz     .wait
; 6. The command has not completed in time. Abort it, so that the ring does not
; stay blocked forever, and report the failure.
        dbgstr 'XHCI: command timeout'
        mov     edi, [esi+xhci_controller.OpBase-XCD]
; CRCR reads back as zeros in every field but CRR, so a read-modify-write
; preserves nothing: write the Command Abort bit directly. The controller
; answers with a Command Ring Stopped event, which the event dispatcher drops
; as an unmatched command completion - CmdWaitPhys is already zero by then.
        movi    eax, 4                  ; Command Abort
        mov     [edi+XhciCmdRingReg], eax
        and     [esi+xhci_controller.CmdWaitPhys-XCD], 0
        xor     eax, eax
        xor     edx, edx
        jmp     .nothing
.complete:
        and     [esi+xhci_controller.CmdWaitPhys-XCD], 0
        mov     eax, [esi+xhci_controller.CmdStatus-XCD]
        mov     edx, [esi+xhci_controller.CmdSlotId-XCD]
.nothing:
        pop     edi ecx ebx
        ret
endp

; The same, taking and releasing CmdLock.
proc xhci_cmd_sync
        push    eax ebx ecx edx
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexLock
        pop     edx ecx ebx eax
        call    xhci_cmd_submit
        push    eax edx
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexUnlock
        pop     edx eax
        ret
endp

include 'xhci_context.inc'
include 'xhci_transfer.inc'
if XHCI_SUPERSPEED
include 'xhci_ss.inc'
end if

; =============================================================================
; ============================= Initialization ================================
; =============================================================================

; Computes how much of the MMIO area has to be mapped. The register blocks are
; scattered by offsets taken from the capability registers, and the extended
; capabilities can live far beyond the first page.
; in: edi = MMIO base, with at least the capability registers mapped.
; out: eax = size in bytes, rounded up to a page.
proc xhci_mmio_size
        push    ecx edx
        mov     eax, [edi+XhciDoorbellOffReg]
        and     eax, not 3
        add     eax, 4*256
        mov     ecx, [edi+XhciRuntimeOffReg]
        and     ecx, not 1Fh
        add     ecx, 20h + 32*8
        cmp     eax, ecx
        jae     @f
        mov     eax, ecx
@@:
        mov     ecx, [edi+XhciStructParams1]
        shr     ecx, 24                 ; number of root hub ports
        shl     ecx, 4
        add     ecx, XhciPortsReg
        mov     edx, [edi+XhciCapLengthReg]
        movzx   edx, dl
        add     ecx, edx
        cmp     eax, ecx
        jae     @f
        mov     eax, ecx
@@:
; Reserve a page for the extended capability chain itself.
        mov     ecx, [edi+XhciCapParams1]
        shr     ecx, 16
        and     ecx, 0FFFFh
        jz      @f
        shl     ecx, 2
        add     ecx, 0x1000
        cmp     eax, ecx
        jae     @f
        mov     eax, ecx
@@:
        add     eax, 0FFFh
        and     eax, not 0FFFh
        pop     edx ecx
        ret
endp

; Controller-specific pre-initialization: take ownership from the BIOS.
; in: esi -> PCIDEV.
proc xhci_kickoff_bios
        push    ebx ecx edx edi
; 1. Map the capability registers, then remap with the size they imply: the
; extended capabilities often live beyond the first page.
        invoke  PciRead32, dword [esi+PCIDEV.bus], dword [esi+PCIDEV.devfn], 10h
        mov     ebx, eax
; A 64-bit BAR assigned above 4G cannot be mapped from here; leave the
; controller to the BIOS then, xhci_init will refuse it with a message.
        test    bl, 100b
        jz      @f
        push    ebx esi                 ; nothing survives a kernel call
        invoke  PciRead32, dword [esi+PCIDEV.bus], dword [esi+PCIDEV.devfn], 14h
        pop     esi ebx
        test    eax, eax
        jnz     .nothing
@@:
        mov     eax, ebx
        and     al, not 0Fh
        mov     ebx, eax                ; ebx = physical base address
        invoke  MapIoMem, ebx, 0x1000, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .nothing
        mov     edi, eax
        call    xhci_mmio_size
        cmp     eax, 0x1000
        ja      .remap
        push    0x1000
        jmp     .mapped
.remap:
        push    eax ebx
        invoke  FreeKernelSpace, edi
        pop     ebx ecx
        push    ecx
        invoke  MapIoMem, ebx, ecx, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .unmapped
        mov     edi, eax
.mapped:
        pop     ebx                     ; ebx = size of the mapping
; 2. Walk the extended capability list looking for USB Legacy Support, ID 1.
        mov     edx, [edi+XhciCapParams1]
        shr     edx, 16
        and     edx, 0FFFFh
        jz      .done
        shl     edx, 2                  ; byte offset from the MMIO base
        movi    ecx, 100h               ; limit on the number of capabilities
.next_cap:
        lea     eax, [edx+8]
        cmp     eax, ebx
        ja      .done
; A memory-mapped register must be read with its natural width, so every field
; is extracted from a whole dword.
        mov     eax, [edi+edx]
        cmp     al, 1
        jz      .found
        movzx   eax, ah
        test    eax, eax
        jz      .done
        shl     eax, 2
        add     edx, eax
        dec     ecx
        jnz     .next_cap
        jmp     .done
.found:
; 3. Request ownership by setting the HC OS Owned Semaphore, then wait up to
; one second for the BIOS to clear the HC BIOS Owned Semaphore.
        mov     eax, [edi+edx]
        test    eax, 1 shl 16
        jz      .disable_smi
        or      eax, 1 shl 24
        mov     [edi+edx], eax
        movi    ecx, 100
@@:
        test    dword [edi+edx], 1 shl 16
        jz      .disable_smi
        push    ecx esi
        movi    esi, 1
        invoke  Sleep
        pop     esi ecx
        dec     ecx
        jnz     @b
        dbgstr 'XHCI: taking ownership from BIOS timed out'
; The BIOS did not answer. Clear its ownership bit and hope for the best.
        mov     eax, [edi+edx]
        and     eax, not (1 shl 16)
        mov     [edi+edx], eax
.disable_smi:
; 4. Disable every SMI source and acknowledge the pending SMI status bits, so
; that the BIOS cannot interfere anymore.
        mov     eax, [edi+edx+4]
        and     eax, 0FFFF1FEEh
        or      eax, 0E0000000h
        mov     [edi+edx+4], eax
.done:
; 5. Make sure the controller raises no interrupt before the system is ready to
; serve them.
        mov     eax, [edi+XhciCapLengthReg]
        movzx   eax, al
        add     eax, edi
        and     dword [eax+XhciCommandReg], not (XHCI_CMD_RUN + XHCI_CMD_INTE)
        invoke  FreeKernelSpace, edi
        jmp     .nothing
.unmapped:
; The remapping has failed; the old mapping is already gone.
        pop     ecx
.nothing:
        pop     edi edx ecx ebx
        ret
endp

; Parses the Supported Protocol extended capabilities and builds the map from
; the port indices used by the kernel to real port numbers. USB2 ports always
; occupy the first slots of the map: when a controller has more ports than the
; kernel can serve, the SuperSpeed ones are sacrificed first. In the USB2-only
; build the second pass is compiled out, keeping the old behavior of skipping
; SuperSpeed ports altogether.
; in: edi = MMIO base, esi -> usb_controller.
; out: [esi+usb_controller.NumPorts] filled in.
proc xhci_parse_protocols
        push    ebx ecx edx edi
; 1. Start with an empty map.
        xor     eax, eax
        mov     ecx, XHCI_MAX_PORTS
        push    edi
        lea     edi, [esi+xhci_controller.PortMap-XCD]
        rep stosd
        pop     edi
        and     [esi+usb_controller.NumPorts], 0
if XHCI_SUPERSPEED
        and     [esi+xhci_controller.SSPortMask-XCD], 0
end if
; 2. Collect the USB2 ports.
        movi    ebx, 2
        call    xhci_parse_protocols_pass
if XHCI_SUPERSPEED
; 3. Append the SuperSpeed ports after them.
        movi    ebx, 3
        call    xhci_parse_protocols_pass
end if
; 4. Some controllers do not provide the capability at all; assume then that
; every port is a USB2 port.
        cmp     [esi+usb_controller.NumPorts], 0
        jnz     .nothing
        mov     ecx, [esi+xhci_controller.NumRealPorts-XCD]
        cmp     ecx, XHCI_MAX_PORTS
        jbe     @f
        movi    ecx, XHCI_MAX_PORTS
@@:
        mov     [esi+usb_controller.NumPorts], ecx
        xor     eax, eax
.fill:
        cmp     eax, ecx
        jae     .nothing
        lea     ebx, [eax+1]
        mov     [esi+xhci_controller.PortMap-XCD+eax*4], ebx
        inc     eax
        jmp     .fill
.nothing:
        pop     edi edx ecx ebx
        ret
endp

; One pass of the walk above: adds the ports of every Supported Protocol
; capability with the given major revision to the map.
; in: edi = MMIO base, esi -> usb_controller, ebx = major revision (2 or 3).
proc xhci_parse_protocols_pass
        push    ecx edx
        mov     edx, [edi+XhciCapParams1]
        shr     edx, 16
        and     edx, 0FFFFh
        jz      .done
        shl     edx, 2
        movi    ecx, 100h
.next_cap:
        lea     eax, [edx+16]
        cmp     eax, [esi+xhci_controller.MMIOSize-XCD]
        ja      .done
; Registers are read as whole dwords: a memory-mapped register must be accessed
; with its natural width, byte reads of it are not guaranteed to work.
        mov     eax, [edi+edx]
        cmp     bl, 2
        jnz     @f                      ; dump each capability once, not per pass
        DEBUGF 1,'K : XHCI extcap at %x: %x %x\n',edx,eax,[edi+edx+8]
@@:
        cmp     al, 2                   ; Supported Protocol capability
        jnz     .advance
        shr     eax, 24                 ; Major Revision
        cmp     eax, ebx
        jnz     .advance
; Add every port of the range to the map.
        push    ebx ecx
        mov     ebx, [edi+edx+8]
        movzx   ecx, bh                 ; Compatible Port Count
        movzx   ebx, bl                 ; Compatible Port Offset, 1-based
        test    ebx, ebx
        jz      .range_done
.port_loop:
        test    ecx, ecx
        jz      .range_done
        mov     eax, [esi+usb_controller.NumPorts]
        cmp     eax, XHCI_MAX_PORTS
        jae     .overflow
        mov     [esi+xhci_controller.PortMap-XCD+eax*4], ebx
if XHCI_SUPERSPEED
        cmp     dword [esp+4], 3        ; the saved major revision
        jnz     @f
        bts     [esi+xhci_controller.SSPortMask-XCD], eax
@@:
end if
        inc     eax
        mov     [esi+usb_controller.NumPorts], eax
        inc     ebx
        dec     ecx
        jmp     .port_loop
.overflow:
        DEBUGF 1,'K : XHCI: over 16 root ports, the rest is dropped\n'
.range_done:
        pop     ecx ebx
.advance:
        mov     eax, [edi+edx]
        movzx   eax, ah                 ; Next Capability Pointer
        test    eax, eax
        jz      .done
        shl     eax, 2
        add     edx, eax
        dec     ecx
        jnz     .next_cap
.done:
        pop     edx ecx
        ret
endp

; Allocates and programs the scratchpad buffers requested by the controller.
; in: esi -> usb_controller. out: eax = 0 on failure.
proc xhci_init_scratchpad
        push    ebx ecx edx edi
; 1. The number of buffers is split between two fields of HCSPARAMS2.
        mov     edi, [esi+xhci_controller.MMIOBase-XCD]
        mov     eax, [edi+XhciStructParams2]
        mov     ecx, eax
        shr     ecx, 27
        and     ecx, 1Fh                ; low five bits of the count
        shr     eax, 21
        and     eax, 1Fh                ; high five bits
        shl     eax, 5
        or      ecx, eax
        jz      .ok
        cmp     ecx, XHCI_MAX_SCRATCH
        ja      .fail
; 2. Allocate one page per buffer and store its physical address in the
; scratchpad buffer array.
        mov     edi, [esi+xhci_controller.CommonPage-XCD]
        add     edi, XHCI_SCRATCH_OFS
        mov     ebx, ecx
.loop:
        call    xhci_alloc_page
        test    eax, eax
        jz      .fail
        mov     [edi], edx
        and     dword [edi+4], 0
        add     edi, 8
        dec     ebx
        jnz     .loop
; 3. The first entry of the DCBAA points to the scratchpad buffer array.
        mov     eax, [esi+xhci_controller.CommonPhys-XCD]
        add     eax, XHCI_SCRATCH_OFS
        mov     edi, [esi+xhci_controller.CommonPage-XCD]
        mov     [edi], eax
        and     dword [edi+4], 0
.ok:
        movi    eax, 1
.nothing:
        pop     edi edx ecx ebx
        ret
.fail:
        dbgstr 'XHCI: cannot allocate scratchpad buffers'
        xor     eax, eax
        jmp     .nothing
endp

; Controller-specific initialization.
; in: eax -> xhci_controller to initialize, [ebp-4] = (bus shl 8) + devfn.
; out: eax = 0 on failure, otherwise eax -> usb_controller.
proc xhci_init
.devfn   equ ebp - 4
.bus     equ ebp - 3
        push    ebx esi edi
        lea     esi, [eax+XCD]          ; esi -> usb_controller
; 1. Initialize the static heads of the three pipe lists and the two mutexes.
        lea     edi, [esi+xhci_controller.ControlED-XCD]
        invoke  usbhc_api.usb_init_static_endpoint
        lea     edi, [esi+xhci_controller.BulkED-XCD]
        invoke  usbhc_api.usb_init_static_endpoint
        lea     edi, [esi+xhci_controller.IntED-XCD]
        invoke  usbhc_api.usb_init_static_endpoint
        lea     ecx, [esi+xhci_controller.CmdLock-XCD]
        invoke  MutexInit
        lea     ecx, [esi+xhci_controller.EventLock-XCD]
        invoke  MutexInit
; 2. Enable memory space and bus master access; also make sure the legacy
; interrupt line is not disabled, since this driver does not use MSI.
        invoke  PciRead16, dword [.bus], dword [.devfn], 4
        or      al, 6
        and     ah, not 4               ; clear Interrupt Disable
        invoke  PciWrite16, dword [.bus], dword [.devfn], 4, eax
; 3. Map the registers. How much has to be mapped is only known after reading
; the capability registers, so map one page first.
        invoke  PciRead32, dword [.bus], dword [.devfn], 10h
        mov     ebx, eax
; The xHCI register BAR is normally 64 bits wide, and this driver lives in a
; 32-bit address space: a base assigned above 4G cannot be mapped at all, so
; refuse it with a message instead of mapping whatever the low dword aliases.
        test    bl, 100b                ; a 64-bit BAR?
        jz      @f
        push    ebx                     ; nothing survives a kernel call
        invoke  PciRead32, dword [.bus], dword [.devfn], 14h
        pop     ebx
        test    eax, eax
        jz      @f
        DEBUGF 1,'K : XHCI BAR above 4G (%x:%x), cannot serve this controller\n',eax,ebx
        jmp     .fail
@@:
        mov     eax, ebx
        and     al, not 0Fh
        mov     ebx, eax                ; ebx = physical base address
        invoke  MapIoMem, ebx, 0x1000, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .fail
        mov     edi, eax
        call    xhci_mmio_size
        mov     ecx, eax
; Exported kernel functions are not guaranteed to preserve anything, so keep
; the physical base and the size across the call on the stack.
        push    ebx ecx
        invoke  FreeKernelSpace, edi
        pop     ecx ebx
        push    ecx
        invoke  MapIoMem, ebx, ecx, PG_SW+PG_NOCACHE
        pop     ecx
        test    eax, eax
        jz      .fail
        mov     edi, eax
        mov     [esi+xhci_controller.MMIOBase-XCD], eax
        mov     [esi+xhci_controller.MMIOSize-XCD], ecx
        mov     ecx, [edi+XhciCapLengthReg]
        movzx   ecx, cl
        lea     eax, [edi+ecx]
        mov     [esi+xhci_controller.OpBase-XCD], eax
        mov     eax, [edi+XhciRuntimeOffReg]
        and     eax, not 1Fh
        add     eax, edi
        mov     [esi+xhci_controller.RtBase-XCD], eax
        mov     eax, [edi+XhciDoorbellOffReg]
        and     eax, not 3
        add     eax, edi
        mov     [esi+xhci_controller.DbBase-XCD], eax
        mov     eax, [edi+XhciCapParams1]
        mov     [esi+xhci_controller.CapParams1-XCD], eax
        movi    ecx, 32
        test    al, 4                   ; Context Size
        jz      @f
        movi    ecx, 64
@@:
        mov     [esi+xhci_controller.ContextSize-XCD], ecx
        mov     ebx, [edi+XhciStructParams1]
        movzx   eax, bl
        mov     [esi+xhci_controller.MaxSlots-XCD], eax
        mov     eax, ebx
        shr     eax, 24
        mov     [esi+xhci_controller.NumRealPorts-XCD], eax
        DEBUGF 1,'K : XHCI HCSPARAMS1=%x HCCPARAMS1=%x context size %d\n',ebx,[esi+xhci_controller.CapParams1-XCD],ecx
; 4. Stop the controller, then reset it.
        mov     edi, [esi+xhci_controller.OpBase-XCD]
        test    dword [edi+XhciStatusReg], XHCI_STS_HCH
        jnz     .stopped
        and     dword [edi+XhciCommandReg], not XHCI_CMD_RUN
        movi    ecx, 50
.stop_wait:
        push    ecx esi
        movi    esi, 1
        invoke  Sleep
        pop     esi ecx
        test    dword [edi+XhciStatusReg], XHCI_STS_HCH
        jnz     .stopped
        dec     ecx
        jnz     .stop_wait
        dbgstr 'XHCI: failed to stop the controller'
        jmp     .fail_unmap
.stopped:
        or      dword [edi+XhciCommandReg], XHCI_CMD_HCRST
        movi    ecx, 100
.reset_wait:
        push    ecx esi
        movi    esi, 1
        invoke  Sleep
        pop     esi ecx
        test    dword [edi+XhciCommandReg], XHCI_CMD_HCRST
        jnz     .reset_next
        test    dword [edi+XhciStatusReg], XHCI_STS_CNR
        jz      .reset_ok
.reset_next:
        dec     ecx
        jnz     .reset_wait
        dbgstr 'XHCI: failed to reset the controller'
        jmp     .fail_unmap
.reset_ok:
; 5. Allocate the structures that the controller reads.
        call    xhci_alloc_page
        test    eax, eax
        jz      .fail_unmap
        mov     [esi+xhci_controller.CommonPage-XCD], eax
        mov     [esi+xhci_controller.CommonPhys-XCD], edx
        call    xhci_alloc_page
        test    eax, eax
        jz      .fail_unmap
        mov     [esi+xhci_controller.CmdRing-XCD], eax
        mov     [esi+xhci_controller.CmdRingPhys-XCD], edx
        mov     [esi+xhci_controller.CmdCycle-XCD], XHCI_TRB_C
        and     [esi+xhci_controller.CmdEnqueue-XCD], 0
; The last entry of the command ring is a Link TRB back to its beginning.
        lea     ecx, [eax+(XHCI_CMD_TRBS-1)*16]
        mov     [ecx], edx
        mov     dword [ecx+12], (XHCI_TRB_LINK shl 10) + XHCI_TRB_TC
        call    xhci_alloc_page
        test    eax, eax
        jz      .fail_unmap
        mov     [esi+xhci_controller.EventRing-XCD], eax
        mov     [esi+xhci_controller.EventRingPhys-XCD], edx
        mov     [esi+xhci_controller.EventCycle-XCD], XHCI_TRB_C
        and     [esi+xhci_controller.EventDequeue-XCD], 0
        call    xhci_alloc_page
        test    eax, eax
        jz      .fail_unmap
        mov     [esi+xhci_controller.InputCtx-XCD], eax
        mov     [esi+xhci_controller.InputCtxPhys-XCD], edx
; 6. Program the number of device slots to enable.
        mov     eax, [esi+xhci_controller.MaxSlots-XCD]
        test    eax, eax
        jz      .fail_unmap
        cmp     eax, 255
        jbe     @f
        movi    eax, 255
@@:
        mov     [esi+xhci_controller.MaxSlots-XCD], eax
        mov     edi, [esi+xhci_controller.OpBase-XCD]
        mov     [edi+XhciConfigReg], eax
; 7. The scratchpad buffers, then the Device Context Base Address Array.
        call    xhci_init_scratchpad
        test    eax, eax
        jz      .fail_unmap
        mov     edi, [esi+xhci_controller.OpBase-XCD]
        add     edi, XhciDcbaapReg
        mov     eax, [esi+xhci_controller.CommonPhys-XCD]
        call    xhci_write64
; 8. The command ring.
        mov     edi, [esi+xhci_controller.OpBase-XCD]
        add     edi, XhciCmdRingReg
        mov     eax, [esi+xhci_controller.CmdRingPhys-XCD]
        or      eax, 1                  ; Ring Cycle State
        call    xhci_write64
; 9. The event ring: one segment described by a single ERST entry.
        mov     eax, [esi+xhci_controller.CommonPage-XCD]
        mov     edx, [esi+xhci_controller.EventRingPhys-XCD]
        mov     [eax+XHCI_ERST_OFS], edx
        and     dword [eax+XHCI_ERST_OFS+4], 0
        mov     dword [eax+XHCI_ERST_OFS+8], XHCI_EVENT_TRBS
        and     dword [eax+XHCI_ERST_OFS+12], 0
        mov     ecx, [esi+xhci_controller.RtBase-XCD]
        mov     dword [ecx+XhciErstSize0Reg], 1
        lea     edi, [ecx+XhciErdp0Reg]
        mov     eax, [esi+xhci_controller.EventRingPhys-XCD]
        call    xhci_write64
        mov     ecx, [esi+xhci_controller.RtBase-XCD]
        lea     edi, [ecx+XhciErstBase0Reg]
        mov     eax, [esi+xhci_controller.CommonPhys-XCD]
        add     eax, XHCI_ERST_OFS
        call    xhci_write64
; 10. Build the map of the ports to serve.
; This is done before hooking the interrupt: it is the last step that can
; fail, and there is no way to detach a handler once attached.
        mov     edi, [esi+xhci_controller.MMIOBase-XCD]
        call    xhci_parse_protocols
        cmp     [esi+usb_controller.NumPorts], 0
        jnz     @f
if XHCI_SUPERSPEED
        dbgstr 'XHCI: no ports, nothing to serve'
else
        dbgstr 'XHCI: no USB2 ports, nothing to serve'
end if
        jmp     .fail_unmap
@@:
; 11. Hook the interrupt. An interrupt line of 0 or 255 means the firmware has
; assigned none: the kernel would reject such a number anyway, and the driver
; has to fall back to polling in ProcessDeferred.
        invoke  PciRead8, dword [.bus], dword [.devfn], 3Ch
        movzx   eax, al
        test    al, al
        jz      .no_irq
        cmp     al, 0FFh
        jz      .no_irq
        invoke  AttachIntHandler, eax, xhci_irq, esi
        test    eax, eax
        jnz     .irq_done
.no_irq:
        mov     [esi+xhci_controller.NoIrq-XCD], 1
        dbgstr 'XHCI: no usable interrupt line, falling back to polling'
; Nobody will ever acknowledge an interrupt, so the controller must not raise
; one: otherwise INTx would stay asserted on a line the system knows nothing
; about. Set the PCI Interrupt Disable bit as well.
        invoke  PciRead16, dword [.bus], dword [.devfn], 4
        or      ah, 4
        invoke  PciWrite16, dword [.bus], dword [.devfn], 4, eax
.irq_done:
; 12. Configure the primary interrupter; leave interrupts disabled when the
; controller works in polling mode.
        mov     edi, [esi+xhci_controller.RtBase-XCD]
        mov     dword [edi+XhciImod0Reg], 4000  ; about one millisecond
        movi    eax, 1                          ; just clear the pending bit
        cmp     [esi+xhci_controller.NoIrq-XCD], 0
        jnz     @f
        movi    eax, 3                          ; clear pending, enable
@@:
        mov     [edi+XhciIman0Reg], eax
; 13. Run the controller.
        mov     eax, XHCI_CMD_RUN + XHCI_CMD_INTE + XHCI_CMD_HSEE
        cmp     [esi+xhci_controller.NoIrq-XCD], 0
        jz      @f
        movi    eax, XHCI_CMD_RUN
@@:
        mov     edi, [esi+xhci_controller.OpBase-XCD]
        mov     [edi+XhciCommandReg], eax
if XHCI_SUPERSPEED
        DEBUGF 1,'K : XHCI controller at %x:%x with %d ports initialized, SS mask %x\n',[.bus]:2,[.devfn]:2,[esi+usb_controller.NumPorts],[esi+xhci_controller.SSPortMask-XCD]
else
        DEBUGF 1,'K : XHCI controller at %x:%x with %d USB2 ports initialized\n',[.bus]:2,[.devfn]:2,[esi+usb_controller.NumPorts]
end if
; In polling mode nothing would wake the USB thread by itself, so create one
; global timer that keeps kicking it; the thread then polls the event ring and
; the ports of every controller on each tick.
        cmp     [esi+xhci_controller.NoIrq-XCD], 0
        jz      .no_heartbeat
        cmp     [xhci_heartbeat_timer], 0
        jnz     .no_heartbeat
        invoke  TimerHS, 1, 1, xhci_heartbeat, 0
        mov     [xhci_heartbeat_timer], eax
        test    eax, eax
        jnz     .no_heartbeat
        dec     [xhci_heartbeat_timer] ; -1: tried and failed, do not retry
.no_heartbeat:
; 14. Apply power to every served port.
        xor     ecx, ecx
.power_loop:
        cmp     ecx, [esi+usb_controller.NumPorts]
        jae     .powered
        call    xhci_port_reg
        mov     dword [edx], XHCI_PORT_PP
        inc     ecx
        jmp     .power_loop
.powered:
        push    esi
        movi    esi, 20
        invoke  Sleep
        pop     esi
; 15. Report the state of every port and force a scan on the first pass, so
; that devices already plugged in at boot are noticed even if their connect
; event was generated before the event ring became operational.
        xor     ecx, ecx
.dump_loop:
        cmp     ecx, [esi+usb_controller.NumPorts]
        jae     .dumped
        call    xhci_port_reg
        mov     eax, [edx]
        mov     ebx, [esi+xhci_controller.PortMap-XCD+ecx*4]
        DEBUGF 1,'K : XHCI port %d (real %d) PORTSC=%x\n',ecx,ebx,eax
; A device that was already plugged in at boot may have signalled its connect
; before the event ring became operational, so trust the connect status bit
; rather than the change bit here.
        test    al, XHCI_PORT_CCS
        jz      .dump_next
        push    ecx
        invoke  GetTimerTicks
        pop     ecx
        mov     [esi+usb_controller.ConnectedTime+ecx*4], eax
        bts     [esi+usb_controller.NewConnected], ecx
.dump_next:
        inc     ecx
        jmp     .dump_loop
.dumped:
        mov     [esi+xhci_controller.PortChanged-XCD], 1
; 16. Return the pointer to usb_controller.
        mov     eax, esi
        pop     edi esi ebx
        ret
.fail_unmap:
        invoke  FreeKernelSpace, [esi+xhci_controller.MMIOBase-XCD]
.fail:
        xor     eax, eax
        pop     edi esi ebx
        ret
endp

; Periodic timer callback used when no controller has an interrupt line;
; it only kicks the USB thread, the real work happens in ProcessDeferred.
proc xhci_heartbeat stdcall uses ebx, arg:dword
        movi    ebx, 1          ; the wakeup only happens when ebx is set
        invoke  usbhc_api.usb_wakeup_if_needed
        ret
endp

; IRQ handler for xHCI controllers.
proc xhci_irq
        push    ebx esi edi
virtual at esp
        rd      3       ; saved registers
        dd      ?       ; return address
.controller     dd      ?
end virtual
        mov     esi, [.controller]
        mov     edi, [esi+xhci_controller.OpBase-XCD]
        spin_lock_irqsave [esi+usb_controller.WaitSpinlock]
        mov     eax, [edi+XhciStatusReg]
; Only the bits this driver cares about are considered; if none of them is set,
; the interrupt belongs to another device sharing the same line.
        test    eax, XHCI_STS_EINT + XHCI_STS_PCD + XHCI_STS_HSE + XHCI_STS_HCE
        jz      .noint
; Acknowledge the status bits, then the interrupter itself.
        mov     [edi+XhciStatusReg], eax
        or      [esi+xhci_controller.DeferredActions-XCD], eax
        mov     edi, [esi+xhci_controller.RtBase-XCD]
        mov     edx, [edi+XhciIman0Reg]
        mov     [edi+XhciIman0Reg], edx         ; write one to clear
        spin_unlock_irqrestore [esi+usb_controller.WaitSpinlock]
; usb_wakeup_if_needed only wakes the USB thread when ebx is nonzero; without
; that the events collected above would wait for the next polling timeout, and
; would never be noticed at all once the thread asks for an infinite one.
        movi    ebx, 1
        invoke  usbhc_api.usb_wakeup_if_needed
        movi    eax, 1
        pop     edi esi ebx
        ret
.noint:
        spin_unlock_irqrestore [esi+usb_controller.WaitSpinlock]
        xor     eax, eax
        pop     edi esi ebx
        ret
endp

section '.data' readable writable
include '../../peimport.inc'
include_debug_strings
IncludeIGlobals
IncludeUGlobals
align 4
usbhc_api usbhc_func
