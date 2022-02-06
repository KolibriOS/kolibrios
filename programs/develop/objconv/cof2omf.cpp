/****************************  cof2omf.cpp   ********************************
* Author:        Agner Fog
* Date created:  2007-02-03
* Last modified: 2007-02-03
* Project:       objconv
* Module:        cof2omf.cpp
* Description:
* Module for converting 32 bit PE/COFF file to OMF file
*
* Copyright 2007-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

CCOF2OMF::CCOF2OMF () {
   // Constructor
   memset(this, 0, sizeof(*this));
}


void CCOF2OMF::Convert() {
   // Do the conversion
   if (WordSize != 32) {
      err.submit(2317, WordSize);                // Wrong word size
      return;
   }

   // Allocate variable size buffers
   SectionBuffer.SetNum(NSections + 2);          // Allocate buffer for section translation list
   SectionBuffer.SetZero();                      // Initialize
   SymbolBuffer.SetNum(NumberOfSymbols + 1);     // Allocate buffer for symbol translation list
   SymbolBuffer.SetZero();                       // Initialize
   NameBuffer.Push(0, 1);                        // Make first entry in NameBuffer empty

   // Call the subfunctions
   ToFile.SetFileType(FILETYPE_OMF);             // Set type of output file
   MakeSegmentList();                            // Make temporary segment conversion list
   MakeSymbolList();                             // Make temporary symbol conversion list
   MakeRelocationsList();                        // Make temporary list of relocations (fixups) and sort it
   MakeLNAMES();                                 // Make THEADR and LNAMES records
   MakeSEGDEF();                                 // Make SEGDEF and GRPDEF records
   MakeEXTDEF();                                 // Make EXTDEF records
   MakePUBDEF();                                 // Make PUBDEF records
   MakeLEDATA();                                 // Make LEDATA, LIDATA and FIXUPP records
   MakeMODEND();                                 // Finish output file
   *this << ToFile;                              // Take over new file buffer containing the converted file
}


void CCOF2OMF::MakeSegmentList() {
   // Make temporary segment conversion list
   const char * oldname;                         // Old name of section
   uint32 namei;                                 // Name index into NameBuffer
   uint32 align;                                 // Segment alignment = 2^align
   int32 align2;                                 // align2 = 2^align
   uint32 flags;                                 // Old flags
   int i, j;                                     // Loop counters
   int oldsec;                                   // Old section number

   // Loop through old sections
   for (j = 0; j < NSections; j++) {
      // Old section number
      oldsec = j + 1;

      // Get old section header
      SCOFF_SectionHeader * pSectionHeader = &SectionHeaders[j];

      // Get name
      oldname = GetSectionName(pSectionHeader->Name);

      // Check for debug sections
      if (strnicmp(oldname,"debug",5) == 0 || strnicmp(oldname+1,"debug",5) == 0) {
         // This is a debug section
         if (cmd.DebugInfo == CMDL_DEBUG_STRIP) {
            // Remove debug info
            SectionBuffer[oldsec].NewNumber = 0;
            cmd.CountDebugRemoved();
            continue;
         }
         else if (cmd.InputType != cmd.OutputType) {
            err.submit(1029); // Warn that debug information is incompatible
         }
      }

      // Check for directive sections
      if (strnicmp(oldname,".drectve",8) == 0 || (pSectionHeader->Flags & (PE_SCN_LNK_INFO | PE_SCN_LNK_REMOVE))) {
         // This is a directive section
         if (cmd.ExeptionInfo) {
            // Remove directive section
            SectionBuffer[oldsec].NewNumber = 0;
            cmd.CountExceptionRemoved();
            continue;
         }
      }

      // Get alignment
      align = (pSectionHeader->Flags & PE_SCN_ALIGN_MASK) / PE_SCN_ALIGN_1;
      if (align > 0) align--;                 // Alignment = 2^align
      align2 = 1 << align;                    // 2^align

      // Check for previous sections with same name
      for (i = 0; i < SectionBufferNum; i++) {
         if (strcmp(oldname, NameBuffer.Buf() + SectionBuffer[i].OldName) == 0) break; // Found same name
      }
      if (i < SectionBufferNum) {
         // Previous section with same name found.
         // i = first section with this name, oldsec = current section with this name
         SectionBuffer[oldsec] = SectionBuffer[i];    // Copy record
         SectionBuffer[oldsec].NewNameI = 0;          // Indicate this is not the first record

         // Check if alignment is the same
         if (align != SectionBuffer[i].Align) {
            err.submit(1060, oldname);           // Warning different alignments
            if (align > SectionBuffer[i].Align) SectionBuffer[i].Align = align; // Use highest alignment
         }

         // Get section header
         SectionBuffer[oldsec].psechdr = pSectionHeader;

         // Get size of this section
         SectionBuffer[oldsec].Size = pSectionHeader->SizeOfRawData;

         // Get offset relative to first section with same name
         SectionBuffer[oldsec].Offset = SectionBuffer[i].Offset + SectionBuffer[i].SegmentSize;

         // Align this section (We are aligning each section in the segment)
         SectionBuffer[oldsec].Offset = (SectionBuffer[oldsec].Offset + align2 - 1) & (- align2);

         // Update total size of all sections with same name
         SectionBuffer[i].SegmentSize = SectionBuffer[oldsec].Offset + SectionBuffer[oldsec].Size;
      }
      else {
         // No previous section found with same name. Make SOMFSegmentList record
         SectionBufferNum = oldsec + 1;                  // End of entries in SectionBuffer

         // Assign a number to this segment
         SectionBuffer[oldsec].NewNumber = ++NumSegments;
         SectionBuffer[oldsec].NewNameI  = NumSegments + OMF_LNAME_LAST;

         // Point to old section header
         SectionBuffer[oldsec].psechdr = pSectionHeader;

         // Give it a name
         namei = NameBuffer.PushString(oldname);      // Save name in buffer, because it is volatile
         SectionBuffer[oldsec].OldName = namei;            // Index to name
         // Segment names like .text and _TEXT are both common. No need to convert the name
         // Only restriction is length < 256.
         // Do we need a unique segment name if the alignment is different from segments
         // with same name in another module?
         if (strlen(oldname) > 255) {
            // Segment name too long. This is very unlikely
            namei = NameBuffer.Push(oldname, 255);    // Make truncated name
            NameBuffer.Push(0, 1);                    // Terminate by zero
         }
         SectionBuffer[oldsec].NewName = namei;            // Index to name

         // Size
         SectionBuffer[oldsec].Size = pSectionHeader->SizeOfRawData;
         SectionBuffer[oldsec].SegmentSize = pSectionHeader->SizeOfRawData;
         SectionBuffer[oldsec].Offset = 0;

         // Alignment
         SectionBuffer[oldsec].Align = align;

         // Segment type
         flags = pSectionHeader->Flags;
      
         // Get segment class
         if (flags & (PE_SCN_CNT_CODE | PE_SCN_MEM_EXECUTE)) {
            // Code segment
            SectionBuffer[oldsec].Class = OMF_LNAME_CODE;
         }
         else if (flags & PE_SCN_CNT_UNINIT_DATA) {
            // Uninitialized data
            SectionBuffer[oldsec].Class = OMF_LNAME_BSS;
         }
         else if (!(flags & PE_SCN_MEM_WRITE)) {
            // Read only
            SectionBuffer[oldsec].Class = OMF_LNAME_CONST;
         }
         else {
            // Normal data
            SectionBuffer[oldsec].Class = OMF_LNAME_DATA;
         }
      }
   }

   // Add 1 to section count because new section numbers are 1-based
   SectionBufferNum = NSections + 1;
}


void CCOF2OMF::MakeSymbolList() {
   // Make temporary symbol conversion list
   int isym = 0;  // current symbol table entry
   //int jsym = 0;  // auxiliary entry number
   union {        // Pointer to symbol table
      SCOFF_SymTableEntry * p;  // Normal pointer
      int8 * b;                 // Used for address calculation
   } Symtab;

   Symtab.p = SymbolTable;      // Set pointer to begin of SymbolTable

   // Loop through symbol table
   while (isym < NumberOfSymbols) {
      // Check scope
      if (Symtab.p->s.StorageClass == COFF_CLASS_EXTERNAL) {
         // Scope is public or external

         if (Symtab.p->s.SectionNumber > 0) {

            // Symbol is public
            SymbolBuffer[isym].Scope = S_PUBLIC;               // Scope = public
            SymbolBuffer[isym].NewIndex = ++NumPublicSymbols;  // Public symbol number
            // Get name
            SymbolBuffer[isym].Name = NameBuffer.PushString(GetSymbolName(Symtab.p->s.Name));

            // Find section in SectionBuffer
            uint32 OldSection = Symtab.p->s.SectionNumber;
            SymbolBuffer[isym].Segment = SectionBuffer[OldSection].NewNumber; // New segment number

            // Calculate offset = offset into old section + offset of old section to first section with same name
            SymbolBuffer[isym].Offset = Symtab.p->s.Value + SectionBuffer[OldSection].Offset;
         }
         else if (Symtab.p->s.SectionNumber == 0) {

            // Symbol is external
            SymbolBuffer[isym].Scope = S_EXTERNAL;        // Scope = external
            SymbolBuffer[isym].NewIndex = ++NumExternalSymbols;  // External symbol number
            SymbolBuffer[isym].Name = NameBuffer.PushString(GetSymbolName(Symtab.p->s.Name));
         }
         else if (Symtab.p->s.SectionNumber == COFF_SECTION_ABSOLUTE) {

            // Symbol is public, absolute
            SymbolBuffer[isym].Scope = S_PUBLIC;        // Scope = public
            SymbolBuffer[isym].NewIndex = ++NumPublicSymbols;  // Public symbol number
            // Get name
            SymbolBuffer[isym].Name = NameBuffer.PushString(GetSymbolName(Symtab.p->s.Name));

            SymbolBuffer[isym].Segment = 0;          // 0 indicates absolute

            SymbolBuffer[isym].Offset = Symtab.p->s.Value;   // Store value in Offset
         }
         else {
            // COFF_SECTION_DEBUG, COFF_SECTION_N_TV, COFF_SECTION_P_TV
            // Ignore
         }
      }

      // Skip auxiliary symbol table entries and increment pointer
      isym += Symtab.p->s.NumAuxSymbols + 1;
      Symtab.b += (Symtab.p->s.NumAuxSymbols + 1) * SIZE_SCOFF_SymTableEntry;
   }
}


void CCOF2OMF::MakeRelocationsList() {
   // Make temporary list of relocations (fixups) and sort it
   uint32 i;                                     // Relocation number in old file
   int j;                                        // Section number of relocation source in old file
   int isym;                                     // Symbol table index in old file
   //int32 * paddend = 0;                          // Pointer to inline addend
   uint32 TargetOldSection;                      // Section number of relocation target in old file

   SOMFRelocation NewRel;                        // Entry in RelocationBuffer

   union {                                       // Pointer to symbol table
      SCOFF_SymTableEntry * p;                   // Normal pointer
      int8 * b;                                  // Used for address calculation
   } Symtab;

   union {                                       // Pointer to relocation entry
      SCOFF_Relocation * p;                      // pointer to record
      int8 * b;                                  // used for address calculation and incrementing
   } Reloc;

   // Loop through section headers of old file
   for (j = 0; j < NSections; j++) {
      SCOFF_SectionHeader * SectionHeaderp = &SectionHeaders[j];

      // Pointer to first relocation entry in section
      Reloc.b = Buf() + SectionHeaderp->PRelocations;

      // Loop through relocations in section
      for (i = 0; i < SectionHeaderp->NRelocations; i++) {

         // Find symbol table entry
         isym = Reloc.p->SymbolTableIndex;
         if ((uint32)isym >= (uint32)NumberOfSymbols) {
            err.submit(2040);                    // SymbolTableIndex points outside Symbol Table
            isym = 0;
         }
         Symtab.p = SymbolTable;                 // Set pointer to begin of SymbolTable
         Symtab.b += SIZE_SCOFF_SymTableEntry * isym; // Calculate address of entry isym

         // Find inline addend
         if (Reloc.p->Type < COFF32_RELOC_SEG12) {
            //paddend = (int32*)(Buf()+SectionHeaderp->PRawData+Reloc.p->VirtualAddress);
         }
         //else paddend = 0;

         // Make entry in RelocationBuffer
         // Relocation type
         switch (Reloc.p->Type) {
         case COFF32_RELOC_DIR32:                // Direct 32 bit
            NewRel.Mode = 1;  break;             // 0 = EIP-relative, 1 = direct

         case COFF32_RELOC_REL32:                // 32 bit EIP relative
            NewRel.Mode = 0;  break;             // 0 = EIP-relative, 1 = direct

         case COFF32_RELOC_ABS:                  // Ignore
            continue;

         default:                                // Other. Not supported
            NewRel.Mode = -1;                    // -1 = unsupported. 
            // Postpone error message in case it refers to a debug section that is being removed
            NewRel.TargetOffset = Reloc.p->Type; // Remember relocation type
            //err.submit(2030, Reloc.p->Type); continue;  // Unsupported relocation type
            break;
         }
         // Get source
         NewRel.Section = j + 1;                 // Section number in old file
         NewRel.SourceOffset = Reloc.p->VirtualAddress;// Offset of source relative to section

         // Get target
         if (Symtab.p->s.SectionNumber > 0) {
            // Local
            NewRel.Scope  = S_LOCAL;                   // 0 = local, 2 = external
            TargetOldSection = Symtab.p->s.SectionNumber;    // Target section
            if (TargetOldSection > uint32(NSections)) {
               // SectionNumber out of range
               err.submit(2035);  continue;
            }
            // Segment index of target in new file:
            NewRel.TargetSegment = SectionBuffer[TargetOldSection].NewNumber; 
            // Offset relative to old section
            NewRel.TargetOffset = Symtab.p->s.Value;          
            // Add offset relative to first section with same name to get offset relative to new segment
            NewRel.TargetOffset += SectionBuffer[TargetOldSection].Offset; 
         }
         else {
            // External
            NewRel.Scope  = S_EXTERNAL;               // 0 = local, 2 = external
            NewRel.TargetOffset = 0;                  // Any addend is inline    
            // Find EXTDEF index in SymbolBuffer
            NewRel.TargetSegment = SymbolBuffer[isym].NewIndex;
         }

         // Put NewRel into RelocationBuffer
         RelocationBuffer.Push(NewRel);

         // Increment pointer to relocation record
         Reloc.b += SIZE_SCOFF_Relocation;       // Next relocation record
      }
   }
   // Sort RelocationBuffer
   RelocationBuffer.Sort();

   // Store number of relocations
   NumRelocations = RelocationBuffer.GetNumEntries();

   // Check for overlapping relocation sources
   for (uint32 i = 1; i < RelocationBuffer.GetNumEntries(); i++) {
      if (RelocationBuffer[i].Section == RelocationBuffer[i-1].Section
      && RelocationBuffer[i].SourceOffset >= RelocationBuffer[i-1].SourceOffset
      && RelocationBuffer[i].SourceOffset <  RelocationBuffer[i-1].SourceOffset + 4
      && (RelocationBuffer[i].Mode == 0 || RelocationBuffer[i].Mode == 1)) {
         err.submit(2210);                       // Error: overlapping relocation sources
      }
   }
}


void CCOF2OMF::MakeLNAMES() {
   // Make THEADR and LNAMES records
   int Sec;                                      // Loop counter
   uint32 NameI;                                 // Name index

   // Make first record in output file = Translator header
   ToFile.StartRecord(OMF_THEADR);
   // Remove path from file name and limit length
   char * ShortName = CLibrary::ShortenMemberName(OutputFileName);   
   ToFile.PutString(ShortName);
   ToFile.EndRecord();

   // Make LNAMES record containing names of segments, groups and classes
   ToFile.StartRecord(OMF_LNAMES);

   // Store default group and class names
   ToFile.PutString("FLAT");                     // 1: FLAT  = group name
   ToFile.PutString("CODE");                     // 2: CODE  = class name for code
   ToFile.PutString("DATA");                     // 3: DATA  = class name for data segment
   ToFile.PutString("BSS");                      // 4: BSS   = class name for uninitialized data
   ToFile.PutString("CONST");                    // 5: CONST = class name for readonly data

   NameI = OMF_LNAME_LAST + 1;                   // Index of next name

   // Get segment names
   for (Sec = 0; Sec < SectionBufferNum; Sec++) {
      if (SectionBuffer[Sec].NewNameI == NameI) {
         // Found next segment name to add
         // Check if current record too big
         if (ToFile.GetSize() >= 1024 - 256) {   // Max size = 1024
            ToFile.EndRecord();                  // End current record
            ToFile.StartRecord(OMF_LNAMES);      // Start new LNAMES record
         }
         // Store name of this segment
         ToFile.PutString(NameBuffer.Buf() + SectionBuffer[Sec].NewName);
         NameI++;                                // Ready for next name
      }
   }
   // End LNAMES record
   ToFile.EndRecord();
}


void CCOF2OMF::MakeSEGDEF() {
   // Make SEGDEF and GRPDEF records
   int Sec;                                      // Index into SectionBuffer
   uint32 SegNum = 0;                            // Segment number in new file
   OMF_SAttrib Attr;                             // Segment attributes bitfield
   uint32 align;                                 // Alignment in new file

   // Loop through SectionBuffer
   for (Sec = 0; Sec < SectionBufferNum; Sec++) {

      if (SectionBuffer[Sec].NewNumber == SegNum+1 && SectionBuffer[Sec].NewNameI) {

         // New segment found
         SegNum++;                               // Segment number

         // Make a SEGDEF record for this segment 
         ToFile.StartRecord(OMF_SEGDEF + 1);     // Odd record number = 32 bit

         // Attributes bitfield
         Attr.u.P = 1;                           // Indicate 32 bit segment
         Attr.u.B = 0;                           // 1 indicates 4 Gbytes
         Attr.u.C = 2;                           // Indicates public combination

         // Translate alignment
         switch(SectionBuffer[Sec].Align) {
         case 0:  // Align by 1
            align = 1;  break;

         case 1:  // Align by 2
            align = 2;  break;

         case 2:  // Align by 4
            align = 5;  break;

         case 3:  // Align by 8 not supported, use 16
         case 4:  // Align by 16
            align = 3;  break;

         default:  // 32 or higher. Use 'page' alignment
            // Note: this gives 256 on some systems, 4096 on other systems
            align = 4;
            if (SectionBuffer[Sec].Align > 8) {
               err.submit(1205, 1 << SectionBuffer[Sec].Align); // Warning: alignment not supported
            }
         }
         Attr.u.A = align;                      // Put alignment into bitfield

         ToFile.PutByte(Attr.b);                // Save attributes bitfield

         // Segment length
         ToFile.PutNumeric(SectionBuffer[Sec].SegmentSize);

         // Segment name given by index into LNAMES record
         ToFile.PutIndex(SectionBuffer[Sec].NewNameI);

         // Class name index
         ToFile.PutIndex(SectionBuffer[Sec].Class);

         // Overlay index (ignored)
         ToFile.PutIndex(0);

         // End SEGDEF record
         ToFile.EndRecord();
      }
   }

   // Make GRPDEF record for the FLAT group
   ToFile.StartRecord(OMF_GRPDEF);               // Strart GRPDEF record
   ToFile.PutIndex(OMF_LNAME_FLAT);              // Index of name "FLAT"
   // No need to put segment indices into the GRPDEF record when group is FLAT
   // End GRPDEF record
   ToFile.EndRecord();
}


void CCOF2OMF::MakeEXTDEF() {
   // Make EXTDEF records
   uint32 j;                                     // SymbolBuffer entry index
   uint32 ExtSymNum = 0;                         // External symbol number

   if (NumExternalSymbols > 0) {                 // Are there any external symbols?

      // Make EXTDEF record for one or more symbols
      ToFile.StartRecord(OMF_EXTDEF);            // Start record

      // Loop through SymbolBuffer
      for (j = 0; j < SymbolBuffer.GetNumEntries(); j++) {
         if (SymbolBuffer[j].Scope == S_EXTERNAL && SymbolBuffer[j].NewIndex == ExtSymNum+1) {
            // Found external symbol
            ExtSymNum++;                         // Symbol number   

            // Check if current record too big
            if (ToFile.GetSize() >= 1024 - 257) {// Max size = 1024
               ToFile.EndRecord();               // End current record
               ToFile.StartRecord(OMF_EXTDEF);   // Start new EXTDEF record
            }
            // Put symbol name in record
            ToFile.PutString(NameBuffer.Buf() + SymbolBuffer[j].Name);

            // Type index
            ToFile.PutIndex(0);                  // Not used any more
         }
      }
      ToFile.EndRecord();                        // End EXTDEF record
   }
}


void CCOF2OMF::MakePUBDEF() {
   // Make PUBDEF records
   uint32 j;                                     // SymbolBuffer entry index
   uint32 PubSymNum = 0;                         // Public symbol number

   // Loop through SymbolBuffer
   for (j = 0; j < SymbolBuffer.GetNumEntries(); j++) {
      if (SymbolBuffer[j].Scope == S_PUBLIC && SymbolBuffer[j].NewIndex == PubSymNum+1) {
         // Found public symbol
         PubSymNum++;                            // Symbol number   

         // Make PUBDEF record for this symbol
         ToFile.StartRecord(OMF_PUBDEF + 1);     // Start new PUBDEF record, 32 bit

         // Group index
         uint32 Group = SymbolBuffer[j].Segment ? OMF_LNAME_FLAT : 0; // Group = FLAT, except for absolute symbols
         ToFile.PutIndex(Group);                 // Group name index

         // Segment index
         ToFile.PutIndex(SymbolBuffer[j].Segment);

         // Base frame field if segment = 0
         if (SymbolBuffer[j].Segment == 0) ToFile.PutWord(0);

         // Put symbol name in record
         ToFile.PutString(NameBuffer.Buf() + SymbolBuffer[j].Name);

         // Offset relative to segment
         ToFile.PutNumeric(SymbolBuffer[j].Offset);

         // Type index
         ToFile.PutIndex(0);                     // Not used any more

         // End record
         ToFile.EndRecord();                     // End PUBDEF record
      }
   }
}


void CCOF2OMF::MakeLEDATA() {
/* 
This function makes both LEDATA records, containing binary data, and FIXUPP
records, containing relocations.

The function is quite complicated because the LEDATA and FIXUPP records are 
mutually interdependent. Some explanation is in place here.

I am using the word segment for the collection of all sections in the old file
having the same name. A section is a record of binary data in the old file.
Each section is stored as one or more LEDATA records in the new file.
A segment may thus be split into multiple sections, which again may be split
into multiple LEDATA records. The sections must be aligned according to the
specified alignment for the segment. The LEDATA records need not be aligned,
and they may be misaligned for reasons explained below.

Each LEDATA record is followed by a FIXUPP record containing all relocations 
referring to a source in the LEDATA record, if any.

The size of a LEDATA record is limited to 1024 bytes because each entry in
the FIXUPP record has only 10 bits for addressing it. Some linkers impose
the 1024 bytes size limit to all OMF records, although this limitation is
strictly necessary only for LEDATA records. If the code has many relocations
then it may be necessary to make a LEDATA record smaller than 1024 bytes
in order to avoid that the corresponding FIXUPP record becomes bigger than
1024 bytes. Furthermore, the size of a LEDATA record may need adjustment to
avoid that a relocation source crosses a LEDATA record boundary.

I have stored all relocations in a temporary list RelocationBuffer which is
sorted by relocation source address in order to make it easier to find all
relocations with a source address in the current LEDATA record.
*/
   int    Segment;                               // Segment index in new file
   int    OldSection;                            // Section index in old file
   uint32 SegOffset;                             // Offset of section relative to segment
   uint32 SectOffset;                            // Offset of LEDATA record relative to section
   uint32 CutOff;                                // Size limit for splitting section into multiple LEDATA records
   uint32 RelFirst;                              // Index into RelocationBuffer of first relocation for section/record
   uint32 RelLast;                               // Index into RelocationBuffer of last relocation for record + 1
   uint32 Rel;                                   // Current index into RelocationBuffer
   uint32 FrameDatum;                            // Frame datum field in FIXUPP record
   uint32 TargetDatum;                           // Target datum field in FIXUPP record
   uint32 TargetDisplacement;                    // Target displacement field in FIXUPP record
   uint32 AlignmentFiller;                       // Number of unused bytes from end of one section to begin of next section due to alignment

   SOMFRelocation Reloc0;                        // Reference relocation record for compare and search
   OMF_SLocat Locat;                             // Locat bitfield for FIXUPP record
   OMF_SFixData FixData;                         // FixData bitfield for FIXUPP record

   // Loop through segment numbers
   for (Segment = 1; Segment <= NumSegments; Segment++) {
      SegOffset = 0;                             // SegOffset = 0 for first section in segment

      // Search through SectionBuffer for old sections contributing to this segment
      for (OldSection = 0; OldSection < SectionBufferNum; OldSection++) {

         if (SectionBuffer[OldSection].NewNumber == (uint32)Segment && SectionBuffer[OldSection].Size) {
            // This section contributes to Segment. Make LEDATA record(s)

            if (SectionBuffer[OldSection].Offset > SegOffset) {
               // Fillers needed for alignment after previous section
               AlignmentFiller = SectionBuffer[OldSection].Offset - SegOffset;
            }
            else {
               AlignmentFiller = 0;
            }
            SectOffset = 0;                      // Offset of LEDATA record relative to section start

            if (AlignmentFiller > 0 
            && AlignmentFiller < 4096 && AlignmentFiller < (1u << SectionBuffer[OldSection].Align)
            && SectionBuffer[OldSection].Class == OMF_LNAME_CODE) {
               // This is a code segment and there is a space from previous section
               // Fill the alignment space with NOP's
               // Make LIDATA record with NOP's
               ToFile.StartRecord(OMF_LIDATA);             // Start new LEDATA record
               ToFile.PutIndex(Segment);                   // Segment index 
               ToFile.PutNumeric(SegOffset);               // Offset of LIDATA relative to segment
               ToFile.PutNumeric(AlignmentFiller);         // Repeat count
               ToFile.PutWord(0);                          // Block count
               ToFile.PutByte(1);                          // Byte count
               ToFile.PutByte(0x90);                       // NOP opcode
               ToFile.EndRecord();                         // End LIDATA record
            }

            SegOffset = SectionBuffer[OldSection].Offset; // Offset of section to segment

            // Search for relocations for this section
            Reloc0.Section = OldSection;
            Reloc0.SourceOffset = 0;
            RelFirst = RelocationBuffer.FindFirst(Reloc0); // Points to first relocation for this section            
            RelLast = RelFirst;

            // Loop for possibly more than one LEDATA records for this section
            while (SectOffset < SectionBuffer[OldSection].Size) {

               CutOff = SectionBuffer[OldSection].Size - SectOffset; // Size of rest of section
               if (CutOff > 1024 && SectionBuffer[OldSection].Class != OMF_LNAME_BSS) {
                  CutOff = 1024; // Maximum LEDATA size
               }

               // Search for last relocation entry
               while (RelLast < NumRelocations) {
                  if (RelocationBuffer[RelLast].Section != (uint32)OldSection) {
                     break; // Reached end of relocations for this section
                  }
                  if (RelocationBuffer[RelLast].SourceOffset >= CutOff + SectOffset) {
                     break; // Reached size limit of LEDATA record
                  }
                  if (RelocationBuffer[RelLast].SourceOffset + 4 > CutOff + SectOffset) {
                     // Relocation crosses LEDATA boundary. 
                     // Reduce limit of LEDATA to before this relocation source
                     CutOff = RelocationBuffer[RelLast].SourceOffset - SectOffset;
                     if (CutOff == 0) {
                        err.submit(2302); // Relocation source extends beyond end of section. 
                        CutOff = 4;       // Prevent infinite loop
                     }
                     break;
                  }
                  if (RelLast - RelFirst > 100) {
                     // FIXUPP record will become too big. End LEDATA record here
                     CutOff = RelocationBuffer[RelLast].SourceOffset - SectOffset;
                     break;
                  }
                  RelLast++;
               } // End of search for last relocation entry for this LEDATA

               if (SectionBuffer[OldSection].Class == OMF_LNAME_BSS) {
                  // BSS: Unitialized data section needs no LEDATA record and no FIXUPP
                  if (RelLast > RelFirst) {
                     // Error: Relocation of uninitialized data
                     err.submit(2041);
                  }
               }
               else {
                  // Section contains initialized data. Needs LEDATA and FIXUPP records

                  // Make LEDATA record for section data
                  ToFile.StartRecord(OMF_LEDATA + 1);// Start new LEDATA record, 32 bit

                  // Segment index
                  ToFile.PutIndex(Segment);         

                  // Offset of LEDATA relative to segment
                  ToFile.PutNumeric(SegOffset + SectOffset);

                  // Binary data
                  ToFile.PutBinary(Buf() + SectionBuffer[OldSection].psechdr->PRawData + SectOffset, CutOff);

                  // End LEDATA record
                  ToFile.EndRecord();

                  if (RelLast > RelFirst) {      // If there are any relocations

                     // Make FIXUPP record with (RelLast-RelFirst) relocation entries
                     ToFile.StartRecord(OMF_FIXUPP + 1); // Start FIXUPP record, 32 bit

                     // Loop through relocations
                     for (Rel = RelFirst; Rel < RelLast; Rel++) {

                        if (RelocationBuffer[Rel].Mode < 0) {
                           // Unsupported mode. Make error message
                           err.submit(2030, RelocationBuffer[Rel].TargetOffset);   // TargetOffset contains COFF relocation mode
                           continue;
                        }

                        // Make Locat word bitfield 
                        Locat.s.one = 1;              // Indicates FIXUP subrecord
                        Locat.s.M = RelocationBuffer[Rel].Mode; // Direct / EIP-relative
                        Locat.s.Location = 9;              // Indicates 32-bit offset

                        // Offset of source relative to section (10 bits)
                        uint32 RelocOffset = RelocationBuffer[Rel].SourceOffset - SectOffset; // Offset of relocation source to begin of LEDATA record
                        if (RelocOffset >= 1024) err.submit(9000); // Check that it fits into 10 bits
                        Locat.s.Offset = RelocOffset;

                        // Make FixData byte bitfield
                        FixData.b = 0;              // Start with all bits 0

                        if (RelocationBuffer[Rel].Scope == S_LOCAL) {
                           // Local target
                           FixData.s.Target = 0;      // Target method T0 or T4: Target = segment
                           FixData.s.Frame = 1;       // Frame method F1: specified by a group index (FLAT)
                           FrameDatum = OMF_LNAME_FLAT; // Target frame = FLAT group
                           TargetDatum = RelocationBuffer[Rel].TargetSegment; // Fixup target = segment
                           TargetDisplacement = RelocationBuffer[Rel].TargetOffset; // Displacement or addend (?)
                        }
                        else {
                           // External symbol target
                           FixData.s.Frame = 5;       // Frame method F5: Frame specified by target, not here
                           FixData.s.Target = 2;      // Target method T2 or T6: Target specified by EXTDEF index
                           TargetDatum = RelocationBuffer[Rel].TargetSegment; // Index into EXTDEF
                           TargetDisplacement = RelocationBuffer[Rel].TargetOffset; // This is zero. Any addend is inline
                        }
                        if (TargetDisplacement) {
                           FixData.s.P = 0;           // Displacement field present
                        }
                        else {
                           FixData.s.P = 1;           // Displacement field absent
                        }

                        // Put these data into record
                        // Locat bytes in reverse order:
                        ToFile.PutByte(Locat.bytes[1]);  
                        ToFile.PutByte(Locat.bytes[0]);
                        // FixData byte
                        ToFile.PutByte(FixData.b);      
                        // Frame datum field only if FixData.F = 0 and Frame < 4
                        if (FixData.s.Frame < 4) ToFile.PutIndex(FrameDatum); 
                        // Target datum field if FixData.T = 0, which it is here
                        ToFile.PutIndex(TargetDatum);
                        // Target displacement field if FixData.P = 0
                        if (FixData.s.P == 0) ToFile.PutNumeric(TargetDisplacement);

                     } // End of loop through relocation for last LEDATA

                     // End FIXUPP record
                     ToFile.EndRecord();
                  }
               }
               // Update pointers to after this LEDATA
               SectOffset += CutOff;
               RelFirst = RelLast;

            } // End of loop for multiple LEDATA records for one section

            // Find end of section
            SegOffset += SectionBuffer[OldSection].Size;   // End of section

         } // End of if section in segment
      } // End of loop through multiple sections for same segment
   } // End of loop through segments
}


void CCOF2OMF::MakeMODEND() {
   // Make MODEND record and finish file
   ToFile.StartRecord(OMF_MODEND);               // Start MODEND record
   ToFile.PutByte(0);                            // Module type field. 0 if not a startup module with start address
   ToFile.EndRecord();                           // End MODEND record
}
