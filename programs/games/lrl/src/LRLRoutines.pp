unit LRLRoutines;

{$mode objfpc}
{$asmmode intel}


interface


procedure ImagePut(var Screen, ImageBuffer; X, Y, WinX1, WinY1, WinX2, WinY2: Integer);
procedure ImagePutTransparent(var Screen, ImageBuffer; X, Y, WinX1, WinY1, WinX2, WinY2: Integer);
procedure ImageFill(var ImageBuffer; SizeX, SizeY: Word; Value: Byte);
function  ImageSizeX(var ImageBuffer): Word;
function  ImageSizeY(var ImageBuffer): Word;
procedure ImageStringGet(Source: String; var FontData, Buffer; ColorOffs: Byte);
procedure ScreenApply(var Buffer);
procedure ImageClear(var Buffer);
procedure ScreenMode(Mode: Integer);

function ScanToChar(Code: Word): Char;
procedure KeyboardInitialize;
function Keypressed: Boolean;
function ReadKey: Word;
procedure KeyboardFlush;

procedure MouseInitialize;
function MSMouseInArea(x1, y1, x2, y2: Integer): Boolean;
function MSMouseDriverExist: Boolean;
procedure MSMouseGetXY(var x, y: Integer);
function MSMouseButtonStatusGet: Word;
function MSMouseButtonWasPressed(Button: Word; var x, y: Integer): Boolean;
function MSMouseButtonWasReleased(Button: Word; var x, y: Integer): Boolean;
procedure MSMouseSetXY(x, y: Integer);

procedure Palette256Set(var Palette256);
procedure Palette256Get(var Palette256);
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
function SetInterrupt(Int: Byte; NewAddress: Pointer): Pointer;
procedure FadeClear;
procedure FadeTo(pal: Pointer);
procedure DecompressRepByte(var InArray, OutArray; InArraySize: Word; var OutArraySize: Word);

function GetInterrupt(Int: Byte): Pointer;
procedure WaitForEvent(Timeout: DWord = 0);
procedure AssignFile(var AFile: File; AFileName: String);
function LastDosTick(): Longword;


const
  KEY_GREY    = $E000;
  KEY_UP_BASE = $8000;
  KEY_ESC   = $0100;
  KEY_1     = $0200;
  KEY_2     = $0300;
  KEY_3     = $0400;
  KEY_4     = $0500;
  KEY_5     = $0600;
  KEY_6     = $0700;
  KEY_7     = $0800;
  KEY_8     = $0900;
  KEY_9     = $0A00;
  KEY_0     = $0B00;
  KEY_SUBTRACT = $0C00;
  KEY_ADD      = $0D00;
  KEY_BACK  = $0E00;

  KEY_Q     = $1000;
  KEY_W     = $1100;
  KEY_E     = $1200;
  KEY_R     = $1300;
  KEY_T     = $1400;
  KEY_Y     = $1500;
  KEY_U     = $1600;
  KEY_I     = $1700;
  KEY_O     = $1800;
  KEY_P     = $1900;
  KEY_LBRACKET = $1A00;
  KEY_RBRACKET = $1B00;
  KEY_ENTER = $1C00;

  KEY_A     = $1E00;
  KEY_S     = $1F00;
  KEY_D     = $2000;
  KEY_F     = $2100;
  KEY_G     = $2200;
  KEY_H     = $2300;
  KEY_J     = $2400;
  KEY_K     = $2500;
  KEY_L     = $2600;
  KEY_SEMICOLON = $2700;
  KEY_QUOTE     = $2800;

  KEY_LSHIFT = $2A00;
  KEY_Z     = $2C00;
  KEY_X     = $2D00;
  KEY_C     = $2E00;
  KEY_V     = $2F00;
  KEY_B     = $3000;
  KEY_N     = $3100;
  KEY_M     = $3200;
  KEY_COMMA = $3300;
  KEY_DECIMAL = $3400;
  KEY_DIVIDE  = $3500;
  KEY_RSHIFT  = $3600;

  KEY_ALT   = $3800;
  KEY_CAPITAL = $3600;
  KEY_F1    = $3B00;
  KEY_UP    = $4800;
  KEY_LEFT  = $4B00;
  KEY_GREY5 = $4C00;
  KEY_RIGHT = $4D00;
  KEY_END   = $4F00;
  KEY_DOWN  = $5000;
  KEY_PGDN  = $5100;

type
  ScanToCharRecord = record
    Scan: Word;
    CL: Char;
    CU: Char;
    Caps: Boolean;
  end;

var
  ScreenTitle: PChar = nil;
  ScanToCharTable: array[1..45] of ScanToCharRecord = (
    (Scan: KEY_0; CL: '0'; CU: ')'; Caps: False), (Scan: KEY_1; CL: '1'; CU: '!'; Caps: False),
    (Scan: KEY_2; CL: '2'; CU: '@'; Caps: False), (Scan: KEY_3; CL: '3'; CU: '#'; Caps: False),
    (Scan: KEY_4; CL: '4'; CU: '$'; Caps: False), (Scan: KEY_5; CL: '5'; CU: '%'; Caps: False),
    (Scan: KEY_6; CL: '6'; CU: '^'; Caps: False), (Scan: KEY_7; CL: '7'; CU: '&'; Caps: False),
    (Scan: KEY_8; CL: '8'; CU: '*'; Caps: False), (Scan: KEY_9; CL: '9'; CU: '('; Caps: False),
    (Scan: KEY_SUBTRACT; CL: '-'; CU: '_'; Caps: False), (Scan: KEY_ADD; CL: '='; CU: '+'; Caps: False),

    (Scan: KEY_Q; CL: 'q'; CU: 'Q'; Caps: True), (Scan: KEY_W; CL: 'w'; CU: 'W'; Caps: True),
    (Scan: KEY_E; CL: 'e'; CU: 'E'; Caps: True), (Scan: KEY_R; CL: 'r'; CU: 'R'; Caps: True),
    (Scan: KEY_T; CL: 't'; CU: 'T'; Caps: True), (Scan: KEY_Y; CL: 'y'; CU: 'Y'; Caps: True),
    (Scan: KEY_U; CL: 'u'; CU: 'U'; Caps: True), (Scan: KEY_I; CL: 'i'; CU: 'I'; Caps: True),
    (Scan: KEY_O; CL: 'o'; CU: 'O'; Caps: True), (Scan: KEY_P; CL: 'p'; CU: 'P'; Caps: True),
    (Scan: KEY_LBRACKET; CL: '['; CU: '{'; Caps: False), (Scan: KEY_RBRACKET; CL: ']'; CU: '}'; Caps: False),

    (Scan: KEY_A; CL: 'a'; CU: 'A'; Caps: True), (Scan: KEY_S; CL: 's'; CU: 'S'; Caps: True),
    (Scan: KEY_D; CL: 'd'; CU: 'D'; Caps: True), (Scan: KEY_F; CL: 'f'; CU: 'F'; Caps: True),
    (Scan: KEY_G; CL: 'g'; CU: 'G'; Caps: True), (Scan: KEY_H; CL: 'h'; CU: 'H'; Caps: True),
    (Scan: KEY_J; CL: 'j'; CU: 'J'; Caps: True), (Scan: KEY_K; CL: 'k'; CU: 'K'; Caps: True),
    (Scan: KEY_L; CL: 'l'; CU: 'L'; Caps: True),
    (Scan: KEY_SEMICOLON; CL: ';'; CU: ':'; Caps: False), (Scan: KEY_QUOTE; CL: ''''; CU: '"'; Caps: False),

    (Scan: KEY_Z; CL: 'z'; CU: 'Z'; Caps: True), (Scan: KEY_X; CL: 'x'; CU: 'X'; Caps: True),
    (Scan: KEY_C; CL: 'c'; CU: 'C'; Caps: True), (Scan: KEY_V; CL: 'v'; CU: 'V'; Caps: True),
    (Scan: KEY_B; CL: 'b'; CU: 'B'; Caps: True), (Scan: KEY_N; CL: 'n'; CU: 'N'; Caps: True),
    (Scan: KEY_M; CL: 'm'; CU: 'M'; Caps: True),
    (Scan: KEY_COMMA; CL: ','; CU: '<'; Caps: False), (Scan: KEY_DECIMAL; CL: '.'; CU: '>'; Caps: False),
    (Scan: KEY_DIVIDE; CL: '/'; CU: '?'; Caps: False)
    );


implementation


uses
  SysUtils;


const
  BUFFER_WIDTH  = 320;
  BUFFER_HEIGHT = 200;

type
  PRGBColor = ^TRGBColor;
  TRGBColor = packed record
    R, G, B: Byte;
  end;

  PRGBPalette = ^TRGBPalette;
  TRGBPalette = array[Byte] of TRGBColor;


var
  ScreenRGBPalette: TRGBPalette;
  ScreenRGBBuffer : PRGBColor = nil;
  ScreenRGBTemporary: PRGBColor = nil;
  ScreenPalBuffer : array[0..BUFFER_HEIGHT - 1, 0..BUFFER_WIDTH - 1] of Byte;

  WindowWidth : Longint;
  WindowHeight: Longint;
  ScreenWidth : Longword;
  ScreenHeight: Longword;
  CurrentScreenMode: Integer = 0;

  LastKeyEvent: Word = $FFFF;
  LastKeyUp  : Boolean = True;
  LastKeyDown: Boolean = False;
  AltDown    : Boolean = False;
  ShiftDown  : Boolean = False;
  LShiftDown : Boolean = False;
  RShiftDown : Boolean = False;
  CapsPressed: Boolean = False;



procedure Paint;
begin
  kos_begindraw();
  kos_definewindow(10, 10, 100, 100, $64000000);
  if CurrentScreenMode <> 0	then
  begin
    kos_setcaption(ScreenTitle);
    if Assigned(ScreenRGBBuffer) then
      kos_drawimage24(0, 0, ScreenWidth, ScreenHeight, ScreenRGBBuffer) else
      kos_drawrect(0, 0, ScreenWidth, ScreenHeight, $FF00FF);
  end;
  kos_enddraw();
end;


procedure UpdateRGBBuffer;
var
  XStep, YStep: Longword;

  procedure Horizontal;
  var
    X, Y, I: Longword;
    B: PByte;
    C: PRGBColor;
  begin
    C := ScreenRGBTemporary;
    for Y := 0 to BUFFER_HEIGHT - 1 do
    begin
      I := 0;
      B := @ScreenPalBuffer[Y, 0];
      for X := 0 to ScreenWidth - 1 do
      begin
        C^ := ScreenRGBPalette[PByte(B + (I shr 16))^];
        Inc(I, XStep);
        Inc(C);
      end;
    end;
  end;

  procedure Vertical;
  var
    Y, I: Longword;
    S: PRGBColor;
    C: PRGBColor;
  begin
    I := 0;
    S := ScreenRGBTemporary;
    C := ScreenRGBBuffer;
    for Y := 0 to ScreenHeight - 1 do
    begin
      Move(PRGBColor(S + (I shr 16) * ScreenWidth)^, C^, ScreenWidth * SizeOf(C^));
      Inc(I, YStep);
      Inc(C, ScreenWidth);
    end;
  end;

var
  I, J: Longint;
  B: PByte;
  C: PRGBColor;

begin
  if (ScreenWidth = BUFFER_WIDTH) and (ScreenHeight = BUFFER_HEIGHT) then
  begin
    {перенос один в один}
    B := @ScreenPalBuffer;
    C := ScreenRGBBuffer;
    for I := 0 to BUFFER_HEIGHT - 1 do
    for J := 0 to BUFFER_WIDTH - 1 do
    begin
      C^ := ScreenRGBPalette[B^];
      Inc(B);
      Inc(C);
    end;
  end else
  begin
    {масштабирование}
    XStep := (BUFFER_WIDTH shl 16) div ScreenWidth;
    YStep := (BUFFER_HEIGHT shl 16) div ScreenHeight;
    Horizontal;
    Vertical;
  end;

  Paint;
end;


procedure ImagePut(var Screen, ImageBuffer; X, Y, WinX1, WinY1, WinX2, WinY2: Integer);
var
  Width, Height: Longint;
  I, J, K: Integer;
  P: Pointer;
begin
  Width  := PWord(@ImageBuffer)[0];
  Height := PWord(@ImageBuffer)[1];

  P := @ImageBuffer + 4;
  for I := Y to Y + Height - 1 do
  begin
    if (I >= 0) and (I < BUFFER_HEIGHT) and (I >= WinY1) and (I <= WinY2) then
    begin
      if X < WinX1 then
        J := WinX1 - X else
        J := 0;
      if X + Width - 1 > WinX2 then
        K := WinX2 - X - J + 1 else
        K := Width - J;
      Move((P + J)^, (@Screen + I * BUFFER_WIDTH + X + J)^, K);
    end;
    Inc(P, Width);
  end;
end;


procedure ImagePutTransparent(var Screen, ImageBuffer; X, Y, WinX1, WinY1, WinX2, WinY2: Integer);
var
  Width, Height: Longint;
  I, J, K, L: Integer;
  PI, PO: PByte;
begin
  Width  := PWord(@ImageBuffer)[0];
  Height := PWord(@ImageBuffer)[1];
  
  PI := @ImageBuffer + 4;

  for I := Y to Y + Height - 1 do
  begin
    if (I >= 0) and (I < BUFFER_HEIGHT) and (I >= WinY1) and (I <= WinY2) then
    begin
      if X < WinX1 then
        J := WinX1 - X else
        J := 0;
      if X + Width - 1 > WinX2 then
        K := WinX2 - X - J + 1 else
        K := Width - J;

      Inc(PI, J);
      PO := @Screen + I * BUFFER_WIDTH + X + J;
      for L := 1 to K do
      begin
		if PI^ > 0 then
          PO^ := PI^;
        Inc(PI);
        Inc(PO);
      end;
      Dec(PI, J + K);
    end;
    Inc(PI, Width);
  end;
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
  Move(Buffer, ScreenPalBuffer, SizeOf(ScreenPalBuffer));
  UpdateRGBBuffer;
end;

procedure ImageClear(var Buffer);
begin
  FillChar(Buffer, BUFFER_WIDTH * BUFFER_HEIGHT, 0);
end;

procedure ScreenMode(Mode: Integer);
var
  ThreadInfo: TKosThreadInfo;
begin
  if Mode <> CurrentScreenMode then
  begin
    if Assigned(ScreenRGBBuffer) then FreeMem(ScreenRGBBuffer);
    if Assigned(ScreenRGBTemporary) then FreeMem(ScreenRGBTemporary);

    case Mode of
      -2: begin
        ScreenWidth  := BUFFER_WIDTH div 2;
        ScreenHeight := BUFFER_HEIGHT div 2;
      end;
      1..3: begin
        ScreenWidth  := BUFFER_WIDTH * Mode;
        ScreenHeight := BUFFER_HEIGHT * Mode;
      end;
    end;

    if CurrentScreenMode = 0 then Paint;

    kos_threadinfo(@ThreadInfo);

    with ThreadInfo, WindowRect do
    begin
      WindowWidth  := Width - ClientRect.Width + Longint(ScreenWidth);
      WindowHeight := Height - ClientRect.Height + Longint(ScreenHeight);
      kos_movewindow(Left, Top, WindowWidth, WindowHeight);
    end;

    CurrentScreenMode := Mode;

    ScreenRGBBuffer    := GetMem(ScreenWidth * ScreenHeight * SizeOf(ScreenRGBBuffer^));
    ScreenRGBTemporary := GetMem(ScreenWidth * BUFFER_HEIGHT * SizeOf(ScreenRGBTemporary^));

    UpdateRGBBuffer;
  end;
end;



function ScanToChar(Code: Word): Char;
var
  I: Word;
begin
  for I := Low(ScanToCharTable) to High(ScanToCharTable) do
  with ScanToCharTable[I] do
  if Scan = Code then
  begin
    if not CapsPressed then
      if not ShiftDown then
        Result := CL else
        Result := CU
    else
      if not ShiftDown then
        if not Caps then
          Result := CL else
          Result := CU
      else
        if not Caps then
          Result := CL else
          Result := CL;
    Exit;
  end;
  Result := #0;
end;

procedure KeyboardInitialize;
begin
  kos_setkeyboardmode(1);
end;

function ReadKeyLoop: Word;
var
  Event: Word;
begin
  kos_maskevents(ME_PAINT or ME_KEYBOARD);
  repeat
    Event := kos_getevent();
    if Event = SE_PAINT then Paint;
  until Event = SE_KEYBOARD;
  Result := kos_getkey();
end;

function TranslateKey(Key: Word): Word;
begin
  if Key = KEY_GREY then
    Result := kos_getkey() else
    Result := Key;

  LastKeyDown := Result < KEY_UP_BASE;
  LastKeyUp   := not LastKeyDown;
  if LastKeyUp then Dec(Result, KEY_UP_BASE);

  if Result = KEY_ALT then
  begin
    AltDown := LastKeyDown;
    Result  := $FFFF;
  end else

  if Result = KEY_LSHIFT then
  begin
    LShiftDown := LastKeyDown;
    ShiftDown  := LShiftDown or RShiftDown;
    Result     := $FFFF;
  end else

  if Result = KEY_RSHIFT then
  begin
    RShiftDown := LastKeyDown;
    ShiftDown  := LShiftDown or RShiftDown;
    Result     := $FFFF;
  end else

  if AltDown then
  case Result of
    KEY_1: begin Result := $FFFF; if LastKeyDown then ScreenMode(1); end;
    KEY_2: begin Result := $FFFF; if LastKeyDown then ScreenMode(2); end;
    KEY_3: begin Result := $FFFF; if LastKeyDown then ScreenMode(3); end;
    KEY_9: begin Result := $FFFF; if LastKeyDown then ScreenMode(-2); end;
    KEY_0: begin Result := $FFFF; if LastKeyDown then ScreenMode(100); end;
  end;
end;

function Keypressed: Boolean;
begin
  if (LastKeyEvent < KEY_UP_BASE) and LastKeyDown then
    Result := True else
  begin
    kos_maskevents(ME_KEYBOARD);
    if kos_getevent(False) = SE_KEYBOARD then
    begin
      LastKeyEvent := TranslateKey(kos_getkey());
      if LastKeyEvent < KEY_UP_BASE then
        Result := LastKeyDown else
        Result := False;
    end else
    begin
      LastKeyEvent := $FFFF;
      Result := False;
    end;
  end;
end;

function ReadKey: Word;
begin
  repeat
    if LastKeyEvent < KEY_UP_BASE then
      Result := LastKeyEvent else
      Result := TranslateKey(ReadKeyLoop);
    LastKeyEvent := $FFFF;
  until (Result < KEY_UP_BASE) and LastKeyDown;
end;

procedure KeyboardFlush;
begin
end;

procedure ProcessKeyboard;
begin
  LastKeyEvent := TranslateKey(kos_getkey());
end;



const
  MK_LBUTTON = 1;
  MK_RBUTTON = 2;
  MK_MBUTTON = 4;
  MouseButtonsCount = 3;

var
  MouseButtonsState   : DWord;
  MouseButtonsPressed : array[1..MouseButtonsCount] of DWord;
  MouseButtonsReleased: array[1..MouseButtonsCount] of DWord;


procedure ProcessMouse;
var
  I: Longint;
  Buttons, ButtonMask: DWord;
  NowPressed, WasPressed: Boolean;
begin
  Buttons := kos_getmousebuttons();

  for I := 1 to MouseButtonsCount do
  begin
    ButtonMask := 1 shl (I - 1);
    NowPressed := (Buttons and ButtonMask) <> 0;
    WasPressed := (MouseButtonsState and ButtonMask) <> 0;

    if NowPressed and not WasPressed then Inc(MouseButtonsPressed[I]) else
    if not NowPressed and WasPressed then Inc(MouseButtonsReleased[I]);
  end;
  
  MouseButtonsState := Buttons;
end;

procedure MouseInitialize;
var
  I: Longint;
begin
  MouseButtonsState := kos_getmousebuttons();
  for I := 1 to MouseButtonsCount do
  begin
    MouseButtonsPressed[I]  := 0;
    MouseButtonsReleased[I] := 0;
  end;
  ProcessMouse;
end;

function MSMouseInArea(x1, y1, x2, y2: Integer): Boolean;
var
  X, Y: Integer;
begin
  MSMouseGetXY(X, Y);
  Result := (X >= x1) and (X <= x2) and (Y >= y1) and (Y <= y2);
end;

function MSMouseDriverExist: Boolean;
begin
  Result := True;
end;

procedure MSMouseGetXY(var X, Y: Integer);
var
  WinPos: TKosPoint;
begin
  WinPos := kos_getmousewinpos();

  X := Round(Double(WinPos.X) * BUFFER_WIDTH / ScreenWidth);
  if X <  0            then X := 0 else
  if X >= BUFFER_WIDTH then X := BUFFER_WIDTH - 1;

  Y := Round(Double(WinPos.Y) * BUFFER_HEIGHT / ScreenHeight);
  if Y <  0             then Y := 0 else
  if Y >= BUFFER_HEIGHT then Y := BUFFER_HEIGHT - 1;
end;

function MSMouseButtonStatusGet: Word;
begin
  Result := Word(kos_getmousebuttons());
end;

function MSMouseButtonWasPressed(Button: Word; var x, y: Integer): Boolean;
begin
  Inc(Button);
  if Button < MouseButtonsCount then
  begin
    Result := MouseButtonsPressed[Button] > 0;
    MouseButtonsPressed[Button] := 0;
  end else
    Result := False;
  MSMouseGetXY(x, y);
end;

function MSMouseButtonWasReleased(Button: Word; var x, y: Integer): Boolean;
begin
  Inc(Button);
  if Button < MouseButtonsCount then
  begin
    Result := MouseButtonsReleased[Button] > 0;
    MouseButtonsReleased[Button] := 0;
  end else
    Result := False;
  MSMouseGetXY(x, y);
end;

procedure MSMouseSetXY(x, y: Integer);
begin
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

procedure Palette256Darken(var Palette256; StartElement, EndElement, Decrement, MinValue: Byte);
var
  I, J: Byte;
  PB  : PByte;
begin
  PB := @Palette256;
  Inc(PB, StartElement * 3);
  for I := StartElement to EndElement do
  for J := 1 to 3 do
  begin
    if PB^ > MinValue then
    if PB^ < Decrement then
      PB^ := MinValue else
      Dec(PB^, Decrement);
    Inc(PB);
  end;
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
  PB: PByte;
begin
  PB := @Buffer + BufferOffset;
  for I := 1 to Count do
  begin
    if PB^ > 0 then
      Inc(PB^, Amount);
    Inc(PB);
  end;
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
  for I := 0 to 32 do
  begin
    Palette256Transform(Pal1^, Pal^);
    Palette256Transform(Pal1^, Pal^);
    Palette256Set(Pal1^);
    kos_delay(1);
  end;
  FreeMem(Pal1, 768);
end;


procedure DecompressRepByte(var InArray, OutArray; InArraySize: Word; var OutArraySize: Word);
var
  I, J: Word;
  PIn : PByte;
  POut: PByte;
begin
  I := 0;
  PIn  := @InArray;
  POut := @OutArray;

  while I < InArraySize do
  begin
    Inc(I);

    if PIn^ = 0 then
    begin
      Inc(PIn);
      J := PIn^;
      Inc(I, 2);
      Inc(PIn);
      Inc(OutArraySize, J);
      while J > 0 do
      begin
        POut^ := PIn^;
        Inc(POut);
        Dec(J);
      end;
      Inc(PIn);
    end else

    if PIn^ < 4 then
    begin
      J := PIn^;
      Inc(I);
      Inc(PIn);
      Inc(OutArraySize, J);
      while J > 0 do
      begin
        POut^ := PIn^;
        Inc(POut);
        Dec(J);
      end;
      Inc(PIn);
    end else

    begin
      POut^ := PIn^;
      Inc(PIn);
      Inc(POut);
      Inc(OutArraySize);
    end;
  end;
end;


function GetInterrupt(Int: Byte): Pointer;
begin
  Result := nil;
end;


procedure WaitForEvent(Timeout: DWord = 0);
var
  Event: Word;
begin
  kos_maskevents(ME_PAINT or ME_KEYBOARD or ME_MOUSE);
  Event := kos_waitevent(Timeout);
  case Event of
    SE_PAINT: Paint;
    SE_KEYBOARD: ProcessKeyboard;
    SE_MOUSE: ProcessMouse;
  end;
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
