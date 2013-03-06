;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2011. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision $


iglobal
  szKernel            db 'KERNEL', 0
  szVersion           db 'version',0

  szRegService        db 'RegService',0
  szGetService        db 'GetService',0
  szServiceHandler    db 'ServiceHandler',0
  szAttachIntHandler  db 'AttachIntHandler',0
;  szGetIntHandler     db 'GetIntHandler', 0
  szFpuSave           db 'FpuSave',0
  szFpuRestore        db 'FpuRestore',0
  szReservePortArea   db 'ReservePortArea',0
  szBoot_Log          db 'Boot_Log',0

  szMutexInit         db 'MutexInit',0
  szMutexLock         db 'MutexLock',0
  szMutexUnlock       db 'MutexUnlock',0

  szPciApi            db 'PciApi', 0
  szPciRead32         db 'PciRead32', 0
  szPciRead16         db 'PciRead16', 0
  szPciRead8          db 'PciRead8', 0
  szPciWrite8         db 'PciWrite8',0
  szPciWrite16        db 'PciWrite16',0
  szPciWrite32        db 'PciWrite32',0

  szAllocPage         db 'AllocPage',0
  szAllocPages        db 'AllocPages',0
  szFreePage          db 'FreePage',0
  szGetPgAddr         db 'GetPgAddr',0
  szMapPage           db 'MapPage',0
  szMapSpace          db 'MapSpace',0
  szMapIoMem          db 'MapIoMem',0
  szCommitPages       db 'CommitPages',0
  szReleasePages      db 'ReleasePages',0

  szAllocKernelSpace  db 'AllocKernelSpace',0
  szFreeKernelSpace   db 'FreeKernelSpace',0
  szKernelAlloc       db 'KernelAlloc',0
  szKernelFree        db 'KernelFree',0
  szUserAlloc         db 'UserAlloc',0
  szUserFree          db 'UserFree',0
  szKmalloc           db 'Kmalloc',0
  szKfree             db 'Kfree',0
  szCreateRingBuffer  db 'CreateRingBuffer',0

  szGetPid            db 'GetPid',0
  szCreateObject      db 'CreateObject',0
  szDestroyObject     db 'DestroyObject',0
  szCreateEvent       db 'CreateEvent',0
  szRaiseEvent        db 'RaiseEvent',0
  szWaitEvent         db 'WaitEvent',0
  szDestroyEvent      db 'DestroyEvent',0
  szClearEvent        db 'ClearEvent',0

  szLoadCursor        db 'LoadCursor',0

  szSysMsgBoardStr    db 'SysMsgBoardStr', 0
  szSysMsgBoardChar   db 'SysMsgBoardChar', 0
  szGetCurrentTask    db 'GetCurrentTask',0
  szLFBAddress        db 'LFBAddress',0
  szLoadFile          db 'LoadFile',0
  szSendEvent         db 'SendEvent',0
  szSetMouseData      db 'SetMouseData',0
  szSetKeyboardData   db 'SetKeyboardData',0
  szRegKeyboard       db 'RegKeyboard',0
  szDelKeyboard       db 'DelKeyboard',0
  szSleep             db 'Sleep',0
  szGetTimerTicks     db 'GetTimerTicks',0

  szGetDisplay        db 'GetDisplay',0
  szSetScreen         db 'SetScreen',0

  szStrncat           db 'strncat',0
  szStrncpy           db 'strncpy',0
  szstrncmp           db 'strncmp',0
  szStrnlen           db 'strnlen',0
  szStrchr            db 'strchr',0
  szStrrchr           db 'strrchr',0

  szDiskAdd           db 'DiskAdd',0
  szDiskDel           db 'DiskDel',0
  szDiskMediaChanged  db 'DiskMediaChanged',0

  szTimerHS           db 'TimerHS',0
  szCancelTimerHS     db 'CancelTimerHS',0

  szNetRegDev         db 'NetRegDev',0
  szNetUnRegDev       db 'NetUnRegDev',0
  szNetPtrToNum       db 'NetPtrToNum',0
  szNetLinkChanged    db 'NetLinkChanged',0
  szEth_input         db 'Eth_input',0
  szIPv4_input        db 'IPv4_input',0


align 16
kernel_export:
  dd szRegService      , reg_service
  dd szGetService      , get_service
  dd szServiceHandler  , srv_handler
  dd szAttachIntHandler, attach_int_handler
;  dd szGetIntHandler   , get_int_handler
  dd szFpuSave         , fpu_save
  dd szFpuRestore      , fpu_restore
  dd szReservePortArea , r_f_port_area
  dd szBoot_Log        , boot_log

  dd szMutexInit       , mutex_init            ;gcc fastcall
  dd szMutexLock       , mutex_lock            ;gcc fastcall
  dd szMutexUnlock     , mutex_unlock          ;gcc fastcall

  dd szPciApi          , pci_api_drv
  dd szPciRead32       , pci_read32
  dd szPciRead16       , pci_read16
  dd szPciRead8        , pci_read8
  dd szPciWrite8       , pci_write8
  dd szPciWrite16      , pci_write16
  dd szPciWrite32      , pci_write32

  dd szAllocPage       , alloc_page            ;stdcall
  dd szAllocPages      , alloc_pages           ;stdcall
  dd szFreePage        , free_page
  dd szMapPage         , map_page              ;stdcall
  dd szMapSpace        , map_space
  dd szMapIoMem        , map_io_mem            ;stdcall
  dd szGetPgAddr       , get_pg_addr
  dd szCommitPages     , commit_pages          ;not implemented
  dd szReleasePages    , release_pages

  dd szAllocKernelSpace, alloc_kernel_space    ;stdcall
  dd szFreeKernelSpace , free_kernel_space     ;stdcall
  dd szKernelAlloc     , kernel_alloc          ;stdcall
  dd szKernelFree      , kernel_free           ;stdcall
  dd szUserAlloc       , user_alloc            ;stdcall
  dd szUserFree        , user_free             ;stdcall
  dd szKmalloc         , malloc
  dd szKfree           , free
  dd szCreateRingBuffer, create_ring_buffer    ;stdcall

  dd szGetPid          , get_pid
  dd szCreateObject    , create_kernel_object
  dd szDestroyObject   , destroy_kernel_object
  dd szCreateEvent     , create_event          ;see EVENT.inc for specification
  dd szRaiseEvent      , raise_event           ;see EVENT.inc for specification
  dd szWaitEvent       , wait_event            ;see EVENT.inc for specification
  dd szDestroyEvent    , destroy_event         ;see EVENT.inc for specification
  dd szClearEvent      , clear_event           ;see EVENT.inc for specification

  dd szLoadCursor      , load_cursor           ;stdcall

  dd szSysMsgBoardStr  , sys_msg_board_str
  dd szSysMsgBoardChar , sys_msg_board
  dd szGetCurrentTask  , get_curr_task
  dd szLoadFile        , load_file             ;retval eax, ebx
  dd szSendEvent       , send_event            ;see EVENT.inc for specification
  dd szSetMouseData    , set_mouse_data        ;stdcall
  dd szSetKeyboardData , set_keyboard_data
  dd szRegKeyboard     , register_keyboard
  dd szDelKeyboard     , delete_keyboard
  dd szSleep           , delay_ms
  dd szGetTimerTicks   , get_timer_ticks

  dd szGetDisplay      , get_display
  dd szSetScreen       , set_screen

  dd szStrncat         , strncat
  dd szStrncpy         , strncpy
  dd szstrncmp         , strncmp
  dd szStrnlen         , strnlen
  dd szStrchr          , strchr
  dd szStrrchr         , strrchr

  dd szDiskAdd         , disk_add
  dd szDiskDel         , disk_del
  dd szDiskMediaChanged, disk_media_changed

  dd szTimerHS         , timer_hs
  dd szCancelTimerHS   , cancel_timer_hs

  dd szNetRegDev       , NET_add_device
  dd szNetUnRegDev     , NET_remove_device
  dd szNetPtrToNum     , NET_ptr_to_num
  dd szNetLinkChanged  , NET_link_changed
  dd szEth_input       , ETH_input
  dd szIPv4_input      , IPv4_input

exp_lfb:
  dd szLFBAddress      , 0
  dd 0                      ;terminator, must be zero

endg
