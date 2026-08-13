// XD Pascal - a 32-bit compiler for Windows
// Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov

{$APPTYPE CONSOLE}
{$I-}
{$H-}

program XDPW;


uses SysUtils, Common, Scanner, Parser, CodeGen, Linker;




procedure SplitPath(const Path: TString; var Folder, Name, Ext: TString);
var
  DotPos, SlashPos, i: Integer;
begin
Folder := '';  
Name := Path;  
Ext := '';

DotPos := 0;  
SlashPos := 0;

for i := Length(Path) downto 1 do
  if (Path[i] = '.') and (DotPos = 0) then 
    DotPos := i
  else if (Path[i] = '\') and (SlashPos = 0) then
    SlashPos := i; 

if DotPos > 0 then
  begin
  Name := Copy(Path, 1, DotPos - 1);
  Ext  := Copy(Path, DotPos, Length(Path) - DotPos + 1);
  end;
  
if SlashPos > 0 then
  begin
  Folder := Copy(Path, 1, SlashPos);
  Name   := Copy(Path, SlashPos + 1, Length(Name) - SlashPos);
  end;  

end;




procedure NoticeProc(ClassInstance: Pointer; const Msg: TString);
begin
WriteLn(Msg);  
end;




procedure WarningProc(ClassInstance: Pointer; const Msg: TString);
begin
if NumUnits >= 1 then
  Notice(ScannerFileName + ' (' + IntToStr(ScannerLine) + ') Warning: ' + Msg)
else
  Notice('Warning: ' + Msg);  
end;




procedure ErrorProc(ClassInstance: Pointer; const Msg: TString);
begin
if NumUnits >= 1 then
  Notice(ScannerFileName + ' (' + IntToStr(ScannerLine) + ') Error: ' + Msg)
else
  Notice('Error: ' + Msg);  

repeat FinalizeScanner until not RestoreScanner;
FinalizeCommon;
Halt(1);
end;




var
  CompilerPath, CompilerFolder, CompilerName, CompilerExt,
  PasPath, PasFolder, PasName, PasExt,
  ExePath: TString;
  


begin
SetWriteProcs(nil, @NoticeProc, @WarningProc, @ErrorProc);

Notice('XD Pascal for Windows ' + VERSION);
Notice('Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov');

if ParamCount < 1 then
  begin
  Notice('Usage: xdpw <file.pas>');
  Halt(1);
  end;
  
CompilerPath := TString(ParamStr(0));
SplitPath(CompilerPath, CompilerFolder, CompilerName, CompilerExt);  

PasPath := TString(ParamStr(1));
SplitPath(PasPath, PasFolder, PasName, PasExt);

InitializeCommon;
InitializeLinker;
InitializeCodeGen;

Folders[1] := PasFolder;
Folders[2] := CompilerFolder + 'units\';
NumFolders := 2;
   
CompileProgramOrUnit('system.pas');
CompileProgramOrUnit(PasName + PasExt);

ExePath := PasFolder + PasName + '.exe';
LinkAndWriteProgram(ExePath);

Notice('Complete. Code size: ' + IntToStr(GetCodeSize) + ' bytes. Data size: ' + IntToStr(InitializedGlobalDataSize + UninitializedGlobalDataSize) + ' bytes');

repeat FinalizeScanner until not RestoreScanner;
FinalizeCommon;
end.

