;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2020. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;; Version 2, or (at your option) any later version.            ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

format pe64 efi
entry main

section '.text' code executable readable

include '../../struct.inc'
include '../../macros.inc'
include '../../const.inc'

purge DQ
include 'uefi64.inc'

MEMORY_MAP_SIZE = 0x10000
GOP_BUFFER_SIZE = 0x100
LIP_BUFFER_SIZE = 0x100
FILE_BUFFER_SIZE = 0x1000

KERNEL_TRAMPOLINE = 0x8f80      ; just before BOOT_LO
KERNEL_BASE  =  0x10000
RAMDISK_BASE = 0x100000
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

load_file:
virtual at rsp+8
  .root   dq ?
  .name   dq ?
  .buffer dq ?
  .size   dq ?
  .fatal  dq ?
end virtual
        eficall [.root], EFI_FILE_PROTOCOL.Open, [.root], file_handle, \
                [.name], EFI_FILE_MODE_READ, 0
        test    eax, eax
        jz      @f
        xor     eax, eax
        cmp     [.fatal], 1
        jnz     .done
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error_open_file
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                [.name]
        jmp     $
@@:

        lea     rax, [.size]
        eficall [file_handle], EFI_FILE_PROTOCOL.Read, [file_handle], rax, \
                [.buffer]
        eficall [file_handle], EFI_FILE_PROTOCOL.Close, [file_handle]
        mov     rax, [.size]
.done:
        push    rax
        call    clearbuf
        mov     rdi, msg
        call    num2dec
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_file_size
        pop     rbx
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        pop     rbx
        pop     rax
        ret     8*5

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

parse_config:
virtual at rsp+8
  .buffer      dq ?
end virtual
;        mov     rsi, [.buffer]
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_parsing_config
        pop     rbx
        mov     rsi, KERNEL_BASE
.next_line:
        call    parse_line
        cmp     byte[rsi], 0
        jnz     .next_line
        ret     1*8

read_options_from_config:
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.HandleProtocol, [efi_handle], lipuuid, \
                lip_interface
        test    eax, eax
        jz      @f
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error_efi_lip_handle
        pop     rbx
        jmp     $
@@:
        mov     rax, [lip_interface]

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.HandleProtocol, \
                [rax+EFI_LOADED_IMAGE_PROTOCOL.DeviceHandle], sfspguid, \
                sfsp_interface
        test    eax, eax
        jz      @f
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error_lip_dev_sfsp
        pop     rbx
        jmp     $
@@:
        eficall [sfsp_interface], EFI_SIMPLE_FILE_SYSTEM_PROTOCOL.OpenVolume, \
                [sfsp_interface], esp_root
        test    eax, eax
        jz      @f
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error_sfsp_openvolume
        pop     rbx
        jmp     $
@@:
        push    0 ; not fatal, i.e. it's ok to not find this file
        push    FILE_BUFFER_SIZE
        push    KERNEL_BASE
;        push    file_name
        mov     rax, file_name
        push    rax
        push    [esp_root]
        call    load_file

        test    eax, eax
        jz      @f
        push    KERNEL_BASE
        call    parse_config
@@:

.error:
        ret

print_vmode:
        push    rax rbx rcx rdx rsi rdi
        mov     rbx, rcx
        call    clearbuf
        mov     eax, [rbx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        mov     rdi, msg
        call    num2dec
        mov     eax, [rbx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        mov     rdi, msg+8*2
        call    num2dec

        mov     eax, [rbx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat]
        mov     rdi, msg+16*2
        call    num2dec

        mov     eax, [rbx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
        mov     rdi, msg+24*2
        call    num2dec
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        pop     rdi rsi rdx rcx rbx rax
        ret

find_vmode_index_by_resolution:
        mov     [cfg_opt_used_resolution], 1
        mov     [cfg_opt_value_vmode], 0
.next_mode:
        movzx   eax, [cfg_opt_value_vmode]
        eficall [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode, \
                [gop_interface], rax, gop_info_size, gop_info
        cmp     rax, EFI_SUCCESS
        jnz     .error
        mov     rcx, [gop_info]
        call    print_vmode
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

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error_no_such_vmode
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error
        jmp     $
.error:
.done:
        ret

ask_for_params:
        ret
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.HandleProtocol, [rax], gopuuid, \
                msg_ask_for_params
        jmp     $

        xor     ebx, ebx
.next_mode:
        call    clearbuf
        mov     eax, ebx
        lea     rdi, [msg]
        call    num2dec

        push    rbx
        eficall [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode, \
                [gop_interface], rbx, gop_info_size, gop_info
        cmp     rax, EFI_SUCCESS
        jnz     .error
        mov     rcx, [gop_info]
        ; PixelBlueGreenRedReserved8BitPerColor
        cmp     [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat], 1
        jnz     .skip
        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        lea     rdi, [msg+4*2]
        call    num2dec
        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        lea     rdi, [msg+9*2]
        call    num2dec
;        mov     eax, [rcx+EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
;        lea     rdi, [msg+14*2]
;        call    num2dec
.skip:
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        cmp     rax, EFI_SUCCESS
        jnz     .error

        pop     rbx
        inc     rbx
        mov     rcx, [gop_interface]
        mov     rdx, [rcx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        cmp     ebx, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.MaxMode]
        jnz     .next_mode


        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConIn]
        eficall rbx, SIMPLE_INPUT_INTERFACE.Reset, rbx, 1
        cmp     rax, EFI_SUCCESS
        jnz     .error
        xor     ecx, ecx
    @@:
        push    rcx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConIn]
        eficall rbx, SIMPLE_INPUT_INTERFACE.ReadKeyStroke, rbx, msg
        pop     rcx
        mov     rdx, EFI_DEVICE_ERROR
        cmp     rax, rdx
        jz      .error
        mov     rdx, EFI_NOT_READY
        cmp     rax, rdx
        jz      @b
;        cmp     rax, EFI_SUCCESS
        movzx   eax, word[msg+2]
;jmp .key_done
        cmp     al, 0x0D
        jz      .key_done
        imul    ecx, 10
        sub     eax, '0'
        add     ecx, eax
        jmp     @b
.key_done:
        mov     [cfg_opt_value_vmode], cl
.error:
.done:
        ret

main:
        mov     [efi_handle], rcx
        mov     [efi_table], rdx

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.Reset, rbx, 1
        test    eax, eax
        jz      @f
        jmp     $       ; what can I do here?
@@:
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_u4k_loaded

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_read_options
        call    read_options_from_config

        ; read kernel file
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_load_kernel
        push    1       ; fatal
        push    MAX_FILE_SIZE
        push    KERNEL_BASE
;        push    kernel_name
        mov     rax, kernel_name
        push    rax
        push    [esp_root]
        call    load_file

        ; read ramdisk image
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_load_ramdisk
        push    1       ; fatal
        push    MAX_FILE_SIZE
        push    RAMDISK_BASE
;        push    ramdisk_name
        mov     rax, ramdisk_name
        push    rax
        push    [esp_root]
        call    load_file

        ; alloc buffer for devices.dat
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_alloc_devicesdat
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.AllocatePages, \
                EFI_ALLOCATE_MAX_ADDRESS, EFI_RESERVED_MEMORY_TYPE, 1, \
                devicesdat_data
        cmp     eax, EFI_SUCCESS
        jnz     .error

        ; read devices.dat
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_load_devicesdat

        push    0 ; not fatal
        push    [devicesdat_size]
        push    [devicesdat_data]
;        push    devicesdat_name
        mov     rax, devicesdat_name
        push    rax
        push    [esp_root]
        call    load_file
        mov     [devicesdat_size], rax

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_locate_gop_handlers

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.LocateHandle, \
                EFI_LOCATE_SEARCH_TYPE.ByProtocol, gopuuid, 0, \
                gop_buffer_size, gop_buffer
        mov     [status], rax

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_gop_buffer_size
        call    clearbuf
        mov     rax, [gop_buffer_size]
        mov     rdi, msg
        call    num2hex
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg

        mov     rax, [status]
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        jmp     $
@@:

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_look_for_gop_handler

        mov     rbx, gop_buffer
.next_gop_handle:
        mov     rax, rbx
        mov     rcx, gop_buffer
        sub     rax, rcx
        cmp     rax, [gop_buffer_size]
        jb      @f
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error_out_of_handlers
        pop     rbx
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg_error
        pop     rbx
        jmp     $
@@:
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_query_handler
        pop     rbx

        mov     rax, rbx
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.HandleProtocol, [rax], gopuuid, \
                gop_interface
        pop     rbx
;mov rax, 0x8000_0000_0000_0003
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        pop     rbx

        add     rbx, 8
        jmp     .next_gop_handle
@@:

        call    find_rsdp

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_acpi_tables_done

        cmp     [cfg_opt_used_resolution], 0
        jz      .not_used_resolution
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
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
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        call    find_vmode_index_by_resolution
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

        movzx   ecx, [cfg_opt_value_vmode]
        eficall [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.SetMode, \
                [gop_interface], rcx
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error
        jmp     $
@@:

        mov     rcx, [gop_interface]
        mov     rdx, [rcx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        mov     rdi, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase]
        mov     [fb_base], rdi

        mov     ebx, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.Mode]
        eficall [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode, \
                [gop_interface], rbx, gop_info_size, gop_info
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

        mov     byte[rdx+BOOT_LO.pci_data+0], 1    ; PCI access mechanism
        mov     byte[rdx+BOOT_LO.pci_data+1], 8    ; last bus, don't know how to count them
        mov     byte[rdx+BOOT_LO.pci_data+2], 0x10 ; PCI version
        mov     byte[rdx+BOOT_LO.pci_data+3], 0x02
        mov     dword[rdx+BOOT_LO.pci_data+4], 0xe3

        ; kernel
;        eficall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                450000/0x1000, EFI_ALLOCATE_ADDRESS

        ; ramdisk
;        eficall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                2880*512/0x1000, EFI_ALLOCATE_ADDRESS

        call    calc_memmap
;        call    dump_memmap

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.ExitBootServices, [efi_handle], \
                [memory_map_key]
        call    halt_on_error

        cli

        xor     edx, edx
        xor     esi, esi
        mov     [esi+BOOT_LO.bpp], 32
        mov     [esi+BOOT_LO.vesa_mode], dx
        mov     [esi+BOOT_LO.bank_switch], edx
        mov     rdi, [fb_base]
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
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error
        jmp     $


halt_on_error:
        test    eax, eax
        jz      @f
        call    clearbuf
        mov     rdi, msg
        call    num2hex
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_error
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        jmp     $
@@:
        ret

find_rsdp:
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_look_for_rsdp
        pop     rbx

        mov     rbx, [efi_table]
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

calc_memmap:
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.AllocatePages, EFI_ALLOCATE_ANY_PAGES, \
                EFI_RESERVED_MEMORY_TYPE, MEMORY_MAP_SIZE/0x1000, memory_map
        call    halt_on_error

        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.BootServices]
        eficall rbx, EFI_BOOT_SERVICES.GetMemoryMap, memory_map_size, \
                [memory_map], memory_map_key, descriptor_size, descriptor_ver
        call    halt_on_error

        mov     rdi, BOOT_LO.memmap_blocks
        mov     dword[rdi-4], 0 ; memmap_block_cnt
        mov     rsi, [memory_map]
        mov     rbx, rsi
        add     rbx, [memory_map_size]
.next_descr:
        call    add_uefi_memmap
        add     rsi, [descriptor_size]
        cmp     rsi, rbx
        jb      .next_descr
        ret

dump_memmap:
        xor     eax, eax
        mov     rsi, BOOT_LO.memmap_blocks
        mov     ebx, [rax+BOOT_LO.memmap_block_cnt]

        call    clearbuf
        mov     eax, ebx
        mov     rdi, msg
        call    num2dec
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, \
                msg_memmap
        pop     rbx
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        pop     rbx
        call    clearbuf
.next_mapping:
        dec     ebx
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
        push    rbx
        mov     rbx, [efi_table]
        mov     rbx, [rbx+EFI_SYSTEM_TABLE.ConOut]
        eficall rbx, EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL.OutputString, rbx, msg
        pop     rbx
        add     rsi, sizeof.e820entry
        jmp     .next_mapping
.done:
        ret


; linux/arch/x86/platform/efi/efi.c
; do_add_efi_memmap
add_uefi_memmap:
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


num2dec:
        push    rax rbx rcx rdx rsi rdi

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

        pop     rdi rsi rdx rcx rbx rax
        ret


num2hex:
        push    rax rbx rcx rdx rsi rdi

        xchg    rdx, rax
        mov     ecx, 16
.next_tetra:
        rol     rdx, 4
        movzx   eax, dl
        and     eax, 0x0f
        movzx   eax, byte[hex+eax]
        stosw
        loop    .next_tetra

        pop     rdi rsi rdx rcx rbx rax
        ret

hex db '0123456789ABCDEF'

clearbuf:
        push    rax rbx rcx rdx rsi rdi
        mov     eax, 0x0020
        mov     ecx, 79
        mov     rdi, msg
        rep stosw
        pop     rdi rsi rdx rcx rbx rax
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

        push    KERNEL_BASE
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
gopuuid         db EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
lipuuid         db EFI_LOADED_IMAGE_PROTOCOL_GUID
sfspguid        db EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID

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
msg_read_options          du "Read options from config file",13,10,0
msg_file_size             du "File size:",13,10,0
msg_parsing_config        du "Parsing config file",13,10,0
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


section '.data' data readable writeable
efi_handle  dq 0
efi_table   dq 0
uefi_rsptmp dq 0

fb_base         dq 0

gop_buffer_size dq GOP_BUFFER_SIZE
gop_handle      dq 0
gop_interface   dq 0
gop_info_size   dq 0
gop_info        dq 0

lip_buffer_size dq LIP_BUFFER_SIZE
lip_handle      dq 0
lip_interface   dq 0

sfsp_interface  dq 0

esp_root        dq ?
file_handle     dq ?
file_buffer_size dq FILE_BUFFER_SIZE-1  ; leave the last byte for \0

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
efi_fs_info EFI_FILE_SYSTEM_INFO

memory_map      dq ?
gop_buffer      rq GOP_BUFFER_SIZE/8
devicesdat_data dq 0xffffffff
devicesdat_size dq 0x1000
status          dq ?

section '.reloc' fixups data discardable
