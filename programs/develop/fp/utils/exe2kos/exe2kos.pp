{$mode objfpc}
{$apptype console}

program exe2kos;

uses
  SysUtils, Classes, ExeTypes, KosTypes;

const
  ARGS_SIZE = 512;
  PATH_SIZE = 512;

type
  EExeError = class(Exception);

  TExeImage = class;
  TExeImageSection = class;

  TExeImageSection = class
  private
    FExeImage: TExeImage;
    FHeader: IMAGE_SECTION_HEADER;
    FName: String;
    procedure Read(var Buffer);
  public
    constructor Create(AExeImage: TExeImage; APosition: DWord);
    property Name: String read FName;
    property VirtualSize: DWord read FHeader.PhysicalAddress;
    property SectionRVA: DWord read FHeader.VirtualAddress;
    property PhysicalSize: DWord read FHeader.SizeOfRawData;
    property PhysicalOffset: DWord read FHeader.PointerToRawData;
    property ObjectFlags: DWord read FHeader.Characteristics;
  end;

  TExeImageSections = class
  private
    FCount: DWord;
    FItems: array of TExeImageSection;
    function GetItem(Index: DWord): TExeImageSection;
  public
    constructor Create(AExeImage: TExeImage; APosition, ACount: DWord);
    destructor Destroy; override;
    function ByName(AName: String): TExeImageSection;
    property Count: DWord read FCount;
    property Items[Index: DWord]: TExeImageSection read GetItem; default;
  end;

  TExeImage = class
  private
    FFileName: String;
    FFileStream: TStream;
    FDosHeader: IMAGE_DOS_HEADER;
    FNTHeader: IMAGE_NT_HEADERS;
    FSections: TExeImageSections;
    procedure Read(var Buffer; Position, Size: Longint);
    function GetSizeOfCode(): DWord;
    function GetSizeOfInitData(): DWord;
    function GetSizeOfUnInitData(): DWord;
    function GetEntryPoint(): DWord;
    function GetImageBase(): DWord;
    function GetObjectAlign(): DWord;
    function GetFileAlign(): DWord;
    function GetImageSize(): DWord;
    function GetHeaderSize(): DWord;
    function GetStackReserveSize(): DWord;
    function GetStackCommitSize(): DWord;
    function GetHeapReserveSize(): DWord;
    function GetHeapCommitSize(): DWord;
  public
    constructor Create(AFileName: String);
    destructor Destroy; override;
    property FileName: String read FFileName;
    property Sections: TExeImageSections read FSections;
    property SizeOfCode: DWord read GetSizeOfCode;
    property SizeOfInitializedData: DWord read GetSizeOfInitData;
    property SizeOfUninitializedData: DWord read GetSizeOfUnInitData;
    property EntryPoint: DWord read FNTHeader.OptionalHeader.AddressOfEntryPoint{GetEntryPoint};
    property ImageBase: DWord read FNTHeader.OptionalHeader.ImageBase{GetImageBase};
    property ObjectAlign: DWord read GetObjectAlign;
    property FileAlign: DWord read GetFileAlign;
    property ImageSize: DWord read GetImageSize;
    property HeaderSize: DWord read GetHeaderSize;
    property StackReserveSize: DWord read GetStackReserveSize;
    property StackCommitSize: DWord read GetStackCommitSize;
    property HeapReserveSize: DWord read GetHeapReserveSize;
    property HeapCommitSize: DWord read GetHeapCommitSize;
  end;


constructor TExeImage.Create(AFileName: String);
begin
  FFileName := AFileName;
  FFileStream := TFileStream.Create(FFileName, fmOpenRead);

  Read(FDosHeader, 0, SizeOf(FDosHeader));
  if not FDosHeader.e_magic = IMAGE_DOS_SIGNATURE then
    EExeError.Create('Unrecognized file format');

  Read(FNTHeader, FDosHeader.e_lfanew, SizeOf(FNTHeader));
  if FNTHeader.Signature <> IMAGE_NT_SIGNATURE then
    EExeError.Create('Not a PE (WIN32 Executable) file');

  FSections := TExeImageSections.Create(Self,
    FDosHeader.e_lfanew + SizeOf(FNTHeader), FNTHeader.FileHeader.NumberOfSections);
end;

destructor TExeImage.Destroy;
begin
  FSections.Free;
  FFileStream.Free;
end;

procedure TExeImage.Read(var Buffer; Position, Size: Longint);
begin
  FFileStream.Position := Position;
  if FFileStream.Read(Buffer, Size) <> Size then
    EExeError.Create('Damaged or unrecognized file');
end;

function TExeImage.GetSizeOfCode(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfCode;
end;

function TExeImage.GetSizeOfInitData(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfInitializedData;
end;

function TExeImage.GetSizeOfUnInitData(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfUninitializedData;
end;

function TExeImage.GetEntryPoint(): DWord;
begin
  Result := FNTHeader.OptionalHeader.AddressOfEntryPoint;
end;

function TExeImage.GetImageBase(): DWord;
begin
  Result := FNTHeader.OptionalHeader.ImageBase;
end;

function TExeImage.GetObjectAlign(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SectionAlignment;
end;

function TExeImage.GetFileAlign(): DWord;
begin
  Result := FNTHeader.OptionalHeader.FileAlignment;
end;

function TExeImage.GetImageSize(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfImage;
end;

function TExeImage.GetHeaderSize(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfHeaders;
end;

function TExeImage.GetStackReserveSize(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfStackReserve;
end;

function TExeImage.GetStackCommitSize(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfStackCommit;
end;

function TExeImage.GetHeapReserveSize(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfHeapReserve;
end;

function TExeImage.GetHeapCommitSize(): DWord;
begin
  Result := FNTHeader.OptionalHeader.SizeOfHeapCommit;
end;


constructor TExeImageSections.Create(AExeImage: TExeImage; APosition, ACount: DWord);
var
  i: Integer;
begin
  FCount := ACount;
  SetLength(FItems, ACount);
  for i := 0 to ACount - 1 do
  begin
    FItems[i] := TExeImageSection.Create(AExeImage, APosition);
    Inc(APosition, SizeOf(IMAGE_SECTION_HEADER));
  end;
end;

destructor TExeImageSections.Destroy;
var
  i: Integer;
begin
  for i := 0 to Length(FItems) - 1 do FItems[i].Free;
  SetLength(FItems, 0);
end;

function TExeImageSections.GetItem(Index: DWord): TExeImageSection;
begin
  Result := FItems[Index];
end;

function TExeImageSections.ByName(AName: String): TExeImageSection;
var
  i: Integer;
begin
  for i := 0 to Length(FItems) - 1 do
  if FItems[i].Name = AName then
  begin
    Result := FItems[i];
    Exit;
  end;
  Result := nil;
end;



constructor TExeImageSection.Create(AExeImage: TExeImage; APosition: DWord);
begin
  FExeImage := AExeImage;
  FExeImage.Read(FHeader, APosition, SizeOf(FHeader));
  FName := FHeader.Name;
end;

procedure TExeImageSection.Read(var Buffer);
begin
  FExeImage.Read(Buffer, PhysicalOffset, PhysicalSize);
end;


procedure WriteHead(s: String);
var
  i: Integer;
begin
  WriteLn;
  WriteLn(s);
  for i:=1 to Length(s) do Write('-');
  WriteLn;
end;

procedure Convert(InputFile, OutputFile: String);
var
  ExeImage: TExeImage;
  KosHeader: TKosHeader;
  FileStream: TStream;
  ImageBase, ImageSize, Size: DWord;
  Buffer: Pointer;
  i: Integer;
begin
  ExeImage := TExeImage.Create(InputFile);

{$ifdef debug}
  WriteHead('NT Header');
  WriteLn(Format('Size of Code: %d', [ExeImage.SizeOfCode]));
  WriteLn(Format('Size of Init Data: %d', [ExeImage.SizeOfInitializedData]));
  WriteLn(Format('Size of UnInit Data: %d', [ExeImage.SizeOfUninitializedData]));
  WriteLn(Format('Entry Point: 0x%x', [ExeImage.EntryPoint]));
  WriteLn(Format('Image Base: 0x%x', [ExeImage.ImageBase]));
  WriteLn(Format('Object Align: %d; File Align: %d', [ExeImage.ObjectAlign, ExeImage.FileAlign]));
  WriteLn(Format('Image Size: %d; Header Size: %d', [ExeImage.ImageSize, ExeImage.HeaderSize]));
  WriteLn(Format('Stack Reserve Size: %d; Stack Commit Size: %d', [ExeImage.StackReserveSize, ExeImage.StackCommitSize]));
  WriteLn(Format('Heap Reserve Size: %d; Heap Comit Size: %d', [ExeImage.HeapReserveSize, ExeImage.HeapCommitSize]));
{$endif}

  ImageBase := ExeImage.ImageBase;
  ImageSize := 0;

  { запись секций }
  FileStream := TFileStream.Create(OutputFile, fmCreate);
  for i:=0 to ExeImage.Sections.Count-1 do
  with ExeImage.Sections[i] do
  begin

{$ifdef debug}
    WriteHead(Format('Section %s (0x%x)', [Name, ObjectFlags]));
    WriteLn(Format('Section RVA/Size: 0x%x / %d', [SectionRVA, VirtualSize]));
    WriteLn(Format('Physical Offset/Size: 0x%x / %d', [PhysicalOffset, PhysicalSize]));
{$endif}

    Size := ImageBase + SectionRVA;
    FileStream.Position := Size;
    Inc(Size, VirtualSize);
    if Size > ImageSize then ImageSize := Size;

    if PhysicalSize > 0 then
    begin
      GetMem(Buffer, PhysicalSize);
      Read(Buffer^);
      FileStream.Write(Buffer^, PhysicalSize);
      FreeMem(Buffer);
    end;
  end;

  FillByte(KosHeader, SizeOf(KosHeader), 0);
  with KosHeader do
  begin
    sign    := KOS_SIGN;
    version := 1;
    start   := ImageBase + ExeImage.EntryPoint;
    size    := FileStream.Size;
    args    := ImageSize;
    path    := args + ARGS_SIZE;
    stack   := path + PATH_SIZE + ExeImage.StackReserveSize;
    memory  := stack;
  end;
  FileStream.Position := 0;
  FileStream.Write(KosHeader, SizeOf(KosHeader));
  FileStream.Free();
end;


var
  InputFile, OutputFile: String;
begin
  if ParamCount < 1 then
  begin
    WriteLn(Format('%s <exe input file> [kos output file]', [ExtractFileName(ParamStr(0))]));
    Exit;
  end;

  InputFile := ParamStr(1);
  if ParamCount <2 then
    OutputFile := ChangeFileExt(InputFile, '') else
    OutputFile := ParamStr(2);

  if InputFile = OutputFile then
    WriteLn(Format('Cannot convert the file "%s" onto itself.', [InputFile])) else

  if not FileExists(InputFile) then
    WriteLn(Format('Input the file "%s", not found.', [InputFile])) else

  begin
    WriteLn(Format('Converting "%s" to "%s"...', [InputFile, OutputFile]));
    Convert(InputFile, OutputFile);
  end;
end.
