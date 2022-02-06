/****************************  containers.cpp  **********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2016-07-07
* Project:       objconv
* Module:        containers.cpp
* Description:
* Objconv is a portable C++ program for converting object file formats.
* Compile for console mode on any platform.
*
* This module contains container classes CMemoryBuffer and CFileBuffer for
* dynamic memory allocation and file read/write. See containers.h for
* further description.
*
* Copyright 2006-2016 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#include "stdafx.h"

// Names of file formats
SIntTxt FileFormatNames[] = {
    {FILETYPE_COFF,         "COFF"},
    {FILETYPE_OMF,          "OMF"},
    {FILETYPE_ELF,          "ELF"},
    {FILETYPE_MACHO_LE,     "Mach-O Little Endian"},
    {FILETYPE_MACHO_BE,     "Mach-O Big Endian"},
    {FILETYPE_DOS,          "DOS executable"},
    {FILETYPE_WIN3X,        "Windows 3.x executable"},
    {FILETYPE_LIBRARY,      "Function library"},
    {FILETYPE_OMFLIBRARY,   "Function library (OMF)"},
    {IMPORT_LIBRARY_MEMBER, "Windows import library member"},
    {FILETYPE_MAC_UNIVBIN,  "MacIntosh universal binary"},   
    {FILETYPE_MS_WPO,       "Whole program optimization intermediate file, Microsoft specific"},
    {FILETYPE_INTEL_WPO,    "Whole program optimization intermediate file, Intel specific"},
    {FILETYPE_WIN_UNKNOWN,  "Unknown subtype, Windows"},   
    {FILETYPE_ASM,          "Disassembly"}
};


// Members of class CMemoryBuffer
CMemoryBuffer::CMemoryBuffer() {  
    // Constructor
    buffer = 0;
    NumEntries = DataSize = BufferSize = 0;
}

CMemoryBuffer::~CMemoryBuffer() {
    // Destructor
    SetSize(0);                         // De-allocate buffer
}

void CMemoryBuffer::SetSize(uint32 size) {
    // Allocate, reallocate or deallocate buffer of specified size.
    // DataSize is initially zero. It is increased by Push or PushString.
    // Setting size > DataSize will allocate more buffer and fill it with zeroes but not increase DataSize.
    // Setting size < DataSize will decrease DataSize so that some of the data are discarded.
    // Setting size = 0 will discard all data and de-allocate the buffer.
    if (size == 0) {
        // Deallocate
        if (buffer) delete[] buffer;     // De-allocate buffer
        buffer = 0;
        NumEntries = DataSize = BufferSize = 0;
        return;
    }
    if (size < DataSize) {
        // Request to delete some data
        DataSize = size;
        return;
    }
    if (size <= BufferSize) {
        // Request to reduce size but not delete it
        return;                          // Ignore
    }
//  size = (size + 15) & uint32(-16);   // Round up size to value divisible by 16
    size = (size + BufferSize + 15) & uint32(-16);   // Double size and round up to value divisible by 16
    int8 * buffer2 = 0;                 // New buffer
    buffer2 = new int8[size];           // Allocate new buffer
    if (buffer2 == 0) {err.submit(9006); return;} // Error can't allocate
    memset (buffer2, 0, size);          // Initialize to all zeroes
    if (buffer) {
        // A smaller buffer is previously allocated
        memcpy (buffer2, buffer, BufferSize); // Copy contents of old buffer into new
        delete[] buffer;                 // De-allocate old buffer
    }
    buffer = buffer2;                   // Save pointer to buffer
    BufferSize = size;                  // Save size
}

uint32 CMemoryBuffer::Push(void const * obj, uint32 size) {
    // Add object to buffer, return offset
    // Parameters: 
    // obj = pointer to object, 0 if fill with zeroes
    // size = size of object to push

    // Old offset will be offset to new object
    uint32 OldOffset = DataSize;

    // New data size will be old data size plus size of new object
    uint32 NewOffset = DataSize + size;

    if (NewOffset > BufferSize) {
        // Buffer too small, allocate more space.
        // We can use SetSize for this only if it is certain that obj is not 
        // pointing to an object previously allocated in the old buffer
        // because it would be deallocated before copied into the new buffer:
        // SetSize (NewOffset + NewOffset / 2 + 1024);

        // Allocate more space without using SetSize:
        // Double the size + 1 kB, and round up size to value divisible by 16
        uint32 NewSize = (NewOffset * 2 + 1024 + 15) & uint32(-16);
        int8 * buffer2 = 0;                        // New buffer
        // Allocate new buffer
        buffer2 = new int8[NewSize];
        if (buffer2 == 0) {
            // Error can't allocate
            err.submit(9006);  return 0;
        }
        // Initialize to all zeroes
        memset (buffer2, 0, NewSize);
        if (buffer) {
            // A smaller buffer is previously allocated
            // Copy contents of old buffer into new
            memcpy (buffer2, buffer, BufferSize);
        }
        BufferSize = NewSize;                      // Save size
        if (obj && size) {                         
            // Copy object to new buffer
            memcpy (buffer2 + OldOffset, obj, size);
            obj = 0;                                // Prevent copying once more
        }
        // Delete old buffer after copying object
        if (buffer) delete[] buffer;

        // Save pointer to new buffer
        buffer = buffer2;
    }
    // Copy object to buffer if nonzero
    if (obj && size) {
        memcpy (buffer + OldOffset, obj, size);
    }
    if (size) {
        // Adjust new offset
        DataSize = NewOffset;
        NumEntries++;
    }
    // Return offset to allocated object
    return OldOffset;
}

uint32 CMemoryBuffer::PushString(char const * s) {
    // Add ASCIIZ string to buffer, return offset
    return Push (s, uint32(strlen(s))+1);
}

uint32 CMemoryBuffer::GetLastIndex() {
    // Index of last object pushed (zero-based)
    return NumEntries - 1;
}

void CMemoryBuffer::SetDataSize(uint32 size) {
    if (size > BufferSize) {
        // Allocate more space
        SetSize(size + 2048);
    }
    // Set DataSize to after alignment space
    DataSize = size;
}

void CMemoryBuffer::Align(uint32 a) {
    // Align next entry to address divisible by a
    SetDataSize((DataSize + a - 1) / a * a);
}

// Members of class CFileBuffer
CFileBuffer::CFileBuffer() : CMemoryBuffer() {  
    // Default constructor
    FileName = 0;
    OutputFileName = 0;
    FileType = WordSize = Executable = 0;
}

CFileBuffer::CFileBuffer(char const * filename) : CMemoryBuffer() {  
    // Constructor
    FileName = filename;
    FileType = WordSize = 0;
}

void CFileBuffer::Read(int IgnoreError) {                   
    // Read file into buffer
    uint32 status;                             // Error status

#ifdef _MSC_VER  // Microsoft compiler prefers this:

    int fh;                                    // File handle
    fh = _open(FileName, O_RDONLY | O_BINARY); // Open file in binary mode
    if (fh == -1) {
        // Cannot read file
        if (!IgnoreError) err.submit(2103, FileName); // Error. Input file must be read
        SetSize(0); return;                     // Make empty file buffer
    }
    DataSize = filelength(fh);                 // Get file size
    if (DataSize <= 0) {
        if (!IgnoreError) err.submit(2105, FileName); // Wrong size
        return;}
    SetSize(DataSize + 2048);                  // Allocate buffer, 2k extra
    status = _read(fh, Buf(), DataSize);       // Read from file
    if (status != DataSize) err.submit(2103, FileName);
    status = _close(fh);                       // Close file
    if (status != 0) err.submit(2103, FileName);

#else            // Works with most compilers:

    FILE * fh = fopen(FileName, "rb");
    if (!fh) {
        // Cannot read file
        if (!IgnoreError) err.submit(2103, FileName); // Error. Input file must be read
        SetSize(0); return;                     // Make empty file buffer
    }
    // Find file size
    fseek(fh, 0, SEEK_END);
    long int fsize = ftell(fh);
    if (fsize <= 0 || (unsigned long)fsize >= 0xFFFFFFFF) {
        // File too big or zero size
        err.submit(2105, FileName); fclose(fh); return;
    }
    DataSize = (uint32)fsize;
    rewind(fh);
    // Allocate buffer
    SetSize(DataSize + 2048);                 // Allocate buffer, 2k extra
    // Read entire file
    status = (uint32)fread(Buf(), 1, DataSize, fh);
    if (status != DataSize) err.submit(2103, FileName);
    status = fclose(fh);
    if (status != 0) err.submit(2103, FileName);

#endif
}

void CFileBuffer::Write() {                  
    // Write buffer to file:
    if (OutputFileName) FileName = OutputFileName;
    // Two alternative ways to write a file:

#ifdef _MSC_VER       // Microsoft compiler prefers this:

    int fh;                                       // File handle
    uint32 status;                                // Error status
    // Open file in binary mode
    fh = _open(FileName, O_RDWR | O_BINARY | O_CREAT | O_TRUNC, _S_IREAD | _S_IWRITE); 
    // Check if error
    if (fh == -1) {err.submit(2104, FileName);  return;}
    // Write file
    status = _write(fh, Buf(), DataSize);
    // Check if error
    if (status != DataSize) err.submit(2104, FileName);
    // Close file
    status = _close(fh);
    // Check if error
    if (status != 0) err.submit(2104, FileName);

#else                // Works with most compilers:

    // Open file in binary mode
    FILE * ff = fopen(FileName, "wb");
    // Check if error
    if (!ff) {err.submit(2104, FileName);  return;}
    // Write file
    uint32 n = (uint32)fwrite(Buf(), 1, DataSize, ff);
    // Check if error
    if (n != DataSize) err.submit(2104, FileName);
    // Close file
    n = fclose(ff);
    // Check if error
    if (n) {err.submit(2104, FileName);  return;}

#endif
}

int CFileBuffer::GetFileType() {
    // Detect file type
    if (FileType) return FileType;            // File type already known
    if (!DataSize) return 0;                  // No file
    if (!Buf()) return 0;                     // No contents

    uint32 namelen = FileName ? (uint32)strlen(FileName) : 0;

    if (strncmp(Buf(),"!<arch>",7) == 0) {
        // UNIX style library. Contains members of file type COFF, ELF or MACHO
        FileType = FILETYPE_LIBRARY;
    }
    else if (strncmp(Buf(),ELFMAG,4) == 0) {
        // ELF file
        FileType = FILETYPE_ELF;
        Executable = Get<Elf32_Ehdr>(0).e_type != ET_REL;
        switch (Buf()[EI_CLASS]) {
        case ELFCLASS32:
            WordSize = 32; break;
        case ELFCLASS64:
            WordSize = 64; break;
        }
    }
    else if (Get<uint32>(0) == MAC_MAGIC_32) {
        // Mach-O 32 little endian
        FileType = FILETYPE_MACHO_LE;
        WordSize = 32;
        Executable = Get<MAC_header_32>(0).filetype != MAC_OBJECT;
    }
    else if (Get<uint32>(0) == MAC_MAGIC_64) {
        // Mach-O 64 little endian
        FileType = FILETYPE_MACHO_LE;
        WordSize = 64;
        Executable = Get<MAC_header_64>(0).filetype != MAC_OBJECT;
    }
    else if (Get<uint32>(0) == MAC_CIGAM_32) {
        // Mach-O 32 big endian
        FileType = FILETYPE_MACHO_BE;
        WordSize = 32;
    }
    else if (Get<uint32>(0) == MAC_CIGAM_64) {
        // Mach-O 64 big endian
        FileType = FILETYPE_MACHO_BE;
        WordSize = 64;
    }
    else if (Get<uint32>(0) == MAC_CIGAM_UNIV) {
        // MacIntosh universal binary
        FileType = FILETYPE_MAC_UNIVBIN;
        WordSize = 0;
    }   
    else if (Get<uint32>(0) == 0xFFFF0000 || Get<uint32>(0) == 0x10000) {
        // Windows subtypes:
        if (Get<uint16>(4) == 0) {
            // This type only occurs when attempting to extract a member from an import library
            FileType = IMPORT_LIBRARY_MEMBER;
        }
        else if (Get<uint16>(4) == 1) {
            // Whole program optimization intermediate file for MS compiler. Undocumented
            FileType = FILETYPE_MS_WPO;
        }
        else {
            // Other subtypes not known
            FileType = FILETYPE_WIN_UNKNOWN;
        }
        // Get word size
        if (Get<uint16>(6) == PE_MACHINE_I386) {
            WordSize = 32;
        }
        else if (Get<uint16>(6) == PE_MACHINE_X8664) {
            WordSize = 64;
        }
        else {
            WordSize = 0;
        }
    }
    else if (Get<uint16>(0) == PE_MACHINE_I386) {
        // COFF/PE 32
        FileType = FILETYPE_COFF;
        WordSize = 32;
        Executable = (Get<SCOFF_FileHeader>(0).Flags & PE_F_EXEC) != 0;
    }
    else if (Get<uint16>(0) == PE_MACHINE_X8664) {
        // COFF64/PE32+
        FileType = FILETYPE_COFF;
        WordSize = 64;
        Executable = (Get<SCOFF_FileHeader>(0).Flags & PE_F_EXEC) != 0;
    }
    else if (Get<uint8>(0) == OMF_THEADR) {
        // OMF 16 or 32
        FileType = FILETYPE_OMF;
        // Word size can only be determined by searching through records in file:
        GetOMFWordSize(); // Determine word size
    }
    else if (Get<uint8>(0) == OMF_LIBHEAD) {
        // OMF Library 16 or 32
        FileType = FILETYPE_OMFLIBRARY;
    }
    else if ((Get<uint16>(0) & 0xFFF9) == 0x5A49) {
        // DOS file or file with DOS stub
        FileType = FILETYPE_DOS;
        WordSize = 16;
        Executable = 1;
        uint32 Signature = Get<uint32>(0x3C);
        if (Signature + 8 < DataSize) {
            if (Get<uint16>(Signature) == 0x454E) {
                // Windows 3.x file
                FileType = FILETYPE_WIN3X;
            }
            else if (Get<uint16>(Signature) == 0x4550) {
                // COFF file
                uint16 MachineType = Get<uint16>(Signature + 4);
                if (MachineType == PE_MACHINE_I386) {
                    FileType = FILETYPE_COFF;
                    WordSize = 32;
                }
                else if (MachineType == PE_MACHINE_X8664) {
                    FileType = FILETYPE_COFF;
                    WordSize = 64;
                }
            }
        }
    }
    else if (namelen > 4 && stricmp(FileName + namelen - 4, ".com") == 0) {
        // DOS .com file recognized only from its extension
        FileType = FILETYPE_DOS;
        WordSize = 16;  Executable = 1;
    }
    else if (Get<uint16>(0) == 0 && namelen > 4 && stricmp(FileName + namelen - 4, ".obj") == 0) {
        // Possibly alias record in COFF library
        FileType = FILETYPE_COFF;
        WordSize = 0;
        Executable = 0;
    }
    else {
        // Unknown file type
        int utype = Get<uint32>(0);        
        err.submit(2018, utype, FileName); 
        FileType = 0;
    }
    return FileType;
}


char const * CFileBuffer::GetFileFormatName(int FileType) {
    // Get name of file format type
    return Lookup (FileFormatNames, FileType);
}


void CFileBuffer::SetFileType(int type) {
    // Set file format type
    FileType = type;
}

void CFileBuffer::Reset() {
    // Set all members to zero
    SetSize(0);                          // Deallocate memory buffer
    memset(this, 0, sizeof(*this));
}

char * CFileBuffer::SetFileNameExtension(const char * f) {
    // Set file name extension according to FileType
    static char name[MAXFILENAMELENGTH+8];
    int i;

    if (strlen(f) > MAXFILENAMELENGTH) err.submit(2203, f);
    strncpy(name, f, MAXFILENAMELENGTH);

    // Search for last '.' in file name
    for (i = (int)strlen(name)-1; i > 0; i--) if (name[i] == '.') break;
    if (i < 1) {
        // '.' not found. Append '.' to name
        i = (int)strlen(name); if (i > MAXFILENAMELENGTH-4) i = MAXFILENAMELENGTH-4;
    }
    // Get default extension
    if (cmd.OutputType == FILETYPE_ASM) {
        strcpy(name+i, ".asm"); // Assembly file
    }
    else if (cmd.OutputType == FILETYPE_COFF || cmd.OutputType == FILETYPE_OMF) {
        if ((FileType & (FILETYPE_LIBRARY | FILETYPE_OMFLIBRARY)) || (cmd.LibraryOptions & CMDL_LIBRARY_ADDMEMBER)) {
            strcpy(name+i, ".lib"); // Windows function library
        }
        else {
            strcpy(name+i, ".obj"); // Windows object file
        }
    }
    else { // output type is ELF or MACHO
        if ((FileType & (FILETYPE_LIBRARY | FILETYPE_OMFLIBRARY)) || (cmd.LibraryOptions & CMDL_LIBRARY_ADDMEMBER)) {
            strcpy(name+i, ".a");   // Linux/BSD/Mac function library
        }
        else {
            strcpy(name+i, ".o");   // Linux/BSD/Mac object file
        }
    }
    return name;
}

void CFileBuffer::CheckOutputFileName() {
    // Make output file name or check that requested name is valid
    if (!(cmd.FileOptions & CMDL_FILE_OUTPUT)) return;

    OutputFileName = cmd.OutputFile;
    if (OutputFileName == 0) {
        // Output file name not specified. Make filename
        OutputFileName = cmd.OutputFile = SetFileNameExtension(FileName);
    }
    if (strcmp(FileName,OutputFileName) == 0 && !(cmd.FileOptions & CMDL_FILE_IN_OUT_SAME)) {
        // Input and output files have same name
        err.submit(2005, FileName);
    }
}

void operator >> (CFileBuffer & a, CFileBuffer & b) {
    // Transfer ownership of buffer and other properties from a to b
    b.SetSize(0);                            // De-allocate old buffer from target if it has one
    b.buffer = a.buffer;                     // Transfer buffer
    a.buffer = 0;                            // Remove buffer from source, so that buffer has only one owner

    // Copy properties
    b.DataSize   = a.GetDataSize();          // Size of data, offset to vacant space
    b.BufferSize = a.GetBufferSize();        // Size of allocated buffer
    b.NumEntries = a.GetNumEntries();        // Number of objects pushed
    b.Executable = a.Executable;             // File is executable
    if (a.WordSize) b.WordSize = a.WordSize; // Segment word size (16, 32, 64)
    if (a.FileName) b.FileName = a.FileName; // Name of input file
    if (a.OutputFileName) b.OutputFileName = a.OutputFileName;// Name of output file
    if (a.GetFileType())  b.FileType = a.GetFileType();       // Object file type
    a.SetSize(0);                            // Reset a's properties
}

void CFileBuffer::GetOMFWordSize() {
    // Determine word size for OMF file.
    // There is no simple way to get the word size. Looking for odd-numbered
    // record types is not sufficient. A 32-bit OMF file may use 16-bit SEGDEF 
    // records. We have to look for segments with the 'P' attribute set. And
    // even this is not completely safe, because MASM may generate empty 32-bit
    // segments so we have to look only at segments with nonzero size. 
    // We can still have any mixture of 16- and 32-bit segments, though, so no
    // method is absolutely safe.

    // We have to parse through all records in file buffer
    uint8  RecordType;                            // Type of current record
    uint32 RecordStart;                           // Index to start of current record
    uint32 RecordEnd;                             // Index to end of current record
    uint32 RecordLength;                          // Length of current record
    uint32 Index = 0;                             // Current offset from buffer while reading
    OMF_SAttrib SegAttr;                          // Segment attributed
    uint32 SegLength;                             // Segment length

    WordSize = 16;                                // WordSize = 16 if no 32 bit records found

    while (Index < GetDataSize()) {
        RecordStart = Index;                       // Record starts here
        RecordType = Get<uint8>(Index++);          // Get first byte of record = type
        RecordLength = Get<uint16>(Index);         // Next two bytes = length
        Index += 2;
        RecordEnd = RecordStart + RecordLength + 3;// End of record
        if (RecordEnd > GetDataSize()) {
            // Record goes beyond end of file
            err.submit(2301);  break;
        }
        if ((RecordType & 1) && RecordType < OMF_LIBHEAD) { // Odd-numbered type means 32 bit
            WordSize = 32;                                   // ..but this method is not safe
        }
        if ((RecordType & 0xFE) == OMF_SEGDEF) {   // Segment definition record
            SegAttr.b = Get<uint8>(Index++);        // Get segment attributes
            if (SegAttr.u.A == 0) {
                // Frame and Offset only included if A = 0
                Index += 2+1;
            }
            SegLength = (RecordType & 1) ? Get<uint32>(Index) : Get<uint16>(Index); // Segment length

            if (SegAttr.u.P && SegLength) {         // if segment has P attribute and nonzero length
                WordSize = 32;                       // .. then it is a 32-bit segment
            }
        }
        Index = RecordEnd;                         // Point to next record
    }
}


// Class CTextFileBuffer is used for building text files
// Constructor
CTextFileBuffer::CTextFileBuffer() {
    column = 0;
    // Use UNIX linefeeds only if GASM output
    LineType = (cmd.SubType == SUBTYPE_GASM) ? 1 : 0;
}

void CTextFileBuffer::Put(const char * text) {
    // Write text string to buffer
    uint32 len = (uint32)strlen(text);            // Length of text
    Push(text, len);                              // Add to buffer without terminating zero
    column += len;                                // Update column
}

void CTextFileBuffer::Put(const char character) {
    // Write single character to buffer
    Push(&character, 1);                          // Add to buffer
    column ++;                                    // Update column
}

void CTextFileBuffer::NewLine() {
    // Add linefeed
    if (LineType == 0) {
        Push("\r\n", 2);                           // DOS/Windows style linefeed
    }
    else {
        Push("\n", 1);                             // UNIX style linefeed
    }
    column = 0;                                   // Reset column
}

void CTextFileBuffer::Tabulate(uint32 i) {
    // Insert spaces until column i
    uint32 j;
    if (i > column) {                             // Only insert spaces if we are not already past i
        for (j = column; j < i; j++) Push(" ", 1); // Insert i - column spaces
        column = i;                                // Update column
    }
}

void CTextFileBuffer::PutDecimal(int32 x, int IsSigned) {
    // Write decimal number to buffer, unsigned or signed
    char text[16];
    sprintf(text, IsSigned ? "%i" : "%u", x);
    Put(text);
}

void CTextFileBuffer::PutHex(uint8 x, int MasmForm) {
    // Write hexadecimal 8 bit number to buffer
    // If MasmForm >= 1 then the function will write the number in a
    // way that can be read by the assembler, e.g. 0FFH or 0xFF
    char text[16];
    if (MasmForm && cmd.SubType == SUBTYPE_GASM) {
        // Needs 0x prefix
        sprintf(text, "0x%02X", x);
        Put(text);
        return;
    }
    if (MasmForm && x >= 0xA0) {
        Put("0");                                  // Make sure it doesn't begin with a letter
    }
    sprintf(text, "%02X", x);
    Put(text);
    if (MasmForm) Put("H");
}

void CTextFileBuffer::PutHex(uint16 x, int MasmForm) {
    // Write hexadecimal 16 bit number to buffer
    // If MasmForm >= 1 then the function will write the number in a
    // way that can be read by the assembler, e.g. 0FFH or 0xFF
    // If MasmForm == 2 then leading zeroes are stripped
    char text[16];
    if (MasmForm && cmd.SubType == SUBTYPE_GASM) {
        // Needs 0x prefix
        sprintf(text, MasmForm==1 ? "0x%04X" : "0x%X", x);
        Put(text);
        return;
    }
    sprintf(text, (MasmForm < 2) ? "%04X" : "%X", x);
    // Check if leading zero needed
    if (MasmForm && text[0] > '9') {
        Put("0");                                  // Leading zero needed
    }
    Put(text);
    if (MasmForm) Put("H");
}

void CTextFileBuffer::PutHex(uint32 x, int MasmForm) {
    // Write hexadecimal 32 bit number to buffer
    // If MasmForm >= 1 then the function will write the number in a
    // way that can be read by the assembler, e.g. 0FFH or 0xFF
    // If MasmForm == 2 then leading zeroes are stripped
    char text[16];
    if (MasmForm && cmd.SubType == SUBTYPE_GASM) {
        // Needs 0x prefix
        sprintf(text, MasmForm==1 ? "0x%08X" : "0x%X", x);
        Put(text);
        return;
    }

    sprintf(text, (MasmForm < 2) ? "%08X" : "%X", x);
    // Check if leading zero needed
    if (MasmForm && text[0] > '9') {
        Put("0");                                  // Leading zero needed
    }
    Put(text);
    if (MasmForm) Put("H");
}

void CTextFileBuffer::PutHex(uint64 x, int MasmForm) {
    // Write unsigned hexadecimal 64 bit number to buffer
    // If MasmForm >= 1 then the function will write the number in a
    // way that can be read by the assembler, e.g. 0FFH or 0xFF
    // If MasmForm == 2 then leading zeroes are stripped
    char text[32];
    if (MasmForm < 2) {  // Print all digits
        sprintf(text, "%08X%08X", HighDWord(x), uint32(x));
    }
    else { // Skip leading zeroes
        if (HighDWord(x)) {
            sprintf(text, "%X%08X", HighDWord(x), uint32(x));
        }
        else {
            sprintf(text, "%X", uint32(x));
        }
    }
    if (MasmForm) {
        if (cmd.SubType == SUBTYPE_GASM) {
            // Needs 0x prefix
            Put("0x");
            Put(text);
        }
        else {
            // use 0FFH form
            if (text[0] > '9')  Put("0");           // Leading zero needed
            Put(text);
            Put("H");
        }
    }
    else {
        // write hexadecimal number only
        Put(text);
    }
}

void CTextFileBuffer::PutFloat(float x) {
    // Write floating point number to buffer
    char text[64];
    sprintf(text, "%.7G", x);
    Put(text);
}

void CTextFileBuffer::PutFloat(double x) {
    // Write floating point number to buffer
    char text[64];
    sprintf(text, "%.16G", x);
    Put(text);
}
