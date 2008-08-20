
format MS COFF


include '../macros.inc'

$Revision: 849 $

__REV__ = __REV

include "../proc32.inc"
include "../kglobals.inc"
include "../lang.inc"

CR0_PE         equ    0x00000001   ;protected mode
CR0_WP         equ    0x00010000   ;write protect
CR0_PG         equ    0x80000000   ;paging

public _16bit_start
public _16bit_end

public _enter_bootscreen

public _bx_from_load

extrn __setvars

section '.boot' code readable align 16

_16bit_start:

org 0

use16

_enter_bootscreen:

           mov eax, cr0
           and eax, not 0x80000001
           mov cr0, eax
           jmp far 0x1000:start_of_code

version db    'Kolibri OS  version 0.7.1.0      ',13,10,13,10,0

include "bootstr.inc"     ; language-independent boot messages
include "preboot.inc"

if lang eq en
include "booteng.inc"     ; english system boot messages
else if lang eq ru
include "bootru.inc"      ; russian system boot messages
include "ru.inc"          ; Russian font
else if lang eq et
include "bootet.inc"      ; estonian system boot messages
include "et.inc"          ; Estonian font
else
include "bootge.inc"      ; german system boot messages
end if

include "../data16.inc"

include "bootcode.inc"    ; 16 bit system boot code
include "../bus/pci/pci16.inc"
include "../detect/biosdisk.inc"

;include "boot/shutdown.inc" ; shutdown or restart

           cli

           mov eax, cr0
           or eax, CR0_PG+CR0_WP+CR0_PE
           mov cr0, eax

           jmp pword 0x08:__setvars

align 4
_enter_16bit:
           mov eax, cr0
           and eax, not 0x80000001
           mov cr0, eax
           jmp far 0x1000:@F

align 4
_leave_16bit:

           cli
           mov eax, cr0
           or eax, CR0_PG+CR0_WP+CR0_PE
           mov cr0, eax
           hlt

align 4
@@:
           mov eax, 0x3000
           mov ss, ax
           mov esp, 0xEC00

           mov ebx, 0x1000
           mov ds, bx
           mov es, bx
           cli
           hlt

align 4
_16bit_end:
