/****************************    elf.h    ***********************************
* Author:        Agner Fog
* Date created:  2006-07-18
* Last modified: 2009-07-15
* Project:       objconv
* Module:        elf.h
* Description:
* Header file for definition of structures in 32 and 64 bit ELF object file 
* format.
*
* Copyright 2006-2009 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#ifndef ELF_H
#define ELF_H

/********************** FILE HEADER **********************/

struct Elf32_Ehdr {
  uint8   e_ident[16];   // Magic number and other info
  uint16  e_type;        // Object file type
  uint16  e_machine;     // Architecture
  uint32  e_version;     // Object file version
  uint32  e_entry;       // Entry point virtual address
  uint32  e_phoff;       // Program header table file offset
  uint32  e_shoff;       // Section header table file offset
  uint32  e_flags;       // Processor-specific flags
  uint16  e_ehsize;      // ELF header size in bytes
  uint16  e_phentsize;   // Program header table entry size
  uint16  e_phnum;       // Program header table entry count
  uint16  e_shentsize;   // Section header table entry size
  uint16  e_shnum;       // Section header table entry count
  uint16  e_shstrndx;    // Section header string table index
};

struct Elf64_Ehdr {
  uint8   e_ident[16];   // Magic number and other info
  uint16  e_type;        // Object file type
  uint16  e_machine;     // Architecture
  uint32  e_version;     // Object file version
  uint64  e_entry;       // Entry point virtual address
  uint64  e_phoff;       // Program header table file offset
  uint64  e_shoff;       // Section header table file offset
  uint32  e_flags;       // Processor-specific flags
  uint16  e_ehsize;      // ELF header size in bytes
  uint16  e_phentsize;   // Program header table entry size
  uint16  e_phnum;       // Program header table entry count
  uint16  e_shentsize;   // Section header table entry size
  uint16  e_shnum;       // Section header table entry count
  uint16  e_shstrndx;    // Section header string table index
};


/* Fields in the e_ident array.  The EI_* macros are indices into the
   array.  The macros under each EI_* macro are the values the byte
   may have.  */

/* Conglomeration of the identification bytes, for easy testing as a word.  */
#define ELFMAG        "\177ELF"

#define EI_CLASS      4    /* File class byte index */
#define ELFCLASSNONE  0    /* Invalid class */
#define ELFCLASS32    1    /* 32-bit objects */
#define ELFCLASS64    2    /* 64-bit objects */
#define ELFCLASSNUM   3

#define EI_DATA       5    /* Data encoding byte index */
#define ELFDATANONE   0    /* Invalid data encoding */
#define ELFDATA2LSB   1    /* 2's complement, little endian */
#define ELFDATA2MSB   2    /* 2's complement, big endian */
#define ELFDATANUM    3

#define EI_VERSION    6    /* File version byte index */
          /* Value must be EV_CURRENT */

#define EI_OSABI  7         /* OS ABI identification */
#define ELFOSABI_SYSV    0  /* UNIX System V ABI */
#define ELFOSABI_HPUX    1  /* HP-UX */
#define ELFOSABI_ARM    97  /* ARM */
#define ELFOSABI_STANDALONE  255  /* Standalone (embedded) application */

#define EI_ABIVERSION    8    /* ABI version */

#define EI_PAD           9    /* Byte index of padding bytes */

/* Legal values for e_type (object file type).  */

#define ET_NONE          0    /* No file type */
#define ET_REL           1    /* Relocatable file */
#define ET_EXEC          2    /* Executable file */
#define ET_DYN           3    /* Shared object file */
#define ET_CORE          4    /* Core file */
#define ET_NUM           5    /* Number of defined types */
#define ET_LOOS     0xfe00    /* OS-specific range start */
#define ET_HIOS     0xfeff    /* OS-specific range end */
#define ET_LOPROC   0xff00    /* Processor-specific range start */
#define ET_HIPROC   0xffff    /* Processor-specific range end */

/* Legal values for e_machine (architecture).  */

#define EM_NONE          0    /* No machine */
#define EM_M32           1    /* AT&T WE 32100 */
#define EM_SPARC         2    /* SUN SPARC */
#define EM_386           3    /* Intel 80386 */
#define EM_68K           4    /* Motorola m68k family */
#define EM_88K           5    /* Motorola m88k family */
#define EM_860           7    /* Intel 80860 */
#define EM_MIPS          8    /* MIPS R3000 big-endian */
#define EM_S370          9    /* IBM System/370 */
#define EM_MIPS_RS3_LE  10    /* MIPS R3000 little-endian */
#define EM_PARISC       15    /* HPPA */
#define EM_VPP500       17    /* Fujitsu VPP500 */
#define EM_SPARC32PLUS  18    /* Sun's "v8plus" */
#define EM_960          19    /* Intel 80960 */
#define EM_PPC          20    /* PowerPC */
#define EM_PPC64        21    /* PowerPC 64-bit */
#define EM_S390         22    /* IBM S390 */
#define EM_V800         36    /* NEC V800 series */
#define EM_FR20         37    /* Fujitsu FR20 */
#define EM_RH32         38    /* TRW RH-32 */
#define EM_RCE          39    /* Motorola RCE */
#define EM_ARM          40    /* ARM */
#define EM_FAKE_ALPHA   41    /* Digital Alpha */
#define EM_SH           42    /* Hitachi SH */
#define EM_SPARCV9      43    /* SPARC v9 64-bit */
#define EM_TRICORE      44    /* Siemens Tricore */
#define EM_ARC          45    /* Argonaut RISC Core */
#define EM_H8_300       46    /* Hitachi H8/300 */
#define EM_H8_300H      47    /* Hitachi H8/300H */
#define EM_H8S          48    /* Hitachi H8S */
#define EM_H8_500       49    /* Hitachi H8/500 */
#define EM_IA_64        50    /* Intel Merced */
#define EM_MIPS_X       51    /* Stanford MIPS-X */
#define EM_COLDFIRE     52    /* Motorola Coldfire */
#define EM_68HC12       53    /* Motorola M68HC12 */
#define EM_MMA          54    /* Fujitsu MMA Multimedia Accelerator*/
#define EM_PCP          55    /* Siemens PCP */
#define EM_NCPU         56    /* Sony nCPU embeeded RISC */
#define EM_NDR1         57    /* Denso NDR1 microprocessor */
#define EM_STARCORE     58    /* Motorola Start*Core processor */
#define EM_ME16         59    /* Toyota ME16 processor */
#define EM_ST100        60    /* STMicroelectronic ST100 processor */
#define EM_TINYJ        61    /* Advanced Logic Corp. Tinyj emb.fam*/
#define EM_X86_64       62    /* AMD x86-64 architecture */
#define EM_PDSP         63    /* Sony DSP Processor */
#define EM_FX66         66    /* Siemens FX66 microcontroller */
#define EM_ST9PLUS      67    /* STMicroelectronics ST9+ 8/16 mc */
#define EM_ST7          68    /* STmicroelectronics ST7 8 bit mc */
#define EM_68HC16       69    /* Motorola MC68HC16 microcontroller */
#define EM_68HC11       70    /* Motorola MC68HC11 microcontroller */
#define EM_68HC08       71    /* Motorola MC68HC08 microcontroller */
#define EM_68HC05       72    /* Motorola MC68HC05 microcontroller */
#define EM_SVX          73    /* Silicon Graphics SVx */
#define EM_AT19         74    /* STMicroelectronics ST19 8 bit mc */
#define EM_VAX          75    /* Digital VAX */
#define EM_CRIS         76    /* Axis Communications 32-bit embedded processor */
#define EM_JAVELIN      77    /* Infineon Technologies 32-bit embedded processor */
#define EM_FIREPATH     78    /* Element 14 64-bit DSP Processor */
#define EM_ZSP          79    /* LSI Logic 16-bit DSP Processor */
#define EM_MMIX         80    /* Donald Knuth's educational 64-bit processor */
#define EM_HUANY        81    /* Harvard University machine-independent object files */
#define EM_PRISM        82    /* SiTera Prism */
#define EM_AVR          83    /* Atmel AVR 8-bit microcontroller */
#define EM_FR30         84    /* Fujitsu FR30 */
#define EM_D10V         85    /* Mitsubishi D10V */
#define EM_D30V         86    /* Mitsubishi D30V */
#define EM_V850         87    /* NEC v850 */
#define EM_M32R         88    /* Mitsubishi M32R */
#define EM_MN10300      89    /* Matsushita MN10300 */
#define EM_MN10200      90    /* Matsushita MN10200 */
#define EM_PJ           91    /* picoJava */
#define EM_NUM          92
#define EM_ALPHA    0x9026

/* Legal values for e_version (version).  */

#define EV_NONE          0    /* Invalid ELF version */
#define EV_CURRENT       1    /* Current version */
#define EV_NUM           2

/* Section header.  */

struct Elf32_Shdr {
  uint32  sh_name;      // Section name (string tbl index)
  uint32  sh_type;      // Section type
  uint32  sh_flags;     // Section flags
  uint32  sh_addr;      // Section virtual addr at execution
  uint32  sh_offset;    // Section file offset
  uint32  sh_size;      // Section size in bytes
  uint32  sh_link;      // Link to another section
  uint32  sh_info;      // Additional section information
  uint32  sh_addralign; // Section alignment
  uint32  sh_entsize;   // Entry size if section holds table
};

struct Elf64_Shdr {
  uint32  sh_name;      // Section name (string tbl index)
  uint32  sh_type;      // Section type
  uint64  sh_flags;     // Section flags
  uint64  sh_addr;      // Section virtual addr at execution
  uint64  sh_offset;    // Section file offset
  uint64  sh_size;      // Section size in bytes
  uint32  sh_link;      // Link to another section
  uint32  sh_info;      // Additional section information
  uint64  sh_addralign; // Section alignment
  uint64  sh_entsize;   // Entry size if section holds table
};


/* Special section indices.  */

#define SHN_UNDEF                   0  // Undefined section
#define SHN_LORESERVE  ((int16)0xff00) // Start of reserved indices
#define SHN_LOPROC     ((int16)0xff00) // Start of processor-specific
#define SHN_HIPROC     ((int16)0xff1f) // End of processor-specific
#define SHN_LOOS       ((int16)0xff20) // Start of OS-specific
#define SHN_HIOS       ((int16)0xff3f) // End of OS-specific
#define SHN_ABS        ((int16)0xfff1) // Associated symbol is absolute
#define SHN_COMMON     ((int16)0xfff2) // Associated symbol is common
#define SHN_XINDEX     ((int16)0xffff) // Index is in extra table
#define SHN_HIRESERVE  ((int16)0xffff) // End of reserved indices

// Legal values for sh_type (section type).

#define SHT_NULL                    0  // Section header table entry unused
#define SHT_PROGBITS                1  // Program data
#define SHT_SYMTAB                  2  // Symbol table
#define SHT_STRTAB                  3  // String table
#define SHT_RELA                    4  // Relocation entries with addends. Warning: Works only in 64 bit mode in my tests!
#define SHT_HASH                    5  // Symbol hash table
#define SHT_DYNAMIC                 6  // Dynamic linking information
#define SHT_NOTE                    7  // Notes
#define SHT_NOBITS                  8  // Program space with no data (bss)
#define SHT_REL                     9  // Relocation entries, no addends
#define SHT_SHLIB                  10  // Reserved
#define SHT_DYNSYM                 11  // Dynamic linker symbol table
#define SHT_INIT_ARRAY             14  // Array of constructors
#define SHT_FINI_ARRAY             15  // Array of destructors
#define SHT_PREINIT_ARRAY          16  // Array of pre-constructors
#define SHT_GROUP                  17  // Section group
#define SHT_SYMTAB_SHNDX           18  // Extended section indeces
#define SHT_NUM                    19  // Number of defined types. 
#define SHT_LOOS           0x60000000  // Start OS-specific
#define SHT_CHECKSUM       0x6ffffff8  // Checksum for DSO content. 
#define SHT_LOSUNW         0x6ffffffa  // Sun-specific low bound. 
#define SHT_SUNW_move      0x6ffffffa
#define SHT_SUNW_COMDAT    0x6ffffffb
#define SHT_SUNW_syminfo   0x6ffffffc
#define SHT_GNU_verdef     0x6ffffffd  // Version definition section. 
#define SHT_GNU_verneed    0x6ffffffe  // Version needs section. 
#define SHT_GNU_versym     0x6fffffff  // Version symbol table. 
#define SHT_HISUNW         0x6fffffff  // Sun-specific high bound. 
#define SHT_HIOS           0x6fffffff  // End OS-specific type
#define SHT_LOPROC         0x70000000  // Start of processor-specific
#define SHT_HIPROC         0x7fffffff  // End of processor-specific
#define SHT_LOUSER         0x80000000  // Start of application-specific
#define SHT_HIUSER         0x8fffffff  // End of application-specific
#define SHT_REMOVE_ME      0xffffff99  // Specific to objconv program: Removed debug or exception handler section

// Legal values for sh_flags (section flags). 

#define SHF_WRITE            (1 << 0)  // Writable
#define SHF_ALLOC            (1 << 1)  // Occupies memory during execution
#define SHF_EXECINSTR        (1 << 2)  // Executable
#define SHF_MERGE            (1 << 4)  // Might be merged
#define SHF_STRINGS          (1 << 5)  // Contains nul-terminated strings
#define SHF_INFO_LINK        (1 << 6)  // `sh_info' contains SHT index
#define SHF_LINK_ORDER       (1 << 7)  // Preserve order after combining
#define SHF_OS_NONCONFORMING (1 << 8)  // Non-standard OS specific handling required
#define SHF_MASKOS         0x0ff00000  // OS-specific. 
#define SHF_MASKPROC       0xf0000000  // Processor-specific

/* Section group handling.  */
#define GRP_COMDAT  0x1    /* Mark group as COMDAT.  */


/* Symbol table entry.  */

struct Elf32_Sym {
  uint32  st_name;       // Symbol name (string tbl index)
  uint32  st_value;      // Symbol value
  uint32  st_size;       // Symbol size
  uint8   st_type: 4,    // Symbol type
          st_bind: 4;    // Symbol binding
  uint8   st_other;      // Symbol visibility
  uint16  st_shndx;      // Section index
};

struct Elf64_Sym {
  uint32  st_name;       // Symbol name (string tbl index)
  uint8   st_type: 4,    // Symbol type
          st_bind: 4;    // Symbol binding
  uint8   st_other;      // Symbol visibility
  uint16  st_shndx;      // Section index
  uint64  st_value;      // Symbol value
  uint64  st_size;       // Symbol size
};


/* The syminfo section if available contains additional information about
   every dynamic symbol.  */

struct Elf32_Syminfo {
  uint16 si_boundto;    /* Direct bindings, symbol bound to */
  uint16 si_flags;      /* Per symbol flags */
};

struct Elf64_Syminfo {
  uint16 si_boundto;    /* Direct bindings, symbol bound to */
  uint16 si_flags;      /* Per symbol flags */
};

/* Possible values for si_boundto.  */
#define SYMINFO_BT_SELF        0xffff  /* Symbol bound to self */
#define SYMINFO_BT_PARENT      0xfffe  /* Symbol bound to parent */
#define SYMINFO_BT_LOWRESERVE  0xff00  /* Beginning of reserved entries */

/* Possible bitmasks for si_flags.  */
#define SYMINFO_FLG_DIRECT     0x0001  /* Direct bound symbol */
#define SYMINFO_FLG_PASSTHRU   0x0002  /* Pass-thru symbol for translator */
#define SYMINFO_FLG_COPY       0x0004  /* Symbol is a copy-reloc */
#define SYMINFO_FLG_LAZYLOAD   0x0008  /* Symbol bound to object to be lazy loaded */
/* Syminfo version values.  */
#define SYMINFO_NONE                0
#define SYMINFO_CURRENT             1
#define SYMINFO_NUM                 2


/* Special section index.  */

#define SHN_UNDEF  0    /* No section, undefined symbol.  */

// How to extract and insert information held in the st_info field.
// Both Elf32_Sym and Elf64_Sym use the same one-byte st_info field.

//#define ELF32_ST_BIND(val)        (((uint8) (val)) >> 4)
//#define ELF32_ST_TYPE(val)        ((val) & 0xf)
//#define ELF32_ST_INFO(bind,type)  (((bind) << 4) + ((type) & 0xf))

// Legal values for ST_BIND subfield of st_info (symbol binding).

#define STB_LOCAL    0    // Local symbol
#define STB_GLOBAL   1    // Global symbol
#define STB_WEAK     2    // Weak symbol
#define STB_NUM      3    // Number of defined types. 
#define STB_LOOS    10    // Start of OS-specific
#define STB_HIOS    12    // End of OS-specific
#define STB_LOPROC  13    // Start of processor-specific
#define STB_HIPROC  15    // End of processor-specific

// Legal values for ST_TYPE subfield of st_info (symbol type). 

#define STT_NOTYPE   0    // Symbol type is unspecified
#define STT_OBJECT   1    // Symbol is a data object
#define STT_FUNC     2    // Symbol is a code object
#define STT_SECTION  3    // Symbol associated with a section
#define STT_FILE     4    // Symbol's name is file name
#define STT_COMMON   5    // Symbol is a common data object
#define STT_NUM      6    // Number of defined types. 
#define STT_LOOS    10    // Start of OS-specific
#define STT_GNU_IFUNC 10  // Symbol is an indirect code object (function dispatcher)
#define STT_HIOS    12    // End of OS-specific
#define STT_LOPROC  13    // Start of processor-specific
#define STT_HIPROC  15    // End of processor-specific


/* Symbol table indices are found in the hash buckets and chain table
   of a symbol hash table section.  This special index value indicates
   the end of a chain, meaning no further symbols are found in that bucket.  */

#define STN_UNDEF  0    /* End of a chain.  */


/* How to extract and insert information held in the st_other field.  */

#define ELF32_ST_VISIBILITY(o)  ((o) & 0x03)

/* For ELF64 the definitions are the same.  */
#define ELF64_ST_VISIBILITY(o)  ELF32_ST_VISIBILITY (o)

/* Symbol visibility specification encoded in the st_other field.  */
#define STV_DEFAULT    0    /* Default symbol visibility rules */
#define STV_INTERNAL   1    /* Processor specific hidden class */
#define STV_HIDDEN     2    /* Sym unavailable in other modules */
#define STV_PROTECTED  3    /* Not preemptible, not exported */


// Relocation table entry structures
// How to extract and insert information held in the r_info field.
//#define ELF32_R_SYM(val)        ((val) >> 8)
//#define ELF32_R_TYPE(val)       ((val) & 0xff)
//#define ELF32_R_INFO(sym,type)  (((sym) << 8) + ((type) & 0xff))

//#define ELF64_R_SYM(i)          ((uint32)((i) >> 32))
//#define ELF64_R_TYPE(i)         ((i) & 0xffffffff)
//#define ELF64_R_INFO(sym,type)  ((((uint64) (sym)) << 32) + (type))


// Relocation table entry without addend (in section of type SHT_REL)
struct Elf32_Rel {
  uint32  r_offset;             // Address
  uint32  r_type: 8,            // Relocation type
          r_sym: 24;            // Symbol index
};

struct Elf64_Rel {
  uint64  r_offset;             // Address
  uint32  r_type;               // Relocation type
  uint32  r_sym;                // Symbol index
};

// Relocation table entry with addend (in section of type SHT_RELA)

// Warning: Elf32_Rela doesn't work in any of the systems I have tried. 
// Use Elf32_Rel instead with addend in relocated field.
// Use Elf64_Rela in 64 bit mode. Elf64_Rel not accepted?

struct Elf32_Rela {
  uint32  r_offset;               // Address
  uint32  r_type: 8,              // Relocation type
          r_sym: 24;              // Symbol index
  int32   r_addend;               // Addend
};

struct Elf64_Rela {
  uint64  r_offset;               // Address
  uint32  r_type;                 // Relocation type
  uint32  r_sym;                  // Symbol index
  int64   r_addend;               // Addend
};

// i386 Relocation types

#define R_386_NONE      0    // No reloc
#define R_386_32        1    // Direct 32 bit
#define R_386_PC32      2    // Self-relative 32 bit (not EIP relative in the sense used in COFF files)
#define R_386_GOT32     3    // 32 bit GOT entry
#define R_386_PLT32     4    // 32 bit PLT address
#define R_386_COPY      5    // Copy symbol at runtime
#define R_386_GLOB_DAT  6    // Create GOT entry
#define R_386_JMP_SLOT  7    // Create PLT entry
#define R_386_RELATIVE  8    // Adjust by program base
#define R_386_GOTOFF    9    // 32 bit offset to GOT 
#define R_386_GOTPC    10    // 32 bit self relative offset to GOT
#define R_386_IRELATIVE 42   // Reference to PLT entry of indirect function (STT_GNU_IFUNC)
//#define R_386_NUM      11    // Number of entries

// AMD x86-64 relocation types
#define R_X86_64_NONE       0  // No reloc
#define R_X86_64_64         1  // Direct 64 bit 
#define R_X86_64_PC32       2  // Self relative 32 bit signed (not RIP relative in the sense used in COFF files)
#define R_X86_64_GOT32      3  // 32 bit GOT entry
#define R_X86_64_PLT32      4  // 32 bit PLT address
#define R_X86_64_COPY       5  // Copy symbol at runtime
#define R_X86_64_GLOB_DAT   6  // Create GOT entry
#define R_X86_64_JUMP_SLOT  7  // Create PLT entry
#define R_X86_64_RELATIVE   8  // Adjust by program base
#define R_X86_64_GOTPCREL   9  // 32 bit signed self relative offset to GOT
#define R_X86_64_32        10  // Direct 32 bit zero extended
#define R_X86_64_32S       11  // Direct 32 bit sign extended
#define R_X86_64_16        12  // Direct 16 bit zero extended
#define R_X86_64_PC16      13  // 16 bit sign extended self relative
#define R_X86_64_8         14  // Direct 8 bit sign extended
#define R_X86_64_PC8       15  // 8 bit sign extended self relative
#define R_X86_64_IRELATIVE 37  // Reference to PLT entry of indirect function (STT_GNU_IFUNC)
//#define R_X86_64_NUM       16  // Number of entries
// Pseudo-record when ELF is used as intermediary between COFF and MachO:
#define R_UNSUPPORTED_IMAGEREL 21  // Image-relative not supported



// Program segment header.

struct Elf32_Phdr {
  uint32  p_type;      /* Segment type */
  uint32  p_offset;    /* Segment file offset */
  uint32  p_vaddr;     /* Segment virtual address */
  uint32  p_paddr;     /* Segment physical address */
  uint32  p_filesz;    /* Segment size in file */
  uint32  p_memsz;     /* Segment size in memory */
  uint32  p_flags;     /* Segment flags */
  uint32  p_align;     /* Segment alignment */
};

struct Elf64_Phdr {
  uint32  p_type;      /* Segment type */
  uint32  p_flags;     /* Segment flags */
  uint64  p_offset;    /* Segment file offset */
  uint64  p_vaddr;     /* Segment virtual address */
  uint64  p_paddr;     /* Segment physical address */
  uint64  p_filesz;    /* Segment size in file */
  uint64  p_memsz;     /* Segment size in memory */
  uint64  p_align;     /* Segment alignment */
};

/* Legal values for p_type (segment type).  */

#define PT_NULL             0    /* Program header table entry unused */
#define PT_LOAD             1    /* Loadable program segment */
#define PT_DYNAMIC          2    /* Dynamic linking information */
#define PT_INTERP           3    /* Program interpreter */
#define PT_NOTE             4    /* Auxiliary information */
#define PT_SHLIB            5    /* Reserved */
#define PT_PHDR             6    /* Entry for header table itself */
#define PT_NUM              7    /* Number of defined types */
#define PT_LOOS    0x60000000    /* Start of OS-specific */
#define PT_HIOS    0x6fffffff    /* End of OS-specific */
#define PT_LOPROC  0x70000000    /* Start of processor-specific */
#define PT_HIPROC  0x7fffffff    /* End of processor-specific */

/* Legal values for p_flags (segment flags).  */

#define PF_X           (1 << 0)  /* Segment is executable */
#define PF_W           (1 << 1)  /* Segment is writable */
#define PF_R           (1 << 2)  /* Segment is readable */
#define PF_MASKOS    0x0ff00000  /* OS-specific */
#define PF_MASKPROC  0xf0000000  /* Processor-specific */

/* Legal values for note segment descriptor types for core files. */

#define NT_PRSTATUS    1    /* Contains copy of prstatus struct */
#define NT_FPREGSET    2    /* Contains copy of fpregset struct */
#define NT_PRPSINFO    3    /* Contains copy of prpsinfo struct */
#define NT_PRXREG      4    /* Contains copy of prxregset struct */
#define NT_PLATFORM    5    /* String from sysinfo(SI_PLATFORM) */
#define NT_AUXV        6    /* Contains copy of auxv array */
#define NT_GWINDOWS    7    /* Contains copy of gwindows struct */
#define NT_PSTATUS    10    /* Contains copy of pstatus struct */
#define NT_PSINFO     13    /* Contains copy of psinfo struct */
#define NT_PRCRED     14    /* Contains copy of prcred struct */
#define NT_UTSNAME    15    /* Contains copy of utsname struct */
#define NT_LWPSTATUS  16    /* Contains copy of lwpstatus struct */
#define NT_LWPSINFO   17    /* Contains copy of lwpinfo struct */
#define NT_PRFPXREG   20    /* Contains copy of fprxregset struct*/

/* Legal values for the note segment descriptor types for object files.  */

#define NT_VERSION  1    /* Contains a version string.  */


/* Dynamic section entry.  */

struct Elf32_Dyn {
  int32  d_tag;          /* Dynamic entry type */
  union   {
      uint32 d_val;      /* Integer value */
      uint32 d_ptr;      /* Address value */
    } d_un;
};

struct Elf64_Dyn {
  int64  d_tag;          /* Dynamic entry type */
  union    {
      uint64 d_val;      /* Integer value */
      uint64 d_ptr;      /* Address value */
    } d_un;
};

/* Legal values for d_tag (dynamic entry type).  */

#define DT_NULL             0    /* Marks end of dynamic section */
#define DT_NEEDED           1    /* Name of needed library */
#define DT_PLTRELSZ         2    /* Size in bytes of PLT relocs */
#define DT_PLTGOT           3    /* Processor defined value */
#define DT_HASH             4    /* Address of symbol hash table */
#define DT_STRTAB           5    /* Address of string table */
#define DT_SYMTAB           6    /* Address of symbol table */
#define DT_RELA             7    /* Address of Rela relocs */
#define DT_RELASZ           8    /* Total size of Rela relocs */
#define DT_RELAENT          9    /* Size of one Rela reloc */
#define DT_STRSZ           10    /* Size of string table */
#define DT_SYMENT          11    /* Size of one symbol table entry */
#define DT_INIT            12    /* Address of init function */
#define DT_FINI            13    /* Address of termination function */
#define DT_SONAME          14    /* Name of shared object */
#define DT_RPATH           15    /* Library search path (deprecated) */
#define DT_SYMBOLIC        16    /* Start symbol search here */
#define DT_REL             17    /* Address of Rel relocs */
#define DT_RELSZ           18    /* Total size of Rel relocs */
#define DT_RELENT          19    /* Size of one Rel reloc */
#define DT_PLTREL          20    /* Type of reloc in PLT */
#define DT_DEBUG           21    /* For debugging; unspecified */
#define DT_TEXTREL         22    /* Reloc might modify .text */
#define DT_JMPREL          23    /* Address of PLT relocs */
#define DT_BIND_NOW        24    /* Process relocations of object */
#define DT_INIT_ARRAY      25    /* Array with addresses of init fct */
#define DT_FINI_ARRAY      26    /* Array with addresses of fini fct */
#define DT_INIT_ARRAYSZ    27    /* Size in bytes of DT_INIT_ARRAY */
#define DT_FINI_ARRAYSZ    28    /* Size in bytes of DT_FINI_ARRAY */
#define DT_RUNPATH         29    /* Library search path */
#define DT_FLAGS           30    /* Flags for the object being loaded */
#define DT_ENCODING        32    /* Start of encoded range */
#define DT_PREINIT_ARRAY   32    /* Array with addresses of preinit fct*/
#define DT_PREINIT_ARRAYSZ 33    /* size in bytes of DT_PREINIT_ARRAY */
#define DT_NUM             34    /* Number used */
#define DT_LOOS    0x60000000    /* Start of OS-specific */
#define DT_HIOS    0x6fffffff    /* End of OS-specific */
#define DT_LOPROC  0x70000000    /* Start of processor-specific */
#define DT_HIPROC  0x7fffffff    /* End of processor-specific */
#define DT_PROCNUM DT_MIPS_NUM  /* Most used by any processor */

/* DT_* entries which fall between DT_VALRNGHI & DT_VALRNGLO use the
   Dyn.d_un.d_val field of the Elf*_Dyn structure.  This follows Sun's
   approach.  */
#define DT_VALRNGLO   0x6ffffd00
#define DT_CHECKSUM   0x6ffffdf8
#define DT_PLTPADSZ   0x6ffffdf9
#define DT_MOVEENT    0x6ffffdfa
#define DT_MOVESZ     0x6ffffdfb
#define DT_FEATURE_1  0x6ffffdfc  /* Feature selection (DTF_*).  */
#define DT_POSFLAG_1  0x6ffffdfd  /* Flags for DT_* entries, effecting the following DT_* entry.  */
#define DT_SYMINSZ    0x6ffffdfe  /* Size of syminfo table (in bytes) */
#define DT_SYMINENT   0x6ffffdff  /* Entry size of syminfo */
#define DT_VALRNGHI   0x6ffffdff

/* DT_* entries which fall between DT_ADDRRNGHI & DT_ADDRRNGLO use the
   Dyn.d_un.d_ptr field of the Elf*_Dyn structure.

   If any adjustment is made to the ELF object after it has been
   built these entries will need to be adjusted.  */
#define DT_ADDRRNGLO  0x6ffffe00
#define DT_SYMINFO    0x6ffffeff  /* syminfo table */
#define DT_ADDRRNGHI  0x6ffffeff

/* The versioning entry types.  The next are defined as part of the GNU extension.  */
#define DT_VERSYM     0x6ffffff0

#define DT_RELACOUNT  0x6ffffff9
#define DT_RELCOUNT   0x6ffffffa

/* These were chosen by Sun.  */
#define DT_FLAGS_1    0x6ffffffb  /* State flags, see DF_1_* below.  */
#define DT_VERDEF     0x6ffffffc  /* Address of version definition table */
#define DT_VERDEFNUM  0x6ffffffd  /* Number of version definitions */
#define DT_VERNEED    0x6ffffffe  /* Address of table with needed versions */
#define DT_VERNEEDNUM 0x6fffffff  /* Number of needed versions */
#define DT_VERSIONTAGIDX(tag) (DT_VERNEEDNUM - (tag))  /* Reverse order! */
#define DT_VERSIONTAGNUM 16

/* Sun added these machine-independent extensions in the "processor-specific"
   range.  Be compatible.  */
#define DT_AUXILIARY    0x7ffffffd      /* Shared object to load before self */
#define DT_FILTER       0x7fffffff      /* Shared object to get values from */
#define DT_EXTRATAGIDX(tag)  ((uint32)-((int32) (tag) <<1>>1)-1)
#define DT_EXTRANUM  3

/* Values of `d_un.d_val' in the DT_FLAGS entry.  */
#define DF_ORIGIN    0x00000001  /* Object may use DF_ORIGIN */
#define DF_SYMBOLIC  0x00000002  /* Symbol resolutions starts here */
#define DF_TEXTREL   0x00000004  /* Object contains text relocations */
#define DF_BIND_NOW  0x00000008  /* No lazy binding for this object */

/* State flags selectable in the `d_un.d_val' element of the DT_FLAGS_1
   entry in the dynamic section.  */
#define DF_1_NOW        0x00000001  /* Set RTLD_NOW for this object.  */
#define DF_1_GLOBAL     0x00000002  /* Set RTLD_GLOBAL for this object.  */
#define DF_1_GROUP      0x00000004  /* Set RTLD_GROUP for this object.  */
#define DF_1_NODELETE   0x00000008  /* Set RTLD_NODELETE for this object.*/
#define DF_1_LOADFLTR   0x00000010  /* Trigger filtee loading at runtime.*/
#define DF_1_INITFIRST  0x00000020  /* Set RTLD_INITFIRST for this object*/
#define DF_1_NOOPEN     0x00000040  /* Set RTLD_NOOPEN for this object.  */
#define DF_1_ORIGIN     0x00000080  /* $ORIGIN must be handled.  */
#define DF_1_DIRECT     0x00000100  /* Direct binding enabled.  */
#define DF_1_TRANS      0x00000200
#define DF_1_INTERPOSE  0x00000400  /* Object is used to interpose.  */
#define DF_1_NODEFLIB   0x00000800  /* Ignore default lib search path.  */
#define DF_1_NODUMP     0x00001000
#define DF_1_CONFALT    0x00002000
#define DF_1_ENDFILTEE  0x00004000

/* Flags for the feature selection in DT_FEATURE_1.  */
#define DTF_1_PARINIT   0x00000001
#define DTF_1_CONFEXP   0x00000002

/* Flags in the DT_POSFLAG_1 entry effecting only the next DT_* entry.  */
#define DF_P1_LAZYLOAD  0x00000001  /* Lazyload following object.  */
#define DF_P1_GROUPPERM 0x00000002  /* Symbols from next object are not generally available.  */

/* Version definition sections.  */

struct Elf32_Verdef  {
  uint16  vd_version;  /* Version revision */
  uint16  vd_flags;    /* Version information */
  uint16  vd_ndx;      /* Version Index */
  uint16  vd_cnt;      /* Number of associated aux entries */
  uint32  vd_hash;     /* Version name hash value */
  uint32  vd_aux;      /* Offset in bytes to verdaux array */
  uint32  vd_next;     /* Offset in bytes to next verdef entry */
};

struct Elf64_Verdef {
  uint16  vd_version;  /* Version revision */
  uint16  vd_flags;    /* Version information */
  uint16  vd_ndx;      /* Version Index */
  uint16  vd_cnt;      /* Number of associated aux entries */
  uint32  vd_hash;     /* Version name hash value */
  uint32  vd_aux;      /* Offset in bytes to verdaux array */
  uint32  vd_next;     /* Offset in bytes to next verdef entry */
};


/* Legal values for vd_version (version revision).  */
#define VER_DEF_NONE     0    /* No version */
#define VER_DEF_CURRENT  1    /* Current version */
#define VER_DEF_NUM      2    /* Given version number */

/* Legal values for vd_flags (version information flags).  */
#define VER_FLG_BASE   0x1    /* Version definition of file itself */
#define VER_FLG_WEAK   0x2    /* Weak version identifier */

/* Auxialiary version information.  */

struct Elf32_Verdaux {
  uint32  vda_name;    /* Version or dependency names */
  uint32  vda_next;    /* Offset in bytes to next verdaux entry */
};

struct Elf64_Verdaux {
  uint32  vda_name;    /* Version or dependency names */
  uint32  vda_next;    /* Offset in bytes to next verdaux entry */
};


/* Version dependency section.  */

struct Elf32_Verneed {
  uint16  vn_version;    /* Version of structure */
  uint16  vn_cnt;        /* Number of associated aux entries */
  uint32  vn_file;       /* Offset of filename for this dependency */
  uint32  vn_aux;        /* Offset in bytes to vernaux array */
  uint32  vn_next;       /* Offset in bytes to next verneed entry */
};

struct Elf64_Verneed {
  uint16  vn_version;    /* Version of structure */
  uint16  vn_cnt;        /* Number of associated aux entries */
  uint32  vn_file;       /* Offset of filename for this dependency */
  uint32  vn_aux;        /* Offset in bytes to vernaux array */
  uint32  vn_next;       /* Offset in bytes to next verneed entry */
};


/* Legal values for vn_version (version revision).  */
#define VER_NEED_NONE    0    /* No version */
#define VER_NEED_CURRENT 1    /* Current version */
#define VER_NEED_NUM     2    /* Given version number */

/* Auxiliary needed version information.  */

struct Elf32_Vernaux {
  uint32  vna_hash;     /* Hash value of dependency name */
  uint16  vna_flags;    /* Dependency specific information */
  uint16  vna_other;    /* Unused */
  uint32  vna_name;     /* Dependency name string offset */
  uint32  vna_next;     /* Offset in bytes to next vernaux entry */
};

struct Elf64_Vernaux {
  uint32  vna_hash;     /* Hash value of dependency name */
  uint16  vna_flags;    /* Dependency specific information */
  uint16  vna_other;    /* Unused */
  uint32  vna_name;     /* Dependency name string offset */
  uint32  vna_next;     /* Offset in bytes to next vernaux entry */
};


/* Legal values for vna_flags.  */
#define VER_FLG_WEAK  0x2    /* Weak version identifier */


/* Note section contents.  Each entry in the note section begins with
   a header of a fixed form.  */

struct Elf32_Nhdr {
  uint32 n_namesz;      /* Length of the note's name.  */
  uint32 n_descsz;      /* Length of the note's descriptor.  */
  uint32 n_type;        /* Type of the note.  */
};

struct Elf64_Nhdr {
  uint32 n_namesz;      /* Length of the note's name.  */
  uint32 n_descsz;      /* Length of the note's descriptor.  */
  uint32 n_type;        /* Type of the note.  */
};

/* Known names of notes.  */

/* Solaris entries in the note section have this name.  */
#define ELF_NOTE_SOLARIS  "SUNW Solaris"

/* Note entries for GNU systems have this name.  */
#define ELF_NOTE_GNU      "GNU"


/* Defined types of notes for Solaris.  */

/* Value of descriptor (one word) is desired pagesize for the binary.  */
#define ELF_NOTE_PAGESIZE_HINT  1


/* Defined note types for GNU systems.  */

/* ABI information.  The descriptor consists of words:
   word 0: OS descriptor
   word 1: major version of the ABI
   word 2: minor version of the ABI
   word 3: subminor version of the ABI
*/
#define ELF_NOTE_ABI    1

/* Known OSes.  These value can appear in word 0 of an ELF_NOTE_ABI
   note section entry.  */
#define ELF_NOTE_OS_LINUX     0
#define ELF_NOTE_OS_GNU       1
#define ELF_NOTE_OS_SOLARIS2  2


/* Move records.  */
struct Elf32_Move {
  uint64 m_value;      /* Symbol value.  */
  uint32 m_info;       /* Size and index.  */
  uint32 m_poffset;    /* Symbol offset.  */
  uint16 m_repeat;     /* Repeat count.  */
  uint16 m_stride;     /* Stride info.  */
};

struct Elf64_Move {
  uint64 m_value;     /* Symbol value.  */
  uint64 m_info;      /* Size and index.  */
  uint64 m_poffset;   /* Symbol offset.  */
  uint16 m_repeat;    /* Repeat count.  */
  uint16 m_stride;    /* Stride info.  */
};

/* Macro to construct move records.  */
#define ELF32_M_SYM(info)        ((info) >> 8)
#define ELF32_M_SIZE(info)       ((uint8) (info))
#define ELF32_M_INFO(sym, size)  (((sym) << 8) + (uint8) (size))

#define ELF64_M_SYM(info)        ELF32_M_SYM (info)
#define ELF64_M_SIZE(info)       ELF32_M_SIZE (info)
#define ELF64_M_INFO(sym, size)  ELF32_M_INFO (sym, size)


/********************** Strings **********************/
#define ELF_CONSTRUCTOR_NAME    ".ctors"   // Name of constructors segment


// Macros listing all word-size dependent structures, used as template parameter list
#define ELFSTRUCTURES    TELF_Header, TELF_SectionHeader, TELF_Symbol, TELF_Relocation
#define ELF32STRUCTURES  Elf32_Ehdr, Elf32_Shdr, Elf32_Sym, Elf32_Rela
#define ELF64STRUCTURES  Elf64_Ehdr, Elf64_Shdr, Elf64_Sym, Elf64_Rela

#endif // #ifndef ELF_H
