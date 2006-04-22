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

; constants definition
WSTATE_NORMAL    = 00000000b
WSTATE_MAXIMIZED = 00000001b
WSTATE_MINIMIZED = 00000010b
WSTATE_ROLLEDUP  = 00000100b

; structures definition
struc WDATA {
  .left        dd ?
  .top         dd ?
  .width       dd ?
  .height      dd ?
  .cl_workarea dd ?
  .cl_titlebar dd ?
  .cl_frames   dd ?
  .reserved    db ?
  .fl_wstate   db ?
  .fl_wdrawn   db ?
  .fl_redraw   db ?
}
virtual at 0
 WDATA WDATA
end virtual
label WDATA.fl_wstyle byte at 0x13 ; WDATA.cl_workarea+3

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

;// mike.dld, 2006-29-01 ]


; Core functions
include "core/sync.inc"
include "core/sys32.inc"    ; process management
include "core/sched.inc"    ; process scheduling
include "core/syscall.inc"  ; system call
include "core/mem.inc"      ; high-level memory management
include "core/newproce.inc" ;new process management
include "core/physmem.inc"  ; access to physical memory for applications
include "core/sync.inc"     ; macros for synhronization objects

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
include "fs/fat12.inc"    ; read / write for fat12 filesystem
include "blkdev/rd.inc"   ; ramdisk read /write
include "fs/fs_v2.inc"    ; syscall, version 2

; sound

include "sound/sb16.inc"     ; playback for Sound Blaster 16
include "sound/playnote.inc" ; player Note for Speaker PC

; display

include "video/vesa12.inc"   ; Vesa 1.2 functions
include "video/vesa20.inc"   ; Vesa 2.0 functions
include "video/vga.inc"      ; VGA 16 color functions

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

; CD drive controller

include "blkdev/cdrom.inc"

; Character devices

include "hid/keyboard.inc"
include "hid/mousedrv.inc"

; setting date,time,clock and alarm-clock

include "hid/set_dtc.inc"

;% -include
