/****************************   cof2cof.cpp   *********************************
* Author:        Agner Fog
* Date created:  2006-07-28
* Last modified: 2006-07-28
* Project:       objconv
* Module:        cof2cof.cpp
* Description:
* Module for changing symbol names in PE/COFF file
*
* Copyright 2006-2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"


CCOF2COF::CCOF2COF () {
   // Constructor
}

void CCOF2COF::Convert() {
   // Do the conversion

   // Call the subfunctions
   MakeSymbolTable();          // Symbol table and string tables
   MakeBinaryFile();           // Putting sections together
   *this << ToFile;            // Take over new file buffer
}


void CCOF2COF::MakeSymbolTable() {
   // Convert subfunction: Make symbol table and string tables
   int isym;                   // current symbol table entry
   int numaux;                 // Number of auxiliary entries in source record
   int symboltype = 0;         // Symbol type
   int action = 0;             // Symbol change action
   int isec;                   // Section number

   const char * name1;         // Old name of symbol
   const char * name2;         // Changed name of symbol
   const char * name3;         // New name to store

   // Pointer to old symbol table
   union {
      SCOFF_SymTableEntry * p; // Symtab entry pointer
      int8 * b;                // Used for increment
   } OldSymtab;

   // Initialize new string table. Make space for size
   NewStringTable.Push(0, 4);

   // Loop through source symbol table
   OldSymtab.p = SymbolTable;  // Pointer to source symbol table
   for (isym = 0; isym < NumberOfSymbols; isym += numaux+1, OldSymtab.b += SIZE_SCOFF_SymTableEntry*(numaux+1)) {

      // Number of auxiliary records belonging to same symbol
      numaux = OldSymtab.p->s.NumAuxSymbols;  if (numaux < 0) numaux = 0;

      // Get first aux record if numaux > 0
      SCOFF_SymTableEntry * sa = (SCOFF_SymTableEntry *)(OldSymtab.b + SIZE_SCOFF_SymTableEntry);

      // Check symbol type
      if (numaux && OldSymtab.p->s.StorageClass == COFF_CLASS_STATIC) {
         // This is a section definition record
         // aux record contains length and number of relocations. Ignore aux record
         symboltype = SYMT_SECTION;
         name1 = GetSymbolName(OldSymtab.p->s.Name);
      }
      else if (OldSymtab.p->s.StorageClass == COFF_CLASS_FILE) {
         // This is a filename record
         symboltype = SYMT_OTHER;
         name1 = GetShortFileName(OldSymtab.p);
         // or long file name ?!
      }
      else if (OldSymtab.p->s.Type == 0 && OldSymtab.p->s.StorageClass == COFF_CLASS_FUNCTION) {
         // This is a .bf, .lf, or .ef record following a function record
         // Contains line number information etc. Ignore this record
         name1 = 0;
      }
      else {
         // This is a symbol record
         // Symbol name
         name1 = GetSymbolName(OldSymtab.p->s.Name);
         if (OldSymtab.p->s.StorageClass == COFF_CLASS_EXTERNAL) {
            // This is a public or external symbol
            if (OldSymtab.p->s.SectionNumber <= 0) {
               // This is an external symbol
               symboltype = SYMT_EXTERNAL;
            }
            else {
               // This is a public symbol
               symboltype = SYMT_PUBLIC;
            }
         }
         else {
            // This is a local symbol
            symboltype = SYMT_LOCAL;
         }
      }
      name3 = name1;
      // Check if any change required for this symbol
      action = cmd.SymbolChange(name1, &name2, symboltype);

      switch (action) {
      case SYMA_NOCHANGE:
         // No change
         break;

      case SYMA_MAKE_WEAK:
         // Make symbol weak
         if (cmd.OutputType == FILETYPE_COFF) {
            // PE/COFF format does not support weak publics. Use this only when converting to ELF
            err.submit(2200);
         }
         // Make weak when converting to ELF
         OldSymtab.p->s.StorageClass = COFF_CLASS_WEAK_EXTERNAL;
         break;

      case SYMA_MAKE_LOCAL:
         // Make public symbol local, make external symbol ignored
         OldSymtab.p->s.StorageClass = COFF_CLASS_STATIC;
         break;

      case SYMA_CHANGE_NAME:
         // Change name of symbol
         if (OldSymtab.p->s.StorageClass == COFF_CLASS_FILE) {
            // File name is stored in aux records, not in symbol table
            if ((uint32)strlen(name2) > (uint32)numaux * SIZE_SCOFF_SymTableEntry) {
               // Name too long. I don't want to add more aux records
               err.submit(2201, name2); 
            }
            else {
               // Insert new file name in aux records
               memset(sa, 0, numaux * SIZE_SCOFF_SymTableEntry);
               memcpy(sa, name2, strlen(name2));
            }
         }
         else {
            // Symbol name stored in normal way
            name3 = name2;
         }
         break;

      case SYMA_ALIAS: {
         // Make alias and keep old name
         SCOFF_SymTableEntry AliasEntry = *OldSymtab.p;
         AliasEntry.s.Type = 0;  // Make alias a label, not a function
         AliasEntry.s.NumAuxSymbols = 0;  // No auxiliary .bf and .ef records
         // Put new name into AliasEntry
         memset(AliasEntry.s.Name, 0, 8);
         if (strlen(name2) > 8) {
            // Long name. use string table
            // Store string table offset
            ((uint32 *)(AliasEntry.s.Name))[1] = NewStringTable.GetDataSize();
            // Put name into new string table
            NewStringTable.PushString(name2);
         }
         else {
           // Short name. Store in record
            memcpy(AliasEntry.s.Name, name2, strlen(name2));
         }
         // Add new entry to extra symbol table
         NewSymbolTable.Push(&AliasEntry, SIZE_SCOFF_SymTableEntry);
         break;}

      default:
         err.submit(9000); // unknown error
      }

      if (name3 && OldSymtab.p->s.StorageClass != COFF_CLASS_FILE) {
         // Store old or new name
         if (strlen(name3) > 8) {
            // Name is long. use string table
            // Type-case Name field to string table entry
            uint32 * LongNameStorage = (uint32 *)(OldSymtab.p->s.Name);
            // Start with 0 to indicate long name
            LongNameStorage[0] = 0;
            // Index into new string table
            LongNameStorage[1] = NewStringTable.GetDataSize();
            // Put name into new string table
            NewStringTable.PushString(name3);
         }
         else {
            if (name3 != name1) {
               // Store new name in Name field
               memset(OldSymtab.p->s.Name, 0, 8);
               memcpy(OldSymtab.p->s.Name, name3, strlen(name3));
            }
         }
      }
   }  // End symbol table loop

   // Loop through section headers to search for section names
   uint32 SectionOffset = sizeof(SCOFF_FileHeader) + FileHeader->SizeOfOptionalHeader;
   for (isec = 0; isec < NSections; isec++) {
      SCOFF_SectionHeader * pSectHeader;
      pSectHeader = &Get<SCOFF_SectionHeader>(SectionOffset);
      SectionOffset += sizeof(SCOFF_SectionHeader);

      // Get section name
      name1 = GetSectionName(pSectHeader->Name);

      // Check if change required
      action = cmd.SymbolChange(name1, &name2, SYMT_SECTION);
      if (action == SYMA_CHANGE_NAME) name1 = name2;

      // Store name (changed or unchanged)
      memset(pSectHeader->Name, 0, 8);
      if (strlen(name1) <= 8) {
         // Short name. Store in section header
         memcpy(pSectHeader->Name, name1, strlen(name1));
      }
      else {
         // Long name. Store in string table
         sprintf(pSectHeader->Name, "/%i", NewStringTable.GetDataSize());
         //pSectHeader->Name[0] = '/';
         //itoa(NewStringTable.GetDataSize(), pSectHeader->Name+1, 10);
         NewStringTable.PushString(name1);
      }
   }
}


void CCOF2COF::MakeBinaryFile() {
   // Convert subfunction: Combine everything into the new binary file
   int i;

   // New file header = copy of old file header
   SCOFF_FileHeader NewFileHeader = *FileHeader;

   ToFile.SetFileType(FILETYPE_COFF); // Set type of output file
   ToFile.WordSize = WordSize;
   ToFile.FileName = FileName;

   // Copy file header, section headers and sections to new file
   ToFile.Push(Buf(), NewFileHeader.PSymbolTable);

   // Copy symbol table
   ToFile.Push(SymbolTable, NumberOfSymbols * SIZE_SCOFF_SymTableEntry);

   // Additions to symbol table
   int NumAddedSymbols = NewSymbolTable.GetNumEntries();
   if (NumAddedSymbols) {
      // Append to symbols table
      ToFile.Push(NewSymbolTable.Buf(), NumAddedSymbols * SIZE_SCOFF_SymTableEntry);
      // Update NumberOfSymbols in file header
      NewFileHeader.NumberOfSymbols += NumAddedSymbols;
   }

   // Insert new string table
   uint32 NewStringTableSize = NewStringTable.GetDataSize();
   // First 4 bytes = size
   ToFile.Push(&NewStringTableSize, sizeof(uint32));
   // Then the string table itself, except the first 4 bytes
   if (NewStringTableSize > 4) 
      ToFile.Push(NewStringTable.Buf() + 4, NewStringTableSize - 4);

   // Find end of old and new string tables
   uint32 EndOfOldStringTable = FileHeader->PSymbolTable 
      + NumberOfSymbols * SIZE_SCOFF_SymTableEntry + StringTableSize;

   uint32 EndOfNewStringTable = FileHeader->PSymbolTable 
      + (NumberOfSymbols + NumAddedSymbols) * SIZE_SCOFF_SymTableEntry + NewStringTableSize;

   // Check if there is anything after the string table
   if (GetDataSize() > EndOfOldStringTable) {
      // Old file has something after the string table

      if (EndOfNewStringTable < EndOfOldStringTable) {
         // New symboltable + string table smaller than old
         // Fill the space with zeroes so that the data that come after the string table
         // will have the same address as before
         ToFile.Push(0, EndOfOldStringTable - EndOfNewStringTable);
         EndOfNewStringTable = EndOfOldStringTable;
      }

      // Copy the data that come after the string table
      ToFile.Push(Buf() + EndOfOldStringTable, GetDataSize() - EndOfOldStringTable);

      if (EndOfNewStringTable > EndOfOldStringTable) {
         // New symboltable + string table bigger than old
         // Find all references to the data that come after the string table and fix them
         // Search all section headers
         uint32 SectionOffset = sizeof(SCOFF_FileHeader) + NewFileHeader.SizeOfOptionalHeader;
         for (i = 0; i < NSections; i++) {
            SCOFF_SectionHeader * pSectHeader;
            pSectHeader = &Get<SCOFF_SectionHeader>(SectionOffset);
            SectionOffset += sizeof(SCOFF_SectionHeader);
            if (pSectHeader->PRawData >= EndOfOldStringTable && pSectHeader->PRawData <= GetDataSize()) {
               pSectHeader->PRawData += EndOfNewStringTable - EndOfOldStringTable;
            }
            if (pSectHeader->PRelocations >= EndOfOldStringTable && pSectHeader->PRawData <= GetDataSize()) {
               pSectHeader->PRelocations += EndOfNewStringTable - EndOfOldStringTable;
            }            if (pSectHeader->PLineNumbers >= EndOfOldStringTable && pSectHeader->PRawData <= GetDataSize()) {
               pSectHeader->PLineNumbers += EndOfNewStringTable - EndOfOldStringTable;
            }
         }
      }
   }
   // Update file header
   memcpy(ToFile.Buf(), &NewFileHeader, sizeof(NewFileHeader));

   // Note: The checksum in the optional header may need to be updated.
   // This is relevant for DLL's only. The checksum algorithm is undisclosed and
   // must be calculated with IMAGHELP.DLL. You may add a calculation of the checksum
   // here if you want the program to be able to change names in a Windows DLL,
   // but the program will then only be able to compile under Windows.
}
