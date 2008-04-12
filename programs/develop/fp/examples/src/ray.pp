
{ В FreePascal 2.2.0 кодировка cp866 не реализована. }
{-$codepage cp866}

{$mode objfpc}
{$apptype gui}
{$r-}

program Ray;

const
  MSG_PRESSKEY = 'Нажми на кнопку...';
  MSG_USAGE    = 'Для перемешения и врашения используй стрелки';

  WIDTH  = 320;
  HEIGHT = 200;
  COLORS = 128;

  FlatPalette: array[1..COLORS * 3] of Byte = (
    0,10,20,48,48,48,1,0,43,1,3,43,2,5,44,2,7,44,3,9,45,4,11,46,5,13,47,6,15,48,
    7,17,49,8,19,50,9,21,51,10,22,52,11,24,52,12,26,54,13,28,54,14,30,56,15,32,
    56,16,34,58,17,34,58,17,36,58,18,38,60,19,40,60,20,42,62,21,44,62,10,31,0,
    11,31,0,11,31,1,11,32,1,12,32,1,12,32,2,12,33,2,13,33,2,14,33,3,15,33,3,15,
    34,3,15,34,4,15,35,4,16,35,4,16,35,5,16,36,5,17,36,5,17,36,6,18,37,6,18,38,
    7,19,38,8,20,39,8,20,40,9,21,40,10,22,41,10,22,42,11,23,42,12,24,43,12,24,
    44,13,25,44,14,25,45,14,26,46,15,27,46,16,27,47,17,28,47,18,28,48,19,29,49,
    19,30,49,20,30,50,21,31,51,21,32,51,22,32,52,23,33,53,23,34,53,24,34,54,25,
    35,55,25,36,55,26,36,56,27,37,57,27,38,57,27,39,57,27,41,57,27,42,57,27,43,
    57,27,44,57,27,45,57,27,46,57,27,47,57,27,49,57,27,50,57,27,51,57,27,52,57,
    27,53,57,27,55,57,27,56,57,27,57,57,27,58,57,27,58,57,26,58,57,25,58,57,24,
    58,56,23,58,55,22,58,54,20,58,53,19,58,51,18,58,50,17,58,50,16,58,49,15,58,
    48,14,58,47,13,58,46,12,58,45,11,58,44,11,58,44,10,58,43,10,58,42,9,57,41,
    8,57,40,8,56,39,7,56,38,6,55,37,5,55,35,4,54,33,4,54,31,2,32,32,32,63,63,63,
    63,63,63,63,63,63,63,63,63,48,48,48,63,63,63,63,63,63);

type
  TRGBColor = packed record
    R, G, B: Byte;
  end;

  PRGBPalette = ^TRGBPalette;
  TRGBPalette = array[0..COLORS - 1] of TRGBColor;

  PRGBBuffer = ^TRGBBuffer;
  TRGBBuffer = array[0..HEIGHT - 1, 0..WIDTH - 1] of TRGBColor;

  lrgarr = array[Word] of Byte;
  sq     = array[0..254, 0..255] of Byte;


var
  mp: ^lrgarr;
  rng: array[0..320] of Byte;
  fcos, fsin: array[0..359] of Integer;

  RGBBuffer : PRGBBuffer;
  RGBPalette: TRGBPalette absolute FlatPalette;

  Message: String = '';


function NCol(mc, n, dvd: Longint): Byte;
var
  loc: Byte;
begin
  loc := Byte((mc + n - Random(2 * n)) div dvd);
  if loc > 100 then Result := 100 else
  if loc < 5   then Result := 5   else
                    Result := loc;
end;


procedure Plasma(x1, y1, x2, y2: Word);
var
  xn, yn, dxy   : Word;
  p1, p2, p3, p4: Word;
begin
  if (x2 - x1 > 1) or (y2 - y1 > 1) then
  begin
    p1 := mp^[Word(y1 shl 8 + x1)];
    p2 := mp^[Word(y2 shl 8 + x1)];
    p3 := mp^[Word(y1 shl 8 + x2)];
    p4 := mp^[Word(y2 shl 8 + x2)];
    xn := (x2 + x1) shr 1;
    yn := (y2 + y1) shr 1;
    dxy:= 5 * (x2 - x1 + y2 - y1) div 3;

    if mp^[y1 shl 8 + xn] = 0 then mp^[Word(y1 shl 8 + xn)] := NCol(p1 + p3, dxy, 2);
    if mp^[yn shl 8 + x1] = 0 then mp^[Word(yn shl 8 + x1)] := NCol(p1 + p2, dxy, 2);
    if mp^[yn shl 8 + x2] = 0 then mp^[Word(yn shl 8 + x2)] := NCol(p3 + p4, dxy, 2);
    if mp^[y2 shl 8 + xn] = 0 then mp^[Word(y2 shl 8 + xn)] := NCol(p2 + p4, dxy, 2);
    mp^[Word(yn shl 8 + xn)] := NCol(p1 + p2 + p3 + p4, dxy, 4);

    Plasma(x1, y1, xn, yn);
    Plasma(xn, y1, x2, yn);
    Plasma(x1, yn, xn, y2);
    Plasma(xn, yn, x2, y2);
  end;
end;


procedure Draw(xp, yp, dir: Integer);
var
  z, zobs               : Integer;
  ix, iy, iy1, iyp, ixp : Integer;
  x, y                  : Integer;
  s, csf, snf, mpc, i, j: Integer;
begin
  while dir <  0            do Inc(dir, SizeOf(fcos));
  while dir >= SizeOf(fcos) do Dec(dir, SizeOf(fcos));

  FillChar(rng, SizeOf(rng), 200);
  FillChar(RGBBuffer^, SizeOf(RGBBuffer^), 0);

  zobs := 300 + mp^[Word(yp shl 8 + xp)];

  csf := fcos[dir];
  snf := fsin[dir];

  for iy := yp to yp+150 do
  begin
    iy1 := 1 + 2 * (iy - yp);
    s   := 4 + 300 div iy1;

    for ix := xp + yp - iy to xp - yp + iy do
    begin
      ixp := xp + ((ix - xp) * csf + (iy - yp) * snf) shr 8;
      iyp := yp + ((iy - yp) * csf - (ix - xp) * snf) shr 8;
      x := 160 + 360 * (ix - xp) div iy1;

      if (x >= 0) and (x + s < 319) then
      begin
        z   := mp^[Word(iyp shl 8 + ixp)];
        mpc := z shr 1;

        if z < 40 then z := 40;
        y := 100 + (zobs - z) * 30 div iy1;

        if (y < 200) and (y >= 0) then
        for j := x to x + s do
        if y < rng[j] then
        begin
          for i := y to rng[j] do
            RGBBuffer^[i - 1, j] := RGBPalette[mpc];
          rng[j] := y;
        end;
      end;
    end;
  end;
end;


procedure Paint;
begin
  kos_begindraw();
  kos_definewindow(100, 100, WIDTH - 1, HEIGHT - 1, $01000000);
  kos_drawimage24(0, 0, WIDTH, HEIGHT, RGBBuffer);
  if Message <> '' then
    kos_drawtext(12, HEIGHT - 12 - 9, Message, $00FF00, $FF000000);
  kos_enddraw();
end;


function ReadKey: Char;
var
  Event: Word;
begin
  kos_maskevents(ME_PAINT or ME_KEYBOARD);
  repeat
    Event := kos_getevent();
    if Event = SE_PAINT then Paint;
  until Event = SE_KEYBOARD;
  Result := Chr(kos_getkey() shr 8);
end;


procedure Pause;
begin
  kos_maskevents(ME_PAINT or ME_KEYBOARD);
  Message := MSG_PRESSKEY;
  Paint;
  ReadKey;
end;


var
  dir, i, j, x, y: Longint;
  C: Char;
  B: Byte;
  Terminate: Boolean;

begin
  { таблицы значений синуса и косинуса }
  for i := 0 to 359 do
  begin
    fcos[i] := Trunc(256 * Cos(i / 180 * Pi));
    fsin[i] := Trunc(256 * Sin(i / 180 * Pi));
  end;

  { создаем буфер с эффектом "плазма" }
  New(mp);
  FillChar(mp^, SizeOf(mp^), 0);
  mp^[$0000] := 128;
  Plasma(0, 0, 256, 256);

  { создаем "пустой" буфер кадра }
  New(RGBBuffer);
  FillChar(RGBBuffer^, SizeOf(RGBBuffer^), 0);

  { преобразование палитры из 63 в 255 }

  for i := 0 to COLORS - 1 do
  begin
    B := Round(RGBPalette[i].R / 63 * 255);
    RGBPalette[i].R := Round(RGBPalette[i].B / 63 * 255);
    RGBPalette[i].G := Round(RGBPalette[i].G / 63 * 255);
    RGBPalette[i].B := B;
  end;

  for j := 0 to 199 do
    for i := 0 to 255 do
      RGBBuffer^[j, i + (WIDTH - 256) shr 1] := RGBPalette[sq(Pointer(mp)^)[j, i]];

  Pause;

  x   := 0;
  y   := 0;
  dir := 0;

  Message := MSG_USAGE;

  Terminate := False;
  while not Terminate do
  begin
    dir := dir mod 360;
    if dir < 0 then dir := 360 + dir;

    Draw(x, y, dir);
    Paint;

    C := ReadKey;
    if C = #$B0 then Dec(dir, 13) else
    if C = #$B3 then Inc(dir, 13) else
    if C = #$B2 then
    begin
      y := y + fcos[dir] shr 6;
      x := x + fsin[dir] shr 6;
    end else
    if C = #$B1 then
    begin
      y := y - fcos[dir] shr 6;
      x := x - fsin[dir] shr 6;
    end;
    if C = #27 then Terminate := True;
  end;
end.
