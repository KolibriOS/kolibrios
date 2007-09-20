;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; all exported kernel functions and data

if used RegService
  extrn RegService
end if
if used GetService
  extrn GetService
end if
if used ServiceHandler
  extrn ServiceHandler
end if
if used AttachIntHandler
  extrn AttachIntHandler
end if
if used FpuSave
  extrn FpuSave
end if
if used FpuRestore
  extrn FpuRestore
end if

if used PciApi
  extrn PciApi
end if
if used PciRead32
  extrn PciRead32
end if
if used PciRead16
 extrn PciRead16
end if
if used PciRead8
  extrn PciRead8
end if
if used PciWrite8
  extrn PciWrite8
end if
if used PciWrite16
  extrn PciWrite16
end if

if used AllocPage
  extrn AllocPage
end if
if used AllocPages
  extrn AllocPages
end if
if used FreePage
  extrn FreePage
end if
if used MapPage
  extrn MapPage
end if
if used MapSpace
  extrn MapSpace
end if
if used GetPgAddr
  extrn GetPgAddr
end if
if used CommitPages
  extrn CommitPages
end if
if used ReleasePages
  extrn ReleasePages
end if

if used AllocKernelSpace
  extrn AllocKernelSpace
end if
if used FreeKernelSpace
  extrn FreeKernelSpace
end if
if used KernelAlloc
  extrn KernelAlloc
end if
if used KernelFree
  extrn KernelFree
end if
if used UserAlloc
  extrn UserAlloc
end if
if used UserFree
  extrn UserFree
end if
if used Kmalloc
  extrn Kmalloc
end if
if used Kfree
  extrn Kfree
end if
if used CreateRingBuffer
  extrn CreateRingBuffer
end if

if used GetPid
  extrn GetPid
end if
if used CreateObject
  extrn CreateObject
end if
if used DestroyObject
  extrn DestroyObject
end if
if used CreateEvent
  extrn CreateEvent
end if
if used RaiseEvent
  extrn RaiseEvent
end if
if used WaitEvent
  extrn WaitEvent
end if
if used DestroyEvent
  extrn DestroyEvent
end if
if used ClearEvent
  extrn ClearEvent
end if

if used LoadCursor
  extrn LoadCursor
end if

if used SelectHwCursor
  extrn SelectHwCursor
end if
if used SetHwCursor
  extrn SetHwCursor
end if
if used HwCursorRestore
  extrn HwCursorRestore
end if
if used HwCursorCreate
  extrn HwCursorCreate
end if

if used SysMsgBoardStr
  extrn SysMsgBoardStr
end if
if used GetCurrentTask
  extrn GetCurrentTask
end if
if used LoadFile
  extrn LoadFile
end if
if used SendEvent
  extrn SendEvent
end if
if used SetMouseData
  extrn SetMouseData
end if
if used Sleep
  extrn Sleep
end if
if used GetTimerTicks
  extrn GetTimerTicks
end if

if used strncat
  extrn strncat
end if
if used strncpy
  extrn strncpy
end if
if used strncmp
  extrn strncmp
end if
if used strnlen
  extrn strnlen
end if
if used strchr
  extrn strchr
end if
if used strrchr
  extrn strrchr
end if

if used LFBAddress
  extrn LFBAddress
end if

