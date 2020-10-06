format pe64 dll efi
entry main

section '.text' code executable readable

include '../../struct.inc'
include '../../macros.inc'
include '../../const.inc'
include 'uefi.inc'

MEMORY_MAP_SIZE = 0x4000
GOP_BUFFER_SIZE = 0x100
LIP_BUFFER_SIZE = 0x100
FILE_BUFFER_SIZE = 0x1000

KERNEL_BASE  =  0x10000
RAMDISK_BASE = 0x100000

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
        cmp     rax, EFI_SUCCESS
        jz      @f
        xor     eax, eax
        cmp     [.fatal], 1
        jnz     .done
        eficall ConOut, OutputString, ConOut, msg_error_open_file
        eficall ConOut, OutputString, ConOut, [.name]
        jmp     $
@@:

        lea     rax, [.size]
        eficall [file_handle], EFI_FILE_PROTOCOL.Read, [file_handle], rax, \
                [.buffer]
        eficall [file_handle], EFI_FILE_PROTOCOL.Close, [file_handle]
        mov     rax, [.size]
.done:
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
.done:
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
        mov     rsi, KERNEL_BASE
.next_line:
        call    parse_line
        cmp     byte[rsi], 0
        jnz     .next_line
        ret     1*8

read_options_from_config:
        eficall BootServices, HandleProtocol, qword[efi_handler], lipuuid, \
                lip_interface
        cmp     eax, EFI_SUCCESS
        jnz     .error
        mov     rax, [lip_interface]

        eficall BootServices, HandleProtocol, \
                [rax+EFI_LOADED_IMAGE_PROTOCOL.DeviceHandle], sfspguid, \
                sfsp_interface
        cmp     rax, EFI_SUCCESS
        jnz     .error

        eficall [sfsp_interface], EFI_SIMPLE_FILE_SYSTEM_PROTOCOL.OpenVolume, \
                [sfsp_interface], esp_root
        cmp     rax, EFI_SUCCESS
        jnz     .error

        push    0 ; not fatal, i.e. it's ok to not find this file
        push    FILE_BUFFER_SIZE
        push    KERNEL_BASE
        push    file_name
        push    [esp_root]
        call    load_file

        test    eax, eax
        jz      @f
        push    KERNEL_BASE
        call    parse_config
@@:

.error:
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
.error:
.done:
        ret

ask_for_params:
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
        eficall ConOut, OutputString, ConOut, msg
        cmp     rax, EFI_SUCCESS
        jnz     .error

        pop     rbx
        inc     rbx
        mov     rcx, [gop_interface]
        mov     rdx, [rcx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        cmp     ebx, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.MaxMode]
        jnz     .next_mode


        eficall ConIn, Reset, ConIn, 1
        cmp     rax, EFI_SUCCESS
        jnz     .error
        xor     ecx, ecx
    @@:
        push    rcx
        eficall ConIn, ReadKeyStroke, ConIn, msg
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
        sub     rsp, 0x38

        ; initialize UEFI library
        InitializeLib
        jc      .error

;        eficall ConOut, Reset, ConOut, 1
;        cmp     rax, EFI_SUCCESS
;        jnz     .error

        eficall ConOut, ClearScreen, ConOut
        cmp     rax, EFI_SUCCESS
        jnz     .error

        call    read_options_from_config

        ; read kernel file
        push    1
        push    -1
        push    KERNEL_BASE
        push    kernel_name
        push    [esp_root]
        call    load_file

        ; read ramdisk image
        push    1
        push    -1
        push    RAMDISK_BASE
        push    ramdisk_name
        push    [esp_root]
        call    load_file

        ; alloc buffer for devices.dat
        eficall BootServices, AllocatePages, EFI_ALLOCATE_MAX_ADDRESS, \
                EFI_RESERVED_MEMORY_TYPE, 1, devicesdat_data
        cmp     eax, EFI_SUCCESS
        jnz     .error

        ; read devices.dat
        push    0 ; not fatal
        push    [devicesdat_size]
        push    [devicesdat_data]
        push    devicesdat_name
        push    [esp_root]
        call    load_file
        mov     [devicesdat_size], rax

        eficall BootServices, LocateHandle, 2, gopuuid, 0, gop_buffer_size, \
                gop_buffer
        cmp     eax, EFI_SUCCESS
        jnz     .error
        mov     rsi, gop_buffer
        lodsq
        mov     [gop_handle], rax
        eficall BootServices, HandleProtocol, qword[gop_handle], gopuuid, \
                gop_interface
        cmp     eax, EFI_SUCCESS
        jnz     .error

        mov     rbx, [efi_ptr]
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

        cmp     [cfg_opt_used_resolution], 0
        jz      .not_used_resolution
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
        cmp     eax, EFI_SUCCESS
        jnz     .error

        mov     rcx, [gop_interface]
        mov     rdx, [rcx+EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        mov     rdi, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase]
        mov     [fb_base], rdi


        mov     ebx, [rdx+EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.Mode]
        eficall [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode, \
                [gop_interface], rbx, gop_info_size, gop_info
        cmp     rax, EFI_SUCCESS
        jnz     .error
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


        eficall BootServices, AllocatePages, EFI_ALLOCATE_ANY_PAGES, \
                EFI_RESERVED_MEMORY_TYPE, MEMORY_MAP_SIZE/0x1000, memory_map
        cmp     eax, EFI_SUCCESS
        jnz     .error

        eficall BootServices, GetMemoryMap, memory_map_size, [memory_map], \
                memory_map_key, descriptor_size, descriptor_ver
        cmp     eax, EFI_SUCCESS
        jnz     .error

        mov     rdi, BOOT_LO.memmap_block_cnt
        mov     dword[rdi], 0
        mov     rdi, BOOT_LO.memmap_blocks
        mov     rax, [memory_map_size]
        xor     edx, edx
        mov     rcx, [descriptor_size]
        div     ecx
        mov     ecx, eax
        mov     rsi, [memory_map]
  .next_descr:
        call    add_uefi_memmap
        add     rsi, [descriptor_size]
        add     rdi, sizeof.e820entry
        dec     ecx
        test    ecx, ecx
        jnz     .next_descr

        ; kernel
;        eficall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                450000/0x1000, EFI_ALLOCATE_ADDRESS

        ; ramdisk
;        eficall BootServices, AllocatePages, EFI_RESERVED_MEMORY_TYPE, \
;                2880*512/0x1000, EFI_ALLOCATE_ADDRESS

        eficall BootServices, ExitBootServices, [efi_handler], [memory_map_key]
        cmp     eax, EFI_SUCCESS
        jnz     .error


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

        lgdt    [cs:GDTR]

        mov     ax, DATA_32_SELECTOR
        mov     ds, ax
        mov     es, ax
        mov     fs, ax
        mov     gs, ax
        mov     ss, ax

        push    CODE_32_SELECTOR
        lea     rax, [.next]
        push    rax
;        push    .next
        retf
use32
align 16
  .next:
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

use64
  .error:
        eficall ConOut, OutputString, ConOut, msg_error
        jmp     .quit
  .quit:
        mov     rcx, -1
        loop    $


; linux/arch/x86/platform/efi/efi.c
; do_add_efi_memmap
add_uefi_memmap:
        push    rax rbx rcx rdx rsi rdi

        mov     r10d, [rsi+0]
        mov     r11, [rsi+8]
;        mov     r12, [rsi+16]
        mov     r13, [rsi+24]
        mov     r14, [rsi+32]

        mov     [rdi+e820entry.addr], r11
        mov     rax, r13
        shl     rax, 12
        mov     [rdi+e820entry.size], rax


        cmp     r10d, EFI_LOADER_CODE
        jz      .case0
        cmp     r10d, EFI_LOADER_DATA
        jz      .case0
        cmp     r10d, EFI_BOOT_SERVICES_CODE
        jz      .case0
        cmp     r10d, EFI_BOOT_SERVICES_DATA
        jz      .case0
        cmp     r10d, EFI_CONVENTIONAL_MEMORY
        jz      .case0
        cmp     r10d, EFI_ACPI_RECLAIM_MEMORY
        jz      .case1
        cmp     r10d, EFI_ACPI_MEMORY_NVS
        jz      .case2
        cmp     r10d, EFI_UNUSABLE_MEMORY
        jz      .case3
        cmp     r10d, EFI_PERSISTENT_MEMORY
        jz      .case4
        jmp     .default

  .case0:
        test    r14, EFI_MEMORY_WB
        jz      @f
        mov     eax, E820_RAM
        jmp     .done
    @@:
        mov     eax, E820_RESERVED
        jmp     .done
  .case1:
        mov     eax, E820_ACPI
        jmp     .done
  .case2:
        mov     eax, E820_NVS
        jmp     .done
  .case3:
        mov     eax, E820_UNUSABLE
        jmp     .done
  .case4:
        mov     eax, E820_PMEM
        jmp     .done
  .default:
        mov     eax, E820_RESERVED
        jmp     .done

  .done:
        mov     [rdi+e820entry.type], eax

        mov     rax, BOOT_LO.memmap_block_cnt
        inc     word[rax]

        pop     rdi rsi rdx rcx rbx rax
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

section '.data' data readable writeable

GDTR:
        dw 4*8-1
        dq GDT
GDT:
        dw 0, 0, 0, 0
        dw 0FFFFh,0,9A00h,0CFh          ; 32-bit code
        dw 0FFFFh,0,9200h,0CFh          ; flat data
        dw 0FFFFh,0,9A00h,0AFh          ; 64-bit code


fb_base         dq 0

gopuuid         db EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID
gop_buffer_size dq GOP_BUFFER_SIZE
gop_handle      dq 0
gop_interface   dq 0
gop_info_size   dq 0
gop_info        dq 0

lipuuid         db EFI_LOADED_IMAGE_PROTOCOL_GUID
lip_buffer_size dq LIP_BUFFER_SIZE
lip_handle      dq 0
lip_interface   dq 0

sfspguid        db EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID
sfsp_interface  dq 0

esp_root        dq ?
file_handle     dq ?
file_name       du '\EFI\BOOT\KOLIBRI.INI',0
kernel_name     du '\EFI\BOOT\KOLIBRI.KRN',0
ramdisk_name    du '\EFI\BOOT\KOLIBRI.IMG',0
devicesdat_name du '\EFI\BOOT\DEVICES.DAT',0
file_buffer_size dq FILE_BUFFER_SIZE-1  ; leave the last byte for \0

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

cfg_opt_name_resolution db 'resolution',0
cfg_opt_name_acpi db 'acpi',0
cfg_opt_name_debug_print db 'debug_print',0
cfg_opt_name_launcher_start db 'launcher_start',0
cfg_opt_name_mtrr db 'mtrr',0
cfg_opt_name_ask_params db 'ask_params',0
cfg_opt_name_imgfrom db 'imgfrom',0
cfg_opt_name_syspath db 'syspath',0

cfg_opt_cmnt_resolution db "# Graphic mode",0
cfg_opt_cmnt_acpi db "# ACPI settings",0xa, \
                     "#   0: don't use",0xa, \
                     "#   1: parse ACPI tables",0xa, \
                     "#   2: + call _PIC method",0xa, \
                     "#   3: + get APIC interrupts",0xa,0
cfg_opt_cmnt_debug_print db "# Duplicate debug output to the screen",0
cfg_opt_cmnt_launcher_start db "# Start LAUNCHER app after kernel is loaded",0
cfg_opt_cmnt_mtrr db "# Configure MTRR's",0
cfg_opt_cmnt_ask_params db "# Interrupt booting to ask the user for boot params",0
cfg_opt_cmnt_imgfrom db "# Where to load ramdisk image from",0
cfg_opt_cmnt_syspath db "# Path to /sys directory",0

cfg_opt_used_resolution db 0
cfg_opt_used_acpi db 0
cfg_opt_used_debug_print db 0
cfg_opt_used_launcher_start db 0
cfg_opt_used_mtrr db 0
cfg_opt_used_ask_params db 0
cfg_opt_used_imgfrom db 0
cfg_opt_used_syspath db 0

cfg_opt_value_vmode db 0
cfg_opt_value_acpi db 0
cfg_opt_value_debug_print db 0
cfg_opt_value_launcher_start db 1
cfg_opt_value_mtrr db 0
cfg_opt_value_ask_params db 1
cfg_opt_value_imgfrom db RD_LOAD_FROM_MEMORY
cfg_opt_value_syspath db '/RD/1',0
                      rb 20

memory_map_key  dq 0
descriptor_size dq 0
descriptor_ver  dq 0
memory_map_size dq MEMORY_MAP_SIZE

msg_success     du 'Success!',13,10,0
msg_error       du 'Error!',13,10,0
msg             du 79 dup ' ',13,10,0
msg_error_open_file du "Error: can't open file ",0

efi_fs_info_id db EFI_FILE_SYSTEM_INFO_ID
efi_fs_info_size dq sizeof.EFI_FILE_SYSTEM_INFO
efi_fs_info EFI_FILE_SYSTEM_INFO

;section '.bss' data readable writeable

memory_map      dq ?
gop_buffer      rb GOP_BUFFER_SIZE
lip_buffer      rb LIP_BUFFER_SIZE
devicesdat_data dq 0xffffffff
devicesdat_size dq 0x1000

section '.reloc' fixups data discardable
