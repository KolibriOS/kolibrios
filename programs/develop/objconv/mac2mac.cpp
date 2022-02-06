/****************************    mac2mac.cpp    *****************************
* Author:        Agner Fog
* Date created:  2008-05-25
* Last modified: 2008-05-25
* Project:       objconv
* Module:        mac2mac.cpp
* Description:
* Module for changing symbol names in Mach-O file
*
* Copyright 2008 GNU General Public License http://www.gnu.org/licenses
*****************************************************************************/
#include "stdafx.h"
// All functions in this module are templated to make two versions: 32 and 64 bits.
// See instantiations at the end of this file.


// Constructor
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
CMAC2MAC<MACSTRUCTURES>::CMAC2MAC() {
   // Initialize everything
   memset(this, 0, sizeof(*this));
}


// Convert()
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2MAC<MACSTRUCTURES>::Convert() {
   MakeSymbolTable();                  // Remake symbol tables and string tables
   MakeBinaryFile();                   // Put everyting together into ToFile
   ChangeSegments();                   // Modify section names and relocation table symbol indices
   *this << ToFile;                    // Take over new file buffer
}


// MakeSymbolTable()
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2MAC<MACSTRUCTURES>::MakeSymbolTable() {
   // Remake symbol tables and string table
   int OldScope = 0;                   // Old scope of symbol. 0=local, 1=public, 2=external
   int NewScope;                       // New scope of symbol. 0=local, 1=public, 2=external
   uint32 symi;                        // Old index of symbol
   const char * Name1;                 // Old symbol name
   const char * Name2;                 // New symbol name
   int action;                         // Action to take on symbol
   int SymType;                        // Symbol type
   int SymDesc;                        // Symbol descriptor
   uint8 Section;                      // Symbol section

   // pointer to symbol table
   TMAC_nlist * symp = (TMAC_nlist*)(this->Buf() + this->SymTabOffset);

   // pointer to string table
   char * strtab = (char*)(this->Buf() + this->StringTabOffset); 

   // loop through symbol table
   for (symi = 0; symi < this->SymTabNumber; symi++, symp++) {

      // Check indices for first symbol of each scope category
      if (symi == this->iextdefsym && this->nextdefsym) OldScope = 1;
      if (symi == this->iundefsym  && this->nundefsym)  OldScope = 2;
      NewScope = OldScope;

      if (symp->n_strx >= this->StringTabSize) {
         // Index out of range
         err.submit(2112); continue;
      }

      // Get symbol name
      Name1 = strtab + symp->n_strx;

      // Get type, descriptor and section
      SymType = symp->n_type;          // Symbol type
      SymDesc = symp->n_desc;          // Symbol descriptor
      Section = symp->n_sect;          // Symbol section

      // Check if any change required for this symbol
      action = cmd.SymbolChange(Name1, &Name2, SYMT_LOCAL + OldScope);

      switch (action) {
      case SYMA_NOCHANGE:
         // No change
         break;

      case SYMA_MAKE_WEAK:
         // Make symbol weak
         if (cmd.OutputType == FILETYPE_COFF) {
            // PE/COFF format does not support weak publics
            err.submit(2200);
         }
         // Make weak
         if (OldScope == 1) {
            SymDesc |= MAC_N_WEAK_DEF;   // Weak public. Allowed only in coalesced (communal) section
         }
         else if (OldScope == 2) {
            SymDesc |= MAC_N_WEAK_REF;   // Weak external
         }
         else {
            err.submit(1020, Name1);     // Local symbol
         }
         break;

      case SYMA_MAKE_LOCAL:
         // Make public symbol local, make external symbol ignored
         if (OldScope == 1) {
            NewScope = 0;  // Public changed to local
            SymType &= ~MAC_N_EXT;
         }
         else if (OldScope == 2) {
            Section = MAC_NO_SECT;  // External symbol. Set to 0
            SymDesc = 0;
            SymType = MAC_N_UNDF;
         }
         else err.submit(1021, Name1);
         break;

      case SYMA_CHANGE_NAME:
         // Change name of symbol
         Name1 = Name2;  Name2 = 0;
         break;

      case SYMA_ALIAS: 
         // Make alias and keep old name
         if (OldScope != 1) {
            err.submit(1022, Name1); break;
         }
         // Make alias
         NewSymbols[1].AddSymbol(-1, Name2, SymType, SymDesc, Section, symp->n_value);
         break;

      default:
         err.submit(9000); // unknown error
      }

      // Store symbol, possibly modified
      NewSymbols[NewScope].AddSymbol(symi, Name1, SymType, SymDesc, Section, symp->n_value);
   }

   // Put everything into symbol table and string table
   if (this->SymTabNumber) {
      NewStringTable.SetDataSize(1); // First record must indicate empty string (see nlist.n_un in Mach-O File Format Reference)
   }
   for (NewScope = 0; NewScope < 3; NewScope++) {
      NewSymbols[NewScope].SortList();  // Sort each list alphabetically
      NewSymbols[NewScope].StoreList(&NewSymbolTable, &NewStringTable);
   }

   // Indices to local, public and external symbols
   NewIlocalsym = 0;	                            // index to local symbols
   NewNlocalsym = NewSymbols[0].GetNumEntries(); // number of local symbols 
   NewIextdefsym = NewNlocalsym;	                // index to public symbols
   NewNextdefsym = NewSymbols[1].GetNumEntries();// number of public symbols 
   NewIundefsym = NewNlocalsym + NewNextdefsym;  // index to external symbols
   NewNundefsym = NewSymbols[2].GetNumEntries(); // number of external symbols

   // Calculate difference in size of new tables versus old tables
   // (this calculation is moved to MakeBinaryFile)
   // SizeDifference = NewSymbolTable.GetDataSize + NewStringTable.GetDataSize() 
   // - this->SymTabNumber * sizeof(TMAC_nlist) - this->StringTabSize;
}

template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
int CMAC2MAC<MACSTRUCTURES>::NewSymbolIndex(int32 OldIndex) {
   // Convert subfunction: Translate old to new symbol index
   int NewIndex;
   int Scope;
   // Search for symbol in all scopes
   for (Scope = 0; Scope < 3; Scope++) {
      NewIndex = NewSymbols[Scope].TranslateIndex(OldIndex);
      if (NewIndex >= 0) {
         // OldIndex found. Add offset into appropriate table
         if (Scope == 1) NewIndex += NewIextdefsym;
         else if (Scope == 2) NewIndex += NewIundefsym;
         return NewIndex;
      }
   }
   //err.submit(2031);
   return -1;
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
uint32 CMAC2MAC<MACSTRUCTURES>::NewFileOffset(uint32 OldOffset) {
   // Convert subfunction: Translate old to new file offset
   if (OldOffset <= NewSymtabOffset) {
      // Before symbol table. No change
      return OldOffset;
   }
   if (OldOffset >= OldTablesEnd) {
      // After string table. Add size difference
      return OldOffset + SizeDifference;
   }
   // Between symbol table and string table. 
   // The possibility of something between these two tables has not been accounted for
   err.submit(2052);
   return 0;
}


// MakeBinaryFile()
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2MAC<MACSTRUCTURES>::MakeBinaryFile() {
   uint32 OldSymtabEnd;                // End of old symbol table
   uint32 OldStringtabEnd;             // End of old string table
   const int WordSize = sizeof(MInt) * 8;  // Word size, 32 or 64 bits

   // Offset to symbol table and string table
   NewSymtabOffset = this->SymTabOffset;
   if (this->StringTabOffset && this->StringTabOffset < NewSymtabOffset) NewSymtabOffset = this->StringTabOffset;
   if (NewSymtabOffset == 0) NewSymtabOffset = this->GetDataSize();

   // Copy all headers and all data until TablesOffset
   ToFile.Push(this->Buf(), NewSymtabOffset);
   ToFile.Align(WordSize/8);
   NewSymtabOffset = ToFile.GetDataSize(); 

   // Copy new symbol table
   ToFile.Push(NewSymbolTable.Buf(), NewSymbolTable.GetDataSize());

   // Copy new string table
   NewStringtabOffset = ToFile.GetDataSize();
   ToFile.Push(NewStringTable.Buf(), NewStringTable.GetDataSize());
   ToFile.Align(2);
   NewStringtabEnd = ToFile.GetDataSize();

   // Find end of old tables
   OldSymtabEnd = this->SymTabOffset + this->SymTabNumber * sizeof(TMAC_nlist);
   OldStringtabEnd = this->StringTabOffset + this->StringTabSize;
   OldTablesEnd = OldStringtabEnd;
   if (OldSymtabEnd > OldTablesEnd) OldTablesEnd = OldSymtabEnd;
   if (OldTablesEnd == 0) OldTablesEnd = this->GetDataSize();

   // Size difference between new and old tables
   SizeDifference = NewStringtabEnd - OldTablesEnd;

   // Is there anything in the old file after these tables?
   if (OldTablesEnd && this->GetDataSize() > OldTablesEnd) {
      // There is something after these tables. Copy it
      ToFile.Push(this->Buf() + OldTablesEnd, this->GetDataSize() - OldTablesEnd);
   }
}


// ChangeSegments()
template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2MAC<MACSTRUCTURES>::ChangeSegments() {
   // Convert subfunction: Change section names if needed and adjust all relocation tables

   uint32 FileOffset;                  // Current offset into file
   uint32 lcmd;                        // Load command
   uint32 cmdsize = 0;                 // Command size
   uint32 icmd;                        // Loop counter
   int action;                         // Name change action
   char * Name1;                       // Old name
   const char * Name2;                 // New name

   FileOffset = sizeof(TMAC_header);
   // Loop through file commands
   for (icmd = 1; icmd <= this->FileHeader.ncmds; icmd++, FileOffset += cmdsize) {
      lcmd    = ((MAC_load_command*)(ToFile.Buf() + FileOffset)) -> cmd;
      cmdsize = ((MAC_load_command*)(ToFile.Buf() + FileOffset)) -> cmdsize;

      // Interpret specific command type
      switch(lcmd) {
      case MAC_LC_SEGMENT: { // 32-bit segment
         MAC_segment_command_32 * sh = (MAC_segment_command_32*)(ToFile.Buf() + FileOffset);
         Name1 = sh->segname;
         // Check if any change required for this symbol
         action = cmd.SymbolChange(Name1, &Name2, SYMT_SECTION);
         if (action == SYMA_CHANGE_NAME) {
            // Change segment name
            if (strlen(Name2) > 16) err.submit(1040);
            strncpy(Name1, Name2, 16);
         }
         // Change section names and relocations in all sections under this segment
         ChangeSections(FileOffset + sizeof(MAC_segment_command_32), sh->nsects);
         break;}

      case MAC_LC_SEGMENT_64: { // 64-bit segment
         MAC_segment_command_64 * sh = (MAC_segment_command_64*)(ToFile.Buf() + FileOffset);
         Name1 = sh->segname;
         // Check if any change required for this symbol
         action = cmd.SymbolChange(Name1, &Name2, SYMT_SECTION);
         if (action == SYMA_CHANGE_NAME) {
            // Change segment name
            if (strlen(Name2) > 16) err.submit(1040);
            strncpy(Name1, Name2, 16);
         }
         // Change section names and relocations in all sections under this segment
         ChangeSections(FileOffset + sizeof(MAC_segment_command_64), sh->nsects);
         break;}

      case MAC_LC_SYMTAB: { // Symbol table header
         MAC_symtab_command * sh = (MAC_symtab_command*)(ToFile.Buf() + FileOffset);
         // Change table addresses
         sh->symoff = NewSymtabOffset;
         sh->nsyms = NewSymbolTable.GetDataSize() / sizeof(TMAC_nlist);
         sh->stroff = NewStringtabOffset;
         sh->strsize = NewStringtabEnd - NewStringtabOffset;
         break;}

      case MAC_LC_DYSYMTAB: { // dynamic link-edit symbol table
         MAC_dysymtab_command * sh = (MAC_dysymtab_command*)(ToFile.Buf() + FileOffset);
         // Change indices to symbol tables
         sh->ilocalsym = NewIlocalsym;
         sh->nlocalsym = NewNlocalsym;
         sh->iextdefsym = NewIextdefsym;
         sh->nextdefsym = NewNextdefsym;
         sh->iundefsym = NewIundefsym;            
         sh->nundefsym = NewNundefsym;

         // Change table addresses
         sh->tocoff = NewFileOffset(sh->tocoff);
         sh->modtaboff = NewFileOffset(sh->modtaboff);
         sh->extrefsymoff = NewFileOffset(sh->extrefsymoff);
         sh->indirectsymoff = NewFileOffset(sh->indirectsymoff);
         sh->extreloff = NewFileOffset(sh->extreloff);
         sh->locreloff = NewFileOffset(sh->locreloff);

         if (sh->nindirectsyms) {
            // Change symbol indices in import table
            ChangeImportTable(sh->indirectsymoff, sh->nindirectsyms);
         }
         break;}
      }
   }
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2MAC<MACSTRUCTURES>::ChangeSections(uint32 HeaderOffset, uint32 Num) {
   // Convert subfunction: Change section names and relocation records if needed
   int action;                         // Name change action
   char * Name1;                       // Old name
   const char * Name2;                 // New name
   uint32 isec1;                       // Section index
   TMAC_section * secp;                // Pointer to section header
   uint32 irel;                        // Relocation index
   MAC_relocation_info * relp;         // Pointer to relocation record

   // Loop through section headers
   for (isec1 = 0; isec1 < Num; isec1++) {
      // Find section header
      secp = (TMAC_section*)(ToFile.Buf() + HeaderOffset + isec1*sizeof(TMAC_section));

      // Segment name
      Name1 = secp->segname;
      action = cmd.SymbolChange(Name1, &Name2, SYMT_SECTION);
      if (action == SYMA_CHANGE_NAME) {
         // Change segment name
         if (strlen(Name2) > 16) err.submit(1040);
         strncpy(Name1, Name2, 16);
      }

      // Section name
      Name1 = secp->sectname;
      action = cmd.SymbolChange(Name1, &Name2, SYMT_SECTION);
      if (action == SYMA_CHANGE_NAME) {
         // Change section name
         if (strlen(Name2) > 16) err.submit(1040);
         strncpy(Name1, Name2, 16);
      }

      // Update file offset
      secp->offset = NewFileOffset(secp->offset);

      if (secp->nreloc) {
         // This section has relocations
         // Update relocations offset
         secp->reloff = NewFileOffset(secp->reloff);

         // Pointer to relocation records
         relp = (MAC_relocation_info*)(ToFile.Buf() + secp->reloff);

         // Loop through relocations, if any
         for (irel = 0; irel < secp->nreloc; irel++, relp++) {
            // Only non-scattered r_extern relocations have symbol index
            if (!(relp->r_address & R_SCATTERED) && relp->r_extern) {
               // Update symbol index
               relp->r_symbolnum = NewSymbolIndex(relp->r_symbolnum);
            }
         }
      }
   }
}


template <class TMAC_header, class TMAC_segment_command, class TMAC_section, class TMAC_nlist, class MInt>
void CMAC2MAC<MACSTRUCTURES>::ChangeImportTable(uint32 FileOffset, uint32 Num) {
   // Convert subfunction: Change symbol indices in import table if needed
   uint32 i;                           // Index
   uint32 * p;                         // pointer to current entry

   // Find first entry
   p = (uint32*)(ToFile.Buf() + FileOffset);

   // Loop through table
   for (i = 0;  i < Num; i++, p++) {
      // Translate symbol index
      *p = NewSymbolIndex(*p);
   }
}


// Make template instances for 32 and 64 bits
template class CMAC2MAC<MAC32STRUCTURES>;
template class CMAC2MAC<MAC64STRUCTURES>;
