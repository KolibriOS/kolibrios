include 'kglobals.inc'

PG_UNMAP            equ 0x000
PG_MAP              equ 0x001
PG_WRITE            equ 0x002
PG_SW               equ 0x003
PG_USER             equ 0x005
PG_UW               equ 0x007
PG_NOCACHE          equ 0x018
PG_LARGE            equ 0x080
PG_GLOBAL           equ 0x100

DRV_ENTRY    equ  1
DRV_EXIT     equ -1

struct  LHEAD
        next            dd ?   ;next object in list
        prev            dd ?   ;prev object in list
ends

struct  MUTEX
        lhead   LHEAD
        count   dd ?
ends

struct  PCIDEV
        bk              dd ?
        fd              dd ?
        vendor_device_id dd ?
        class           dd ?
        devfn           db ?
        bus             db ?
                        rb 2
        owner           dd ? ; pointer to SRV or 0
ends

; The following macro assume that we are on uniprocessor machine.
; Serious work is needed for multiprocessor machines.
macro spin_lock_irqsave spinlock
{
        pushf
        cli
}
macro spin_unlock_irqrestore spinlock
{
        popf
}
macro spin_lock_irq spinlock
{
        cli
}
macro spin_unlock_irq spinlock
{
        sti
}

; \begin{diamond}[29.09.2006]
; may be useful for kernel debugging
; example 1:
;       dbgstr 'Hello, World!'
; example 2:
;       dbgstr 'Hello, World!', save_flags
macro dbgstr string*, f
{
local a
iglobal_nested
a db 'K : ',string,13,10,0
endg_nested
if ~ f eq
        pushfd
end if
        push    esi
        mov     esi, a
if defined SysMsgBoardStr._pe_import
        invoke  SysMsgBoardStr
else
        call    SysMsgBoardStr
end if
        pop     esi
if ~ f eq
        popfd
end if
}
; \end{diamond}[29.09.2006]

; MOV Immediate.
; Useful for things like movi eax,10:
; shorter than regular mov, but slightly slower,
; do not use it in performance-critical places.
macro movi dst, imm
{
if imm >= -0x80 & imm <= 0x7F
        push    imm
        pop     dst
else
        mov     dst, imm
end if
}

macro call name
{
if name eqtype func & defined name#._pe_import
err Use invoke, not call/stdcall for PE imports!
end if
        call    name
}
