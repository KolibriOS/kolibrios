// Factorization demo

{$APPTYPE CONSOLE}

program Factor;



var
  LowBound, HighBound, Number, Dividend, Divisor: Integer;
  DivisorFound: Boolean;



begin
WriteLn;
WriteLn('Integer factorization demo');
WriteLn;
Write('From number: '); ReadLn(LowBound);
Write('To number  : '); ReadLn(HighBound);
WriteLn;

if LowBound < 2 then
  begin
  WriteLn('Numbers should be greater than 1');
  ReadLn;
  Halt(1);
  end;

for Number := LowBound to HighBound do
  begin
  Write(Number, ' = ');
  
  Dividend := Number;
  while Dividend > 1 do
    begin
    Divisor := 1;
    DivisorFound := FALSE;
    
    while sqr(Divisor) <= Dividend do
      begin
      Inc(Divisor);
      if Dividend mod Divisor = 0 then
        begin
        DivisorFound := TRUE;
        Break;
        end;
      end;
   
    if not DivisorFound then Divisor := Dividend;                // Prime number

    Write(Divisor, ' ');
    Dividend := Dividend div Divisor;
    end; // while
    
  WriteLn;  
  end; // for     

WriteLn;
WriteLn('Done.');

ReadLn;
end.