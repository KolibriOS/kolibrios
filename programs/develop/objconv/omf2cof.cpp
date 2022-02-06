/****************************  omf2cof.cpp   *********************************
* Author:        Agner Fog
* Date created:  2007-02-08
* Last modified: 2018-08-15
* Project:       objconv
* Module:        omf2cof.cpp
* Description:
* Module for converting OMF file to PE/COFF file
*
* Copyright 2007-2018 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"

// Alignment value translation table
static const uint32 OMFAlignTranslate[8] = {0,1,2,16,256,4,0,0};

COMF2COF::COMF2COF() {
    // Constructor
    memset(this, 0, sizeof(*this));               // Reset everything
}

void COMF2COF::Convert() {
    // Do the conversion
    // Allocate variable size buffers
    //NewSectIndex.SetNum(this->NSections);// Allocate section translation table
    //NewSectIndex.SetZero();              // Initialize

    // Call the subfunctions
    ToFile.SetFileType(FILETYPE_COFF);  // Set type of to file
    MakeFileHeader();                   // Make file header
    MakeSymbolTable1();                 // Make symbol table and string table entries for file and segments
    MakeSymbolTable2();                 // Make symbol table and string table entries for external symbols
    MakeSymbolTable3();                 // Make symbol table and string table entries for public symbols
    MakeSymbolTable4();                 // Make symbol table and string table entries for communal symbols
    MakeSymbolTable5();                 // Make symbol table and string table entries for local symbols
    MakeSections();                     // Make sections and relocation tables
    CheckUnsupportedRecords();          // Make warnings if file containes unsupported record types
    MakeBinaryFile();                   // Put sections together
    *this << ToFile;                    // Take over new file buffer
}


void COMF2COF::MakeFileHeader() {
    // Convert subfunction: File header
    // Make PE file header
    NewFileHeader.Machine = PE_MACHINE_I386;
    NewFileHeader.TimeDateStamp = (uint32)time(0);
    NewFileHeader.SizeOfOptionalHeader = 0;
    NewFileHeader.Flags = 0;

    // Values inserted later:
    NewFileHeader.NumberOfSections = 0;
    NewFileHeader.PSymbolTable = 0;
    NewFileHeader.NumberOfSymbols = 0;
}


void COMF2COF::MakeSymbolTable1() {
    // Make symbol table string table and section table entries for file and segments
    SCOFF_SymTableEntry sym;                      // Symbol table entry
    SCOFF_SectionHeader sec;                      // Section header entry
    char * ClassName;                             // Old segment class name

    // Initialize new string table. make space for 4-bytes size
    NewStringTable.Push(0, 4);

    // Allocate SegmentTranslation buffer
    SegmentTranslation.SetNum(SegmentNameOffset.GetNumEntries());

    // Make symbol table entry for file name
    memset(&sym, 0, SIZE_SCOFF_SymTableEntry);
    strcpy(sym.s.Name, ".file");
    sym.s.SectionNumber = COFF_SECTION_DEBUG;
    sym.s.StorageClass = COFF_CLASS_FILE;
    char * ShortFileName = CLibrary::ShortenMemberName(OutputFileName);
    sym.s.NumAuxSymbols = 1;  // File name is truncated so it will fit into the 18 bytes of SIZE_SCOFF_SymTableEntry
    NewSymbolTable.Push(sym); // Store symbol table entry
    // Needs auxiliary entry:
    memset(&sym, 0, SIZE_SCOFF_SymTableEntry);
    if (strlen(ShortFileName) < SIZE_SCOFF_SymTableEntry) {
        strcpy(sym.s.Name, ShortFileName);
    }
    NewSymbolTable.Push(sym); // Store auxiliary symbol table entry

    // Define structure of attributes
    OMF_SAttrib Attributes;
    // Other segment properties
    uint32 SegLength, NameIndex, ClassIndex;
    //uint32 Offset;
    const char * sname;                           // Segment/section name
    uint32 SegNum = 0;                            // Segment/section number
    uint32 StringI;                               // New sting table index
    uint32 i;                                     // Record number
    int32  j, n;                                  // Temporary

    // Loop through segments of old file
    for (i = 0; i < NumRecords; i++) {
        if (Records[i].Type2 == OMF_SEGDEF) {
            // SEGDEF record
            Records[i].Index = 3;
            // Loop through entries in record. There should be only 1
            while (Records[i].Index < Records[i].End) {
                Attributes.b = Records[i].GetByte(); // Read attributes
                if (Attributes.u.A == 0) {
                    // Frame and Offset only included if A = 0
                    Records[i].GetWord();             // Frame ignored                    
                    Records[i].GetByte();
                }
                //else Offset = 0;
                SegLength  = Records[i].GetNumeric();
                NameIndex  = Records[i].GetIndex();
                ClassIndex = Records[i].GetIndex();  // Class index
                Records[i].GetIndex();               // Overlay index ignored
                sname = GetLocalName(NameIndex);     // Segment name = new section name

                if (Attributes.u.B) {
                    // Segment is big
                    if (Attributes.u.P) {
                        // 32 bit segment. Big means 2^32 bytes!
                        err.submit(2306);
                    }
                    else {
                        // 16 bit segment. Big means 2^16 bytes
                        SegLength = 0x10000;
                    }
                }

                // make symbol table entry
                memset(&sym, 0, SIZE_SCOFF_SymTableEntry);

                // Put name into string table
                StringI = NewStringTable.PushString(sname);

                // Put name into symbol table
                //COFF_PutNameInSymbolTable(sym, sname, NewStringTable);
                ((uint32*)(sym.s.Name))[1] = StringI;

                sym.s.SectionNumber = ++SegNum;        // Count section number
                sym.s.StorageClass = COFF_CLASS_STATIC;
                sym.s.NumAuxSymbols = 1;               // Needs 1 aux record

                // Remember NewSymbolTable index
                SegmentTranslation[SegNum] = NewSymbolTable.GetNumEntries();
                NewSymbolTable.Push(sym);            // Store symbol table entry

                // Make auxiliary entry
                memset(&sym, 0, SIZE_SCOFF_SymTableEntry);

                // Insert section size
                sym.section.Length = SegLength;

                // Remember to insert NumberOfRelocations here later

                // Store auxiliary symbol table entry
                NewSymbolTable.Push(sym);

                // Make section header            
                memset(&sec, 0, sizeof(sec));        // Reset section header

                // Put name into section header
                sprintf(sec.Name, "/%i", StringI);

                // Put size into section header
                sec.SizeOfRawData = SegLength;

                // Alignment
                switch (Attributes.u.A) {
                case 0:  // Absolute segment
                    err.submit(2307);  break;
                case 1:  // Byte
                    sec.Flags |= PE_SCN_ALIGN_1;  break;
                case 2:  // Word
                    sec.Flags |= PE_SCN_ALIGN_2;  break;
                case 3:  // Paragraph
                    sec.Flags |= PE_SCN_ALIGN_16;  break;
                case 4:  // Page. May be 256 or 4096, depending on system
                    // If we use 4096 where the source intended 256, we may get
                    // size and symbol offsets wrong!
                    sec.Flags |= PE_SCN_ALIGN_256;  break;
                case 5:  // Dword
                    sec.Flags |= PE_SCN_ALIGN_4;  break;
                default: // Unknown alignment
                    err.submit(2308, Attributes.u.A);
                    sec.Flags |= PE_SCN_ALIGN_16;  break;
                }

                // Get further attributes from class name
                ClassName = GetLocalName(ClassIndex);

                // Convert class name to upper case
                n = (int32)strlen(ClassName);
                for (j = 0; j < n; j++) ClassName[j] &= ~0x20;

                // Search for known class names.
                // Standard names are CODE, DATA, BSS, CONST, STACK
                if (strstr(ClassName, "CODE") || strstr(ClassName, "TEXT")) {
                    // Code segment
                    sec.Flags |= PE_SCN_CNT_CODE | PE_SCN_MEM_EXECUTE | PE_SCN_MEM_READ;
                }
                else if (strstr(ClassName, "DATA")) {
                    // Data segment
                    sec.Flags |= PE_SCN_CNT_INIT_DATA | PE_SCN_MEM_READ | PE_SCN_MEM_WRITE;
                }
                else if (strstr(ClassName, "BSS")) {
                    // Unitialized data segment
                    sec.Flags |= PE_SCN_CNT_UNINIT_DATA | PE_SCN_MEM_READ | PE_SCN_MEM_WRITE;
                }
                else if (strstr(ClassName, "CONST")) {
                    // Constant data segment
                    sec.Flags |= PE_SCN_CNT_INIT_DATA | PE_SCN_MEM_READ;
                }
                else if (strstr(ClassName, "STACK")) {
                    // Stack segment. Ignore
                    sec.Flags |= PE_SCN_LNK_REMOVE;
                    err.submit(1206); // Warning: ignored
                }
                else {
                    // Unknown/user defined class. Assume data segment
                    sec.Flags |= PE_SCN_CNT_INIT_DATA | PE_SCN_MEM_READ | PE_SCN_MEM_WRITE;
                }

                // Insert pointers to relocations and raw data later
                // Store section header
                NewSectionHeaders.Push(sec);
            }
            if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
        }
        if (Records[i].Type2 == OMF_COMDAT || Records[i].Type2 == OMF_COMDEF) {
            // Communal sections
            err.submit(1055);
        }
    }
}

void COMF2COF::MakeSymbolTable2() {
    // Make symbol table and string table entries for external symbols
    uint32 i;
    SCOFF_SymTableEntry sym;                      // new symbol table entry
    uint32 NumExtSym = SymbolNameOffset.GetNumEntries(); // Number of external symbols
    ExtdefTranslation.SetNum(NumExtSym+1);          // Allocate space in translation table

    // Loop through external symbol names
    for (i = 1; i < NumExtSym; i++) {
        // Reset symbol table entry
        memset(&sym, 0, SIZE_SCOFF_SymTableEntry);

        // Insert name
        COFF_PutNameInSymbolTable(sym, GetSymbolName(i), NewStringTable);

        // Insert storage class
        sym.s.StorageClass = COFF_CLASS_EXTERNAL;

        // Store symbol table entry
        NewSymbolTable.Push(sym);

        // Update table for translating old EXTDEF number (1-based) to new symbol table index (0-based)
        ExtdefTranslation[i] = NewSymbolTable.GetNumEntries() - 1;
    }
}


void COMF2COF::MakeSymbolTable3() {
    // Make symbol table and string table entries for public symbols
    SCOFF_SymTableEntry sym;                      // new symbol table entry
    uint32 i;                                     // Record index
    char * string;                                // Symbol name
    uint32 Segment;                               // Segment
    uint32 Offset;                                // Offset
    uint32 Namei;                                 // Index into symbol table
    SOMFLocalSymbol localsym;                     // Entry into LocalSymbols

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

                // Reset symbol table entry
                memset(&sym, 0, SIZE_SCOFF_SymTableEntry);

                // Insert name
                Namei = COFF_PutNameInSymbolTable(sym, string, NewStringTable);

                // Insert storage class
                sym.s.StorageClass = COFF_CLASS_EXTERNAL;

                // Store offset
                sym.s.Value = Offset;

                // Section number = segment number
                if (Segment == 0) sym.s.SectionNumber = COFF_SECTION_ABSOLUTE;
                else sym.s.SectionNumber = (int16)Segment;

                // Store symbol table entry
                NewSymbolTable.Push(sym);

                // Make entry into LocalSymbols to indicate that this symbol has a name
                if (Segment > 0) {
                    // Make index into NewStringTable if we don't allready have one
                    if (Namei == 0) Namei = NewStringTable.PushString(string);
                    localsym.Offset = Offset;
                    localsym.Segment = Segment;
                    localsym.Name = Namei;
                    localsym.NewSymtabIndex = NewSymbolTable.GetNumEntries() - 1; // 0-based index into new symbol table
                    LocalSymbols.PushUnique(localsym);
                }
            }
            if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
        }
    }
}

void COMF2COF::MakeSymbolTable4() {
    // Make symbol table and string table entries for communal symbols
    // Warning: Currently not supported!
}

void COMF2COF::MakeSymbolTable5() {
    // Make symbol table and string table entries for local symbols.
    // There is no table for local symbols in OMF files. We have to search
    // through all FIXUPP records for relocation targets and assign arbitrary
    // names to them.

    uint32 i;                                     // Loop counter
    uint32 Target, TargetDisplacement;            // Contents of FIXUPP record
    uint8 byte1, byte2;                           // First two bytes of FIXUPP subrecord
    // Bitfields in subrecords
    OMF_SLocat Locat;                             // Structure of first two bytes of FIXUP subrecord swapped = Locat field
    OMF_SFixData FixData;                         // Structure of FixData field in FIXUP subrecord of FIXUPP record
    OMF_STrdDat TrdDat;                           // Structure of Thread Data field in THREAD subrecord of FIXUPP record
    int8 * LastDataRecordPointer = 0;             // Pointer to data in the data record that relocations refer to
    uint32 LastDataRecordSize = 0;                // Size of the data record that relocations refer to
    SOMFLocalSymbol localsym;                     // Entry into LocalSymbols
    uint32 LocalSymNum = 0;                       // Number of unnamed local symbols
    char NewName[32];                             // Buffer for making new name
    SCOFF_SymTableEntry sym;                      // New symbol table entry

    // Search for FIXUPP records and data records
    for (i = 0; i < NumRecords; i++) {

        if (Records[i].Type2 == OMF_LEDATA) {
            // LEDATA record. Remember pointer to binary data in order to read inline offset
            Records[i].Index = 3;                   // Initialize record reading
            Records[i].GetIndex();                  // Read segment and offset
            Records[i].GetNumeric();
            LastDataRecordSize = Records[i].End - Records[i].Index; // Calculate size of data
            LastDataRecordPointer = Records[i].buffer + Records[i].FileOffset + Records[i].Index;
        }

        if (Records[i].Type2 == OMF_FIXUPP) {
            // FIXUPP record
            Records[i].Index = 3;                   // Initialize record reading

            // Loop through entries in record
            while (Records[i].Index < Records[i].End) {

                // Read first byte
                byte1 = Records[i].GetByte();

                if (byte1 & 0x80) {
                    // This is a FIXUP subrecord

                    Target = 0; TargetDisplacement = 0;
                    // read second byte
                    byte2 = Records[i].GetByte();
                    // swap bytes and put into byte12 bitfield
                    Locat.bytes[1] = byte1;
                    Locat.bytes[0] = byte2;
                    // Read FixData
                    FixData.b = Records[i].GetByte();
                    // Read conditional fields
                    if (FixData.s.F == 0) {
                        if (FixData.s.Frame < 4) {
                            Records[i].GetIndex();  // Frame. Ignore
                        }
                    }
                    if (FixData.s.T == 0) {
                        // Target specified
                        Target = Records[i].GetIndex();
                        //uint32 TargetMethod = FixData.s.Target + FixData.s.P * 4;
                    }
                    if (FixData.s.P == 0) {
                        TargetDisplacement = Records[i].GetNumeric();
                    }
                    // Get inline addend
                    if (LastDataRecordPointer && Locat.s.Offset < LastDataRecordSize) {
                        int8 * inlinep = LastDataRecordPointer + Locat.s.Offset;
                        if (Locat.s.Location == 9 || Locat.s.Location == 13) {
                            TargetDisplacement += *(int32*)inlinep;
                        }
                    }
                    if (FixData.s.T == 0 && (FixData.s.Target == 0 || FixData.s.Target == 1)) {
                        // Target is local symbol

                        // Make entry in LocalSymbols
                        localsym.Offset = TargetDisplacement;    // Offset to segment
                        localsym.Segment = Target;               // Target segment
                        localsym.Name = 0;                       // Has no name yet
                        localsym.NewSymtabIndex = 0;             // Not in new symbol table yet
                        // Make entry in LocalSymbols.
                        // PushUnique will not make an entry if this target address already
                        // has an entry in LocalSymbols and possibly a public name
                        LocalSymbols.PushUnique(localsym);       
                    }
                }
                else {
                    // This is a THREAD subrecord. Read and ignore
                    TrdDat.b = byte1;                 // Put byte into bitfield
                    if (TrdDat.s.Method < 4) {
                        Records[i].GetIndex();         // has index field if method < 4 ?
                    }
                }
            } // Finished loop through subrecords
            if (Records[i].Index != Records[i].End) err.submit(1203);   // Check for consistency
        }
    } // Finished loop through records

    // Now check LocalSymbols for unnamed symbols
    for (i = 0; i < LocalSymbols.GetNumEntries(); i++) {
        if (LocalSymbols[i].Name == 0) {

            // Unnamed symbol. Give it a name
            sprintf(NewName, "?NoName%02i", ++LocalSymNum);

            // Make index in new symbol table
            // Reset symbol table entry
            memset(&sym, 0, SIZE_SCOFF_SymTableEntry);

            // Insert name
            LocalSymbols[i].Name = COFF_PutNameInSymbolTable(sym, NewName, NewStringTable);
            // if (LocalSymbols[i].Name == 0) LocalSymbols[i].Name = NewStringTable.PushString(NewName);

            // Store offset
            sym.s.Value = LocalSymbols[i].Offset;

            // Section number = segment number
            sym.s.SectionNumber = LocalSymbols[i].Segment;

            // Storage class
            sym.s.StorageClass = COFF_CLASS_STATIC;

            // Store symbol table entry
            NewSymbolTable.Push(sym);

            // Store index into new symbol table (0 - based)
            LocalSymbols[i].NewSymtabIndex = NewSymbolTable.GetNumEntries() - 1;
        }
    }
}


void COMF2COF::MakeSections() {
    // Make sections and relocation tables
    uint32 SegNum;                                // Index into NewSectionHeaders = segment - 1
    uint32 DesiredSegment;                        // Old segment number = new section number
    uint32 RecNum;                                // Old record number
    CMemoryBuffer TempBuf;                        // Temporary buffer for building raw data
    CMemoryBuffer RelocationTable;                // Temporary buffer for building new relocation table
    SCOFF_Relocation rel;                         // New relocation table record
    uint32 LastDataRecord = 0;                    // Index to the data record that relocations refer to
    uint32 LastDataRecordSize = 0;                // Size of the data record that relocations refer to
    int8 * LastDataRecordPointer = 0;             // Pointer to data in the data record that relocations refer to
    uint32 Segment = 0;                           // Segment of last LEDATA, LIDATA or COMDEF record
    uint32 Offset;                                // Offset of LEDATA or LIDATA record to segment
    uint32 Size;                                  // Size of data in LEDATA or LIDATA record
    uint32 SegmentSize;                           // Total size of segment
    uint32 LastOffset;                            // Offset after last LEDATA into segment
    uint32 FileOffsetData;                        // File offset of first raw data and relocations in new file
    uint32 FileOffset;                            // File offset of current raw data or relocations

    // File offset of first data = size of file header and section headers
    FileOffsetData = sizeof(SCOFF_FileHeader) + NewSectionHeaders.GetNumEntries() * sizeof(SCOFF_SectionHeader);

    // Loop through segments
    for (SegNum = 0; SegNum < NewSectionHeaders.GetNumEntries(); SegNum++) {

        DesiredSegment = SegNum + 1;               // Search for records referring to this segment

        SegmentSize = NewSectionHeaders[SegNum].SizeOfRawData;
        if (SegmentSize == 0) continue;            // Empty segment

        // Allocate temporary data buffer and reset it
        TempBuf.SetSize(SegmentSize + 16);
        int FillByte = 0;                          // Byte to fill memory with
        if (NewSectionHeaders[SegNum].Flags & PE_SCN_CNT_CODE) {
            // Code segment. Fill any unused bytes with NOP opcode = 0x90
            FillByte = 0x90;
        }
        memset(TempBuf.Buf(), FillByte, SegmentSize + 16);// Reset to all 0 or NOP

        // Reset relocation table buffer
        RelocationTable.SetSize(0);

        LastOffset = 0;  LastDataRecordSize = 0;

        // Search for LEDATA, LIDATA and FIXUPP records for this segment
        for (RecNum = 0; RecNum < NumRecords; RecNum++) {
            if (Records[RecNum].Type2 == OMF_LEDATA) {

                // LEDATA record
                Records[RecNum].Index = 3;           // Initialize record reading
                Segment = Records[RecNum].GetIndex();// Read segment number

                if (Segment != DesiredSegment) continue; // Does not refer to this segment

                Offset  = Records[RecNum].GetNumeric();// Read offset
                Size    = Records[RecNum].End - Records[RecNum].Index; // Calculate size of data
                LastDataRecord = RecNum;             // Save for later FIXUPP that refers to this record
                
                // Check if data within segment
                if (Offset + Size > SegmentSize) {
                    err.submit(2309, GetSegmentName(Segment));
                    return;
                }

                if (Offset < LastOffset + LastDataRecordSize && LastOffset < Offset + Size) {
                    // Overlapping data records
                    if (Offset + 8 < LastOffset + LastDataRecordSize || !(NewSectionHeaders[SegNum].Flags & PE_SCN_CNT_CODE)) {
                        // Overlapping data by more than 7 bytes or not executable code
                        err.submit(1207);
                    }
                    else {
                        // Possibly backpatched code
                        err.submit(1208);              // Warning
                        err.ClearError(1208);          // Report only once
                    }
                }

                LastDataRecordSize = Size;
                LastDataRecordPointer = Records[RecNum].buffer + Records[RecNum].FileOffset + Records[RecNum].Index;
                LastOffset = Offset;                 // Save offset for subsequent FIXUPP records

                /*// Check if data within segment
                if (Offset + Size > SegmentSize) {
                    err.submit(2309, GetSegmentName(Segment));
                    continue;
                } */

                // Put raw data into temporary buffer
                memcpy(TempBuf.Buf() + Offset, LastDataRecordPointer, Size);

            } // Finished with LEDATA record

            if (Records[RecNum].Type2 == OMF_LIDATA) {
                // LIDATA record
                Records[RecNum].Index = 3;           // Initialize record reading
                Segment = Records[RecNum].GetIndex();

                if (Segment != DesiredSegment) continue; // Does not refer to this segment

                LastDataRecord = RecNum;             // Save for later FIXUPP that refers to this record

                Offset  = Records[RecNum].GetNumeric();// Read offset

                if (Offset > SegmentSize) {
                    err.submit(2310); return;       // Error: outside bounds
                }

                // Unpack LIDATA blocks recursively
                Size = Records[RecNum].UnpackLIDATABlock(TempBuf.Buf() + Offset, SegmentSize - Offset);

                if (Offset < LastOffset + LastDataRecordSize && LastOffset < Offset + Size) {
                    // Overlapping data records
                    err.submit(1207);                 // Warning
                }
                LastDataRecordSize = Size;           // Save data size
                LastOffset = Offset;                 // Save offset for subsequent FIXUPP records

            } // Finished with LIDATA record

            if (Records[RecNum].Type2 == OMF_COMDAT) {
                // COMDAT record. Currently not supported by objconv
                LastDataRecord = RecNum;             // Save for later FIXUPP that refers to this record
                Segment = 0;                         // Ignore any relocation referring to this
            }

            if (Records[RecNum].Type2 == OMF_FIXUPP) {
                // FIXUPP record

                if (Segment != DesiredSegment) continue; // Does not refer to this segment

                uint32 Target, TargetDisplacement; // Contents of FIXUPP record
                //uint32 Frame; // Contents of FIXUPP record
                uint8 byte1, byte2;                 // First two bytes of subrecord

                // Bitfields in subrecords
                OMF_SLocat Locat;         // Structure of first two bytes of FIXUP subrecord swapped = Locat field
                OMF_SFixData FixData;     // Structure of FixData field in FIXUP subrecord of FIXUPP record
                OMF_STrdDat TrdDat;       // Structure of Thread Data field in THREAD subrecord of FIXUPP record

                Records[RecNum].Index = 3;

                if (Records[LastDataRecord].Type2 != OMF_LEDATA && Records[RecNum].Index < Records[RecNum].End) {
                    // Non-empty FIXUPP record does not refer to LEDATA record
                    if (Records[LastDataRecord].Type2 == OMF_COMDAT) {
                        // COMDAT currently not supported. Ignore!
                    }
                    else if (Records[LastDataRecord].Type2 == OMF_LIDATA) {
                        err.submit(2311);              // Error: Relocation of iterated data not supported
                    }
                    else {
                        err.submit(2312);              // Does not refer to data record
                    }
                    continue;                         // Ignore this FIXUPP record
                }

                // Loop through entries in record
                while (Records[RecNum].Index < Records[RecNum].End) {

                    // Read first byte
                    byte1 = Records[RecNum].GetByte();
                    if (byte1 & 0x80) {

                        // This is a FIXUP subrecord
                        //Frame = 0; 
                        Target = 0; TargetDisplacement = 0;

                        // read second byte
                        byte2 = Records[RecNum].GetByte();
                        // swap bytes and put into byte12 bitfield
                        Locat.bytes[1] = byte1;
                        Locat.bytes[0] = byte2;
                        // Read FixData
                        FixData.b = Records[RecNum].GetByte();

                        // Read conditional fields
                        if (FixData.s.F == 0) {
                            if (FixData.s.Frame < 4) {
                                Records[RecNum].GetIndex();
                            }
                        }

                        if (FixData.s.T == 0) {
                            // Target specified
                            Target = Records[RecNum].GetIndex();
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

                        // Get inline addend and check relocation method
                        if (LastDataRecordPointer && Locat.s.Offset < LastDataRecordSize) {
                            // Pointer to relocation source inline in raw data:
                            int8 * inlinep = LastDataRecordPointer + Locat.s.Offset;

                            switch (Locat.s.Location) { // Relocation method

                            case 9: case 13: // 32 bit
                                // The OMF format may indicate a relocation target by an
                                // offset stored inline in the relocation source.
                                // We prefer to store the target address explicitly in a
                                // symbol table entry. 

                                // Add the inline offset to the explicit offset
                                TargetDisplacement += *(uint32*)inlinep;

                                // Remove the inline addend to avoid adding it twice:
                                // We have to do this in the new buffer TempBuf because 
                                // the data have already been copied to TempBuf
                                if (*(uint32*)(TempBuf.Buf() + LastOffset + Locat.s.Offset) != *(uint32*)inlinep) {
                                    // Check that the data in Buf() and TempBuf.Buf() are the same
                                    err.submit(9000);
                                }
                                // Remove the inline addend to avoid adding it twice
                                *(uint32*)(TempBuf.Buf() + LastOffset + Locat.s.Offset) = 0;
                                break;

                            case 0: case 4:  // 8 bit. Not supported
                                err.submit(2316, "8 bit");  break;

                            case 1: case 2: case 5: // 16 bit. Not supported
                                err.submit(2316, "16 bit");  break;

                            case 3: // 16+16 bit. Not supported
                                err.submit(2316, "16+16 bit far");  break;

                            case 6: case 11: // 16+32 bit. Not supported
                                err.submit(2316, "16+32 bit far");  break;
                            }
                        }

                        // Make relocation record
                        // Offset of relocation source
                        rel.VirtualAddress = Locat.s.Offset + LastOffset; 

                        SOMFLocalSymbol locsym; // Symbol record for search in LocalSymbols table
                        int32 LocalSymbolsIndex; // Index into LocalSymbols table

                        // Relocation type: direct or EIP-relative
                        // (The displacement between relocation source and EIP for 
                        // self-relative relocations is implicit in both OMF and COFF 
                        // files. No need for correction)
                        rel.Type = Locat.s.M ? COFF32_RELOC_DIR32 : COFF32_RELOC_REL32;

                        switch (FixData.s.Target) { // = Target method modulo 4
                        case 0: // T0 and T4: Target = segment

                            // Local or public symbol. Search in LocalSymbols table
                            locsym.Segment = Target;     // Target segment
                            locsym.Offset = TargetDisplacement;  // Target offset including inline displacement
                            // Find in LocalSymbols table
                            LocalSymbolsIndex = LocalSymbols.Exists(locsym);
                            if (LocalSymbolsIndex < 0) {err.submit(9000); continue;} // Not found

                            // Get index into new symbol table
                            rel.SymbolTableIndex = LocalSymbols[LocalSymbolsIndex].NewSymtabIndex;
                            break;

                        case 1: // T1 and T5: Target = segment group
                            // Don't know how to handle group-relative relocation. Make error message
                            err.submit(2315, GetLocalName(Target));
                            continue;

                        case 2: // T2 and T6: Target = external symbol

                            // Translate old EXTDEF index to new symbol table index
                            if (Target >= ExtdefTranslation.GetNumEntries()) {
                                Target = 0; err.submit(2312);
                                continue;
                            }
                            rel.SymbolTableIndex = ExtdefTranslation[Target];

                            // Put addend inline in new file
                            if (LastOffset + Locat.s.Offset < SegmentSize) {
                                *(uint32*)(TempBuf.Buf() + LastOffset + Locat.s.Offset) = TargetDisplacement;
                            }
                            break;

                        default: // Unknown method
                            err.submit(2314, FixData.s.Target + FixData.s.P * 4);
                        }

                        // Store in temporary relocation table
                        RelocationTable.Push(&rel, SIZE_SCOFF_Relocation);

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
        } // End of loop to search for LEDATA, LIDATA and FIXUPP records for this segment

        // Transfer raw data from TempBuf to NewData buffer
        FileOffset = NewData.Push(TempBuf.Buf(), SegmentSize);

        // Put file offset of raw data into section header
        NewSectionHeaders[SegNum].PRawData = FileOffsetData + FileOffset;

        // Align relocation table by 4
        NewData.Align(4);

        // Transfer relocation table from RelocationTable to NewData buffer
        FileOffset = NewData.Push(RelocationTable.Buf(), RelocationTable.GetDataSize());

        // Put file offset of relocations into section header
        NewSectionHeaders[SegNum].PRelocations = FileOffsetData + FileOffset;

        // Put number of relocations into section header
        NewSectionHeaders[SegNum].NRelocations = (uint16)(RelocationTable.GetNumEntries());

        // Put number of relocations into symbol table auxiliary entry.
        // Search for the symbol table entry for this section:
        for (uint32 sym = 0; sym < NewSymbolTable.GetNumEntries(); sym++) {
            if ((uint32)NewSymbolTable[sym].s.SectionNumber == DesiredSegment
                && NewSymbolTable[sym].s.StorageClass == COFF_CLASS_STATIC 
                && NewSymbolTable[sym].s.NumAuxSymbols == 1) {
                    // Found right symbol table entry. Insert NumberOfRelocations
                    NewSymbolTable[sym+1].section.NumberOfRelocations = NewSectionHeaders[SegNum].NRelocations;
                    break;  // No need to search further
            }
        }
    } // End of loop through segments
}


void COMF2COF::CheckUnsupportedRecords() {
    // Make warnings if file containes unsupported record types
    uint32 RecNum;                                // Record number
    uint32 NumComdat = 0;                         // Number of COMDAT records
    uint32 NumComent = 0;                         // Number of COMENT records

    // Loop through all records
    for (RecNum = 0; RecNum < NumRecords; RecNum++) {
        // Check record type
        switch (Records[RecNum].Type2) {
        case OMF_THEADR: case OMF_MODEND: case OMF_EXTDEF: case OMF_PUBDEF:
        case OMF_LNAMES: case OMF_SEGDEF: case OMF_GRPDEF: case OMF_FIXUPP:
        case OMF_LEDATA: case OMF_LIDATA: case OMF_COMDEF: case OMF_VERNUM:
            // These record types are supported or can safely be ignored
            break;

        case OMF_LINNUM: case OMF_LINSYM:
            // Debug records
            cmd.CountDebugRemoved();  break;

        case OMF_COMDAT: case OMF_LCOMDEF: case OMF_CEXTDEF:
            NumComdat++;  break;                    // Count COMDAT records

        case OMF_COMENT: 
            NumComent++;  break;                    // Count COMENT records

        default:                                   // Warning for unknown record type
            err.submit(1212, COMF::GetRecordTypeName(Records[RecNum].Type2));
        }
    }
    // Report number of unsupported sections found
    if (NumComdat) err.submit(2305, NumComdat);
    if (NumComent) err.submit(1211, NumComent);
}


void COMF2COF::MakeBinaryFile() {
    // Putting sections together
    uint32 i;

    // Get number of symbols and sections into file header
    NewFileHeader.NumberOfSymbols = NewSymbolTable.GetNumEntries();
    NewFileHeader.NumberOfSections = NewSectionHeaders.GetNumEntries();

    // Put file header into new file
    ToFile.Push(&NewFileHeader, sizeof(NewFileHeader));

    // Put section headers into new file
    if (NewSectionHeaders.GetNumEntries()) {
        ToFile.Push(&NewSectionHeaders[0], NewSectionHeaders.GetNumEntries() * sizeof(SCOFF_SectionHeader));
    }

    // Put raw data and relocation tables into new file
    ToFile.Push(NewData.Buf(), NewData.GetDataSize());

    // Get address of symbol table into file header
    ToFile.Get<SCOFF_FileHeader>(0).PSymbolTable = ToFile.GetDataSize();

    // Put symbol table into new file
    for (i = 0; i < NewSymbolTable.GetNumEntries(); i++) {
        ToFile.Push(&NewSymbolTable[i], SIZE_SCOFF_SymTableEntry);
    }

    // Insert string table size
    NewStringTable.Get<uint32>(0) = NewStringTable.GetDataSize();

    // Put string table into new file
    ToFile.Push(NewStringTable.Buf(), NewStringTable.GetDataSize());
}
