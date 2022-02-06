/****************************  cof2asm.cpp   ********************************
* Author:        Agner Fog
* Date created:  2007-02-25
* Last modified: 2009-12-20
* Project:       objconv
* Module:        cof2asm.cpp
* Description:
* Module for disassembling PE/COFF file
*
* Copyright 2007-2009 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

CCOF2ASM::CCOF2ASM () {
   // Constructor
}

void CCOF2ASM::Convert() {
   // Do the conversion
   if (ImageBase) Disasm.Init(2, ImageBase);     // Executable file or DLL. Set image base
   MakeSectionList();                            // Make Sections list and Relocations list in Disasm
   MakeSymbolList();                             // Make Symbols list in Disasm
   if (ImageBase) {
      // Executable file
      MakeDynamicRelocations();                  // Make dynamic base relocations for executable files
      MakeImportList();                          // Make imported symbols for executable files
      MakeExportList();                          // Make exported symbols for executable files
      MakeListLabels();                          // Put labels on all image directory tables
   }
   Disasm.Go();                                  // Disassemble
   *this << Disasm.OutFile;                      // Take over output file from Disasm
}

void CCOF2ASM::MakeSectionList() {
   // Make Sections list and Relocations list in Disasm
   uint32 isec;                                  // Section index
   uint32 irel;                                  // Relocation index

   // Loop through sections
   for (isec = 0; isec < (uint32)NSections; isec++) {

      // Get section header
      SCOFF_SectionHeader * SectionHeader = &SectionHeaders[isec];

      // Section properties
      const char * Name  = GetSectionName(SectionHeader->Name);
      uint8 * Buffer = (uint8*)Buf() + SectionHeader->PRawData;
      uint32 InitSize = SectionHeader->SizeOfRawData;
      uint32 TotalSize = SectionHeader->VirtualSize;

      uint32 SectionAddress = SectionHeader->VirtualAddress;
      uint32 Type  = (SectionHeader->Flags & PE_SCN_CNT_CODE) ? 1 : 2;
      if (SectionHeader->Flags & PE_SCN_CNT_UNINIT_DATA) {
         // BSS segment. No data in file
         Buffer = 0;
         Type = 3;
      }
      else if (!(SectionHeader->Flags & (PE_SCN_MEM_WRITE | PE_SCN_MEM_EXECUTE))) {
         // Constant segment
         Type = 4;
      }
      if (SectionHeader->Flags & PE_SCN_LNK_COMDAT) {
         // Communal section
         Type |= 0x1000;
      }
      if (strnicmp(Name,"debug",5) == 0 || strnicmp(Name+1,"debug",5) == 0) {
         // This is a debug section
         Type = 0x10;
      }
      if (strnicmp(Name,".pdata", 6) == 0) {
         // This section has exception information
         Type = 0x11;
      }

      uint32 Align = (SectionHeader->Flags & PE_SCN_ALIGN_MASK) / PE_SCN_ALIGN_1;
      if (Align) Align--;

      // Save section record
      Disasm.AddSection(Buffer, InitSize, TotalSize, SectionAddress, Type, Align, WordSize, Name);

      // Get relocations 
      // Pointer to relocation entry
      union {
         SCOFF_Relocation * p;  // pointer to record
         int8 * b;              // used for address calculation and incrementing
      } Reloc;
      Reloc.b = Buf() + SectionHeader->PRelocations;

      for (irel = 0; irel < SectionHeader->NRelocations; irel++, Reloc.b += SIZE_SCOFF_Relocation) {

         // Relocation properties
         int32 Section = isec + 1;
         uint32 Offset = Reloc.p->VirtualAddress;
         int32 Addend  = 0;
         uint32 TargetIndex = Reloc.p->SymbolTableIndex;

         // Translate relocation type
         uint32 Type = 0, Size = 0;
         if (WordSize == 32) {
            // 32 bit relocation types
            // 0 = unknown, 1 = direct, 2 = self-relative, 3 = image-relative, 4 = segment relative
            switch(Reloc.p->Type) {
            case COFF32_RELOC_ABS:  // Ignore
               continue;
            case COFF32_RELOC_DIR32: // Direct, 32 bits
               Type = 1;
               Size = 4;
               break;
            case COFF32_RELOC_REL32: // Self-relative, 32 bits
               Type = 2;
               Size = 4;
               Addend = -4;
               break;
            case COFF32_RELOC_IMGREL: // Image relative, 32 bits
               Type = 4;
               Size = 4;
               break;
            case COFF32_RELOC_SECREL: // Section relative, 32 bits
               Type = 8;
               Size = 4;
               break;
            case COFF32_RELOC_SECTION: // Section index of symbol, 16 bits
               Type = 0x200;
               Size = 2;
               break;
            default: // Other/unknown
               Type = 0;
               Size = 4;
            }
         }
         else { // WordSize = 64
            switch(Reloc.p->Type) {
            case COFF64_RELOC_ABS:  // Ignore
               continue;
            case COFF64_RELOC_ABS32: // Absolute 32 bit
               Type = 1;
               Size = 4;
               break;
            case COFF64_RELOC_ABS64: // Absolute 64 bit
               Type = 1;
               Size = 8;
               break;
            case COFF64_RELOC_IMGREL: // Image relative 32 bit
               Type = 4;
               Size = 4;
               break;
            case COFF64_RELOC_REL32:    // Self-relative, 32 bits
            case COFF64_RELOC_REL32_1:  // Self-relative + 1
            case COFF64_RELOC_REL32_2:  // Self-relative + 2
            case COFF64_RELOC_REL32_3:  // Self-relative + 3
            case COFF64_RELOC_REL32_4:  // Self-relative + 4
            case COFF64_RELOC_REL32_5:  // Self-relative + 5
               Type = 2;
               Size = 4;
               Addend = - (Reloc.p->Type + 4 - COFF64_RELOC_REL32);
               break;
            case COFF64_RELOC_SECREL:   // Section relative
               Type = 8;
               Size = 4;
               break;
            default: // Other/unknown
               Type = 0;
               Size = 4;
            }
         }
         // Save relocation record
         Disasm.AddRelocation(Section, Offset, Addend, Type, Size, TargetIndex);
      }
   }
}

void CCOF2ASM::MakeSymbolList() {
   // Make Symbols list in Disasm
   uint32 isym;                                  // Symbol index
   uint32 naux = 0;                              // Number of auxiliary entries in old symbol table

   union {                                       // Pointer to old symbol table entries
      SCOFF_SymTableEntry * p;                   // Normal pointer
      int8 * b;                                  // Used for address calculation
   } Sym, SymAux;

   // Set pointer to old SymbolTable
   Sym.p = SymbolTable;

   // Loop through old symbol table
   for (isym = 0; isym < (uint32)NumberOfSymbols; isym += 1+naux, Sym.b += (1+naux) * SIZE_SCOFF_SymTableEntry) {

      // Number of auxiliary entries
      naux = Sym.p->s.NumAuxSymbols;

      if (Sym.p->s.SectionNumber != COFF_SECTION_ABSOLUTE
      && (Sym.p->s.SectionNumber < 0 
      || (Sym.p->s.StorageClass != COFF_CLASS_EXTERNAL && Sym.p->s.StorageClass != COFF_CLASS_STATIC && Sym.p->s.StorageClass != COFF_CLASS_LABEL))) {
         // Ignore irrelevant symbol table entries
         continue;
      }

      // Symbol properties
      uint32 Index   = isym;
      int32  Section = Sym.p->s.SectionNumber;
      uint32 Offset  = Sym.p->s.Value;
      uint32 Size    = 0;
      uint32 Type    = (Sym.p->s.Type == COFF_TYPE_FUNCTION) ? 0x83 : 0;

      // Identify segment entries in symbol table
      if (Sym.p->s.Value == 0 && Sym.p->s.StorageClass == COFF_CLASS_STATIC 
      && naux && Sym.p->s.Type != 0x20) {
         // Note: The official MS specification says that a symbol table entry 
         // is a section if the storage class is static and the value is 0,
         // but I have encountered static functions that meet these criteria.
         // Therefore, I am also checking Type and naux.
         Type = 0x80000082;
      }

      const char * Name = GetSymbolName(Sym.p->s.Name);

      // Get scope. Note that these values are different from the constants defined in maindef.h
      uint32 Scope = 0;
      if (Sym.p->s.StorageClass == COFF_CLASS_STATIC || Sym.p->s.StorageClass == COFF_CLASS_LABEL) {
         Scope = 2;             // Local
      }
      else if (Sym.p->s.SectionNumber > 0 || (Sym.p->s.SectionNumber == -1 && Sym.p->s.StorageClass == COFF_CLASS_EXTERNAL)) {
         Scope = 4;             // Public
      }
      else {
         Scope = 0x20;          // External
      }

      // Check auxiliary symbol table entries
      if (naux && Sym.p->s.Type == COFF_TYPE_FUNCTION) {
         // Function symbol has auxiliary entry. Get information about size
         SymAux.b = Sym.b + SIZE_SCOFF_SymTableEntry;
         Size = SymAux.p->func.TotalSize;
      }
      // Check for special section values
      if (Section < 0) {
         if (Section == COFF_SECTION_ABSOLUTE) {
            // Symbol is an absolute constant
            Section = ASM_SEGMENT_ABSOLUTE;
         }
         else {
            // Debug symbols, etc
            Section = ASM_SEGMENT_ERROR;
         }
      }

      // Store new symbol record
      Disasm.AddSymbol(Section, Offset, Size, Type, Scope, Index, Name);
   }
}

void CCOF2ASM::MakeDynamicRelocations() {
   // Make dynamic base relocations for executable files
   
   // Find base relocation table
   SCOFF_ImageDirAddress reldir;
   if (!GetImageDir(5, &reldir)) {
      // No base relocation table found
      return;
   }

   SCOFF_BaseRelocationBlock * pBaseRelocation;  // Start of dynamic base relocation section

   // Beginning of .reloc section is first base relocation block
   pBaseRelocation = &Get<SCOFF_BaseRelocationBlock>(reldir.FileOffset);

   uint32 ROffset = 0;                        // Offset into .reloc section
   uint32 BlockEnd;                           // Offset of end of current block
   uint32 PageOffset;                         // Image-relative address of begin of page

   // Make pointer to header or entry in .reloc section
   union {
      SCOFF_BaseRelocationBlock * header;
      SCOFF_BaseRelocation * entry;
      int8 * b;
   } Pointer;

   // Loop throung .reloc section
   // while (ROffset < reldir.MaxOffset) {
   while (ROffset < reldir.Size) {
      // Set Pointer to current position
      Pointer.header = pBaseRelocation;
      Pointer.b += ROffset;

      // Read base relocation block
      PageOffset = Pointer.header->PageRVA;
      BlockEnd = ROffset + Pointer.header->BlockSize;

      // Read entries in this block
      ROffset   += sizeof(SCOFF_BaseRelocationBlock);
      Pointer.b += sizeof(SCOFF_BaseRelocationBlock);
      // Loop through entries
      while (ROffset < BlockEnd) {
         // Set Pointer to current position
         Pointer.header = pBaseRelocation;
         Pointer.b += ROffset;

         if (Pointer.entry->Type == COFF_REL_BASED_HIGHLOW) {
            // Add relocation record, 32 bit
            // Section = ASM_SEGMENT_IMGREL means offset is image-relative
            // Type = 0x20 means already relocated to image base
            Disasm.AddRelocation(ASM_SEGMENT_IMGREL, Pointer.entry->Offset + PageOffset, 0, 0x21, 4, 0);
         }
         else if (Pointer.entry->Type == COFF_REL_BASED_DIR64) {
            // Add relocation record, 64 bit
            Disasm.AddRelocation(ASM_SEGMENT_IMGREL, Pointer.entry->Offset + PageOffset, 0, 0x21, 8, 0);
         }

         // Go to next
         ROffset += sizeof(SCOFF_BaseRelocation);
         if (Pointer.entry->Type == COFF_REL_BASED_HIGHADJ) ROffset += sizeof(SCOFF_BaseRelocation);
      }
      // Finished block. Align by 4
      ROffset = (ROffset + 3) & uint32(-4);
   }
}

void CCOF2ASM::MakeImportList() {
   // Make imported symbols for executable files

   // Find import table
   SCOFF_ImageDirAddress impdir;
   if (!GetImageDir(1, &impdir)) {
      // No import table found
      return;
   }

   // Beginning of import section is import directory
   SCOFF_ImportDirectory * pImportDirectory = &Get<SCOFF_ImportDirectory>(impdir.FileOffset);

   // Check if 64 bit
   int Is64bit = OptionalHeader->h64.Magic == COFF_Magic_PE64; // 1 if 64 bit
   uint32 EntrySize = Is64bit ? 8 : 4;           // Size of address table entries

   uint32 NameOffset;                            // Offset to name
   const char * SymbolName;                      // Name of symbol
   const char * DLLName;                         // Name of DLL containing symbol
   char NameBuffer[64];                          // Buffer for creating name of ordinal symbols
   uint32 SectionOffset;                         // Section-relative address of current entry
   uint32 HintNameOffset;                        // Section-relative address of hint/name table
   uint32 FirstHintNameOffset = 0;               // First HintNameOffset = start of hint/name table
   uint32 AddressTableOffset;                    // Offset of import address table relative to import lookup table

   // Pointer to current import directory entry
   SCOFF_ImportDirectory * ImportEntry = pImportDirectory;
   // Pointer to current import lookup table entry
   int32 * LookupEntry = 0;
   // Pointer to current hint/name table entry
   SCOFF_ImportHintName * HintNameEntry;

   // Loop through import directory until null entry
   while (ImportEntry->DLLNameRVA) {
      // Get DLL name
      NameOffset = ImportEntry->DLLNameRVA - impdir.VirtualAddress;
      if (NameOffset < impdir.MaxOffset) {
         DLLName = &Get<char>(impdir.FileOffset + NameOffset);
      }
      else {
         DLLName = "?";
      }

      // Get lookup table
      SectionOffset = ImportEntry->ImportLookupTableRVA;
      if (SectionOffset == 0) SectionOffset = ImportEntry->ImportAddressTableRVA;
      if (SectionOffset == 0) continue;
      // Get distance from import lookup table to import address table
      AddressTableOffset = ImportEntry->ImportAddressTableRVA - SectionOffset;
      // Section relative address
      SectionOffset -= impdir.VirtualAddress;
      if (SectionOffset >= impdir.MaxOffset) break;  // Out of range

      // Loop through lookup table
      while (1) {
         // Pointer to lookup table entry
         LookupEntry = &Get<int32>(impdir.FileOffset + SectionOffset);

         // End when entry is empty
         if (!LookupEntry[0]) break;

         if (LookupEntry[Is64bit] < 0) {
            // Imported by ordinal. Give it a name
            strncpy(NameBuffer, DLLName, 20);
            // Remove dot from name
            char * dot = strchr(NameBuffer,'.');
            if (dot) *dot = 0;
            // Add ordinal number to name
            sprintf(NameBuffer+strlen(NameBuffer), "_Ordinal_%i", uint16(LookupEntry[0]));
            SymbolName = NameBuffer;
         }
         else {
            // Find entry in hint/name table
            HintNameOffset = (LookupEntry[0] & 0x7FFFFFFF) - impdir.VirtualAddress;
            if (HintNameOffset >= impdir.MaxOffset) goto LOOPNEXT;  // Out of range
            if (!FirstHintNameOffset) FirstHintNameOffset = HintNameOffset;
            HintNameEntry = &Get<SCOFF_ImportHintName>(impdir.FileOffset + HintNameOffset);
            // Get name
            SymbolName = HintNameEntry->Name;
         }
         // Add symbol
         Disasm.AddSymbol(ASM_SEGMENT_IMGREL, impdir.VirtualAddress + SectionOffset + AddressTableOffset,
            EntrySize, 0xC, 0x20, 0, SymbolName, DLLName);

         // Loop next
         LOOPNEXT:
         SectionOffset += EntrySize;
      }

      // Loop next
      ImportEntry++;
   }

   // Make label for import name table
   if (FirstHintNameOffset) {
      Disasm.AddSymbol(ASM_SEGMENT_IMGREL, impdir.VirtualAddress + FirstHintNameOffset, 1, 1, 1, 0, "Import_name_table");
   }
}

void CCOF2ASM::MakeExportList() {
   // Make exported symbols for executable files

   // Define entry point
   if (OptionalHeader->h32.AddressOfEntryPoint) {
      Disasm.AddSymbol(ASM_SEGMENT_IMGREL, OptionalHeader->h32.AddressOfEntryPoint, 0, 0x83, 4, 0, "Entry_point");
   }

   // Get export table directory address
   SCOFF_ImageDirAddress expdir;

   // Exported names
   if (!GetImageDir(0, &expdir)) {
      // No export directory
      return;
   }

   // Beginning of export section is export directory
   SCOFF_ExportDirectory * pExportDirectory = &Get<SCOFF_ExportDirectory>(expdir.FileOffset);

   // Find ExportAddressTable
   uint32 ExportAddressTableOffset = pExportDirectory->ExportAddressTableRVA - expdir.VirtualAddress;
   if (ExportAddressTableOffset == 0 || ExportAddressTableOffset >= expdir.MaxOffset) {
      // Points outside section
      err.submit(2035);  return;
   }
   uint32 * pExportAddressTable = &Get<uint32>(expdir.FileOffset + ExportAddressTableOffset);

   // Find ExportNameTable
   if (pExportDirectory->NamePointerTableRVA == 0) {
       return;  // I don't know why this happens
   }
   uint32 ExportNameTableOffset = pExportDirectory->NamePointerTableRVA - expdir.VirtualAddress;
   if (ExportNameTableOffset == 0 || ExportNameTableOffset >= expdir.MaxOffset) {
      // Points outside section
      err.submit(2035);  return;
   }
   uint32 * pExportNameTable = &Get<uint32>(expdir.FileOffset + ExportNameTableOffset);

   // Find ExportOrdinalTable
   uint32 ExportOrdinalTableOffset = pExportDirectory->OrdinalTableRVA - expdir.VirtualAddress;
   if (ExportOrdinalTableOffset == 0 || ExportOrdinalTableOffset >= expdir.MaxOffset) {
      // Points outside section
      err.submit(2035);  return;
   }
   uint16 * pExportOrdinalTable = &Get<uint16>(expdir.FileOffset + ExportOrdinalTableOffset);

   // Get further properties
   uint32 NumExports = pExportDirectory->AddressTableEntries;
   uint32 NumExportNames = pExportDirectory->NamePointerEntries;
   uint32 OrdinalBase = pExportDirectory->OrdinalBase;

   uint32 i;                                     // Index into pExportOrdinalTable and pExportNameTable
   uint32 Ordinal;                               // Index into pExportAddressTable
   uint32 Address;                               // Image-relative address of symbol
   uint32 NameOffset;                            // Section-relative address of name
   uint32 FirstName = 0;                         // Image-relative address of first name table entry
   const char * Name = 0;                        // Name of symbol
   char NameBuffer[64];                          // Buffer for making name

   // Loop through export tables
   for (i = 0; i < NumExports; i++) {

      Address = 0;
      Name = "?";

      // Get ordinal from table
      Ordinal = pExportOrdinalTable[i];
      // Address table is indexed by ordinal
      if (Ordinal < NumExports) {
         Address = pExportAddressTable[Ordinal];
      }
      // Find name if there is a name list entry
      if (i < NumExportNames) {
         NameOffset = pExportNameTable[i] - expdir.VirtualAddress;
         if (NameOffset && NameOffset < expdir.MaxOffset) {
            Name = &Get<char>(expdir.FileOffset + NameOffset);
            if (FirstName == 0) FirstName = pExportNameTable[i];
         }
      }
      else {
         // No name. Make name from ordinal number
         sprintf(NameBuffer, "Ordinal_%i", Ordinal + OrdinalBase);
         Name = NameBuffer;
      }

      // Define symbol
      Disasm.AddSymbol(ASM_SEGMENT_IMGREL, Address, 0, 0x83, 4, 0, Name);
   }

   // Make label for export section
   Disasm.AddSymbol(ASM_SEGMENT_IMGREL, expdir.VirtualAddress, 4, 3, 2, 0, "Export_tables");

   // Make labels for export tables
   Disasm.AddSymbol(ASM_SEGMENT_IMGREL, ExportAddressTableOffset - expdir.FileOffset + expdir.VirtualAddress, 4, 3, 2, 0, "Export_address_table");
   Disasm.AddSymbol(ASM_SEGMENT_IMGREL, ExportOrdinalTableOffset - expdir.FileOffset + expdir.VirtualAddress, 4, 3, 2, 0, "Export_ordinal_table");
   Disasm.AddSymbol(ASM_SEGMENT_IMGREL, ExportNameTableOffset - expdir.FileOffset + expdir.VirtualAddress, 4, 3, 2, 0, "Export_name_pointer_table");
   Disasm.AddSymbol(ASM_SEGMENT_IMGREL, FirstName, 1, 1, 2, 0, "Export_name_table");
}

void CCOF2ASM::MakeListLabels() {
   // Attach names to all image directories
   SCOFF_ImageDirAddress dir;
   uint32 i;

   for (i = 0; i < NumImageDirs; i++) {
      if (GetImageDir(i, &dir)) {
         // Found a directory. Make label for it
         Disasm.AddSymbol(ASM_SEGMENT_IMGREL, dir.VirtualAddress, 4, 0, 1, 0, dir.Name);
      }
   }
}
