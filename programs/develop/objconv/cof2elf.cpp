/****************************  cof2elf.cpp   ********************************
* Author:        Agner Fog
* Date created:  2006-07-20
* Last modified: 2008-05-22
* Project:       objconv
* Module:        cof2elf.cpp
* Description:
* Module for converting PE/COFF file to ELF file
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
CCOF2ELF<ELFSTRUCTURES>::CCOF2ELF () {
   // Constructor
   memset(this, 0, sizeof(*this));
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CCOF2ELF<ELFSTRUCTURES>::Convert() {
   // Do the conversion
   NumSectionsNew = 5;                                    // Number of sections generated so far

   // Allocate variable size buffers
   MaxSectionsNew    = NumSectionsNew + 2 * NSections;    // Max number of sections needed
   NewSections.SetNum(MaxSectionsNew);                    // Allocate buffers for each section
   NewSections.SetZero();                                 // Initialize
   NewSectionHeaders.SetNum(MaxSectionsNew);              // Allocate array for temporary section headers
   NewSectionHeaders.SetZero();                           // Initialize
   NewSectIndex.SetNum(NSections);                        // Array for translating old section index (0-based) to new section index
   NewSectIndex.SetZero();                                // Initialize
   NewSymbolIndex.SetNum(NumberOfSymbols);                // Array of new symbol indices
   NewSymbolIndex.SetZero();                              // Initialize

   // Call the subfunctions
   ToFile.SetFileType(FILETYPE_ELF);   // Set type of to file
   MakeSegments();                     // Make segment headers and code/data segments
   MakeSymbolTable();                  // Symbol table and string tables
   MakeRelocationTables();             // Relocation tables
   MakeBinaryFile();                   // Putting sections together
   *this << ToFile;                    // Take over new file buffer
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CCOF2ELF<ELFSTRUCTURES>::MakeSegments() {
   // Convert subfunction: Make segment headers and code/data segments
   TELF_SectionHeader NewSecHeader;    // New section header
   int oldsec;                         // Section index in old file
   int newsec;                         // Section index in new file
   uint32 SecNameIndex;                // Section name index into shstrtab
   char const * SecName;               // Name of new section
   const int WordSize = sizeof(NewFileHeader.e_entry) * 8; // word size 32 or 64 bits

   // Special segment names
   static const char * SpecialSegmentNames[] = {
      "Null", ".symtab", ".shstrtab", ".strtab", ".stabstr"
   };
   // Indexes to these are:
   symtab      = 1;               // Symbol table section number
   shstrtab    = 2;               // Section name string table section number
   strtab      = 3;               // Object name string table section number
   stabstr     = 4;               // Debug string table section number

   // Number of special segments = number of names in SpecialSegmentNames:
   const int NumSpecialSegments = sizeof(SpecialSegmentNames)/sizeof(SpecialSegmentNames[0]);

   // Make first section header string table entry empty
   NewSections[shstrtab].PushString("");

   // Loop through special sections, except the first Null section:
   for (newsec = 0; newsec < NumSpecialSegments; newsec++) {
      // Put data into new section header:
      // Initialize to zero
      memset(&NewSecHeader, 0, sizeof(NewSecHeader));

      if (newsec > 0) {
         // Put name into section header string table
         SecName = SpecialSegmentNames[newsec];
         SecNameIndex = NewSections[shstrtab].PushString(SecName);

         // Put name into new section header
         NewSecHeader.sh_name = SecNameIndex;
      }

      // Put section header into temporary buffer
      NewSectionHeaders[newsec] = NewSecHeader;
   }

   // Put type, flags, etc. into special segments:
   NewSectionHeaders[symtab]  .sh_type  = SHT_SYMTAB;
   NewSectionHeaders[symtab]  .sh_entsize = sizeof(TELF_Symbol);
   NewSectionHeaders[symtab]  .sh_link  = strtab;
   NewSectionHeaders[shstrtab].sh_type  = SHT_STRTAB;
   NewSectionHeaders[shstrtab].sh_flags = SHF_STRINGS;
   NewSectionHeaders[shstrtab].sh_addralign = 1;
   NewSectionHeaders[strtab]  .sh_type  = SHT_STRTAB;
   NewSectionHeaders[strtab]  .sh_flags = SHF_STRINGS;
   NewSectionHeaders[strtab]  .sh_addralign = 1;
   NewSectionHeaders[stabstr] .sh_type  = SHT_STRTAB;
   NewSectionHeaders[stabstr] .sh_flags = SHF_STRINGS;
   NewSectionHeaders[stabstr] .sh_addralign = 1;

   if (newsec != NumSectionsNew) {
      // Check my program for internal consistency
      // If you get this error then change the value of NumSectionsNew in 
      // the constructor CCOF2ELF::CCOF2ELF to equal the number of entries in 
      // SpecialSegmentNames, including the Null segment
      err.submit(9000);
   }

   // Loop through source file sections
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Pointer to old section header
      SCOFF_SectionHeader * SectionHeader = &this->SectionHeaders[oldsec];

      // Get section name
      SecName = this->GetSectionName(SectionHeader->Name);
      if (strnicmp(SecName,"debug",5) == 0 || strnicmp(SecName+1,"debug",5) == 0) {
         // This is a debug section
         if (cmd.DebugInfo == CMDL_DEBUG_STRIP) {
            // Remove debug info
            NewSectIndex[oldsec] = COFF_SECTION_REMOVE_ME;  // Remember that this section is removed
            cmd.CountDebugRemoved();
            continue;
         }
         else if (cmd.InputType != cmd.OutputType) {
            err.submit(1029); // Warn that debug information is incompatible
         }
      }
      if (strnicmp(SecName,".drectve",8) == 0 || (SectionHeader->Flags & (PE_SCN_LNK_INFO | PE_SCN_LNK_REMOVE))) {
         // This is a directive section
         if (cmd.ExeptionInfo) {
            // Remove directive section
            NewSectIndex[oldsec] = COFF_SECTION_REMOVE_ME;  // Remember that this section is removed
            cmd.CountExceptionRemoved();
            continue;
         }
      }
      if (strnicmp(SecName,".pdata", 6) == 0) {
         // This section has exception information
         if (cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
            // Remove exception info
            NewSectIndex[oldsec] = COFF_SECTION_REMOVE_ME;  // Remember that this section is removed
            cmd.CountExceptionRemoved();
            continue;
         }
         else if (cmd.InputType != cmd.OutputType) {
            err.submit(1030); // Warn that exception information is incompatible
         }
      }
      
      if (strnicmp(SecName,".cormeta", 8) == 0) {
         // This is a .NET Common Language Runtime section
         err.submit(2014);
      }
      if (strnicmp(SecName,".rsrc", 5) == 0) {
         // This section has Windows resource information
         err.submit(1031);
      }

      // Store section index in index translation table (zero-based index)
      NewSectIndex[oldsec] = newsec;

      // Store section data
      if (SectionHeader->SizeOfRawData > 0) {
         NewSections[newsec].Push(Buf()+SectionHeader->PRawData, SectionHeader->SizeOfRawData);
      }

      // Put data into new section header:
      // Initialize to zero
      memset(&NewSecHeader, 0, sizeof(NewSecHeader));

      // Section type
      if (!(SectionHeader->Flags & PE_SCN_LNK_REMOVE)) {
         NewSecHeader.sh_type = SHT_PROGBITS;  // Program code or data
         NewSecHeader.sh_flags |= SHF_ALLOC;   // Occupies memory during execution
      }
      if (SectionHeader->Flags & PE_SCN_CNT_UNINIT_DATA) {
         NewSecHeader.sh_type = SHT_NOBITS;    // BSS
      }

      // Section flags
      if (SectionHeader->Flags & PE_SCN_MEM_WRITE) {
         NewSecHeader.sh_flags |= SHF_WRITE;
      }
      if (SectionHeader->Flags & PE_SCN_MEM_EXECUTE) {
         NewSecHeader.sh_flags |= SHF_EXECINSTR;
      }

      // Check for special sections
      if (strcmp(SecName, COFF_CONSTRUCTOR_NAME)==0) {
         // Constructors segment
         SecName = ELF_CONSTRUCTOR_NAME;
         NewSecHeader.sh_flags = SHF_WRITE | SHF_ALLOC;
      }

      // Put name into section header string table
      SecNameIndex = NewSections[shstrtab].PushString(SecName);

      // Put name into new section header
      NewSecHeader.sh_name = SecNameIndex;

      // Section virtual memory address
      NewSecHeader.sh_addr = SectionHeader->VirtualAddress;

      // Section size in memory
      NewSecHeader.sh_size = SectionHeader->VirtualSize;

      // Section alignment
      if (SectionHeader->Flags & PE_SCN_ALIGN_MASK) {
         NewSecHeader.sh_addralign = uint32(1 << (((SectionHeader->Flags & PE_SCN_ALIGN_MASK) / PE_SCN_ALIGN_1) - 1));
      }

      // Put section header into temporary buffer
      NewSectionHeaders[newsec] = NewSecHeader;

      // Increment section number
      newsec++;

      if (SectionHeader->NRelocations > 0) {
         // Source section has relocations. 
         // Make a relocation section in destination file

         // Put data into relocation section header:
         // Initialize to zero
         memset(&NewSecHeader, 0, sizeof(NewSecHeader));

         // Name for relocation section = ".rel" or ".rela" + name of section
         const int MAXSECTIONNAMELENGTH = 256;
         char RelocationSectionName[MAXSECTIONNAMELENGTH] = ".rel";
         if (WordSize == 64) strcat(RelocationSectionName, "a"); // 32-bit: .rel, 64-bit: .rela

         strncat(RelocationSectionName, SecName, MAXSECTIONNAMELENGTH-5);
         RelocationSectionName[MAXSECTIONNAMELENGTH-1] = 0;

         // Put name into section header string table
         uint32 SecNameIndex = NewSections[shstrtab].PushString(RelocationSectionName);

         // Put name into new section header
         NewSecHeader.sh_name = SecNameIndex;

         // Section type
         NewSecHeader.sh_type = (WordSize == 32) ? SHT_REL : SHT_RELA;  // Relocation section

         // Put section header into temporary buffer
         NewSectionHeaders[newsec] = NewSecHeader;

         // Increment section number
         newsec++;
      }
   }
   // Number of sections generated
   NumSectionsNew = newsec;
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CCOF2ELF<ELFSTRUCTURES>::MakeSymbolTable() {
   // Convert subfunction: Make symbol table and string tables
   int isym;                           // current symbol table entry
   int numaux;                         // Number of auxiliary entries in source record
   int OldSectionIndex;                // Index into old section table. 1-based
   int NewSectionIndex;                // Index into new section table. 0-based
   //const int WordSize = sizeof(NewFileHeader.e_entry) * 8; // word size 32 or 64 bits

   TELF_Symbol sym;                    // Temporary symbol table record
   const char * name1;                 // Name of section or main record

   // Pointer to old symbol table
   union {
      SCOFF_SymTableEntry * p;         // Symtab entry pointer
      int8 * b;                        // Used for increment
   } OldSymtab;

   // Make the first record empty
   NewSections[symtab].Push(0, sizeof(TELF_Symbol));

   // Make first string table entries empty
   NewSections[strtab] .PushString("");
   NewSections[stabstr].PushString("");

   // Loop twice through source symbol table to get local symbols first, global symbols last
   // Loop 1: Look for local symbols only
   OldSymtab.p = SymbolTable; // Pointer to source symbol table
   for (isym = 0; isym < this->NumberOfSymbols; isym += numaux+1, OldSymtab.b += SIZE_SCOFF_SymTableEntry*(numaux+1)) {

      if (OldSymtab.b >= Buf() + DataSize) {
         err.submit(2040);
         break;
      }

      // Number of auxiliary records belonging to same symbol
      numaux = OldSymtab.p->s.NumAuxSymbols;  if (numaux < 0) numaux = 0;

      if (OldSymtab.p->s.StorageClass != COFF_CLASS_EXTERNAL && OldSymtab.p->s.StorageClass != COFF_CLASS_WEAK_EXTERNAL) {
         // Symbol is local

         // Reset destination entry
         memset(&sym, 0, sizeof(sym));

         // Binding
         sym.st_bind = STB_LOCAL;

         // Get first aux record if numaux > 0
         //SCOFF_SymTableEntryAux * sa = (SCOFF_SymTableEntryAux *)(OldSymtab.b + SIZE_SCOFF_SymTableEntry);

         // Symbol name
         name1 = this->GetSymbolName(OldSymtab.p->s.Name);

         // Symbol value
         sym.st_value = OldSymtab.p->s.Value;

         // Get section
         OldSectionIndex = OldSymtab.p->s.SectionNumber;  // 1-based index into old section table
         NewSectionIndex = 0;                 // 0-based index into old section table
         if (OldSectionIndex > 0 && OldSectionIndex <= this->NSections) {
            // Subtract 1 from OldSectionIndex because NewSectIndex[] is zero-based while OldSectionIndex is 1-based
            // Get new section index from translation table
            NewSectionIndex = NewSectIndex[OldSectionIndex-1]; 
         }
         if (NewSectionIndex == COFF_SECTION_REMOVE_ME) {
            continue; // Section has been removed. Remove symbol too
         }

         sym.st_shndx = (uint16)NewSectionIndex;

         // Check symbol type
         if (OldSymtab.p->s.StorageClass == COFF_CLASS_FILE) {
            // This is a filename record
            if (numaux > 0 && numaux < 20) {
               // Get filename from subsequent Aux records.
               // Remove path from filename because the path makes no sense on a different platform.
               const char * filename = GetShortFileName(OldSymtab.p);
               // Put file name into string table and debug string table
               sym.st_name = NewSections[strtab].PushString(filename);
               NewSections[stabstr].PushString(filename);
            }
            // Attributes for filename record
            sym.st_shndx  = (uint16)SHN_ABS;
            sym.st_type   = STT_FILE;
            sym.st_bind   = STB_LOCAL;
            sym.st_value  = 0;
         }
         else if (numaux && OldSymtab.p->s.StorageClass == COFF_CLASS_STATIC
         && OldSymtab.p->s.Value == 0 && OldSymtab.p->s.Type != 0x20) {
            // This is a section definition record
            sym.st_name  = 0;  name1 = 0;
            sym.st_type  = STT_SECTION;
            sym.st_bind  = STB_LOCAL;
            sym.st_value = 0;
            // aux record contains length and number of relocations. Ignore aux record
         }
         else if (OldSymtab.p->s.SectionNumber < 0) {
            // This is an absolute or debug symbol
            sym.st_type  = STT_NOTYPE;
            sym.st_shndx = (uint16)SHN_ABS;
         }
         else if (OldSymtab.p->s.Type == 0 && OldSymtab.p->s.StorageClass == COFF_CLASS_FUNCTION) {
            // This is a .bf, .lf, or .ef record following a function record
            // Contains line number information etc. Ignore this record
            continue;
         }
         else if (OldSymtab.p->s.SectionNumber <= 0) {
            // Unknown
            sym.st_type = STT_NOTYPE;
         }
         else {
            // This is a local data definition record
            sym.st_type = STT_OBJECT;
            // The size is not specified in COFF record,
            // so we may give it an arbitrary size:
            // sym.size = 4;
         }

         // Put symbol name into string table if we have not already done so
         if (sym.st_name == 0 && name1) {
            sym.st_name = NewSections[strtab].PushString(name1);
         }

         // Put record into new symbol table
         NewSections[symtab].Push(&sym, sizeof(sym));

         // Insert into symbol translation table
         NewSymbolIndex[isym] = NewSections[symtab].GetLastIndex();

      } // End if not external
   }  // End loop 1

   // Finished with local symbols
   // Make index to first global symbol
   NewSectionHeaders[symtab].sh_info = NewSections[symtab].GetLastIndex() + 1;

   // Loop 2: Look for global symbols only
   OldSymtab.p = SymbolTable; // Pointer to source symbol table
   for (isym = 0; isym < NumberOfSymbols; isym += numaux+1, OldSymtab.b += SIZE_SCOFF_SymTableEntry*(numaux+1)) {

      // Number of auxiliary records belonging to same symbol
      numaux = OldSymtab.p->s.NumAuxSymbols;  if (numaux < 0) numaux = 0;

      if (OldSymtab.p->s.StorageClass == COFF_CLASS_EXTERNAL || OldSymtab.p->s.StorageClass == COFF_CLASS_WEAK_EXTERNAL) {
         // Symbol is global (public or external)

         // Reset destination entry
         memset(&sym, 0, sizeof(sym));

         // Binding
         sym.st_bind = STB_GLOBAL;
         if (OldSymtab.p->s.StorageClass == COFF_CLASS_WEAK_EXTERNAL) sym.st_bind = STB_WEAK;

         // Get first aux record if numaux > 0
         SCOFF_SymTableEntry * sa = (SCOFF_SymTableEntry*)(OldSymtab.b + SIZE_SCOFF_SymTableEntry);

         // Symbol name
         name1 = GetSymbolName(OldSymtab.p->s.Name);

         // Symbol value
         sym.st_value = OldSymtab.p->s.Value;

         // Get section
         OldSectionIndex = OldSymtab.p->s.SectionNumber; // 1-based index into old section table
         NewSectionIndex = 0;                          // 0-based index into old section table
         if (OldSectionIndex > 0 && OldSectionIndex <= NSections) {
            // Subtract 1 from OldSectionIndex because NewSectIndex[] is zero-based while OldSectionIndex is 1-based
            // Get new section index from translation table
            NewSectionIndex = NewSectIndex[OldSectionIndex-1]; 
         }
         if (NewSectionIndex == COFF_SECTION_REMOVE_ME) {
            continue; // Section has been removed. Remove symbol too
         }
         if ((int16)OldSectionIndex == COFF_SECTION_ABSOLUTE) {
            NewSectionIndex = SHN_ABS;
         }

         sym.st_shndx = (uint16)NewSectionIndex;

         // Check symbol type
         if (OldSymtab.p->s.SectionNumber < 0) {
            // This is an absolute or debug symbol
            sym.st_type = STT_NOTYPE;
         }
         else if (OldSymtab.p->s.Type == COFF_TYPE_FUNCTION && OldSymtab.p->s.SectionNumber > 0) {
            // This is a function definition record
            sym.st_type = STT_FUNC;
            if (numaux) {
               // Get size from aux record
               sym.st_size = sa->func.TotalSize;
            }
            if (sym.st_size == 0) {
               // The size is not specified in the COFF file. 
               // We may give it an arbitrary size:
               // sym.size = 1;
            }
         }
         else if (OldSymtab.p->s.SectionNumber <= 0) {
            // This is an external symbol
            sym.st_type = STT_NOTYPE;
         }
         else {
            // This is a data definition record
            sym.st_type = STT_OBJECT;
            // Symbol must have a size. The size is not specified in COFF record,
            // so we just give it an arbitrary size
            sym.st_size = 4;
         }

         // Put symbol name into string table if we have not already done so
         if (sym.st_name == 0 && name1) {
            sym.st_name = NewSections[strtab].PushString(name1);
         }

         // Put record into new symbol table
         NewSections[symtab].Push(&sym, sizeof(sym));

         // Insert into symbol translation table
         NewSymbolIndex[isym] = NewSections[symtab].GetLastIndex();

      } // End if external
   }  // End loop 2
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CCOF2ELF<ELFSTRUCTURES>::MakeRelocationTables() {
   // Convert subfunction: Relocation tables
   int32 oldsec;                                 // Relocated section number in source file
   int32 newsec;                                 // Relocated section number in destination file
   int32 newsecr;                                // Relocation table section number in destination file
   TELF_SectionHeader * NewRelTableSecHeader;    // Section header for new relocation table
   char TempText[32];                            // Temporary text buffer
   const int WordSize = sizeof(NewFileHeader.e_entry) * 8; // word size 32 or 64 bits

   // Loop through source file sections
   for (oldsec = 0; oldsec < NSections; oldsec++) {

      // New section index
      newsec = NewSectIndex[oldsec];
      if (newsec == COFF_SECTION_REMOVE_ME) {
         continue;   // This is a debug or exception handler section which has been removed
      }

      // Pointer to old section header
      SCOFF_SectionHeader * SectionHeader = &this->SectionHeaders[oldsec];

      if (SectionHeader->NRelocations > 0) {
         // This section has relocations

         // Finc new relocation table section
         newsecr = newsec + 1;

         // Check that we have allocated a relocation section
         if (oldsec+1 < this->NSections && NewSectIndex[oldsec+1] == newsecr) err.submit(9000);
         if (newsecr >= NumSectionsNew) err.submit(9000);

         // New relocation table section header
         NewRelTableSecHeader = &NewSectionHeaders[newsecr];

         // Insert header info
         NewRelTableSecHeader->sh_type  = (WordSize == 32) ? SHT_REL : SHT_RELA;
         NewRelTableSecHeader->sh_flags = 0;
         NewRelTableSecHeader->sh_addralign = WordSize / 8; // Alignment
         NewRelTableSecHeader->sh_link = symtab; // Point to symbol table
         NewRelTableSecHeader->sh_info = newsec; // Point to relocated section
         // Entry size:
         NewRelTableSecHeader->sh_entsize = (WordSize == 32) ? sizeof(Elf32_Rel) : sizeof(Elf64_Rela);

         // Pointer to old relocation entry
         union {
            SCOFF_Relocation * p;  // pointer to record
            int8 * b;              // used for address calculation and incrementing
         } OldReloc;

         // Loop through relocations

         OldReloc.b = Buf() + SectionHeader->PRelocations;
         for (int i = 0; i < SectionHeader->NRelocations; i++, OldReloc.b += SIZE_SCOFF_Relocation) {

            // Make new relocation entry and set to zero
            TELF_Relocation NewRelocEntry;
            memset(&NewRelocEntry, 0, sizeof(NewRelocEntry));

            // Section offset of relocated address
            NewRelocEntry.r_offset = OldReloc.p->VirtualAddress;

            // Target symbol
            uint32 TargetSymbol = OldReloc.p->SymbolTableIndex;
            if (TargetSymbol >= (uint32)NumberOfSymbols) {
               err.submit(2031);  // Symbol not in table
            }
            else {  // Translate symbol number
               NewRelocEntry.r_sym = NewSymbolIndex[TargetSymbol];
            }

            if (WordSize == 32) {
               // Interpret 32-bit COFF relocation types
               switch (OldReloc.p->Type) {
               case COFF32_RELOC_ABS:     // Ignored
                  NewRelocEntry.r_type = R_386_NONE;  break;

               case COFF32_RELOC_TOKEN:   // .NET common language runtime token
                  err.submit(2014);       // Error message
                  // Continue in next case and insert absolute address as token:
               case COFF32_RELOC_DIR32:   // 32-bit absolute virtual address
                  NewRelocEntry.r_type = R_386_32;  break;

               case COFF32_RELOC_IMGREL:  // 32-bit image relative address
                  // Image-relative relocation not supported in ELF
                  if (cmd.OutputType == FILETYPE_MACHO_LE) {
                     // Intermediate during conversion to MachO
                     NewRelocEntry.r_type = R_UNSUPPORTED_IMAGEREL;
                     break;
                  }
                  // Work-around unsupported image-relative relocation
                  // Convert to absolute
                  NewRelocEntry.r_type = R_386_32; // Absolute relocation
                  if (cmd.ImageBase == 0) {
                     // Default image base for 32-bit Linux
                     cmd.ImageBase = 0x8048000; // 0x400000 ?
                  }
                  NewRelocEntry.r_addend -= cmd.ImageBase;
                  // Warn that image base must be set to the specified value
                  sprintf(TempText, "%X", cmd.ImageBase); // write value as hexadecimal
                  err.submit(1301, TempText);  err.ClearError(1301);
                  break;

               case COFF32_RELOC_REL32:   // 32-bit self-relative
                  NewRelocEntry.r_type = R_386_PC32;
                  // Difference between EIP-relative and self-relative relocation = size of address field
                  NewRelocEntry.r_addend = -4;  break; 
                  /* !! error  if self-relative relocation with offset
                   !! test data that fails = testpic32.obj */

               case COFF32_RELOC_SECTION:   // 16-bit section index in file
               case COFF32_RELOC_SECREL:    // 32-bit section-relative
               case COFF32_RELOC_SECREL7:   //  8-bit section-relative
                  // These fixup types are not supported in ELF files
                  if (cmd.DebugInfo != CMDL_DEBUG_STRIP) {
                     // Issue warning. Ignore if stripping debug info
                     err.submit(1010);
                  }
                  break;
                  
               default:
                  err.submit(2030, OldReloc.p->Type);  break; // Error: Unknown relocation type (%i) ignored
               }
            }
            else {
               // Interpret 64-bit COFF relocation types
               switch (OldReloc.p->Type) {
               case COFF64_RELOC_ABS:        // Ignored
                  NewRelocEntry.r_type = R_X86_64_NONE;  break;

               case COFF64_RELOC_TOKEN:      // .NET common language runtime token
                  err.submit(2014);          // Error message
                  // Continue in next case and insert absolute address as token:

               case COFF64_RELOC_ABS64:      // 64 bit absolute virtual address
                  NewRelocEntry.r_type = R_X86_64_64;  break;

               case COFF64_RELOC_PPC_TOKEN:
                  err.submit(2014);          // Error message
                  // Continue in next case and insert absolute address as token:

               case COFF64_RELOC_ABS32:      // 32 bit absolute address
                  NewRelocEntry.r_type = R_X86_64_32S;  break;

               case COFF64_RELOC_IMGREL:     // 32 bit image-relative
                  // Image-relative relocation not supported in ELF
                  if (cmd.OutputType == FILETYPE_MACHO_LE) {
                     // Intermediate during conversion to MachO
                     NewRelocEntry.r_type = R_UNSUPPORTED_IMAGEREL;
                     break;
                  }
                  // Work-around unsupported image-relative relocation
                  // Convert to absolute
                  NewRelocEntry.r_type = R_X86_64_32S; // Absolute 32-bit relocation
                  if (cmd.ImageBase == 0) {
                     // Default image base for 64-bit Linux
                     cmd.ImageBase = 0x400000;
                  }
                  NewRelocEntry.r_addend -= cmd.ImageBase;
                  // Warn that image base must be set to the specified value
                  sprintf(TempText, "%X", cmd.ImageBase); // write value as hexadecimal
                  err.submit(1301, TempText);  err.ClearError(1301);
                  break;

               case COFF64_RELOC_REL32:      // 32 bit, RIP-relative
               case COFF64_RELOC_REL32_1:    // 32 bit, relative to RIP - 1. For instruction with immediate byte operand
               case COFF64_RELOC_REL32_2:    // 32 bit, relative to RIP - 2. For instruction with immediate word operand
               case COFF64_RELOC_REL32_3:    // 32 bit, relative to RIP - 3. (useless)
               case COFF64_RELOC_REL32_4:    // 32 bit, relative to RIP - 4. For instruction with immediate dword operand
               case COFF64_RELOC_REL32_5:    // 32 bit, relative to RIP - 5. (useless)
                  NewRelocEntry.r_type = R_X86_64_PC32;
                  // Note:
                  // The microprocessor calculates RIP-relative addresses 
                  // relative to the value of the instruction pointer AFTER 
                  // the instruction. This is equal to the address of the 
                  // relocated field plus the size of the relocated field 
                  // itself plus the size of any immediate operand coming 
                  // after the relocated field.
                  // The COFF format makes the correction for this offset in 
                  // the linker by using a differet relocation type for 
                  // immediate operand size = 0, 1, 2 or 4.
                  // The ELF format makes the same correction by an explicit 
                  // addend, which is -4, -5, -6 or -8, respectively.
                  // The difference between RIP-relative and self-relative 
                  // relocation is equal to the size of the address field plus
                  // the size of any immediate operand:
                  NewRelocEntry.r_addend = -(4 + OldReloc.p->Type - COFF64_RELOC_REL32);                  
                  break;

               case COFF64_RELOC_SECTION:   // 16-bit section index in file
               case COFF64_RELOC_SECREL:    // 32-bit section-relative
               case COFF64_RELOC_SECREL7:   //  8-bit section-relative

                  // These fixup types are not supported in ELF files
                  if (cmd.DebugInfo != CMDL_DEBUG_STRIP) {
                     // Issue warning. Ignore if stripping debug info
                     err.submit(1010);
                  }
                  break; 

               default:
                  err.submit(2030, OldReloc.p->Type);  break; // Error: Unknown relocation type (%i) ignored
               }
            }

            // Find inline addend
            int32 * paddend = 0;
            if (OldReloc.p->VirtualAddress + 4 > NewSections[newsec].GetDataSize()
               || NewSectionHeaders[newsec].sh_type == SHT_NOBITS) {
                  // Address of relocation is invalid
                  err.submit(2032);
               }
            else {
               // Make pointer to inline addend
               paddend = (int32*)(NewSections[newsec].Buf() 
                  + NewSectionHeaders[newsec].sh_offset + OldReloc.p->VirtualAddress);
            } 

            // Put relocation record into table
            if (WordSize == 32) {
               if (NewRelocEntry.r_addend != 0) {
                  // Use inline addends in 32 bit ELF (SHT_REL)
                  // Put addend inline
                  * paddend += uint32(NewRelocEntry.r_addend);
                  NewRelocEntry.r_addend = 0;
               }

               // Save 32-bit relocation record Elf32_Rel, not Elf32_Rela
               if (NewRelocEntry.r_addend) err.submit(9000);
               NewSections[newsecr].Push(&NewRelocEntry, sizeof(Elf32_Rel));
            }
            else {
               // 64 bit
               /*
               if (*paddend != 0) {
                  // Use explicit addend in 64 bit ELF (SHT_RELA)
                  // Explicit addend may cause link error if it appears to point outside section
                  NewRelocEntry.r_addend += *paddend;
                  *paddend = 0;
               }*/

               // Save 64-bit relocation record. Must be Elf64_Rela
               NewSections[newsecr].Push(&NewRelocEntry, sizeof(Elf64_Rela));
            }
         }
      }
   }
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CCOF2ELF<ELFSTRUCTURES>::MakeBinaryFile() {
   // Convert subfunction: Make section headers and file header,
   // and combine everything into a single memory buffer.
   int32  newsec;              // Section index
   uint32 SecOffset;           // Section offset in file
   uint32 SecSize;             // Section size in file
   uint32 SectionHeaderOffset; // File offset to section headers

   // Set file type in ToFile
   ToFile.SetFileType(FILETYPE_ELF);
   
   // Make space for file header in ToFile, but don't fill data into it yet
   ToFile.Push(0, sizeof(TELF_Header));

   // Loop through new section buffers
   for (newsec = 0; newsec < NumSectionsNew; newsec++) {

      // Size of section
      SecSize = NewSections[newsec].GetDataSize();

      // Put section into ToFile
      SecOffset = ToFile.Push(NewSections[newsec].Buf(), SecSize);

      // Put size and offset into section header
      NewSectionHeaders[newsec].sh_offset = SecOffset;
      NewSectionHeaders[newsec].sh_size   = SecSize;

      // Align before next entry
      ToFile.Align(16);
   }

   // Start offset of section headers
   SectionHeaderOffset = ToFile.GetDataSize();

   // Loop through new section headers
   for (newsec = 0; newsec < NumSectionsNew; newsec++) {

      // Put section header into ToFile
      ToFile.Push(&NewSectionHeaders[newsec], sizeof(TELF_SectionHeader));
   }

   // Make file header
   TELF_Header FileHeader;
   memset(&FileHeader, 0, sizeof(FileHeader)); // Initialize to 0

   // Put file type magic number in
   strcpy((char*)(FileHeader.e_ident), ELFMAG);
   // File class
   FileHeader.e_ident[EI_CLASS] = (WordSize == 32) ? ELFCLASS32 : ELFCLASS64;
   // Data Endian-ness
   FileHeader.e_ident[EI_DATA] = ELFDATA2LSB;
   // ELF version
   FileHeader.e_ident[EI_VERSION] = EV_CURRENT;
   // ABI
   FileHeader.e_ident[EI_OSABI] = ELFOSABI_SYSV;
   // ABI version
   FileHeader.e_ident[EI_ABIVERSION] = 0;
   // File type
   FileHeader.e_type = ET_REL;
   // Machine architecture
   FileHeader.e_machine = (WordSize == 32) ? EM_386 : EM_X86_64;
   // Version
   FileHeader.e_version = EV_CURRENT;
   // Flags
   FileHeader.e_flags = 0;

   // Section header table offset
   FileHeader.e_shoff = SectionHeaderOffset;

   // File header size
   FileHeader.e_ehsize = sizeof(TELF_Header);

   // Section header size
   FileHeader.e_shentsize = sizeof(TELF_SectionHeader);

   // Number of section headers
   FileHeader.e_shnum = (uint16)NumSectionsNew;

   // Section header string table index
   FileHeader.e_shstrndx = (uint16)shstrtab;

   // Put file header into beginning of ToFile where we made space for it
   memcpy(ToFile.Buf(), &FileHeader, sizeof(FileHeader));
}


// Make template instances for 32 and 64 bits
template class CCOF2ELF<ELF32STRUCTURES>;
template class CCOF2ELF<ELF64STRUCTURES>;
