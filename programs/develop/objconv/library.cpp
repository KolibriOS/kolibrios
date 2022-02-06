/****************************  library.cpp  **********************************
* Author:        Agner Fog
* Date created:  2006-08-27
* Last modified: 2017-07-27
* Project:       objconv
* Module:        library.cpp
* Description:
* This module contains code for reading, writing and manipulating function
* libraries (archives) of the UNIX type and OMF type.
*
* Copyright 2006-2017 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#include "stdafx.h"

// OMF Library flag names
SIntTxt OMFLibraryFlags[] = {
    {0,      "Flag = 0"},
    {1,      "Case sensitive"}
};


CLibrary::CLibrary() {
    // Constructor
    CurrentOffset = 0;
    CurrentNumber = 0;
    LongNames = 0;
    LongNamesSize = 0;
    AlignBy = 0;
    MemberFileType = 0;
    RepressWarnings = 0;
    PageSize = 16;
}


void CLibrary::Go() {
    // Do to library whatever the command line says
    char const * MemberName1 = 0;  // Name of library member
    char const * MemberName2 = 0;  // Modified name of library member
    int action = 0;                // Action to take on member
    int FileType1 = 0;             // File type of current member
    int WordSize1 = 0;             // Word size of current member

    if (cmd.DumpOptions && !(cmd.LibraryOptions & CMDL_LIBRARY_EXTRACTMEM)) {
        // Dump library, but not its members
        Dump();
        return;
    }

    // Remove path form member names and check member type before extracting or adding members
    AlignBy = 2;
    if (GetDataSize()) FixNames();    

    if (err.Number()) return; // Stop if error

    // check LibraryOptions and whether an output file is specified
    if (cmd.FileOptions & CMDL_FILE_OUTPUT) {
        // Output is a library file
        if ((cmd.OutputFile == 0 || cmd.OutputFile[0] == 0) && !(cmd.LibraryOptions & CMDL_LIBRARY_ADDMEMBER)) {
            err.submit(2503); // Output file name missing
            return;
        }
        // Check extension
        if (strstr(cmd.OutputFile, ".lib") && strstr(cmd.OutputFile, ".LIB") && strstr(cmd.OutputFile, ".a")) {
            err.submit(1101); // Warning wrong extension
        }
        if (cmd.OutputType >= IMPORT_LIBRARY_MEMBER) {
            // Wrong output type
            if (cmd.OutputType == FILETYPE_ASM) {
                // Attempt to disassemble whole library
                err.submit(2620);
            }
            else {
                err.submit(2621);
            }
            return;
        }
    }

    // Desired alignment = 2 for COFF and ELF, 8 for Mach-O
    AlignBy = 2;
    if (cmd.OutputType == FILETYPE_MACHO_LE) AlignBy = 8;

    // Determine library type and subtype
    if (cmd.LibrarySubtype == 0) {    
        switch (cmd.OutputType) {
        case FILETYPE_OMF: case FILETYPE_OMFLIBRARY:
            cmd.LibrarySubtype = LIBTYPE_OMF;  break;
        case FILETYPE_COFF: case FILETYPE_DOS: 
            cmd.LibrarySubtype = LIBTYPE_WINDOWS;  break;
        case FILETYPE_ELF:
            cmd.LibrarySubtype = LIBTYPE_LINUX;  break;
        case FILETYPE_MACHO_LE: case FILETYPE_MACHO_BE:
            cmd.LibrarySubtype = LIBTYPE_BSD_MAC;  break;
        case FILETYPE_LIBRARY:
            switch (cmd.InputType) {
            case FILETYPE_COFF: case FILETYPE_DOS: 
                cmd.LibrarySubtype = LIBTYPE_WINDOWS;  break;
            case FILETYPE_ELF:
                cmd.LibrarySubtype = LIBTYPE_LINUX;  break;
            case FILETYPE_MACHO_LE: case FILETYPE_MACHO_BE:
                cmd.LibrarySubtype = LIBTYPE_BSD_MAC;  break;
            }
            break;
        default:
            cmd.LibrarySubtype = LIBTYPE_SHORTNAMES;
        }
    }

    // Reserve space for data buffer
    DataBuffer.SetSize(GetBufferSize());

    // Check options
    if (!cmd.LibraryOptions) cmd.LibraryOptions = CMDL_LIBRARY_CONVERT;

    if (cmd.Verbose) {
        // Tell what we are doing
        if ((cmd.LibraryOptions & CMDL_LIBRARY_ADDMEMBER) && GetDataSize() == 0) {
            // Creating library
            printf("\nCreating library %s (%s)", FileName, GetFileFormatName(cmd.OutputType));
        }
        else if (OutputFileName) {
            // Print input file name
            printf("\nInput library: %s", FileName);
            if (cmd.InputType != cmd.OutputType) {
                // Converting library type. Print input file type
                int InType = cmd.InputType; 
                if (InType == FILETYPE_LIBRARY || InType == FILETYPE_OMFLIBRARY) InType = cmd.MemberType;
                printf(", Format: %s", GetFileFormatName(InType));
                if (cmd.DesiredWordSize) printf("%i", cmd.DesiredWordSize);
            }
            // Print output file name and type
            printf(", Output: %s, Format: %s", OutputFileName, GetFileFormatName(cmd.OutputType));
            if (cmd.DesiredWordSize) printf("%i", cmd.DesiredWordSize);
        }
        else {
            printf("\nExtracting from library file: %s", FileName);
        }
    }

    // Convert library or extract or add or dump all members
    StartExtracting();                           // Initialize before ExtractMember()

    // Loop through input library
    while ((MemberName1 = ExtractMember(&MemberBuffer)) != 0) {

        // Check if any specific action required for this member
        action = cmd.SymbolChange(MemberName1, &MemberName2, SYMT_LIBRARYMEMBER);
        /*
        if ((action != SYMA_CHANGE_NAME && action != SYMA_EXTRACT_MEMBER) || MemberName2 == 0) {
            MemberName2 = MemberName1; // No name change
        } */
        MemberBuffer.FileName = MemberName1;
        MemberBuffer.OutputFileName = MemberName2 ? MemberName2 : MemberName1;

        if (action == SYMA_DELETE_MEMBER) {
            // Remove this member from library
            if (cmd.Verbose) {
                printf("\nRemoving member %s from library", MemberName1);
            }
            continue;
        }
        if (action == SYMA_ADD_MEMBER) {
            // Replace this member with new file
            // (Message comes later when adding new member)
            continue;
        }

        // Check type of this member
        FileType1 = MemberBuffer.GetFileType();
        if (FileType1 == 0) continue;
        WordSize1 = MemberBuffer.WordSize;

        if (!(cmd.LibraryOptions & (CMDL_LIBRARY_EXTRACTMEM | CMDL_LIBRARY_ADDMEMBER))) {
            // Not adding or extracting members. Apply conversion options to all members
            if (cmd.SymbolChangesRequested() || FileType1 != cmd.OutputType) {

                // Check file type before conversion
                int FileType0 = MemberBuffer.GetFileType();
                // Conversion or name change requested
                MemberBuffer.Go();                   // Do required conversion
                if (err.Number()) break;             // Stop if error
                // Check type again after conversion
                FileType1 = MemberBuffer.GetFileType();
                if (MemberBuffer.OutputFileName == 0 || FileType1 != FileType0) {
                    MemberBuffer.OutputFileName = MemberBuffer.SetFileNameExtension(MemberBuffer.FileName);
                }
                MemberBuffer.FileName = MemberBuffer.OutputFileName;
            }
        }
        if (MemberFileType == 0) MemberFileType = FileType1;
        if (WordSize == 0) WordSize = WordSize1;
        if (FileType1 != MemberFileType || WordSize1 != WordSize) {
            if (WordSize1 == 0) {
                // Library member has no wordsize
                err.submit(1109, MemberBuffer.FileName);
            }
            else {
                // Library members have different type or word size
                err.submit(1102); 
            }
        }

        if (cmd.LibraryOptions & CMDL_LIBRARY_EXTRACTMEM) {
            // Extract member(s)
            if (action == SYMA_EXTRACT_MEMBER || cmd.LibraryOptions == CMDL_LIBRARY_EXTRACTALL) {
                // Extract this member
                if (cmd.DumpOptions == 0 && cmd.OutputType != CMDL_OUTPUT_DUMP) {
                    // Write this member to file
                    if (err.Number()) return; // Check first if error

                    if (cmd.SymbolChangesRequested() || FileType1 != cmd.OutputType) {
                        // Conversion or name change requested

                        // Check type before conversion
                        int FileType0 = MemberBuffer.GetFileType();
                        MemberBuffer.Go();
                        if (err.Number()) return; // Stop if error
                        // Check type after conversion
                        FileType1 = MemberBuffer.GetFileType();
                        if (MemberBuffer.OutputFileName == 0 /*|| FileType1 != FileType0*/) {
                            MemberBuffer.OutputFileName = MemberBuffer.SetFileNameExtension(MemberBuffer.FileName);
                        }
                    }
                    if (MemberBuffer.OutputFileName == 0) {
                        MemberBuffer.OutputFileName = MemberBuffer.FileName;
                    }
                    if (cmd.Verbose) {
                        // Tell what we are doing
                        if (MemberName1 == MemberName2) {
                            printf("\nExtracting file %s from library", MemberName1);
                        }
                        else {
                            printf("\nExtracting library member %s to file %s", MemberName1, MemberBuffer.OutputFileName);
                        }
                    }
                    if (WordSize1 == 0) {
                        err.submit(1109, MemberName1);
                    }
                    // Write this member to file
                    MemberBuffer.Write();
                }
                else {
                    // Dump this member
                    MemberBuffer.Go();
                }
            }
        }
        else if (cmd.DumpOptions == 0) {
            // Add member to new library
            if (MemberName2 == 0) MemberName2 = MemberName1;
            if (cmd.Verbose) {
                // Tell what we are doing
                if (strcmp(MemberName1, MemberName2) != 0) {
                    printf("\nRenaming member %s to %s", MemberName1, MemberName2);
                }
            }
            // Put into new library
            InsertMember(&MemberBuffer);
        }
    } // End of loop through library
    // Stop if error
    if (err.Number()) return; 

    if (cmd.LibraryOptions & CMDL_LIBRARY_ADDMEMBER) {
        // Add object files to library
        SSymbolChange const * sym;
        // Loop through file names to add
        while ((sym = cmd.GetMemberToAdd()) != 0) {
            MemberBuffer.Reset();                     // Reset MemberBuffer
            // Name of object file
            MemberBuffer.FileName = sym->Name2;       // Name of object file
            MemberBuffer.OutputFileName = sym->Name1; // Name of new member
            // Read object file
            MemberBuffer.Read();
            // Stop if read failed
            if (err.Number()) continue; 
            // Detect file type
            int NewMemberType = MemberBuffer.GetFileType();
            if (cmd.Verbose) {
                // Tell what we are doing
                if (sym->Done) {
                    printf("\nReplacing member %s with file %s", sym->Name1, sym->Name2);
                }
                else {
                    printf("\nAdding member %s from file %s", sym->Name1, sym->Name2);
                }
                if (NewMemberType != cmd.OutputType) {
                    // Converting type
                    printf(". Converting from %s.", GetFileFormatName(NewMemberType));
                }
            }
            // Do any conversion required
            MemberBuffer.Go();

            // Stop if error
            if (err.Number()) continue; 

            // Check if file type is right after conversion
            MemberFileType = MemberBuffer.FileType;
            if (WordSize == 0) WordSize = MemberBuffer.WordSize;
            if (MemberFileType != cmd.OutputType) {
                // Library members have different type
                err.submit(2504, GetFileFormatName(MemberBuffer.FileType)); continue; 
            }
            if (MemberBuffer.WordSize != WordSize) {
                // Library members have different word size
                err.submit(2505, MemberBuffer.WordSize); continue; 
            }
            // Put into library
            MemberBuffer.FileName = MemberBuffer.OutputFileName;
            InsertMember(&MemberBuffer);
        } // End of loop through object file names
    } 
    // Stop if error
    if (err.Number()) return; 

    if (cmd.LibraryOptions & CMDL_LIBRARY_EXTRACTMEM) {
        cmd.CheckExtractSuccess();  // Check if members to extract were found
    }

    if (cmd.FileOptions & CMDL_FILE_OUTPUT) {
        // Make output library
        MakeBinaryFile();
        // Take over OutFile buffer
        *this << OutFile;
    }
    else {
        // No output library
        OutputFileName = 0;
    }
}

void CLibrary::FixNames() {
    // Rebuild library or fix member names
    // Dispatch according to library type
    switch (cmd.InputType) {
    case FILETYPE_OMF: case FILETYPE_OMFLIBRARY:
        // Rebuild OMF style library and fix long member names
        RebuildOMF();  break;

    case FILETYPE_LIBRARY:
    default:
        // Fix names in UNIX style library
        StripMemberNamesUNIX();  break;
    }
}

/*  Unused:
void CLibrary::Rebuild() {
    // Rebuild library: fix member names
    // Dispatch according to library type
    switch (cmd.InputType) {
    case FILETYPE_OMF: case FILETYPE_OMFLIBRARY:
        // Rebuild OMF style library
        RebuildOMF();  break;

    case FILETYPE_LIBRARY:
    default:
        // Rebuild UNIX style library
        RebuildUNIX();  break;
    }
}

void CLibrary::RebuildUNIX() {
    // Rebuild UNIX style library
    // Make member names unique and without path. Rebuild symbol table
    char const * MemberName1 = 0;  // Name of library member
    uint32 OutputFileType;

    // Save OutputType before changing it
    OutputFileType = cmd.OutputType;

    // Loop through input library
    CurrentOffset = 8;  CurrentNumber = 0;
    while ((MemberName1 = ExtractMember(&MemberBuffer)) != 0) {
        // Properties of member
        MemberBuffer.FileName = MemberBuffer.OutputFileName = MemberName1;
        // Check if import library
        if (MemberBuffer.Get<uint32>(0) == 0xFFFF0000) {
            // Import library. Cannot do anything sensible
            err.submit(2507, cmd.InputFile);  return;
        }
        // Remember member type
        cmd.MemberType = MemberBuffer.GetFileType();
        // Temporarily set OutputType to MemberType
        cmd.OutputType = cmd.MemberType;
        // Put into new library
        InsertMember(&MemberBuffer);
    }
    // Avoid getting warnings twice for duplicate symbols
    RepressWarnings = 1; 
    // Make library header etc. and add all members to OutFile
    MakeBinaryFile();
    RepressWarnings = 0;

    // Restore OutputType
    cmd.OutputType = OutputFileType;

    // Replace file buffer by OutFile
    *this << OutFile;

    // Clear buffers used for building OutFile
    StringBuffer.SetSize(0);
    StringEntries.SetNum(0);
    Indexes.SetNum(0);
    DataBuffer.SetSize(0);
}
*/

void CLibrary::RebuildOMF() {
    // Rebuild OMF style library.
    // Removes paths from member names, truncates to 16 characters, and makes unique.
    // Symbol table is removed, not remade

    SOMFRecordPointer rec;                        // Current OMF record
    char * ModuleName;                            // Module name
    SStringEntry se;                              // String entry record to save
    COMFFileBuilder NewBuffer;                    // Buffer for rebuilt library
    uint32 Align;                                 // Alignment

    // Remember member file type
    cmd.MemberType = FILETYPE_OMF;

    // Initialize record pointer
    rec.Start(Buf(), 0, GetDataSize());

    // Read header
    if (rec.Type2 != OMF_LIBHEAD) {err.submit(2500); return;} // Does not start with library header

    // Read library header
    DictionaryOffset = rec.GetDword();
    DictionarySize = rec.GetWord();
    if ((uint64)DictionaryOffset + DictionarySize >= GetDataSize()) {err.submit(2035); return;}

    rec.GetByte(); // Ignore flag
    // Page size / alignment for members
    PageSize = rec.End + 1;
    Align = 1 << FloorLog2(PageSize);       // Make power of 2
    if (PageSize != Align) {
        err.submit(2601, PageSize);          // Error: not a power of 2
    }

    // Copy library header to new buffer
    NewBuffer.Push(Buf(), PageSize);

    // Reset record loop end when DictionaryOffset is known
    rec.FileEnd = DictionaryOffset;

    // Next record is start of first module
    rec.GetNext();
    if (rec.Type2 != OMF_THEADR) err.submit(2500);  // Member does not start with THEADR

    // Loop through the records of all OMF modules
    do {
        // Check record type
        switch (rec.Type2) {

        case OMF_THEADR:     // Start of module

            // Get name
            ModuleName = rec.GetString();

            // Remove path, truncate name and make unique
            ModuleName = ShortenMemberName(ModuleName);

            // Remember name to check for duplicates
            // Not needed any more:
            se.String = StringBuffer.PushString(ModuleName);
            se.Member = NewBuffer.GetDataSize();
            StringEntries.Push(se);

            // Make new THEADR record
            NewBuffer.StartRecord(OMF_THEADR);
            NewBuffer.PutString(ModuleName);
            NewBuffer.EndRecord();
            break;

        case OMF_MODEND: case OMF_LIBEND:   // End of module or library
            // Copy MODEND record
            NewBuffer.Push(Buf() + rec.FileOffset, rec.End + 1);

            // Align output file by PageSize
            NewBuffer.Align(PageSize);
            break;

        default:   // Any other record in module
            // Copy record unchanged
            NewBuffer.Push(Buf() + rec.FileOffset, rec.End + 1);
            break;
        }
    }  // Point to next record
    while (rec.GetNext(PageSize));                // End of loop through records

    // Put dictionary offset in LIBHEAD record
    DictionaryOffset = NewBuffer.GetDataSize();
    NewBuffer.Get<uint32>(3) = DictionaryOffset;

    // Take over modified library file
    *this << NewBuffer;

    // Empty used buffers
    StringEntries.SetNum(0);
    StringBuffer.SetSize(0);
}


void CLibrary::StripMemberNamesUNIX() {
    // Remove path from member names, set extension to default
    char * MemberName1 = 0;  // Name of library member

    // Loop through input library
    CurrentOffset = 8;  CurrentNumber = 0;
    while ((MemberName1 = ExtractMember(&MemberBuffer)) != 0) {
        // Properties of member
        // Check if import library
        if (MemberBuffer.Get<uint32>(0) == 0xFFFF0000) {
            // Import library. Cannot do anything sensible
            err.submit(2507, cmd.InputFile);  return;
        }
        if (MemberName1[0] == '/') continue;  // names record
        // remember member type
        if (cmd.MemberType == 0) {
            cmd.MemberType = MemberBuffer.GetFileType();
        }
        // Fix name
        StripMemberName(MemberName1);
        // Check word size
        if (cmd.DesiredWordSize == 0) {
            cmd.DesiredWordSize = MemberBuffer.WordSize;
        }
        else if (cmd.DesiredWordSize != MemberBuffer.WordSize && MemberBuffer.WordSize != 0) {
            err.submit(2012, MemberBuffer.WordSize, cmd.DesiredWordSize);  // wrong word size
            return;
        }
        if (cmd.OutputType == FILETYPE_LIBRARY || cmd.OutputType == FILETYPE_OMFLIBRARY) {
            cmd.OutputType = cmd.MemberType;
        }
    }
}


void CLibrary::Dump() {
    // Print contents of library

    // Dispatch according to library type
    switch (cmd.InputType) {
    case FILETYPE_LIBRARY:
        DumpUNIX();  break;                        // Print contents of UNIX style library

    case FILETYPE_OMFLIBRARY:
        DumpOMF();   break;                        // Print contents of OMF style library

    default:
        err.submit(9000);                          // Should not occur
    }
}

void CLibrary::DumpOMF() {
    // Print contents of OMF style library
    uint8  Flags;                                 // Dictionary flags
    uint32 i;                                     // Loop counter
    uint32 Align;                                 // Member alignment
    uint32 RecordEnd;                             // End of OMF record
    SOMFRecordPointer rec;                        // Current OMF record
    char * MemberName;                            // Name of library member
    char * SymbolName;                            // Name of public symbol in member
    uint32 MemberStart = 0;                       // Start of member
    uint32 MemberEnd;                             // End of member
    uint32 MemberNum = 0;                         // Member number
    uint32 FirstPublic;                           // Index to first public name of current member
    CMemoryBuffer Strings;                        // Local string buffer
    CSList<SStringEntry> MemberIndex;             // Local member index buffer
    COMF Member;                                  // Local buffer for member

    DictionaryOffset = GetDataSize();             // Loop end. This value is changed when library header is read
    rec.Start(Buf(), 0, DictionaryOffset);        // Initialize record pointer

    PageSize = 0;

    printf("\nDump of library %s\nExported symbols by member:\n", cmd.InputFile);

    // Loop through the records of all OMF modules
    do {
        // Check record type
        switch (rec.Type2) {

        case OMF_LIBHEAD:  // Library header. This should be the first record
            if (PageSize || rec.FileOffset > 0) {
                err.submit(2600);  break;            // More than one header
            }
            // Read library header
            DictionaryOffset = rec.GetDword();
            DictionarySize = rec.GetWord();
            if ((uint64)DictionaryOffset + DictionarySize >= GetDataSize()) {err.submit(2035); return;}
            Flags = rec.GetByte();
            // Page size / alignment for members
            PageSize = rec.End + 1;
            Align = 1 << FloorLog2(PageSize);       // Make power of 2
            if (PageSize != Align) {
                err.submit(2601, PageSize);          // Error: not a power of 2
            }
            // Reset record loop end when DictionaryOffset is known
            rec.FileEnd = DictionaryOffset;

            // Print values from LIBHEAD
            printf("\nOMF Library. Page size %i. %s.",
                PageSize, Lookup(OMFLibraryFlags, Flags));
            break;

        case OMF_THEADR: // Module header. Member starts here
            MemberName = rec.GetString();           // Get name
            MemberStart = rec.FileOffset;           // Get start address
            printf("\nMember %s Offset 0x%X", MemberName, MemberStart);// Print member name
            break;

        case OMF_MODEND: // Member ends here.
            RecordEnd = rec.FileOffset + rec.End + 1;// End of record
            MemberEnd = RecordEnd;                   // = member end address

            // Store member in temporary buffer
            Member.SetSize(0);
            Member.Push(Buf() + MemberStart, MemberEnd - MemberStart);

            // Get public names from member
            FirstPublic = MemberIndex.GetNumEntries();
            Member.PublicNames(&Strings, &MemberIndex, ++MemberNum); 

            // Print public names
            for (i = FirstPublic; i < MemberIndex.GetNumEntries(); i++) {
                SymbolName = Strings.Buf() + MemberIndex[i].String;
                printf("\n  %s", SymbolName);
            }
            // Align next member by PageSize;
            MemberEnd = (MemberEnd + PageSize - 1) & - (int32)PageSize;
            rec.End = MemberEnd - rec.FileOffset - 1;
            break;

        case OMF_LIBEND: // Last member should end here
            RecordEnd = rec.FileOffset + rec.End + 1;// End of record
            if (RecordEnd != DictionaryOffset) err.submit(2602);
            break;
        }     
    }  // Go to next record
    while (rec.GetNext());                        // End of loop through records

    // Check hash table integrity
    CheckOMFHash(Strings, MemberIndex);

    // Check if there is an extended library dictionary
    uint32 ExtendedDictionaryOffset = DictionaryOffset + DictionarySize * 512;

    if (ExtendedDictionaryOffset > GetDataSize()) {
        err.submit(2500);                          // File is truncated
    }
    if (ExtendedDictionaryOffset < GetDataSize()) {
        // Library contains extended dictionary
        uint32 ExtendedDictionarySize = GetDataSize() - ExtendedDictionaryOffset;
        uint8 DictionaryType = Get<uint8>(ExtendedDictionaryOffset); // Read first byte of extended dictionary
        if (DictionaryType == OMF_LIBEXT) {
            // Extended dictionary in the official format
            printf("\nExtended dictionary IBM/MS format. size %i", ExtendedDictionarySize);
        }
        else if (ExtendedDictionarySize >= 10 && (DictionaryType == 0xAD || Get<uint16>(ExtendedDictionaryOffset + 2) == MemberNum)) {
            // Extended dictionary in the proprietary Borland format, documented only in US Patent 5408665, 1995
            printf("\nExtended dictionary Borland format. size %i", ExtendedDictionarySize);
        }
        else {
            // Unknown format
            printf("\nExtended dictionary size %i, unknown type 0x%02X", 
                ExtendedDictionarySize, DictionaryType);
        }
    }
}

void CLibrary::DumpUNIX() {
    // Print contents of UNIX style library

    const char * MemberName = 0;
    CurrentOffset = 8;  CurrentNumber = 0;

    printf("\nDump of library %s", cmd.InputFile);

    if (cmd.DumpOptions & DUMP_SECTHDR) {
        // dump headers
        SUNIXLibraryHeader * Header = 0;    // Member header
        uint32 MemberSize = 0;              // Size of member
        //uint32 HeaderExtra = 0;             // Extra added to size of header
        //uint32 NameIndex;                 // Index into long names member
        char * Name = 0;                    // Name of member
        int symindex = 0;                   // count symbol index records
        int i;                              // loop counter

        // Search for member
        while (CurrentOffset) {
            //HeaderExtra = 0;
            // Extract next library member from input library
            Header = &Get<SUNIXLibraryHeader>(CurrentOffset);
            // Size of member
            MemberSize = (uint32)atoi(Header->FileSize);
            // Member name
            Name = Header->Name;
            // Terminate name
            for (i = 0; i < 15; i++) {
                if (Name[i] == ' ') {
                    Name[i+1] = 0;  break;
                }
            }
            if (i == 16) Name[i] = 0;

            if (strncmp(Name, "//", 2) == 0) {
                // This is the long names member. 
                printf("\nLongnames header \"%s\". Offset 0x%X, size 0x%X", Name,
                    CurrentOffset + (uint32)sizeof(SUNIXLibraryHeader), MemberSize);
            }
            else if (Name[0] == '/' && Name[1] <= ' ') {
                // Symbol index
                printf("\nSymbol index %i, \"%s\"", ++symindex, Name);
            }
            else if (strncmp(Name, "__.SYMDEF", 9) == 0) {
                // Mac/BSD Symbol index
                printf("\nSymbol index %i, \"%s\"", ++symindex, Name);
            }
            else if (strncmp(Name, "#1/", 3) == 0) {
                // Name refers to long name after the header
                // This variant is used by Mac and some versions of BSD
                //HeaderExtra = atoi(Name+3);
                Name += sizeof(SUNIXLibraryHeader);
                if (strncmp(Name, "__.SYMDEF", 9) == 0) {
                    // Symbol table "__.SYMDEF SORTED" as long name
                    printf("\nSymbol index %i, \"%s\"", ++symindex, Name);
                }
            }
            else break;
            // Point to next member
            CurrentOffset = NextHeader(CurrentOffset);
        }
        CurrentOffset = 8;  CurrentNumber = 0; 
    }

    printf("\n\nExported symbols by member:\n");

    // Loop through library
    while (CurrentOffset + sizeof(SUNIXLibraryHeader) < DataSize) {

        // Reset buffers
        StringBuffer.SetSize(0);
        MemberBuffer.Reset();
        StringEntries.SetNum(0);

        // Get member name
        MemberName = ExtractMember(&MemberBuffer);
        if (MemberName == 0) break;
        printf("\nMember %s", MemberName);
        MemberBuffer.FileName = MemberName;

        // Detect file type of member
        MemberFileType = MemberBuffer.GetFileType();

        WordSize = MemberBuffer.WordSize;
        printf (" - %s", GetFileFormatName(MemberFileType));
        if (WordSize) {
            printf("-%i", MemberBuffer.WordSize);
        }
        else {
            printf(". Type not specified. Possibly alias record");
        }

        // Get symbol table for specific file type
        switch (MemberFileType) {
        case FILETYPE_ELF: 
            if (WordSize == 32) {
                // Make instance of file parser, 32 bit template
                CELF<ELF32STRUCTURES> elf;
                MemberBuffer >> elf;        // Transfer MemberBuffer to elf object
                elf.PublicNames(&StringBuffer, &StringEntries, 0);
                break;
            }
            else {
                // Make instance of file parser, 64 bit template
                CELF<ELF64STRUCTURES> elf;
                MemberBuffer >> elf;        // Transfer MemberBuffer to elf object
                elf.PublicNames(&StringBuffer, &StringEntries, 0);
                break;
            }

        case FILETYPE_COFF: {
            CCOFF coff;
            MemberBuffer >> coff;       // Transfer MemberBuffer to coff object
            coff.PublicNames(&StringBuffer, &StringEntries, 0);
            break;}

        case FILETYPE_MACHO_LE:
            if (WordSize == 32) {
                // Make instance of file parser, 32 bit template
                CMACHO<MAC32STRUCTURES> mac;
                MemberBuffer >> mac;       // Transfer MemberBuffer to coff object
                mac.PublicNames(&StringBuffer, &StringEntries, 0);
                break;
            }
            else {
                // Make instance of file parser, 64 bit template
                CMACHO<MAC64STRUCTURES> mac;
                MemberBuffer >> mac;       // Transfer MemberBuffer to coff object
                mac.PublicNames(&StringBuffer, &StringEntries, 0);
                break;
            }

        case IMPORT_LIBRARY_MEMBER: {
            // This is an import library
            char * name1 = MemberBuffer.Buf() + 20;
            printf("\n  Import %s from %s", name1, name1 + strlen(name1) + 1);
            break;} 

        default:
            printf("\n   Cannot extract symbol names from this file type");
            break;
        }

        // Loop through table of public names
        for (uint32 i = 0; i < StringEntries.GetNumEntries(); i++) {
            uint32 j = StringEntries[i].String;
            printf("\n   %s", StringBuffer.Buf() + j);
        }
    }
}


uint32 CLibrary::NextHeader(uint32 Offset) {

    // Loop through library headers.
    // Input = current offset. Output = next offset
    SUNIXLibraryHeader * Header;   // Member header
    int32 MemberSize;          // Size of member
    //uint32 HeaderExtra = 0;    // Extra added to size of header
    uint32 NextOffset;         // Offset of next header

    if (Offset + sizeof(SUNIXLibraryHeader) >= DataSize) {
        // No more members
        return 0;
    }
    // Find header
    Header = &Get<SUNIXLibraryHeader>(Offset);

    // Size of member
    MemberSize = atoi(Header->FileSize);
    if (MemberSize < 0 || MemberSize + Offset + sizeof(SUNIXLibraryHeader) > DataSize) {
        err.submit(2500);  // Points outside file
        return 0;
    }
    if (strncmp(Header->Name, "#1/", 3) == 0) {
        // Name refers to long name after the header
        // This variant is used by Mac and some versions of BSD.
        // HeaderExtra is included in MemberSize:
        // HeaderExtra = atoi(Header->Name+3);
    }

    // Get next offset
    NextOffset = Offset + sizeof(SUNIXLibraryHeader) + MemberSize;
    // Round up to align by 2
    NextOffset = (NextOffset + 1) & ~ 1;
    // Check if last
    if (NextOffset >= DataSize) NextOffset = 0;
    return NextOffset;
}


void CLibrary::StartExtracting() {
    // Initialize before ExtractMember() 
    if (cmd.InputType == FILETYPE_OMFLIBRARY) {
        SOMFRecordPointer rec;                     // OMF records
        rec.Start(Buf(), 0, GetDataSize());        // Initialize record pointer
        if (rec.Type2 != OMF_LIBHEAD) {
            err.submit(2500);  return;              // This should not happen
        }
        // Read library header
        DictionaryOffset = rec.GetDword();         // Read dictionary offset
        DictionarySize   = rec.GetWord();          // Read dictionary size
        rec.GetByte();                             // Read flag
        // Page size / alignment for members
        PageSize = rec.End + 1;
        uint32 align = 1 << FloorLog2(PageSize);   // Make power of 2
        if (PageSize != align) {
            err.submit(2601, PageSize);             // Error: not a power of 2
        }
        CurrentOffset = PageSize;                  // Offset to first library member
    }
    else {
        // Unix style library. First header at offset 8
        CurrentOffset = 8;
    }
    // Current member number
    CurrentNumber = 0;
}


char * CLibrary::ExtractMember(CFileBuffer * Destination) {
    // Extract library member
    // Dispatch according to library type
    if (cmd.InputType == FILETYPE_OMFLIBRARY || cmd.InputType == FILETYPE_OMF) {
        return ExtractMemberOMF(Destination);
    }
    else {
        return ExtractMemberUNIX(Destination);
    }
}


char * CLibrary::ExtractMemberOMF(CFileBuffer * Destination) {
    // Extract member of OMF style library

    uint32 RecordEnd;                             // End of OMF record
    SOMFRecordPointer rec;                        // Current OMF record
    char * MemberName = 0;                        // Name of library member
    uint32 MemberStart = 0;                       // Start of member
    uint32 MemberEnd = 0;                         // End of member

    if (CurrentOffset >= DictionaryOffset) return 0;// No more members

    rec.Start(Buf(), CurrentOffset, DictionaryOffset);// Initialize record pointer

    // Loop through the records of all OMF modules
    do {
        // Check record type
        switch (rec.Type2) {

        case OMF_THEADR: // Module header. Member starts here
            MemberName  = rec.GetString();          // Get name
            MemberStart = rec.FileOffset;           // Get start address
            break;

        case OMF_MODEND: // Member ends here.
            RecordEnd = rec.FileOffset + rec.End +1;// End of record
            MemberEnd = RecordEnd;                  // = member end address

            // Save member as raw data
            if (Destination) {
                Destination->SetSize(0);             // Make sure destination buffer is empty
                Destination->FileType = Destination->WordSize = 0;
                Destination->Push(Buf() + MemberStart, MemberEnd - MemberStart);
            }

            // Align next member by PageSize;
            rec.GetNext(PageSize);
            CurrentOffset = rec.FileOffset;

            // Check name
            //!!if (MemberName[0] == 0) MemberName = (char*)"NoName!";

            // Return member name
            return MemberName;

        case OMF_LIBEND: // Last member should end here
            RecordEnd = rec.FileOffset + rec.End + 1;// End of record
            if (RecordEnd != DictionaryOffset) err.submit(2602);

            // No more members:
            return 0;
        }     
    }  // Go to next record
    while (rec.GetNext());                        // End of loop through records

    err.submit(2610);                             // Library end record not found
    return 0;
}


char * CLibrary::ExtractMemberUNIX(CFileBuffer * Destination) {
    // Extract member of UNIX style library
    // This function is called repeatedly to get each member of library/archive
    SUNIXLibraryHeader * Header = 0;     // Member header
    uint32 MemberSize = 0;              // Size of member
    uint32 HeaderExtra = 0;             // Extra added to size of header
    uint32 NameIndex;                   // Index into long names member
    char * Name = 0;                    // Name of member
    int Skip = 1;                       // Skip record and search for next
    int i;                              // Loop counter
    char * p;                           // Used for loop through string

    if (CurrentOffset == 0 || CurrentOffset + sizeof(SUNIXLibraryHeader) >= DataSize) {
        // No more members
        return 0;
    }

    // Search for member
    while (Skip && CurrentOffset) {
        HeaderExtra = 0;
        // Extract next library member from input library
        Header = &Get<SUNIXLibraryHeader>(CurrentOffset);
        // Size of member
        MemberSize = (uint32)atoi(Header->FileSize);
        if (MemberSize + CurrentOffset + sizeof(SUNIXLibraryHeader) > DataSize) {
            err.submit(2500);  // Points outside file
            return 0;
        }
        // Member name
        Name = Header->Name;
        if (strncmp(Name, "// ", 3) == 0) {
            // This is the long names member. Remember its position
            LongNames = CurrentOffset + sizeof(SUNIXLibraryHeader);
            LongNamesSize = MemberSize;
            // The long names are terminated by '/' or 0, depending on system,
            // but may contain non-terminating '/'. Find out which type we have:
            // Pointer to LongNames record
            p = Buf() + LongNames;
            // Find out whether we have terminating zeroes:
            if ((LongNamesSize > 1 && p[LongNamesSize-1] == '/') || (p[LongNamesSize-1] <= ' ' && p[LongNamesSize-2] == '/')) {
                // Names are terminated by '/'. Replace all '/' by 0 in the longnames record
                for (uint32 j = 0; j < LongNamesSize; j++, p++) {
                    if (*p == '/') *p = 0;
                }
            }
        }
        else if (strncmp(Name, "/ ", 2) == 0
            || strncmp(Name, "__.SYMDEF", 9) == 0) {
                // This is a symbol index member.
                // The symbol index is not used because we are always building a new symbol index.
        }
        else if (Name[0] == '/' && Name[1] >= '0' && Name[1] <= '9' && LongNames) {
            // Name contains index into LongNames record
            NameIndex = atoi(Name+1);
            if (NameIndex < LongNamesSize) {
                Name = Buf() + LongNames + NameIndex;
            }
            else {
                Name = (char*)"NoName!";
            }
            Skip = 0;
        }
        else if (strncmp(Name, "#1/", 3) == 0) {
            // Name refers to long name after the header
            // This variant is used by Mac and some versions of BSD
            HeaderExtra = atoi(Name+3);
            Name += sizeof(SUNIXLibraryHeader);
            if (MemberSize > HeaderExtra) {
                // The length of the name, HeaderExtra, is included in the 
                // Header->FileSize field. Subtract to get the real file size
                MemberSize -= HeaderExtra;
            }
            if (strncmp(Name, "__.SYMDEF", 9) == 0) {
                // Symbol table "__.SYMDEF SORTED" as long name
                Skip = 1;
            }
            else {
                Skip = 0;
            }
        }
        else {
            // Ordinary short name
            // Name may be terminated by '/' or space. Replace termination char by 0
            for (i = 15; i >= 0; i--) {
                if (Name[i] == ' ' || Name[i] == '/') Name[i] = 0;
                else break;
            }
            // Terminate name with max length by overwriting Date field, which we are not using
            Name[16] = 0;
            Skip = 0;
        }
        // Point to next member
        CurrentOffset = NextHeader(CurrentOffset);
        // Increment number
        CurrentNumber += !Skip;
    }  // End of while loop

    // Save member as raw data
    if (Destination) {
        Destination->SetSize(0);       // Make sure destination buffer is empty
        Destination->FileType = Destination->WordSize = 0;
        Destination->Push((int8*)Header + sizeof(SUNIXLibraryHeader) + HeaderExtra, MemberSize);
    }

    // Check name
    if (Name[0] == 0) Name = (char*)"NoName!";

    // Return member name
    return Name;
}

void CLibrary::InsertMember(CFileBuffer * member) {
    // Add member to output library
    if (cmd.OutputType == FILETYPE_OMF) {
        InsertMemberOMF(member);                   // OMF style output library
    }
    else {
        InsertMemberUNIX(member);                  // UNIX style output library
    }
}


void CLibrary::InsertMemberOMF(CFileBuffer * member) {
    // Add member to OMF library

    // Check file type
    if (member->GetFileType() != FILETYPE_OMF) err.submit(9000);

    // Get word size
    WordSize = member->WordSize;

    // Store offset
    uint32 offset = DataBuffer.GetDataSize();
    Indexes.Push(offset);

    // Store member
    DataBuffer.Push(member->Buf(), member->GetDataSize());
    DataBuffer.Align(PageSize);

    // Member index
    uint32 mindex = Indexes.GetNumEntries() - 1;

    // Get public string table
    COMF omf;
    *member >> omf;     // Translate member to class OMF
    omf.PublicNames(&StringBuffer, &StringEntries, mindex); // Make list of public names
    *member << omf;     // Return buffer to member
}


void CLibrary::InsertMemberUNIX(CFileBuffer * member) {
    // Add next library member to output library
    uint32 RawSize = 0;                 // Size of binary file
    uint32 AlignmentPadding = 0;        // Padding after file
    char * name = 0;                    // Name of member
    int NameLength = 0;                 // length of name
    int NameAfter = 0;                  // length of name after MachO header 
    int i;                              // loop counter, index

    // Get word size
    WordSize = member->WordSize;

    // Make member header
    SUNIXLibraryHeader header;
    memset(&header, ' ', sizeof(SUNIXLibraryHeader));  // Fill with spaces

    // Name of member
    if (member->OutputFileName == 0 || *member->OutputFileName == 0) member->OutputFileName = member->FileName;

    if (cmd.LibrarySubtype == LIBTYPE_SHORTNAMES) {
        // Make short name
        name = ShortenMemberName(member->OutputFileName);
    }
    else {
        // Remove path from library member name. Original long name is overwritten
        name = StripMemberName((char*)(member->OutputFileName));
    }
    NameLength = strlen(name);

    if (cmd.OutputType == FILETYPE_MACHO_LE && cmd.LibrarySubtype != LIBTYPE_SHORTNAMES) {
        // Mach-O library stores name after header record.
        // Name is zero padded to length 4 modulo 8 to align by 8
        int pad = 8 - ((NameLength + 4) & 7);
        NameAfter = NameLength + pad;
        sprintf(header.Name, "#1/%i ", NameAfter);
    }
    else {
        // ELF and COFF library store names < 16 characters in the name field
        if (NameLength < 16) {
            // (Don't use sprintf to write header.Name here: It seems that Gnu sprintf checks the size of the 
            // buffer it is writing to. Gives buffer overrun error when termniating zero goes beyond the name field)
            memset(header.Name, ' ', 16);
            memcpy(header.Name, name, NameLength);
            header.Name[NameLength] = '/';
        }
        else {
            // store in LongNamesBuffer
            if (cmd.OutputType == FILETYPE_COFF) {
                // COFF: Name is zero-terminated
                i = LongNamesBuffer.PushString(name);
            }
            else {
                // ELF: Name terminated by "/\n"
                i = LongNamesBuffer.Push(name, NameLength);
                LongNamesBuffer.Push("/\n", 2);
            }
            // store index into long names member
            sprintf(header.Name, "/%i ", i);
        }
    }

    // Date
    sprintf(header.Date, "%u ", (uint32)time(0));

    // User and group id
    header.UserID[0] = '0';
    header.GroupID[0] = '0';
    // File mode
    strcpy(header.FileMode, "100666");
    // Size of binary file
    RawSize = member->GetDataSize();
    // Calculate alignment padding
    if (AlignBy) {
        AlignmentPadding = uint32(-int32(RawSize)) & (AlignBy-1);
    }

    // File size including name string
    sprintf(header.FileSize, "%u ", NameAfter + RawSize + AlignmentPadding);

    // Header end
    header.HeaderEnd[0] = '`';
    header.HeaderEnd[1] = '\n';

    // Remove terminating zeroes
    for (uint32 i = 0; i < sizeof(SUNIXLibraryHeader); i++) {
        if (((char*)&header)[i] == 0) ((char*)&header)[i] = ' ';
    }

    // Store offset
    uint32 offset = DataBuffer.GetDataSize();
    Indexes.Push(offset);

    // Store member header
    DataBuffer.Push(&header, sizeof(header));

    if (cmd.OutputType == FILETYPE_MACHO_LE) {    
        // Store member name after header if Mach-O
        if (NameAfter) {
            // Mach-O library stores name after header record.
            DataBuffer.PushString(name); 
        }
        DataBuffer.Align(AlignBy);
    }

    // Store member
    DataBuffer.Push(member->Buf(), RawSize);

    // Align by padding with '\n'
    for (uint32 i = 0; i < AlignmentPadding; i++) {
        DataBuffer.Push("\n", 1);
    }

    // Member index
    uint32 mindex = Indexes.GetNumEntries() - 1;

    // Get public string table
    switch(member->GetFileType()) {
    case FILETYPE_COFF: {
        CCOFF coff;
        *member >> coff;     // Translate member to type COFF
        coff.PublicNames(&StringBuffer, &StringEntries, mindex); // Make list of public names
        *member << coff;     // Return buffer to member
        break;}

    case FILETYPE_OMF: {
        COMF omf;
        *member >> omf;      // Translate member to type COFF
        omf.PublicNames(&StringBuffer, &StringEntries, mindex); // Make list of public names
        *member << omf;      // Return buffer to member
        break;}

    case FILETYPE_ELF:
        if (WordSize == 32) {
            // Make instance of file parser, 32 bit template
            CELF<ELF32STRUCTURES> elf;
            *member >> elf;      // Translate member to type ELF
            elf.PublicNames(&StringBuffer, &StringEntries, mindex); // Make list of public names
            *member << elf;      // Return buffer to member
            break;
        }
        else {
            // Make instance of file parser, 64 bit template
            CELF<ELF64STRUCTURES> elf;
            *member >> elf;      // Translate member to type ELF
            elf.PublicNames(&StringBuffer, &StringEntries, mindex); // Make list of public names
            *member << elf;      // Return buffer to member
            break;
        }

    case FILETYPE_MACHO_LE:
        if (WordSize == 32) {
            // Make instance of file parser, 32 bit template
            CMACHO<MAC32STRUCTURES> mac;
            *member >> mac;      // Translate member to type ELF
            mac.PublicNames(&StringBuffer, &StringEntries, mindex); // Make list of public names
            *member << mac;      // Return buffer to member
            break;
        }
        else {
            // Make instance of file parser, 64 bit template
            CMACHO<MAC64STRUCTURES> mac;
            *member >> mac;      // Translate member to type ELF
            mac.PublicNames(&StringBuffer, &StringEntries, mindex); // Make list of public names
            *member << mac;      // Return buffer to member
            break;
        }

    default:                // Type not supported
        err.submit(2501, GetFileFormatName(member->GetFileType()));
        break;
    }
}

/*   unused:
char * CLibrary::TruncateMemberName(char const * name) {
    // Remove path and truncate object file name to 15 characters
    // This function removes any path from the member name,
    // changes the extension to the default for the the output file type,
    // changes any spaces to underscores, and 
    // truncates the member name to 15 characters for the sake of compatibility.
    // The return value is an ASCII string in a static buffer
    static char TruncName[32];          // Truncated name
    int maxlen;                         // Max length, not including extension
    char const * p1;                    // Point to start of name without path
    char const * extension;             // Default extension for file type
    int i;                              // Loop counter
    int len;                            // String length
    static int DummyNumber = 0;         // Count invalid/null names
    int FileType;                       // File type

    // Remove path
    len = (int)strlen(name);  p1 = name;
    for (i = len-1; i >= 0; i--) {
        if (name[i] == '/' || name[i] == '\\' || name[i] == ':') {
            p1 = name + i + 1; break;
        }
    }
    // Remove extension
    len = (int)strlen(p1);
    for (i = len-1; i >= 0; i--) {
        if (p1[i] == '.') {
            len = i;  break;
        }
    }

    // Check if any name remains
    if (len == 0) {     // No name. Make one
        sprintf(TruncName, "NoName%i", ++DummyNumber);
        p1 = TruncName;  len = (int)strlen(p1); 
    }

    // Get file type
    FileType = cmd.OutputType;
    if (FileType == CMDL_OUTPUT_DUMP || FileType == 0) FileType = cmd.InputType;
    if (FileType >= FILETYPE_LIBRARY) FileType = cmd.MemberType;

    // Get default extension and max length of name without extension
    if (FileType == FILETYPE_COFF || FileType == FILETYPE_OMF) {
        maxlen = 11;  extension = ".obj";
    }
    else {
        maxlen = 13;  extension = ".o";
    }
    if (len > maxlen) len = maxlen;

    // Truncate name
    strncpy(TruncName, p1, len);
    TruncName[len] = 0;

    // Remove any spaces or other illegal characters
    len = (int)strlen(TruncName);
    for (i = 0; i < len; i++) {
        if ((uint8)TruncName[i] <= 0x20) TruncName[i] = '_';
    }

    // Add default extension
    strcpy(TruncName+strlen(TruncName), extension);

    // Terminate
    TruncName[15] = 0;  
    return TruncName;
}
*/

char * CLibrary::StripMemberName(char * name) {
    // Remove path from zero-terminated library member name and set extension to default.
    // Note: Original long name is overwritten
    char * p1;                          // Point to start of name without path
    const char * extension = 0;         // Default extension for file type
    int i;                              // Loop counter
    int len0;                           // Original string length
    int len;                            // String length
    int nlen;                           // length of name without extension
    int elen = 0;                       // length of extension
    static int DummyNumber = 0;         // Count invalid/null names
    int FileType;                       // File type

    // Length
    len0 = len = (int)strlen(name);

    // Remove path
    p1 = name;
    for (i = len-1; i >= 0; i--) {
        if (name[i] == '/' || name[i] == '\\' || name[i] == ':') {
            p1 = name + i + 1; break;
        }
    }
    len -= i + 1;

    // move to begin of buffer
    if (p1 > name) {
        memmove(name, p1, len + 1);
    }
        
    // Get file type
    if (cmd.MemberType) {
        FileType = cmd.MemberType;
    }
    else if (cmd.LibraryOptions & CMDL_LIBRARY_EXTRACTMEM) {
        FileType = cmd.InputType;
    }
    else {    
        FileType = cmd.OutputType;
    }
    if (FileType == CMDL_OUTPUT_DUMP || FileType == 0) FileType = cmd.InputType;
    if (FileType >= FILETYPE_LIBRARY) FileType = cmd.MemberType;

    // Get default extension and max length of name without extension
    if (FileType == FILETYPE_COFF || FileType == FILETYPE_OMF) {
        extension = ".obj";  elen = 4;
    }
    else if (FileType == FILETYPE_ELF || FileType == FILETYPE_MACHO_LE || FileType == FILETYPE_MACHO_BE) {
        extension = ".o";  elen = 2;
    }

    // find extension
    for (nlen = len-1; nlen >= 0; nlen--) {
        if (name[nlen] == '.') {
            break;
        }
    }

    // Remove any spaces or other illegal characters
    for (i = 0; i < nlen; i++) {
        if ((uint8)name[i] <= 0x20 || name[i] == '.') name[i] = '_';
    }

    // Check if any name remains
    if ((len == 0 && len0 > 12) || nlen == 0) {     // No name. Make one
        sprintf(name, "NoName%i", ++DummyNumber);
        len = (int)strlen(name); 
    }

    // Replace extension
    if (len + elen <= len0 && extension != 0) {
        strcpy(name + nlen, extension);
    }
    // Terminate
    return name;
}


char * CLibrary::ShortenMemberName(char const *name) {
    // Truncate library member name to 15 characters and make unique
    // The path is removed and the extension set to default.
    // The original long name is not overwritten
    static char fixedName[32];          // Modified name
    char const * p1;                    // Point to start of name without path
    char const * extension;             // Default extension for file type
    int i;                              // Loop counter
    int len;                            // Filename length
    int len0;                           // Filename length without extension
    int elen;                           // length of extension
    static int RunningNumber = 0;       // Enumerate truncated names
    int FileType;                       // File type

    // Length
    len = (int)strlen(name);

    // Skip path
    p1 = name;
    for (i = len-1; i >= 0; i--) {
        if (name[i] == '/' || name[i] == '\\' || name[i] == ':') {
            p1 = name + i + 1; break;
        }
    }
    len = (int)strlen(name);

    // move to static buffer
    if (len > 15) len = 15;
    memcpy(fixedName, p1, len);
    fixedName[len] = 0;

    // find extension
    for (i = len-1; i >= 0; i--) {
        if (fixedName[i] == '.') {
            fixedName[i] = 0;  break;
        }
    }
    // length without extension
    len0 = (int)strlen(fixedName);

    // Remove any spaces or other illegal characters
    for (i = 0; i < len0; i++) {
        if ((uint8)fixedName[i] <= 0x20 || fixedName[i] == '.') fixedName[i] = '_';
    }

    // Check if any name remains
    if (len0 == 0) {     // No name. Make one
        sprintf(fixedName, "NoName_%X", RunningNumber++);
        len0 = (int)strlen(fixedName); 
    }

    // Get file type
    FileType = cmd.OutputType;
    if (FileType == CMDL_OUTPUT_DUMP || FileType == 0) FileType = cmd.InputType;
    if (FileType >= FILETYPE_LIBRARY) FileType = cmd.MemberType;

    // Get default extension and max length of name without extension
    if (FileType == FILETYPE_COFF || FileType == FILETYPE_OMF) {
        extension = ".obj";  elen = 4;
    }
    else {
        extension = ".o";  elen = 2;
    }

    // Make unique and add extension
    if (len0 + elen >= 15) {
        // Name is truncated or possibly identical to some other truncated name.
        // Insert 2-, 3- or 4-digit running hexadecimal number.
        if (RunningNumber < 0x100) {
            sprintf(fixedName + 12 - elen, "_%02X%s", RunningNumber++, extension);
        }
        else if (RunningNumber < 0x1000) {
            sprintf(fixedName + 12 - elen, "%03X%s", RunningNumber++, extension);
        }
        else {
            sprintf(fixedName + 11 - elen, "%04X%s", (RunningNumber++ & 0xFFFF), extension);
        }
    }
    else {
        // Short name. Just add extension
        strcpy(fixedName + len0, extension);
    }

    // Return static name buffer
    return fixedName;
}

/*  Unused:
int CLibrary::MemberNameExistsUNIX(char * name) {
    // Check if DataBuffer contains a member with this name
    char Name1[20], Name2[20];
    uint32 i, j;

    // Terminate name without extension
    memcpy(Name1, name, 16);
    for (j = 0; j < 16; j++) {
        if (Name1[j] == '.' || Name1[j] == '/') Name1[j] = 0;
    }

    // Loop through previous members in DataBuffer
    for (i = 0; i < Indexes.GetNumEntries(); i++) {
        uint32 offset = Indexes[i];
        // Copy name of member i
        memcpy(Name2, DataBuffer.Buf() + offset, 16);
        // Terminate name2
        for (j = 0; j < 16; j++) {
            if (Name2[j] == '.' || Name2[j] == '/') Name2[j] = 0;
        }
        // Case-insensitive compare of names
        if (stricmp(Name1, Name2) == 0) {
            // Identical name found
            return i + 1;
        }
    }
    // No identical name found
    return 0;
}*/


void CLibrary::SortStringTable() {
    // Sort the string table in ASCII order

    // Length of table
    int32 n = StringEntries.GetNumEntries();
    if (n <= 0) return;

    // Point to table of SStringEntry records
    SStringEntry * Table = &StringEntries[0];
    // String pointers
    char * s1, * s2;

    // Simple Shell sort with Sedgewick gaps:
    int32 i, j, k, gap;
    for (k = 15; k >= 0; k--) {
        gap = (1 << 2 * k) | (3 << k >> 1) | 1;  // Sedgewick gap grants O(N^4/3) 
        for (i = gap; i < n; i++) {
            SStringEntry key = Table[i];
            char * strkey = StringBuffer.Buf() + key.String;
            for (j = i - gap; j >= 0 && strcmp(strkey, StringBuffer.Buf() + Table[j].String) < 0; j -= gap) {
                Table[j + gap] = Table[j];
            }
            Table[j + gap] = key;
        }
    }

    // Now StringEntries has been sorted. Reorder StringBuffer to the sort order.
    CMemoryBuffer SortedStringBuffer;    // Temporary buffer for strings in sort order
    for (i = 0; i < n; i++) {
        // Pointer to old string
        s1 = StringBuffer.Buf() + Table[i].String;
        // Update table to point to new string
        Table[i].String = SortedStringBuffer.GetDataSize();
        // Put string into SortedStringBuffer
        SortedStringBuffer.PushString(s1);
    }
    if (SortedStringBuffer.GetDataSize() != StringBuffer.GetDataSize()) {
        // The two string buffers should be same size
        err.submit(9000); return;
    }
    // Copy SortedStringBuffer into StringBuffer
    memcpy(StringBuffer.Buf(), SortedStringBuffer.Buf(), StringBuffer.GetDataSize());

    // Check for duplicate symbols
    for (i = 0; i < n-1; i++) {
        s1 = StringBuffer.Buf() + Table[i].String;
        for (j = i + 1; j < n; j++) {
            s2 = StringBuffer.Buf() + Table[j].String;
            if (strcmp(s1,s2) == 0) {
                // Duplicate found
                // Compose error string "Modulename1 and Modulename2"
                uint32 errstring = LongNamesBuffer.GetDataSize();
                LongNamesBuffer.PushString(GetModuleName(Table[i].Member));
                LongNamesBuffer.SetSize(LongNamesBuffer.GetDataSize()-1); // remove terminating zero
                LongNamesBuffer.Push(" and ", 5);
                LongNamesBuffer.PushString(GetModuleName(Table[j].Member));
                err.submit(1214, s1, (char*)LongNamesBuffer.Buf() + errstring);
                LongNamesBuffer.SetSize(errstring);  // remove string again
                i++;  // Prevent excessive messages
            }
            else {
                break;
            }
        }
    }
}


uint32 EndianChange(uint32 n) {
    // Convert little-endian to big-endian number, or vice versa
    return (n << 24) | ((n & 0x0000FF00) << 8) | ((n & 0x00FF0000) >> 8) | (n >> 24);
}


uint32 RoundEven(uint32 n) {
    // Round up number to nearest even
    return (n + 1) & uint32(-2);
}


uint32 Round4(uint32 n) {
    // Round up number to nearest multiple of 4
    return (n + 3) & uint32(-4);
}


void CLibrary::MakeSymbolTableUnix() {
    // Make symbol table for COFF, ELF or MACHO library
    // Uses UNIX archive format for COFF, BSD and Mac
    uint32 i;                              // Loop counter
    uint32 MemberOffset;                   // Offset to member
    uint32 LongNameSize = 0;               // Length of symbol table name if stored after record

    int SymbolTableType = cmd.OutputType;  // FILETYPE_COFF       = 1: COFF
    // FILETYPE_ELF        = 3: ELF
    // FILETYPE_MACHO_LE   = 4: Mac, unsorted
    //              0x10000004: Mac, sorted
    // Newer Mac tools require the sorted type, unless there are multiple publics with same name
    if (SymbolTableType == FILETYPE_MACHO_LE) SymbolTableType |= 0x10000000; 

    // Make symbol table header
    SUNIXLibraryHeader SymTab;             // Symbol table header
    memset(&SymTab, ' ', sizeof(SymTab));  // Fill with spaces
    SymTab.Name[0] = '/';                  // Name = '/'
    // The silly Mac linker requires that the symbol table has a date stamp not 
    // older than the .a file. Fix this by post-dating the symbol table:
    uint32 PostDate = 0;
    if (SymbolTableType & 0x10000000) PostDate = 100; // Post-date if mac sorted symbol table
    sprintf(SymTab.Date, "%u ", (uint32)time(0) + PostDate); // Date stamp for symbol table

    SymTab.UserID[0] = '0';                // UserID = 0  (may be omitted in COFF)
    SymTab.GroupID[0] = '0';               // GroupID = 0 (may be omitted in COFF)
    strcpy(SymTab.FileMode, "100666");     // FileMode = 0100666 (may be 0 in COFF)

    SymTab.HeaderEnd[0] = '`';             // End with "`\n"
    SymTab.HeaderEnd[1] = '\n';

    // File header
    OutFile.Push("!<arch>\n", 8);

    uint32 NumMembers = Indexes.GetNumEntries();        // Number of members
    uint32 NumStrings = StringEntries.GetNumEntries();  // Number of symbol names
    uint32 StringsLen = StringBuffer.GetDataSize();     // Size of string table

    // Calculate sizes of string index records, not including header
    // Unsorted index, used in ELF and COFF libraries
    uint32 Index1Size = (NumStrings+1)*4 + StringsLen;
    // Sorted index, used in COFF libraries as second member
    uint32 Index2Size = (NumMembers+2)*4 + NumStrings*2 + StringsLen;
    // Sorted index, used in Mach-O libraries
    uint32 Index3Size = Round4(NumStrings*8 + 8 + StringsLen);
    // Longnames member
    uint32 LongnamesMemberSize = 0;
    // Official MS COFF reference says that the "//" longnames member must be present,
    // even if it is unused, but MS LIB does not make it unless it is needed.
    // Here, we will include the longnames member only if it is needed
    if ((SymbolTableType == FILETYPE_COFF || SymbolTableType == FILETYPE_ELF) && LongNamesBuffer.GetDataSize()) {
        LongnamesMemberSize = sizeof(SUNIXLibraryHeader) + LongNamesBuffer.GetDataSize();
    }

    // Offset to first member
    uint32 FirstMemberOffset = 0;
    switch (SymbolTableType) {
    case FILETYPE_COFF:
        FirstMemberOffset = 8 + 2*sizeof(SUNIXLibraryHeader) + RoundEven(Index1Size) 
            + RoundEven(Index2Size) + RoundEven(LongnamesMemberSize);
        break;
    case FILETYPE_ELF:
        FirstMemberOffset = 8 + sizeof(SUNIXLibraryHeader) + RoundEven(Index1Size) 
            + RoundEven(LongnamesMemberSize);
        break;
    case FILETYPE_MACHO_LE:
        FirstMemberOffset = 8 + sizeof(SUNIXLibraryHeader) + Index3Size;
        break;
    case FILETYPE_MACHO_LE | 0x10000000: // Mac, sorted
        LongNameSize = 20;
        FirstMemberOffset = 8 + sizeof(SUNIXLibraryHeader) + Index3Size + LongNameSize;
        break;      
    default:
        err.submit(2501, GetFileFormatName(cmd.OutputType));
    }

    // Make unsorted symbol table for COFF or ELF output
    if (SymbolTableType == FILETYPE_COFF || SymbolTableType == FILETYPE_ELF) {

        // Put file size into symbol table header
        sprintf(SymTab.FileSize, "%u ", Index1Size);
        // Remove terminating zeroes
        for (i = 0; i < sizeof(SymTab); i++) {
            if (((char*)&SymTab)[i] == 0) ((char*)&SymTab)[i] = ' ';
        }

        // Store header
        OutFile.Push(&SymTab, sizeof(SymTab));

        // Store table of offsets
        uint32 BigEndian;             // Number converted to big-endian
        BigEndian = EndianChange(NumStrings);
        OutFile.Push(&BigEndian, sizeof(BigEndian));  // Number of symbols

        // Loop through strings
        for (i = 0; i < NumStrings; i++) {
            // Get record in temporary symbol table
            SStringEntry * psym = &StringEntries[i];
            // Get offset of member in DataBuffer
            MemberOffset = Indexes[psym->Member];
            // Add size of headers to compute member offset in final file
            BigEndian = EndianChange(MemberOffset + FirstMemberOffset);
            // Store offset as big endian number
            OutFile.Push(&BigEndian, sizeof(BigEndian));
        }

        // Store strings
        OutFile.Push(StringBuffer.Buf(), StringBuffer.GetDataSize());
        // Align by 2
        if (OutFile.GetDataSize() & 1) {
            OutFile.Push("\n", 1);
        }
    }

    // Sort string table
    if (!RepressWarnings && SymbolTableType != FILETYPE_MACHO_LE) SortStringTable();

    // Make sorted symbol table, COFF style
    if (SymbolTableType == FILETYPE_COFF) {
        if (NumMembers > 0xFFFF) err.submit(2502);  // Too many members

        // Reuse symbol table header, change size entry
        sprintf(SymTab.FileSize, "%u ", Index2Size);

        // Remove terminating zeroes
        for (i = 0; i < sizeof(SymTab); i++) {
            if (((char*)&SymTab)[i] == 0) ((char*)&SymTab)[i] = ' ';
        }
        // Store header
        OutFile.Push(&SymTab, sizeof(SymTab));

        // Store number of members
        OutFile.Push(&NumMembers, sizeof(NumMembers));

        // Store member offsets
        for (i = 0; i < NumMembers; i++) {
            MemberOffset = Indexes[i] + FirstMemberOffset;
            OutFile.Push(&MemberOffset, sizeof(MemberOffset));
        }

        // Store number of symbols
        OutFile.Push(&NumStrings, sizeof(NumStrings));

        // Store member index for each string
        // Loop through strings
        for (i = 0; i < NumStrings; i++) {
            // Get record in temporary symbol table
            SStringEntry * psym = &StringEntries[i];
            // Get member index, 16 bits
            uint16 MemberI = (uint16)(psym->Member + 1);
            OutFile.Push(&MemberI, sizeof(MemberI));
        }

        // Store strings
        OutFile.Push(StringBuffer.Buf(), StringBuffer.GetDataSize());
        // Align by 2
        if (OutFile.GetDataSize() & 1) {
            OutFile.Push("\n", 1);
        }
    }

    // Make longnames table member for COFF or ELF output
    // (The decision whether to include a "//" longnames member is taken above)
    if (LongnamesMemberSize) {
        // reuse SymTab
        strcpy(SymTab.Name, "//       ");    // Name = "//"
        memset(SymTab.FileSize, ' ', 10);
        sprintf(SymTab.FileSize, "%u", LongNamesBuffer.GetDataSize());

        // Remove terminating zeroes
        for (i = 0; i < sizeof(SymTab); i++) {
            if (((char*)&SymTab)[i] == 0) ((char*)&SymTab)[i] = ' ';
        }
        // Store header
        OutFile.Push(&SymTab, sizeof(SymTab));
        // Store data
        OutFile.Push(LongNamesBuffer.Buf(), LongNamesBuffer.GetDataSize());
        // Align by 2
        if (OutFile.GetDataSize() & 1) {
            OutFile.Push("\n", 1);
        }
    }

    // Make sorted or unsorted symbol table, Mach-O style
    if ((SymbolTableType & 0xFFFF) == FILETYPE_MACHO_LE) {

        if (SymbolTableType & 0x10000000) {
            // Sorted table. "__.SYMDEF SORTED" stored as long name
            memcpy(SymTab.Name, "#1/20           ", 16);
            // Put file size into symbol table header, including long name length
            sprintf(SymTab.FileSize, "%u ", Index3Size + LongNameSize);
        }
        else {
            // Unsorted table. "__.SYMDEF" stored as short name
            memcpy(SymTab.Name, "__.SYMDEF       ", 16);
            // Put file size into symbol table header
            sprintf(SymTab.FileSize, "%u ", Index3Size);
        }

        // Remove terminating zeroes
        for (i = 0; i < sizeof(SymTab); i++) {
            if (((char*)&SymTab)[i] == 0) ((char*)&SymTab)[i] = ' ';
        }

        // Store header
        OutFile.Push(&SymTab, sizeof(SymTab));

        if (SymbolTableType & 0x10000000) {
            // Store long name "__.SYMDEF SORTED"
            OutFile.Push("__.SYMDEF SORTED\0\0\0\0", LongNameSize);
        }

        // Store an array of records of string index and member offsets
        // Store length first
        uint32 ArrayLength = NumStrings * sizeof(SStringEntry);
        OutFile.Push(&ArrayLength, sizeof(ArrayLength));

        // Loop through strings
        for (i = 0; i < NumStrings; i++) {
            // Get record in temporary symbol table
            SStringEntry * psym = &StringEntries[i];
            SStringEntry Record;
            Record.String = psym->String;
            Record.Member = Indexes[psym->Member] + FirstMemberOffset;
            // Store symbol record
            OutFile.Push(&Record, sizeof(Record));
        }

        // Store length of string table
        StringsLen = Round4(StringsLen);             // Round up to align by 4
        OutFile.Push(&StringsLen, sizeof(StringsLen));
        // Store strings
        OutFile.Push(StringBuffer.Buf(), StringBuffer.GetDataSize());
        // Align by 4
        OutFile.Align(4);
        // Cross check precalculated size (8 is the size of "!<arch>\n" file identifier) 
        if (OutFile.GetDataSize() != Index3Size + sizeof(SymTab) + 8 + LongNameSize) err.submit(9000);
    }
}


void CLibrary::MakeBinaryFile() {
    if (cmd.OutputType == FILETYPE_OMF) {
        MakeBinaryFileOMF();                       // OMF style output library
    }
    else {
        MakeBinaryFileUNIX();                      // UNIX style output library
    }
}


void CLibrary::MakeBinaryFileOMF() {
    // Make OMF library
    uint32 PageSize;                              // Page size / alignment for output library
    uint32 temp;                                  // Temporary
    uint16 temp16;                                // Temporary
    uint8  temp8;                                 // Temporary
    uint32 MemberI;                               // Member number
    uint32 MemberOffset;                          // File offset of member in output file
    uint32 MemberStart;                           // Start of member in DataBuffer
    uint32 MemberEnd;                             // End of member in DataBuffer
    uint32 SymbolI;                               // Public symbol number
    uint32 DictionaryOffset2;                     // Offset to hash table
    CSList<uint32> MemberPageIndex;               // Remember page index of each member

    // Check number of entries
    if (DataBuffer.GetNumEntries() >= 0x8000) {
        err.submit(2606);  return;                 // Error: too big
    }

    // Find optimal page size
    PageSize = DataBuffer.GetDataSize() / (0x8000 - DataBuffer.GetNumEntries());
    // Make power of 2, minimum 16
    temp = FloorLog2(PageSize) + 1;
    if (temp < 4) temp = 4;
    PageSize = 1 << temp;

    // Make library header
    temp8 = OMF_LIBHEAD;
    OutFile.Push(&temp8, 1);                      // Library header type byte
    temp16 = PageSize - 3;
    OutFile.Push(&temp16, 2);                     // Record length
    OutFile.Push(0, 6);                           // Dictionary offset and size: insert later
    temp8 = 1;
    OutFile.Push(&temp8, 1);                      // Flag: case sensitive
    OutFile.Align(PageSize);                      // Align for first member

    // Allocate MemberPageIndex
    MemberPageIndex.SetNum(Indexes.GetNumEntries());

    // Insert members
    for (MemberI = 0; MemberI < Indexes.GetNumEntries(); MemberI++) {

        // Find member in DataBuffer 
        MemberStart = Indexes[MemberI];            // Start of member in DataBuffer
        if (MemberI+1 < Indexes.GetNumEntries()) {
            // Not last member
            MemberEnd = Indexes[MemberI+1];         // End of member in DataBuffer = start of next member
        }
        else {
            // Last member
            MemberEnd = DataBuffer.GetDataSize();   // End of member in DataBuffer = end of DataBuffer
        }

        // Put member into output file
        MemberOffset = OutFile.Push(DataBuffer.Buf() + MemberStart, MemberEnd - MemberStart);

        // Align next member
        OutFile.Align(PageSize);

        // Member page index
        MemberPageIndex[MemberI] = MemberOffset / PageSize;
    }

    // Change member index to member page index in StringEntries
    // Loop through StringEntries
    for (SymbolI = 0; SymbolI < StringEntries.GetNumEntries(); SymbolI++) {
        // Member index
        MemberI = StringEntries[SymbolI].Member;
        if (MemberI < MemberPageIndex.GetNumEntries()) {
            // Change to member page
            StringEntries[SymbolI].Member = MemberPageIndex[MemberI];
        }
    }

    // Make OMF_LIBEND record
    temp8 = OMF_LIBEND;
    OutFile.Push(&temp8, 1);                      // Library header type byte
    temp16 = PageSize - 3;                        // Length of rest of record
    OutFile.Push(&temp16, 2);                     // Record length
    OutFile.Align(PageSize);                      // Align

    // Offset to hash table
    DictionaryOffset2 = OutFile.GetDataSize();

    // Make hash table for public symbols
    COMFHashTable HashTable;
    HashTable.MakeHashTable(StringEntries, StringBuffer, OutFile, this); // Make hash table

    // Insert missing values in library header
    // Hash table offset
    OutFile.Get<uint32>(3) = DictionaryOffset2;

    // Hash table size
    OutFile.Get<uint16>(7) = (OutFile.GetDataSize() - DictionaryOffset2) / OMFBlockSize;
}


void CLibrary::MakeBinaryFileUNIX() {
    // Make UNIX library
    // Combine string index and members into binary file

    // Reserve file buffer for output file
    OutFile.SetSize(GetBufferSize());

    if (cmd.OutputType == FILETYPE_COFF || cmd.OutputType == FILETYPE_ELF || cmd.OutputType == FILETYPE_MACHO_LE) {
        // COFF, ELF and MAach-O libraries all use Unix-style archive with 
        // differences in symbol table format

        // Make symbol table
        MakeSymbolTableUnix();

        // Store all members
        OutFile.Push(DataBuffer.Buf(), DataBuffer.GetDataSize());
    }
    else {
        err.submit(2501, GetFileFormatName(cmd.OutputType));
    }
}


void CLibrary::CheckOMFHash(CMemoryBuffer &stringbuf, CSList<SStringEntry> &index) {
    // Check if OMF library hash table has correct entries for all symbol names
    uint32 i;                                     // Loop counter
    int8 * Name;                                  // Public symbol name
    COMFHashTable HashTab;                        // OMF hash table interpreter
    uint32 NString;                               // Number of occurrences of Name in hash table
    uint32 Module;                                // Module with first occurrence of Name
    uint32 Conf, ConfSum = 0;                     // Count number of conflicting entries in hash table

    // Initialize hash table interpreter
    HashTab.Init(&Get<SOMFHashBlock>(DictionaryOffset), DictionarySize);

    // Loop through public symbol names
    for (i = 0; i < index.GetNumEntries(); i++) {
        // Get public name
        Name = stringbuf.Buf() + index[i].String;
        // Make hash
        HashTab.MakeHash(Name);
        // Search for string
        NString = HashTab.FindString(Module, Conf);
        // Count conflicting strings
        ConfSum += Conf;

        // Make error message if not 1 occurrence of Name
        if (NString == 0) err.submit(2603, Name);  // Error if not found
        if (NString >  1) err.submit(1213, NString, Name);  // Warning more than one occurrence

        //printf("\n%i occurence of %s, module offset %i", NString, Name, Module);
    }
    printf("\n\nHash table %i blocks x 37 buckets at offet 0x%X.\n Efficiency: %i conflicts for %i entries",
        DictionarySize, DictionaryOffset, ConfSum, index.GetNumEntries());
}


const char * CLibrary::GetModuleName(uint32 Index) {
    // Get name of module from index (UNIX) or page index (OMF)
    static char name[32];
    if (cmd.OutputType == FILETYPE_OMF || cmd.OutputType == FILETYPE_OMFLIBRARY) {
        // Get name of module in OMF library
        if (Index * PageSize < OutFile.GetDataSize() && OutFile.Get<uint8>(Index * PageSize) == OMF_THEADR) {
            SOMFRecordPointer rec;                     // Record pointer
            rec.Start(OutFile.Buf(), Index * PageSize, OutFile.GetDataSize());
            if (rec.Type2 == OMF_THEADR) {
                // Get module name from THEADR record
                strncpy(name, rec.GetString(), 16);
                // Make sure name is not too long
                name[16] = 0;
                // Return name
                return name;
            }
        }
        // No module starts here
        return "?";
    }
    // UNIX style library.
    if (Index < Indexes.GetNumEntries()) {
        // Get offset from Index
        uint32 Offset = Indexes[Index];
        if (Offset < DataBuffer.GetDataSize()) {
            // Copy name from header
            memcpy(name, DataBuffer.Buf() + Offset, 16);
            // Check for long name
            if (strncmp(name, "#1/", 3) == 0) {
                // Long name after record
                memcpy(name, DataBuffer.Buf()+Offset+sizeof(SUNIXLibraryHeader), 16);
            }
            else if (name[0] == '/') {
                // Long name in longnames record
                uint32 NameIndex = atoi(name+1);
                if (NameIndex < LongNamesSize) {
                    return LongNamesBuffer.Buf() + NameIndex;
                }
                else {
                    return "?";
                }
            }

            // Find terminating '/'
            for (int i = 0; i < 16; i++) if (name[i] == '/') name[i] = 0;
            // Make sure name is not too long
            name[16] = 0;
            // return name
            return name;
        }
    }
    // Error
    return "?";
}
