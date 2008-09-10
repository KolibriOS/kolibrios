
format MS COFF

include '../macros.inc'

$Revision: 849 $

include "../const.inc"

public __start

extrn  _high_code
extrn  __os_stack
extrn  _boot_mbi
extrn  _sys_pdbr

extrn  _gdts
extrn  __edata

section '.start' code readable align 16

use32

align 4

mboot:
  dd  0x1BADB002
  dd  0x00010003
  dd  -(0x1BADB002 + 0x00010003)
  dd  mboot
  dd  0x100000
  dd  __edata; - OS_BASE
  dd  LAST_PAGE
  dd  __start

align 16
__start:
           cld

           mov esp, __os_stack +(0x100000000-OS_BASE)
           push 0
           popf

           cmp eax, 0x2BADB002
           mov ecx, sz_invboot
           jne .fault

           bt dword [ebx], 3
           mov ecx, sz_nomods
           jnc .fault

           bt dword [ebx], 6
           mov ecx, sz_nommap
           jnc .fault

           mov [_boot_mbi+(0x100000000-OS_BASE)], ebx

           xor eax, eax
           cpuid
           cmp eax, 0
           mov ecx, sz_nopse
           jbe .fault

           mov eax, 1
           cpuid
           bt edx, 3
           mov ecx, sz_nopse
           jnc .fault

; ENABLE PAGING

           mov ecx, 64
           mov edi, _sys_pdbr+(OS_BASE shr 20)+(0x100000000-OS_BASE)
           mov eax, PG_LARGE+PG_SW
@@:
           stosd
           add eax, 4*1024*1024
           loop @B

           mov dword [_sys_pdbr+(0x100000000-OS_BASE)],   PG_LARGE+PG_SW
           mov dword [_sys_pdbr+(0x100000000-OS_BASE)+4], PG_LARGE+PG_SW+4*1024*1024
           mov dword [_sys_pdbr+(0x100000000-OS_BASE)+(page_tabs shr 20)], _sys_pdbr+PG_SW+(0x100000000-OS_BASE)

           mov ebx, cr4
           or ebx, CR4_PSE
           and ebx, not CR4_PAE
           mov cr4, ebx

           mov eax, _sys_pdbr+(0x100000000-OS_BASE)
           mov ebx, cr0
           or ebx,CR0_PG+CR0_WP

           mov cr3, eax
           mov cr0, ebx

           mov ebx, [_boot_mbi+(0x100000000-OS_BASE)]

           mov edx, [ebx+20]
           mov esi, [ebx+24]
           mov ecx, LAST_PAGE
           test edx, edx
           jz .no_mods
.scan_mod:
           mov ecx, [esi+4]
           add esi, 16
           dec edx
           jnz .scan_mod

.no_mods:
           add ecx, 4095
           and ecx, not 4095

           lgdt [_gdts+(0x100000000-OS_BASE)]
           jmp pword 0x08:_high_code


.fault:
;           push ecx
;           call _lcls
;           call __bprintf
_hlt:
           hlt
           jmp _hlt

sz_invboot db 'Invalid multiboot loader magic value',0x0A
           db 'Halted',0

sz_nomods  db 'No modules loaded',0x0A
           db 'Halted',0

sz_nommap  db 'No memory table', 0x0A
           db 'Halted',0

sz_nopse   db 'Page size extensions not supported',0x0A
           db 'Halted',0
