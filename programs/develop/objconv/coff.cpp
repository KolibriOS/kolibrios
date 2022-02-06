/****************************   coff.cpp   ***********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2009-01-22
* Project:       objconv
* Module:        coff.cpp
* Description:
* Module for reading PE/COFF files
*
* Class CCOFF is used for reading, interpreting and dumping PE/COFF files.
*
* Copyright 2006-2009 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

// Relocation type names

SIntTxt COFF32RelNames[] = {
   {COFF32_RELOC_ABS,     "Absolute"},         // Ignored
   {COFF32_RELOC_DIR32,   "Direct32"},         // 32-bit absolute virtual address
   {COFF32_RELOC_IMGREL,  "Image relative"},   // 32-bit image relative virtual address
   {COFF32_RELOC_SECTION, "Section16"},        // 16-bit section index in file
   {COFF32_RELOC_SECREL,  "Section relative"}, // 32-bit section-relative
   {COFF32_RELOC_SECREL7, "7 bit section relative"}, // 7-bit section-relative
   {COFF32_RELOC_TOKEN,   "CLR token"},        // CLR token
   {COFF32_RELOC_REL32,   "EIP relative"}      // 32-bit relative to end of address field
};

SIntTxt COFF64RelNames[] = {
   {COFF64_RELOC_ABS,     "Ignore"},           // Ignored
   {COFF64_RELOC_ABS64,   "64 bit absolute"},  // 64 bit absolute virtual address
   {COFF64_RELOC_ABS32,   "32 bit absolute"},  // 32 bit absolute virtual address
   {COFF64_RELOC_IMGREL,  "Image relative"},   // 32 bit image-relative
   {COFF64_RELOC_REL32,   "RIP relative"},     // 32 bit, RIP-relative
   {COFF64_RELOC_REL32_1, "RIP relative-1"},   // 32 bit, relative to RIP - 1. For instruction with immediate byte operand
   {COFF64_RELOC_REL32_2, "RIP relative-2"},   // 32 bit, relative to RIP - 2. For instruction with immediate word operand
   {COFF64_RELOC_REL32_3, "RIP relative-3"},   // 32 bit, relative to RIP - 3. Useless 
   {COFF64_RELOC_REL32_4, "RIP relative-4"},   // 32 bit, relative to RIP - 4. For instruction with immediate dword operand
   {COFF64_RELOC_REL32_5, "RIP relative-5"},   // 32 bit, relative to RIP - 5. Useless
   {COFF32_RELOC_SECTION, "Section index"},    // 16-bit section index in file
   {COFF64_RELOC_SECREL,  "Section relative"}, // 32-bit section-relative
   {COFF64_RELOC_SECREL7, "7 bit section rel"},//  7-bit section-relative
   {COFF64_RELOC_TOKEN,   "CLR token"},        // 64 bit absolute virtual address without inline addend
   {COFF64_RELOC_SREL32,  "32b span dependent"},        // 
   {COFF64_RELOC_PAIR,    "pair after span dependent"}, // 
   {COFF64_RELOC_PPC_REFHI,"high 16 of 32 bit abs"},    // 
   {COFF64_RELOC_PPC_REFLO,"low 16 of 32 bit abs"},     // 
   {COFF64_RELOC_PPC_PAIR, "pair after high 16"},       // 
   {COFF64_RELOC_PPC_SECRELO,"low 16 of 32 bit section relative"},
   {COFF64_RELOC_PPC_GPREL,  "16 bit GP relative"},     // 
   {COFF64_RELOC_PPC_TOKEN,  "CLR token"}               // 
};

// Machine names

SIntTxt COFFMachineNames[] = {
   {0,     "Any/unknown"},     // Any machine/unknown
   {0x184, "Alpha"},           // Alpha AXP
   {0x1C0, "Arm"},             // Arm
   {0x284, "Alpha 64 bit"},    // Alpha AXP 64 bit
   {0x14C, "I386"},            // x86, 32 bit
   {0x200, "IA64"},            // Intel Itanium
   {0x268, "Motorola68000"},   // Motorola 68000 series
   {0x266, "MIPS16"},  
   {0x366, "MIPSwFPU"},
   {0x466, "MIPS16wFPU"},
   {0x1F0, "PowerPC"},
   {0x1F1, "PowerPC"},
   {0x162, "R3000"},
   {0x166, "R4000MIPS"},
   {0x168, "R10000"},
   {0x1A2, "SH3"},
   {0x1A6, "SH4"},
   {0x1C2, "Thumb"},
   {0x8664, "x86-64"}      // x86-64 / AMD64 / Intel EM64T
};

// Storage class names
SIntTxt COFFStorageClassNames[] = {
   {COFF_CLASS_END_OF_FUNCTION, "EndOfFunc"},
   {COFF_CLASS_AUTOMATIC, "AutoVariable"},
   {COFF_CLASS_EXTERNAL, "External/Public"},
   {COFF_CLASS_STATIC, "Static/Nonpublic"},
   {COFF_CLASS_REGISTER, "Register"},
   {COFF_CLASS_EXTERNAL_DEF, "ExternalDef"},
   {COFF_CLASS_LABEL, "Label"},
   {COFF_CLASS_UNDEFINED_LABEL, "UndefLabel"},
   {COFF_CLASS_MEMBER_OF_STRUCTURE, "StructMem"},
   {COFF_CLASS_ARGUMENT, "FuncArgument"},
   {COFF_CLASS_STRUCTURE_TAG, "StructTag"},
   {COFF_CLASS_MEMBER_OF_UNION, "UnionMember"},
   {COFF_CLASS_UNION_TAG, "UnionTag"},
   {COFF_CLASS_TYPE_DEFINITION, "TypeDef"},
   {COFF_CLASS_UNDEFINED_STATIC, "UndefStatic"},
   {COFF_CLASS_ENUM_TAG, "EnumTag"},
   {COFF_CLASS_MEMBER_OF_ENUM, "EnumMem"},
   {COFF_CLASS_REGISTER_PARAM, "RegisterParameter"},
   {COFF_CLASS_BIT_FIELD, "BitField"},
   {COFF_CLASS_AUTO_ARGUMENT, "AutoArgument"},
   {COFF_CLASS_LASTENTRY, "DummyLastEntry"},
   {COFF_CLASS_BLOCK, "bb/eb_block"},
   {COFF_CLASS_FUNCTION, "Function_bf/ef"},
   {COFF_CLASS_END_OF_STRUCT, "EndOfStruct"},
   {COFF_CLASS_FILE, "FileName"},
   {COFF_CLASS_LINE, "LineNumber"},
   {COFF_CLASS_SECTION, "SectionLineNumber"},
   {COFF_CLASS_ALIAS, "Alias"},
   {COFF_CLASS_WEAK_EXTERNAL, "WeakExternal"},
   {COFF_CLASS_HIDDEN, "Hidden"}
};

// Names of section characteristics
SIntTxt COFFSectionFlagNames[] = {
   {PE_SCN_CNT_CODE,        "Text"},
   {PE_SCN_CNT_INIT_DATA,   "Data"},
   {PE_SCN_CNT_UNINIT_DATA, "BSS"},
   {PE_SCN_LNK_INFO,        "Comments"},
   {PE_SCN_LNK_REMOVE,      "Remove"},
   {PE_SCN_LNK_COMDAT,      "Comdat"},
/* {PE_SCN_ALIGN_1,         "Align by 1"},
   {PE_SCN_ALIGN_2,         "Align by 2"},
   {PE_SCN_ALIGN_4,         "Align by 4"},
   {PE_SCN_ALIGN_8,         "Align by 8"},
   {PE_SCN_ALIGN_16,        "Align by 16"},
   {PE_SCN_ALIGN_32,        "Align by 32"},
   {PE_SCN_ALIGN_64,        "Align by 64"},
   {PE_SCN_ALIGN_128,       "Align by 128"},
   {PE_SCN_ALIGN_256,       "Align by 256"},
   {PE_SCN_ALIGN_512,       "Align by 512"},
   {PE_SCN_ALIGN_1024,      "Align by 1024"},
   {PE_SCN_ALIGN_2048,      "Align by 2048"},
   {PE_SCN_ALIGN_4096,      "Align by 4096"},
   {PE_SCN_ALIGN_8192,      "Align by 8192"}, */
   {PE_SCN_LNK_NRELOC_OVFL, "extended relocations"},
   {PE_SCN_MEM_DISCARDABLE, "Discardable"},
   {PE_SCN_MEM_NOT_CACHED,  "Cannot be cached"},
   {PE_SCN_MEM_NOT_PAGED,   "Not pageable"},
   {PE_SCN_MEM_SHARED,      "Can be shared"},
   {PE_SCN_MEM_EXECUTE,     "Executable"},
   {PE_SCN_MEM_READ,        "Readable"},
   {PE_SCN_MEM_WRITE,       "Writeable"}
};

// Names of image data directories in optional header
SIntTxt COFFImageDirNames[] = {
   {0,   "Export_table"},
   {1,   "Import_table"},
   {2,   "Resource_table"},
   {3,   "Exception_table"},
   {4,   "Certificate_table"},
   {5,   "Base_relocation_table"},
   {6,   "Debug_table"},
   {7,   "Architecture_table"},
   {8,   "Global_pointer"},
   {9,   "Thread_local_storage_table"},
   {10,  "Load_configuration_table"},
   {11,  "Bound_import_table"},
   {12,  "Import_address_table"},
   {13,  "Delay_import_descriptor"},
   {14,  "Common_language_runtime_header"},
   {15,  "Reserved_table"}
};

// Class CCOFF members:
// Constructor
CCOFF::CCOFF() {
   // Set everything to zero
   memset(this, 0, sizeof(*this));
}

void CCOFF::ParseFile(){
   // Load and parse file buffer
   // Get offset to file header
   uint32 FileHeaderOffset = 0;
   if ((Get<uint16>(0) & 0xFFF9) == 0x5A49) {
      // File has DOS stub
      uint32 Signature = Get<uint32>(0x3C);
      if (Signature + 8 < DataSize && Get<uint16>(Signature) == 0x4550) {
         // Executable PE file
         FileHeaderOffset = Signature + 4;
      }
      else {
         err.submit(9000);
         return;
      }
   }
   // Find file header
   FileHeader = &Get<SCOFF_FileHeader>(FileHeaderOffset);
   NSections = FileHeader->NumberOfSections;

   // check header integrity
   if ((uint64)FileHeader->PSymbolTable + FileHeader->NumberOfSymbols * SIZE_SCOFF_SymTableEntry > GetDataSize()) err.submit(2035);

   // Find optional header if executable file
   if (FileHeader->SizeOfOptionalHeader && FileHeaderOffset) {
      OptionalHeader = &Get<SCOFF_OptionalHeader>(FileHeaderOffset + sizeof(SCOFF_FileHeader));
      // Find image data directories
      if (OptionalHeader) {
         if (OptionalHeader->h64.Magic == COFF_Magic_PE64) {
            // 64 bit version
            pImageDirs = &(OptionalHeader->h64.ExportTable);
            NumImageDirs = OptionalHeader->h64.NumberOfRvaAndSizes;
            EntryPoint = OptionalHeader->h64.AddressOfEntryPoint;
            ImageBase = OptionalHeader->h64.ImageBase;
         }
         else {
            // 32 bit version
            pImageDirs = &(OptionalHeader->h32.ExportTable);
            NumImageDirs = OptionalHeader->h32.NumberOfRvaAndSizes;
            EntryPoint = OptionalHeader->h32.AddressOfEntryPoint;
            ImageBase = OptionalHeader->h32.ImageBase;
         }
      }
   }

   // Allocate buffer for section headers
   SectionHeaders.SetNum(NSections);
   SectionHeaders.SetZero();

   // Find section headers
   uint32 SectionOffset = FileHeaderOffset + sizeof(SCOFF_FileHeader) + FileHeader->SizeOfOptionalHeader;
   for (int i = 0; i < NSections; i++) {
      SectionHeaders[i] = Get<SCOFF_SectionHeader>(SectionOffset);
      SectionOffset += sizeof(SCOFF_SectionHeader);
      // Check for _ILDATA section
      if (strcmp(SectionHeaders[i].Name, "_ILDATA") == 0) {
         // This is an intermediate file for Intel compiler
         err.submit(2114);
      }
   }
   if (SectionOffset > GetDataSize()) {
      err.submit(2110);  return;             // Section table points to outside file
   }
   // Find symbol table
   SymbolTable = &Get<SCOFF_SymTableEntry>(FileHeader->PSymbolTable);
   NumberOfSymbols = FileHeader->NumberOfSymbols;

   // Find string table
   StringTable = (Buf() + FileHeader->PSymbolTable + NumberOfSymbols * SIZE_SCOFF_SymTableEntry);
   StringTableSize = *(int*)StringTable; // First 4 bytes of string table contains its size
}

// Debug dump
void CCOFF::Dump(int options) {
   uint32 i, j;

   if (options & DUMP_FILEHDR) {
      // File header
      printf("\nDump of PE/COFF file %s", FileName);
      printf("\n-----------------------------------------------");
      printf("\nFile size: %i", GetDataSize());
      printf("\nFile header:");
      printf("\nMachine: %s", Lookup(COFFMachineNames,FileHeader->Machine));
      printf("\nTimeDate: 0x%08X", FileHeader->TimeDateStamp);
      printf(" - %s", timestring(FileHeader->TimeDateStamp));
      printf("\nNumber of sections: %2i", FileHeader->NumberOfSections);
      printf("\nNumber of symbols:  %2i", FileHeader->NumberOfSymbols);
      printf("\nOptional header size: %i", FileHeader->SizeOfOptionalHeader);
      printf("\nFlags: 0x%04X", FileHeader->Flags);

      // May be removed:
      printf("\nSymbol table offset: 0x%X", FileHeader->PSymbolTable);
      printf("\nString table offset: 0x%X", FileHeader->PSymbolTable + FileHeader->NumberOfSymbols * SIZE_SCOFF_SymTableEntry);
      printf("\nSection headers offset: 0x%X", (uint32)sizeof(SCOFF_FileHeader) + FileHeader->SizeOfOptionalHeader);

      // Optional header
      if (OptionalHeader) {
         printf("\n\nOptional header:");
         if (OptionalHeader->h32.Magic != COFF_Magic_PE64) {
            // 32 bit optional header
            printf("\nMagic number: 0x%X", OptionalHeader->h32.Magic);
            printf("\nSize of code: 0x%X", OptionalHeader->h32.SizeOfCode);
            printf("\nSize of uninitialized data: 0x%X", OptionalHeader->h32.SizeOfInitializedData);
            printf("\nAddress of entry point: 0x%X", OptionalHeader->h32.AddressOfEntryPoint);
            printf("\nBase of code: 0x%X", OptionalHeader->h32.BaseOfCode);
            printf("\nBase of data: 0x%X", OptionalHeader->h32.BaseOfData);
            printf("\nImage base: 0x%X", OptionalHeader->h32.ImageBase);
            printf("\nSection alignment: 0x%X", OptionalHeader->h32.SectionAlignment);
            printf("\nFile alignment: 0x%X", OptionalHeader->h32.FileAlignment);
            printf("\nSize of image: 0x%X", OptionalHeader->h32.SizeOfImage);
            printf("\nSize of headers: 0x%X", OptionalHeader->h32.SizeOfHeaders);
            printf("\nDll characteristics: 0x%X", OptionalHeader->h32.DllCharacteristics);
            printf("\nSize of stack reserve: 0x%X", OptionalHeader->h32.SizeOfStackReserve);
            printf("\nSize of stack commit: 0x%X", OptionalHeader->h32.SizeOfStackCommit);
            printf("\nSize of heap reserve: 0x%X", OptionalHeader->h32.SizeOfHeapReserve);
            printf("\nSize of heap commit: 0x%X", OptionalHeader->h32.SizeOfHeapCommit);
         }
         else {
            // 64 bit optional header
            printf("\nMagic number: 0x%X", OptionalHeader->h64.Magic);
            printf("\nSize of code: 0x%X", OptionalHeader->h64.SizeOfCode);
            printf("\nSize of uninitialized data: 0x%X", OptionalHeader->h64.SizeOfInitializedData);
            printf("\nAddress of entry point: 0x%X", OptionalHeader->h64.AddressOfEntryPoint);
            printf("\nBase of code: 0x%X", OptionalHeader->h64.BaseOfCode);
            printf("\nImage base: 0x%08X%08X", HighDWord(OptionalHeader->h64.ImageBase), uint32(OptionalHeader->h64.ImageBase));
            printf("\nSection alignment: 0x%X", OptionalHeader->h64.SectionAlignment);
            printf("\nFile alignment: 0x%X", OptionalHeader->h64.FileAlignment);
            printf("\nSize of image: 0x%X", OptionalHeader->h64.SizeOfImage);
            printf("\nSize of headers: 0x%X", OptionalHeader->h64.SizeOfHeaders);
            printf("\nDll characteristics: 0x%X", OptionalHeader->h64.DllCharacteristics);
            printf("\nSize of stack reserve: 0x%08X%08X", HighDWord(OptionalHeader->h64.SizeOfStackReserve), uint32(OptionalHeader->h64.SizeOfStackReserve));
            printf("\nSize of stack commit: 0x%08X%08X", HighDWord(OptionalHeader->h64.SizeOfStackCommit), uint32(OptionalHeader->h64.SizeOfStackCommit));
            printf("\nSize of heap reserve: 0x%08X%08X", HighDWord(OptionalHeader->h64.SizeOfHeapReserve), uint32(OptionalHeader->h64.SizeOfHeapReserve));
            printf("\nSize of heap commit: 0x%08X%08X", HighDWord(OptionalHeader->h64.SizeOfHeapCommit), uint32(OptionalHeader->h64.SizeOfHeapCommit));
         }
         // Data directories
         SCOFF_ImageDirAddress dir;

         for (i = 0; i < NumImageDirs; i++) {
            if (GetImageDir(i, &dir)) {
               printf("\nDirectory %2i, %s:\n  Address 0x%04X, Size 0x%04X, Section %i, Offset 0x%04X", 
                  i, dir.Name,
                  dir.VirtualAddress, dir.Size, dir.Section, dir.SectionOffset);
            }
         }
      }
   }

   if ((options & DUMP_STRINGTB) && FileHeader->PSymbolTable && StringTableSize > 4) {
      // String table
      char * p = StringTable + 4;
      uint32 nread = 4, len;
      printf("\n\nString table:");
      while (nread < StringTableSize) {
         len = (int)strlen(p);
         if (len > 0) {
            printf("\n>>%s<<", p);
            nread += len + 1;
            p += len + 1;
         }
      }
   }
   // Symbol tables
   if (options & DUMP_SYMTAB) {
      // Symbol table (object file)
      if (NumberOfSymbols) PrintSymbolTable(-1);

      // Import and export tables (executable file)
      if (OptionalHeader) PrintImportExport();
   }

   // Section headers
   if (options & (DUMP_SECTHDR | DUMP_SYMTAB | DUMP_RELTAB)) {
      for (j = 0; j < (uint32)NSections; j++) {
         SCOFF_SectionHeader * SectionHeader = &SectionHeaders[j];
         printf("\n\n%2i Section %s", j+1, GetSectionName(SectionHeader->Name));

         //printf("\nFile offset of header: 0x%X", (int)((int8*)SectionHeader-buffer));
         printf("\nVirtual size: 0x%X", SectionHeader->VirtualSize);
         if (SectionHeader->VirtualAddress) {
            printf("\nVirtual address: 0x%X", SectionHeader->VirtualAddress);}
         if (SectionHeader->PRawData || SectionHeader->SizeOfRawData) {
            printf("\nSize of raw data: 0x%X", SectionHeader->SizeOfRawData);
            printf("\nRaw data pointer: 0x%X", SectionHeader->PRawData);
         }
         printf("\nCharacteristics: ");
         PrintSegmentCharacteristics(SectionHeader->Flags);

         // print relocations
         if ((options & DUMP_RELTAB) && SectionHeader->NRelocations > 0) {
            printf("\nRelocation entries: %i", SectionHeader->NRelocations);
            printf("\nRelocation entries pointer: 0x%X", SectionHeader->PRelocations);

            // Pointer to relocation entry
            union {
               SCOFF_Relocation * p;  // pointer to record
               int8 * b;              // used for address calculation and incrementing
            } Reloc;
            Reloc.b = Buf() + SectionHeader->PRelocations;

            printf("\nRelocations:");
            for (i = 0; i < SectionHeader->NRelocations; i++) {
               printf("\nAddr: 0x%X, symi: %i, type: %s",
                  Reloc.p->VirtualAddress,
                  Reloc.p->SymbolTableIndex,
                  (WordSize == 32) ? Lookup(COFF32RelNames,Reloc.p->Type) : Lookup(COFF64RelNames,Reloc.p->Type));
               if (Reloc.p->Type < COFF32_RELOC_SEG12) 
               {
                  // Check if address is within file
                  if (SectionHeader->PRawData + Reloc.p->VirtualAddress < GetDataSize()) {
                     int32 addend = *(int32*)(Buf() + SectionHeader->PRawData + Reloc.p->VirtualAddress);
                     if (addend) printf(", Implicit addend: %i", addend);
                  }
                  else {
                     printf(". Error: Address is outside file");
                  }
               }
               
               PrintSymbolTable(Reloc.p->SymbolTableIndex);
               Reloc.b += SIZE_SCOFF_Relocation; // Next relocation record
            }
         }
         // print line numbers
         if (SectionHeader->NLineNumbers > 0) {
            printf("\nLine number entries: %i", SectionHeader->NLineNumbers);
            printf("  Line number pointer: %i\nLines:", SectionHeader->PLineNumbers);
            
            // Pointer to line number entry
            union {
               SCOFF_LineNumbers * p;  // pointer to record
               int8 * b;              // used for address calculation and incrementing
            } Linnum;
            Linnum.b = Buf() + SectionHeader->PLineNumbers;
            for (i = 0; i < SectionHeader->NLineNumbers; i++) {
               if (Linnum.p->Line) {  // Record contains line number
                  printf(" %i:%i", Linnum.p->Line, Linnum.p->Addr);
               }
               else { // Record contains function name
               }
               Linnum.b += SIZE_SCOFF_LineNumbers;  // Next line number record
            }         
         }
      }
   }
}


char const * CCOFF::GetSymbolName(int8* Symbol) {
   // Get symbol name from 8 byte entry
   static char text[16];
   if (*(uint32*)Symbol != 0) {
      // Symbol name not more than 8 bytes
      memcpy(text, Symbol, 8);   // Copy to local buffer
      text[8] = 0;                    // Append terminating zero
      return text;                    // Return text
   }
   else {
      // Longer than 8 bytes. Get offset into string table
      uint32 offset = *(uint32*)(Symbol + 4);
      if (offset >= StringTableSize || offset >= GetDataSize()) {err.submit(2035); return "";}
      char * s = StringTable + offset;
      if (*s) return s;               // Return string table entry
   }
   return "NULL";                     // String table entry was empty
}


char const * CCOFF::GetSectionName(int8* Symbol) {
   // Get section name from 8 byte entry
   static char text[16];
   memcpy(text, Symbol, 8);        // Copy to local buffer
   text[8] = 0;                    // Append terminating zero
   if (text[0] == '/') {
      // Long name is in string table. 
      // Convert decimal ASCII number to string table index
      uint32 sindex = atoi(text + 1);
      // Get name from string table
      if (sindex < StringTableSize) {
         char * s = StringTable + sindex;
         if (*s) return s;}                 // Return string table entry
   }
   else {
      // Short name is in text buffer
      return text;
   }
   return "NULL";                           // In case of error
}

char const * CCOFF::GetStorageClassName(uint8 sc) {
   // Get storage class name
   return Lookup(COFFStorageClassNames, sc);
}

void CCOFF::PrintSegmentCharacteristics(uint32 flags) {
   // Print segment characteristics
   int n = 0;
   // Loop through all bits of integer
   for (uint32 i = 1; i != 0; i <<= 1) {
      if (i & flags & ~PE_SCN_ALIGN_MASK) {
         if (n++) printf(", ");
         printf("%s", Lookup(COFFSectionFlagNames, i));
      }
   }
   if (flags & PE_SCN_ALIGN_MASK) {
      int a = 1 << (((flags & PE_SCN_ALIGN_MASK) / PE_SCN_ALIGN_1) - 1);
      printf(", Align by 0x%X", a); n++;
   }
   if (n == 0) printf("None");
}

const char * CCOFF::GetFileName(SCOFF_SymTableEntry * syme) {
   // Get file name from records in symbol table
   if (syme->s.NumAuxSymbols < 1 || syme->s.StorageClass != COFF_CLASS_FILE) {
      return ""; // No file name found
   }
   // Set limit to file name length = 576
   const uint32 MAXCOFFFILENAMELENGTH = 32 * SIZE_SCOFF_SymTableEntry;
   // Buffer to store file name. Must be static
   static char text[MAXCOFFFILENAMELENGTH+1];
   // length of name in record
   uint32 len = syme->s.NumAuxSymbols * SIZE_SCOFF_SymTableEntry;
   if (len > MAXCOFFFILENAMELENGTH) len = MAXCOFFFILENAMELENGTH;
   // copy name from auxiliary records
   memcpy(text, (int8*)syme + SIZE_SCOFF_SymTableEntry, len);
   // Terminate string
   text[len] = 0;
   // Return name
   return text;
}

const char * CCOFF::GetShortFileName(SCOFF_SymTableEntry * syme) {
   // Same as above. Strips path before filename
   // Full file name
   const char * fullname = GetFileName(syme);
   // Length
   uint32 len = (uint32)strlen(fullname);
   if (len < 1) return fullname;
   // Scan backwards for '/', '\', ':'
   for (int scan = len-2; scan >= 0; scan--) {
      char c = fullname[scan];
      if (c == '/' || c == '\\' || c == ':') {
         // Path found. Short name starts after this character
         return fullname + scan + 1;
      }
   }
   // No path found. Return full name
   return fullname;
}

void CCOFF::PrintSymbolTable(int symnum) {
   // Print one or all public symbols for object file.
   // Dump symbol table if symnum = -1, or
   // Dump symbol number symnum (zero based) when symnum >= 0
   int isym = 0;  // current symbol table entry
   int jsym = 0;  // auxiliary entry number
   union {        // Pointer to symbol table
      SCOFF_SymTableEntry * p;  // Normal pointer
      int8 * b;                 // Used for address calculation
   } Symtab;

   Symtab.p = SymbolTable;      // Set pointer to begin of SymbolTable
   if (symnum == -1) printf("\n\nSymbol table:");
   if (symnum >= 0) {
      // Print one symbol only
      if (symnum >= NumberOfSymbols) {
         printf("\nSymbol %i not found", symnum);
         return;
      }
      isym = symnum;
      Symtab.b += SIZE_SCOFF_SymTableEntry * isym;
   }
   while (isym < NumberOfSymbols) {
      // Print symbol table entry
      SCOFF_SymTableEntry *s0;
      printf("\n");
      if (symnum >= 0) printf("  ");
      printf("Symbol %i - Name: %s\n  Value=%i, ", 
         isym, GetSymbolName(Symtab.p->s.Name), Symtab.p->s.Value);
      if (Symtab.p->s.SectionNumber > 0) {
         printf("Section=%i", Symtab.p->s.SectionNumber);
      }
      else { // Special section numbers
         switch (Symtab.p->s.SectionNumber) {
         case COFF_SECTION_UNDEF:
            printf("External"); break;
         case COFF_SECTION_ABSOLUTE:
            printf("Absolute"); break;
         case COFF_SECTION_DEBUG:
            printf("Debug"); break;
         case COFF_SECTION_N_TV:
            printf("Preload transfer"); break;
         case COFF_SECTION_P_TV:
            printf("Postload transfer"); break;
         }
      }
      printf(", Type=0x%X, StorClass=%s, NumAux=%i",
         Symtab.p->s.Type,
         GetStorageClassName(Symtab.p->s.StorageClass), Symtab.p->s.NumAuxSymbols);
      if (Symtab.p->s.StorageClass == COFF_CLASS_FILE && Symtab.p->s.NumAuxSymbols > 0) {
         printf("\n  File name: %s", GetFileName(Symtab.p));
      }
      // Increment point
      s0 = Symtab.p;
      Symtab.b += SIZE_SCOFF_SymTableEntry;
      isym++;  jsym = 0;
      // Get auxiliary records
      while (jsym < s0->s.NumAuxSymbols && isym + jsym < NumberOfSymbols) {
         // Print auxiliary symbol table entry
         SCOFF_SymTableEntry * sa = Symtab.p;
         // Detect auxiliary entry type
         if (s0->s.StorageClass == COFF_CLASS_EXTERNAL
            && s0->s.Type == COFF_TYPE_FUNCTION
            && s0->s.SectionNumber > 0) {
            // This is a function definition aux record
            printf("\n  Aux function definition:");
            printf("\n  .bf_tag_index: 0x%X, code_size: %i, PLineNumRec: %i, PNext: %i",
               sa->func.TagIndex, sa->func.TotalSize, sa->func.PointerToLineNumber,
               sa->func.PointerToNextFunction);
         }
         else if (strcmp(s0->s.Name,".bf")==0 || strcmp(s0->s.Name,".ef")==0) {
            // This is a .bf or .ef aux record
            printf("\n  Aux .bf/.ef definition:");
            printf("\n  Source line number: %i",
               sa->bfef.SourceLineNumber);
            if (strcmp(s0->s.Name,".bf")==0 ) {
               printf(", PNext: %i", sa->bfef.PointerToNextFunction);
            }
         }
         else if (s0->s.StorageClass == COFF_CLASS_EXTERNAL && 
            s0->s.SectionNumber == COFF_SECTION_UNDEF &&
            s0->s.Value == 0) {
            // This is a Weak external aux record
            printf("\n  Aux Weak external definition:");
            printf("\n  Symbol2 index: %i, Characteristics: 0x%X",
               sa->weak.TagIndex, sa->weak.Characteristics);
            }
         else if (s0->s.StorageClass == COFF_CLASS_FILE) {
            // This is filename aux record. Contents has already been printed
         }
         else if (s0->s.StorageClass == COFF_CLASS_STATIC) {
            // This is section definition aux record
            printf("\n  Aux section definition record:");
            printf("\n  Length: %i, Num. relocations: %i, Num linenums: %i, checksum 0x%X,"
               "\n  Number: %i, Selection: %i",
               sa->section.Length, sa->section.NumberOfRelocations, sa->section.NumberOfLineNumbers, 
               sa->section.CheckSum, sa->section.Number, sa->section.Selection);
         }
         else if (s0->s.StorageClass == COFF_CLASS_ALIAS) {
            // This is section definition aux record
            printf("\n  Aux alias definition record:");
            printf("\n  symbol index: %i, ", sa->weak.TagIndex);
            switch (sa->weak.Characteristics) {
            case IMAGE_WEAK_EXTERN_SEARCH_NOLIBRARY:
                printf("no library search"); break;
            case IMAGE_WEAK_EXTERN_SEARCH_LIBRARY:
                printf("library search"); break;
            case IMAGE_WEAK_EXTERN_SEARCH_ALIAS:
                printf("alias symbol"); break;
            default:
                printf("unknown characteristics 0x%X", sa->weak.Characteristics);
            }
         }         
         else {
            // Unknown aux record type
            printf("\n  Unknown Auxiliary record type %i", s0->s.StorageClass);
         }
         Symtab.b += SIZE_SCOFF_SymTableEntry;
         jsym++;
      }
      isym += jsym;
      if (symnum >= 0) break;
   }
}

void CCOFF::PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m) {
   // Make list of public names in object file
   // Strings will receive ASCIIZ strings
   // Index will receive records of type SStringEntry with Member = m

   // Interpret header:
   ParseFile();

   int isym = 0;  // current symbol table entry
   union {        // Pointer to symbol table
      SCOFF_SymTableEntry * p;  // Normal pointer
      int8 * b;                 // Used for address calculation
   } Symtab;

   // Loop through symbol table
   Symtab.p = SymbolTable;
   while (isym < NumberOfSymbols) {
      // Check within buffer
      if (Symtab.b >= Buf() + DataSize) {
         err.submit(2040);
         break;
      }

      // Search for public symbol
      if (Symtab.p->s.SectionNumber > 0 && Symtab.p->s.StorageClass == COFF_CLASS_EXTERNAL) {
         // Public symbol found
         SStringEntry se;
         se.Member = m;

         // Store name
         se.String = Strings->PushString(GetSymbolName(Symtab.p->s.Name));
         // Store name index
         Index->Push(se);
      }
      if ((int8)Symtab.p->s.NumAuxSymbols < 0) Symtab.p->s.NumAuxSymbols = 0;

      // Increment point
      isym += Symtab.p->s.NumAuxSymbols + 1;
      Symtab.b += (1 + Symtab.p->s.NumAuxSymbols) * SIZE_SCOFF_SymTableEntry;
   }
}

int CCOFF::GetImageDir(uint32 n, SCOFF_ImageDirAddress * dir) {
   // Find address of image directory for executable files
   int32  Section;
   uint32 FileOffset;

   if (pImageDirs == 0 || n >= NumImageDirs || dir == 0) {
      // Failure
      return 0;
   }
   // Get virtual address and size of directory
   dir->VirtualAddress = pImageDirs[n].VirtualAddress;
   dir->Size           = pImageDirs[n].Size;
   dir->Name           = Lookup(COFFImageDirNames, n);

   // Check if nonzero
   if (dir->VirtualAddress == 0 || dir->Size == 0) {
      // Empty
      return 0;
   }

   // Search for section containing this address
   for (Section = 0; Section < NSections; Section++) {
      if (dir->VirtualAddress >= SectionHeaders[Section].VirtualAddress
      && dir->VirtualAddress < SectionHeaders[Section].VirtualAddress + SectionHeaders[Section].SizeOfRawData) {
         // Found section
         dir->Section = Section + 1;
         // Section-relative offset
         dir->SectionOffset = dir->VirtualAddress - SectionHeaders[Section].VirtualAddress;
         // Calculate file offset
         FileOffset = SectionHeaders[Section].PRawData + dir->SectionOffset;
         if (FileOffset == 0 || FileOffset >= DataSize) {
            // points outside file
            err.submit(2035);
            return 0;
         }
         // FileOffset is within range
         dir->FileOffset = FileOffset;

         // Maximum allowed offset
         dir->MaxOffset = SectionHeaders[Section].SizeOfRawData - dir->SectionOffset;

         // Return success
         return Section;
      }
   }
   // Import section not found
   return 0;
}

void CCOFF::PrintImportExport() {
   // Print imported and exported symbols

   // Table directory address
   SCOFF_ImageDirAddress dir;

   uint32 i;                                     // Index into OrdinalTable and NamePointerTable
   uint32 Ordinal;                               // Index into ExportAddressTable
   uint32 Address;                               // Virtual address of exported symbol
   uint32 NameOffset;                            // Section offset of symbol name
   uint32 SectionOffset;                         // Section offset of table
   const char * Name;                            // Name of symbol

   // Check if 64 bit
   int Is64bit = OptionalHeader->h64.Magic == COFF_Magic_PE64;

   // Exported names
   if (GetImageDir(0, &dir)) {

      // Beginning of export section is export directory
      SCOFF_ExportDirectory * pExportDirectory = &Get<SCOFF_ExportDirectory>(dir.FileOffset);

      // Find ExportAddressTable
      SectionOffset = pExportDirectory->ExportAddressTableRVA - dir.VirtualAddress;
      if (SectionOffset == 0 || SectionOffset >= dir.MaxOffset) {
         // Points outside section
         err.submit(2035);  return;
      }
      uint32 * pExportAddressTable = &Get<uint32>(dir.FileOffset + SectionOffset);

      // Find ExportNameTable
      SectionOffset = pExportDirectory->NamePointerTableRVA - dir.VirtualAddress;
      if (SectionOffset == 0 || SectionOffset >= dir.MaxOffset) {
         // Points outside section
         err.submit(2035);  return;
      }
      uint32 * pExportNameTable = &Get<uint32>(dir.FileOffset + SectionOffset);

      // Find ExportOrdinalTable
      SectionOffset = pExportDirectory->OrdinalTableRVA - dir.VirtualAddress;
      if (SectionOffset == 0 || SectionOffset >= dir.MaxOffset) {
         // Points outside section
         err.submit(2035);  return;
      }
      uint16 * pExportOrdinalTable = &Get<uint16>(dir.FileOffset + SectionOffset);

      // Get further properties
      uint32 NumExports = pExportDirectory->AddressTableEntries;
      uint32 NumExportNames = pExportDirectory->NamePointerEntries;
      uint32 OrdinalBase = pExportDirectory->OrdinalBase;

      // Print exported names
      printf("\n\nExported symbols:");

      // Loop through export tables
      for (i = 0; i < NumExports; i++) {

         Address = 0;
         Name = "(None)";

         // Get ordinal from table
         Ordinal = pExportOrdinalTable[i];
         // Address table is indexed by ordinal
         if (Ordinal < NumExports) {
            Address = pExportAddressTable[Ordinal];
         }
         // Find name if there is a name list entry
         if (i < NumExportNames) {
            NameOffset = pExportNameTable[i] - dir.VirtualAddress;
            if (NameOffset && NameOffset < dir.MaxOffset) {
               Name = &Get<char>(dir.FileOffset + NameOffset);
            }
         }
         // Print ordinal, address and name
         printf("\n  Ordinal %3i, Address %6X, Name %s",
            Ordinal + OrdinalBase, Address, Name);
      }
   }
   // Imported names
   if (GetImageDir(1, &dir)) {

      // Print imported names
      printf("\n\nImported symbols:");

      // Pointer to current import directory entry
      SCOFF_ImportDirectory * ImportEntry = &Get<SCOFF_ImportDirectory>(dir.FileOffset);
      // Pointer to current import lookup table entry
      int32 * LookupEntry = 0;
      // Pointer to current hint/name table entry
      SCOFF_ImportHintName * HintNameEntry;

      // Loop through import directory until null entry
      while (ImportEntry->DLLNameRVA) {
         // Get DLL name
         NameOffset = ImportEntry->DLLNameRVA - dir.VirtualAddress;
         if (NameOffset < dir.MaxOffset) {
            Name = &Get<char>(dir.FileOffset + NameOffset);
         }
         else {
            Name = "Error";
         }
         // Print DLL name
         printf("\nFrom %s", Name);

         // Get lookup table
         SectionOffset = ImportEntry->ImportLookupTableRVA;
         if (SectionOffset == 0) SectionOffset = ImportEntry->ImportAddressTableRVA;
         if (SectionOffset == 0) continue;
         SectionOffset -= dir.VirtualAddress;
         if (SectionOffset >= dir.MaxOffset) break;  // Out of range
         LookupEntry = &Get<int32>(dir.FileOffset + SectionOffset);

         // Loop through lookup table
         while (LookupEntry[0]) {
            if (LookupEntry[Is64bit] < 0) {
               // Imported by ordinal
               printf("\n  Ordinal %i", uint16(LookupEntry[0]));
            }
            else {
               // Find entry in hint/name table
               SectionOffset = (LookupEntry[0] & 0x7FFFFFFF) - dir.VirtualAddress;;
               if (SectionOffset >= dir.MaxOffset) continue;  // Out of range
               HintNameEntry = &Get<SCOFF_ImportHintName>(dir.FileOffset + SectionOffset);

               // Print name
               printf("\n  %04X  %s", HintNameEntry->Hint, HintNameEntry->Name);

               // Check if exported
               if (HintNameEntry->Hint) {
                 // printf(",  Export entry %i", HintNameEntry->Hint);
               }
            }
            // Loop next
            LookupEntry += Is64bit ? 2 : 1;
         }

         // Loop next
         ImportEntry++;
      }   
   }
}

// Functions for manipulating COFF files

uint32 COFF_PutNameInSymbolTable(SCOFF_SymTableEntry & sym, const char * name, CMemoryBuffer & StringTable) {
   // Function to put a name into SCOFF_SymTableEntry. 
   // Put name in string table if longer than 8 characters.
   // Returns index into StringTable if StringTable used
   int len = (int)strlen(name);                  // Length of name
   if (len <= 8) {
      // Short name. store in section header
      memcpy(sym.s.Name, name, len);
      // Pad with zeroes
      for (; len < 8; len++) sym.s.Name[len] = 0;
   }
   else {
      // Long name. store in string table
      sym.stringindex.zeroes = 0;
      sym.stringindex.offset = StringTable.PushString(name);     // Second integer = entry into string table
      return sym.stringindex.offset;
   }
   return 0;
}

void COFF_PutNameInSectionHeader(SCOFF_SectionHeader & sec, const char * name, CMemoryBuffer & StringTable) {
   // Function to put a name into SCOFF_SectionHeader. 
   // Put name in string table if longer than 8 characters
   int len = (int)strlen(name);                  // Length of name
   if (len <= 8) {
      // Short name. store in section header
      memcpy(sec.Name, name, len);
      // Pad with zeroes
      for (; len < 8; len++) sec.Name[len] = 0;
   }
   else {
      // Long name. store in string table
      sprintf(sec.Name, "/%i", StringTable.PushString(name));
   }
}
