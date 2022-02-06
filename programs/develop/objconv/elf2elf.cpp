/****************************    elf2elf.cpp    *****************************
* Author:        Agner Fog
* Date created:  2006-01-13
* Last modified: 2013-11-27
* Project:       objconv
* Module:        elf2elf.cpp
* Description:
* Module for changing symbol names in ELF file
*
* Copyright 2006-2013 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"
// All functions in this module are templated to make two versions: 32 and 64 bits.
// See instantiations at the end of this file.


// Constructor
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
CELF2ELF<ELFSTRUCTURES>::CELF2ELF() {
   // Initialize everything
   memset(this, 0, sizeof(*this));
}


// Convert()
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ELF<ELFSTRUCTURES>::Convert() {
   // Some compilers require this-> for accessing members of template base class,
   // according to the so-called two-phase lookup rule.
   MakeSymbolTable();               // Remake symbol tables and string tables
   ChangeSections();                // Modify section names and relocation table symbol indices
   MakeBinaryFile();                // Put everyting together into ToFile
   *this << ToFile;                 // Take over new file buffer
}


// MakeSymbolTable()
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ELF<ELFSTRUCTURES>::MakeSymbolTable() {
   uint32 SectionNumber;      // Section number
   char * SectionName;        // Section name
   uint32 SecNamei;           // Section name index
   uint32 OldSymi;            // Old symbol index
   uint32 NewSymi;            // New symbol index
   int isymt;                 // 0 = symtab, 1 = dynsym
   const char * name1;        // Old name of symbol
   const char * name2;        // Changed name of symbol
   int SymbolType;            // Symbol type for cmd.SymbolChange
   int action;                // Symbol change action
   int binding;               // Symbol binding
   TELF_Symbol sym;               // Symbol table entry
   TELF_Symbol AliasEntry;        // Symbol table alias entry
   uint32 symnamei;           // New symbol name index
   CMemoryBuffer TempGlobalSymbolTable; // Temporary storage of public and external symbols

   // Find symbol table and string tables
   for (SectionNumber = 0; SectionNumber < this->NSections; SectionNumber++) {
      // Get copy of 32-bit header or converted 64-bit header
      TELF_SectionHeader sheader = this->SectionHeaders[SectionNumber];
      switch (sheader.sh_type) {
      case SHT_SYMTAB:
         isymtab[0] = SectionNumber;                // Symbol table found
         istrtab[0] = this->SectionHeaders[SectionNumber].sh_link; // Associated string table
         break;

      case SHT_DYNSYM:
         isymtab[1] = SectionNumber;             // Dynamic symbol table found
         istrtab[1] = this->SectionHeaders[SectionNumber].sh_link; // Associated string table
         break;

      case SHT_STRTAB:
         SecNamei = sheader.sh_name;
         if (SecNamei >= this->SecStringTableLen) {
             err.submit(2112); return;}
         SectionName = this->SecStringTable + SecNamei;
         if (SectionNumber == this->FileHeader.e_shstrndx || !strcmp(SectionName,".shstrtab")) {
            istrtab[2] = SectionNumber;           // Section header string table found
         }
         else if (!strcmp(SectionName,".strtab") && !istrtab[0]) {
            istrtab[0] = SectionNumber;     // Symbol string table found
         }
         else if (!strcmp(SectionName,".stabstr")) {
            istrtab[3] = SectionNumber;    // Debug string table found
         }
         break;
      }
   }

   // Make new symbol tables and string tables
   // Loop through possibly two symbol tables
   for (isymt = 0; isymt < 2; isymt++) {

      if (isymtab[isymt] && isymtab[isymt] < this->NSections 
      &&  istrtab[isymt] && istrtab[isymt] < this->NSections) {
         
         // Symbol table header
         uint32 SymTabHeaderOffset = uint32(this->FileHeader.e_shoff + isymtab[isymt] * this->SectionHeaderSize);
         //TELF_SectionHeader SymTabHeader = this->Get<TELF_SectionHeader>(SymTabHeaderOffset);
         // Some compilers fail with the double template here. Avoid the template:
         TELF_SectionHeader SymTabHeader = *(TELF_SectionHeader*)(this->Buf() + SymTabHeaderOffset);

         // Find symbol table
         uint32 symtabsize = (uint32)(SymTabHeader.sh_size);
         int8 * symtab = this->Buf() + SymTabHeader.sh_offset;
         int8 * symtabend = symtab + symtabsize;
         int entrysize = (int)(SymTabHeader.sh_entsize);
         if (entrysize <= 0) entrysize = sizeof(TELF_Symbol);

         // Find string table
         char * StringTable = this->Buf() + this->SectionHeaders[istrtab[isymt]].sh_offset;
         uint32 StringTableLen = uint32(this->SectionHeaders[istrtab[isymt]].sh_size);

         NewStringTable[isymt].Push(0, 1); // Initialize new string table, first entry 0

         if (isymt == 0) {
            // Allocate NewSymbolIndex
            NumOldSymbols = (symtabsize + entrysize - 1) / entrysize; // round up to nearest integer to be safe
            NewSymbolIndex.SetNum(NumOldSymbols);   // Allocate array
            NewSymbolIndex.SetZero();               // Initialize
         }

         // Loop through old symbol table
         for (OldSymi = 0; symtab < symtabend; symtab += entrysize, OldSymi++) {

            // Symbol table entry
            sym = *(TELF_Symbol*)symtab;

            // Symbol name
            if (sym.st_name < StringTableLen) {
               name1 = StringTable + sym.st_name;}
            else {
               err.submit(2035);  name1 = 0;
            }
            name2 = 0;

            // Symbol type
            int type = sym.st_type;
            binding = sym.st_bind;
            if (binding == STB_LOCAL) {
               SymbolType = SYMT_LOCAL;    // Symbol is local
            }
            else if (type == STT_OBJECT || type == STT_FUNC || type == STT_NOTYPE) {
               if (int16(sym.st_shndx) > 0) { // Check section number
                  SymbolType = SYMT_PUBLIC;  // Symbol is public
               }
               else {
                  SymbolType = SYMT_EXTERNAL; // Symbol is external
               }
            }
            else {
               SymbolType = SYMT_OTHER; // Symbol is section or filename or debug
            }

            // Check if any change required for this symbol
            action = cmd.SymbolChange(name1, &name2, SymbolType);

            switch (action) {
            case SYMA_NOCHANGE:
               // No change
               break;

            case SYMA_MAKE_WEAK:
               // Make symbol weak
               if (cmd.OutputType == FILETYPE_COFF) {
                  // PE/COFF format does not support weak publics. Use this only when converting to ELF
                  err.submit(2200);
               }
               // Make weak
               binding = STB_WEAK;
               sym.st_bind = binding;
               sym.st_type = type ;
               break;

            case SYMA_MAKE_LOCAL:
               // Make public symbol local, make external symbol ignored
               binding = STB_LOCAL;  SymbolType = SYMT_LOCAL;
               sym.st_bind = binding;
               sym.st_type = type ;
               break;

            case SYMA_CHANGE_NAME:
               // Change name of symbol
               name1 = name2;  name2 = 0;
               break;

            case SYMA_ALIAS: 
               // Make alias and keep old name
               if (isymt != 0) err.submit(1033, name1); // alias in dynsym not supported yet
               AliasEntry = sym;
               break;

            default:
               err.submit(9000); // unknown error
            }

            // Add entry to new string table
            if (name1 && *name1) {
               symnamei = NewStringTable[isymt].PushString(name1);
            }
            else {
               symnamei = 0;
            }
            sym.st_name = symnamei;

            if (isymt == 0) {
               // The symtab symbol table must be ordered with local symbols first.
               // Therefore the public and external symbols are temporarily stored
               // in TempGlobalSymbolTable and the high bit of NewSymi is set.
               // The two tables are joined together when the number of local symbols
               // is known and the indexes into TempGlobalSymbolTable are adjusted
               // to indexes into the joined table.
               if (SymbolType == SYMT_LOCAL) {
                  NewSymbolTable[isymt].Push(&sym, entrysize);
                  NewSymi = NewSymbolTable[isymt].GetLastIndex();
               }
               else {
                  TempGlobalSymbolTable.Push(&sym, entrysize);
                  NewSymi = TempGlobalSymbolTable.GetLastIndex() | 0x80000000;
               }
               // Insert into symbol index translation table
               NewSymbolIndex[OldSymi] = NewSymi;

               if (action == SYMA_ALIAS && name2 && *name2) {
                  // Make one more entry for new alias
                  symnamei = NewStringTable[isymt].PushString(name2);
                  AliasEntry.st_name = symnamei;
                  TempGlobalSymbolTable.Push(&AliasEntry, entrysize);
               }
            }
            else {
               // dynsym table has no local symbols
               // no index translation table is currently needed
               NewSymbolTable[isymt].Push(&sym, entrysize);
            }

         } // End of loop through old symbol table

         if (isymt == 0) {
            // The symbol table has been temporarily split into local and non-local
            // Save index to first nonlocal symbol
            FirstGlobalSymbol = NewSymbolTable[isymt].GetNumEntries();

            // Adjust symbol index translation table
            for (OldSymi = 0; OldSymi < NumOldSymbols; OldSymi++) {
               if (NewSymbolIndex[OldSymi] & 0x80000000) {
                  // Translate index into TempGlobalSymbolTable to index into joined table
                  NewSymbolIndex[OldSymi] = (NewSymbolIndex[OldSymi] & ~0x80000000) + FirstGlobalSymbol;
               }
            }

            // Join the two tables
            NewSymbolTable[isymt].Push(TempGlobalSymbolTable.Buf(), TempGlobalSymbolTable.GetDataSize());
         }
      }
   } // End of isymt loop through possibly two symbol tables
}


// ChangeSections()
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ELF<ELFSTRUCTURES>::ChangeSections() {
   // Convert subfunction: Change section names if needed and adjust all relocation tables
   uint32 SectionNumber;        // Section number
   const char * name1;          // Section name
   const char * name2;          // Changed section name
   int action;                  // Name change action
   TELF_SectionHeader * sheaderp;   // Pointer to section header
   uint32 SectionHeaderOffset;  // File offset to section header
   uint32 namei;                // Section name index into string table
   TELF_Relocation * relocp;        // Pointer to relocation entry
   uint32 oldsymi, newsymi;     // Relocation symbol index

   // Initialize section header string table .shstrtab. First entry = 0
   NewStringTable[2].Push(0, 1);

   // Loop through sections
   SectionHeaderOffset = uint32(this->FileHeader.e_shoff);
   for (SectionNumber = 0; SectionNumber < this->NSections; SectionNumber++, SectionHeaderOffset += this->FileHeader.e_shentsize) {

      // Get section header
      sheaderp = (TELF_SectionHeader*)(this->Buf() + SectionHeaderOffset);

      // Section name
      namei = sheaderp->sh_name;
      if (namei >= this->SecStringTableLen) {
          err.submit(2112); sheaderp->sh_name = 0; return;}
      name1 = this->SecStringTable + namei;

      // Check if name change
      action = cmd.SymbolChange(name1, &name2, SYMT_SECTION);
      if (action == SYMA_CHANGE_NAME) name1 = name2;

      // Store name in .shstrtab string table
      if (name1 && *name1) {
         namei = NewStringTable[2].PushString(name1);
      }
      else {
         namei = 0;
      }
      sheaderp->sh_name = namei;   // Put new string index into section header

      if (sheaderp->sh_type == SHT_REL || sheaderp->sh_type == SHT_RELA) {
         // This is a relocation section. Update all symbol indices

         int8 * reltab = this->Buf() + sheaderp->sh_offset;
         int8 * reltabend = reltab + sheaderp->sh_size;
         int entrysize = (int)(sheaderp->sh_entsize);
         if (entrysize <= 0) entrysize = sizeof(TELF_Relocation);

         // Loop through entries
         for (; reltab < reltabend; reltab += entrysize) {
            relocp = (TELF_Relocation*)reltab;

            oldsymi = relocp->r_sym;

            if (oldsymi >= NumOldSymbols) {
               err.submit(2040);  oldsymi = 0;
            }
            // Translate symbol index
            newsymi = NewSymbolIndex[oldsymi];

            // Put back into relocation entry
            relocp->r_sym = newsymi;
         }
      }
   }
}


// MakeBinaryFile()
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF2ELF<ELFSTRUCTURES>::MakeBinaryFile() {

   uint32 SectionNumber;               // Section number
   CMemoryBuffer NewSectionHeaders;    // Temporary storage of section headers

   // Copy file header
   ToFile.Push(this->Buf(), sizeof(TELF_Header));

   // Copy program header if any
   if (this->FileHeader.e_phnum) {
      ToFile.Push(this->Buf() + this->FileHeader.e_phoff, this->FileHeader.e_phentsize * this->FileHeader.e_phnum);
      ((TELF_Header*)ToFile.Buf())->e_phoff = sizeof(TELF_Header);
   }

   // Copy section data
   uint32 SectionHeaderOffset = uint32(this->FileHeader.e_shoff);
   TELF_SectionHeader sheader;                     // Section header

   // Loop through sections
   for (SectionNumber = 0; SectionNumber < this->NSections; SectionNumber++, SectionHeaderOffset += this->FileHeader.e_shentsize) {

      // Get section header
      //sheader = this->Get<TELF_SectionHeader>(SectionHeaderOffset);
      // Some compilers fail with the double template here. Avoid the template:
      sheader = *(TELF_SectionHeader*)(this->Buf() + SectionHeaderOffset);

      // Check for null section
      if (SectionNumber == 0 && sheader.sh_type != 0) {
         // First section must be null
         err.submit(2036, 0);
      }

      // Align
      ToFile.Align(16);

      // Check for sections that have been modified
      if (SectionNumber == isymtab[0]) {
         // Static symbol table .symtab
         sheader.sh_offset = ToFile.Push(NewSymbolTable[0].Buf(), NewSymbolTable[0].GetDataSize());
         sheader.sh_size = NewSymbolTable[0].GetDataSize();
         sheader.sh_info = FirstGlobalSymbol;
      }
      else if (SectionNumber == isymtab[1]) {
         // Dynamic symbol table .dynsym
         sheader.sh_offset = ToFile.Push(NewSymbolTable[1].Buf(), NewSymbolTable[1].GetDataSize());
         sheader.sh_size = NewSymbolTable[1].GetDataSize();
      }
      else if (SectionNumber == istrtab[0]) {
         // Symbol string table .strtab
         sheader.sh_offset = ToFile.Push(NewStringTable[0].Buf(), NewStringTable[0].GetDataSize());
         sheader.sh_size = NewStringTable[0].GetDataSize();
      }
      else if (SectionNumber == istrtab[1] && SectionNumber != istrtab[0]) {
         // Dynamic symbol string table if different from .strtab
         sheader.sh_offset = ToFile.Push(NewStringTable[1].Buf(), NewStringTable[1].GetDataSize());
         sheader.sh_size = NewStringTable[1].GetDataSize();
      }
      else if (SectionNumber == istrtab[2]) {
         // Section name string table .shstrtab
         sheader.sh_offset = ToFile.Push(NewStringTable[2].Buf(), NewStringTable[2].GetDataSize());
         sheader.sh_size = NewStringTable[2].GetDataSize();
      }
      else if (sheader.sh_type == SHT_NOBITS) {
         // BSS section. Nothing
         ;
      }
      else {
         // Any other section (including istrtab[3] = .stabstr)
         sheader.sh_offset = ToFile.Push(this->Buf() + (uint32)sheader.sh_offset, (uint32)sheader.sh_size);
      }

      // Store section header
      NewSectionHeaders.Push(&sheader, sizeof(sheader));

   } // End of section loop

   // Align
   ToFile.Align(16);

   // Store section headers
   uint32 SectionHeadersOffset = ToFile.Push(NewSectionHeaders.Buf(), NewSectionHeaders.GetDataSize());

   // Update file header
   ((TELF_Header*)ToFile.Buf())->e_shoff = SectionHeadersOffset;
   ((TELF_Header*)ToFile.Buf())->e_shentsize = sizeof(TELF_SectionHeader);
   ((TELF_Header*)ToFile.Buf())->e_shnum = NewSectionHeaders.GetNumEntries();
   ((TELF_Header*)ToFile.Buf())->e_shstrndx = istrtab[2];
}


// Make template instances for 32 and 64 bits
template class CELF2ELF<ELF32STRUCTURES>;
template class CELF2ELF<ELF64STRUCTURES>;
