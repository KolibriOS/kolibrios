
iglobal
  szKernel            db 'KERNEL', 0
  szVersion           db 'version',0
  szAttachIntHandler  db 'AttachIntHandler',0
  szSysMsgBoardStr    db 'SysMsgBoardStr', 0
  szPciApi            db 'PciApi', 0
  szPciRead32         db 'PciRead32', 0
  szPciRead8          db 'PciRead8', 0
  szPciWrite8         db 'PciWrite8',0
  szAllocKernelSpace  db 'AllocKernelSpace',0
  szMapPage           db 'MapPage',0
  szRegService        db 'RegService',0
  szKernelAlloc       db 'KernelAlloc',0
  szKernelFree        db 'KernelFree',0
  szGetPgAddr         db 'GetPgAddr',0
  szGetCurrentTask    db 'GetCurrentTask',0
  szGetService        db 'GetService',0
  szServiceHandler    db 'ServiceHandler',0
  szFpuSave           db 'FpuSave',0
  szFpuRestore        db 'FpuRestore',0
  szLoadCursor        db 'LoadCursor',0
  szSetHwCursor       db 'SetHwCursor',0
  szHwCursorRestore   db 'HwCursorRestore', 0
  szHwCursorCreate    db 'HwCursorCreate', 0
  szLFBAddress        db 'LFBAddress',0
  szLoadFile          db 'LoadFile',0
  szSendEvent         db 'SendEvent',0

align 16
kernel_export:
  dd szAttachIntHandler, attach_int_handler
  dd szSysMsgBoardStr  , sys_msg_board_str
  dd szPciApi          , pci_api
  dd szPciRead32       , pci_read32
  dd szPciRead8        , pci_read8
  dd szPciWrite8       , pci_write8
  dd szAllocKernelSpace, alloc_kernel_space
  dd szMapPage         , map_page
  dd szRegService      , reg_service
  dd szKernelAlloc     , kernel_alloc
  dd szKernelFree      , kernel_free
  dd szGetPgAddr       , get_pg_addr
  dd szGetCurrentTask  , get_curr_task
  dd szGetService      , get_service
  dd szServiceHandler  , srv_handler
  dd szFpuSave         , fpu_save
  dd szFpuRestore      , fpu_restore
  dd szLoadCursor      , load_cursor
  dd szSetHwCursor     , set_hw_cursor
  dd szHwCursorRestore , hw_restore
  dd szHwCursorCreate  , create_cursor
  dd szLoadFile        , load_file
  dd szSendEvent       , send_event
exp_lfb:
  dd szLFBAddress      , 0
  dd 0

endg

