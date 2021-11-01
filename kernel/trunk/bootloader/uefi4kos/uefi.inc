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

EFI_LOCATE_SEARCH_TYPE:
.AllHandles       = 0
.ByRegisterNotify = 1
.ByProtocol       = 2

; EFI_MEMORY_TYPE
EFI_RESERVED_MEMORY_TYPE        = 0
EFI_LOADER_CODE                 = 1
EFI_LOADER_DATA                 = 2
EFI_BOOT_SERVICES_CODE          = 3
EFI_BOOT_SERVICES_DATA          = 4
EFI_RUNTIME_SERVICES_CODE       = 5
EFI_RUNTIME_SERVICES_DATA       = 6
EFI_CONVENTIONAL_MEMORY         = 7
EFI_UNUSABLE_MEMORY             = 8
EFI_ACPI_RECLAIM_MEMORY         = 9
EFI_ACPI_MEMORY_NVS             = 10
EFI_MEMORY_MAPPED_IO            = 11
EFI_MEMORY_MAPPED_IO_PORT_SPACE = 12
EFI_PAL_CODE                    = 13
EFI_PERSISTENT_MEMORY           = 14
EFI_MAX_MEMORY_TYPE             = 15

; EFI_ALLOCATE_TYPE
EFI_ALLOCATE_ANY_PAGES   = 0
EFI_ALLOCATE_MAX_ADDRESS = 1
EFI_ALLOCATE_ADDRESS     = 2

EFI_MEMORY_UC                   = 0x00000001
EFI_MEMORY_WC                   = 0x00000002
EFI_MEMORY_WT                   = 0x00000004
EFI_MEMORY_WB                   = 0x00000008
EFI_MEMORY_UCE                  = 0x00000010
EFI_MEMORY_WP                   = 0x00001000
EFI_MEMORY_RP                   = 0x00002000
EFI_MEMORY_XP                   = 0x00004000
EFI_MEMORY_NV                   = 0x00008000
EFI_MEMORY_MORE_RELIABLE        = 0x00010000
EFI_MEMORY_RO                   = 0x00020000

EFI_SUCCESS                     = 0
EFI_LOAD_ERROR                  = EFIERR or 1
EFI_INVALID_PARAMETER           = EFIERR or 2
EFI_UNSUPPORTED                 = EFIERR or 3
EFI_BAD_BUFFER_SIZE             = EFIERR or 4
EFI_BUFFER_TOO_SMALL            = EFIERR or 5
EFI_NOT_READY                   = EFIERR or 6
EFI_DEVICE_ERROR                = EFIERR or 7
EFI_WRITE_PROTECTED             = EFIERR or 8
EFI_OUT_OF_RESOURCES            = EFIERR or 9
EFI_VOLUME_CORRUPTED            = EFIERR or 10
EFI_VOLUME_FULL                 = EFIERR or 11
EFI_NO_MEDIA                    = EFIERR or 12
EFI_MEDIA_CHANGED               = EFIERR or 13
EFI_NOT_FOUND                   = EFIERR or 14
EFI_ACCESS_DENIED               = EFIERR or 15
EFI_NO_RESPONSE                 = EFIERR or 16
EFI_NO_MAPPING                  = EFIERR or 17
EFI_TIMEOUT                     = EFIERR or 18
EFI_NOT_STARTED                 = EFIERR or 19
EFI_ALREADY_STARTED             = EFIERR or 20
EFI_ABORTED                     = EFIERR or 21
EFI_ICMP_ERROR                  = EFIERR or 22
EFI_TFTP_ERROR                  = EFIERR or 23
EFI_PROTOCOL_ERROR              = EFIERR or 24


EFI_FILE_SYSTEM_INFO_ID equ 0x93,0x6e,0x57,0x09,0x3f,0x6d,0xd2,0x11, \
                            0x39,0x8e,0x00,0xa0,0xc9,0x69,0x72,0x3b

EFI_SYSTEM_TABLE_SIGNATURE equ 0x49,0x42,0x49,0x20,0x53,0x59,0x53,0x54

EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID equ 0x22,0x5b,0x4e,0x96, \
                                         0x59,0x64,0xd2,0x11, \
                                         0x8e,0x39,0x00,0xa0, \
                                         0xc9,0x69,0x72,0x3b

EFI_LOADED_IMAGE_PROTOCOL_GUID equ 0xA1,0x31,0x1b,0x5b,0x62,0x95,0xd2,0x11, \
                                    0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B

EFI_BLOCK_IO_PROTOCOL_GUID equ 0x21,0x5b,0x4e,0x96,0x59,0x64,0xd2,0x11, \
                               0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b

EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID equ 0xde,0xa9,0x42,0x90,0xdc,0x23,0x38,0x4a, \
                                      0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a

EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL_GUID equ 0xBB, 0x7E, 0x70, 0x2F, \
                                         0x1A, 0x4A, 0xd4, 0x11, \
                                         0x9A, 0x38, 0x00, 0x90, \
                                         0x27, 0x3F, 0xC1, 0x4D

EFI_FILE_MODE_READ   = 1
EFI_FILE_MODE_WRITE  = 2
EFI_FILE_MODE_CREATE = 0x8000000000000000

struct EFI_MEMORY_DESCRIPTOR
  Type          dd ?
                dd ?    ; align
  PhysicalStart DQ ?
  VirtualStart  DQ ?
  NumberOfPages DQ ?
  Attribute     DQ ?
ends

struct EFI_FILE_SYSTEM_INFO
  Size          DQ ?
  ReadOnly      db ?
                rb 7
  VolumeSize    DQ ?
  FreeSpace     DQ ?
  BlockSize     dd ?
  VolumeLabel   rw 32
ends

struct EFI_TABLE_HEADER
  Signature             DQ ?
  Revision              dd ?
  HeaderSize            dd ?
  CRC32                 dd ?
  Reserved              dd ?
ends

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL
  Reset                 DN ?
  OutputString          DN ?
  TestString            DN ?
  QueryMode             DN ?
  SetMode               DN ?
  SetAttribute          DN ?
  ClearScreen           DN ?
  SetCursorPosition     DN ?
  EnableCursor          DN ?
  Mode                  DN ?
ends


struct SIMPLE_INPUT_INTERFACE
  Reset                 DN ?
  ReadKeyStroke         DN ?
  WaitForKey            DN ?
ends

struct EFI_BOOT_SERVICES
  Hdr                        EFI_TABLE_HEADER
  RaisePriority              DN ?
  RestorePriority            DN ?
  AllocatePages              DN ?
  FreePages                  DN ?
  GetMemoryMap               DN ?
  AllocatePool               DN ?
  FreePool                   DN ?
  CreateEvent                DN ?
  SetTimer                   DN ?
  WaitForEvent               DN ?
  SignalEvent                DN ?
  CloseEvent                 DN ?
  CheckEvent                 DN ?
  InstallProtocolInterface   DN ?
  ReInstallProtocolInterface DN ?
  UnInstallProtocolInterface DN ?
  HandleProtocol             DN ?
  Reserved                   DN ?
  RegisterProtocolNotify     DN ?
  LocateHandle               DN ?
  LocateDevicePath           DN ?
  InstallConfigurationTable  DN ?
  ImageLoad                  DN ?
  ImageStart                 DN ?
  Exit                       DN ?
  ImageUnLoad                DN ?
  ExitBootServices           DN ?
  GetNextMonotonicCount      DN ?
  Stall                      DN ?
  SetWatchdogTimer           DN ?
  ConnectController          DN ?
  DisConnectController       DN ?
  OpenProtocol               DN ?
  CloseProtocol              DN ?
  OpenProtocolInformation    DN ?
  ProtocolsPerHandle         DN ?
  LocateHandleBuffer         DN ?
  LocateProtocol             DN ?
  InstallMultipleProtocolInterfaces DN ?
  UnInstallMultipleProtocolInterfaces DN ?
  CalculateCrc32             DN ?
  CopyMem                    DN ?
  SetMem                     DN ?
ends

struct EFI_RUNTIME_SERVICES
  Hdr                   EFI_TABLE_HEADER
  GetTime               DN ?
  SetTime               DN ?
  GetWakeUpTime         DN ?
  SetWakeUpTime         DN ?
  SetVirtualAddressMap  DN ?
  ConvertPointer        DN ?
  GetVariable           DN ?
  GetNextVariableName   DN ?
  SetVariable           DN ?
  GetNextHighMonoCount  DN ?
  ResetSystem           DN ?
ends

struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL
  Revision              DQ ?
  OpenVolume            DN ?
ends

struct EFI_FILE_PROTOCOL
  Revision              DQ ?
  Open                  DN ?
  Close                 DN ?
  Delete                DN ?
  Read                  DN ?
  Write                 DN ?
  GetPosition           DN ?
  SetPosition           DN ?
  GetInfo               DN ?
  SetInfo               DN ?
  Flush                 DN ?
  OpenEx                DN ?
  ReadEx                DN ?
  WriteEx               DN ?
  FlushEx               DN ?
ends

struct EFI_GRAPHICS_OUTPUT_PROTOCOL
  QueryMode             DN ?
  SetMode               DN ?
  Blt                   DN ?
  Mode                  DN ?
ends

struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
  MaxMode               dd ?
  Mode                  dd ?
  Info                  DN ?
  SizeOfInfo            DN ?
  FrameBufferBase       DQ ?
  FrameBufferSize       DN ?
ends

EFI_GRAPHICS_PIXEL_FORMAT:
.PixelRedGreenBlueReserved8BitPerColor = 0
.PixelBlueGreenRedReserved8BitPerColor = 1
.PixelBitMask = 2
.PixelBltOnly = 3
.PixelFormatMax = 4

struct EFI_PIXEL_BITMASK
  RedMask               dd ?
  GreenMask             dd ?
  BlueMask              dd ?
  ReservedMask          dd ?
ends

struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION
  Version               dd ?
  HorizontalResolution  dd ?
  VerticalResolution    dd ?
  PixelFormat           dd ?
  PixelInformation      EFI_PIXEL_BITMASK
  PixelsPerScanLine     dd ?
ends

struct PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS
  Read  DN ?
  Write DN ?
ends

struct EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL
  ParentHandle          DN ? ; EFI_HANDLE
  PollMem               DN ?
  PollIo                DN ?
  Mem                   PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS
  Io                    PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS
  Pci                   PCI_ROOT_BRIDGE_IO_PROTOCOL_ACCESS
  CopyMem               DN ?
  Map                   DN ?
  Unmap                 DN ?
  AllocateBuffer        DN ?
  FreeBuffer            DN ?
  Flush                 DN ?
  GetAttributes         DN ?
  SetAttributes         DN ?
  Configuration         DN ?
  SegmentNumber         dd ?
ends

struct EFI_QWORD_ADDRESS_SPACE_DESCRIPTOR       ; described in acpi spec
  Type              db ?
  Length            dw ?
  ResourceType      db ?
  GeneralFlags      db ?
  SpecificFlags     db ?
  Granularity       dq ?
  RangeMinimum      dq ?
  RangeMaximum      dq ?
  TranslationOffset dq ?
  AddressLength dq ?
ends

EFI_RESOURCE_DESCRIPTOR_TYPE.QWORD_ADDRESS_SPACE = 0x8a
EFI_RESOURCE_DESCRIPTOR_TYPE.END_TAG = 0x79

EFI_RESOURCE_TYPE.MEMORY = 0
EFI_RESOURCE_TYPE.IO = 1
EFI_RESOURCE_TYPE.BUS = 2
