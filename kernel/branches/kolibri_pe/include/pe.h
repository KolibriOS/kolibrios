
typedef unsigned short WORD;
typedef unsigned int DWORD;
typedef unsigned int LONG;
typedef unsigned char BYTE;

#define IMAGE_DOS_SIGNATURE  0x5A4D
#define IMAGE_NT_SIGNATURE   0x00004550
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10b

#pragma pack(push,2)
typedef struct _IMAGE_DOS_HEADER
{
    WORD    e_magic;
    WORD    e_cblp;
    WORD    e_cp;
    WORD    e_crlc;
    WORD    e_cparhdr;
    WORD    e_minalloc;
    WORD    e_maxalloc;
    WORD    e_ss;
    WORD    e_sp;
    WORD    e_csum;
    WORD    e_ip;
    WORD    e_cs;
    WORD    e_lfarlc;
    WORD    e_ovno;
    WORD    e_res[4];
    WORD    e_oemid;
    WORD    e_oeminfo;
    WORD    e_res2[10];
    LONG    e_lfanew;
} IMAGE_DOS_HEADER,*PIMAGE_DOS_HEADER;
#pragma pack(pop)


#pragma pack(push,4)
typedef struct _IMAGE_FILE_HEADER
{
    WORD    Machine;
    WORD    NumberOfSections;
    DWORD   TimeDateStamp;
    DWORD   PointerToSymbolTable;
    DWORD   NumberOfSymbols;
    WORD    SizeOfOptionalHeader;
    WORD    Characteristics;
} IMAGE_FILE_HEADER, *PIMAGE_FILE_HEADER;

typedef struct _IMAGE_DATA_DIRECTORY {
    DWORD   VirtualAddress;
    DWORD   Size;
} IMAGE_DATA_DIRECTORY,*PIMAGE_DATA_DIRECTORY;

#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES    16

typedef struct _IMAGE_OPTIONAL_HEADER {
    WORD    Magic;
    BYTE    MajorLinkerVersion;
    BYTE    MinorLinkerVersion;
    DWORD   SizeOfCode;
    DWORD   SizeOfInitializedData;
    DWORD   SizeOfUninitializedData;
    DWORD   AddressOfEntryPoint;
    DWORD   BaseOfCode;
    DWORD   BaseOfData;
    DWORD   ImageBase;
    DWORD   SectionAlignment;
    DWORD   FileAlignment;
    WORD    MajorOperatingSystemVersion;
    WORD    MinorOperatingSystemVersion;
    WORD    MajorImageVersion;
    WORD    MinorImageVersion;
    WORD    MajorSubsystemVersion;
    WORD    MinorSubsystemVersion;
    DWORD   Win32VersionValue;
    DWORD   SizeOfImage;
    DWORD   SizeOfHeaders;
    DWORD   CheckSum;
    WORD    Subsystem;
    WORD    DllCharacteristics;
    DWORD   SizeOfStackReserve;
    DWORD   SizeOfStackCommit;
    DWORD   SizeOfHeapReserve;
    DWORD   SizeOfHeapCommit;
    DWORD   LoaderFlags;
    DWORD   NumberOfRvaAndSizes;
	IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} IMAGE_OPTIONAL_HEADER,*PIMAGE_OPTIONAL_HEADER;

#pragma pack(pop)


#pragma pack(push,4)
typedef struct _IMAGE_NT_HEADERS
{
    DWORD Signature;
	IMAGE_FILE_HEADER FileHeader;
	IMAGE_OPTIONAL_HEADER OptionalHeader;
} IMAGE_NT_HEADERS32,*PIMAGE_NT_HEADERS32;

#define IMAGE_SIZEOF_SHORT_NAME    8

typedef struct _IMAGE_SECTION_HEADER
{
	BYTE Name[IMAGE_SIZEOF_SHORT_NAME];
    union
    {
        DWORD PhysicalAddress;
		DWORD VirtualSize;
	} Misc;
    DWORD   VirtualAddress;
    DWORD   SizeOfRawData;
    DWORD   PointerToRawData;
    DWORD   PointerToRelocations;
    DWORD   PointerToLinenumbers;
    WORD    NumberOfRelocations;
    WORD    NumberOfLinenumbers;
    DWORD   Characteristics;
} IMAGE_SECTION_HEADER,*PIMAGE_SECTION_HEADER;
#pragma pack(pop)

#pragma pack(push,4)
typedef struct _IMAGE_BASE_RELOCATION {
	DWORD VirtualAddress;
	DWORD SizeOfBlock;
} IMAGE_BASE_RELOCATION,*PIMAGE_BASE_RELOCATION;
#pragma pack(pop)

typedef struct _IMAGE_IMPORT_DESCRIPTOR
{
    union
    {
		DWORD Characteristics;
		DWORD OriginalFirstThunk;
    };
    DWORD   TimeDateStamp;
    DWORD   ForwarderChain;
    DWORD   Name;
    DWORD   FirstThunk;
} IMAGE_IMPORT_DESCRIPTOR,*PIMAGE_IMPORT_DESCRIPTOR;

typedef struct _IMAGE_THUNK_DATA32
{
    union
    {
		DWORD ForwarderString;
		DWORD Function;
		DWORD Ordinal;
		DWORD AddressOfData;
	} u1;
} IMAGE_THUNK_DATA32,*PIMAGE_THUNK_DATA32;

typedef struct _IMAGE_IMPORT_BY_NAME
{
	WORD Hint;
	BYTE Name[1];
} IMAGE_IMPORT_BY_NAME,*PIMAGE_IMPORT_BY_NAME;

#define IMAGE_ORDINAL_FLAG 0x80000000

typedef struct _IMAGE_EXPORT_DIRECTORY {
	DWORD Characteristics;
	DWORD TimeDateStamp;
	WORD MajorVersion;
	WORD MinorVersion;
	DWORD Name;
	DWORD Base;
	DWORD NumberOfFunctions;
	DWORD NumberOfNames;
	DWORD AddressOfFunctions;
	DWORD AddressOfNames;
	DWORD AddressOfNameOrdinals;
} IMAGE_EXPORT_DIRECTORY,*PIMAGE_EXPORT_DIRECTORY;


typedef struct
{
    link_t  link;

    addr_t  img_base;
    size_t  img_size;
    char   *img_name;
    md_t   *img_md;

    PIMAGE_NT_HEADERS32      img_hdr;
    PIMAGE_SECTION_HEADER    img_sec;
    PIMAGE_EXPORT_DIRECTORY  img_exp;
    u32_t   img_map[8];                  /* mapped treads */
}dll_t;

extern dll_t core_dll;

#define MakePtr( cast, ptr, addValue ) (cast)( (addr_t)(ptr) + (addr_t)(addValue) )


dll_t * find_dll(link_t *list, const char *name);


md_t* __fastcall load_image(const char *path);

void __export create_image(addr_t img_base, addr_t image) asm ("CreateImage");


