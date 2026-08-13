// XD Pascal - a 32-bit compiler for Windows
// Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov

{$I-}
{$H-}

unit Linker;


interface


uses Common, CodeGen;


procedure InitializeLinker;
procedure SetProgramEntryPoint;
function AddImportFunc(const ImportLibName, ImportFuncName: TString): LongInt;
procedure LinkAndWriteProgram(const ExeName: TString);



implementation

 
const
  IMGBASE           = $0;
  SECTALIGN         = $20;
  FILEALIGN         = SECTALIGN;
  
  MAXIMPORTLIBS     = 100;
  MAXIMPORTS        = 2000;

    
type
  TDOSStub = array [0..127] of Byte;
 

  TPEHeader = packed record
    PE: array [0..3] of TCharacter;
    Machine: Word;
    NumberOfSections: Word;
    TimeDateStamp: LongInt;
    PointerToSymbolTable: LongInt;
    NumberOfSymbols: LongInt;
    SizeOfOptionalHeader: Word;
    Characteristics: Word;
  end;


  TPEOptionalHeader = packed record
    Magic: Word;
    MajorLinkerVersion: Byte;
    MinorLinkerVersion: Byte;
    SizeOfCode: LongInt;
    SizeOfInitializedData: LongInt;
    SizeOfUninitializedData: LongInt;
    AddressOfEntryPoint: LongInt;
    BaseOfCode: LongInt;
    BaseOfData: LongInt;
    ImageBase: LongInt;
    SectionAlignment: LongInt;
    FileAlignment: LongInt;
    MajorOperatingSystemVersion: Word;
    MinorOperatingSystemVersion: Word;
    MajorImageVersion: Word;
    MinorImageVersion: Word;
    MajorSubsystemVersion: Word;
    MinorSubsystemVersion: Word;
    Win32VersionValue: LongInt;
    SizeOfImage: LongInt;
    SizeOfHeaders: LongInt;
    CheckSum: LongInt;
    Subsystem: Word;
    DllCharacteristics: Word;
    SizeOfStackReserve: LongInt;
    SizeOfStackCommit: LongInt;
    SizeOfHeapReserve: LongInt;
    SizeOfHeapCommit: LongInt;
    LoaderFlags: LongInt;
    NumberOfRvaAndSizes: LongInt;
  end;
  
  
  TDataDirectory = packed record
    VirtualAddress: LongInt;
    Size: LongInt;
  end;  


  TPESectionHeader = packed record
    Name: array [0..7] of TCharacter;
    VirtualSize: LongInt;
    VirtualAddress: LongInt;
    SizeOfRawData: LongInt;
    PointerToRawData: LongInt;
    PointerToRelocations: LongInt;
    PointerToLinenumbers: LongInt;
    NumberOfRelocations: Word;
    NumberOfLinenumbers: Word;
    Characteristics: LongInt;
  end;
  
  
  THeaders = packed record
    Signature:  array [0..7] of TCharacter;
    Version:    LongInt;
    EntryPoint: LongInt;
    EndImage:   LongInt;
    Memory:     LongInt;
    StackTop:   LongInt;
    CmdLine:    LongInt;
    FilePath:   LongInt;
  end;
  
  
  TImportLibName = array [0..15] of TCharacter;
  TImportFuncName = array [0..31] of TCharacter;


  TImportDirectoryTableEntry = packed record
    Characteristics: LongInt;
    TimeDateStamp: LongInt;
    ForwarderChain: LongInt;
    Name: LongInt;
    FirstThunk: LongInt;
  end; 


  TImportNameTableEntry = packed record
    Hint: Word;
    Name: TImportFuncName;
  end;
  
  
  TImport = record
    LibName, FuncName: TString;
  end; 


  TImportSectionData = record
    DirectoryTable: array [1..MAXIMPORTLIBS + 1] of TImportDirectoryTableEntry;
    LibraryNames: array [1..MAXIMPORTLIBS] of TImportLibName;
    LookupTable: array [1..MAXIMPORTS + MAXIMPORTLIBS] of LongInt;
    NameTable: array [1..MAXIMPORTS] of TImportNameTableEntry;   
    NumImports, NumImportLibs: Integer;
  end;



var
  Headers: THeaders; 
  Import: array [1..MAXIMPORTS] of TImport;
  ImportSectionData: TImportSectionData;
  LastImportLibName: TString;
  ProgramEntryPoint: LongInt;
  
  CodeSectionHeader_VirtualAddress:   LongInt;
  DataSectionHeader_VirtualAddress:   LongInt;
  BSSSectionHeader_VirtualAddress:    LongInt;
  ImportSectionHeader_VirtualAddress: LongInt;  
{  
const
  DOSStub: TDOSStub = 
    (
    $4D, $5A, $90, $00, $03, $00, $00, $00, $04, $00, $00, $00, $FF, $FF, $00, $00,
    $B8, $00, $00, $00, $00, $00, $00, $00, $40, $00, $00, $00, $00, $00, $00, $00,
    $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
    $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $00, $00, $00,
    $0E, $1F, $BA, $0E, $00, $B4, $09, $CD, $21, $B8, $01, $4C, $CD, $21, $54, $68,
    $69, $73, $20, $70, $72, $6F, $67, $72, $61, $6D, $20, $63, $61, $6E, $6E, $6F,
    $74, $20, $62, $65, $20, $72, $75, $6E, $20, $69, $6E, $20, $44, $4F, $53, $20,
    $6D, $6F, $64, $65, $2E, $0D, $0D, $0A, $24, $00, $00, $00, $00, $00, $00, $00
    );
}
 

 
procedure Pad(var f: file; Size, Alignment: Integer);
var
  i: Integer;
  b: Byte;
begin
b := 0;
for i := 0 to Align(Size, Alignment) - Size - 1 do
  BlockWrite(f, b, 1);
end;



  
procedure FillHeaders(CodeSize, InitializedDataSize, UninitializedDataSize, ImportSize: Integer);
const
  PATH_SIZE   = 1024;
  PARAMS_SIZE = 256;
  STACK_SIZE  = 1024 * 1024;

begin
FillChar(Headers, SizeOf(Headers), #0);

  CodeSectionHeader_VirtualAddress   := Align(SizeOf(Headers), SECTALIGN);
  DataSectionHeader_VirtualAddress   := Align(SizeOf(Headers), SECTALIGN) + Align(CodeSize, SECTALIGN);
  BSSSectionHeader_VirtualAddress    := Align(SizeOf(Headers), SECTALIGN) + Align(CodeSize, SECTALIGN) + Align(InitializedDataSize, SECTALIGN);
  ImportSectionHeader_VirtualAddress := Align(SizeOf(Headers), SECTALIGN) + Align(CodeSize, SECTALIGN) + Align(InitializedDataSize, SECTALIGN) + Align(UninitializedDataSize, SECTALIGN);

  with Headers do
  begin
    Signature[0] := 'M';
    Signature[1] := 'E';
    Signature[2] := 'N';
    Signature[3] := 'U';
    Signature[4] := 'E';
    Signature[5] := 'T';
    Signature[6] := '0';
    Signature[7] := '1';
    Version      := 1;
    EntryPoint   := Align(SizeOf(Headers), SECTALIGN) + ProgramEntryPoint;  
    EndImage     := Align(SizeOf(Headers), SECTALIGN) + Align(CodeSize, SECTALIGN) + Align(InitializedDataSize, SECTALIGN);
    FilePath     := EndImage + Align(UninitializedDataSize, SECTALIGN);
    Memory       := FilePath + PATH_SIZE + PARAMS_SIZE + STACK_SIZE;
    StackTop     := FilePath + PATH_SIZE + PARAMS_SIZE + STACK_SIZE;
    CmdLine      := FilePath + PATH_SIZE;  
  end;  
end;




procedure InitializeLinker;
begin
FillChar(Import, SizeOf(Import), #0);
FillChar(ImportSectionData, SizeOf(ImportSectionData), #0);
LastImportLibName := '';
ProgramEntryPoint := 0;
end;




procedure SetProgramEntryPoint;
begin
if ProgramEntryPoint <> 0 then
  Error('Duplicate program entry point');
  
ProgramEntryPoint := GetCodeSize;
end;


    

function AddImportFunc(const ImportLibName, ImportFuncName: TString): LongInt;
begin
with ImportSectionData do
  begin  
  Inc(NumImports);
  if NumImports > MAXIMPORTS then
    Error('Maximum number of import functions exceeded');

  Import[NumImports].LibName := ImportLibName;
  Import[NumImports].FuncName := ImportFuncName;
  
  if ImportLibName <> LastImportLibName then
    begin
    Inc(NumImportLibs);
    if NumImportLibs > MAXIMPORTLIBS then
      Error('Maximum number of import libraries exceeded');
    LastImportLibName := ImportLibName;
    end;
    
  Result := (NumImports - 1 + NumImportLibs - 1) * SizeOf(LongInt);  // Relocatable  
  end;
end;




procedure FillImportSection(var ImportSize, LookupTableOffset: Integer);
var
  ImportIndex, ImportLibIndex, LookupIndex: Integer;
  LibraryNamesOffset, NameTableOffset: Integer;

begin
with ImportSectionData do
  begin
  LibraryNamesOffset :=                      SizeOf(DirectoryTable[1]) * (NumImportLibs + 1);  
  LookupTableOffset  := LibraryNamesOffset + SizeOf(LibraryNames[1])   *  NumImportLibs;
  NameTableOffset    := LookupTableOffset  + SizeOf(LookupTable[1])    * (NumImports + NumImportLibs);
  ImportSize         := NameTableOffset    + SizeOf(NameTable[1])      *  NumImports;  
  
  LastImportLibName := '';
  ImportLibIndex := 0;
  LookupIndex := 0;
    
  for ImportIndex := 1 to NumImports do
    begin   
    // Add new import library
    if (ImportLibIndex = 0) or (Import[ImportIndex].LibName <> LastImportLibName) then
      begin    
      if ImportLibIndex <> 0 then Inc(LookupIndex);  // Add null entry before the first thunk of a new library    

      Inc(ImportLibIndex);

      DirectoryTable[ImportLibIndex].Name       := LibraryNamesOffset + SizeOf(LibraryNames[1]) * (ImportLibIndex - 1);                                                                             
      DirectoryTable[ImportLibIndex].FirstThunk := LookupTableOffset  + SizeOf(LookupTable[1])  *  LookupIndex;

      Move(Import[ImportIndex].LibName[1], LibraryNames[ImportLibIndex], Length(Import[ImportIndex].LibName));

      LastImportLibName := Import[ImportIndex].LibName;   
      end; // if

    // Add new import function
    Inc(LookupIndex);
    if LookupIndex > MAXIMPORTS + MAXIMPORTLIBS then
      Error('Maximum number of lookup entries exceeded');
      
    LookupTable[LookupIndex] := NameTableOffset + SizeOf(NameTable[1]) * (ImportIndex - 1);                                              

    Move(Import[ImportIndex].FuncName[1], NameTable[ImportIndex].Name, Length(Import[ImportIndex].FuncName));
    end;
  end; 
end;




procedure FixupImportSection(VirtualAddress: LongInt);
var
  i: Integer;
begin
with ImportSectionData do
  begin
  for i := 1 to NumImportLibs do
    with DirectoryTable[i] do
      begin
      Name := Name + VirtualAddress;
      FirstThunk := FirstThunk + VirtualAddress;
      end;
      
  for i := 1 to NumImports + NumImportLibs do
    if LookupTable[i] <> 0 then 
      LookupTable[i] := LookupTable[i] + VirtualAddress;
  end;  
end;




procedure LinkAndWriteProgram(const ExeName: TString);
var
  OutFile: TOutFile;
  CodeSize, ImportSize, LookupTableOffset: Integer;
  
begin
if ProgramEntryPoint = 0 then 
  Error('Program entry point not found');

CodeSize := GetCodeSize;

FillImportSection(ImportSize, LookupTableOffset);
FillHeaders(CodeSize, InitializedGlobalDataSize, UninitializedGlobalDataSize, ImportSize);

Relocate(IMGBASE + CodeSectionHeader_VirtualAddress,  
         IMGBASE + DataSectionHeader_VirtualAddress,  
         IMGBASE + BSSSectionHeader_VirtualAddress,   
         IMGBASE + ImportSectionHeader_VirtualAddress + LookupTableOffset);

//FixupImportSection(Headers.ImportSectionHeader.VirtualAddress);

// Write output file
Assign(OutFile, TGenericString(ExeName));
Rewrite(OutFile, 1);

if IOResult <> 0 then
  Error('Unable to open output file ' + ExeName);
  
BlockWrite(OutFile, Headers, SizeOf(Headers));
Pad(OutFile, SizeOf(Headers), FILEALIGN);

BlockWrite(OutFile, Code, CodeSize);
Pad(OutFile, CodeSize, FILEALIGN);

BlockWrite(OutFile, InitializedGlobalData, InitializedGlobalDataSize);
Pad(OutFile, InitializedGlobalDataSize, FILEALIGN);
{
with ImportSectionData do
  begin
  BlockWrite(OutFile, DirectoryTable, SizeOf(DirectoryTable[1]) * (NumImportLibs + 1));
  BlockWrite(OutFile, LibraryNames,   SizeOf(LibraryNames[1])   *  NumImportLibs);
  BlockWrite(OutFile, LookupTable,    SizeOf(LookupTable[1])    * (NumImports + NumImportLibs));
  BlockWrite(OutFile, NameTable,      SizeOf(NameTable[1])      *  NumImports);
  end;  
Pad(OutFile, ImportSize, FILEALIGN);
}
Close(OutFile); 
end;


end. 

