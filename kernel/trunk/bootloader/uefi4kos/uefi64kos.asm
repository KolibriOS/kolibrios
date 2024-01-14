;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2020-2024. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;; Version 2, or (at your option) any later version.            ;;
;;                                                              ;;
;; Written by Ivan Baravy                                       ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format pe64 efi
entry main

section '.text' code executable readable

include '../../struct.inc'
include '../../macros.inc'
include '../../kglobals.inc'
fastcall fix fstcall
include 'proc64.inc'
include '../../const.inc'

purge DQ
include 'uefi64.inc'

MEMORY_MAP_SIZE = 0x10000
PROTOCOL_HANDLERS_BUFFER_SIZE = 0x100
FILE_BUFFER_SIZE = 0x1000

KERNEL_TRAMPOLINE = 0x8f80      ; just before BOOT_LO
MAX_FILE_SIZE = 0x10000000

CODE_32_SELECTOR = 8
DATA_32_SELECTOR = 16
CODE_64_SELECTOR = 24

; linux/arch/x86/include/uapi/asm/e820.h
E820_RAM       = 1
E820_RESERVED  = 2
E820_ACPI      = 3
E820_NVS       = 4
E820_UNUSABLE  = 5
E820_PMEM      = 7

proc load_file _root, _name, _buffer, _size, _fatal
        mov     [_root], rcx
        mov     [_name], rdx
        mov     [_buffer], r8
        mov     [_size], r9
        mov     r10, [_root]
        mov     r11, [_name]
        fstcall [r10+EFI_FILE_PROTOCOL.Open], r10, file_handle, \
                r11, EFI_FILE_MODE_READ, 0
        test    eax, eax
        jz      @f
        mov     [_size], 0
        cmp     [_fatal], 1
        jnz     .done
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error_open_file
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        mov     rdx, [_name]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, [_name]
        jmp     $
@@:
        lea     rdx, [_size]
        mov     r8, [_buffer]
        mov     r10, [file_handle]
        fstcall [r10+EFI_FILE_PROTOCOL.Read], [file_handle], rdx, r8
        mov     r10, [file_handle]
        fstcall [r10+EFI_FILE_PROTOCOL.Close], [file_handle]
.done:
        call    clearbuf
        mov     rax, [_size]
        mov     rdi, msg
        call    num2dec
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_file_size
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        mov     rax, [_size]
        ret
endp

skip_whitespace:
.next_char:
        cmp     byte[rsi], 0
        jz      .done
        cmp     byte[rsi], 0x20 ; ' '
        jz      .whitespace
        cmp     byte[rsi], 9    ; '\t'
        jz      .whitespace
        jmp     .done
.whitespace:
        inc     rsi
        jmp     .next_char
.done:
        ret

skip_until_newline:
.next_char:
        cmp     byte[rsi], 0
        jz      .done
        cmp     byte[rsi], 0xd  ; '\r'
        jz      .done
        cmp     byte[rsi], 0xa  ; '\n'
        jz      .done
        inc     rsi
        jmp     .next_char
.done:
        ret

skip_newline:
.next_char:
        cmp     byte[rsi], 0xd  ; '\r'
        jz      .newline
        cmp     byte[rsi], 0xa  ; '\n'
        jz      .newline
        jmp     .done
.newline:
        inc     rsi
        jmp     .next_char
.done:
        ret

skip_line:
        call    skip_until_newline
        call    skip_newline
        ret

dec2bin:
        mov     edx, 0
.next_char:
        movzx   eax, byte[rsi]
        test    eax, eax
        jz      .done
        sub     eax, '0'
        jb      .done
        cmp     eax, 9
        ja      .done
        inc     rsi
        imul    edx, 10
        add     edx, eax
        jmp     .next_char
.done:
        mov     eax, edx
        ret

parse_option:
        mov     rbx, config_options-3*8
.try_next_option:
        add     rbx, 3*8
        mov     rdi, rsi
        mov     rdx, [rbx]      ; option name
        test    rdx, rdx
        jz      .done
.next_char:
        cmp     byte[rdx], 0
        jnz     @f
        cmp     byte[rdi], '='
        jz      .opt_name_ok
@@:
        cmp     byte[rdi], 0
        jz      .done
        movzx   eax, byte[rdi]
        cmp     [rdx], al
        jnz     .try_next_option
        inc     rdi
        inc     rdx
        jmp     .next_char
.opt_name_ok:
        inc     rdi
        mov     rsi, rdi
        call    qword[rbx+8]
.done:
        ret

parse_line:
.next_line:
        cmp     byte[rsi], 0
        jz      .done
        cmp     byte[rsi], 0xd  ; '\r'
        jz      .skip
        cmp     byte[rsi], 0xa  ; '\n'
        jz      .skip
        cmp     byte[rsi], '#'
        jz      .skip
        call    parse_option
        call    skip_line
        jmp     .next_line
.skip:
        call    skip_line
        jmp     .next_line
.done:
        ret

cfg_opt_func_resolution:
        call    dec2bin
        xor     edx, edx
        mov     [rdx+BOOT_LO.x_res], ax
        cmp     byte[rsi], 'x'
        jz      @f
        cmp     byte[rsi], '*'
        jz      @f
        jmp     .done
@@:
        inc     rsi
        call    dec2bin
        xor     edx, edx
        mov     [rdx+BOOT_LO.y_res], ax
        mov     [cfg_opt_used_resolution], 1
.done:
        ret

cfg_opt_func_acpi:
        call    dec2bin
        mov     [cfg_opt_used_acpi], 1
        mov     [cfg_opt_value_acpi], al
        ret

cfg_opt_func_debug_print:
        call    dec2bin
        mov     [cfg_opt_used_debug_print], 1
        mov     [cfg_opt_value_debug_print], al
        ret

cfg_opt_func_launcher_start:
        call    dec2bin
        mov     [cfg_opt_used_launcher_start], 1
        mov     [cfg_opt_value_launcher_start], al
        ret

cfg_opt_func_mtrr:
        call    dec2bin
        mov     [cfg_opt_used_mtrr], 1
        mov     [cfg_opt_value_mtrr], al
        ret

cfg_opt_func_ask_params:
        call    dec2bin
        mov     [cfg_opt_used_ask_params], 1
        mov     [cfg_opt_value_ask_params], al
        ret

cfg_opt_func_imgfrom:
        call    dec2bin
        mov     [cfg_opt_used_imgfrom], 1
        mov     [cfg_opt_value_imgfrom], al
        ret

cfg_opt_func_syspath:
        mov     rdi, cfg_opt_value_syspath
.next_char:
        movzx   eax, byte[rsi]
        cmp     al, 0xd ; \r
        jz      .done
        cmp     al, 0xa ; \n
        jz      .done
        inc     rsi
        stosb
        jmp     .next_char
.done:
        mov     byte[rdi], 0
        ret

proc parse_config uses rbx rsi rdi, _buffer
        mov     rsi, rcx
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_parsing_config
.next_line:
        call    parse_line
        cmp     byte[rsi], 0
        jnz     .next_line
        ret
endp

proc read_options_from_config
        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.HandleProtocol], [efi_handle], \
                lip_guid, lip_interface
        test    eax, eax
        jz      @f
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error_efi_lip_handle
        jmp     $
@@:
        mov     r11, [lip_interface]
        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.HandleProtocol], \
                [r11+EFI_LOADED_IMAGE_PROTOCOL.DeviceHandle], sfsp_guid, \
                sfsp_interface
        test    eax, eax
        jz      @f
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error_lip_dev_sfsp
        jmp     $
@@:
        mov     r10, [sfsp_interface]
        fstcall [r10+EFI_SIMPLE_FILE_SYSTEM_PROTOCOL.OpenVolume], \
                [sfsp_interface], esp_root
        test    eax, eax
        jz      @f
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error_sfsp_openvolume
        jmp     $
@@:
        fstcall load_file, [esp_root], file_name, KERNEL_BASE, \
                FILE_BUFFER_SIZE, 0     ; not fatal

        test    eax, eax
        jz      @f
        fstcall parse_config, KERNEL_BASE
@@:

.error:
        ret
endp

proc print_vmode uses rax rbx rcx rdx rsi rdi
        mov     r10, rcx
        call    clearbuf
        mov     eax, [r10+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        mov     rdi, msg
        call    num2dec
        mov     eax, [r10+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        mov     rdi, msg+8*2
        call    num2dec

        mov     eax, [r10+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat]
        mov     rdi, msg+16*2
        call    num2dec

        mov     eax, [r10+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
        mov     rdi, msg+24*2
        call    num2dec
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        ret
endp

proc find_vmode_index_by_resolution
        mov     [cfg_opt_used_resolution], 1
        mov     [cfg_opt_value_vmode], 0
.next_mode:
        movzx   edx, [cfg_opt_value_vmode]
        mov     r10, [gop_interface]
        fstcall [r10+EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode], [gop_interface], \
                rdx, gop_info_size, gop_info
        cmp     rax, EFI_SUCCESS
        jnz     .error
        mov     rcx, [gop_info]
        fstcall print_vmode
        ; PixelBlueGreenRedReserved8BitPerColor
        cmp     [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat], 1
        jnz     .skip_mode
        xor     edx, edx
        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        cmp     ax, [rdx+BOOT_LO.x_res]
        jnz     .skip_mode
        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        cmp     ax, [rdx+BOOT_LO.y_res]
        jnz     .skip_mode
        jmp     .done
.skip_mode:
        inc     [cfg_opt_value_vmode]
        movzx   eax, [cfg_opt_value_vmode]
        mov     rcx, [gop_interface]
        mov     rdx, [rcx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        cmp     eax, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.MaxMode]
        jnz     .next_mode
        mov     [cfg_opt_used_resolution], 0
        mov     [cfg_opt_value_ask_params], 1

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error_no_such_vmode
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error
        jmp     $
.error:
.done:
        ret
endp

ask_for_params:
        jmp     $

proc detect_pci_config
        fstcall get_protocol_interface, pcirbiop_guid
        mov     [pcirbiop_interface], rax
        mov     r10, rax
        fstcall [r10+EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL.Configuration], r10, \
                pcirbiop_resources
;        fstcall dump_pci_resources
        fstcall get_last_pci_bus
        call    clearbuf
        movzx   eax, [pci_last_bus]
        mov     rdi, msg
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_pci_last_bus
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        ret
endp

proc get_last_pci_bus
        mov     rsi, [pcirbiop_resources]
.next_resource:
        cmp     [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.Type], \
                EFI_RESOURCE_DESCRIPTOR_TYPE.END_TAG
        jz      .not_found
        mov     rax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.RangeMaximum]
        cmp     [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.ResourceType], \
                EFI_RESOURCE_TYPE.BUS
        jz      .found
        movzx   eax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.Length]
        lea     rsi, [rsi+rax+3]
        jmp     .next_resource
.found:
        mov     [pci_last_bus], al
.not_found:
        ret
endp

proc main _efi_handle, _efi_table
        mov     [efi_handle], rcx
        mov     [efi_table], rdx
        mov     rbx, rdx

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.Reset], rcx, 1
        test    eax, eax
        jnz     $       ; what can I do here?
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_u4k_loaded

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_detect_pci_config
        fstcall detect_pci_config

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_read_options
        fstcall read_options_from_config

        ; read kernel file
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_load_kernel
        fstcall load_file, [esp_root], kernel_name, KERNEL_BASE, \
                MAX_FILE_SIZE, 1 ; fatal

        ; read ramdisk image
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_load_ramdisk
        fstcall load_file, [esp_root], ramdisk_name, RAMDISK_BASE, \
                MAX_FILE_SIZE, 1 ; fatal

        ; alloc buffer for devices.dat
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_alloc_devicesdat
        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.AllocatePages], \
                EFI_ALLOCATE_MAX_ADDRESS, EFI_RESERVED_MEMORY_TYPE, 1, \
                devicesdat_data
        cmp     eax, EFI_SUCCESS
        jnz     .error

        ; read devices.dat
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_load_devicesdat

        fstcall load_file, [esp_root], devicesdat_name, [devicesdat_data], \
                [devicesdat_size], 0    ; not fatal
        mov     [devicesdat_size], rax

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_locate_gop_interface

        fstcall get_protocol_interface, gop_guid
        mov     [gop_interface], rax

        fstcall find_rsdp

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_acpi_tables_done

        cmp     [cfg_opt_used_resolution], 0
        jz      .not_used_resolution
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_opt_resolution
        call    clearbuf
        xor     edx, edx
        movzx   eax, [rdx+BOOT_LO.x_res]
        mov     rdi, msg
        call    num2dec
        xor     edx, edx
        movzx   eax, [rdx+BOOT_LO.y_res]
        mov     rdi, msg+8*2
        call    num2dec
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        fstcall find_vmode_index_by_resolution
.not_used_resolution:
        cmp     [cfg_opt_used_debug_print], 0
        jz      .not_used_debug_print
        movzx   eax, [cfg_opt_value_debug_print]
        xor     edx, edx
        mov     [rdx+BOOT_LO.debug_print], al
.not_used_debug_print:

        cmp     [cfg_opt_value_ask_params], 0
        jz      @f
        call    ask_for_params
@@:

        movzx   edx, [cfg_opt_value_vmode]
        mov     r10, [gop_interface]
        fstcall [r10+EFI_GRAPHICS_OUTPUT_PROTOCOL.SetMode], [gop_interface], rdx
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error
        jmp     $
@@:

        mov     rcx, [gop_interface]
        mov     rdx, [rcx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        mov     rdi, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase]
        mov     [fb_base], rdi

        mov     edx, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.Mode]
        mov     r10, [gop_interface]
        fstcall [r10+EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode], [gop_interface], \
                rdx, gop_info_size, gop_info
        test    eax, eax
        jz      @f
        jmp     .error
@@:
        mov     rcx, [gop_info]
        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        xor     rdx, rdx
        mov     [rdx+BOOT_LO.x_res], ax
        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        mov     [rdx+BOOT_LO.y_res], ax
        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
        shl     eax, 2
        mov     [rdx+BOOT_LO.pitch], ax

        mov     [rdx+BOOT_LO.pci_data.access_mechanism], 1
        movzx   eax, [pci_last_bus]
        mov     [rdx+BOOT_LO.pci_data.last_bus], al
        mov     [rdx+BOOT_LO.pci_data.version], 0x0300  ; PCI 3.0
        mov     [rdx+BOOT_LO.pci_data.pm_entry], 0

        ; kernel
;        fstcall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                450000/0x1000, EFI_ALLOCATE_ADDRESS

        ; ramdisk
;        fstcall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                2880*512/0x1000, EFI_ALLOCATE_ADDRESS

        fstcall calc_memmap
;        fstcall dump_memmap

        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.ExitBootServices], [efi_handle], \
                [memory_map_key]
        fstcall halt_on_error

        cli

        xor     edx, edx
        xor     esi, esi
        mov     [rsi+BOOT_LO.bpp], 32
        mov     [rsi+BOOT_LO.vesa_mode], dx
        mov     [rsi+BOOT_LO.bank_switch], edx
        mov     rdi, [fb_base]
        mov     [rsi+BOOT_LO.lfb], edi

        movzx   eax, [cfg_opt_value_mtrr]
        mov     [rsi+BOOT_LO.mtrr], al

        movzx   eax, [cfg_opt_value_launcher_start]
        mov     [rsi+BOOT_LO.launcher_start], al

        movzx   eax, [cfg_opt_value_debug_print]
        mov     [rsi+BOOT_LO.debug_print], al

        mov     [rsi+BOOT_LO.dma], dl
        mov     [rsi+BOOT_LO.apm_entry], edx
        mov     [rsi+BOOT_LO.apm_version], dx
        mov     [rsi+BOOT_LO.apm_flags], dx
        mov     [rsi+BOOT_LO.apm_code_32], dx
        mov     [rsi+BOOT_LO.apm_code_16], dx
        mov     [rsi+BOOT_LO.apm_data_16], dx
        mov     [rsi+BOOT_LO.bios_hd_cnt], dl

        movzx   eax, [cfg_opt_value_imgfrom]
        mov     [rsi+BOOT_LO.rd_load_from], al

        mov     eax, dword[devicesdat_size]
        mov     [rdx+BOOT_LO.devicesdat_size], eax
        mov     eax, dword[devicesdat_data]
        mov     [rdx+BOOT_LO.devicesdat_data], eax

        mov     rsi, cfg_opt_value_syspath
        mov     rdi, BOOT_LO.syspath
        mov     ecx, 0x17
        rep movsb

        ; kernel trampoline
        mov     rsi, kernel_trampoline
        mov     rdi, KERNEL_TRAMPOLINE
        mov     ecx, kernel_trampoline.size
        rep movsb

        mov     rax, GDTR
        lgdt    [cs:rax]

        mov     ax, DATA_32_SELECTOR
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax
        mov     ss, ax

        push    CODE_32_SELECTOR
        mov     rax, KERNEL_TRAMPOLINE
        push    rax
        retf

.error:
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error
        jmp     $
endp

proc halt_on_error
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        jmp     $
@@:
        ret
endp

proc get_protocol_interface uses rsi rdi, _guid
locals
        .status dq ?
endl
        mov     [_guid], rcx
        mov     [prot_handlers_buffer_size], PROTOCOL_HANDLERS_BUFFER_SIZE
        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.LocateHandle], \
                EFI_LOCATE_SEARCH_TYPE.ByProtocol, [_guid], 0, \
                prot_handlers_buffer_size, prot_handlers_buffer
        mov     [.status], rax

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_protocol_buffer_size
        call    clearbuf
        mov     rax, [prot_handlers_buffer_size]
        mov     rdi, msg
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg

        mov     rax, [.status]
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        jmp     $
@@:

        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_look_for_handler

        mov     rsi, prot_handlers_buffer
.try_next_handle:
        mov     rax, rsi
        mov     rcx, prot_handlers_buffer
        sub     rax, rcx
        cmp     rax, [prot_handlers_buffer_size]
        jb      @f
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error_out_of_handlers
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_error
        jmp     $
@@:
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_query_handler

        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.HandleProtocol], qword[rsi], [_guid], \
                prot_interface
;mov rax, 0x8000_0000_0000_0003
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg

        add     rsi, 8
        jmp     .try_next_handle
@@:
        mov     rax, [prot_interface]
        ret
endp


proc find_rsdp uses rsi rdi
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_look_for_rsdp

        mov     rdi, [rbx+EFI_SYSTEM_TABLE.ConfigurationTable]
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.NumberOfTableEntries]
        mov     rax, 0x11d3e4f18868e871
        mov     rdx, 0x81883cc7800022bc
.next_table:
        dec     ecx
        js      .all_tables_done
        cmp     [rdi+0], rax
        jnz     .not_acpi20
        cmp     [rdi+8], rdx
        jnz     .not_acpi20
        mov     rax, [rdi+16]
        mov     rdx, BOOT_LO.acpi_rsdp
        mov     [rdx], eax
        jmp     .all_tables_done
.not_acpi20:
        add     rdi, 24
        jmp     .next_table
.all_tables_done:
        ret
endp

proc dump_pci_resources uses rsi rdi
        xor     eax, eax
        mov     rsi, [pcirbiop_resources]
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_dump_pci_resources
.next_resource:
        call    clearbuf
        movzx   eax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.Type]
        cmp     eax, EFI_RESOURCE_DESCRIPTOR_TYPE.END_TAG
        jz      .done
        mov     rdi, msg
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        call    clearbuf
        movzx   eax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.ResourceType]
        mov     rdi, msg
        call    num2dec
        mov     rax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.RangeMinimum]
        mov     rdi, msg+2*2
        call    num2hex
        mov     rax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.RangeMaximum]
        mov     rdi, msg+19*2
        call    num2hex
        mov     rax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.TranslationOffset]
        mov     rdi, msg+36*2
        call    num2hex
        mov     rax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.AddressLength]
        mov     rdi, msg+53*2
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        movzx   eax, [rsi+EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR.Length]
        add     eax, 3
        add     rsi, rax
        jmp     .next_resource
.done:
        ret
endp

proc calc_memmap uses rsi rdi
        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.AllocatePages], EFI_ALLOCATE_ANY_PAGES, \
                EFI_RESERVED_MEMORY_TYPE, MEMORY_MAP_SIZE/0x1000, memory_map
        call    halt_on_error

        mov     r10, [rbx+EFI_SYSTEM_TABLE.BootServices]
        fstcall [r10+EFI_BOOT_SERVICES.GetMemoryMap], memory_map_size, \
                [memory_map], memory_map_key, descriptor_size, descriptor_ver
        call    halt_on_error

        mov     rdi, BOOT_LO.memmap_blocks
        mov     dword[rdi-4], 0 ; memmap_block_cnt
        mov     rsi, [memory_map]
        mov     r10, rsi
        add     r10, [memory_map_size]
.next_descr:
        fstcall add_uefi_memmap
        add     rsi, [descriptor_size]
        cmp     rsi, r10
        jb      .next_descr
        ret
endp

proc dump_memmap uses rsi rdi
        xor     eax, eax
        mov     rsi, BOOT_LO.memmap_blocks
        mov     r12, [rax+BOOT_LO.memmap_block_cnt]

        call    clearbuf
        mov     eax, ebx
        mov     rdi, msg
        call    num2dec
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, \
                msg_memmap
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        call    clearbuf
.next_mapping:
        dec     r12d
        js      .done
        mov     rax, rsi
        mov     rcx, BOOT_LO.memmap_blocks
        sub     rax, rcx
        mov     ecx, sizeof.e820entry
        xor     edx, edx
        div     ecx
        mov     rdi, msg
        call    num2dec
        mov     rax, [rsi+e820entry.addr]
        mov     rdi, msg+4*2
        call    num2hex
        mov     rax, [rsi+e820entry.size]
        mov     rdi, msg+24*2
        call    num2hex
        mov     rcx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        fstcall [rcx+EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString], rcx, msg
        add     rsi, sizeof.e820entry
        jmp     .next_mapping
.done:
        ret
endp

; linux/arch/x86/platform/efi/efi.c
; do_add_efi_memmap
proc add_uefi_memmap
        xor     eax, eax
        cmp     [rax+BOOT_LO.memmap_block_cnt], MAX_MEMMAP_BLOCKS
        jz      .done

        mov     rax, [rsi+EFI_MEMORY_DESCRIPTOR.PhysicalStart]
        mov     [rdi+e820entry.addr], rax

        mov     rax, [rsi+EFI_MEMORY_DESCRIPTOR.NumberOfPages]
        shl     rax, 12
        mov     [rdi+e820entry.size], rax

        mov     ecx, [rsi+EFI_MEMORY_DESCRIPTOR.Type]
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
        test    [rsi+EFI_MEMORY_DESCRIPTOR.Attribute], dword EFI_MEMORY_WB
        mov     eax, E820_RAM
        jnz     .type_done
.reserved:
        mov     eax, E820_RESERVED
.type_done:
        mov     [rdi+e820entry.type], eax
        cmp     eax, E820_RAM
        jnz     @f
        xor     eax, eax
        inc     [rax+BOOT_LO.memmap_block_cnt]
        add     rdi, sizeof.e820entry
@@:
.done:
        ret
endp

num2dec:
        push    rbx rcx rdx rdi

        xor     ecx, ecx
        mov     ebx, 10
.next_digit:
        xor     edx, edx
        div     ebx
        push    rdx
        inc     ecx
        test    eax, eax
        jnz     .next_digit

.next_char:
        pop     rax
        add     eax, '0'
        stosw
        loop    .next_char

        pop     rdi rdx rcx rbx
        ret


num2hex:
        push    rbx rcx rdx rdi

        xchg    rdx, rax
        mov     ecx, 16
.next_tetra:
        rol     rdx, 4
        movzx   eax, dl
        and     eax, 0x0f
        movzx   eax, byte[hex_abc+eax]
        stosw
        loop    .next_tetra

        pop     rdi rdx rcx rbx
        ret

clearbuf:
        push    rcx rdi
        mov     eax, ' '
        mov     ecx, 79
        mov     rdi, msg
        rep stosw
        pop     rdi rcx
        ret

use32
kernel_trampoline:
org KERNEL_TRAMPOLINE
        mov     eax, cr0
        and     eax, not CR0_PG
        mov     cr0, eax

        mov     ecx, MSR_AMD_EFER
        rdmsr
        btr     eax, 8                  ; LME
        wrmsr

        mov     eax, cr4
        and     eax, not CR4_PAE
        mov     cr4, eax

        mov     eax, KERNEL_BASE
        ; add the 32-bit entry point
        add     eax, [eax+kernel_header.b32_offset]
        push    eax
        retn

align 16
GDTR:
        dw 4*8-1
        dq GDT
align 16
GDT:
        dw 0, 0, 0, 0
        dw 0FFFFh,0,9A00h,0CFh          ; 32-bit code
        dw 0FFFFh,0,9200h,0CFh          ; flat data
        dw 0FFFFh,0,9A00h,0AFh          ; 64-bit code
assert $ < BOOT_LO
kernel_trampoline.size = $ - KERNEL_TRAMPOLINE

section '.rodata' data readable
gop_guid        db EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
lip_guid        db EFI_LOADED_IMAGE_PROTOCOL_GUID
sfsp_guid       db EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
pcirbiop_guid   db EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID

file_name       du '\EFI\KOLIBRIOS\KOLIBRI.INI',0
kernel_name     du '\EFI\KOLIBRIOS\KOLIBRI.KRN',0
ramdisk_name    du '\EFI\KOLIBRIOS\KOLIBRI.IMG',0
devicesdat_name du '\EFI\KOLIBRIOS\DEVICES.DAT',0

config_options  dq cfg_opt_name_resolution, cfg_opt_func_resolution, \
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

msg_u4k_loaded            du "uefi64kos loaded",13,10,0
msg_detect_pci_config     du "Detect PCI configuration",13,10,0
msg_dump_pci_resources    du "Dump PCI resources",13,10,0
msg_pci_last_bus          du "Last PCI bus",13,10,0
msg_read_options          du "Read options from config file",13,10,0
msg_file_size             du "File size:",13,10,0
msg_parsing_config        du "Parsing config file",13,10,0
msg_load_kernel           du "Load kernel",13,10,0
msg_load_ramdisk          du "Load ramdisk",13,10,0
msg_load_devicesdat       du "Load DEVICES.DAT",13,10,0
msg_alloc_devicesdat      du "Allocate memory for DEVICES.DAT",13,10,0
msg_locate_gop_interface  du "Locate GOP interface",13,10,0
msg_look_for_handler      du "Look for protocol handler",13,10,0
msg_query_handler         du "Query handler",13,10,0
msg_query_vmode           du "Query vmode",13,10,0
msg_vmode_found           du "Video mode found",13,10,0
msg_look_for_rsdp         du "Look for RSDP",13,10,0
msg_rsdp_found            du "RSDP found",13,10,0
msg_acpi_tables_done      du "ACPI tables done",13,10,0
msg_ask_for_params        du "Ask for params",13,10,0
msg_set_graphic_mode      du "Set graphic mode",13,10,0
msg_success               du "Success!",13,10,0
msg_protocol_buffer_size  du "Protocol buffer size",13,10,0
msg_opt_resolution        du "Option resolution: ",0
msg_memmap                du "Memmap",13,10,0
msg_error                 du "Error!",13,10,0
msg_error_efi_lip_handle  du "efi_handle can't handle LIP",13,10,0
msg_error_lip_dev_sfsp    du "LIP device handle can't handle SFSP",13,10,0
msg_error_sfsp_openvolume du "SFSP OpenVolume failed",13,10,0
msg_error_no_such_vmode   du "No such vmode",13,10,0
msg_error_out_of_handlers du "Out of handlers",13,10,0
msg_error_open_file       du "Error: can't open file ",0
msg_error_exit_boot_services du "Error: Exit boot services",13,10,0
msg                       du 79 dup " ",13,10,0
msg_debug                 du "Debug ",13,10,0

section '.data' data readable writeable
efi_handle  dq 0
efi_table   dq 0

fb_base         dq 0

gop_interface   dq 0
gop_info_size   dq 0
gop_info        dq 0

lip_interface   dq 0

sfsp_interface  dq 0

pci_last_bus db 254

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

memory_map_key  dq 0
descriptor_size dq 0
descriptor_ver  dq 0
memory_map_size dq MEMORY_MAP_SIZE

efi_fs_info_id db EFI_FILE_SYSTEM_INFO_ID
efi_fs_info_size dq sizeof.EFI_FILE_SYSTEM_INFO

devicesdat_data dq 0xffffffff
devicesdat_size dq 0x1000

hex_abc db '0123456789ABCDEF'

section '.bss' data readable writeable discardable
prot_handlers_buffer_size dq ?
prot_interface  dq ?
esp_root        dq ?
file_handle     dq ?
pcirbiop_interface dq ?
pcirbiop_resources dq ?
efi_fs_info EFI_FILE_SYSTEM_INFO
memory_map      dq ?
prot_handlers_buffer rq PROTOCOL_HANDLERS_BUFFER_SIZE/8
pcirbio_buffer  rq PROTOCOL_HANDLERS_BUFFER_SIZE/8

section '.reloc' fixups data discardable
