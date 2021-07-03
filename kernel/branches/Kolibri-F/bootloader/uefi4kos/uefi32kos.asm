;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2020. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;; Version 2, or (at your option) any later version.            ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format pe efi
entry main

section '.text' code executable readable

include '../../struct.inc'
include '../../macros.inc'
include '../../proc32.inc'
include '../../const.inc'
include 'uefi32.inc'

MEMORY_MAP_SIZE = 0x4000
GOP_BUFFER_SIZE = 0x100
LIP_BUFFER_SIZE = 0x100
FILE_BUFFER_SIZE = 0x1000

KERNEL_BASE  =  0x10000
RAMDISK_BASE = 0x100000
MAX_FILE_SIZE = 0x10000000

CODE_32_SELECTOR = 8
DATA_32_SELECTOR = 16

; linux/arch/x86/include/uapi/asm/e820.h
E820_RAM       = 1
E820_RESERVED  = 2
E820_ACPI      = 3
E820_NVS       = 4
E820_UNUSABLE  = 5
E820_PMEM      = 7

proc load_file stdcall uses ebx esi edi, _root, _name, _buffer, _size, _fatal
        mov     eax, [_root]
        ccall   [eax+EFI_FILE_PROTOCOL.Open], eax, file_handle, [_name], \
                EFI_FILE_MODE_READ, 0
        test    eax, eax
        jz      @f
        xor     eax, eax
        cmp     [_fatal], 1
        jnz     .done
        mov     ebx, [efi_table]
        mov     ebx, [ebx+EFI_SYSTEM_TABLE.ConOut]
        ccall   [ebx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], ebx, \
                msg_error_open_file
        ccall   [ebx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], ebx, \
                [_name]
        jmp     $
@@:
        mov     eax, [file_handle]
        lea     ecx, [_size]
        ccall   [eax+EFI_FILE_PROTOCOL.Read], eax, ecx, [_buffer]
        mov     eax, [file_handle]
        ccall   [eax+EFI_FILE_PROTOCOL.Close], eax
        mov     eax, [_size]
.done:
        ret
endp

proc skip_whitespace
.next_char:
        cmp     byte[esi], 0
        jz      .done
        cmp     byte[esi], 0x20 ; ' '
        jz      .whitespace
        cmp     byte[esi], 9    ; '\t'
        jz      .whitespace
        jmp     .done
.whitespace:
        inc     esi
        jmp     .next_char
.done:
        ret
endp

proc skip_until_newline
.next_char:
        cmp     byte[esi], 0
        jz      .done
        cmp     byte[esi], 0xd  ; '\r'
        jz      .done
        cmp     byte[esi], 0xa  ; '\n'
        jz      .done
        inc     esi
        jmp     .next_char
.done:
        ret
endp

proc skip_newline
.next_char:
        cmp     byte[esi], 0xd  ; '\r'
        jz      .newline
        cmp     byte[esi], 0xa  ; '\n'
        jz      .newline
        jmp     .done
.newline:
        inc     esi
        jmp     .next_char
.done:
        ret
endp

proc skip_line
        call    skip_until_newline
        call    skip_newline
        ret
endp

proc dec2bin
        mov     edx, 0
.next_char:
        movzx   eax, byte[esi]
        test    eax, eax
        jz      .done
        sub     eax, '0'
        jb      .done
        cmp     eax, 9
        ja      .done
        inc     esi
        imul    edx, 10
        add     edx, eax
        jmp     .next_char
.done:
        mov     eax, edx
        ret
endp

proc parse_option
        mov     ebx, config_options-3*4
.try_next_option:
        add     ebx, 3*4
        mov     edi, esi
        mov     edx, [ebx]      ; option name
        test    edx, edx
        jz      .done
.next_char:
        cmp     byte[edx], 0
        jnz     @f
        cmp     byte[edi], '='
        jz      .opt_name_ok
@@:
        cmp     byte[edi], 0
        jz      .done
        movzx   eax, byte[edi]
        cmp     [edx], al
        jnz     .try_next_option
        inc     edi
        inc     edx
        jmp     .next_char
.opt_name_ok:
        inc     edi
        mov     esi, edi
        call    dword[ebx+4]
.done:
        ret
endp

proc parse_line
.next_line:
        cmp     byte[esi], 0
        jz      .done
        cmp     byte[esi], 0xd  ; '\r'
        jz      .skip
        cmp     byte[esi], 0xa  ; '\n'
        jz      .skip
        cmp     byte[esi], '#'
        jz      .skip
        call    parse_option
        call    skip_line
        jmp     .next_line
.skip:
        call    skip_line
        jmp     .next_line
.done:
        ret
endp

proc cfg_opt_func_resolution
        call    dec2bin
        xor     edx, edx
        mov     [edx+BOOT_LO.x_res], ax
        cmp     byte[esi], 'x'
        jz      @f
        cmp     byte[esi], '*'
        jz      @f
        jmp     .done
@@:
        inc     esi
        call    dec2bin
        xor     edx, edx
        mov     [edx+BOOT_LO.y_res], ax
        mov     [cfg_opt_used_resolution], 1
.done:
        ret
endp

proc cfg_opt_func_acpi
        call    dec2bin
        mov     [cfg_opt_used_acpi], 1
        mov     [cfg_opt_value_acpi], al
        ret
endp

proc cfg_opt_func_debug_print
        call    dec2bin
        mov     [cfg_opt_used_debug_print], 1
        mov     [cfg_opt_value_debug_print], al
        ret
endp

proc cfg_opt_func_launcher_start
        call    dec2bin
        mov     [cfg_opt_used_launcher_start], 1
        mov     [cfg_opt_value_launcher_start], al
        ret
endp

proc cfg_opt_func_mtrr
        call    dec2bin
        mov     [cfg_opt_used_mtrr], 1
        mov     [cfg_opt_value_mtrr], al
        ret
endp

proc cfg_opt_func_ask_params
        call    dec2bin
        mov     [cfg_opt_used_ask_params], 1
        mov     [cfg_opt_value_ask_params], al
        ret
endp

proc cfg_opt_func_imgfrom
        call    dec2bin
        mov     [cfg_opt_used_imgfrom], 1
        mov     [cfg_opt_value_imgfrom], al
        ret
endp

proc cfg_opt_func_syspath
        mov     edi, cfg_opt_value_syspath
.next_char:
        movzx   eax, byte[esi]
        cmp     al, 0xd ; \r
        jz      .done
        cmp     al, 0xa ; \n
        jz      .done
        inc     esi
        stosb
        jmp     .next_char
.done:
        mov     byte[edi], 0
        ret
endp

proc parse_config stdcall uses ebx esi edi, _buffer
;        mov     esi, [_buffer]
        mov     esi, KERNEL_BASE
.next_line:
        call    parse_line
        cmp     byte[esi], 0
        jnz     .next_line
        ret
endp

proc read_options_from_config stdcall uses ebx esi edi
        mov     ebx, [efi_table]
        mov     ebx, [ebx+EFI_SYSTEM_TABLE.BootServices]
        ccall   [ebx+EFI_BOOT_SERVICES.HandleProtocol], [efi_handle], \
                lipuuid, lip_interface
        test    eax, eax
        jnz     .error
        mov     eax, [lip_interface]

        mov     ebx, [efi_table]
        mov     ebx, [ebx+EFI_SYSTEM_TABLE.BootServices]
        ccall   [ebx+EFI_BOOT_SERVICES.HandleProtocol], \
                [eax+EFI_LOADED_IMAGE_PROTOCOL.DeviceHandle], sfspguid, \
                sfsp_interface
        test    eax, eax
        jnz     .error

        mov     eax, [sfsp_interface]
        ccall   [eax+EFI_SIMPLE_FILE_SYSTEM_PROTOCOL.OpenVolume], eax, esp_root
        test    eax, eax
        jnz     .error

        stdcall load_file, [esp_root], file_name, KERNEL_BASE, \
                FILE_BUFFER_SIZE, 0
        test    eax, eax
        jz      @f
        stdcall parse_config, KERNEL_BASE
@@:
.error:
        ret
endp

proc print_vmode uses eax ebx ecx esi edi, _gop_if
        mov     ebx, [_gop_if]
        call    clearbuf
        mov     eax, [ebx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        mov     edi, msg
        call    num2dec
        mov     eax, [ebx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        mov     edi, msg+8*2
        call    num2dec

        mov     eax, [ebx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat]
        mov     edi, msg+16*2
        call    num2dec

        mov     eax, [ebx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
        mov     edi, msg+24*2
        call    num2dec
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg
        ret
endp

proc find_vmode_index_by_resolution uses ebx esi edi
        mov     [cfg_opt_value_vmode], 0
.next_mode:
;        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
;        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
;                msg_query_vmode

        movzx   ecx, [cfg_opt_value_vmode]
        mov     eax, [gop_interface]
        ccall   [eax+EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode], eax, ecx, \
                gop_info_size, gop_info
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     edi, msg
        call    num2hex
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg
        jmp     .skip_mode
@@:
        mov     ecx, [gop_info]
        stdcall print_vmode, ecx
        ; PixelBlueGreenRedReserved8BitPerColor
        cmp     [ecx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat], 1
        jnz     .skip_mode
        xor     edx, edx
        mov     eax, [ecx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        cmp     ax, [edx+BOOT_LO.x_res]
        jnz     .skip_mode
        mov     eax, [ecx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        cmp     ax, [edx+BOOT_LO.y_res]
        jnz     .skip_mode
        jmp     .done
.skip_mode:
        inc     [cfg_opt_value_vmode]
        movzx   eax, [cfg_opt_value_vmode]
        mov     ecx, [gop_interface]
        mov     edx, [ecx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        cmp     eax, [edx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.MaxMode]
        jnz     .next_mode
        mov     [cfg_opt_used_resolution], 0
        mov     [cfg_opt_value_ask_params], 1

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_error_no_such_vmode
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg_error
        jmp     $
.error:
.done:
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_vmode_found
        ret
endp

proc ask_for_params
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_ask_for_params
        jmp     $
.error:
.done:
        ret
endp

main:
        mov     esi, [esp+4]
        mov     [efi_handle], esi
        mov     esi, [esp+8]
        mov     [efi_table], esi

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.Reset], eax, 1
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_u4k_loaded

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_read_options
        call    read_options_from_config

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_load_kernel
        stdcall load_file, [esp_root], kernel_name, KERNEL_BASE, MAX_FILE_SIZE, 1

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_load_ramdisk
        stdcall load_file, [esp_root], ramdisk_name, RAMDISK_BASE, MAX_FILE_SIZE, 1

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_alloc_devicesdat

        mov     eax, [esi+EFI_SYSTEM_TABLE.BootServices]
        ccall   [eax+EFI_BOOT_SERVICES.AllocatePages], \
                EFI_ALLOCATE_MAX_ADDRESS, EFI_RESERVED_MEMORY_TYPE, 1, \
                devicesdat_data
        call    halt_on_error

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_load_devicesdat

        ccall   load_file, [esp_root], devicesdat_name, [devicesdat_data], \
                [devicesdat_size], 0
        mov     [devicesdat_size], eax

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_locate_gop_handlers

        mov     eax, [esi+EFI_SYSTEM_TABLE.BootServices]
        ccall   [eax+EFI_BOOT_SERVICES.LocateHandle], \
                EFI_LOCATE_SEARCH_TYPE.ByProtocol, gopuuid, 0, \
                gop_buffer_size, gop_buffer
        mov     [status], eax

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_gop_buffer_size
        call    clearbuf
        mov     eax, [gop_buffer_size]
        mov     edi, msg
        call    num2hex
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg

        mov     eax, [status]
        call    halt_on_error

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_look_for_gop_handler

        mov     ebx, gop_buffer
.next_gop_handle:
        mov     eax, ebx
        sub     eax, gop_buffer
        cmp     eax, [gop_buffer_size]
        jb      @f
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_error_out_of_handlers
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg_error
        jmp     $
@@:
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_query_handler

        mov     eax, [esi+EFI_SYSTEM_TABLE.BootServices]
        ccall   [eax+EFI_BOOT_SERVICES.HandleProtocol], \
                [ebx], gopuuid, gop_interface
;mov eax, 0x80000003
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     edi, msg
        call    num2hex
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg

        add     ebx, 4
        jmp     .next_gop_handle
@@:

        call    find_rsdp

        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_acpi_tables_done

        cmp     [cfg_opt_used_resolution], 0
        jz      .not_used_resolution
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_opt_resolution
        call    clearbuf
        xor     edx, edx
        movzx   eax, [edx+BOOT_LO.x_res]
        mov     edi, msg
        call    num2dec
        xor     edx, edx
        movzx   eax, [edx+BOOT_LO.y_res]
        mov     edi, msg+8*2
        call    num2dec
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg

        call    find_vmode_index_by_resolution
.not_used_resolution:
        cmp     [cfg_opt_used_debug_print], 0
        jz      .not_used_debug_print
        movzx   eax, [cfg_opt_value_debug_print]
        xor     edx, edx
        mov     [edx+BOOT_LO.debug_print], al
.not_used_debug_print:

        cmp     [cfg_opt_value_ask_params], 0
        jz      @f
        call    ask_for_params
@@:

        movzx   ecx, [cfg_opt_value_vmode]
        mov     eax, [gop_interface]
        ccall   [eax+EFI_GRAPHICS_OUTPUT_PROTOCOL.SetMode], eax, ecx
        call    halt_on_error

        mov     ecx, [gop_interface]
        mov     edx, [ecx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        mov     edi, [edx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase.lo]
        mov     [fb_base], edi


        mov     ebx, [edx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.Mode]
        mov     eax, [gop_interface]
        ccall   [eax+EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode], eax, ebx, \
                gop_info_size, gop_info
        test    eax, eax
        jnz     .error
        mov     ecx, [gop_info]
        mov     eax, [ecx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        xor     edx, edx
        mov     [edx+BOOT_LO.x_res], ax
        mov     eax, [ecx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        mov     [edx+BOOT_LO.y_res], ax
        mov     eax, [ecx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
        shl     eax, 2
        mov     [edx+BOOT_LO.pitch], ax

        mov     byte[edx+BOOT_LO.pci_data+0], 1    ; PCI access mechanism
        mov     byte[edx+BOOT_LO.pci_data+1], 8    ; last bus, don't know how to count them
        mov     byte[edx+BOOT_LO.pci_data+2], 0x10 ; PCI version
        mov     byte[edx+BOOT_LO.pci_data+3], 0x02
        mov     dword[edx+BOOT_LO.pci_data+4], 0xe3

        ; kernel
;        eficall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                450000/0x1000, EFI_ALLOCATE_ADDRESS

        ; ramdisk
;        eficall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                2880*512/0x1000, EFI_ALLOCATE_ADDRESS

        call    calc_memmap
;        call    dump_memmap

        mov     eax, [efi_table]
        mov     eax, [eax+EFI_SYSTEM_TABLE.BootServices]
        ccall   [eax+EFI_BOOT_SERVICES.ExitBootServices], [efi_handle], \
                [memory_map_key]
        call    halt_on_error

        cli

        xor     edx, edx
        xor     esi, esi
        mov     [esi+BOOT_LO.bpp], 32
        mov     [esi+BOOT_LO.vesa_mode], dx
        mov     [esi+BOOT_LO.bank_switch], edx
        mov     edi, [fb_base]
        mov     [esi+BOOT_LO.lfb], edi

        movzx   eax, [cfg_opt_value_mtrr]
        mov     [esi+BOOT_LO.mtrr], al

        movzx   eax, [cfg_opt_value_launcher_start]
        mov     [esi+BOOT_LO.launcher_start], al

        movzx   eax, [cfg_opt_value_debug_print]
        mov     [esi+BOOT_LO.debug_print], al

        mov     [esi+BOOT_LO.dma], dl
;        mov     qword[esi+BOOT_LO.pci_data], 0
        mov     [esi+BOOT_LO.apm_entry], edx
        mov     [esi+BOOT_LO.apm_version], dx
        mov     [esi+BOOT_LO.apm_flags], dx
        mov     [esi+BOOT_LO.apm_code_32], dx
        mov     [esi+BOOT_LO.apm_code_16], dx
        mov     [esi+BOOT_LO.apm_data_16], dx
        mov     [esi+BOOT_LO.bios_hd_cnt], dl

        movzx   eax, [cfg_opt_value_imgfrom]
        mov     [esi+BOOT_LO.rd_load_from], al

        mov     eax, dword[devicesdat_size]
        mov     [edx+BOOT_LO.devicesdat_size], eax
        mov     eax, dword[devicesdat_data]
        mov     [edx+BOOT_LO.devicesdat_data], eax

        mov     esi, cfg_opt_value_syspath
        mov     edi, BOOT_LO.syspath
        mov     ecx, 0x17
        rep movsb

        lgdt    [cs:GDTR]

        mov     ax, DATA_32_SELECTOR
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax
        mov     ss, ax

        push    CODE_32_SELECTOR
        lea     eax, [.next]
        push    eax
        retf

.next:
        mov     eax, cr0
        and     eax, not CR0_PG
        mov     cr0, eax

        mov     eax, cr4
        and     eax, not CR4_PAE
        mov     cr4, eax

        push    KERNEL_BASE
        retn

.error:
        mov     esi, [efi_table]
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_error
        jmp     $

halt_on_error:
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     edi, msg
        call    num2hex
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_error
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, msg
        jmp     $
@@:
        ret

proc find_rsdp
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_look_for_rsdp

        mov     edi, [esi+EFI_SYSTEM_TABLE.ConfigurationTable]
        mov     ecx, [esi+EFI_SYSTEM_TABLE.NumberOfTableEntries]
.next_table:
        dec     ecx
        js      .all_tables_done
        ; EFI_ACPI_TABLE_GUID
        cmp     dword[edi+EFI_CONFIGURATION_TABLE.VendorGUID+0x0], 0x8868e871
        jnz     .not_acpi20
        cmp     dword[edi+EFI_CONFIGURATION_TABLE.VendorGUID+0x4], 0x11d3e4f1
        jnz     .not_acpi20
        cmp     dword[edi+EFI_CONFIGURATION_TABLE.VendorGUID+0x8], 0x800022bc
        jnz     .not_acpi20
        cmp     dword[edi+EFI_CONFIGURATION_TABLE.VendorGUID+0xc], 0x81883cc7
        jnz     .not_acpi20
        mov     eax, [edi+EFI_CONFIGURATION_TABLE.VendorTable]
        mov     edx, BOOT_LO.acpi_rsdp
        mov     [edx], eax
        mov     eax, [esi+EFI_SYSTEM_TABLE.ConOut]
        ccall   [eax+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], eax, \
                msg_rsdp_found
        jmp     .all_tables_done
.not_acpi20:
        add     edi, sizeof.EFI_CONFIGURATION_TABLE
        jmp     .next_table
.all_tables_done:
        ret
endp

proc calc_memmap
        mov     eax, [esi+EFI_SYSTEM_TABLE.BootServices]
        ccall   [eax+EFI_BOOT_SERVICES.AllocatePages], EFI_ALLOCATE_ANY_PAGES, \
                EFI_RESERVED_MEMORY_TYPE, MEMORY_MAP_SIZE/0x1000, memory_map
        call    halt_on_error

        mov     eax, [esi+EFI_SYSTEM_TABLE.BootServices]
        ccall   [eax+EFI_BOOT_SERVICES.GetMemoryMap], memory_map_size, \
                [memory_map], memory_map_key, descriptor_size, descriptor_ver
        call    halt_on_error

        push    esi
        mov     edi, BOOT_LO.memmap_blocks
        mov     dword[edi-4], 0 ; memmap_block_cnt
        mov     esi, [memory_map]
        mov     ebx, esi
        add     ebx, [memory_map_size]
.next_descr:
        call    add_uefi_memmap
        add     esi, [descriptor_size]
        cmp     esi, ebx
        jb      .next_descr
        pop     esi
        ret
endp

; linux/arch/x86/platform/efi/efi.c
; do_add_efi_memmap
proc add_uefi_memmap
        cmp     [BOOT_LO.memmap_block_cnt], MAX_MEMMAP_BLOCKS
        jz      .done

        mov     eax, [esi+EFI_MEMORY_DESCRIPTOR.PhysicalStart.lo]
        mov     edx, [esi+EFI_MEMORY_DESCRIPTOR.PhysicalStart.hi]
        mov     [edi+e820entry.addr.lo], eax
        mov     [edi+e820entry.addr.hi], edx

        mov     eax, [esi+EFI_MEMORY_DESCRIPTOR.NumberOfPages.lo]
        mov     edx, [esi+EFI_MEMORY_DESCRIPTOR.NumberOfPages.hi]
        shld    edx, eax, 12
        shl     eax, 12
        mov     [edi+e820entry.size.lo], eax
        mov     [edi+e820entry.size.hi], edx

        mov     ecx, [esi+EFI_MEMORY_DESCRIPTOR.Type]
        cmp     ecx, EFI_LOADER_CODE
        jz      .mem_ram_if_wb
        cmp     ecx, EFI_LOADER_DATA
        jz      .mem_ram_if_wb
        cmp     ecx, EFI_BOOT_SERVICES_CODE
        jz      .mem_ram_if_wb
        cmp     ecx, EFI_BOOT_SERVICES_DATA
        jz      .mem_ram_if_wb
        cmp     ecx, EFI_CONVENTIONAL_MEMORY
        jz      .mem_ram_if_wb
        cmp     ecx, EFI_ACPI_RECLAIM_MEMORY
        mov     eax, E820_ACPI
        jz      .type_done
        cmp     ecx, EFI_ACPI_MEMORY_NVS
        mov     eax, E820_NVS
        jz      .type_done
        cmp     ecx, EFI_UNUSABLE_MEMORY
        mov     eax, E820_UNUSABLE
        jz      .type_done
        cmp     ecx, EFI_PERSISTENT_MEMORY
        mov     eax, E820_PMEM
        jz      .type_done
        jmp     .reserved
.mem_ram_if_wb:
        test    [esi+EFI_MEMORY_DESCRIPTOR.Attribute.lo], EFI_MEMORY_WB
        mov     eax, E820_RAM
        jnz     .type_done
.reserved:
        mov     eax, E820_RESERVED
.type_done:
        mov     [edi+e820entry.type], eax
        cmp     eax, E820_RAM
        jnz     @f
        inc     [BOOT_LO.memmap_block_cnt]
        add     edi, sizeof.e820entry
@@:
.done:
        ret
endp


proc num2dec
        pushad

        xor     ecx, ecx
        mov     ebx, 10
.next_digit:
        xor     edx, edx
        div     ebx
        push    edx
        inc     ecx
        test    eax, eax
        jnz     .next_digit

.next_char:
        pop     eax
        add     eax, '0'
        stosw
        loop    .next_char

        popad
        ret
endp


proc num2hex
        pushad

        xchg    edx, eax
        mov     ecx, 8
.next_tetra:
        rol     edx, 4
        movzx   eax, dl
        and     eax, 0x0f
        movzx   eax, byte[hex+eax]
        stosw
        loop    .next_tetra

        popad
        ret
endp


hex db '0123456789ABCDEF'

proc clearbuf
        pushad
        mov     eax, 0x0020
        mov     ecx, 79
        mov     edi, msg
        rep stosw
        popad
        ret
endp

section '.rodata' data readable
align 16
GDTR:
        dw 3*8-1
        dq GDT
align 16
GDT:
        dw 0, 0, 0, 0
        dw 0FFFFh,0,9A00h,0CFh          ; 32-bit code
        dw 0FFFFh,0,9200h,0CFh          ; flat data

gopuuid         db EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
lipuuid         db EFI_LOADED_IMAGE_PROTOCOL_GUID
sfspguid        db EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID

file_name       du "\EFI\KOLIBRIOS\KOLIBRI.INI",0
kernel_name     du "\EFI\KOLIBRIOS\KOLIBRI.KRN",0
ramdisk_name    du "\EFI\KOLIBRIOS\KOLIBRI.IMG",0
devicesdat_name du "\EFI\KOLIBRIOS\DEVICES.DAT",0

config_options  dd cfg_opt_name_resolution, cfg_opt_func_resolution, \
                   cfg_opt_cmnt_resolution, \
                   cfg_opt_name_acpi,  cfg_opt_func_acpi, cfg_opt_cmnt_acpi, \
                   cfg_opt_name_debug_print, cfg_opt_func_debug_print, \
                   cfg_opt_cmnt_debug_print, \
                   cfg_opt_name_launcher_start, cfg_opt_func_launcher_start, \
                   cfg_opt_cmnt_launcher_start, \
                   cfg_opt_name_mtrr,  cfg_opt_func_mtrr, cfg_opt_cmnt_mtrr, \
                   cfg_opt_name_ask_params,  cfg_opt_func_ask_params, \
                   cfg_opt_cmnt_ask_params, \
                   cfg_opt_name_imgfrom, cfg_opt_func_imgfrom, \
                   cfg_opt_cmnt_imgfrom, \
                   cfg_opt_name_syspath, cfg_opt_func_syspath, \
                   cfg_opt_cmnt_syspath, \
                   0

cfg_opt_name_resolution     db "resolution",0
cfg_opt_name_acpi           db "acpi",0
cfg_opt_name_debug_print    db "debug_print",0
cfg_opt_name_launcher_start db "launcher_start",0
cfg_opt_name_mtrr           db "mtrr",0
cfg_opt_name_ask_params     db "ask_params",0
cfg_opt_name_imgfrom        db "imgfrom",0
cfg_opt_name_syspath        db "syspath",0

cfg_opt_cmnt_resolution     db "# Graphic mode",0
cfg_opt_cmnt_acpi           db "# ACPI settings",0xa, \
                               "#   0: don't use",0xa, \
                               "#   1: parse ACPI tables",0xa, \
                               "#   2: + call _PIC method",0xa, \
                               "#   3: + get APIC interrupts",0xa,0
cfg_opt_cmnt_debug_print    db "# Duplicate debug output to the screen",0
cfg_opt_cmnt_launcher_start db "# Start LAUNCHER app after kernel is loaded",0
cfg_opt_cmnt_mtrr           db "# Configure MTRR's",0
cfg_opt_cmnt_ask_params     db "# Interrupt booting to ask the user for boot", \
                               " params",0
cfg_opt_cmnt_imgfrom        db "# Where to load ramdisk image from",0
cfg_opt_cmnt_syspath        db "# Path to /sys directory",0

msg_u4k_loaded            du "uefi32kos loaded",13,10,0
msg_read_options          du "Read options from config file",13,10,0
msg_load_kernel           du "Load kernel",13,10,0
msg_load_ramdisk          du "Load ramdisk",13,10,0
msg_load_devicesdat       du "Load DEVICES.DAT",13,10,0
msg_alloc_devicesdat      du "Allocate memory for DEVICES.DAT",13,10,0
msg_locate_gop_handlers   du "Locate GOP handlers",13,10,0
msg_look_for_gop_handler  du "Look for GOP handler",13,10,0
msg_query_handler         du "Query handler",13,10,0
msg_query_vmode           du "Query vmode",13,10,0
msg_vmode_found           du "Video mode found",13,10,0
msg_look_for_rsdp         du "Look for RSDP",13,10,0
msg_rsdp_found            du "RSDP found",13,10,0
msg_acpi_tables_done      du "ACPI tables done",13,10,0
msg_ask_for_params        du "Ask for params",13,10,0
msg_set_graphic_mode      du "Set graphic mode",13,10,0
msg_success               du "Success!",13,10,0
msg_gop_buffer_size       du "GOP buffer size",13,10,0
msg_opt_resolution        du "option resolution: ",0
msg_error                 du "Error!",13,10,0
msg_error_no_such_vmode   du "No such vmode",13,10,0
msg_error_out_of_handlers du "Out of handlers",13,10,0
msg_error_open_file       du "Error: can't open file ",0
msg                       du 79 dup " ",13,10,0


section '.data' data readable writeable
efi_handle dd 0
efi_table  dd 0

fb_base         dd 0

gop_buffer_size dd GOP_BUFFER_SIZE
gop_handle      dd 0
gop_interface   dd 0
gop_info_size   dd 0
gop_info        dd 0

lip_buffer_size dd LIP_BUFFER_SIZE
lip_handle      dd 0
lip_interface   dd 0

sfsp_interface  dd 0

esp_root        dd ?
file_handle     dd ?
file_buffer_size dd FILE_BUFFER_SIZE-1  ; leave the last byte for \0

cfg_opt_used_resolution     db 0
cfg_opt_used_acpi           db 0
cfg_opt_used_debug_print    db 0
cfg_opt_used_launcher_start db 0
cfg_opt_used_mtrr           db 0
cfg_opt_used_ask_params     db 0
cfg_opt_used_imgfrom        db 0
cfg_opt_used_syspath        db 0

cfg_opt_value_vmode          db 0
cfg_opt_value_acpi           db 0
cfg_opt_value_debug_print    db 0
cfg_opt_value_launcher_start db 1
cfg_opt_value_mtrr           db 0
cfg_opt_value_ask_params     db 0
cfg_opt_value_imgfrom        db RD_LOAD_FROM_MEMORY
cfg_opt_value_syspath        db "/RD/1",0
                             rb 20

memory_map_key  dd 0
descriptor_size dd 0
descriptor_ver  dd 0
memory_map_size dd MEMORY_MAP_SIZE

efi_fs_info_id db EFI_FILE_SYSTEM_INFO_ID
efi_fs_info_size dq sizeof.EFI_FILE_SYSTEM_INFO
efi_fs_info EFI_FILE_SYSTEM_INFO

memory_map      dd ?
gop_buffer      rd GOP_BUFFER_SIZE/4
devicesdat_data dd 0xffffffff
devicesdat_size dd 0x1000
status dd ?

section '.reloc' fixups data discardable
