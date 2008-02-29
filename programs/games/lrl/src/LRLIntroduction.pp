unit LRLIntroduction;


interface


uses
  SysUtils,
  LRLRoutines, LRLSprites;


procedure LRLIntro;


implementation


const
  IntroText: array[1..14] of String = (
    'Lode Runner LIVE. FREEWARE Version 1.4b',
    'KolibriOS port by bw (Vladimir V. Byrgazov)',
    'Copyright (c) 1995 Aleksey V. Vaneev',
    'Copyright (c) 2008 bw',
    '',
    'Send comments to Aleksey V. Vaneev',
    '2:5003/15@FidoNet',
    'ikomi@glas.apc.org',
    '',
    'Send comments to bw',
    'bw@handsdriver.net',
    '',
    '',
    '');

  SPACE40 = '                                        ';


var
  TimeToRefresh: Boolean;


procedure LRLIntro;
var
  i, j, l: Integer;
  Count: Word;
  k: Word;
  MainScreen: Pointer;
begin
  GetMem(MainScreen, 64004);

  Seek(ImageFile, LRLImagesFilePosition + 7940);
  BlockRead(ImageFile, MainScreen^, 64004, k);
  Palette256Set(LRLMenuPalette^);
  ImageFill(LRLFontBuffer^, 320, 55, 0);
  ImageClear(LRLScreen^);

  for i := -50 to 4 do
  begin
    ImagePut(LRLScreen^, LRLFontBuffer^, 0, 0, 0, 0, 319, 199);
    ImagePut(LRLScreen^, LRLLogo^, 3, i, 0, 0, 319, 199);
    ScreenApply(LRLScreen^);
    if Keypressed then
    begin
      ReadKey;
      FreeMem(MainScreen, 64004);
      Exit;
    end;
    Sleep(10);
  end;

  ImageFill(LRLFontBuffer^, 320, 55, 0);
  for i := 0 to 10 do
  begin
    for k := 0 to 20 do
    for j := 0 to 16 do
      ImagePutTransparent(LRLScreen^, MainScreen^, 0, 0,
        j * 20 - 10 - i, k * 20 - 10 - i,
        j * 20 - 10 + i, k * 20 - 10 + i);

    Sleep(50);

    ImagePut(LRLScreen^, LRLFontBuffer^, 0, 182, 0, 0, 319, 199);
    ScreenApply(LRLScreen^);
    if Keypressed then
    begin
      ReadKey;
      FreeMem(MainScreen, 64004);
      Exit;
    end;
  end;

  Count := 1;
  k := 1;
  repeat
    if TimeToRefresh then
    begin
      Inc(Count);
      TimeToRefresh := False;
    end;

    if Count >= 2 then
    begin
      ImageStringGet(SPACE40 + IntroText[k] + SPACE40, LRLFont^, LRLFontBuffer^, 110);
      for l := 200 downto 184 do
      begin
        ImagePut(LRLScreen^, LRLFontBuffer^, 160 - ImageSizeX(LRLFontBuffer^) div 2, l, 0, 0, 319, 199);
        ScreenApply(LRLScreen^);
        Sleep(20);
      end;
      Inc(k);
      if k > Length(IntroText) then k := 1;
      Count := 0;
    end;

    for I := 1 to 8 do
    if Keypressed then
      Break else
      Sleep(250);

    TimeToRefresh := True;
  until KeyPressed;

  ReadKey;
  FadeClear;
  FreeMem(MainScreen, 64004);
end;


end.
