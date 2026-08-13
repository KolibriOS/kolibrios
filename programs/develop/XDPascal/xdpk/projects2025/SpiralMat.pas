{ see original code here http://rosettacode.org/wiki/Spiral_matrix#Pascal }

{$APPTYPE CONSOLE}

program Spiralmat;

type
  TDir = (Left, Down, Right, Up);

  TDXY = record
           DX, DY: LongInt;
         end;

  TDeltaDir = array[TDir] of TDXY;

const
  Nextdir: array[TDir] of TDir = (Down, Right, Up, Left);
  cDir: TDeltaDir = ((dx: 1; dy: 0), (dx: 0; dy: 1), (dx: -1; dy: 0), (dx: 0; dy: -1));
  cMaxN = 32;

type
  TSpiral =  array[0..cMaxN, 0..cMaxN] of LongInt;

function FillSpiral(n: LongInt): TSpiral;
var
  b, i, k, dn, x, y : LongInt;
  dir: TDir;
  tmpSp: TSpiral;
begin
  b := 0;
  x := 0;
  y := 0;
  //only for the first line
  k := -1;
  dn := n - 1;
  tmpSp[x, y] := b;
  dir :=  left;
  repeat
    i := 0;
    while i < dn do
    begin
      inc(b);
      tmpSp[x,y] := b;
      x := x + cDir[dir].dx;
      y := y + cDir[dir].dy;
      Inc(i);
    end;
    Dir := NextDir[dir];
    Inc(k);
    if k > 1 then
    begin
      k := 0;
      //shorten the line every second direction change
      dn := dn-1;
      if dn <= 0 then
        Break;
    end;
  until false;
  //the last
  tmpSp[x, y] := b + 1;
  FillSpiral := tmpSp;
end;

var
  a: TSpiral;
  x, y, n: LongInt;
begin
  for n := 1 to 5 do
  begin
    A := FillSpiral(n);
    for y := 0 to n - 1 do
    begin
      for x := 0 to n - 1 do
        Write(A[x, y]:4);
      WriteLn;
    end;
    WriteLn;
  end;
end.