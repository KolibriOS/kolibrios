/****************************   coff.h   *************************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2008-06-04
* Project:       objconv
* Module:        coff.h
* Description:
* Header file for definition of structures in MS Windows COFF Intel x86 (PE)
* object file format.
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
* Parts (c) 1995 DJ Delorie GNU General Public License
*****************************************************************************/

/*****************************************************************************
* Note: The COFF data structures do not fit the default alignment of modern
* compilers. All structures should be compiled without any alignment padding.
* The specification of structure packing is not standardized among compilers.
* You may remove or replace the #pragma pack directives if you make sure that
* you never use the sizeof() operator or pointer arithmetics on any of the 
* structures that need packing. See coff.cpp for examples.
*****************************************************************************/

#ifndef PECOFF_H
#define PECOFF_H

/********************** FILE HEADER **********************/

struct SCOFF_FileHeader {
 uint16 Machine;              // Machine ID (magic number)
 uint16 NumberOfSections;     // number of sections
 uint32 TimeDateStamp;        // time & date stamp 
 uint32 PSymbolTable;         // file pointer to symbol table
 uint32 NumberOfSymbols;      // number of symbol table entries 
 uint16 SizeOfOptionalHeader; // size of optional header
 uint16 Flags;                // Flags indicating attributes
};

// Values of Machine:
#define PE_MACHINE_I386       0x14c
#define PE_MACHINE_X8664     0x8664

// Bits for Flags:
#define PE_F_RELFLG 0x0001   // relocation info stripped from file
#define PE_F_EXEC   0x0002   // file is executable (no unresolved external references)
#define PE_F_LNNO   0x0004   // line numbers stripped from file
#define PE_F_LSYMS  0x0008   // local symbols stripped from file


// Structure used in optional header
struct SCOFF_IMAGE_DATA_DIRECTORY {
   uint32 VirtualAddress;              // Image relative address of table
   uint32 Size;                        // Size of table
};

// Extended structure used internally with virtual address translated to section:offset
struct SCOFF_ImageDirAddress : public SCOFF_IMAGE_DATA_DIRECTORY {
   int32  Section;                     // Section containing table
   uint32 SectionOffset;               // Offset relative to section
   uint32 FileOffset;                  // Offset relative to file
   uint32 MaxOffset;                   // Section size - SectionOffset
   const char * Name;                  // Name of table
};

// Optional header
union SCOFF_OptionalHeader {
   // 32 bit version
   struct {
      uint16 Magic;                    // Magic number
      uint8  LinkerMajorVersion;
      uint8  LinkerMinorVersion;
      uint32 SizeOfCode;
      uint32 SizeOfInitializedData;
      uint32 SizeOfUninitializedData;
      uint32 AddressOfEntryPoint;      // Entry point relative to image base
      uint32 BaseOfCode;
      uint32 BaseOfData;
      // Windows specific fields
      int32  ImageBase;                // Image base
      uint32 SectionAlignment;
      uint32 FileAlignment;
      uint16 MajorOperatingSystemVersion;
      uint16 MinorOperatingSystemVersion;
      uint16 MajorImageVersion;
      uint16 MinorImageVersion;
      uint16 MajorSubsystemVersion;
      uint16 MinorSubsystemVersion;
      uint32 Win32VersionValue;        // must be 0
      uint32 SizeOfImage;
      uint32 SizeOfHeaders;
      uint32 CheckSum;
      uint16 Subsystem;
      uint16 DllCharacteristics;
      uint32 SizeOfStackReserve;
      uint32 SizeOfStackCommit;
      uint32 SizeOfHeapReserve;
      uint32 SizeOfHeapCommit;
      uint32 LoaderFlags;              // 0
      uint32 NumberOfRvaAndSizes;
      // Data directories
      SCOFF_IMAGE_DATA_DIRECTORY ExportTable;
      SCOFF_IMAGE_DATA_DIRECTORY ImportTable;
      SCOFF_IMAGE_DATA_DIRECTORY ResourceTable;
      SCOFF_IMAGE_DATA_DIRECTORY ExceptionTable;
      SCOFF_IMAGE_DATA_DIRECTORY CertificateTable;
      SCOFF_IMAGE_DATA_DIRECTORY BaseRelocationTable;
      SCOFF_IMAGE_DATA_DIRECTORY Debug;
      SCOFF_IMAGE_DATA_DIRECTORY Architecture;   // 0
      SCOFF_IMAGE_DATA_DIRECTORY GlobalPtr;      // 0
      SCOFF_IMAGE_DATA_DIRECTORY TLSTable;
      SCOFF_IMAGE_DATA_DIRECTORY LoadConfigTable;
      SCOFF_IMAGE_DATA_DIRECTORY BoundImportTable;
      SCOFF_IMAGE_DATA_DIRECTORY ImportAddressTable;
      SCOFF_IMAGE_DATA_DIRECTORY DelayImportDescriptor;
      SCOFF_IMAGE_DATA_DIRECTORY CLRRuntimeHeader;
      SCOFF_IMAGE_DATA_DIRECTORY Reserved;       // 0
   } h32;
   // 64 bit version
   struct {
      uint16 Magic;                    // Magic number
      uint8  LinkerMajorVersion;
      uint8  LinkerMinorVersion;
      uint32 SizeOfCode;
      uint32 SizeOfInitializedData;
      uint32 SizeOfUninitializedData;
      uint32 AddressOfEntryPoint;      // Entry point relative to image base
      uint32 BaseOfCode;
      // Windows specific fields
      int64  ImageBase;                // Image base
      uint32 SectionAlignment;
      uint32 FileAlignment;
      uint16 MajorOperatingSystemVersion;
      uint16 MinorOperatingSystemVersion;
      uint16 MajorImageVersion;
      uint16 MinorImageVersion;
      uint16 MajorSubsystemVersion;
      uint16 MinorSubsystemVersion;
      uint32 Win32VersionValue;        // must be 0
      uint32 SizeOfImage;
      uint32 SizeOfHeaders;
      uint32 CheckSum;
      uint16 Subsystem;
      uint16 DllCharacteristics;
      uint64 SizeOfStackReserve;
      uint64 SizeOfStackCommit;
      uint64 SizeOfHeapReserve;
      uint64 SizeOfHeapCommit;
      uint32 LoaderFlags;              // 0
      uint32 NumberOfRvaAndSizes;
      // Data directories
      SCOFF_IMAGE_DATA_DIRECTORY ExportTable;
      SCOFF_IMAGE_DATA_DIRECTORY ImportTable;
      SCOFF_IMAGE_DATA_DIRECTORY ResourceTable;
      SCOFF_IMAGE_DATA_DIRECTORY ExceptionTable;
      SCOFF_IMAGE_DATA_DIRECTORY CertificateTable;
      SCOFF_IMAGE_DATA_DIRECTORY BaseRelocationTable;
      SCOFF_IMAGE_DATA_DIRECTORY Debug;
      SCOFF_IMAGE_DATA_DIRECTORY Architecture;   // 0
      SCOFF_IMAGE_DATA_DIRECTORY GlobalPtr;      // 0
      SCOFF_IMAGE_DATA_DIRECTORY TLSTable;
      SCOFF_IMAGE_DATA_DIRECTORY LoadConfigTable;
      SCOFF_IMAGE_DATA_DIRECTORY BoundImportTable;
      SCOFF_IMAGE_DATA_DIRECTORY ImportAddressTable;
      SCOFF_IMAGE_DATA_DIRECTORY DelayImportDescriptor;
      SCOFF_IMAGE_DATA_DIRECTORY CLRRuntimeHeader;
      SCOFF_IMAGE_DATA_DIRECTORY Reserved;       // 0
   } h64;
};

// Value of Magic for optional header
#define COFF_Magic_PE32  0x10B
#define COFF_Magic_PE64  0x20B

// Export directory table
struct SCOFF_ExportDirectory {
   uint32 Flags;
   uint32 DateTime;
   uint16 VersionMajor;
   uint16 VersionMinor;
   uint32 DLLNameRVA;                  // Image-relative address of DLL name
   uint32 OrdinalBase;                 // Ordinal number of first export
   uint32 AddressTableEntries;         // Number of entries in export address table
   uint32 NamePointerEntries;          // Number of entries in name pointer table
   uint32 ExportAddressTableRVA;       // Image-relative address of export address table
   uint32 NamePointerTableRVA;         // Image-relative address of export name pointer table
   uint32 OrdinalTableRVA;             // Image-relative address of ordinal table
};

// Import directory table
struct SCOFF_ImportDirectory {
   uint32 ImportLookupTableRVA;        // Image-relative address of import lookup table
   uint32 DateTime;
   uint32 ForwarderChain;
   uint32 DLLNameRVA;                  // Image-relative address of DLL name string
   uint32 ImportAddressTableRVA;       // Image-relative address of import address table
};

// Import hint/name table entry
struct SCOFF_ImportHintName {
   uint16 Hint;                        // Index into export name pointer table
   char   Name[2];                     // Variable length
};

// Base relocation block header
struct SCOFF_BaseRelocationBlock {
   uint32 PageRVA;                     // Image-relative base to add to offset
   uint32 BlockSize;                   // Size of SCOFF_BaseRelocationBlock plus all SCOFF_BaseRelocation
};

// Base relocation block entry
struct SCOFF_BaseRelocation {
   uint16 Offset:12;                   // Offset relative to PageRVA
   uint16 Type:4;                      // Base relocation type
};

// Base relocation types
#define  COFF_REL_BASED_ABSOLUTE   0   // Ignore
#define  COFF_REL_BASED_HIGH       1   // High 16 bits
#define  COFF_REL_BASED_LOW        2   // Low 16 bits
#define  COFF_REL_BASED_HIGHLOW    3   // 32 bits
#define  COFF_REL_BASED_HIGHADJ    4   // Two consecutive records: 16 bits high, 16 bits low
#define  COFF_REL_BASED_DIR64     10   // 64 bits


/********************** SECTION HEADER **********************/

struct SCOFF_SectionHeader {
 char    Name[8];        // section name
 uint32  VirtualSize;    // size of section when loaded. (Should be 0 for object files, but it seems to be accumulated size of all sections)
 uint32  VirtualAddress; // subtracted from offsets during relocation. preferably 0
 uint32  SizeOfRawData;  // section size in file
 uint32  PRawData;       // file  to raw data for section
 uint32  PRelocations;   // file  to relocation entries
 uint32  PLineNumbers;   // file  to line number entries
 uint16  NRelocations;   // number of relocation entries
 uint16  NLineNumbers;   // number of line number entries
 uint32  Flags;          // flags   
};

// Section flags values
#define PE_SCN_CNT_CODE         0x00000020  // section contains executable code
#define PE_SCN_CNT_INIT_DATA    0x00000040  // section contains initialized data
#define PE_SCN_CNT_UNINIT_DATA  0x00000080  // section contains unintialized data
#define PE_SCN_LNK_INFO         0x00000200  // section contains comments or .drectve
#define PE_SCN_LNK_REMOVE       0x00000800  // will not be part of the image. object files only
#define PE_SCN_LNK_COMDAT       0x00001000  // section contains communal data
#define PE_SCN_ALIGN_1          0x00100000  // Align data by 1
#define PE_SCN_ALIGN_2          0x00200000  // Align data by 2
#define PE_SCN_ALIGN_4          0x00300000  // Align data by 4
#define PE_SCN_ALIGN_8          0x00400000  // Align data by 8
#define PE_SCN_ALIGN_16         0x00500000  // Align data by 16
#define PE_SCN_ALIGN_32         0x00600000  // Align data by 32
#define PE_SCN_ALIGN_64         0x00700000  // Align data by 64
#define PE_SCN_ALIGN_128        0x00800000  // Align data by 128
#define PE_SCN_ALIGN_256        0x00900000  // Align data by 256
#define PE_SCN_ALIGN_512        0x00a00000  // Align data by 512
#define PE_SCN_ALIGN_1024       0x00b00000  // Align data by 1024
#define PE_SCN_ALIGN_2048       0x00c00000  // Align data by 2048
#define PE_SCN_ALIGN_4096       0x00d00000  // Align data by 4096
#define PE_SCN_ALIGN_8192       0x00e00000  // Align data by 8192
#define PE_SCN_ALIGN_MASK       0x00f00000  // Mask for extracting alignment info
#define PE_SCN_LNK_NRELOC_OVFL  0x01000000  // section contains extended relocations
#define PE_SCN_MEM_DISCARDABLE  0x02000000  // section is discardable
#define PE_SCN_MEM_NOT_CACHED   0x04000000  // section cannot be cached
#define PE_SCN_MEM_NOT_PAGED    0x08000000  // section is not pageable
#define PE_SCN_MEM_SHARED       0x10000000  // section can be shared
#define PE_SCN_MEM_EXECUTE      0x20000000  // section is executable
#define PE_SCN_MEM_READ         0x40000000  // section is readable
#define PE_SCN_MEM_WRITE        0x80000000  // section is writeable

/* names of "special" sections 
#define _TEXT ".text"
#define _DATA ".data"
#define _BSS ".bss"
#define _COMMENT ".comment"
#define _LIB ".lib"  */

/********************** LINE NUMBERS **********************/

/* 1 line number entry for every "breakpointable" source line in a section.
 * Line numbers are grouped on a per function basis; first entry in a function
 * grouping will have l_lnno = 0 and in place of physical address will be the
 * symbol table index of the function name.
 */
//#pragma pack(push, 1)
struct SCOFF_LineNumbers {
 union {
  uint32 Fname;    // function name symbol table index, if Line == 0
  uint32 Addr;     // section-relative address of code that corresponds to line
 };
 uint16 Line;      // line number
};

// Warning: Size does not fit standard alignment!
// Use SIZE_SCOFF_LineNumbers instead of sizeof(SCOFF_LineNumbers)
#define SIZE_SCOFF_LineNumbers  6  // Size of SCOFF_LineNumbers packed

//#pragma pack(pop)


/******** Symbol table entry and auxiliary Symbol table entry ********/
//#pragma pack(push, 1)  //__attribute__((packed));

union SCOFF_SymTableEntry {
   // Normal symbol table entry
   struct {
      char   Name[8];
      uint32 Value;
      int16  SectionNumber;
      uint16 Type;
      uint8  StorageClass;
      uint8  NumAuxSymbols;
   } s;
   // Auxiliary symbol table entry types:

   // Function definition
   struct {
      uint32 TagIndex; // Index to .bf entry
      uint32 TotalSize; // Size of function code
      uint32 PointerToLineNumber; // Pointer to line number entry
      uint32 PointerToNextFunction; // Symbol table index of next function
      uint16 x_tvndx;      // Unused
   } func;

   // .bf abd .ef
   struct {
      uint32 Unused1;
      uint16 SourceLineNumber; // Line number in source file
      uint16 Unused2;
      uint32 Unused3; // Pointer to line number entry
      uint32 PointerToNextFunction; // Symbol table index of next function
      uint16 Unused4;      // Unused
   } bfef;

   // Weak external
   struct {
      uint32 TagIndex; // Symbol table index of alternative symbol2
      uint32 Characteristics; //
      uint32 Unused1; 
      uint32 Unused2; 
      uint16 Unused3;      // Unused
   } weak;

   // File name
   struct {
      char FileName[18];// File name
   } filename;

   // String table index
   struct {          // MS COFF uses multiple aux records rather than a string table entry!
      uint32 zeroes; // zeroes if name file name longer than 18
      uint32 offset; // string table entry
   } stringindex;

   // Section definition
   struct {
      uint32 Length;
      uint16 NumberOfRelocations;  // Line number in source file
      uint16 NumberOfLineNumbers;
      uint32 CheckSum;             // Pointer to line number entry
      uint16 Number;               // Symbol table index of next function
      uint8  Selection;            // Unused
      uint8  Unused1[3];
   } section;
};

// Warning: Size does not fit standard alignment!
// Use SIZE_SCOFF_SymTableEntry instead of sizeof(SCOFF_SymTableEntry)
#define SIZE_SCOFF_SymTableEntry  18  // Size of SCOFF_SymTableEntry packed

// values of weak.Characteristics
#define IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY  1
#define IMAGE_WEAK_EXTERN_SEARCH_LIBRARY    2
#define IMAGE_WEAK_EXTERN_SEARCH_ALIAS      3

/*
#define N_BTMASK (0xf)
#define N_TMASK  (0x30)
#define N_BTSHFT (4)
#define N_TSHIFT (2)  */

//#pragma pack(pop)

/********************** Section number values for symbol table entries **********************/
    
#define COFF_SECTION_UNDEF ((int16)0)      // external symbol
#define COFF_SECTION_ABSOLUTE ((int16)-1)  // value of symbol is absolute
#define COFF_SECTION_DEBUG ((int16)-2)     // debugging symbol - value is meaningless
#define COFF_SECTION_N_TV ((int16)-3)      // indicates symbol needs preload transfer vector
#define COFF_SECTION_P_TV ((int16)-4)      // indicates symbol needs postload transfer vector
#define COFF_SECTION_REMOVE_ME ((int16)-99)// Specific for objconv program: Debug or exception section being removed

/*
 * Type of a symbol, in low N bits of the word

#define T_NULL  0
#define T_VOID  1 // function argument (only used by compiler) 
#define T_CHAR  2 // character  
#define T_SHORT  3 // short integer 
#define T_INT  4 // integer  
#define T_LONG  5 // long integer  
#define T_FLOAT  6 // floating point 
#define T_DOUBLE 7 // double word  
#define T_STRUCT 8 // structure   
#define T_UNION  9 // union   
#define T_ENUM  10 // enumeration   
#define T_MOE  11 // member of enumeration
#define T_UCHAR  12 // unsigned character 
#define T_USHORT 13 // uint16 
#define T_UINT  14 // unsigned integer 
#define T_ULONG  15 // uint32 
#define T_LNGDBL 16 // long double  
 */
/*
 * derived types, in n_type

#define DT_NON  (0) // no derived type 
#define DT_PTR  (1) // pointer 
 #define DT_FCN  (2) // function 
#define DT_ARY  (3) // array 

#define BTYPE(x) ((x) & N_BTMASK)

#define ISPTR(x) (((x) & N_TMASK) == (DT_PTR << N_BTSHFT))
#define ISFCN(x) (((x) & N_TMASK) == (DT_FCN << N_BTSHFT))
#define ISARY(x) (((x) & N_TMASK) == (DT_ARY << N_BTSHFT))
#define ISTAG(x) ((x)==C_STRTAG||(x)==C_UNTAG||(x)==C_ENTAG)
#define DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))
 */
/********************** Storage classes for symbol table entries **********************/

#define COFF_CLASS_NULL                    0
#define COFF_CLASS_AUTOMATIC               1 // automatic variable
#define COFF_CLASS_EXTERNAL                2 // external symbol 
#define COFF_CLASS_STATIC                  3 // static
#define COFF_CLASS_REGISTER                4 // register variable
#define COFF_CLASS_EXTERNAL_DEF            5 // external definition 
#define COFF_CLASS_LABEL                   6 // label
#define COFF_CLASS_UNDEFINED_LABEL         7 // undefined label
#define COFF_CLASS_MEMBER_OF_STRUCTURE     8 // member of structure
#define COFF_CLASS_ARGUMENT                9 // function argument
#define COFF_CLASS_STRUCTURE_TAG          10 // structure tag 
#define COFF_CLASS_MEMBER_OF_UNION        11 // member of union 
#define COFF_CLASS_UNION_TAG              12 // union tag 
#define COFF_CLASS_TYPE_DEFINITION        13 // type definition
#define COFF_CLASS_UNDEFINED_STATIC       14 // undefined static 
#define COFF_CLASS_ENUM_TAG               15 // enumeration tag 
#define COFF_CLASS_MEMBER_OF_ENUM         16 // member of enumeration
#define COFF_CLASS_REGISTER_PARAM         17 // register parameter
#define COFF_CLASS_BIT_FIELD              18 // bit field  
#define COFF_CLASS_AUTO_ARGUMENT          19 // auto argument 
#define COFF_CLASS_LASTENTRY              20 // dummy entry (end of block)
#define COFF_CLASS_BLOCK                 100 // ".bb" or ".eb" 
#define COFF_CLASS_FUNCTION              101 // ".bf" or ".ef" 
#define COFF_CLASS_END_OF_STRUCT         102 // end of structure 
#define COFF_CLASS_FILE                  103 // file name  
#define COFF_CLASS_LINE                  104 // line # reformatted as symbol table entry 
#define COFF_CLASS_SECTION               104 // line # reformatted as symbol table entry
#define COFF_CLASS_ALIAS                 105 // duplicate tag 
#define COFF_CLASS_WEAK_EXTERNAL         105 // duplicate tag  
#define COFF_CLASS_HIDDEN                106 // ext symbol in dmert public lib 
#define COFF_CLASS_END_OF_FUNCTION      0xff // physical end of function 

/********************** Type for symbol table entries **********************/
#define COFF_TYPE_FUNCTION              0x20 // Symbol is function
#define COFF_TYPE_NOT_FUNCTION          0x00 // Symbol is not a function


/********************** Relocation table entry **********************/
//#pragma pack(push, 1)  //__attribute__((packed));

struct SCOFF_Relocation {
  uint32 VirtualAddress;   // Section-relative address of relocation source
  uint32 SymbolTableIndex; // Zero-based index into symbol table
  uint16 Type;             // Relocation type
};
#define SIZE_SCOFF_Relocation  10  // Size of SCOFF_Relocation packed
//#pragma pack(pop)


/********************** Relocation types for 32-bit COFF **********************/

#define COFF32_RELOC_ABS         0x00   // Ignored
#define COFF32_RELOC_DIR16       0x01   // Not supported
#define COFF32_RELOC_REL16       0x02   // Not supported
#define COFF32_RELOC_DIR32       0x06   // 32-bit absolute virtual address
#define COFF32_RELOC_IMGREL      0x07   // 32-bit image relative virtual address
#define COFF32_RELOC_SEG12       0x09   // not supported
#define COFF32_RELOC_SECTION     0x0A   // 16-bit section index in file
#define COFF32_RELOC_SECREL      0x0B   // 32-bit section-relative
#define COFF32_RELOC_SECREL7     0x0D   // 7-bit section-relative
#define COFF32_RELOC_TOKEN       0x0C   // CLR token
#define COFF32_RELOC_REL32       0x14   // 32-bit EIP-relative

/********************** Relocation types for 64-bit COFF **********************/
// Note: These values are obtained by my own testing.
// I haven't found any official values 

#define COFF64_RELOC_ABS         0x00   // Ignored
#define COFF64_RELOC_ABS64       0x01   // 64 bit absolute virtual address
#define COFF64_RELOC_ABS32       0x02   // 32 bit absolute virtual address
#define COFF64_RELOC_IMGREL      0x03   // 32 bit image-relative
#define COFF64_RELOC_REL32       0x04   // 32 bit, RIP-relative
#define COFF64_RELOC_REL32_1     0x05   // 32 bit, relative to RIP - 1. For instruction with immediate byte operand
#define COFF64_RELOC_REL32_2     0x06   // 32 bit, relative to RIP - 2. For instruction with immediate word operand
#define COFF64_RELOC_REL32_3     0x07   // 32 bit, relative to RIP - 3. (useless)
#define COFF64_RELOC_REL32_4     0x08   // 32 bit, relative to RIP - 4. For instruction with immediate dword operand
#define COFF64_RELOC_REL32_5     0x09   // 32 bit, relative to RIP - 5. (useless)
#define COFF64_RELOC_SECTION     0x0A   // 16-bit section index in file. For debug purpose
#define COFF64_RELOC_SECREL      0x0B   // 32-bit section-relative
#define COFF64_RELOC_SECREL7     0x0C   //  7-bit section-relative
#define COFF64_RELOC_TOKEN       0x0D   // CLR token = 64 bit absolute virtual address. Inline addend ignored
#define COFF64_RELOC_SREL32      0x0E   // 32 bit signed span dependent
#define COFF64_RELOC_PAIR        0x0F   // pair after span dependent
#define COFF64_RELOC_PPC_REFHI   0x10   // high 16 bits of 32 bit abs addr
#define COFF64_RELOC_PPC_REFLO   0x11   // low  16 bits of 32 bit abs addr
#define COFF64_RELOC_PPC_PAIR    0x12   // pair after REFHI
#define COFF64_RELOC_PPC_SECRELO 0x13   // low 16 bits of section relative
#define COFF64_RELOC_PPC_GPREL   0x15   // 16 bit signed relative to GP
#define COFF64_RELOC_PPC_TOKEN   0x16   // CLR token

/********************** Strings **********************/
#define COFF_CONSTRUCTOR_NAME    ".CRT$XCU"   // Name of constructors segment


// Function prototypes

// Function to put a name into SCOFF_SymTableEntry. Put name in string table
// if longer than 8 characters
uint32 COFF_PutNameInSymbolTable(SCOFF_SymTableEntry & sym, const char * name, CMemoryBuffer & StringTable);

// Function to put a name into SCOFF_SectionHeader. Put name in string table
// if longer than 8 characters
void COFF_PutNameInSectionHeader(SCOFF_SectionHeader & sec, const char * name, CMemoryBuffer & StringTable);


#endif // #ifndef PECOFF_H
