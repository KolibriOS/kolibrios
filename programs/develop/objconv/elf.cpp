/****************************    elf.cpp    *********************************
* Author:        Agner Fog
* Date created:  2006-07-18
* Last modified: 2017-10-18
* Project:       objconv
* Module:        elf.cpp
* Description:
* Module for reading ELF files
*
* Class CELF is used for reading, interpreting and dumping ELF files.
*
* Copyright 2006-2017 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"
// All functions in this module are templated to make two versions: 32 and 64 bits.
// See instantiations at the end of this file.


// File class names
SIntTxt ELFFileClassNames[] = {
   {ELFCLASSNONE,      "None"},
   {ELFCLASS32,        "32-bit object"},
   {ELFCLASS64,        "64-bit object"}
};

// Data encoding names
SIntTxt ELFDataEncodeNames[] = {
   {ELFDATANONE,        "None"},
   {ELFDATA2LSB,        "Little Endian"},
   {ELFDATA2MSB,        "Big Endian"}
};

// ABI names
SIntTxt ELFABINames[] = {
   {ELFOSABI_SYSV,      "System V"},
   {ELFOSABI_HPUX,      "HP-UX"},
   {ELFOSABI_ARM,       "ARM"},
   {ELFOSABI_STANDALONE,"Embedded"},
};

// File type names
SIntTxt ELFFileTypeNames[] = {
   {ET_NONE,   "None"},
   {ET_REL,    "Relocatable"},
   {ET_EXEC,   "Executable"},
   {ET_DYN,    "Shared object"},
   {ET_CORE,   "Core file"}
};

// Section type names
SIntTxt ELFSectionTypeNames[] = {
   {SHT_NULL,          "None"},
   {SHT_PROGBITS,      "Program data"},
   {SHT_SYMTAB,        "Symbol table"},
   {SHT_STRTAB,        "String table"},
   {SHT_RELA,          "Relocation w addends"},
   {SHT_HASH,          "Symbol hash table"},
   {SHT_DYNAMIC,       "Dynamic linking info"},
   {SHT_NOTE,          "Notes"},
   {SHT_NOBITS,        "bss"},
   {SHT_REL,           "Relocation entries"},
   {SHT_SHLIB,         "Reserved"},
   {SHT_DYNSYM,        "Dynamic linker symbol table"},
   {SHT_INIT_ARRAY,    "Array of constructors"},
   {SHT_FINI_ARRAY,    "Array of destructors"},
   {SHT_PREINIT_ARRAY, "Array of pre-constructors"},
   {SHT_GROUP,         "Section group"},
   {SHT_SYMTAB_SHNDX,  "Extended section indices"}
};

// Section flag names
SIntTxt ELFSectionFlagNames[] = {
   {SHF_WRITE,         "Writeable"},
   {SHF_ALLOC,         "Allocate"},
   {SHF_EXECINSTR,     "Executable"},
   {SHF_MERGE,         "Merge"},
   {SHF_STRINGS,       "Strings"},
   {SHF_INFO_LINK,     "sh_info"},
   {SHF_LINK_ORDER,    "Preserve order"},
   {SHF_OS_NONCONFORMING,"OS specific"}
};

// Symbol binding names
SIntTxt ELFSymbolBindingNames[] = {
   {STB_LOCAL,  "Local"},
   {STB_GLOBAL, "Global"},
   {STB_WEAK,   "Weak"}
};

// Symbol Type names
SIntTxt ELFSymbolTypeNames[] = {
   {STT_NOTYPE,  "None"},
   {STT_OBJECT,  "Object"},
   {STT_FUNC,    "Function"},
   {STT_SECTION, "Section"},
   {STT_FILE,    "File"},
   {STT_COMMON,  "Common"},
   {STT_GNU_IFUNC, "Indirect function/dispatcher"}
};

// Relocation type names x86 32 bit
SIntTxt ELF32RelocationNames[] = {
   {R_386_NONE,         "None"},
   {R_386_32,           "Absolute 32 bit"},
   {R_386_PC32,         "Self-relative 32 bit"},
   {R_386_GOT32,        "32 bit GOT entry"},
   {R_386_PLT32,        "32 bit PLT address"},
   {R_386_COPY,         "Copy symbol at runtime"},
   {R_386_GLOB_DAT,     "Create GOT entry"},
   {R_386_JMP_SLOT,     "Create PLT entry"},
   {R_386_RELATIVE,     "Adjust by image base"},
   {R_386_GOTOFF,       "32 bit offset to GOT"},
   {R_386_GOTPC,        "32 bit PC relative offset to GOT"},
   {R_386_IRELATIVE,    "32 bit ref. to indirect function PLT"}
};

// Relocation type names x86 64 bit
SIntTxt ELF64RelocationNames[] = {
   {R_X86_64_NONE,      "None"},
   {R_X86_64_64,        "Direct 64 bit"},
   {R_X86_64_PC32,      "Self relative 32 bit signed"},
   {R_X86_64_GOT32,     "32 bit GOT entry"},
   {R_X86_64_PLT32,     "32 bit PLT address"},
   {R_X86_64_COPY,      "Copy symbol at runtime"},
   {R_X86_64_GLOB_DAT,  "Create GOT entry"},
   {R_X86_64_JUMP_SLOT, "Create PLT entry"},
   {R_X86_64_RELATIVE,  "Adjust by program base"},
   {R_X86_64_GOTPCREL,  "32 bit signed pc relative offset to GOT"},
   {R_X86_64_32,        "Direct 32 bit zero extended"},
   {R_X86_64_32S,       "Direct 32 bit sign extended"},
   {R_X86_64_16,        "Direct 16 bit zero extended"},
   {R_X86_64_PC16,      "16 bit sign extended pc relative"},
   {R_X86_64_8,         "Direct 8 bit sign extended"},
   {R_X86_64_PC8,       "8 bit sign extended pc relative"},
   {R_X86_64_IRELATIVE, "32 bit ref. to indirect function PLT"}
};


// Machine names
SIntTxt ELFMachineNames[] = {
   {EM_NONE,        "None"},     // No machine
   {EM_M32,         "AT&T WE 32100"},
   {EM_SPARC,       "SPARC"},
   {EM_386,         "Intel x86"},
   {EM_68K,         "Motorola m68k"},
   {EM_88K,         "Motorola m88k"},
   {EM_860,         "MIPS R3000 big-endian"},
   {EM_MIPS,        "MIPS R3000 big-endian"},
   {EM_S370,        "IBM System/370"},
   {EM_MIPS_RS3_LE, "NMIPS R3000 little-endianone"},
   {EM_PARISC,      "HPPA"},
   {EM_VPP500,      "Fujitsu VPP500"},
   {EM_SPARC32PLUS, "Sun v8plus"},
   {EM_960,         "Intel 80960"},
   {EM_PPC,         "PowerPC"},
   {EM_PPC64,       "PowerPC 64-bit"},
   {EM_S390,        "IBM S390"},
   {EM_V800,        "NEC V800"},
   {EM_FR20,        "Fujitsu FR20"},
   {EM_RH32,        "TRW RH-32"},
   {EM_RCE,         "Motorola RCE"},
   {EM_ARM,         "ARM"},
   {EM_FAKE_ALPHA,  "Digital Alpha"},
   {EM_SH,          "Hitachi SH"},
   {EM_SPARCV9,     "SPARC v9 64-bit"},
   {EM_TRICORE,     "Siemens Tricore"},
   {EM_ARC,         "Argonaut RISC"},
   {EM_H8_300,      "Hitachi H8/300"},
   {EM_H8_300H,     "Hitachi H8/300H"},
   {EM_H8S,         "Hitachi H8S"},
   {EM_H8_500,      "EM_H8_500"},
   {EM_IA_64,       "Intel IA64"},
   {EM_MIPS_X,      "Stanford MIPS-X"},
   {EM_COLDFIRE,    "Motorola Coldfire"},
   {EM_68HC12,      "Motorola M68HC12"},
   {EM_MMA,         "Fujitsu MMA"},
   {EM_PCP,         "Siemens PCP"},
   {EM_NCPU,        "Sony nCPU"},
   {EM_NDR1,        "Denso NDR1"},
   {EM_STARCORE,    "Motorola Start*Core"},
   {EM_ME16,        "Toyota ME16"},
   {EM_ST100,       "ST100"},
   {EM_TINYJ,       "Tinyj"},
   {EM_X86_64,      "x86-64"},
   {EM_PDSP,        "Sony DSP"},
   {EM_FX66,        "Siemens FX66"},
   {EM_ST9PLUS,     "ST9+ 8/16"},
   {EM_ST7,         "ST7 8"},
   {EM_68HC16,      "MC68HC16"},
   {EM_68HC11,      "MC68HC11"},
   {EM_68HC08,      "MC68HC08"},
   {EM_68HC05,      "MC68HC05"},
   {EM_SVX,         "SVx"},
   {EM_AT19,        "ST19"},
   {EM_VAX,         "VAX"},
   {EM_CRIS,        "Axis"},
   {EM_JAVELIN,     "Infineon"},
   {EM_FIREPATH,    "Element 14"},
   {EM_ZSP,         "LSI Logic"},
   {EM_HUANY,       "Harvard"},
   {EM_PRISM,       "SiTera Prism"},
   {EM_AVR,         "Atmel AVR"},
   {EM_FR30,        "FR30"},
   {EM_D10V,        "D10V"},
   {EM_D30V,        "D30V"},
   {EM_V850,        "NEC v850"},
   {EM_M32R,        "M32R"},
   {EM_MN10300,     "MN10300"},
   {EM_MN10200,     "MN10200"},
   {EM_PJ,          "picoJava"},
   {EM_ALPHA,       "Alpha"}
};

// Program header type names
SIntTxt ELFPTypeNames[] = {
   {PT_NULL,        "Unused"},
   {PT_LOAD,        "Loadable program segment"},
   {PT_DYNAMIC,     "Dynamic linking information"},
   {PT_INTERP,      "Program interpreter"},
   {PT_NOTE,        "Auxiliary information"},
   {PT_SHLIB,       "Reserved"},
   {PT_PHDR,        "Entry for header table itself"}
};


// Class CELF members:
// Constructor
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
CELF<ELFSTRUCTURES>::CELF() {
   memset(this, 0, sizeof(*this));
}

// ParseFile
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF<ELFSTRUCTURES>::ParseFile(){
   // Load and parse file buffer
   uint32 i;
   FileHeader = *(TELF_Header*)Buf();   // Copy file header
   NSections = FileHeader.e_shnum;
   SectionHeaders.SetNum(NSections);    // Allocate space for section headers
   SectionHeaders.SetZero();
   uint32 Symtabi = 0;                  // Index to symbol table

   // check header integrity
   if (FileHeader.e_phoff > GetDataSize() || FileHeader.e_phoff + FileHeader.e_phentsize > GetDataSize()) err.submit(2035);
   if (FileHeader.e_shoff > GetDataSize() || FileHeader.e_shoff + FileHeader.e_shentsize > GetDataSize()) err.submit(2035);

   // Find section headers
   SectionHeaderSize = FileHeader.e_shentsize;
   if (SectionHeaderSize <= 0) err.submit(2033);
   uint32 SectionOffset = uint32(FileHeader.e_shoff);

   for (i = 0; i < NSections; i++) {
      SectionHeaders[i] = Get<TELF_SectionHeader>(SectionOffset);
      // check section header integrity
      if (SectionHeaders[i].sh_type != SHT_NOBITS && (SectionHeaders[i].sh_offset > GetDataSize() 
          || SectionHeaders[i].sh_offset + SectionHeaders[i].sh_size > GetDataSize() 
          || SectionHeaders[i].sh_offset + SectionHeaders[i].sh_entsize > GetDataSize())) {
              err.submit(2035);
      }
      SectionOffset += SectionHeaderSize;
      if (SectionHeaders[i].sh_type == SHT_SYMTAB) {
         // Symbol table found
         Symtabi = i;
      }
   }

   // if (Buf() && GetNumEntries()) {
   if (Buf() && GetDataSize()) {
       SecStringTable = Buf() + uint32(SectionHeaders[FileHeader.e_shstrndx].sh_offset);
       SecStringTableLen = uint32(SectionHeaders[FileHeader.e_shstrndx].sh_size);
   }
   if (SectionOffset > GetDataSize()) {
      err.submit(2110);     // Section table points to outside file
   }
   if (Symtabi) {
      // Save offset to symbol table
      SymbolTableOffset = (uint32)(SectionHeaders[Symtabi].sh_offset);
      SymbolTableEntrySize = (uint32)(SectionHeaders[Symtabi].sh_entsize); // Entry size of symbol table
      if (SymbolTableEntrySize == 0) {err.submit(2034); return;} // Avoid division by zero
      SymbolTableEntries = uint32(SectionHeaders[Symtabi].sh_size) / SymbolTableEntrySize;
      // Find associated string table
      uint32 Stringtabi = SectionHeaders[Symtabi].sh_link;
      if (Stringtabi < NSections) {
         SymbolStringTableOffset = (uint32)(SectionHeaders[Stringtabi].sh_offset);
         SymbolStringTableSize = (uint32)(SectionHeaders[Stringtabi].sh_size);
      }
      else {
         Symtabi = 0;  // Error
      }   
   }
}


// Dump
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF<ELFSTRUCTURES>::Dump(int options) {
   uint32 i;
   if (options & DUMP_FILEHDR) {
      // File header
      printf("\nDump of ELF file %s", FileName);
      printf("\n-----------------------------------------------");
      printf("\nFile size: %i", GetDataSize());
      printf("\nFile header:");
      printf("\nFile class: %s, Data encoding: %s, ELF version %i, ABI: %s, ABI version %i",
         Lookup(ELFFileClassNames, FileHeader.e_ident[EI_CLASS]),
         Lookup(ELFDataEncodeNames, FileHeader.e_ident[EI_DATA]),
         FileHeader.e_ident[EI_VERSION],
         Lookup(ELFABINames, FileHeader.e_ident[EI_OSABI]),
         FileHeader.e_ident[EI_ABIVERSION]);

      printf("\nFile type: %s, Machine: %s, version: %i", 
         Lookup(ELFFileTypeNames, FileHeader.e_type),
         Lookup(ELFMachineNames, FileHeader.e_machine),
         FileHeader.e_version);
      printf("\nNumber of sections: %2i, Processor flags: 0x%X", 
         NSections, FileHeader.e_flags);
   }

   if ((options & DUMP_SECTHDR) && FileHeader.e_phnum) {      
       // Dump program headers
       uint32 nProgramHeaders = FileHeader.e_phnum;
       uint32 programHeaderSize = FileHeader.e_phentsize;   
       if (programHeaderSize <= 0) err.submit(2033);
       uint32 programHeaderOffset = (uint32)FileHeader.e_phoff;
       Elf64_Phdr pHeader;
       for (i = 0; i < nProgramHeaders; i++) {
           if (WordSize == 32) {
               Elf32_Phdr pHeader32 = Get<Elf32_Phdr>(programHeaderOffset);
               pHeader.p_type = pHeader32.p_type;
               pHeader.p_offset = pHeader32.p_offset;
               pHeader.p_vaddr = pHeader32.p_vaddr;
               pHeader.p_paddr = pHeader32.p_paddr;
               pHeader.p_filesz = pHeader32.p_filesz;
               pHeader.p_memsz = pHeader32.p_memsz;
               pHeader.p_flags = pHeader32.p_flags;
               pHeader.p_align = pHeader32.p_align;
           }
           else {
               pHeader = Get<Elf64_Phdr>(programHeaderOffset);
           }
           printf("\nProgram header Type: %s, flags 0x%X", 
               Lookup(ELFPTypeNames, (uint32)pHeader.p_type), (uint32)pHeader.p_flags);
           printf("\noffset = 0x%X, vaddr = 0x%X, paddr = 0x%X, filesize = 0x%X, memsize = 0x%X, align = 0x%X", 
               (uint32)pHeader.p_offset, (uint32)pHeader.p_vaddr, (uint32)pHeader.p_paddr, (uint32)pHeader.p_filesz, (uint32)pHeader.p_memsz, (uint32)pHeader.p_align);
           programHeaderOffset += programHeaderSize;
           if (pHeader.p_filesz < 0x100 && (int32)pHeader.p_offset < GetDataSize() && memchr(Buf()+pHeader.p_offset, 0, (uint32)pHeader.p_filesz)) {
               printf("\nContents: %s", Buf()+(int32)pHeader.p_offset);
           }
       }
   }

   if (options & DUMP_SECTHDR) {
      // Dump section headers
      printf("\n\nSection headers:");
      for (uint32 sc = 0; sc < NSections; sc++) {
         // Get copy of 32-bit header or converted 64-bit header
         TELF_SectionHeader sheader = SectionHeaders[sc];
         uint32 entrysize = (uint32)(sheader.sh_entsize);
         uint32 namei = sheader.sh_name;
         if (namei >= SecStringTableLen) {err.submit(2112); break;}
         printf("\n%2i Name: %-18s Type: %s", sc, SecStringTable + namei,
            Lookup(ELFSectionTypeNames, sheader.sh_type));
         if (sheader.sh_flags) {
            printf("\n  Flags: 0x%X:", uint32(sheader.sh_flags));
            for (int fi = 1; fi < (1 << 30); fi <<= 1) {
               if (uint32(sheader.sh_flags) & fi) {
                  printf(" %s", Lookup(ELFSectionFlagNames,fi));
               }
            }
         }
         if (sheader.sh_addr) {
            printf("\n  Address: 0x%X", uint32(sheader.sh_addr));
         }
         if (sheader.sh_offset || sheader.sh_size) {
            printf("\n  FileOffset: 0x%X, Size: 0x%X", 
               uint32(sheader.sh_offset), uint32(sheader.sh_size));
         }
         if (sheader.sh_addralign) {
            printf("\n  Alignment: 0x%X", uint32(sheader.sh_addralign));
         }
         if (sheader.sh_entsize) {
            printf("\n  Entry size: 0x%X", uint32(sheader.sh_entsize));
            switch (sheader.sh_type) {
            case SHT_DYNAMIC:
               printf("\n  String table: %i", sheader.sh_link);
               break;
            case SHT_HASH:
               printf("\n  Symbol table: %i", sheader.sh_link);
               break;
            case SHT_REL: case SHT_RELA:
               printf("\n  Symbol table: %i, Reloc. section: %i", 
                  sheader.sh_link, sheader.sh_info);
               break;
            case SHT_SYMTAB: case SHT_DYNSYM:
               printf("\n  Symbol string table: %i, First global symbol: %i", 
                  sheader.sh_link, sheader.sh_info);
               break;
            default:
               if (sheader.sh_link) {
                  printf("\n  Link: %i", sheader.sh_link);
               }
               if (sheader.sh_info) {
                  printf("\n  Info: %i", sheader.sh_info);
               }
            }
         }
         if (sheader.sh_type == SHT_STRTAB && (options & DUMP_STRINGTB)) {
            // Print string table
            printf("\n  String table:");
            char * p = Buf() + uint32(sheader.sh_offset) + 1;
            uint32 nread = 1, len;
            while (nread < uint32(sheader.sh_size)) {
               len = (uint32)strlen(p);
               printf(" >>%s<<", p);
               nread += len + 1;
               p += len + 1;
            }
         }
         if ((sheader.sh_type==SHT_SYMTAB || sheader.sh_type==SHT_DYNSYM) && (options & DUMP_SYMTAB)) {
            // Dump symbol table

            // Find associated string table
            if (sheader.sh_link >= (uint32)NSections) {err.submit(2035); sheader.sh_link = 0;}
            int8 * strtab = Buf() + uint32(SectionHeaders[sheader.sh_link].sh_offset);

            // Find symbol table
            uint32 symtabsize = (uint32)(sheader.sh_size);
            int8 * symtab = Buf() + uint32(sheader.sh_offset);
            int8 * symtabend = symtab + symtabsize;
            if (entrysize < sizeof(TELF_Symbol)) {err.submit(2033); entrysize = sizeof(TELF_Symbol);}

            printf("\n  Symbols:");
            // Loop through symbol table
            int symi;  // Symbol number
            for (symi = 0; symtab < symtabend; symtab += entrysize, symi++) {
               // Copy 32 bit symbol table entry or convert 64 bit entry
               TELF_Symbol sym = *(TELF_Symbol*)symtab;
               int type = sym.st_type;
               int binding = sym.st_bind;
               if (*(strtab + sym.st_name)) {
                  printf("\n  %2i Name: %s,", symi, strtab + sym.st_name);}
               else {
                  printf("\n  %2i Unnamed,", symi);}
               if (sym.st_value || type == STT_OBJECT || type == STT_FUNC || type == STT_GNU_IFUNC || int16(sym.st_shndx) < 0) 
                  printf(" Value: 0x%X", uint32(sym.st_value));
               if (sym.st_size)  printf(" Size: %i", uint32(sym.st_size));
               if (sym.st_other) printf(" Other: 0x%X", sym.st_other);
               if (int16(sym.st_shndx) >= 0) printf(" Section: %i", sym.st_shndx);
               else { // Special segment values
                  switch (int16(sym.st_shndx)) {
                  case SHN_ABS:
                     printf(" Absolute,"); break;
                  case SHN_COMMON:
                     printf(" Common,"); break;
                  case SHN_XINDEX:
                     printf(" Index in extra table,"); break;
                  default:
                     printf(" Section: 0x%X", sym.st_shndx);
                  }
               }
               if (sym.st_type || sym.st_bind) {
                  printf(" Type: %s, Binding: %s", 
                     Lookup(ELFSymbolTypeNames, type),
                     Lookup(ELFSymbolBindingNames, binding));
               }
            }
         }
         if ((sheader.sh_type==SHT_REL || sheader.sh_type==SHT_RELA ) && (options & DUMP_RELTAB)) {
            printf("\n  Relocations:");
            int8 * reltab = Buf() + uint32(sheader.sh_offset);
            int8 * reltabend = reltab + uint32(sheader.sh_size);
            uint32 expectedentrysize = sheader.sh_type == SHT_RELA ? 
               sizeof(TELF_Relocation) :              // Elf32_Rela, Elf64_Rela
               sizeof(TELF_Relocation) - WordSize/8;  // Elf32_Rel,  Elf64_Rel
            if (entrysize < expectedentrysize) {err.submit(2033); entrysize = expectedentrysize;}

            // Loop through entries
            for (; reltab < reltabend; reltab += entrysize) {
               // Copy relocation table entry with or without addend
               TELF_Relocation rel;  rel.r_addend = 0;
               memcpy(&rel, reltab, entrysize);
               printf ("\n  Offset: 0x%X, Symbol: %i, Name: %s\n   Type: %s", 
                  uint32(rel.r_offset), rel.r_sym, SymbolName(rel.r_sym),
                  (WordSize == 32) ?
                  Lookup (ELF32RelocationNames, rel.r_type) :
                  Lookup (ELF64RelocationNames, rel.r_type));
               if (rel.r_addend) printf (", Addend: 0x%X", uint32(rel.r_addend));

               // Find inline addend
               TELF_SectionHeader relsheader = SectionHeaders[sheader.sh_info];
               uint32 relsoffset = uint32(relsheader.sh_offset);
               if (relsoffset+rel.r_offset < GetDataSize()) {
                  int32 * piaddend = (int32*)(Buf()+relsoffset+rel.r_offset);
                  if (* piaddend) printf (", Inline addend: 0x%X", * piaddend);
               }
            }
         }
      }
   }
}


// PublicNames
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
void CELF<ELFSTRUCTURES>::PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m) {
   // Make list of public names
   // Interpret header:
   ParseFile();

   // Loop through section headers
   for (uint32 sc = 0; sc < NSections; sc++) {
      // Get copy of 32-bit header or converted 64-bit header
      TELF_SectionHeader sheader = SectionHeaders[sc];
      uint32 entrysize = uint32(sheader.sh_entsize);

      if (sheader.sh_type==SHT_SYMTAB || sheader.sh_type==SHT_DYNSYM) {
         // Dump symbol table

         // Find associated string table
         if (sheader.sh_link >= (uint32)NSections) {err.submit(2035); sheader.sh_link = 0;}
         int8 * strtab = Buf() + uint32(SectionHeaders[sheader.sh_link].sh_offset);

         // Find symbol table
         uint32 symtabsize = uint32(sheader.sh_size);
         int8 * symtab = Buf() + uint32(sheader.sh_offset);
         int8 * symtabend = symtab + symtabsize;
         if (entrysize < sizeof(TELF_Symbol)) {err.submit(2033); entrysize = sizeof(TELF_Symbol);}

         // Loop through symbol table
         for (int symi = 0; symtab < symtabend; symtab += entrysize, symi++) {
            // Copy 32 bit symbol table entry or convert 64 bit entry
            TELF_Symbol sym = *(TELF_Symbol*)symtab;
            int type = sym.st_type;
            int binding = sym.st_bind;
            if (int16(sym.st_shndx) > 0 
            && type != STT_SECTION && type != STT_FILE
            && (binding == STB_GLOBAL || binding == STB_WEAK)) {
               // Public symbol found
               SStringEntry se;
               se.Member = m;
               // Store name
               se.String = Strings->PushString(strtab + sym.st_name);
               // Store name index
               Index->Push(se);
            }
         }
      }
   }
}

// SymbolName
template <class TELF_Header, class TELF_SectionHeader, class TELF_Symbol, class TELF_Relocation>
const char * CELF<ELFSTRUCTURES>::SymbolName(uint32 index) {
   // Get name of symbol. (ParseFile() must be called first)
   const char * symname = "?";  // Symbol name
   uint32 symi;           // Symbol index
   uint32 stri;           // String index
   if (SymbolTableOffset) {
      symi = SymbolTableOffset + index * SymbolTableEntrySize;
      if (symi < GetDataSize()) {
         stri = Get<TELF_Symbol>(symi).st_name;
         if (stri < SymbolStringTableSize) {
            symname = Buf() + SymbolStringTableOffset + stri;
         }
      }
   }
   return symname;
}


// Make template instances for 32 and 64 bits
template class CELF<ELF32STRUCTURES>;
template class CELF<ELF64STRUCTURES>;
