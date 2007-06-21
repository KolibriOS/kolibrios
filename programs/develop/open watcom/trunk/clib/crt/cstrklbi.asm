
        name    cstrklbri
.386p
        assume  nothing

        extrn   __KolibriMain   : near
        extrn   ___begtext      : near
        extrn   __STACKTOP      : near
        extrn   __STACKLOW      : near
        extrn   __FiniRtns      : near
        extrn   _LpCmdLine      : near
        extrn   _LpPgmName      : near

_TEXT   segment use32 dword public 'CODE'

        public  _cstart_
        public  mainCRTStartup
        public __exit_

        assume  cs:_TEXT

_cstart_ proc near
mainCRTStartup:
          
        mov edx, 0x400
        mov eax, 0xff
        out dx, al
         
        mov   eax, esp
        sub   eax, 8192
        mov   dword ptr [__STACKTOP],esp        ; set stack top
        mov   dword ptr [__STACKLOW],eax 
        mov   eax, dword ptr [ds:0x001c]
        mov   ebx, dword ptr [ds:0x0020]
        mov   dword ptr [_LpCmdLine], eax
        mov   dword ptr [_LpPgmName], ebx            
        jmp   __KolibriMain
        dd    ___begtext        ; reference module with segment definitions
;
; copyright message
;
        db      "Open Watcom C/C++32 Run-Time system. "
        db      "Portions Copyright (c) Sybase, Inc. 1988-2002."
_cstart_ endp


__exit_  proc near
        mov     eax,00h                 ; run finalizers
        mov     edx,0fh                 ; less than exit
        call    __FiniRtns              ; call finalizer routines
        mov eax, -1
        int 0x40
        ret
__exit_ endp

public _scalbn
_scalbn proc
        fild  dword ptr [esp+12]
        fld   qword ptr [esp+4]
        fscale
        fstp  st[1]
        ret
_scalbn endp
 

_TEXT   ends



        end     _cstart_  ;programm entry point
