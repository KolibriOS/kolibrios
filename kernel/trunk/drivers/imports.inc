;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
macro kernel_export [name]{
forward
  if used name
        if DEBUG
        display 'uses: ',`name,#13,#10
        end if
    extrn name
  end if
}
; all exported kernel functions and data


kernel_export \
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
\
              GetPid,\
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
              SysMsgBoardChar,\
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
              DiskAdd,\
              DiskMediaChanged,\
              DiskDel
