
; Macroinstructions for making import section
; Based on import32.inc from FASM, with small modifications
; and list of actual kernel exports.

macro library [name,string]
 { common
    import.data:
   forward
    local _label
    if defined name#.redundant
     if ~ name#.redundant
      dd 0,0,0,RVA _label,RVA name#.address
     end if
    end if
    name#.referred = 1
   common
    dd 0,0,0,0,0
   forward
    if defined name#.redundant
     if ~ name#.redundant
      _label db string,0
     end if
    end if }

macro import name,[label]
 { common
    rb (- rva $) and 3
    if defined name#.referred
     name#.address:
   forward
     if used label
       local _label
       label dd RVA _label
       label#._pe_import = 1
     end if
   common
     if $ > name#.address
      name#.redundant = 0
      dw 0
     else
      name#.redundant = 1
     end if
   forward
     if used label
     _label dw 0
            db `label
     end if
   common
     db 0
    end if }

align 4
data import
library core,'core.dll'
import core,\
              RegService,\
              GetService,\
              ServiceHandler,\
              AttachIntHandler,\
              GetIntHandler,\
              FpuSave,\
              FpuRestore,\
              ReservePortArea,\
              Boot_Log,\
\
          MutexInit,\
          MutexLock,\
          MutexUnlock,\
\
              PciApi,\
              PciRead32,\
              PciRead16,\
              PciRead8,\
              PciWrite8,\
              PciWrite16,\
              PciWrite32,\
\
              AllocPage,\
              AllocPages,\
              FreePage,\
              MapPage,\
              MapSpace,\
              MapIoMem,\
              GetPgAddr,\
              GetPhysAddr,\
              CommitPages,\
              ReleasePages,\
\
              AllocKernelSpace,\
              FreeKernelSpace,\
              KernelAlloc,\
              KernelFree,\
              UserAlloc,\
              UserFree,\
              Kmalloc,\
              Kfree,\
              CreateRingBuffer,\
              AllocDMA24,\
\
              GetPid,\
              CreateThread,\
              CreateObject,\
              DestroyObject,\
              CreateEvent,\
              RaiseEvent,\
              WaitEvent,\
              DestroyEvent,\
              ClearEvent,\
\
              LoadCursor,\
              SelectHwCursor,\
              SetHwCursor,\
              HwCursorRestore,\
              HwCursorCreate,\
\
              SysMsgBoardStr,\
              SysMsgBoard,\
              GetCurrentTask,\
              LoadFile,\
              SendEvent,\
              SetMouseData,\
              SetKeyboardData,\
              RegKeyboard,\
              DelKeyboard,\
              Sleep,\
              GetTimerTicks,\
\
              strncat,\
              strncpy,\
              strncmp,\
              strnlen,\
              strchr,\
              strrchr,\
\
              LFBAddress,\
              GetDisplay,\
              SetScreen,\
\
              RegUSBDriver,\
              USBOpenPipe,\
              USBNormalTransferAsync,\
              USBControlTransferAsync,\
              USBGetParam,\
              USBHCFunc,\
\
              DiskAdd,\
              DiskMediaChanged,\
              DiskDel,\
\
              TimerHS,\
              CancelTimerHS,\
\
              NetRegDev,\
              NetUnRegDev,\
              NetPtrToNum,\
              NetLinkChanged,\
              EthInput,\
              NetAlloc,\
              NetFree,\
\
              GetPCIList
end data
