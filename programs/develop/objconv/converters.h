/****************************  converters.h   ********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2008-05-25
* Project:       objconv
* Module:        converters.h
* Description:
* Header file for file conversion classes.
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

/*******************************   Classes   ********************************

This header file declares various classes for interpreting and converting
different types of object files. These classes are all derived from the 
container class CFileBuffer, declared in containers.h.

See containers.h for an explanation of the container classes and the 
operators >> and << which can transfer a data buffer from an object of one
class to an object of another class.

*****************************************************************************/

#ifndef CONVERTERS_H
#define CONVERTERS_H


// Structure for string index entry in library
struct SStringEntry {
   uint32 String;                      // Offset to string
   uint32 Member;                      // Library member
};


// Class CResponseFileBuffer is used for storage of a command line response file
class CResponseFileBuffer : public CFileBuffer {
public:
   CResponseFileBuffer(char const * filename);   // Constructor
   ~CResponseFileBuffer();                       // Destructor
   CResponseFileBuffer * next;                   // Linked list if more than one buffer
};


// Class for deciding what to do with input file
// Its memory buffer contains the input file and later the output file
class CMain : public CFileBuffer {
public:
   CMain();                            // Constructor
   void Go();                          // Do whatever the command line parameters say
};


// Class CConverter is used for converting or dumping a file of any type
class CConverter : public CFileBuffer {
public:
   CConverter();                       // Constructor
   void Go();                          // Do whatever the command line parameters say
protected:
   void DumpCOF();                     // Dump PE/COFF file
   void DumpELF();                     // Dump ELF file
   void DumpMACHO();                   // Dump Mach-O file
   void DumpOMF();                     // Dump OMF file
   void ParseMACUnivBin();             // Dump Mac universal binary
   void COF2COF();                     // Make changes in PE file
   void COF2ELF();                     // Convert PE/COFF to ELF file
   void COF2OMF();                     // Convert PE/COFF to OMF file
   void ELF2ELF();                     // Make changes in ELF file
   void ELF2COF();                     // Convert ELF to PE file
   void ELF2MAC();                     // Convert ELF to Mach-O file
   void OMF2COF();                     // Convert OMF file to PE/COFF
   void COF2ASM();                     // Disassemble PE/COFF file
   void ELF2ASM();                     // Disassemble ELF file
   void MAC2ELF();                     // Convert Mach-O file to ELF file
   void MAC2MAC();                     // Make changes in Mach-O file
   void MAC2ASM();                     // Disassemble Mach-O file
   void OMF2ASM();                     // Disassemble OMF file
};

// Class for interpreting and dumping PE/COFF files
class CCOFF : public CFileBuffer {
public:
   CCOFF();                                      // Default constructor
   void ParseFile();                             // Parse file buffer
   void Dump(int options);                       // Dump file
   void PrintSymbolTable(int symnum);            // Dump symbol table entries
   void PrintImportExport();                     // Print imported and exported symbols
   static void PrintSegmentCharacteristics(uint32 flags); // Print segment characteristics
   char const * GetSymbolName(int8* Symbol);     // Get symbol name from 8 byte entry
   char const * GetSectionName(int8* Symbol);    // Get section name from 8 byte entry
   const char * GetFileName(SCOFF_SymTableEntry *);    // Get file name from records in symbol table
   const char * GetShortFileName(SCOFF_SymTableEntry*);// Same as above. Strips path before filename
   char const * GetStorageClassName(uint8 sc);   // Get storage class name
   void PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m); // Make list of public names
   int  GetImageDir(uint32 n, SCOFF_ImageDirAddress * dir); // Find address of image directory for executable files
protected:
   CArrayBuf<SCOFF_SectionHeader> SectionHeaders;// Copy of section headers
   int NSections;                                // Number of sections
   SCOFF_FileHeader * FileHeader;                // File header
   SCOFF_SymTableEntry * SymbolTable;            // Pointer to symbol table (for object files)
   char * StringTable;                           // Pointer to string table (for object files)
   uint32 StringTableSize;                       // Size of string table (for object files)
   int NumberOfSymbols;                          // Number of symbol table entries (for object files)
   uint64 ImageBase;                             // Image base (for executable files)
   SCOFF_OptionalHeader * OptionalHeader;        // Optional header (for executable files)
   SCOFF_IMAGE_DATA_DIRECTORY * pImageDirs;      // Pointer to image directories (for executable files)
   uint32 NumImageDirs;                          // Number of image directories (for executable files)
   uint32 EntryPoint;                            // Entry point (for executable files)
};


// Class for interpreting and dumping ELF files. Has templates for 32 and 64 bit version
template <class TFileHeader, class TSectionHeader, class TSymbol, class TRelocation>
class CELF : public CFileBuffer {
public:
   CELF();                                       // Default constructor
   void ParseFile();                             // Parse file buffer
   void Dump(int options);                       // Dump file
   void PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m); // Make list of public names
protected:
   const char * SymbolName(uint32 index);        // Get name of symbol
   TFileHeader FileHeader;                       // Copy of file header
   char * SecStringTable;                        // Section header string table
   uint32 SecStringTableLen;                     // Length of section header string table
   uint32 NSections;                             // Number of sections
   int SectionHeaderSize;                        // Size of each section header
   CArrayBuf<TSectionHeader> SectionHeaders;     // Copy of section headers
   uint32 SymbolTableOffset;                     // Offset to symbol table
   uint32 SymbolTableEntrySize;                  // Entry size of symbol table
   uint32 SymbolTableEntries;                    // Number of symbols
   uint32 SymbolStringTableOffset;               // Offset to symbol string table
   uint32 SymbolStringTableSize;                 // Size of symbol string table
};


// Class for interpreting and dumping Mach-O files
class COMF : public CFileBuffer {
public:
   COMF();                                       // Default constructor
   void ParseFile();                             // Parse file buffer
   void Dump(int options);                       // Dump file
   void PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m); // Make list of public names
protected:
   uint32 NumRecords;                            // Number of records
   CSList<SOMFRecordPointer> Records;            // Record pointers (List is 0-based)
   CMemoryBuffer NameBuffer;                     // Store segment names and symbol names
   CSList<uint32> LocalNameOffset;               // Offset into NameBuffer of segment names by name index
   CSList<uint32> SegmentNameOffset;             // Offset into NameBuffer of segment names by segment index
   CSList<uint32> SymbolNameOffset;              // Offset into NameBuffer of external symbol names
   CSList<uint32> GroupNameOffset;               // Offset into NameBuffer of group names
   char * GetLocalName(uint32 i);                // Get segment name by name index
   uint32 GetLocalNameO(uint32 i);               // Get segment name by converting name index offset into NameBuffer
   const char * GetSegmentName(uint32 i);        // Get segment name by segment index
   const char * GetSymbolName(uint32 i);         // Get external symbol name
   const char * GetGroupName(uint32 i);          // Get group name by index
   static const char * GetRecordTypeName(uint32 i);// Get OMF record type name
   void DumpRecordTypes();                       // Dump summary of records
   void DumpNames();                             // Dump local names records
   void DumpSymbols();                           // Dump public and external names records
   void DumpSegments();                          // Dump segment records
   void DumpRelocations();                       // Dump fixup records
   void DumpComments();                          // Dump coment records
};

// Class for interpreting and dumping Mach-O files. Has templates for 32 and 64 bit version
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
class CMACHO : public CFileBuffer {
public:
   CMACHO();                                     // Default constructor
   void ParseFile();                             // Parse file buffer
   void Dump(int options);                       // Dump file
   void PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m); // Make list of public names
protected:
   TMAC_header FileHeader;                       // Copy of file header
   uint64 ImageBase;                             // Image base for executable file
   uint32 SegmentOffset;                         // File offset of segment
   uint32 SegmentSize;                           // Size of segment
   uint32 SectionHeaderOffset;                   // File offset of section headers
   uint32 NumSections;                           // Number of sections
   uint32 SymTabOffset;                          // File offset of symbol table
   uint32 SymTabNumber;                          // Number of entries in symbol table
   uint32 StringTabOffset;                       // File offset of string table
   uint32 StringTabSize;                         // Size of string table
   uint32 ilocalsym;	                            // index to local symbols
   uint32 nlocalsym;	                            // number of local symbols 
   uint32 iextdefsym;	                         // index to public symbols
   uint32 nextdefsym;	                         // number of public symbols 
   uint32 iundefsym;	                            // index to external symbols
   uint32 nundefsym;	                            // number of external symbols
   uint32 IndirectSymTabOffset;                  // file offset to the indirect symbol table
   uint32 IndirectSymTabNumber;                  // number of indirect symbol table entries
};

// Class for parsing Macintosh universal binary
class CMACUNIV : public CFileBuffer {
public:
   CMACUNIV();                                   // Default constructor
   void Go(int options);                         // Apply command line options to all components
};


// class CCOF2ELF handles conversion from PE/COFF file to ELF file. Has templates for 32 and 64 bit version
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
class CCOF2ELF : public CCOFF {
public:
   CCOF2ELF();                                    // Constructor
   void Convert();                                // Do the conversion
protected:
   void MakeSegments();                           // Convert subfunction: Segments
   void MakeSymbolTable();                        // Convert subfunction: Symbol table and string tables
   void MakeRelocationTables();                   // Convert subfunction: Relocation tables
   void MakeBinaryFile();                         // Convert subfunction: Putting sections together
   int symtab;                                    // Symbol table section number
   int shstrtab;                                  // Section name string table section number
   int strtab;                                    // Object name string table section number
   int stabstr;                                   // Debug string table section number
   int NumSectionsNew;                            // Number of sections generated for 'to' file
   int MaxSectionsNew;                            // Number of section buffers allocated for 'to' file
   CArrayBuf<CMemoryBuffer> NewSections;          // Buffers for building each section
   CArrayBuf<TELF_SectionHeader> NewSectionHeaders;// Buffer for temporary section headers
   CArrayBuf<int> NewSectIndex;                   // Buffers for array of new section indices
   CArrayBuf<int> NewSymbolIndex;                 // Buffers for array of new symbol indices
   CFileBuffer ToFile;                            // File buffer for ELF file
   TELF_Header NewFileHeader;                     // New file header
};


// class CCOF2OMF handles conversion from PE/COFF file to OMF file
class CCOF2OMF : public CCOFF {
public:
   CCOF2OMF();                                    // Constructor
   void Convert();                                // Do the conversion
protected:
   void MakeSegmentList();                        // Make temporary segment conversion list
   void MakeSymbolList();                         // Make temporary symbol conversion list
   void MakeRelocationsList();                    // Make temporary list of relocations (fixups) and sort it
   void MakeLNAMES();                             // Make THEADR and LNAMES records
   void MakeSEGDEF();                             // Make SEGDEF and GRPDEF records
   void MakeEXTDEF();                             // Make EXTDEF records
   void MakePUBDEF();                             // Make PUBDEF records
   void MakeLEDATA();                             // Make LEDATA, LIDATA and FIXUPP records
   void MakeMODEND();                             // Make MODEND record and finish file
   CArrayBuf<SOMFSegmentList> SectionBuffer;      // Summarize old sections. Translate section index to segment index
   CArrayBuf<SOMFSymbolList> SymbolBuffer;        // Translate old symbol index to new public/external index
   CSList<SOMFRelocation> RelocationBuffer;       // Summarize and sort relocations
   CMemoryBuffer NameBuffer;                      // Temporary storage of text strings
   COMFFileBuilder ToFile;                        // File buffer for new OMF file
   int  NumSegments;                              // Number of segments in new file
   int  SectionBufferNum;                         // Number of entries in SectionBuffer
   uint32 NumPublicSymbols;                       // Number of public symbols in new file
   uint32 NumExternalSymbols;                     // Number of external symbols in new file
   uint32 NumRelocations;                         // Number of entries in RelocationBuffer
};


// class COMF2COF handles conversion from OMF file to PE/COFF file
class COMF2COF : public COMF {
public:
   COMF2COF();                                    // Constructor
   void Convert();                                // Do the conversion
protected:
   // Convert subfunctions:
   void MakeFileHeader();                        // File header
   void MakeSymbolTable1();                      // Make symbol table and string table entries for file and segments
   void MakeSymbolTable2();                      // Make symbol table and string table entries for external symbols
   void MakeSymbolTable3();                      // Make symbol table and string table entries for public symbols
   void MakeSymbolTable4();                      // Make symbol table and string table entries for communal symbols
   void MakeSymbolTable5();                      // Make symbol table and string table entries for local symbols
   void MakeSections();                          // Make sections and relocation tables
   void MakeBinaryFile();                        // Putting sections together
   void CheckUnsupportedRecords();               // Make warnings if file containes unsupported record types
   int  NumSectionsNew;                          // Number of sections in new file
   CFileBuffer ToFile;                           // File buffer for PE/COFF file
   CSList<SCOFF_SymTableEntry> NewSymbolTable;   // New symbol table entries
   CSList<SCOFF_SectionHeader> NewSectionHeaders;// New section headers
   CMemoryBuffer NewStringTable;                 // Buffer for building new string table
   CMemoryBuffer NewData;                        // Raw data for each section in new file and its relocation table
   CSList<uint32> SegmentTranslation;            // Translate old segment number to new symbol table index
   CSList<uint32> ExtdefTranslation;             // Translate old external symbol number to new symbol table index
   CSList<SOMFLocalSymbol> LocalSymbols;         // List for assigning names to unnamed local symbols
   SCOFF_FileHeader NewFileHeader;               // New file header
};


// class CELF2COF handles conversion from ELF file to PE/COFF file. Has templates for 32 and 64 bit version
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
class CELF2COF : public CELF<ELFSTRUCTURES> {
public:
   CELF2COF();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   void MakeFileHeader();                        // Convert subfunction: File header
   void MakeSectionsIndex();                     // Convert subfunction: Make section index translation table
   void MakeSections();                          // Convert subfunction: Make sections and relocation tables
   void MakeSymbolTable();                       // Convert subfunction: Symbol table and string tables
   void HideUnusedSymbols();                     // Convert subfunction: Hide unused symbols
   void MakeBinaryFile();                        // Convert subfunction: Putting sections together
   int NumSectionsNew;                           // Number of sections in new file
   CArrayBuf<int32> NewSectIndex;                // Array of new section indices
   CArrayBuf<int32> SymbolsUsed;                 // Array of new symbol indices
   CSList<int32> NewSymbolIndex;                 // Buffer for array of new symbol indices
   CMemoryBuffer NewSymbolTable;                 // Buffer for building new symbol table
   CMemoryBuffer NewStringTable;                 // Buffer for building new string table
   CMemoryBuffer NewRawData;                     // Buffer for building new raw data area
   uint32 RawDataOffset;                         // File offset for raw data
   CFileBuffer ToFile;                           // File buffer for PE/COFF file
   SCOFF_FileHeader NewFileHeader;               // New file header
};


// class CELF2MAC handles conversion from ELF file to Mach-O file
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
class CELF2MAC : public CELF<ELFSTRUCTURES> {
public:
   CELF2MAC();                         // Constructor
   void Convert();                     // Do the conversion
protected:
   void MakeFileHeader();              // Convert subfunction: File header
   void MakeSectionsIndex();           // Convert subfunction: Make section index translation table
   void MakeSections();                // Convert subfunction: Make sections and relocation tables
   void MakeSymbolTable();             // Convert subfunction: Symbol table and string tables
   void FindUnusedSymbols();           // Convert subfunction: Check if symbols used, remove unused symbols
   void MakeBinaryFile();              // Convert subfunction: Putting sections together
   // Translate relocations, seperate function for 32 and 64 bits:
   void Elf2MacRelocations(Elf32_Shdr &, MAC_section_32 &, uint32 NewRawDataOffset, uint32 oldsec);
   void Elf2MacRelocations(Elf64_Shdr &, MAC_section_64 &, uint32 NewRawDataOffset, uint32 oldsec);
   int  GetImagebaseSymbol();          // Symbol table index of __mh_execute_header
   CFileBuffer   ToFile;               // File buffer for new Mach-O file
   CMemoryBuffer NewRawData;           // Buffer for building new raw data area
   CMemoryBuffer NewRelocationTab;     // Buffer for new relocation tables
   CMemoryBuffer NewStringTable;       // Buffer for building new string table
   CMemoryBuffer UnnamedSymbolsTable;  // Buffer for assigning names to unnamed symbols
   CArrayBuf<int> NewSectIndex;        // Array of new section indices
   CArrayBuf<MInt> NewSectOffset;      // Array of new section offsets
   CArrayBuf<int> OldSymbolScope;      // Table of symbol bindings: 0 = local, 1 = public, 2 = external
   CArrayBuf<int> OldSymbolUsed;       // Check if symbol is used
   MacSymbolTableBuilder<TMAC_nlist, MInt> NewSymTab[3]; // New symbol tables for local, public, external symbols
   uint32 NumSymbols[4];               // Accumulated number of entries in each NewSymTab[]
   uint32 NewSectHeadOffset;           // File offset to first section header
   uint32 NewSymtabOffset;             // File offset to symtab command
   int NumSectionsNew;                 // Number of sections in new file
   uint32 RawDataOffset;               // Offset to raw data in old file
   uint32 NumOldSymbols;               // Number of symbols in old file
   uint32 CommandOffset;               // Offset to first load command = segment header
};

// class MAC2ELF handles conversion from Mach-O file to ELF file
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
class CMAC2ELF : public CMACHO<MACSTRUCTURES> {
public:
   CMAC2ELF();                         // Constructor
   void Convert();                     // Do the conversion
protected:
   void MakeSegments();                           // Convert subfunction: Segments
   void MakeSymbolTable();                        // Convert subfunction: Symbol table and string tables
   void MakeRelocationTables(MAC_header_32&);     // Convert subfunction: Relocation tables, 32-bit version
   void MakeRelocationTables(MAC_header_64&);     // Convert subfunction: Relocation tables, 64-bit version
   void MakeImportTables();                       // Convert subfunction: Fill import tables
   void MakeBinaryFile();                         // Convert subfunction: Putting sections together
   void TranslateAddress(MInt addr, uint32 & section, uint32 & offset); // Translate address to section + offset
   uint32 MakeGOTEntry(int symbol);               // Make entry in fake GOT for symbol
   void MakeGOT();                                // Make fake Global Offset Table
   int symtab;                                    // Symbol table section number
   int shstrtab;                                  // Section name string table section number
   int strtab;                                    // Object name string table section number
   int stabstr;                                   // Debug string table section number
   uint32 NumSectionsNew;                         // Number of sections generated for 'to' file
   uint32 MaxSectionsNew;                         // Number of section buffers allocated for 'to' file
   uint32 HasGOT;                                 // Contains references to global offset table
   int FakeGOTSection;                            // Fake GOT section number
   int FakeGOTSymbol;                             // Symbol index for fake GOT
   TELF_Header NewFileHeader;                     // New file header
   CArrayBuf<CMemoryBuffer> NewSections;          // Buffers for building each section
   CArrayBuf<TELF_SectionHeader> NewSectionHeaders;// Array of temporary section headers
   CArrayBuf<int> NewSectIndex;                   // Array of new section indices
   CArrayBuf<int> NewSymbolIndex;                 // Array of new symbol indices
   CArrayBuf<int> SectionSymbols;                 // Array of new symbol indices for sections
   CFileBuffer ToFile;                            // File buffer for ELF file
   CSList<int> GOTSymbols;                        // List of symbols needing GOT entry
};


// class CCOF2COF handles symbol changes in a PE/COFF file
class CCOF2COF : public CCOFF {
public:
   CCOF2COF();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   void MakeSymbolTable();                       // Convert subfunction: Symbol table and string tables
   void MakeBinaryFile();                        // Convert subfunction: Putting sections together
   CMemoryBuffer NewSymbolTable;                 // Buffers for building new symbol table
   CMemoryBuffer NewStringTable;                 // Buffers for building new string table
   CFileBuffer ToFile;                           // File buffer for modified PE file
};


// class CELF2ELF handles symbol changes in ELF file. Has templates for 32 and 64 bit version
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
class CELF2ELF : public CELF<ELFSTRUCTURES> {
public:
   CELF2ELF();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   void MakeSymbolTable();                       // Convert subfunction: Symbol table and string tables
   void ChangeSections();                        // Convert subfunction: Change section names if needed
   void MakeBinaryFile();                        // Convert subfunction: Putting sections together
   uint32 isymtab[2];                            // static and dynamic symbol table section number
   uint32 istrtab[4];                            // string table section number: symbols, dynamic symbols, sections, debug
   CMemoryBuffer NewSymbolTable[2];              // Buffers for building new symbol tables: static, dynamic
   CMemoryBuffer NewStringTable[4];              // Buffers for building new string tables: symbols, dynamic symbols, sections, debug
   CArrayBuf<uint32> NewSymbolIndex;             // Array for translating old to new symbol indices
   uint32 NumOldSymbols;                         // Size of NewSymbolIndex table
   uint32 FirstGlobalSymbol;                     // Index to first global symbol in .symtab
   CFileBuffer ToFile;                           // File buffer for modified PE file
};


// class CMAC2MAC handles symbol changes in Mach-O file. Has templates for 32 and 64 bit version
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
class CMAC2MAC : public CMACHO<MACSTRUCTURES> {
public:
   CMAC2MAC();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   void MakeSymbolTable();                       // Convert subfunction: Symbol table and string tables
   void ChangeSegments();                        // Convert subfunction: Change segment names if needed
   void ChangeSections(uint32 HeaderOffset, uint32 Num);// Convert subfunction: Change section names and relocation records if needed
   void ChangeImportTable(uint32 FileOffset, uint32 Num);// Convert subfunction: Change symbol indices in import table if needed
   void MakeBinaryFile();                        // Convert subfunction: Putting sections together
   int  NewSymbolIndex(int OldIndex);            // Convert subfunction: Translate old to new symbol index
   uint32 NewFileOffset(uint32 OldOffset);       // Convert subfunction: Translate old to new file offset
   MacSymbolTableBuilder<TMAC_nlist, MInt> NewSymbols[3];// Buffers for building new symbol tables: local, public, external
   CMemoryBuffer NewSymbolTable;                 // Buffer for building new symbol table
   CMemoryBuffer NewStringTable;                 // Buffer for building new string table
   CFileBuffer ToFile;                           // File buffer for modified PE file
   uint32 NumOldSymbols;                         // Size of NewSymbolIndex table
   uint32 NewIlocalsym;	                         // index to local symbols
   uint32 NewNlocalsym;	                         // number of local symbols 
   uint32 NewIextdefsym;	                      // index to public symbols
   uint32 NewNextdefsym;	                      // number of public symbols 
   uint32 NewIundefsym;	                         // index to external symbols
   uint32 NewNundefsym;	                         // number of external symbols
   uint32 NewSymtabOffset;                       // Offset to new symbol table
   uint32 NewStringtabOffset;                    // Offset to new string table
   uint32 NewStringtabEnd;                       // Offset to end of new string table
   uint32 OldTablesEnd;                          // End of old symbol table and string table
   int32  SizeDifference;                        // Size of new file minus size of old file
};


// class CCOF2ASM handles disassembly of PE/COFF file
class CCOF2ASM : public CCOFF {
public:
   CCOF2ASM();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   CDisassembler Disasm;                         // Disassembler
   void MakeSectionList();                       // Make Sections list and Relocations list in Disasm
   void MakeSymbolList();                        // Make Symbols list in Disasm
   void MakeDynamicRelocations();                // Make dynamic base relocations for executable files
   void MakeImportList();                        // Make imported symbols for executable files
   void MakeExportList();                        // Make exported symbols for executable files
   void MakeListLabels();                        // Attach names to all image directories
};

// class CELF2ASM handles disassembly of ELF file
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
class CELF2ASM : public CELF<ELFSTRUCTURES> {
public:
   CELF2ASM();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   CDisassembler Disasm;                         // Disassembler
   CArrayBuf<int32>SectionNumberTranslate;       // Translate section numbers in source file to section numbers in asm file
   CArrayBuf<uint32>SymbolTableOffset;           // Addend to add to symbol number for each symbol table
   int64 ImageBase;                              // Image base if executable file
   uint32 ExeType;                               // File type: 0 = object, 1 = DLL/shared object, 2 = executable
   uint32 NumSymbols;                            // Number of symbols defined
   void FindImageBase();                         // Find image base
   void MakeSectionList();                       // Make Sections list in Disasm
   void MakeSymbolList();                        // Make Symbols list in Disasm
   void MakeRelocations();                       // Make relocations for object and executable files
   void MakeImportList();                        // Make imported symbols for executable files
   void MakeExportList();                        // Make exported symbols for executable files
   void MakeListLabels();                        // Attach names to all image directories
};

// class CMAC2ASM handles disassembly of Mach-O file
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
class CMAC2ASM : public CMACHO<MACSTRUCTURES> {
public:
   CMAC2ASM();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   void MakeSectionList();                       // Make Sections list in Disasm
   void MakeSymbolList();                        // Make Symbols list in Disasm
   void MakeRelocations();                       // Make relocation list in Disasm
   void MakeImports();                           // Make symbol entries for imported symbols
   CDisassembler Disasm;                         // Disassembler
   CMemoryBuffer StringBuffer;                   // Buffer for making section names
   CSList<MAC_SECT_WITH_RELOC> RelocationQueue;  // List of relocation tables
   CSList<TMAC_section*> ImportSections;          // List of sections needing extra symbols: import tables, literals, etc.
};

// class COMF2ASM handles disassembly of OMF object files
class COMF2ASM : public COMF {
public:
   COMF2ASM();                                   // Constructor
   void Convert();                               // Do the conversion
protected:
   void CountSegments();                         // Make temporary Segments table
   void MakeExternalSymbolsTable();              // Make external symbols in Disasm
   void MakePublicSymbolsTable();                // Make symbol table entries for public symbols
   void MakeCommunalSymbolsTable();              // Make symbol table entries for communal symbols
   void MakeGroupDefinitions();                  // Make segment group definitions
   void MakeSegmentList();                       // Make Segments list in Disasm
   void MakeRelocations(int32 Segment, uint32 RecNum, uint32 SOffset, uint32 RSize, uint8 * SData);// Make relocation list in Disasm
   CDisassembler Disasm;                         // Disassembler
   CSList<SOMFSegment> Segments;                 // Name, size, etc. of all segments
   CSList<uint32> ExtdefTranslation;             // Translate old external symbol number to disasm symbol table index
   CSList<uint32> PubdefTranslation;             // Translate old public symbol number to disasm symbol table index
   CMemoryBuffer SegmentData;                    // Binary segment data
   int32 NumSegments;                            // Number of segments
   int32 FirstComDatSection;                     // First COMDAT section. All sections before this are SEGDEF segments
};

#endif // #ifndef CONVERTERS_H
