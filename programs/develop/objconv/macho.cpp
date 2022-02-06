/****************************    macho.cpp    *******************************
* Author:        Agner Fog
* Date created:  2007-01-06
* Last modified: 2008-06-02
* Project:       objconv
* Module:        macho.cpp
* Description:
* Module for reading Mach-O files
*
* Class CMACHO is used for reading, interpreting and dumping Mach-O files.
*
* Copyright 2007-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

// Machine names
SIntTxt MacMachineNames[] = {
   {MAC_CPU_TYPE_I386,      "Intel 32 bit"},
   {MAC_CPU_TYPE_X86_64,    "Intel 64 bit"},
   {MAC_CPU_TYPE_ARM,       "Arm"},
   {MAC_CPU_TYPE_SPARC,     "Sparc"},
   {MAC_CPU_TYPE_POWERPC,   "Power PC 32 bit"},
   {MAC_CPU_TYPE_POWERPC64, "Power PC 64 bit"}
};

// CPU subtype names
SIntTxt MacCPUSubtypeNames[] = {
   {MAC_CPU_SUBTYPE_POWERPC_ALL,  "Power PC All"},
   {MAC_CPU_SUBTYPE_I386_ALL,     "Intel All"}
};

// File type names
SIntTxt MacFileTypeNames[] = {
   {MAC_OBJECT,     "Relocatable object file"},
   {MAC_EXECUTE,    "demand paged executable file"},
   {MAC_FVMLIB,     "fixed VM shared library file"},
   {MAC_CORE,       "core file"},
   {MAC_PRELOAD,    "preloaded executable file"},
   {MAC_DYLIB,      "dynamicly bound shared library file"},
   {MAC_DYLINKER,   "dynamic link editor"},
   {MAC_BUNDLE,     "dynamicly bound bundle file"}
};

// Command type names
SIntTxt MacCommandTypeNames[] = {
   {MAC_LC_SEGMENT,        "Segment"},
   {MAC_LC_SYMTAB,         "Symbol table"},
   {MAC_LC_SYMSEG,         "gdb symbol table info (obsolete)"},
   {MAC_LC_THREAD,         "thread"},
   {MAC_LC_UNIXTHREAD,     "unix thread"},
   {MAC_LC_LOADFVMLIB,     "load a specified fixed VM shared library"},
   {MAC_LC_IDFVMLIB,       "fixed VM shared library identification"},
   {MAC_LC_IDENT,          "object identification info (obsolete)"},
   {MAC_LC_FVMFILE,        "fixed VM file inclusion (internal use)"},
   {MAC_LC_PREPAGE,        "prepage command (internal use)"},
   {MAC_LC_DYSYMTAB,       "dynamic link-edit symbol table info"},
   {MAC_LC_LOAD_DYLIB,     "load a dynamicly linked shared library"},
   {MAC_LC_ID_DYLIB,       "dynamicly linked shared lib identification"},
   {MAC_LC_LOAD_DYLINKER,  "load a dynamic linker"},
   {MAC_LC_ID_DYLINKER,    "dynamic linker identification"},
   {MAC_LC_PREBOUND_DYLIB, "modules prebound for a dynamicly linked shared library"},
   {MAC_LC_ROUTINES,       "image routines"},
   {MAC_LC_SUB_FRAMEWORK,  "sub framework"},
   {MAC_LC_SUB_UMBRELLA,   "sub umbrella"},
   {MAC_LC_SUB_CLIENT,     "sub client"},
   {MAC_LC_SUB_LIBRARY,    "sub library"},
   {MAC_LC_TWOLEVEL_HINTS, "two-level namespace lookup hints"},
   {MAC_LC_PREBIND_CKSUM,  "prebind checksum"},
   {MAC_LC_LOAD_WEAK_DYLIB&0xFF, "load a dynamically linked shared library, all symbols weak"},
   {MAC_LC_SEGMENT_64,     "64-bit segment"},
   {MAC_LC_ROUTINES_64,    "64-bit image routine"},
   {MAC_LC_UUID,           "uuid"}
};

// Relocation type names, 32 bit
SIntTxt Mac32RelocationTypeNames[] = {
   {MAC32_RELOC_VANILLA,        "Generic"},
   {MAC32_RELOC_PAIR,           "Second entry of a pair"},
   {MAC32_RELOC_SECTDIFF,       "Section diff"},
   {MAC32_RELOC_PB_LA_PTR,      "Prebound lazy "},
   {MAC32_RELOC_LOCAL_SECTDIFF, "SectDif local"}
};

// Relocation type names, 64 bit
SIntTxt Mac64RelocationTypeNames[] = {
   {MAC64_RELOC_UNSIGNED,    "absolute address"},
   {MAC64_RELOC_SIGNED,      "signed 32-bit displ."},
   {MAC64_RELOC_BRANCH,      "Rel. jump 32-bit displ."},
   {MAC64_RELOC_GOT_LOAD,    "MOVQ load of a GOT entry"},
   {MAC64_RELOC_GOT,         "other GOT reference"},
   {MAC64_RELOC_SUBTRACTOR,  "Subtractor"},
   {MAC64_RELOC_SIGNED_1,    "signed 32-bit displacement with -1 addend"},
   {MAC64_RELOC_SIGNED_2,    "signed 32-bit displacement with -2 addend"},
   {MAC64_RELOC_SIGNED_4,    "signed 32-bit displacement with -4 addend"}
};

// Symbol type names
SIntTxt MacSymbolTypeNames[] = {
   {MAC_N_UNDF,    "Undefined, no section"},
   {MAC_N_ABS,     "Absolute, no section"},
   {MAC_N_SECT,    "Defined"},
   {MAC_N_PBUD,    "Prebound undefined (defined in a dylib)"},
   {MAC_N_INDR,    "Indirect"}
};

// Symbol reference type names
SIntTxt MacSymbolReferenceTypeNames[] = {
   {MAC_REF_FLAG_UNDEFINED_NON_LAZY,         "External non lazy"},
   {MAC_REF_FLAG_UNDEFINED_LAZY,             "External lazy (function call)"},
   {MAC_REF_FLAG_DEFINED,                    "Defined public"},
   {MAC_REF_FLAG_PRIVATE_DEFINED,            "Defined private"},
   {MAC_REF_FLAG_PRIVATE_UNDEFINED_NON_LAZY, "Private undefined non lazy"},
   {MAC_REF_FLAG_PRIVATE_UNDEFINED_LAZY,     "Private undefined lazy"}
};

// Symbol descriptor flag names
SIntTxt MacSymbolDescriptorFlagNames[] = {
   {MAC_REFERENCED_DYNAMICALLY, "Referenced dynamically"},
// {MAC_N_DESC_DISCARDED,       "Discarded"},
   {MAC_N_NO_DEAD_STRIP,        "Don't dead-strip"},
   {MAC_N_WEAK_REF,             "Weak external"},
   {MAC_N_WEAK_DEF,             "Weak public"}
};



// Class CMACHO members:
// Constructor
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
CMACHO<MACSTRUCTURES>::CMACHO() {
   // Set everything to zero
   memset(this, 0, sizeof(*this));
}

template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMACHO<MACSTRUCTURES>::ParseFile(){
   // Load and parse file buffer
   FileHeader = *(TMAC_header*)Buf();   // Copy file header

   // Loop through file commands
   uint32 cmd, cmdsize;
   uint32 currentoffset = sizeof(TMAC_header);
   for (uint32 i = 1; i <= FileHeader.ncmds; i++) {
      if (currentoffset >= this->GetDataSize()) {
         err.submit(2016); return;
      }
      uint8 * currentp = (uint8*)(Buf() + currentoffset);
      cmd     = ((MAC_load_command*)currentp) -> cmd;
      cmdsize = ((MAC_load_command*)currentp) -> cmdsize;
      // Interpret specific command type
      switch(cmd) {
         case MAC_LC_SEGMENT: {
            if (WordSize != 32) err.submit(2320); // mixed segment size
            MAC_segment_command_32 * sh = (MAC_segment_command_32*)currentp;
            SegmentOffset = sh->fileoff;              // File offset of segment
            SegmentSize = sh->filesize;               // Size of segment
            NumSections = sh->nsects;                 // Number of sections
            SectionHeaderOffset = currentoffset + sizeof(TMAC_segment_command); // File offset of section headers
            if (!ImageBase && strcmp(sh->segname, "__TEXT")==0) ImageBase = sh->vmaddr; // Find image base
            break;}

         case MAC_LC_SEGMENT_64: {
            if (WordSize != 64) err.submit(2320); // mixed segment size
            MAC_segment_command_64 * sh = (MAC_segment_command_64*)currentp;
            SegmentOffset = (uint32)sh->fileoff;      // File offset of segment
            SegmentSize = (uint32)sh->filesize;       // Size of segment
            NumSections = sh->nsects;                 // Number of sections
            SectionHeaderOffset = currentoffset + sizeof(TMAC_segment_command); // File offset of section headers
            if (!ImageBase && strcmp(sh->segname, "__TEXT")==0) ImageBase = sh->vmaddr; // Find image base
            break;}

         case MAC_LC_SYMTAB: {
            MAC_symtab_command * sh = (MAC_symtab_command*)currentp;
            SymTabOffset = sh->symoff;                // File offset of symbol table
            SymTabNumber = sh->nsyms;                 // Number of entries in symbol table
            StringTabOffset = sh->stroff;             // File offset of string table
            StringTabSize = sh->strsize;              // Size of string table
            break;}

         case MAC_LC_DYSYMTAB: {
            MAC_dysymtab_command * sh = (MAC_dysymtab_command*)currentp;
            ilocalsym = sh->ilocalsym;	               // index to local symbols
            nlocalsym = sh->nlocalsym;	               // number of local symbols 
            iextdefsym = sh->iextdefsym;	            // index to externally defined symbols
            nextdefsym = sh->nextdefsym;	            // number of externally defined symbols 
            iundefsym = sh->iundefsym;	               // index to undefined symbols
            nundefsym = sh->nundefsym;	               // number of undefined symbols
            IndirectSymTabOffset = sh->indirectsymoff;// file offset to the indirect symbol table
            IndirectSymTabNumber = sh->nindirectsyms; // number of indirect symbol table entries
            break;}
      }
      currentoffset += cmdsize;
   }
}

// Debug dump
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMACHO<MACSTRUCTURES>::Dump(int options) {
   uint32 icmd;                        // Command index
   int32  isec1;                       // Section index within segment
   int32  isec2;                       // Section index global
   int32  nsect;                        // Number of sections in segment

   if (options & DUMP_FILEHDR) {
      // File header
      printf("\nDump of Mach-O file %s", FileName);
      printf("\n-----------------------------------------------");
      printf("\nFile size: 0x%X", this->GetDataSize());
      printf("\nFile header:");
      printf("\n  CPU type: %s, subtype: %s",
         Lookup(MacMachineNames, FileHeader.cputype), 
         Lookup(MacCPUSubtypeNames, FileHeader.cpusubtype));
      
      printf("\n  File type: %s - %s", 
         GetFileFormatName(FileType), Lookup(MacFileTypeNames, FileHeader.filetype));

      printf("\n  Number of load commands: %i, Size of commands: 0x%X, Flags: %X",
         FileHeader.ncmds, FileHeader.sizeofcmds, FileHeader.flags);
   }

   uint32 cmd;                         // Load command
   uint32 cmdsize;                     // Command size
   // Pointer to current position
   uint8 * currentp = (uint8*)(Buf() + sizeof(TMAC_header));

   // Loop through file commands
   for (icmd = 1; icmd <= FileHeader.ncmds; icmd++) {
      cmd     = ((MAC_load_command*)currentp) -> cmd;
      cmdsize = ((MAC_load_command*)currentp) -> cmdsize;

      if (options & DUMP_SECTHDR) {
         // Dump command header
         printf("\n\nCommand %i: %s, size: 0x%X", icmd,
         Lookup(MacCommandTypeNames, cmd), cmdsize);

         // Interpret specific command type
         switch(cmd) {
            case MAC_LC_SEGMENT: {
               MAC_segment_command_32 * sh = (MAC_segment_command_32*)currentp;
               printf("\n  Name: %s, Memory address 0x%X, Memory size 0x%X"
                  "\n  File offset 0x%X, File size 0x%X, Maxprot 0x%X, Initprot 0x%X"
                  "\n  Number of sections %i, Flags 0x%X",
                  sh->segname, sh->vmaddr, sh->vmsize,
                  sh->fileoff, sh->filesize, sh->maxprot, sh->initprot, 
                  sh->nsects, sh->flags);
               break;}

            case MAC_LC_SEGMENT_64: {
               MAC_segment_command_64 * sh = (MAC_segment_command_64*)currentp;
               printf("\n  Name: %s, \n  Memory address 0x%08X%08X, Memory size 0x%08X%08X"
                  "\n  File offset 0x%08X%08X, File size 0x%08X%08X\n  Maxprot 0x%X, Initprot 0x%X"
                  "\n  Number of sections %i, Flags 0x%X",
                  sh->segname, (uint32)(sh->vmaddr>>32), (uint32)sh->vmaddr, 
                  (uint32)(sh->vmsize>>32), (uint32)sh->vmsize,
                  (uint32)(sh->fileoff>>32), (uint32)sh->fileoff, 
                  (uint32)(sh->filesize>>32), (uint32)sh->filesize, 
                  sh->maxprot, sh->initprot, 
                  sh->nsects, sh->flags);
               break;}

            case MAC_LC_SYMTAB: {
               MAC_symtab_command * sh = (MAC_symtab_command*)currentp;
               printf("\n  Symbol table offset 0x%X, number of symbols %i,"
                  "\n  String table offset 0x%X, String table size 0x%X",
                  sh->symoff, sh->nsyms, sh->stroff, sh->strsize);
               break;}

            case MAC_LC_DYSYMTAB: {
               MAC_dysymtab_command * sh = (MAC_dysymtab_command*)currentp;
               printf("\n  Index to local symbols %i, number of local symbols %i,"
                  "\n  Index to external symbols %i, number of external symbols %i,"
                  "\n  Index to undefined symbols %i, number of undefined symbols %i,"
                  "\n  File offset to TOC 0x%X, number of entries in TOC %i,",
                  sh->ilocalsym, sh->nlocalsym, sh->iextdefsym, sh->nextdefsym, 
                  sh->iundefsym, sh->nundefsym, sh->tocoff, sh->ntoc);
               printf("\n  File offset to module table 0x%X, Number of module table entries %i,"
                  "\n  Offset to referenced symbol table 0x%X, Number of referenced symtab entries %i"
                  "\n  Offset to indirect symbol table 0x%X, Number of indirect symtab entries %i"
                  "\n  Offset to external relocation entries 0x%X, Number of external reloc. entries %i"
                  "\n  Offset to local relocation entries 0x%X, Number of local reloc. entries %i",
                  sh->modtaboff, sh->nmodtab, sh->extrefsymoff, sh->nextrefsyms, 
                  sh->indirectsymoff, sh->nindirectsyms, sh->extreloff, sh->nextrel,
                  sh->locreloff, sh->nlocrel);	
               break;}
         }

      }
      currentp += cmdsize;
   }

   // Dump section headers
   if (options & DUMP_SECTHDR) {
      printf("\n\nSections:");

      // Reset current pointer
      currentp = (uint8*)(Buf() + sizeof(TMAC_header));
      isec2 = 0;

      // Loop through load commands
      for (icmd = 1; icmd <= FileHeader.ncmds; icmd++) {
         cmd     = ((MAC_load_command*)currentp) -> cmd;
         cmdsize = ((MAC_load_command*)currentp) -> cmdsize;

         if (cmd == MAC_LC_SEGMENT) {
            // This is a 32-bit segment command
            // Number of sections in segment
            nsect   = ((MAC_segment_command_32*)currentp) -> nsects;

            // Find first section header
            MAC_section_32 * sectp = (MAC_section_32*)(currentp + sizeof(MAC_segment_command_32));

            // Loop through section headers
            for (isec1 = 1; isec1 <= nsect; isec1++, sectp++) {
               printf("\n\nSection %i: Name: %s, Segment: %s.", 
                  ++isec2, sectp->sectname, sectp->segname);
               printf("\n  Memory address 0x%X, Size 0x%X, File offset 0x%X"
                  "\n  Alignment %i, Reloc. ent. offset 0x%X, Num reloc. %i"
                  "\n  Flags 0x%X, reserved1 0x%X, reserved2 0x%X",
                  sectp->addr, sectp->size, sectp->offset, 1 << sectp->align,
                  sectp->reloff, sectp->nreloc, sectp->flags, 
                  sectp->reserved1, sectp->reserved2);

               if (sectp->nreloc && (options & DUMP_RELTAB)) {
                  // Dump relocations
                  printf("\n  Relocations:");
                  if (sectp->reloff >= this->GetDataSize()) {err.submit(2035); break;}
                  MAC_relocation_info * relp = (MAC_relocation_info*)(Buf() + sectp->reloff);
                  for (uint32 r = 1; r <= sectp->nreloc; r++, relp++) {
                     if (relp->r_address & R_SCATTERED) {
                        // scattered relocation into
                        MAC_scattered_relocation_info * scatp = (MAC_scattered_relocation_info*)relp;

                        if (!(scatp->r_type & MAC32_RELOC_PAIR)) {
                           printf ("\n    Offset: 0x%X, Value: 0x%X, Length: %i, Scat. Type: %s",
                              scatp->r_address, scatp->r_value, 1 << scatp->r_length, 
                              Lookup(Mac32RelocationTypeNames, scatp->r_type));
                           if (scatp->r_address < sectp->size) {
                              printf(", Inline: 0x%X", *(int32*)(Buf()+sectp->offset+scatp->r_address));
                           }
                        }
                        else {
                           // Second entry of a pair
                           printf ("\n     Offset2: 0x%X, Value2: 0x%X, Length2: %i",
                              scatp->r_address, scatp->r_value, 1 << scatp->r_length);
                        }
                        if (scatp->r_pcrel) printf(", PC relative");
                     }
                     else {
                        // non-scattered
                        if (relp->r_extern) printf ("\n    Symbol number %i, ", relp->r_symbolnum);
                        else printf ("\n    Section: %i, ", relp->r_symbolnum);
                        printf ("Offset: 0x%X, ", relp->r_address);
                        if (relp->r_pcrel) printf ("PC relative, ");
                        printf ("\n     Length: %i, Extern: %i, Type: %s",
                           1 << relp->r_length, relp->r_extern,
                           Lookup(Mac32RelocationTypeNames, relp->r_type));
                        if (relp->r_address < sectp->size) {
                           printf(", Inline: 0x%X", *(int32*)(Buf()+sectp->offset+relp->r_address));
                        }
                     }
                  }
               }
            }
         }
         if (cmd == MAC_LC_SEGMENT_64) {
            // This is a 64-bit segment command
            // Number of sections in segment
            nsect   = ((MAC_segment_command_64*)currentp) -> nsects;

            // Find first section header
            MAC_section_64 * sectp = (MAC_section_64*)(currentp + sizeof(MAC_segment_command_64));

            // Loop through section headers
            for (isec1 = 1; isec1 <= nsect; isec1++, sectp++) {
               printf("\n\nSection %i: Name: %s, Segment: %s.", 
                  ++isec2, sectp->sectname, sectp->segname);
               printf("\n  Memory address 0x%X, Size 0x%X, File offset 0x%X"
                  "\n  Alignment %i, Reloc. ent. offset 0x%X, Num reloc. %i"
                  "\n  Flags 0x%X, reserved1 0x%X, reserved2 0x%X",
                  (uint32)sectp->addr, (uint32)sectp->size, sectp->offset, 1 << sectp->align,
                  sectp->reloff, sectp->nreloc, sectp->flags, 
                  sectp->reserved1, sectp->reserved2);

               if (sectp->nreloc && (options & DUMP_RELTAB)) {
                  // Dump relocations
                  printf("\n  Relocations:");
                  MAC_relocation_info * relp = (MAC_relocation_info*)(Buf() + sectp->reloff);
                  for (uint32 r = 1; r <= sectp->nreloc; r++, relp++) {
                     if (relp->r_address & R_SCATTERED) {
                        // scattered relocation into (not used in 64-bit Mach-O)
                        MAC_scattered_relocation_info * scatp = (MAC_scattered_relocation_info*)relp;
                        if (!(scatp->r_type & MAC32_RELOC_PAIR)) {
                           printf ("\n    Unexpected scattered relocation. Offset: 0x%X, Value: 0x%X, Length: %i, Scat. Type: %s",
                              scatp->r_address, scatp->r_value, 1 << scatp->r_length, 
                              Lookup(Mac64RelocationTypeNames, scatp->r_type));
                           if (scatp->r_address < sectp->size) {
                              printf(", Inline: 0x%X", *(int32*)(Buf()+sectp->offset+scatp->r_address));
                           }
                        }
                        else {
                           // Second entry of a pair
                           printf ("\n     Offset2: 0x%X, Value2: 0x%X, Length2: %i",
                              scatp->r_address, scatp->r_value, 1 << scatp->r_length);
                        }
                        if (scatp->r_pcrel) printf(", PC relative");
                     }
                     else {
                        // non-scattered
                        if (relp->r_extern) printf ("\n    Symbol number %i, ", relp->r_symbolnum);
                        else printf ("\n    Section: %i, ", relp->r_symbolnum);
                        printf ("Offset: 0x%X, ", relp->r_address);
                        if (relp->r_pcrel) printf ("PC relative, ");
                        printf ("\n     Length: %i, Extern: %i, Type: %s",
                           1 << relp->r_length, relp->r_extern,
                           Lookup(Mac64RelocationTypeNames, relp->r_type));
                        if (relp->r_type != MAC64_RELOC_SUBTRACTOR && relp->r_address < sectp->size) {
                           // Print inline addend
                           if (relp->r_length == 3) {
                              // 8 bytes inline addend
                              printf(", Inline: 0x%08X%08X", *(int32*)(Buf()+sectp->offset+relp->r_address+4), *(int32*)(Buf()+sectp->offset+relp->r_address));
                           }
                           else {
                              // 4 bytes inline addend
                              printf(", Inline: 0x%08X", *(int32*)(Buf()+sectp->offset+relp->r_address));
                           }
                        }
                     }
                  }
               }
            }
         }
         currentp += cmdsize;
      }
   }

   // pointer to string table
   char * strtab = (char*)(Buf() + StringTabOffset); 
   // pointer to symbol table
   TMAC_nlist * symp0 = (TMAC_nlist*)(Buf() + SymTabOffset);

   // Dump symbol table
   if (options & DUMP_SYMTAB) {
      printf("\n\nSymbol table:");
      uint32 i;
      TMAC_nlist * symp;

      // loop through symbol table
      for (i = 0, symp = symp0; i < SymTabNumber; i++, symp++) {

         // Header for first symbol of each category: (alphabetical within each category)
         if (i == ilocalsym && nlocalsym)   printf("\n\n  Local symbols:");
         if (i == iextdefsym && nextdefsym) printf("\n\n  Public symbols:");
         if (i == iundefsym && nundefsym)   printf("\n\n  External symbols:");

         if (symp->n_strx < StringTabSize && !(symp->n_type & MAC_N_STAB)) {
            printf("\n  %2i %s, Section %i, Value 0x%X\n    ",
               i, strtab + symp->n_strx, symp->n_sect, uint32(symp->n_value));
         }
         else {
            printf("\n  String table offset: 0x%X, Section %i, Value 0x%X\n    ",
               symp->n_strx, symp->n_sect, uint32(symp->n_value));
         }

         if (symp->n_type & MAC_N_STAB) {
            printf ("Debug symbol, stab = 0x%X, ", symp->n_type);
         }
         else {
            if (symp->n_type & MAC_N_PEXT) printf ("Private external (limited global scope), ");
            if (symp->n_type & MAC_N_EXT ) printf ("External, ");
            printf("%s", Lookup(MacSymbolTypeNames, symp->n_type & MAC_N_TYPE));
         }
         printf("\n    Reference type: %s,  Flags: ",
            Lookup(MacSymbolReferenceTypeNames, symp->n_desc & MAC_REF_TYPE));
         for (uint32 f = MAC_REFERENCED_DYNAMICALLY; f <= MAC_N_WEAK_DEF; f <<= 1) {
            if (symp->n_desc & f) {
               printf("%s, ", Lookup(MacSymbolDescriptorFlagNames, f));
            }
         }
      }
      // Check if indirect symbol table is valid
      if (IndirectSymTabNumber && IndirectSymTabOffset + IndirectSymTabNumber*4 < this->GetDataSize()) {

         // Write indirect symbol table
         printf("\n\n  Indirect symbols:");

         // loop through indirect symbol table
         uint32 * IndSymip = (uint32*)(Buf() + IndirectSymTabOffset);

         for (i = 0; i < IndirectSymTabNumber; i++, IndSymip++) {

            // Check if index within symbol table
            if (*IndSymip >= SymTabNumber) {
               //err.submit(2016); 
               printf("\n   Unknown(0x%X)", *IndSymip);
               continue;
            }
            // Find record
            TMAC_nlist * pIndSym = symp0 + *IndSymip;
            // Find name
            uint32 StringIndex = pIndSym->n_strx;
            if (StringIndex >= StringTabSize) {
               err.submit(2035); continue;
            }
            // print name
            printf("\n   %s", strtab + StringIndex);
            // print type, etc.
            printf(", type 0x%X, sect %i, desc 0x%X, val 0x%X",
               pIndSym->n_type, pIndSym->n_sect, pIndSym->n_desc, uint32(pIndSym->n_value));
         }
      }
   }

   // Dump string table
   if (options & DUMP_STRINGTB) {
      printf("\n\nString table:");
      uint32 str = 0, istr = 0;
      while (str < StringTabSize) {
         char * p = (char*)(Buf() + StringTabOffset + str);
         printf("\n  %3i: %s", str, p);
         istr++;  str += (uint32)strlen(p) + 1;
      }
   }

}

template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMACHO<MACSTRUCTURES>::PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m) {
   // Make list of public names
   uint32 i;
   SStringEntry se;                    // Entry in Index

   // Interpret header:
   ParseFile();

   // pointer to string table
   char * strtab = (char*)(Buf() + StringTabOffset); 

   // loop through public symbol table
   TMAC_nlist * symp = (TMAC_nlist*)(Buf() + SymTabOffset + iextdefsym * sizeof(TMAC_nlist));
   for (i = 0; i < nextdefsym; i++, symp++) {
      if (symp->n_strx < StringTabSize && !(symp->n_type & MAC_N_STAB)) {
         // Public symbol found
         se.Member = m;
         // Store name
         se.String = Strings->PushString(strtab + symp->n_strx);         
         // Store name index
         Index->Push(se);
      }
   }
}

// Member functions for class MacSymbolTableBuilder

template <class TMAC_nlist, class MInt>
MacSymbolTableBuilder<TMAC_nlist, MInt>::MacSymbolTableBuilder() {                       // Constructor
   sorted = 0;
}

template <class TMAC_nlist, class MInt>
void MacSymbolTableBuilder<TMAC_nlist, MInt>::AddSymbol(int OldIndex, const char * name, int type, int Desc, int section, MInt value) {
   // Add symbol to list
   MacSymbolRecord<TMAC_nlist> rec;
   memset(&rec, 0, sizeof(rec));                 // Set to zero
/* !!
   if (GetNumEntries() == 0) {
      // First record must indicate empty string
      rec.Name = StringBuffer.PushString("");    // Empty string
      Push(&rec, sizeof(rec));                   // Put empty record in memory buffer
   }
*/
   rec.n_type = (uint8)type;                     // Copy values
   rec.n_sect = (uint8)section;
   rec.n_desc = (int16)Desc;
   rec.n_value = value;
   rec.Name = StringBuffer.PushString(name);     // Copy name and store index
   rec.OldIndex = OldIndex;                      // Remember old index
   Push(&rec, sizeof(rec));                      // Put in memory buffer
   sorted = 0;                                   // Remember not sorted
}

template <class TMAC_nlist, class MInt>
void MacSymbolTableBuilder<TMAC_nlist, MInt>::SortList() {
   // Sort the list
   if (sorted) return; // allready sorted

   MacSymbolRecord<TMAC_nlist> * p = (MacSymbolRecord<TMAC_nlist>*)Buf();     // Point to list

   // Simple Shell sort with Sedgewick gaps:
   int i, j, k, gap, n = (int)GetNumEntries();
   for (k = 15; k >= 0; k--) {
      gap = (1 << 2 * k) | (3 << k >> 1) | 1;   // Sedgewick gap grants O(N^4/3) 
      for (i = gap; i < n; i++) {
         MacSymbolRecord<TMAC_nlist> key = p[i];
         char * strkey = StringBuffer.Buf() + key.Name;
         for (j = i - gap; j >= 0 && strcmp(strkey, StringBuffer.Buf() + p[j].Name) < 0; j -= gap) {
            p[j + gap] = p[j];
         }
         p[j + gap] = key;
      }
   }

   sorted = 1;
}

template <class TMAC_nlist, class MInt>
int MacSymbolTableBuilder<TMAC_nlist, MInt>::TranslateIndex(int OldIndex) {
   // Translate old index to new index (0-based)
   // Returns -1 if not found

   // Don't sort list. This would change indices if __mh_executer_header added later
   // if (!sorted) SortList();

   MacSymbolRecord<TMAC_nlist> * p = (MacSymbolRecord<TMAC_nlist>*)Buf();     // Point to list

   // Search through list for OldIndex
   for (int i = 0; i < (int)GetNumEntries(); i++) {
      if (p[i].OldIndex == OldIndex) {
         // Match found
         return i;
      }
   }
   // Not found
   return -1;
}

template <class TMAC_nlist, class MInt>
void MacSymbolTableBuilder<TMAC_nlist, MInt>::StoreList(CMemoryBuffer * SymbolTable, CMemoryBuffer * StringTable) {
   // Store sorted list in buffers

   // Don't sort list unless commanded to do so. Will mess up indices
   // if (!sorted) SortList();                           // Make sure list is sorted

   MacSymbolRecord<TMAC_nlist> * p = (MacSymbolRecord<TMAC_nlist>*)Buf();     // Point to list

   for (uint32 i = 0; i < GetNumEntries(); i++, p++) {
      p->n_strx = StringTable->PushString(StringBuffer.Buf()+p->Name);   // Put name in string table
      SymbolTable->Push(p, sizeof(TMAC_nlist));        // Store only the TMAC_nlist part of the record in SymbolTable
   }
}

template <class TMAC_nlist, class MInt>
int MacSymbolTableBuilder<TMAC_nlist, MInt>::Search(const char * name) {
   // Search for name. Return -1 if not found.
   MacSymbolRecord<TMAC_nlist> * p = (MacSymbolRecord<TMAC_nlist>*)Buf();     // Point to list
   for (int i = 0; i < (int)GetNumEntries(); i++) {
      if (strcmp(StringBuffer.Buf()+p[i].Name, name) == 0) {
         return i;  // Found
      }
   }
   return -1;   // Not found
}

template <class TMAC_nlist, class MInt>
MacSymbolRecord<TMAC_nlist> & MacSymbolTableBuilder<TMAC_nlist, MInt>::operator[] (uint32 i) {
   // Access member
   uint32 Offset = i * sizeof(MacSymbolRecord<TMAC_nlist>);
   if (i + sizeof(MacSymbolRecord<TMAC_nlist>) > this->GetDataSize()) {
      err.submit(9003);  Offset = 0;
   }
   return Get<MacSymbolRecord<TMAC_nlist> >(Offset);
}


/****** Class CMACUNIV for parsing Macintosh universal binary *************/
CMACUNIV::CMACUNIV() {
   // Default constructor
}


void CMACUNIV::Go(int options) {
   // Apply command options to all components

   // Check file size
   if (GetDataSize() < 28) return;

   // Read number of components
   uint32 NumComponents = EndianChange(Get<MAC_UNIV_FAT_HEADER>(0).num_arch);
   if (NumComponents == 0 || NumComponents > 10) {
      // Number of components too big or too small
      err.submit(2701, NumComponents);
      return;
   }

   uint32 i;                                     // Component number
   uint32 fo;                                    // File offset of component pointer
   CConverter ComponentBuffer;                   // Used for converting component
   CConverter OutputBuffer;                      // Temporary storage of output file
   int DesiredWordSize = cmd.DesiredWordSize;    // Desired word size, if specified on command line

   // Loop through components
   for (i = 0, fo = sizeof(MAC_UNIV_FAT_HEADER); i < NumComponents; i++, fo += sizeof(MAC_UNIV_FAT_ARCH)) {

      // Get component pointer
      MAC_UNIV_FAT_ARCH & ComponentPointer = Get<MAC_UNIV_FAT_ARCH>(fo);

      // Get offset and size of component
      uint32 ComponentOffset = EndianChange(ComponentPointer.offset);
      uint32 ComponentSize   = EndianChange(ComponentPointer.size);

      // Check within range
      if (ComponentOffset + ComponentSize > GetDataSize()) {
         err.submit(2016);
         return;
      }

      // Put component into buffer
      ComponentBuffer.Reset();
      ComponentBuffer.Push(Buf() + ComponentOffset, ComponentSize);

      // Indicate component
      printf("\n\n\nComponent file number %i:\n", i + 1);

      // Check type
      uint32 ComponentType = ComponentBuffer.GetFileType();
      if (DesiredWordSize && DesiredWordSize != ComponentBuffer.WordSize) {
         err.submit(1151, ComponentBuffer.WordSize);
      }
      else if (ComponentType != FILETYPE_MACHO_LE) {
         // Format not supported
         printf("  Format not supported: %s", GetFileFormatName(ComponentType));
      }
      else {
         // Format OK. Handle component
         if (cmd.DumpOptions == 0 && OutputBuffer.GetDataSize()) {
            // More than one component that can be converted
            err.submit(1150);
         }
         else {
            // Transfer filenames
            ComponentBuffer.FileName = FileName;
            ComponentBuffer.OutputFileName = OutputFileName;
            // Do command
            ComponentBuffer.Go();
            // Is there an output file?
            if (cmd.DumpOptions == 0) {
               // Save output file
               ComponentBuffer >> OutputBuffer;
            }
         }
      }
   }
   // Is there an output file?
   if (OutputBuffer.GetDataSize()) {
      // Take over output file and skip remaining components
      *this << OutputBuffer;
   }
}


// Make template instances for 32 and 64 bits
template class CMACHO<MAC32STRUCTURES>;
template class CMACHO<MAC64STRUCTURES>;
template class MacSymbolTableBuilder<MAC_nlist_32, int32>;
template class MacSymbolTableBuilder<MAC_nlist_64, int64>;
