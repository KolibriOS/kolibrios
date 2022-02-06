/****************************  elf2cof.cpp   *********************************
* Author:        Agner Fog
* Date created:  2006-08-19
* Last modified: 2013-11-27
* Project:       objconv
* Module:        elf2cof.cpp
* Description:
* Module for converting ELF file to PE/COFF file
*
* Copyright 2006-2013 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"
// All functions in this module are templated to make two versions: 32 and 64 bits.
// See instantiations at the end of this file.


// Constructor
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
CELF2COF<ELFSTRUCTURES>::CELF2COF() {
   // Reset all
   memset(this, 0, sizeof(*this));
}


// Convert(): Do the conversion
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2COF<ELFSTRUCTURES>::Convert() {

   // Some compilers require this-> for accessing members of template base class,
   // according to the so-called two-phase lookup rule.

   // Allocate variable size buffers
   NewSectIndex.SetNum(this->NSections);// Allocate section translation table
   NewSectIndex.SetZero();              // Initialize

   // Call the subfunctions
   ToFile.SetFileType(FILETYPE_COFF);  // Set type of to file
   MakeFileHeader();                   // Make file header
   MakeSectionsIndex();                // Make sections index translation table
   MakeSymbolTable();                  // Make symbol table and string tables
   MakeSections();                     // Make sections and relocation tables
   HideUnusedSymbols();                // Hide unused symbols
   MakeBinaryFile();                   // Put sections together
   *this << ToFile;                    // Take over new file buffer
}


// MakeFileHeader(): Convert subfunction to make file header
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2COF<ELFSTRUCTURES>::MakeFileHeader() {
   
   // Make PE file header
   NewFileHeader.Machine = (this->WordSize == 32) ? PE_MACHINE_I386 : PE_MACHINE_X8664;
   NewFileHeader.TimeDateStamp = (uint32)time(0);
   NewFileHeader.SizeOfOptionalHeader = 0;
   NewFileHeader.Flags = 0;

   // Values inserted later:
   NewFileHeader.NumberOfSections = 0;
   NewFileHeader.PSymbolTable = 0;
   NewFileHeader.NumberOfSymbols = 0;

   // Put file header into file
   ToFile.Push(&NewFileHeader, sizeof(NewFileHeader));
}


// MakeSectionsIndex(): Make sections index translation table
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2COF<ELFSTRUCTURES>::MakeSectionsIndex() {
   // We must make this table before the segments, because it is needed for the
   // symbol table, and we must make the symbol table before the relocation table,
   // and we must make the relocation table together with the sections.

   uint32 oldsec;                     // Section number in old file
   uint32 newsec = 0;                 // Section number in new file

   // Loop through old sections
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Get section name
      const char * sname = "";
      uint32 namei = this->SectionHeaders[oldsec].sh_name;
      if (namei >= this->SecStringTableLen) err.submit(2112);
      else sname = this->SecStringTable + namei;

      if (cmd.DebugInfo == CMDL_DEBUG_STRIP) {
         // Check for debug section names
         if (strncmp(sname, ".note",    5) == 0
         ||  strncmp(sname, ".comment", 8) == 0
         ||  strncmp(sname, ".stab",    5) == 0
         ||  strncmp(sname, ".debug",   6) == 0) {
            // Remove this section
            this->SectionHeaders[oldsec].sh_type = SHT_REMOVE_ME;
            cmd.CountDebugRemoved();
         }
      }

      if (cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
         // Check for exception section name
         if (strncmp(sname, ".eh_frame", 9) == 0) {
            // Remove this section
            this->SectionHeaders[oldsec].sh_type = SHT_REMOVE_ME;
            cmd.CountExceptionRemoved();
         }
      }

      // Search for program data sections only
      if (this->SectionHeaders[oldsec].sh_type == SHT_PROGBITS 
      ||  this->SectionHeaders[oldsec].sh_type == SHT_NOBITS) {
         // Section index translation table
         NewSectIndex[oldsec] = newsec++;
      }
      else {
         NewSectIndex[oldsec] = 0;
      }
   }
   // Store number of sections in new file
   NumSectionsNew = newsec;

   // Calculate file offset of raw data
   RawDataOffset = sizeof(SCOFF_FileHeader) + NumSectionsNew * sizeof(SCOFF_SectionHeader);
}


// MakeSections(): Convert subfunction to make sections and relocation tables
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2COF<ELFSTRUCTURES>::MakeSections() {
   uint32 oldsec;                   // Section number in old file
   uint32 relsec;                   // Relocation section in old file
   SCOFF_SectionHeader NewHeader;   // New section header
   TELF_SectionHeader OldHeader;        // Old section header
   TELF_SectionHeader OldRelHeader;     // Old relocation section header
   TELF_Relocation OldRelocation;       // Old relocation table entry
   SCOFF_Relocation NewRelocation;  // New relocation table entry

   // Loop through old sections
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Copy old header for convenience
      OldHeader = this->SectionHeaders[oldsec];

      // Search for program data sections only
      if (OldHeader.sh_type == SHT_PROGBITS || OldHeader.sh_type == SHT_NOBITS) {

         // Reset new section header
         memset(&NewHeader, 0, sizeof(NewHeader));

         // Section name
         const char * sname = "";
         uint32 namei = OldHeader.sh_name;
         if (namei >= this->SecStringTableLen) err.submit(2112);
         else sname = this->SecStringTable + namei;

         // Check for special names
         if (strcmp(sname, ELF_CONSTRUCTOR_NAME)==0) {
            // This is the constructors segment
            sname = COFF_CONSTRUCTOR_NAME;
            OldHeader.sh_flags &= ~ SHF_WRITE;
         }

         // Store name in section header
         COFF_PutNameInSectionHeader(NewHeader, sname, NewStringTable);

         // Raw data
         NewHeader.SizeOfRawData = uint32(OldHeader.sh_size);  // section size in file
         if (OldHeader.sh_size && OldHeader.sh_type != SHT_NOBITS) {
            // File  to raw data for section
            NewHeader.PRawData = NewRawData.GetDataSize() + RawDataOffset;

            // Copy raw data
            NewRawData.Push(this->Buf()+(uint32)(OldHeader.sh_offset), (uint32)(OldHeader.sh_size)); 
            NewRawData.Align(4);
         }

         // Section flags
         NewHeader.Flags = PE_SCN_MEM_READ;
         if (OldHeader.sh_flags & SHF_WRITE) NewHeader.Flags |= PE_SCN_MEM_WRITE;
         if (OldHeader.sh_flags & SHF_EXECINSTR) {
            NewHeader.Flags |= PE_SCN_MEM_EXECUTE | PE_SCN_CNT_CODE;
         }
         else {
            NewHeader.Flags |= (OldHeader.sh_type == SHT_PROGBITS) ? 
            PE_SCN_CNT_INIT_DATA : PE_SCN_CNT_UNINIT_DATA;
         }
         // Alignment
         int NewAlign = FloorLog2(uint32(OldHeader.sh_addralign)) + 1;
         if (NewAlign > 14) NewAlign = 14;   // limit for highest alignment
         NewHeader.Flags |= PE_SCN_ALIGN_1 * NewAlign;

         // Find relocation table for this section by searching through all sections
         for (relsec = 1; relsec < this->NSections; relsec++) {

            // Get section header
            OldRelHeader = this->SectionHeaders[relsec];

            // Check if this is a relocations section referring to oldsec
            if ((OldRelHeader.sh_type == SHT_REL || OldRelHeader.sh_type == SHT_RELA) // if section is relocation
            && OldRelHeader.sh_info == oldsec) { // and if section refers to current section

               // Found the right relocation table. Get pointer
               int8 * reltab = this->Buf() + uint32(OldRelHeader.sh_offset);
               int8 * reltabend = reltab + uint32(OldRelHeader.sh_size);

               // Get entry size
               int entrysize = uint32(OldRelHeader.sh_entsize);
               int expectedentrysize = (OldRelHeader.sh_type == SHT_RELA) ? 
                  sizeof(TELF_Relocation) :                    // Elf32_Rela, Elf64_Rela
                  sizeof(TELF_Relocation) - this->WordSize/8;  // Elf32_Rel,  Elf64_Rel
               if (entrysize < expectedentrysize) {err.submit(2033); entrysize = expectedentrysize;}

               // File pointer for new relocations
               NewHeader.PRelocations = NewRawData.GetDataSize() + RawDataOffset;   // file  to relocation entries

               // Loop through relocation table entries
               for (; reltab < reltabend; reltab += entrysize) {

                  // Copy relocation table entry with or without addend
                  OldRelocation.r_addend = 0;
                  memcpy(&OldRelocation, reltab, entrysize); 

                  // Find inline addend
                  uint32 InlinePosition = (uint32)(NewHeader.PRawData - RawDataOffset + OldRelocation.r_offset);

                  // Check that address is valid
                  if (InlinePosition >= this->GetDataSize()) {
                     // Address is invalid
                     err.submit(2032);
                     break;
                  }

                  // Pointer to inline addend
                  int32 * piaddend = (int32*)(NewRawData.Buf() + InlinePosition);

                  // Symbol offset
                  NewRelocation.VirtualAddress = uint32(OldRelocation.r_offset);

                  // Symbol table index
                  if (OldRelocation.r_sym < NewSymbolIndex.GetNumEntries()) { 
                     NewRelocation.SymbolTableIndex = NewSymbolIndex[OldRelocation.r_sym];
                  }
                  else {
                     NewRelocation.SymbolTableIndex = 0; // Symbol table index out of range
                  }

                  // Get relocation type and fix addend
                  if (this->WordSize == 32) {
                     switch(OldRelocation.r_type) {
                     case R_386_NONE:    // Ignored
                        NewRelocation.Type = COFF32_RELOC_ABS;  break;

                     case R_386_IRELATIVE:
                        err.submit(1063); // Warning: Gnu indirect function cannot be converted
                        // continue in next case?:
                     case R_386_32:      // 32-bit absolute virtual address
                        NewRelocation.Type = COFF32_RELOC_DIR32;  
                        *piaddend += uint32(OldRelocation.r_addend);  
                        break;

                     case R_386_PC32:   // 32-bit self-relative
                        NewRelocation.Type = COFF32_RELOC_REL32;  
                        // Difference between EIP-relative and self-relative relocation = size of address field
                        // Adjust inline addend for different relocation method:
                        *piaddend += 4 + uint32(OldRelocation.r_addend);
                        break;

                     case R_386_GOT32: case R_386_GLOB_DAT: case R_386_GOTOFF: case R_386_GOTPC:
                        // Global offset table
                        err.submit(2042);     // cannot convert position-independent code
                        err.ClearError(2042); // report this error only once
                        NewRelocation.Type = 0;
                        break;

                     case R_386_PLT32: case R_386_JMP_SLOT: 
                        // procedure linkage table
                        err.submit(2043);     // cannot convert import table
                        err.ClearError(2043); // report this error only once
                        NewRelocation.Type = 0;
                        break;

                     case R_386_RELATIVE:  // adjust by program base
                     default:              // Unknown or unsupported relocation method
                        err.submit(2030, OldRelocation.r_type); 
                        err.ClearError(2030); // report this error only once
                        NewRelocation.Type = 0; 
                        break;
                     }
                  }
                  else { // WordSize == 64
                     switch(OldRelocation.r_type) {
                     case R_X86_64_NONE:     // Ignored
                        NewRelocation.Type = COFF64_RELOC_ABS;  
                        break;

                     case R_X86_64_64:      // 64 bit absolute virtual addres
                        NewRelocation.Type = COFF64_RELOC_ABS64;  
                        *(int64*)piaddend += OldRelocation.r_addend;  
                        break;

                     case R_X86_64_IRELATIVE:
                        err.submit(1063); // Warning: Gnu indirect function cannot be converted
                        // continue in next case?:
                     case R_X86_64_32S:     // 32 bit absolute virtual address, sign extended
                     case R_X86_64_32:      // 32 bit absolute virtual address, zero extended
                        NewRelocation.Type = COFF64_RELOC_ABS32;  
                        *piaddend += uint32(OldRelocation.r_addend);  
                        break;

                     case R_X86_64_PC32:    // 32 bit, self-relative
                        // See COFF2ELF.cpp for an explanation of the difference between
                        // COFF and ELF relative relocation methods
                        *piaddend += uint32(OldRelocation.r_addend);
                        if (*piaddend >= -8 && *piaddend <= -4) {
                           NewRelocation.Type = (uint16)(COFF64_RELOC_REL32 - *piaddend - 4);  
                           *piaddend = 0;
                        }
                        else {
                           NewRelocation.Type = COFF64_RELOC_REL32;
                           *piaddend += 4;
                        }
                        break;

                     case R_X86_64_RELATIVE:  // Adjust by program base
                        err.submit(2030, OldRelocation.r_type); 
                        err.ClearError(2030); // report this error only once
                        NewRelocation.Type = 0;
                        break;

                     case R_X86_64_GOT32: case R_X86_64_GLOB_DAT: case R_X86_64_GOTPCREL:
                        // Global offset table
                        err.submit(2042);     // cannot convert position-independent code
                        err.ClearError(2042); // report this error only once
                        NewRelocation.Type = 0;
                        break;

                     case R_X86_64_PLT32: case R_X86_64_JUMP_SLOT: 
                        // procedure linkage table
                        err.submit(2042);     // cannot convert import table
                        err.ClearError(2043); // report this error only once
                        NewRelocation.Type = 0;
                        break;

                     default:              // Unknown or unsupported relocation method
                        err.submit(2030, OldRelocation.r_type); 
                        err.ClearError(2030); // report this error only once
                        NewRelocation.Type = 0; 
                        break;
                     }
                  }

                  // Store relocation entry
                  NewRawData.Push(&NewRelocation, SIZE_SCOFF_Relocation);
                  NewHeader.NRelocations++;

                  // Remember that symbol is used
                  if (OldRelocation.r_type) {
                     SymbolsUsed[NewRelocation.SymbolTableIndex]++;
                  }                                  

               } // End of relocations loop

            } // End of if right relocation table

         } // End of search for relocation table

         // Align raw data for next section
         NewRawData.Align(4);

         // Store section header in file
         ToFile.Push(&NewHeader, sizeof(NewHeader));

      } // End of if section has program data

   } // End of loop through old sections

} // End of function MakeSections


// Check for overflow when converting 64 bit symbol value to 32 bits.
// Value may be signed or unsigned
static int SymbolOverflow(uint64 x) {
   uint32 Upper = HighDWord(x);        // Upper 32 bits of 64 bit value
   if (Upper == 0xFFFFFFFF) {          // Check for signed overflow
      return int32(x) >= 0;            // Overflow if not signed
   }
   return Upper != 0;                  // Check for unsigned overflow
}
static int SymbolOverflow(uint32 x) {  // Overloaded 32 bit version
   return 0;                           // Cannot overflow if already 32 bits
}


// MakeSymbolTable(): Convert subfunction to make symbol table and string tables
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2COF<ELFSTRUCTURES>::MakeSymbolTable() {
   uint32 oldsec;                      // Section number in old file
   TELF_SectionHeader OldHeader;           // Old section header
   int FoundSymTab = 0;                // Found symbol table
   int8 * strtab;                      // Old symbol string table
   int8 * symtab;                      // Old symbol table
   uint32 symtabsize;                  // Size of old symbol table
   uint32 stringtabsize;               // Size of old string table
   int8 * symtabend;                   // End of old symbol table
   uint32 entrysize;                   // Size of each entry in old symbol table
   uint32 OldSymI;                     // Symbol index in old symbol table
   uint32 NewSymI = 0;                 // Symbol index in new symbol table
   const char * symname = 0;           // Symbol name
   TELF_Symbol OldSym;                     // Old symbol table record
   SCOFF_SymTableEntry NewSym;         // New symbol table record
   SCOFF_SymTableEntry AuxSym;         // Auxiliary symbol table entry
   uint32 numaux;                      // Number of auxiliary records for new entry

   // Initialize new string table. make space for 4-bytes size
   NewStringTable.Push(0, 4);

   // Loop through old sections to find symbol table
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Search for program data sections only
      if (this->SectionHeaders[oldsec].sh_type == SHT_SYMTAB 
      || this->SectionHeaders[oldsec].sh_type==SHT_DYNSYM) {
         FoundSymTab++;  numaux = 0;

         // Copy symbol table header for convenience
         OldHeader = this->SectionHeaders[oldsec];

         // Find associated string table
         if (OldHeader.sh_link >= this->NSections) {err.submit(2035); OldHeader.sh_link = 0;}
         strtab = this->Buf() + uint32(this->SectionHeaders[OldHeader.sh_link].sh_offset);
         stringtabsize = uint32(this->SectionHeaders[OldHeader.sh_link].sh_size);
            

         // Find old symbol table
         entrysize = uint32(OldHeader.sh_entsize);
         if (entrysize < sizeof(TELF_Symbol)) {err.submit(2033); entrysize = sizeof(TELF_Symbol);}

         symtab = this->Buf() + uint32(OldHeader.sh_offset);
         symtabsize = uint32(OldHeader.sh_size);
         symtabend = symtab + symtabsize;

         // Loop through old symbol table
         for (OldSymI = 0; symtab < symtabend; symtab += entrysize, OldSymI++) {

            // Copy old symbol table entry
            OldSym = *(TELF_Symbol*)symtab;

            // Reset new symbol table entry
            memset(&NewSym, 0, sizeof(NewSym));

            // New symbol index
            NewSymI = NewSymbolTable.GetNumEntries(); 

            // Symbol type
            int type = OldSym.st_type;

            // Symbol storage class = binding
            int binding = OldSym.st_bind;

            // Get symbol name
            if (OldSym.st_name < stringtabsize) {
               symname = strtab + OldSym.st_name;

               if (symname && *symname && type != STT_FILE) {
                  // Symbol has a name that we want to store
                  COFF_PutNameInSymbolTable(NewSym, symname, NewStringTable);
               }
            }
            else { // points outside string table
               err.submit(2112); continue;
            }

            // Value
            NewSym.s.Value = uint32(OldSym.st_value);
            // Check for overflow if converting 64 bit symbol value to 32 bits
            if (SymbolOverflow(OldSym.st_value)) err.submit(2020, symname); 

            // Section
            if (OldSym.st_shndx == SHN_UNDEF) {
               NewSym.s.SectionNumber = COFF_SECTION_UNDEF; // External
            }
            else if ((int16)(OldSym.st_shndx) == SHN_ABS) {
               NewSym.s.SectionNumber = COFF_SECTION_ABSOLUTE; // Absolute symbol
            }
            else if (OldSym.st_shndx >= this->NSections) {
               err.submit(2036, OldSym.st_shndx); // Special/unknown section index or out of range
            }
            else {
               // Normal section index. 
               // Look up in section index translation table and add 1 because it is 1-based
               NewSym.s.SectionNumber = (int16)(NewSectIndex[OldSym.st_shndx] + 1);
            }

            // Convert binding/storage class
            switch (binding) {
            case STB_LOCAL:
               NewSym.s.StorageClass = COFF_CLASS_STATIC; break;

            case STB_GLOBAL:
               NewSym.s.StorageClass = COFF_CLASS_EXTERNAL; break;

            case STB_WEAK:
               err.submit(1051, symname); // Weak public symbol not supported
               NewSym.s.StorageClass = COFF_CLASS_WEAK_EXTERNAL; break;

            default: 
               err.submit(2037, binding); // Other. Not supported
            }

            // Make record depending on type
            switch (type) {
            case STT_OBJECT: case STT_NOTYPE:
               // Data object
               NewSym.s.Type = COFF_TYPE_NOT_FUNCTION;  
               if (OldSymI > 0) { // First symbol entry in ELF file is unused
                  NewSymbolTable.Push(&NewSym, SIZE_SCOFF_SymTableEntry);
               }
               break;

            case STT_GNU_IFUNC:
               err.submit(1063); // Warning: Gnu indirect function cannot be converted
               // continue in next case:
            case STT_FUNC:
               // Function
               NewSym.s.Type = COFF_TYPE_FUNCTION;  
               NewSymbolTable.Push(&NewSym, SIZE_SCOFF_SymTableEntry);
               // Aux records needed only if debug information included
               break;

            case STT_FILE: {
               // File name record
               memset(&NewSym, 0, sizeof(NewSym));
               strcpy(NewSym.s.Name, ".file");
               NewSym.s.StorageClass = COFF_CLASS_FILE;
               NewSym.s.SectionNumber = COFF_SECTION_DEBUG;
               // Remove path from file name
               const char * shortname = symname;
               uint32 len = (uint32)strlen(symname);
               if (len > 1) {
                  // Scan backwards for last '/'
                  for (int scan = len-2; scan >= 0; scan--) {
                     if (symname[scan] == '/' || symname[scan] == '\\') {
                        // Path found. Short name starts after this character
                        shortname = symname + scan + 1;
                        break;
                     }
                  }
               }
               len = (uint32)strlen(shortname);
               if (len > 35) len = 35;  // arbitrary limit to file name length

               // Number of auxiliary records for storing file name
               numaux = (len + SIZE_SCOFF_SymTableEntry - 1) / SIZE_SCOFF_SymTableEntry;
               NewSym.s.NumAuxSymbols = (uint8)numaux;
               // Store regular record
               NewSymbolTable.Push(&NewSym, SIZE_SCOFF_SymTableEntry);               
               // Store numaux auxiliary records for file name
               for (uint32 i = 0; i < numaux; i++) { // Can't push all in one operation because NumEntries will be wrong
                  NewSymbolTable.Push(0, SIZE_SCOFF_SymTableEntry);
               }
               // copy name into NewSymbolTable aux records
               int8 * PointAux = NewSymbolTable.Buf() + NewSymbolTable.GetDataSize();
               memcpy(PointAux - numaux*SIZE_SCOFF_SymTableEntry, shortname, len);
               break;}

            case STT_SECTION: {
               // Section name record
               NewSym.s.Value = 0;
               NewSym.s.Type = 0;
               NewSym.s.StorageClass = COFF_CLASS_STATIC;
               NewSym.s.NumAuxSymbols = (uint8)(numaux = 1);

               // Find corresponding section header
               TELF_SectionHeader * OldSecHdr = 0;
               if (OldSym.st_shndx < this->NSections) {
                  OldSecHdr = &(this->SectionHeaders[OldSym.st_shndx]);

                  // Find section name
                  char * sname;
                  if (OldSecHdr->sh_name < this->SecStringTableLen) {
                     sname = this->SecStringTable + OldSecHdr->sh_name;
                     // Put into symbol table
                     COFF_PutNameInSymbolTable(NewSym, sname, NewStringTable);
                  }
               }

               // Store regular record
               NewSymbolTable.Push(&NewSym, SIZE_SCOFF_SymTableEntry);               

               // Make auxiliary record
               memset(&AuxSym, 0, sizeof(AuxSym));
               if (OldSecHdr) {
                  AuxSym.section.Length = uint32(OldSecHdr->sh_size);
                  // Find corresponding relocation section header
                  // Assume that relocation section comes immediately after section record
                  if ((uint32)OldSym.st_shndx + 1 < this->NSections    // if not last section
                  && (OldSecHdr[1].sh_type == SHT_REL || OldSecHdr[1].sh_type == SHT_RELA) // and if next section is relocation
                  && OldSecHdr[1].sh_info == OldSym.st_shndx // and if next section refers to current section
                  && OldSecHdr[1].sh_entsize > 0) { // Avoid division by 0
                     // Calculate number of relocations
                     AuxSym.section.NumberOfRelocations = (uint16)(uint32(OldSecHdr[1].sh_size) / uint32(OldSecHdr[1].sh_entsize));
                  }
               }
               // Store auxiliary record
               NewSymbolTable.Push(&AuxSym, SIZE_SCOFF_SymTableEntry);               
               break;}

            case STT_COMMON:
            default:
               err.submit(2038, type); // Symbol type not supported
            }

            if (FoundSymTab == 1) {
               // Make translation table from old symbol index to new symbol index,
               // assuming there is only one symbol table.
               // Make sure all old symbols have an entry in the NewSymbolIndex table,
               // even if they are discarded.
               NewSymbolIndex.Push(NewSymI);
            }
         } // End OldSymI loop
      }
   } // End search for symbol table
   if (FoundSymTab == 0) err.submit(2034); // Symbol table not found
   if (FoundSymTab  > 1) err.submit(1032); // More than one symbol table found

   // Allocate space for SymbolsUsed table
   SymbolsUsed.SetNum(NewSymI+1);
   SymbolsUsed.SetZero();                  // Initialize
}


// HideUnusedSymbols(): Hide unused symbols if stripping debug info or exception info
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2COF<ELFSTRUCTURES>::HideUnusedSymbols() {

   if (cmd.DebugInfo != CMDL_DEBUG_STRIP && cmd.ExeptionInfo != CMDL_EXCEPTION_STRIP) {
      // No sections removed. Do nothing
      return;
   }

   // Pointer to new symbol table
   union {
      SCOFF_SymTableEntry * p; // Symtab entry pointer
      int8 * b;                // Used for increment
   } NewSymtab;
   NewSymtab.b = NewSymbolTable.Buf();
   int numaux = 0, isym;
   int NumberOfSymbols = NewSymbolTable.GetNumEntries();

   // Loop through new symbol table
   for (isym = 0; isym < NumberOfSymbols; isym += numaux+1, NewSymtab.b += SIZE_SCOFF_SymTableEntry*(numaux+1)) {

      // Number of auxiliary records belonging to same symbol
      numaux = NewSymtab.p->s.NumAuxSymbols;  if (numaux < 0) numaux = 0;

      if (NewSymtab.p->s.StorageClass == COFF_CLASS_EXTERNAL
      ||  NewSymtab.p->s.StorageClass == COFF_CLASS_WEAK_EXTERNAL) {
         if (NewSymtab.p->s.SectionNumber == COFF_SECTION_UNDEF) {
            // External symbol. Check if it is used
            if (!SymbolsUsed[isym]) {
               // Symbol is unused. Hide it to prevent linking errors
               NewSymtab.p->s.StorageClass = COFF_CLASS_NULL;
               NewSymtab.p->s.SectionNumber = COFF_SECTION_UNDEF;
               NewSymtab.p->s.Type = COFF_TYPE_NOT_FUNCTION;
               cmd.CountSymbolsHidden();
            }
         }
      }
   }
}

// MakeBinaryFile(): Convert subfunction to put all sections together
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2COF<ELFSTRUCTURES>::MakeBinaryFile() {

   // Insert string table size
   //NewStringTable.Get<uint32>(0) = NewStringTable.GetDataSize();
   // Some compilers fail with the double template here. Avoid the template:
   *(uint32*)(NewStringTable.Buf()) = NewStringTable.GetDataSize();

   // Update file header
   NewFileHeader.NumberOfSections = (uint16)NumSectionsNew;
   NewFileHeader.PSymbolTable = RawDataOffset + NewRawData.GetDataSize();
   NewFileHeader.NumberOfSymbols = NewSymbolTable.GetNumEntries();

   // Replace file header in new file with updated version
   memcpy(ToFile.Buf(), &NewFileHeader, sizeof(NewFileHeader));

   // Section headers have already been inserted.
   // Insert raw data in file
   ToFile.Push(NewRawData.Buf(), NewRawData.GetDataSize());

   // Insert symbol table
   ToFile.Push(NewSymbolTable.Buf(), NewSymbolTable.GetDataSize());

   // Insert string table
   ToFile.Push(NewStringTable.Buf(), NewStringTable.GetDataSize());
}


// Make template instances for 32 and 64 bits
template class CELF2COF<ELF32STRUCTURES>;
template class CELF2COF<ELF64STRUCTURES>;
