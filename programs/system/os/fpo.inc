; Magic for proc32.inc: prologue/epilogue for esp-based addressing
; fpo stands from Frame Pointer Omission
macro fpo_prologue procname,flag,parmbytes,localbytes,reglist
{
local loc,regcount
    loc = (localbytes+3) and (not 3)
    regcount = 0
    irps reg, reglist
    \{
        regcount = regcount+1
        push    reg
    \}
    if loc
        sub     esp, loc
    end if
    parmbase@proc equ esp+4+loc+regcount*4
    localbase@proc equ esp
    fpo_localsize = loc
    fpo_delta = 0
}
macro fpo_epilogue procname,flag,parmbytes,localbytes,reglist
{
    if fpo_localsize
        add     esp, fpo_localsize
    end if
    irps reg, reglist \{ reverse pop reg \}
    if flag and 10000b
        retn
    else
        retn    parmbytes
    end if
}
macro deflocal@proc name,def,[val]
{
   common
    deflocal@proc name#_unique_suffix,def,val
    all@vars equ all@vars,name
    name equ name#_unique_suffix+fpo_delta
}
macro defargs@proc [arg]
{
   common
    rawargs equ
    srcargs equ arg
   forward
    rawargs equ rawargs,arg#_unique_suffix
   common
    match =,tmp,rawargs \{
    defargs@proc tmp
    uniqargs equ args@proc
    restore args@proc
    args@proc equ uniqargs,srcargs
    \}
   forward
    arg equ arg#_unique_suffix+fpo_delta
}
macro stdcall proc,[arg]                ; directly call STDCALL procedure
{  common
        fpo_delta_base = fpo_delta
    if ~ arg eq
   reverse
        pushd   arg
        fpo_delta = fpo_delta + 4
   common
    end if
        call    proc
        fpo_delta = fpo_delta_base
}
macro ccall proc,[arg]                  ; directly call CDECL procedure
{  common
        fpo_delta_base = fpo_delta
        size@ccall = 0
    if ~ arg eq
   reverse
        pushd   arg
        fpo_delta = fpo_delta + 4
        size@ccall = size@ccall+4
   common
    end if
        call    proc
    if size@ccall
        add     esp, size@ccall
    end if
        fpo_delta = fpo_delta_base
}
fpo_delta = 0
