program ClrEolTest;
 
uses
  CRT;
 
begin

  CRT_initialization;
  
  WriteLn('Hello, this is a ClrEolTest!');
  Delay(500);
  ClrScr;
  Writeln('The function CLREOL clears all characters from the');
  Delay(500);
  Writeln('cursor position to the end of the line within the');
  Delay(500);
  Writeln('current text window, without moving the cursor.');
  Delay(500);
  Writeln('Press any key to continue . . .');
  GotoXY(14, 4);
  ReadKey;
  ClrEOL;
  ReadKey;  
end.
