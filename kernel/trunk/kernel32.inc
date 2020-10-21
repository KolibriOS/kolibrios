;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2016. All rights reserved. ;;
;;  Distributed under terms of the GNU General Public License.  ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

include "core/sync.inc"         ; macros for synhronization objects
include "core/sys32.inc"        ; process management
include "core/sched.inc"        ; process scheduling
include "core/syscall.inc"
include "core/fpu.inc"          ; all fpu/sse support
include "core/memory.inc"
include "core/mtrr.inc"
include "core/heap.inc"
include "core/malloc.inc"       ; small kernel heap
include "core/taskman.inc"
include "core/dll.inc"
include "core/peload.inc"
include "core/exports.inc"
include "core/string.inc"
include "core/v86.inc"          ; 16-bit mode machine
include "core/irq.inc"          ; interrupt handling functions
include "core/apic.inc"
include "core/hpet.inc"
include "core/timers.inc"
include "core/clipboard.inc"
include "core/slab.inc"

include "acpi/acpi.inc"

include "posix/posix.inc"

include "boot/shutdown.inc"     ; kernel shutdown

include "video/vesa20.inc"
include "video/blitter.inc"
include "video/vga.inc"         ; VGA 16 color functions
include "video/cursors.inc"
include "video/framebuffer.inc"

include "gui/window.inc"
include "gui/event.inc"
include "gui/font.inc"
include "gui/button.inc"
include "gui/mouse.inc"         ; cursor
include "gui/skincode.inc"      ; windows' skin

include "hid/keyboard.inc"
include "hid/mousedrv.inc"
include "hid/set_dtc.inc"       ; setting date,time,clock and alarm-clock

include "sound/playnote.inc"    ; player Note for PC Speaker

include "bus/pci/pci32.inc"
include "bus/usb/init.inc"

include "blkdev/flp_drv.inc"    ; floppy driver
include "blkdev/fdc.inc"
include "blkdev/cd_drv.inc"     ; CD driver
include "blkdev/ide_cache.inc"  ; CD cache
include "blkdev/hd_drv.inc"     ; HDD driver
include "blkdev/bd_drv.inc"     ; BIOS disks driver
include "blkdev/rd.inc"         ; ramdisk driver
include "blkdev/disk.inc"       ; support for plug-n-play disks
include "blkdev/disk_cache.inc" ; caching for plug-n-play disks

include "fs/fs_lfn.inc"         ; sysfunction 70

include "network/stack.inc"

include "crc.inc"               ; checksums
include "unicode.inc"

; include "imports.inc"
; include "core/ext_lib.inc"
; include "core/conf_lib.inc"
