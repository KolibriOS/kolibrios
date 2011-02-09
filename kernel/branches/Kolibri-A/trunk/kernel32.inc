;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2008. All rights reserved. ;;
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



struc POINT {
  .x dd ?
  .y dd ?
  .sizeof:
}
virtual at 0
 POINT POINT
end virtual

struc RECT {
  .left   dd ?
  .top	  dd ?
  .right  dd ?
  .bottom dd ?
  .sizeof:
}
virtual at 0
 RECT RECT
end virtual

struc BOX {
  .left   dd ?
  .top	  dd ?
  .width  dd ?
  .height dd ?
  .sizeof:
}
virtual at 0
 BOX BOX
end virtual

struc DISPMODE {
  .width   rw 1
  .height  rw 1
  .bpp	   rw 1
  .freq    rw 1
}

; constants definition
WSTATE_NORMAL	 = 00000000b
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

TSTATE_RUNNING	      = 0
TSTATE_RUN_SUSPENDED  = 1
TSTATE_WAIT_SUSPENDED = 2
TSTATE_ZOMBIE	      = 3
TSTATE_TERMINATING    = 4
TSTATE_WAITING	      = 5
TSTATE_FREE	      = 9

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
  .sizeof:
}
virtual at 0
 WDATA WDATA
end virtual
label WDATA.fl_wstyle byte at WDATA.cl_workarea + 3

struc APPDATA
{
  .app_name	   db 11  dup(?)
		   db 5   dup(?)

  .fpu_state	   dd ? 	   ;+16
  .ev_count_	   dd ? ;unused    ;+20
  .exc_handler	   dd ? 	   ;+24
  .except_mask	   dd ? 	   ;+28
  .pl0_stack	   dd ? ;unused    ;+32
  .heap_base	   dd ? 	   ;+36
  .heap_top	   dd ? 	   ;+40
  .cursor	   dd ? 	   ;+44
  .fd_ev	   dd ? 	   ;+48
  .bk_ev	   dd ? 	   ;+52
  .fd_obj	   dd ? 	   ;+56
  .bk_obj	   dd ? 	   ;+60
  .saved_esp	   dd ? 	   ;+64
  .io_map	   rd 2 	   ;+68
  .dbg_state	   dd ? 	   ;+76
  .cur_dir	   dd ? 	   ;+80
  .wait_timeout    dd ? 	   ;+84
  .saved_esp0	   dd ? 	   ;+88
  .wait_begin	   dd ? 	   ;+92   +++
  .wait_test	   dd ? 	   ;+96   +++
  .wait_param	   dd ? 	   ;+100  +++
  .tls_base	   dd ? 	   ;+104
  .dlls_list_ptr   dd ? 	   ;+108
		   db 16 dup(?)    ;+112

  .wnd_shape	   dd ? 	   ;+128
  .wnd_shape_scale dd ? 	   ;+132
		   dd ? 	   ;+136
  .mem_size	   dd ? 	   ;+140
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

struc MUTEX
{
    .count  rd 1
    .next   rd 1
    .prev   rd 1
}

virtual at 0
 MUTEX MUTEX
end virtual


; Core functions
include "core/sync.inc"     ; macros for synhronization objects
include "core/sys32.inc"    ; process management
include "core/sched.inc"    ; process scheduling
include "core/syscall.inc"  ; system call
include "core/fpu.inc"	    ; all fpu/sse support
include "core/memory.inc"
include "core/heap.inc"     ; kernel and app heap
include "core/malloc.inc"   ; small kernel heap
include "core/taskman.inc"
include "core/dll.inc"
include "core/peload.inc"   ;
include "core/exports.inc"
include "core/string.inc"
include "core/v86.inc"	    ; virtual-8086 manager

; GUI stuff
include "gui/window.inc"
include "gui/event.inc"
include "gui/font.inc"
include "gui/button.inc"

; shutdown

; file system

include "fs/fs.inc"	  ; syscall
include "fs/fat32.inc"	  ; read / write for fat32 filesystem
include "fs/ntfs.inc"	  ; read / write for ntfs filesystem
include "fs/fat12.inc"	  ; read / write for fat12 filesystem
include "blkdev/rd.inc"   ; ramdisk read /write
include "fs/fs_lfn.inc"    ; syscall, version 2
;include "fs/iso9660.inc"  ; read for iso9660 filesystem CD
include "fs/ext2.inc"	  ; read / write for ext2 filesystem

; sound

include "sound/playnote.inc"	; player Note for Speaker PC
;include "sound/FHT.inc" 	; fast Fourier transform routines

; display

include "video/graph32.inc"   	; 32bpp graphics
include "video/cursors.inc"  	; cursors functions

; Network Interface & TCPIP Stack

include "network/stack.inc"

;include "drivers/uart.inc"


; Mouse pointer

include "gui/mouse.inc"

; Window skinning

include "gui/skincode.inc"

; Pci functions
include "bus/pci/pci32.inc"
;include "bus/pci/PCIe.inc"
include "bus/HT.inc"		; AMD HyperTransport bus control

; Floppy drive controller

include "blkdev/fdc.inc"
include "blkdev/flp_drv.inc"

; IDE cache
include "blkdev/ide_cache.inc"

; HD drive controller
include "blkdev/hd_drv.inc"

; CD drive controller

;include "blkdev/cdrom.inc"
;include "blkdev/cd_drv.inc"

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

