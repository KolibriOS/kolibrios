{TODO}

function SysOSAlloc(Size: PtrInt): Pointer;
begin
  Result := kos_alloc(Size);
end;

{$define HAS_SYSOSFREE}
procedure SysOSFree(P: Pointer; Size: PtrInt);
begin
  kos_free(P);
end;

(*
{DEBUG version}

var
  SysMemoryBlocks: array[Byte] of record
    Used: Boolean;
    Address: Pointer;
    Size: Longint;
  end;

function SysOSAlloc(Size: PtrInt): Pointer;
var
  I: Longint;
begin
  Result := kos_alloc(Size);

  for I := 0 to High(SysMemoryBlocks) do
  if not SysMemoryBlocks[I].Used then
  begin
    SysMemoryBlocks[I].Used := True;
    SysMemoryBlocks[I].Address := Result;
    SysMemoryBlocks[I].Size := Size;
    Break;
  end;
end;

{$define HAS_SYSOSFREE}
procedure SysOSFree(P: Pointer; Size: PtrInt);
var
  B: Byte;
  I: Longint;
begin
  B := 0;
  for I := 0 to High(SysMemoryBlocks) do
  if SysMemoryBlocks[I].Address = P then
  begin
    SysMemoryBlocks[I].Used := False;
    if SysMemoryBlocks[I].Size <> Size then B := 1 div B;
    Break;
  end;

  kos_free(P);
end;*)
