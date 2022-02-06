/****************************  omf2asm.cpp   *********************************
* Author:        Agner Fog, modified by Don Clugston
* Date created:  2007-05-27
* Last modified: 2014-05-32
* Project:       objconv
* Module:        omf2asm.cpp
* Description:
* Module for disassembling OMF object files
*
* (c) 2007-2014 GNU General Public License www.gnu.org/copyleft/gpl.html
*****************************************************************************/
#include "stdafx.h"


// Constructor
COMF2ASM::COMF2ASM() {
}


// Convert
void COMF2ASM::Convert() {
   // Do the conversion
   
   // Tell disassembler
   Disasm.Init(0, 0);

   // Make temporary Segments table
   CountSegments();

   // Make external symbols in Disasm
   MakeExternalSymbolsTable();

   // Make public symbols in Disasm
   MakePublicSymbolsTable();

   // Make symbol table entries for communal symbols.
   MakeCommunalSymbolsTable();

   // Make Segment list and relocations list
   MakeSegmentList();

   // Make group definitions
   MakeGroupDefinitions();

   // Disassemble
   Disasm.Go();  

   // Take over output file from Disasm
   *this << Disasm.OutFile;
}

void COMF2ASM::CountSegments() {
   // Make temporary Segments table
   uint32 i;                                     // Record number
   uint32 NameIndex;                             // Name index
   uint32 ClassIndex;                            // Class name index
   SOMFSegment SegRecord;                        // Segment record

   // Define structure of attributes
   OMF_SAttrib Attributes;

   // Initialize temporary list of segments. Entry 0 is blank
   Segments.PushZero();
   
   // Search for SEGDEF records
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_SEGDEF) {
         // SEGDEF record
         Records[i].Index = 3;
         // Loop through entries in record. There should be only 1
         while (Records[i].Index < Records[i].End) {
            // Read segment attributes
            Attributes.b = Records[i].GetByte();
            if (Attributes.u.A == 0) {
               // Frame and Offset only included if A = 0
               Records[i].GetWord();             // Frame ignored
               SegRecord.Offset = Records[i].GetByte();
            }
            else SegRecord.Offset = 0;

            SegRecord.Size = Records[i].GetNumeric();
            NameIndex  = Records[i].GetIndex();
            ClassIndex = Records[i].GetIndex();  // Class index
            Records[i].GetIndex();               // Overlay index ignored
            SegRecord.NameO = GetLocalNameO(NameIndex);     // Segment name

            if (Attributes.u.B) {
               // Segment is big
               if (Attributes.u.P) {
                  // 32 bit segment. Big means 2^32 bytes!
                  err.submit(2306);
               }
               else {
                  // 16 bit segment. Big means 2^16 bytes
                  SegRecord.Size = 0x10000;
               }
            }

            // Get word size
            SegRecord.WordSize = Attributes.u.P ? 32 : 16;

            // Get alignment
            switch (Attributes.u.A) {
            case 0:  // Absolute segment
            case 1:  // Byte alignment
               SegRecord.Align = 0;
               break;

            case 2:  // Word alignment
               SegRecord.Align = 1;
               break;

            case 3:  // Paragraph alignment
               SegRecord.Align = 4;
               break;

            case 4:  // Page alignment
               SegRecord.Align = 16;
               break;

            case 5:  // DWord alignment
               SegRecord.Align = 2;
               break;

            default: // Unknown
               SegRecord.Align = 3; // Arbitrary value
               break;
            }

            // Get further attributes from class name
            char * ClassName = GetLocalName(ClassIndex);

            // Convert class name to upper case
            uint32 n = (uint32)strlen(ClassName);
            for (uint32 j = 0; j < n; j++) ClassName[j] &= ~0x20;

            // Search for known class names.
            // Standard names are CODE, DATA, BSS, CONST, STACK
            if (strstr(ClassName, "CODE") || strstr(ClassName, "TEXT")) {
               // Code segment
               SegRecord.Type = 1;
            }
            else if (strstr(ClassName, "DATA")) {
               // Data segment
               SegRecord.Type = 2;
            }
            else if (strstr(ClassName, "BSS")) {
               // Unitialized data segment
               SegRecord.Type = 3;
            }
            else if (strstr(ClassName, "CONST")) {
               // Constant data segment
               SegRecord.Type = 4;
            }
            else if (strstr(ClassName, "STACK")) {
               // Stack segment.
               SegRecord.Type = 0;
            }
            else {
               // Unknown/user defined class. Assume data segment
               SegRecord.Type = 2;
            }

            // Store temporary segment record
            Segments.Push(SegRecord);
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }

   FirstComDatSection = Segments.GetNumEntries();
   // Communal sections (as used by Digital Mars):
   // This part by Don Clugston
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_COMDAT) {
         Records[i].Index = 3;

         uint8 flags = Records[i].GetByte();
         if ((flags & 2) != 0) {
            // don't support iterated data yet
            err.submit(2318);           // Error message: not supported
            continue;
         }
         uint8 attribs = Records[i].GetByte();
         uint8 align = Records[i].GetByte();
         uint32 ofs  = Records[i].GetNumeric();
         Records[i].GetIndex(); // type (ignore)
         //uint16 publicBase = 0;
         uint16 publicSegment = 0;
         // From the OMF Spec 1.1: "If alloc type is EXPLICIT, public base is present and is
         // identical to public base fields BaseGroup, Base Segment & BaseFrame in the PUBDEF."
         // BUT: In the diagram in the spec it is described as 1-2 bytes (ie, an Index field).
         // but in PUBDEF, those fields are Index, Index, or Index, zero, Index. (2-5 bytes)
         // The diagram appears to be erroneous.
         if ((attribs & 0xF) == 0){
            //publicBase = Records[i].GetIndex();
            publicSegment = Records[i].GetIndex();
            if (publicSegment == 0) {
                //Records[i].GetIndex(); // skip frame in this case
                // I don't have the Digital Mars obj spec, but this seems to help ??
                publicSegment = Records[i].GetIndex(); // ??
            }
         }
         uint16 publicName = Records[i].GetIndex();
         uint32 RecSize = Records[i].End - Records[i].Index; // Calculate size of data
         if (attribs & 0xF) {
            SegRecord.Type = 0x1000  | (attribs & 0xFF);
            SegRecord.WordSize = (attribs & 0x2) ? 32 : 16;
         }
         else {			 
            // use value from segdef
            SegRecord.Type = 0x1000 | Segments[publicSegment].Type;
            SegRecord.WordSize = Segments[publicSegment].WordSize;
         }

         //SegRecord.Type |= 1;//!!

         if (align != 0) {
            // alignment: (none), byte, word, paragraph, page, dword, arbitrary, arbitrary.
            static const int alignvalues[] = {0, 0, 1, 4, 16, 2, 3, 3};
            SegRecord.Align = alignvalues[align & 0x7];
         }
         else { // use value from segdef
            SegRecord.Align = Segments[publicSegment].Align;
         }
         SegRecord.Size = RecSize;

         // Get function name
         const char * name = GetLocalName(publicName);

         // Make a section name by putting _text$ before function name
         uint32 ComdatSectionNameIndex = NameBuffer.Push("_text$", 6);
         NameBuffer.PushString(name); // append function name
         SegRecord.NameO = ComdatSectionNameIndex;
         SegRecord.NameIndex = publicName;

         if (flags & 1) {
            // continuation.
            // Add to the length to the previous entry.
            Segments[Segments.GetNumEntries()-1].Size += RecSize;
         } 
         else {
            SegRecord.Offset = ofs;
            Segments.Push(SegRecord);
         }
      }
   }

   // Communal sections (as used by Borland):
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_COMDEF) {
         uint32 DType, DSize = 0, DNum;
         uint16 Segment = 0;
         const char * FuncName = 0;

         // Loop through possibly multiple entries in record
         while (Records[i].Index < Records[i].End) {
            // Get function name
            FuncName = Records[i].GetString();
            Records[i].GetByte(); // Type index, should be 0, ignored
            DType = Records[i].GetByte(); // Data type
            switch (DType) {
            case 0x61:
               DNum  = Records[i].GetLength();
               DSize = Records[i].GetLength() * DNum;
               break;
            case 0x62:
               DSize = Records[i].GetLength();
               break;
            default:
               DSize = Records[i].GetLength();
               if (DType < 0x60) { // Borland segment index
                  Segment = DType;
                  break;
               }
               err.submit(2016); // unknown type
               break;
            }
         }
         if (Segment >= Segments.GetNumEntries()) {err.submit(2016); return;}

         // Copy segment record
         SegRecord = Segments[Segment];

         // Make a section name as SEGMENTNAME$FUNCTIONNAME
         const char * SegmentName = NameBuffer.Buf() + SegRecord.NameO;
         uint32 ComdatSectionNameIndex = NameBuffer.Push(SegmentName, strlen(SegmentName));
         NameBuffer.Push("$", 1);
         NameBuffer.PushString(FuncName); // append function name
         SegRecord.NameO = ComdatSectionNameIndex;
         SegRecord.Size = DSize;
         SegRecord.Type |= 0x1000;
         //SegRecord.BufOffset = ??

         // Store segment
         Segments.Push(SegRecord);

         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
   // Number of segments, not including blank zero entry
   NumSegments = Segments.GetNumEntries() - 1;
}


void COMF2ASM::MakeExternalSymbolsTable() {
   // Make symbol table and string table entries for external symbols
   uint32 iextsym;                               // External symbol index
   uint32 isymo;                                 // Symbol index in disassembler
   uint32 NumExtSym = SymbolNameOffset.GetNumEntries(); // Number of external symbols
   ExtdefTranslation.SetNum(NumExtSym+1);        // Allocate space in symbol index translation table

   // Loop through external symbol names
   for (iextsym = 1; iextsym < NumExtSym; iextsym++) {

      // Get name
      const char * Name = GetSymbolName(iextsym);

      // Define symbol
      isymo = Disasm.AddSymbol(0, 0, 0, 0, 0x20, 0, Name);

      // Update table for translating old EXTDEF number to disassembler symbol index
      ExtdefTranslation[iextsym] = isymo;
   }
}


void COMF2ASM::MakePublicSymbolsTable() {
   // Make symbol table entries for public symbols
   uint32 i;                                     // Record index
   char * string;                                // Symbol name
   uint32 Segment;                               // Segment
   uint32 Offset;                                // Offset
   uint32 isymo;                                 // Symbol number in disasm
   uint32 CommunalSection = FirstComDatSection;  // Index to communal section

   PubdefTranslation.Push(0);                    // Make index 0 = 0

   // Search for PUBDEF records
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_PUBDEF) {
         // PUBDEF record

         Records[i].Index = 3;
         Records[i].GetIndex();                  // Group. Ignore
         Segment = Records[i].GetIndex();        // Segment
         if (Segment == 0) Records[i].GetWord(); // Base frame. Ignore

         // Loop through strings in record
         while (Records[i].Index < Records[i].End) {
            string = Records[i].GetString();     // Symbol name
            Offset = Records[i].GetNumeric();    // Offset to segment
            Records[i].GetIndex();               // Type index. Ignore

            // Define symbol
            isymo = Disasm.AddSymbol(Segment, Offset, 0, 0, 4, 0, string);

            // Update table for translating old PUBDEF number to disassembler symbol index
            PubdefTranslation.Push(isymo);
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }

   // Search for OMF_COMDEF records
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_COMDEF) {
         // COMDEF record, Borland communal name
         uint32 DType;
         //uint32 DSize;
         //uint32 DNum;
         Records[i].Index = 3;

         // Loop through possibly multiple entries in record
         while (Records[i].Index < Records[i].End) {
            string = Records[i].GetString();
            Records[i].GetByte(); // Type index, should be 0, ignore
            DType = Records[i].GetByte(); // Data type            
            switch (DType) {
            case 0x61:
               //DNum  = Records[i].GetLength();
               //DSize = Records[i].GetLength();
               continue; // Don't know what to do with this type. Ignore
            case 0x62:
               //DSize = Records[i].GetLength();
               continue; // Don't know what to do with this type. Ignore
            default:
               //DSize = Records[i].GetLength();
               if (DType < 0x60) { // Borland segment index
                  break;
               }
               continue; // Unknown type. Ignore
            }
            // Define symbol
            Segment = CommunalSection;
            isymo = Disasm.AddSymbol(Segment, 0, 0, 0, 0x10, 0, string);

            // Update table for translating old PUBDEF number to disassembler symbol index
            PubdefTranslation.Push(isymo);
         }
         CommunalSection++;

         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
}


void COMF2ASM::MakeCommunalSymbolsTable() {
   // Make symbol table entries for communal symbols
   char * string;                                // Symbol name

   // Search for communal records
   for (uint32 i = 0; i < NumRecords; i++) {
      // Count communal records
      if (Records[i].Type2 == OMF_CEXTDEF) {
         Records[i].Index = 3;
         // Loop through strings in record
         while (Records[i].Index < Records[i].End) {			 
            uint32 LIndex = Records[i].GetIndex();
            Records[i].GetIndex(); // Group. Ignore
            string = GetLocalName(LIndex);

            // find section with same name
            int32 section = 0;
            for (uint32 j = 0; j < Segments.GetNumEntries(); j++) {
               if (Segments[j].NameIndex == LIndex) {
                  section = (int32)j; break;
               }
            }

            // Define symbol
            Disasm.AddSymbol(section, 0, 0, 0, 0x10, 0, string);
         }
      }
   }
}


void COMF2ASM::MakeGroupDefinitions() {
   // Make segment group definitions
   uint32 i;                                     // Record index

   // Search for group records
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_GRPDEF) {
         // GRPDEF record
         Records[i].Index = 3;
         // Get group name
         uint32 ClassIndex = Records[i].GetIndex();
         char * GroupName = GetLocalName(ClassIndex);

         // Define group
         Disasm.AddSectionGroup(GroupName, 0);

         // Loop through remaining entries in record
         while (Records[i].Index < Records[i].End) {
            // Entry type should be 0xFF
            uint8 Type = Records[i].GetByte();
            // Get member name
            int32 NameIndex = Records[i].GetIndex();
            // Check if type valid
            if (Type == 0xFF && NameIndex > 0) {
               // A group member is found. Add member to group
               Disasm.AddSectionGroup(GroupName, NameIndex);
            }
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
}


// MakeSegmentList
void COMF2ASM::MakeSegmentList() {
   // Make Sections list in Disasm
   int32  SegNum;                                // Segment number
   int32  Segment = 0;                           // Segment number in OMF record
   uint32 RecNum;                                // OMF record number
   uint32 LastDataRecord;                        // OMF record number of last LEDATA record
   uint32 RecOffset;                             // Segment offset of LEDATA, LIDATA record
   uint32 RecSize;                               // Data size of LEDATA, LIDATA record
   uint32 LastDataRecordSize;                    // Last RecSize
   uint32 LastOffset;                            // Last RecOffset
   int8 * LastDataRecordPointer;                 // Point to last raw data
   uint32 BufOffset;                             // Offset of segment into SegmentData buffer
   CMemoryBuffer TempBuf;                        // Temporary buffer for building raw data

   // Loop through segments
   for (SegNum = 1; SegNum <= NumSegments; SegNum++) {

      // Get size
      uint32 SegmentSize = Segments[SegNum].Size;
      if (SegmentSize == 0) continue;            // Empty segment

      // Allocate temporary data buffer and reset it
      TempBuf.SetSize(SegmentSize + 16);
      int FillByte = 0;                          // Byte to fill memory with
      if (Segments[SegNum].Type == 1) {
         // Code segment. Fill any unused bytes with NOP opcode = 0x90
         FillByte = 0x90;
      }
      memset(TempBuf.Buf(), FillByte, SegmentSize + 16);// Reset to all 0 or NOP

      LastDataRecord = 0;
      LastDataRecordSize = 0;
      LastDataRecordPointer = 0;
      LastOffset = 0;
      int comdatsSoFar = 0;

      // Search for LEDATA, LIDATA and FIXUPP records for this segment
      for (RecNum = 0; RecNum < NumRecords; RecNum++) {

         if (Records[RecNum].Type2 == OMF_LEDATA) {

            // LEDATA record
            Records[RecNum].Index = 3;           // Initialize record reading
            Segment = Records[RecNum].GetIndex();// Read segment number

            if ((Segment & 0xC000) == 0x4000) {
               // Refers to Borland communal section
               Segment = (Segment & ~0x4000) + FirstComDatSection - 1;
            }

            if (Segment != SegNum) continue; // Does not refer to this segment

            RecOffset = Records[RecNum].GetNumeric();// Read offset of this record
            RecSize = Records[RecNum].End - Records[RecNum].Index; // Calculate size of data
            LastDataRecord = RecNum;             // Save for later FIXUPP that refers to this record

            if (RecOffset < LastOffset + LastDataRecordSize && LastOffset < RecOffset + RecSize) {
               // Overlapping data records
               if (RecOffset + 8 < LastOffset + LastDataRecordSize || Segments[SegNum].Type != 1) {
                  // Overlapping data by more than 7 bytes or not executable code
                  err.submit(1207);
               }
               else {
                  // Possibly backpatched code
                  err.submit(1208);              // Warning
                  err.ClearError(1208);          // Report only once
               }
            }

            LastDataRecordSize = RecSize;
            LastDataRecordPointer = Records[RecNum].buffer + Records[RecNum].FileOffset + Records[RecNum].Index;
            LastOffset = RecOffset;                 // Save offset for subsequent FIXUPP records

            // Check if data within segment
            if (RecOffset + RecSize > SegmentSize) {
               err.submit(2309, GetSegmentName(Segment));
               continue;
            }

            // Put raw data into temporary buffer
            memcpy(TempBuf.Buf() + RecOffset, LastDataRecordPointer, RecSize);

         } // Finished with LEDATA record

         if (Records[RecNum].Type2 == OMF_LIDATA) {
            // LIDATA record
            Records[RecNum].Index = 3;           // Initialize record reading
            Segment = Records[RecNum].GetIndex();

            if (Segment != SegNum) continue; // Does not refer to this segment

            LastDataRecord = RecNum;             // Save for later FIXUPP that refers to this record

            RecOffset = Records[RecNum].GetNumeric();// Read offset

            if (RecOffset > SegmentSize) {
               err.submit(2310); continue;       // Error: outside bounds
            }

            // Unpack LIDATA blocks recursively
            RecSize = Records[RecNum].UnpackLIDATABlock(TempBuf.Buf() + RecOffset, SegmentSize - RecOffset);

            if (RecOffset < LastOffset + LastDataRecordSize && LastOffset < RecOffset + RecSize) {
               // Overlapping data records
               err.submit(1207);                 // Warning
            }
            LastDataRecordSize = RecSize;           // Save data size
            LastOffset = RecOffset;                 // Save offset for subsequent FIXUPP records

         } // Finished with LIDATA record

         if (Records[RecNum].Type2 == OMF_COMDAT) {
            // COMDAT record.

            Records[RecNum].Index = 3;           // Initialize record reading
            uint16 flags = Records[RecNum].GetByte();
            if ((flags&1)==0) { // not a continuation
               ++comdatsSoFar;
               LastDataRecord = RecNum;             // Save for later FIXUPP that refers to this record
            }
            Segment = FirstComDatSection + comdatsSoFar-1;
            if (SegNum != Segment) continue;

            uint16 attribs = Records[RecNum].GetByte();
            Records[RecNum].GetByte(); // align (ignore)
            RecOffset = Records[RecNum].GetNumeric();
            Records[RecNum].GetIndex(); // type (ignore)
            if ((attribs&0xF)==0) {
               Records[RecNum].GetIndex(); // public base
               uint16 publicSegment = Records[RecNum].GetIndex();
               if (publicSegment==0) Records[RecNum].GetIndex(); // public frame (ignore)
            }
            Records[RecNum].GetIndex(); // public name (ignore)
            RecSize = Records[RecNum].End - Records[RecNum].Index; // Calculate size of data

            LastDataRecord = RecNum;             // Save for later FIXUPP that refers to this record
            LastDataRecordSize = RecSize;
            LastDataRecordPointer = Records[RecNum].buffer + Records[RecNum].Index+Records[RecNum].FileOffset;
            LastOffset = RecOffset;
            // Put raw data into temporary buffer
            memcpy(TempBuf.Buf() + RecOffset, LastDataRecordPointer, RecSize);
         } // Finished with COMDAT record

         if (Records[RecNum].Type2 == OMF_FIXUPP) {
            // FIXUPP record
            if (Segment != SegNum) continue; // Does not refer to this segment
            Records[RecNum].Index = 3;

            if (Records[LastDataRecord].Type2 == OMF_LEDATA) {
               // FIXUPP for last LEDATA record
               // Make relocation records
               MakeRelocations(Segment, RecNum, LastOffset, LastDataRecordSize, (uint8*)TempBuf.Buf());
            } 
            else if (Records[RecNum].Index < Records[RecNum].End) {
               // Non-empty FIXUPP record does not refer to LEDATA record
               if (Records[LastDataRecord].Type2 == OMF_COMDAT) {
                  // FIXUPP for last COMDAT record
                  // Make relocation records
                  MakeRelocations(Segment, RecNum, LastOffset, LastDataRecordSize, (uint8*)TempBuf.Buf());
               }
               else if (Records[LastDataRecord].Type2 == OMF_LIDATA) {
                  err.submit(2311);              // Error: Relocation of iterated data not supported
               }
               else {
                  err.submit(2312);              // Does not refer to data record
               }
            }
         }
      } // End of loop to search for LEDATA, LIDATA and FIXUPP records for this segment

      // Transfer raw data from TempBuf to SegmentData buffer
      BufOffset = SegmentData.Push(TempBuf.Buf(), SegmentSize);

      // Remember offset into SegmentData
      Segments[SegNum].BufOffset = BufOffset;

   } // End of first loop through segments

   // We must put all segments into SegmentData buffer before we assign pointers to
   // the raw data because otherwise the SegmentData buffer might me reallocated
   // when it grows and the pointers become invalid. This is the reasons why we
   // have two loops through the segments here.

   // Second loop through segments
   int totalcodesize=0;
   for (SegNum = 1; SegNum <= NumSegments; SegNum++) {

      // Pointer to merged raw data
      uint8 * RawDatap = (uint8*)SegmentData.Buf() + Segments[SegNum].BufOffset;

      // Size of raw data
      uint32 InitSize = (Segments[SegNum].Type == 3) ? 0 : Segments[SegNum].Size;

      // Define segment
      const char * SegmentName = NameBuffer.Buf() + Segments[SegNum].NameO;
      Disasm.AddSection(RawDatap, InitSize, Segments[SegNum].Size, Segments[SegNum].Offset,
         Segments[SegNum].Type, Segments[SegNum].Align, Segments[SegNum].WordSize, SegmentName);
      if (Segments[SegNum].Type == 1 || Segments[SegNum].Type == 0x1001) {
         totalcodesize += Segments[SegNum].Size;
      }
   }
}


// MakeRelocations
void COMF2ASM::MakeRelocations(int32 Segment, uint32 RecNum, uint32 SOffset, uint32 RSize, uint8 * SData) {
   // Make relocations for object and executable files
   // Parameters:
   // Segment = segment index of last LEDATA record
   // RecNum = FIXUPP record number
   // SOffset = segment relative offset of last LEDATA record
   // RSize = Size of last LEDATA record
   // SData = pointer to raw segment data

   uint32 Frame, Target, TargetDisplacement; // Contents of FIXUPP record
   uint8  byte1, byte2;                // First two bytes of subrecord
   int32  Inline;                      // Inline address or addend in relocation source
   //int16  InlineSeg;                   // Segment address stored in relocation source
   int32  Addend;                      // Correction to add to target address
   int32  SourceSize;                  // Size of relocation source
   uint32 RelType;                     // Relocation type, as defined in disasm.h
   int32  TargetSegment;               // Target segment or group
   uint32 TargetOffset;                // Target offset
   uint32 TargetSymbol;                // Symbol index of target
   uint32 ReferenceIndex;              // Segment/group index of reference frame

   // Bitfields in subrecords
   OMF_SLocat Locat;         // Structure of first two bytes of FIXUP subrecord swapped = Locat field
   OMF_SFixData FixData;     // Structure of FixData field in FIXUP subrecord of FIXUPP record
   OMF_STrdDat TrdDat;       // Structure of Thread Data field in THREAD subrecord of FIXUPP record

   Records[RecNum].Index = 3;

   // Loop through entries in record
   while (Records[RecNum].Index < Records[RecNum].End) {

      // Read first byte
      byte1 = Records[RecNum].GetByte();
      if (byte1 & 0x80) {

         // This is a FIXUP subrecord
         Frame = 0; Target = 0; TargetDisplacement = 0;  Addend = 0;  ReferenceIndex = 0;

         // read second byte
         byte2 = Records[RecNum].GetByte();
         // swap bytes and put into byte12 bitfield
         Locat.bytes[1] = byte1;
         Locat.bytes[0] = byte2;

         // Read FixData
         FixData.b = Records[RecNum].GetByte();

         // Read conditional fields
         if (FixData.s.F) {
            // Frame specified by previously define thread
            // Does anybody still use compression of repeated fixup targets?
            // I don't care to support this if it is never used
            err.submit(2313);           // Error message: not supported
            continue;
         }
         else {
            if (FixData.s.Frame < 4) {
               // Frame datum field present
               Frame = Records[RecNum].GetIndex();
            }
            else Frame = 0;

            switch (FixData.s.Frame) { // Frame method
            case 0:  // F0: segment
               ReferenceIndex = Frame;
               break;

            case 1:  // F1: group
               // Groups defined after segments. Add number of segments to get group index
               ReferenceIndex = Frame + NumSegments;
               break;

            default:
            case 2:  // F2: external symbol
               ReferenceIndex = 0;
               break;

            case 4:  // F4: traget frame = source frame
               Frame = Segment;
               break;

            case 5:  // F5: target frame = target segment
               Frame = 0;
               break;
            }
         }

         if (FixData.s.T == 0) {
            // Target specified
            Target = Records[RecNum].GetIndex();
            if ((Target & 0xC000) == 0x4000) {
               // Refers to Borland communal section
               Target = (Target & ~0x4000) + FirstComDatSection - 1;
            }
            //uint32 TargetMethod = FixData.s.Target + FixData.s.P * 4;
         }
         else {
            // Target specified in previous thread
            // Does anybody still use compression of repeated fixup targets?
            // I don't care to support this if it is never used
            err.submit(2313);           // Error message: not supported
            continue;
         }

         if (FixData.s.P == 0) {
            TargetDisplacement = Records[RecNum].GetNumeric();
         }

         if (!SData || Locat.s.Offset > RSize) {
            err.submit(2032); // Relocation points outside segment
            return;
         }
         // Get inline addend and check relocation method

         // Pointer to relocation source inline in raw data:
         uint8 * inlinep = SData + SOffset + Locat.s.Offset;
         Inline = 0;  SourceSize = 0; 
         //InlineSeg = 0;  
         TargetSegment = 0;  TargetOffset = 0;  TargetSymbol = 0;

         // Relocation type
         if (Locat.s.M) {
            // Segment relative
            RelType = 8;
         }
         else {
            // (E)IP relative
            RelType = 2;
         }

         switch (Locat.s.Location) {// Relocation method
         case OMF_Fixup_8bit:       // 8 bit
            SourceSize = 1;
            Inline = *(int8*)inlinep;
            break;

         case OMF_Fixup_16bit:      // 16 bit 
            SourceSize = 2;
            Inline = *(int16*)inlinep;
            break;

         case OMF_Fixup_32bit:      // 32 bit
            SourceSize = 4;
            Inline = *(int32*)inlinep;
            break;

         case OMF_Fixup_Far:        // far 16+16 bit
            RelType = 0x400;
            SourceSize = 4;
            Inline = *(int16*)inlinep;
            break;

         case OMF_Fixup_Farword:    // far 32+16 bit
         case OMF_Fixup_Pharlab48:
            RelType = 0x400;
            SourceSize = 6;
            Inline = *(int32*)inlinep;
            break;

         case OMF_Fixup_Segment:    // segment selector
            if (TargetDisplacement || FixData.s.Target == 2) {
               // An offset is specified or an external symbol.
               // Segment of symbol is required (seg xxx)
               RelType = 0x200;
            }
            else {
               // A segment name or group name is required
               RelType = 0x100;
            };
            SourceSize = 2;
            Inline = *(int16*)inlinep;
            break;

         case OMF_Fixup_16bitLoader: // 16-bit loader resolved
            RelType = 0x21;
            SourceSize = 2;
            Inline = *(int16*)inlinep;
            break;

         case OMF_Fixup_32bitLoader: // 32-bit loader resolved
            RelType = 0x21;
            SourceSize = 4;
            Inline = *(int32*)inlinep;
            break;

         default:                   // unknown or not supported
            RelType = 0;
            SourceSize = 0;
            Inline = 0;
         } // end switch


         // Offset of relocation source
         uint32 SourceOffset = SOffset + Locat.s.Offset;

         // Relocation type: direct or (E)IP-relative
         if (RelType == 2) {
            // (E)IP-relative
            // Correct for difference between source address and end of instruction
            Addend = -SourceSize;
         }

         // Check target method
         switch (FixData.s.Target) {             // = Target method modulo 4
         case 0: // T0 and T4: Target = segment
            // Local or public symbol
            TargetSegment = Target;              // Target segment
            TargetOffset = TargetDisplacement;   // Target offset 
            if (RelType != 0x100) {
               // Add inline to target address, except if target is a segment only
               TargetOffset += Inline;
               Addend -= Inline;                 // Avoid adding Inline twice
            }
            break;

         case 1: // T1 and T5: Target = segment group
            // Warning: this method has not occurred. Not tested!
            // Groups are numbered in sequence after segments in Disasm. Add number of segments to group index
            TargetSegment = Target + NumSegments;// Target group
            TargetOffset = TargetDisplacement;   // Target offset 
            if (RelType != 0x100) {
               // Add inline to target address, except if target is a segment only
               TargetOffset += Inline;
               Addend -= Inline;                 // Avoid adding Inline twice
            }
            break;

         case 2: // T2 and T6: Target = external symbol
            // Translate old EXTDEF index to new symbol table index
            if (Target < ExtdefTranslation.GetNumEntries()) {
               TargetSymbol = ExtdefTranslation[Target];
            }
            break;

         default: // Unknown method
            err.submit(2314, FixData.s.Target + FixData.s.P * 4);
         }

         if (TargetSymbol == 0) {
            // Make symbol record for target
            TargetSymbol = Disasm.AddSymbol(TargetSegment, TargetOffset, 0, 0, 2, 0, 0);
         }

         if (FixData.s.Frame == 4 && FixData.s.Target + FixData.s.P*4 == 6) {
            // Note:
            // Frame method F4 is apparently used by 16-bit Borland compiler for
            // indicating floating point instructions that can be emulated if no
            // 8087 processor is present. I can't find this documented anywhere.
            // I don't know what the exact criterion is for indicating that a FIXUP
            // subrecord is not a relocation record but a f.p. emulating record.
            // I have chosen to consider all subrecords with frame method F4 and 
            // target method T6 to be ignored.
            ;
         }
         else {
            // This is a proper relocation subrecord
            Disasm.AddRelocation(Segment, SourceOffset, Addend, RelType, SourceSize, TargetSymbol, ReferenceIndex); 
         }
      }
      else {
         // This is a THREAD subrecord.
         // I don't think this feature for compressing fixup data is
         // used any more, if it ever was. I am not supporting it here.
         // Frame threads can be safely ignored. A target thread cannot
         // be ignored if there is any reference to it. The error is 
         // reported above at the reference to a target thread, not here.
         TrdDat.b = byte1;              // Put byte into bitfield
         if (TrdDat.s.Method < 4) {     // Make sure we read this correctly, even if ignored
            Records[RecNum].GetIndex(); // has index field if method < 4 ?
         }
      }
   } // Finished loop through subrecords

   if (Records[RecNum].Index != Records[RecNum].End) err.submit(1203);   // Check for consistency
}
