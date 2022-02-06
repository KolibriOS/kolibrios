/*****************************   omf.h   *************************************
* Author:        Agner Fog
* Date created:  2007-01-29
* Last modified: 2007-01-29
* Project:       objconv
* Module:        omf.h
* Description:
* Header file for definition of data structures and constants in OMF object 
* file format. Also defines class COMFFileBuilder.
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
******************************************************************************
*
* An OMF file consists of a chain of records which all have the same basic
* structure: 
* 1. One byte describing the type of record
* 2. A 16-bit word indicating the length of the rest of the record
* 3. Data of variable types and sizes (max 1024 bytes)
* 4. One byte for checksum. Some systems just set this byte to 0
*
* The OMF format is designed for compactness. All integers of type "index"
* are represented by one byte if no bigger than 127, or by two bytes if
* 128 or more. The most significant bit of the first byte indicates whether
* there are one or two bytes. Integers indicating a segment offset are 16
* bits if the type byte is even, or 32 bits if the type byte is odd.
* Some fields can be left out if they are zero or repeated. The precense
* or absense of a particular field may depend on certain bits in the preceding
* fields. The records cannot be defined as C++ structures because a field
* with variable size or a field that can be absent makes the position of the
* subsequent fields variable.
*
* For these reasons, you will not find any structures defining OMF records
* in this header file. Only the bitfields that are used are defined here. 
* Instead, I have defined the member functions of SOMFRecordPointer for 
* reading fields of various types occurring in OMF records, and the member
* functions of COMFFileBuilder for writing these fields. The structure of 
* an OMF record is simply defined by calling these functions in the right 
* order.
*
* The size of the data field is limited to 1024 bytes because records of type
* FIXUPP have only 10 bits for indexing into the data field of a preceding
* record of type LEDATA or LIDATA. Most tools (but not all!) limit the size
* of all types of records to 1024 bytes of data, although this limitation
* is technically necessary only for LEDATA and LIDATA records. A segment 
* bigger than one kilobyte must be split into several LEDATA records. Each
* LEDATA record is followed by a FIXUPP record if it has relocations.
*
* Symbol names and other text strings are stored with one byte indicating the
* length of the string followed by an ASCII string without terminating zero.
* Consequently, the length of all symbol names is limited to 255 characters.
*
*****************************************************************************/

#ifndef OMF_H
#define OMF_H

//********************** Record types **********************

#define OMF_THEADR   0x80  // Translator Header Record
#define OMF_LHEADR   0x82  // Library Module Header Record
#define OMF_COMENT   0x88  // Comment Record (Including all comment class extensions)
#define OMF_MODEND   0x8A  // (0x8B) Module End Record
#define OMF_EXTDEF   0x8C  // External Names Definition Record
#define OMF_PUBDEF   0x90  // (0x91) Public Names Definition Record
#define OMF_LINNUM   0x94  // (0x95) Line Numbers Record
#define OMF_LNAMES   0x96  // List of Names Record
#define OMF_SEGDEF   0x98  // (0x99) Segment Definition Record
#define OMF_GRPDEF   0x9A  // Group Definition Record
#define OMF_FIXUPP   0x9C  // (0x9D) Fixup Record
#define OMF_LEDATA   0xA0  // (0xA1) Logical Enumerated Data Record
#define OMF_LIDATA   0xA2  // (0xA3) Logical Iterated Data Record
#define OMF_COMDEF   0xB0  // Communal Names Definition Record
#define OMF_BAKPAT   0xB2  // (0xB3) Backpatch Record
#define OMF_LEXTDEF  0xB4  // Local External Names Definition Record
#define OMF_LPUBDEF  0xB6  // (0xB7) Local Public Names Definition Record
#define OMF_LCOMDEF  0xB8  // Local Communal Names Definition Record
#define OMF_CEXTDEF  0xBC  // COMDAT External Names Definition Record
#define OMF_COMDAT   0xC2  // (0xC3) Initialized Communal Data Record
#define OMF_LINSYM   0xC4  // (0xC5) Symbol Line Numbers Record
#define OMF_ALIAS    0xC6  // Alias Definition Record
#define OMF_NBKPAT   0xC8  // (0xC9) Named Backpatch Record
#define OMF_LLNAMES  0xCA  // Local Logical Names Definition Record
#define OMF_VERNUM   0xCC  // OMF Version Number Record
#define OMF_VENDEXT  0xCE  // Vendor-specific OMF Extension Record
#define OMF_LIBHEAD  0xF0  // Library Header Record
#define OMF_LIBEND   0xF1  // Library End Record
#define OMF_LIBEXT   0xF2  // Library extended dictionary


/********************** Relocation types **********************/

#define OMF_Fixup_8bit         0  // 8 bit or low byte of 16 bits
#define OMF_Fixup_16bit        1  // 16 bit offset
#define OMF_Fixup_Segment      2  // 16 bit segment selector
#define OMF_Fixup_Far          3  // 16 bit offset + 16 big segment
#define OMF_Fixup_Hi8bit       4  // High 8 bits of 16 bit offset
#define OMF_Fixup_16bitLoader  5  // 16 bit, loader resolved
#define OMF_Fixup_Pharlab48    6  // 32 bit offset + 16 bit segment, Pharlab only
#define OMF_Fixup_32bit        9  // 32 bit offset
#define OMF_Fixup_Farword     11  // 32 bit offset + 16 bit segment
#define OMF_Fixup_32bitLoader 13  // 32 bit, loader resolved

// Define fixed indexes in LNAMES for default group and class names
#define OMF_LNAME_FLAT         1  // Default group name 
#define OMF_LNAME_CODE         2  // Default class for code
#define OMF_LNAME_DATA         3  // Default class for data
#define OMF_LNAME_BSS          4  // Default class for uninitialized data
#define OMF_LNAME_CONST        5  // Default class for constant data
#define OMF_LNAME_LAST         5  // Last default name. Nondefault names start at OMF_LNAME_LAST + 1
                                  // Class name STACK not used


// Define bitfield structures used in OMF records

union OMF_SAttrib { // Structure of attributes in SEGDEF record
   uint8 b;                            // Byte
   struct {
      uint8 P:1,                       // 0: 16 bit, 1: 32 bit
            B:1,                       // Big
            C:3,                       // Combination (private, public, stack, common)
            A:3;                       // Alignment
   } u;
};

union OMF_SLocat { // Structure of first two bytes of FIXUP subrecord swapped = Locat field
   uint8 bytes[2];                     // First two bytes swapped
   struct {
      uint16 Offset:10,                // Offset into LEDATA (or LIDATA)
             Location:4,               // Relocation method
             M:1,                      // 0 = self-relative, 1 = direct
             one:1;                    // 1 = FIXUP subrecord, 0 = THREAD subrecord
   } s;
};

union OMF_SFixData { // Structure of FixData field in FIXUP subrecord of FIXUPP record
   uint8 b;                            // Byte
   struct {
      uint8  Target:2,                 // Target method (T=0) or target thread number (T=1)
             P:1,                      // 0 = target displacement field present, 1 = displacement is zero
             T:1,                      // 0 = target field present, 1 = target defined by thread
             Frame:3,                  // Frame method (F=0) or frame thread (F=1)
             F:1;                      // 0 = target frame field present, 1 = frame defined by thread
   } s;
};

union OMF_STrdDat { // Structure of Thread Data field in THREAD subrecord of FIXUPP record
   uint8 b;                            // Byte
   struct {
      uint8  Thread:2,                 // Thread number
             Method:3,                 // Method (T0 - T3, F0 - F6)
             Unused:1,                 // 0
             D:1,                      // 0 = Target thread, 1 = Frame thread
             Zero:1;                   // 1 = FIXUP subrecord, 0 = THREAD subrecord
   } s;
};


// Structure of OMF record pointer
struct SOMFRecordPointer {
public:
   uint8  Type;            // Record type
   uint8  Type2;           // Record type, made even
   uint16 Unused;          // Align
   uint32 FileOffset;      // Position in file
   uint32 FileEnd;         // End of file = file size
   uint32 Index;           // Offset to current byte while parsing from start of record
   uint32 End;             // Offset to checksum byte from start of record
   int8 * buffer;          // Pointer to file buffer
   uint8  GetByte();       // Read next byte from buffer
   uint16 GetWord();       // Read next 16 bit word from buffer
   uint32 GetDword();      // Read next 32 bit dword from buffer
   uint32 GetIndex();      // Read byte or word, depending on sign of first byte
   uint32 GetNumeric();    // Read word or dword, depending on record type even or odd
   uint32 GetLength();     // Read 1, 2, 3 or 4 bytes, depending on value of first byte
   char * GetString();     // Read string and return as ASCIIZ string
   void   Start(int8 * Buffer, uint32 FileOffset, uint32 FileEnd); // Start scanning through records
   uint8  GetNext(uint32 align = 0);// Get next record
   uint32 InterpretLIDATABlock(); // Interpret Data block in LIDATA record recursively
   uint32 UnpackLIDATABlock(int8 * destination, uint32 MaxSize); // Unpack Data block in LIDATA record recursively and store data at destination
};


// Class for building OMF files
class COMFFileBuilder : public CFileBuffer {
public:
   COMFFileBuilder();                  // Constructor
   void StartRecord(uint8 type);       // Start building new record
   void EndRecord();                   // Finish building current record
   void PutByte(uint8);                // Put byte into buffer
   void PutWord(uint16);               // Put 16 bit word into buffer
   void PutDword(uint32);              // Put 32 bit dword into buffer
   void PutIndex(uint32);              // Put byte or word into buffer (word if > 127)
   void PutNumeric(uint32);            // Put word or dword into buffer, depending on type being even or odd
   void PutString(const char *);       // Put ASCII string into buffer, preceded by size
   void PutBinary(void *, uint32);     // Put binary data of any length
   uint32 GetSize();                   // Get size of data added so far to current record
protected:
   uint8  Type;                        // Record type
   uint32 Index;                       // Index to current offset
   uint32 RecordStart;                 // Index to start of current record
};


// Structure for temporary segment list used while building OMF file
struct SOMFSegmentList {
public:
   uint32 NewNumber;                   // Segment index in new file
   uint32 OldName;                     // Segment name in old file as index into NameBuffer
   uint32 NewName;                     // Segment name in new file as index into NameBuffer
   uint32 NewNameI;                    // Segment name in new file as index into LNAMES record. Zero for subsequent entries with same segment name
   SCOFF_SectionHeader * psechdr;      // Pointer to old section header
   uint32 Align;                       // Alignment = 2^Align
   uint32 Class;                       // Class in new file
   uint32 Offset;                      // Offset of section in old file to first section with same name
   uint32 Size;                        // Size of section. First record has combined size of all sections with same name
   uint32 SegmentSize;                 // Size of segment = combined size of all sections with same name. Stored only in first section of segment
};


// Structure for temporary symbol list used while building OMF file
struct SOMFSymbolList {
public:
   uint32 Scope;                       // 0 = local, 1 = public, 2 = external
   uint32 NewIndex;                    // PUBDEF index if Scope = 1, EXTDEF index if scope = 2
   uint32 Segment;                     // New segment index
   uint32 Offset;                      // Offset relative to segment = first section with same name
   uint32 Name;                        // Symbol name in new file as index into NameBuffer
};


// Structure for temporary relocation (fixup) list used while building OMF file
struct SOMFRelocation {
public:
   uint32 Section;                     // Section number in old file
   uint32 SourceOffset;                // Offset of source relative to section
   int32  Mode;                        // 0 = EIP-relative, 1 = direct, -1 = unsupported
   uint32 Scope;                       // 0 = local, 2 = external
   uint32 TargetSegment;               // Segment index or EXTDEF index of target in new file 
   uint32 TargetOffset;                // Offset relative to segment in new file of target
   int operator < (SOMFRelocation const & x) const {// operator < for sorting by CSList::Sort()
      return Section < x.Section || (Section == x.Section && SourceOffset < x.SourceOffset);
   }
};

// Structure for assigning names to unnamed local symbols while converting OMF file
struct SOMFLocalSymbol {
   uint32 Offset;                      // Offset into segment
   uint32 Segment;                     // Segment number in old file
   uint32 Name;                        // Assigned name as index into new string table
   uint32 NewSymtabIndex;              // Index into new symbol table
   // Operator < needed for sorting table of SOMFLocalSymbol:
   int operator < (const SOMFLocalSymbol & b) const {
      return Segment < b.Segment || (Segment == b.Segment && Offset < b.Offset);
   }
};

// Structure for interpreted SEGDEF record used during disassembly
struct SOMFSegment {
   uint32 NameO;                       // Segment name, as offset into NameBuffer
   uint32 Offset;                      // Segment address
   uint32 Size;                        // Segment size
   uint32 Align;                       // Alignment = 1 << Align
   uint32 Type;                        // Segment type (as defined in disasm.h)
   uint32 WordSize;                    // 16 or 32 bits
   uint32 BufOffset;                   // Offset of raw data into SegmentData buffer
   uint32 NameIndex;                   // Name index, used for COMDAT segment only
};

#endif // #ifndef OMF_H
