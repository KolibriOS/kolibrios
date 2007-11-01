unit LRLHighScores;


interface


uses
  LRLRoutines, LRLSprites, StrUnit;


procedure LRLLoadHighScores;
procedure LRLShowHighScores;
function LRLBestScore(Score: longint): boolean;
procedure LRLInsertScore(Name: string; Score: longint);
procedure LRLSaveHighScores;
function LRLEnterName: string;


implementation


const
  HighsFileName = 'LRL.HSR';
  HighsFileHeader: string[29] = 'Lode Runner Live High Scores'#26;

type
  TSupers = packed record
    Name:  string[20];
    Score: longint;
  end;

var
  MainScreen: POINTER;
  HighFrame:  POINTER;
  HighTable:  array[1..5] of TSupers;

procedure LoadData;
var
  j: word;
begin
  GETMEM(MainScreen, 64004);
  GETMEM(HighFrame, 45000);
  DFAFilePositionSet(ImageFile, LRLImagesFilePosition, DFASeekFromStart);
  DFAFileRead(ImageFile, MainScreen^, 7940, j);
  DecompressRepByte(MainScreen^, HighFrame^, 7940, j);
  DFAFileRead(ImageFile, MainScreen^, 64004, j);
end;

procedure DisposeData;
begin
  FREEMEM(MainScreen, 64004);
  FREEMEM(HighFrame, 45000);
end;

procedure LRLShowHighScores;
var
  p: POINTER;
  i: integer;
  s: string;
begin
  LRLLoadHighScores;
  GETMEM(p, 768);
  DataFill(p^, 768, 0, 0);
  Palette256Set(p^);
  FREEMEM(p, 768);
  LoadData;
  ImagePut(LRLScreen^, MainScreen^, 0, 0, 0, 0, 319, 199);
  ImagePut(LRLScreen^, HighFrame^, 6, 50, 0, 0, 319, 199);
  for i := 1 to 5 do
  begin
    ImageStringGet(CHR(i + 48) + '. ' + HighTable[i].Name, LRLFont^, LRLFontBuffer^, 110);
    ImagePut(LRLScreen^, LRLFontBuffer^, 55, 85 + i * 17, 8, 0, 319, 199);
    STR(HighTable[i].Score, s);
    ImageStringGet(s, LRLFont^, LRLFontBuffer^, 46);
    ImagePut(LRLScreen^, LRLFontBuffer^, 260 - ImageSizex(LRLFontBuffer^), 85 + i * 17, 8, 0, 319, 199);
  end;
  ScreenApply(LRLScreen^);
  FadeTo(LRLMenuPalette);
  READKEY;
  FadeClear;
  ImageClear(LRLScreen^);
  ScreenApply(LRLScreen^);
  DisposeData;
end;

procedure LRLLoadHighScores;
var
  InFile: TDFAFileHandle;
  i, j:  word;
  high:  TSupers;
  dummy: string[30];
begin
  high.Name := 'Lode Runner';
  DFAFileOpen(InFile, HighsFileName, DFAAccessReadWrite);
  if DFALastResult(InFile) <> 0 then
  begin
    DFAFileCreate(InFile, HighsFileName, DFAAttributeArchive);
    DFAFileWrite(InFile, HighsFileHeader[1], 29, i);
    for i := 1 to 5 do
    begin
      high.score := 60000 - i * 10000;
      DFAFileWrite(InFile, high, SIZEOF(high), j);
    end;
  end;
  DFAFilePositionSet(InFile, 0, DFASeekFromStart);
  DFAFileRead(InFile, dummy[1], 29, j);
  if (DFALastResult(InFile) <> 0) or
    (not DataIdentical(dummy[1], HighsFileHeader[1], 29, 0, 0)) then
  begin
    WRITELN('Error: Invalid file with high scores! (try to remove LRL.HSR file)');
    WRITELN('Ошибка: Неверный файл с рекордами! (попробуйте удалить файл LRL.HSR)');
    Halt(1);
  end;
  DFAFileRead(InFile, HighTable, SIZEOF(TSupers) * 5, j);
  DFAFileClose(InFile);
end;

procedure LRLSaveHighScores;
var
  InFile: TDFAFileHandle;
  i, j: word;
begin
  DFAFileOpen(InFile, HighsFileName, DFAAccessReadWrite);
  DFAFilePositionSet(InFile, 29, DFASeekFromStart);
  DFAFileWrite(InFile, HighTable, SIZEOF(TSupers) * 5, j);
  DFAFileClose(InFile);
end;

function LRLBestScore(Score: longint): boolean;
var
  i: integer;
begin
  LRLBestScore := True;
  LRLLoadHighScores;
  i := 1;
  while True do
  begin
    if Score >= HighTable[i].Score then
      EXIT;
    Inc(i);
    if i > 5 then
    begin
      LRLBestScore := False;
      EXIT;
    end;
  end;
end;

procedure LRLInsertScore(Name: string; Score: longint);
var
  i, j: word;
begin
  LRLLoadHighScores;
  i := 1;
  while True do
  begin
    if Score >= HighTable[i].Score then
    begin
      for j := 4 downto i do
      begin
        HighTable[j + 1].Name  := HighTable[j].Name;
        HighTable[j + 1].Score := HighTable[j].Score;
      end;
      HighTable[i].Name  := Name;
      HighTable[i].Score := Score;
      LRLSaveHighScores;
      EXIT;
    end;
    Inc(i);
    if i > 5 then
    begin
      EXIT;
    end;
  end;
end;

function LRLEnterName: string;
var
  p: POINTER;
  i: integer;
  RedrawName: boolean;
  Keypress: word;
  Name: string;
begin
  Name := '';
  GETMEM(p, 768);
  DataFill(p^, 768, 0, 0);
  Palette256Set(p^);
  FREEMEM(p, 768);
  ImageClear(LRLScreen^);
  ImagePut(LRLScreen^, LRLLogo^, 3, 3, 0, 0, 319, 199);
  ImageStringGet('Congratulations! You are in Top-Five!', LRLFont^, LRLFontBuffer^, 110);
  ImagePut(LRLScreen^, LRLFontBuffer^, 160 - ImageSizex(LRLFontBuffer^) shr 1, 85, 0, 0, 319, 199);
  ImageStringGet('Enter your name below, Champ', LRLFont^, LRLFontBuffer^, 111);
  ImagePut(LRLScreen^, LRLFontBuffer^, 160 - ImageSizex(LRLFontBuffer^) shr
    1, 110, 0, 0, 319, 199);
  ImageStringGet('---------------------------', LRLFont^, LRLFontBuffer^, 100);
  ImagePut(LRLScreen^, LRLFontBuffer^, 160 - ImageSizex(LRLFontBuffer^) shr
    1, 155, 0, 0, 319, 199);
  ScreenApply(LRLScreen^);
  FadeTo(LRLMenuPalette);
  RedrawName := True;
  repeat
    if RedrawName = True then
    begin
      ImageFill(LRLFontBuffer^, 320, 20, 0);
      ImagePut(LRLScreen^, LRLFontBuffer^, 0, 140, 0, 0, 319, 199);
      ImageStringGet(Name, LRLFont^, LRLFontBuffer^, 100);
      ImagePut(LRLScreen^, LRLFontBuffer^, 160 - ImageSizex(LRLFontBuffer^) shr
        1, 140, 0, 0, 319, 199);
      ScreenApply(LRLScreen^);
      RedrawName := False;
    end;
    Keypress := READKEY;
    if (LO(Keypress) = 8) and (LENGTH(Name) > 0) then
    begin
      Name[0] := char(Ord(Name[0]) - 1);
      RedrawName := True;
    end;
    if (LO(Keypress) > 31) and (LENGTH(Name) < 20) then
    begin
      Name := Name + char(LO(Keypress));
      RedrawName := True;
    end;
  until LO(Keypress) = 13;
  FadeClear;
  Name := StringTrimAll(Name, ' ');
  if LENGTH(Name) = 0 then
    Name := 'Anonymous';
  LRLEnterName := Name;
end;


end.
