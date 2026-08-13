program ExpSinGraph_CRT;

{ example from K. Jensen, N. Wirth "Pascal User Manual and Report" }

uses
  CRT;

const
  d = 1 / 16;   // 16 lines for interval [x, x + 1]
  s = 32 / 1.5; // 32 character widths for interval [y, y + 1]
  h = 34;       // character position of x-axis
  c = 2 * PI;
  lim = 32;

var
  i, n: Integer;
  x, y: Extended;

  Colors: array [0..14] of Byte = (LightRed, Yellow, LightGreen, LightCyan, LightBlue,
    LightMagenta, White, LightGray, DarkGray,  Magenta, Blue, Cyan, Green, Brown, Red);

  ColorIndex: Integer = 0;

procedure SwitchColor;
begin
  TextColor(Colors[ColorIndex]);
  Inc(ColorIndex);
  if ColorIndex > High(Colors) then
    ColorIndex := Low(Colors);
end;

begin

  CRT_initialization();
  
  for i := 0 to lim do
  begin
    x := d * i;
    y := Exp(-x) * Sin(c * x);
    n := Round(s * y) + h;
    SwitchColor();
    repeat
      Write(' ');
      Dec(n);
    until n = 0;
    WriteLn('*');
  end;
end.