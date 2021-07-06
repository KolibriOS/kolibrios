(************************************************************

            Simple Stripped PE Binary File Dumper

 ************************************************************)
Unit SPEDump;
(* -------------------------------------------------------- *)
Interface
(* -------------------------------------------------------- *)
Uses KolibriOS;
(* -------------------------------------------------------- *)
Type
  Dword = LongWord;

  PDword = ^Dword;

  TDwordArray = Packed Array[0..0] Of Dword;

  PDwordArray = ^TDwordArray;

Const
  STRIPPED_PE_SIGNATURE   = $4503; // 'PE' Xor 'S'
  SPE_DIRECTORY_IMPORT    = 0;
  SPE_DIRECTORY_EXPORT    = 1;
  SPE_DIRECTORY_BASERELOC = 2;

  SPE_MAX_DIRECTORY_ENTRIES = SPE_DIRECTORY_BASERELOC;

Type
  TStrippedPEHeader = Packed Record
    Signature:           Word;
    Characteristics:     Word;
    AddressOfEntryPoint: Dword;
    ImageBase:           Dword;
    SectionAlignmentLog: Byte;
    FileAlignmentLog:    Byte;
    MajorOSVersion:      Byte;
    MinorOSVersion:      Byte;
    SizeOfImage:         Dword;
    SizeOfStackReserve:  Dword;
    SizeOfHeapReserve:   Dword;
    SizeOfHeaders:       Dword;
    Subsystem:           Byte;
    NumberOfRvaAndSizes: Byte;
    NumberOfSections:    Word;
  End;

  PStrippedPEHeader = ^TStrippedPEHeader;

  TStrippedSectionHeader = Packed Record
    Name: Packed Array[0..7] Of Char;
    VirtualSize:          Dword;
    VirtualAddress:       Dword;
    SizeOfRawData:        Dword;
    PointerToRawData:     Dword;
    Characteristics:      Dword;
  End;

  PStrippedSectionHeader = ^TStrippedSectionHeader;

  TDataDirectory = Packed Record
    VirtualAddress: Dword;
    Size:           Dword;
  End;

  PDataDirectory = ^TDataDirectory;

  TDataDirectoryArray = Packed Array[0..SPE_MAX_DIRECTORY_ENTRIES] Of TDataDirectory;

  PDataDirectoryArray = ^TDataDirectoryArray;

  TImportDescriptor = Packed Record
    OriginalFirstThunk: Dword;
    TimeDateStamp:      Dword;
    ForwarderChain:     Dword;
    Name:               Dword;
    FirstThunk:         Dword;
  End;

  PImportDescriptor = ^TImportDescriptor;

  TExportDescriptor = Packed Record
    Characteristics:       Dword;
    TimeDateStamp:         Dword;
    MajorVersion:          Word;
    MinorVersion:          Word;
    Name:                  Dword;
    Base:                  Dword;
    NumberOfFunctions:     Dword;
    NumberOfNames:         Dword;
    AddressOfFunctions:    Dword;
    AddressOfNames:        Dword;
    AddressOfNameOrdinals: Dword;
  End;

  PExportDescriptor = ^TExportDescriptor;

Var
  FileName:         PChar;
  FileHandle:       Integer;
  FileLength:       Dword;
  BytesRead:        Dword;
  Buffer:           PStrippedPEHeader;
  Section:          PStrippedSectionHeader;
  DataDirectory:    PDataDirectoryArray;
  ImportDescriptor: PImportDescriptor;
  ExportDescriptor: PExportDescriptor;
  Thunk:            PDword;

(* --------------------- Console stuff -------------------- *)
  hConsole:         Pointer;
  ConsoleInit:      Procedure(WndWidth, WndHeight, ScrWidth, ScrHeight: Dword; Caption: PChar); StdCall;
  ConsoleExit:      Procedure(bCloseWindow: Boolean); StdCall;
  Printf:           Function(Const Format: PChar): Integer; CDecl VarArgs;
  GetCh:            Function: Integer; StdCall;
  WriteN:           Procedure(Const Str: PChar; Count: Dword); StdCall;
  Write:            Procedure(Const Str: PChar); StdCall;

(* -------------------------------------------------------- *)
Procedure Main;
Function  FileIsValid: Boolean;
Procedure WriteHex(Number: Dword);
Procedure WriteLn(Text: PChar);
(* -------------------------------------------------------- *)
Implementation
(* -------------------------------------------------------- *)
Function FileIsValid : Boolean;
Begin
  FileIsValid := FALSE;
  With Buffer^ Do Begin
    If BytesRead < SizeOf(TStrippedPEHeader) Then Exit;
    If Signature <> STRIPPED_PE_SIGNATURE Then Exit;
  End;
  FileIsValid := TRUE;
End;
(* -------------------------------------------------------- *)
Function RVA2Offset(RVA: Dword; StrippedPEHeader: PStrippedPEHeader): Dword;
Var
  i: Dword;
  StrippedSectionHeader: PStrippedSectionHeader;
Begin
  With StrippedPEHeader^ Do Begin
    StrippedSectionHeader := PStrippedSectionHeader(Dword(StrippedPEHeader) + SizeOf(TStrippedPEHeader) + NumberOfRvaAndSizes * SizeOf(TDataDirectory));
    For i := 0 To NumberOfSections Do Begin
      With StrippedSectionHeader^ Do Begin
        If (RVA >= VirtualAddress) And (RVA < VirtualAddress + SizeOfRawData) Then Begin
          Result := PointerToRawData + RVA - VirtualAddress;
          Exit;
        End;
      End;
      Inc(StrippedSectionHeader);
    End;
  End;
  Result := 0;
End;
(* -------------------------------------------------------- *)
Procedure WriteHex(Number: Dword); Begin Printf('%X', Number); End;
(* -------------------------------------------------------- *)
Procedure WriteLn(Text: PChar); Begin Printf('%s'#10, Text); End;
(* -------------------------------------------------------- *)
Procedure Main;
Const
  CmdLine = PPChar(28);
Var 
  i: Dword;
Begin
  hConsole    := LoadLibrary('/sys/lib/console.obj');
  ConsoleInit := GetProcAddress(hConsole, 'con_init');
  ConsoleExit := GetProcAddress(hConsole, 'con_exit');
  Printf      := GetProcAddress(hConsole, 'con_printf');
  GetCh       := GetProcAddress(hConsole, 'con_getch');
  WriteN      := GetProcAddress(hConsole, 'con_write_string');
  Write       := GetProcAddress(hConsole, 'con_write_asciiz');
  ConsoleInit($FFFFFFFF, $FFFFFFFF, $FFFFFFFF, $FFFFFFFF, 'SPEDump');

  (*             skip spaces             *)
  i := 0; While CmdLine^[i] = ' ' Do Inc(i);
  FileName := @CmdLine^[i];
  
  WriteLn('Simple Stripped PE Binary File Dumper Version 0.1; 2018.');
  If FileName[0] = #0 Then Begin
    WriteLn('Usage: SPEDump [<file>]')
  End Else Begin
    WriteLn(''); Write('Dump of "'); Write(FileName); WriteLn('"'); WriteLn('');    
    Buffer := PStrippedPEHeader(LoadFile(FileName, BytesRead));
    If Buffer <> Nil Then Begin         
    
      If FileIsValid Then Begin
        WriteLn('File header');
        WriteLn('-----------');

        With Buffer^ Do Begin
          Write('  Signature             = '); WriteHex(Signature);           WriteLn('');
          Write('  Characteristics       = '); WriteHex(Characteristics);     WriteLn('');
          Write('  AddressOfEntryPoint   = '); WriteHex(AddressOfEntryPoint); WriteLn('');
          Write('  ImageBase             = '); WriteHex(ImageBase);           WriteLn('');
          Write('  SectionAlignmentLog   = '); WriteHex(SectionAlignmentLog); WriteLn('');
          Write('  FileAlignmentLog      = '); WriteHex(FileAlignmentLog);    WriteLn('');
          Write('  MajorOSVersion        = '); WriteHex(MajorOSVersion);      WriteLn('');
          Write('  MinorOSVersion        = '); WriteHex(MinorOSVersion);      WriteLn('');
          Write('  SizeOfImage           = '); WriteHex(SizeOfImage);         WriteLn('');
          Write('  SizeOfStackReserve    = '); WriteHex(SizeOfStackReserve);  WriteLn('');
          Write('  SizeOfHeapReserve     = '); WriteHex(SizeOfHeapReserve);   WriteLn('');
          Write('  SizeOfHeaders         = '); WriteHex(SizeOfHeaders);       WriteLn('');
          Write('  Subsystem             = '); WriteHex(Subsystem);           WriteLn('');
          Write('  NumberOfRvaAndSizes   = '); WriteHex(NumberOfRvaAndSizes); WriteLn('');
          Write('  NumberOfSections      = '); WriteHex(NumberOfSections);    WriteLn('');

          WriteLn('');

          If NumberOfSections > 0 Then Begin
            i := 1;
            Section := PStrippedSectionHeader(Dword(Buffer) + SizeOf(TStrippedPEHeader) + NumberOfRvaAndSizes * SizeOf(TDataDirectory));
            Repeat
              Write('Section #'); WriteHex(i); WriteLn('');
              WriteLn('-----------');
              With Section^ Do Begin                           
                Write('  Name                  = ');
                (* Handle situation when Name length = 8 Then Name is NOT ASCIIZ *)
                If Name[High(Name)] <> #0 Then WriteN(Name, 8) Else Write(Name);                                   
                WriteLn('');                
                Write('  VirtualSize           = '); WriteHex(VirtualSize);      WriteLn('');
                Write('  VirtualAddress        = '); WriteHex(VirtualAddress);   WriteLn('');
                Write('  SizeOfRawData         = '); WriteHex(SizeOfRawData);    WriteLn('');
                Write('  PointerToRawData      = '); WriteHex(PointerToRawData); WriteLn('');
                Write('  Flags                 = '); WriteHex(Characteristics);  WriteLn('');
              End;
              WriteLn('');
              inc(Section);
              inc(i);
            Until i > NumberOfSections;

            DataDirectory := PDataDirectoryArray(Dword(Buffer) + SizeOf(TStrippedPEHeader));

            If NumberOfRvaAndSizes > SPE_DIRECTORY_IMPORT Then Begin
              If DataDirectory[SPE_DIRECTORY_IMPORT].VirtualAddress <> 0 Then Begin
                WriteLn('Imports');
                WriteLn('-------');
                ImportDescriptor := PImportDescriptor(RVA2Offset(DataDirectory[SPE_DIRECTORY_IMPORT].VirtualAddress, Buffer) + Dword(Buffer));
                While ImportDescriptor.Name <> 0 Do Begin
                  With ImportDescriptor^ Do Begin
                    Write('  OriginalFirstThunk    = '); WriteHex(OriginalFirstThunk); WriteLn('');
                    Write('  TimeDateStamp         = '); WriteHex(TimeDateStamp);      WriteLn('');
                    Write('  ForwarderChain        = '); WriteHex(ForwarderChain);     WriteLn('');
                    Write('  Name                  = '); WriteLn(PChar(RVA2Offset(Name, Buffer) + Dword(Buffer)));
                    Write('  FirstThunk            = '); WriteHex(FirstThunk);         WriteLn('');
                  End;
                  Thunk := PDword(RVA2Offset(ImportDescriptor.FirstThunk, Buffer) + Dword(Buffer));
                  While Thunk^ <> 0 Do Begin
                    Write('    '); WriteLn(PChar(RVA2Offset(Thunk^, Buffer) + Dword(Buffer) + SizeOf(Word)));
                    Inc(Thunk);
                  End;
                  WriteLn('');
                  Inc(ImportDescriptor);
                End;
              End;
            End;

            If NumberOfRvaAndSizes > SPE_DIRECTORY_EXPORT Then Begin
              If DataDirectory[SPE_DIRECTORY_EXPORT].VirtualAddress <> 0 Then Begin
                WriteLn('Exports');
                WriteLn('-------');
                ExportDescriptor := PExportDescriptor(RVA2Offset(DataDirectory[SPE_DIRECTORY_EXPORT].VirtualAddress, Buffer) + Dword(Buffer));
                With ExportDescriptor^ Do Begin
                  Write('  Characteristics       = '); WriteHex(Characteristics);       WriteLn('');
                  Write('  TimeDateStamp         = '); WriteHex(TimeDateStamp);         WriteLn('');
                  Write('  MajorVersion          = '); WriteHex(MajorVersion);          WriteLn('');
                  Write('  MinorVersion          = '); WriteHex(MinorVersion);          WriteLn('');
                  Write('  Name                  = '); WriteLn(PChar(RVA2Offset(Name, Buffer) + Dword(Buffer)));
                  Write('  Base                  = '); WriteHex(Base);                  WriteLn('');
                  Write('  NumberOfFunctions     = '); WriteHex(NumberOfFunctions);     WriteLn('');
                  Write('  NumberOfNames         = '); WriteHex(NumberOfNames);         WriteLn('');
                  Write('  AddressOfFunctions    = '); WriteHex(AddressOfFunctions);    WriteLn('');
                  Write('  AddressOfNames        = '); WriteHex(AddressOfNames);        WriteLn('');
                  Write('  AddressOfNameOrdinals = '); WriteHex(AddressOfNameOrdinals); WriteLn('');
                  For i := 0 To NumberOfNames - 1 Do Begin
                    Write('    '); WriteLn(PChar(RVA2Offset(PDwordArray(RVA2Offset(AddressOfNames, Buffer) + Dword(Buffer))^[i], Buffer)) + Dword(Buffer));
                  End;
                End;
              End;
            End;

          End;
        End;
      End Else Begin
        WriteLn('File corrupted or invalid.')
      End;
    End Else Begin
      WriteLn('ReadFile Error.');
    End;
  End;
  GetCh;
  ConsoleExit(TRUE);
  ThreadTerminate;
End;
(* -------------------------------------------------------- *)
End.