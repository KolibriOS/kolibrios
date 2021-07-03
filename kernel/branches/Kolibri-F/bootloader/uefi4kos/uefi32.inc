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

DN fix dd       ; native

include "uefi.inc"

EFIERR = 0x80000000

struct EFI_SYSTEM_TABLE
  Hdr                   EFI_TABLE_HEADER
  FirmwareVendor        dd ?
  FirmwareRevision      dd ?
  ConsoleInHandle       dd ?
  ConIn                 dd ?
  ConsoleOutHandle      dd ?
  ConOut                dd ?
  StandardErrorHandle   dd ?
  StdErr                dd ?
  RuntimeServices       dd ?
  BootServices          dd ?
  NumberOfTableEntries  dd ?
  ConfigurationTable    dd ?
ends

struct EFI_CONFIGURATION_TABLE
  VendorGUID    rd 4
  VendorTable   dd ?
ends

struct EFI_LOADED_IMAGE_PROTOCOL
  Revision              dd ?
  ParentHandle          dd ?
  SystemTable           dd ?
  DeviceHandle          dd ?
  FilePath              dd ?
  Reserved              dd ?
  LoadOptionsSize       dd ?
  ImageBase             dd ?
  ImageSize             DQ ?
  ImageCodeType         dd ?
  ImageDataType         dd ?
  UnLoad                dd ?
ends

section '.text' code executable readable

uefifunc:
        ret
