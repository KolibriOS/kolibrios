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
   name@struct fix name
   struc name arg {
 }

macro struct_helper name
 {
   virtual at 0
   name name
   sizeof.#name = $ - name
   name equ sizeof.#name
   end virtual
 }

ends fix } struct_helper name@struct


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
include "blkdev/rd.inc"       ; ramdisk read /write

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

; setting date,time,clock and alarm-clock

include "hid/set_dtc.inc"

;% -include
