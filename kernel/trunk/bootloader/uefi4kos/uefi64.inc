;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;                                                              ;;
;; Copyright (C) KolibriOS team 2020-2021. All rights reserved. ;;
;; Distributed under terms of the GNU General Public License    ;;
;; Version 2, or (at your option) any later version.            ;;
;;                                                              ;;
;; Written by Ivan Baravy                                       ;;
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
