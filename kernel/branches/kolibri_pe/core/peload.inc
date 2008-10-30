;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

$Revision$

include 'export.inc'

align 4


align 16
_kernel_exports:
          export 'KERNEL',   \
          commit_pages,          'CommitPages',        \      ; eax, ebx, ecx
          create_kernel_object,  'CreateObject',       \
          create_ring_buffer,    'CreateRingBuffer',   \      ; stdcall
          destroy_kernel_object, 'DestroyObject',      \
          free_kernel_space,     'FreeKernelSpace',    \      ; stdcall
          malloc,                'Kmalloc',            \
          free,                  'Kfree',              \
          map_io_mem,            'MapIoMem',           \      ; stdcall
          get_pg_addr,           'GetPgAddr',          \      ; eax
          @mem_alloc@8,          'mem_alloc',          \      ; fastcall
          @mem_free@4,           'mem_free',           \      ; fastcall
\
          select_hw_cursor,      'SelectHwCursor',     \      ; stdcall
          set_hw_cursor,         'SetHwCursor',        \      ; stdcall
          hw_restore,            'HwCursorRestore',    \      ;
          create_cursor,         'HwCursorCreate',     \      ;
\
          set_screen,            'SetScreen',          \
          pci_api,               'PciApi',             \
          pci_read8,             'PciRead8',           \      ; stdcall
          pci_read16,            'PciRead16',          \      ; stdcall
          pci_read32,            'PciRead32',          \      ; stdcall
          pci_write8,            'PciWrite8',          \      ; stdcall
          pci_write16,           'PciWrite16',         \      ; stdcall
          pci_write32,           'PciWrite32',         \      ; stdcall
\
          reg_service,           'RegService',         \      ; stdcall
          user_alloc,            'UserAlloc',          \      ; stdcall
          user_free,             'UserFree',           \      ; stdcall
          unmap_pages,           'UnmapPages',         \      ; eax, ecx
          sys_msg_board_str,     'SysMsgBoardStr'



