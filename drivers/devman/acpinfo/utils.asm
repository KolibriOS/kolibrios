format MS COFF

extrn  _acpi_rsdt_base

public _acpi_locate
public @rsdt_find@8

OS_BASE                    equ 0x80000000

ACPI_HI_RSDP_WINDOW_START  equ (OS_BASE+0x000E0000)
ACPI_HI_RSDP_WINDOW_END    equ (OS_BASE+0x00100000)
ACPI_RSDP_CHECKSUM_LENGTH  equ 20

section '.text' code readable executable align 16


_acpi_locate:
        push ebx
        mov ebx, ACPI_HI_RSDP_WINDOW_START
.check:
        cmp [ebx], dword 0x20445352
        jne .next
        cmp [ebx+4], dword 0x20525450
        jne .next

        mov edx, ebx
        mov ecx, ACPI_RSDP_CHECKSUM_LENGTH
        xor eax, eax
.sum:
        add al, [edx]
        inc edx
        loop .sum

        test al, al
        jnz .next

        mov eax, ebx
        pop ebx
        ret
.next:
        add ebx, 16
        cmp ebx, ACPI_HI_RSDP_WINDOW_END
        jb .check

        pop ebx
        xor eax, eax
        ret

align 4
@rsdt_find@8:           ;ecx= rsdt edx= SIG
        push ebx
        push esi

        lea ebx, [ecx+36]
        mov esi, [ecx+4]
        add esi, ecx
.next:
        mov eax, [ebx]
        sub eax, [_acpi_rsdt_base]
        add eax, ecx

        cmp [eax], edx
        je .done

        add ebx, 4
        cmp ebx, esi
        jb .next

        xor eax, eax
        pop esi
        pop ebx
        ret

.done:
        mov eax, [ebx]
        pop esi
        pop ebx
        ret

