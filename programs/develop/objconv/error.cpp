/****************************   error.cpp   **********************************
* Author:        Agner Fog
* Date created:  2006-07-15
* Last modified: 2017-10-18
* Project:       objconv
* Module:        error.cpp
* Description:
* Standard procedure for error reporting to stderr
*
* Copyright 2006-2017 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

// Define this if you get problems:
// #define OBJCONV_ERROR_CPP 1


#include "stdafx.h"

#define MAX_ERROR_TEXT_LENGTH 1024 // Maximum length of error text including extra info


// Make and initialize error reporter object
CErrorReporter err;

SErrorText ErrorTexts[] = {
   // Unknown error
   {0,    2, "Unknown error number!"},

   // Warning messages
   {1001, 1, "Empty command line option"},
   {1002, 1, "Unknown command line option: %s"},
   {1003, 1, "Unknown warning/error number: %i"},
   {1006, 1, "Nothing do do. Copying file unchanged"},
   {1008, 1, "Converting COFF file to ELF and back again."},
   {1009, 1, "Converting OMF file to COFF and back again."},
   {1010, 1, "Section index and section-relative fixup not supported in ELF file. Probably a debug record"},
   {1011, 1, "Converting Mach-O file to ELF and back again."},
   {1020, 1, "Non-public symbol %s cannot be made weak"},
   {1021, 1, "Non-public symbol %s cannot be made local"},
   {1022, 1, "Non-public symbol %s cannot get an alias"},
   {1023, 1, "External symbol %s made local. Access to this symbol will cause error"},
   {1024, 1, "Cannot change prefix on name %s, not a symbol"},
   {1029, 1, "Debug information may be incompatible"},
   {1030, 1, "Exception information may be incompatible"},
   {1031, 1, "Windows resource information not translated"},   
   {1032, 1, "More than one symbol table found in ELF file"},
   {1033, 1, "Sorry, cannot currently make alias in dynamic symbol table. Symbol = %s"},
   {1040, 1, "Name of section %s too long. Truncating to 16 characters"},
   {1050, 0, "Position dependent references will not work in .so file. (First occurrence is symbol %s. This message can be turned off with -wd1050)"},
   {1051, 1, "Weak public not supported in target file type, symbol %s"},
   {1052, 1, "Indirect symbol index out of range"},
   {1053, 1, "Common constant converted to public: %s"},
   {1054, 1, "Cannot find import table"},
   {1055, 1, "Communal section currently not supported by objconv. Section dropped"},
   {1060, 1, "Different alignments specified for same segment, %s. Using highest alignment"},
   {1061, 1, "Symbol %s has lazy binding"},
   {1062, 1, "Symbol %s has unknown type"},
   {1063, 1, "Gnu indirect function (CPU dispatcher) cannot be converted"},
   {1101, 1, "Output file name should have extension .lib or .a"},
   {1102, 1, "Library members have different type"},
   {1103, 1, "Output file name ignored"},
   {1104, 1, "Library member %s not found. Extraction failed"},
   {1105, 1, "Library member %s not found. Deletion failed"},
   {1106, 1, "Symbol %s not found. Modification of this symbol failed"},
   {1107, 1, "Name of library member %s should have extension .o or .obj"},
   {1108, 1, "Name of library member %s too long. Truncating to 15 characters"},
   {1109, 1, "Library member %s has unknown type. Possibly alias record without code"},
   {1150, 1, "Universal binary contains more than one component that can be converted. Specify desired word size or use lipo to extract desired component"},
   {1151, 1, "Skipping component with wordsize %i"},

   {1202, 1, "OMF Record checksum error"},
   {1203, 1, "Unrecognized data in OMF subrecord"},
   {1204, 1, "String too long building OMF file. Truncating to 255 characters: %s"},
   {1205, 1, "Alignment by %i possibly not supported for OMF file. Using page alignment (256 or 4096 depending on system)"},
   {1206, 1, "Stack segment ignored"},
   {1207, 1, "Overlapping data"},
   {1208, 1, "Back-patched code (possibly OK)"},
   {1211, 1, "%i comment records ignored"},
   {1212, 1, "Record type (%s) not supported"},
   {1213, 1, "Hash table has %i occurrences of name %s"},
   {1214, 1, "Symbol %s defined in both modules %s"},
   {1215, 1, "More than 251 blocks required in symbol hash table. May fail with some linkers"},
   {1300, 1, "File contains 32-bit absolute address. Must link with option -image_base %s -pagezero_size 1000"},
   {1301, 1, "Image-relative address converted to absolute. Assumes image base = "},
   {1302, 1, "64-bit relocation with arbitrary reference point converted to 32-bit self-relative. Will fail if offset is negative"},
   {1303, 1, "Cannot find imported symbol"},
   {1304, 1, "Unknown relocation address"},

   // Error messages
   {2001, 2, "No more than one input file and one output file can be specified"},
   {2002, 2, "Word size (%i) not supported for output file"},
   {2003, 2, "Only one output format option can be specified. Command line error at %s"},
   {2004, 2, "Unknown command line option: %s"},
   {2005, 2, "Input file and output file cannot have same name: %s"},
   {2006, 2, "Unsupported file type for file %s: %s"},
   {2007, 2, "Cannot dump and convert file in the same command"},
   {2008, 2, "This option must have two symbol names: %s"},
   {2009, 2, "This option must have one symbol name: %s"},
   {2010, 2, "Sorry. Dump of file type %s is not supported"},
   {2011, 2, "Sorry. Conversion of file type %s is not supported"},
   {2012, 2, "Cannot convert from word size %i to word size %i"},
   {2013, 2, "Sorry. Conversion of file type %s to %s is not supported"},
   {2014, 2, "File contains information for .NET common language runtime. Cannot convert"},
   {2015, 2, "More than one option specified for symbol %s"},
   {2016, 2, "Index out of range"},
   {2017, 2, "File name %s specified more than once"},
   {2018, 2, "Unknown type 0x%X for file: %s"},
   {2020, 2, "Overflow when converting value of symbol %s to 32 bits"},
   {2021, 2, "File contains information for objective-C runtime code. Cannot convert"},
   {2022, 2, "Cannot convert executable file"},

   {2030, 2, "Unsupported relocation type (%i)"},
   {2031, 2, "Relocated symbol not found"},
   {2032, 2, "Relocation points outside segment"},
   {2033, 2, "Error in ELF file. Record size not specified"},
   {2034, 2, "Symbol table not found in ELF file"},
   {2035, 2, "Pointer out of range in object file"},
   {2036, 2, "Unknown section index in ELF file: %i"},
   {2037, 2, "Symbol storage/binding type %i not supported"},
   {2038, 2, "Symbol type %i not supported"},
   {2040, 2, "Symbol table corrupt in object file"},
   {2041, 2, "File has relocation of uninitialized data"},
   {2042, 2, "Relocation to global offset table found. Cannot convert position-independent code"},
   {2043, 2, "Relocation to procedure linkage table found. Cannot convert"},
   {2044, 2, "Relocation relative to arbitrary reference point that cannot be converted"},
   {2045, 2, "Unknown import table type"},
   {2050, 2, "Inconsistent relocation record pair"},
   {2051, 2, "Too many symbols for Mach-O file. Maximum = 16M"},
   {2052, 2, "Unexpected data between symbol table and string table"},

   {2103, 2, "Cannot read input file %s"},
   {2104, 2, "Cannot write output file %s"},
   {2105, 2, "Wrong size of file %s"},
   {2107, 2, "Too many response files"},
   {2110, 2, "COFF file section table corrupt"},
   {2112, 2, "String table corrupt"},
   {2114, 2, "This is an intermediate file for whole-program-optimization in Intel compiler"},
   {2200, 2, "Weak public symbols not supported in this file format"},
   {2202, 2, "Symbol name %s too long. Cannot change prefix"},
   {2203, 2, "File name %s too long"},
   {2210, 2, "File contains overlapping relocation sources"},
   {2301, 2, "OMF Record extends beyond end of file"},
   {2302, 2, "Fixup source extends beyond end of section"},
   {2303, 2, "Too many symbols for OMF file. Index exceeds 32767"},
   {2304, 2, "Word-size index exceeds 65535"},
   {2305, 2, "%i Communal sections found. Currently not supported by Objconv"},
   {2306, 2, "Segment size is 4 Gbytes"},
   {2307, 2, "Segment address is absolute"},
   {2308, 2, "Unknown alignment %i"},
   {2309, 2, "Data outside bounds of segment %s"},
   {2310, 2, "Iterated data outside bounds of segment"},
   {2311, 2, "Relocation of iterated data not supported by objconv"},
   {2312, 2, "FIXUPP record does not refer to data record"},
   {2313, 2, "OMF file has compression of repeated relocation target (thread). This is not supported in objconv"},
   {2314, 2, "Unknown relocation method T%i"},
   {2315, 2, "Group-relative relocation to %s not supported"},
   {2316, 2, "Incompatible relocation method: %s"},
   {2317, 2, "Incompatible word size: %i"},
   {2318, 2, "OMF file has compression of repeated communal data. This is not supported in objconv"},
   {2320, 2, "Mixed 32-bit and 64-bit segments"},
   {2321, 2, "Wrong record size found"},
   {2330, 2, "Imagebase specified more than once"},
   {2331, 2, "Imagebase must be divisible by page size 1000 (hexadecimal)"},
   {2332, 2, "Imagebase must > 0 and < 80000000 (hexadecimal)"},

   {2500, 2, "Library/archive file is corrupt"},
   {2501, 2, "Cannot store file of type %s in library"},
   {2502, 2, "Too many members in library"},
   {2503, 2, "Output file name must be specified"},
   {2504, 2, "Object file type (%s) does not match library"},
   {2505, 2, "Object file word size (%i) does not match library"},
   {2506, 2, "Overflow of buffer for library member names"},
   {2507, 2, "%s is an import library. Cannot convert to static library"},
   {2600, 2, "Library has more than one header"},
   {2601, 2, "Library page size (%i) is not a power of 2"},
   {2602, 2, "Library end record does not match dictionary offset in OMF library"},
   {2603, 2, "Public name %s not found in hash table"},
   {2605, 2, "Symbol hash table too big. Creation of library failed"},
   {2606, 2, "Too many library members. Creation of library failed"},
   {2610, 2, "Library end record not found"},
   {2620, 2, "You need to extract library members before disassembling"},
   {2621, 2, "Wrong output file type"},

   {2701, 2, "Wrong number of members in universal binary (%i)"},

   {3000, 1, "Internal error in opcode table"},
   {3001, 1, "Internal error: Unknown register type 0x%X"},

   // Fatal errors makes the program stop immediately:
   {9000, 9, "Objconv program internal inconsistency"},
   {9001, 9, "Objconv program has been compiled with wrong integer sizes"},
   {9002, 9, "Objconv cannot run on machine with big-endian memory organization"},
   {9003, 9, "Array index out of range"},
   {9004, 9, "Cannot resize array of type CArrayBuf"},
   {9005, 9, "Exceeding 1kb size limit while building OMF record"},
   {9006, 9, "Memory allocation failed"},
   {9007, 9, "Objcopy internal error in opcode map 0x%X"},

   // Mark end of list
   {9999, 9999, "End of error text list"}
};


// Constructor for CErrorReporter
CErrorReporter::CErrorReporter() {
   NumErrors = NumWarnings = WorstError = 0;
   MaxWarnings = 50;      // Max number of warning messages to pring
   MaxErrors   = 50;      // Max number of error messages to print
}

SErrorText * CErrorReporter::FindError(int ErrorNumber) {
   // Search for error in ErrorTexts
   int e;
   const int ErrorTextsLength = sizeof(ErrorTexts) / sizeof(ErrorTexts[0]);
   for (e = 0; e < ErrorTextsLength; e++) {
      if (ErrorTexts[e].ErrorNumber == ErrorNumber) return ErrorTexts + e;
   }
   // Error number not found
   static SErrorText UnknownErr = ErrorTexts[0];
   UnknownErr.ErrorNumber = ErrorNumber;
   UnknownErr.Status      = 0x102;  // Unknown error
   return &UnknownErr;
}


void CErrorReporter::submit(int ErrorNumber) {
   // Print error message with no extra info
   SErrorText * err = FindError(ErrorNumber);
   HandleError(err, err->Text);
}

void CErrorReporter::submit(int ErrorNumber, int extra) { 
   // Print error message with extra numeric info
   // ErrorTexts[ErrorNumber] must contain %i where extra is to be inserted
   char text[MAX_ERROR_TEXT_LENGTH];
   SErrorText * err = FindError(ErrorNumber);
   snprintf(text, MAX_ERROR_TEXT_LENGTH, err->Text, extra);
   HandleError(err, text);
}

void CErrorReporter::submit(int ErrorNumber, int extra1, int extra2) { 
   // Print error message with 2 extra numeric values inserted
   // ErrorTexts[ErrorNumber] must contain two %i fields where extra numbers are to be inserted
   char text[MAX_ERROR_TEXT_LENGTH];
   SErrorText * err = FindError(ErrorNumber);
   snprintf(text, MAX_ERROR_TEXT_LENGTH, err->Text, extra1, extra2);
   HandleError(err, text);
}

void CErrorReporter::submit(int ErrorNumber, char const * extra) {
   // Print error message with extra text info
   // ErrorTexts[ErrorNumber] must contain %s where extra is to be inserted
   char text[MAX_ERROR_TEXT_LENGTH];
   SErrorText * err = FindError(ErrorNumber);
   snprintf(text, MAX_ERROR_TEXT_LENGTH, err->Text, extra);
   HandleError(err, text);
}

void CErrorReporter::submit(int ErrorNumber, char const * extra1, char const * extra2) {
   // Print error message with two extra text info fields
   // ErrorTexts[ErrorNumber] must contain %s where extra texts are to be inserted
   char text[MAX_ERROR_TEXT_LENGTH];
   if (extra1 == 0) extra1 = "???"; if (extra2 == 0) extra2 = "???";
   SErrorText * err = FindError(ErrorNumber);
   snprintf(text, MAX_ERROR_TEXT_LENGTH, err->Text, extra1, extra2);
   HandleError(err, text);
}

void CErrorReporter::submit(int ErrorNumber, int extra1, char const * extra2) {
   // Print error message with two extra text fields inserted
   // ErrorTexts[ErrorNumber] must contain %i and %s where extra texts are to be inserted
   char text[MAX_ERROR_TEXT_LENGTH];
   if (extra2 == 0) extra2 = "???";
   SErrorText * err = FindError(ErrorNumber);
   snprintf(text, MAX_ERROR_TEXT_LENGTH, err->Text, extra1, extra2);
   HandleError(err, text);
}

void CErrorReporter::HandleError(SErrorText * err, char const * text) {
   // HandleError is used by submit functions
   // check severity
   int severity = err->Status & 0x0F;
   if (severity == 0) {
      return;  // Ignore message
   }
   if (severity > 1 && err->ErrorNumber > WorstError) {
      // Store highest error number
      WorstError = err->ErrorNumber;
   }
   if (severity == 1) {
      // Treat message as warning
      if (++NumWarnings > MaxWarnings) return; // Maximum number of warnings has been printed
      // Treat message as warning
      fprintf(stderr, "\nWarning %i: %s", err->ErrorNumber, text);
      if (NumWarnings == MaxWarnings) {
         // Maximum number reached
         fprintf(stderr, "\nSupressing further warning messages");
      }
   }
   else {
      // Treat message as error
      if (++NumErrors > MaxErrors) return; // Maximum number of warnings has been printed
      fprintf(stderr, "\nError %i: %s", err->ErrorNumber, text);
      if (NumErrors == MaxErrors) {
         // Maximum number reached
         fprintf(stderr, "\nSupressing further warning messages");
      }
   }
   if (severity == 9) {
      // Abortion required
      fprintf(stderr, "\nAborting\n");
      exit(err->ErrorNumber);
   }
}

int CErrorReporter::Number() {
   // Get number of fatal errors
   return NumErrors;
}

int CErrorReporter::GetWorstError() {
   // Get highest warning or error number encountered
   return WorstError;
}

void CErrorReporter::ClearError(int ErrorNumber) {
   // Ignore further occurrences of this error
   int e;
   const int ErrorTextsLength = sizeof(ErrorTexts) / sizeof(ErrorTexts[0]);
   for (e = 0; e < ErrorTextsLength; e++) {
      if (ErrorTexts[e].ErrorNumber == ErrorNumber) break;
   }
   if (e < ErrorTextsLength) {
      ErrorTexts[e].Status = 0;
   }
}
