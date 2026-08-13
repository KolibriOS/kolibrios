program INI_File_Read;

uses
  LibINI;
  
procedure IniKeyCallback(FileName, SectionName, KeyName, KeyValue: PAnsiChar) stdcall;
begin
  Write('  ', PString(KeyName)^);
  Write(' = ');
  WriteLn(PString(KeyValue)^);
end;

procedure IniSectionCallback(FileName, SectionName: PAnsiChar) stdcall;
begin
  WriteLn(PString(SectionName)^);
  ini_enum_keys(FileName, SectionName, @IniKeyCallback);
end;

const
  FILE_PATH = '/sys/settings/system.ini';

var
  SystemLang: array [0..255] of AnsiChar;
  MouseSpeed: Integer;

begin
  LibINI_initialization;
  
  WriteLn('Hello, this is a test for LibINI!');
  WriteLn;
  ini_enum_sections(FILE_PATH, @IniSectionCallback);
  WriteLn;
  
  WriteLn('Try to get system.language and mouse.speed:');
  
  if ini_get_str(FILE_PATH, 'system', 'language', SystemLang, SizeOf(SystemLang), 'ru') = 0 then
    WriteLn('Lang = "', SystemLang, '"')
  else
    WriteLn('Lang not found, used default "', SystemLang , '"');
  
  MouseSpeed := ini_get_int(FILE_PATH, 'mouse', 'speed', -1);
  if MouseSpeed <> -1 then
    WriteLn('MouseSpeed = "', MouseSpeed, '"')
  else
  begin
    MouseSpeed := 5;
    WriteLn('MouseSpeed not found, used default "', MouseSpeed , '"');
  end; 
end.
