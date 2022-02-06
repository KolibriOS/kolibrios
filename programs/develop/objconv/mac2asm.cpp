/****************************  mac2asm.cpp   *********************************
* Author:        Agner Fog
* Date created:  2007-05-24
* Last modified: 2008-05-12
* Project:       objconv
* Module:        mac2asm.cpp
* Description:
* Module for disassembling Mach-O files
*
* Copyright 2007-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

// Constructor
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
CMAC2ASM<MACSTRUCTURES>::CMAC2ASM() {
}

// Convert
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2ASM<MACSTRUCTURES>::Convert() {
   // Do the conversion

   // Check cpu type
   switch (this->FileHeader.cputype) {
   case MAC_CPU_TYPE_I386:
      this->WordSize = 32;  break;

   case MAC_CPU_TYPE_X86_64:
      this->WordSize = 64;  break;

   default:
      // Wrong type
      err.submit(2011, "");  return;
   }

   // check object/executable file type
   uint32 ExeType;                     // File type: 0 = object, 1 = position independent shared object, 2 = executable

   switch (this->FileHeader.filetype) {
   case MAC_OBJECT:   // Relocatable object file
      ExeType = 0;  break;

   case MAC_FVMLIB:   // fixed VM shared library file
   case MAC_DYLIB:    // dynamicly bound shared library file
   case MAC_BUNDLE:   // part of universal binary
      ExeType = 1;  break;

   case MAC_EXECUTE:  // demand paged executable file
   case MAC_CORE:     // core file
   case MAC_PRELOAD:  // preloaded executable file
      ExeType = 2;  break;

   default:  // Other types
      err.submit(2011, "");  return;
   }

   // Tell disassembler
   // Disasm.Init(ExeType, this->ImageBase);
   Disasm.Init(ExeType, 0);

   // Make Sections list and relocations list
   MakeSectionList();

   // Make Symbols list in Disasm
   MakeSymbolList();

   // Make relocations list in Disasm
   MakeRelocations();

   // Make symbol entries for imported symbols
   MakeImports();

   Disasm.Go();                                  // Disassemble

   *this << Disasm.OutFile;                      // Take over output file from Disasm
}

// MakeSectionList

template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2ASM<MACSTRUCTURES>::MakeSectionList() {
   // Make Sections list and Relocations list in Disasm

   uint32 icmd;                        // Command index
   int32  isec1;                       // Section index within segment
   int32  isec2 = 0;                   // Section index global
   int32  nsect;                       // Number of sections in segment
   uint32 cmd;                         // Load command
   uint32 cmdsize;                     // Command size

   StringBuffer.Push(0, 1);            // Initialize string buffer

   // Pointer to current position
   uint8 * currentp = (uint8*)(this->Buf() + sizeof(TMAC_header));

   // Loop through file commands
   for (icmd = 1; icmd <= this->FileHeader.ncmds; icmd++) {
      cmd     = ((MAC_load_command*)currentp) -> cmd;
      cmdsize = ((MAC_load_command*)currentp) -> cmdsize;

      if (cmd == MAC_LC_SEGMENT || cmd == MAC_LC_SEGMENT_64) {
         // This is a segment command
         if ((this->WordSize == 64) ^ (cmd == MAC_LC_SEGMENT_64)) {
            // Inconsistent word size
            err.submit(2320);  break;
         }

         // Number of sections in segment
         nsect   = ((TMAC_segment_command*)currentp) -> nsects;

         // Find first section header
         TMAC_section * sectp = (TMAC_section*)(currentp + sizeof(TMAC_segment_command));

         // Loop through section headers
         for (isec1 = 1; isec1 <= nsect; isec1++, sectp++) {

            if (sectp->offset >= this->GetDataSize()) {
               // points outside file
               err.submit(2035);  break;
            }

            // Get section properties
            isec2++;                   // Section number
            uint32 MacSectionType = sectp->flags & MAC_SECTION_TYPE;
            uint8 * Buffer = (uint8*)(this->Buf()) + sectp->offset;
            uint32 TotalSize = (uint32)sectp->size;
            uint32 InitSize = TotalSize;
            if (MacSectionType == MAC_S_ZEROFILL) InitSize = 0;
            uint32 SectionAddress = (uint32)sectp->addr;
            uint32 Align = sectp->align;

            // Get section type
            // 0 = unknown, 1 = code, 2 = data, 3 = uninitialized data, 4 = constant data
            uint32 Type = 0;
            if (sectp->flags & (MAC_S_ATTR_PURE_INSTRUCTIONS | MAC_S_ATTR_SOME_INSTRUCTIONS)) {
               Type = 1; // code
            }
            else if (MacSectionType == MAC_S_ZEROFILL) {
               Type = 3; // uninitialized data
            }
            else {
               Type = 2; // data or anything else
            }

            // Make section name by combining segment name and section name
            uint32 NameOffset = StringBuffer.Push(sectp->segname, (uint32)strlen(sectp->segname)); // Segment name
            StringBuffer.Push(".", 1);  // Separate by dot
            StringBuffer.PushString(sectp->sectname);  // Section name
            char * Name = StringBuffer.Buf() + NameOffset;

            // Save section record
            Disasm.AddSection(Buffer, InitSize, TotalSize, SectionAddress, Type, Align, this->WordSize, Name);

            // Save information about relocation list for this section
            if (sectp->nreloc) {
               MAC_SECT_WITH_RELOC RelList = {isec2, sectp->offset, sectp->nreloc, sectp->reloff};
               RelocationQueue.Push(RelList);
            }

            // Find import tables
            if (MacSectionType >= MAC_S_NON_LAZY_SYMBOL_POINTERS && MacSectionType <= MAC_S_LAZY_SYMBOL_POINTERS /*?*/) {
               // This is an import table
               ImportSections.Push(sectp);
            }
            // Find literals sections
            if (MacSectionType == MAC_S_4BYTE_LITERALS || MacSectionType == MAC_S_8BYTE_LITERALS) {
               // This is a literals section
               ImportSections.Push(sectp);
            }
         }
      }
      currentp += cmdsize;
   }
}

// MakeRelocations
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2ASM<MACSTRUCTURES>::MakeRelocations() {
   // Make relocations for object and executable files
   uint32 iqq;                         // Index into RelocationQueue = table of relocation tables
   uint32 irel;                        // Index into relocation table
   int32  Section;                     // Section index
   uint32 SectOffset;                  // File offset of section binary data
   uint32 NumReloc;                    // Number of relocations records for this section
   uint32 ReltabOffset;                // File offset of relocation table for this section
   uint32 SourceOffset;                // Section-relative offset of relocation source
   uint32 SourceSize;                  // Size of relocation source
   int32  Inline = 0;                  // Inline addend at relocation source
   uint32 TargetAddress;               // Base-relative address of relocation target
   uint32 TargetSymbol;                // Symbol index of target
   //int32  TargetSection;             // Target section
   int32  Addend;                      // Offset to add to target
   uint32 ReferenceAddress;            // Base-relative address of reference point
   uint32 ReferenceSymbol;             // Symbol index of reference point
   uint32 R_Type;                      // Relocation type in Mach-O record
   uint32 R_Type2;                     // Relocation type of second entry of a pair
   uint32 R_PCRel;                     // Relocation is self-relative
   uint32 RelType = 0;                 // Relocation type translated to disasm record

   // Loop through RelocationQueue. There is one entry for each relocation table
   for (iqq = 0; iqq < RelocationQueue.GetNumEntries(); iqq++) {
      Section = RelocationQueue[iqq].Section;              // Section index
      SectOffset = RelocationQueue[iqq].SectOffset;        // File offset of section binary data
      NumReloc = RelocationQueue[iqq].NumReloc;            // Number of relocations records for this section
      ReltabOffset = RelocationQueue[iqq].ReltabOffset;    // File offset of relocation table for this section

      if (NumReloc == 0) continue;

      if (ReltabOffset == 0 || ReltabOffset >= this->GetDataSize() || ReltabOffset + NumReloc*sizeof(MAC_relocation_info) >= this->GetDataSize()) {
         // Pointer out of range
         err.submit(2035);  return;
      }

      // pointer to relocation info
      union {
         MAC_relocation_info * r;
         MAC_scattered_relocation_info * s;
         int8 * b;
      } relp;
      // Point to first relocation entry
      relp.b = this->Buf() + ReltabOffset;

      // Loop through relocation table
      for (irel = 0; irel < NumReloc; irel++, relp.r++) {

         // Set defaults
         ReferenceAddress = ReferenceSymbol = TargetSymbol = Addend = 0;

         if (relp.s->r_scattered) {
            // scattered relocation entry
            SourceOffset  = relp.s->r_address;
            SourceSize    = 1 << relp.s->r_length;
            R_PCRel       = relp.s->r_pcrel;
            R_Type        = relp.s->r_type;
            TargetAddress = relp.s->r_value;
            TargetSymbol  = 0;
         }
         else {
            // non-scattered relocation entry
            SourceOffset  = relp.r->r_address;
            SourceSize    = 1 << relp.r->r_length;
            R_PCRel       = relp.r->r_pcrel;
            R_Type        = relp.r->r_type;
            if (relp.r->r_extern) {
               TargetSymbol = relp.r->r_symbolnum + 1;
            }
            else {
               //TargetSection = relp.r->r_symbolnum;
            }
            TargetAddress = 0;
         }

         if (this->WordSize == 32 && (R_Type == MAC32_RELOC_SECTDIFF || R_Type == MAC32_RELOC_LOCAL_SECTDIFF)) {
            // This is the first of a pair of relocation entries.
            // Get second entry containing reference point
            irel++;  relp.r++;
            if (irel >= NumReloc) {err.submit(2050); break;}

            if (relp.s->r_scattered) {
               // scattered relocation entry
               R_Type2          = relp.s->r_type;
               ReferenceAddress = relp.s->r_value;
               ReferenceSymbol  = 0;
            }
            else {
               // non-scattered relocation entry
               ReferenceSymbol  = relp.r->r_symbolnum + 1;
               R_Type2          = relp.r->r_type;
               ReferenceAddress = 0;
            }
            if (R_Type2 != MAC32_RELOC_PAIR) {err.submit(2050); break;}

            if (ReferenceSymbol == 0) {
               // Reference point has no symbol index. Make one
               ReferenceSymbol = Disasm.AddSymbol(ASM_SEGMENT_IMGREL, ReferenceAddress, 0, 0, 2, 0, 0);
            }
         }

         if (this->WordSize == 64 && R_Type == MAC64_RELOC_SUBTRACTOR) {
            // This is the first of a pair of relocation entries.
            // The first entry contains reference point to subtract
            irel++;  relp.r++;
            if (irel >= NumReloc || relp.s->r_scattered || relp.r->r_type != MAC64_RELOC_UNSIGNED) {
               err.submit(2050); break;
            }
            ReferenceSymbol = TargetSymbol;
            R_PCRel       = relp.r->r_pcrel;
            if (relp.r->r_extern) {
               TargetSymbol = relp.r->r_symbolnum + 1;
            }
            else {
               //TargetSection = relp.r->r_symbolnum;
            }
            TargetAddress = 0;
         }

         // Get inline addend or address
         if (SectOffset + SourceOffset < this->GetDataSize()) {
            switch (SourceSize) {
            case 1:
               Inline = CMemoryBuffer::Get<int8>(SectOffset+SourceOffset);
               // (this->Get<int8> doesn't work on Gnu compiler 4.0.1)
               break;
            case 2:
               Inline = CMemoryBuffer::Get<int16>(SectOffset+SourceOffset);
               break;
            case 4: case 8:
               Inline = CMemoryBuffer::Get<int32>(SectOffset+SourceOffset);
               break;
            default:
               Inline = 0;
            }
         }

         if (this->WordSize == 32) {
            // Calculate target address and addend, 32 bit system
            if (R_Type == MAC32_RELOC_SECTDIFF || R_Type == MAC32_RELOC_LOCAL_SECTDIFF) {
               // Relative to reference point
               // Compensate for inline value = TargetAddress - ReferenceAddress;
               Addend = ReferenceAddress - TargetAddress;
            }
            else if (R_PCRel) {
               // Self-relative
               TargetAddress += Inline + SourceOffset + SourceSize;
               Addend = -4 - Inline;
            }
            else {
               // Direct
               TargetAddress += Inline;
               Addend = -Inline;
            }
         }

         if (TargetSymbol == 0) {
            // Target has no symbol index. Make one
            TargetSymbol = Disasm.AddSymbol(ASM_SEGMENT_IMGREL, TargetAddress, 0, 0, 2, 0, 0);
         }

         // Find type
         if (this->WordSize == 32) {
            switch (R_Type) {
            case MAC32_RELOC_VANILLA:
               // Direct or self-relative
               RelType = R_PCRel ? 2 : 1;
               break;

            case MAC32_RELOC_SECTDIFF: case MAC32_RELOC_LOCAL_SECTDIFF:
               // Relative to reference point
               RelType = 0x10;
               break;

            case MAC32_RELOC_PB_LA_PTR:
               // Lazy pointer
               RelType = 0x41; //??
               break;

            default:
               // Unknown type
               err.submit(2030, R_Type);
               break;
            }
         }
         else { // 64-bit relocation types
            switch (R_Type) {
            case MAC64_RELOC_UNSIGNED:
               // Absolute address
               RelType = 1;  
               break;
            case MAC64_RELOC_BRANCH:
               // Signed 32-bit displacement with implicit -4 addend
            case MAC64_RELOC_SIGNED:
               // Signed 32-bit displacement with implicit -4 addend
            case MAC64_RELOC_SIGNED_1:
               // Signed 32-bit displacement with implicit -4 addend and explicit -1 addend
            case MAC64_RELOC_SIGNED_2:
               // Signed 32-bit displacement with implicit -4 addend and explicit -2 addend
            case MAC64_RELOC_SIGNED_4:
               // Signed 32-bit displacement with implicit -4 addend and explicit -4 addend
               RelType = 2;  Addend -= 4;  
               break;
            case MAC64_RELOC_GOT:
               // Absolute or relative reference to GOT?
               // RelType = 0x1001; break;
            case MAC64_RELOC_GOT_LOAD: 
               // Signed 32-bit displacement to GOT
               RelType = 0x1002;  Addend -= 4;  
               break;
            case MAC64_RELOC_SUBTRACTOR:
               // 32 or 64 bit relative to arbitrary reference point
               RelType = 0x10;  
               break;
            default:
               // Unknown type
               err.submit(2030, R_Type);
               break;
            }
         }

         // Make relocation record
         Disasm.AddRelocation(Section, SourceOffset, Addend, 
            RelType, SourceSize, TargetSymbol, ReferenceSymbol);
      }
   }
}

// MakeSymbolList
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2ASM<MACSTRUCTURES>::MakeSymbolList() {
   // Make Symbols list in Disasm
   uint32 symi;                        // Symbol index, 0-based
   uint32 symn = 0;                    // Symbol number, 1-based
   char * Name;                        // Symbol name
   int32  Section;                     // Section number (1-based). 0 = external, ASM_SEGMENT_ABSOLUTE = absolute, ASM_SEGMENT_IMGREL = image-relative
   uint32 Offset;                      // Offset into section. (Value for absolute symbol)
   uint32 Type;                        // Symbol type. Use values listed above for SOpcodeDef operands. 0 = unknown type
   uint32 Scope;                       // 1 = function local, 2 = file local, 4 = public, 8 = weak public, 0x10 = communal, 0x20 = external

   // pointer to string table
   char * strtab = (char*)(this->Buf() + this->StringTabOffset); 

   // loop through symbol table
   TMAC_nlist * symp = (TMAC_nlist*)(this->Buf() + this->SymTabOffset);
   for (symi = 0; symi < this->SymTabNumber; symi++, symp++) {

      if (symp->n_type & MAC_N_STAB) {
         // Debug symbol. Ignore
         continue;
      }

      if (symp->n_strx < this->StringTabSize) {
         // Normal symbol
         Section = symp->n_sect;
         Offset  = (uint32)symp->n_value;
         Name    = strtab + symp->n_strx;
         symn    = symi + 1;           // Convert 0-based to 1-based index

         // Get scope
         if (symi < this->iextdefsym) {
            // Local
            Scope = 2;
         }
         else if (Section && (symp->n_type & MAC_N_TYPE) != MAC_N_UNDF) {
            // Public
            Scope = 4;
         }
         else {
            // External
            Scope = 0x20;
         }
         // Check if absolute
         if ((symp->n_type & MAC_N_TYPE) == MAC_N_ABS) {
            // Absolute
            Section = ASM_SEGMENT_ABSOLUTE;  Scope = 4;
         }
         // Check if weak/communal
         if (symp->n_type & MAC_N_PEXT) {
            // Communal?
            Scope = 0x10;
         }
         else if (symp->n_desc & MAC_N_WEAK_DEF) {
            // Weak public
            Scope = 8;
         }
         else if (symp->n_desc & MAC_N_WEAK_REF) {
            // Weak external (not supported by disassembler)
            Scope = 0x20;
         }
         // Get type
         Type = 0;

         // Offset is always based, not section-relative
         if (Section > 0) Section = ASM_SEGMENT_IMGREL;

         // Add symbol to diassembler
         Disasm.AddSymbol(Section, Offset, 0, Type, Scope, symn, Name);
      }
   }
}

template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2ASM<MACSTRUCTURES>::MakeImports() {
   // Make symbol entries for all import tables
   uint32 isec;                        // Index into ImportSections list
   uint32 SectionType;                 // Section type
   TMAC_section * sectp;                // Pointer to section
   TMAC_nlist * symp0 = (TMAC_nlist*)(this->Buf() + this->SymTabOffset); // Pointer to symbol table
   uint32 * IndSymp = (uint32*)(this->Buf() + this->IndirectSymTabOffset); // Pointer to indirect symbol table
   uint32 iimp;                        // Index into import table
   char * strtab = (char*)(this->Buf() + this->StringTabOffset);    // pointer to string table

   // Loop through import sections
   for (isec = 0; isec < ImportSections.GetNumEntries(); isec++) {
      // Pointer to section header
      sectp = ImportSections[isec];
      // Section type
      SectionType = sectp->flags & MAC_SECTION_TYPE;
      if (SectionType >= MAC_S_NON_LAZY_SYMBOL_POINTERS && SectionType <= MAC_S_MOD_INIT_FUNC_POINTERS) {

         // This section contains import tables
         // Entry size in import table
         uint32 EntrySize = sectp->reserved2;
         // Entry size is 4 if not specified
         if (EntrySize == 0) EntrySize = 4;
         // Number of entries
         uint32 NumEntries = (uint32)sectp->size / EntrySize;
         // Index into indirect symbol table entry of first entry in import table
         uint32 Firsti = sectp->reserved1;
         // Check if within range
         if (Firsti + NumEntries > this->IndirectSymTabNumber) {
            // This occurs when disassembling 64-bit Mach-O executable
            // I don't know how to interpret the import table
            err.submit(1054);  continue;
         }
         // Loop through import table entries
         for (iimp = 0; iimp < NumEntries; iimp++) {
            // Address of import table entry
            uint32 ImportAddress = (uint32)sectp->addr + iimp * EntrySize;
            // Get symbol table index from indirect symbol table
            uint32 symi = IndSymp[iimp + Firsti];
            // Check index
            if (symi == 0x80000000) {
               // This value occurs. Maybe it means ignore?
               continue;
            }
            // Check if index within symbol table
            if (symi >= this->SymTabNumber) {
               err.submit(1052); continue;
            }
            // Find name
            uint32 StringIndex = symp0[symi].n_strx;
            if (StringIndex >= this->StringTabSize) {
               err.submit(1052); continue;
            }
            const char * Name = strtab + StringIndex;
            // Name of .so to import from
            const char * DLLName = "?";

            // Symbol type
            uint32 Type = 0;
            switch (SectionType) {
         case MAC_S_NON_LAZY_SYMBOL_POINTERS:
         case MAC_S_LAZY_SYMBOL_POINTERS:
            // pointer to symbol
            Type = 3;  break;
         case MAC_S_SYMBOL_STUBS:
            // jump to function
            Type = 0x83;  
            // Make appear as direct call
            DLLName = 0;
            break;
         case MAC_S_MOD_INIT_FUNC_POINTERS:
            // function pointer?
            Type = 0x0C;  break;
            }

            // Make symbol record for disassembler
            Disasm.AddSymbol(ASM_SEGMENT_IMGREL, ImportAddress, 4, Type, 2, 0, Name, DLLName);
         }
      }
      else if (SectionType == MAC_S_4BYTE_LITERALS) {
         // Section contains 4-byte float constants. 
         // Make symbol
         Disasm.AddSymbol(ASM_SEGMENT_IMGREL, (uint32)sectp->addr, 4, 0x43, 2, 0, "Float_constants");
      }
      else if (SectionType == MAC_S_8BYTE_LITERALS) {
         // Section contains 8-byte double constants. 
         // Make symbol
         Disasm.AddSymbol(ASM_SEGMENT_IMGREL, (uint32)sectp->addr, 8, 0x44, 2, 0, "Double_constants");
      }
   }
}


// Make template instances for 32 and 64 bits
template class CMAC2ASM<MAC32STRUCTURES>;
template class CMAC2ASM<MAC64STRUCTURES>;
