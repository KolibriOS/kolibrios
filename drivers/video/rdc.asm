; RDC M2010/M2012 video driver.

; Standard driver stuff
format PE DLL native at 0
entry start
__DEBUG__ equ 1
__DEBUG_LEVEL__ equ 1
section '.flat' readable writable executable
include '../proc32.inc'
include '../struct.inc'
include '../macros.inc'
include '../fdo.inc'

; Display-specific driver stuff
; Kernel passes to init_cursor cursors with fixed size 32x32
KERNEL_CURSOR_WIDTH = 32
KERNEL_CURSOR_HEIGHT = 32

; Constants for IOCTL codes
SRV_GETVERSION = 0
SRV_ENUM_MODES = 1
SRV_SET_MODE   = 2

; Constants for SRV_GETVERSION result
CURRENT_API     = 0x0200
COMPATIBLE_API  = 0x0100
API_VERSION     = (COMPATIBLE_API shl 16) + CURRENT_API

struct  RWSEM
        wait_list       LHEAD
        count           dd ?
ends

; Some structures
struct  display_t
        x               dd ?
        y               dd ?
        width           dd ?
        height          dd ?
        bits_per_pixel  dd ?
        vrefresh        dd ?
        lfb             dd ?
        lfb_pitch       dd ?

        win_map_lock    RWSEM
        win_map         dd ?
        win_map_pitch   dd ?
        win_map_size    dd ?

        modes           dd ?
        ddev            dd ?
        connector       dd ?
        crtc            dd ?

        cr_list.next    dd ?
        cr_list.prev    dd ?

        cursor          dd ?

        init_cursor     dd ?
        select_cursor   dd ?
        show_cursor     dd ?
        move_cursor     dd ?
        restore_cursor  dd ?
        disable_mouse   dd ?
        mask_seqno      dd ?
        check_mouse     dd ?
        check_m_pixel   dd ?

        bytes_per_pixel dd ?
ends

struct  APPOBJ                  ; common object header
        magic           dd ?    ;
        destroy         dd ?    ; internal destructor
        fd              dd ?    ; next object in list
        bk              dd ?    ; prev object in list
        pid             dd ?    ; owner id
ends

struct  CURSOR          APPOBJ
        base            dd ?   ;allocated memory
        hot_x           dd ?   ;hotspot coords
        hot_y           dd ?

        list_next       dd ?   ;next cursor in cursor list
        list_prev       dd ?   ;prev cursor in cursor list
        dev_obj         dd ?   ;device depended data
ends

; Constants specific to our drivers
; We are handling two videocards: M2010 and M2012
PCI_VENDOR_RDC          = 0x17F3
; I like this approach to select device IDs!
PCI_CHIP_M2010          = 0x2010
PCI_CHIP_M2012          = 0x2012

; I/O ports for CRT registers
COLOR_CRTC_INDEX        = 0x3D4
COLOR_CRTC_DATA         = 0x3D5
; The value that unlocks extended CRT registers:
; index 0x80, value 0xA8
ENABLE_EXTENDED_REGS    = 0xA880

; Hardware cursors have size 64x64
HW_CURSOR_WIDTH = 64
HW_CURSOR_HEIGHT = 64
; Multiplication to powers of two can be replaced with shifts,
; x*HW_CURSOR_WIDTH = x shl HW_CURSOR_WIDTH_SHIFT
HW_CURSOR_WIDTH_SHIFT = 6
HW_CURSOR_HEIGHT_SHIFT = 6

; MMIO registers responsible for hardware cursor, see PRM
HWC_MMIO_CTRL           = 0x580
HWC_MMIO_OFFSET         = 0x584
HWC_MMIO_POSITION       = 0x588
HWC_MMIO_ADDRESS        = 0x58C

; Data for hardware cursors must be stored in videomemory,
; so we need an allocator for objects inside videomemory.
; Currently, we just reserve a fixed amount of memory for cursors
; (cursor size is the same for all cursors) and keep a bitfield
; that describes free blocks. 32 bits fit nicely in one dword.
MAX_CURSORS = 32

; === Entry points for external code ===

; Called once when driver is loading and once at shutdown.
; When loading, must initialize itself, register itself in the system
; and return eax = value obtained when registering.
; Cdecl with two parameters.
proc start
        push    ebx esi ; save used registers to be stdcall
virtual at esp
                rd      2 ; saved registers
                dd      ? ; return address
.reason         dd      ? ; DRV_ENTRY or DRV_EXIT
.cmdline        dd      ? ; normally NULL
end virtual
; 1. Check the reason for the call, do nothing unless initializing.
        cmp     [.reason], DRV_ENTRY
        jnz     .nothing
; 2. Find the PCI device for our videocard.
; If not found, just return zero.
        invoke  GetPCIList
        mov     ebx, eax
.look_pcidev_loop:
        mov     ebx, [ebx+PCIDEV.fd]
        cmp     ebx, eax
        jz      .pcidev_notfound
        cmp     [ebx+PCIDEV.vendor_device_id], PCI_VENDOR_RDC + (PCI_CHIP_M2010 shl 16)
        jz      .pcidev_found
        cmp     [ebx+PCIDEV.vendor_device_id], PCI_VENDOR_RDC + (PCI_CHIP_M2012 shl 16)
        jnz     .look_pcidev_loop
.pcidev_found:
; 3. Get addresses, sizes and pointers from the hardware.
; 3a. Create mapping for MMIO.
        invoke  PciRead32, dword [ebx+PCIDEV.bus], dword [ebx+PCIDEV.devfn], 14h
        and     al, not 0xF
        invoke  MapIoMem, eax, 10000h, PG_NOCACHE+PG_SW
        test    eax, eax
        jz      .nothing
        mov     [mmio], eax
; 3b. Get videomemory size. It is stored in 3 lower bits of 0xAA extended register
; logarithmically started with 8Mb.
        mov     dx, COLOR_CRTC_INDEX
        mov     ax, ENABLE_EXTENDED_REGS
        out     dx, ax
        mov     al, 0xAA
        out     dx, al
        inc     edx
        in      al, dx
        and     eax, 7
        mov     ecx, eax
        mov     eax, 8 shl 20
        shl     eax, cl
        mov     [video_mem_size], eax
; 3c. Reserve area for cursors in the last part of videomemory.
        sub     eax, MAX_CURSORS * HW_CURSOR_WIDTH * HW_CURSOR_HEIGHT * 4
        mov     [cursors_base_offset], eax
; 3d. Create mapping for part of videomemory that we have reserved for cursors.
; Note: we can't just use system-wide mapping at 0xFE000000, it is too short.
        invoke  PciRead32, dword [ebx+PCIDEV.bus], dword [ebx+PCIDEV.devfn], 10h
        and     al, not 0xF
        add     eax, [cursors_base_offset]
        invoke  MapIoMem, eax, MAX_CURSORS * HW_CURSOR_WIDTH * HW_CURSOR_HEIGHT * 4, PG_SW
        test    eax, eax
        jz      .nothing
        mov     [cursors_base_va], eax
; 4. Install cursor handlers.
; 4a. Get pointer to the structure that keeps everything display-related.
        invoke  GetDisplay
        mov     ebx, eax
; 4b. Make sure that no one tries to use partially-changed structure.
        pushf
        cli
; 4c. Ask the previous handler to restore image hidden beyond cursor.
        stdcall [ebx+display_t.restore_cursor], 0, 0
; 4d. Store pointers to our functions.
        mov     [ebx+display_t.init_cursor], init_cursor
        mov     [ebx+display_t.select_cursor], select_cursor
        mov     [ebx+display_t.show_cursor], 0
        mov     [ebx+display_t.move_cursor], move_cursor
        mov     [ebx+display_t.restore_cursor], restore_cursor
        mov     [ebx+display_t.disable_mouse], disable_mouse
; 4e. The kernel will pass all new cursors to our init_cursor,
; but we must process already created cursors ourselves to be able to
; select_cursor them when requested. Do it now: pass every cursor
; in the list display_t.cr_list to init_cursor.
        add     ebx, display_t.cr_list.next
        mov     esi, ebx
.init_old_cursors_loop:
        mov     esi, [esi]
        cmp     esi, ebx
        jz      .init_old_cursors_done
        lea     eax, [esi-CURSOR.list_next]
        push    eax
        call    init_cursor
        pop     eax
        jmp     .init_old_cursors_loop
.init_old_cursors_done:
; 4f. Setup the current cursor.
        stdcall move_cursor, [ebx+display_t.cursor-display_t.cr_list.next], 0, 0
        stdcall select_cursor, [ebx+display_t.cursor-display_t.cr_list.next]
; 4g. It is safe now to work with display structure; restore after 4b.
        popf
; 5. Say something happy to the (curious) user.
        mov     esi, success_msg
        invoke  SysMsgBoardStr
; 6. Register ourselves as a service.
; Note: not really needed currently as we don't do any useful in ioctl_handler,
; but do it nevertheless for future expansion.
        invoke  RegService, rdc_name, ioctl_handler
.nothing:
        pop     esi ebx ; restore used registers to be stdcall
        ret
.pcidev_notfound:
        xor     eax, eax
        jmp     .nothing
endp

; Service procedure for the driver - handle all IOCTL requests for the driver.
; Stdcall with one parameter.
proc ioctl_handler
virtual at esp
                dd      ?       ; return address
.ioctl          dd      ?
end virtual
; Not very useful currently - just return API_VERSION as a response to SRV_GETVERSION = 0.
        mov     edx, [.ioctl]
        mov     eax, [edx+IOCTL.io_code]
        test    eax, eax
        jz      .getversion
.error:
        or      eax, -1         ; fail everything unknown
        retn    4
.getversion:
        cmp     [edx+IOCTL.out_size], 4
        jnz     .error
        mov     eax, [edx+IOCTL.output]
        mov     dword [eax], API_VERSION
        xor     eax, eax
        retn    4
endp

; === Cursors ===

; This function is called when an application registers a new cursor.
; Cdecl with one parameter, return value ignored.
proc init_cursor
        push    esi edi
virtual at esp
                rd      2       ; saved registers
                dd      ?       ; return address
.cursor         dd      ?
end virtual
; We store one specific dword in CURSOR.dev_obj,
; index in cursors area from 0 to NUM_CURSORS-1, or -1 for error.
; 1. Prepare: store -1 to CURSOR.dev_obj and pointer to destroy function.
        mov     edx, [.cursor]
        mov     [edx+CURSOR.dev_obj], -1
        mov     [edx+CURSOR.destroy], destroy_cursor
; 2. Allocate videomemory.
        bsr     edi, [free_cursors]
        jz      .nocopy
        btr     [free_cursors], edi
; 3. Store the allocated item to CURSOR.dev_obj.
        mov     [edx+CURSOR.dev_obj], edi
; 4. Copy data from kernel-provided cursor to videomemory,
; transforming KERNEL_CURSOR_WIDTH*KERNEL_CURSOR_HEIGHT*(4 bytes RGBA) to
; HW_CURSOR_WIDTH*HW_CURSOR_HEIGHT*(4 bytes RGBA).
        shl     edi, HW_CURSOR_WIDTH_SHIFT + HW_CURSOR_HEIGHT_SHIFT + 2
        add     edi, [cursors_base_va]
        mov     esi, [edx+CURSOR.base]
        push    KERNEL_CURSOR_HEIGHT
        xor     eax, eax
@@:
        mov     ecx, KERNEL_CURSOR_WIDTH
        rep movsd
        mov     ecx, HW_CURSOR_WIDTH - KERNEL_CURSOR_WIDTH
        rep stosd
        dec     dword [esp]
        jnz     @b
        pop     ecx
        mov     ecx, HW_CURSOR_WIDTH * (HW_CURSOR_HEIGHT - KERNEL_CURSOR_HEIGHT)
        rep stosd
.nocopy:
; 5. We don't need kernel-provided data anymore; free it now.
        invoke  KernelFree, [edx+CURSOR.base]
        pop     edi esi
        ret
endp

; This function is called when a thread that has created a cursor
; is terminating and we need to free the cursor.
proc destroy_cursor
; 1. Free allocated videomemory.
        mov     edx, [eax+CURSOR.dev_obj]
        test    edx, edx
        js      .nofree
        bts     [free_cursors], edx
.nofree:
; 2. Remove the cursor from the overall list at display_t.cr_list.
        pushf
        cli
        mov     ecx, [eax+CURSOR.list_next]
        mov     edx, [eax+CURSOR.list_prev]
        mov     [ecx+4], edx
        mov     [edx], ecx
        popf
; 3. Free memory allocated for kernel object.
        jmp     [DestroyObject]
endp

; This function is called when cursor shape needs to be changed,
; either due to explicit request from application
; or due to moving from one window to another.
; Stdcall with one parameter, return value ignored.
proc select_cursor
virtual at esp
                dd      ?       ; return address
.cursor         dd      ?
end virtual
        mov     eax, [.cursor]
        mov     eax, [eax+CURSOR.dev_obj]
        cmp     eax, -1
        jz      .nothing
; Setup base address of cursor, relative to videomemory,
; and enable hardware cursor.
; See PRM for details.
        shl     eax, HW_CURSOR_WIDTH_SHIFT + HW_CURSOR_HEIGHT_SHIFT + 2
        add     eax, [cursors_base_offset]
        shr     eax, 3
        mov     edx, [mmio]
        mov     [edx+HWC_MMIO_ADDRESS], eax
        mov     dword [edx+HWC_MMIO_CTRL], (1 shl 31) + (1 shl 1) + (1 shl 0)
.nothing:
        ret     4
endp

; This function is called when cursor is moved to a new place.
; Stdcall with three parameters, return value ignored.
proc move_cursor
virtual at esp
                dd      ?       ; return address
.cursor         dd      ?
.x              dd      ?
.y              dd      ?
.xoffset        dd      ?
.yoffset        dd      ?
end virtual
; If cursor x is smaller than hotspot x,
; only part of cursor is shown at position x=0
; with x offset = (hotspot x) - (cursor x).
; Otherwise, the entire cursor is shown (x offset = 0)
; at position x = (cursor x) - (hotspot x).
; Similar for y. Refer to PRM for details.
        xor     ecx, ecx
        mov     edx, [.cursor]
        mov     eax, [.x]
        sub     eax, [edx+CURSOR.hot_x]
        jae     @f
        sub     ecx, eax
        xor     eax, eax
@@:
        mov     [.x], eax
        mov     eax, [.y]
        sub     eax, [edx+CURSOR.hot_y]
        jae     @f
        shl     eax, 8
        sub     ecx, eax
        xor     eax, eax
@@:
        mov     [.y], eax
        mov     edx, [mmio]
        mov     eax, [.y]
        add     eax, HW_CURSOR_HEIGHT - 1       ; no scaling
        shl     eax, 16
        add     eax, ecx
        mov     [edx+HWC_MMIO_OFFSET], eax
        mov     eax, [.y]
        shl     eax, 16
        add     eax, [.x]
        mov     [edx+HWC_MMIO_POSITION], eax
        mov     eax, [edx+HWC_MMIO_CTRL]
        mov     [edx+HWC_MMIO_CTRL], eax
        ret     12
endp

; Stdcall with two parameters, return value ignored.
proc restore_cursor
; No-operation for hardware cursors.
        ret     8
endp

; No parameters, return value ignored.
proc disable_mouse
; No-operation for hardware cursors.
        ret
endp

; === Data ===
rdc_name db 'DISPLAY',0
success_msg db 'RDC: using hardware cursors',13,10,0

align 4
; Look at the comment before definition of NUM_CURSORS.
free_cursors    dd      0xFFFFFFFF

data fixups
end data

include '../peimport.inc'
;include_debug_strings
IncludeIGlobals
IncludeUGlobals
align 4
mmio            dd      ?       ; virtual address of MMIO for our device
video_mem_size  dd      ?       ; total size of video memory, in bytes
cursors_base_offset     dd      ?       ; base of cursor data, relative to video memory start
cursors_base_va         dd      ?       ; mapped virtual address of cursor data
