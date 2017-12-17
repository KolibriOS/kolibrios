format pe64 dll efi at 0
entry main

 
section '.text' code executable readable
 
include '../struct.inc'
include '../macros.inc'
include '../const.inc'
include 'uefi.inc'

MEMORY_MAP_SIZE = 0x4000
GOP_BUFFER_SIZE = 0x800

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
;E820_MAX       = 128

main:
        sub     rsp, 0x38
        ; initialize UEFI library
        InitializeLib
        jc      .error
 
;        uefi_call_wrapper ConOut, Reset, ConOut, 1
;        cmp     rax, EFI_SUCCESS
;        jnz     .error

        uefi_call_wrapper ConOut, ClearScreen, ConOut
        cmp     rax, EFI_SUCCESS
        jnz     .error

;        uefi_call_wrapper ConOut, OutputString, ConOut, msg_hello
;        cmp     eax, EFI_SUCCESS
;        jnz     .error

        uefi_call_wrapper BootServices, LocateHandle, 2, gopuuid, 0, gop_buffer_size, gop_buffer
        cmp     eax, EFI_SUCCESS
        jnz     .error

        mov     rsi, gop_buffer
        lodsq
        mov     [gop_handle], rax
        uefi_call_wrapper BootServices, HandleProtocol, qword [gop_handle], gopuuid, gop_interface
        cmp     eax, EFI_SUCCESS
        jnz     .error

        mov     rbx, [efi_ptr]
        mov     rdi, [rbx + EFI_SYSTEM_TABLE.ConfigurationTable]
        mov     rcx, [rbx + EFI_SYSTEM_TABLE.NumberOfTableEntries]
        mov     rax, 0x11d3e4f18868e871
        mov     rdx, 0x81883cc7800022bc
  .next_table:
        dec     ecx
        js      .all_tables_done
        cmp     [rdi + 0], rax
        jnz     .not_acpi20
        cmp     [rdi + 8], rdx
        jnz     .not_acpi20
        mov     rax, [rdi + 16]
        mov     rdx, BOOT_LO.acpi_rsdp
        mov     [rdx], eax
;jmp $
        jmp     .all_tables_done
  .not_acpi20:
        add     rdi, 24
        jmp     .next_table
  .all_tables_done:

        xor     ebx, ebx
  .next_mode:
        call    clearbuf
        mov     eax, ebx
        lea     rdi, [msg]
        call    num2dec

        push    rbx
        uefi_call_wrapper [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode, [gop_interface], rbx, gop_info_size, gop_info
        cmp     rax, EFI_SUCCESS
        jnz     .error
        mov     rcx, [gop_info]
        cmp     [rcx + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelFormat], 1     ; PixelBlueGreenRedReserved8BitPerColor
        jnz     .skip
        mov     eax, [rcx + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        lea     rdi, [msg+4*2]
        call    num2dec
        mov     eax, [rcx + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        lea     rdi, [msg+9*2]
        call    num2dec
;        mov     eax, [rcx + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
;        lea     rdi, [msg+14*2]
;        call    num2dec
  .skip:
        uefi_call_wrapper ConOut, OutputString, ConOut, msg
        cmp     rax, EFI_SUCCESS
        jnz     .error

        pop     rbx
        inc     rbx
        mov     rcx, [gop_interface]
        mov     rdx, [rcx + EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        cmp     ebx, [rdx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.MaxMode]
        jnz     .next_mode


        uefi_call_wrapper ConIn, Reset, ConIn, 1
        cmp     rax, EFI_SUCCESS
        jnz     .error
        xor     ecx, ecx
    @@:
        push    rcx
        uefi_call_wrapper ConIn, ReadKeyStroke, ConIn, msg
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

        uefi_call_wrapper [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.SetMode, [gop_interface], rcx
        cmp     eax, EFI_SUCCESS
        jnz     .error

        mov     rcx, [gop_interface]
        mov     rdx, [rcx + EFI_GRAPHICS_OUTPUT_PROTOCOL.Mode]
        mov     rdi, [rdx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.FrameBufferBase]
        mov     [fb_base], rdi


        mov     ebx, [rdx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.Mode]
        uefi_call_wrapper [gop_interface], EFI_GRAPHICS_OUTPUT_PROTOCOL.QueryMode, [gop_interface], rbx, gop_info_size, gop_info
        cmp     rax, EFI_SUCCESS
        jnz     .error
        mov     rcx, [gop_info]
        mov     eax, [rcx + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.HorizontalResolution]
        xor     rdx, rdx
        mov     word [rdx + BOOT_LO.x_res], ax
        mov     eax, [rcx + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.VerticalResolution]
        mov     word [rdx + BOOT_LO.y_res], ax
        mov     eax, [rcx + EFI_GRAPHICS_OUTPUT_MODE_INFORMATION.PixelsPerScanLine]
        shl     eax, 2
        mov     word [rdx + BOOT_LO.pitch], ax

        mov     byte [rdx + BOOT_LO.pci_data + 0], 1
        mov     byte [rdx + BOOT_LO.pci_data + 1], 0
        mov     byte [rdx + BOOT_LO.pci_data + 2], 0x10
        mov     byte [rdx + BOOT_LO.pci_data + 3], 0x02
        mov     dword [rdx + BOOT_LO.pci_data + 4], 0xe3


        uefi_call_wrapper BootServices, GetMemoryMap, memory_map_size, memory_map, memory_map_key, descriptor_size, descriptor_ver
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
        mov     rsi, memory_map
  .next_descr:
        call    add_uefi_memmap
        add     rsi, [descriptor_size]
        add     rdi, sizeof.e820entry
        dec     rcx
        cmp     rcx, 0
        jnz     .next_descr

        mov     [memory_map_size], MEMORY_MAP_SIZE
        uefi_call_wrapper BootServices, GetMemoryMap, memory_map_size, memory_map, memory_map_key, descriptor_size, descriptor_ver
        cmp     eax, EFI_SUCCESS
        jnz     .error

        uefi_call_wrapper BootServices, ExitBootServices, [efi_handler], [memory_map_key]
        cmp     eax, EFI_SUCCESS
        jnz     .error


        cli

        mov     rsi, kernel_bin_data_begin
        mov     rdi, KERNEL_BASE
        mov     rcx, (kernel_bin_data_end - kernel_bin_data_begin + 7) / 8
        rep movsq

        mov     rsi, kolibri_img_data_begin
        mov     rdi, RAMDISK_BASE
        mov     rcx, (kolibri_img_data_end - kolibri_img_data_begin + 7 ) / 8
        rep movsq

        xor     esi, esi
        mov     byte[esi + BOOT_LO.bpp], 32
        mov     word[esi + BOOT_LO.vesa_mode], 0
        mov     dword[esi + BOOT_LO.bank_switch], 0
        mov     rdi, [fb_base]
        mov     dword[esi + BOOT_LO.lfb], edi
        mov     byte[esi + BOOT_LO.mtrr], 1
        mov     byte[esi + BOOT_LO.launcher_start], 1
        mov     byte[esi + BOOT_LO.debug_print], 1
        mov     byte[esi + BOOT_LO.dma], 0
;        mov     qword[esi + BOOT_LO.pci_data], 0
        mov     dword[esi + BOOT_LO.apm_entry], 0
        mov     word[esi + BOOT_LO.apm_version], 0
        mov     dword[esi + BOOT_LO.apm_flags], 0
        mov     word[esi + BOOT_LO.apm_code_32], 0
        mov     word[esi + BOOT_LO.apm_code_16], 0
        mov     word[esi + BOOT_LO.apm_data_16], 0
        mov     byte[esi + BOOT_LO.bios_hd_cnt], 0
        mov     word[esi + BOOT_LO.bx_from_load], 'r1'   ; boot from /rd/1


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
        uefi_call_wrapper ConOut, OutputString, ConOut, msg_error
        jmp     .quit
  .quit:
        mov     rcx, -1
        loop    $


; linux/arch/x86/platform/efi/efi.c
; do_add_efi_memmap
add_uefi_memmap:
        push    rax rbx rcx rdx rsi rdi

        mov     r10d, [rsi + 0]
        mov     r11, [rsi + 8]
;        mov     r12, [rsi + 16]
        mov     r13, [rsi + 24]
        mov     r14, [rsi + 32]

        mov     [rdi + e820entry.addr], r11
        mov     rax, r13
        shl     rax, 12
        mov     [rdi + e820entry.size], rax


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
        mov     [rdi + e820entry.type], eax

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

gopuuid         db EFI_GRAPHICS_OUTPUT_PROTOCOL_UUID
gop_buffer_size dq GOP_BUFFER_SIZE
gop_handle      dq 0
gop_interface   dq 0
gop_info_size   dq 0
gop_info        dq 0

memory_map_key  dq 0
descriptor_size dq 0
descriptor_ver  dq 0
memory_map_size dq MEMORY_MAP_SIZE

msg_success     du 'Success!',13,10,0
msg_error       du 'Error!',13,10,0
msg             du 79 dup ' ',13,10,0

memory_map      rb MEMORY_MAP_SIZE
gop_buffer      rb GOP_BUFFER_SIZE


kernel_bin_data_begin:
file '../kernel.bin'
kernel_bin_data_end:
 
kolibri_img_data_begin:
file '../../../data/kolibri.img'
kolibri_img_data_end:

align 16
data fixups
end data
