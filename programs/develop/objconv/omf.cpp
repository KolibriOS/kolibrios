/****************************    omf.cpp    *********************************
* Author:        Agner Fog
* Date created:  2007-01-29
* Last modified: 2018-05-26
* Project:       objconv
* Module:        omf.cpp
* Description:
* Module for reading OMF files
*
* Class COMF is used for reading, interpreting and dumping OMF files.
*
* Copyright 2007-2018 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

// OMF Record type names
SIntTxt OMFRecordTypeNames[] = {
   {OMF_THEADR,      "Translator Header"},
   {OMF_LHEADR,      "Library Module Header"},
   {OMF_COMENT,      "Comment"},
   {OMF_MODEND,      "Module End"},
   {OMF_EXTDEF,      "External Names Definition"},
   {OMF_PUBDEF,      "Public Names Definition"},
   {OMF_LINNUM,      "Line Numbers"},
   {OMF_LNAMES,      "List of Names"},
   {OMF_SEGDEF,      "Segment Definition"},
   {OMF_GRPDEF,      "Group Definition"},
   {OMF_FIXUPP,      "Fixup"},
   {OMF_LEDATA,      "Enumerated Data"},
   {OMF_LIDATA,      "Iterated Data"},
   {OMF_COMDEF,      "Communal Names Definition"},
   {OMF_BAKPAT,      "Backpatch"},
   {OMF_LEXTDEF,     "Local External Names"},
   {OMF_LPUBDEF,     "Local Public Names"},
   {OMF_LCOMDEF,     "Local Communal Names"},
   {OMF_CEXTDEF,     "COMDAT External Names"},
   {OMF_COMDAT,      "Initialized Communal Data"},
   {OMF_LINSYM,      "Symbol Line Numbers"},
   {OMF_ALIAS,       "Alias Definition"},
   {OMF_NBKPAT,      "Named Backpatch"},
   {OMF_LLNAMES,     "Local Logical Names"},
   {OMF_VERNUM,      "OMF Version Number"},
   {OMF_VENDEXT,     "Vendor-specific OMF Extension"},
   {OMF_LIBHEAD,     "Library Header"},
   {OMF_LIBEND,      "Library End"}
};

// Segment combination names
SIntTxt OMFSegmentCombinationNames[] = {
   {0,     "Private"},
   {1,     "Invalid"},
   {2,     "Public"},
   {3,     "Invalid"},
   {4,     "Public 4"},
   {5,     "Stack"},
   {6,     "Common"},
   {7,     "Public 7"}
};

// Relocation mode names
static SIntTxt OMFRelocationModeNames[] = {
   {0,     "Relatv"},
   {1,     "Direct"}
};


// Fixup location names
static SIntTxt OMFFixupLocationNames[] = {
   {OMF_Fixup_8bit,        "8 bit"},                       // 0
   {OMF_Fixup_16bit,       "16 bit"},                      // 1
   {OMF_Fixup_Segment,     "segment selector, 16 bit"},    // 2
   {OMF_Fixup_Far,         "far pointer 16+16 bit"},       // 3
   {OMF_Fixup_Hi8bit,      "high 8 bit of 16 bits"},       // 4
   {OMF_Fixup_16bitLoader, "16 bit loader resolved"},      // 5
   {OMF_Fixup_Pharlab48,   "farword 48 bit, Pharlab only"},// 6
   {OMF_Fixup_32bit,       "32 bit"},                      // 9
   {OMF_Fixup_Farword,     "farword 32+16 bit"},           // 11
   {OMF_Fixup_32bitLoader, "32 bit loader resolved"}       // 13
};

// Alignment value translation table
static const uint32 OMFAlignTranslate[8] = {0,1,2,16,256,4,0,0};


// Class COMF members:
// Constructor

COMF::COMF() {
   // Default constructor
   memset(this, 0, sizeof(*this));     // reset everything
}


void COMF::ParseFile() {
   // Parse file buffer
   //uint8  RecordType;                            // Type of current record
   uint32 Checksum;                              // Record checksum
   uint32 ChecksumZero = 0;                      // Count number of records with zero checksum
   SOMFRecordPointer rec;                        // Current record pointer

   // Make first entry zero in name lists
   LocalNameOffset.PushZero();
   SegmentNameOffset.PushZero();
   GroupNameOffset.PushZero();
   SymbolNameOffset.PushZero();

   // Initialize record pointer
   rec.Start(Buf(), 0, GetDataSize());

   // Loop through records to set record pointers and store names
   do {
      // Read record
      //RecordType = rec.Type2;                    // First byte of record = type

      // Compute checksum
      Checksum = 0;  rec.Index = 0;
      while (rec.Index < rec.End) Checksum += rec.GetByte();
      uint32 CheckByte = rec.GetByte();
      if ((Checksum + CheckByte) & 0xFF) {
         // Checksum failed
         if (CheckByte == 0) {
            ChecksumZero++;
         }
         else err.submit(1202);                  // Checksum error
      }

      // Store record pointer
      rec.Index = 3;                             // Offset to current byte while parsing
      Records.Push(rec);                         // Store record pointer in list

      if (rec.Type2 == OMF_LNAMES) {
         // LNAMES record. Store local names by name index
         // Loop through strings in record
         while (rec.Index < rec.End) {
            char * LocalName = rec.GetString();
            uint32 LocalNameIndex = NameBuffer.PushString(LocalName); // Store local name
            LocalNameOffset.Push(LocalNameIndex);// Store local name index
         }
         if (rec.Index != rec.End) err.submit(1203);  // Check for consistency
      }

      if (rec.Type2 == OMF_SEGDEF) {
         // SEGDEF record. Store segment names by segment index
         OMF_SAttrib Attributes;
         if (rec.Type2 == OMF_SEGDEF) {
            Attributes.b = rec.GetByte();     // Read attributes
            if (Attributes.u.A == 0) {
               // Frame and Offset only included if A = 0
               rec.GetWord();  rec.GetByte();
            }
            rec.GetNumeric(); // Length
         }
         uint32 NameIndex  = rec.GetIndex();
         if (NameIndex < LocalNameOffset.GetNumEntries()) {
            SegmentNameOffset.Push(LocalNameOffset[NameIndex]); // List by segment index
         }
      }

      if (rec.Type2 == OMF_GRPDEF) {
         // GRPDEF record. Store group name
         uint32 NameIndex  = rec.GetIndex();
         if (NameIndex < LocalNameOffset.GetNumEntries()) {
            GroupNameOffset.Push(LocalNameOffset[NameIndex]); // List by group index
         }
      }

      if (rec.Type2 == OMF_EXTDEF) {
         // EXTDEF record. Store external symbol names
         // Loop through strings in record
         while (rec.Index < rec.End) {
            char * symbolname = rec.GetString();
            rec.GetIndex();
            uint32 SymbolNameIndex = NameBuffer.PushString(symbolname); // Store external name
            SymbolNameOffset.Push(SymbolNameIndex);   // Save in name index table
         }
         if (rec.Index != rec.End) err.submit(1203);  // Check for consistency
      }

      if (rec.Type2 == OMF_CEXTDEF) {
         // CEXTDEF record. Store communal symbol names
         // Loop through entries in record
         uint32 SymbolNameIndex;                 // Index into NameBuffer
         while (rec.Index < rec.End) {
            uint32 LIndex = rec.GetIndex();      // Index into preceding LNAMES
            rec.GetIndex();                      // Type index. Ignore
            // Get name from LocalNameOffset and put into SymbolNameOffset.
            if (LIndex < LocalNameOffset.GetNumEntries()) {
               SymbolNameIndex = LocalNameOffset[LIndex];
            }
            else SymbolNameIndex = 0;
            SymbolNameOffset.Push(SymbolNameIndex);   // Save in name index table
         }
         if (rec.Index != rec.End) err.submit(1203);  // Check for consistency
      }
   }  // Point to next record
   while (rec.GetNext());                        // End of loop through records

   NumRecords = Records.GetNumEntries();         // Number of records

   if (ChecksumZero) printf("\nChecksums are zero"); // This is taken out of the loop to report it only once
}


void COMF::Dump(int options) {
   // Dump file
   if (options & DUMP_FILEHDR) DumpRecordTypes(); // Dump summary of record types

   if (options & DUMP_STRINGTB) DumpNames(); // Dump names records

   if (options & DUMP_SYMTAB) DumpSymbols(); // Dump public/external name records   

   if (options & DUMP_SECTHDR) DumpSegments(); // Dump segment records

   if (options & DUMP_RELTAB) DumpRelocations(); // Dump fixup records

   if (options & DUMP_COMMENT) DumpComments(); // Dump coment records
}

void COMF::DumpRecordTypes() {
   // Dump summary of records
   printf("\nSummary of records:");
   for (uint32 i = 0; i < NumRecords; i++) {
      // Print record type
      printf("\n  Record %02X, %s%s, total length %i", Records[i].Type,
         Lookup(OMFRecordTypeNames, Records[i].Type2), 
         (Records[i].Type & 1) ? ".32" : "",
         Records[i].End+1);
   }
}


void COMF::DumpNames() {
   // Dump local names records
   uint32 i;           // Record index
   uint32 ln = 0;     // Local name index
   printf("\n\nLocal names:");
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_LNAMES) {
         // LNAMES record. There should be only one
         // Loop through strings in record
         while (Records[i].Index < Records[i].End) {
            printf("\n  %2i  %s", ++ln, Records[i].GetString());
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }

      if (Records[i].Type2 == OMF_THEADR || Records[i].Type2 == OMF_LHEADR) {
         // Module header record
         // Loop through strings in record
         while (Records[i].Index < Records[i].End) {
            printf("\n  Module: %s\n", Records[i].GetString());
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
      if (Records[i].Type2 == OMF_COMDEF) {
         // COMDEF record. Communal names
         uint32 DType, DSize, DNum;
         printf("\n\n Communal names:");

         // Loop through strings in record
         while (Records[i].Index < Records[i].End) {
            printf("\n  \"%s\":", Records[i].GetString());
            printf(" %i", Records[i].GetByte()); // Type index, should be 0
            DType = Records[i].GetByte(); // Data type            
            switch (DType) {
            case 0x61:
               DNum  = Records[i].GetLength();
               DSize = Records[i].GetLength();
               printf(" FAR: %i*%i bytes", DNum, DSize); 
               break;
            case 0x62:
               DSize = Records[i].GetLength();
               printf(" NEAR: 0x%X bytes", DSize); 
               break;
            default:
               DSize = Records[i].GetLength();
               if (DType < 0x60) { // Borland segment index
                  printf(" segment %i, size 0x%X", DType, DSize); 
                  break;
               }
               printf(" unknown type %i, size 0x%X", DType, DSize); 
               break;
            }
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
}

void COMF::DumpSymbols() {
   // Dump public, external and communal names records
   uint32 i;          // Record index
   uint32 xn = 0;     // External name index
   char * string;
   uint32 TypeIndex;
   uint32 Group;
   uint32 Segment;
   uint32 BaseFrame;
   uint32 Offset;

   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_EXTDEF) {
         // EXTDEF record.
         Records[i].Index = 3;
         printf("\n\nExternal names:");

         // Loop through strings in record
         while (Records[i].Index < Records[i].End) {
            string = Records[i].GetString();
            TypeIndex = Records[i].GetIndex();
            printf("\n  %2i  %s, Type %i", ++xn, string, TypeIndex);
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }

      if (Records[i].Type2 == OMF_PUBDEF) {
         // PUBDEF record.
         printf("\n\nPublic names:");
         Records[i].Index = 3;
         Group = Records[i].GetIndex();
         Segment = Records[i].GetIndex();
         BaseFrame = 0;
         if (Segment == 0) BaseFrame = Records[i].GetWord();
         // Loop through strings in record
         while (Records[i].Index < Records[i].End) {
            string = Records[i].GetString();
            Offset = Records[i].GetNumeric();
            TypeIndex = Records[i].GetIndex();
            printf("\n  %s, Segment %s, Group %s, Offset 0x%X, Type %i", 
               string, GetSegmentName(Segment), GetGroupName(Group), Offset, TypeIndex);
            if (BaseFrame) printf(", Frame %i", BaseFrame);
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }

      if (Records[i].Type2 == OMF_CEXTDEF) {
         // CEXTDEF record.
         printf("\n\nCommunal names:");
         Records[i].Index = 3;
         while (Records[i].Index < Records[i].End) {
            uint32 LIndex = Records[i].GetIndex();    // Index into preceding LNAMES
            uint32 Type = Records[i].GetIndex();      // Type index. Ignored
            printf("\n  %2i  %s, Type %i", ++xn, GetLocalName(LIndex), Type);
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
}


void COMF::DumpSegments() {
   // Dump all segment records

   // Define structure of attributes
   OMF_SAttrib Attributes;

   // Record values
   uint32 Frame, Offset, SegLength, NameIndex, ClassIndex, OverlayIndex;

   uint32 i;                                     // Record number
   uint32 SegNum = 0;                            // Segment number
   
   printf("\n\nSegment records:");
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_SEGDEF) {
         // SEGDEF record
         Records[i].Index = 3;
         // Loop through entries in record. There should be only 1
         while (Records[i].Index < Records[i].End) {
            Attributes.b = Records[i].GetByte();     // Read attributes
            if (Attributes.u.A == 0) {
               // Frame and Offset only included if A = 0
               Frame = Records[i].GetWord();  Offset = Records[i].GetByte();
            }
            else Frame = Offset = 0;
            SegLength    = Records[i].GetNumeric();
            NameIndex    = Records[i].GetIndex();
            ClassIndex   = Records[i].GetIndex();
            OverlayIndex = Records[i].GetIndex();

            printf("\n  Segment %2i, Name %s, Class %s, Align %i, %s, %i bit",
               ++SegNum, GetLocalName(NameIndex), GetLocalName(ClassIndex), 
               OMFAlignTranslate[Attributes.u.A], 
               Lookup(OMFSegmentCombinationNames, Attributes.u.C),
               Attributes.u.P ? 32 : 16);
            if (Attributes.u.B) printf(", big");
            if (Attributes.u.A == 0) printf(", Frame %i, Offset 0x%X", Frame, Offset);
            printf(", Length %i", SegLength);
            if (OverlayIndex) printf("\n   Overlay %i", OverlayIndex);
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
   printf("\n\nGroup records:");
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_GRPDEF) {
         // GRPDEF record
         Records[i].Index = 3;
         ClassIndex = Records[i].GetIndex();
         printf("\n  Group: %s\n   Segments:", GetLocalName(ClassIndex));

         // Loop through remaining entries in record
         while (Records[i].Index < Records[i].End) {
            uint8 Type = Records[i].GetByte();
            if (Type != 0xFF) printf(" Type=%X:", Type);
            NameIndex =  Records[i].GetIndex();
            printf(" %s", GetSegmentName(NameIndex));
         }
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
}


void COMF::DumpRelocations() {
   // Dump all LEDATA, LIDATA, COMDAT and FIXUPP records
   //uint32 LastDataRecord = 0;          // Index to the data record that relocations refer to
   uint32 LastDataRecordSize = 0;      // Size of the data record that relocations refer to
   int8 * LastDataRecordPointer = 0;   // Pointer to data in the data record that relocations refer to
   uint32 i;                           // Loop counter
   uint32 Segment, Offset, Size;       // Contents of LEDATA or LIDATA record
   uint32 LastOffset = 0;              // Offset of last LEDATA or LIDATA record
   uint32 Frame, Target, TargetDisplacement; // Contents of FIXUPP record
   uint8 byte1, byte2;                 // First two bytes of subrecord

   // Bitfields in subrecords
   OMF_SLocat Locat;         // Structure of first two bytes of FIXUP subrecord swapped = Locat field
   OMF_SFixData FixData;     // Structure of FixData field in FIXUP subrecord of FIXUPP record
   OMF_STrdDat TrdDat;       // Structure of Thread Data field in THREAD subrecord of FIXUPP record

   printf("\n\nLEDATA, LIDATA, COMDAT and FIXUPP records:");
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_LEDATA) {
         // LEDATA record
         Segment = Records[i].GetIndex();             // Read segment and offset
         Offset  = Records[i].GetNumeric();
         Size    = Records[i].End - Records[i].Index; // Calculate size of data
         //LastDataRecord = i;                          // Save for later FIXUPP that refers to this record
         LastDataRecordSize = Size;
         LastDataRecordPointer = Records[i].buffer + Records[i].FileOffset + Records[i].Index;
         if (Segment < 0x4000) {
            printf("\n  LEDATA: segment %s, Offset 0x%X, Size 0x%X", // Dump segment, offset, size
               GetSegmentName(Segment), Offset, Size);
            LastOffset = Offset;
         }
         else { // Undocumented Borland communal section
            printf("\n  LEDATA communal section %i, Offset 0x%X, Size 0x%X", // Dump segment, offset, size
               (Segment & ~0x4000), Offset, Size);
            LastOffset = Offset;
         }
      }

      if (Records[i].Type2 == OMF_LIDATA) {
         // LIDATA record
         Segment = Records[i].GetIndex();
         Offset  = Records[i].GetNumeric();
         //LastDataRecord = i;
         LastDataRecordSize = Records[i].End - Records[i].Index; // Size before expansion of repeat blocks
         LastDataRecordPointer = Records[i].buffer + Records[i].FileOffset + Records[i].Index;
         printf("\n  LIDATA: segment %s, Offset 0x%X, Size ",
            GetSegmentName(Segment), Offset);
         // Call recursive function to interpret repeat data block
         Size = Records[i].InterpretLIDATABlock();
         printf(" = 0x%X", Size);
         LastOffset = Offset;
      }

      if (Records[i].Type2 == OMF_COMDAT) {
         // COMDAT record
         //uint32 Flags = Records[i].GetByte(); // 1 = continuation, 2 = iterated, 4 = local, 8 = data in code segment
         uint32 Attributes = Records[i].GetByte(); 
         uint32 Base = 0;
         // 0 = explicit, 1 = far code, 2 = far data, 3 = code32, 4 = data32
         // 0x00 = no match, 0x10 = pick any, 0x20 = same size, 0x30 = exact match
         uint32 Align = Records[i].GetByte(); // Alignment
         Offset  = Records[i].GetNumeric();    // Offset
         uint32 TypeIndex = Records[i].GetIndex(); // Type index
         if ((Attributes & 0x0F) == 0) {
            Base = Records[i].GetIndex(); // Public base
         }
         uint32 NameIndex = Records[i].GetIndex(); // LNAMES index
         Size    = Records[i].End - Records[i].Index; // Calculate size of data

         printf("\n  COMDAT: name %s, Offset 0x%X, Size 0x%X, Attrib 0x%02X, Align %i, Type %i, Base %i",
            GetLocalName(NameIndex), Offset, Size, Attributes, Align, TypeIndex, Base);
         LastOffset = Offset;
      }

      if (Records[i].Type2 == OMF_FIXUPP) {
         // FIXUPP record
         printf("\n  FIXUPP:");
         Records[i].Index = 3;

         // Loop through entries in record
         while (Records[i].Index < Records[i].End) {

            // Read first byte
            byte1 = Records[i].GetByte();
            if (byte1 & 0x80) {
               // This is a FIXUP subrecord
               Frame = 0; Target = 0; TargetDisplacement = 0;

               // read second byte
               byte2 = Records[i].GetByte();
               // swap bytes and put into byte12 bitfield
               Locat.bytes[1] = byte1;
               Locat.bytes[0] = byte2;
               // Read FixData
               FixData.b = Records[i].GetByte();

               // print mode and location
               printf("\n   %s %s, Offset 0x%X", 
                  Lookup(OMFRelocationModeNames, Locat.s.M),
                  Lookup(OMFFixupLocationNames, Locat.s.Location),
                  Locat.s.Offset + LastOffset);

               // Read conditional fields
               if (FixData.s.F == 0) {
                  if (FixData.s.Frame < 4) {
                     Frame = Records[i].GetIndex();
                  }
                  else Frame = 0;

                  switch (FixData.s.Frame) { // Frame method
                  case 0:  // F0: segment
                     printf(", segment %s", GetSegmentName(Frame));  break;

                  case 1:  // F1: group
                     printf(", group %s", GetGroupName(Frame));  break;

                  case 2:  // F2: external symbol
                     printf(", external frame %s", GetSymbolName(Frame));  break;

                  case 4:  // F4: frame = source, 
                           // or Borland floating point emulation record (undocumented?)
                     printf(", frame = source; or Borland f.p. emulation record");
                     break;

                  case 5:  // F5: frame = target
                     printf(", frame = target");  break;

                  default:
                     printf(", target frame %i method F%i", Frame, FixData.s.Frame);
                  }
               }
               else {
                  printf(", frame uses thread %i", FixData.s.Frame);
               }
               
               if (FixData.s.T == 0) {
                  // Target specified
                  Target = Records[i].GetIndex();
                  uint32 TargetMethod = FixData.s.Target + FixData.s.P * 4;

                  switch (FixData.s.Target) { // = Target method modulo 4
                  case 0: // T0 and T4: Target = segment
                  case 1: // T1 and T5: Target = segment group
                     printf(". Segment %s (T%i)",
                        GetSegmentName(Target), TargetMethod);
                     break;
                  case 2: // T2 and T6: Target = external symbol
                     printf(". Symbol %s (T%i)",
                        GetSymbolName(Target), TargetMethod);
                     break;
                  default: // Unknown method
                     printf(", target %i unknown method T%i", Target, TargetMethod);
                  }
               }
               else {
                  // Target specified in previous thread
                  printf(", target uses thread %i", FixData.s.Target);
               }

               if (FixData.s.P == 0) {
                  TargetDisplacement = Records[i].GetNumeric();
                  printf("\n    target displacement %i", TargetDisplacement);
               }
               // Get inline addend
               if (LastDataRecordPointer && Locat.s.Offset < LastDataRecordSize) {
                  int8 * inlinep = LastDataRecordPointer + Locat.s.Offset;
                  switch (Locat.s.Location) {
                  case 0: case 4:  // 8 bit
                     printf(", inline 0x%X", *inlinep);  break;

                  case 1: case 2: case 5: // 16 bit
                     printf(", inline 0x%X", *(int16*)inlinep);  break;

                  case 3: // 16+16 bit
                     printf(", inline 0x%X:0x%X", *(int16*)(inlinep+2), *(int16*)inlinep);  break;

                  case 9: case 13: // 32 bit
                     printf(", inline 0x%X", *(int32*)inlinep);  break;

                  case 6: case 11: // 16+32 bit
                     printf(", inline 0x%X:0x%X", *(int16*)(inlinep+4), *(int32*)inlinep);  break;
                  }
               }
            }
            else {
               // This is a THREAD subrecord
               TrdDat.b = byte1;                 // Put byte into bitfield

               uint32 Index  = 0;
               if (TrdDat.s.Method < 4) {
                  Index  = Records[i].GetIndex(); // has index field if method < 4 ?
               }
               printf("\n   %s Thread %i. Method %s%i, index %i",
                  (TrdDat.s.D ? "Frame" : "Target"), TrdDat.s.Thread,
                  (TrdDat.s.D ? "F" : "T"), TrdDat.s.Method, Index);
            }
         } // Finished loop through subrecords
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   } // Finished loop through records
}


void COMF::DumpComments() {
   // Dump COMENT records
   uint32 i;           // Record index
   int startindex;
   printf("\n");
   for (i = 0; i < NumRecords; i++) {
      if (Records[i].Type2 == OMF_COMENT) {
         // COMENT record
         printf("\nCOMENT record:\n");
         startindex = Records[i].Index;
         // Print as hex
         while (Records[i].Index < Records[i].End) {
            printf("%02X ", Records[i].GetByte());
         }
         // Print again as string
         Records[i].Index = startindex;
         printf("\n");
         while (Records[i].Index < Records[i].End) {
            printf("%c ", Records[i].GetByte());
         }
         printf("\n");
         if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
      }
   }
}


void COMF::PublicNames(CMemoryBuffer * Strings, CSList<SStringEntry> * Index, int m) {
   // Make list of public names
   // Strings will receive ASCIIZ strings
   // Index will receive records of type SStringEntry with Member = m
   SOMFRecordPointer rec;                        // Current OMF record
   char * string;                                // Symbol name
   SStringEntry se;                              // String entry record to save

   // Initialize record pointer
   rec.Start(Buf(), 0, GetDataSize());

   // Loop through records and search for PUBDEF records
   do {
      // Read record
      if (rec.Type2 == OMF_PUBDEF) {

         // Public symbols definition found
         rec.GetIndex(); // Read group
         uint32 Segment = rec.GetIndex(); // Read segment
         if (Segment == 0) rec.GetWord(); // Read base frame
         // Loop through strings in record
         while (rec.Index < rec.End) {
            string = rec.GetString();  // Read name
            rec.GetNumeric(); // Read offset
            rec.GetIndex(); // Read type
            // Make SStringEntry record
            se.Member = m;
            // Store name
            se.String = Strings->PushString(string);
            // Store name index
            Index->Push(se);
         }
         if (rec.Index != rec.End) err.submit(1203);   // Check for consistency
      }
      if (rec.Type2 == OMF_CEXTDEF) {
         // CEXTDEF record. Store communal symbol names
         // Loop through entries in record
         while (rec.Index < rec.End) {
            uint32 LIndex = rec.GetIndex() - 1;  // Index into preceding LNAMES
            rec.GetIndex();                      // Type index. Ignore
            // Check if index valid
            if (LIndex < LocalNameOffset.GetNumEntries()) {
               // Make SStringEntry record
               se.Member = m;
               // Get name
               char * name = GetLocalName(LIndex);
               if (strlen(name) > 0) {
                  // Store name
                  se.String = Strings->PushString(name);
                  // Store name index
                  Index->Push(se);
               }
            }
         }
         if (rec.Index != rec.End) err.submit(1203);  // Check for consistency
      }
      if (rec.Type2 == OMF_LNAMES) {
         // LNAMES record. Check if file has been parsed
         if (Records.GetNumEntries() == 0) {
            // ParseFile has not been called. We need to store LNAMES table because
            // these names may be needed by subsequent EXTDEF records.
            // Loop through strings in record
            while (rec.Index < rec.End) {
               char * LocalName = rec.GetString();
               uint32 LocalNameIndex = NameBuffer.PushString(LocalName); // Store local name
               LocalNameOffset.Push(LocalNameIndex);// Store local name index
            }
            if (rec.Index != rec.End) err.submit(1203);  // Check for consistency
         }
      }
   } // Get next record
   while (rec.GetNext());                        // End of loop through records
}

char * COMF::GetLocalName(uint32 i) {
   // Get section name or class name by name index
    if (i == 0 || i >= LocalNameOffset.GetNumEntries())  {
        i = NameBuffer.PushString("null");
        return NameBuffer.Buf() + i;
    }
    return NameBuffer.Buf() + LocalNameOffset[i];
}

uint32 COMF::GetLocalNameO(uint32 i) {
   // Get section name or class by converting name index offset into NameBuffer
   if (i > 0 && i < LocalNameOffset.GetNumEntries()) {
      return LocalNameOffset[i];
   }
   return 0;
}

const char * COMF::GetSegmentName(uint32 i) {
   // Get section name by segment index
   if (i == 0) return "none";
   if ((i & 0xC000) == 0x4000) {
      // Borland communal section
      static char text[32];
      sprintf(text, "communal section %i", i - 0x4000);
      return text;
   }
   if (i <= NumRecords) {
      return NameBuffer.Buf() + SegmentNameOffset[i];
   }
   return "?";
}


const char * COMF::GetSymbolName(uint32 i) {
   // Get external symbol name by index
   if (i == 0) return "null";
   if (i < SymbolNameOffset.GetNumEntries()) {
      return NameBuffer.Buf() + SymbolNameOffset[i];
   }
   // return "?";
   // index out of range
   static char temp[100];
   sprintf(temp, "Unknown index %i", i);
   return temp;
}

const char * COMF::GetGroupName(uint32 i) {
   // Get group name by index
   if (i == 0) return "none";
   if (i <= NumRecords) {
      return NameBuffer.Buf() + GroupNameOffset[i];
   }
   return "?";
}

const char * COMF::GetRecordTypeName(uint32 i) {
   // Get record type name
   return Lookup(OMFRecordTypeNames, i);
}

// Member functions for parsing SOMFRecordPointer
uint8  SOMFRecordPointer::GetByte() {
   // Read next byte from buffer
   return *(buffer + FileOffset + Index++);
}

uint16 SOMFRecordPointer::GetWord() {
   // Read next 16 bit word from buffer
   uint16 x = *(uint16*)(buffer + FileOffset + Index);
   Index += 2;
   return x;
}

uint32 SOMFRecordPointer::GetDword() {
   // Read next 32 bit dword from buffer
   uint32 x = *(uint32*)(buffer + FileOffset + Index);
   Index += 4;
   return x;
}

uint32 SOMFRecordPointer::GetIndex() {
   // Read byte or word, depending on sign of first byte
   uint32 byte1, byte2;
   byte1 = GetByte();
   if (byte1 & 0x80) {
      // Two byte index
      byte2 = GetByte();
      return ((byte1 & 0x7F) << 8) | byte2;
   }
   else {
      // One byte index
      return byte1;
   }
}

uint32 SOMFRecordPointer::GetNumeric(){
   // Read word or dword, depending on record type even or odd
   if (Type & 1) {
      // Odd record type. Number is 32 bits
      return GetDword();
   }
   else {
      // Even record type. Number is 16 bit s
      return GetWord();
   }
}

uint32 SOMFRecordPointer::GetLength() {
   // Read 1, 2, 3 or 4 bytes, depending on value of first byte
   uint32 x = GetByte();
   switch (x) {
   case 0x81: // 16-bit value
      return GetWord();
   case 0x82: // 24-bit value
      x = GetWord();
      return (GetByte() << 16) + x;
   case 0x84: // 32-bit value
      return GetDword();
   default: // 8-bit value
      if (x > 0x80) err.submit(1203);
      return x;
   }
}

char * SOMFRecordPointer::GetString() {
   // Read string and return as ASCIIZ string in static buffer
   static char String[256];
   uint8 Length = GetByte();
   if (Length == 0 /*|| Length >= sizeof(String)*/) {
      String[0] = 0;
   }
   else {
      // Copy string
      memcpy(String, buffer + FileOffset + Index, Length);
      // Terminate by 0
      String[Length] = 0;
   }
   // Point to next
   Index += Length;

   return String;
}

void SOMFRecordPointer::Start(int8 * Buffer, uint32 FileOffset, uint32 FileEnd) {
   // Start scanning through records
   this->buffer = Buffer;
   this->FileOffset = FileOffset;
   this->FileEnd = FileEnd;
   Index = 0;
   Type = GetByte();
   Type2 = Type;  if (Type2 < OMF_LIBHEAD) Type2 &= ~1; // Make even
   uint16 RecordSize = GetWord();
   End = Index + RecordSize - 1;
   if (FileOffset + RecordSize + 3 > FileEnd) err.submit(2301); // Extends beyond end of file
}

uint8 SOMFRecordPointer::GetNext(uint32 align) {
   // Get next record. Returns record type, made even. Returns 0 if finished
   // align = alignment after MODEND records = page size. Applies to lib files only
   FileOffset += End + 1;

   // Check if alignment needed
   if (align > 1 && Type2 == OMF_MODEND) {
      // Align after MODEND record in library
      FileOffset = (FileOffset + align - 1) & - (int32)align;
   }
   if (FileOffset >= FileEnd) return 0;          // End of file
   Index = 0;                                    // Start reading record
   Type = GetByte();                             // Get record type
   Type2 = Type;  if (Type2 < OMF_LIBHEAD) Type2 &= ~1; // Make even
   uint16 RecordSize = GetWord();                // Get record size
   End = Index + RecordSize - 1;                 // Point to checksum byte
   if ((uint64)FileOffset + RecordSize + 3 > FileEnd) err.submit(2301); // Extends beyond end of file
   return Type2;
}

uint32 SOMFRecordPointer::InterpretLIDATABlock() {
   // Interpret Data block in LIDATA record recursively
   // Prints repeat count and returns total size
   uint32 RepeatCount = GetNumeric();
   uint32 BlockCount  = GetWord();
   uint32 Size = 0;
   printf("%i * ", RepeatCount);
   if (BlockCount == 0) {
      Size = GetByte();
      Index += Size;
      printf("%i", Size);
      return RepeatCount * Size;
   }
   // Nested repeat blocks
   printf("(");
   for (uint32 i = 0; i < BlockCount; i++) {
      // Recursion
      Size += InterpretLIDATABlock();
      if (i+1 < BlockCount) printf(" + ");
   }
   printf(")");
   return RepeatCount * Size;
}


uint32 SOMFRecordPointer::UnpackLIDATABlock(int8 * destination, uint32 MaxSize) {
   // Unpack Data block in LIDATA record recursively and store data at destination
   uint32 RepeatCount = GetNumeric();            // Outer repeat count
   uint32 BlockCount  = GetWord();               // Inner repeat count
   uint32 Size = 0;                              // Size of data expanded so far
   uint32 RSize;                                 // Size of recursively expanded data
   uint32 SaveIndex;                             // Save Index for repetition
   uint32 i, j;                                  // Loop counters
   if (BlockCount == 0) {
      // Contains one repeated block
      Size = GetByte();                          // Size of repeated block
      if (RepeatCount * Size > MaxSize) {
         // Data outside allowed area
         err.submit(2310);                       // Error message
         Index += Size;                          // Point to after block
         return 0;                               // No data stored
      }

      // Loop RepeatCount times
      for (i = 0; i < RepeatCount; i++) {
         // copy data block into destination
         memcpy(destination, buffer + FileOffset + Index, Size);
         destination += Size;
      }
      Index += Size;                             // Point to after block
      return RepeatCount * Size;                 // Size of expanded data
   }
   // Nested repeat blocks
   SaveIndex = Index;
   // Loop RepeatCount times
   for (i = 0; i < RepeatCount; i++) {
      // Go back and repeat unpacking
      Index = SaveIndex;
      // Loop BlockCount times
      for (j = 0; j < BlockCount; j++) {
         // Recursion
         RSize = UnpackLIDATABlock(destination, MaxSize);
         destination += RSize;
         MaxSize -= RSize;
         Size += RSize;
      }
   }
   return Size;
}


// Members of COMFFileBuilder, class for building OMF files
COMFFileBuilder::COMFFileBuilder() {
   // Constructor
   Index = 0;
}

void COMFFileBuilder::StartRecord(uint8 type) {
   // Start building new record
   this->Type = type;                            // Save type
   RecordStart = Index = GetDataSize();          // Remember start position
   PutByte(Type);                                // Put type into record
   PutWord(0);                                   // Reserve space for size, put in later
}

void COMFFileBuilder::EndRecord() {
   // Finish building current record
   // Update length
   Get<uint16>(RecordStart + 1) = GetSize() + 1;

   // Make checksum
   int8 checksum = 0;
   for (uint32 i = RecordStart; i < Index; i++) checksum += Buf()[i];
   PutByte(-checksum);

   // Check size limit
   if (GetSize() > 0x407) {
      err.submit(9005);
   }
}

void COMFFileBuilder::PutByte(uint8 x) {
   // Put byte into buffer
   Push(&x, 1);
   Index++;
}

void COMFFileBuilder::PutWord(uint16 x) {
   // Put 16 bit word into buffer
   Push(&x, 2);
   Index += 2;
}

void COMFFileBuilder::PutDword(uint32 x) {
   // Put 32 bit dword into buffer
   Push(&x, 4);
   Index += 4;
}

void COMFFileBuilder::PutIndex(uint32 x) {
   // Put byte or word into buffer (word if > 0x7F)
   if (x < 0x80) {
      // One byte
      PutByte(x);
   }
   else {
      // Two bytes
      if (x > 0x7fff) {
         err.submit(2303);             // Index out of range
      }
      PutByte((uint8)(x >> 8) | 0x80); // First byte = high byte | 0x80
      PutByte(uint8(x));               // Second byte = low byte
   }
}

void COMFFileBuilder::PutNumeric(uint32 x) {
   // Put word or dword into buffer, depending on type being even or odd
   if (Type & 1) {
      PutDword(x);                     // Type is odd, put 32 bits
   }
   else {
      if (x > 0xffff) err.submit(2304);// Index out of range
      PutWord(uint16(x));              // Type is even, put 16 bits
   }
}

void COMFFileBuilder::PutString(const char * s) {
   // Put ASCII string into buffer, preceded by size
   uint32 len = (uint32)strlen(s);     // Check length
   if (len > 255) {
      // String too long
      err.submit(1204, s);             // Issue warning
      len = 255;                       // Truncate string to 255 characters
   }
   PutByte(uint8(len));                // Store length
   Push(s, len);                       // Store len bytes
   Index += len;                       // Update index
}

void COMFFileBuilder::PutBinary(void * p, uint32 Size) {
   // Put binary data of any length
   if (Size > 1024) {err.submit(9000); Size = 1024;}  // 1024 bytes size limit
   Push(p, Size);
   Index += Size;
}

uint32 COMFFileBuilder::GetSize() {
   // Get size of data added so far
   if (Index <= RecordStart + 3) return 0;
   return Index - RecordStart - 3;     // Type and size fields not included in size
}
