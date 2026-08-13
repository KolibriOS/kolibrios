{$APPTYPE CONSOLE}

program EnterNumber;

var
  Number: LongInt;

begin
  Write('Enter Number please:');
  ReadLn(Number);
  WriteLn('You entered "', Number, '"');
end.