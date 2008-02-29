{$mode delphi}

unit ExeImage;

interface

uses
  TypInfo, Classes, SysUtils, Windows, RXTypes;

const
  MF_END = $80;

type

{ Exceptions }

  EExeError = class(Exception);
  ENotImplemented = class(Exception)
  public
    constructor Create();
  end;

{ Forward Declarations }

  TResourceItem = class;
  TResourceClass = class of TResourceItem;
  TResourceList = class;

{ TExeImage }

  TExeImage = class(TComponent)
  private
    FFileName: string;
    FFileHandle: THandle;
    FFileMapping: THandle;
    FFileBase: Pointer;
    FDosHeader: PIMAGE_DOS_HEADER;
    FNTHeader: PIMAGE_NT_HEADERS;
    FResourceList: TResourceList;
    FIconResources: TResourceItem;
    FCursorResources: TResourceItem;
    FResourceBase: Longint;
    FResourceRVA: Longint;
    function GetResourceList: TResourceList;
    function GetSectionHdr(const SectionName: string;
      var Header: PIMAGE_SECTION_HEADER): Boolean;
  public
    constructor CreateImage(AOwner: TComponent; const AFileName: string);
    destructor Destroy; override;
    property FileName: string read FFileName;
    property Resources: TResourceList read GetResourceList;
  end;

{ TResourceItem }

  TResourceItem = class(TComponent)
  private
    FList: TResourceList;
    FDirEntry: PIMAGE_RESOURCE_DIRECTORY_ENTRY;
    function DataEntry: PIMAGE_RESOURCE_DATA_ENTRY;
    function FExeImage: TExeImage;
    function FirstChildDirEntry: PIMAGE_RESOURCE_DIRECTORY_ENTRY;
    function GetResourceItem(Index: Integer): TResourceItem;
    function GetResourceType: TResourceType;
  protected
    function GetName: string; virtual;
    function GetResourceList: TResourceList; virtual;
  public
    constructor CreateItem(AOwner: TComponent; ADirEntry: Pointer);
    function IsList: Boolean; virtual;
    function Offset: Integer;
    function Size: Integer;
    function RawData: Pointer;
    function ResTypeStr: string;
    procedure SaveToFile(const FileName: string);
    procedure SaveToStream(Stream: TStream); virtual;
    property Items[Index: Integer]: TResourceItem read GetResourceItem; default;
    property List: TResourceList read GetResourceList;
    property Name: string read GetName;
    property ResType: TResourceType read GetResourceType;
  end;

{ TIconResource }

  TIconResource = class(TResourceItem)
  protected
    function GetResourceList: TResourceList; override;
  public
    function IsList: Boolean; override;
  end;

{ TIconResEntry }

  TIconResEntry = class(TResourceItem)
  protected
    FResInfo: PIconResInfo;
    function GetName: string; override;
    procedure AssignTo(Dest: TPersistent); override;
  public
    procedure SaveToStream(Stream: TStream); override;
  end;

{ TCursorResource }

  TCursorResource = class(TIconResource)
  protected
    function GetResourceList: TResourceList; override;
  end;

{ TCursorResEntry }

  TCursorResEntry = class(TIconResEntry)
  protected
    FResInfo: PCursorResInfo;
    function GetName: string; override;
  end;

{ TBitmapResource }

  TBitmapResource = class(TResourceItem)
  protected
    procedure AssignTo(Dest: TPersistent); override;
  public
    procedure SaveToStream(Stream: TStream); override;
  end;

{ TStringResource }

  TStringResource = class(TResourceItem)
  protected
    procedure AssignTo(Dest: TPersistent); override;
  end;

{ TMenuResource }

  TMenuResource = class(TResourceItem)
  private
    FNestStr: string;
    FNestLevel: Integer;
    procedure SetNestLevel(Value: Integer);
  protected
    procedure AssignTo(Dest: TPersistent); override;
    property NestLevel: Integer read FNestLevel write SetNestLevel;
    property NestStr: string read FNestStr;
  end;

{ TResourceList }

  TResourceList = class(TComponent)
  protected
    FList: TList;
    FResDir: PIMAGE_RESOURCE_DIRECTORY;
    FExeImage: TExeImage;
    FResType: Integer;
    function List: TList; virtual;
    function GetResourceItem(Index: Integer): TResourceItem;
  public
    constructor CreateList(AOwner: TComponent; ResDirOfs: Longint;
      AExeImage: TExeImage);
    destructor Destroy; override;
    function Count: Integer;
    property Items[Index: Integer]: TResourceItem read GetResourceItem; default;
  end;

{ TIconResourceList }

  TIconResourceList = class(TResourceList)
  protected
    function List: TList; override;
  end;

{ TCursorResourceList }

  TCursorResourceList = class(TResourceList)
  protected
    function List: TList; override;
  end;

implementation

constructor ENotImplemented.Create();
begin
  inherited Create('Not Implemented');
end;


{ This function maps a resource type to the associated resource class }

function GetResourceClass(ResType: Integer): TResourceClass;
const
  TResourceClasses: array[TResourceType] of TResourceClass = (
    TResourceItem,      { rtUnknown0 }
    TCursorResEntry,    { rtCursorEntry }
    TBitmapResource,    { rtBitmap }
    TIconResEntry,      { rtIconEntry }
    TMenuResource,      { rtMenu }
    TResourceItem,      { rtDialog }
    TStringResource,    { rtString }
    TResourceItem,      { rtFontDir }
    TResourceItem,      { rtFont }
    TResourceItem,      { rtAccelerators }
    TResourceItem,      { rtRCData }
    TResourceItem,      { rtMessageTable }
    TCursorResource,    { rtGroupCursor }
    TResourceItem,      { rtUnknown13 }
    TIconResource,      { rtIcon }
    TResourceItem,      { rtUnknown15 }
    TResourceItem);     { rtVersion }
begin
  if (ResType >= Integer(Low(TResourceType))) and
    (ResType <= Integer(High(TResourceType))) then
    Result := TResourceClasses[TResourceType(ResType)] else
    Result := TResourceItem;
end;

{ Utility Functions }

function Min(A, B: Integer): Integer;
begin
  if A < B then Result := A
  else Result := B;
end;

{ This function checks if an offset is a string name, or a directory }
{Assumes: IMAGE_RESOURCE_NAME_IS_STRING = IMAGE_RESOURCE_DATA_IS_DIRECTORY}

function HighBitSet(L: Longint): Boolean;
begin
  Result := (L and IMAGE_RESOURCE_DATA_IS_DIRECTORY) <> 0;
end;

function StripHighBit(L: Longint): Longint;
begin
  Result := L and IMAGE_OFFSET_STRIP_HIGH;
end;

function StripHighPtr(L: Longint): Pointer;
begin
  Result := Pointer(L and IMAGE_OFFSET_STRIP_HIGH);
end;

{ This function converts a pointer to a wide char string into a pascal string }

function WideCharToStr(WStr: PWChar; Len: Integer): string;
begin
  if Len = 0 then Len := -1;
  Len := WideCharToMultiByte(CP_ACP, 0, WStr, Len, nil, 0, nil, nil);
  SetLength(Result, Len);
  WideCharToMultiByte(CP_ACP, 0, WStr, Len, PChar(Result), Len, nil, nil);
end;

{ Exceptions }

procedure ExeError(const ErrMsg: string);
begin
  raise EExeError.Create(ErrMsg);
end;

{ TExeImage }

constructor TExeImage.CreateImage(AOwner: TComponent; const AFileName: string);
begin
  inherited Create(AOwner);
  FFileName := AFileName;
  FFileHandle := CreateFile(PChar(FFileName), GENERIC_READ, FILE_SHARE_READ,
    nil, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
  if FFileHandle = INVALID_HANDLE_VALUE then ExeError('Couldn''t open: '+FFileName);
    FFileMapping := CreateFileMapping(FFileHandle, nil, PAGE_READONLY, 0, 0, nil);
  if FFileMapping = 0 then ExeError('CreateFileMapping failed');
    FFileBase := MapViewOfFile(FFileMapping, FILE_MAP_READ, 0, 0, 0);
  if FFileBase = nil then ExeError('MapViewOfFile failed');
    FDosHeader := PIMAGE_DOS_HEADER(FFileBase);
  if not FDosHeader.e_magic = IMAGE_DOS_SIGNATURE then
    ExeError('unrecognized file format');
  FNTHeader := PIMAGE_NT_HEADERS(Longint(FDosHeader) + FDosHeader.e_lfanew);
  if IsBadReadPtr(FNTHeader, sizeof(IMAGE_NT_HEADERS)) or
     (FNTHeader.Signature <> IMAGE_NT_SIGNATURE) then
    ExeError('Not a PE (WIN32 Executable) file');
 end;

destructor TExeImage.Destroy;
begin
  if FFileHandle <> INVALID_HANDLE_VALUE then
  begin
    UnmapViewOfFile(FFileBase);
    CloseHandle(FFileMapping);
    CloseHandle(FFileHandle);
  end;
  inherited Destroy;
end;

function TExeImage.GetSectionHdr(const SectionName: string;
  var Header: PIMAGE_SECTION_HEADER): Boolean;
var
  I: Integer;
begin
  Header := PIMAGE_SECTION_HEADER(FNTHeader);
  Inc(PIMAGE_NT_HEADERS(Header));
  Result := True;
  for I := 0 to FNTHeader.FileHeader.NumberOfSections - 1 do
  begin
    if Strlicomp(Header.Name, PChar(SectionName), IMAGE_SIZEOF_SHORT_NAME) = 0 then Exit;
    Inc(Header);
  end;
  Result := False;
end;

function TExeImage.GetResourceList: TResourceList;
var
  ResSectHdr: PIMAGE_SECTION_HEADER;
begin
  if not Assigned(FResourceList) then
  begin
    if GetSectionHdr('.rsrc', ResSectHdr) then
    begin
      FResourceBase := ResSectHdr.PointerToRawData + LongWord(FDosHeader);
      FResourceRVA := ResSectHdr.VirtualAddress;
      FResourceList := TResourceList.CreateList(Self, FResourceBase, Self);
    end
    else
      ExeError('No resources in this file.');
  end;
  Result := FResourceList;
end;

{ TResourceItem }

constructor TResourceItem.CreateItem(AOwner: TComponent; ADirEntry: Pointer);
begin
  inherited Create(AOwner);
  FDirEntry := ADirEntry;
end;

function TResourceItem.DataEntry: PIMAGE_RESOURCE_DATA_ENTRY;
begin
  Result := PIMAGE_RESOURCE_DATA_ENTRY(FirstChildDirEntry.OffsetToData
    + Cardinal(FExeImage.FResourceBase));
end;

function TResourceItem.FirstChildDirEntry: PIMAGE_RESOURCE_DIRECTORY_ENTRY;
begin
  Result := PIMAGE_RESOURCE_DIRECTORY_ENTRY(StripHighBit(FDirEntry.OffsetToData) +
    FExeImage.FResourceBase + SizeOf(IMAGE_RESOURCE_DIRECTORY));
end;

function TResourceItem.FExeImage: TExeImage;
begin
  Result := (Owner as TResourceList).FExeImage;
end;

function TResourceItem.GetResourceItem(Index: Integer): TResourceItem;
begin
  Result := List[Index];
end;

function TResourceItem.GetResourceType: TResourceType;
begin
  Result := TResourceType((Owner as TResourceList).FResType);
end;

function TResourceItem.IsList: Boolean;
begin
  Result := HighBitSet(FirstChildDirEntry.OffsetToData);
end;

function TResourceItem.GetResourceList: TResourceList;
begin
  if not IsList then ExeError('ResourceItem is not a list');
  if not Assigned(FList) then
    FList := TResourceList.CreateList(Self, StripHighBit(FDirEntry.OffsetToData) +
      FExeImage.FResourceBase, FExeImage);
  Result := FList;
end;

function TResourceItem.GetName: string;
var
  PDirStr: PIMAGE_RESOURCE_DIR_STRING_U;
begin
  { Check for Level1 entries, these are resource types. }
  if (Owner.Owner = FExeImage) and not HighBitSet(FDirEntry.Name) and
    (FDirEntry.Name <= 16) then
  begin
    Result := Copy(GetEnumName(TypeInfo(TResourceType), FDirEntry.Name), 3, 20);
    Exit;
  end;

  if HighBitSet(FDirEntry.Name) then
  begin
    PDirStr := PIMAGE_RESOURCE_DIR_STRING_U(StripHighBit(FDirEntry.Name) +
      FExeImage.FResourceBase);
    Result := WideCharToStr(@PDirStr.NameString, PDirStr.Length);
    Exit;
  end;
  Result := Format('%d', [FDirEntry.Name]);
end;

function TResourceItem.Offset: Integer;
begin
  if IsList then
    Result := StripHighBit(FDirEntry.OffsetToData)
  else
    Result := DataEntry.OffsetToData;
end;

function TResourceItem.RawData: Pointer;
begin
  with FExeImage do
    Result := pointer(FResourceBase - FResourceRVA + LongInt(DataEntry.OffsetToData));
end;

function TResourceItem.ResTypeStr: string;
begin
  Result := Copy(GetEnumName(TypeInfo(TResourceType), Ord(ResType)), 3, 20);
end;

procedure TResourceItem.SaveToFile(const FileName: string);
var
  FS: TFileStream;
begin
  FS := TFileStream.Create(FileName, fmCreate);
  try
    Self.SaveToStream(FS);
  finally
    FS.Free;
  end;
end;

procedure TResourceItem.SaveToStream(Stream: TStream);
begin
  Stream.Write(RawData^, Size);
end;

function TResourceItem.Size: Integer;
begin
  if IsList then
    Result := 0
  else
    Result := DataEntry.Size;
end;

{ TBitmapResource }

procedure TBitmapResource.AssignTo(Dest: TPersistent);
{var
  MemStr: TMemoryStream;
  BitMap: TBitMap;}
begin
  raise ENotImplemented.Create();

  {if (Dest is TPicture) then
  begin
    BitMap := TPicture(Dest).Bitmap;
    MemStr := TMemoryStream.Create;
    try
      SaveToStream(MemStr);
      MemStr.Seek(0,0);
      BitMap.LoadFromStream(MemStr);
    finally
      MemStr.Free;
    end
  end
  else
    inherited AssignTo(Dest);}
end;

procedure TBitmapResource.SaveToStream(Stream: TStream);

  {function GetDInColors(BitCount: Word): Integer;
  begin
    case BitCount of
      1, 4, 8: Result := 1 shl BitCount;
    else
      Result := 0;
    end;
  end;

var
  BH: TBitmapFileHeader;
  BI: PBitmapInfoHeader;
  BC: PBitmapCoreHeader;
  ClrUsed: Integer;}
begin
  raise ENotImplemented.Create();

  {FillChar(BH, sizeof(BH), #0);
  BH.bfType := $4D42;
  BH.bfSize := Self.Size + sizeof(BH);
  BI := PBitmapInfoHeader(RawData);
  if BI.biSize = sizeof(TBitmapInfoHeader) then
  begin
    ClrUsed := BI.biClrUsed;
    if ClrUsed = 0 then
      ClrUsed := GetDInColors(BI.biBitCount);
    BH.bfOffBits :=  ClrUsed * SizeOf(TRgbQuad) +
      sizeof(TBitmapInfoHeader) + sizeof(BH);
  end
  else
  begin
    BC := PBitmapCoreHeader(RawData);
    ClrUsed := GetDInColors(BC.bcBitCount);
    BH.bfOffBits :=  ClrUsed * SizeOf(TRGBTriple) +
      sizeof(TBitmapCoreHeader) + sizeof(BH);
  end;
  Stream.Write(BH, SizeOf(BH));
  Stream.Write(RawData^, Self.Size);}
end;


{ TIconResource }

function TIconResource.GetResourceList: TResourceList;
begin
  if not Assigned(FList) then
    FList := TIconResourceList.CreateList(Owner, LongInt(RawData), FExeImage);
  Result := FList;
end;

function TIconResource.IsList: Boolean;
begin
  Result := True;
end;

{ TIconResEntry }

procedure TIconResEntry.AssignTo(Dest: TPersistent);
{var
  hIco: HIcon;}
begin
  raise ENotImplemented.Create();

  {if Dest is TPicture then
  begin
    hIco := CreateIconFromResource(RawData, Size, (ResType = rtIconEntry), $30000);
    TPicture(Dest).Icon.Handle := hIco;
  end
  else
    inherited AssignTo(Dest);}
end;

function TIconResEntry.GetName: string;
begin
  if Assigned(FResInfo) then
    with FResInfo^ do
      Result := Format('%d X %d %d Colors', [bWidth, bHeight, bColorCount])
  else
    Result := inherited GetName;
end;

procedure TIconResEntry.SaveToStream(Stream: TStream);
begin
  raise ENotImplemented.Create();

  {with TIcon.Create do
  try
    Handle := CreateIconFromResource(RawData, Self.Size, (ResType <> rtIcon), $30000);
    SaveToStream(Stream);
  finally
    Free;
  end;}
end;

{ TCursorResource }

function TCursorResource.GetResourceList: TResourceList;
begin
  if not Assigned(FList) then
    FList := TCursorResourceList.CreateList(Owner, LongInt(RawData), FExeImage);
  Result := FList;
end;

{ TCursorResEntry }

function TCursorResEntry.GetName: string;
begin
  if Assigned(FResInfo) then
    with FResInfo^ do
      Result := Format('%d X %d %d Bit(s)', [wWidth, wWidth, wBitCount])
  else
    Result := inherited GetName;
end;

{ TStringResource }

procedure TStringResource.AssignTo(Dest: TPersistent);
var
  P: PWChar;
  ID: Integer;
  Cnt: Cardinal;
  Len: Word;
begin
  if (Dest is TStrings) then
    with TStrings(Dest) do
    begin
      BeginUpdate;
      try
        Clear;
        P := RawData;
        Cnt := 0;
        while Cnt < StringsPerBlock do
        begin
          Len := Word(P^);
          if Len > 0 then
          begin
            Inc(P);
            ID := ((FDirEntry.Name - 1) shl 4) + Cnt;
            Add(Format('%d,  "%s"', [ID, WideCharToStr(P, Len)]));
            Inc(P, Len);
          end;
          Inc(Cnt);
        end;
      finally
        EndUpdate;
      end;
    end
  else
    inherited AssignTo(Dest);
end;

{ TMenuResource }

procedure TMenuResource.SetNestLevel(Value: Integer);
begin
  FNestLevel := Value;
  SetLength(FNestStr, Value * 2);
  FillChar(FNestStr[1], Value * 2, ' ');
end;

procedure TMenuResource.AssignTo(Dest: TPersistent);
var
  IsPopup: Boolean;
  Len: Word;
  MenuData: PWord;
  MenuEnd: PChar;
  MenuText: PWChar;
  MenuID: Word;
  MenuFlags: Word;
  S: string;
begin
  if (Dest is TStrings) then
    with TStrings(Dest) do
    begin
      BeginUpdate;
      try
        Clear;
        MenuData := RawData;
        MenuEnd := PChar(RawData) + Size;
        Inc(MenuData, 2);
        NestLevel := 0;
        while PChar(MenuData) < MenuEnd do
        begin
          MenuFlags := MenuData^;
          Inc(MenuData);
          IsPopup := (MenuFlags and MF_POPUP) = MF_POPUP;
          MenuID := 0;
          if not IsPopup then
          begin
            MenuID := MenuData^;
            Inc(MenuData);
          end;
          MenuText := PWChar(MenuData);
          Len := lstrlenw(MenuText);
          if Len = 0 then
            S := 'MENUITEM SEPARATOR'
          else
          begin
            S := WideCharToStr(MenuText, Len);
            if IsPopup then
              S := Format('POPUP "%s"', [S]) else
              S := Format('MENUITEM "%s",  %d', [S, MenuID]);
          end;
          Inc(MenuData, Len + 1);
          Add(NestStr + S);
          if (MenuFlags and MF_END) = MF_END then
          begin
            NestLevel := NestLevel - 1;
            Add(NestStr + 'ENDPOPUP');
          end;
          if IsPopup then
            NestLevel := NestLevel + 1;
        end;
      finally
        EndUpdate;
      end;
    end
  else
    inherited AssignTo(Dest);
end;

{ TResourceList }

constructor TResourceList.CreateList(AOwner: TComponent; ResDirOfs: Longint;
  AExeImage: TExeImage);
var
  DirEntry: PIMAGE_RESOURCE_DIRECTORY_ENTRY;
begin
  inherited Create(AOwner);
  FExeImage := AExeImage;
  FResDir := Pointer(ResDirOfs);
  if AOwner <> AExeImage then
    if AOwner.Owner.Owner = AExeImage then
    begin
      DirEntry := PIMAGE_RESOURCE_DIRECTORY_ENTRY(FResDir);
      inc(PIMAGE_RESOURCE_DIRECTORY(DirEntry));
      FResType := TResourceItem(Owner).FDirEntry.Name;
    end
    else
      FResType := (AOwner.Owner.Owner as TResourceList).FResType;
end;

destructor TResourceList.Destroy;
begin
  inherited Destroy;
  FList.Free;
end;

function TResourceList.List: TList;
var
  I: Integer;
  DirEntry: PIMAGE_RESOURCE_DIRECTORY_ENTRY;
  DirCnt: Integer;
  ResItem: TResourceItem;
begin
  if not Assigned(FList) then
  begin
    FList := TList.Create;
    DirEntry := PIMAGE_RESOURCE_DIRECTORY_ENTRY(FResDir);
    inc(PIMAGE_RESOURCE_DIRECTORY(DirEntry));
    DirCnt := FResDir.NumberOfNamedEntries + FResDir.NumberOfIdEntries - 1;
    for I := 0 to DirCnt do
    begin
      { Handle Cursors and Icons specially }
      ResItem := GetResourceClass(FResType).CreateItem(Self, DirEntry);
      if Owner = FExeImage then
        if (TResourceType(DirEntry.Name) in [rtCursorEntry, rtIconEntry]) then
        begin
          if TResourceType(DirEntry.Name) = rtCursorEntry then
            FExeImage.FCursorResources := ResItem else
            FExeImage.FIconResources := ResItem;
          Inc(DirEntry);
          Continue;
        end;
      FList.Add(ResItem);
      Inc(DirEntry);
    end;
  end;
  Result := FList;
end;

function TResourceList.Count: Integer;
begin
  Result := List.Count;
end;

function TResourceList.GetResourceItem(Index: Integer): TResourceItem;
begin
  Result := List[Index];
end;

{ TIconResourceList }

function TIconResourceList.List: TList;
var
  I,  J, Cnt: Integer;
  ResData: PIconResInfo;
  ResList: TResourceList;
  ResOrd: Cardinal;
  IconResource: TIconResEntry;
begin
  if not Assigned(FList) then
  begin
    FList := TList.Create;
    Cnt := PIconHeader(FResDir).wCount;
    PChar(ResData) := PChar(FResDir) + SizeOf(TIconHeader);
    ResList := FExeImage.FIconResources.List;
    for I := 0 to Cnt - 1 do
    begin
      ResOrd := ResData.wNameOrdinal;
      for J := 0 to ResList.Count - 1 do
      begin
        if ResOrd = ResList[J].FDirEntry.Name then
        begin
          IconResource := ResList[J] as TIconResEntry;
          IconResource.FResInfo := ResData;
          FList.Add(IconResource);
        end;
      end;
      Inc(ResData);
    end;
  end;
  Result := FList;
end;

{ TCursorResourceList }

function TCursorResourceList.List: TList;
var
  I, J, Cnt: Integer;
  ResData: PCursorResInfo;
  ResList: TResourceList;
  ResOrd: Cardinal;
  CursorResource: TCursorResEntry;
begin
  if not Assigned(FList) then
  begin
    FList := TList.Create;
    Cnt := PIconHeader(FResDir).wCount;
    PChar(ResData) := PChar(FResDir) + SizeOf(TIconHeader);
    ResList := FExeImage.FCursorResources.List;
    for I := 0 to Cnt - 1 do
    begin
      ResOrd := ResData.wNameOrdinal;
      for J := 0 to ResList.Count - 1 do
      begin
        if ResOrd = ResList[J].FDirEntry.Name then
        begin
          CursorResource := ResList[J] as TCursorResEntry;
          CursorResource.FResInfo := ResData;
          FList.Add(CursorResource);
        end;
      end;
      Inc(ResData);
    end;
  end;
  Result := FList;
end;

end.
