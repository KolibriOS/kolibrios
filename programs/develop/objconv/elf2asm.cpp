/****************************  elf2asm.cpp   *********************************
* Author:        Agner Fog
* Date created:  2007-04-22
* Last modified: 2016-11-06
* Project:       objconv
* Module:        elf2asm.cpp
* Description:
* Module for disassembling ELF
*
* Copyright 2007-2016 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"
// All functions in this module are templated to make two versions: 32 and 64 bits.
// See instantiations at the end of this file.


// Constructor
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
CELF2ASM<ELFSTRUCTURES>::CELF2ASM() {
}


// FindImageBase()
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::FindImageBase() {
   // Find image base if executable file

   // Check if executable
   switch (this->FileHeader.e_type) {
   case ET_REL: default:
      // Not an executable file
      ExeType = 0;  ImageBase = 0;
      return;
   case ET_DYN: // Shared object
      ExeType = 1;
      break;
   case ET_EXEC: // Executable file
      ExeType = 2;
      break;
   }

   // Loop through sections to find the first allocated section
   for (uint32 sc = 0; sc < this->NSections; sc++) {
      if (this->SectionHeaders[sc].sh_type == SHT_PROGBITS    // Must be code or data section
      && (this->SectionHeaders[sc].sh_flags & SHF_ALLOC)      // Must be allocated
      && this->SectionHeaders[sc].sh_offset <= this->SectionHeaders[sc].sh_addr) { // Avoid negative
         // Image base can be calculated from this section
         ImageBase = this->SectionHeaders[sc].sh_addr - this->SectionHeaders[sc].sh_offset;
         // Make sure ImageBase is divisible by page size
         ImageBase = ImageBase & - 0x1000;
         // Stop searching
         return;
      }
   }
   // Failure. Cannot compute image base from any of the sections
   ImageBase = 0;
   return;    
}


// Convert
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::Convert() {
   // Do the conversion

   // Find image base and executable type
   FindImageBase();

   // Tell disassembler
   Disasm.Init(ExeType, ImageBase);                       // Set image base

   // Make Sections list in Disasm
   MakeSectionList();

   // Make Symbols list in Disasm
   MakeSymbolList();

   // Make relocations for object and executable files
   MakeRelocations();

   if (ImageBase) {
      // Executable file
      MakeImportList();                          // Make imported symbols for executable files
      MakeExportList();                          // Make exported symbols for executable files
      MakeListLabels();                          // Put labels on all image directory tables
   }
   Disasm.Go();                                  // Disassemble
   *this << Disasm.OutFile;                      // Take over output file from Disasm
}

// MakeSectionList
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::MakeSectionList() {
   // Make Sections list and Relocations list in Disasm

   // Allocate array for translating oroginal section numbers to new index
   SectionNumberTranslate.SetNum(this->NSections + 1);
   uint32 NewSectionIndex = 0;

   for (uint32 sc = 0; sc < this->NSections; sc++) {
      // Get copy of 32-bit header or converted 64-bit header
      TELF_SectionHeader sheader = this->SectionHeaders[sc];
      //int entrysize = (uint32)(sheader.sh_entsize);
      uint32 namei = sheader.sh_name;
      if (namei >= this->SecStringTableLen) {err.submit(2112); break;}

//      if (sheader.sh_type == SHT_PROGBITS || sheader.sh_type == SHT_NOBITS) {
//         // This is a code, data or bss section

      if (sheader.sh_flags & SHF_ALLOC) {
         // This is an allocated section

         // Give it a new index
         SectionNumberTranslate[sc] = ++NewSectionIndex;

         // Get section parameters
         uint8 * Buffer = (uint8*)(this->Buf()) + (uint32)sheader.sh_offset;
         uint32  InitSize = (sheader.sh_type == SHT_NOBITS) ? 0 : (uint32)sheader.sh_size;
         uint32  TotalSize = (uint32)sheader.sh_size;
         uint32  SectionAddress = (uint32)sheader.sh_addr - (uint32)ImageBase;
         uint32  Align = FloorLog2((uint32)sheader.sh_addralign);
         const char * Name = this->SecStringTableLen ? this->SecStringTable + namei : "???";

         // Detect segment type
         uint32  Type = 0;
         if (sheader.sh_flags & SHF_ALLOC) {
            // Allocate
            if (sheader.sh_type == SHT_NOBITS) {
               // Uninitialized data
               Type = 3;
            }
            else if (sheader.sh_flags & SHF_EXECINSTR) {
               // Executable
               Type = 1;
            }
            else if (!(sheader.sh_flags & SHF_WRITE)) {
               // Not writeable
               Type = 4;
            }
            else {
               // Initialized writeable data
               Type = 2;
            }
         }

         // Save section record
         Disasm.AddSection(Buffer, InitSize, TotalSize, SectionAddress, Type, Align, this->WordSize, Name);
      }
   }
}

// MakeSymbolList
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::MakeSymbolList() {
   // Make Symbols list in Disasm

   // Allocate array for translate symbol indices for multiple symbol tables in
   // source file to a single symbol table in disassembler
   SymbolTableOffset.SetNum(this->NSections + 1);
   NumSymbols = 0;

   for (uint32 sc = 0; sc < this->NSections; sc++) {
      // Get copy of 32-bit header or converted 64-bit header
      TELF_SectionHeader sheader = this->SectionHeaders[sc];
      int entrysize = (uint32)(sheader.sh_entsize);

      if (sheader.sh_type==SHT_SYMTAB || sheader.sh_type==SHT_DYNSYM) {
         // This is a symbol table

         // Offset for symbols in this symbol table = number of preceding symbols from other symbol tables
         SymbolTableOffset[sc] = NumSymbols;

         // Find associated string table
         if (sheader.sh_link >= this->NSections) {err.submit(2035); sheader.sh_link = 0;}
         int8 * strtab = this->Buf() + uint32(this->SectionHeaders[sheader.sh_link].sh_offset);

         // Find symbol table
         uint32 symtabsize = (uint32)(sheader.sh_size);
         int8 * symtab = this->Buf() + uint32(sheader.sh_offset);
         int8 * symtabend = symtab + symtabsize;
         if (entrysize < (int)sizeof(TELF_Symbol)) {err.submit(2033); entrysize = (int)sizeof(TELF_Symbol);}

         // Loop through symbol table
         uint32 symi1;                           // Symbol number in this table
         uint32 symi2;                           // Symbol number in joined table
         symtab += entrysize;                    // Skip symbol number 0
         for (symi1 = 1; symtab < symtabend; symtab += entrysize, symi1++) {

            // Symbol number in joined table = symi1 + number of symbols in preceding tables
            symi2 = SymbolTableOffset[sc] + symi1;
            
            // Copy 32 bit symbol table entry or convert 64 bit entry
            TELF_Symbol sym = *(TELF_Symbol*)symtab;

            // Parameters
            uint32 Offset = uint32(sym.st_value);
            uint32 Size = (uint32)sym.st_size;

            // Get section
            int32 Section = int16(sym.st_shndx);
            if (Section >= (int32)(this->NSections)) {
               // Error. wrong section
               Section = 0;
            }
            if (Section > 0) {
               // Translate to new section index
               Section = SectionNumberTranslate[Section];
            }
            else if ((int16)Section < 0) {
               // Special section values
               if ((int16)Section == SHN_ABS) {
                  // Absolute symbol
                  Section = ASM_SEGMENT_ABSOLUTE;
               }
               else {
                  // Other special values
                  Section = ASM_SEGMENT_ERROR;
               }
            }

            // Get name
            const char * Name = 0;
            if (*(strtab + sym.st_name)) {
               Name = strtab + sym.st_name;
            }
            
            // Get import .so name
            const char * DLLName = 0;
            if (sheader.sh_type==SHT_DYNSYM && sym.st_value == 0 
            && sym.st_shndx == 0 && sym.st_size > 0) {
               // I don't know how to find out which .so the symbol is imported from
               // It must be something in the .dynamic section.
               DLLName = "?.so";
            }

            // Get scope
            uint32 Scope = 0;
            switch (sym.st_bind) {
            case STB_LOCAL:
               Scope = 2;
               break;
            case STB_WEAK:
               Scope = 8;
               if (Section > 0) break;
               // Section == 0: continue as global
            case STB_GLOBAL:
               // Public or external
               Scope = (sym.st_shndx > 0) ? 4 : 0x20;
               break;
            }
            // Get type
            uint32 Type = 0;

            if (sym.st_type == STT_FUNC) {
               // Function
               Type = 0x83;
            }
            else if (sym.st_type == STT_GNU_IFUNC) {
               // Gnu indirect function
               Type = 0x40000083;
            }
            else if (sym.st_type == STT_OBJECT) {
               // Probably a data object
               switch (Size) {
               case 1:
                  Type = 1; 
                  break;
               case 2:
                  Type = 2; 
                  break;
               case 4:
                  Type = 3;
                  break;
               case 8:
                  Type = 4;
                  break;
               default:
                  Type = 1;
                  break;
               }
            }
            else if (sym.st_type == STT_COMMON) {
               // Communal?
               Type = 0;
               Scope = 0x10;
            }
            else if (sym.st_type == STT_SECTION) {
               // This is a section
               Type = 0x80000082;
               Scope = 0;
            }
            else if (sym.st_type == STT_NOTYPE) {
               Type = 0;
            }
            else if (sym.st_type == STT_FILE) {
               // file name. ignore
               continue;
            }            
            else {
               // unknown type. warning
               err.submit(1062, Name);
               Type = 0;
               //continue;
            }

            if (Scope != 0x20) {
               // Not external
               // Check if offset is absolute or section relative
               if (ExeType && Offset >= (uint32)ImageBase) {
                  // Offset is absolute address
                  if (Section >= 0 
                     && (uint32)Section < this->NSections 
                     && Offset >= (uint32)this->SectionHeaders[Section].sh_addr
                     && Offset - (uint32)this->SectionHeaders[Section].sh_addr < (uint32)(this->SectionHeaders[Section].sh_size)) {
                        // Change to section relative offset
                        Offset -= (uint32)(this->SectionHeaders[Section].sh_addr);
                     }
                  else {
                     // Address is outside specified section or otherwise inconsistent. 
                     // Let Disasm try to find the address
                     Section = ASM_SEGMENT_IMGREL;
                     Offset -= (uint32)ImageBase;
                  }
               }
            }

            // Store new symbol record
            Disasm.AddSymbol(Section, Offset, Size, Type, Scope, symi2, Name, DLLName);

            // Count symbols
            NumSymbols++;
         }
      }
   }
}

// MakeRelocations
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::MakeRelocations() {
   // Make relocations for object and executable files

   int32 Section;                                // Source section new index

   // Loop through sections
   for (uint32 sc = 0; sc < this->NSections; sc++) {
      // Get copy of 32-bit header or converted 64-bit header
      TELF_SectionHeader sheader = this->SectionHeaders[sc];
      int entrysize = (uint32)(sheader.sh_entsize);

      if (sheader.sh_type == SHT_REL || sheader.sh_type == SHT_RELA) {
         // Relocations section
         int8 * reltab = this->Buf() + uint32(sheader.sh_offset);
         int8 * reltabend = reltab + uint32(sheader.sh_size);
         int expectedentrysize = sheader.sh_type == SHT_RELA ? 
            sizeof(TELF_Relocation) :              // Elf32_Rela, Elf64_Rela
            sizeof(TELF_Relocation) - this->WordSize/8;  // Elf32_Rel,  Elf64_Rel
         if (entrysize < expectedentrysize) {err.submit(2033); entrysize = expectedentrysize;}

         // Loop through entries
         for (; reltab < reltabend; reltab += entrysize) {
            // Copy relocation table entry with or without addend
            TELF_Relocation rel;  rel.r_addend = 0;
            memcpy(&rel, reltab, entrysize);

            // Get section-relative or absolute address
            uint32  Offset = (uint32)rel.r_offset;

            // Get addend, if any
            int32   Addend = (uint32)rel.r_addend;

            // Find target symbol
            uint32  TargetIndex = rel.r_sym;
            if (sheader.sh_link < this->NSections) {
               // sh_link indicates which symbol table r_sym refers to
               TargetIndex += SymbolTableOffset[sheader.sh_link];
            }

            // Find section
            if (sheader.sh_info < this->NSections) {            
               Section = SectionNumberTranslate[sheader.sh_info];
            }
            else {
               // Not found. Try to let disasm find by absolute address
               Section = ASM_SEGMENT_IMGREL;
               if (Offset < (uint32)ImageBase) Offset += (uint32)ImageBase;
            }

            // Get relocation type and size
            uint32  Type = 0;
            uint32  Size = 0;
            if (this->WordSize == 32) {
               switch (rel.r_type) {
               case R_386_RELATIVE: // Adjust by program base
                  Type = 0x21;  Size = 4;
                  break;
               case R_386_JMP_SLOT: // Create PLT entry
                  Type = 0x41;  Size = 4;
                  break;
               case R_386_PLT32: // Self-relative to PLT
                  Type = 0x2002;  Size = 4;
                  break;
               case R_386_32:
                  // Direct 32 bit
                  Type = 1;  Size = 4;
                  break;
               case R_386_PC32:
                  // Self-relative 32 bit
                  Type = 2;  Size = 4;
                  break;
               case R_386_GOTPC:
                  // Self-relative offset to GOT
                  Type = 0x1002;  Size = 4;
                  break;
               case R_386_IRELATIVE:
                  // Reference to Gnu indirect function
                  Type = 0x81;  Size = 4;
                  break;
               case R_386_GLOB_DAT: 
               case R_386_GOT32:
               case R_386_GOTOFF:
                  // Create GOT entry
                  Type = 0x1001;  Size = 4;
                  break;
               }
            }
            else {
               // 64 bit
               switch (rel.r_type) {
               case R_X86_64_RELATIVE:  // Adjust by program base
                  Type = 0x21;  Size = 8;
                  break;
               case R_X86_64_JUMP_SLOT: // Create PLT entry
                  Type = 0x41;  Size = 8;
                  break;
               case R_X86_64_64: 
                  // Direct 64 bit
                  Type = 1;  Size = 8;
                  break;
               case R_X86_64_PC32:
                  // Self relative 32 bit signed
                  Type = 2;  Size = 4;
                  break;
               case R_X86_64_32: case R_X86_64_32S:
                  // Direct 32 bit zero extended or sign extend
                  Type = 1;  Size = 4;
                  break;
               case R_X86_64_16:
                  // Direct 16 bit zero extended
                  Type = 1;  Size = 2;
                  break;
               case R_X86_64_PC16:
                  // 16 bit sign extended pc relative
                  Type = 2;  Size = 2;
                  break;
               case R_X86_64_8:
                  // Direct 8 bit sign extended
                  Type = 1;  Size = 1;
                  break;
               case R_X86_64_PC8:
                  // 8 bit sign extended pc relative
                  Type = 2;  Size = 1;
                  break;
               case R_X86_64_GOTPCREL:
                  // Self relative 32 bit signed offset to GOT entry
                  Type = 0x1002;  Size = 4;
                  break;
               case R_X86_64_IRELATIVE:
                  // Reference to Gnu indirect function
                  Type = 0x81;  Size = 4;
                  break;
               case R_X86_64_PLT32:  // Self-relative to PLT
                  Type = 0x2002;  Size = 4;
                  break;
               case R_X86_64_GLOB_DAT:  // Create GOT entry
               case R_X86_64_GOT32:
                  Type = 0x1001;  Size = 4;
                  break;
               }
            }

            // Check if offset is absolute or section relative
            if (ImageBase && Offset > (uint32)ImageBase) {
               // Offset is absolute address
               if (Section > 0 && (uint32)Section < this->NSections 
               && Offset >= (uint32)(this->SectionHeaders[Section].sh_addr)
               && Offset - (uint32)(this->SectionHeaders[Section].sh_addr) < (uint32)(this->SectionHeaders[Section].sh_size)) {
                  // Change to section relative offset
                  Offset -= (uint32)(this->SectionHeaders[Section].sh_addr);
               }
               else {
                  // Inconsistent. Let Disasm try to find the address
                  Section = ASM_SEGMENT_IMGREL;
                  Offset -= (uint32)ImageBase;
               }
            }

            // Save relocation record
            Disasm.AddRelocation(Section, Offset, Addend, Type, Size, TargetIndex);
         }
      }
   }
}


// MakeImportList
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::MakeImportList() {
   // Make imported symbols for executable files
}

// MakeExportList
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::MakeExportList() {
   // Make exported symbols for executable files
}

// MakeListLabels
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ASM<ELFSTRUCTURES>::MakeListLabels() {
   // Attach names to all image directories
}


// Make template instances for 32 and 64 bits
template class CELF2ASM<ELF32STRUCTURES>;
template class CELF2ASM<ELF64STRUCTURES>;
