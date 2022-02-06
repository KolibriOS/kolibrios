/****************************  cmdline.h   ***********************************
* Author:        Agner Fog
* Date created:  2006-07-25
* Last modified: 2006-07-25
* Project:       objconv
* Module:        cmdline.h
* Description:
* Header file for command line interpreter cmdline.cpp
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#ifndef CMDLINE_H
#define CMDLINE_H

/**************************  Define constants  ******************************/
// Max number of response files on command line
#define MAX_COMMAND_FILES  10

// Constants for output file type
#define CMDL_OUTPUT_DUMP          0x80       // No output file, just dump contents
#define CMDL_OUTPUT_ELF    FILETYPE_ELF      // ELF file
#define CMDL_OUTPUT_PE     FILETYPE_COFF     // MS-COFF/PE file
#define CMDL_OUTPUT_OMF    FILETYPE_OMF      // OMF file
#define CMDL_OUTPUT_MACHO  FILETYPE_MACHO_LE // Mach-O file, little endian
#define CMDL_OUTPUT_MASM   FILETYPE_ASM      // Disassembly

// Constants for subtypes
#define SUBTYPE_MASM                 0       // Disassembly MASM/TASM
#define SUBTYPE_YASM                 1       // Disassembly NASM/YASM
#define SUBTYPE_GASM                 2       // Disassembly GAS(Intel)

// Constants for verbose or silent console output
#define CMDL_VERBOSE_NO              0     // Silent. No console output if no errors or warnings
#define CMDL_VERBOSE_YES             1     // Output messages about file names and types
#define CMDL_VERBOSE_DIAGNOSTICS     2     // Output more messages

// Constants for dump options
#define DUMP_NONE               0x0000     // Dump nothing
#define DUMP_FILEHDR            0x0001     // Dump file header
#define DUMP_SECTHDR            0x0002     // Dump section headers
#define DUMP_SYMTAB             0x0010     // Dump symbol table
#define DUMP_RELTAB             0x0020     // Dump relocation table
#define DUMP_STRINGTB           0x0040     // Dump string table
#define DUMP_COMMENT            0x0080     // Dump comment records

// Constants for stripping or converting debug information from file
#define CMDL_DEBUG_DEFAULT           0     // Remove if output is different format
#define CMDL_DEBUG_STRIP             1     // Remove debugging information from file
#define CMDL_DEBUG_PRESERVE          2     // Leave debugging information unchanged
#define CMDL_DEBUG_LINNUM            4     // Convert line number information (not supported)
#define CMDL_DEBUG_SYMBOLS           8     // Convert symbol information (not supported)

// Constants for stripping exception handler information from file
#define CMDL_EXCEPTION_DEFAULT       0     // Remove if output is different format
#define CMDL_EXCEPTION_STRIP         1     // Remove exception handler information from file
#define CMDL_EXCEPTION_PRESERVE      2     // Leave exception handler information unchanged

// Constants for adding/removing leading underscores from symbol names
#define CMDL_UNDERSCORE_NOCHANGE     0     // Don't add or remove underscores
#define CMDL_UNDERSCORE_CHANGE       1     // Change underscores to default for target
#define CMDL_UNDERSCORE_REMOVE       2     // Remove underscores from symbol names
#define CMDL_UNDERSCORE_ADD          3     // Add underscores to symbol names
#define CMDL_KEEP_ALIAS          0x100     // Keep old name as alias

// Constants for replacing leading dot with underscore or vice versa in section names
#define CMDL_SECTIONDOT_NOCHANGE     0     // Don't change section names
#define CMDL_SECTIONDOT_CHANGE       1     // Change leading character in section names to default for target
#define CMDL_SECTIONDOT_U2DOT        2     // Change underscore to dot in section names
#define CMDL_SECTIONDOT_DOT2U        3     // Change dot to underscore in unknown section names

// Constants for library options
#define CMDL_LIBRARY_DEFAULT         0     // No option specified
#define CMDL_LIBRARY_CONVERT         1     // Convert or modify library
#define CMDL_LIBRARY_ADDMEMBER       2     // Add object file to library
#define CMDL_LIBRARY_EXTRACTMEM  0x100     // Extract specified object file(s) from library
#define CMDL_LIBRARY_EXTRACTALL  0x110     // Extract all object files from library

// Constants for file input/output options
#define CMDL_FILE_INPUT              1     // Input file required
#define CMDL_FILE_IN_IF_EXISTS       2     // Read input file if it exists
#define CMDL_FILE_OUTPUT          0x10     // Write output file required
#define CMDL_FILE_IN_OUT_SAME     0x20     // Input and output files may have the same name

#define MAXSYMBOLLENGTH           1024     // Maximum length of symbols for changing underscore or dot

// Constants for symbol type as input to CCommandLineInterpreter::SymbolChange()
#define SYMT_OTHER                   0     // File name or unknown symbol type
#define SYMT_SECTION                 1     // Segment or section name
#define SYMT_LOCAL                   2     // Local symbol (not imported or exported)
#define SYMT_PUBLIC                  3     // Public or weak symbol (exported)
#define SYMT_EXTERNAL                4     // External symbol (imported)
#define SYMT_LIBRARYMEMBER      0x1000     // Name of library member

// Constants for symbol change action as defined in SSymbolChange::Action 
// and output from CCommandLineInterpreter::SymbolChange()
#define SYMA_NOCHANGE                0     // Do nothing
#define SYMA_MAKE_WEAK               1     // Make symbol weak
#define SYMA_MAKE_LOCAL              2     // Make symbol local
#define SYMA_CHANGE_NAME          0x10     // Change name of symbol
#define SYMA_CHANGE_PREFIX        0x11     // Change beginning of symbol name
#define SYMA_CHANGE_SUFFIX        0x12     // Change end of symbol name
#define SYMA_ALIAS               0x100     // Make alias of public symbol and keep old name, must be combined 
                                           // with SYMA_CHANGE_NAME, SYMA_CHANGE_PREFIX or SYMA_CHANGE_SUFFIX
#define SYMA_ADD_MEMBER         0x1001     // Add member to library
#define SYMA_DELETE_MEMBER      0x1002     // Remove member from library
#define SYMA_EXTRACT_MEMBER     0x1004     // Extract member from library

// Structure for specifying desired change of a specific symbol
struct SSymbolChange {
   char * Name1;                           // Symbol name to look for
   char * Name2;                           // Replace with this name
   int    Action;                          // Action to take on symbol
   int    Done;                            // Count how many times this has been done
};

// Class for interpreting command line
class CCommandLineInterpreter {
public:
   CCommandLineInterpreter();                // Default constructor
   ~CCommandLineInterpreter();               // Destructor
   void ReadCommandLine(int argc, char * argv[]);     // Read and interpret command line
   int  SymbolChange(char const * oldname, char const ** newname, int symtype); // Check if symbol has to be changed
   int  SymbolIsInList(char const * name);   // Check if symbol is in SymbolList
   int  SymbolBinSearch(char const * name, int nsym, int * found);
   int  SymbolChangesRequested();            // Any kind of symbol change requested on command line
   void ReportStatistics();                  // Report statistics about name changes etc.
   void CountDebugRemoved();                 // Increment CountDebugSectionsRemoved
   void CountExceptionRemoved();             // Increment CountExceptionSectionsRemoved
   void CountSymbolsHidden();                // Increment CountUnusedSymbolsHidden
   SSymbolChange const * GetMemberToAdd();   // Get names of object files to add to library
   void CheckExtractSuccess();               // Check if library members to extract were found
   void CheckSymbolModifySuccess();          // Check if symbols to modify were found
   char * InputFile;                         // Input file name
   char * OutputFile;                        // Output file name
   int    InputType;                         // Input file type (detected from file)
   int    OutputType;                        // Output type (file type or dump)
   int    SubType;                           // Subtype of output type. Assembly language dialect or library type
   int    MemberType;                        // File type of library members
   int    DesiredWordSize;                   // Desired word size for output file
   uint32 Verbose;                           // How much diagnostics to print on screen
   uint32 DumpOptions;                       // Options for dumping file
   uint32 DebugInfo;                         // Strip or convert debug info
   uint32 ExeptionInfo;                      // Strip or preserve exception handler info and other incompatible info
   uint32 Underscore;                        // Add/remove underscores in symbol names
   uint32 SegmentDot;                        // Change underscore/dot in beginning of segment names
   uint32 LibraryOptions;                    // Options for manipulating library
   uint32 LibrarySubtype;                    // Options for manipulating library
   uint32 FileOptions;                       // Options for input and output files
   uint32 ImageBase;                         // Specified image base
   int    ShowHelp;                          // Help screen printed
protected:
   int  libmode;                             // -lib option has been encountered
   void ReadCommandItem(char *);             // Read one option from command line
   void ReadCommandFile(char *);             // Read commands from file
   void InterpretFileName(char *);           // Interpret input or output filename from command line
   void InterpretCommandOption(char *);      // Interpret one option from command line
   void InterpretOutputTypeOption(char *);   // Interpret output type option from command line
   void InterpretVerboseOption(char *);      // Interpret silent/verbose option from command line
   void InterpretDumpOption(char *);         // Interpret dump option from command line
   void InterpretDebugInfoOption(char *);    // Interpret debug info option from command line
   void InterpretExceptionInfoOption(char*); // Interpret exception handler info option from command line
   void InterpretErrorOption(char *);        // Interpret error option from command line
   void InterpretSymbolNameChangeOption(char *);  // Interpret various options for changing symbol names
   void InterpretLibraryOption(char *);      // Interpret options for manipulating library/archive files
   void InterpretImagebaseOption(char *);    // Interpret image base option
   void AddObjectToLibrary(char * filename, char * membername); // Add object file to library
   void Help();                              // Print help message
   CArrayBuf<CFileBuffer> ResponseFiles;     // Array of up to 10 response file buffers
   int NumBuffers;                           // Number of response file buffers
   int SymbolChangeEntries;                  // Number of entries in SymbolList, except library entries
   CMemoryBuffer SymbolList;                 // List of symbol names to change. Contains entries of type SSymbolChange. Kept in sorted order
   CMemoryBuffer PrefixSuffixList;           // List of prefix/suffix to change. Contains entries of type SSymbolChange
   CMemoryBuffer MemberNames;                // Buffer containing truncated member names
   uint32 MemberNamesAllocated;              // Size of buffer in MemberNames
   uint32 CurrentSymbol;                     // Pointer into SymbolList
   // Statistics counters
   int CountUnderscoreConversions;           // Count number of times symbol leading underscores are changed
   int CountSectionDotConversions;           // Count number of times leading character is changed on section names
   int CountSymbolNameChanges;               // Count number of times symbol names are changed at specific command
   int CountSymbolNameAliases;               // Count number of times symbol names are aliased at specific command or underscore command
   int CountSymbolsWeakened;                 // Count number of times symbol names are made weak at specific command
   int CountSymbolsMadeLocal;                // Count number of times symbol names are made local at specific command
   int CountUnusedSymbolsHidden;             // Count number of times unused symbols are hidden
   int CountDebugSectionsRemoved;            // Count number of debug sections removed
   int CountExceptionSectionsRemoved;        // Count number of exception handler sections removed
};

extern CCommandLineInterpreter cmd;          // Command line interpreter

#endif // #ifndef CMDLINE_H
