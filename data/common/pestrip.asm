; If you know macro language of FASM, there is almost nothing to comment here.
; If you don't know macro language of FASM, comments would not help you.

filename equ '%EXENAME%'

SPE_DIR_ORDER fix IMPORT EXPORT BASERELOC EXCEPTION TLS BOUND_IMPORT RESOURCE
count = 0
irps dir,SPE_DIR_ORDER
{
SPE_DIRECTORY_#dir = count
count = count + 1
}

IMAGE_DIRECTORY_ENTRY_EXPORT = 0
IMAGE_DIRECTORY_ENTRY_IMPORT = 1
IMAGE_DIRECTORY_ENTRY_RESOURCE = 2
IMAGE_DIRECTORY_ENTRY_EXCEPTION = 3
IMAGE_DIRECTORY_ENTRY_BASERELOC = 5
IMAGE_DIRECTORY_ENTRY_TLS = 9
IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT = 11

virtual at 0
file filename:3Ch,4
load pehea dword from 0
end virtual

virtual at 0
file filename:pehea,0F8h
load NumberOfSections           word  from 6
load TimeDateStamp              dword from 8
load SizeOfOptionalHeader       word  from 14h
if SizeOfOptionalHeader<>0E0h
error Nonstandard PE header
end if
load Characteristics            word  from 16h
load AddressOfEntryPoint        dword from 28h
load ImageBase                  dword from 34h
load SectionAlignment           dword from 38h
load FileAlignment              dword from 3Ch
load MajorOperatingSystemVersion word from 40h
load MinorOperatingSystemVersion word from 42h
load MajorSubsystemVersion      word  from 48h
load MinorSubsystemVersion      word  from 4Ah
load SizeOfImage                dword from 50h
load SizeOfHeaders              dword from 54h
load Subsystem                  word  from 5Ch
load SizeOfStackReserve         dword from 60h
load SizeOfHeapReserve          dword from 68h
load SrcNumberOfRvaAndSizes     dword from 74h

DstNumberOfRvaAndSizes = 0
irps dir,SPE_DIR_ORDER
{
if IMAGE_DIRECTORY_ENTRY_#dir < SrcNumberOfRvaAndSizes
load DirRVA_#dir dword from 78h + 8*IMAGE_DIRECTORY_ENTRY_#dir
load DirSize_#dir dword from 7Ch + 8*IMAGE_DIRECTORY_ENTRY_#dir
else
DirRVA_#dir = 0
DirSize_#dir = 0
end if
if DirRVA_#dir > 0 & DirSize_#dir > 0
DstNumberOfRvaAndSizes = SPE_DIRECTORY_#dir + 1
end if
}

end virtual

SectionAlignmentLog = 0
while SectionAlignment <> 1 shl SectionAlignmentLog
SectionAlignmentLog = SectionAlignmentLog + 1
end while
FileAlignmentLog = 0
while FileAlignment <> 1 shl FileAlignmentLog
FileAlignmentLog = FileAlignmentLog + 1
end while

; header
        dw      'PE' xor 'S'    ; Signature
        dw      Characteristics or 0x100        ; IMAGE_FILE_32BIT_MACHINE
        dd      AddressOfEntryPoint
        dd      ImageBase
        db      SectionAlignmentLog
        db      FileAlignmentLog
        db      MajorSubsystemVersion
        db      MinorSubsystemVersion
        dd      SizeOfImage
        dd      SizeOfStackReserve
        dd      SizeOfHeapReserve
SizeOfHeadersField:
        dd      0
        db      Subsystem
        db      DstNumberOfRvaAndSizes
        dw      NumberOfSections
; directories
irps dir,SPE_DIR_ORDER
{
if SPE_DIRECTORY_#dir < DstNumberOfRvaAndSizes
        dd      DirRVA_#dir, DirSize_#dir
end if
}

NumBytesDeleted = pehea + 0F8h - $ + NumberOfSections*0Ch
DeltaDeleted = NumBytesDeleted and not (FileAlignment - 1)
; Use store instead of declaring SizeOfHeaders - DeltaDeleted directly in dd
; to avoid the second compilation pass.
store dword SizeOfHeaders - DeltaDeleted at SizeOfHeadersField
TimeStampInExportTable = 0
; sections
repeat NumberOfSections
file filename:pehea+0F8h+(%-1)*28h,18h
load VirtualSize dword from $-10h
load VirtualAddress dword from $-0Ch
load SizeOfRawData dword from $-8
load PointerToRawData dword from $-4
PointerToRawData = PointerToRawData - DeltaDeleted
store dword PointerToRawData at $-4
if DirRVA_EXPORT <> 0 & DirRVA_EXPORT+4 >= VirtualAddress & DirRVA_EXPORT+8 <= VirtualAddress + VirtualSize & DirRVA_EXPORT+8 <= VirtualAddress + SizeOfRawData
TimeStampInExportTable = DirRVA_EXPORT+4 - VirtualAddress + PointerToRawData
end if
file filename:pehea+0F8h+(%-1)*28h+24h,4
end repeat
; padding to keep FileAlignment
times NumBytesDeleted - DeltaDeleted db 0
; data
file filename:pehea+0F8h+NumberOfSections*28h
if TimeDateStamp <> 0 & TimeStampInExportTable <> 0
store dword TimeDateStamp at TimeStampInExportTable
end if
