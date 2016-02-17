;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2015. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;; KERNEL32.INC                                                 ;;
;;                                                              ;;
;; Included 32 bit kernel files for MenuetOS                    ;;
;;                                                              ;;
;; This file is kept separate as it will be easier to           ;;
;; maintain and compile with an automated SETUP program         ;;
;; in the future.                                               ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

; Core functions
include "core/sync.inc"     ; macros for synhronization objects
include "core/sys32.inc"    ; process management
include "core/sched.inc"    ; process scheduling
include "core/syscall.inc"  ; system call
include "core/fpu.inc"      ; all fpu/sse support
include "core/memory.inc"
include "core/mtrr.inc"
include "core/heap.inc"     ; kernel and app heap
include "core/malloc.inc"   ; small kernel heap
include "core/taskman.inc"
include "core/dll.inc"
include "core/peload.inc"   ;
include "core/exports.inc"
include "core/string.inc"
include "core/v86.inc"      ; virtual-8086 manager
include "core/irq.inc"      ; irq handling functions
include "core/apic.inc"     ; Interrupt Controller functions
include "core/timers.inc"
include "core/clipboard.inc" ; custom clipboard

; GUI stuff
include "gui/window.inc"
include "gui/event.inc"
include "gui/font.inc"
include "gui/button.inc"

include "boot/shutdown.inc" ; kernel shutdown

; file system

include "blkdev/disk.inc" ; support for plug-n-play disks
include "blkdev/disk_cache.inc" ; caching for plug-n-play disks
include "blkdev/rd.inc"   ; ramdisk read /write
include "fs/fat.inc"      ; read / write for fat filesystem
include "fs/ntfs.inc"     ; read / write for ntfs filesystem
include "fs/fs_lfn.inc"    ; syscall, version 2
include "fs/iso9660.inc"  ; read for iso9660 filesystem CD
include "fs/ext2/ext2.asm"     ; read / write for ext2 filesystem
include "fs/xfs.asm"      ; read / write for xfs filesystem

; sound

include "sound/playnote.inc" ; player Note for Speaker PC

; display

;include "video/vesa12.inc"   ; Vesa 1.2 functions
include "video/vesa20.inc"   ; Vesa 2.0 functions
include "video/blitter.inc"  ;
include "video/vga.inc"      ; VGA 16 color functions
include "video/cursors.inc"  ; cursors functions

; Network Interface & TCPIP Stack

include "network/stack.inc"

;include "drivers/uart.inc"


; Mouse pointer

include "gui/mouse.inc"

; Window skinning

include "gui/skincode.inc"

; Pci functions

include "bus/pci/pci32.inc"

; USB functions
include "bus/usb/init.inc"

; Floppy drive controller

include "blkdev/fdc.inc"
include "blkdev/flp_drv.inc"

; IDE cache
include "blkdev/ide_cache.inc"

; HD drive controller
include "blkdev/hd_drv.inc"
; Access through BIOS
include "blkdev/bd_drv.inc"

; CD drive controller

include "blkdev/cd_drv.inc"

; Character devices

include "hid/keyboard.inc"
include "hid/mousedrv.inc"

; setting date,time,clock and alarm-clock

include "hid/set_dtc.inc"

;% -include

;parser file names
include "fs/parse_fn.inc"

; work with conf lib
include "core/conf_lib.inc"

; load external lib
include "core/ext_lib.inc"

; list of external functions
include "imports.inc"
