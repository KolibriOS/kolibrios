unit LRLLevels;

{$mode objfpc}


interface


uses
  SysUtils,
  LRLRoutines, LRLSprites;


type
  TLRLPlayerPosition = packed record
    x, y:         Byte;
    xoffs, yoffs: ShortInt;
  end;

  TLRLPlayer = packed record
    Command:       Byte;
    { pictures:
      1 - running left <-
      2 - running right ->
      3 - climbing up ^
      4 - climbing down v
      5 - falling
      6 - ~~~~~ left <-
      7 - ~~~~~ right ->
      8 - firing left <-
      9 - firing right ->
    }
    NewCommandWas: Boolean;
    NewCommand:    Byte;
    Position:      TLRLPlayerPosition;
    Sprite:        Byte;
    SpriteData:    Byte;
    Controller:    Byte;
    {
      controllers:
      0 - not playing
      1 - human/keyboard
      2 - computer
    }
    Prizes:        Byte;
    {
      max 1 if computer player
      a) computer player leaves prize if falling into hole
      b) takes prize if he has no prizes
    }
    Colour:        Byte;
  end;

  TLRLBrick = packed record
    Image:     Byte;
    Count:     Byte;
    Flags:     Byte;
    { flags:
      bit 0 - needed to animate this brick 5 sprites then pause
        and then finnally 5 sprites
      bit 1 - set if fatal brick
      bit 2 - set if allowable to jump
      bit 3 - allowable to walk thru
      bit 4 - hidden
      bit 5 - background
      bit 6 - wait now
      bit 7 - not draw it
    }
    IdleCount: Byte;
  end;

  TLRLLevel = packed record
    Field:  array[1..30, 1..16] of TLRLBrick;
    Player: array[1..20] of TLRLPlayer;
  end;


const
  BrickFlags: array[1..20] of Byte = (
    48, 4 + 8 + 32 + 128,
    49, 8 + 32,
    50, 4 + 8 + 32,
    51, 4 + 8 + 32,
    52, 2,
    53, 4,
    54, 4 + 8,
    55, 2,
    56, 2,
    65, 4 + 8 + 16 + 32);


const
  KeyboardControls: array[1..21] of Word = (
    KEY_LEFT,  1, 1,
    KEY_RIGHT, 1, 2,
    KEY_UP,    1, 3,
    KEY_DOWN,  1, 4,
    KEY_GREY5, 1, 5,
    KEY_END,   1, 6,
    KEY_PGDN,  1, 7);
  ControlNumber = 7;


var
  ShowLives:        Boolean;
  ShowScore:        Boolean;
  ShowLevel:        Boolean;
  LRLLevel:         TLRLLevel;
  LRLLives:         Integer;
  LRLScore:         Longint;
  LRLCLevel:        Word;
  ComputerTurn:     Word;
  ComputerReaction: Word;
  TimeToRefresh:    Boolean;
  OldTimer:         Pointer;
  TotalPrizes:      Integer;
  GameStarted:      Boolean;
  EndOfGame:        Boolean;
  GameResult:       Word;
  Paused:           Boolean;


procedure LRLLoadLevel(Number: Byte);
procedure LRLUpdatePlayers;
procedure LRLDrawOrnamental(x1, y1, x2, y2, ornament: Byte);
procedure LRLPlayLevel(Number: Byte);
function LRLLevelCount: Word;
procedure LRLDeleteLevel(Count: Word);
procedure LRLInsertLevel(After: Word);
procedure LRLSaveLevel(Count: Word);


implementation


const
  LevelFileName = 'LRL.LEV';
  LevelFileHeader: ShortString = 'Lode Runner Live Levels'#26;

  ERR_OPENFILE = 'Невозможно открыть файл уровней';
  ERR_BADFILE  = 'Неверный или поврежденный файл уровней';


function LRLLevelCount: Word;
var
  LevelFile: File;
  c, k: Word;
begin
  c := 0;
  AssignFile(LevelFile, LevelFileName);
  Reset(LevelFile, 1);
  Seek(LevelFile, 24);
  BlockRead(LevelFile, c, 1, k);
  LRLLevelCount := c;
  Close(LevelFile);
end;


procedure LRLSaveLevel(Count: Word);
var
  LevelFile: File;
  i, j: Integer;
  k: Word;
  b: Pointer;
begin
  GetMem(b, 480);
  if (Count = 0) or (Count > LRLLevelCount) then
    Exit;
  FileMode := 2;
  AssignFile(LevelFile, LevelFileName);
  Reset(LevelFile, 1);
  Seek(LevelFile, Longint(25 + 520 * (Longint(Count) - 1)));
  for i := 1 to 10 do
  begin
    DataBytePut(b^, (i - 1) * 4, LRLLevel.Player[i].Position.x);
    DataBytePut(b^, (i - 1) * 4 + 1, LRLLevel.Player[i].Position.y);
    DataBytePut(b^, (i - 1) * 4 + 2, LRLLevel.Player[i].Colour);
    DataBytePut(b^, (i - 1) * 4 + 3, LRLLevel.Player[i].Controller);
  end;
  BlockWrite(LevelFile, b^, 40, k);
  for i := 1 to 16 do
  for j := 1 to 30 do
    DataBytePut(b^, (i - 1) * 30 + j - 1, LRLLevel.Field[j, i].Image + 47);
  BlockWrite(LevelFile, b^, 480, k);
  Close(LevelFile);
  FreeMem(b, 480);
end;


procedure LRLDeleteLevel(Count: Word);
var
  Buffer: Pointer;
  LevelFile: File;
  j: Integer;
  l: Longint;
  k: Word;
begin
  GetMem(Buffer, 1000);
  j := LRLLevelCount;
  if (j < Count) or (j < 2) or (Count = 0) then
    Exit;
  FileMode := 2;
  AssignFile(LevelFile, LevelFileName);
  Reset(LevelFile, 1);
  for l := Count + 1 to j do
  begin
    Seek(LevelFile, Longint(25 + 520 * (Longint(l) - 1)));
    BlockRead(LevelFile, Buffer^, 520, k);
    Seek(LevelFile, Longint(25 + 520 * (Longint(l - 1) - 1)));
    BlockWrite(LevelFile, Buffer^, 520, k);
  end;
  Seek(LevelFile, 24);
  Dec(j);
  BlockWrite(LevelFile, j, 1, k);
  Seek(LevelFile, FileSize(LevelFile) - 520);
  Truncate(LevelFile);
  Close(LevelFile);
  FreeMem(Buffer, 1000);
end;


procedure LRLInsertLevel(After: Word);
var
  Buffer: Pointer;
  LevelFile: File;
  j: Integer;
  l: Longint;
  k: Word;
begin
  GetMem(Buffer, 1000);
  j := LRLLevelCount;
  if (After > j) or (After = 0) then
    Exit;
  FileMode := 2;
  AssignFile(LevelFile, LevelFileName);
  Reset(LevelFile, 1);
  for l := j downto After + 1 do
  begin
    Seek(LevelFile, Longint(25 + 520 * (Longint(l) - 1)));
    BlockRead(LevelFile, Buffer^, 520, k);
    Seek(LevelFile, Longint(25 + 520 * (Longint(l + 1) - 1)));
    BlockWrite(LevelFile, Buffer^, 520, k);
  end;
  Seek(LevelFile, 24);
  Inc(j);
  BlockWrite(LevelFile, j, 1, k);
  Seek(LevelFile, Longint(25 + 520 * (Longint(After + 1) - 1)));
  DataFill(Buffer^, 40, 0, 0);
  DataFill(Buffer^, 480, 48, 40);
  BlockWrite(LevelFile, Buffer^, 520, k);
  Close(LevelFile);
  FreeMem(Buffer, 1000);
end;


procedure LRLLoadLevel(Number: Byte);
var
  LevelFile: File;
  InBuffer: Pointer;
  i, j, k: Word;
  a, b, c: Byte;
begin
  TotalPrizes := 0;
  GetMem(InBuffer, $FFF0);

  AssignFile(LevelFile, LevelFileName);
  Reset(LevelFile, 1);
  if IOResult <> 0 then
    raise Exception.Create(ERR_OPENFILE);

  BlockRead(LevelFile, InBuffer^, 24, k);
  BlockRead(LevelFile, c, 1, k);
  if (c = 0) or (IOResult <> 0) or (not DataIdentical(InBuffer^, LevelFileHeader[1], 24, 0, 0)) then
    raise Exception.Create(ERR_BADFILE);

  if (Number = 0) or (Number > c) then Number := 1;
  Seek(LevelFile, Longint(25 + 520 * (Longint(Number) - 1)));
  BlockRead(LevelFile, InBuffer^, 40, k);

  for i := 1 to 10 do
  with LRLLevel.Player[i] do
  begin
    Command := 10;
    NewCommandWas := False;
    NewCommand := 10;
    Position.x := DataByteGet(InBuffer^, (i - 1) * 4 + 0);
    Position.y := DataByteGet(InBuffer^, (i - 1) * 4 + 1);
    Position.xoffs := 0;
    Position.yoffs := 0;
    Sprite  := 1;
    SpriteData := 1;
    Controller := DataByteGet(InBuffer^, (i - 1) * 4 + 3);
    Prizes  := 0;
    Colour  := DataByteGet(InBuffer^, (i - 1) * 4 + 2);
  end;

  BlockRead(LevelFile, InBuffer^, 480, k);
  for i := 1 to 16 do for j := 1 to 30 do
  with LRLLevel.Field[j, i] do
  begin
    a := DataByteGet(InBuffer^, (i - 1) * 30 + (j - 1));
    for b := 1 to 10 do
    if BrickFlags[b * 2 - 1] = a then
      Flags := BrickFlags[b * 2];
    Count := 1;
    if a < 64 then
      a := a - 47 else
      a := a - 63;
    Image := a;
    IdleCount := 0;
    if Image = 4 then Inc(TotalPrizes);
  end;

  BlockRead(LevelFile, InBuffer^, 480, k);
  Close(LevelFile);
  LRLCLevel := Number;
  FreeMem(InBuffer, $FFF0);
end;


procedure LRLDrawOrnamental(x1, y1, x2, y2, ornament: Byte);
var
  i: Integer;
begin
  ImagePut(LRLScreen^, LRLDecoration[ornament].Image[6].Data^, x1 * 10, y1 * 10, 0, 0, 319, 199);
  ImagePut(LRLScreen^, LRLDecoration[ornament].Image[7].Data^, x2 * 10, y1 * 10, 0, 0, 319, 199);
  ImagePut(LRLScreen^, LRLDecoration[ornament].Image[5].Data^, x1 * 10, y2 * 10, 0, 0, 319, 199);
  ImagePut(LRLScreen^, LRLDecoration[ornament].Image[8].Data^, x2 * 10, y2 * 10, 0, 0, 319, 199);
  for i := x1 + 1 to x2 - 1 do
  begin
    ImagePut(LRLScreen^, LRLDecoration[ornament].Image[3].Data^, i * 10, y1 * 10, 0, 0, 319, 199);
    ImagePut(LRLScreen^, LRLDecoration[ornament].Image[4].Data^, i * 10, y2 * 10, 0, 0, 319, 199);
  end;
  for i := y1 + 1 to y2 - 1 do
  begin
    ImagePut(LRLScreen^, LRLDecoration[ornament].Image[2].Data^, x1 * 10, i * 10, 0, 0, 319, 199);
    ImagePut(LRLScreen^, LRLDecoration[ornament].Image[1].Data^, x2 * 10, i * 10, 0, 0, 319, 199);
  end;
end;


procedure LRLRedrawLevel;
var
  i, j: Integer;
  s: string;
begin
  ImageClear(LRLScreen^);
  for i := 1 to 16 do for j := 1 to 30 do
  with LRLLevel.Field[j, i] do
  if ((Flags and 128) = 0) and ((Flags and 32) <> 0) and ((Flags and 16) = 0) then
    ImagePut(LRLScreen^, LRLEnvironment[Image].Image[Count].Data^,j * 10, i * 10, 0, 0, 319, 199);

  for i := 1 to 10 do
  with LRLLevel.Player[i] do
  if Controller <> 0 then
    ImagePutTransparent(LRLScreen^, LRLFigure[Colour, SpriteData].Image[Sprite].Data^,Position.x * 10 + Position.xoffs, Position.y * 10 + Position.yoffs, 0, 0, 319, 199);

  for i := 1 to 16 do for j := 1 to 30 do
  with LRLLevel.Field[j, i] do
  if ((Flags and 128) = 0) and ((Flags and 32) = 0) and ((Flags and 16) = 0) then
    ImagePutTransparent(LRLScreen^, LRLEnvironment[Image].Image[LRLLevel.Field[j, i].Count].Data^, j * 10, i * 10, 0, 0, 319, 199);

  if not Paused then
  begin
    if ShowScore then
    begin
      STR(LRLScore, s);
      ImageStringGet(s, LRLFont^, LRLFontBuffer^, 222);
      ImagePut(LRLScreen^, LRLFontBuffer^, 56, 185, 0, 0, 319, 199);
      ImageStringGet('Score: ', LRLFont^, LRLFontBuffer^, 254);
      ImagePut(LRLScreen^, LRLFontBuffer^, 10, 185, 0, 0, 319, 199);
    end;
    if ShowLives then
    begin
      STR(LRLLives, s);
      ImageStringGet(s, LRLFont^, LRLFontBuffer^, 222);
      ImagePut(LRLScreen^, LRLFontBuffer^, 177, 185, 0, 0, 319, 199);
      ImageStringGet('Lives: ', LRLFont^, LRLFontBuffer^, 254);
      ImagePut(LRLScreen^, LRLFontBuffer^, 135, 185, 0, 0, 319, 199);
    end;
    if ShowLevel then
    begin
      Str(LRLCLevel, s);
      ImageStringGet(s, LRLFont^, LRLFontBuffer^, 222);
      ImagePut(LRLScreen^, LRLFontBuffer^, 292, 185, 0, 0, 319, 199);
      ImageStringGet('Level: ', LRLFont^, LRLFontBuffer^, 254);
      ImagePut(LRLScreen^, LRLFontBuffer^, 250, 185, 0, 0, 319, 199);
    end;
  end
  else
  begin
    ImageStringGet('Game now paused', LRLFont^, LRLFontBuffer^, 254);
    ImagePut(LRLScreen^, LRLFontBuffer^, 160 - ImageSizex(LRLFontBuffer^) div
      2, 185, 0, 0, 319, 199);
  end;
  LRLDrawOrnamental(0, 0, 31, 17, 1);
end;


procedure LRLStartSequence;
var
  tmpScreen1: Pointer;
  tmpScreen2: Pointer;
  i: Integer;
begin
  GetMem(tmpScreen1, 64000);
  GetMem(tmpScreen2, 49000);
  ImageFill(tmpScreen2^, 300, 160, 0);
  LRLRedrawLevel;
  i := 0;
  while i < 100 do
  begin
    DataMove(LRLScreen^, tmpScreen1^, 64000, 0, 0);
    ImagePut(tmpScreen1^, tmpScreen2^, 10, 10, 0, i, 319, 199 - i);
    ScreenApply(tmpScreen1^);
    Sleep(20);
    i := i + 4;
  end;
  ScreenApply(LRLScreen^);
  FreeMem(tmpScreen1, 64000);
  FreeMem(tmpScreen2, 49000);
end;


procedure LRLEndSequence;
var
  tmpScreen1: Pointer;
  tmpScreen2: Pointer;
  i: Integer;
begin
  GetMem(tmpScreen1, 64000);
  GetMem(tmpScreen2, 49000);
  ImageFill(tmpScreen2^, 300, 160, 0);
  LRLRedrawLevel;
  i := 100;
  while i > 0 do
  begin
    DataMove(LRLScreen^, tmpScreen1^, 64000, 0, 0);
    ImagePut(tmpScreen1^, tmpScreen2^, 10, 10, 0, i, 319, 199 - i);
    ScreenApply(tmpScreen1^);
    Sleep(20);
    i := i - 4;
  end;
  ImagePut(LRLScreen^, tmpScreen2^, 10, 10, 0, 0, 319, 199);
  ScreenApply(LRLScreen^);
  FreeMem(tmpScreen1, 64000);
  FreeMem(tmpScreen2, 49000);
end;


{ GameResult:
  1 - замуровали
  2 - поймали
  10 - все сделано
  50 - нет больше уровней
  60 - нет человеческих юнитов
  100 - нажата Esc }

procedure LRLUpdatePlayers;
var
  i, k: Integer;
  spd: Word;
begin
  for i := 1 to 10 do
  begin
    with LRLLevel.Player[i] do
    begin
      if Controller <> 0 then
      begin
        if (LRLLevel.Field[Position.x, Position.y].Flags and 2 <> 0) then
        begin
          if i = 1 then
          begin
            EndOfGame  := True;
            GameResult := 1;
            Exit;
          end;
          if Prizes <> 0 then
          begin
            Prizes := 0;
            LRLLevel.Field[Position.x, Position.y - 1].Image := 4;
            LRLLevel.Field[Position.x, Position.y - 1].Flags := BrickFlags[8];
          end;
          repeat
            Position.y := Random(2) + 1;
            Position.x := Random(30) + 1;
          until (LRLLevel.Field[Position.x, Position.y].Image = 1) or
            (LRLLevel.Field[Position.x, Position.y].Image = 2) or
            (LRLLevel.Field[Position.x, Position.y].Image = 3) or
            (LRLLevel.Field[Position.x, Position.y].Image = 4);
          Command := 10;
          Continue;
        end;

        if LRLLevel.Field[Position.x, Position.y].Image = 4 then
        if Controller = 2 then
        if Prizes = 0 then
        begin
          Inc(Prizes);
          LRLLevel.Field[Position.x, Position.y].Image := 1;
          LRLLevel.Field[Position.x, Position.y].Flags := BrickFlags[2];
        end else else
        begin
          Dec(TotalPrizes);
          LRLScore := LRLScore + 100 * Longint(LRLCLevel);
          LRLLevel.Field[Position.x, Position.y].Image := 1;
          LRLLevel.Field[Position.x, Position.y].Flags := BrickFlags[2];
        end;

        if (i = 1) then
        begin
          if (TotalPrizes = 0) and (Position.y = 1) and
             (LRLLevel.Field[Position.x, Position.y].Image = 2) then
          begin
            EndOfGame  := True;
            GameResult := 10;
            Exit;
          end;
          for k := 2 to 10 do
            if (LRLLevel.Player[k].Controller <> 0) and
               (LRLLevel.Player[k].Position.x = Position.x) and
               (LRLLevel.Player[k].Position.y = Position.y) then
            begin
              EndOfGame  := True;
              GameResult := 2;
              Exit;
            end;
        end;
        if (LRLLevel.Field[Position.x, Position.y].Flags and 1 <> 0) then
        begin
          if (Controller = 2) then
          begin
            if Prizes <> 0 then
            begin
              Prizes := 0;
              LRLLevel.Field[Position.x, Position.y - 1].Image := 4;
              LRLLevel.Field[Position.x, Position.y - 1].Flags := BrickFlags[8];
            end;
          end;
        end;
        if Controller = 2 then
          spd := 2
        else
          spd := 3;

        if (LRLLevel.Field[Position.x, Position.y + 1].Flags and 4 <> 0) and
           (LRLLevel.Field[Position.x, Position.y].Image <> 3) and
           ((LRLLevel.Field[Position.x, Position.y].Image <> 2) or
            (LRLLevel.Field[Position.x, Position.y].Flags and 16 <> 0)) and
           (Position.y < 16) then
        begin
          k := 2;
          while k <= 10 do
          if (k <> i) and (LRLLevel.Player[k].Controller <> 0) and
             (LRLLevel.Player[k].Position.x = Position.x) and
             (LRLLevel.Player[k].Position.y = Position.y + 1) and
             (Position.y < 16) then
          begin
            k := 100;
            Break;
          end else
            Inc(k);

          if k <> 100 then
          begin
            NewCommand := 5;
            NewCommandWas := True;
          end;
        end;

        if NewCommandWas then
        begin
          if (NewCommand <> Command) and (Command <> 5) then
          begin
            Command := NewCommand;
            Sprite  := 1;
          end;
          NewCommandWas := False;
        end;

        if (Command = 1) then
        begin
          if (LRLLevel.Field[Position.x, Position.y].Image = 3) then
          begin
            if Position.xoffs < 1 then
            begin
              if ((LRLLevel.Field[Position.x - 1, Position.y].Flags and 8 = 0) and
                (LRLLevel.Field[Position.x - 1, Position.y].Image <> 3)) or
                (LRLLevel.Field[Position.x, Position.y].Image <> 3) or (Position.x = 1) then
              begin
                Command := 10;
                Position.xoffs := 0;
              end;
            end;
            if (Command <> 10) and (SpriteData <> 6) then
            begin
              SpriteData := 6;
              Sprite := 1;
            end;
          end else
          begin
            if Position.xoffs < 1 then
            begin
              if (LRLLevel.Field[Position.x - 1, Position.y].Flags and 8 = 0) or (Position.x = 1) then
              begin
                Command := 10;
                Position.xoffs := 0;
              end;
            end;
            if (Command <> 10) and (SpriteData <> 1) then SpriteData := 1;
          end;

          if Command <> 10 then
          begin
            k := 1;
            while (k > 0) do
            begin
              Inc(k);
              if k = 11 then
              begin
                if (SpriteData = 6) then
                begin
                  if (Sprite = 2) then Dec(Position.xoffs, 5) else
                  if (Sprite = 3) then Dec(Position.xoffs, 1);
                end else
                  Dec(Position.xoffs, spd);
                Break;
              end;
              if (k <> i) and (i <> 1) and
                 (LRLLevel.Player[k].Controller <> 0) and
                 (LRLLevel.Player[k].Position.x = Position.x - 1) and
                 (LRLLevel.Player[k].Position.y = Position.y) then
                begin
                  Command := 10;
                  Break;
                end;
            end;
          end;
        end;

        if (Command = 2) then
        begin
          if (LRLLevel.Field[Position.x, Position.y].Image = 3) then
          begin
            if Position.xoffs > -1 then
            begin
              if ((LRLLevel.Field[Position.x + 1, Position.y].Flags and 8 = 0) and
                (LRLLevel.Field[Position.x + 1, Position.y].Image <> 3)) or
                (LRLLevel.Field[Position.x, Position.y].Image <> 3) or (Position.x = 30) then
              begin
                Command := 10;
                Position.xoffs := 0;
              end;
            end;
            if (Command <> 10) and (SpriteData <> 7) then
            begin
              SpriteData := 7;
              Sprite := 1;
            end;
          end
          else
          begin
            if Position.xoffs > -1 then
            begin
              if (LRLLevel.Field[Position.x + 1, Position.y].Flags and 8 = 0) or (Position.x = 30) then
              begin
                Command := 10;
                Position.xoffs := 0;
              end;
            end;
            if (Command <> 10) and (SpriteData <> 2) then
              SpriteData := 2;
          end;
          if Command <> 10 then
          begin
            k := 1;
            while (k > 0) do
            begin
              Inc(k);
              if k = 11 then
              begin
                if (SpriteData = 7) then
                begin
                  if (Sprite = 2) then
                    Inc(Position.xoffs, 5);
                  if (Sprite = 3) then
                    Inc(Position.xoffs, 1);
                end
                else
                  Inc(Position.xoffs, spd);
                Break;
              end;
              if (k <> i) and (i <> 1) and (LRLLevel.Player[k].Controller <> 0) then
                if (LRLLevel.Player[k].Position.x = Position.x + 1) and
                  (LRLLevel.Player[k].Position.y = Position.y) then
                begin
                  Command := 10;
                  Break;
                end;
            end;
          end;
        end;

        if (Command = 3) then
        begin
          if Position.yoffs < 1 then
          begin
            if ((LRLLevel.Field[Position.x, Position.y].Image <> 2) or (LRLLevel.Field[Position.x, Position.y].Flags and 16 <> 0)) or
               ((LRLLevel.Field[Position.x, Position.y - 1].Flags and 4 = 0) and
                 ((LRLLevel.Field[Position.x, Position.y - 1].Image <> 2) or (LRLLevel.Field[Position.x, Position.y - 1].Flags and 16 <> 0))) or
               (Position.y < 2) then
            begin
              Command := 10;
              Position.yoffs := 0;
            end;
          end;
          if (Command <> 10) and (SpriteData <> 3) then
            SpriteData := 3;
          if Command <> 10 then
          begin
            k := 1;
            while (k > 0) do
            begin
              Inc(k);
              if k = 11 then
              begin
                Dec(Position.yoffs, spd);
                Break;
              end;
              if (k <> i) and (i <> 1) and (LRLLevel.Player[k].Controller <> 0) then
                if (LRLLevel.Player[k].Position.y = Position.y - 1) and
                  (LRLLevel.Player[k].Position.x = Position.x) then
                begin
                  Command := 10;
                  Break;
                end;
            end;
          end;
        end;

        if (Command = 4) then
        begin
          if (LRLLevel.Field[Position.x, Position.y].Image = 3) and
             ((LRLLevel.Field[Position.x, Position.y + 1].Image <> 2) or
              (LRLLevel.Field[Position.x, Position.y + 1].Flags and 16 <> 0)) and
             (Position.y < 16) then
          begin
            Command := 5;
            Sprite  := 1;
            if (LRLLevel.Field[Position.x, Position.y + 1].Flags and 4 <> 0) then
              Inc(Position.yoffs);
          end
          else
          begin
            if Position.yoffs > -1 then
            begin
              if (((LRLLevel.Field[Position.x, Position.y + 1].Image <> 2) or
                (LRLLevel.Field[Position.x, Position.y + 1].Flags and 16 <> 0)) and
                (LRLLevel.Field[Position.x, Position.y + 1].Flags and 4 = 0)) or
                (Position.y = 16) then
              begin
                Command := 10;
                Position.yoffs := 0;
              end;
            end;
            if (Command <> 10) and (SpriteData <> 4) then
              SpriteData := 4;
            if Command <> 10 then
            begin
              k := 1;
              while (k > 0) do
              begin
                Inc(k);
                if k = 11 then
                begin
                  Inc(Position.yoffs, spd);
                  Break;
                end;
                if (k <> i) and (i <> 1) and (LRLLevel.Player[k].Controller <> 0) then
                  if (LRLLevel.Player[k].Position.y = Position.y + 1) and
                    (LRLLevel.Player[k].Position.x = Position.x) then
                  begin
                    Command := 10;
                    Break;
                  end;
              end;
            end;
          end;
        end;

        if (Command = 5) then
        begin
          if Position.yoffs < 1 then
          begin
            if (LRLLevel.Field[Position.x, Position.y + 1].Flags and 4 = 0) or
              (Position.y = 16) or (LRLLevel.Field[Position.x, Position.y].Image = 3) or
              ((LRLLevel.Field[Position.x, Position.y].Flags and 1 <> 0) and (i <> 1)) then
            begin
              Command := 10;
              if (LRLLevel.Field[Position.x, Position.y].Image = 3) then
                SpriteData := 5;
              Position.yoffs := 0;
              Position.xoffs := 0;
            end;
            for k := 2 to 10 do
              if (k <> i) and (LRLLevel.Player[k].Controller <> 0) then
                if (LRLLevel.Player[k].Position.x = Position.x) and
                  (LRLLevel.Player[k].Position.y = Position.y + 1) and
                  (LRLLevel.Field[Position.x, Position.y + 1].Flags and 1 <> 0) and
                  (Position.y < 16) then
                begin
                  Command := 10;
                  Position.yoffs := 0;
                  Break;
                end;
          end;
          if (Command <> 10) and (SpriteData <> 5) then
          begin
            SpriteData := 5;
            Sprite := 1;
          end;
          if Command <> 10 then
          begin
            Inc(Position.yoffs, 2);
          end;
        end;

        if (Command = 6) then
        begin
          if (Position.y < 16) and (Position.x > 1) and
            (LRLLevel.Field[Position.x - 1, Position.y + 1].Image = 9) and
            (LRLLevel.Field[Position.x - 1, Position.y + 1].Flags and 1 = 0) and
            (((LRLLevel.Field[Position.x - 1, Position.y].Image = 1) or
            (LRLLevel.Field[Position.x - 1, Position.y].Flags and 1 <> 0)) or
            (LRLLevel.Field[Position.x - 1, Position.y].Flags and 16 <> 0)) then
          begin
            NewCommandWas := True;
            for k := 2 to 10 do
              if (k <> i) and (LRLLevel.Player[k].Controller <> 0) then
                if (LRLLevel.Player[k].Position.x = Position.x - 1) and
                  (LRLLevel.Player[k].Position.y = Position.y) then
                begin
                  NewCommandWas := False;
                  Break;
                end;
            if NewCommandWas then
            begin
              LRLLevel.Field[Position.x - 1, Position.y + 1].Flags :=
                LRLLevel.Field[Position.x - 1, Position.y + 1].Flags or 1;
              Position.xoffs := 0;
              SpriteData := 8;
              NewCommandWas := False;
            end;
          end;
          Command := 10;
        end;

        if (Command = 7) then
        begin
          if (Position.y < 16) and (Position.x < 30) and
            (LRLLevel.Field[Position.x + 1, Position.y + 1].Image = 9) and
            (LRLLevel.Field[Position.x + 1, Position.y + 1].Flags and 1 = 0) and
            (((LRLLevel.Field[Position.x + 1, Position.y].Image = 1) or
            (LRLLevel.Field[Position.x + 1, Position.y].Flags and 1 <> 0)) or
            (LRLLevel.Field[Position.x + 1, Position.y].Flags and 16 <> 0)) then
          begin
            NewCommandWas := True;
            for k := 2 to 10 do
              if (k <> i) and (LRLLevel.Player[k].Controller <> 0) then
                if (LRLLevel.Player[k].Position.x = Position.x + 1) and
                  (LRLLevel.Player[k].Position.y = Position.y) then
                begin
                  NewCommandWas := False;
                  Break;
                end;
            if NewCommandWas then
            begin
              LRLLevel.Field[Position.x + 1, Position.y + 1].Flags :=
                LRLLevel.Field[Position.x + 1, Position.y + 1].Flags or 1;
              Position.xoffs := 0;
              SpriteData := 9;
              NewCommandWas := False;
            end;
          end;
          Command := 10;
        end;

        if (Command = 1) or (Command = 2) then
          if Position.yoffs < 0 then Inc(Position.yoffs) else
          if Position.yoffs > 0 then  Dec(Position.yoffs);

        if (Command = 3) or (Command = 4) or (Command = 5) then
          if Position.xoffs < 0 then Inc(Position.xoffs) else
          if Position.xoffs > 0 then Dec(Position.xoffs);

        if Command < 6 then
        begin
          Inc(Sprite);
          if Sprite > LRLFigure[Colour, SpriteData].ImageCount then Sprite := 1;
          if Position.xoffs < -4 then
          begin
            Dec(Position.x);
            Position.xoffs := 10 + Position.xoffs;
          end;
          if Position.xoffs > 5 then
          begin
            Inc(Position.x);
            Position.xoffs := Position.xoffs - 10;
          end;
          if Position.yoffs < -4 then
          begin
            Dec(Position.y);
            Position.yoffs := 10 + Position.yoffs;
          end;
          if Position.yoffs > 5 then
          begin
            Inc(Position.y);
            Position.yoffs := Position.yoffs - 10;
          end;
        end;
      end;
    end;
  end;
end;


procedure LRLUpdateBricks;
var
  i, j, k: Integer;
begin
  for i := 1 to 16 do
    for j := 1 to 30 do
    begin
      if LRLLevel.Field[j, i].Flags and 1 <> 0 then
      begin
        if LRLLevel.Field[j, i].Count = 1 then
        begin
          LRLLevel.Field[j, i].Flags := LRLLevel.Field[j, i].Flags and $FF - 2;
          LRLLevel.Field[j, i].Flags := LRLLevel.Field[j, i].Flags or 4 + 8;
        end;
        if LRLLevel.Field[j, i].IdleCount = 0 then
        begin
          Inc(LRLLevel.Field[j, i].Count);
          if LRLLevel.Field[j, i].Count < 6 then
          begin
            for k := 2 to 10 do
              if (LRLLevel.Player[k].Controller <> 0) then
                if (LRLLevel.Player[k].Position.x = j) and
                  (LRLLevel.Player[k].Position.y = i - 1) then
                begin
                  LRLLevel.Field[j, i].Count := 13 - LRLLevel.Field[j, i].Count;
                  LRLLevel.Field[j, i].Flags := LRLLevel.Field[j, i].Flags or 2;
                  LRLLevel.Field[j, i].Flags := LRLLevel.Field[j, i].Flags and $FE - 4 - 8;
                  LRLLevel.Field[j, i].Count := 1;
                  Break;
                end;
          end;
          if LRLLevel.Field[j, i].Count = 6 then
          begin
            LRLLevel.Field[j, i].IdleCount := 100;
          end;
        end
        else
          Dec(LRLLevel.Field[j, i].IdleCount);
        if LRLLevel.Field[j, i].Count = 12 then
        begin
          LRLLevel.Field[j, i].Flags := LRLLevel.Field[j, i].Flags or 2;
          LRLLevel.Field[j, i].Flags := LRLLevel.Field[j, i].Flags and $FE - 4 - 8;
          LRLLevel.Field[j, i].Count := 1;
        end;
      end;
    end;
end;


procedure LRLComputerPlayer;
var
  k, l, m, f1, f2, i: Integer;
begin
  if ComputerTurn >= ComputerReaction then
  begin
    ComputerTurn := 0;
    for k := 1 to 10 do
    begin
      with LRLLevel.Player[k] do
      begin
        if Controller = 2 then
        begin
          NewCommandWas := True;
          NewCommand := 10;
          if (Position.y > LRLLevel.Player[1].Position.y) then
          begin
            if ((LRLLevel.Field[Position.x, Position.y].Image = 2) and
              (LRLLevel.Field[Position.x, Position.y].Flags and 16 = 0) and
              ((LRLLevel.Field[Position.x, Position.y - 1].Image = 2) or
              (LRLLevel.Field[Position.x, Position.y - 1].Flags and 4 <> 0)) and
              (Position.y > 1)) then
            begin
              NewCommand := 3;
            end
            else
            begin
              m := 1;
              l := Position.x;
              i := 1;
              while i <> 0 do
              begin
                l := l + i;
                if ((LRLLevel.Field[l, Position.y].Image = 2) and
                  (LRLLevel.Field[l, Position.y].Flags and 16 = 0)) and
                  ((LRLLevel.Field[l, Position.y - 1].Image = 2) and
                  (LRLLevel.Field[l, Position.y - 1].Flags and 16 = 0)) and (Position.y <> 1) then
                begin
                  if m = 0 then
                  begin
                    f2 := Position.x - l;
                    Break;
                  end;
                  m  := 0;
                  i  := not i + 1;
                  f1 := l - Position.x;
                  l  := Position.x;
                end
                else
                if (LRLLevel.Field[l, Position.y].Flags and 8 = 0) or (l > 30) or (l < 1) then
                begin
                  if m = 0 then
                  begin
                    f2 := 100;
                    Break;
                  end;
                  m  := 0;
                  i  := not i + 1;
                  l  := Position.x;
                  f1 := 100;
                end;
              end;
              if (f1 = 100) and (f2 = 100) then
                NewCommand := 10
              else
              begin
                if f1 > f2 then
                  NewCommand := 1
                else
                  NewCommand := 2;
              end;
            end;
          end else

          if (Position.y < LRLLevel.Player[1].Position.y) then
          begin
            if (((LRLLevel.Field[Position.x, Position.y + 1].Image = 2) and
              (LRLLevel.Field[Position.x, Position.y + 1].Flags and 16 = 0)) or
              (LRLLevel.Field[Position.x, Position.y + 1].Flags and 4 <> 0)) and
              (Position.y < 16) and (LRLLevel.Field[Position.x, Position.y + 1].Flags and 1 = 0) then
            begin
              NewCommand := 4;
            end
            else
            begin
              m := 1;
              l := Position.x;
              i := 1;
              while i <> 0 do
              begin
                l := l + i;
                if ((LRLLevel.Field[l, Position.y + 1].Image = 2) and
                  (LRLLevel.Field[l, Position.y + 1].Flags and 16 = 0)) or
                  ((LRLLevel.Field[l, Position.y + 1].Flags and 4 <> 0) and
                  (LRLLevel.Field[l, Position.y + 1].Flags and 1 = 0)) then
                begin
                  if m = 0 then
                  begin
                    f2 := Position.x - l;
                    Break;
                  end;
                  m  := 0;
                  i  := not i + 1;
                  f1 := l - Position.x;
                  l  := Position.x;
                end
                else
                if (LRLLevel.Field[l, Position.y].Flags and 8 = 0) or (l > 30) or (l < 1) then
                begin
                  if m = 0 then
                  begin
                    f2 := 100;
                    Break;
                  end;
                  m  := 0;
                  i  := not i + 1;
                  l  := Position.x;
                  f1 := 100;
                end;
              end;
              if (f1 = 100) and (f2 = 100) then
                NewCommand := 10
              else
              begin
                if f1 > f2 then
                  NewCommand := 1
                else
                  NewCommand := 2;
              end;
            end;
          end
          else
          begin
            if (Position.x > LRLLevel.Player[1].Position.x) then
              NewCommand := 1;
            if (Position.x < LRLLevel.Player[1].Position.x) then
              NewCommand := 2;
          end;
        end;
      end;
    end;
  end else
    Inc(ComputerTurn);
end;


procedure LRLPlayLevel(Number: Byte);
var
  Keypress: Word;
  i: Word;
  L, C: Longword;
begin
  Randomize;
  ComputerReaction := 1;
  LRLLoadLevel(Number);

  if LRLCLevel <> Number then
  begin
    GameResult := 50;
    Exit;
  end;
  if LRLLevel.Player[1].Controller <> 1 then
  begin
    GameResult := 60;
    Exit;
  end;

  TimeToRefresh := True;
  GameStarted := False;
  GameResult := 0;
  Paused := False;
  EndOfGame := False;

  LRLStartSequence;

  Keypress := 0;
  L := 0;

  repeat
    C := LastDosTick();
    if L <> C then
    begin
      L := C;
      if GameStarted and not Paused then
      begin
        LRLComputerPlayer;
        LRLUpdatePlayers;
        LRLUpdateBricks;
      end;
      LRLRedrawLevel;
      ScreenApply(LRLScreen^);
    end else
      Sleep(20);

    if Keypressed then
    begin
      Keypress := ReadKey;
      GameStarted := True;
      Paused := False;

      for i := 0 to ControlNumber - 1 do
      if KeyboardControls[i * 3 + 1] = Keypress then
      begin
        LRLLevel.Player[KeyboardControls[i * 3 + 2]].NewCommand := KeyboardControls[i * 3 + 3];
        LRLLevel.Player[KeyboardControls[i * 3 + 2]].NewCommandWas := True;
      end;

      if Keypress = KEY_P then
        Paused := True;
    end;
  until (Keypress = KEY_ESC) or EndOfGame;

  if EndOfGame then
    LRLEndSequence else
    GameResult := 100;
end;


end.
