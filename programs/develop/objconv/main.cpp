/****************************   main.cpp   **********************************
* Author:        Agner Fog
* Date created:  2006-07-26
* Last modified: 2011-10-28
* Project:       objconv
* Module:        main.cpp
* Description:
* Objconv is a portable C++ program for converting object file formats.
* Compile for console mode on any platform.
*
* Module main contains the program entry
*
* Copyright 2006-2011 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/

#include "stdafx.h"

// Texts of option feedback. Adding or removing leading underscores on symbols
SIntTxt UnderscoreOptionNames[] = {
   {CMDL_UNDERSCORE_NOCHANGE, "Not adding or removing underscores for this filetype"},
   {CMDL_UNDERSCORE_REMOVE,   "Removing leading underscores from symbol names"},
   {CMDL_UNDERSCORE_ADD,      "Adding leading underscores to symbol names"},
   {CMDL_UNDERSCORE_REMOVE|CMDL_KEEP_ALIAS, "Removing leading underscores from symbol names. Keeping old name as alias"},
   {CMDL_UNDERSCORE_ADD|CMDL_KEEP_ALIAS,    "Adding leading underscores to symbol names. Keeping old name as alias"}
};

// Texts of option feedback. Changing leading dot or underscore on section names
SIntTxt SectionDotOptionNames[] = {
   {CMDL_SECTIONDOT_NOCHANGE, "Not changing leading character on section names for this filetype"},
   {CMDL_SECTIONDOT_U2DOT, "Changing leading underscores on section names to dot"},
   {CMDL_SECTIONDOT_DOT2U, "Changing leading dot on nonstandard section names to underscore"}
};

// Check that integer type definitions are correct.
// Will generate an error message if the compiler makes the integer types 
// with wrong size.
static void CheckIntegerTypes() {
   if (
      sizeof(uint8) != 1 ||
      sizeof(int16) != 2 ||
      sizeof(int32) != 4 ||
      sizeof(int64) != 8) {
      err.submit(9001);                // Make error message if type definitions are wrong
   }
}

// Check that we are running on a machine with little-endian memory organization
static void CheckEndianness() {
   static uint8 bytes[4] = {1, 2, 3, 4};
   if (*(uint32*)bytes != 0x04030201) {
      // Big endian
      err.submit(9002);
   }
}

// Function to convert powers of 2 to index
int FloorLog2(uint32 x) {
   int i = -1;
   do {
      x >>= 1;
      i++;
   } while (x);
   return i;
}

const char * timestring(uint32 t) {
   // Convert 32 bit time stamp to string
   // Fix the problem that time_t may be 32 bit or 64 bit
   union {
      time_t t;
      uint32 t32;
   } utime;
   utime.t = 0;
   utime.t32 = t;
   const char * string = ctime(&utime.t);
   if (string == 0) string = "?";
   return string;
}

// Main. Program starts here
int main(int argc, char * argv[]) {
   CheckIntegerTypes();                // Check that compiler has the right integer sizes
   CheckEndianness();                  // Check that machine is little-endian

#ifdef  _DEBUG
   // For debugging only. Remove this
   if (argc == 1) {
      char * dummyarg[] = {"", "@resp.txt"}; // Read command line from file resp.txt
      argc = 2; argv = dummyarg;}
#endif

   cmd.ReadCommandLine(argc, argv);    // Read command line parameters   
   if (cmd.ShowHelp) return 0;         // Help screen has been printed. Do nothing else

   CMain maincvt;                      // This object takes care of all conversions etc.
   maincvt.Go();          
   // Do everything the command line says

   if (cmd.Verbose) printf("\n");      // End with newline
   return err.GetWorstError();         // Return with error code
}


// Class CMainConverter is used for control of the conversion process
CMain::CMain() : CFileBuffer() {
}

void CMain::Go() {
   // Do whatever the command line parameters say
   FileName = cmd.InputFile;           // Get input file name from command line
   // Ignore nonexisting filename when building library
   int IgnoreError = (cmd.FileOptions & CMDL_FILE_IN_IF_EXISTS) && !cmd.OutputFile;
   Read(IgnoreError);                  // Read input file
   GetFileType();                      // Determine file type
   cmd.InputType = FileType;           // Save input file type in cmd for access from other modules
   if (cmd.OutputType == 0) {
       // desired type not specified
       cmd.OutputType = FileType;
   }
   if (err.Number()) return;           // Return if error
   CheckOutputFileName();              // Construct output file name with default extension
   if (err.Number()) return;

   if ((FileType & (FILETYPE_LIBRARY | FILETYPE_OMFLIBRARY)) 
   || (cmd.LibraryOptions & CMDL_LIBRARY_ADDMEMBER)) {
      // Input file is a library or we are building a library
      CLibrary lib;                    // Library handler object
      *this >> lib;                    // Transfer my file buffer to lib
      lib.Go();                        // Do conversion or dump
      *this << lib;                    // Get file buffer back
   }
   else {
      // Input file is an object file
      CConverter conv;                 // Make converter object
      *this >> conv;                   // Transfer my file buffer to conv
      conv.Go();                       // Do conversion or dump
      *this << conv;                   // Get file buffer back
   }
   if ((cmd.FileOptions & CMDL_FILE_OUTPUT) && OutputFileName) {
      // There is an output file to write
      cmd.CheckSymbolModifySuccess();  // Check if symbols to modify were found
      if (err.Number()) return;        // Return if error
      FileName = OutputFileName;       // Output file name
      Write();                         // Write output file
      if (cmd.Verbose) cmd.ReportStatistics(); // Report statistics
   }
}

CConverter::CConverter() {
   // Constructor
}

void CConverter::DumpCOF() {
   // Dump COFF file
   CCOFF cof;                          // Make object for interpreting COFF file
   *this >> cof;                       // Give it my buffer
   cof.ParseFile();                    // Parse file buffer
   if (err.Number()) return;           // Return if error
   cof.Dump(cmd.DumpOptions);          // Dump file
   *this << cof;                       // Take back my buffer
}

void CConverter::DumpELF() {
   // Dump ELF file
   if (WordSize == 32) {
      // Make object for interpreting 32 bit ELF file
      CELF<ELF32STRUCTURES> elf;
      *this >> elf;                    // Give it my buffer
      elf.ParseFile();                 // Parse file buffer
      if (err.Number()) return;        // Return if error
      elf.Dump(cmd.DumpOptions);       // Dump file
      *this << elf;                    // Take back my buffer
   }
   else {
      // Make object for interpreting 32 bit ELF file
      CELF<ELF64STRUCTURES> elf;
      *this >> elf;                    // Give it my buffer
      elf.ParseFile();                 // Parse file buffer
      if (err.Number()) return;        // Return if error
      elf.Dump(cmd.DumpOptions);       // Dump file
      *this << elf;                    // Take back my buffer
   }
}

void CConverter::DumpMACHO() {
   // Dump Mach-O file
   if (WordSize == 32) {
      // Make object for interpreting 32 bit Mach-O file
      CMACHO<MAC32STRUCTURES> macho;
      *this >> macho;                     // Give it my buffer
      macho.ParseFile();                  // Parse file buffer
      if (err.Number()) return;           // Return if error
      macho.Dump(cmd.DumpOptions);        // Dump file
      *this << macho;                     // Take back my buffer
   }
   else {
      // Make object for interpreting 64 bit Mach-O file
      CMACHO<MAC64STRUCTURES> macho;
      *this >> macho;                     // Give it my buffer
      macho.ParseFile();                  // Parse file buffer
      if (err.Number()) return;           // Return if error
      macho.Dump(cmd.DumpOptions);        // Dump file
      *this << macho;                     // Take back my buffer
   }
}

void CConverter::ParseMACUnivBin() {
   // Dump Mac universal binary
   CMACUNIV macuniv;                   // Make object for interpreting Mac universal binary file
   *this >> macuniv;                   // Give it my buffer
   macuniv.Go(cmd.DumpOptions);        // Dump file components
   *this << macuniv;                   // Take back my buffer
}

void CConverter::DumpOMF() {
   // Dump OMF file
   COMF omf;                           // Make object for interpreting OMF file
   *this >> omf;                       // Give it my buffer
   omf.ParseFile();                    // Parse file buffer
   if (err.Number()) return;           // Return if error
   omf.Dump(cmd.DumpOptions);          // Dump file
   *this << omf;                       // Take back my buffer
}

void CConverter::COF2ELF() {
   // Convert COFF to ELF file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CCOF2ELF<ELF32STRUCTURES> conv;  // Make object for conversion 
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CCOF2ELF<ELF64STRUCTURES> conv;  // Make object for conversion 
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
}

void CConverter::COF2OMF() {
   // Convert COFF to OMF file
   CCOF2OMF conv;                      // Make object for conversion 
   *this >> conv;                      // Give it my buffer
   conv.ParseFile();                   // Parse file buffer
   if (err.Number()) return;           // Return if error
   conv.Convert();                     // Convert
   *this << conv;                      // Take back converted buffer
}

void CConverter::OMF2COF() {
   // Convert OMF to COFF file 
   COMF2COF conv;                      // Make object for conversion 
   *this >> conv;                      // Give it my buffer
   conv.ParseFile();                   // Parse file buffer
   if (err.Number()) return;           // Return if error
   conv.Convert();                     // Convert
   *this << conv;                      // Take back converted buffer
}

void CConverter::ELF2COF() {
   // Convert ELF to COFF file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CELF2COF<ELF32STRUCTURES> conv;
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CELF2COF<ELF64STRUCTURES> conv;
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
}

void CConverter::ELF2MAC() {
   // Convert ELF to Mach-O file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CELF2MAC<ELF32STRUCTURES,MAC32STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CELF2MAC<ELF64STRUCTURES,MAC64STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
}

void CConverter::MAC2ELF() {
   // Convert Mach-O file to ELF file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CMAC2ELF<MAC32STRUCTURES,ELF32STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CMAC2ELF<MAC64STRUCTURES,ELF64STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
}

void CConverter::COF2ASM() {
   // Disassemble COFF file
   CCOF2ASM conv;                      // Make object for conversion 
   *this >> conv;                      // Give it my buffer
   conv.ParseFile();                   // Parse file buffer
   if (err.Number()) return;           // Return if error
   conv.Convert();                     // Convert
   *this << conv;                      // Take back converted buffer
}

void CConverter::ELF2ASM() {
   // Disassemble ELF file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CELF2ASM<ELF32STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CELF2ASM<ELF64STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
}

void CConverter::MAC2ASM() {
   // Disassemble Mach-O file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CMAC2ASM<MAC32STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CMAC2ASM<MAC64STRUCTURES> conv;
      *this >> conv;                      // Give it my buffer
      conv.ParseFile();                   // Parse file buffer
      if (err.Number()) return;           // Return if error
      conv.Convert();                     // Convert
      *this << conv;                      // Take back converted buffer
   }
}

void CConverter::OMF2ASM() {
   // Disassemble OMF file
   COMF2ASM conv;                      // Make object for conversion 
   *this >> conv;                      // Give it my buffer
   conv.ParseFile();                   // Parse file buffer
   if (err.Number()) return;           // Return if error
   conv.Convert();                     // Convert
   *this << conv;                      // Take back converted buffer
}

void CConverter::COF2COF() {
   // Make changes in COFF file
   CCOF2COF conv;                      // Make instance of converter
   *this >> conv;                      // Give it my buffer
   conv.ParseFile();                   // Parse file buffer
   if (err.Number()) return;           // Return if error
   conv.Convert();                     // Convert
   *this << conv;                      // Take back converted buffer
}

void CConverter::ELF2ELF() {
   // Make changes in ELF file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CELF2ELF<ELF32STRUCTURES> conv;
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CELF2ELF<ELF64STRUCTURES> conv;
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
}

void CConverter::MAC2MAC() {
   // Make changes in Mach-O file
   if (WordSize == 32) {
      // Make instance of converter, 32 bit template
      CMAC2MAC<MAC32STRUCTURES> conv;
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
   else {
      // Make instance of converter, 64 bit template
      CMAC2MAC<MAC64STRUCTURES> conv;
      *this >> conv;                   // Give it my buffer
      conv.ParseFile();                // Parse file buffer
      if (err.Number()) return;        // Return if error
      conv.Convert();                  // Convert
      *this << conv;                   // Take back converted buffer
   }
}

void CConverter::Go() {
   // Convert or dump file, depending on command line parameters
   GetFileType();                      // Determine file type
   cmd.InputType = FileType;           // Save input file type in cmd for access from other modules
   if (err.Number()) return;           // Return if error

   if (cmd.OutputType == CMDL_OUTPUT_DUMP) {
      // File dump requested
      if (cmd.Verbose > 0) {
         // Tell what we are doing:
         printf("\nDump of file: %s, type: %s%i", FileName, GetFileFormatName(FileType), WordSize);
      }

      switch(FileType) {
      case FILETYPE_ELF:
         DumpELF();  break;

      case FILETYPE_COFF:
         DumpCOF();   break;

      case FILETYPE_MACHO_LE:
         DumpMACHO();   break;

      case FILETYPE_OMF:
         DumpOMF();   break;

      case FILETYPE_MAC_UNIVBIN:
         ParseMACUnivBin();   break;

      default:
         err.submit(2010, GetFileFormatName(FileType));  // Dump of this file type not supported
      }
      printf("\n");                              // New line
   }
   else {
      // File conversion requested
      if (cmd.DesiredWordSize == 0) cmd.DesiredWordSize = WordSize;
      if (WordSize && WordSize != cmd.DesiredWordSize) {
         err.submit(2012, WordSize, cmd.DesiredWordSize); // Cannot convert word size
         return;
      }
      if (Executable && cmd.OutputType != CMDL_OUTPUT_MASM) {
         // Attempt to convert executable file
         err.submit(2022);
      }
      if (err.Number()) return;        // Return if error

      if (cmd.Verbose > (uint32)(cmd.LibraryOptions != 0)) {
         // Tell what we are doing:
         printf("\nInput file: %s, output file: %s", FileName, OutputFileName);
         if (FileType != cmd.OutputType) {
            printf("\nConverting from %s%2i to %s%2i", 
               GetFileFormatName(FileType), WordSize, 
               GetFileFormatName(cmd.OutputType), WordSize);
         }
         else {
            printf("\nModifying %s%2i file", GetFileFormatName(FileType), WordSize);
         }
      }

      // Check underscore options
      if (cmd.Underscore && cmd.OutputType != 0) {
         if (cmd.Underscore == CMDL_UNDERSCORE_CHANGE) {
            // Find underscore option for desired conversion
            if (WordSize == 32) {
               // In 32-bit, all formats except ELF have underscores
               if (FileType == FILETYPE_ELF && cmd.OutputType != FILETYPE_ELF) {
                  // Converting from ELF32. Add underscores
                  cmd.Underscore = CMDL_UNDERSCORE_ADD;
               }
               else if (FileType != FILETYPE_ELF && cmd.OutputType == FILETYPE_ELF) {
                  // Converting to ELF32. Remove underscores
                  cmd.Underscore = CMDL_UNDERSCORE_REMOVE;
               }
               else {
                  // Anything else 32-bit. No change
                  cmd.Underscore = CMDL_UNDERSCORE_NOCHANGE;
               }
            }
            else { 
               // In 64-bit, only Mach-O has underscores
               if (FileType == FILETYPE_MACHO_LE && cmd.OutputType != FILETYPE_MACHO_LE) {
                  // Converting from MachO-64. Remove underscores
                  cmd.Underscore = CMDL_UNDERSCORE_REMOVE;
               }
               else if (FileType != FILETYPE_MACHO_LE && cmd.OutputType == FILETYPE_MACHO_LE) {
                  // Converting to MachO-64. Add underscores
                  cmd.Underscore = CMDL_UNDERSCORE_ADD;
               }
               else {
                  // Anything else 64-bit. No change
                  cmd.Underscore = CMDL_UNDERSCORE_NOCHANGE;
               }
            }
         }
         if (cmd.Verbose > (uint32)(cmd.LibraryOptions != 0)) { // Tell which option is chosen
            printf("\n%s", Lookup(UnderscoreOptionNames, cmd.Underscore));
         }
      }

      // Check sectionname options
      if (cmd.SegmentDot && cmd.OutputType != 0) {
         if (cmd.SegmentDot == CMDL_SECTIONDOT_CHANGE) {
            if (cmd.OutputType == FILETYPE_COFF || cmd.OutputType == FILETYPE_MACHO_LE || cmd.OutputType == FILETYPE_OMF) {
               // Change leading '.' to '_' in nonstandard section names
               cmd.SegmentDot = CMDL_SECTIONDOT_DOT2U;
            }
            else if (cmd.OutputType == FILETYPE_ELF) {
               // Change leading '_' to '.' in nonstandard section names
               cmd.SegmentDot = CMDL_SECTIONDOT_U2DOT;
            }
            else {
               cmd.SegmentDot = CMDL_SECTIONDOT_NOCHANGE;
            }
         }
         if (cmd.Verbose > (uint32)(cmd.LibraryOptions != 0)) { // Tell which option is chosen
            printf("\n%s", Lookup(SectionDotOptionNames, cmd.SegmentDot));
         }
      }

      // Check debug info options
      if (cmd.DebugInfo == CMDL_DEBUG_DEFAULT) {
         cmd.DebugInfo = (FileType != cmd.OutputType) ? CMDL_DEBUG_STRIP : CMDL_DEBUG_PRESERVE;
      }

      // Check exception handler info options
      if (cmd.ExeptionInfo == CMDL_EXCEPTION_DEFAULT) {
         cmd.ExeptionInfo = (FileType != cmd.OutputType) ? CMDL_EXCEPTION_STRIP : CMDL_EXCEPTION_PRESERVE;
      }

      // Choose conversion
      switch (FileType) {

      // Conversion from ELF
      case FILETYPE_ELF:
         switch (cmd.OutputType) {
         case FILETYPE_COFF:
            // Conversion from ELF to COFF
            ELF2ELF();                 // Make symbol changes in ELF file
            if (err.Number()) return;  // Return if error
            ELF2COF();                 // Convert to COFF
            break;

         case FILETYPE_MACHO_LE:
            // Conversion from ELF to Mach-O
            ELF2MAC();                 // Convert to Mach-O
            if (err.Number()) return;  // Return if error
            MAC2MAC();                 // Make symbol changes in Mach-O file, sort symbol tables alphabetically
            break;

         case FILETYPE_OMF:
            // Conversion from ELF to OMF
            ELF2ELF();                 // Make symbol changes in ELF file
            if (err.Number()) return;  // Return if error
            ELF2COF();                 // Convert to COFF first
            if (err.Number()) return;  // Return if error
            COF2OMF();                 // Then convert to OMF
            break;

         case FILETYPE_ELF:
            // Make changes in ELF file
            if (cmd.SymbolChangesRequested()) {
               ELF2ELF();              // Make symbol changes in ELF file
            }
            else if (!cmd.LibraryOptions) {
               err.submit(1006);       // Warning: nothing to do
            }
            break;

         case CMDL_OUTPUT_MASM:
            // Disassemble ELF file
            ELF2ASM();                 // Disassemble
            break;
            
         default:
            // Conversion not supported
            err.submit(2013, GetFileFormatName(FileType), GetFileFormatName(cmd.OutputType));
         }
         break;


      // Conversion from COFF
      case FILETYPE_COFF:
         switch (cmd.OutputType) {
         case FILETYPE_COFF:
            // No conversion. Modify file
            if (cmd.DebugInfo == CMDL_DEBUG_STRIP || cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
               COF2ELF();              // Convert to ELF and back again to strip debug and exception info
               if (err.Number()) return;  // Return if error
               ELF2COF();
               err.submit(1008);       // Warning: Converting to ELF and back again
            }
            if (cmd.SymbolChangesRequested()) {
               COF2COF();              // Make symbol name changes in COFF file
            }
            else if (cmd.DebugInfo != CMDL_DEBUG_STRIP && cmd.ExeptionInfo != CMDL_EXCEPTION_STRIP && !cmd.LibraryOptions) {
               err.submit(1006);       // Warning: nothing to do
            }
            break;

         case FILETYPE_ELF:
            COF2COF();                 // Make symbol changes in COFF file
            if (err.Number()) return;  // Return if error
            COF2ELF();                 // Convert to ELF
            break;
            
         case FILETYPE_OMF:
            COF2COF();                 // Make symbol changes in COFF file
            if (err.Number()) return;  // Return if error
            COF2OMF();                 // Convert to OMF
            break;
            
         case FILETYPE_MACHO_LE:
            COF2ELF();                 // Convert from COFF to ELF
            if (err.Number()) return;  // Return if error
            ELF2MAC();                 // Then convert from ELF to Mach-O
            if (err.Number()) return;  // Return if error
            MAC2MAC();                 // Make symbol changes in Mach-O file and sort symbol table
            break;

         case CMDL_OUTPUT_MASM:
            // Disassemble COFF file
            COF2ASM();                 // Disassemble
            break;
            
         default:
            // Conversion not supported
            err.submit(2013, GetFileFormatName(FileType), GetFileFormatName(cmd.OutputType));
         }
         break;


      // Conversion from OMF
      case FILETYPE_OMF:
         switch (cmd.OutputType) {
         case FILETYPE_OMF:
            // No conversion. Modify file
            if (cmd.SymbolChangesRequested() || cmd.DebugInfo == CMDL_DEBUG_STRIP || cmd.ExeptionInfo == CMDL_EXCEPTION_STRIP) {
               OMF2COF();              // Convert to COFF and back again to do requested changes
               if (err.Number()) return;  // Return if error
               COF2COF();              // Make symbol changes in COFF file
               if (err.Number()) return;  // Return if error
               COF2OMF();
               err.submit(1009);       // Warning: Converting to COFF and back again
            }
            break;

         case FILETYPE_COFF:
            OMF2COF();                 // Convert to COFF
            if (err.Number()) return;  // Return if error
            COF2COF();                 // Make symbol changes in COFF file
            break;

         case FILETYPE_ELF:
            OMF2COF();                 // Convert to COFF
            if (err.Number()) return;  // Return if error
            COF2COF();                 // Make symbol changes in COFF file
            if (err.Number()) return;  // Return if error
            COF2ELF();                 // Convert to ELF
            break;
            
         case FILETYPE_MACHO_LE:
            OMF2COF();                 // Convert to COFF
            if (err.Number()) return;  // Return if error
            COF2ELF();                 // Convert from COFF to ELF
            if (err.Number()) return;  // Return if error
            ELF2MAC();                 // Then convert from ELF to Mach-O
            if (err.Number()) return;  // Return if error
            MAC2MAC();                 // Make symbol changes in Mach-O file and sort symbol table
            break;

         case CMDL_OUTPUT_MASM:
            // Disassemble OMF file
            OMF2ASM();                 // Disassemble
            break;
            
         default:
            // Conversion not supported
            err.submit(2013, GetFileFormatName(FileType), GetFileFormatName(cmd.OutputType));
         }
         break;

      // Conversions from Mach-O
      case FILETYPE_MACHO_LE:

         switch (cmd.OutputType) {
         case FILETYPE_ELF:
            MAC2ELF();                 // Convert to ELF
            if (err.Number()) return;  // Return if error
            ELF2ELF();                 // Make symbol changes in ELF file
            break;

         case FILETYPE_COFF:
            MAC2ELF();                 // Convert to ELF
            if (err.Number()) return;  // Return if error
            ELF2ELF();                 // Make symbol changes in ELF file
            if (err.Number()) return;  // Return if error
            ELF2COF();                 // Convert to COFF
            break;

         case FILETYPE_OMF:
            MAC2ELF();                 // Convert to ELF
            if (err.Number()) return;  // Return if error
            ELF2ELF();                 // Make symbol changes in ELF file
            if (err.Number()) return;  // Return if error
            ELF2COF();                 // Convert to COFF
            if (err.Number()) return;  // Return if error
            COF2OMF();                 // Convert to OMF
            break;

         case FILETYPE_MACHO_LE:
            MAC2MAC();                 // Make symbol changes in mACH-o file
            break;

         case CMDL_OUTPUT_MASM:
            // Disassemble Mach-O file
            MAC2ASM();                 // Disassemble
            break;
            
         default:
            // Conversion not supported
            err.submit(2013, GetFileFormatName(FileType), GetFileFormatName(cmd.OutputType));
         }
         break;

      case FILETYPE_MAC_UNIVBIN:
         ParseMACUnivBin();   break;

      // Conversion from other types
      default:
         err.submit(2006, FileName, GetFileFormatName(FileType));   // Conversion of this file type not supported
      }
   }
}
