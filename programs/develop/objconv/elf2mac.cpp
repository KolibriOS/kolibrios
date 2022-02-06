/****************************  elf2mac.cpp   *********************************
* Author:        Agner Fog
* Date created:  2007-01-10
* Last modified: 2012-05-05
* Project:       objconv
* Module:        elf2mac.cpp
* Description:
* Module for converting ELF file to Mach-O file
*
* Copyright 2007-2012 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#include "stdafx.h"

template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation, 
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
   CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::CELF2MAC() {
   // Constructor
      memset(this, 0, sizeof(*this));                   // Reset everything
}

template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation, 
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
   void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::Convert() {
   // Do the conversion
   // Some compilers require this-> for accessing members of template base class,
   // according to the so-called two-phase lookup rule.

   // Call the subfunctions
   ToFile.SetFileType(FILETYPE_MACHO_LE);             // Set type of new file
   MakeFileHeader();                                  // Make file header
   MakeSectionsIndex();                               // Make sections index translation table
   FindUnusedSymbols();                               // Check if symbols used, remove unused symbols
   MakeSymbolTable();                                 // Make symbol table and string tables
   MakeSections();                                    // Make sections and relocation tables
   MakeBinaryFile();                                  // Put sections together
   *this << ToFile;                                   // Take over new file buffer
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::MakeFileHeader() {
   // Convert subfunction: Make file header and load segment command
   TMAC_header NewHeader;                             // new file header
   NewHeader.magic      = (this->WordSize == 32) ? MAC_MAGIC_32 : MAC_MAGIC_64; // Mach magic number identifier
   NewHeader.cputype    = (this->WordSize == 32) ? MAC_CPU_TYPE_I386 : MAC_CPU_TYPE_X86_64;
   NewHeader.cpusubtype = MAC_CPU_SUBTYPE_I386_ALL;
   NewHeader.filetype   = MAC_OBJECT;
   NewHeader.ncmds      = 3;                          // Three commands = segment, symbol table, dynsymtab
   NewHeader.sizeofcmds = 0;                          // Set this later
   NewHeader.flags      = 0;                          // No flags needed

   // put file header in OutFile
   ToFile.Push(&NewHeader, sizeof(NewHeader));
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::MakeSectionsIndex() {
   // Make sections index translation table and section offset table.

   // We must make these tables before the sections, because they are needed for the
   // symbol tables and relocation tables, and we must make the symbol tables before 
   // the relocation tables, and we must make the relocation tables together with the
   // sections. 
   uint32 oldsec;                         // Section number in old file
   uint32 newsec = 0;                     // Section number in new file
   NewSectIndex. SetNum(this->NSections); // Allocate size for section index table
   NewSectIndex. SetZero();               // Initialize
   NewSectOffset.SetNum(this->NSections); // Allocate buffer for section offset table
   NewSectOffset.SetZero();               // Initialize

   MInt NewVirtualAddress = 0;            // Virtual address of new section as specified in Mach-O file

   // First loop through old sections
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {
      NewSectIndex[oldsec]  = 0;
      NewSectOffset[oldsec] = 0;

      // Get section name
      const char * sname = "";
      uint32 namei = this->SectionHeaders[oldsec].sh_name;
      if (namei >= this->SecStringTableLen) {
         err.submit(2112);
      }
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
            continue;
         }
      }

      if (cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
         // Check for exception section name
         if (strncmp(sname, ".eh_frame", 9) == 0) {
            // Remove this section
            this->SectionHeaders[oldsec].sh_type = SHT_REMOVE_ME;
            cmd.CountExceptionRemoved();
            continue;
         }
      }

      // Search for program data sections only
      if (this->SectionHeaders[oldsec].sh_type != SHT_PROGBITS 
      &&  this->SectionHeaders[oldsec].sh_type != SHT_NOBITS) {
         // Has no data. Ignore
         continue;
      }

      if (this->SectionHeaders[oldsec].sh_size == 0) {
         // Remove empty section
         // The linker has a bug with empty sections
         continue;
      }

      // Section index translation table
      NewSectIndex[oldsec] = newsec++;

      // Calculate virtual memory address of section. This address does not have 
      // much to do with the final address, but it is needed in relocation entries.

      // Alignment
      int NewAlign = FloorLog2((uint32)this->SectionHeaders[oldsec].sh_addralign);
      if (NewAlign > 12) NewAlign = 12;   // What is the limit for highest alignment?
      int AlignBy = 1 << NewAlign;

      // Align memory address
      NewVirtualAddress = (NewVirtualAddress + AlignBy - 1) & -(MInt)AlignBy;

      // Virtual memory address of new section
      NewSectOffset[oldsec] = NewVirtualAddress;

      // Increment memory address
      NewVirtualAddress += this->SectionHeaders[oldsec].sh_size;

      // Fix v. 2.14: Align end of memory address by 4
      NewVirtualAddress = (NewVirtualAddress + 3) & MInt(-4);
   }

   // Store number of sections in new file
   NumSectionsNew = newsec;

   // Calculate file offset of first raw data
   RawDataOffset = sizeof(TMAC_header) 
      + sizeof(TMAC_segment_command) 
      + NumSectionsNew * sizeof(TMAC_section)
      + sizeof(MAC_symtab_command) 
      + sizeof(MAC_dysymtab_command);

   // Align end of memory address by 4
   NewVirtualAddress = (NewVirtualAddress + 3) & MInt(-4);

   // Make segment command
   TMAC_segment_command NewSegment;
   memset(&NewSegment, 0, sizeof(NewSegment));
   NewSegment.cmd      = (this->WordSize == 32) ? MAC_LC_SEGMENT : MAC_LC_SEGMENT_64;
   NewSegment.cmdsize  = sizeof(TMAC_segment_command) + NumSectionsNew * sizeof(TMAC_section);
   NewSegment.fileoff  = RawDataOffset;
   NewSegment.nsects   = NumSectionsNew;
   NewSegment.maxprot  = NewSegment.initprot = 7; // 1=read, 2=write, 4=execute
   NewSegment.vmsize   = NewVirtualAddress;
   NewSegment.filesize = 0;                       // Changed later

   // put segment command in OutFile
   CommandOffset = ToFile.Push(&NewSegment, sizeof(NewSegment));
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::MakeSymbolTable() {
   // Convert subfunction: Symbol table and string tables
   uint32 oldsec;                  // Section number in old file
   TELF_SectionHeader OldHeader;   // Old section header
   int FoundSymTab = 0;            // Found symbol table
   int8 * strtab;                  // Old symbol string table
   int8 * symtab;                  // Old symbol table
   uint32 symtabsize;              // Size of old symbol table
   int8 * symtabend;               // End of old symbol table
   uint32 entrysize;               // Size of each entry in old symbol table
   TELF_Symbol OldSym;             // Old symbol table record
   uint32 OldSymI;                 // Symbol index in old symbol table
   const char * symname;           // Symbol name
   int NewSection = 0;             // New section index
   int NewType;                    // New symbol type
   int NewDesc;                    // New symbol reference type
   MInt Value;                     // Symbol value
   uint32 Scope;                   // 0: Local, 1: Public, 2: External 

   // Loop through old sections to find symbol table
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Search for program data sections only
      if (this->SectionHeaders[oldsec].sh_type == SHT_SYMTAB 
      ||  this->SectionHeaders[oldsec].sh_type == SHT_DYNSYM) {
         FoundSymTab++;

         // Copy symbol table header for convenience
         OldHeader = this->SectionHeaders[oldsec];

         // Find associated string table
         if (OldHeader.sh_link >= (uint32)(this->NSections)) {
            err.submit(2035); OldHeader.sh_link = 0;
         }
         strtab = this->Buf() + (uint32)this->SectionHeaders[OldHeader.sh_link].sh_offset;

         // Find old symbol table
         entrysize = (uint32)OldHeader.sh_entsize;
         if (entrysize < sizeof(TELF_Symbol)) {err.submit(2033); entrysize = sizeof(TELF_Symbol);}

         symtab = this->Buf() + (uint32)OldHeader.sh_offset;
         symtabsize = (uint32)OldHeader.sh_size;
         symtabend = symtab + symtabsize;

         if (NewSymTab[0].GetNumEntries() == 0) {
            // make empty symbol record for index 0
            NewSymTab[0].AddSymbol(0, "", 0, 0, 0, 0);
         }

         // Loop through old symbol table
         for (OldSymI = 0; symtab < symtabend; symtab += entrysize, OldSymI++) {

            if (OldSymI == 0) continue; // First symbol entry in ELF file is unused

            // Copy 32 bit symbol table entry or convert 64 bit entry
            OldSym = *(TELF_Symbol*)symtab;
 
            // Old symbol type
            int type = OldSym.st_type;

            // Old symbol storage class = binding
            int binding = OldSym.st_bind;

            // Get symbol name
            if (OldSym.st_name < this->SymbolStringTableSize) {
               symname = strtab + OldSym.st_name;
            }
            else {
               err.submit(2112); // String table corrupt
               continue;       // Ignore
            }
            if (symname == 0 || *symname == 0) {
               // Symbol has no name. Give it a name
               // Mac linker messes this up if the symbol doesn't have a unique name.
               char tempbuf[80];
               sprintf(tempbuf, "?unnamed%i", OldSymI);
               int os = UnnamedSymbolsTable.PushString(tempbuf);
               symname = UnnamedSymbolsTable.Buf() + os;
            }
            
            NewType = NewDesc = 0; // New symbol type

            // Value = address
            Value = OldSym.st_value;

            // Section
            if (OldSym.st_shndx == SHN_UNDEF) {
               NewSection = 0; // External
            }
            else if ((int16)(OldSym.st_shndx) == SHN_ABS) {
               NewType |= MAC_N_ABS; // Absolute symbol
               NewDesc |= MAC_N_NO_DEAD_STRIP;
               NewSection = 0; 
            }
            else if ((int16)(OldSym.st_shndx) == SHN_COMMON) {
               NewType |= MAC_N_ABS; // Common symbol. Translate to abs and make warning
               NewDesc |= MAC_N_NO_DEAD_STRIP;
               NewSection = 0; 
               err.submit(1053, symname); // Warning. Common symbol
            }            
            else if (OldSym.st_shndx >= this->NSections) {
               err.submit(2036, OldSym.st_shndx); // Special/unknown section index or out of range
            }
            else {
               // Normal section index. 
               // Look up in section index translation table and add 1 because it is 1-based
               NewSection = NewSectIndex[OldSym.st_shndx] + 1;
               // Value must be absolute address. Add section address
               Value += NewSectOffset[OldSym.st_shndx];
            }

            // Convert binding/storage class
            switch (binding) {
            case STB_LOCAL:   // Local
               Scope = S_LOCAL;
               if (!(NewType & MAC_N_ABS)) NewType |= MAC_N_SECT;
               break;

            case STB_GLOBAL:
               if (NewSection || (NewType & MAC_N_ABS)) {
                  // Public
                  Scope = S_PUBLIC;
                  NewType |= MAC_N_EXT;
                  if (!(NewType & MAC_N_ABS)) NewType |= MAC_N_SECT;
               }
               else {
                  // External
                  Scope = S_EXTERNAL;
                  NewType |= MAC_N_EXT;
               }
               NewDesc |= MAC_REF_FLAG_UNDEFINED_NON_LAZY;
               break;

            case STB_WEAK:
               if (NewSection) {
                  // Weak public
                  Scope = S_PUBLIC;
                  NewType |= MAC_N_EXT | MAC_N_SECT;
                  NewDesc |= MAC_N_WEAK_DEF;
                  if (this->WordSize == 32) {
                     err.submit(1051, symname);  // Weak public only allowed in coalesced section of MachO-32
                  }
               }
               else {
                  // Weak external
                  Scope = S_EXTERNAL;
                  NewType |= MAC_N_EXT;
                  NewDesc |= MAC_N_WEAK_REF;
               }
               break;

            default:
               Scope = S_LOCAL;
               err.submit(2037, binding); // Other. Not supported
            }

            // Make record depending on type
            switch (type) {
            case STT_OBJECT: case STT_NOTYPE:
               // Data object
               break;

            case STT_GNU_IFUNC:
               err.submit(1063); // Warning: Gnu indirect function cannot be converted
               // continue in next case:

            case STT_FUNC:
               // Function
               break;

            case STT_FILE: 
               // File name record. Ignore
               continue;

            case STT_SECTION:
               // Section name record. (Has no name)
               break;

            case STT_COMMON:
            default:
               err.submit(2038, type); // Symbol type not supported
               continue;
            }

            // Discard unused symbols
            if (Scope != S_PUBLIC && !OldSymbolUsed[OldSymI]) continue;

            // Store new symbol record in the appropriate table
            if (Scope > 2) err.submit(9000);

            NewSymTab[Scope].AddSymbol(OldSymI, symname, NewType, NewDesc, NewSection, Value);

            // Store scope in OldSymbolScope
            if (OldSymI < NumOldSymbols) {
               OldSymbolScope[OldSymI] = Scope;
            }
         } // End OldSymI loop
      }
   } // End search for symbol table
   if (FoundSymTab == 0) err.submit(2034); // Symbol table not found
   if (FoundSymTab  > 1) err.submit(1032); // More than one symbol table found
}


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::Elf2MacRelocations(Elf32_Shdr & OldRelHeader, MAC_section_32 & NewHeader, uint32 NewRawDataOffset, uint32 oldsec) {
   // Convert 32-bit relocations from ELF to MAC
   // (This function has two template instances, only the 32-bit instance is used)

   Elf32_Rela OldRelocation;  // Old relocation table entry
   MAC_scattered_relocation_info scat; // Scattered relocation entry
   memset(&scat, 0, sizeof(scat));

   // Get pointer to old relocation table
   int8 * reltab = this->Buf() + OldRelHeader.sh_offset;
   int8 * reltabend = reltab + OldRelHeader.sh_size;

   // Get entry size
   uint32 entrysize = (uint32)OldRelHeader.sh_entsize;
   uint32 expectedentrysize = (OldRelHeader.sh_type == SHT_REL) ? sizeof(Elf32_Rel) : sizeof(Elf32_Rela);
   if (entrysize < expectedentrysize) {err.submit(2033); entrysize = expectedentrysize;}

   // File pointer to relocations
   NewHeader.reloff = NewRelocationTab.GetNumEntries()*sizeof(MAC_relocation_info);  // Offset to first relocation table added later

   // Loop through relocation table entries
   for (; reltab < reltabend; reltab += entrysize) {

      // Copy relocation entry with or without addend
      OldRelocation.r_addend = 0;
      memcpy(&OldRelocation, reltab, entrysize);

      // Find inline addend
      uint32 InlinePosition = (uint32)(NewRawDataOffset + OldRelocation.r_offset);

      // Check that address is valid
      if (InlinePosition >= this->GetDataSize()) {
         // Address is invalid
         err.submit(2032);  break;
      }

      // Pointer to inline addend
      int32 * piaddend = (int32*)(NewRawData.Buf() + InlinePosition);

      // Add old addend if any
      *piaddend += (int32)OldRelocation.r_addend;

      // Define relocation parameters
      uint32  r_address = 0;      // section-relative offset to relocation source
      uint32  r_symbolnum = 0;    // symbol index if r_extern == 1 or section ordinal if r_extern == 0
      // uint32  r_value = 0;        // value of relocation target
      // int     r_scattered = 0;    // use scattered relocation
      int     r_pcrel = 0;        // self relative
      int     r_length = 2;       // size of source: 0=byte, 1=2 bytes, 2=4 bytes, 3=8 bytes
      int     r_extern = 0;       // public or external
      int     r_type = 0;         // if not 0, machine specific relocation type
      int     Scope = 0;          // Symbol scope: 0 = local, 1 = public, 2 = external

      // source offset
      r_address = (uint32)OldRelocation.r_offset;

      // target scope
      if (OldRelocation.r_sym < NumOldSymbols) {
         Scope = OldSymbolScope[OldRelocation.r_sym];
      }

      // Get r_extern: 0 = local target referenced by address,
      //               1 = external symbol referenced by symbol table index
      switch (Scope) {
      case S_LOCAL:  // Local target must be referenced by address
         r_extern = 0;  break;

      case S_PUBLIC:  // Public target is optionally referenced by index or by address
         r_extern = 0;
         // r_extern = 1; is not allowed!
         break;

      case S_EXTERNAL:  // External target is always referenced by index
         r_extern = 1;  break;
      }

      // Get zero-based index into NewSymTab[Scope]
      int newindex = NewSymTab[Scope].TranslateIndex(OldRelocation.r_sym);
      if (newindex < 0) {
         // Symbol not found or wrong type
         err.submit(2031); 
         break;
      }
      if (r_extern) {
         // r_symbolnum is zero based index into combined symbol tables.
         // Add number of entries in preceding NewSymTab tables to index 
         // into NewSymTab[Scope]
         r_symbolnum = newindex + NumSymbols[Scope];
      }
      else {
         // r_extern = 0. r_symbolnum = target section
         r_symbolnum = NewSymTab[Scope][newindex].n_sect;

         // Absolute address of target stored inline in source
         *piaddend  += (uint32)NewSymTab[Scope][newindex].n_value;
      }

      // Get relocation type and fix addend
      switch(OldRelocation.r_type) {
      case R_386_NONE:    // Ignored
         continue;

      case R_386_IRELATIVE:
         err.submit(1063); // Warning: Gnu indirect function cannot be converted
         // continue in next case?:
      case R_386_32:       // 32-bit absolute virtual address
         r_type  = MAC32_RELOC_VANILLA;  
         break;

      case R_386_PC32:   // 32-bit self-relative
         r_type  = MAC32_RELOC_VANILLA;  
         r_pcrel = 1;
         // Mach-O 32 bit format requires that self-relative addresses must have
         // self-relative values already before relocation. Therefore
         // the source address is subtracted.
         // (The PC reference point is the end of the source = start
         // of source + 4, but ELF files have the same offset so no further
         // correction is needed when converting from ELF file).

         // !! ToDo: Self-relative relocations plus offset to local symbol in a different section 
         // sometimes causes problems in Mac linker, perhaps because it fails to determine 
         // correctly which section the target is in. Use a relocation with a reference point
         // instead. This probably occurs only in assembler-coded self-relative 32-bit code.
         // (Use asmlib A_strtoupper and A_strcspn as test cases - they fail if dummy data
         // at the end of .data section is removed)
         *piaddend -= r_address + (uint32)NewHeader.addr;
         break;

      case R_UNSUPPORTED_IMAGEREL:  // 32-bit image-relative
         // This occurs only when converting from COFF (via ELF)
         // Needs scattered relocation entry
         scat.r_address   = r_address;
         scat.r_length    = 2;
         scat.r_pcrel     = 0;
         scat.r_scattered = 1;
         scat.r_type      = MAC32_RELOC_SECTDIFF;
         scat.r_value     = r_symbolnum;
         // Store first entry of scattered pair
         NewRelocationTab.Push(&scat, sizeof(scat));
         NewHeader.nreloc++;
         // Make subtractor record for image base
         scat.r_type      = MAC32_RELOC_PAIR;
         scat.r_value     = GetImagebaseSymbol();
         // Store second entry of scattered pair
         NewRelocationTab.Push(&scat, sizeof(scat));
         NewHeader.nreloc++;
         continue;

      case R_386_GOT32: case R_386_GLOB_DAT: case R_386_GOTOFF: case R_386_GOTPC:
         // Global offset table
         err.submit(2042);     // cannot convert position-independent code
         err.ClearError(2042); // report this error only once
         r_type = 0;
         break;

      case R_386_PLT32: case R_386_JMP_SLOT: 
         // procedure linkage table
         err.submit(2043);     // cannot convert import table
         err.ClearError(2043); // report this error only once
         r_type = 0;
         break;

      default:      // Unknown or unsupported relocation method
         err.submit(2030, OldRelocation.r_type); 
         r_type = 0;  break;
      }

      if (!r_pcrel) {
         // Warn for position dependent code. 
         // This warning is currently turned off in error.cpp.
         err.submit(1050, this->SymbolName(OldRelocation.r_sym)); 
         // Write this error only once
         err.ClearError(1050);
      }

      // Make relocation entry
      MAC_relocation_info rel;
      memset(&rel, 0, sizeof(rel));

      // Make non-scattered relocation entry
      rel.r_address   = r_address; 
      rel.r_symbolnum = r_symbolnum; 
      rel.r_pcrel     = r_pcrel; 
      rel.r_length    = r_length; 
      rel.r_extern    = r_extern; 
      rel.r_type      = r_type; 

      // Store relocation entry
      NewRelocationTab.Push(&rel, sizeof(rel));
      NewHeader.nreloc++;

      // Remember that symbol is used
      // if (SymbolsUsed && OldRelocation.r_type && NewRelocation.r_symbolnum < ?) {
      // SymbolsUsed[NewRelocation.r_symbolnum]++;}

   } // End of relocations loop
}

template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::Elf2MacRelocations(Elf64_Shdr & OldRelHeader, MAC_section_64 & NewHeader, uint32 NewRawDataOffset, uint32 oldsec) {
   // Convert 64-bit relocations from ELF to MAC
   // (This function has two template instances, only the 64-bit instance is used)

   // Make relocation entry for dummy subtractor
   MAC_relocation_info relsub;
   memset(&relsub, 0, sizeof(relsub));

   Elf64_Rela OldRelocation;  // Old relocation table entry

   // Get pointer to old relocation table
   int8 * reltab = this->Buf() + OldRelHeader.sh_offset;
   int8 * reltabend = reltab + OldRelHeader.sh_size;

   // Get entry size
   uint32 entrysize = (uint32)OldRelHeader.sh_entsize;
   uint32 expectedentrysize = (OldRelHeader.sh_type == SHT_REL) ? sizeof(Elf64_Rel) : sizeof(Elf64_Rela);
   if (entrysize < expectedentrysize) {err.submit(2033); entrysize = expectedentrysize;}

   // File pointer to relocations
   NewHeader.reloff = NewRelocationTab.GetNumEntries()*sizeof(MAC_relocation_info);  // Offset to first relocation table added later

   // Loop through relocation table entries
   for (; reltab < reltabend; reltab += entrysize) {

      // Copy relocation entry with or without addend
      OldRelocation.r_addend = 0;
      memcpy(&OldRelocation, reltab, entrysize);

      // Find inline addend
      uint32 InlinePosition = (uint32)(NewRawDataOffset + OldRelocation.r_offset);

      // Check that address is valid
      if (InlinePosition >= this->GetDataSize()) {
         // Address is invalid
         err.submit(2032);
         break;
      }

      // Pointer to inline addend
      int32 * piaddend = (int32*)(NewRawData.Buf() + InlinePosition);

      // Add old addend if any
      *piaddend += (uint32)OldRelocation.r_addend;

      // Define relocation parameters
      uint32  r_address = 0;      // section-relative offset to relocation source
      uint32  r_symbolnum = 0;    // symbol index if r_extern == 1 or section ordinal if r_extern == 0
      // uint32  r_value = 0;        // value of relocation target
      // int     r_scattered = 0;    // scattered relocations not used in 64 bit
      int     r_pcrel = 0;        // self relative
      int     r_length = 2;       // size of source: 0=byte, 1=2 bytes, 2=4 bytes, 3=8 bytes
      int     r_extern = 0;       // public or external
      int     r_type = 0;         // if not 0, machine specific relocation type
      int     Scope = 0;          // Symbol scope: 0 = local, 1 = public, 2 = external

      // source offset
      r_address = (uint32)OldRelocation.r_offset;

      // target scope
      if (OldRelocation.r_sym < NumOldSymbols) {
         Scope = OldSymbolScope[OldRelocation.r_sym];
      }

      // Get r_extern: 0 = local target referenced by address,
      //               1 = public or external symbol referenced by symbol table index
      switch (Scope) {
      case S_LOCAL:   // Local target
         // r_extern = 0;  // Local target must be referenced by address
         // Note: the description in reloc.h says that local targets are addressed
         // relative to any preceding public target. If there is no preceding label
         // then referenced by address in the segment. However, the Gnu compiler
         // uses reference to a local symbol and sets r_extern = 1 to indicate that
         // it refers to a symbol record, not to an address. I have chosen to use the
         // latter method because it is simpler, though undocumented.
         r_extern = 1;
         break;

      case S_PUBLIC:  // Public target is optionally referenced by index or by address
         r_extern = 1;
         break;

      case S_EXTERNAL:  // External target is always referenced by index
         r_extern = 1;  
         break;
      }

      // Get zero-based index into NewSymTab[Scope]
      int newindex = NewSymTab[Scope].TranslateIndex(OldRelocation.r_sym);
      if (newindex < 0) {
         // Symbol not found or wrong type
         err.submit(2031); 
         break;
      }

      // r_symbolnum is zero based index into combined symbol tables.
      // Add number of entries in preceding NewSymTab tables to index 
      // into NewSymTab[Scope]
      r_symbolnum = newindex + NumSymbols[Scope];

      // Get relocation type and fix addend, 64 bit
      switch(OldRelocation.r_type) {
      case R_X86_64_NONE:    // Ignored
         continue;

      case R_X86_64_64:      
         // 64-bit absolute virtual address
         r_type  = MAC64_RELOC_UNSIGNED;  r_length = 3;
         break;

      case R_X86_64_IRELATIVE:
         err.submit(1063); // Warning: Gnu indirect function cannot be converted
         // continue in next case?:
      case R_X86_64_32: case R_X86_64_32S: {
         // 32-bit absolute virtual address
         // Note: The linker doesn't accept a 32-bit absolute address
         // Make address relative to the image base, and add the value of the image base to compensate
         if (cmd.ImageBase == 0) {
            // Default image base if not specified
            cmd.ImageBase = 0x400000;
         }

         // Make subtractor relocation entry for image base
         relsub.r_address   = r_address;
         relsub.r_symbolnum = GetImagebaseSymbol();
         relsub.r_length    = 2; 
         relsub.r_extern    = 1; 
         relsub.r_type      = MAC64_RELOC_SUBTRACTOR; 

         NewRelocationTab.Push(&relsub, sizeof(relsub));
         NewHeader.nreloc++;
         // Add image base to compensate for subtracted image base
         *piaddend += cmd.ImageBase;
         
         // Now we can add the address we really want:
         r_type  = MAC64_RELOC_UNSIGNED;  
         r_length = 2;
         
         // Warn that image base must be set to the specified value
         char ImageBaseHex[32];
         sprintf(ImageBaseHex, "%X", cmd.ImageBase); // write value as hexadecimal
         err.submit(1300, ImageBaseHex);  err.ClearError(1300);
         break;}       
 
      case R_X86_64_PC32:   // 32-bit self-relative
         r_type  = MAC64_RELOC_BRANCH;
         // MAC64_RELOC_SIGNED does the same, but the linker complains if external symbol
         r_length = 2;  
         r_pcrel = 1;
         // Difference between EIP-relative and self-relative relocation = size of address field
         // Adjust inline addend for different relocation method:
         *piaddend += 4;
         break;

      case R_UNSUPPORTED_IMAGEREL:  // 32-bit image-relative
         // This occurs only when converting from COFF (via ELF)
         // Make subtractor relocation entry for image base
         relsub.r_address   = r_address;
         relsub.r_symbolnum = GetImagebaseSymbol();
         relsub.r_length    = 2; 
         relsub.r_extern    = 1; 
         relsub.r_type      = MAC64_RELOC_SUBTRACTOR; 

         NewRelocationTab.Push(&relsub, sizeof(relsub));
         NewHeader.nreloc++;
         
         // Second record adds the target address
         r_type  = MAC64_RELOC_UNSIGNED;  
         r_length = 2;
         break;

      case R_X86_64_GLOB_DAT: case R_X86_64_GOTPCREL:
         // Create 64-bit GOT entry??
         r_type  = MAC64_RELOC_GOT;  r_length = 2;
         break;         

      case R_X86_64_GOT32:
         // 32-bit GOT entry
         err.submit(2042);     // cannot convert 32-bit GOT
         err.ClearError(2042); // report this error only once
         r_type = 0;
         break;

      case R_X86_64_PLT32: case R_X86_64_JUMP_SLOT: 
         // procedure linkage table
         err.submit(2043);     // cannot convert import table
         err.ClearError(2043); // report this error only once
         r_type = 0;
         break;

      case R_X86_64_COPY: case R_X86_64_RELATIVE:
      default:      // Unknown or unsupported relocation method
         err.submit(2030, OldRelocation.r_type); 
         r_type = 0;  break;
      }

      // Make relocation entry
      MAC_relocation_info rel;
      memset(&rel, 0, sizeof(rel));

      // Make non-scattered relocation entry
      rel.r_address   = r_address; 
      rel.r_symbolnum = r_symbolnum; 
      rel.r_pcrel     = r_pcrel; 
      rel.r_length    = r_length; 
      rel.r_extern    = r_extern; 
      rel.r_type      = r_type; 

      // Store relocation entry
      NewRelocationTab.Push(&rel, sizeof(rel));
      NewHeader.nreloc++;

      // Remember that symbol is used
      // if (SymbolsUsed && OldRelocation.r_type && NewRelocation.r_symbolnum < ?) {
      // SymbolsUsed[NewRelocation.r_symbolnum]++;}

   } // End of relocations loop
}

template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::MakeSections() {
   // Convert subfunction: Make sections and relocation tables
   uint32 oldsec;                  // Section number in old file
   uint32 relsec;                  // Relocation section in old file
   TMAC_section NewHeader;         // New section header
   TELF_SectionHeader OldHeader;   // Old section header
   TELF_SectionHeader OldRelHeader;// Old relocation section header
   uint32 NewVirtualAddress = 0;   // Virtual address of new section
   uint32 NewRawDataOffset = 0;    // Offset into NewRawData of section. 
   // NewRawDataOffset is different from NewVirtualAddress if alignment of sections in 
   // the object file is different from alignment of sections in memory

   // Count cumulative number of symbols in each scope
   NumSymbols[0] = 0;
   NumSymbols[1] = NumSymbols[0] + NewSymTab[0].GetNumEntries();
   NumSymbols[2] = NumSymbols[1] + NewSymTab[1].GetNumEntries();
   NumSymbols[3] = NumSymbols[2] + NewSymTab[2].GetNumEntries();
   if (NumSymbols[3] >= 0x1000000) err.submit(2051); // Too many symbols, max = 2^24

   NewSectHeadOffset = ToFile.GetDataSize();

   // Second loop through old sections
   for (oldsec = 0; oldsec < this->NSections; oldsec++) {

      // Copy old header for convenience
      OldHeader = this->SectionHeaders[oldsec];

      if (OldHeader.sh_size == 0) {
         // Remove empty section
         // The linker has a bug with empty sections
         continue;
      }

      // Search for program data sections only
      if (OldHeader.sh_type == SHT_PROGBITS || OldHeader.sh_type == SHT_NOBITS) {

         // Reset new section header
         memset(&NewHeader, 0, sizeof(NewHeader));

         // Section name
         const char * sname = "";
         uint32 namei = OldHeader.sh_name;
         if (namei >= this->SecStringTableLen) err.submit(2112);
         else sname = this->SecStringTable + namei;

         // Translate section name and truncate to 16 characters
         if (!stricmp(sname,".text") || !stricmp(sname,"_text")) {
            strcpy(NewHeader.sectname, "__text");
            strcpy(NewHeader.segname,  "__TEXT");
         }
         else if (!stricmp(sname,".data") || !stricmp(sname,"_data")) {
            strcpy(NewHeader.sectname, "__data");
            strcpy(NewHeader.segname,  "__DATA");
         }
         else if (!strnicmp(sname+1,"bss", 3)) {
            strcpy(NewHeader.sectname, "__bss");
            strcpy(NewHeader.segname,  "__DATA");
         }
         else if (!strnicmp(sname+1,"const", 5) || !strnicmp(sname+1,"rodata", 6)) {
            strcpy(NewHeader.sectname, "__const");
            strcpy(NewHeader.segname,  "__DATA");
         }
         else if (!strnicmp(sname, ELF_CONSTRUCTOR_NAME, 5)) {
            // Constructors
            strcpy(NewHeader.sectname, MAC_CONSTRUCTOR_NAME);
            strcpy(NewHeader.segname,  "__DATA");
            NewHeader.flags = MAC_S_MOD_INIT_FUNC_POINTERS;
         }
         else if (OldHeader.sh_flags & SHF_EXECINSTR) {
            // Other code section
            if (strlen(NewHeader.sectname) > 16) err.submit(1040, NewHeader.sectname); // Warning: name truncated
            strncpy(NewHeader.sectname, sname, 16);
            strcpy(NewHeader.segname,  "__TEXT");
         }
         else {
            // Other data section. Truncate name to 16 characters
            if (strlen(NewHeader.sectname) > 16) err.submit(1040, NewHeader.sectname); // Warning: name truncated
            strncpy(NewHeader.sectname, sname, 16);
            strcpy(NewHeader.segname,  "__DATA");
         }
         if (NewHeader.sectname[0] == '.') {
            // Make sure name begins with '_'
            NewHeader.sectname[0] = '_';
         }

         // Raw data
         NewHeader.size = OldHeader.sh_size;  // section size in file

         // File  to raw data for section
         NewHeader.offset = NewRawData.GetDataSize() + RawDataOffset;

         if (OldHeader.sh_size && OldHeader.sh_type != SHT_NOBITS) { // Not for .bss segment
            // Copy raw data
            NewRawDataOffset = NewRawData.Push(this->Buf()+(uint32)OldHeader.sh_offset, (uint32)OldHeader.sh_size); 
            NewRawData.Align(4);
         }

         // Section flags
         if (OldHeader.sh_flags & SHF_EXECINSTR) {
            NewHeader.flags |= MAC_S_ATTR_PURE_INSTRUCTIONS | MAC_S_ATTR_SOME_INSTRUCTIONS;
         }
         else if (OldHeader.sh_type == SHT_NOBITS) {
            NewHeader.flags |= MAC_S_ZEROFILL;              // .bss segment
         }

         // Alignment
         NewHeader.align = FloorLog2((uint32)OldHeader.sh_addralign);
         if (NewHeader.align > 12) NewHeader.align = 12;   // What is the limit for highest alignment?
         int AlignBy = 1 << NewHeader.align;

         // Align memory address
         NewVirtualAddress = (NewVirtualAddress + AlignBy - 1) & -AlignBy;

         // Virtual memory address of new section 
         NewHeader.addr = NewVirtualAddress; 
         NewVirtualAddress += (uint32)OldHeader.sh_size;

         // Find relocation table for this section by searching through all sections
         for (relsec = 1; relsec < this->NSections; relsec++) {

            // Get section header
            OldRelHeader = this->SectionHeaders[relsec];            
            
            // Check if this is a relocations section referring to oldsec
            if ((OldRelHeader.sh_type == SHT_REL || OldRelHeader.sh_type == SHT_RELA) // if section is relocation
            && OldRelHeader.sh_info == oldsec) { // and if section refers to current section
               Elf2MacRelocations(OldRelHeader, NewHeader, NewRawDataOffset, oldsec);
            }
         } // End of search for relocation table

         // Align raw data for next section
         NewRawData.Align(4);

         // Fix v. 2.14: adjust NewVirtualAddress to match above alignment
         NewVirtualAddress = (NewVirtualAddress + 3) & MInt(-4);

         // Store section header in file
         ToFile.Push(&NewHeader, sizeof(NewHeader));

      } // End of if section has program data

   } // End of loop through old sections

} // End of function MakeSections


template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::FindUnusedSymbols() {
   // Check if symbols used, remove unused symbols

   // Allocate table OldSymbolScope and OldSymbolUsed
   NumOldSymbols = this->SymbolTableEntries;
   if (NumOldSymbols > 0 && NumOldSymbols < 0x1000000) {
      OldSymbolScope.SetNum(NumOldSymbols);
      OldSymbolScope.SetZero();
      OldSymbolUsed.SetNum(NumOldSymbols);
      OldSymbolUsed.SetZero();
   }

   // Loop through section headers
   for (uint32 sc = 0; sc < this->NSections; sc++) {
      uint32 entrysize = uint32(this->SectionHeaders[sc].sh_entsize);
      // printf("\n%2i Name: %-18s ", sc, SecStringTable + sheader.sh_name);

      if ((this->SectionHeaders[sc].sh_type==SHT_REL || this->SectionHeaders[sc].sh_type==SHT_RELA)) {
         // Relocation list
         int8 * reltab = this->Buf() + uint32(this->SectionHeaders[sc].sh_offset);
         int8 * reltabend = reltab + uint32(this->SectionHeaders[sc].sh_size);
         uint32 expectedentrysize = this->SectionHeaders[sc].sh_type == SHT_RELA ? 
            sizeof(TELF_Relocation) :              // Elf32_Rela, Elf64_Rela
            sizeof(TELF_Relocation) - this->WordSize/8;  // Elf32_Rel,  Elf64_Rel
         if (entrysize < expectedentrysize) {err.submit(2033); entrysize = expectedentrysize;}

         // Loop through entries
         for (; reltab < reltabend; reltab += entrysize) {
            int isymbol = ((TELF_Relocation*)reltab)->r_sym;
            // printf("\n>SymbolUsed: %i, Name: %s",isymbol,SymbolName(isymbol)); 
            // Remember symbol used
            OldSymbolUsed[isymbol]++;
         }
      }
   }
}

template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::MakeBinaryFile() {
   // Convert subfunction: Putting sections together
   // File header, segment command and section headers have been inserted.
   int i;

   // Update segment header for segment size in file
   ((TMAC_segment_command*)(ToFile.Buf()+CommandOffset))->filesize = NewRawData.GetDataSize();

   // Make symbol table command
   MAC_symtab_command symtab;
   memset(&symtab, 0, sizeof(symtab));
   symtab.cmd = MAC_LC_SYMTAB;
   symtab.cmdsize = sizeof(symtab);
   // symoff, nsyms, stroff, strsize inserted later
   // Store symtab command
   NewSymtabOffset = ToFile.Push(&symtab, sizeof(symtab));

   // Make MAC_dysymtab_command command
   MAC_dysymtab_command dysymtab;
   memset(&dysymtab, 0, sizeof(dysymtab));
   dysymtab.cmd        = MAC_LC_DYSYMTAB;
   dysymtab.cmdsize    = sizeof(dysymtab);
   dysymtab.ilocalsym  = 0;                            // index to local symbols
   dysymtab.nlocalsym  = NewSymTab[0].GetNumEntries(); // number of local symbols 
   dysymtab.iextdefsym = dysymtab.nlocalsym;           // index to externally defined symbols
   dysymtab.nextdefsym = NewSymTab[1].GetNumEntries(); // number of externally defined symbols 
   dysymtab.iundefsym  = dysymtab.iextdefsym + dysymtab.nextdefsym;	// index to public symbols
   dysymtab.nundefsym  = NewSymTab[2].GetNumEntries(); // number of public symbols
   // Store MAC_dysymtab_command command
   ToFile.Push(&dysymtab, sizeof(dysymtab));

   // Store section data
   uint32 Current = ToFile.Push(NewRawData.Buf(), NewRawData.GetDataSize());
   if (Current != RawDataOffset) err.submit(9000);

   ToFile.Align(4);

   // Store relocation tables
   uint32 Reltabs = ToFile.Push(NewRelocationTab.Buf(), NewRelocationTab.GetDataSize());

   // Initialize new string table. First string is empty
   NewStringTable.Push(0, 1);

   // Store symbol tables and make string table
   // Tables are not sorted alphabetically yet. This will be done subsequently
   // by CMAC2MAC
   uint32 Symtabs = ToFile.GetDataSize();
   uint32 NumSyms = 0;
   for (i = 0; i < 3; i++) {
      NumSyms += NewSymTab[i].GetNumEntries();
      NewSymTab[i].StoreList(&ToFile, &NewStringTable);
   }

   // Store string table
   uint32 StringTab = ToFile.Push(NewStringTable.Buf(), NewStringTable.GetDataSize());

   // Set missing values in file header
   ((TMAC_header*)ToFile.Buf())->sizeofcmds = RawDataOffset - sizeof(TMAC_header);

   // Adjust relocation offsets in section headers
   TMAC_section* sectp = (TMAC_section*)(ToFile.Buf() + NewSectHeadOffset);
   for (i = 0; i < NumSectionsNew; i++, sectp++) {
      sectp->reloff += Reltabs;
   }

   // Set missing symoff, nsyms, stroff, strsize in symtab command
   MAC_symtab_command * symtabp = (MAC_symtab_command*)(ToFile.Buf() + NewSymtabOffset);
   symtabp->symoff  = Symtabs;
   symtabp->nsyms   = NumSyms;
   symtabp->stroff  = StringTab;
   symtabp->strsize = NewStringTable.GetDataSize();
}

template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation,
          class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
int CELF2MAC<ELFSTRUCTURES,MACSTRUCTURES>::GetImagebaseSymbol() {
   // Get symbol table index of __mh_execute_header = image base
   // Create this symbol table entry if it doesn't exist

   const char * ImageBaseName = "__mh_execute_header";
   static int ImagebaseSymbol = -1;

   if (ImagebaseSymbol >= 0) {
      // Found previously
      return ImagebaseSymbol;
   }
   // Search for name among external symbols
   int index2 = NewSymTab[2].Search(ImageBaseName);
   if (index2 >= 0) {
      // found 
      ImagebaseSymbol = index2 + NumSymbols[2];
      return ImagebaseSymbol;
   }
   // Not found. Create symbol
   NewSymTab[2].AddSymbol(NumOldSymbols, ImageBaseName, MAC_N_EXT, 0, 0, 0);
   ImagebaseSymbol = NewSymTab[2].TranslateIndex(NumOldSymbols) + NumSymbols[2];
   NumSymbols[3]++;
   return ImagebaseSymbol;
}


// Make template instances for 32 and 64 bits
template class CELF2MAC<ELF32STRUCTURES,MAC32STRUCTURES>;
template class CELF2MAC<ELF64STRUCTURES,MAC64STRUCTURES>;
