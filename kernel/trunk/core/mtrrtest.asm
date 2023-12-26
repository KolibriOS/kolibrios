; Simple test for ring-3 debugging of mtrr.inc.
; Contains some inputs taken from real-life MTRRs and expected outputs.
format PE console
;include 'win32a.inc'
macro $Revision [args]
{
}
macro ignore_empty_revision_keyword {
  macro $Revi#sion$ \{\}
}
ignore_empty_revision_keyword
include '../proc32.inc'
include '../struct.inc'
entry start

; one test has 8, another test has 10
; this is the maximal value for storing/copying, real value is in MTRRCAP
MAX_VARIABLE_MTRR = 10

start:
; Copy test inputs, run init_pat_mtrr, compare with test outputs. Repeat.
        mov     esi, test1_in_data
        mov     edi, mtrrdata
        mov     ecx, mtrrdata_size / 4
        rep movsd
        call    init_pat_mtrr
        mov     esi, test1_out_data
        mov     edi, mtrrdata
        mov     ecx, mtrrdata_size / 4
        repz cmpsd
        jnz     .fail
        mov     esi, test2_in_data
        mov     edi, mtrrdata
        mov     ecx, mtrrdata_size / 4
        rep movsd
        call    init_pat_mtrr
        mov     esi, test2_out_data
        mov     edi, mtrrdata
        mov     ecx, mtrrdata_size / 4
        repz cmpsd
        jnz     .fail
        ret

.fail:
        int3
        jmp     $

; Helper procedure for _rdmsr/_wrmsr, replacements of rdmsr/wrmsr.
; Returns pointer to memory containing the given MSR.
; in: ecx = MSR
; out: esi -> MSR data
proc get_msr_ptr
        mov     esi, mtrrcap
        cmp     ecx, 0xFE
        jz      .ok
        mov     esi, mtrr_def_type
        cmp     ecx, 0x2FF
        jz      .ok
        lea     esi, [ecx-0x200]
        cmp     esi, MAX_VARIABLE_MTRR*2
        jae     .fail
        lea     esi, [mtrr+esi*8]
.ok:
        ret
.fail:
        int3
        ret
endp

; Emulates rdmsr.
proc _rdmsr
        push    esi
        call    get_msr_ptr
        mov     eax, [esi]
        mov     edx, [esi+4]
        pop     esi
        ret
endp

; Emulates wrmsr.
proc _wrmsr
        push    esi
        call    get_msr_ptr
        mov     [esi], eax
        mov     [esi+4], edx
        pop     esi
        ret
endp

; Macro to substitute rdmsr/wrmsr with emulating code.
macro rdmsr
{
        call    _rdmsr
}
macro wrmsr
{
        call    _wrmsr
}
; Our emulation of rdmsr/wrmsr has nothing to do with real cache
; and system-wide settings,
; remove all attempts to wbinvd and disable/enable cache in cr0.
macro wbinvd
{
}
macro mov a,b
{
if ~(a eq cr0) & ~(b eq cr0)
        mov     a, b
end if
}
macro movi r,i
{
        push    i
        pop     r
}

include '../kglobals.inc'
CAPS_MTRR  = 12
MSR_MTRR_DEF_TYPE = 0x2FF
CAPS_PGE   = 13
CAPS_PAT   = 16
MSR_CR_PAT = 0x277
PAT_VALUE  = 0x00070106 ; (UC<<24)|(UCM<<16)|(WC<<8)|WB
MEM_WB     = 6               ;write-back memory
MEM_WC     = 1               ;write combined memory
MEM_UC     = 0               ;uncached memory
include 'mtrr.inc'

BOOT_VARS = 0
BOOT.mtrr       db      1
align 4
cpu_caps        dd      1 shl CAPS_MTRR
LFBAddress      dd      0xE0000000
LFBSize         dd      0x10000000
MEM_AMOUNT      dd      0       ; not used, needed for compilation

align 4
; Test 1: input
test1_in_data:
test1_phys_addr_width   db      36
                        rb      3
test1_in_mtrrcap        dq      0xD08
test1_in_mtrr_def_type dq 0xC00
test1_in_mtrrs:
                dq      0x000000006, 0xF00000800
                dq      0x100000006, 0xFC0000800
                dq      0x0BC000000, 0xFFC000800
                dq      0x0C0000000, 0xFC0000800
                dq      0x138000000, 0xFF8000800
                dq      0, 0
                dq      0, 0
                dq      0, 0
                dq      -1, -1  ; not used
                dq      -1, -1  ; not used
; Test 1: output
test1_out_data:
                dd      36      ; phys_addr_width, readonly
                dq      0xD08   ; MTRRCAP, readonly
                dq      0xC00   ; MTRR_DEF_TYPE, should be the same
                dq      0x000000006, 0xF80000800
                dq      0x080000006, 0xFC0000800
                dq      0x0BC000000, 0xFFC000800
                dq      0x100000006, 0xFC0000800
                dq      0x138000000, 0xFF8000800
                dq      0x0E0000001, 0xFFF000800        ; added for [LFBAddress]
                dq      0, 0
                dq      0, 0
                dq      -1, -1  ; not used
                dq      -1, -1  ; not used

; Test 2: input
test2_in_data:
test2_phys_addr_width   db      39
                        rb      3
test2_in_mtrrcap        dq      0xD0A
test2_in_mtrr_def_type  dq      0xC00
test2_in_mtrrs:
                dq      0x0000000006, 0x7F00000800
                dq      0x0100000006, 0x7FE0000800
                dq      0x00E0000000, 0x7FE0000800
                dq      0x00DC000000, 0x7FFC000800
                dq      0x00DBC00000, 0x7FFFC00800
                dq      0x011F800000, 0x7FFF800800
                dq      0x011F400000, 0x7FFFC00800
                dq      0x011F200000, 0x7FFFE00800
                dq      0, 0
                dq      0, 0

; Test 2: output
test2_out_data:
                dd      39      ; phys_addr_width, readonly
                dq      0xD0A   ; MTRRCAP, readonly
                dq      0xC00   ; MTRR_DEF_TYPE, should be the same
                dq      0x0000000006, 0x7F80000800
                dq      0x0080000006, 0x7FC0000800
                dq      0x00C0000006, 0x7FE0000800
                dq      0x00DC000000, 0x7FFC000800
                dq      0x00DBC00000, 0x7FFFC00800
                dq      0x0100000006, 0x7FE0000800
                dq      0x011F800000, 0x7FFF800800
                dq      0x011F400000, 0x7FFFC00800
                dq      0x011F200000, 0x7FFFE00800
                dq      0x00E0000001, 0x7FFF000800      ; added for [LFBAddress]
IncludeIGlobals
align 4
mtrrdata:
cpu_phys_addr_width     db      ?
                rb      3
mtrrcap         dq      ?
mtrr_def_type   dq      ?
mtrr            rq      MAX_VARIABLE_MTRR*2
mtrrdata_size = $ - mtrrdata
IncludeUGlobals
