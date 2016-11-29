struct STRIPPED_PE_HEADER
  Signature             dw ?
  Characteristics       dw ?
  AddressOfEntryPoint   dd ?
  ImageBase             dd ?
  SectionAlignmentLog   db ?
  FileAlignmentLog      db ?
  MajorOSVersion        db ?
  MinorOSVersion        db ?
  SizeOfImage           dd ?
  SizeOfStackReserve    dd ?
  SizeOfHeapReserve     dd ?
  SizeOfHeaders         dd ?
  Subsystem             db ?
  NumberOfRvaAndSizes   db ?
  NumberOfSections      dw ?
ends
STRIPPED_PE_SIGNATURE = 0x4503 ; 'PE' xor 'S'
SPE_DIRECTORY_IMPORT    = 0
SPE_DIRECTORY_EXPORT    = 1
SPE_DIRECTORY_BASERELOC = 2
SPE_DIRECTORY_EXCEPTION = 3
SPE_DIRECTORY_TLS       = 4
SPE_DIRECTORY_BOUND_IMPORT = 5

struct IMAGE_DATA_DIRECTORY
  VirtualAddress dd  ?
  isize          dd  ?
ends

struct IMAGE_OPTIONAL_HEADER32
  Magic                         dw  ?
  MajorLinkerVersion            db  ?
  MinorLinkerVersion            db  ?
  SizeOfCode                    dd  ?
  SizeOfInitializedData         dd  ?
  SizeOfUninitializedData       dd  ?
  AddressOfEntryPoint           dd  ?
  BaseOfCode                    dd  ?
  BaseOfData                    dd  ?
  ImageBase                     dd  ?
  SectionAlignment              dd  ?
  FileAlignment                 dd  ?
  MajorOperatingSystemVersion   dw  ?
  MinorOperatingSystemVersion   dw  ?
  MajorImageVersion             dw  ?
  MinorImageVersion             dw  ?
  MajorSubsystemVersion         dw  ?
  MinorSubsystemVersion         dw  ?
  Win32VersionValue             dd  ?
  SizeOfImage                   dd  ?
  SizeOfHeaders                 dd  ?
  CheckSum                      dd  ?
  Subsystem                     dw  ?
  DllCharacteristics            dw  ?
  SizeOfStackReserve            dd  ?
  SizeOfStackCommit             dd  ?
  SizeOfHeapReserve             dd  ?
  SizeOfHeapCommit              dd  ?
  LoaderFlags                   dd  ?
  NumberOfDirectories           dd  ?
  DataDirectory                 IMAGE_DATA_DIRECTORY ?
  Directories                   rb  sizeof.IMAGE_DATA_DIRECTORY*15
ends
IMAGE_DIRECTORY_ENTRY_EXPORT = 0
IMAGE_DIRECTORY_ENTRY_IMPORT = 1
IMAGE_DIRECTORY_ENTRY_BASERELOC = 5
IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT = 11

IMAGE_SUBSYSTEM_UNKNOWN = 0
IMAGE_SUBSYSTEM_NATIVE = 1
IMAGE_SUBSYSTEM_WINDOWS_GUI = 2
IMAGE_SUBSYSTEM_WINDOWS_CUI = 3

struct IMAGE_FILE_HEADER
  Machine               dw ?
  NumberOfSections      dw ?
  TimeDateStamp         dd ?
  PointerToSymbolTable  dd ?
  NumberOfSymbols       dd ?
  SizeOfOptionalHeader  dw ?
  Characteristics       dw ?
ends
IMAGE_FILE_RELOCS_STRIPPED = 1
IMAGE_FILE_DLL = 0x2000

struct IMAGE_NT_HEADERS
  Signature         dd ?
  FileHeader        IMAGE_FILE_HEADER
  OptionalHeader    IMAGE_OPTIONAL_HEADER32
ends

struct IMAGE_EXPORT_DIRECTORY
  Characteristics       dd  ?
  TimeDateStamp         dd  ?
  MajorVersion          dw  ?
  MinorVersion          dw  ?
  Name                  dd  ?
  Base                  dd  ?
  NumberOfFunctions     dd  ?
  NumberOfNames         dd  ?
  AddressOfFunctions    dd  ?
  AddressOfNames        dd  ?
  AddressOfNameOrdinals dd  ?
ends

struct IMAGE_IMPORT_DESCRIPTOR
  OriginalFirstThunk    dd ?
  TimeDateStamp         dd ?
  ForwarderChain        dd ?
  Name                  dd ?
  FirstThunk            dd ?
ends

struct IMAGE_IMPORT_BY_NAME
  Hint                  dw ?
  Name                  rb 0
ends

struct IMAGE_BASE_RELOCATION
  VirtualAddress        dd ?
  SizeOfBlock           dd ?
ends
IMAGE_REL_BASED_ABSOLUTE = 0
IMAGE_REL_BASED_HIGHLOW = 3

struct IMAGE_DOS_HEADER
  e_magic               dw ?
  e_cblp                dw ?
  e_cp                  dw ?
  e_crlc                dw ?
  e_cparhdr             dw ?
  e_minalloc            dw ?
  e_maxalloc            dw ?
  e_ss                  dw ?
  e_sp                  dw ?
  e_csum                dw ?
  e_ip                  dw ?
  e_cs                  dw ?
  e_lfarlc              dw ?
  e_ovno                dw ?
  e_res                 rw 4
  e_oemid               dw ?
  e_oeminfo             dw ?
  e_res2                rw 10
  e_lfanew              dd ?
ends

struct IMAGE_SECTION_HEADER
  Name                  rb 8
  VirtualSize           dd ?
  VirtualAddress        dd ?
  SizeOfRawData         dd ?
  OffsetToRawData       dd ?
  OffsetToRelocations   dd ?
  OffsetToLinenumbers   dd ?
  NumberOfRelocations   dw ?
  NumberOfLinenumbers   dw ?
  Characteristics       dd ?
ends

struct IMAGE_BOUND_IMPORT_DESCRIPTOR
  TimeDateStamp         dd ?
  OffsetModuleName      dw ?
  NumberOfModuleForwarderRefs dw ?
ends
