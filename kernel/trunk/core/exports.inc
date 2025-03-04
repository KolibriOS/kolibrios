;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


 align 4
__exports:
          export 'KERNEL',   \
          alloc_kernel_space,    'AllocKernelSpace',   \      ; stdcall
          alloc_page,            'AllocPage',          \      ; gcc ABI
          alloc_pages,           'AllocPages',         \      ; stdcall
          commit_pages,          'CommitPages',        \      ; eax, ebx, ecx
\
          disk_add,              'DiskAdd',            \      ;stdcall
          disk_del,              'DiskDel',            \
          disk_media_changed,    'DiskMediaChanged',   \      ;stdcall
\
          fs_read32_sys,         'FsRead32Sys',        \
          fs_read32_app,         'FsRead32App',        \
          fs_read64_sys,         'FsRead64Sys',        \
          fs_read64_app,         'FsRead64App',        \
\
          fs_write32_sys,        'FsWrite32Sys',       \
          fs_write32_app,        'FsWrite32App',       \
          fs_write64_sys,        'FsWrite64Sys',       \
          fs_write64_app,        'FsWrite64App',       \
\
          fs_add,                'FsAdd',              \
\         ;fs_del,                'FsDel',              \
\
          disk_sync,             'DiskSync',           \
\
          create_event,          'CreateEvent',        \      ; ecx, esi
          destroy_event,         'DestroyEvent',       \      ;
          raise_event,           'RaiseEvent',         \      ; eax, ebx, edx, esi
          wait_event,            'WaitEvent',          \      ; eax, ebx
          wait_event_timeout,    'WaitEventTimeout',   \      ; eax, ebx, ecx
          get_event_ex,          'GetEvent',           \      ; edi
          clear_event,           'ClearEvent',         \      ;see EVENT.inc for specification
          send_event,            'SendEvent',          \      ;see EVENT.inc for specification
\
          create_kernel_object,  'CreateObject',       \
          create_ring_buffer,    'CreateRingBuffer',   \      ; stdcall
          destroy_kernel_object, 'DestroyObject',      \
          free_kernel_space,     'FreeKernelSpace',    \      ; stdcall
          free_page,             'FreePage',           \      ; eax
          kernel_alloc,          'KernelAlloc',        \      ; stdcall
          kernel_free,           'KernelFree',         \      ; stdcall
          malloc,                'Kmalloc',            \
          free,                  'Kfree',              \
          map_io_mem,            'MapIoMem',           \      ; stdcall
          map_page,              'MapPage',            \      ; stdcall
          get_pg_addr,           'GetPgAddr',          \      ; eax
          get_phys_addr,         'GetPhysAddr',        \      ; eax
          map_space,             'MapSpace',           \
          release_pages,         'ReleasePages',       \
          alloc_dma24,           'AllocDMA24',         \      ; stdcall
\
          init_rwsem,            'InitRwsem',          \      ; gcc fastcall
          down_read,             'DownRead',           \      ; gcc fastcall
          down_write,            'DownWrite',          \      ; gcc fastcall
          up_read,               'UpRead',             \      ; gcc fastcall
          up_write,              'UpWrite',            \      ; gcc fastacll
          mutex_init,            'MutexInit',          \      ; gcc fastcall
          mutex_lock,            'MutexLock',          \      ; gcc fastcall
          mutex_unlock,          'MutexUnlock',        \      ; gcc fastcall
\
          get_display,           'GetDisplay',         \
          set_screen,            'SetScreen',          \
          set_framebuffer,       'SetFramebuffer',     \      ; gcc fastcall
          window._.get_rect,     'GetWindowRect',      \      ; gcc fastcall
          pci_api_drv,           'PciApi',             \
          pci_read8,             'PciRead8',           \      ; stdcall
          pci_read16,            'PciRead16',          \      ; stdcall
          pci_read32,            'PciRead32',          \      ; stdcall
          pci_write8,            'PciWrite8',          \      ; stdcall
          pci_write16,           'PciWrite16',         \      ; stdcall
          pci_write32,           'PciWrite32',         \      ; stdcall
\
          get_pid,               'GetPid',             \
          get_service,           'GetService',         \      ; stdcall
          reg_service,           'RegService',         \      ; stdcall
          attach_int_handler,    'AttachIntHandler',   \      ; stdcall
          user_alloc,            'UserAlloc',          \      ; stdcall
          user_alloc_at,         'UserAllocAt',        \      ; stdcall
          user_free,             'UserFree',           \      ; stdcall
          unmap_pages,           'UnmapPages',         \      ; eax, ecx
          sys_msg_board_str,     'SysMsgBoardStr',     \
          sys_msg_board,         'SysMsgBoard',        \
          get_clock_ns,          'GetClockNs',         \      ;retval edx:eax 64-bit value
          get_timer_ticks,       'GetTimerTicks',      \
          get_stack_base,        'GetStackBase',       \
          delay_hs,              'Delay',              \      ; ebx
          set_mouse_data,        'SetMouseData',       \      ;
          set_keyboard_data,     'SetKeyboardData',    \      ; gcc fastcall
          register_keyboard,     'RegKeyboard',        \
          delete_keyboard,       'DelKeyboard',        \
          get_cpu_freq,          'GetCpuFreq',         \
\
          new_sys_threads,       'CreateThread',       \      ; ebx, ecx, edx
\
          srv_handler,           'ServiceHandler',     \
          fpu_save,              'FpuSave',            \
          fpu_restore,           'FpuRestore',         \
          avx_save_size,         'AvxSaveSize',        \
          avx_save,              'AvxSave',            \
          avx_restore,           'AvxRestore',         \
          r_f_port_area,         'ReservePortArea',    \
          boot_log,              'Boot_Log',           \
\
          load_cursor,           'LoadCursor',         \      ;stdcall
\
          get_curr_slot,         'GetCurrSlot',        \
          change_task,           'ChangeTask',         \
          load_file,             'LoadFile',           \      ;retval eax, ebx
          delay_ms,              'Sleep',              \
\
          strncat,               'strncat',            \
          strncpy,               'strncpy',            \
          strncmp,               'strncmp',            \
          strnlen,               'strnlen',            \
          strchr,                'strchr',             \
          strrchr,               'strrchr',            \
\
          timer_hs,              'TimerHS',            \
          timer_hs,              'TimerHs',            \      ; shit happens
          cancel_timer_hs,       'CancelTimerHS',      \
\
          reg_usb_driver,        'RegUSBDriver',       \
          usb_open_pipe,         'USBOpenPipe',        \
          usb_close_pipe,        'USBClosePipe',       \
          usb_normal_transfer_async, 'USBNormalTransferAsync', \
          usb_control_async,     'USBControlTransferAsync', \
          usb_get_param,         'USBGetParam',        \
          usb_hc_func,           'USBHCFunc',          \
\
\         ; The intrakernel function of working with the file system.
\         ; Duplicates system call 70.
          file_system_lfn_protected_registers, 'FS_Service', \
\
          net_add_device,        'NetRegDev',          \
          net_remove_device,     'NetUnRegDev',        \
          net_ptr_to_num,        'NetPtrToNum',        \
          net_link_changed,      'NetLinkChanged',     \
          eth_input,             'EthInput',           \
          net_buff_alloc,        'NetAlloc',           \
          net_buff_free,         'NetFree',            \
\
          get_pcidev_list,       'GetPCIList',         \
\
          acpi_get_root_ptr,     'AcpiGetRootPtr',     \
\
          0,                     'LFBAddress' ; must be the last one

load kernel_exports_count dword from __exports + 24
load kernel_exports_addresses dword from __exports + 28
exp_lfb = OS_BASE + kernel_exports_addresses + (kernel_exports_count - 1) * 4 - 4
