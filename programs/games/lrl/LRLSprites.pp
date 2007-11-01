unit LRLSprites;

{$mode objfpc}
{$i-}


interface


uses
  SysUtils,
  LRLRoutines;

{
  all coordinates in standard style:
  0          +
  0 +---------->  x
  |
  |
  |
  |
  + v

  y
}

type
  TLRLImage = packed record
    Data: Pointer; { standard 256-colour image data }
    Size: Word;    { size of image (for destruction) }
  end;

  PLRLSprite = ^TLRLSprite;

  TLRLSprite = packed record
    Image:      array[1..12] of TLRLImage; { moving image }
    ImageCount: Byte; { how many images there }
  end;


type
  TButton = packed record
    Lit:           Boolean;
    DarkIcon:      Pointer;
    LightIcon:     Pointer;
    DarkIconSize:  Word;
    LightIconSize: Word;
    x1, y1:        Integer;
    x2, y2:        Integer;
    Command:       Word;
  end;


var
  ImageFile:       File;
  LRLEnvironment:  array[1..20] of TLRLSprite;
  LRLFigure:       array[1..4, 1..9] of TLRLSprite;
  LRLDecoration:   array[1..1] of TLRLSprite;
  LRLPalette:      Pointer;
  LRLScreen:       Pointer;
  LRLMenuPalette:  Pointer;
  LRLLogo:         Pointer;
  LRLFont:         Pointer;
  LRLFontBuffer:   Pointer;
  LRLMousePointer: Pointer;
  LRLImagesFilePosition: longint;
  LRLEditorButton: array[1..6] of TButton;


procedure ImagesInitialize;
procedure ImagesDeinitialize;


implementation


const
  ImageFileName = 'LRL.IMG';
  ImageFileHeader: ShortString = 'Lode Runner Live Images'#26;

  ERR_OPENFILE = 'Невозможно открыть файл картинок';
  ERR_BADFILE  = 'Неверный или поврежденный файл картинок';


procedure LoadImages;
var
  InBuffer: Pointer;
  i, j, k, l, x, y: Word;
  a, b, c:  Byte;
begin
  GetMem(InBuffer, $FFF0);

  AssignFile(ImageFile, ImageFileName);
  Reset(ImageFile, 1);
  if IOResult <> 0 then
    raise Exception.Create(ERR_OPENFILE);

  BlockRead(ImageFile, InBuffer^, 24, k);
  if (IOResult <> 0) or not DataIdentical(InBuffer^, ImageFileHeader[1], 24, 0, 0) then
    raise Exception.Create(ERR_BADFILE);

  { load palette }
  GetMem(LRLPalette, 768);
  BlockRead(ImageFile, LRLPalette^, 768, k);

  { figures loading loop }
  for i := 1 to 9 do
  begin
    BlockRead(ImageFile, a, 1, k);
    LRLFigure[1, i].ImageCount := a;
    for j := 1 to a do
    begin
      GetMem(LRLFigure[1, i].Image[j].Data, 104);
      BlockRead(ImageFile, LRLFigure[1, i].Image[j].Data^, 104, k);
      x := DataWordGet(LRLFigure[1, i].Image[j].Data^, 0);
      y := DataWordGet(LRLFigure[1, i].Image[j].Data^, 2);
      LRLFigure[1, i].Image[j].Size := x * y + 4;
      for l := 2 to 4 do
      begin
        LRLFigure[l, i].Image[j].Size := LRLFigure[1, i].Image[j].Size;
        LRLFigure[l, i].ImageCount := a;
        GetMem(LRLFigure[l, i].Image[j].Data, LRLFigure[l, i].Image[j].Size);
        DataMove(LRLFigure[1, i].Image[j].Data^, LRLFigure[l, i].Image[j].Data^, LRLFigure[l, i].Image[j].Size, 0, 0);
        DataAdd(LRLFigure[l, i].Image[j].Data^, LRLFigure[l, i].Image[j].Size, (l - 1) shl 5, 4);
      end;
    end;
  end;

  { decoration loading loop }
  for i := 1 to 1 do
  begin
    BlockRead(ImageFile, a, 1, k);
    LRLDecoration[i].ImageCount := a;
    for j := 1 to a do
    begin
      GetMem(LRLDecoration[i].Image[j].Data, 104);
      BlockRead(ImageFile, LRLDecoration[i].Image[j].Data^, 104, k);
      x := DataWordGet(LRLDecoration[i].Image[j].Data^, 0);
      y := DataWordGet(LRLDecoration[i].Image[j].Data^, 2);
      LRLDecoration[i].Image[j].Size := x * y + 4;
    end;
  end;

  { environment loading loop }
  for i := 1 to 9 do
  begin
    BlockRead(ImageFile, a, 1, k);
    LRLEnvironment[i].ImageCount := a;
    for j := 1 to a do
    begin
      GetMem(LRLEnvironment[i].Image[j].Data, 104);
      BlockRead(ImageFile, LRLEnvironment[i].Image[j].Data^, 104, k);
      x := DataWordGet(LRLEnvironment[i].Image[j].Data^, 0);
      y := DataWordGet(LRLEnvironment[i].Image[j].Data^, 2);
      LRLEnvironment[i].Image[j].Size := x * y + 4;
    end;
  end;

  y := 181;
  x := 212;

  for i := 1 to 6 do
  begin
    if i = 4 then
    begin
      y := 191;
      x := 212;
    end;

    LRLEditorButton[i].x1  := x;
    LRLEditorButton[i].y1  := y;
    LRLEditorButton[i].x2  := x + 32;
    LRLEditorButton[i].y2  := y + 9;
    LRLEditorButton[i].Lit := False;
    LRLEditorButton[i].Command := i;
    LRLEditorButton[i].DarkIconSize := 292;
    LRLEditorButton[i].LightIconSize := 292;

    GetMem(LRLEditorButton[i].DarkIcon, LRLEditorButton[i].DarkIconSize);
    GetMem(LRLEditorButton[i].LightIcon, LRLEditorButton[i].LightIconSize);
    BlockRead(ImageFile, LRLEditorButton[i].LightIcon^, 292, l);
    BlockRead(ImageFile, LRLEditorButton[i].DarkIcon^, 292, l);

    Inc(x, 33);
  end;

  { load font }
  GetMem(LRLFont, 20455);
  BlockRead(ImageFile, LRLFont^, 20455, k);

  { load Pointer }
  GetMem(LRLMousePointer, 174);
  BlockRead(ImageFile, LRLMousePointer^, 174, k);

  { load palette }
  GetMem(LRLMenuPalette, 768);
  BlockRead(ImageFile, LRLMenuPalette^, 768, k);

  { load logo }
  GetMem(LRLLogo, 12524);
  BlockRead(ImageFile, LRLLogo^, 12524, k);

  LRLImagesFilePosition := FilePos(ImageFile);
  FreeMem(InBuffer, $FFF0);
end;


procedure ImagesInitialize;
begin
  LoadImages;
  GetMem(LRLScreen, 64000);
  GetMem(LRLFontBuffer, 32000);
end;


procedure ImagesDeinitialize;
var
  i, j, l: Integer;
begin
  FreeMem(LRLPalette, 768);

  for i := 1 to 9 do
  for j := 1 to LRLFigure[1, i].ImageCount do
  begin
    FreeMem(LRLFigure[1, i].Image[j].Data, 104);
    for l := 2 to 4 do
      FreeMem(LRLFigure[l, i].Image[j].Data, LRLFigure[l, i].Image[j].Size);
  end;

  for i := 1 to 1 do
  for j := 1 to LRLDecoration[i].ImageCount do
    FreeMem(LRLDecoration[i].Image[j].Data, 104);

  for i := 1 to 9 do
  for j := 1 to LRLEnvironment[i].ImageCount do
    FreeMem(LRLEnvironment[i].Image[j].Data, 104);

  for i := 1 to 6 do
  begin
    FreeMem(LRLEditorButton[i].DarkIcon, LRLEditorButton[i].DarkIconSize);
    FreeMem(LRLEditorButton[i].LightIcon, LRLEditorButton[i].LightIconSize);
  end;

  FreeMem(LRLFont, 20455);
  FreeMem(LRLMousePointer, 174);
  FreeMem(LRLMenuPalette, 768);
  FreeMem(LRLLogo, 12524);
  FreeMem(LRLScreen, 64000);
  FreeMem(LRLFontBuffer, 32000);

  Close(ImageFile);
end;


end.
