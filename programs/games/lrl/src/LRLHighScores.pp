unit LRLHighScores;

{$mode objfpc}
{$i-}


interface


uses
  SysUtils,
  LRLRoutines, LRLSprites;


procedure LRLLoadHighScores;
procedure LRLShowHighScores;
function LRLBestScore(Score: Longint): Boolean;
procedure LRLInsertScore(Name: String; Score: Longint);
procedure LRLSaveHighScores;
function LRLEnterName: String;


implementation


const
  HighsFileName = 'LRL.HSR';
  HighsFileHeader: String[29] = 'Lode Runner Live High Scores'#26;

type
  TSupers = packed record
    Name:  String[20];
    Score: Longint;
  end;

var
  MainScreen: Pointer;
  HighFrame:  Pointer;
  HighTable:  array[1..5] of TSupers;


procedure LoadData;
var
  j: Word;
begin
  GetMem(MainScreen, 64004);
  GetMem(HighFrame, 45000);
  Seek(ImageFile, LRLImagesFilePosition);
  BlockRead(ImageFile, MainScreen^, 7940, j);
  DecompressRepByte(MainScreen^, HighFrame^, 7940, j);
  BlockRead(ImageFile, MainScreen^, 64004, j);
end;


procedure DisposeData;
begin
  FreeMem(MainScreen, 64004);
  FreeMem(HighFrame, 45000);
end;


procedure LRLShowHighScores;
var
  p: Pointer;
  i: Integer;
  s: String;
begin
  LRLLoadHighScores;

  GetMem(p, 768);
  DataFill(p^, 768, 0, 0);
  Palette256Set(p^);
  FreeMem(p, 768);

  LoadData;
  ImagePut(LRLScreen^, MainScreen^, 0, 0, 0, 0, 319, 199);
  ImagePut(LRLScreen^, HighFrame^, 6, 50, 0, 0, 319, 199);

  for i := 1 to 5 do
  begin
    ImageStringGet(Chr(i + 48) + '. ' + HighTable[i].Name, LRLFont^, LRLFontBuffer^, 110);
    ImagePut(LRLScreen^, LRLFontBuffer^, 55, 85 + i * 17, 8, 0, 319, 199);
    Str(HighTable[i].Score, s);
    ImageStringGet(s, LRLFont^, LRLFontBuffer^, 46);
    ImagePut(LRLScreen^, LRLFontBuffer^, 260 - ImageSizex(LRLFontBuffer^), 85 + i * 17, 8, 0, 319, 199);
  end;

  ScreenApply(LRLScreen^);
  FadeTo(LRLMenuPalette);

  ReadKey;

  FadeClear;
  ImageClear(LRLScreen^);
  ScreenApply(LRLScreen^);

  DisposeData;
end;


procedure LRLLoadHighScores;
var
  InFile: File;
  i, j:  Word;
  Dummy: String[30];
begin
  FileMode := 0;
  AssignFile(InFile, HighsFileName);
  Reset(InFile, 1);

  if IOResult <> 0 then
  begin
    for i := 1 to 5 do
    begin
      HighTable[i].Name := 'Lode Runner';
      HighTable[i].score := 60000 - i * 10000;
    end;
    AssignFile(InFile, HighsFileName);
    Rewrite(InFile, 1);
    BlockWrite(InFile, HighsFileHeader[1], 29, i);
    BlockWrite(InFile, HighTable, SizeOf(TSupers) * 5, j);
  end else
  begin
    Seek(InFile, 0);
    BlockRead(InFile, Dummy[1], 29, j);
    if (IOResult <> 0) or (not DataIdentical(Dummy[1], HighsFileHeader[1], 29, 0, 0)) then
  	  raise Exception.Create('Error: Invalid file with high scores! (try to remove LRL.HSR file)');
    BlockRead(InFile, HighTable, SizeOf(TSupers) * 5, j);
  end;

  Close(InFile);
end;


procedure LRLSaveHighScores;
var
  InFile: File;
  j: Word;
begin
  FileMode := 2;
  AssignFile(InFile, HighsFileName);
  Reset(InFile, 1);
  Seek(InFile, 29);
  BlockWrite(InFile, HighTable, SizeOf(TSupers) * 5, j);
  Close(InFile);
end;


function LRLBestScore(Score: Longint): Boolean;
var
  i: Integer;
begin
  LRLBestScore := True;
  LRLLoadHighScores;
  i := 1;
  while True do
  begin
    if Score >= HighTable[i].Score then
      Exit;
    Inc(i);
    if i > 5 then
    begin
      LRLBestScore := False;
      Exit;
    end;
  end;
end;


procedure LRLInsertScore(Name: String; Score: Longint);
var
  i, j: Word;
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
      Exit;
    end;
    Inc(i);
    if i > 5 then
    begin
      Exit;
    end;
  end;
end;


function LRLEnterName: String;
var
  p: Pointer;
  RedrawName: Boolean;
  Keypress: Word;
  Name: String;
  C: Char;
begin
  Name := '';

  GetMem(p, 768);
  DataFill(p^, 768, 0, 0);
  Palette256Set(p^);
  FreeMem(p, 768);

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
      ImagePut(LRLScreen^, LRLFontBuffer^, 160 - ImageSizex(LRLFontBuffer^) shr 1, 140, 0, 0, 319, 199);
      ScreenApply(LRLScreen^);
      RedrawName := False;
    end;

    Keypress := ReadKey;

    if (Keypress = KEY_BACK) and (Length(Name) > 0) then
    begin
	  SetLength(Name, Length(Name) - 1);
      RedrawName := True;
    end;

    C := ScanToChar(Keypress);
    if (C > #31) and (Length(Name) < 20) then
    begin
      Name := Name + C;
      RedrawName := True;
    end;

  until Keypress = KEY_ENTER;
  FadeClear;

  Name := Trim(Name);
  if Length(Name) = 0 then
    Name := 'Anonymous';
  LRLEnterName := Name;
end;


end.
