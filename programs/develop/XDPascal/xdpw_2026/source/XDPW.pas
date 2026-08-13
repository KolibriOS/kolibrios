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




function UnquotedParamStr(Index: Integer; var NextIndex: Integer): TString;
var
  Fragment: TString;
begin
// XD Pascal's own ParseCmdLine splits the command line at spaces only and keeps the quotes,
// so a quoted argument (as passed by the make.bat files) must be reassembled and unquoted here.
// Compilers built by Delphi do this in their own run-time library
Result := TString(ParamStr(Index));
NextIndex := Index + 1;

if (Length(Result) > 0) and (Result[1] = '"') then
  begin
  Result := Copy(Result, 2, Length(Result) - 1);

  // A quoted path may contain spaces and thus be split into several fragments - glue them back
  while (Length(Result) = 0) or (Result[Length(Result)] <> '"') do
    begin
    if NextIndex > ParamCount then Break;                       // Unterminated quote

    Fragment := TString(ParamStr(NextIndex));
    Inc(NextIndex);
    Result := Result + ' ' + Fragment;
    end;

  if (Length(Result) > 0) and (Result[Length(Result)] = '"') then
    Result := Copy(Result, 1, Length(Result) - 1);
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
  NextParamIndex, LastParamIndex: Integer;
  


begin
SetWriteProcs(nil, @NoticeProc, @WarningProc, @ErrorProc);

Notice('XD Pascal for Windows ' + VERSION);
Notice('Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov');

if ParamCount < 1 then
  begin
  Notice('Usage: xdpw <file.pas> [-nosmart]');
  Halt(1);
  end;

CompilerPath := TString(ParamStr(0));
SplitPath(CompilerPath, CompilerFolder, CompilerName, CompilerExt);

PasPath := UnquotedParamStr(1, NextParamIndex);
SplitPath(PasPath, PasFolder, PasName, PasExt);

InitializeSmartLinker;

if NextParamIndex <= ParamCount then
  if UnquotedParamStr(NextParamIndex, LastParamIndex) = '-nosmart' then
    DisableSmartLinking := TRUE;

// First pass: compile everything and build the routine call graph
SuppressWarnings := TRUE;

InitializeCommon;
InitializeLinker;
InitializeCodeGen;

Folders[1] := PasFolder;
Folders[2] := CompilerFolder + 'units\';
NumFolders := 2;

CompileProgramOrUnit('system.pas');
CompileProgramOrUnit(PasName + PasExt);

FinalizeCommon;

// Second pass: recompile, discarding the code of unreachable routines
ComputeLiveProcs;
IsSecondPass := TRUE;
SuppressWarnings := FALSE;

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

