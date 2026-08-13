{
  launch this program with some
  command line arguments
  if argument has spaces then
  need to quote them
  for example:
  CmdLineArgs.kex param1 param2 "p a r a m 3"
  note that ParamStr(0) is always present
  and contents full file path to CmdLineArgs.kex
}

{$APPTYPE CONSOLE}

program CmdLineArgs;

var
  i: LongInt;

begin
  WriteLn('ParamCount = ', ParamCount);
  for i := 0 to ParamCount do
    WriteLn('ParamStr(', i, ') = ', ParamStr(i));
end.