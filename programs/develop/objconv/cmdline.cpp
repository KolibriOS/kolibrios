/****************************  cmdline.cpp  **********************************
* Author:        Agner Fog
* Date created:  2006-07-25
* Last modified: 2018-01-29
* Project:       objconv
* Module:        cmdline.cpp
* Description:
* This module is for interpretation of command line options
* Also contains symbol change function
*
* Copyright 2006-2018 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#include "stdafx.h"

// List of recognized output file type options
static SIntTxt TypeOptionNames[] = {
    {CMDL_OUTPUT_ELF,   "elf"},
    {CMDL_OUTPUT_PE,    "pe"},
    {CMDL_OUTPUT_PE,    "coff"},
    {CMDL_OUTPUT_PE,    "cof"},
    {CMDL_OUTPUT_PE,    "win"},
    {CMDL_OUTPUT_OMF,   "omf"},
    {CMDL_OUTPUT_MACHO, "mac"},
    {CMDL_OUTPUT_MACHO, "macho"},
    {CMDL_OUTPUT_MACHO, "mach-o"},
    {CMDL_OUTPUT_MACHO, "mach"},
    {CMDL_OUTPUT_MASM,  "asm"},
    {CMDL_OUTPUT_MASM,  "masm"},
    {CMDL_OUTPUT_MASM,  "tasm"},
    {CMDL_OUTPUT_MASM,  "nasm"},
    {CMDL_OUTPUT_MASM,  "yasm"},
    {CMDL_OUTPUT_MASM,  "gasm"},
    {CMDL_OUTPUT_MASM,  "gas"}
};

// List of subtype names
static SIntTxt SubtypeNames[] = {
    {SUBTYPE_MASM,  "asm"},
    {SUBTYPE_MASM,  "masm"},
    {SUBTYPE_MASM,  "tasm"},
    {SUBTYPE_YASM,  "nasm"},
    {SUBTYPE_YASM,  "yasm"},
    {SUBTYPE_GASM,  "gasm"},
    {SUBTYPE_GASM,  "gas"}
};

// List of standard names that are always translated
const uint32 MaxType = FILETYPE_MACHO_LE;

// Standard names in 32-bit mode
const char * StandardNames32[][MaxType+1] = {
    //  0,    COFF,          OMF,           ELF,                MACHO
    {0,"___ImageBase","___ImageBase","__executable_start","__mh_execute_header"}
};

// Standard names in 64-bit mode
// COFF removes an underscore in 32-bit. There is no 64-bit OMF 
const char * StandardNames64[][MaxType+1] = {
    //  0,    COFF,       OMF,         ELF,                MACHO
    {0,"__ImageBase",  "",    "__executable_start","__mh_execute_header"}
};

const int NumStandardNames = sizeof(StandardNames32) / sizeof(StandardNames32[0]);


// Command line interpreter
CCommandLineInterpreter cmd;                  // Instantiate command line interpreter

CCommandLineInterpreter::CCommandLineInterpreter() {
    // Default constructor
    memset(this, 0, sizeof(*this));            // Set all to zero
    Verbose        = CMDL_VERBOSE_YES;         // How much diagnostics to print on screen
    DumpOptions    = DUMP_NONE;                // Dump options
    DebugInfo      = CMDL_DEBUG_DEFAULT;       // Strip or convert debug info
    ExeptionInfo   = CMDL_EXCEPTION_DEFAULT;   // Strip or preserve exception handling info
    SegmentDot     = CMDL_SECTIONDOT_NOCHANGE; // Change underscore/dot in beginning of segment names
    Underscore     = CMDL_UNDERSCORE_NOCHANGE; // Add/remove underscores in symbol names
    LibraryOptions = CMDL_LIBRARY_DEFAULT;     // Library options
}


CCommandLineInterpreter::~CCommandLineInterpreter() { // Destructor
}


void CCommandLineInterpreter::ReadCommandLine(int argc, char * argv[]) {

    // Read command line
    for (int i = 1; i < argc; i++) {
        ReadCommandItem(argv[i]);
    }
    if (ShowHelp || (InputFile == 0 && OutputFile == 0) /* || !OutputType */) {
        // No useful command found. Print help
        Help();  ShowHelp = 1;
        return;
    }
    // Check file options
    FileOptions = CMDL_FILE_INPUT;
    if (LibraryOptions == CMDL_LIBRARY_ADDMEMBER) {
        // Adding object files to library. Library may not exist
        FileOptions = CMDL_FILE_IN_IF_EXISTS;
    }
    if (DumpOptions || ((LibraryOptions & CMDL_LIBRARY_EXTRACTMEM) && !(LibraryOptions & CMDL_LIBRARY_ADDMEMBER))) {
        // Dumping or extracting. Output file not used
        if (OutputFile) err.submit(1103); // Output file name ignored
        OutputFile = 0;
    }
    else {
        // Output file required
        FileOptions |= CMDL_FILE_OUTPUT;
    }
    if ((LibraryOptions & CMDL_LIBRARY_ADDMEMBER) && !(LibraryOptions & CMDL_LIBRARY_CONVERT)) {
        // Adding library members only. Output file may have same name as input file
        FileOptions |= CMDL_FILE_IN_OUT_SAME;
    }
    // Check output type
    if (!OutputType) {
        // Output type not defined yet
        if (LibraryOptions & (CMDL_LIBRARY_CONVERT | CMDL_LIBRARY_ADDMEMBER)) {
            OutputType = FILETYPE_LIBRARY;
        }
    }
}


void CCommandLineInterpreter::ReadCommandItem(char * string) {
    // Read one option from command line
    // Skip leading whitespace
    while (*string != 0 && *string <= ' ') string++;
    if (*string == 0) return;  // Empty string

    // Look for option prefix and response file prefix
    const char OptionPrefix1 = '-';  // Option must begin with '-'
#if defined (_WIN32) || defined (__WINDOWS__)
    const char OptionPrefix2 = '/';  // '/' allowed instead of '-' in Windows only
#else
    const char OptionPrefix2 = '-';
#endif
    const char ResponseFilePrefix = '@';  // Response file name prefixed by '@'
    if (*string == OptionPrefix1 || *string == OptionPrefix2) {
        // Option prefix found. This is a command line option
        InterpretCommandOption(string+1);
    }
    else if (*string == ResponseFilePrefix) {
        // Response file prefix found. Read more options from response file
        ReadCommandFile(string+1);
    }
    else {
        // No prefix found. This is an input or output file name
        InterpretFileName(string);
    }

    int loc, last = SymbolList.GetNumEntries() - 1;
    if (last > 0) {
        // relocate last entry (binary insertion sort):
        SSymbolChange * List = (SSymbolChange *)SymbolList.Buf();
        SSymbolChange key = List[last];
        SymbolBinSearch(key.Name1, last, &loc);
        memmove(List + loc + 1, List + loc, (last - loc) * sizeof(SSymbolChange));
        List[loc] = key;
    }
}


void CCommandLineInterpreter::ReadCommandFile(char * filename) {
    // Read commands from file
    if (*filename <= ' ') {
        err.submit(1001); return;    // Warning: empty filename
    }

    // Check if too many response file buffers (possibly because file includes itself)
    if (++NumBuffers > MAX_COMMAND_FILES) {err.submit(2107); return;}

    // Allocate buffer for response files.
    if (ResponseFiles.GetNumEntries() == 0) {
        ResponseFiles.SetNum(MAX_COMMAND_FILES);
        ResponseFiles.SetZero();
    }

    // Read response file into new buffer
    ResponseFiles[NumBuffers-1].FileName = filename;
    ResponseFiles[NumBuffers-1].Read();

    // Get buffer with file contents
    char * buffer = ResponseFiles[NumBuffers-1].Buf();
    char * ItemBegin, * ItemEnd;  // Mark begin and end of token in buffer

    // Check if buffer is allocated
    if (buffer) {

        // Parse contents of response file for tokens
        while (*buffer) {

            // Skip whitespace
            while (*buffer != 0 && uint8(*buffer) <= uint8(' ')) buffer++;
            if (*buffer == 0) break; // End of buffer found
            ItemBegin = buffer;

            // Find end of token
            ItemEnd = buffer+1;
            while (uint8(*ItemEnd) > uint8(' ')) ItemEnd++;
            if (*ItemEnd == 0) {
                buffer = ItemEnd;
            }
            else {
                buffer = ItemEnd + 1;
                *ItemEnd = 0;    // Mark end of token
            }
            // Found token. 
            // Check if it is a comment beginning with '#' or '//'
            if (ItemBegin[0] == '#' || (ItemBegin[0] == '/' && ItemBegin[1] == '/' )) {
                // This is a comment. Skip to end of line
                ItemEnd = buffer;
                while (*ItemEnd != 0 && *ItemEnd != '\n') {
                    ItemEnd++;
                }
                if (*ItemEnd == 0) {
                    buffer = ItemEnd;
                }
                else {
                    buffer = ItemEnd + 1;
                }
                continue;
            }
            // Not a comment. Interpret token
            ReadCommandItem(ItemBegin);
        }
    }
}


void CCommandLineInterpreter::InterpretFileName(char * string) {
    // Interpret input or output filename from command line

    switch (libmode) {
    case 1:            // First filename after -lib = inputfile and outputfile
        InputFile = string;
        libmode = 2;
        return;

    case 2:            // Second or later filename after -lib = object file to add to library
        AddObjectToLibrary(string, string);
        return;
    }
    // libmode = 0: Ordinary input or output file

    if (!InputFile) {
        // Input file not specified yet
        InputFile = string;
    }
    else if (!OutputFile) {
        // Output file not specified yet
        OutputFile = string;
    }
    else {
        // Both input and output files already specified
        err.submit(2001);
    }
}


void CCommandLineInterpreter::InterpretCommandOption(char * string) {
    // Interpret one option from command line
    if (*string <= ' ') {
        err.submit(1001); return;    // Warning: empty option
    }

    // Detect option type
    switch(string[0]) {
    case 'f': case 'F':   // output file format
        if (string[1] == 'd') {
            // -fd == deprecated dump option
            InterpretDumpOption(string+2);  break;
        }
        InterpretOutputTypeOption(string+1);  break;

    case 'v': case 'V':   // verbose/silent
        InterpretVerboseOption(string+1);  break;

    case 'd': case 'D':   // dump option
        InterpretDumpOption(string+1);  break;
        // Debug info option
        //InterpretDebugInfoOption(string+1);  break;

    case 'x': case 'X':   // Exception handler info option
        InterpretExceptionInfoOption(string+1);  break;

    case 'h': case 'H': case '?':  // Help
        ShowHelp = 1;  break;

    case 'e': case 'E':   // Error option
    case 'w': case 'W':   // Warning option
        InterpretErrorOption(string);  break;

    case 'n': case 'N':   // Symbol name change option
    case 'a': case 'A':   // Symbol name alias option
        InterpretSymbolNameChangeOption(string);  break;

    case 'i': case 'I':   // Imagebase
        if ((string[1] | 0x20) == 'm') {
            InterpretImagebaseOption(string);
        }
        break;

    case 'l': case 'L':   // Library option
        InterpretLibraryOption(string);  break;

    case 'c':  // Count instruction codes supported
        // This is an easter egg: You can only get it if you know it's there
        if (strncmp(string,"countinstructions", 17) == 0) {
            CDisassembler::CountInstructions();
            exit(0);
        }

    default:    // Unknown option
        err.submit(1002, string);
    }
}


void CCommandLineInterpreter::InterpretLibraryOption(char * string) {
    // Interpret options for manipulating library/archive files

    // Check for -lib command
    if (stricmp(string, "lib") == 0) {  // Found -lib command
        if (InputFile) {
            libmode = 2;                  // Input file already specified. Remaining file names are object files to add
        }
        else {
            libmode = 1;                  // The rest of the command line must be interpreted as library name and object file names
        }
        return;
    }

    SSymbolChange sym = {0,0,0,0};      // Symbol change record
    int i;                              // Loop counter

    // Check for member name and optional new name in this command
    char * name1 = 0, * name2 = 0, separator;
    if ((string[2] == ':' || string[2] == '|') && string[3]) {
        // name1 found
        separator = string[2];
        name1 = string+3;
        // Search for second separator or end
        name2 = name1 + 1;
        while (name2[0] != 0) {
            if (name2[0] == separator) {
                *name2 = 0;  // Mark end of name1
                if (name2[1]) {
                    // name2 found
                    name2++;     // Name2 starts here
                    break;
                }
            }
            name2++;
        }
        if (name2 == 0 || name2[0] == 0 || name2[0] == separator) {
            // name 2 is blank
            //name2 = name1;
            name2 = 0;
        }
        else {
            // Check if name2 ends with separator
            for (i = 0; i < (int)strlen(name2); i++) {
                if (name2[i] == separator) name2[i] = 0;
            }
        }
    }
    // Check for duplicate name
    if (SymbolIsInList(name1)) {
        // This symbol is already in list
        err.submit(2017, name1);
        return;
    }

    sym.Name1 = name1;     // Store names in symbol change record
    sym.Name2 = name2;     

    switch (string[1]) {
    case 'a': case 'A':      // Add input file to library
        if (name1) {
            AddObjectToLibrary(name1, name2);
        }
        else err.submit(2004, string);
        break;

    case 'x': case 'X':      // Extract member(s) from library
        if (name1) {
            // Extract specified member
            cmd.LibraryOptions = CMDL_LIBRARY_EXTRACTMEM;
            sym.Action  = SYMA_EXTRACT_MEMBER;
            SymbolList.Push(&sym, sizeof(sym));
        }
        else {
            // Extract all members
            cmd.LibraryOptions = CMDL_LIBRARY_EXTRACTALL;
        }
        break;

    case 'd': case 'D':  // Delete member from library
        if (name1) {
            // Delete specified member
            cmd.LibraryOptions = CMDL_LIBRARY_CONVERT;
            sym.Action  = SYMA_DELETE_MEMBER;
            SymbolList.Push(&sym, sizeof(sym));
        }
        else err.submit(2004, string);
        break;

    case 's': case 'S':  // Use short member names for compatibility
        cmd.LibrarySubtype = LIBTYPE_SHORTNAMES;
        break;

    default:
        err.submit(2004, string);  // Unknown option
    }
}


void CCommandLineInterpreter::AddObjectToLibrary(char * filename, char * membername) {
    // Add object file to library 
    if (!filename || !*filename) {          
        err.submit(2004, filename-1);  return;     // Empty string
    }

    if (!membername || !*membername) membername = filename;

    SSymbolChange Sym = {0,0,0,0};                // Symbol change record

    Sym.Name2 = filename;                         // Object file name

    if (!MemberNamesAllocated) {
        // Allocate space for truncated member names
        const int SafetySpace = 1024;

        // Get size of response files
        if (ResponseFiles.GetNumEntries()) {
            MemberNamesAllocated = ResponseFiles[0].GetDataSize() + ResponseFiles[1].GetDataSize();
        }
        // Allocate this size + SafetySpace
        MemberNames.SetSize(MemberNamesAllocated + SafetySpace);

        // Remember allocated buffer size
        MemberNamesAllocated = MemberNames.GetBufferSize();
    }

    // Truncate name and store it in MemberNames
    //uint32 Name1Offset = MemberNames.PushString(CLibrary::TruncateMemberName(membername));
    uint32 Name1Offset = MemberNames.PushString(membername);   
    Sym.Name1 = (char*)(MemberNames.Buf() + Name1Offset);
    CLibrary::StripMemberName(Sym.Name1);

    // Note: Sym.Name1 points to allocated memory in violation of good programming practice.
    // Check that it is not reallocated:
    if (MemberNames.GetBufferSize() != MemberNamesAllocated) {
        err.submit(2506); // Cannot reallocate MemberNames because we have pointers to in in SymbolList
        return;
    }

    // Check for duplicate name
    if (SymbolIsInList(Sym.Name1)) {
        // This symbol is already in list
        err.submit(2017, Sym.Name1);
        return;
    }

    // Store options
    cmd.LibraryOptions |= CMDL_LIBRARY_ADDMEMBER;
    Sym.Action  = SYMA_ADD_MEMBER;

    // Store SYMA_ADD_MEMBER record in symbol list
    SymbolList.Push(&Sym, sizeof(Sym));
}


void CCommandLineInterpreter::InterpretOutputTypeOption(char * string) {
    // Interpret output file format option from command line

    int opt;
    for (opt = 0; opt < TableSize(TypeOptionNames); opt++) {
        int len = (int)strlen(TypeOptionNames[opt].b);
        if (strncmp(string, TypeOptionNames[opt].b, len) == 0) {
            // Match found
            if (OutputType)  err.submit(2003, string);  // More than one output type specified
            if (DumpOptions) err.submit(2007);          // Both dump and convert specified

            // Save desired output type
            OutputType = TypeOptionNames[opt].a;

            // Check if name is followed by a word size
            int wordsize = 0;
            if (string[len]) wordsize = atoi(string+len);
            switch (wordsize) {
            case 0:  // No word size specified
                break;

            case 32: case 64:  // Valid word size
                DesiredWordSize = wordsize;
                break;

            default:  // Illegal word size
                err.submit(2002, wordsize);
            }
            break;   // Finished searching
        }
    }

    // Check if found
    if (opt >= TableSize(TypeOptionNames)) err.submit(2004, string-1);

    if (OutputType == CMDL_OUTPUT_MASM) {
        // Get subtype
        for (opt = 0; opt < TableSize(SubtypeNames); opt++) {
            int len = (int)strlen(SubtypeNames[opt].b);
            if (strncmp(string, SubtypeNames[opt].b, len) == 0) {
                // Match found
                SubType = SubtypeNames[opt].a;  break;
            }
        }
    }
}


void CCommandLineInterpreter::InterpretVerboseOption(char * string) {
    // Interpret silent/verbose option from command line
    Verbose = atoi(string);
}


void CCommandLineInterpreter::InterpretDumpOption(char * string) {
    // Interpret dump option from command line
    if (OutputType || DumpOptions) err.submit(2007);          // Both dump and convert specified

    char * s1 = string;
    while (*s1) {
        switch (*(s1++)) {
        case 'f': case 'F':  // dump file header
            DumpOptions |= DUMP_FILEHDR;  break;
        case 'h': case 'H':  // dump section headers
            DumpOptions |= DUMP_SECTHDR;  break;
        case 's': case 'S':  // dump symbol table
            DumpOptions |= DUMP_SYMTAB;  break;
        case 'r': case 'R':  // dump relocations
            DumpOptions |= DUMP_RELTAB;  break;
        case 'n': case 'N':  // dump string table
            DumpOptions |= DUMP_STRINGTB;  break;
        case 'c': case 'C':  // dump comment records (currently only for OMF)
            DumpOptions |= DUMP_COMMENT;  break;         
        default:
            err.submit(2004, string-1);  // Unknown option
        }
    }
    if (DumpOptions == 0) DumpOptions = DUMP_FILEHDR;
    OutputType = CMDL_OUTPUT_DUMP;
    if (OutputType && OutputType != CMDL_OUTPUT_DUMP) err.submit(2007); // Both dump and convert specified
    OutputType = CMDL_OUTPUT_DUMP;
}


void CCommandLineInterpreter::InterpretDebugInfoOption(char * string) {
    // Interpret debug info option from command line
    if (strlen(string) > 1) err.submit(2004, string-1);  // Unknown option
    switch (*string) {
    case 's': case 'S': case 'r': case 'R':  // Strip (remove)
        DebugInfo = CMDL_DEBUG_STRIP;  break;
    case 'p': case 'P':                      // Preserve
        DebugInfo = CMDL_DEBUG_PRESERVE;  break;
    case 'l': case 'L':                      // (Not supported)
        DebugInfo = CMDL_DEBUG_LINNUM;  break;
    case 'c': case 'C':                      // (Not supported)
        DebugInfo = CMDL_DEBUG_SYMBOLS;  break;
    default:
        err.submit(2004, string-1);  // Unknown option
    }
}


void CCommandLineInterpreter::InterpretExceptionInfoOption(char * string) {
    // Interpret exception handler info option from command line
    if (strlen(string) > 1) err.submit(2004, string-1);  // Unknown option
    switch (*string) {
    case 's': case 'S': case 'r': case 'R':  // Strip (remove)
        ExeptionInfo = CMDL_EXCEPTION_STRIP;  break;
    case 'p': case 'P':                      // Preserve
        ExeptionInfo = CMDL_EXCEPTION_PRESERVE;  break;
    default:
        err.submit(2004, string-1);  // Unknown option
    }
}


void CCommandLineInterpreter::InterpretErrorOption(char * string) {
    // Interpret warning/error option from command line
    if (strlen(string) < 3) {
        err.submit(2004, string); return; // Unknown option
    } 
    int newstatus;   // New status for this error number

    switch (string[1]) {
    case 'd': case 'D':  // Disable
        newstatus = 0;  break;

    case 'w': case 'W':  // Treat as warning
        newstatus = 1;  break;

    case 'e': case 'E':  // Treat as error
        newstatus = 2;  break;

    default:
        err.submit(2004, string);  // Unknown option
        return;
    }
    if (string[2] == 'x' || string[2] == 'X') {
        // Apply new status to all non-fatal messages
        for (SErrorText * ep = ErrorTexts; ep->Status < 9; ep++) {
            ep->Status = newstatus;  // Change status of all errors
        }
    }
    else {
        int ErrNum = atoi(string+2);
        if (ErrNum == 0 && string[2] != '0') {
            err.submit(2004, string);  return; // Unknown option
        }
        // Search for this error number
        SErrorText * ep = err.FindError(ErrNum);
        if (ep->Status & 0x100) {
            // Error number not found
            err.submit(1003, ErrNum);  return; // Unknown error number
        }
        // Change status of this error
        ep->Status = newstatus;
    }
}

void CCommandLineInterpreter::InterpretSymbolNameChangeOption(char * string) {
    // Interpret various options for changing symbol names
    SSymbolChange sym = {0,0,0,0};   // Symbol change record
    string[0] |= 0x20;  // change first letter to lower case

    // Check for symbol names in this command
    char * name1 = 0, * name2 = 0;
    if (string[2] == ':' && string[3]) {
        // name1 found
        name1 = string+3;
        // Search for second ':' or end
        name2 = name1 + 1;
        while (name2[0] != 0) {
            if (name2[0] == ':') {
                *name2 = 0;  // Mark end of name1
                if (name2[1]) {
                    // name2 found
                    name2++;     // Name2 starts here
                    break;
                }
            }
            name2++;
        }
        if (name2 && name2[0]) {
            // name2 found. check if it ends with ':'
            for (uint32 i = 0; i < (uint32)strlen(name2); i++) {
                if (name2[i] == ':') name2[i] = 0;
            }
        }
        if (name2[0] == 0) name2 = 0;
    }
    // Check for duplicate name
    if (name1 && SymbolIsInList(name1)) {
        // This symbol is already in list
        err.submit(2015, name1);
        return;
    }

    switch (string[1]) {
    case 'u': case 'U':  // underscore option
        switch (string[2]) {
        case 0:
            Underscore = CMDL_UNDERSCORE_CHANGE; 
            if (string[0] == 'a') Underscore |= CMDL_KEEP_ALIAS;
            break;
        case '+': case 'a': case 'A': 
            Underscore = CMDL_UNDERSCORE_ADD; 
            if (string[0] == 'a') Underscore |= CMDL_KEEP_ALIAS;
            break;
        case '-': case 'r': case 'R': 
            Underscore = CMDL_UNDERSCORE_REMOVE; 
            if (string[0] == 'a') Underscore |= CMDL_KEEP_ALIAS;
            break;
        default:
            err.submit(2004, string);  // Unknown option
        }
        break;

    case 'd': case 'D':  // section name dot option
        SegmentDot = CMDL_SECTIONDOT_CHANGE; 
        break;

    case 'r': case 'R':  // name replace option
        if (name1 == 0 || name2 == 0 || *name1 == 0 || *name2 == 0) {
            err.submit(2008, string); return;
        }
        sym.Name1 = name1;
        sym.Name2 = name2;
        sym.Action  = SYMA_CHANGE_NAME;
        if (string[0] == 'a') sym.Action |= SYMA_ALIAS;
        SymbolList.Push(&sym, sizeof(sym));  SymbolChangeEntries++;
        break;

    case 'p': case 'P':  // prefix replace option
        if (name1 == 0 || *name1 == 0) {
            err.submit(2008, string); return;
        }
        if (name2 == 0) name2 = (char*)"";
        sym.Name1 = name1;
        sym.Name2 = name2;
        sym.Action  = SYMA_CHANGE_PREFIX;
        if (string[0] == 'a') sym.Action |= SYMA_ALIAS;
        PrefixSuffixList.Push(&sym, sizeof(sym));  SymbolChangeEntries++;
        break;

    case 's': case 'S':  // suffix replace option
        if (name1 == 0 || *name1 == 0) {
            err.submit(2008, string); return;
        }
        if (name2 == 0) name2 = (char*)"";
        sym.Name1 = name1;
        sym.Name2 = name2;
        sym.Action  = SYMA_CHANGE_SUFFIX;
        if (string[0] == 'a') sym.Action |= SYMA_ALIAS;
        PrefixSuffixList.Push(&sym, sizeof(sym));  SymbolChangeEntries++;
        break;

    case 'w': case 'W':  // Weaken symbol
        if (name1 == 0 || *name1 == 0 || name2) {
            err.submit(2009, string); return;
        }
        sym.Name1 = name1;
        sym.Action  = SYMA_MAKE_WEAK;
        SymbolList.Push(&sym, sizeof(sym));  SymbolChangeEntries++;
        break;

    case 'l': case 'L':  // Make symbol local or hidden
        if (name1 == 0 || *name1 == 0 || name2) {
            err.submit(2009, string); return;
        }
        sym.Name1 = name1;
        sym.Action  = SYMA_MAKE_LOCAL;
        SymbolList.Push(&sym, sizeof(sym));  SymbolChangeEntries++;
        break;

    default:
        err.submit(2004, string);  // Unknown option
    }
}

void CCommandLineInterpreter::InterpretImagebaseOption(char * string) {
    // Interpret image base option
    char * p = strchr(string, '=');
    if ((strnicmp(string, "imagebase", 9) && strnicmp(string, "image_base", 10)) || !p) {
        // Unknown option
        err.submit(1002, string);
        return;
    }
    if (ImageBase) err.submit(2330); // Imagebase specified more than once

    p++;  // point to number following '='
    // Loop through string to interpret hexadecimal number
    while (*p) {
        char letter = *p | 0x20; // lower case letter
        if (*p >= '0' && *p <= '9') {
            // 0 - 9 hexadecimal digit
            ImageBase = (ImageBase << 4) + *p - '0';
        }
        else if (letter >= 'a' && letter <= 'f') {
            // A - F hexadecimal digit
            ImageBase = (ImageBase << 4) + letter - 'a' + 10;
        }
        else if (letter == 'h') {
            // Hexadecimal number may end with 'H'
            break;
        }
        else if (letter == 'x' || letter == ' ') {
            // Hexadecimal number may begin with 0x
            if (ImageBase) {
                // 'x' preceded by number other than 0
                err.submit(1002, string); break;
            }
        }
        else {
            // Any other character not allowed
            err.submit(1002, string); break;
        }
        // next character
        p++;
    }
    if (ImageBase & 0xFFF) {
        // Must be divisible by page size
        err.submit(2331, string);
    }
    if ((int32)ImageBase <= 0) {
        // Cannot be zero or > 2^31
        err.submit(2332, string);
    }
}


SSymbolChange const * CCommandLineInterpreter::GetMemberToAdd() {
    // Get names of object files to add to library
    // replaced will be set to 1 if a member with the same name is replaced

    // Search through SymbolList, continuing from last CurrentSymbol
    while (CurrentSymbol < SymbolList.GetDataSize()) {
        // Get pointer to current symbol record
        SSymbolChange * Sym = (SSymbolChange *)(SymbolList.Buf() + CurrentSymbol);
        // Increment pointer
        CurrentSymbol += sizeof(SSymbolChange);
        // Check record type
        if (Sym->Action == SYMA_ADD_MEMBER) {
            // Name found
            return Sym;
        }
    }
    // No more names found
    return 0;
}


void CCommandLineInterpreter::CheckExtractSuccess() {
    // Check if library members to extract were found

    // Search through SymbolList for extract records
    for (uint32 i = 0; i < SymbolList.GetDataSize(); i += sizeof(SSymbolChange)) {
        SSymbolChange * Sym = (SSymbolChange *)(SymbolList.Buf() + i);
        if (Sym->Action == SYMA_EXTRACT_MEMBER && Sym->Done == 0) {
            // Member has not been extracted
            err.submit(1104, Sym->Name1);
        }
    }
}


void CCommandLineInterpreter::CheckSymbolModifySuccess() {
    // Check if symbols to modify were found

    // Search through SymbolList for symbol change records
    for (uint32 i = 0; i < SymbolList.GetDataSize(); i += sizeof(SSymbolChange)) {
        SSymbolChange * Sym = (SSymbolChange *)(SymbolList.Buf() + i);
        if (Sym->Action >= SYMA_MAKE_WEAK && Sym->Action < SYMA_ADD_MEMBER && Sym->Done == 0) {
            // Member has not been extracted
            err.submit(1106, Sym->Name1);
        }
        if (Sym->Action == SYMA_DELETE_MEMBER && Sym->Done == 0) {
            // Member has not been extracted
            err.submit(1105, Sym->Name1);
        }
    }
}


int CCommandLineInterpreter::SymbolIsInList(char const * name) {
    // Check if name is already in symbol list
    int unused;
    return SymbolBinSearch(name, SymbolList.GetNumEntries(), &unused);
}


int CCommandLineInterpreter::SymbolBinSearch(char const * name, int nsym, int * location) {
    SSymbolChange * List = (SSymbolChange *)SymbolList.Buf();
    int lo = 0, hi = nsym - 1;
    while (lo <= hi) {
        int mid = (lo + hi) >> 1;
        int comp = strcmp(name, List[mid].Name1);
        if (!comp) {
            *location = mid;
            return 1;
        } else if (comp < 0) {
            hi = mid - 1;
        } else {
            lo = mid + 1;
        }
    }
    *location = lo;
    return 0;
}


int CCommandLineInterpreter::SymbolChange(char const * oldname, char const ** newname, int symtype) {
    // Check if symbol has to be changed
    int action = 0, i, isym;
    int nsym = SymbolList.GetNumEntries();
    int n_prefix_suffix = PrefixSuffixList.GetNumEntries();
    if (oldname == 0) return SYMA_NOCHANGE;
    if (newname) *newname = 0;

    // Convert standard names if type conversion
    if (cmd.InputType != cmd.OutputType 
        && uint32(cmd.InputType) <= MaxType && uint32(cmd.OutputType) <= MaxType) {
            if (DesiredWordSize == 32) {
                // Look for standard names to translate, 32-bit
                for (i = 0; i < NumStandardNames; i++) {
                    if (strcmp(oldname, StandardNames32[i][cmd.InputType]) == 0) {
                        // Match found
                        *newname = StandardNames32[i][cmd.OutputType];
                        CountSymbolNameChanges++;
                        return SYMA_CHANGE_NAME; // Change name of symbol
                    }
                }
            }
            else {
                // Look for standard names to translate, 64-bit
                for (i = 0; i < NumStandardNames; i++) {
                    if (strcmp(oldname, StandardNames64[i][cmd.InputType]) == 0) {
                        // Match found
                        *newname = StandardNames64[i][cmd.OutputType];
                        CountSymbolNameChanges++;
                        return SYMA_CHANGE_NAME; // Change name of symbol
                    }
                }
            }
    }

    // See if there are other conversions to do
    if (Underscore == 0 && SegmentDot == 0 && nsym == 0 && n_prefix_suffix == 0) return SYMA_NOCHANGE;  // Nothing to do
    if (oldname == 0 || *oldname == 0) return SYMA_NOCHANGE;                    // No name

    static char NameBuffer[MAXSYMBOLLENGTH];

    // search for name in list of names specified by user on command line
    SSymbolChange * psym;
    int found = SymbolBinSearch(oldname, nsym, &isym);
    if (found) {
        psym = (SSymbolChange *)SymbolList.Buf() + isym;
    } else {
        // Search prefix/suffix match
        psym = (SSymbolChange *)PrefixSuffixList.Buf();
        for (isym = 0; isym < n_prefix_suffix; isym++, psym++) {
            int n1len = (int)strlen(psym->Name1); // Length of string to search for
            int onlen = (int)strlen(oldname);     // Length of string to match
            if ((psym->Action&~SYMA_ALIAS) == SYMA_CHANGE_PREFIX && strncmp(oldname, psym->Name1, n1len)==0) break; // matching prefix found
            if ((psym->Action&~SYMA_ALIAS) == SYMA_CHANGE_SUFFIX && strcmp(oldname+onlen-n1len, psym->Name1)==0) break; // matching suffix found
        }
        found = isym < n_prefix_suffix;
    }

    if (found) {
        // A matching name was found.
        action = psym->Action;
        // Whatever action is specified here is overriding any general option
        // Statistics counting
        switch (action & ~SYMA_ALIAS) {

        case SYMA_MAKE_WEAK: // Make public symbol weak
            if (symtype == SYMT_PUBLIC) {
                CountSymbolsWeakened++;  psym->Done++;
            }
            else { // only public symbols can be weakened
                err.submit(1020, oldname); // cannot make weak
                action = SYMA_NOCHANGE;
            }
            break;

        case SYMA_MAKE_LOCAL: // Hide public or external symbol
            if (symtype == SYMT_PUBLIC || symtype == SYMT_EXTERNAL) {
                CountSymbolsMadeLocal++;  psym->Done++;
                if (symtype == SYMT_EXTERNAL) err.submit(1023, oldname);
            }
            else { // only public and external symbols can be made local
                err.submit(1021, oldname); // cannot make local
                action = SYMA_NOCHANGE;
            }
            break;

        case SYMA_CHANGE_NAME: // Change name of symbol or segment or library member
            CountSymbolNameChanges++;  psym->Done++;  
            *newname = psym->Name2;
            break;

        case SYMA_CHANGE_PREFIX: // Change beginning of symbol name
            if (symtype == SYMT_PUBLIC || symtype == SYMT_EXTERNAL || symtype == SYMT_LOCAL || symtype == SYMT_SECTION) {
                if (strlen(oldname) - strlen(psym->Name1) + strlen(psym->Name2) >= MAXSYMBOLLENGTH) {
                    err.submit(2202, oldname);  // Name too long
                    action = SYMA_NOCHANGE;  break;
                }
                strcpy(NameBuffer, psym->Name2);
                strcpy(NameBuffer + strlen(psym->Name2), oldname + strlen(psym->Name1));
                action = SYMA_CHANGE_NAME;
                *newname = NameBuffer;
                CountSymbolNameChanges++;  psym->Done++;
            }
            else { // only symbols and segments can change prefix
                err.submit(1024, oldname);
                action = SYMA_NOCHANGE;
            }
            break;

        case SYMA_CHANGE_SUFFIX: // Change end of symbol name
            if (symtype == SYMT_PUBLIC || symtype == SYMT_EXTERNAL || symtype == SYMT_LOCAL || symtype == SYMT_SECTION) {
                if (strlen(oldname) - strlen(psym->Name1) + strlen(psym->Name2) >= MAXSYMBOLLENGTH) {
                    err.submit(2202, oldname);  // Name too long
                    action = SYMA_NOCHANGE;  break;
                }
                strcpy(NameBuffer, oldname);
                strcpy(NameBuffer + strlen(oldname) - strlen(psym->Name1), psym->Name2);
                action = SYMA_CHANGE_NAME;
                *newname = NameBuffer;
                CountSymbolNameChanges++;  psym->Done++;
            }
            else { // only symbols and segments can change prefix
                err.submit(1024, oldname);
                action = SYMA_NOCHANGE;
            }
            break;

        case SYMA_EXTRACT_MEMBER:
            *newname = psym->Name2;
            // continue in next case
        case SYMA_DELETE_MEMBER: case SYMA_ADD_MEMBER:
            if (symtype == SYMT_LIBRARYMEMBER) {
                // Change to library member
                psym->Done++;
            }
            else {
                // Ignore action for symbols that have the same name as a library member
                action = SYMA_NOCHANGE;
            }
        }

        if (action && (psym->Action & SYMA_ALIAS)) {
            // Keep old name as alias
            if (symtype == SYMT_PUBLIC) {
                CountSymbolNameAliases++;  psym->Done++;
                action = SYMA_ALIAS;
            }
            else { // only public symbols can have aliases
                CountSymbolNameChanges--;
                err.submit(1022, oldname); // cannot make alias
                action = SYMA_NOCHANGE;
            }
        }

        // Action to take
        return action;
    }

    // Not found in list. Check for section options
    if (symtype == SYMT_SECTION) {
        if (!strncmp(oldname, ".rela", 5)) {
            // ELF relocation section must have same name change as mother section
            const char * name2;
            int action2 = SymbolChange(oldname+5, &name2, symtype);
            if (action2 == SYMA_CHANGE_NAME && strlen(name2) + 6 < MAXSYMBOLLENGTH) {
                sprintf(NameBuffer, ".rela%s", name2);
                *newname = NameBuffer;
                return action2;
            }
        }
        if (!strncmp(oldname, ".rel", 4)) {
            // ELF relocation section must have same name change as mother section
            const char * name2;
            int action2 = SymbolChange(oldname+4, &name2, symtype);
            if (action2 == SYMA_CHANGE_NAME && strlen(name2) + 5 < MAXSYMBOLLENGTH) {
                sprintf(NameBuffer, ".rel%s", name2);
                *newname = NameBuffer;
                return action2;
            }
        }
        if (SegmentDot) {
            // Change section name

            if (SegmentDot == CMDL_SECTIONDOT_U2DOT && oldname[0] == '_') {
                // replace '_' by '.'
                strncpy(NameBuffer, oldname, MAXSYMBOLLENGTH-1);
                NameBuffer[MAXSYMBOLLENGTH-1] = 0;  // Terminate string
                NameBuffer[0] = '.';
                *newname = NameBuffer;
                CountSectionDotConversions++;
                return SYMA_CHANGE_NAME;
            }
            if (SegmentDot == CMDL_SECTIONDOT_DOT2U && oldname[0] == '.') {
                // replace '.' by '_'
                // Note: Microsoft and Intel compilers have . on standard names
                // and _ on nonstandard names in COFF files
                // Borland requires _ on all segment names in OMF files
                /* 
                // Standard section names that should not be changed
                static char const * StandardSectionNames[] = {
                ".text", ".data", ".bss", ".comment", ".lib"
                };
                for (uint32 i = 0; i < sizeof(StandardSectionNames)/sizeof(StandardSectionNames[0]); i++) {
                if (stricmp(oldname,StandardSectionNames[i]) == 0) {
                // Standard name. Don't change
                return SYMA_NOCHANGE;
                }
                }*/
                strncpy(NameBuffer, oldname, MAXSYMBOLLENGTH-1);
                NameBuffer[MAXSYMBOLLENGTH-1] = 0;  // Terminate string
                NameBuffer[0] = '_';
                *newname = NameBuffer;
                CountSectionDotConversions++;
                return SYMA_CHANGE_NAME;
            }
        }
    }

    // Check for underscore options
    if ((Underscore & 0x0F) == CMDL_UNDERSCORE_REMOVE && oldname[0] == '_') {
        // Remove underscore
        if ((Underscore & CMDL_KEEP_ALIAS) && symtype == SYMT_PUBLIC) {
            // Alias only applicable to public symbols
            // Make alias without underscore
            *newname = oldname + 1;
            CountUnderscoreConversions++;  CountSymbolNameAliases++;
            return SYMA_ALIAS;
        }
        // Change name applicable to public and external symbols
        if (symtype == SYMT_PUBLIC || symtype == SYMT_EXTERNAL) {
            // Make new name without underscore
            *newname = oldname + 1;
            CountUnderscoreConversions++;
            return SYMA_CHANGE_NAME;
        }
    }
    if ((Underscore & 0x0F) == CMDL_UNDERSCORE_ADD) {
        // Add underscore even if it already has one
        if ((Underscore & CMDL_KEEP_ALIAS) && symtype == SYMT_PUBLIC) {
            // Alias only applicable to public symbols
            // Make alias with underscore
            strncpy(NameBuffer+1, oldname, MAXSYMBOLLENGTH-2);
            NameBuffer[MAXSYMBOLLENGTH-1] = 0;  // Terminate string
            NameBuffer[0] = '_';
            *newname = NameBuffer;
            CountUnderscoreConversions++;  CountSymbolNameAliases++;
            return SYMA_ALIAS;
        }
        // Change name applicable to public and external symbols
        if (symtype == SYMT_PUBLIC || symtype == SYMT_EXTERNAL) {
            // Make new name with underscore
            strncpy(NameBuffer+1, oldname, MAXSYMBOLLENGTH-2);
            NameBuffer[MAXSYMBOLLENGTH-1] = 0;  // Terminate string
            NameBuffer[0] = '_';
            *newname = NameBuffer;
            CountUnderscoreConversions++;
            return SYMA_CHANGE_NAME;
        }
    }
    return SYMA_NOCHANGE;
}


int CCommandLineInterpreter::SymbolChangesRequested() {
    // Any kind of symbol change requested on command line
    return (Underscore != 0) 
        | (SegmentDot != 0) << 1 
        | (SymbolChangeEntries != 0) << 2;
}


void CCommandLineInterpreter::CountDebugRemoved() {
    // Count debug sections removed
    CountDebugSectionsRemoved++;
}


void CCommandLineInterpreter::CountExceptionRemoved() {
    // Count exception handler sections removed
    CountExceptionSectionsRemoved++;
}


void CCommandLineInterpreter::CountSymbolsHidden() {
    // Count unused external references hidden
    CountUnusedSymbolsHidden++;
}


void CCommandLineInterpreter::ReportStatistics() {
    // Report statistics about name changes etc.
    if (DebugInfo == CMDL_DEBUG_STRIP || ExeptionInfo == CMDL_EXCEPTION_STRIP 
        || Underscore || SegmentDot || SymbolList.GetNumEntries()) {
            printf ("\n");
    }
    if (DebugInfo == CMDL_DEBUG_STRIP) {
        printf ("\n%3i Debug sections removed", CountDebugSectionsRemoved);
    }
    if (ExeptionInfo == CMDL_EXCEPTION_STRIP) {
        printf ("\n%3i Exception sections removed", CountExceptionSectionsRemoved);
    }
    if ((DebugInfo == CMDL_DEBUG_STRIP || ExeptionInfo == CMDL_EXCEPTION_STRIP) 
        && CountUnusedSymbolsHidden) {
            printf ("\n%3i Unused external symbol references hidden", CountUnusedSymbolsHidden);
    }

    if (Underscore || SegmentDot || SymbolList.GetNumEntries()) {
        if (CountUnderscoreConversions || Underscore) {
            printf ("\n%3i Changes in leading underscores on symbol names", CountUnderscoreConversions);
        }
        if (CountSectionDotConversions || SegmentDot) {
            printf ("\n%3i Changes in leading characters on section names", CountSectionDotConversions);
        }
        if (CountSymbolNameChanges) {
            printf ("\n%3i Symbol names changed", CountSymbolNameChanges);
        }
        if (CountSymbolNameAliases) {
            printf ("\n%3i Public symbol names aliased", CountSymbolNameAliases);
        }
        if (CountSymbolsWeakened) {
            printf ("\n%3i Public symbol names made weak", CountSymbolsWeakened);
        }
        if (CountSymbolsMadeLocal) {
            printf ("\n%3i Public or external symbol names made local", CountSymbolsMadeLocal);
        }
        if (SymbolChangeEntries && !CountSymbolNameChanges && !CountSymbolNameAliases && !CountSymbolsWeakened && !CountSymbolsMadeLocal) {
            printf ("\n    No symbols to change were found");
        }
    }
}


void CCommandLineInterpreter::Help() {
    // Print help message
    printf("\nObject file converter version %.2f for x86 and x86-64 platforms.", OBJCONV_VERSION);
    printf("\nCopyright (c) 2018 by Agner Fog. Gnu General Public License.");
    printf("\n\nUsage: objconv options inputfile [outputfile]");
    printf("\n\nOptions:");
    printf("\n-fXXX[SS]  Output file format XXX, word size SS. Supported formats:");
    printf("\n           PE, COFF, ELF, OMF, MACHO\n");
    printf("\n-fasm      Disassemble file (-fmasm, -fnasm, -fyasm, -fgasm)\n");
    printf("\n-dXXX      Dump file contents to console.");
    printf("\n           Values of XXX (can be combined):");
    printf("\n           f: File header, h: section Headers, s: Symbol table,");
    printf("\n           r: Relocation table, n: string table.\n");

    printf("\n-nu        change symbol Name Underscores to the default for the target format.");
    printf("\n-nu-       remove Underscores from symbol Names.");
    printf("\n-nu+       add Underscores to symbol Names.");
    printf("\n-nd        replace Dot/underscore in section names.");
    printf("\n-nr:N1:N2  Replace symbol Name N1 with N2.");
    printf("\n-np:N1:N2  Replace symbol Prefix N1 with N2.");
    printf("\n-ns:N1:N2  Replace symbol Suffix N1 with N2.");
    printf("\n-ar:N1:N2  make Alias N2 for existing public name N1.");
    printf("\n-ap:N1:N2  Replace symbol Prefix and keep old name as alias.");
    printf("\n-as:N1:N2  Replace symbol Suffix and keep old name as alias.");
    printf("\n-nw:N1     make public symbol Name N1 Weak (ELF and MAC64 only).");
    printf("\n-nl:N1     make public symbol Name N1 Local (invisible).\n");
    //printf("\n-ds        Strip Debug info.");    // default if input and output are different formats
    //printf("\n-dp        Preserve Debug info, even if it is incompatible.");
    printf("\n-xs        Strip exception handling info and other incompatible info.");  // default if input and output are different formats. Hides unused symbols
    printf("\n-xp        Preserve exception handling info and other incompatible info.\n");

    printf("\n-lx        eXtract all members from Library.");
    printf("\n-lx:N1:N2  eXtract member N1 from Library to file N2.");
    printf("\n-ld:N1     Delete member N1 from Library.");
    printf("\n-la:N1:N2  Add object file N1 to Library as member N2.");
    printf("\n           Alternative: -lib LIBRARYNAME OBJECTFILENAMES.\n");

    printf("\n-vN        Verbose options. Values of N:");
    printf("\n           0: Silent, 1: Print file names and types, 2: Tell about conversions.");

    printf("\n-wdNNN     Disable Warning NNN.");
    printf("\n-weNNN     treat Warning NNN as Error. -wex: treat all warnings as errors.");
    printf("\n-edNNN     Disable Error number NNN.");
    printf("\n-ewNNN     treat Error number NNN as Warning.\n");

    printf("\n-h         Print this help screen.\n");

    printf("\n@RFILE     Read additional options from response file RFILE.\n");
    printf("\n\nExample:");
    printf("\nobjconv -felf32 -nu filename.obj filename.o\n\n");
}
