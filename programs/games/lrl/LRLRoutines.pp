unit LRLRoutines;

{$mode objfpc}
{$asmmode intel}


interface


procedure ImagePut(var Screen, ImageBuffer; X, Y: Integer; Winx1, Winy1, Winx2, Winy2: Word);
procedure ImagePutTransparent(var Screen, ImageBuffer; X, Y: Integer; Winx1, Winy1, Winx2, Winy2: Word);
procedure ImageFill(var ImageBuffer; SizeX, SizeY: Word; Value: Byte);
function  ImageSizeX(var ImageBuffer): Word;
function  ImageSizeY(var ImageBuffer): Word;
procedure ImageStringGet(Source: String; var FontData, Buffer; ColorOffs: Byte);
procedure ScreenApply(var Buffer);
procedure ImageClear(var Buffer);

procedure Palette256Set(var Palette256);
procedure Palette256Get(var Palette256);
procedure Palette256Grayscale(var Palette256; StartElement, EndElement: Byte);
procedure Palette256Darken(var Palette256; StartElement, EndElement, Decrement, MinValue: Byte);
procedure Palette256Transform(var SourcePalette, DestinationPalette);
function DataByteGet(var Buffer; BufferOffset: Word): Byte;
procedure DataBytePut(var Buffer; BufferOffset: Word; Value: Byte);
function DataWordGet(var Buffer; BufferOffset: Word): Word;
procedure DataWordPut(var Buffer; BufferOffset: Word; Value: Word);
procedure DataMove(var Source, Destination; Count: Word; SourceOffset, DestinationOffset: Word);
procedure DataAdd(var Buffer; Count: Word; Amount: Byte; BufferOffset: Word);
procedure DataFill(var Buffer; Count: Word; Value: Byte; BufferOffset: Word);
function DataIdentical(var Array1, Array2; Count: Word; Array1Offset, Array2Offset: Word): Boolean;
function ReadKey: Word;
function Keypressed: Boolean;
function SetInterrupt(Int: Byte; NewAddress: Pointer): Pointer;
procedure FadeClear;
procedure FadeTo(pal: Pointer);
procedure DecompressRepByte(var InArray, OutArray; InArraySize: Word; var OutArraySize: Word);
function MSMouseInArea(x1, y1, x2, y2: Integer): Boolean;
function MSMouseDriverExist: Boolean;
procedure MSMouseGetXY(var x, y: Integer);
function MSMouseButtonStatusGet: Word;
function MSMouseButtonWasPressed(Button: Word; var x, y: Integer): Boolean;
function MSMouseButtonWasReleased(Button: Word; var x, y: Integer): Boolean;
procedure MSMouseSetXY(x, y: Integer);
procedure KeyboardFlush;
function GetInterrupt(Int: Byte): Pointer;

procedure AssignFile(var AFile: File; AFileName: String);
function LastDosTick(): Longword;


implementation


uses
  SysUtils;


const
  SCREEN_WIDTH  = 320;
  SCREEN_HEIGHT = 200;

type
  PRGBColor = ^TRGBColor;
  TRGBColor = packed record
    R, G, B: Byte;
  end;

  PRGBPalette = ^TRGBPalette;
  TRGBPalette = array[Byte] of TRGBColor;

var
  ScreenRGBPalette: TRGBPalette;
  ScreenRGBBuffer : array[0..SCREEN_HEIGHT - 1, 0..SCREEN_WIDTH - 1] of TRGBColor;
  ScreenBuffer    : array[0..SCREEN_WIDTH * SCREEN_HEIGHT - 1] of Byte;

  AlreadyKeyPressed: Boolean = False;


procedure Paint;
begin
  kos_begindraw();
  kos_definewindow(500, 100, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, $01000000);
  kos_drawimage24(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, @ScreenRGBBuffer);
  kos_enddraw();
end;

procedure UpdateRGBBuffer;
var
  I, J: Longint;
  B: PByte;
begin
  B := @ScreenBuffer;
  for I := 0 to SCREEN_HEIGHT - 1 do
  for J := 0 to SCREEN_WIDTH - 1 do
  begin
    ScreenRGBBuffer[I, J] := ScreenRGBPalette[B^];
    Inc(B);
  end;
  Paint;
end;


procedure ImagePut(var Screen, ImageBuffer; X, Y: Integer; WinX1, WinY1, WinX2, WinY2: Word);
var
  Width, Height: Word;
  I, J, K: Integer;
  P: Pointer;
begin
  Width  := PWord(@ImageBuffer)[0];
  Height := PWord(@ImageBuffer)[1];

  P := @ImageBuffer + 4;
  for I := Y to Y + Height - 1 do
  begin
    if (I >= 0) and (I < SCREEN_HEIGHT) and (I >= WinY1) and (I <= WinY2) then
    begin
      if X < WinX1 then
        J := WinX1 - X else
        J := 0;
      K := Width - J;
      if WinX1 + K - 1 > WinX2 then
        K := WinX2 - WinX1 + 1;
      Move((P + J)^, (@Screen + I * SCREEN_WIDTH + X + J)^, K);
    end;
    Inc(P, Width);
  end;
end;

procedure ImagePutTransparent(var Screen, ImageBuffer; X, Y: Integer; Winx1, Winy1, Winx2, Winy2: Word);
begin
  ImagePut(Screen, ImageBuffer, X, Y, Winx1, Winy1, Winx2, Winy2);
end;

procedure ImageFill(var ImageBuffer; SizeX, SizeY: Word; Value: Byte);
begin
  PWord(@ImageBuffer)^     := SizeX;
  PWord(@ImageBuffer + 2)^ := SizeY;
  FillChar((@ImageBuffer + 4)^, SizeX * SizeY, Value);
end;

function ImageSizeX(var ImageBuffer): Word;
begin
  Result := PWord(@ImageBuffer)^;
end;

function ImageSizeY(var ImageBuffer): Word;
begin
  Result := PWord(@ImageBuffer + 2)^;
end;

procedure ImageStringGet(Source: String; var FontData, Buffer; ColorOffs: Byte);
var
  Width, Height: Word;
  Table: PWord;
  P, B: PByte;
  X, I, J, K, C: Word;
begin
  Height := PWord(@FontData + 2)^;
  Table  := PWord(@FontData + 4);

  { расчет длины строки }
  Width := 0;
  for I := 1 to Length(Source) do
  begin
    P := @Table[Ord(Source[I])];
    Inc(Width, PWord(P + PWord(P)^)^);
  end;

  PWord(@Buffer)^     := Width;
  PWord(@Buffer + 2)^ := Height;

  { вывод строки }
  X := 0;
  for I := 1 to Length(Source) do
  begin
    P := @Table[Ord(Source[I])];
    B := PByte(P + PWord(P)^);
    C := PWord(B)^;
    Inc(B, 2);

    P := PByte(@Buffer + 4 + X);
    for K := 0 to Height - 1 do
    begin
      for J := 0 to C - 1 do
      begin
        if B^ = 0 then
          P^ := 0 else
          P^ := B^ + ColorOffs;
        Inc(P);
        Inc(B);
      end;
      Inc(P, Width - C);
    end;

    Inc(X, C);
  end;
end;

procedure ScreenApply(var Buffer);
begin
  Move(Buffer, ScreenBuffer, SizeOf(ScreenBuffer));
  UpdateRGBBuffer;
end;


procedure ImageClear(var Buffer);
begin
  FillChar(Buffer, SCREEN_WIDTH * SCREEN_HEIGHT, 0);
end;


procedure Palette256Set(var Palette256);
var
  I: Longint;
  P: PRGBColor;
begin
  P := @Palette256;
  for I := 0 to 255 do
  with ScreenRGBPalette[I] do
  begin
    R := Round(P^.B / 63 * 255);
    G := Round(P^.G / 63 * 255);
    B := Round(P^.R / 63 * 255);
    Inc(P);
  end;
  UpdateRGBBuffer;
end;

procedure Palette256Get(var Palette256);
var
  I: Longint;
  P: PRGBColor;
begin
  P := @Palette256;
  for I := 0 to 255 do
  with ScreenRGBPalette[I] do
  begin
    P^.R := Round(B / 255 * 63);
    P^.G := Round(G / 255 * 63);
    P^.B := Round(R / 255 * 63);
    Inc(P);
  end;
end;

procedure Palette256Grayscale(var Palette256; StartElement, EndElement: Byte);
begin
end;

procedure Palette256Darken(var Palette256; StartElement, EndElement, Decrement, MinValue: Byte);
begin
end;

procedure Palette256Transform(var SourcePalette, DestinationPalette);
var
  I: Longint;
  S, D: PByte;
begin
  S := @SourcePalette;
  D := @DestinationPalette;
  for I := 0 to 767 do
  begin
    if S^ < D^ then Inc(S^) else
    if S^ > D^ then Dec(S^);
    Inc(S);
    Inc(D);
  end;
end;

function DataByteGet(var Buffer; BufferOffset: Word): Byte;
begin
  Result := PByte(@Buffer + BufferOffset)^;
end;

procedure DataBytePut(var Buffer; BufferOffset: Word; Value: Byte);
begin
  PByte(@Buffer + BufferOffset)^ := Value;
end;

function DataWordGet(var Buffer; BufferOffset: Word): Word;
begin
  Result := PWord(@Buffer + BufferOffset)^;
end;

procedure DataWordPut(var Buffer; BufferOffset: Word; Value: Word);
begin
  PWord(@Buffer + BufferOffset)^ := Value;
end;

procedure DataMove(var Source, Destination; Count: Word; SourceOffset, DestinationOffset: Word);
begin
  Move((@Source + SourceOffset)^, (@Destination + DestinationOffset)^, Count);
end;

procedure DataFill(var Buffer; Count: Word; Value: Byte; BufferOffset: Word);
begin
  FillChar((@Buffer + BufferOffset)^, Count, Value);
end;

function DataIdentical(var Array1, Array2; Count: Word; Array1Offset, Array2Offset: Word): Boolean;
begin
  Result := CompareByte((@Array1 + Array1Offset)^, (@Array2 + Array2Offset)^, Count) = 0;
end;

procedure DataAdd(var Buffer; Count: Word; Amount: Byte; BufferOffset: Word);
var
  I: Word;
begin
  for I := 0 to Count do
    Inc(PByte(@Buffer + BufferOffset + I)^, Amount);
    {if >0 then += amount}
end;

function ReadKey: Word;
var
  Event: Word;
begin
  if not AlreadyKeyPressed then
  begin
    kos_maskevents(ME_PAINT or ME_KEYBOARD);
    repeat
      Event := kos_getevent();
      if Event = SE_PAINT then Paint;
    until Event = SE_KEYBOARD;
  end;
  Result := kos_getkey() shr 8;
  AlreadyKeyPressed := False;
  {WriteLn('ReadKey -> ', IntToHex(Result, 2));}
end;

function Keypressed: Boolean;
begin
  if AlreadyKeyPressed then
    Result := True else
  begin
    kos_maskevents(ME_KEYBOARD);
    Result := kos_getevent(False) = SE_KEYBOARD;
    AlreadyKeyPressed := Result;
  end;
end;

procedure KeyboardFlush;
var
  Event: Word;
begin
  kos_maskevents(ME_KEYBOARD);
  repeat
    Event := kos_getevent(False);
    if Event = SE_KEYBOARD then kos_getkey();
  until Event = 0;
  AlreadyKeyPressed := False;
end;

function SetInterrupt(Int: Byte; NewAddress: Pointer): Pointer;
begin
  Result := nil;
end;

procedure FadeClear;
var
  Pal1, Pal2: Pointer;
  i: Integer;
begin
  GetMem(Pal1, 768);
  GetMem(Pal2, 768);
  Palette256Get(Pal1^);
  for i := 0 to 32 do
  begin
    DataMove(Pal1^, Pal2^, 768, 0, 0);
    Palette256Darken(Pal2^, 0, 255, i * 2, 0);
    Palette256Set(Pal2^);
  end;
  FreeMem(Pal1, 768);
  FreeMem(Pal2, 768);
end;

procedure FadeTo(Pal: Pointer);
var
  Pal1: Pointer;
  I: Integer;
begin
  GetMem(Pal1, 768);
  Palette256Get(Pal1^);
  for I := 0 to 63 do
  begin
    Palette256Transform(Pal1^, Pal^);
    Palette256Set(Pal1^);
    kos_delay(1);
  end;
  FreeMem(Pal1, 768);
end;

procedure DecompressRepByte(var InArray, OutArray; InArraySize: Word; var OutArraySize: Word);
begin
{asm
  PUSH DS

  xor DX,DX
  xor AX,AX

  LDS SI,InArray
  LES DI,OutArray

  MOV CX,InArraySize
  JCXZ @Done

  @Loop1:
  LODSB
  CMP AL,0
  JE @VsePonyatno
  CMP AL,4
  JB @MensheTreh

  INC DX
  STOSB
  JMP @DoLoop

  @MensheTreh:
  SUB CX,1
  MOV BX,CX

  MOV CX,AX
  ADD DX,AX
  LODSB
  REP STOSB

  MOV CX,BX
  JMP @DoLoop

  @VsePonyatno:
  LODSB
  SUB CX,2
  MOV BX,CX
  MOV CX,AX
  ADD DX,AX
  LODSB
  REP STOSB
  MOV CX,BX

  @DoLoop:
  JCXZ @Done
  LOOP @Loop1

  @Done:
  LES DI,OutArraySize
  MOV[ES:DI],DX
  POP DS}
end;

function MSMouseInArea(x1, y1, x2, y2: Integer): Boolean;
begin
  Result := False;
end;

function MSMouseDriverExist: Boolean;
begin
  Result := True;
end;

procedure MSMouseGetXY(var x, y: Integer);
begin
end;

function MSMouseButtonStatusGet: Word;
begin
  Result := 0;
end;

function MSMouseButtonWasPressed(Button: Word; var x, y: Integer): Boolean;
begin
  Result := False;
end;

function MSMouseButtonWasReleased(Button: Word; var x, y: Integer): Boolean;
begin
  Result := False;
end;

procedure MSMouseSetXY(x, y: Integer);
begin
end;

function GetInterrupt(Int: Byte): Pointer;
begin
  Result := nil;
end;

procedure AssignFile(var AFile: File; AFileName: String);
begin
  Assign(AFile, IncludeTrailingPathDelimiter(ExtractFilePath(ParamStr(0))) + AFileName);
end;

function LastDosTick(): Longword;
begin
  Result := Round(kos_timecounter() * 0.182);
end;


end.
