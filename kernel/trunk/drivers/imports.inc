
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
if used PciRead8
  extrn PciRead8
end if
if used PciWrite8
  extrn PciWrite8
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

if used CreateObject
  extrn CreateObject
end if
if used DestroyObject
  extrn DestroyObject
end if

if used LoadCursor
  extrn LoadCursor
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
if used LFBAddress
  extrn LFBAddress
end if

