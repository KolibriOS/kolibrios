/****************************  mac2elf.cpp   *********************************
* Author:        Agner Fog
* Date created:  2008-05-15
* Last modified: 2009-05-19
* Project:       objconv
* Module:        mac2elf.cpp
* Description:
* Module for converting Mach-O file to ELF file
*
* Copyright 2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#include "stdafx.h"


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::CMAC2ELF () {
   // Constructor
   memset(this, 0, sizeof(*this));                    // Reset everything
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::Convert() {
   // Do the conversion
   // Some compilers require this-> for accessing members of template base class,
   // according to the so-called two-phase lookup rule.

   NumSectionsNew = 5;                                    // Number of sections generated so far

   // Allocate variable size buffers
   MaxSectionsNew = NumSectionsNew + 2 * this->NumSections + 2;// Max number of sections needed
   NewSections.SetNum(MaxSectionsNew+1);                  // Allocate buffers for each section
   NewSections.SetZero();                                 // Initialize
   NewSectionHeaders.SetNum(MaxSectionsNew+1);            // Allocate array for temporary section headers
   NewSectionHeaders.SetZero();                           // Initialize
   NewSectIndex.SetNum(this->NumSections+1);              // Array for translating old section index to new section index
   NewSectIndex.SetZero();                                // Initialize
   SectionSymbols.SetNum(this->MaxSectionsNew+1);         // Array of new symbol indices for sections
   SectionSymbols.SetZero();                              // Initialize
   NewSymbolIndex.SetNum(this->SymTabNumber);             // Array of new symbol indices
   NewSymbolIndex.SetZero();                              // Initialize

   // Call the subfunctions
   ToFile.SetFileType(FILETYPE_ELF);       // Set type of to file
   MakeSegments();                         // Make segment headers and code/data segments
   MakeSymbolTable();                      // Symbol table and string tables
   MakeRelocationTables(this->FileHeader); // Make relocation tables
   MakeImportTables();                     // Fill import tables
   MakeGOT();                              // Make fake Global Offset Table
   MakeBinaryFile();                       // Putting sections together
   *this << ToFile;                        // Take over new file buffer
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeSegments() {
   // Convert subfunction: Make segment headers and code/data segments
   TELF_SectionHeader NewSecHeader; // New section header
   uint32 oldsec;                   // Section index in old file
   uint32 newsec;                   // Section index in new file
   uint32 SecNameIndex;             // Section name index into shstrtab
   char const * SecName;            // Name of new section
   const int MAXSECTIONNAMELENGTH = 256;
   char RelocationSectionName[MAXSECTIONNAMELENGTH];
   const int WordSize = sizeof(MInt) * 8;

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
   const uint32 NumSpecialSegments = sizeof(SpecialSegmentNames)/sizeof(SpecialSegmentNames[0]);

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
      // the constructor to equal the number of entries in 
      // SpecialSegmentNames, including the Null segment
      err.submit(9000);
   }

   // Find sections in old file
   uint32 icmd;                        // Current load command
   uint32 command;                     // Load command
   uint32 cmdsize = 0;                 // Command size

   // Pointer to current position in old file
   uint8 * currentp = (uint8*)(this->Buf() + sizeof(TMAC_header));

   // Loop through file commands
   for (icmd = 1; icmd <= this->FileHeader.ncmds; icmd++, currentp += cmdsize) {
      command = ((MAC_load_command*)currentp) -> cmd;
      cmdsize = ((MAC_load_command*)currentp) -> cmdsize;

      if (command == MAC_LC_SEGMENT || command == MAC_LC_SEGMENT_64) {
         // This is the segment command (there should be only one)
         if ((command == MAC_LC_SEGMENT) ^ (WordSize == 32)) {
            // 32-bit segment in 64-bit file or vice versa
            err.submit(2320);  return;
         }
         if (cmdsize < sizeof(TMAC_segment_command)) {
            // Zero cmdsize or too small
            err.submit(2321); return;
         }
         // Point to segment command
         TMAC_segment_command * sh = (TMAC_segment_command*)currentp;

         if (stricmp(sh->segname, MAC_SEG_OBJC) == 0) {
            // objective-C runtime segment
            err.submit(2021);  continue;
         }

         // Find first section header
         TMAC_section * sectp = (TMAC_section*)(currentp + sizeof(TMAC_segment_command));

         // Loop through section headers
         for (oldsec = 1; oldsec <= this->NumSections; oldsec++, sectp++) {

            // Get section name
            SecName = sectp->sectname;

            // Check for special section names
            if (stricmp(SecName,"__eh_frame") == 0) {
               // This is an exception handler section
               if (cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
                  // Remove exception handler section
                  cmd.CountExceptionRemoved();
                  continue;
               }
               else if (cmd.InputType != cmd.OutputType) {
                  err.submit(1030); // Warn that exception information is incompatible
               }
            }
            if (sectp->flags & MAC_S_ATTR_DEBUG) {
               // This section has debug information
               if (cmd.DebugInfo == CMDL_DEBUG_STRIP) {
                  // Remove debug info
                  cmd.CountDebugRemoved();
                  continue;
               }
               else if (cmd.InputType != cmd.OutputType) {
                  err.submit(1029); // Warn that debug information is incompatible
               }
            }

            // Store section index in index translation table
            NewSectIndex[oldsec] = newsec;

            // Store section data
            if (sectp->size > 0 && !((sectp->flags & MAC_SECTION_TYPE) == MAC_S_ZEROFILL || (sectp->flags & MAC_SECTION_TYPE)==MAC_S_GB_ZEROFILL)) {
               NewSections[newsec].Push(this->Buf()+sectp->offset, uint32(sectp->size));
            }

            // Put data into new section header:
            // Initialize to zero
            memset(&NewSecHeader, 0, sizeof(NewSecHeader));

            uint32 type = sectp->flags & MAC_SECTION_TYPE;
            uint32 attributes = sectp->flags & MAC_SECTION_ATTRIBUTES;

            // Section type
            if (type == MAC_S_ZEROFILL || type == MAC_S_GB_ZEROFILL) {
               // BSS section
               NewSecHeader.sh_type = SHT_NOBITS;    // BSS
            }
            else {
               // Normal code or data section
               NewSecHeader.sh_type = SHT_PROGBITS;  // Program code or data
            }

            // Section flags
            NewSecHeader.sh_flags |= SHF_ALLOC;      // Occupies memory during execution
            if (attributes & (MAC_S_ATTR_SOME_INSTRUCTIONS | MAC_S_ATTR_PURE_INSTRUCTIONS)) {
               // Executable
               NewSecHeader.sh_flags |= SHF_EXECINSTR;
            }
            else {
               switch (type) {
               case MAC_S_CSTRING_LITERALS:
               case MAC_S_4BYTE_LITERALS:
               case MAC_S_8BYTE_LITERALS:
               case MAC_S_16BYTE_LITERALS:
               case MAC_S_LITERAL_POINTERS:
                  // not writeable
                  break;
               default:
                  // writeable
                  NewSecHeader.sh_flags |= SHF_WRITE;
                  break;
               }
            }

            // Check for special sections
            if (strcmp(SecName, MAC_CONSTRUCTOR_NAME) == 0) {
               // Constructors segment
               SecName = ELF_CONSTRUCTOR_NAME;
               NewSecHeader.sh_flags = SHF_WRITE | SHF_ALLOC;
            }

            // Put name into section header string table
            SecNameIndex = NewSections[shstrtab].PushString(SecName);

            // Put name into new section header
            NewSecHeader.sh_name = SecNameIndex;

            // Section virtual memory address
            NewSecHeader.sh_addr = sectp->addr;

            // Section size in memory
            NewSecHeader.sh_size = sectp->size;

            // Section alignment
            NewSecHeader.sh_addralign = uint32(1 << sectp->align);

            // Put section header into temporary buffer
            NewSectionHeaders[newsec] = NewSecHeader;

            // Increment section number
            newsec++;

            // Check if section is import table
            int SectionType   = sectp->flags & MAC_SECTION_TYPE;
            int IsImportTable = SectionType >= MAC_S_NON_LAZY_SYMBOL_POINTERS && SectionType <= MAC_S_SYMBOL_STUBS;

            if (sectp->nreloc > 0 || IsImportTable) {
               // Source section has relocations. 
               // Make a relocation section in destination file

               // Put data into relocation section header:
               // Initialize to zero
               memset(&NewSecHeader, 0, sizeof(NewSecHeader));

               // Name for relocation section = ".rel" or ".rela" + name of section
               if (WordSize == 32) {
                  strcpy(RelocationSectionName, ".rel");
               }
               else {
                  strcpy(RelocationSectionName, ".rela");
               }
               strncat(RelocationSectionName, SecName, MAXSECTIONNAMELENGTH-5);
               RelocationSectionName[MAXSECTIONNAMELENGTH-1] = 0;

               // Put name into section header string table
               uint32 SecNameIndex = NewSections[shstrtab].PushString(RelocationSectionName);

               // Put name into new section header
               NewSecHeader.sh_name = SecNameIndex;

               // Section type
               NewSecHeader.sh_type = (WordSize == 32) ? SHT_REL : SHT_RELA;  // Relocation section

               // Entry size
               NewSecHeader.sh_entsize = (WordSize == 32) ? sizeof(Elf32_Rel) : sizeof(Elf64_Rela);  // Relocation section

               // Section alignment
               NewSecHeader.sh_addralign = WordSize / 8;

               // Link to the section it relocates for
               NewSecHeader.sh_info = newsec - 1;

               // Put section header into temporary buffer
               NewSectionHeaders[newsec] = NewSecHeader;

               // Increment section number
               newsec++;

               // Check if there are any GOT relocations
               // Pointer to old relocation entry
               if (sectp->reloff >= this->GetDataSize()) {err.submit(2035); break;}
               MAC_relocation_info * relp = (MAC_relocation_info*)(this->Buf() + sectp->reloff);
               // Loop through old relocations
               for (uint32 oldr = 1; oldr <= sectp->nreloc; oldr++, relp++) {
                  uint32 RType = relp->r_type;         // relocation type
                  // No scattered relocation in 64-bit mode. GOT only in 64-bit mode
                  if (WordSize == 64 && (RType == MAC64_RELOC_GOT_LOAD || RType == MAC64_RELOC_GOT)) {
                     HasGOT++;
                  }
               }
            }
         }

         // Check if GOT needed
         if (HasGOT && WordSize == 64) {
            // Make a fake Global Offset Table
            FakeGOTSection = newsec;

            // Put name and data into section header
            memset(&NewSecHeader, 0, sizeof(NewSecHeader));
            SecNameIndex = NewSections[shstrtab].PushString("_fakeGOT");
            NewSecHeader.sh_name = SecNameIndex;
            NewSecHeader.sh_type = SHT_PROGBITS; // Type
            NewSecHeader.sh_flags = SHF_ALLOC;   // Flags
            NewSecHeader.sh_addralign = 8;       // Alignment

            // Put section header into temporary buffer
            NewSectionHeaders[newsec++] = NewSecHeader;

            // Make relocation section for fake GOT
            memset(&NewSecHeader, 0, sizeof(NewSecHeader));
            // Put name and data into section header
            SecNameIndex = NewSections[shstrtab].PushString("_rela.fakeGOT");
            NewSecHeader.sh_name = SecNameIndex;
            NewSecHeader.sh_type = SHT_RELA;     // Type
            NewSecHeader.sh_flags = 0;           // Flags
            NewSecHeader.sh_addralign = 8;       // Alignment
            NewSecHeader.sh_entsize = sizeof(Elf64_Rela);  // Entry size
            NewSecHeader.sh_info = newsec - 1;   // Link to the section it relocates for
            NewSecHeader.sh_link = symtab;       // Link to symbol table
            // Put section header into temporary buffer
            NewSectionHeaders[newsec++] = NewSecHeader;
         }
         // Number of sections generated
         NumSectionsNew = newsec;
      }
   }
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeSymbolTable() {
   // Convert subfunction: Make symbol table and string tables
   uint32 isym;            // current old symbol table entry
   uint32 OldSectionIndex; // Index into old section table. 1-based
   uint32 NewSectionIndex; // Index into new section table. 0-based
   const char * name1;     // Name of symbol
   TELF_Symbol sym;        // Temporary symbol table record
   uint32 DebugRemoved = 0;// Debug symbols removed

   // pointer to old string table
   char * oldstringtab = (char*)(this->Buf() + this->StringTabOffset); 

   // pointer to old symbol table
   TMAC_nlist * symp0, *symp;
   symp0 = (TMAC_nlist*)(this->Buf() + this->SymTabOffset);

   // Check within range
   if (this->SymTabOffset + this->SymTabNumber * sizeof(TMAC_nlist) > this->DataSize) {
      err.submit(2040);  return;
   }

   // Make the first symbol record empty
   NewSections[symtab].Push(0, sizeof(TELF_Symbol));

   // Make first string table entries empty
   NewSections[strtab] .PushString("");
   NewSections[stabstr].PushString("");

   // Make symbol records for the start of each section in case they are needed
   // by section-relative relocations (r_extern = 0 in MAC_relocation_info)
   for (uint32 sec = 1; sec < NumSectionsNew; sec++) {
      uint32 type = NewSectionHeaders[sec].sh_type;
      if (type == SHT_PROGBITS || type == SHT_NOBITS) {
         // Make unnamed symbol table entry for this section
         memset(&sym, 0, sizeof(sym));
         sym.st_shndx = sec;
         sym.st_type = STT_SECTION;
         // Put record into new symbol table
         NewSections[symtab].Push(&sym, sizeof(sym));
         // Insert into section symbol translation table
         SectionSymbols[sec] = NewSections[symtab].GetLastIndex();
      }
   }

   // Loop through old symbol table. Local symbols first, global symbols last
   for (isym = 0, symp = symp0; isym < this->SymTabNumber; isym++, symp++) {

      if ((symp->n_type & MAC_N_STAB) && (cmd.DebugInfo & CMDL_DEBUG_STRIP)) {
         // Debug symbol should be removed
         DebugRemoved++;  continue;
      }

      // Reset destination entry
      memset(&sym, 0, sizeof(sym));

      // Get binding
      if (isym < this->iextdefsym) {
         // Local
         sym.st_bind = STB_LOCAL;
      }
      else if (symp->n_desc & (MAC_N_WEAK_REF | MAC_N_WEAK_DEF)) {
         // Weak public or weak external
         sym.st_bind = STB_WEAK;
      }
      else {
         // Global (public or external)
         sym.st_bind = STB_GLOBAL;
      }

      // Symbol name
      if (symp->n_strx < this->StringTabSize) {
         name1 = oldstringtab + symp->n_strx;
      }
      else {
         err.submit(2112);  break;
      }
         
      // Symbol value
      sym.st_value = symp->n_value;
         
      // Get section
      OldSectionIndex = symp->n_sect;
      if (OldSectionIndex > this->NumSections) {
         err.submit(2016); break;
      }
      // Get new section index
      NewSectionIndex = 0;
      if (OldSectionIndex > 0) {
         // Get new section index from translation table
         NewSectionIndex = NewSectIndex[OldSectionIndex];
         // Change symbol address to section-relative
         // (Also in 64-bit mode)
         sym.st_value -= NewSectionHeaders[NewSectionIndex].sh_addr;
      }
      sym.st_shndx = (uint16)NewSectionIndex;

      if (OldSectionIndex && !NewSectionIndex) {
         // Section has been removed. Remove symbol also
         continue;
      }

      // Check symbol type
      int32 RefType = symp->n_desc & MAC_REF_TYPE;
      if (RefType == MAC_REF_FLAG_UNDEFINED_LAZY || RefType == MAC_REF_FLAG_PRIVATE_UNDEFINED_LAZY) {
         // Lazy binding
         err.submit(1061, name1);
      }
      else if ((symp->n_type & MAC_N_TYPE) == MAC_N_ABS) {
         // Absolute symbol
         sym.st_type  = STT_NOTYPE;
         sym.st_shndx = (uint16)SHN_ABS;
         if (sym.st_bind == STB_LOCAL) {
            continue; // Remove absolute local symbol (not allowed in COFF)
         }
      }
      else if (sym.st_shndx == 0) {  // added by Vladimir 'phcoder' Serbinenko:
         // This is an external
         sym.st_type = STT_NOTYPE;
      }		  
      else {
         // This is a data definition record
         if (NewSectionHeaders[NewSectionIndex].sh_flags & SHF_EXECINSTR) {
            // Code section, assume this is a function
            sym.st_type = STT_FUNC;
         }
         else {
            // This is a data object
            sym.st_type = STT_OBJECT;
         }
         if (sym.st_bind == STB_GLOBAL && NewSectionIndex) {
            // Symbol is public
            // The size is not specified in Mac record,
            // so we may give it an arbitrary size:
            sym.st_size = 4;
         }
      }

      // Put symbol name into string table
      if (name1 && *name1) {
         sym.st_name = NewSections[strtab].PushString(name1);
      }

      // Put record into new symbol table
      NewSections[symtab].Push(&sym, sizeof(sym));

      // Insert into symbol translation table
      NewSymbolIndex[isym] = NewSections[symtab].GetLastIndex();

      // Make index to first global symbol
      if (isym >= this->iextdefsym && !NewSectionHeaders[symtab].sh_info) {
         // This is the first global symbol
         NewSectionHeaders[symtab].sh_info = NewSymbolIndex[isym];
      }
   }
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeRelocationTables(MAC_header_32&) {
   // Convert subfunction: Relocation tables, 32-bit version

   uint32 oldsec;        // Relocated section number in source file
   uint32 newsec;        // Relocated section number in destination file
   uint32 newsecr;       // Relocation table section number in destination file
   MInt   SectAddr;      // Section address of relocation source
   MInt   SourceAddress; // Address of relocation source including section address
   MInt   TargetAddress; // Target address including section address
   uint32 TargetSection; // New section index of relocation target
   uint32 TargetOffset;  // Section-relative offset of relocation target
   uint32 RefAddress;    // Reference point address including section address
   uint32 RefSection;    // New section index of reference point
   uint32 RefOffset;     // Section-relative offset of reference point
   int32 * inlinep = 0;  // Pointer to inline addend
   //const int WordSize = sizeof(MInt) * 8;

   TELF_SectionHeader * NewRelTableSecHeader;  // Section header for new relocation table

   // Number of symbols
   uint32 NumSymbols = NewSections[symtab].GetNumEntries();

   // New symbol table
   //Elf32_Sym * NewSymbolTable = (Elf32_Sym *)(NewSections[symtab].Buf());

   // Find first section header
   MAC_section_32 * sectp = (MAC_section_32*)(this->Buf() + this->SectionHeaderOffset);

   // Loop through section headers
   for (oldsec = 1; oldsec <= this->NumSections; oldsec++, sectp++) {

      if (sectp->nreloc > 0) {
         // Source section has relocations

         // New section index
         newsec = NewSectIndex[oldsec];

         // Check that section has not been deleted
         if (newsec > 0) {

            // Section address
            SectAddr = NewSectionHeaders[newsec].sh_addr;

            // Finc new relocation table section
            newsecr = newsec + 1;
            if (newsecr >= NewSectionHeaders.GetNumEntries()) {
               err.submit(9000); return;}

            // New relocation table section header
            NewRelTableSecHeader = &NewSectionHeaders[newsecr];

            // Check that we have allocated this as a relocation section
            if (NewRelTableSecHeader->sh_info != newsec) {
               err.submit(9000); return;
            }

            // Insert header info
            NewRelTableSecHeader->sh_type  = SHT_REL;
            NewRelTableSecHeader->sh_flags = 0;
            NewRelTableSecHeader->sh_addralign = 4;
            NewRelTableSecHeader->sh_link = symtab; // Point to symbol table
            NewRelTableSecHeader->sh_info = newsec; // Point to relocated section
            NewRelTableSecHeader->sh_entsize = sizeof(Elf32_Rel); // Entry size:

            // Pointer to old relocation entry
            if (sectp->reloff >= this->GetDataSize()) {err.submit(2035); break;}
            MAC_relocation_info * relp = (MAC_relocation_info*)(this->Buf() + sectp->reloff);

            // Loop through old relocations
            for (uint32 oldr = 1; oldr <= sectp->nreloc; oldr++, relp++) {

               // Make new relocation entry and set to zero
               Elf32_Rel NewRelocEntry;

               memset(&NewRelocEntry, 0, sizeof(NewRelocEntry));

               if (relp->r_address & R_SCATTERED) {
                  // scattered relocation into
                  MAC_scattered_relocation_info * scatp = (MAC_scattered_relocation_info*)relp;

                  // Address of source
                  NewRelocEntry.r_offset = scatp->r_address;
                  if (NewRelocEntry.r_offset >= NewSections[newsec].GetDataSize()) {
                     err.submit(2035); continue; // Out of range
                  }
                  // Pointer to inline addend
                  inlinep = (int32*)(NewSections[newsec].Buf() + NewRelocEntry.r_offset);
                  if (scatp->r_pcrel) {
                     // Self-relative scattered
                     if (scatp->r_type != MAC32_RELOC_VANILLA) {
                        err.submit(2030, scatp->r_type); continue; // Unexpected type
                     }
                     // Scattered, self-relative, vanilla
                     // Note: I have never seen this relocation method, so I have not
                     // been able to test it. I don't know for sure how it works and 
                     // the documentation is poor.
                     SourceAddress = SectAddr + scatp->r_address;

                     // Target address
                     TargetAddress = SourceAddress + *inlinep;
                     TranslateAddress(TargetAddress, TargetSection, TargetOffset);
                     if (TargetSection == 0) {err.submit(2031); continue;} // not found
                     NewRelocEntry.r_sym = SectionSymbols[TargetSection];
                     if (NewRelocEntry.r_sym == 0) {
                        err.submit(2031); continue; // refers to non-program section
                     }

                     // inline contains full relative address
                     // compensate by subtracting relative address to target section
                     *inlinep -= int32(NewSectionHeaders[TargetSection].sh_addr - SourceAddress);
                     // Relocation type
                     NewRelocEntry.r_type = R_386_PC32;
                  }
                  else if (scatp->r_type == MAC32_RELOC_VANILLA) {
                     // Scattered, absolute
                     TargetAddress = *inlinep;
                     TranslateAddress(TargetAddress, TargetSection, TargetOffset);
                     if (TargetSection == 0) {
                        err.submit(2031); continue;} // Target not found
                     NewRelocEntry.r_sym = SectionSymbols[TargetSection];
                     *inlinep = TargetOffset;
                     NewRelocEntry.r_type = R_386_32;
                     if (scatp->r_length != 2) {
                        err.submit(2030, scatp->r_type); continue; // Only 32-bit supported
                     }
                  }
                  else if (scatp->r_type == MAC32_RELOC_SECTDIFF || scatp->r_type == MAC32_RELOC_LOCAL_SECTDIFF) {
                     // relative to arbitrary reference point
                     // check that next record is MAC32_RELOC_PAIR
                     if (oldr == sectp->nreloc || (scatp+1)->r_type != MAC32_RELOC_PAIR || scatp->r_length != 2) {                              
                        err.submit(2050); continue;
                     }
                     // Find target address and reference point
                     RefAddress = (scatp+1)->r_value;
                     TranslateAddress(RefAddress, RefSection, RefOffset);
                     TargetAddress = RefAddress + *inlinep;
                     TranslateAddress(TargetAddress, TargetSection, TargetOffset);
                     // Check that both points are found
                     if (RefSection == 0 || TargetSection == 0) {
                        err.submit(2031); oldr++; relp++; continue;
                     }
                     // Address relative to arbitrary reference point can be translated
                     // to self-relative address if reference point is in same section as source
                     if (RefSection != newsec) {
                        err.submit(2044); oldr++; relp++; continue;
                     }
                     // Translation is possible
                     // Get symbol for target section
                     NewRelocEntry.r_sym = SectionSymbols[TargetSection];
                     // Make self-relative relocation
                     NewRelocEntry.r_type = R_386_PC32;
                     // Calculate compensating addend
                     *inlinep = TargetOffset + scatp->r_address - RefOffset;
                     // Linker will add (target section) - (source full address) to *inlinep, which gives
                     // (target full address) - (reference point full address)
                     // Advance pointers because we have used two records
                     oldr++; relp++; 
                  }
                  else if (scatp->r_type == MAC32_RELOC_PB_LA_PTR) {
                     // procedure linkage table. Not supported
                     NewRelocEntry.r_type = R_386_PLT32;
                     err.submit(2043);
                  }
                  else {
                     // unknown scattered relocation type
                     err.submit(2030, scatp->r_type); continue;
                  }
               }
               else {
                  // Non scattered relocation info
                  // Section offset of relocated address
                  NewRelocEntry.r_offset = relp->r_address;
                  if (NewRelocEntry.r_offset >= NewSections[newsec].GetDataSize()) {
                     err.submit(2035); continue; // Out of range
                  }
                  // Pointer to inline addend
                  inlinep = (int32*)(NewSections[newsec].Buf() + NewRelocEntry.r_offset);

                  if (relp->r_extern) {
                     // r_extern = 1: target indicated by symbol index
                     uint32 symold = relp->r_symbolnum;
                     if (symold >= this->SymTabNumber) {
                        err.submit(2031); continue; // index out of range
                     }
                     NewRelocEntry.r_sym = NewSymbolIndex[symold];
                     if (relp->r_pcrel) {
                        // Self-relative. 
                        // Inline contains -(source address)
                        // Add (source address) to compensate
                        *inlinep += int32(SectAddr + relp->r_address);
                     }
                  }
                  else {
                     // r_extern = 0. Target indicated by section + offset
                     // Old section number
                     uint32 secold = relp->r_symbolnum;
                     if (secold > this->NumSections) {
                        err.submit(2031); continue; // index out of range
                     }
                     TargetSection = NewSectIndex[secold];
                     NewRelocEntry.r_sym = SectionSymbols[TargetSection];
                     if (NewRelocEntry.r_sym == 0 || NewRelocEntry.r_sym > NumSymbols) {
                        err.submit(2031); continue; // refers to non-program section
                     }
                     if (relp->r_pcrel) {
                        // Self-relative. 
                        // Inline contains (target address)-(source address)
                        // Subtract this to compensate

                        // Target section address
                        TargetOffset = uint32(NewSectionHeaders[TargetSection].sh_addr);
                        SourceAddress = SectAddr + relp->r_address;
                        *inlinep -= int32(TargetOffset - SourceAddress);
                     }
                     else {
                        // Absolute reference
                        // Inline contains target address, convert to section:offset address
                        TranslateAddress(*inlinep, TargetSection, TargetOffset);
                        if (TargetSection == 0) { // Target not found
                           err.submit(2035); continue;
                        }
                        // Translate to section-relative address by subtracting target section address
                        *inlinep -= int32(NewSectionHeaders[TargetSection].sh_addr);
                     }
                  }
                  // relocation type (32-bit non-scattered)
                  switch (relp->r_type) {
                  case MAC32_RELOC_VANILLA:
                     // Normal relocation
                     if (relp->r_pcrel) { // self relative
                        NewRelocEntry.r_type = R_386_PC32;
                     }
                     else { // direct
                        NewRelocEntry.r_type = R_386_32;
                     }
                     break;
                  default:
                     err.submit(2030, relp->r_type); // unknown type
                     continue;
                  }
                  // size
                  if (relp->r_length != 2) { // wrong size
                     err.submit(2030,relp->r_type);
                  }
               }
               // Put relocation record into table
               NewSections[newsecr].Push(&NewRelocEntry, sizeof(NewRelocEntry));
            }
         }
      }
   }
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeRelocationTables(MAC_header_64&) {
   // Convert subfunction: Relocation tables, 64-bit version

   uint32 oldsec;        // Relocated section number in source file
   uint32 newsec;        // Relocated section number in destination file
   uint32 newsecr;       // Relocation table section number in destination file
   uint32 symold;        // Old index of symbol
   uint32 TargetSym;     // Target symbol
   uint32 TargetSection; // New section index of relocation target
   uint32 RefSym;        // Reference symbol
   uint32 RefSection;    // New section index of reference point
   int64  RefOffset;     // Section-relative offset of reference point
   int64  SectAddr;      // Address of current section
   //const int WordSize = sizeof(MInt) * 8;  // Word size, 32 or 64 bits

   TELF_SectionHeader * NewRelTableSecHeader;  // Section header for new relocation table

   // Number of symbols
   //uint32 NumSymbols = NewSections[symtab].GetNumEntries();

   // New symbol table
   Elf64_Sym * NewSymbolTable = (Elf64_Sym *)(NewSections[symtab].Buf());

   // Find first section header
   MAC_section_64 * sectp = (MAC_section_64*)(this->Buf() + this->SectionHeaderOffset);

   // Loop through section headers
   for (oldsec = 1; oldsec <= this->NumSections; oldsec++, sectp++) {

      if (sectp->nreloc > 0) {
         // Source section has relocations

         // New section index
         newsec = NewSectIndex[oldsec];

         // Check that section has not been deleted
         if (newsec > 0) {

            // Section address
            SectAddr = NewSectionHeaders[newsec].sh_addr;

            // Finc new relocation table section
            newsecr = newsec + 1;
            if (newsecr > NewSectionHeaders.GetNumEntries()) {
               err.submit(9000); return;
            }

            // New relocation table section header
            NewRelTableSecHeader = &NewSectionHeaders[newsecr];

            // Check that we have allocated this as a relocation section
            if (NewRelTableSecHeader->sh_info != newsec) {
               err.submit(9000); return;
            }

            // Insert header info
            NewRelTableSecHeader->sh_type  = SHT_RELA;
            NewRelTableSecHeader->sh_flags = 0;
            NewRelTableSecHeader->sh_addralign = 8;
            NewRelTableSecHeader->sh_link = symtab; // Point to symbol table
            NewRelTableSecHeader->sh_info = newsec; // Point to relocated section
            // Entry size:
            NewRelTableSecHeader->sh_entsize = sizeof(Elf64_Rela);

            // Pointer to old relocation entry
            if (sectp->reloff >= this->GetDataSize()) {err.submit(2035); break;}
            MAC_relocation_info * relp = (MAC_relocation_info*)(this->Buf() + sectp->reloff);

            // Loop through old relocations
            for (uint32 oldr = 1; oldr <= sectp->nreloc; oldr++, relp++) {

               // Make new relocation entry and set to zero
               Elf64_Rela NewRelocEntry;
               memset(&NewRelocEntry, 0, sizeof(NewRelocEntry));

               // Pointer to inline addend
               int32 * inlinep = 0;

               if (relp->r_address & R_SCATTERED) {
                  // scattered not allowed in 64-bit
                  err.submit(2030, ((MAC_scattered_relocation_info*)relp)->r_type); continue;
               }
               else {
                  // Non scattered relocation info
                  // Section offset of relocated address
                  NewRelocEntry.r_offset = relp->r_address;
                  if (NewRelocEntry.r_offset >= NewSections[newsec].GetDataSize()) {
                     err.submit(2035); continue; // Out of range
                  }
                  // Pointer to inline addend
                  inlinep = (int32*)(NewSections[newsec].Buf() + NewRelocEntry.r_offset);

                  // Symbol index of target
                  symold = relp->r_symbolnum;
                  if (relp->r_extern) {
                     if (symold >= this->SymTabNumber) {
                        err.submit(2031); continue; // index out of range
                     }
                     NewRelocEntry.r_sym = NewSymbolIndex[symold];
                  }
                  else {
                     // r_extern = 0, r_symbolnum = section
                     if (symold > NumSectionsNew) {err.submit(2031); continue;}
                     TargetSection = NewSectIndex[symold];
                     NewRelocEntry.r_sym = SectionSymbols[TargetSection];
                     if (relp->r_pcrel) {
                        // Self-relative. 
                        // Inline contains (target address)-(source address)
                        // Subtract this to compensate
                        // Target section address
                        uint64 TargetSectAddr = NewSectionHeaders[TargetSection].sh_addr;
                        uint64 SourceAddress = SectAddr + relp->r_address;
                        *inlinep -= int32(TargetSectAddr - SourceAddress);
                        *inlinep += 4; // Compensate for subtracting 4 below
                     }
                  }

                  // Find relocation type
                  switch (relp->r_type) {
                  case MAC64_RELOC_UNSIGNED: // absolute address, 32 or 64 bits
                     if (relp->r_length == 2) {
                        NewRelocEntry.r_type = R_X86_64_32S; // 32 bit signed
                     }
                     else if (relp->r_length == 3) {
                        NewRelocEntry.r_type = R_X86_64_64;  // 64 bit
                     }
                     else {
                        err.submit(2030,relp->r_type); continue;
                     }
                     break;
                  case MAC64_RELOC_SIGNED:   // rip-relative, implicit addend = -4
                  case MAC64_RELOC_BRANCH:   // rip-relative, implicit addend = -4
                  case MAC64_RELOC_SIGNED_1: // implicit addend = -4, not -5
                  case MAC64_RELOC_SIGNED_2: // implicit addend = -4, not -6
                  case MAC64_RELOC_SIGNED_4: // implicit addend = -4, not -8
                     // These are all the same:
                     // signed 32-bit rip-relative with implicit -4 addend
                     if (relp->r_length != 2) { // wrong size
                        err.submit(2030,relp->r_type); continue;
                     }
                     NewRelocEntry.r_type = R_X86_64_PC32;
                     // ELF = self-relative, Mac64 = rip-relative. Compensate for difference
                     *inlinep -= 4;
                     break;
                  case MAC64_RELOC_SUBTRACTOR:
                     // relative to arbitrary reference point
                     // must be followed by a X86_64_RELOC_UNSIGNED
                     // check that next record is MAC64_RELOC_UNSIGNED
                     if (oldr == sectp->nreloc || (relp+1)->r_type != MAC64_RELOC_UNSIGNED) {                              
                        err.submit(2050); continue;
                     }
                     // Reference symbol
                     RefSym     = NewRelocEntry.r_sym;
                     RefSection = NewSymbolTable[RefSym].st_shndx;
                     RefOffset  = NewSymbolTable[RefSym].st_value;

                     // Target symbol
                     symold = (relp+1)->r_symbolnum;
                     if (symold >= this->SymTabNumber) {
                        err.submit(2031); continue; // index out of range
                     }
                     TargetSym = NewSymbolIndex[symold];
                     NewRelocEntry.r_sym = TargetSym;

                     // Address relative to arbitrary reference point can be translated
                     // to self-relative address if reference point is in same section as source
                     if (RefSection != newsec) {
                        err.submit(2044); oldr++; relp++; continue;
                     }
                     if (relp->r_length == 2) {
                        *inlinep += int32(NewRelocEntry.r_offset) - int32(RefOffset);
                     }
                     else if (relp->r_length == 3) {
                        *(int64*)inlinep += NewRelocEntry.r_offset - RefOffset;
                        // there is no 64-bit self-relative relocation in ELF,
                        // use 32-bit self-relative and hope there is no carry
                        err.submit(1302); // Warn. This will fail if inline value changes sign
                     }
                     else {                              
                        err.submit(2044);  // wrong size
                     }
                     // self-relative type
                     NewRelocEntry.r_type = R_X86_64_PC32;

                     // increment counters because we used two records
                     relp++; oldr++;
                     break;

                  case MAC64_RELOC_GOT_LOAD: // a rip-relative load of a GOT entry
                     *inlinep = -4;  
                     // Continue into next case
                  case MAC64_RELOC_GOT:      // other GOT references
                     // Make fake GOT entry
                     //NewRelocEntry.r_addend = MakeGOTEntry(NewRelocEntry.r_sym) - 4;
                     *inlinep += MakeGOTEntry(NewRelocEntry.r_sym);
                     NewRelocEntry.r_sym = FakeGOTSymbol;
                     NewRelocEntry.r_type = R_X86_64_PC32;
                     break;
                  }
               }

               // Put relocation record into table
               NewSections[newsecr].Push(&NewRelocEntry, sizeof(NewRelocEntry));
            }
         }
      }
   }
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeBinaryFile() {
   // Convert subfunction: Make section headers and file header,
   // and combine everything into a single memory buffer.
   uint32 newsec;              // Section index
   uint32 SecOffset;           // Section offset in file
   uint32 SecSize;             // Section size in file
   uint32 SectionHeaderOffset; // File offset to section headers
   const int WordSize = sizeof(MInt) * 8;

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
      if (SecSize) { // Don't set size of BSS sections to zero
         NewSectionHeaders[newsec].sh_size = SecSize;
      }

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

template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeImportTables() {
   // Convert subfunction: Fill import tables
   uint32 oldsec;            // Old section number
   uint32 Type;              // Old section type
   uint32 NumEntries;        // Number of entries in import table
   uint32 EntrySize;         // Entry size of import table
   uint32 NewSec1;           // New section number of import table
   uint32 NewSec2;           // New section number of relocation for import table
   uint32 Offset;            // Offset of relocation source
   uint32 OldSymbol;         // Old symbol number of import
   uint32 i;                 // Loop counter
   uint32 * IndSymTab;       // Pointer to indirect symbol table
   uint32 IndSymi;           // Index into indirect symbol table
   uint32 IndSymNum;         // Number of entries in indirect symbol table
   TELF_Relocation NewRelocEntry; // New relocation entry
   const int WordSize = sizeof(MInt) * 8;

   // Machine code of jmp instruction
   static const int8 JmpInstruction[5] = {int8(0xE9), int8(0xFC), int8(0xFF), int8(0xFF), int8(0xFF)};

   // Number of indirect symbols
   IndSymNum = this->IndirectSymTabNumber;
   if (IndSymNum == 0) {
      return; // No indirect symbols
   }
   // Find indirect symbol table
   IndSymTab = (uint32*)(this->Buf() + this->IndirectSymTabOffset);

   // Find first section header
   TMAC_section * sectp = (TMAC_section*)(this->Buf() + this->SectionHeaderOffset);

   // Loop through section headers
   for (oldsec = 1; oldsec <= this->NumSections; oldsec++, sectp++) {
      // Search for import tables
      Type = sectp->flags & MAC_SECTION_TYPE;
      if (Type >= MAC_S_NON_LAZY_SYMBOL_POINTERS && Type <= MAC_S_SYMBOL_STUBS) {
         // This is an import table

         // Indirect symbol table first entry
         IndSymi = sectp->reserved1;

         // Entry size:
         EntrySize = sectp->reserved2;
         if (EntrySize == 0) EntrySize = WordSize / 8;

         // Find new section
         NewSec1 = NewSectIndex[oldsec];
         NumEntries = uint32(NewSectionHeaders[NewSec1].sh_size) / EntrySize;

         // Find new relocation section
         NewSec2 = NewSec1 + 1;
         if (NewSectionHeaders[NewSec2].sh_type != SHT_REL && NewSectionHeaders[NewSec2].sh_type != SHT_RELA) {
            err.submit(9000); // This should be a relocation section
         }
         NewSectionHeaders[NewSec2].sh_link = symtab; // Point to symbol table

         // Offset of first relocation
         Offset = EntrySize & 1; // 1 if EntrySize = 5, otherwise 0

         // Loop through entries
         for (i = 0; i < NumEntries; i++, Offset += EntrySize) {
            // Find symbol
            if (IndSymi >= IndSymNum) {
               err.submit(1303); break; // Import symbol table exhausted
            }
            OldSymbol = IndSymTab[IndSymi];
            if (OldSymbol >= this->SymTabNumber) {
               err.submit(1052); break;
            }
            // Increment pointer to import symbol table
            IndSymi++;

            // Make relocation record
            memset(&NewRelocEntry, 0, sizeof(NewRelocEntry));
            NewRelocEntry.r_offset = Offset;
            if (WordSize == 32) {
               if (EntrySize == 4) {
                  NewRelocEntry.r_type = R_386_32;
               }
               else if (EntrySize == 5) {
                  NewRelocEntry.r_type = R_386_PC32;
               }
               else {
                  err.submit(2045);
               }
            }
            else { // 64 bit
               if (EntrySize == 8) {
                  NewRelocEntry.r_type = R_X86_64_64;
               }
               else if (EntrySize == 5) {
                  NewRelocEntry.r_type = R_X86_64_PC32;
               }
               else {
                  err.submit(2045);
               }
            }
            NewRelocEntry.r_sym = NewSymbolIndex[OldSymbol];

            // Store relocation record
            NewSections[NewSec2].Push(&NewRelocEntry, (WordSize==32) ? sizeof(Elf32_Rel) : sizeof(Elf64_Rela));

            // Insert jmp instruction if EntrySize = 5
            if (EntrySize == 5) {
               if (Offset -1 + EntrySize > NewSections[NewSec1].GetDataSize()) {
                  err.submit(9000); // Outside section
               }
               memcpy(NewSections[NewSec1].Buf()+Offset-1, JmpInstruction, 5);
            }
         }
      }
   }
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::TranslateAddress(MInt addr, uint32 & section, uint32 & offset) {
   // Translate 32-bit address to section + offset
   // (Sections are not necessarily ordered by address)
   uint32 sec;
   MInt secstart;
   for (sec = 1; sec < NumSectionsNew; sec++) {
      secstart = NewSectionHeaders[sec].sh_addr;
      if (addr >= secstart && addr < secstart + MInt(NewSectionHeaders[sec].sh_size)) {
         // Section found
         section = sec;
         offset = uint32(addr - secstart);
         return;
      }
   }
   // Not found
   section = offset = 0; 
}

template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
uint32 CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeGOTEntry(int symbol) {
   // Make entry in fake GOT for symbol
   uint32 NumGOTEntries = GOTSymbols.GetNumEntries();
   uint32 symi;  // Symbol index
   const int WordSize = sizeof(MInt) * 8;

   // Get symbol for start of GOT
   FakeGOTSymbol = SectionSymbols[FakeGOTSection];
   
   // Search for symbol in previous entries
   for (symi = 0; symi < NumGOTEntries; symi++) {
      if (GOTSymbols[symi] == symbol) break;
   }
   if (symi == NumGOTEntries) {
      // Not found. Make new entry
      GOTSymbols.Push(symbol);
   }
   return symi * (WordSize / 8);
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt,
          class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CMAC2ELF<MACSTRUCTURES,ELFSTRUCTURES>::MakeGOT() {
   // Make fake Global Offset Table
   const int WordSize = sizeof(MInt) * 8;
   if (!HasGOT) return;

   uint32 NumEntries = GOTSymbols.GetNumEntries();
   NewSections[FakeGOTSection].Push(0, NumEntries*(WordSize/8)); 

   // Make relocations for GOT
   Elf64_Rela NewRelocEntry;
   memset(&NewRelocEntry, 0, sizeof(NewRelocEntry));

   for (uint32 i = 0; i < NumEntries; i++) {
      NewRelocEntry.r_offset = i * (WordSize/8);
      NewRelocEntry.r_sym = GOTSymbols[i];
      NewRelocEntry.r_type = R_X86_64_64;
      NewSections[FakeGOTSection+1].Push(&NewRelocEntry, sizeof(NewRelocEntry));
   }
}


// Make template instances for 32 and 64 bits
template class CMAC2ELF<MAC32STRUCTURES,ELF32STRUCTURES>;
template class CMAC2ELF<MAC64STRUCTURES,ELF64STRUCTURES>;
