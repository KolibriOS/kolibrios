;*********************************************************************
;*                                                                   *
;*          UEFI library for fasm by bzt, Public Domain              *
;*                                                                   *
;*********************************************************************
 
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

EFI_MEMORY_UC                   = 0x0000000000000001
EFI_MEMORY_WC                   = 0x0000000000000002
EFI_MEMORY_WT                   = 0x0000000000000004
EFI_MEMORY_WB                   = 0x0000000000000008
EFI_MEMORY_UCE                  = 0x0000000000000010
EFI_MEMORY_WP                   = 0x0000000000001000
EFI_MEMORY_RP                   = 0x0000000000002000
EFI_MEMORY_XP                   = 0x0000000000004000
EFI_MEMORY_NV                   = 0x0000000000008000
EFI_MEMORY_MORE_RELIABLE        = 0x0000000000010000
EFI_MEMORY_RO                   = 0x0000000000020000
EFI_MEMORY_RUNTIME              = 0x8000000000000000

EFIERR                          = 0x8000000000000000
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
 
EFI_SYSTEM_TABLE_SIGNATURE      equ     0x49,0x42,0x49,0x20,0x53,0x59,0x53,0x54
struct EFI_TABLE_HEADER
  Signature             dq ?
  Revision              dd ?
  HeaderSize            dd ?
  CRC32                 dd ?
  Reserved              dd ?
ends
 
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
 
struct SIMPLE_TEXT_OUTPUT_INTERFACE
  Reset                 dq ?
  OutputString          dq ?
  TestString            dq ?
  QueryMode             dq ?
  SetMode               dq ?
  SetAttribute          dq ?
  ClearScreen           dq ?
  SetCursorPosition     dq ?
  EnableCursor          dq ?
  Mode                  dq ?
ends
 
 
struct SIMPLE_INPUT_INTERFACE
  Reset                 dq ?
  ReadKeyStroke         dq ?
  WaitForKey            dq ?
ends
 
struct EFI_BOOT_SERVICES_TABLE
  Hdr                   EFI_TABLE_HEADER
  RaisePriority         dq ?
  RestorePriority       dq ?
  AllocatePages         dq ?
  FreePages             dq ?
  GetMemoryMap          dq ?
  AllocatePool          dq ?
  FreePool              dq ?
  CreateEvent           dq ?
  SetTimer              dq ?
  WaitForEvent          dq ?
  SignalEvent           dq ?
  CloseEvent            dq ?
  CheckEvent            dq ?
  InstallProtocolInterface dq ?
  ReInstallProtocolInterface dq ?
  UnInstallProtocolInterface dq ?
  HandleProtocol        dq ?
  Void                  dq ?
  RegisterProtocolNotify dq ?
  LocateHandle          dq ?
  LocateDevicePath      dq ?
  InstallConfigurationTable dq ?
  ImageLoad             dq ?
  ImageStart            dq ?
  Exit                  dq ?
  ImageUnLoad           dq ?
  ExitBootServices      dq ?
  GetNextMonotonicCount dq ?
  Stall                 dq ?
  SetWatchdogTimer      dq ?
  ConnectController     dq ?
  DisConnectController  dq ?
  OpenProtocol          dq ?
  CloseProtocol         dq ?
  OpenProtocolInformation dq ?
  ProtocolsPerHandle    dq ?
  LocateHandleBuffer    dq ?
  LocateProtocol        dq ?
  InstallMultipleProtocolInterfaces dq ?
  UnInstallMultipleProtocolInterfaces dq ?
  CalculateCrc32        dq ?
  CopyMem               dq ?
  SetMem                dq ?
ends
 
struct EFI_RUNTIME_SERVICES_TABLE
  Hdr                   EFI_TABLE_HEADER
  GetTime               dq ?
  SetTime               dq ?
  GetWakeUpTime         dq ?
  SetWakeUpTime         dq ?
  SetVirtualAddressMap  dq ?
  ConvertPointer        dq ?
  GetVariable           dq ?
  GetNextVariableName   dq ?
  SetVariable           dq ?
  GetNextHighMonoCount  dq ?
  ResetSystem           dq ?
ends
 
struct EFI_TIME
  Year                  dw ?
  Month                 db ?
  Day                   db ?
  Hour                  db ?
  Minute                db ?
  Second                db ?
  Pad1                  db ?
  Nanosecond            dd ?
  TimeZone              dw ?
  Daylight              db ?
  Pad2                  db ?
  sizeof                db ?
ends
 
EFI_LOADED_IMAGE_PROTOCOL_UUID  equ     0xA1,0x31,0x1b,0x5b,0x62,0x95,0xd2,0x11,0x8E,0x3F,0x00,0xA0,0xC9,0x69,0x72,0x3B
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
 
EFI_BLOCK_IO_PROTOCOL_UUID      equ     0x21,0x5b,0x4e,0x96,0x59,0x64,0xd2,0x11,0x8e,0x39,0x00,0xa0,0xc9,0x69,0x72,0x3b
struct EFI_BLOCK_IO_PROTOCOL
  Revision              dq ?
  Media                 dq ?
  Reset                 dq ?
  ReadBlocks            dq ?
  WriteBlocks           dq ?
  FlushBlocks           dq ?
ends
 
struct EFI_BLOCK_IO_MEDIA
  MediaId               dd ?
  RemovableMedia        db ?
  MediaPresent          db ?
  LogicalPartition      db ?
  ReadOnly              db ?
  WriteCaching          db ?
                        rb 3
  BlockSize             dd ?
  IoAlign               dd ?
  LastBlock             dq ?
ends
 
EFI_GRAPHICS_OUTPUT_PROTOCOL_UUID equ   0xde,0xa9,0x42,0x90,0xdc,0x23,0x38,0x4a,0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a
struct EFI_GRAPHICS_OUTPUT_PROTOCOL
  QueryMode             dq ?
  SetMode               dq ?
  Blt                   dq ?
  Mode                  dq ?
ends
 
struct EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE
  MaxMode               dd ?
  Mode                  dd ?
  nfo                   dq ?
  SizeOfInfo            dq ?
  FrameBufferBase       dq ?
  FrameBufferSize       dq ?
ends
 
struct EFI_GRAPHICS_OUTPUT_MODE_INFORMATION
  Version               dd ?
  HorizontalResolution  dd ?
  VerticalResolution    dd ?
  PixelFormat           dd ?
  RedMask               dd ?
  GreenMask             dd ?
  BlueMask              dd ?
  Reserved              dd ?
  PixelsPerScanLine     dd ?
ends

;---macros to make life easier---
;call it early, after entry point is the best
macro InitializeLib
{
        clc
        test    rdx, rdx
        jz      .badout
        cmp     dword[rdx], 'IBI '      ; 20494249h
        jz      @f
  .badout:
        xor     ecx, ecx
        xor     edx, edx
        stc
    @@:
        mov     [efi_handler], rcx      ; ImageHandle
        mov     [efi_ptr], rdx          ; pointer to SystemTable
}
 
;invoke an UEFI function
macro uefi_call_wrapper interface,function,arg1,arg2,arg3,arg4,arg5,arg6,arg7,arg8,arg9,arg10,arg11
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
    if ~ arg1 in <ConsoleInHandle,ConIn,ConsoleOutHandle,ConOut,StandardErrorHandle,StdErr,RuntimeServices,BootServices>
        mov     rcx, arg1
    end if
  end if
end if

        xor     rax, rax
        mov     al, numarg

if interface in <ConsoleInHandle,ConIn,ConsoleOutHandle,ConOut,StandardErrorHandle,StdErr,RuntimeServices,BootServices>
        mov     rbx, [efi_ptr]
        mov     rbx, [rbx + EFI_SYSTEM_TABLE.#interface]
else
  if ~ interface eq rbx
        mov     rbx, interface
  end if
end if

if arg1 in <ConsoleInHandle,ConIn,ConsoleOutHandle,ConOut,StandardErrorHandle,StdErr,RuntimeServices,BootServices>
        mov     rcx, rbx
end if

if defined SIMPLE_INPUT_INTERFACE.#function
        mov     rbx, [rbx + SIMPLE_INPUT_INTERFACE.#function]
else
 if defined SIMPLE_TEXT_OUTPUT_INTERFACE.#function
        mov     rbx, [rbx + SIMPLE_TEXT_OUTPUT_INTERFACE.#function]
 else
  if defined EFI_BOOT_SERVICES_TABLE.#function
        mov     rbx, [rbx + EFI_BOOT_SERVICES_TABLE.#function]
  else
   if defined EFI_RUNTIME_SERVICES_TABLE.#function
        mov     rbx, [rbx + EFI_RUNTIME_SERVICES_TABLE.#function]
   else
    if defined EFI_GRAPHICS_OUTPUT_PROTOCOL.#function
        mov     rbx, [rbx + EFI_GRAPHICS_OUTPUT_PROTOCOL.#function]
    else
     if defined EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.#function
        mov     rbx, [rbx + EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE.#function]
     else
        mov     rbx, [rbx + function]
     end if
    end if
   end if
  end if
 end if
end if
        call    uefifunc
}
 
;*********************************************************************
;*                       Library functions                           *
;*********************************************************************
 
section '.text' code executable readable
 
uefifunc:
        ;save stack pointer
        mov     qword [uefi_rsptmp], rsp
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
        mov     rsp, qword [uefi_rsptmp]
        ret
 
section '.data' data readable writeable
efi_handler     dq 0
efi_ptr         dq 0
uefi_rsptmp     dq 0
