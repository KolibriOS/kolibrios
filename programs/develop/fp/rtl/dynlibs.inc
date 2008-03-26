{TODO}

{$ifdef readinterface}


type
  TLibHandle = Longint;

const
  NilHandle = 0;


{$else}


function LoadLibrary(Name: AnsiString): TLibHandle;
begin
  Result := 0;
end;

function GetProcedureAddress(Lib: TLibHandle; ProcName: AnsiString): Pointer;
begin
  Result := nil;
end;

function UnloadLibrary(Lib: TLibHandle): Boolean;
begin
  Result := False
end;

{$endif}
