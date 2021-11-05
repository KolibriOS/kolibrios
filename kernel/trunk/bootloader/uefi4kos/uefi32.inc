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

DN fix dd       ; native

include "uefi.inc"

EFIERR = 0x80000000

struct EFI_SYSTEM_TABLE
  Hdr                   EFI_TABLE_HEADER
  FirmwareVendor        DN ?
  FirmwareRevision      dd ?

  ConsoleInHandle       DN ?
  ConIn                 DN ?
  ConsoleOutHandle      DN ?
  ConOut                DN ?
  StandardErrorHandle   DN ?
  StdErr                DN ?
  RuntimeServices       DN ?
  BootServices          DN ?
  NumberOfTableEntries  DN ?
  ConfigurationTable    DN ?
ends

struct EFI_CONFIGURATION_TABLE
  VendorGUID    rd 4
  VendorTable   DN ?
ends

struct EFI_LOADED_IMAGE_PROTOCOL
  Revision              dd ?

  ParentHandle          DN ?
  SystemTable           DN ?
  DeviceHandle          DN ?
  FilePath              DN ?
  Reserved              DN ?
  LoadOptionsSize       dd ?

  ImageBase             DN ?
  ImageSize             DQ ?
  ImageCodeType         dd ?
  ImageDataType         dd ?
  UnLoad                DN ?
ends
