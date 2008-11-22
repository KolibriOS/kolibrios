
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2004-2007. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

include "../macros.inc"
include "../proc32.inc"
include "../const.inc"

$Revision: 847 $

sel_tss         equ  0x08

sel_os_code     equ  0x10
sel_os_stack    equ  0x18

sel_app_code    equ  0x23
sel_app_data    equ  0x2B

sel_srv_code    equ  0x31
sel_srv_stack   equ  0x39

sel_code_16     equ  0x70

format MS COFF

use32

public __os_stack
public _pg_balloc


public high_code

public core_init

public  test_cpu

public cpu_vendor
public cpu_sign
public cpu_info
public cpu_caps

extrn _parse_mbi

extrn _16bit_start
extrn _16bit_end
extrn _enter_bootscreen

extrn init_fpu
extrn init_idt
extrn _init_mm
extrn _slab_cache_init
extrn @init_heap@8
extrn init_malloc
extrn _init_core_dll
extrn _init_threads
extrn init_mtrr
extrn system_init

extrn sysenter_entry
extrn syscall_entry


extrn  @create_systhread@4

extrn _sys_pdbr
extrn _current_thread
extrn _k_reenter:dword

extrn scr_mode:dword
extrn LFBAddress:dword
extrn LFBSize:dword

section '.text' code readable align 16

high_code:

           mov ax, sel_os_stack
           mov dx, sel_app_data
           mov ss, ax
           mov esp, __os_stack

           mov ds, dx
           mov es, dx
           mov fs, dx
           mov gs, dx


         ;  bt [cpu_caps], CAPS_PGE
         ;  jnc @F

         ;  or dword [sys_pgdir-OS_BASE+(OS_BASE shr 20)], PG_GLOBAL

         ;  mov ebx, cr4
         ;  or ebx, CR4_PGE
         ;  mov cr4, ebx
@@:
         ;  mov eax, cr3
         ;  mov cr3, eax           ; flush TLB

           mov edx, 0x3fB
           mov eax, 3
           out dx, al

           call test_cpu
           call _parse_mbi

        ;   mov eax, [_pg_balloc]
        ;   mov [_copy_pg_balloc], eax

__core_restart:

           mov esi, _16bit_start
           mov ecx, _16bit_end
           shr ecx, 2
           mov edi, _16BIT_BASE
           cld
           rep movsd

           jmp far sel_code_16:_enter_bootscreen;

align 16
core_init:
           cld

           mov ax, sel_os_stack
           mov dx, sel_app_data
           mov ss, ax
           mov esp, __os_stack

           mov ds, dx
           mov es, dx
           mov fs, dx
           mov gs, dx

           mov [tss._ss0], sel_os_stack
           mov [tss._esp0], __os_stack
           mov [tss._esp], __os_stack
           mov [tss._cs], sel_os_code
           mov [tss._ss], sel_os_stack
           mov [tss._ds], sel_app_data
           mov [tss._es], sel_app_data
           mov [tss._fs], sel_app_data
           mov [tss._gs], sel_app_data
           mov [tss._io], 128
;Add IO access table - bit array of permitted ports
           mov edi, tss._io_map_0
           xor eax, eax
       ;    not eax
           mov ecx, 8192/4
           rep stosd             ; access to 4096*8=65536 ports

           mov ax, sel_tss
           ltr ax

; -------- Fast System Call init ----------
; Intel SYSENTER/SYSEXIT (AMD CPU support it too)
	   bt [cpu_caps], CAPS_SEP
	   jnc .SEnP   ; SysEnter not Present

	   xor edx, edx
	   mov ecx, MSR_SYSENTER_CS
       mov eax, sel_os_code
	   wrmsr
	   mov ecx, MSR_SYSENTER_ESP
;           mov eax, sysenter_stack ; Check it
	   xor	   eax, eax
	   wrmsr
	   mov ecx, MSR_SYSENTER_EIP
	   mov eax, sysenter_entry
	   wrmsr

.SEnP:
; AMD SYSCALL/SYSRET
	   cmp byte[cpu_vendor], 'A'
	   jne .noSYSCALL
	   mov eax, 0x80000001
	   cpuid
	   test edx, 0x800  ; bit_11 - SYSCALL/SYSRET support
	   jz .noSYSCALL
	   mov ecx, MSR_AMD_EFER
	   rdmsr
	   or eax, 1   ; bit_0 - System Call Extension (SCE)
	   wrmsr

; Bits of EDX :
; Bit 31Ц16 During the SYSRET instruction, this field is copied into the CS register
;  and the contents of this field, plus 8, are copied into the SS register.
; Bit 15Ц0 During the SYSCALL instruction, this field is copied into the CS register
;  and the contents of this field, plus 8, are copied into the SS register.

       mov   edx, ((sel_os_code + 16) shl 16) + sel_os_code

	   mov eax, syscall_entry
	   mov ecx, MSR_AMD_STAR
	   wrmsr
.noSYSCALL:

           call init_fpu

           call init_idt

           call _init_mm

           call init_malloc
           call _slab_cache_init

           mov ecx, 0x80000000
           mov edx, 0x40000000
           call @init_heap@8

           call _init_core_dll
           call _init_threads

; SAVE & CLEAR 0-0xffff

           cld
           xor esi, esi
           mov   edi,BOOT_VAR
           mov   ecx,0x10000 / 4
           rep   movsd

           xor edi, edi
           xor eax, eax
           mov   ecx,0x10000 / 4
           rep   stosd

           mov edi, 0x40000
           mov ecx, (0x90000-0x40000)/4
           rep stosd

           mov dword [_sys_pdbr], eax
           mov dword [_sys_pdbr+4], eax

           xchg bx, bx

           movzx eax,word [BOOT_VAR+0x9008]  ; screen mode
           mov   [scr_mode],eax

           mov eax,[BOOT_VAR+0x9018]
           call map_LFB

           mov eax, cr3
           mov cr3, eax


           jmp  system_init

if 0
           mov ecx, system_init
           call @create_systhread@4

           mov [_current_thread], eax

           xchg bx, bx

           mov ebx, [eax+THR.pdir]
           mov ecx, cr3
           cmp ebx, ecx
           je .skip
           mov cr3, ebx
.skip:
           mov esp, [_current_thread]
        ;   lea eax, [esp+THR.pl0_stack]
        ;   mov [tss._esp0], eax
restart1:
           dec [_k_reenter]
           popad
           add esp, 4                 ; skip return adr
           iretd                      ; continue process
end if

align 4
map_LFB:
           cmp eax, -1
           jne @f

           ret
@@:
           test [scr_mode], 0100000000000000b
           jnz @f
           mov [BOOT_VAR+0x901c],byte 2
           ret
@@:
           mov [LFBAddress], eax
           mov [LFBSize], 0x800000
           call init_mtrr

           mov eax, [LFBAddress]
           or eax, PG_LARGE+PG_UW
           mov [_sys_pdbr+(LFB_BASE shr 20)], eax
           add eax, 0x00400000
           mov [_sys_pdbr+4+(LFB_BASE shr 20)], eax
if SHADOWFB
           mov ecx, 11
           call @core_alloc@4
           or eax, PG_LARGE+PG_UW
           mov [_sys_pdbr+(SHADOWFB shr 20)], eax
           add eax, 0x00400000
           mov [_sys_pdbr+4+(SHADOWFB shr 20)], eax
end if

           bt [cpu_caps], CAPS_PGE
           jnc @F
           or dword [_sys_pdbr+(LFB_BASE shr 20)], PG_GLOBAL
@@:
           mov dword [LFBAddress], LFB_BASE
           ret


align 4
proc test_cpu
           locals
              cpu_type   dd ?
              cpu_id     dd ?
              cpu_Intel  dd ?
              cpu_AMD    dd ?
           endl

           mov [cpu_type], 0
           xor eax, eax
           mov [cpu_caps], eax
           mov [cpu_caps+4], eax

           xor eax, eax
           cpuid

           mov [cpu_vendor], ebx
           mov [cpu_vendor+4], edx
           mov [cpu_vendor+8], ecx
           cmp ebx, dword [intel_str]
           jne .check_AMD
           cmp edx, dword [intel_str+4]
           jne .check_AMD
           cmp ecx, dword [intel_str+8]
           jne .check_AMD
           mov [cpu_Intel], 1
           cmp eax, 1
           jl .end_cpuid
           mov eax, 1
           cpuid
           mov [cpu_sign], eax
           mov [cpu_info],  ebx
           mov [cpu_caps],  edx
           mov [cpu_caps+4],ecx

           shr eax, 8
           and eax, 0x0f
           ret
.end_cpuid:
           mov eax, [cpu_type]
           ret

.check_AMD:
           cmp ebx, dword [AMD_str]
           jne .unknown
           cmp edx, dword [AMD_str+4]
           jne .unknown
           cmp ecx, dword [AMD_str+8]
           jne .unknown
           mov [cpu_AMD], 1
           cmp eax, 1
           jl .unknown
           mov eax, 1
           cpuid
           mov [cpu_sign], eax
           mov [cpu_info],  ebx
           mov [cpu_caps],  edx
           mov [cpu_caps+4],ecx
           shr eax, 8
           and eax, 0x0f
           ret
.unknown:
           mov eax, 1
           cpuid
           mov [cpu_sign], eax
           mov [cpu_info],  ebx
           mov [cpu_caps],  edx
           mov [cpu_caps+4],ecx
           shr eax, 8
           and eax, 0x0f
           ret
endp

intel_str  db "GenuineIntel",0
AMD_str    db "AuthenticAMD",0



if 0
align 4

init_BIOS32:
           mov edi, 0xE0000
.pcibios_nxt:
           cmp dword[edi], '_32_' ; "magic" word
           je .BIOS32_found
.pcibios_nxt2:
           add edi, 0x10
           cmp edi, 0xFFFF0
           je .BIOS32_not_found
           jmp .pcibios_nxt
.BIOS32_found:			; magic word found, check control summ

           movzx ecx, byte[edi + 9]
           shl ecx, 4
           mov esi, edi
           xor eax, eax
           cld   ; paranoia
@@:	lodsb
           add ah, al
           loop @b
           jnz .pcibios_nxt2 ; control summ must be zero
    ; BIOS32 service found !
           mov ebp, [edi + 4]
           mov [bios32_entry], ebp
    ; check PCI BIOS present
           mov eax, '$PCI'
           xor ebx, ebx
           push cs  ; special for 'ret far' from  BIOS
           call ebp
           test al, al
           jnz .PCI_BIOS32_not_found

 ; здесь создаютс€ дискрипторы дл€ PCI BIOS

           add ebx, OS_BASE
           dec ecx
           mov [(pci_code_32-OS_BASE)], cx    ;limit 0-15
           mov [(pci_data_32-OS_BASE)], cx    ;limit 0-15

           mov [(pci_code_32-OS_BASE)+2], bx  ;base  0-15
           mov [(pci_data_32-OS_BASE)+2], bx  ;base  0-15

           shr ebx, 16
           mov [(pci_code_32-OS_BASE)+4], bl  ;base  16-23
           mov [(pci_data_32-OS_BASE)+4], bl  ;base  16-23

           shr ecx, 16
           and cl, 0x0F
           mov ch, bh
           add cx, D32
           mov [(pci_code_32-OS_BASE)+6], cx  ;lim   16-19 &
           mov [(pci_data_32-OS_BASE)+6], cx  ;base  24-31

           mov [(pci_bios_entry-OS_BASE)], edx
         ; jmp .end
.PCI_BIOS32_not_found:
	; здесь должна заполн€тс€ pci_emu_dat
.BIOS32_not_found:
.end:
           ret

end if

section '.data' data writeable align 16

_pg_balloc        dd LAST_PAGE

section '.bss' data writeable align 16

                  rb 8192-512

__os_stack        rb 512

;CPUID information

cpu_vendor        rd 3
cpu_sign          rd 1
cpu_info          rd 1
cpu_caps          rd 4



