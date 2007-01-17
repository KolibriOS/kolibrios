;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                            ;;
;; KERNEL32.INC                                               ;;
;;                                                            ;;
;; Included 32 bit kernel files for MenuetOS                  ;;
;;                                                            ;;
;; This file is kept separate as it will be easier to         ;;
;; maintain and compile with an automated SETUP program       ;;
;; in the future.                                             ;;
;;                                                            ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; structure definition helper
macro struct name, [arg]
 {
  common
   name@struct equ name
   struc name arg {
 }

macro struct_helper name
 {
  match xname,name
  \{
   virtual at 0
   xname xname
   sizeof.#xname = $ - xname
   name equ sizeof.#xname
   end virtual
  \}
 }

ends fix } struct_helper name@struct

;// mike.dld, 2006-29-01 [

; macros definition
macro diff16 title,l1,l2
{
  local s,d
  s = l2-l1
  display title,': 0x'
  repeat 8
    d = 48 + s shr ((8-%) shl 2) and $0F
    if d > 57
      d = d + 65-57-1
    end if
    display d
  end repeat
  display 13,10
}

; \begin{diamond}[29.09.2006]
; may be useful for kernel debugging
; example 1:
;       dbgstr 'Hello, World!'
; example 2:
;       dbgstr 'Hello, World!', save_flags
macro dbgstr string*, f
{
local a
iglobal_nested
a db 'K : ',string,13,10,0
endg_nested
if ~ f eq
        pushfd
end if
        push    esi
        mov     esi, a
        call    sys_msg_board_str
        pop     esi
if ~ f eq
        popfd
end if
}
; \end{diamond}[29.09.2006]

struc db [a] { common . db a
  if ~used .
    display 'not used db: ',`.,13,10
  end if }
struc dw [a] { common . dw a
  if ~used .
    display 'not used dw: ',`.,13,10
  end if }
struc dd [a] { common . dd a
  if ~used .
    display 'not used dd: ',`.,13,10
  end if }
struc dp [a] { common . dp a
  if ~used .
    display 'not used dp: ',`.,13,10
  end if }
struc dq [a] { common . dq a
  if ~used .
    display 'not used dq: ',`.,13,10
  end if }
struc dt [a] { common . dt a
  if ~used .
    display 'not used dt: ',`.,13,10
  end if }

struc RECT {
  .left   dd ?
  .top    dd ?
  .right  dd ?
  .bottom dd ?
}
virtual at 0
 RECT RECT
end virtual

struc BOX {
  .left   dd ?
  .top    dd ?
  .width  dd ?
  .height dd ?
}
virtual at 0
 BOX BOX
end virtual

; constants definition
WSTATE_NORMAL    = 00000000b
WSTATE_MAXIMIZED = 00000001b
WSTATE_MINIMIZED = 00000010b
WSTATE_ROLLEDUP  = 00000100b

WSTATE_REDRAW	 = 00000001b
WSTATE_WNDDRAWN  = 00000010b

WSTYLE_HASCAPTION     = 00010000b
WSTYLE_CLIENTRELATIVE = 00100000b

struc TASKDATA
{
  .event_mask  dd ?
  .pid	       dd ?
	       dw ?
  .state       db ?
               db ?
	       dw ?
  .wnd_number  db ?
	       db ?
  .mem_start   dd ?
  .counter_sum dd ?
  .counter_add dd ?
  .cpu_usage   dd ?
}
virtual at 0
 TASKDATA TASKDATA
end virtual

; structures definition
struc WDATA {
  .box		   BOX
  .cl_workarea	   dd ?
  .cl_titlebar	   dd ?
  .cl_frames	   dd ?
  .reserved	   db ?
  .fl_wstate	   db ?
  .fl_wdrawn	   db ?
  .fl_redraw	   db ?
}
virtual at 0
 WDATA WDATA
end virtual
label WDATA.fl_wstyle byte at 0x13

struc APPDATA
{
  .app_name	   db 11  dup(?)
		   db 5   dup(?)

  .fpu_state       dd ?            ;+16
  .ev_count        dd ?            ;+20
  .fpu_handler     dd ?            ;+24
  .sse_handler     dd ?            ;+28
  .event           dd ?            ;+32
  .heap_base       dd ?            ;+36
  .heap_top        dd ?            ;+40
  .cursor          dd ?            ;+44
  .ev_first        dd ?            ;+48
  .ev_last         dd ?            ;+52

                   db 72 dup(?)    ;+56

  .wnd_shape       dd ?            ;+128
  .wnd_shape_scale dd ?            ;+132
                   dd ?            ;+136
  .mem_size        dd ?            ;+140
  .saved_box	   BOX
  .ipc_start	   dd ?
  .ipc_size	   dd ?
  .event_mask	   dd ?
  .debugger_slot   dd ?
		   dd ?
  .keyboard_mode   db ?
		   db 3   dup(?)
  .dir_table	   dd ?
  .dbg_event_mem   dd ?
  .dbg_regs:
  .dbg_regs.dr0    dd ?
  .dbg_regs.dr1    dd ?
  .dbg_regs.dr2    dd ?
  .dbg_regs.dr3    dd ?
  .dbg_regs.dr7    dd ?
  .wnd_caption	   dd ?
  .wnd_clientbox   BOX
}
virtual at 0
 APPDATA APPDATA
end virtual

;// mike.dld, 2006-29-01 ]


; Core functions
include "core/sync.inc"     ; macros for synhronization objects
include "core/sys32.inc"    ; process management
include "core/sched.inc"    ; process scheduling
include "core/syscall.inc"  ; system call
include "core/fpu.inc"      ; all fpu/sse support
include "core/memory.inc"
include "core/heap.inc"     ; kernel and app heap
include "core/malloc.inc"   ; small kernel heap
include "core/taskman.inc"
include "core/dll.inc"
include "core/exports.inc"

; GUI stuff
include "gui/window.inc"
include "gui/event.inc"
include "gui/font.inc"
include "gui/button.inc"

; shutdown

include "boot/shutdown.inc" ; shutdown or restart

; file system

include "fs/fs.inc"       ; syscall
include "fs/fat32.inc"    ; read / write for fat32 filesystem
include "fs/ntfs.inc"     ; read / write for ntfs filesystem
include "fs/fat12.inc"    ; read / write for fat12 filesystem
include "blkdev/rd.inc"   ; ramdisk read /write
include "fs/fs_lfn.inc"    ; syscall, version 2
include "fs/iso9660.inc"  ; read for iso9660 filesystem CD

; sound

include "sound/sb16.inc"     ; playback for Sound Blaster 16
include "sound/playnote.inc" ; player Note for Speaker PC

; display

include "video/vesa12.inc"   ; Vesa 1.2 functions
include "video/vesa20.inc"   ; Vesa 2.0 functions
include "video/vga.inc"      ; VGA 16 color functions
include "video/cursors.inc"  ; cursors functions

; Network Interface & TCPIP Stack

include "network/stack.inc"

; Mouse pointer

include "gui/mouse.inc"

; Window skinning

include "gui/skincode.inc"

; Pci functions

include "bus/pci/pci32.inc"

; Floppy drive controller

include "blkdev/fdc.inc"
include "blkdev/flp_drv.inc"

; HD drive controller
include "blkdev/hd_drv.inc"

; CD drive controller

include "blkdev/cdrom.inc"
include "blkdev/cd_drv.inc"

; Character devices

include "hid/keyboard.inc"
include "hid/mousedrv.inc"

; setting date,time,clock and alarm-clock

include "hid/set_dtc.inc"

;% -include
