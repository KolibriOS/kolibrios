; HID over I2C pointing device driver (I2C touchpads and mice).
;
; Modern laptop touchpads sit on an I2C bus of the SoC (Intel LPSS with a
; Synopsys DesignWare I2C master inside) and talk the Microsoft "HID over I2C"
; protocol. This driver:
;   1. finds Intel LPSS I2C controllers on the PCI bus (class 0C80, vendor
;      8086) and checks the DesignWare component signature;
;   2. scans the bus for a device answering a HID descriptor request at the
;      common register addresses 0x0001 and 0x0020;
;   3. reads and parses the report descriptor, looking for the mouse
;      compatibility report (relative X/Y + buttons + wheel) that every
;      Windows Precision touchpad is required to expose;
;   4. powers the device on, resets it and polls the input register from a
;      kernel timer, feeding movement into SetMouseData.
;
; What is intentionally NOT here yet:
;   - Intel controllers in "ACPI mode" (hidden from PCI) - their MMIO base is
;     only described in the ACPI DSDT, which KolibriOS cannot parse. AMD
;     parts, whose masters are never on PCI, are handled from a table of
;     the fixed FCH addresses instead (detect_amd_fixed);
;   - GPIO interrupts - polling every timer tick (10 ms) is used instead;
;   - multitouch/gestures: the raw touchpad collection (absolute fingers) is
;     skipped, only the mouse compatibility report is used.

format PE DLL native
entry start

API_VERSION     = 1

; Debug board output. fdo.inc prints a DEBUGF only when its level is not
; below __DEBUG_LEVEL__, so the lower the build level, the more gets out:
;   1 - bring-up chatter: probed addresses, full descriptors, report and
;       touch frame dumps, silence recovery steps. Kilobytes per boot and a
;       steady stream while the pad is used; build with it for a new pad.
;   2 - the normal log: load, the controller and device found, settings,
;       and every error. A few lines per boot.
__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 2

section '.reloc' data readable discardable fixups
section '.text' code readable executable

include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'


; --------------------------- HID over I2C ------------------------------------
; HID descriptor layout (all fields little-endian words).
HIDD_wHIDDescLength       = 0
HIDD_bcdVersion           = 2
HIDD_wReportDescLength    = 4
HIDD_wReportDescRegister  = 6
HIDD_wInputRegister       = 8
HIDD_wMaxInputLength      = 10
HIDD_wOutputRegister      = 12
HIDD_wMaxOutputLength     = 14
HIDD_wCommandRegister     = 16
HIDD_wDataRegister        = 18
HIDD_wVendorID            = 20
HIDD_wProductID           = 22
HIDD_wVersionID           = 24

I2CHID_DESC_LEN     = 30
I2CHID_OP_RESET     = 0x01
I2CHID_OP_SET_POWER = 0x08
I2CHID_PWR_ON       = 0x00

; After this many consecutive failed polls the device is considered gone and
; the timer is stopped: a dead bus must not cost an I2C timeout on every
; single tick forever.
MAX_POLL_ERRORS     = 100

; Polls of silence before the next recovery step is tried (20 ms each).
SILENT_LIMIT        = 150

; ---------------------------- Settings -------------------------------------
; Everything below has a built-in default and can be overridden from the
; [touchpad] section of /sys/settings/system.ini. The file is read once at
; startup and again whenever someone asks the service to reload it, so a
; change takes effect without a reboot.
;
; Pointer SPEED is deliberately NOT a setting here: relative movement goes
; through the kernel's own mouse acceleration, which the existing
; [mouse] speed and acceleration keys already control. A second speed knob
; in the driver would only fight with them.
;
; A finger that rests or presses is never perfectly still: its contact
; patch shifts by a handful of units per frame, and following that puts
; the cursor somewhere else by the time the click lands. So the reported
; position must move CFG_JITTER units away from the position the pointer
; stands on before the pointer follows at all - about half a millimetre on
; a typical pad, the margin libinput uses for the same reason. Wobble stays
; inside the margin and moves nothing; deliberate travel crosses it and
; then tracks one-to-one. This also gives the slightly stepped feel of a
; Windows touchpad rather than the smooth but jittery tracking of a raw
; one-to-one driver. 0 turns the filter off.
CFG_JITTER          = 12
; The same margin for a finger that landed on the button strip, where it
; is pressing rather than steering and wobbles more. Kept separate because
; what feels right for tracking is too loose for aiming at a button.
CFG_JITTER_BTN      = 20
; Pointer speed in percent: 100 is one pixel per two pad units; the
; remainder is carried over so slow travel is not lost to the division.
; This is on top of the kernel's own mouse speed/acceleration, and exists
; only because pads differ in resolution.
CFG_SPEED           = 100
; Pad units of two-finger travel per wheel step.
CFG_SCROLL_STEP     = 64
; Reverse the two-finger scrolling direction.
CFG_SCROLL_INVERT   = 0
; Turn a short, still touch into a click: one finger gives the left
; button, two the right one, three the middle one.
CFG_TAP             = 1
; Tap, then touch again straight away and move: the button stays down for
; as long as that second touch lasts. This is how text is selected and how
; one draws in a paint program without holding the pad pressed.
CFG_TAP_DRAG        = 1
; How long after a tap the button stays down waiting for the finger to
; come back and drag, 1/100 s. A tap is not released at once: were it, a
; tap followed by a touch-and-hold would reach the kernel as two presses
; - a double click - instead of one press held through the drag, which is
; what every touchpad driver does (libinput's tap-and-drag timeout is the
; same 300 ms). A touch arriving in the window continues the hold; none
; arriving releases it at the end of the window.
DRAG_WINDOW         = 20
; How long a right or middle tap click stays pressed, in poll intervals.
; Applications read the button state some time after the kernel tells
; them it changed; a press that is gone by then was never seen.
TAP_PULSE           = 10
; For this long after touch-down (1/100 s) the jitter margin is doubled:
; a finger rolls a little as it lands and again as it leaves, and during a
; tap that roll would otherwise move the pointer off the target.
TAP_SETTLE          = 8
; Pointer motion of more than this many pixels in one frame is logged as a
; jump, a few times.
JUMP_PX             = 120
; How a right click is produced on a clickpad, where the firmware reports
; every press as button 1 and the host has to decide what it meant:
;   0 - never, every press is a left click
;   1 - a press in the bottom-right corner of the surface
;   2 - a press with two fingers down, wherever it is (three fingers then
;       give the middle button)
;   3 - either of the two
CFG_RIGHT_CLICK     = 1
; Geometry of the button strip along the bottom edge, in percent of the
; surface, so it means the same thing on a pad of any size: the height of
; the strip and the point where its left half ends and the right one
; begins. The strip decides which button a press means and refuses taps;
; pointer motion behaves identically over the whole surface.
CFG_BTN_ZONE_PCT    = 12
CFG_RIGHT_SPLIT_PCT = 50
; Fallback surface extents, used only when the report descriptor does not
; declare any - the zones then land where they did on the pad this driver
; was written against.
PAD_MAX_X_DEF       = 1707
PAD_MAX_Y_DEF       = 1060

; A tap is a touch shorter than this (1/100 s) that moved less than
; TAP_MAX_DIST pad units.
TAP_MAX_TICKS       = 30
TAP_MAX_DIST        = 64

; The configuration file is read into a buffer of this size.
INI_MAX             = 4096

MAX_INPUT_BUF       = 256
MAX_REPDESC         = 2048
POLL_DELAY_HS       = 1         ; 1/100 s between polls (10 ms)
DRAIN_MAX           = 8         ; reports collected per poll at most
STATS_AT_POLLS      = 500       ; first line of polling statistics after 5 s
STATS_EVERY_POLLS   = 3000      ; and one every 30 s after that

; =============================================================================
proc start
; The kernel loader keeps the driver's entry address in esi and the image
; base in ebx ACROSS this call and stores them into the service descriptor
; afterwards (load_pe_driver) - and stop_all_services later CALLS that stored
; entry at shutdown. START must therefore preserve the callee-saved
; registers, or the kernel records garbage and the machine page-faults on
; every shutdown. (CreateThread alone clobbers esi inside the kernel.)
        push    ebx esi edi
virtual at esp
                rd      3       ; saved ebx esi edi
                dd      ?       ; return address
.reason         dd      ?       ; DRV_ENTRY or DRV_EXIT
.cmdline        dd      ?
end virtual
        cmp     [.reason], DRV_ENTRY
        jnz     .fail
        DEBUGF 2, "i2chid: loading\n"
        call    read_config
        mov     [XferTimeout], DW_TIMEOUT_INIT
        call    detect_hw
        test    eax, eax
        jz      .fail
; Polling runs in a thread of its own rather than in a TimerHS callback:
; the kernel invokes timer callbacks WITH the timer list lock held, and a
; full-length input read is about 3 ms of bus time - holding that lock for
; 3 ms out of every 20 would put jitter into every other timer in the
; system. In a thread the transfer costs only this thread's own slice, and
; Delay yields through an event instead of spinning.
        mov     [XferTimeout], DW_TIMEOUT_POLL
        movi    ebx, 1
        mov     ecx, i2chid_thread
        xor     edx, edx
        invoke  CreateThread
        test    eax, eax
        jns     @f
        DEBUGF 2, "i2chid: cannot create poll thread (%d)\n", eax
        jmp     .fail
@@:
        DEBUGF 2, "i2chid: polling started\n"
        invoke  RegService, my_service, service_proc
        pop     edi esi ebx
        ret
.fail:
        xor     eax, eax
        pop     edi esi ebx
        ret
endp

; The service the driver registers. Besides reporting its version it lets
; anything in the system ask for the configuration file to be read again,
; which is how a settings change takes effect without a reboot: edit
; /sys/settings/system.ini, then send this.
SRV_GET_VERSION = 0
SRV_RELOAD_CONF = 1

proc service_proc stdcall, ioctl:dword
        mov     edi, [ioctl]
        mov     eax, [edi+IOCTL.io_code]
        cmp     eax, SRV_RELOAD_CONF
        je      .reload
        cmp     eax, SRV_GET_VERSION
        jne     .err
        cmp     [edi+IOCTL.out_size], 4
        jb      .err
        mov     edi, [edi+IOCTL.output]
        mov     dword [edi], API_VERSION
        xor     eax, eax
        ret
.reload:
; Settings are single dwords and the polling thread only ever reads them,
; so they can be replaced underneath it without any locking.
        call    read_config
        xor     eax, eax
        ret
.err:
        or      eax, -1
        ret
endp

; Looks up one integer setting inside the [touchpad] section that locate_section
; found. Returns the default when the file, the section or the key is absent,
; so a half-written configuration can never leave the driver in a strange
; state.
; in: esi -> zero-terminated key name, edx = default.
; out: eax = value.
proc ini_int stdcall uses ebx ecx edx esi edi, key:dword, defval:dword
        mov     eax, [defval]
        mov     ebx, [IniEnd]
        test    ebx, ebx
        jz      .done                   ; no section, no settings
; Measure the key.
        mov     edi, [key]
        xor     ecx, ecx
@@:
        cmp     byte [edi+ecx], 0
        jz      @f
        inc     ecx
        jmp     @b
@@:
        mov     esi, [IniStart]
        sub     ebx, ecx                ; last offset worth testing
.scan:
        cmp     esi, ebx
        ja      .done
        push    ecx esi
        mov     edi, [key]
        repe cmpsb
        pop     esi ecx
        jz      .found
        inc     esi
        jmp     .scan
.found:
        add     esi, ecx
; Step over the separator to the number. Give up if no digit turns up soon,
; so a key mentioned in a comment cannot send the parser wandering.
        mov     ecx, 8
.seek:
        mov     al, [esi]
        cmp     al, '0'
        jb      @f
        cmp     al, '9'
        jbe     .number
@@:
        cmp     al, 13                  ; not past the end of the line
        je      .done_def
        cmp     al, 10
        je      .done_def
        inc     esi
        dec     ecx
        jnz     .seek
.done_def:
        mov     eax, [defval]
        jmp     .done
.number:
        xor     eax, eax
.digits:
        movzx   edx, byte [esi]
        cmp     dl, '0'
        jb      .done
        cmp     dl, '9'
        ja      .done
        sub     dl, '0'
        imul    eax, 10
        add     eax, edx
        inc     esi
        cmp     eax, 100000             ; nonsense guard
        jb      .digits
        mov     eax, [defval]
.done:
        ret
endp

; Reads the configuration file and locates the driver's own section in it,
; then applies every setting. Called at startup and again whenever the
; service is asked to reload, so editing the file and asking for a reload is
; enough - no reboot, no reloading the driver.
proc read_config uses ebx esi edi
        mov     [IniStart], 0
        mov     [IniEnd], 0
        mov     ebx, ini_req
        invoke  FS_Service
; Reading a whole file into a larger buffer ends at the end of the file, and
; the file system reports that as error 6 - which is a success here.
        test    eax, eax
        jz      @f
        cmp     eax, 6
        jne     .apply
@@:
        test    ebx, ebx                ; ebx = bytes actually read
        jz      .apply
        cmp     ebx, INI_MAX
        jbe     @f
        mov     ebx, INI_MAX
@@:
        add     ebx, ini_buf            ; end of the text
        call    locate_section
.apply:
        stdcall ini_int, key_speed, CFG_SPEED
        cmp     eax, 10
        jb      @f
        cmp     eax, 1000
        jbe     .speed_ok
@@:
        movi    eax, CFG_SPEED
.speed_ok:
        mov     [Speed], eax
        stdcall ini_int, key_jitter, CFG_JITTER
        mov     [Jitter], eax
        stdcall ini_int, key_jitter_btn, CFG_JITTER_BTN
        mov     [JitterBtn], eax
        stdcall ini_int, key_scroll_step, CFG_SCROLL_STEP
        test    eax, eax
        jnz     @f
        movi    eax, 1                  ; a zero step would divide by zero
@@:
        mov     [ScrollStep], eax
        stdcall ini_int, key_scroll_invert, CFG_SCROLL_INVERT
        mov     [ScrollInvert], eax
        stdcall ini_int, key_tap, CFG_TAP
        mov     [TapEnable], eax
        stdcall ini_int, key_tap_drag, CFG_TAP_DRAG
        mov     [TapDrag], eax
        stdcall ini_int, key_right_click, CFG_RIGHT_CLICK
        mov     [RightClick], eax
        stdcall ini_int, key_btn_zone, CFG_BTN_ZONE_PCT
        cmp     eax, 100
        jbe     @f
        movi    eax, CFG_BTN_ZONE_PCT
@@:
        mov     [BtnZonePct], eax
        stdcall ini_int, key_right_split, CFG_RIGHT_SPLIT_PCT
        cmp     eax, 100
        jbe     @f
        movi    eax, CFG_RIGHT_SPLIT_PCT
@@:
        mov     [RightSplitPct], eax
        call    apply_geometry
        DEBUGF 2, "i2chid: settings speed %u, jitter %u/%u, scroll %u/%u, tap %u/%u, rclick %u, zones %u%/%u%\n", \
                [Speed], [Jitter], [JitterBtn], [ScrollStep], [ScrollInvert], [TapEnable], \
                [TapDrag], [RightClick], [BtnZonePct], [RightSplitPct]
        ret
endp

; Finds the driver's section in the text and remembers where its body starts
; and ends, so that a key of the same name in another section cannot be
; picked up by mistake.
; in: ebx -> end of the text.
proc locate_section uses ebx ecx esi edi
        mov     esi, ini_buf
        mov     ecx, sec_name_len
        sub     ebx, ecx
.scan:
        cmp     esi, ebx
        ja      .nothing
        push    ecx esi
        mov     edi, sec_name
        repe cmpsb
        pop     esi ecx
        jz      .found
        inc     esi
        jmp     .scan
.found:
        add     esi, ecx
        mov     [IniStart], esi
; The section ends where the next one begins.
        add     ebx, ecx                ; the real end of the text again
.find_end:
        cmp     esi, ebx
        jae     .at_end
        cmp     byte [esi], '['
        je      .at_end
        inc     esi
        jmp     .find_end
.at_end:
        mov     [IniEnd], esi
.nothing:
        ret
endp

; Turns the zone percentages into pad coordinates. Separate from reading the
; settings because the surface extents only become known when the report
; descriptor has been parsed, which happens later - and because a reload has
; to redo this arithmetic with the extents already in hand.
proc apply_geometry uses ebx ecx edx
        mov     ecx, [t_padmaxx]
        test    ecx, ecx
        jnz     @f
        mov     ecx, PAD_MAX_X_DEF
@@:
        mov     eax, ecx
        mul     [RightSplitPct]
        mov     ecx, 100
        div     ecx
        mov     [RightSplitX], eax
        mov     ecx, [t_padmaxy]
        test    ecx, ecx
        jnz     @f
        mov     ecx, PAD_MAX_Y_DEF
@@:
; The strip is measured from the bottom, so the threshold is what is left
; above it.
        mov     eax, 100
        sub     eax, [BtnZonePct]
        mul     ecx
        mov     ecx, 100
        div     ecx
        mov     [BtnZoneY], eax
        ret
endp

; =============================================================================
; Walks the PCI list; every Intel controller of class 0C80xx (serial bus,
; other - that is how LPSS I2C hosts present themselves) is brought up and
; its bus scanned, until a HID device with a usable mouse report is found.
; out: eax = 1 on success.
proc detect_hw uses ebx esi edi
        mov     [CtrlCount], 0
        invoke  GetPCIList
        mov     [PciHead], eax
        mov     esi, eax
.next:
        mov     esi, [esi+PCIDEV.fd]
        cmp     esi, [PciHead]
        je      .none
        mov     eax, [esi+PCIDEV.vendor_device_id]
        cmp     ax, 0x8086
        jne     .next
        mov     eax, [esi+PCIDEV.class]
        shr     eax, 8
; Intel files this block under two different class codes depending on the
; platform: the PCH-based parts (Skylake and newer) call it a serial bus
; controller, while the Atom-derived SoCs (Apollo Lake, Gemini Lake) call it
; a signal processing controller. Both are the same LPSS wrapper around a
; DesignWare I2C master, and the component signature check below is what
; actually decides.
        cmp     eax, 0x0C80
        je      @f
        cmp     eax, 0x1180
        jne     .next
@@:
        movzx   eax, byte [esi+PCIDEV.bus]
        mov     [PciBus], eax
        movzx   eax, byte [esi+PCIDEV.devfn]
        mov     [PciDevfn], eax
        mov     eax, [esi+PCIDEV.vendor_device_id]
        inc     [CtrlCount]
        DEBUGF 1, "i2chid: candidate %x at PCI bus %x devfn %x\n", eax, [PciBus], [PciDevfn]
        call    setup_controller
        test    eax, eax
        jz      .next
        call    scan_bus
        test    eax, eax
        jnz     .found
        invoke  FreeKernelSpace, [I2cMmio]
        mov     [I2cMmio], 0
        jmp     .next
.none:
; Nothing on PCI. AMD machines keep their I2C masters off the PCI bus
; altogether, so give those a chance before giving up.
        call    detect_amd_fixed
        test    eax, eax
        jnz     .found
; A zero here means neither the PCI filter nor the AMD table matched
; anything at all, which is a very different problem from having tried
; controllers and found no device.
        DEBUGF 2, "i2chid: no usable HID over I2C device found (%u controllers tried)\n", [CtrlCount]
        xor     eax, eax
        ret
.found:
        movi    eax, 1
        ret
endp

; Enables the PCI device, puts it into D0, maps BAR0, releases the LPSS
; resets and verifies the DesignWare signature.
; out: eax = 1 on success ([I2cMmio] mapped).
; =============================================================================
; AMD platforms. The FCH's DesignWare I2C masters are not PCI functions: they
; sit at fixed MMIO addresses that an ACPI-aware OS reads out of the DSDT
; (AMDI0010 devices). KolibriOS has no AML interpreter, but the addresses have
; been the same from Stoney Ridge through Renoir and Cezanne, so a table does.
; I2C2 and I2C3 come first: on Picasso and later the firmware keeps I2C0 and
; I2C1 for the PSP and the touchpad hangs off one of the other two, so a hit
; there means the PSP buses are never touched; older parts expose all four.
; out: eax = 1 when a device was found and set up.
proc detect_amd_fixed uses ebx esi edi
        invoke  PciRead16, 0, 0, 0     ; host bridge vendor
        cmp     ax, 0x1022
        jne     .none
        mov     esi, amd_i2c_bases
.next:
        mov     eax, [esi]
        test    eax, eax
        jz      .none
        mov     [FixedBase], eax
        inc     [CtrlCount]
        DEBUGF 2, "i2chid: AMD FCH I2C candidate at %x\n", eax
        call    setup_fixed_controller
        test    eax, eax
        jz      .skip
        call    scan_bus
        test    eax, eax
        jnz     .found
        invoke  FreeKernelSpace, [I2cMmio]
        mov     [I2cMmio], 0
.skip:
        add     esi, 4
        jmp     .next
.none:
        xor     eax, eax
        ret
.found:
        movi    eax, 1
        ret
endp

; Maps one fixed-address controller and checks that a DesignWare master is
; really there. A block that reads back as all ones or all zeroes is powered
; down; the FCH's always-on domain (AOAC) can switch it on, which is what the
; firmware or an ACPI _PS0 would do on the touchpad's behalf.
; out: eax = 1 on success, [I2cMmio] mapped and initialised.
proc setup_fixed_controller uses ebx esi edi
        invoke  MapIoMem, [FixedBase], 0x1000, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .no_map
        mov     [I2cMmio], eax
        mov     ebx, eax
        mov     eax, [ebx+DwIcCompType]
        cmp     eax, DW_COMP_TYPE_VALUE
        je      .good_hw
        DEBUGF 2, "i2chid:   signature reads %x, asking AOAC to power the block on\n", eax
        call    amd_aoac_power_on
        mov     eax, [ebx+DwIcCompType]
        cmp     eax, DW_COMP_TYPE_VALUE
        jne     .bad_hw
.good_hw:
        mov     [DwHcnt], DW_AMD_FS_HCNT
        mov     [DwLcnt], DW_AMD_FS_LCNT
        mov     [DwSdaHold], DW_AMD_SDA_HOLD
        call    dw_ctrl_init
        DEBUGF 2, "i2chid:   DesignWare I2C at %x, FIFO param %x\n", [FixedBase], [ebx+DwIcCompParam1]
        movi    eax, 1
        ret
.bad_hw:
        DEBUGF 2, "i2chid:   still no DW_apb_i2c signature, got %x\n", eax
        invoke  FreeKernelSpace, [I2cMmio]
        mov     [I2cMmio], 0
        jmp     .fail
.no_map:
        DEBUGF 2, "i2chid:   MapIoMem failed for %x\n", [FixedBase]
.fail:
        xor     eax, eax
        ret
endp

; Powers one FCH I2C block up through the AOAC registers: device number
; 5 + n for I2Cn, a control byte at 40h + 2n (bit 3 = power on, bits 1:0 =
; target state D0) and a status byte right after it whose bits 1:0 report
; power and clock as ready. The register page lives at FED81E00h.
proc amd_aoac_power_on uses ebx esi edi
        invoke  MapIoMem, 0xFED81000, 0x1000, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .done
        mov     esi, eax
        mov     eax, [FixedBase]
        sub     eax, 0xFEDC2000
        shr     eax, 12                 ; n
        add     eax, 5                  ; AOAC device number
        lea     edi, [esi+0xE40+eax*2]  ; D3 control byte
        mov     al, [edi]
        or      al, 8
        and     al, not 3
        mov     [edi], al
; Give it a moment and see whether power and clock come up.
        mov     ecx, 20
.wait:
        push    ecx
        mov     esi, 1
        invoke  Sleep
        pop     ecx
        mov     al, [edi+1]
        and     al, 3
        cmp     al, 3
        je      .up
        dec     ecx
        jnz     .wait
        DEBUGF 2, "i2chid:   AOAC did not report the block ready\n"
        jmp     .unmap
.up:
        DEBUGF 2, "i2chid:   AOAC reports the block powered and clocked\n"
.unmap:
        mov     esi, edi
        and     esi, not 0xFFF
        invoke  FreeKernelSpace, esi
.done:
        ret
endp

proc setup_controller uses ebx esi edi
; Power management first: force D0 (firmware parks unused LPSS devices in
; D3, and that is the norm on UEFI machines where no OS driver ever touched
; them). The D3hot->D0 transition soft-resets the function, which CLEARS the
; command register - so memory space decode may only be enabled AFTER the
; power-up, never before, or the enable is silently lost and every MMIO read
; returns FFFFFFFF.
        invoke  PciRead16, [PciBus], [PciDevfn], 6
        test    eax, 0x10               ; capabilities list present?
        jz      .pm_done
        invoke  PciRead8, [PciBus], [PciDevfn], 0x34
        and     eax, 0xFC
        mov     esi, eax
        mov     edi, 48                 ; loop guard
.cap_loop:
        test    esi, esi
        jz      .pm_done
        invoke  PciRead8, [PciBus], [PciDevfn], esi
        cmp     al, 1                   ; PCI PM capability
        je      .pm_cap
        lea     eax, [esi+1]
        invoke  PciRead8, [PciBus], [PciDevfn], eax
        and     eax, 0xFC
        mov     esi, eax
        dec     edi
        jnz     .cap_loop
        jmp     .pm_done
.pm_cap:
        lea     eax, [esi+4]            ; PMCSR
        invoke  PciRead16, [PciBus], [PciDevfn], eax
        test    eax, 3
        jz      .pm_done
        and     eax, not 3              ; -> D0
        mov     ebx, eax
        lea     eax, [esi+4]
        invoke  PciWrite16, [PciBus], [PciDevfn], eax, ebx
        mov     esi, 10
        invoke  Sleep                   ; D3->D0 settle time, 10 ms per PCI PM
.pm_done:
; Now that the device is awake, enable memory space decode.
        invoke  PciRead16, [PciBus], [PciDevfn], 4
        or      eax, 2
        invoke  PciWrite16, [PciBus], [PciDevfn], 4, eax
; BAR0.
        invoke  PciRead32, [PciBus], [PciDevfn], 0x10
        cmp     eax, 0xFFFFFFFF
        je      .no_bar                 ; nothing responds to config reads
        test    eax, 1
        jnz     .io_bar                 ; I/O BAR - not ours
        mov     ebx, eax
        and     ebx, 0xFFFFFFF0
        jz      .no_bar
        and     eax, 6
        cmp     eax, 4                  ; 64-bit BAR: upper half must be zero
        jne     @f
        invoke  PciRead32, [PciBus], [PciDevfn], 0x14
        test    eax, eax
        jnz     .bar_high
@@:
        invoke  MapIoMem, ebx, 0x1000, PG_SW+PG_NOCACHE
        test    eax, eax
        jz      .no_map
        mov     [I2cMmio], eax
        mov     ebx, eax
; A block that is already out of reset identifies itself right away. All
; zeroes means a block held in reset - only then is the LPSS wrapper asked to
; deassert the function and DMA resets (private registers at +200h, RESETS
; at +04h). Any other value means a live device that is not a DesignWare
; I2C master at all (the class filter is not exact), and its registers must
; not be written.
        mov     eax, [ebx+DwIcCompType]
        cmp     eax, DW_COMP_TYPE_VALUE
        je      .good_hw
        test    eax, eax
        jnz     .bad_hw
        mov     dword [ebx+0x204], 7
        mov     eax, [ebx+DwIcCompType]
        cmp     eax, DW_COMP_TYPE_VALUE
        jne     .bad_hw
.good_hw:
        mov     [DwHcnt], DW_FS_HCNT
        mov     [DwLcnt], DW_FS_LCNT
        mov     [DwSdaHold], DW_SDA_HOLD
        call    dw_ctrl_init
        DEBUGF 1, "i2chid: DesignWare I2C mapped at %x\n", ebx
        movi    eax, 1
        ret
.bad_hw:
        DEBUGF 1, "i2chid:   no DW_apb_i2c signature, got %x\n", eax
        invoke  FreeKernelSpace, [I2cMmio]
        mov     [I2cMmio], 0
        jmp     .fail
; Every reason to give up on a controller is named, so that a log from a
; machine where nothing works still says which step failed.
.io_bar:
        DEBUGF 1, "i2chid:   BAR0 is an I/O range, not MMIO\n"
        jmp     .fail
.no_bar:
        DEBUGF 1, "i2chid:   BAR0 not assigned by firmware\n"
        jmp     .fail
.bar_high:
        DEBUGF 1, "i2chid:   BAR0 lives above 4G\n"
        jmp     .fail
.no_map:
        DEBUGF 1, "i2chid:   MapIoMem failed for %x\n", ebx
.fail:
        xor     eax, eax
        ret
endp

; Probes the addresses known to be used by I2C-HID pointing devices, asking
; each for a HID descriptor at the two common register addresses. Two passes:
; some touchpads NAK the very first transaction while waking up.
;
; The bus is deliberately NOT swept from 08 to 77. A probe is a two-byte
; write followed by a read, and to a device that is not an I2C-HID one those
; two bytes may well look like "write value to register" - an audio codec or
; a battery gauge sharing the bus could be reconfigured by a blind sweep.
; Every address below is one that laptop firmware actually assigns to HID
; touchpads and touchscreens.
; out: eax = 1 when a device was found AND configured.
proc scan_bus uses ebx esi edi
        mov     [TimeoutStreak], 0
        mov     [AckCount], 0
        mov     [NakCount], 0
        mov     [TmoCount], 0
        mov     ebx, 2                  ; passes
.pass:
        xor     esi, esi                ; index into hid_addrs
.addr:
        cmp     esi, hid_addrs_cnt
        jae     .pass_done
; A NAK proves the controller alive (the address phase completed), but a
; timeout means SCL never toggled - a clock-gated or otherwise dead
; controller. One such probe costs the full transfer timeout, so after a few
; in a row the whole controller is abandoned instead of burning seconds of
; boot time on the remaining addresses.
        cmp     [TimeoutStreak], 4
        jae     .dead
        movzx   edi, byte [hid_addrs+esi]
        stdcall try_hid_addr, edi, 0x0001
        test    eax, eax
        jnz     .got
        stdcall try_hid_addr, edi, 0x0020
        test    eax, eax
        jnz     .got
        stdcall try_hid_addr, edi, 0x0002
        test    eax, eax
        jnz     .got
.skip:
        inc     esi
        jmp     .addr
.got:
        call    setup_device
        test    eax, eax
        jz      .skip                   ; found but unusable - keep scanning
        movi    eax, 1
        ret
.pass_done:
        dec     ebx
        jz      .fail
        mov     esi, 10
        invoke  Sleep
        jmp     .pass
.dead:
        DEBUGF 1, "i2chid:   controller does not respond, skipping it\n"
.fail:
; The three counters say which of the possible worlds this is: only NAKs
; means a healthy bus with nobody at the probed addresses (widen the table),
; only timeouts means the controller never drove SCL, and a nonzero ack
; count means something answered but did not look like an I2C-HID device.
        DEBUGF 1, "i2chid:   no HID device here (ack %u, nak %u, timeout %u)\n", \
                [AckCount], [NakCount], [TmoCount]
        xor     eax, eax
        ret
endp

; Requests the HID descriptor from addr at register dreg and validates it.
; out: eax = 1 and device parameters stored on success.
proc try_hid_addr stdcall uses ebx esi edi, addr:dword, dreg:dword
        mov     eax, [dreg]
        mov     word [wbuf], ax
        stdcall dw_xfer, [addr], wbuf, 2, hid_desc, I2CHID_DESC_LEN
        cmp     eax, -1                 ; timeout: SCL never moved
        jne     @f
        inc     [TimeoutStreak]
        inc     [TmoCount]
        jmp     .no
@@:
        mov     [TimeoutStreak], 0      ; NAK or success: the bus works
        test    eax, eax
        jz      @f
        inc     [NakCount]
        jmp     .no
@@:
; Something acknowledged its address and returned data. That alone is worth
; reporting together with the raw bytes: if the checks below then reject it,
; the log still shows what is sitting at this address.
        inc     [AckCount]
        DEBUGF 1, "i2chid:   addr %x reg %x answered:", [addr], [dreg]
        mov     esi, hid_desc
        mov     ecx, I2CHID_DESC_LEN
        call    dump_hex
        movzx   eax, word [hid_desc+HIDD_wHIDDescLength]
        cmp     eax, I2CHID_DESC_LEN
        jne     .bad_desc
        movzx   eax, word [hid_desc+HIDD_bcdVersion]
        cmp     eax, 0x0100
        jne     .bad_desc
        movzx   eax, word [hid_desc+HIDD_wMaxInputLength]
        cmp     eax, 3
        jb      .bad_desc
        movzx   ecx, word [hid_desc+HIDD_wReportDescLength]
        test    ecx, ecx
        jz      .bad_desc
        cmp     ecx, MAX_REPDESC
        ja      .bad_desc
        mov     [RepDescLen], ecx
        cmp     eax, MAX_INPUT_BUF
        jbe     @f
        mov     eax, MAX_INPUT_BUF
@@:
        mov     [MaxInput], eax
        mov     eax, [addr]
        mov     [SlaveAddr], eax
        movzx   eax, word [hid_desc+HIDD_wCommandRegister]
        mov     [CmdReg], eax
        movzx   eax, word [hid_desc+HIDD_wDataRegister]
        mov     [DataReg], eax
        movzx   eax, word [hid_desc+HIDD_wInputRegister]
        mov     [InputReg], eax
        movzx   eax, word [hid_desc+HIDD_wReportDescRegister]
        mov     [RepDescReg], eax
        movzx   eax, word [hid_desc+HIDD_wVendorID]
        movzx   ecx, word [hid_desc+HIDD_wProductID]
        DEBUGF 2, "i2chid: HID device at I2C addr %x (desc reg %x), VID %x PID %x\n", [addr], [dreg], eax, ecx
; The SIPODEV SP1064 family (SYNA3602 in ACPI, VID 093A PID 0255) never
; talks to a generic HID driver on Windows - a filter driver feeds the OS
; hardcoded descriptors instead of the firmware's own, so the firmware's
; descriptors cannot be trusted. Remember the device so the poll loop can
; also accept the report layout those hardcoded descriptors describe.
        mov     [IsSp1064], 0
        cmp     eax, 0x093A
        jne     @f
        cmp     ecx, 0x0255
        jne     @f
        mov     [IsSp1064], 1
@@:
        movi    eax, 1
        ret
.bad_desc:
        DEBUGF 1, "i2chid:   ...not a valid HID over I2C descriptor, ignored\n"
.no:
        xor     eax, eax
        ret
endp

; Dumps ecx bytes at esi to the debug board as hex, 32 bytes per line.
;
; The text of a whole line is built in a buffer first and handed over in a
; single call. The debug board is shared with every other process and its
; writes are not atomic, so a dump made of one call per byte comes out
; shredded by whatever else happens to be logging at that moment - which is
; exactly what a descriptor dump must not be.
; Every dump is level 1 output, and the headers that introduce them are
; DEBUGF 1 - so this is compiled out together with them, or the bytes would
; still pour out with the explanation missing. The raw entry point is for
; the few dumps that are always wanted.
proc dump_hex
if __DEBUG_LEVEL__ <= 1
        call    dump_hex_raw
end if
        ret
endp

proc dump_hex_raw uses eax ebx ecx edx esi edi
.line:
        test    ecx, ecx
        jz      .done
        mov     edi, hexbuf
        mov     edx, 32                 ; bytes per line
.byte:
        test    ecx, ecx
        jz      .flush
        mov     byte [edi], ' '
        inc     edi
        movzx   ebx, byte [esi]
        inc     esi
        dec     ecx
        mov     eax, ebx
        shr     eax, 4
        cmp     al, 10
        jb      @f
        add     al, 'A' - '0' - 10
@@:
        add     al, '0'
        mov     [edi], al
        inc     edi
        mov     eax, ebx
        and     eax, 15
        cmp     al, 10
        jb      @f
        add     al, 'A' - '0' - 10
@@:
        add     al, '0'
        mov     [edi], al
        inc     edi
        dec     edx
        jnz     .byte
.flush:
        mov     byte [edi], 0
        DEBUGF 2, "%s", hexbuf
        jmp     .line
.done:
        DEBUGF 2, "\n"
        ret
endp

; Reads and parses the report descriptor, then powers the device up.
; out: eax = 1 on success.
; The order matters and follows what the specification requires: a device that
; has not been powered on and reset is allowed to NAK or to return garbage for
; the report descriptor, so the descriptor is fetched only afterwards.
proc setup_device uses ebx esi edi
; SET_POWER(ON), then RESET.
        stdcall hid_command, I2CHID_PWR_ON, I2CHID_OP_SET_POWER
        test    eax, eax
        jz      @f
        DEBUGF 2, "i2chid: SET_POWER failed (%x)\n", eax
@@:
        mov     esi, 1
        invoke  Sleep
        stdcall hid_command, 0, I2CHID_OP_RESET
        test    eax, eax
        jz      @f
        DEBUGF 2, "i2chid: RESET failed (%x)\n", eax
@@:
; Reset takes time, and the bus must be left completely alone while it runs.
; The specification has the device announce completion by asserting its
; interrupt and offering a zero-length input report, but a whole family of
; controllers never does that (Linux carries I2C_HID_QUIRK_NO_IRQ_AFTER_RESET
; for HANTICK 5288 and friends - the very id the SIPODEV filter driver
; reports) and simply wants an unconditional wait. Polling the input register
; during that window is the one thing that could disturb such a device, so
; the wait is a plain sleep; a single read afterwards consumes the ack if one
; was in fact produced.
        mov     esi, 150
        invoke  Sleep
        stdcall dw_read_block, [SlaveAddr], 0, 0, input_buf, [MaxInput]
        DEBUGF 1, "i2chid: post-reset read status %x, first bytes:", eax
        mov     esi, input_buf
        mov     ecx, 8
        call    dump_hex
; SET_POWER(ON) once more, after the reset. The reference implementation
; (Linux i2c_hid_finish_hwreset) does exactly this: some devices come out of
; RESET with their scan engine asleep - registers answer, but no input
; report is ever produced until they are powered on again.
        stdcall hid_command, I2CHID_PWR_ON, I2CHID_OP_SET_POWER
        test    eax, eax
        jz      @f
        DEBUGF 2, "i2chid: post-reset SET_POWER failed (%x)\n", eax
@@:
        mov     esi, 10
        invoke  Sleep
; Now the device is awake and the report descriptor can be trusted.
        mov     eax, [RepDescReg]
        mov     word [wbuf], ax
        stdcall dw_xfer, [SlaveAddr], wbuf, 2, rep_desc, [RepDescLen]
        test    eax, eax
        jz      @f
        DEBUGF 2, "i2chid: report descriptor read failed (%x)\n", eax
        jmp     .fail
@@:
        mov     esi, rep_desc
        mov     ecx, [RepDescLen]
        call    hid_parse_report_desc
        test    eax, eax
        jnz     .parsed
; Nothing usable was found. The whole descriptor goes to the log: without it
; a failure here cannot be diagnosed at all, and with it the parser can be
; fixed offline against the exact bytes this device sent.
        DEBUGF 1, "i2chid: no relative mouse report; full descriptor (%u bytes):\n", [RepDescLen]
        mov     esi, rep_desc
        mov     ecx, [RepDescLen]
        call    dump_hex
        jmp     .fail
.parsed:
; The whole descriptor goes to the log even on success. It is the only way
; to find out which feature reports the device offers - and on a pad that
; answers every command yet never reports a finger, the mode-selecting
; feature report is the next thing to look at.
        DEBUGF 1, "i2chid: full descriptor (%u bytes):", [RepDescLen]
        mov     esi, rep_desc
        mov     ecx, [RepDescLen]
        call    dump_hex
; Is there a touchpad report we can decode? Finger 0 needs a tip switch and
; both coordinates; the rest is optional.
        mov     [PtpLayoutOk], 0
        cmp     [t_ptp_id], -1
        je      @f
        cmp     [t_fcount], 0
        je      @f
        cmp     dword [t_fxs], 0
        je      @f
        cmp     dword [t_fys], 0
        je      @f
        mov     [PtpLayoutOk], 1
@@:
        DEBUGF 2, "i2chid: touchpad report id %d, %u bytes, %u fingers, tip@%u cid@%u/%u x@%u/%u y@%u/%u count@%u/%u btn@%u/%u ok %u\n", \
                [t_ptp_id], [t_ptp_bytes], [t_fcount], [t_ftip], [t_fcid], [t_fcids], [t_fx], [t_fxs], [t_fy], [t_fys], \
                [t_ccnt_off], [t_ccnt_size], [t_pbtn_off], [t_pbtn_cnt], [PtpLayoutOk]
; The surface extents are known now, so the zone percentages can be turned
; into coordinates.
        call    apply_geometry
        DEBUGF 1, "i2chid: pad %u x %u, button strip from %u, right half from %u\n", \
                [t_padmaxx], [t_padmaxy], [BtnZoneY], [RightSplitX]
        DEBUGF 1, "i2chid: report id %x, %x bytes; Xoff %x Yoff %x btns %x wheel size %x\n", \
                [f_repid], [f_bytes], [t_xoff], [t_yoff], [t_btncnt], [t_wsize]
; The chosen report must fit into the input buffer.
        mov     eax, [f_bytes]
        add     eax, 3                  ; length word + possible report id
        cmp     eax, MAX_INPUT_BUF
        ja      .fail
        cmp     eax, [MaxInput]
        jbe     @f
        mov     [MaxInput], eax
@@:
; How much to read per poll: the longer of the two reports that get decoded,
; plus the length word - never more than the device's maximum. A 100 kHz
; input read of the full maximum can cost more than the interval between
; two reports of a fast pad; reading just the frame keeps up with it.
        mov     eax, [t_ptp_bytes]
        inc     eax                     ; + report id
        mov     ecx, [f_bytes]
        inc     ecx
        cmp     eax, ecx
        jae     @f
        mov     eax, ecx
@@:
        add     eax, 2                  ; the length word
        cmp     eax, [MaxInput]
        jbe     @f
        mov     eax, [MaxInput]
@@:
        mov     [ReadLen], eax
        DEBUGF 2, "i2chid: input reads of %u bytes (device maximum %u)\n", eax, [MaxInput]
; Switch the pad into touchpad mode. Every OS that works with a Precision
; Touchpad writes its Input Mode feature = 3 first; some firmwares (the
; SIPODEV one) never report anything at all in mouse mode.
        call    set_ptp_mode
        movi    eax, 1
        ret
.fail:
        xor     eax, eax
        ret
endp

; Writes a 2-byte command to the command register: [reg lo, reg hi, arg,
; opcode]. A device that has just been powered up may NAK for a few
; milliseconds, so the write is retried.
; out: eax = 0 on success, else the last dw_xfer error.
proc hid_command stdcall uses ebx esi edi, arg:dword, opcode:dword
        mov     eax, [CmdReg]
        mov     word [cmd_buf], ax
        mov     eax, [arg]
        mov     [cmd_buf+2], al
        mov     eax, [opcode]
        mov     [cmd_buf+3], al
        mov     edi, 3
.retry:
        stdcall dw_xfer, [SlaveAddr], cmd_buf, 4, 0, 0
        test    eax, eax
        jz      .done
        dec     edi
        jz      .done
        mov     esi, 10
        invoke  Sleep
        jmp     .retry
.done:
        ret
endp

; Finds the report id of the Input Mode feature (Digitizer usage 0x52) in
; the report descriptor: locates the 09 52 usage item and takes the 85 xx
; Report ID item nearest to it - first searching forward (this firmware
; puts the id after the usage), then backward (the layout of the Linux
; override variant). Heuristic, but both known firmwares satisfy it.
; out: eax = report id, or 0 when not found.
proc find_input_mode_id uses ebx ecx esi
        mov     esi, rep_desc
        mov     ecx, [RepDescLen]
        sub     ecx, 1
        jbe     .none
.scan:
        cmp     word [esi], 0x5209      ; 09 52 little-endian
        je      .found_usage
        inc     esi
        loop    .scan
.none:
        xor     eax, eax
        ret
.found_usage:
        movi    ecx, 16
        mov     ebx, esi
.fwd:
        cmp     byte [ebx], 0x85
        je      .take
        inc     ebx
        loop    .fwd
        movi    ecx, 16
        mov     ebx, esi
.bwd:
        cmp     byte [ebx], 0x85
        je      .take
        dec     ebx
        cmp     ebx, rep_desc
        jb      .none
        loop    .bwd
        jmp     .none
.take:
        movzx   eax, byte [ebx+1]
        ret
endp

; Switches the SIPODEV pad into its precision-touchpad mode: SET_REPORT of
; the Input Mode feature (report id 3, Digitizer usage 0x52) with value 3.
; The mouse TLC of this firmware never produces anything - every OS that
; works with the pad performs exactly this write and then receives PTP
; frames as report id 4. Ids and layout come from the Linux DMI override
; descriptor, captured from the Windows filter driver traffic. A no-op for
; every other device.
; Frame decoder and contact tracker, for every kind of pad.
;
; A Precision Touchpad frame lists its contacts either all in one report
; (SIPODEV: five slots) or one per report (Elan: hybrid mode, the count in
; the first report of a frame). Neither slot position nor report order says
; which finger is which - only the contact id does: a pad that packs all
; fingers into one report moves the remaining finger into slot 0 when the
; first one leaves, and taking slot 0 for finger 0 teleports the pointer to
; the other finger. So every slot of every report is fed into a table of
; contacts keyed by id, and the rest of the decoder is given a single-finger
; view made from that table: the pointer contact (kept once chosen), the
; number of fingers down, and a lift of the pointer contact reported as a
; lift - never as a jump to another finger.
;
; A finger that lands on the button strip is a thumb: it presses the button
; and rests, and any other finger on the pad steers instead of it - which is
; how a window is dragged with the pad button held by the thumb. Thumbs do
; not count towards gestures while another finger is down.
; in: esi -> report id byte.
; out: PtpTip/PtpX/PtpY (the pointer contact), PtpCnt, PtpBtn.
PTP_SLOTS           = HID_MAX_FINGERS
SLOT_SIZE           = 32            ; tip, x, y, last seen, first seen, thumb
CONTACT_AGE         = 8             ; 1/100 s without news = the finger is gone
proc ptp_decode uses eax ebx ecx edx esi edi
        invoke  GetTimerTicks
        mov     edx, eax                ; now
        mov     [LiftTick], eax         ; when a lift is decided this frame
        mov     ecx, [LastFrameTick]
        mov     [PrevFrameTick], ecx
        mov     [LastFrameTick], eax
        mov     [Handover], 0
        mov     [SeenMask], 0
        mov     [NewMask], 0
        inc     esi                     ; offsets count from after the id
        mov     [Payload], esi
; Frame-level fields: contact count and buttons.
        xor     eax, eax
        cmp     [t_ccnt_size], 0
        je      @f
        mov     eax, [t_ccnt_off]
        mov     ecx, [t_ccnt_size]
        call    hid_extract_u
@@:
        mov     [RawCnt], eax
        xor     eax, eax
        cmp     [t_pbtn_cnt], 0
        je      @f
        mov     eax, [t_pbtn_off]
        mov     ecx, [t_pbtn_cnt]
        call    hid_extract_u
        and     eax, 7
@@:
        mov     [PtpBtn], eax
; Every slot of the report goes into the table. hid_extract_u leaves ecx
; alone, so the contact id can stay there across the extractions.
        xor     ebx, ebx                ; slot index
.slot_in:
        cmp     ebx, [t_fcount]
        jae     .slots_done
        mov     esi, [Payload]
        mov     eax, [t_ftip+ebx*4]
        movi    ecx, 1
        call    hid_extract_u
        mov     [SlotTip], eax
        mov     ecx, ebx                ; no id field: the slot is the id
        cmp     dword [t_fcids+ebx*4], 0
        je      @f
        mov     eax, [t_fcid+ebx*4]
        mov     ecx, [t_fcids+ebx*4]
        call    hid_extract_u
        mov     ecx, eax
@@:
        cmp     ecx, PTP_SLOTS
        jae     .slot_next              ; an id we have no room for
        mov     edi, ecx
        shl     edi, 5
        add     edi, contacts
        cmp     [SlotTip], 0
        jne     .slot_down
; A slot with the tip switch off names a contact that just left - in a
; one-contact-per-report pad. In a full frame the empty slots carry id 0
; and tip 0 as well and mean nothing; there, a contact that is not listed
; is the one that left (see below).
        cmp     [t_fcount], 1
        jne     .slot_next
        mov     dword [edi], 0
        jmp     .slot_next
.slot_down:
        bts     [SeenMask], ecx
        mov     eax, [t_fx+ebx*4]
        mov     ecx, [t_fxs+ebx*4]
        call    hid_extract_u
        mov     [edi+4], eax
        mov     eax, [t_fy+ebx*4]
        mov     ecx, [t_fys+ebx*4]
        call    hid_extract_u
        mov     [edi+8], eax
        mov     [edi+12], edx           ; last seen
        cmp     dword [edi], 0
        jne     .slot_known
; A new contact: remember when it came and whether it landed on the strip.
        mov     [edi+16], edx
        mov     dword [edi+20], 0
        cmp     eax, [BtnZoneY]         ; eax = its Y
        jb      @f
        mov     dword [edi+20], 1       ; a thumb
@@:
        mov     ecx, edi
        sub     ecx, contacts
        shr     ecx, 5                  ; its id again
        bts     [NewMask], ecx
.slot_known:
        mov     dword [edi], 1
.slot_next:
        inc     ebx
        jmp     .slot_in
.slots_done:
; A full frame lists every contact that is down: whatever the table has
; that the frame does not is up.
        cmp     [t_fcount], 1
        je      .hybrid_fixups
        xor     ecx, ecx
.absent:
        bt      [SeenMask], ecx
        jc      @f
        mov     esi, ecx
        shl     esi, 5
        mov     dword [contacts+esi], 0
@@:
        inc     ecx
        cmp     ecx, PTP_SLOTS
        jb      .absent
        jmp     .scan_start
.hybrid_fixups:
; A report that carries the contact count opens a frame and says exactly
; how many contacts there are. The pad hands out new ids when a finger
; re-lands, so a slot can outlive its contact; rather than wait for it to
; age out (during which it inflates the finger count and can turn a
; two-finger tap into a middle click), drop the oldest slots until the
; table agrees with the pad.
        mov     eax, [RawCnt]
        test    eax, eax
        jz      .scan_start
.reconcile:
        xor     ebx, ebx                ; live slots
        mov     edi, -1                 ; the oldest of them
        xor     ecx, ecx
.rc_slot:
        mov     esi, ecx
        shl     esi, 5
        add     esi, contacts
        cmp     dword [esi], 0
        je      .rc_next
        inc     ebx
        cmp     edi, -1
        je      .rc_take
        push    eax
        mov     eax, edi
        shl     eax, 5
        mov     eax, dword [contacts+eax+12]
        cmp     [esi+12], eax           ; older than the oldest so far?
        pop     eax
        jae     .rc_next
.rc_take:
        mov     edi, ecx
.rc_next:
        inc     ecx
        cmp     ecx, PTP_SLOTS
        jb      .rc_slot
        cmp     ebx, eax                ; more live slots than contacts?
        jbe     .scan_start
        cmp     edi, -1
        je      .scan_start
        mov     esi, edi
        shl     esi, 5
        mov     dword [contacts+esi], 0
        jmp     .reconcile
.scan_start:
; Forget contacts that went quiet; count the ones down, thumbs apart; find
; the lowest of each kind.
        xor     ebx, ebx                ; fingers down
        mov     [NtCnt], 0              ; of which not thumbs
        mov     edi, -1                 ; lowest contact down
        mov     [LowestNt], -1          ; lowest non-thumb down
        mov     [LiveMask], 0
        xor     ecx, ecx
.scan:
        mov     esi, ecx
        shl     esi, 5
        add     esi, contacts
        cmp     dword [esi], 0
        je      .scan_next
        mov     eax, edx
        sub     eax, [esi+12]
        cmp     eax, CONTACT_AGE
        jbe     .scan_live
        mov     dword [esi], 0
        jmp     .scan_next
.scan_live:
        inc     ebx
        bts     [LiveMask], ecx
        cmp     edi, -1
        jne     @f
        mov     edi, ecx
@@:
        cmp     dword [esi+20], 0
        jne     .scan_next
        inc     [NtCnt]
        cmp     [LowestNt], -1
        jne     .scan_next
        mov     [LowestNt], ecx
.scan_next:
        inc     ecx
        cmp     ecx, PTP_SLOTS
        jb      .scan
; The pointer contact: keep the one we have while it is down - unless it is
; a thumb and a proper finger has arrived, which takes over: the thumb's
; pointer role ends here as a lift (a quiet one, see Handover) and the next
; report picks the finger as a fresh touch.
        mov     eax, [PointerId]
        cmp     eax, -1
        je      .choose
        mov     esi, eax
        shl     esi, 5
        add     esi, contacts
        cmp     dword [esi], 0
        je      .lifted
        cmp     dword [esi+20], 0
        je      .have
        cmp     [NtCnt], 0
        je      .have
        mov     [Handover], 1
        mov     [PointerId], -1
        mov     [PtpTip], 0
        jmp     .coords
.lifted:
; It went up - either its own report said so, or it fell silent. The lift
; is dated to the last time it was heard from, so that a tap is judged by
; how long the finger was really down, not by how long we waited.
        mov     [PointerId], -1
        mov     [PtpTip], 0
        mov     eax, [esi+12]
        cmp     eax, [TapTick]          ; a successor may have been heard from
        jae     @f                      ; before it became the pointer
        mov     eax, [TapTick]
@@:
        mov     [LiftTick], eax
; Other fingers still down will make one of them the pointer on the next
; report, as a fresh touch. Those that were already down are the tail end
; of THIS gesture (a scroll, mostly) and must neither tap nor steer; one
; that arrived with this very report is a genuine new touch.
        test    ebx, ebx
        jz      .coords
        mov     [LiftGuardTick], edx
        mov     eax, [LiveMask]
        mov     ecx, [NewMask]
        not     ecx
        and     eax, ecx
        mov     [LeftoverMask], eax
        jmp     .coords
.choose:
        mov     eax, [LowestNt]
        cmp     eax, -1
        jne     @f
        mov     eax, edi                ; only thumbs: the lowest of them
@@:
        mov     [PointerId], eax
        cmp     eax, -1
        je      .nobody
        mov     [Leftover], 0
        bt      [LeftoverMask], eax
        jnc     @f
        mov     [Leftover], 1
@@:
        mov     esi, eax
        shl     esi, 5
        add     esi, contacts
.have:
        mov     [PtpTip], 1
.coords:
        mov     eax, [esi+4]
        mov     [PtpX], eax
        mov     eax, [esi+8]
        mov     [PtpY], eax
        jmp     .count
.nobody:
        mov     [PtpTip], 0
.count:
; Fingers for the gesture logic: with a thumb resting and a finger down the
; finger is alone; otherwise the frame's own count when it carries one,
; else what the table knows.
        mov     eax, [NtCnt]
        test    eax, eax
        jz      .plain_count
        cmp     eax, ebx
        jne     .cnt_done               ; some thumbs among them: ignored
.plain_count:
        mov     eax, [RawCnt]
        test    eax, eax
        jnz     .cnt_done
        mov     eax, ebx
.cnt_done:
        mov     [PtpCnt], eax
; Leftover status ends with the finger.
        mov     eax, [LiveMask]
        and     [LeftoverMask], eax
        ret
endp

; Forgets every contact: used when the pad has fallen silent with fingers
; still on record.
proc ptp_forget uses eax ecx
        xor     ecx, ecx
@@:
        mov     eax, ecx
        shl     eax, 5
        mov     dword [contacts+eax], 0
        inc     ecx
        cmp     ecx, PTP_SLOTS
        jb      @b
        mov     [PointerId], -1
        mov     [LeftoverMask], 0
        mov     [Handover], 0
        ret
endp

; Switches the pad into its touchpad mode: the Input Mode feature (Digitizer
; usage 0x52) set to 3 via SET_REPORT. In mouse mode a Precision Touchpad
; reports at most what a mouse would, so every gesture depends on this. It
; is only worth doing once the descriptor has yielded a frame layout to
; decode the touchpad report with.
proc set_ptp_mode uses ebx esi edi
        cmp     [PtpLayoutOk], 0
        je      .nothing
        call    find_input_mode_id
        test    eax, eax
        jnz     @f
        cmp     [IsSp1064], 0
        je      .nothing                ; no Input Mode feature: no such mode
        movi    eax, 3                  ; the Linux-override firmware puts it at 3
@@:
        mov     [ModeRepId], eax
; The report has to be written whole. Its first byte is the mode; the rest
; (when there is any) carries the selective-reporting switches, which are
; set as Windows sets them: surface and button reporting both on. A pad
; whose descriptor gives no size for it gets one byte.
        call    hid_feature_bytes
        test    eax, eax
        jnz     @f
        movi    eax, 1
@@:
        cmp     eax, 8
        jbe     @f
        movi    eax, 8
@@:
        mov     [ModeRepLen], eax
        mov     eax, [CmdReg]
        mov     word [setrep_buf], ax
        mov     eax, [ModeRepId]
        and     eax, 15
        or      eax, 0x30                      ; feature report type
        mov     [setrep_buf+2], al
        mov     byte [setrep_buf+3], 0x03      ; SET_REPORT
        mov     eax, [DataReg]
        mov     word [setrep_buf+4], ax
        mov     eax, [ModeRepLen]
        add     eax, 3                         ; len word + id + data
        mov     word [setrep_buf+6], ax
        mov     eax, [ModeRepId]
        mov     [setrep_buf+8], al             ; report id
        lea     edi, [setrep_buf+9]
        mov     ecx, [ModeRepLen]
        mov     al, 3                          ; mode 3, switches 11b
        rep stosb
        mov     ecx, [ModeRepLen]
        add     ecx, 9
        stdcall dw_xfer, [SlaveAddr], setrep_buf, ecx, 0, 0
        DEBUGF 2, "i2chid: PTP input mode set via feature id %u, %u data byte(s) (%x)\n", [ModeRepId], [ModeRepLen], eax
; Read it back: a firmware that silently ignored the write is otherwise
; indistinguishable from one that took it.
        mov     esi, 10
        invoke  Sleep
        mov     eax, [CmdReg]
        mov     word [diag_buf], ax
        mov     eax, [ModeRepId]
        and     eax, 15
        or      eax, 0x30
        mov     [diag_buf+2], al
        mov     byte [diag_buf+3], 0x02        ; GET_REPORT
        mov     eax, [DataReg]
        mov     word [diag_buf+4], ax
        mov     ecx, [ModeRepLen]
        add     ecx, 3
        stdcall dw_read_block, [SlaveAddr], diag_buf, 6, input_buf, ecx
        DEBUGF 2, "i2chid: input mode feature reads back (%x):", eax
        mov     esi, input_buf
        mov     ecx, [ModeRepLen]
        add     ecx, 3
        call    dump_hex_raw
.nothing:
        ret
endp

; Asks the device for every report it declared, as both an Input and a
; Feature report, and logs each answer. A pad whose sensor is alive but
; whose reporting mode was never selected will answer at least one of
; these even while the input register stays empty - and the report id that
; answers says which mode selector to write.
proc get_report_sweep uses ebx esi edi
        DEBUGF 1, "i2chid: GET_REPORT sweep over %u report ids\n", [p_idcnt]
        xor     ebx, ebx                ; index into p_ids
.id_loop:
        cmp     ebx, [p_idcnt]
        jae     .done
        mov     edi, 1                  ; report type: 1 = input, 3 = feature
.type_loop:
        mov     eax, [CmdReg]
        mov     word [diag_buf], ax
        mov     ecx, edi
        shl     ecx, 4
        mov     eax, [p_ids+ebx*4]
        cmp     eax, 15
        jae     .big_id
        or      eax, ecx
        mov     [diag_buf+2], al
        mov     byte [diag_buf+3], 0x02 ; GET_REPORT
        mov     eax, [DataReg]
        mov     word [diag_buf+4], ax
        stdcall dw_read_block, [SlaveAddr], diag_buf, 6, input_buf, 16
        jmp     .asked
.big_id:
; ids above 14 use the sentinel nibble 1111 and travel as a full byte
; placed right after the opcode
        or      ecx, 15
        mov     [diag_buf+2], cl
        mov     byte [diag_buf+3], 0x02
        mov     [diag_buf+4], al
        mov     eax, [DataReg]
        mov     word [diag_buf+5], ax
        stdcall dw_read_block, [SlaveAddr], diag_buf, 7, input_buf, 16
.asked:
        DEBUGF 1, "i2chid:   type %u id %u status %x:", edi, [p_ids+ebx*4], eax
        mov     esi, input_buf
        mov     ecx, 16
        call    dump_hex
        add     edi, 2                  ; 1 -> 3
        cmp     edi, 3
        jbe     .type_loop
        inc     ebx
        jmp     .id_loop
.done:
        ret
endp

; Cancels the pending tap once the finger has wandered further from where
; it landed than a tap is allowed to. Called for one- and two-finger
; touches alike, so that a scroll cancels the tap only when the fingers
; really travelled.
; in: ecx = current X, edx = current Y.
proc tap_travel_check uses eax
        mov     eax, ecx
        sub     eax, [TapX]
        jns     @f
        neg     eax
@@:
        cmp     eax, TAP_MAX_DIST
        ja      .moved
        mov     eax, edx
        sub     eax, [TapY]
        jns     @f
        neg     eax
@@:
        cmp     eax, TAP_MAX_DIST
        jbe     .done
.moved:
        mov     [TapMoved], 1
.done:
        ret
endp

; Logs an idle (zero-length) answer when it is one of the first few, or
; whenever its first bytes differ from the previous one. A touchpad that is
; alive but never fills in a length still tends to change something here;
; a frozen one repeats the identical bytes forever.
proc trace_idle uses eax ecx esi edi
        cmp     [IdleTrace], 0
        jz      .check_change
        dec     [IdleTrace]
        jmp     .dump
.check_change:
        cmp     [IdleChanges], 0
        jz      .nothing
        mov     esi, input_buf
        mov     edi, idle_prev
        mov     ecx, 12
        repe cmpsb
        je      .nothing
        dec     [IdleChanges]
.dump:
        DEBUGF 1, "i2chid: idle answer:"
        mov     esi, input_buf
        mov     ecx, 12
        call    dump_hex
        mov     esi, input_buf
        mov     edi, idle_prev
        mov     ecx, 12
        rep movsb
.nothing:
        ret
endp

; =============================================================================
; The polling thread. Ends only if the device stops answering altogether.
proc i2chid_thread
.loop:
; A tap click went out during the previous cycle; release it now, a full
; poll interval later. Not sooner: the kernel notices a button through the
; change between two of its own passes, and a release that follows the
; press within the same collecting loop can go unseen. Not later: a pad
; that never answers empty (Elan) would otherwise hold it for ever.
        cmp     [TapRelease], 0
        jz      .no_pulse
        dec     [TapRelease]
        jnz     .no_pulse
        mov     [HeldBtn], 0
        invoke  SetMouseData, [BtnLatch], 0, 0, 0, 0
.no_pulse:
; The second click of a double tap: the first one's release went out with
; the lift frame, this press follows a cycle later so the kernel sees the
; edge, and it is then held like any tap's press.
        cmp     [PendingPress], 0
        jz      .no_pending
        mov     [PendingPress], 0
        mov     [HeldBtn], 1
        mov     eax, [BtnLatch]
        or      eax, 1
        invoke  SetMouseData, eax, 0, 0, 0, 0
        mov     [TapHold], 1
        invoke  GetTimerTicks
        mov     [TapHoldTick], eax
        jmp     .no_hold
.no_pending:
; A tap's press is held for the drag window; nobody came back: release.
        cmp     [TapHold], 0
        jz      .no_hold
        invoke  GetTimerTicks
        sub     eax, [TapHoldTick]
        cmp     eax, DRAG_WINDOW
        jbe     .no_hold
        mov     [TapHold], 0
        mov     [HeldBtn], 0
        invoke  SetMouseData, [BtnLatch], 0, 0, 0, 0
.no_hold:
; A finger the pad has said nothing about for a while is gone, whether or
; not the frame saying so ever arrived (some pads skip it after a slide,
; and a polled host can miss one). Without this the touch stayed open and
; the next touch, seconds later, continued it - from the old position,
; which the kernel's acceleration turned into a jump to a screen corner.
; Contacts only age while frames arrive, so silence needs its own clock.
        cmp     [PrevTip], 0
        je      .no_silence
        invoke  GetTimerTicks
        sub     eax, [LastFrameTick]
        cmp     eax, CONTACT_AGE
        jbe     .no_silence
        mov     [ForceLift], 1
.no_silence:
; A pad reports faster than the poll interval - an Elan sends well over a
; hundred frames a second and queues what is not collected - so one report
; per poll leaves a growing backlog that plays back after the finger has
; stopped, which looks like the cursor coasting. Keep reading until the pad
; has nothing more, within reason.
        mov     [DrainLeft], DRAIN_MAX
.again:
        call    i2chid_poll_once
        cmp     [PollStop], 0
        jnz     .exit
        cmp     [Consumed], 0
        je      .sleep
        dec     [DrainLeft]
        jnz     .again
.sleep:
; Bookkeeping for one line of statistics a few seconds in: how many reports
; a poll had to collect says whether the host keeps up with the pad.
        mov     eax, DRAIN_MAX
        sub     eax, [DrainLeft]
        cmp     eax, [StMaxDrain]
        jbe     @f
        mov     [StMaxDrain], eax
@@:
        inc     [StPolls]
        mov     eax, [StPolls]
        cmp     eax, STATS_AT_POLLS
        je      .stats
        xor     edx, edx
        mov     ecx, STATS_EVERY_POLLS
        div     ecx
        test    edx, edx
        jnz     @f
.stats:
        DEBUGF 2, "i2chid: after %u polls: %u reads, %u reports, %u duplicates, %u empty, at most %u per poll, %u lifts by silence\n", \
                [StPolls], [StReads], [StReports], [StDups], [StEmpty], [StMaxDrain], [StForced]
@@:
; Syscall 5 is the delay that sleeps on an event and yields; the Sleep import
; drivers normally use is delay_ms, which busy-waits and would burn a core.
; A kernel thread is an ordinary task, so int 0x40 is available to it.
        movi    eax, 5
        movi    ebx, POLL_DELAY_HS
        int     0x40
        jmp     .loop
.exit:
        or      eax, -1
        int     0x40
endp

; Reads the input register; a length of zero means no new
; report. Extracts buttons/X/Y/wheel of the chosen report and passes them to
; the kernel.
; One poll cycle. Runs only from i2chid_thread, so it needs no reentrancy
; guard of its own.
proc i2chid_poll_once uses ebx esi edi
        mov     [Consumed], 0
; The thread found the pad silent with a finger still on record: end that
; touch as if a lift frame had come, dated to the last frame that did.
        cmp     [ForceLift], 0
        jz      .poll
        mov     [ForceLift], 0
        inc     [StForced]
        call    ptp_forget
        mov     eax, [LastFrameTick]
        mov     [LiftTick], eax
        mov     [BtnLatch], 0
        xor     ebx, ebx
        mov     [v_x], 0
        mov     [v_y], 0
        mov     [v_w], 0
        jmp     .ptp_up
.poll:
; Mode 0 is the plain read the specification prescribes for input reports.
; Mode 1 addresses the input register explicitly first - some register-
; pointer-based firmwares return garbage for plain reads outside of an
; interrupt cycle, and the pointer evidence for that is visible in how such
; devices answer probes of unknown registers.
        xor     ecx, ecx
        cmp     [PollMode], 0
        je      @f
        mov     eax, [InputReg]
        mov     word [pollreg], ax
        movi    ecx, 2
@@:
        stdcall dw_read_block, [SlaveAddr], pollreg, ecx, input_buf, [ReadLen]
        test    eax, eax
        jnz     .err
        mov     [ErrCnt], 0
        inc     [StReads]
        movzx   eax, word [input_buf]
        cmp     eax, 2
        jbe     .silent                 ; 0 = nothing new
        cmp     eax, [MaxInput]
        ja      .done
        cmp     eax, [ReadLen]          ; a longer report than we read:
        jbe     @f                      ; only what arrived can be looked at
        mov     eax, [ReadLen]
@@:
; A pad may answer every read with the report it holds, new or not (the
; Elan does, thousands of times a second). The same bytes as last time are
; nothing new: skip them and stop collecting for this poll, otherwise the
; poll spins up to its cap on stale copies and the cursor moves in bursts.
        push    eax
        mov     esi, input_buf
        mov     edi, prev_rep
        mov     ecx, [ReadLen]
        repe cmpsb
        je      .duplicate
        mov     esi, input_buf
        mov     edi, prev_rep
        mov     ecx, [ReadLen]
        rep movsb
        pop     eax
        mov     [GotAny], 1
        mov     [Consumed], 1
        inc     [StReports]
; The first few reports are dumped raw, before any interpretation, so that a
; touchpad which does send data but whose bytes the driver reads wrongly can
; be told apart from one that says nothing at all.
        cmp     [TraceCount], 0
        jz      .no_trace
        dec     [TraceCount]
        push    eax
        DEBUGF 1, "i2chid: report:"
        mov     esi, input_buf
        mov     ecx, eax
        call    dump_hex
        pop     eax
.no_trace:
        lea     esi, [input_buf+2]
        sub     eax, 2
        cmp     [p_uses_repid], 0
        je      @f
        movzx   ecx, byte [esi]
        cmp     ecx, [f_repid]
        jne     .try_fallback
; The pad keeps its mouse collection alive next to the touchpad one and
; emits an all-zero mouse report now and then. Once PTP frames are known
; to flow, those stale reports must be dropped: an empty one arriving
; between two PTP frames would otherwise release a button mid-click.
        cmp     [PtpMode], 0
        jne     .done
        inc     esi
        dec     eax
@@:
        cmp     eax, [f_bytes]
        jb      .done
; Buttons.
        xor     ebx, ebx
        cmp     [t_btncnt], 0
        je      @f
        mov     eax, [t_btnoff]
        mov     ecx, [t_btncnt]
        call    hid_extract_u
        mov     ebx, eax
@@:
; X.
        mov     eax, [t_xoff]
        mov     ecx, [t_xsize]
        call    hid_extract_s
        mov     [v_x], eax
; Y: HID reports it down-positive, the kernel wants the PS/2 convention.
        mov     eax, [t_yoff]
        mov     ecx, [t_ysize]
        call    hid_extract_s
        neg     eax
        mov     [v_y], eax
; Wheel.
        xor     eax, eax
        cmp     [t_wsize], 0
        je      @f
        mov     eax, [t_woff]
        mov     ecx, [t_wsize]
        call    hid_extract_s
        neg     eax
@@:
        mov     [v_w], eax
.send:
; Whatever button the driver itself is holding - a tap's click, a tap's
; press waiting for a drag, a drag - goes out with EVERY frame, or the
; next frame after a tap (the other finger leaving, say) would release it
; within 10 ms and no application would ever see the click. That was the
; whole story of the right click that only worked when the fingers
; happened to leave in the right order.
        or      ebx, [HeldBtn]
; The first few lifts go to the log with everything the tap decision was
; made of and what came out of it, always: a tap that does not happen is
; otherwise invisible.
        cmp     [LiftPending], 0
        jz      .no_lift_trace
        mov     [LiftPending], 0
        cmp     [LiftTrace], 0
        jz      .no_lift_trace
; Plain one-finger moves are not worth a line; taps, clicks and anything
; with more fingers are.
        test    ebx, ebx
        jnz     .lift_line
        cmp     [TapFingers], 1
        jne     .lift_line
        cmp     [TapMoved], 0
        jne     .no_lift_trace
.lift_line:
        dec     [LiftTrace]
        mov     eax, [LiftTick]
        sub     eax, [TapTick]
        DEBUGF 2, "i2chid: lift: fingers %u, moved %u, strip %u, held %u ticks -> button %x\n", \
                [TapFingers], [TapMoved], [TapInStrip], eax, ebx
.no_lift_trace:
        cmp     [DecodeTrace], 0
        jz      @f
        dec     [DecodeTrace]
        DEBUGF 1, "i2chid: decoded btn %x dx %d dy %d wheel %d\n", ebx, [v_x], [v_y], [v_w]
@@:
        invoke  SetMouseData, ebx, [v_x], [v_y], [v_w], 0
.done:
        ret
.duplicate:
        pop     eax
        inc     [StDups]
        jmp     .done
.silent:
        inc     [StEmpty]
; Nothing arrived. An idle answer is still evidence: if the device reacts
; to a finger at all, something in those bytes changes even when it never
; sets a length, so the first answers and every change are logged.
        call    trace_idle
; While no report has EVER arrived, walk a ladder of recovery attempts,
; one step per silent stretch, and say in the log what each step was.
; If any of them wakes the device, GotAny freezes the ladder for good.
        cmp     [GotAny], 0
        jnz     .done
        inc     [SilentPolls]
        cmp     [SilentPolls], SILENT_LIMIT
        jb      .done
        mov     [SilentPolls], 0
        inc     [RecStep]
        mov     eax, [RecStep]
        cmp     eax, 1
        je      .rec_probe
        cmp     eax, 2
        je      .rec_mode
        cmp     eax, 3
        je      .rec_power
        cmp     eax, 4
        je      .rec_mode
        cmp     eax, 5
        je      .rec_reset
        cmp     eax, 6
        je      .rec_mode
        jmp     .done                   ; ladder exhausted, stay quiet
.rec_mode:
        xor     [PollMode], 1
        DEBUGF 1, "i2chid: still silent, poll mode now %u\n", [PollMode]
        jmp     .done
.rec_power:
; A full power cycle: some firmwares only start scanning on the ON edge.
        DEBUGF 1, "i2chid: still silent, power cycling the device\n"
        stdcall hid_command, 1, I2CHID_OP_SET_POWER      ; SLEEP
        mov     esi, 50
        invoke  Sleep
        stdcall hid_command, I2CHID_PWR_ON, I2CHID_OP_SET_POWER
        DEBUGF 1, "i2chid: power cycle done (%x)\n", eax
        jmp     .done
.rec_reset:
; And a second reset, this time with the device already powered on.
        DEBUGF 1, "i2chid: still silent, resetting the device again\n"
        stdcall hid_command, 0, I2CHID_OP_RESET
        push    eax
        mov     esi, 150
        invoke  Sleep
        pop     eax
        DEBUGF 1, "i2chid: second reset done (%x)\n", eax
        stdcall hid_command, I2CHID_PWR_ON, I2CHID_OP_SET_POWER
; The reset above also cleared the input-mode selection - set it again.
        call    set_ptp_mode
        jmp     .done
.rec_probe:
        call    get_report_sweep
        jmp     .done
.rec_probe_old:
; On the first silence timeout, ask for the input report explicitly once:
; GET_REPORT through the command and data registers. Whatever comes back
; (a report, an empty answer, a NAK code) tells whether the input path
; works at all - completely independent of the input-register mechanism.
        DEBUGF 1, "i2chid: still silent, asking with GET_REPORT\n"
        mov     eax, [CmdReg]
        mov     word [diag_buf], ax
        mov     eax, [f_repid]
        and     eax, 15
        or      eax, 0x10               ; report type = input
        mov     [diag_buf+2], al
        mov     byte [diag_buf+3], 0x02 ; GET_REPORT
        mov     eax, [DataReg]
        mov     word [diag_buf+4], ax
        stdcall dw_read_block, [SlaveAddr], diag_buf, 6, input_buf, 12
        DEBUGF 1, "i2chid: GET_REPORT probe status %x, data:", eax
        mov     esi, input_buf
        mov     ecx, 12
        call    dump_hex
        jmp     .done
.try_fallback:
; The touchpad report, once the pad has been switched into touchpad mode.
        cmp     [PtpLayoutOk], 0
        je      .no_ptp_route
        cmp     ecx, [t_ptp_id]
        je      .ptp
.no_ptp_route:
; The SIPODEV firmware lies in its report descriptor: the Linux DMI override
; documents report id 1 as a plain mouse frame [buttons][dX][dY]. Accept
; that when the id does not match what the device claimed about itself.
        cmp     [IsSp1064], 0
        je      .done
        cmp     ecx, 1
        jne     .done
        cmp     eax, 4                  ; id byte + 3 payload bytes
        jb      .done
        cmp     [FbLogged], 0
        jnz     @f
        mov     [FbLogged], 1
        DEBUGF 2, "i2chid: using SP1064 fallback report layout\n"
@@:
        movzx   ebx, byte [esi+1]
        and     ebx, 3
        movsx   eax, byte [esi+2]
        mov     [v_x], eax
        movsx   eax, byte [esi+3]
        neg     eax
        mov     [v_y], eax
        mov     [v_w], 0
        jmp     .send
.ptp:
; Touchpad frame. The fields are wherever this pad's descriptor put them;
; ptp_decode feeds every contact into the table and hands back the pointer
; contact, the count and the buttons in the Ptp* variables. eax = payload
; length including the id byte, esi -> the id byte.
        lea     ecx, [eax-1]
        cmp     ecx, [t_ptp_bytes]
        jb      .done                   ; short frame: not one of ours
        call    ptp_decode
; The first few frames with a finger on the pad go to the log raw, always:
; they show what the pad really sends, which no descriptor can.
        cmp     [PtpTip], 0
        jz      @f
        cmp     [PtpRawTrace], 0
        jz      @f
        dec     [PtpRawTrace]
        push    eax esi
        DEBUGF 2, "i2chid: touch frame:"
        mov     esi, input_buf
        mov     ecx, 16
        call    dump_hex_raw
        pop     esi eax
@@:
        cmp     [FbLogged], 0
        jnz     @f
        mov     [FbLogged], 1
        DEBUGF 1, "i2chid: PTP frames flowing, single-finger decode\n"
@@:
        mov     ebx, [PtpBtn]
        mov     [PtpMode], 1
; The first few frames that carry any button bits go to the log raw: if the
; firmware does distinguish its buttons, these bytes will show how.
        test    ebx, ebx
        jz      @f
        cmp     [BtnTrace], 0
        jz      @f
        dec     [BtnTrace]
        push    eax esi
        DEBUGF 1, "i2chid: button frame:"
        mov     esi, input_buf
        mov     ecx, 32
        call    dump_hex
        pop     esi eax
@@:
; A clickpad reports every physical click as button 1 and the host decides
; what it meant. A click that starts with the finger inside the right-click
; zone is a right click; the decision is latched for as long as the button
; stays down, so a drag cannot mutate mid-way.
        cmp     ebx, 1
        jne     .btn_done               ; no click, or firmware sent bit1/2
        cmp     [BtnLatch], 0
        jne     .btn_latched
; Fingers on the pad while pressing choose the button when the setting
; asks for it (bit 1 of the mode): two give the right button, three the
; middle one.
        test    [RightClick], 2
        jz      .try_corner
        mov     ecx, [PtpCnt]
        cmp     ecx, 3
        jb      @f
        movi    ebx, 4                  ; middle
        jmp     .btn_done
@@:
        cmp     ecx, 2
        jb      .try_corner
        movi    ebx, 2                  ; right
        jmp     .btn_done
.try_corner:
; A press in the bottom-right corner of the surface, likewise (bit 0).
        test    [RightClick], 1
        jz      .btn_done
        cmp     [PtpTip], 0             ; finger on the pad?
        jz      .btn_done
        mov     ecx, [PtpX]
        cmp     ecx, [RightSplitX]
        jb      .btn_done
        mov     ecx, [PtpY]
        cmp     ecx, [BtnZoneY]
        jb      .btn_done
        movi    ebx, 2
        jmp     .btn_done
.btn_latched:
        mov     ebx, [BtnLatch]
.btn_done:
        mov     [BtnLatch], ebx         ; the latch tracks the physical click only
; A button pressed while the finger is down makes this a click, and a
; click is not a tap: without this, lifting after a physical click on a
; clickpad produced a second click on the way up. Only the pad's own
; buttons count here - the button a drag holds must not cancel the tap
; that a short dragging touch may turn out to be.
        test    ebx, ebx
        jz      @f
        mov     [TapMoved], 1
@@:
        mov     [v_x], 0
        mov     [v_y], 0
        mov     [v_w], 0
        cmp     [PtpTip], 0             ; tip switch
        jz      .ptp_up
        mov     ecx, [PtpX]             ; absolute X
        mov     edx, [PtpY]             ; absolute Y
        mov     [PayloadLen], eax
        cmp     [PrevTip], 0
        jz      .ptp_first
; Two fingers on a long frame = scroll: vertical finger motion becomes the
; wheel and the cursor stays put.
        cmp     [PtpCnt], 2             ; contact count
        jb      .no_scroll
; Remember how many fingers this touch ever had: a tap with two of them
; means the right button, with three the middle one. The tap is only
; cancelled by actual travel, checked further down, so a quick two-finger
; tap still counts as one.
        mov     eax, [PtpCnt]
        cmp     eax, [TapFingers]
        jbe     @f
        mov     [TapFingers], eax
@@:
        call    tap_travel_check
; Finger travel accumulates until it is worth a whole wheel step. The
; delta is taken as PrevY - Y rather than the other way round, which is
; what makes the scrolling natural: the content follows the fingers, so
; moving them down pulls the view up. Negating here rather than after the
; division keeps the sign of the carried-over remainder consistent.
        mov     eax, [PrevY]
        sub     eax, edx
        cmp     [ScrollInvert], 0
        je      @f
        neg     eax
@@:
        add     eax, [ScrollAcc]
        mov     ecx, [ScrollStep]
        cdq
        idiv    ecx                     ; eax = steps, edx = remainder
        mov     [ScrollAcc], edx
        mov     [v_w], eax
        mov     ecx, [PtpX]             ; idiv clobbered ecx
        mov     edx, [PtpY]
        jmp     .ptp_store
.no_scroll:
        cmp     [LeftoverTouch], 0
        jne     .ptp_store              ; the tail of a gesture steers nothing
; X through the hysteresis filter: FiltX is the position the pointer is
; standing on, and it is dragged along only by the part of the movement
; that exceeds the margin. Which margin depends on where the touch began:
; a finger on the buttons is aiming, not steering, and needs a stricter
; one than a finger tracking across the middle of the pad.
        mov     edi, [Jitter]
        cmp     [StartedInBtn], 0
        jz      @f
        mov     edi, [JitterBtn]
@@:
        mov     eax, [LiftTick]         ; = now, ptp_decode stamps every frame
        sub     eax, [TapTick]
        cmp     eax, TAP_SETTLE
        ja      @f
        add     edi, edi                ; the finger is still settling
@@:
        mov     [Margin], edi
        mov     eax, ecx
        sub     eax, [FiltX]
        cmp     eax, [Margin]
        jg      .fx_fwd
        neg     eax
        cmp     eax, [Margin]
        jg      .fx_back_neg
        xor     eax, eax                ; inside the margin: stand still
        jmp     .fx_done
.fx_back_neg:
        neg     eax
        jmp     .fx_back
.fx_fwd:
        sub     eax, [Margin]
        add     [FiltX], eax
        jmp     .fx_done
.fx_back:
        add     eax, [Margin]
        add     [FiltX], eax
.fx_done:
; Units to pixels: units * speed / 200, the remainder carried over. edx
; holds the frame's Y and has to survive the division.
        imul    eax, [Speed]
        add     eax, [MoveAccX]
        push    edx
        cdq
        mov     edi, 200
        idiv    edi
        mov     [MoveAccX], edx
        pop     edx
        mov     [v_x], eax
; Y likewise, then negated: the pad counts downwards, the kernel upwards.
        mov     eax, edx
        sub     eax, [FiltY]
        cmp     eax, [Margin]
        jg      .fy_fwd
        neg     eax
        cmp     eax, [Margin]
        jg      .fy_back_neg
        xor     eax, eax
        jmp     .fy_done
.fy_back_neg:
        neg     eax
        jmp     .fy_back
.fy_fwd:
        sub     eax, [Margin]
        add     [FiltY], eax
        jmp     .fy_done
.fy_back:
        add     eax, [Margin]
        add     [FiltY], eax
.fy_done:
        imul    eax, [Speed]
        add     eax, [MoveAccY]
        push    edx
        cdq
        mov     edi, 200
        idiv    edi
        mov     [MoveAccY], edx
        pop     edx
        neg     eax
        mov     [v_y], eax
; A big jump in one frame is never a finger; the first few go to the log
; with everything needed to see where they came from.
        cmp     [JumpTrace], 0
        jz      .no_jump
        mov     eax, [v_x]
        push    edx
        cdq
        xor     eax, edx
        sub     eax, edx                ; |dx|
        cmp     eax, JUMP_PX
        ja      .jump
        mov     eax, [v_y]
        cdq
        xor     eax, edx
        sub     eax, edx
        cmp     eax, JUMP_PX
        jbe     .jump_no
.jump:
        dec     [JumpTrace]
        mov     eax, [LastFrameTick]
        sub     eax, [PrevFrameTick]
        DEBUGF 2, "i2chid: jump: dx %d dy %d, prev %u/%u now %u/%u, pointer %d, %u ticks since last frame, held %u:", \
                [v_x], [v_y], [PrevX], [PrevY], [PtpX], [PtpY], [PointerId], eax, [HeldBtn]
        push    esi ecx
        mov     esi, input_buf
        mov     ecx, 16
        call    dump_hex_raw
        pop     ecx esi
.jump_no:
        pop     edx
.no_jump:
        call    tap_travel_check
        jmp     .ptp_store
.ptp_first:
        invoke  GetTimerTicks
        mov     [TapTick], eax
        mov     [TapX], ecx
        mov     [TapY], edx
        mov     [TapMoved], 0
        mov     [ScrollAcc], 0
; Right after a multi-finger tap the finger that left last looks like a
; fresh touch; it must not tap on its own. (This has to come after the
; reset above, not before it.)
        sub     eax, [MultiTapTick]
        cmp     eax, DRAG_WINDOW
        ja      @f
        mov     [TapMoved], 1
@@:
        mov     eax, [TapTick]
        sub     eax, [LiftGuardTick]
        cmp     eax, DRAG_WINDOW
        ja      @f
        mov     [TapMoved], 1           ; successor of a lifted pointer
@@:
; The pointer starts standing on the landing spot. Whether the touch began
; on the button strip is decided once, here, and holds for the whole
; touch: it selects the stricter jitter margin and blocks taps there,
; while a finger wandering in from above keeps its normal feel.
        mov     [FiltX], ecx
        mov     [FiltY], edx
        mov     [MoveAccX], 0
        mov     [MoveAccY], 0
        mov     [TapFingers], 1
        mov     eax, [PtpCnt]           ; fingers already down
        test    eax, eax
        jz      @f
        mov     [TapFingers], eax
@@:
; A touch that begins while a tap's press is still held continues it: the
; button simply stays down for as long as this touch lasts. Whether the
; touch is a drag or the second tap of a double tap is decided when it
; ends, so it is not marked as moved here.
        mov     [DragActive], 0
        cmp     [TapHold], 0
        jz      .no_drag
        mov     [TapHold], 0
        mov     [DragActive], 1         ; HeldBtn stays 1: still down
.no_drag:
; The finger that stays after the pointer finger of a multi-finger gesture
; has left is the tail of that gesture, not a touch of its own: it neither
; taps nor steers.
        mov     eax, [Leftover]
        mov     [LeftoverTouch], eax
        test    eax, eax
        jz      @f
        mov     [TapMoved], 1
@@:
        mov     [StartedInBtn], 0
        mov     [TapInStrip], 0
        cmp     edx, [BtnZoneY]
        jb      @f
        mov     [StartedInBtn], 1
        mov     [TapInStrip], 1         ; a one-finger tap there is a rest, not a click
@@:
.ptp_store:
        mov     [PrevX], ecx
        mov     [PrevY], edx
        mov     [PrevTip], 1
        jmp     .send
.ptp_up:
        cmp     [PrevTip], 0
        jz      .send
        mov     [PrevTip], 0
; A thumb handing the pointer over to a finger is not a lift anyone should
; hear about: no tap, no drag end, no trace.
        cmp     [Handover], 0
        je      @f
        mov     [Handover], 0
        jmp     .send
@@:
        mov     [LiftPending], 1        ; traced at .send, with the outcome
; The end of a drag is just a release. The button the drag was holding has
; to be taken out of this very frame, not the next one: the pad falls
; silent once the finger is gone, so a button still set here would stay
; pressed with nothing left to release it. BtnLatch holds what the pad
; itself reports, which is what should remain.
        cmp     [DragActive], 0
        jz      @f
        mov     [DragActive], 0
        mov     [HeldBtn], 0
        mov     ebx, [BtnLatch]         ; the held button goes up now
; A short, still touch was the second tap of a double tap: its click is
; sent as a fresh press on the next cycle (the kernel needs the edge), and
; that press is then held like the first one was. A touch that moved was
; a drag, and the release above is all it needs.
        cmp     [TapMoved], 0
        jne     .send
        mov     eax, [LiftTick]
        sub     eax, [TapTick]
        cmp     eax, TAP_MAX_TICKS
        ja      .send
        mov     [PendingPress], 1
        jmp     .send
@@:
; The finger just left. A short touch that never moved and never pressed
; the button becomes a left click; the release goes out on the next poll.
        cmp     [TapEnable], 0
        je      .send
        cmp     [TapMoved], 0
        jne     .send
        test    ebx, ebx
        jnz     .send
; A thumb resting on the button strip must not click, but a two-finger
; tap is a deliberate gesture wherever it lands.
        cmp     [TapInStrip], 0
        je      @f
        cmp     [TapFingers], 1
        je      .send
@@:
        mov     eax, [LiftTick]
        sub     eax, [TapTick]
        cmp     eax, TAP_MAX_TICKS
        ja      .send
; One finger taps the left button, two the right one, three the middle.
        movi    ebx, 1
        cmp     [TapFingers], 2
        jb      @f
; The fingers of a multi-finger tap rarely leave together. On a pad that
; reports one contact per frame the one still down after the click looks
; like a fresh touch and would tap again on its own; remember when this
; click went out so that such a touch is not allowed to.
        invoke  GetTimerTicks
        mov     [MultiTapTick], eax
        movi    ebx, 2
        cmp     [TapFingers], 3
        jb      @f
        movi    ebx, 4
@@:
; Right and middle taps are a pulse, long enough to be seen. A one-finger
; tap holds its press for the drag window instead, unless dragging by tap
; is turned off.
        mov     [HeldBtn], ebx
        mov     [TapRelease], TAP_PULSE
        cmp     [TapFingers], 1
        jne     .send
        cmp     [TapDrag], 0
        jz      .send
        mov     [TapRelease], 0
        mov     [TapHold], 1
        mov     eax, [LiftTick]
        mov     [TapHoldTick], eax
        jmp     .send
.err:
        inc     [ErrCnt]
        cmp     [ErrCnt], MAX_POLL_ERRORS
        jb      .done
        DEBUGF 2, "i2chid: device stopped responding (%x), polling disabled\n", eax
        mov     [PollStop], 1
        jmp     .done
endp

include 'dwi2c.inc'
include 'hidmin.inc'

; =============================================================================
section '.data' readable writable
include '../peimport.inc'
include_debug_strings

my_service      db 'I2CHID', 0

; Read request for the configuration file, in the layout the kernel file
; system expects: subfunction, 64-bit offset, length, buffer, then the
; name as a zero byte followed by a pointer.
align 4
ini_req         dd 0            ; subfunction 0 = read file
                dd 0, 0         ; offset
                dd INI_MAX
                dd ini_buf
                db 0
                dd ini_path
ini_path        db '/sys/settings/system.ini', 0
sec_name        db '[touchpad]'
sec_name_len    = $ - sec_name
key_speed       db 'speed', 0
key_jitter      db 'jitter', 0
key_jitter_btn  db 'jitter_buttons', 0
key_scroll_step db 'scroll_step', 0
key_scroll_invert db 'scroll_invert', 0
key_tap         db 'tap_enable', 0
key_tap_drag    db 'tap_drag', 0
key_right_click db 'right_click', 0
key_btn_zone    db 'button_zone', 0
key_right_split db 'right_split', 0

; I2C addresses that laptop firmware assigns to HID touchpads and touchscreens
; (Synaptics, Elan, Cypress, Focaltech, Goodix, Atmel and friends). Addresses
; used by non-HID devices that share these buses - EEPROMs at 50..57, audio
; codecs at 1A/1C, fuel gauges at 36, RTCs at 68 - are deliberately absent.
hid_addrs       db 0x2C, 0x15, 0x2A, 0x1F, 0x10, 0x20, 0x2B, 0x0A
                db 0x05, 0x06, 0x07, 0x14, 0x2D, 0x3C, 0x49, 0x4A
                db 0x4B, 0x5D
hid_addrs_cnt   = $ - hid_addrs

; Fixed MMIO bases of the AMD FCH I2C masters, in probing order (see
; detect_amd_fixed). Zero terminates.
align 4
amd_i2c_bases   dd 0xFEDC4000, 0xFEDC5000, 0xFEDC2000, 0xFEDC3000, 0xFEDC6000, 0

align 4
; Controller / device state.
I2cMmio         dd 0
FixedBase       dd 0
; DesignWare timing in effect, set by whichever bring-up path ran.
DwHcnt          dd DW_FS_HCNT
DwLcnt          dd DW_FS_LCNT
DwSdaHold       dd DW_SDA_HOLD
PciHead         dd 0
PciBus          dd 0
PciDevfn        dd 0
SlaveAddr       dd 0
CmdReg          dd 0
InputReg        dd 0
DataReg         dd 0
IsSp1064        dd 0
FbLogged        dd 0
ModeRepId       dd 0
ModeRepLen      dd 0
BtnLatch        dd 0
BtnTrace        dd 4
TapTick         dd 0
TapX            dd 0
TapY            dd 0
TapMoved        dd 0
TapRelease      dd 0
ScrollAcc       dd 0
PtpMode         dd 0
PtpLayoutOk     dd 0
; Finger 0, contact count and buttons of the frame being decoded.
PtpTip          dd 0
PtpX            dd 0
PtpY            dd 0
PtpCnt          dd 0
PtpBtn          dd 0
Consumed        dd 0
DrainLeft       dd 0
ReadLen         dd MAX_INPUT_BUF
MultiTapTick    dd 0
PtpRawTrace     dd 6
PtpCid          dd 0
PointerId       dd -1
LiftTick        dd 0
LiftTrace       dd 32
LiftPending     dd 0
LiftGuardTick   dd 0
StDups          dd 0
; Polling statistics.
StPolls         dd 0
StReads         dd 0
StReports       dd 0
StEmpty         dd 0
StMaxDrain      dd 0
StartedInBtn    dd 0
FiltX           dd 0
FiltY           dd 0
MoveAccX        dd 0
MoveAccY        dd 0
; Settings, all overridable from the configuration file.
Speed           dd CFG_SPEED
Jitter          dd CFG_JITTER
JitterBtn       dd CFG_JITTER_BTN
ScrollStep      dd CFG_SCROLL_STEP
ScrollInvert    dd CFG_SCROLL_INVERT
TapEnable       dd CFG_TAP
TapDrag         dd CFG_TAP_DRAG
RightClick      dd CFG_RIGHT_CLICK
BtnZonePct      dd CFG_BTN_ZONE_PCT
RightSplitPct   dd CFG_RIGHT_SPLIT_PCT
; Derived from the percentages above once the surface extents are known.
BtnZoneY        dd 930
RightSplitX     dd 853
; Bounds of the driver's section inside the configuration text.
IniStart        dd 0
IniEnd          dd 0
Margin          dd CFG_JITTER
PayloadLen      dd 0
TapFingers      dd 1
DragActive      dd 0
TapHold         dd 0
TapHoldTick     dd 0
PendingPress    dd 0
LiveMask        dd 0
LeftoverMask    dd 0
Leftover        dd 0
LeftoverTouch   dd 0
TapInStrip      dd 0
HeldBtn         dd 0
Handover        dd 0
SeenMask        dd 0
NewMask         dd 0
NtCnt           dd 0
LowestNt        dd 0
RawCnt          dd 0
SlotTip         dd 0
Payload         dd 0
LastFrameTick   dd 0
PrevFrameTick   dd 0
ForceLift       dd 0
StForced        dd 0
JumpTrace       dd 8
PrevTip         dd 0
PrevX           dd 0
PrevY           dd 0
PollMode        dd 0
RecStep         dd 0
IdleTrace       dd 3
IdleChanges     dd 8
RepDescReg      dd 0
RepDescLen      dd 0
MaxInput        dd 0
PollStop        dd 0
ErrCnt          dd 0
TimeoutStreak   dd 0
XferTimeout     dd DW_TIMEOUT_INIT
; Diagnostics. The trace counters make the driver noisy for the first few
; reports only; the per-controller probe counters are reset by scan_bus.
CtrlCount       dd 0
AckCount        dd 0
NakCount        dd 0
TmoCount        dd 0
TraceCount      dd 8
DecodeTrace     dd 8
GotAny          dd 0
SilentPolls     dd 0

; Parser state (see hidmin.inc).
p_usage_page    dd 0
p_report_size   dd 0
p_report_count  dd 0
p_report_id     dd 0
p_uses_repid    dd 0
p_usage_cnt     dd 0
p_usage_min     dd 0
p_usage_max     dd 0
p_log_max       dd 0
p_idcnt         dd 0
p_usages        rd HID_MAX_USAGES
p_ids           rd HID_MAX_REPIDS
p_bits          rd HID_MAX_REPIDS
p_fbits         rd HID_MAX_REPIDS
v_flags         dd 0
v_base          dd 0

; Candidate report layout found by the parser.
t_id            dd 0
t_have          dd 0
t_btnid         dd 0
t_btnoff        dd 0
t_btncnt        dd 0
t_xoff          dd 0
t_xsize         dd 0
t_yoff          dd 0
t_ysize         dd 0
t_woff          dd 0
t_wsize         dd 0
; Surface extents, taken from the report descriptor.
; Touchpad report layout, all bit offsets relative to the byte after the id.
t_ptp_id        dd -1
t_ptp_bytes     dd 0
t_fcount        dd 0
t_ftip          rd HID_MAX_FINGERS
t_fx            rd HID_MAX_FINGERS
t_fxs           rd HID_MAX_FINGERS
t_fy            rd HID_MAX_FINGERS
t_fys           rd HID_MAX_FINGERS
t_fcid          rd HID_MAX_FINGERS
t_fcids         rd HID_MAX_FINGERS
t_ccnt_off      dd 0
t_ccnt_size     dd 0
t_pbtn_off      dd 0
t_pbtn_cnt      dd 0
t_padmaxx       dd 0
t_padmaxy       dd 0

; Final layout.
f_repid         dd 0
f_bytes         dd 0

; Scratch for the poll routine.
v_x             dd 0
v_y             dd 0
v_w             dd 0

align 4
hexbuf          rb 32*3+4       ; one line of dump_hex output
wbuf            rb 4
pollreg         rb 4
diag_buf        rb 8
setrep_buf      rb 24
idle_prev       rb 16
ini_buf         rb INI_MAX+4
prev_rep        rb MAX_INPUT_BUF
contacts        rb PTP_SLOTS*SLOT_SIZE
cmd_buf         rb 8
hid_desc        rb I2CHID_DESC_LEN+6
input_buf       rb MAX_INPUT_BUF+8
rep_desc        rb MAX_REPDESC+8
