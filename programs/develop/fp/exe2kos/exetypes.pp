unit EXETypes;

interface

const
  IMAGE_DOS_SIGNATURE    = $5A4D;       { MZ }
  IMAGE_OS2_SIGNATURE    = $454E;       { NE }
  IMAGE_OS2_SIGNATURE_LE = $454C;       { LE }
  IMAGE_VXD_SIGNATURE    = $454C;       { LE }
  IMAGE_NT_SIGNATURE     = $00004550;   { PE00 }

  IMAGE_SIZEOF_SHORT_NAME          = 8;
  IMAGE_SIZEOF_SECTION_HEADER      = 40;
  IMAGE_NUMBEROF_DIRECTORY_ENTRIES = 16;
  IMAGE_RESOURCE_NAME_IS_STRING    = $80000000;
  IMAGE_RESOURCE_DATA_IS_DIRECTORY = $80000000;
  IMAGE_OFFSET_STRIP_HIGH          = $7FFFFFFF;

type
  PIMAGE_DOS_HEADER = ^IMAGE_DOS_HEADER;
  IMAGE_DOS_HEADER = packed record      { DOS .EXE header }
    e_magic         : WORD;             { Magic number }
    e_cblp          : WORD;             { Bytes on last page of file }
    e_cp            : WORD;             { Pages in file }
    e_crlc          : WORD;             { Relocations }
    e_cparhdr       : WORD;             { Size of header in paragraphs }
    e_minalloc      : WORD;             { Minimum extra paragraphs needed }
    e_maxalloc      : WORD;             { Maximum extra paragraphs needed }
    e_ss            : WORD;             { Initial (relative) SS value }
    e_sp            : WORD;             { Initial SP value }
    e_csum          : WORD;             { Checksum }
    e_ip            : WORD;             { Initial IP value }
    e_cs            : WORD;             { Initial (relative) CS value }
    e_lfarlc        : WORD;             { File address of relocation table }
    e_ovno          : WORD;             { Overlay number }
    e_res           : packed array [0..3] of WORD; { Reserved words }
    e_oemid         : WORD;             { OEM identifier (for e_oeminfo) }
    e_oeminfo       : WORD;             { OEM information; e_oemid specific }
    e_res2          : packed array [0..9] of WORD; { Reserved words }
    e_lfanew        : Longint;          { File address of new exe header }
  end;

  PIMAGE_FILE_HEADER = ^IMAGE_FILE_HEADER;
  IMAGE_FILE_HEADER = packed record
    Machine              : WORD;
    NumberOfSections     : WORD;
    TimeDateStamp        : DWORD;
    PointerToSymbolTable : DWORD;
    NumberOfSymbols      : DWORD;
    SizeOfOptionalHeader : WORD;
    Characteristics      : WORD;
  end;

  PIMAGE_DATA_DIRECTORY = ^IMAGE_DATA_DIRECTORY;
  IMAGE_DATA_DIRECTORY = packed record
    VirtualAddress  : DWORD;
    Size            : DWORD;
  end;

  PIMAGE_OPTIONAL_HEADER = ^IMAGE_OPTIONAL_HEADER;
  IMAGE_OPTIONAL_HEADER = packed record
   { Standard fields. }
    Magic           : WORD;
    MajorLinkerVersion : Byte;
    MinorLinkerVersion : Byte;
    SizeOfCode      : DWORD;
    SizeOfInitializedData : DWORD;
    SizeOfUninitializedData : DWORD;
    AddressOfEntryPoint : DWORD;
    BaseOfCode      : DWORD;
    BaseOfData      : DWORD;
   { NT additional fields. }
    ImageBase       : DWORD;
    SectionAlignment : DWORD;
    FileAlignment   : DWORD;
    MajorOperatingSystemVersion : WORD;
    MinorOperatingSystemVersion : WORD;
    MajorImageVersion : WORD;
    MinorImageVersion : WORD;
    MajorSubsystemVersion : WORD;
    MinorSubsystemVersion : WORD;
    Reserved1       : DWORD;
    SizeOfImage     : DWORD;
    SizeOfHeaders   : DWORD;
    CheckSum        : DWORD;
    Subsystem       : WORD;
    DllCharacteristics : WORD;
    SizeOfStackReserve : DWORD;
    SizeOfStackCommit : DWORD;
    SizeOfHeapReserve : DWORD;
    SizeOfHeapCommit : DWORD;
    LoaderFlags     : DWORD;
    NumberOfRvaAndSizes : DWORD;
    DataDirectory   : packed array [0..IMAGE_NUMBEROF_DIRECTORY_ENTRIES-1] of IMAGE_DATA_DIRECTORY;
  end;

  PIMAGE_SECTION_HEADER = ^IMAGE_SECTION_HEADER;
  IMAGE_SECTION_HEADER = packed record
    Name            : packed array [0..IMAGE_SIZEOF_SHORT_NAME-1] of Char;
    PhysicalAddress : DWORD; // or VirtualSize (union);
    VirtualAddress  : DWORD;
    SizeOfRawData   : DWORD;
    PointerToRawData : DWORD;
    PointerToRelocations : DWORD;
    PointerToLinenumbers : DWORD;
    NumberOfRelocations : WORD;
    NumberOfLinenumbers : WORD;
    Characteristics : DWORD;
  end;

  PIMAGE_NT_HEADERS = ^IMAGE_NT_HEADERS;
  IMAGE_NT_HEADERS = packed record
    Signature       : DWORD;
    FileHeader      : IMAGE_FILE_HEADER;
    OptionalHeader  : IMAGE_OPTIONAL_HEADER;
  end;

{ Resources }

  PIMAGE_RESOURCE_DIRECTORY = ^IMAGE_RESOURCE_DIRECTORY;
  IMAGE_RESOURCE_DIRECTORY = packed record
    Characteristics : DWORD;
    TimeDateStamp   : DWORD;
    MajorVersion    : WORD;
    MinorVersion    : WORD;
    NumberOfNamedEntries : WORD;
    NumberOfIdEntries : WORD;
  end;

  PIMAGE_RESOURCE_DIRECTORY_ENTRY = ^IMAGE_RESOURCE_DIRECTORY_ENTRY;
  IMAGE_RESOURCE_DIRECTORY_ENTRY = packed record
    Name: DWORD;        // Or ID: Word (Union)
    OffsetToData: DWORD;
  end;

  PIMAGE_RESOURCE_DATA_ENTRY = ^IMAGE_RESOURCE_DATA_ENTRY;
  IMAGE_RESOURCE_DATA_ENTRY = packed record
    OffsetToData    : DWORD;
    Size            : DWORD;
    CodePage        : DWORD;
    Reserved        : DWORD;
  end;

  PIMAGE_RESOURCE_DIR_STRING_U = ^IMAGE_RESOURCE_DIR_STRING_U;
  IMAGE_RESOURCE_DIR_STRING_U = packed record
    Length          : WORD;
    NameString      : array [0..0] of WCHAR;
  end;

{
    /* Predefined resource types */
    #define    RT_NEWRESOURCE      0x2000
    #define    RT_ERROR            0x7fff
    #define    RT_CURSOR           1
    #define    RT_BITMAP           2
    #define    RT_ICON             3
    #define    RT_MENU             4
    #define    RT_DIALOG           5
    #define    RT_STRING           6
    #define    RT_FONTDIR          7
    #define    RT_FONT             8
    #define    RT_ACCELERATORS     9
    #define    RT_RCDATA           10
    #define    RT_MESSAGETABLE     11
    #define    RT_GROUP_CURSOR     12
    #define    RT_GROUP_ICON       14
    #define    RT_VERSION          16
    #define    RT_NEWBITMAP        (RT_BITMAP|RT_NEWRESOURCE)
    #define    RT_NEWMENU          (RT_MENU|RT_NEWRESOURCE)
    #define    RT_NEWDIALOG        (RT_DIALOG|RT_NEWRESOURCE)

}

type
  TResourceType = (
    rtUnknown0,
    rtCursorEntry,
    rtBitmap,
    rtIconEntry,
    rtMenu,
    rtDialog,
    rtString,
    rtFontDir,
    rtFont,
    rtAccelerators,
    rtRCData,
    rtMessageTable,
    rtCursor,
    rtUnknown13,
    rtIcon,
    rtUnknown15,
    rtVersion);

{ Resource Type Constants }

const
  StringsPerBlock = 16;

{ Resource Related Structures from RESFMT.TXT in WIN32 SDK }

type

  PIconHeader = ^TIconHeader;
  TIconHeader = packed record
    wReserved: Word;         { Currently zero }
    wType: Word;             { 1 for icons }
    wCount: Word;            { Number of components }
  end;

  PIconResInfo = ^TIconResInfo;
  TIconResInfo = packed record
    bWidth: Byte;
    bHeight: Byte;
    bColorCount: Byte;
    bReserved: Byte;
    wPlanes: Word;
    wBitCount: Word;
    lBytesInRes: DWORD;
    wNameOrdinal: Word;      { Points to component }
  end;

  PCursorResInfo = ^TCursorResInfo;
  TCursorResInfo = packed record
    wWidth: Word;
    wHeight: Word;
    wPlanes: Word;
    wBitCount: Word;
    lBytesInRes: DWORD;
    wNameOrdinal: Word;      { Points to component }
  end;


implementation

end.
