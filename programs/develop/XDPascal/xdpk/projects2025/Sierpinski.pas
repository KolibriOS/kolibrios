{ see original code here http://rosettacode.org/wiki/Sierpinski_triangle#Pascal }

{$APPTYPE CONSOLE}

program Sierpinski;

function ipow(b, n: Integer): Integer;
var
   i: Integer;
begin
  Result := 1;
  for i := 1 to n do
    Result := Result * b
end;

function Truth(a: Char): Boolean;
begin
  if a = '*' then
    Truth := True
  else
    Truth := False
end;

function Rule_90(ev: string): string;
var
   l, i: Integer;
   cp: string;
   s: array[0..1] of Boolean;
begin
  l := Length(ev);
  cp := Copy(ev, 1, l);
  for i := 1 to l do
  begin
    if (i-1) < 1 then
      s[0] := False
    else
      s[0] := Truth(ev[i - 1]);
    if (i+1) > l then
      s[1] := False
    else
      s[1] := Truth(ev[i + 1]);
    if (s[0] and not s[1]) or (s[1] and not s[0]) then
      cp[i] := '*'
    else
      cp[i] := ' ';
  end;
  rule_90 := cp
end;

procedure Triangle(N: Integer);
var
   i, l : Integer;
   b : string;
begin
   l := ipow(2, n + 1);
   b := ' ';
   for i := 1 to l do
      b := b + ' ';
   b[Round(l / 2)] := '*';
   WriteLn(b);
   for i := 1 to (Round(l / 2) - 1) do
   begin
      b := Rule_90(b);
      WriteLn(b)
   end
end;

begin
   Triangle(4)
end.