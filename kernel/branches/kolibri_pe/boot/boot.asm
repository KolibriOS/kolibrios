
format MS COFF


include '../macros.inc'

$Revision$

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
public _poweroff

public _bx_from_load

extrn core_init

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

           cli

           mov eax, cr0
           or eax, CR0_PG+CR0_WP+CR0_PE
           mov cr0, eax

           jmp pword 0x10:core_init

align 4
rmode_idt:
           dw 0x400
           dd 0
           dw 0

align 4
_poweroff:
           mov eax, cr0
           and eax, not 0x80000001
           mov cr0, eax
           jmp far 0x1000:@F
@@:
           mov eax, 0x3000
           mov ss, ax
           mov esp, 0xEC00

           mov ebx, 0x1000
           mov ds, bx
           mov es, bx

           lidt [rmode_idt]

APM_PowerOff:
           mov     ax, 5304h
           xor     bx, bx
           int     15h
;!!!!!!!!!!!!!!!!!!!!!!!!
           mov ax,0x5300
           xor bx,bx
           int 0x15
           push ax

           mov ax,0x5301
           xor bx,bx
           int 0x15

           mov ax,0x5308
           mov bx,1
           mov cx,bx
           int 0x15

           mov ax,0x530E
           xor bx,bx
           pop cx
           int 0x15

           mov ax,0x530D
           mov bx,1
           mov cx,bx
           int 0x15

           mov ax,0x530F
           mov bx,1
           mov cx,bx
           int 0x15

           mov ax,0x5307
           mov bx,1
           mov cx,3
           int 0x15
;!!!!!!!!!!!!!!!!!!!!!!!!

           jmp $


align 4
_16bit_end:
