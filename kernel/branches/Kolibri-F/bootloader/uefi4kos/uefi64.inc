;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2020. All rights reserved.      ;;
;; Distributed under terms of the GNU General Public License    ;;
;; Version 2, or (at your option) any later version.            ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Based on UEFI library for fasm by bzt, Public Domain.        ;;
;;                                                              ;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

DN fix dq       ; native

include "uefi.inc"

EFIERR = 0x8000000000000000

struct EFI_SYSTEM_TABLE
  Hdr                   EFI_TABLE_HEADER
  FirmwareVendor        dq ?
  FirmwareRevision      dd ?
                        dd ?
  ConsoleInHandle       dq ?
  ConIn                 dq ?
  ConsoleOutHandle      dq ?
  ConOut                dq ?
  StandardErrorHandle   dq ?
  StdErr                dq ?
  RuntimeServices       dq ?
  BootServices          dq ?
  NumberOfTableEntries  dq ?
  ConfigurationTable    dq ?
ends

struct EFI_LOADED_IMAGE_PROTOCOL
  Revision              dd ?
                        dd ?
  ParentHandle          dq ?
  SystemTable           dq ?
  DeviceHandle          dq ?
  FilePath              dq ?
  Reserved              dq ?
  LoadOptionsSize       dd ?
                        dd ?
  ImageBase             dq ?
  ImageSize             dq ?
  ImageCodeType         dd ?
  ImageDataType         dd ?
  UnLoad                dq ?
ends

macro eficall interface,function,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11
{
numarg = 0

if ~ arg11 eq
  numarg = numarg + 1
  if ~ arg11 eq rdi
        mov     rdi, arg11
  end if
end if

if ~ arg10 eq
  numarg = numarg + 1
  if ~ arg10 eq rsi
        mov     rsi, arg10
  end if
end if

if ~ arg9 eq
  numarg = numarg + 1
  if ~ arg9 eq r14
        mov     r14, arg9
  end if
end if

if ~ arg8 eq
  numarg = numarg + 1
  if ~ arg8 eq r13
        mov     r13, arg8
  end if
end if

if ~ arg7 eq
  numarg = numarg + 1
  if ~ arg7 eq r12
        mov     r12, arg7
  end if
end if

if ~ arg6 eq
  numarg = numarg + 1
  if ~ arg6 eq r11
        mov     r11, arg6
  end if
end if

if ~ arg5 eq
  numarg = numarg + 1
  if ~ arg5 eq r10
        mov     r10, arg5
  end if
end if

if ~ arg4 eq
  numarg = numarg + 1
  if ~ arg4 eq r9
        mov     r9, arg4
  end if
end if

if ~ arg3 eq
  numarg = numarg + 1
  if ~ arg3 eq r8
        mov     r8, arg3
  end if
end if

if ~ arg2 eq
  numarg = numarg + 1
  if ~ arg2 eq rdx
        mov     rdx, arg2
  end if
end if

if ~ arg1 eq
  numarg = numarg + 1
  if ~ arg1 eq rcx
        mov     rcx, arg1
  end if
end if

        xor     eax, eax
        mov     al, numarg

if ~ interface eq rbx
        mov     rbx, interface
end if

        mov     rbx, [rbx + function]
        call    uefifunc
}

section '.text' code executable readable

uefifunc:
        ;save stack pointer
        mov     [uefi_rsptmp], rsp
        ;set up new aligned stack
        and     esp, 0xFFFFFFF0
        ;alignment check on arguments
        bt      eax, 0
        jnc     @f
        push    rax
        ;arguments
@@:
        cmp     al, 11
        jb      @f
        push    rdi
@@:
        cmp     al, 10
        jb      @f
        push    rsi
@@:
        cmp     al, 9
        jb      @f
        push    r14
@@:
        cmp     al, 8
        jb      @f
        push    r13
@@:
        cmp     al, 7
        jb      @f
        push    r12
@@:
        cmp     al, 6
        jb      @f
        push    r11
@@:
        cmp     al, 5
        jb      @f
        push    r10
@@:
        ;space for
        ;r9
        ;r8
        ;rdx
        ;rcx
        sub     rsp, 4*8
        ;call function
        call    rbx
        ;restore old stack
        mov     rsp, [uefi_rsptmp]
        ret
