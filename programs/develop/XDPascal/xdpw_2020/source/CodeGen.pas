// XD Pascal - a 32-bit compiler for Windows
// Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov

{$I-}
{$H-}

unit CodeGen;


interface


uses Common;


const
  MAXCODESIZE =  1 * 1024 * 1024;


var
  Code: array [0..MAXCODESIZE - 1] of Byte;


procedure InitializeCodeGen;
function GetCodeSize: LongInt;
procedure PushConst(Value: LongInt);
procedure PushRealConst(Value: Double);
procedure PushRelocConst(Value: LongInt; RelocType: TRelocType);
procedure Relocate(CodeDeltaAddr, InitDataDeltaAddr, UninitDataDeltaAddr, ImportDeltaAddr: Integer);
procedure PushFunctionResult(ResultType: Integer);
procedure MoveFunctionResultFromFPUToEDXEAX(DataType: Integer);
procedure MoveFunctionResultFromEDXEAXToFPU(DataType: Integer);
procedure PushVarPtr(Addr: Integer; Scope: TScope; DeltaNesting: Byte; RelocType: TRelocType);
procedure DerefPtr(DataType: Integer);
procedure GetArrayElementPtr(ArrType: Integer);
procedure GetFieldPtr(Offset: Integer);
procedure GetCharAsTempString(Depth: Integer);
procedure SaveStackTopToEAX;
procedure RestoreStackTopFromEAX;
procedure SaveStackTopToEDX;
procedure RestoreStackTopFromEDX;
procedure RaiseStackTop(NumItems: Byte);
procedure DiscardStackTop(NumItems: Byte);
procedure DuplicateStackTop;
procedure SaveCodePos;
procedure GenerateIncDec(proc: TPredefProc; Size: Byte; BaseTypeSize: Integer = 0);
procedure GenerateRound(TruncMode: Boolean);
procedure GenerateDoubleFromInteger(Depth: Byte);
procedure GenerateDoubleFromSingle;
procedure GenerateSingleFromDouble;
procedure GenerateMathFunction(func: TPredefProc; ResultType: Integer);
procedure GenerateUnaryOperator(op: TTokenKind; ResultType: Integer);
procedure GenerateBinaryOperator(op: TTokenKind; ResultType: Integer);
procedure GenerateRelation(rel: TTokenKind; ValType: Integer);
procedure GenerateAssignment(DesignatorType: Integer);
procedure GenerateForAssignmentAndNumberOfIterations(CounterType: Integer; Down: Boolean);
procedure GenerateStructuredAssignment(DesignatorType: Integer);
procedure GenerateInterfaceFieldAssignment(Offset: Integer; PopValueFromStack: Boolean; Value: LongInt; RelocType: TRelocType);
procedure InitializeCStack;
procedure PushToCStack(SourceStackDepth: Integer; DataType: Integer; PushByValue: Boolean);
procedure ConvertSmallStructureToPointer(Addr: LongInt; Size: LongInt);
procedure ConvertPointerToSmallStructure(Size: LongInt);
procedure GenerateImportFuncStub(EntryPoint: LongInt);
procedure GenerateCall(EntryPoint: LongInt; CallerNesting, CalleeNesting: Integer);
procedure GenerateIndirectCall(CallAddressDepth: Integer);
procedure GenerateReturn(TotalParamsSize, Nesting: Integer);
procedure GenerateForwardReference;
procedure GenerateForwardResolution(CodePos: Integer);
procedure GenerateIfCondition;
procedure GenerateIfProlog;
procedure GenerateElseProlog;
procedure GenerateIfElseEpilog;
procedure GenerateCaseProlog;
procedure GenerateCaseEpilog(NumCaseStatements: Integer);
procedure GenerateCaseEqualityCheck(Value: LongInt);
procedure GenerateCaseRangeCheck(Value1, Value2: LongInt);
procedure GenerateCaseStatementProlog;
procedure GenerateCaseStatementEpilog;
procedure GenerateWhileCondition;
procedure GenerateWhileProlog;
procedure GenerateWhileEpilog;
procedure GenerateRepeatCondition;
procedure GenerateRepeatProlog;
procedure GenerateRepeatEpilog;
procedure GenerateForCondition;
procedure GenerateForProlog;
procedure GenerateForEpilog(CounterType: Integer; Down: Boolean);
procedure GenerateGotoProlog;
procedure GenerateGoto(LabelIndex: Integer);
procedure GenerateGotoEpilog;
procedure GenerateShortCircuitProlog(op: TTokenKind);
procedure GenerateShortCircuitEpilog;
procedure GenerateNestedProcsProlog;
procedure GenerateNestedProcsEpilog;
procedure GenerateFPUInit;
procedure GenerateStackFrameProlog(PreserveRegs: Boolean);
procedure GenerateStackFrameEpilog(TotalStackStorageSize: LongInt; PreserveRegs: Boolean);
procedure GenerateBreakProlog(LoopNesting: Integer);
procedure GenerateBreakCall(LoopNesting: Integer);
procedure GenerateBreakEpilog(LoopNesting: Integer);
procedure GenerateContinueProlog(LoopNesting: Integer);
procedure GenerateContinueCall(LoopNesting: Integer);
procedure GenerateContinueEpilog(LoopNesting: Integer);
procedure GenerateExitProlog;
procedure GenerateExitCall;
procedure GenerateExitEpilog;



implementation


const
  MAXINSTRSIZE      = 15;
  MAXPREVCODESIZES  = 10;
  MAXRELOCS         = 20000;
  MAXGOTOS          = 100;
  MAXLOOPNESTING    = 20;
  MAXBREAKCALLS     = 100;
  


type
  TRelocatable = record
    RelocType: TRelocType;
    Pos: LongInt;
    Value: LongInt;
  end;
  
  TGoto = record
    Pos: LongInt;
    LabelIndex: Integer;
    ForLoopNesting: Integer;
  end;  
  
  TBreakContinueExitCallList = record
    NumCalls: Integer;
    Pos: array [1..MAXBREAKCALLS] of LongInt;
  end; 
   
  TRegister = (EAX, ECX, EDX, ESI, EDI, EBP);

  
var
  CodePosStack: array [0..1023] of Integer;
  CodeSize, CodePosStackTop: Integer;
  
  PrevCodeSizes: array [1..MAXPREVCODESIZES] of Integer;
  NumPrevCodeSizes: Integer;
  
  Reloc: array [1..MAXRELOCS] of TRelocatable;
  NumRelocs: Integer;

  Gotos: array [1..MAXGOTOS] of TGoto;
  NumGotos: Integer;
  
  BreakCall, ContinueCall: array [1..MAXLOOPNESTING] of TBreakContinueExitCallList;
  ExitCall: TBreakContinueExitCallList;   



procedure InitializeCodeGen;
begin
CodeSize         := 0; 
CodePosStackTop  := 0;
NumPrevCodeSizes := 0;
NumRelocs        := 0;
NumGotos         := 0;
end;




function GetCodeSize: LongInt;
begin
Result := CodeSize;
NumPrevCodeSizes := 0;
end;
 


 
procedure Gen(b: Byte);
begin
Code[CodeSize] := b;
Inc(CodeSize);
end;




procedure GenNew(b: Byte);
var
  i: Integer;
begin
if CodeSize + MAXINSTRSIZE >= MAXCODESIZE then
  Error('Maximum code size exceeded');
  
if NumPrevCodeSizes < MAXPREVCODESIZES then
  Inc(NumPrevCodeSizes)
else
  for i := 1 to MAXPREVCODESIZES - 1 do
    PrevCodeSizes[i] := PrevCodeSizes[i + 1]; 

PrevCodeSizes[NumPrevCodeSizes] := CodeSize;  

Gen(b);
end;




procedure GenAt(Pos: LongInt; b: Byte);
begin
Code[Pos] := b;
end;




procedure GenWord(w: Integer);
var
  i: Integer;
begin
for i := 1 to 2 do
  begin
  Gen(Byte(w and $FF));
  w := w shr 8;
  end;
end;




procedure GenWordAt(Pos: LongInt; w: Integer);
var
  i: Integer;
begin
for i := 0 to 1 do
  begin
  GenAt(Pos + i, Byte(w and $FF));
  w := w shr 8;
  end;
end;




procedure GenDWord(dw: LongInt);
var
  i: Integer;
begin
for i := 1 to 4 do
  begin
  Gen(Byte(dw and $FF));
  dw := dw shr 8;
  end;
end;




procedure GenDWordAt(Pos: LongInt; dw: LongInt);
var
  i: Integer;
begin
for i := 0 to 3 do
  begin
  GenAt(Pos + i, Byte(dw and $FF));
  dw := dw shr 8;
  end;
end;




procedure GenRelocDWord(dw: LongInt; RelocType: TRelocType);
begin
Inc(NumRelocs);
if NumRelocs > MAXRELOCS then
  Error('Maximum number of relocations exceeded');  
Reloc[NumRelocs].RelocType := RelocType;
Reloc[NumRelocs].Pos := CodeSize;
Reloc[NumRelocs].Value := dw;

GenDWord(dw);
end;




function PrevInstrByte(Depth, Offset: Integer): Byte;
begin
Result := 0;

// The last generated instruction starts at Depth = 0, Offset = 0
if Depth < NumPrevCodeSizes then
  Result := Code[PrevCodeSizes[NumPrevCodeSizes - Depth] + Offset];
end;




function PrevInstrDWord(Depth, Offset: Integer): LongInt;
begin
Result := 0;

// The last generated instruction starts at Depth = 0, Offset = 0
if Depth < NumPrevCodeSizes then
  Result := PLongInt(@Code[PrevCodeSizes[NumPrevCodeSizes - Depth] + Offset])^;
end;




function PrevInstrRelocDWordIndex(Depth, Offset: Integer): Integer;
var
  i: Integer;
  Pos: LongInt;
begin
Result := 0;

// The last generated instruction starts at Depth = 0, Offset = 0
if Depth < NumPrevCodeSizes then
  begin
  Pos := PrevCodeSizes[NumPrevCodeSizes - Depth] + Offset;
  for i := NumRelocs downto 1 do
    if Reloc[i].Pos = Pos then
      begin
      Result := i;
      Exit;
      end;
  end;
end;




procedure RemovePrevInstr(Depth: Integer);
begin
if Depth >= NumPrevCodeSizes then
  Error('Internal fault: previous instruction not found');

CodeSize := PrevCodeSizes[NumPrevCodeSizes - Depth];  
NumPrevCodeSizes := NumPrevCodeSizes - Depth - 1;
end;    




procedure PushConst(Value: LongInt);
begin
GenNew($68); GenDWord(Value);                            // push Value
end;




procedure PushRealConst(Value: Double);
type
  TIntegerArray = array [0..1] of LongInt;
  PIntegerArray = ^TIntegerArray;
  
var
  IntegerArray: PIntegerArray;
  
begin
IntegerArray := PIntegerArray(@Value);
PushConst(IntegerArray^[1]);
PushConst(IntegerArray^[0]);
end;




procedure PushRelocConst(Value: LongInt; RelocType: TRelocType);
begin
GenNew($68); GenRelocDWord(Value, RelocType);            // push Value  ; relocatable
end;




procedure Relocate(CodeDeltaAddr, InitDataDeltaAddr, UninitDataDeltaAddr, ImportDeltaAddr: Integer);
var
  i, DeltaAddr: Integer;
begin
DeltaAddr := 0;

for i := 1 to NumRelocs do
  begin
  case Reloc[i].RelocType of
    CODERELOC:        DeltaAddr := CodeDeltaAddr;
    INITDATARELOC:    DeltaAddr := InitDataDeltaAddr;
    UNINITDATARELOC:  DeltaAddr := UninitDataDeltaAddr;
    IMPORTRELOC:      DeltaAddr := ImportDeltaAddr
  else 
    Error('Internal fault: Illegal relocation type');
  end;  
  
  GenDWordAt(Reloc[i].Pos, Reloc[i].Value + DeltaAddr);
  end;
end;




procedure GenPushReg(Reg: TRegister);
begin  
case Reg of
  EAX: GenNew($50);            // push eax
  ECX: GenNew($51);            // push ecx
  EDX: GenNew($52);            // push edx
  ESI: GenNew($56);            // push esi
  EDI: GenNew($57);            // push edi
  EBP: GenNew($55)             // push ebp
else
  Error('Internal fault: Illegal register');  
end;
end;  




procedure GenPopReg(Reg: TRegister);


  function OptimizePopReg: Boolean;
  var
    HasPushRegPrefix: Boolean;
    Value, Addr: LongInt;
    ValueRelocIndex: Integer;
    PrevOpCode: Byte;
    
  begin
  Result := FALSE;
  PrevOpCode := PrevInstrByte(0, 0);

      
  // Optimization: (push Reg) + (pop Reg) -> 0
  if ((Reg = EAX) and (PrevOpCode = $50)) or                                    // Previous: push eax
     ((Reg = ECX) and (PrevOpCode = $51)) or                                    // Previous: push ecx
     ((Reg = EDX) and (PrevOpCode = $52)) or                                    // Previous: push edx
     ((Reg = ESI) and (PrevOpCode = $56)) or                                    // Previous: push esi      
     ((Reg = EDI) and (PrevOpCode = $57)) or                                    // Previous: push edi      
     ((Reg = EBP) and (PrevOpCode = $55))                                       // Previous: push ebp
  then                                 
    begin
    RemovePrevInstr(0);                                                         // Remove: push Reg
    Result := TRUE;
    Exit;
    end
    
                                                 
  // Optimization: (push eax) + (pop ecx) -> (mov ecx, eax)
  else if (Reg = ECX) and (PrevOpCode = $50) then                               // Previous: push eax    
    begin
    RemovePrevInstr(0);                                                         // Remove: push eax
    GenNew($89); Gen($C1);                                                      // mov ecx, eax
    Result := TRUE;
    Exit;
    end
    
        
  // Optimization: (push eax) + (pop esi) -> (mov esi, eax)
  else if (Reg = ESI) and (PrevOpCode = $50) then                               // Previous: push esi        
    begin
    RemovePrevInstr(0);                                                         // Remove: push eax

    // Special case: (mov eax, [epb + Addr]) + (push eax) + (pop esi) -> (mov esi, [epb + Addr])
    if (PrevInstrByte(0, 0) = $8B) and (PrevInstrByte(0, 1) = $85) then         // Previous: mov eax, [epb + Addr]
      begin
      Addr := PrevInstrDWord(0, 2);
      RemovePrevInstr(0);                                                       // Remove: mov eax, [epb + Addr]
      GenNew($8B); Gen($B5); GenDWord(Addr);                                    // mov esi, [epb + Addr]
      end
    else
      begin                                       
      GenNew($89); Gen($C6);                                                    // mov esi, eax
      end;
      
    Result := TRUE;
    Exit;
    end
    
        
  // Optimization: (push esi) + (pop eax) -> (mov eax, esi)
  else if (Reg = EAX) and (PrevOpCode = $56) then                               // Previous: push esi        
    begin
    RemovePrevInstr(0);                                                         // Remove: push esi                                       
    GenNew($89); Gen($F0);                                                      // mov eax, esi
    Result := TRUE;
    Exit;
    end           


  // Optimization: (push Value) + (pop eax) -> (mov eax, Value)
  else if (Reg = EAX) and (PrevOpCode = $68) then                               // Previous: push Value                                                      
    begin
    Value := PrevInstrDWord(0, 1);
    ValueRelocIndex := PrevInstrRelocDWordIndex(0, 1);

    // Special case: (push esi) + (push Value) + (pop eax) -> (mov eax, Value) + (push esi)
    HasPushRegPrefix := PrevInstrByte(1, 0) = $56;                              // Previous: push esi 
    
    RemovePrevInstr(0);                                                         // Remove: push Value                                       
        
    if HasPushRegPrefix then                                                
      RemovePrevInstr(0);                                                       // Remove: push esi
       
    GenNew($B8); GenDWord(Value);                                               // mov eax, Value
    
    if HasPushRegPrefix then
      begin
      if ValueRelocIndex <> 0 then Dec(Reloc[ValueRelocIndex].Pos);             // Relocate Value if necessary                                                                               
      GenPushReg(ESI);                                                          // push esi
      end;
    
    Result := TRUE;
    Exit;
    end
    
    
  // Optimization: (push [esi]) + (pop eax) -> (mov eax, [esi])
  else if (Reg = EAX) and (PrevInstrByte(0, 0) = $FF) and (PrevInstrByte(0, 1) = $36) then    // Previous: push [esi]         
    begin 
    RemovePrevInstr(0);                                                         // Remove: push [esi]
    GenNew($8B); Gen($06);                                                      // mov eax, [esi]      
    Result := TRUE;
    Exit;
    end


  // Optimization: (push [esi + 4]) + (mov eax, [esi]) + (pop edx) -> (mov eax, [esi]) + (mov edx, [esi + 4])
  else if (Reg = EDX) and (PrevInstrByte(1, 0) = $FF) and (PrevInstrByte(1, 1) = $76) and (PrevInstrByte(1, 2) = $04)  // Previous: push [esi + 4]
                      and (PrevInstrByte(0, 0) = $8B) and (PrevInstrByte(0, 1) = $06)                                  // Previous: mov eax, [esi]         
  then  
    begin 
    RemovePrevInstr(1);                                                         // Remove: push [esi + 4], mov eax, [esi] 
    GenNew($8B); Gen($06);                                                      // mov eax, [esi]
    GenNew($8B); Gen($56); Gen($04);                                            // mov edx, [esi + 4]      
    Result := TRUE;
    Exit;
    end
   
    
    
  // Optimization: (push Value) + (pop ecx) -> (mov ecx, Value)  
  else if (Reg = ECX) and (PrevOpCode = $68) then                               // Previous: push Value
    begin
    Value := PrevInstrDWord(0, 1);
    ValueRelocIndex := PrevInstrRelocDWordIndex(0, 1); 

    // Special case: (push eax) + (push Value) + (pop ecx) -> (mov ecx, Value) + (push eax)
    HasPushRegPrefix := PrevInstrByte(1, 0) = $50;                                          // Previous: push eax
    
    RemovePrevInstr(0);                                                         // Remove: push Value                                       
        
    if HasPushRegPrefix then                                                
      RemovePrevInstr(0);                                                       // Remove: push eax / push [ebp + Addr] 
    
    GenNew($B9); GenDWord(Value);                                               // mov ecx, Value
    
    if HasPushRegPrefix then
      begin
      if ValueRelocIndex <> 0 then Dec(Reloc[ValueRelocIndex].Pos);             // Relocate Value if necessary 
      GenPushReg(EAX);                                                          // push eax
      end;
      
    Result := TRUE;
    Exit;
    end
    

  // Optimization: (push Value) + (pop esi) -> (mov esi, Value)  
  else if (Reg = ESI) and (PrevOpCode = $68) then                             // Previous: push Value
    begin
    Value := PrevInstrDWord(0, 1);       
    RemovePrevInstr(0);                                                       // Remove: push Value                                       
    GenNew($BE); GenDWord(Value);                                             // mov esi, Value
    Result := TRUE;
    Exit;
    end


  // Optimization: (push Value) + (mov eax, [Addr]) + (pop esi) -> (mov esi, Value) + (mov eax, [Addr])  
  else if (Reg = ESI) and (PrevInstrByte(1, 0) = $68) and (PrevInstrByte(0, 0) = $A1) then  // Previous: push Value, mov eax, [Addr]
    begin    
    Value := PrevInstrDWord(1, 1);
    Addr  := PrevInstrDWord(0, 1);   
    RemovePrevInstr(1);                                                       // Remove: push Value, mov eax, [Addr]
                                       
    GenNew($BE); GenDWord(Value);                                             // mov esi, Value
    GenNew($A1); GenDWord(Addr);                                              // mov eax, [Addr]
    
    Result := TRUE;
    Exit;
    end

        
  // Optimization: (push esi) + (mov eax, [ebp + Value]) + (pop esi) -> (mov eax, [ebp + Value])
  else if (Reg = ESI) and (PrevInstrByte(1, 0) = $56)                                             // Previous: push esi
                      and (PrevInstrByte(0, 0) = $8B) and (PrevInstrByte(0, 1) = $85)             // Previous: mov eax, [ebp + Value]
  then        
    begin
    Value := PrevInstrDWord(0, 2);    
    RemovePrevInstr(1);                                                       // Remove: push esi, mov eax, [ebp + Value]
    GenNew($8B); Gen($85); GenDWord(Value);                                   // mov eax, [ebp + Value]      
    Result := TRUE;
    Exit;
    end
    
    
  // Optimization: (push dword ptr [esp]) + (pop esi) -> (mov esi, [esp])
  else if (Reg = ESI) and (PrevInstrByte(0, 0) = $FF) and (PrevInstrByte(0, 1) = $34) and (PrevInstrByte(0, 2) = $24) // Previous: push dword ptr [esp] 
  then        
    begin   
    RemovePrevInstr(0);                                                       // Remove: push dword ptr [esp]
    GenNew($8B); Gen($34); Gen($24);                                          // mov esi, [esp]      
    Result := TRUE;
    Exit;
    end

     
  end;


begin // GenPopReg
if not OptimizePopReg then
  case Reg of
    EAX: GenNew($58);            // pop eax
    ECX: GenNew($59);            // pop ecx
    EDX: GenNew($5A);            // pop edx
    ESI: GenNew($5E);            // pop esi
    EDI: GenNew($5F);            // pop edi
    EBP: GenNew($5D)             // pop ebp
  else
    Error('Internal fault: Illegal register');  
  end;

end;




procedure GenPushToFPU;


  function OptimizeGenPushToFPU: Boolean;
  begin
  Result := FALSE;
  
  // Optimization: (fstp qword ptr [esp]) + (fld qword ptr [esp]) -> (fst qword ptr [esp])
  if (PrevInstrByte(0, 0) = $DD) and (PrevInstrByte(0, 1) = $1C) and (PrevInstrByte(0, 2) = $24) then    // Previous: fstp dword ptr [esp]
    begin
    RemovePrevInstr(0);                                                  // Remove: fstp dword ptr [esp]
    GenNew($DD); Gen($14); Gen($24);                                     // fst qword ptr [esp]
    Result := TRUE;
    end

  // Optimization: (push [esi + 4]) + (push [esi]) + (fld qword ptr [esp]) -> (fld qword ptr [esi]) + (sub esp, 8)
  else if (PrevInstrByte(1, 0) = $FF) and (PrevInstrByte(1, 1) = $76) and (PrevInstrByte(1, 2) = $04) and     // Previous: push [esi + 4]
          (PrevInstrByte(0, 0) = $FF) and (PrevInstrByte(0, 1) = $36)                                         // Previous: push [esi]
  then    
    begin
    RemovePrevInstr(1);                                                  // Remove: push [esi + 4], push [esi]
    GenNew($DD); Gen($06);                                               // fld qword ptr [esi]
    RaiseStackTop(2);                                                    // sub esp, 8
    Result := TRUE;
    end;
 
  end;
  
  
begin
if not OptimizeGenPushToFPU then
  begin
  GenNew($DD); Gen($04); Gen($24);                                       // fld qword ptr [esp]
  end;
end;




procedure GenPopFromFPU;
begin
GenNew($DD); Gen($1C); Gen($24);                                         // fstp qword ptr [esp]
end; 




procedure PushFunctionResult(ResultType: Integer);
begin
if Types[ResultType].Kind = REALTYPE then
  GenPushReg(EDX)                                                        // push edx
else if Types[ResultType].Kind = BOOLEANTYPE then
  begin
  GenNew($83); Gen($E0); Gen($01);                                       // and eax, 1
  end
else  
  case TypeSize(ResultType) of
  
    1: if Types[ResultType].Kind in UnsignedTypes then
         begin
         GenNew($0F); Gen($B6); Gen($C0);                                // movzx eax, al
         end
       else  
         begin
         GenNew($0F); Gen($BE); Gen($C0);                                // movsx eax, al
         end; 
         
    2: if Types[ResultType].Kind in UnsignedTypes then
         begin
         GenNew($0F); Gen($B7); Gen($C0);                                // movzx eax, ax
         end
       else  
         begin
         GenNew($0F); Gen($BF); Gen($C0);                                // movsx eax, ax
         end; 
     
  end; // case
  
GenPushReg(EAX);                                                         // push eax
end;




procedure MoveFunctionResultFromFPUToEDXEAX(DataType: Integer);
begin
if Types[DataType].Kind = REALTYPE then
  begin
  RaiseStackTop(2);                                                        // sub esp, 8            ;  expand stack
  GenPopFromFPU;                                                           // fstp qword ptr [esp]  ;  [esp] := st;  pop
  GenNew($8B); Gen($04); Gen($24);                                         // mov eax, [esp]
  GenNew($8B); Gen($54); Gen($24); Gen($04);                               // mov edx, [esp + 4]
  DiscardStackTop(2);                                                      // add esp, 8            ;  shrink stack
  end
else if Types[DataType].Kind = SINGLETYPE then
  begin
  RaiseStackTop(1);                                                        // sub esp, 4            ;  expand stack
  GenNew($D9); Gen($1C); Gen($24);                                         // fstp dword ptr [esp]  ;  [esp] := single(st);  pop
  GenNew($8B); Gen($04); Gen($24);                                         // mov eax, [esp]
  DiscardStackTop(1);                                                      // add esp, 4            ;  shrink stack
  end 
else
  Error('Internal fault: Illegal type'); 
end;




procedure MoveFunctionResultFromEDXEAXToFPU(DataType: Integer);
begin
if Types[DataType].Kind = REALTYPE then
  begin
  GenPushReg(EDX);                                                       // push edx
  GenPushReg(EAX);                                                       // push eax
  GenPushToFPU;                                                          // fld qword ptr [esp]
  DiscardStackTop(2);                                                    // add esp, 8            ;  shrink stack
  end
else if Types[DataType].Kind = SINGLETYPE then
  begin
  GenPushReg(EAX);                                                       // push eax
  GenNew($D9); Gen($04); Gen($24);                                       // fld dword ptr [esp]
  DiscardStackTop(1);                                                    // add esp, 4            ;  shrink stack
  end 
else
  Error('Internal fault: Illegal type'); 
end;




procedure PushVarPtr(Addr: Integer; Scope: TScope; DeltaNesting: Byte; RelocType: TRelocType);
const
  StaticLinkAddr = 2 * 4;
var
  i: Integer;  
begin
// EAX must be preserved

case Scope of
  GLOBAL:                                     // Global variable
    PushRelocConst(Addr, RelocType);
   
  LOCAL:
    begin
    if DeltaNesting = 0 then                  // Strictly local variable
      begin
      GenNew($8D); Gen($B5); GenDWord(Addr);                       // lea esi, [ebp + Addr]
      end
    else                                      // Intermediate level variable
      begin
      GenNew($8B); Gen($75); Gen(StaticLinkAddr);                  // mov esi, [ebp + StaticLinkAddr]
      for i := 1 to DeltaNesting - 1 do
        begin
        GenNew($8B); Gen($76); Gen(StaticLinkAddr);                // mov esi, [esi + StaticLinkAddr]
        end;
      GenNew($8D); Gen($B6); GenDWord(Addr);                       // lea esi, [esi + Addr]      
      end;      
    GenPushReg(ESI);                                               // push esi
    end;
    
end; // case
end;




procedure DerefPtr(DataType: Integer);


  function OptimizeDerefPtr: Boolean;
  var
    Addr, Offset: LongInt;
    AddrRelocIndex: Integer;
  begin
  Result := FALSE;
  
  // Global variable loading
  
  // Optimization: (mov esi, Addr) + (mov... eax, ... ptr [esi]) -> (mov... eax, ... ptr [Addr])  ; relocatable
  if PrevInstrByte(0, 0) = $BE then                                         // Previous: mov esi, Addr        
    begin
    Addr := PrevInstrDWord(0, 1);
    AddrRelocIndex := PrevInstrRelocDWordIndex(0, 1);
    RemovePrevInstr(0);                                                     // Remove: mov esi, Addr
    
    case TypeSize(DataType) of

      1: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B6); Gen($05);                              // movzx eax, byte ptr ...
           end
         else  
           begin
           GenNew($0F); Gen($BE); Gen($05);                              // movsx eax, byte ptr ...
           end; 
           
      2: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B7); Gen($05);                              // movzx eax, word ptr ...
           end
         else  
           begin
           GenNew($0F); Gen($BF); Gen($05);                              // movsx eax, word ptr ...
           end;      
         
      4: begin
         GenNew($A1);                                                    // mov eax, dword ptr ...
         end

    else
      Error('Internal fault: Illegal designator size');
    end;
    
    GenDWord(Addr);                                                      // ... [Addr]
    
    // Relocate Addr if necessary
    if (AddrRelocIndex <> 0) and (TypeSize(DataType) <> 4) then
      with Reloc[AddrRelocIndex] do Pos := Pos + 2;
    
    Result := TRUE;
    Exit;
    end

      
  // Local variable loading
  
  // Optimization: (lea esi, [ebp + Addr]) + (mov... eax, ... ptr [esi]) -> (mov... eax, ... ptr [ebp + Addr])
  else if (PrevInstrByte(0, 0) = $8D) and (PrevInstrByte(0, 1) = $B5) then        // Previous: lea esi, [ebp + Addr]        
    begin
    Addr := PrevInstrDWord(0, 2);
    RemovePrevInstr(0);                                                           // Remove: lea esi, [ebp + Addr]
    
    case TypeSize(DataType) of

      1: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B6); Gen($85);                              // movzx eax, byte ptr [ebp + ...
           end
         else  
           begin
           GenNew($0F); Gen($BE); Gen($85);                              // movsx eax, byte ptr [ebp + ...
           end; 
           
      2: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B7); Gen($85);                              // movzx eax, word ptr [ebp + ...
           end
         else  
           begin
           GenNew($0F); Gen($BF); Gen($85);                              // movsx eax, word ptr [ebp + ...
           end;      
         
      4: begin
         GenNew($8B); Gen($85);                                          // mov eax, dword ptr [ebp + ...
         end

    else
      Error('Internal fault: Illegal designator size');
    end;
    
    GenDWord(Addr);                                                      // ... + Addr]
    
    Result := TRUE;
    Exit;
    end


  // Record field loading
  
  // Optimization: (add esi, Offset) + (mov... eax, ... ptr [esi]) -> (mov... eax, ... ptr [esi + Offset])
  else if (PrevInstrByte(0, 0) = $81) and (PrevInstrByte(0, 1) = $C6) then        // Previous: add esi, Offset        
    begin
    Offset := PrevInstrDWord(0, 2);
    RemovePrevInstr(0);                                                           // Remove: add esi, Offset
    
    case TypeSize(DataType) of

      1: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B6); Gen($86);                              // movzx eax, byte ptr [esi + ...
           end
         else  
           begin
           GenNew($0F); Gen($BE); Gen($86);                              // movsx eax, byte ptr [esi + ...
           end; 
           
      2: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B7); Gen($86);                              // movzx eax, word ptr [esi + ...
           end
         else  
           begin
           GenNew($0F); Gen($BF); Gen($86);                              // movsx eax, word ptr [esi + ...
           end;      
         
      4: begin
         GenNew($8B); Gen($86);                                          // mov eax, dword ptr [esi + ...
         end

    else
      Error('Internal fault: Illegal designator size');
    end;
    
    GenDWord(Offset);                                                   // ... + Offset]
    
    Result := TRUE;
    Exit;
    end;
  
  end;


begin // DerefPtr
GenPopReg(ESI);                                                      // pop esi

if Types[DataType].Kind = REALTYPE then             // Special case: Double
  begin
  GenNew($FF); Gen($76); Gen($04);                                       // push [esi + 4]
  GenNew($FF); Gen($36);                                                 // push [esi]
  end
else                                                // General rule
  begin                                                
  if not OptimizeDerefPtr then
    case TypeSize(DataType) of

      1: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B6); Gen($06);                              // movzx eax, byte ptr [esi]
           end
         else  
           begin
           GenNew($0F); Gen($BE); Gen($06);                              // movsx eax, byte ptr [esi]
           end; 
           
      2: if Types[DataType].Kind in UnsignedTypes then
           begin
           GenNew($0F); Gen($B7); Gen($06);                              // movzx eax, word ptr [esi]
           end
         else  
           begin
           GenNew($0F); Gen($BF); Gen($06);                              // movsx eax, word ptr [esi]
           end;      
         
      4: begin
         GenNew($8B); Gen($06);                                          // mov eax, dword ptr [esi]
         end

    else
      Error('Internal fault: Illegal designator size');
    end;

  GenPushReg(EAX);                                                     // push eax
  end;
end;




procedure GetArrayElementPtr(ArrType: Integer);


  function OptimizeGetArrayElementPtr: Boolean;
  var
    BaseAddr, IndexAddr: LongInt;
    Index: Integer;
  begin
  Result := FALSE;
  
  // Global arrays
  
  // Optimization: (push BaseAddr) + (mov eax, [ebp + IndexAddr]) + (pop esi) -> (mov esi, BaseAddr) + (mov eax, [ebp + IndexAddr]) 
  if (PrevInstrByte(1, 0) = $68) and (PrevInstrByte(0, 0) = $8B) and (PrevInstrByte(0, 1) = $85) then    // Previous: push BaseAddr, mov eax, [ebp + IndexAddr]
    begin
    BaseAddr  := PrevInstrDWord(1, 1);
    IndexAddr := PrevInstrDWord(0, 2);
    
    RemovePrevInstr(1);                             // Remove: push BaseAddr, mov eax, [ebp + IndexAddr]
    
    GenNew($BE); GenDWord(BaseAddr);                // mov esi, BaseAddr         ; suilable for relocatable addresses (instruction length is the same as for push BaseAddr)
    GenNew($8B); Gen($85); GenDWord(IndexAddr);     // mov eax, [ebp + IndexAddr] 
            
    Result := TRUE;
    end
    
  // Optimization: (push BaseAddr) + (mov eax, Index) + (pop esi) -> (mov esi, BaseAddr) + (mov eax, Index) 
  else if (PrevInstrByte(1, 0) = $68) and (PrevInstrByte(0, 0) = $B8) then    // Previous: push BaseAddr, mov eax, Index
    begin
    BaseAddr  := PrevInstrDWord(1, 1);
    Index     := PrevInstrDWord(0, 1);
    
    RemovePrevInstr(1);                             // Remove: push BaseAddr, mov eax, Index
    
    GenNew($BE); GenDWord(BaseAddr);                // mov esi, BaseAddr         ; suitable for relocatable addresses (instruction length is the same as for push BaseAddr)
    GenNew($B8); GenDWord(Index);                   // mov eax, Index 
            
    Result := TRUE;
    end 
    
  // Local arrays  
    
  // Optimization: (mov eax, [ebp + BaseAddr]) + (push eax) + (mov eax, [ebp + IndexAddr]) + (pop esi) -> (mov esi, [ebp + BaseAddr]) + (mov eax, [ebp + IndexAddr]) 
  else if (PrevInstrByte(2, 0) = $8B) and (PrevInstrByte(2, 1) = $85) and     // Previous: mov eax, [ebp + BaseAddr]
          (PrevInstrByte(1, 0) = $50) and                                     // Previous: push eax
          (PrevInstrByte(0, 0) = $8B) and (PrevInstrByte(0, 1) = $85)         // Previous: mov eax, [ebp + IndexAddr]
  then   
    begin
    BaseAddr  := PrevInstrDWord(2, 2);
    IndexAddr := PrevInstrDWord(0, 2);
    
    RemovePrevInstr(2);                             // Remove: mov eax, [ebp + BaseAddr], push eax, mov eax, [ebp + IndexAddr]
    
    GenNew($8B); Gen($B5); GenDWord(BaseAddr);      // mov esi, [ebp + BaseAddr] 
    GenNew($8B); Gen($85); GenDWord(IndexAddr);     // mov eax, [ebp + IndexAddr] 
            
    Result := TRUE;
    end
    
  // Optimization: (mov eax, [ebp + BaseAddr]) + (push eax) + (mov eax, Index) + (pop esi) -> (mov esi, [ebp + BaseAddr]) + (mov eax, Index) 
  else if (PrevInstrByte(2, 0) = $8B) and (PrevInstrByte(2, 1) = $85) and     // Previous: mov eax, [ebp + BaseAddr]
          (PrevInstrByte(1, 0) = $50) and                                     // Previous: push eax
          (PrevInstrByte(0, 0) = $B8)                                         // Previous: mov eax, Index
  then   
    begin
    BaseAddr  := PrevInstrDWord(2, 2);
    Index     := PrevInstrDWord(0, 1);
    
    RemovePrevInstr(2);                             // Remove: mov eax, [ebp + BaseAddr], push eax, mov eax, Index
    
    GenNew($8B); Gen($B5); GenDWord(BaseAddr);      // mov esi, [ebp + BaseAddr] 
    GenNew($B8); GenDWord(Index);                   // mov eax, Index 
            
    Result := TRUE;
    end
    
  end; 


  function Log2(x: LongInt): ShortInt;
  var
    i: Integer;
  begin
  for i := 0 to 31 do
    if x = 1 shl i then 
      begin
      Result := i;
      Exit;
      end;  
  Result := -1;
  end;


var
  BaseTypeSize, IndexLowBound: Integer;
  Log2BaseTypeSize: ShortInt;


begin
GenPopReg(EAX);                                                 // pop eax           ; Array index

if not OptimizeGetArrayElementPtr then
  GenPopReg(ESI);                                                 // pop esi           ; Array base offset

BaseTypeSize := TypeSize(Types[ArrType].BaseType);
IndexLowBound := LowBound(Types[ArrType].IndexType);

if IndexLowBound = 1 then
  GenNew($48)                                                      // dec eax
else if IndexLowBound <> 0 then
  begin
  GenNew($2D); GenDWord(IndexLowBound);                            // sub eax, IndexLowBound
  end;

if (BaseTypeSize <> 1) and (BaseTypeSize <> 2) and (BaseTypeSize <> 4) and (BaseTypeSize <> 8) then
  begin
  Log2BaseTypeSize := Log2(BaseTypeSize);  
  if Log2BaseTypeSize > 0 then
    begin
    GenNew($C1); Gen($E0); Gen(Log2BaseTypeSize);                  // shl eax, Log2BaseTypeSize
    end
  else
    begin
    GenNew($69); Gen($C0); GenDWord(BaseTypeSize);                 // imul eax, BaseTypeSize
    end;  
  end; // if

GenNew($8D); Gen($34);                                             // lea esi, [esi + eax * ...
case BaseTypeSize of
  1:   Gen($06);                                                // ... * 1]
  2:   Gen($46);                                                // ... * 2]
  4:   Gen($86);                                                // ... * 4]
  8:   Gen($C6)                                                 // ... * 8]
  else Gen($06)                                                 // ... * 1]  ; already multiplied above
end; 
 
GenPushReg(ESI);                                                // push esi
end;




procedure GetFieldPtr(Offset: Integer);
  

  function OptimizeGetFieldPtr: Boolean;
  var
    Addr: LongInt;
    BaseTypeSizeCode: Byte;
  begin
  Result := FALSE;
  
  // Optimization: (lea esi, [ebp + Addr]) + (add esi, Offset) -> (lea esi, [ebp + Addr + Offset])
  if (PrevInstrByte(0, 0) = $8D) and (PrevInstrByte(0, 1) = $B5) then       // Previous: lea esi, [ebp + Addr]       
    begin
    Addr := PrevInstrDWord(0, 2);    
    RemovePrevInstr(0);                                                     // Remove: lea esi, [ebp + Addr]
    GenNew($8D); Gen($B5); GenDWord(Addr + Offset);                         // lea esi, [ebp + Addr + Offset]
    Result := TRUE;
    end
        
  // Optimization: (lea esi, [esi + eax * BaseTypeSize]) + (add esi, Offset) -> (lea esi, [esi + eax * BaseTypeSize + Offset])
  else if (PrevInstrByte(0, 0) = $8D) and (PrevInstrByte(0, 1) = $34) then  // Previous: lea esi, [esi + eax * BaseTypeSize]  
    begin
    BaseTypeSizeCode := PrevInstrDWord(0, 2);    
    RemovePrevInstr(0);                                                     // Remove: lea esi, [esi + eax * BaseTypeSize]   
    GenNew($8D); Gen($B4); Gen(BaseTypeSizeCode); GenDWord(Offset);         // lea esi, [esi + eax * BaseTypeSize + Offset]    
    Result := TRUE;
    end;
 
  end;


begin // GetFieldPtr
if Offset <> 0 then
  begin
  GenPopReg(ESI);                                                 // pop esi
  
  if not OptimizeGetFieldPtr then
    begin
    GenNew($81); Gen($C6); GenDWord(Offset);                      // add esi, Offset    
    end; 
   
  GenPushReg(ESI);                                                // push esi
  end;  
end;




procedure GetCharAsTempString(Depth: Integer);
begin
if (Depth <> 0) and (Depth <> SizeOf(LongInt)) then
  Error('Internal fault: Illegal depth');
  
GenPopReg(ESI);                                                   // pop esi                  ; Temporary string address

if Depth = SizeOf(LongInt) then
  GenPopReg(ECX);                                                 // pop ecx                  ; Some other string address
  
GenPopReg(EAX);                                                   // pop eax                  ; Character
GenNew($88); Gen($06);                                            // mov byte ptr [esi], al
GenNew($C6); Gen($46); Gen($01); Gen($00);                        // mov byte ptr [esi + 1], 0
GenPushReg(ESI);                                                  // push esi

if Depth = SizeOf(LongInt) then
  GenPushReg(ECX);                                                // push ecx                  ; Some other string address
end;




procedure SaveStackTopToEAX;
begin
GenPopReg(EAX);                                                    // pop eax
end;




procedure RestoreStackTopFromEAX;
begin
GenPushReg(EAX);                                                   // push eax
end;




procedure SaveStackTopToEDX;
begin
GenPopReg(EDX);                                                    // pop edx
end;




procedure RestoreStackTopFromEDX;
begin
GenPushReg(EDX);                                                   // push edx
end;




procedure RaiseStackTop(NumItems: Byte);
begin
GenNew($81); Gen($EC); GenDWord(SizeOf(LongInt) * NumItems);       // sub esp, 4 * NumItems
end;




procedure DiscardStackTop(NumItems: Byte);


  function OptimizeDiscardStackTop: Boolean;
  var
    Value: LongInt;
  begin
  Result := FALSE;
  
  // Optimization: (push Reg) + (add esp, 4 * NumItems) -> (add esp, 4 * (NumItems - 1))
  if PrevInstrByte(0, 0) in [$50, $51, $52, $56, $57, $55] then                         // Previous: push Reg
    begin
    RemovePrevInstr(0);                                                                 // Remove: push Reg
    
    if NumItems > 1 then
      begin
      GenNew($81); Gen($C4); GenDWord(SizeOf(LongInt) * (NumItems - 1));                // add esp, 4 * (NumItems - 1)
      end;
      
    Result := TRUE;
    end
  
  // Optimization: (sub esp, Value) + (add esp, 4 * NumItems) -> (add esp, 4 * NumItems - Value)  
  else if (PrevInstrByte(0, 0) = $81) and (PrevInstrByte(0, 1) = $EC) then              // Previous: sub esp, Value
    begin
    Value := PrevInstrDWord(0, 2);    
    RemovePrevInstr(0);                                                                 // Remove: sub esp, Value

    if SizeOf(LongInt) * NumItems <> Value then
      begin      
      GenNew($81); Gen($C4); GenDWord(SizeOf(LongInt) * NumItems - Value);              // add esp, 4 * NumItems - Value
      end;
      
    Result := TRUE;        
    end
  end;  


begin  // DiscardStackTop
if not OptimizeDiscardStackTop then
  begin
  GenNew($81); Gen($C4); GenDWord(SizeOf(LongInt) * NumItems);                          // add esp, 4 * NumItems
  end
end;




procedure DiscardStackTopAt(Pos: LongInt; NumItems: Byte);
begin
GenAt(Pos, $81); GenAt(Pos + 1, $C4); GenDWordAt(Pos + 2, SizeOf(LongInt) * NumItems);  // add esp, 4 * NumItems
end;




procedure DuplicateStackTop;
begin
GenNew($FF); Gen($34); Gen($24);                                                        // push dword ptr [esp]
end;




procedure SaveCodePos;
begin
Inc(CodePosStackTop);
CodePosStack[CodePosStackTop] := GetCodeSize;
end;




function RestoreCodePos: LongInt;
begin
Result := CodePosStack[CodePosStackTop];
Dec(CodePosStackTop);
end;




procedure GenerateIncDec(proc: TPredefProc; Size: Byte; BaseTypeSize: Integer = 0);
begin
GenPopReg(ESI);                                                       // pop esi

if BaseTypeSize <> 0 then                // Special case: typed pointer
  begin
  GenNew($81);                                                          // ... dword ptr  ...
 
  case proc of
    INCPROC: Gen($06);                                                  // add ... [esi], ...
    DECPROC: Gen($2E);                                                  // sub ... [esi], ...
  end;
  
  GenDWord(BaseTypeSize);                                               // ... BaseTypeSize
  end
else                                     // General rule
  begin  
  case Size of
    1: begin
       GenNew($FE);                                                     // ... byte ptr ...
       end;
    2: begin
       GenNew($66); Gen($FF);                                           // ... word ptr ...
       end;
    4: begin
       GenNew($FF);                                                     // ... dword ptr ...
       end;
    end;

  case proc of
    INCPROC: Gen($06);                                                  // inc ... [esi]
    DECPROC: Gen($0E);                                                  // dec ... [esi]
    end;
  end;  
end;




procedure GenerateRound(TruncMode: Boolean);
begin
GenPushToFPU;                                                                  // fld qword ptr [esp]  ;  st = operand
DiscardStackTop(1);                                                            // add esp, 4           ;  shrink stack

if TruncMode then
  begin
  GenNew($66); Gen($C7); Gen($44); Gen($24); Gen(Byte(-4)); GenWord($0F7F);    // mov word ptr [esp - 4], 0F7Fh
  GenNew($D9); Gen($6C); Gen($24); Gen(Byte(-4));                              // fldcw word ptr [esp - 4]
  end;
  
GenNew($DB); Gen($1C); Gen($24);                                               // fistp dword ptr [esp] ;  [esp] := round(st);  pop

if TruncMode then
  begin
  GenNew($66); Gen($C7); Gen($44); Gen($24); Gen(Byte(-4)); GenWord($037F);    // mov word ptr [esp - 4], 037Fh
  GenNew($D9); Gen($6C); Gen($24); Gen(Byte(-4));                              // fldcw word ptr [esp - 4]
  end;
  
end;// GenerateRound




procedure GenerateDoubleFromInteger(Depth: Byte);
begin
if Depth = 0 then
  begin
  GenNew($DB); Gen($04); Gen($24);                                         // fild dword ptr [esp]  ;  st := double(operand)
  RaiseStackTop(1);                                                        // sub esp, 4            ;  expand stack
  GenPopFromFPU;                                                           // fstp qword ptr [esp]  ;  [esp] := st;  pop
  end
else if Depth = SizeOf(Double) then
  begin
  GenPushToFPU;                                                            // fld qword ptr [esp]           ;  st := operand2  
  GenNew($DB); Gen($44); Gen($24); Gen(Depth);                             // fild dword ptr [esp + Depth]  ;  st := double(operand), st(1) = operand2
  RaiseStackTop(1);                                                        // sub esp, 4                    ;  expand stack
  GenNew($DD); Gen($5C); Gen($24); Gen(Depth);                             // fstp qword ptr [esp + Depth]  ;  [esp + Depth] := operand;  pop
  GenPopFromFPU;                                                           // fstp qword ptr [esp]          ;  [esp] := operand2;  pop
  end
else
  Error('Internal fault: Illegal stack depth');  
end;// GenerateDoubleFromInteger




procedure GenerateDoubleFromSingle;
begin
GenNew($D9); Gen($04); Gen($24);                                         // fld dword ptr [esp]   ;  st := double(operand)
RaiseStackTop(1);                                                        // sub esp, 4            ;  expand stack
GenPopFromFPU;                                                           // fstp qword ptr [esp]  ;  [esp] := st;  pop
end; // GenerateDoubleFromSingle




procedure GenerateSingleFromDouble;
begin
GenPushToFPU;                                                            // fld qword ptr [esp]   ;  st := operand
DiscardStackTop(1);                                                      // add esp, 4            ;  shrink stack
GenNew($D9); Gen($1C); Gen($24);                                         // fstp dword ptr [esp]  ;  [esp] := single(st);  pop
end; // GenerateDoubleFromSingle




procedure GenerateMathFunction(func: TPredefProc; ResultType: Integer);
begin
if Types[ResultType].Kind = REALTYPE then       // Real type
  begin
  GenPushToFPU;                                                            // fld qword ptr [esp]  ;  st = operand
  case func of
    ABSFUNC:
      begin
      GenNew($D9); Gen($E1);                                               // fabs
      end;
    SQRFUNC:
      begin
      GenNew($DC); Gen($C8);                                               // fmul st, st
      end;
    SINFUNC:
      begin
      GenNew($D9); Gen($FE);                                               // fsin
      end;
    COSFUNC:
      begin
      GenNew($D9); Gen($FF);                                               // fcos
      end;
    ARCTANFUNC:
      begin
      GenNew($D9); Gen($E8);                                               // fld1
      GenNew($D9); Gen($F3);                                               // fpatan    ; st := arctan(x / 1.0)
      end;
    EXPFUNC:
      begin
      GenNew($D9); Gen($EA);                                               // fldl2e
      GenNew($DE); Gen($C9);                                               // fmul
      GenNew($D9); Gen($C0);                                               // fld st
      GenNew($D9); Gen($FC);                                               // frndint
      GenNew($DD); Gen($D2);                                               // fst st(2) ; st(2) := round(x * log2(e))
      GenNew($DE); Gen($E9);                                               // fsub
      GenNew($D9); Gen($F0);                                               // f2xm1     ; st := 2 ^ frac(x * log2(e)) - 1
      GenNew($D9); Gen($E8);                                               // fld1
      GenNew($DE); Gen($C1);                                               // fadd
      GenNew($D9); Gen($FD);                                               // fscale    ; st := 2 ^ frac(x * log2(e)) * 2 ^ round(x * log2(e)) = exp(x)
      end;
    LNFUNC:
      begin
      GenNew($D9); Gen($ED);                                               // fldln2
      GenNew($D9); Gen($C9);                                               // fxch
      GenNew($D9); Gen($F1);                                               // fyl2x     ; st := ln(2) * log2(x) = ln(x)
      end;
    SQRTFUNC:
      begin
      GenNew($D9); Gen($FA);                                               // fsqrt
      end;

  end;// case

  GenPopFromFPU;                                                           // fstp qword ptr [esp]  ;  [esp] := st;  pop
  end
else                                // Ordinal types
  case func of
    ABSFUNC:
      begin
      GenPopReg(EAX);                                                      // pop eax
      GenNew($83); Gen($F8); Gen($00);                                     // cmp eax, 0
      GenNew($7D); Gen($02);                                               // jge +2
      GenNew($F7); Gen($D8);                                               // neg eax
      GenPushReg(EAX);                                                     // push eax
      end;
    SQRFUNC:
      begin
      GenPopReg(EAX);                                                      // pop eax
      GenNew($F7); Gen($E8);                                               // imul eax
      GenPushReg(EAX);                                                     // push eax
      end;
  end;// case
end;// GenerateMathFunction





procedure GenerateUnaryOperator(op: TTokenKind; ResultType: Integer);
begin
if Types[ResultType].Kind = REALTYPE then     // Real type
  begin
  if op = MINUSTOK then
    begin
    GenPushToFPU;                                                          // fld qword ptr [esp]  ;  st = operand
    GenNew($D9); Gen($E0);                                                 // fchs
    GenPopFromFPU;                                                         // fstp qword ptr [esp] ;  [esp] := st;  pop
    end;
  end
else                                              // Ordinal types
  begin
  GenPopReg(EAX);                                                          // pop eax
  case op of
    MINUSTOK:
      begin
      GenNew($F7); Gen($D8);                                               // neg eax
      end;
    NOTTOK:
      begin
      GenNew($F7); Gen($D0);                                               // not eax
      end;
  end;// case
  
  if Types[ResultType].Kind = BOOLEANTYPE then
    begin
    GenNew($83); Gen($E0); Gen($01);                                       // and eax, 1
    end;
    
  GenPushReg(EAX);                                                         // push eax
  end;// else
  
end;




procedure GenerateBinaryOperator(op: TTokenKind; ResultType: Integer);
begin
if Types[ResultType].Kind = REALTYPE then     // Real type
  begin
  GenPushToFPU;                                                            // fld qword ptr [esp]  ;  st = operand2
  DiscardStackTop(2);                                                      // add esp, 8
  GenPushToFPU;                                                            // fld qword ptr [esp]  ;  st(1) = operand2;  st = operand1

  case op of
    PLUSTOK:
      begin
      GenNew($DE); Gen($C1);                                               // fadd  ;  st(1) := st(1) + st;  pop
      end;
    MINUSTOK:
      begin
      GenNew($DE); Gen($E1);                                               // fsubr  ;  st(1) := st - st(1);  pop
      end;
    MULTOK:
      begin
      GenNew($DE); Gen($C9);                                               // fmul  ;  st(1) := st(1) * st;  pop
      end;
    DIVTOK:
      begin
      GenNew($DE); Gen($F1);                                               // fdivr  ;  st(1) := st / st(1);  pop
      end;
  end;// case

  GenPopFromFPU;                                                           // fstp dword ptr [esp]  ;  [esp] := st;  pop
  end // if
else                                          // Ordinal types
  begin
  // For commutative operators, use reverse operand order for better optimization
  if (op = PLUSTOK) or (op = ANDTOK) or (op = ORTOK) or (op = XORTOK) then
    begin
    GenPopReg(EAX);                                                        // pop eax
    GenPopReg(ECX);                                                        // pop ecx
    end
  else
    begin    
    GenPopReg(ECX);                                                        // pop ecx
    GenPopReg(EAX);                                                        // pop eax
    end;

  case op of
    PLUSTOK:
      begin
      GenNew($03); Gen($C1);                                               // add eax, ecx
      end;
    MINUSTOK:
      begin
      GenNew($2B); Gen($C1);                                               // sub eax, ecx
      end;
    MULTOK:
      begin
      GenNew($F7); Gen($E9);                                               // imul ecx
      end;
    IDIVTOK, MODTOK:
      begin
      GenNew($99);                                                         // cdq
      GenNew($F7); Gen($F9);                                               // idiv ecx
      if op = MODTOK then
        begin
        GenNew($8B); Gen($C2);                                             // mov eax, edx         ; save remainder
        end;
      end;
    SHLTOK:
      begin
      GenNew($D3); Gen($E0);                                               // shl eax, cl
      end;
    SHRTOK:
      begin
      GenNew($D3); Gen($E8);                                               // shr eax, cl
      end;
    ANDTOK:
      begin
      GenNew($23); Gen($C1);                                               // and eax, ecx
      end;
    ORTOK:
      begin
      GenNew($0B); Gen($C1);                                               // or eax, ecx
      end;
    XORTOK:
      begin
      GenNew($33); Gen($C1);                                               // xor eax, ecx
      end;

  end;// case

  if Types[ResultType].Kind = BOOLEANTYPE then
    begin
    GenNew($83); Gen($E0); Gen($01);                                       // and eax, 1
    end;  
  
  GenPushReg(EAX);                                                         // push eax
  end;// else
end;




procedure GenerateRelation(rel: TTokenKind; ValType: Integer);


  function OptimizeGenerateRelation: Boolean;
  var
    Value: LongInt;
  begin
  Result := FALSE;
  
  // Optimization: (mov ecx, Value) + (cmp eax, ecx) -> (cmp eax, Value)
  if PrevInstrByte(0, 0) = $B9 then                               // Previous: mov ecx, Value
    begin
    Value := PrevInstrDWord(0, 1);
    RemovePrevInstr(0);                                           // Remove: mov ecx, Value
    GenNew($3D); GenDWord(Value);                                 // cmp eax, Value
    Result := TRUE;
    end;
  end;


begin
if Types[ValType].Kind = REALTYPE then        // Real type
  begin
  GenPushToFPU;                                                            // fld dword ptr [esp]  ;  st = operand2
  DiscardStackTop(2);                                                      // add esp, 8
  GenPushToFPU;                                                            // fld dword ptr [esp]  ;  st(1) = operand2;  st = operand1
  DiscardStackTop(2);                                                      // add esp, 8
  GenNew($DE); Gen($D9);                                                   // fcompp               ;  test st - st(1)
  GenNew($DF); Gen($E0);                                                   // fnstsw ax
  GenNew($9E);                                                             // sahf  
  GenNew($B8); GenDWord(1);                                                // mov eax, 1           ;  TRUE

  case rel of
    EQTOK: GenNew($74);                                                    // je  ...
    NETOK: GenNew($75);                                                    // jne ...
    GTTOK: GenNew($77);                                                    // ja  ...
    GETOK: GenNew($73);                                                    // jae ...
    LTTOK: GenNew($72);                                                    // jb  ...
    LETOK: GenNew($76);                                                    // jbe ...
  end;// case
  end
else                                          // Ordinal types
  begin
  GenPopReg(ECX);                                                          // pop ecx
  GenPopReg(EAX);                                                          // pop eax
  if not OptimizeGenerateRelation then
    begin                                                            
    GenNew($39); Gen($C8);                                                 // cmp eax, ecx
    end;   
  GenNew($B8); GenDWord(1);                                                // mov eax, 1           ;  TRUE
  
  case rel of
    EQTOK: GenNew($74);                                                    // je  ...
    NETOK: GenNew($75);                                                    // jne ...
    GTTOK: GenNew($7F);                                                    // jg  ...
    GETOK: GenNew($7D);                                                    // jge ...
    LTTOK: GenNew($7C);                                                    // jl  ...
    LETOK: GenNew($7E);                                                    // jle ...
  end;// case
  end;// else

Gen($02);                                                                  // ... +2
GenNew($31); Gen($C0);                                                     // xor eax, eax         ;  FALSE
GenPushReg(EAX);                                                           // push eax
end;




procedure GenerateAssignment(DesignatorType: Integer);


  function OptimizeGenerateRealAssignment: Boolean;
  begin
  Result := FALSE;
  
  // Optimization: (fstp qword ptr [esp]) + (pop eax) + (pop edx) + (pop esi) + (mov [esi], eax) + (mov [esi + 4], edx) -> (add esp, 8) + (pop esi) + (fstp qword ptr [esi])
  if (PrevInstrByte(0, 0) = $DD) and (PrevInstrByte(0, 1) = $1C) and (PrevInstrByte(0, 2) = $24) then    // Previous: fstp dword ptr [esp]
    begin
    RemovePrevInstr(0);                                                  // Remove: fstp dword ptr [esp]
    DiscardStackTop(2);                                                  // add esp, 8
    GenPopReg(ESI);                                                      // pop esi
    GenNew($DD); Gen($1E);                                               // fstp qword ptr [esi]
    Result := TRUE;
    end;    
  end;


  function OptimizeGenerateAssignment: Boolean;
  var
    IsMov, IsMovPush: Boolean;
    Value: LongInt;
    ValueRelocIndex: Integer;
    
  begin
  Result := FALSE;
  
  IsMov := PrevInstrByte(0, 0) = $B8;                                           // Previous: mov eax, Value    
  IsMovPush := (PrevInstrByte(1, 0) = $B8) and (PrevInstrByte(0, 0) = $56);     // Previous: mov eax, Value, push esi
  
  if IsMov then
    begin
    Value := PrevInstrDWord(0, 1);
    ValueRelocIndex := PrevInstrRelocDWordIndex(0, 1);
    end
  else
    begin
    Value := PrevInstrDWord(1, 1);
    ValueRelocIndex := PrevInstrRelocDWordIndex(1, 1);
    end;  
  
  // Optimization: (mov eax, Value) + [(push esi) + (pop esi)] + (mov [esi], al/ax/eax) -> (mov byte/word/dword ptr [esi], Value)
  if (IsMov or IsMovPush) and (ValueRelocIndex = 0) then                  // Non-relocatable Value only                              
    begin  
    if IsMovPush then
      GenPopReg(ESI);                                                     // pop esi   ; destination address
      
    RemovePrevInstr(0);                                                   // Remove: mov eax, Value

    if IsMov then
      GenPopReg(ESI);                                                     // pop esi   ; destination address     
                
    case TypeSize(DesignatorType) of
      1: begin
         GenNew($C6); Gen($06); Gen(Byte(Value));                         // mov byte ptr [esi], Value
         end;
      2: begin
         GenNew($66); Gen($C7); Gen($06); GenWord(Word(Value));           // mov word ptr [esi], Value
         end;
      4: begin
         GenNew($C7); Gen($06); GenDWord(Value);                          // mov dword ptr [esi], Value
         end
      else
        Error('Internal fault: Illegal designator size');
      end; // case
    
    Result := TRUE;
    end;
    
  end;
  

begin
if Types[DesignatorType].Kind = REALTYPE then            // Special case: 64-bit real type
  begin
  if not OptimizeGenerateRealAssignment then
    begin
    GenPopReg(EAX);                                                            // pop eax   ; source value
    GenPopReg(EDX);                                                            // pop edx   ; source value
    GenPopReg(ESI);                                                            // pop esi   ; destination address
    GenNew($89); Gen($06);                                                     // mov [esi], eax
    GenNew($89); Gen($56); Gen($04);                                           // mov [esi + 4], edx
    end
  end
else                                                     // General rule: 8, 16, 32-bit types                                                          
  begin 
  if not OptimizeGenerateAssignment then
    begin
    GenPopReg(EAX);                                                            // pop eax   ; source value
    GenPopReg(ESI);                                                            // pop esi   ; destination address
                                                            
    case TypeSize(DesignatorType) of
      1: begin
         GenNew($88); Gen($06);                                                // mov [esi], al
         end;
      2: begin
         GenNew($66); Gen($89); Gen($06);                                      // mov [esi], ax
         end;
      4: begin
         GenNew($89); Gen($06);                                                // mov [esi], eax
         end
    else
      Error('Internal fault: Illegal designator size');
    end; // case
    end;  
  end;
end;




procedure GenerateForAssignmentAndNumberOfIterations(CounterType: Integer; Down: Boolean);


  function OptimizeGenerateForAssignmentAndNumberOfIterations: Boolean;
  var
    InitialValue, FinalValue: LongInt;
    InitialValueRelocIndex, FinalValueRelocIndex: LongInt;
  begin
  Result := FALSE;

  // Optimization: (push InitialValue) + (push FinalValue) + ... -> ... (constant initial and final values)
  if (PrevInstrByte(1, 0) = $68) and (PrevInstrByte(0, 0) = $68) then       // Previous: push InitialValue, push FinalValue
    begin
    InitialValue := PrevInstrDWord(1, 1);
    InitialValueRelocIndex := PrevInstrRelocDWordIndex(1, 1);

    FinalValue := PrevInstrDWord(0, 1);
    FinalValueRelocIndex := PrevInstrRelocDWordIndex(0, 1);
    
    if (InitialValueRelocIndex = 0) and (FinalValueRelocIndex = 0) then     // Non-relocatable values only
      begin
      RemovePrevInstr(1);                                                   // Remove: push InitialValue, push FinalValue
      
      GenPopReg(ESI);                                                       // pop esi       ; counter address

      case TypeSize(CounterType) of
        1: begin
           GenNew($C6); Gen($06); Gen(Byte(InitialValue));                  // mov byte ptr [esi], InitialValue
           end;
        2: begin
           GenNew($66); Gen($C7); Gen($06); GenWord(Word(InitialValue));    // mov word ptr [esi], InitialValue
           end;
        4: begin
           GenNew($C7); Gen($06); GenDWord(InitialValue);                   // mov dword ptr [esi], InitialValue
           end
        else
          Error('Internal fault: Illegal designator size');
        end; // case
      
      // Number of iterations
      if Down then
        PushConst(InitialValue - FinalValue + 1)
      else
        PushConst(FinalValue - InitialValue + 1);

      Result := TRUE;        
      end;
    end;    
  end;
  

begin
if not OptimizeGenerateForAssignmentAndNumberOfIterations then
  begin
  GenPopReg(EAX);                                                 // pop eax       ; final value
  GenPopReg(ECX);                                                 // pop ecx       ; initial value
  GenPopReg(ESI);                                                 // pop esi       ; counter address
                                                            
  case TypeSize(CounterType) of
    1: begin
       GenNew($88); Gen($0E);                                     // mov [esi], cl
       end;
    2: begin
       GenNew($66); Gen($89); Gen($0E);                           // mov [esi], cx
       end;
    4: begin
       GenNew($89); Gen($0E);                                     // mov [esi], ecx
       end
  else
    Error('Internal fault: Illegal designator size');
  end; // case

  // Number of iterations
  if Down then
    begin
    GenNew($29); Gen($C1);                                        // sub ecx, eax
    GenNew($41);                                                  // inc ecx
    GenPushReg(ECX);                                              // push ecx  
    end
  else
    begin
    GenNew($2B); Gen($C1);                                        // sub eax, ecx
    GenNew($40);                                                  // inc eax
    GenPushReg(EAX);                                              // push eax  
    end;  
  end;
end;




procedure GenerateStructuredAssignment(DesignatorType: Integer);
begin
// ECX should be preserved

GenPopReg(ESI);                                                            // pop esi      ; source address
GenPopReg(EDI);                                                            // pop edi      ; destination address

// Copy source to destination
GenPushReg(ECX);                                                           // push ecx
GenNew($B9); GenDWord(TypeSize(DesignatorType));                           // mov ecx, TypeSize(DesignatorType)
GenNew($FC);                                                               // cld          ; increment esi, edi after each step
GenNew($F3); Gen($A4);                                                     // rep movsb
GenPopReg(ECX);                                                            // pop ecx
end;




procedure GenerateInterfaceFieldAssignment(Offset: Integer; PopValueFromStack: Boolean; Value: LongInt; RelocType: TRelocType);
begin
if PopValueFromStack then
  begin
  GenPopReg(ESI);                                                               // pop esi
  GenNew($89); Gen($B5); GenDWord(Offset);                                      // mov dword ptr [ebp + Offset], esi
  GenPushReg(ESI);                                                              // push esi
  end
else
  begin
  GenNew($C7); Gen($85); GenDWord(Offset); GenRelocDWord(Value, RelocType);     // mov dword ptr [ebp + Offset], Value
  end;  
end;




procedure InitializeCStack;
begin
GenNew($89); Gen($E1);                                                          // mov ecx, esp
end;




procedure PushToCStack(SourceStackDepth: Integer; DataType: Integer; PushByValue: Boolean);
var
  ActualSize: Integer;
  
begin
if PushByValue and (Types[DataType].Kind in StructuredTypes) then
  begin  
  ActualSize := Align(TypeSize(DataType), SizeOf(LongInt));
  
  // Copy structure to the C stack
  RaiseStackTop(ActualSize div SizeOf(LongInt));                                // sub esp, ActualSize
  GenNew($8B); Gen($B1); GenDWord(SourceStackDepth);                            // mov esi, [ecx + SourceStackDepth] 
  GenNew($89); Gen($E7);                                                        // mov edi, esp
  GenPushReg(EDI);                                                              // push edi                       ; destination address
  GenPushReg(ESI);                                                              // push esi                       ; source address
  
  GenerateStructuredAssignment(DataType);
  end
else if PushByValue and (Types[DataType].Kind = REALTYPE) then
  begin
  GenNew($FF); Gen($B1); GenDWord(SourceStackDepth + SizeOf(LongInt));          // push [ecx + SourceStackDepth + 4]
  GenNew($FF); Gen($B1); GenDWord(SourceStackDepth);                            // push [ecx + SourceStackDepth]  
  end
else  
  begin 
  GenNew($FF); Gen($B1); GenDWord(SourceStackDepth);                            // push [ecx + SourceStackDepth]
  end; 
end;




procedure ConvertSmallStructureToPointer(Addr: LongInt; Size: LongInt);
begin
// Converts a small structure in EDX:EAX into a pointer in EAX
if Size <= SizeOf(LongInt) then
  begin
  GenNew($89); Gen($85); GenDWord(Addr);                                        // mov [ebp + Addr], eax
  end
else if Size <= 2 * SizeOf(LongInt) then
  begin
  GenNew($89); Gen($85); GenDWord(Addr);                                        // mov [ebp + Addr], eax
  GenNew($89); Gen($95); GenDWord(Addr + SizeOf(LongInt));                      // mov [ebp + Addr + 4], edx  
  end
else
  Error('Internal fault: Structure is too large to return in EDX:EAX');
  
GenNew($8D); Gen($85); GenDWord(Addr);                                          // lea eax, [ebp + Addr]  
end;




procedure ConvertPointerToSmallStructure(Size: LongInt);
begin
// Converts a pointer in EAX into a small structure in EDX:EAX 
if Size <= SizeOf(LongInt) then
  begin
  GenNew($8B); Gen($00);                                                        // mov eax, [eax]
  end
else if Size <= 2 * SizeOf(LongInt) then
  begin
  GenNew($8B); Gen($50); Gen(Byte(SizeOf(LongInt)));                            // mov edx, [eax + 4]
  GenNew($8B); Gen($00);                                                        // mov eax, [eax]  
  end
else
  Error('Internal fault: Structure is too large to return in EDX:EAX');
end;
 



procedure GenerateImportFuncStub(EntryPoint: LongInt);
begin
GenNew($FF); Gen($25); GenRelocDWord(EntryPoint, IMPORTRELOC);                           // jmp ds:EntryPoint  ; relocatable
end;




procedure GenerateCall(EntryPoint: LongInt; CallerNesting, CalleeNesting: Integer);
const
  StaticLinkAddr = 2 * 4;
var
  CodePos: Integer;
  i: Integer;
begin
if (CallerNesting < 0) or (CalleeNesting < 1) or (CallerNesting - CalleeNesting < -1) then
  Error('Internal fault: Illegal nesting level');
  
if CalleeNesting > 1 then                        // If a nested routine is called, push static link as the last hidden parameter
  if CallerNesting - CalleeNesting = -1 then     // The caller and the callee's enclosing routine are at the same nesting level
    begin
    GenPushReg(EBP);                                                         // push ebp
    end
  else                                           // The caller is deeper
    begin
    GenNew($8B); Gen($75); Gen(StaticLinkAddr);                              // mov esi, [ebp + StaticLinkAddr]
    for i := 1 to CallerNesting - CalleeNesting do
      begin
      GenNew($8B); Gen($76); Gen(StaticLinkAddr);                            // mov esi, [esi + StaticLinkAddr]
      end;
    GenPushReg(ESI);                                                         // push esi
    end;

// Call the routine  
CodePos := GetCodeSize;
GenNew($E8); GenDWord(EntryPoint - (CodePos + 5));                           // call EntryPoint
end;




procedure GenerateIndirectCall(CallAddressDepth: Integer);
begin
GenNew($8B); Gen($B4); Gen($24); GenDWord(CallAddressDepth);                 // mov esi, dword ptr [esp + CallAddressDepth]
GenNew($FF); Gen($16);                                                       // call [esi]
end;




procedure GenerateReturn(TotalParamsSize, Nesting: Integer);
begin
GenNew($C2);                                                                 // ret ... 
if Nesting = 1 then
  GenWord(TotalParamsSize)                                                   // ... TotalParamsSize
else  
  GenWord(TotalParamsSize + 4);                                              // ... TotalParamsSize + 4   ; + 4 is for static link
end;




procedure GenerateForwardReference;
begin
GenNew($90);                                                     // nop   ; jump to the procedure entry point will be inserted here
GenNew($90);                                                     // nop
GenNew($90);                                                     // nop
GenNew($90);                                                     // nop
GenNew($90);                                                     // nop
end;




procedure GenerateForwardResolution(CodePos: Integer);
begin
GenAt(CodePos, $E9); GenDWordAt(CodePos + 1, GetCodeSize - (CodePos + 5));      // jmp GetCodeSize
end;




procedure GenerateForwardResolutionToDestination(CodePos, DestPos: Integer);
begin
GenAt(CodePos, $E9); GenDWordAt(CodePos + 1, DestPos - (CodePos + 5));          // jmp DestPos
end;




procedure GenerateIfCondition;


  function OptimizeGenerateIfCondition: Boolean;
  var
    JumpOpCode: Byte;
  begin
  Result := FALSE;
  JumpOpCode := PrevInstrByte(1, 0);
  
  // Optimization: (mov eax, 1) + (jxx +2) + (xor eax, eax) + (test eax, eax) + (jne +5) -> (jxx +5)
  if (PrevInstrByte(2, 0) = $B8) and (PrevInstrDWord(2, 1) = 1) and                                          // Previous: mov eax, 1
     (JumpOpCode in [$74, $75, $77, $73, $72, $76, $7F, $7D, $7C, $7E]) and (PrevInstrByte(1, 1) = $02) and  // Previous: jxx +2
     (PrevInstrByte(0, 0) = $31) and (PrevInstrByte(0, 1) = $C0)                                             // Previous: xor eax, eax
  then
    begin  
    RemovePrevInstr(2);                           // Remove: mov eax, 1,  jxx +2,  xor eax, eax
    GenNew(JumpOpCode); Gen($05);                 // jxx +5
    Result := TRUE;
    end; 
  end;
  

begin
GenPopReg(EAX);                                                  // pop eax

if not OptimizeGenerateIfCondition then
  begin
  GenNew($85); Gen($C0);                                         // test eax, eax
  GenNew($75); Gen($05);                                         // jne +5
  end;
end;




procedure GenerateIfProlog;
begin
SaveCodePos;

GenNew($90);                                                   // nop   ; jump to the IF block end will be inserted here
GenNew($90);                                                   // nop
GenNew($90);                                                   // nop
GenNew($90);                                                   // nop
GenNew($90);                                                   // nop
end;




procedure GenerateElseProlog;
var
  CodePos: Integer;
begin
CodePos := RestoreCodePos;
GenAt(CodePos, $E9); GenDWordAt(CodePos + 1, GetCodeSize - (CodePos + 5) + 5);  // jmp (IF..THEN block end)

GenerateIfProlog;
end;




procedure GenerateIfElseEpilog;
var
  CodePos: Integer;
begin
CodePos := RestoreCodePos;
GenAt(CodePos, $E9); GenDWordAt(CodePos + 1, GetCodeSize - (CodePos + 5));      // jmp (IF..THEN block end)
end;




procedure GenerateCaseProlog;
begin
GenPopReg(ECX);                                                 // pop ecx           ; CASE switch value
GenNew($B0); Gen($00);                                          // mov al, 00h       ; initial flag mask
end;




procedure GenerateCaseEpilog(NumCaseStatements: Integer);
var
  i: Integer;
begin
for i := 1 to NumCaseStatements do
  GenerateIfElseEpilog;
end;




procedure GenerateCaseEqualityCheck(Value: LongInt);
begin
GenNew($81); Gen($F9); GenDWord(Value);                        // cmp ecx, Value
GenNew($9F);                                                   // lahf
GenNew($0A); Gen($C4);                                         // or al, ah
end;




procedure GenerateCaseRangeCheck(Value1, Value2: LongInt);
begin
GenNew($81); Gen($F9); GenDWord(Value1);                       // cmp ecx, Value1
GenNew($7C); Gen($0A);                                         // jl +10
GenNew($81); Gen($F9); GenDWord(Value2);                       // cmp ecx, Value2
GenNew($7F); Gen($02);                                         // jg +2
GenNew($0C); Gen($40);                                         // or al, 40h     ; set zero flag on success
end;




procedure GenerateCaseStatementProlog;
begin
GenNew($24); Gen($40);                                         // and al, 40h    ; test zero flag
GenNew($75); Gen($05);                                         // jnz +5         ; if set, jump to the case statement
GenerateIfProlog;
end;




procedure GenerateCaseStatementEpilog;
var
  StoredCodeSize: LongInt;
begin
StoredCodeSize := GetCodeSize;

GenNew($90);                                                   // nop   ; jump to the CASE block end will be inserted here
GenNew($90);                                                   // nop
GenNew($90);                                                   // nop
GenNew($90);                                                   // nop
GenNew($90);                                                   // nop

GenerateIfElseEpilog;

Inc(CodePosStackTop);
CodePosStack[CodePosStackTop] := StoredCodeSize;
end;




procedure GenerateWhileCondition;
begin
GenerateIfCondition;
end;




procedure GenerateWhileProlog;
begin
GenerateIfProlog;
end;




procedure GenerateWhileEpilog;
var
  CodePos, CurPos, ReturnPos: Integer;
begin
CodePos := RestoreCodePos;
GenAt(CodePos, $E9); GenDWordAt(CodePos + 1, GetCodeSize - (CodePos + 5) + 5);  // jmp (WHILE..DO block end)

ReturnPos := RestoreCodePos;
CurPos := GetCodeSize;
GenNew($E9); GenDWord(ReturnPos - (CurPos + 5));                                   // jmp ReturnPos
end;




procedure GenerateRepeatCondition;
begin
GenerateIfCondition;
end;




procedure GenerateRepeatProlog;
begin
SaveCodePos;
end;




procedure GenerateRepeatEpilog;
var
  CurPos, ReturnPos: Integer;
begin
ReturnPos := RestoreCodePos;
CurPos := GetCodeSize;
GenNew($E9); GenDWord(ReturnPos - (CurPos + 5));               // jmp ReturnPos
end;




procedure GenerateForCondition;
begin
// Check remaining number of iterations
GenNew($83); Gen($3C); Gen($24); Gen($00);                           // cmp dword ptr [esp], 0
GenNew($7F); Gen($05);                                               // jg +5
end;




procedure GenerateForProlog;
begin
Inc(ForLoopNesting);
GenerateIfProlog;
end;




procedure GenerateForEpilog(CounterType: Integer; Down: Boolean);
begin
// Increment/decrement counter variable
if Down then
  GenerateIncDec(DECPROC, TypeSize(CounterType))
else
  GenerateIncDec(INCPROC, TypeSize(CounterType));
  
// Decrement remaining number of iterations
GenNew($FF); Gen($0C); Gen($24);                                     // dec dword ptr [esp]
  
GenerateWhileEpilog;

Dec(ForLoopNesting);
end;




procedure GenerateGotoProlog;
begin
NumGotos := 0;
end;




procedure GenerateGoto(LabelIndex: Integer);
begin
Inc(NumGotos);
Gotos[NumGotos].Pos := GetCodeSize;
Gotos[NumGotos].LabelIndex := LabelIndex;
Gotos[NumGotos].ForLoopNesting := ForLoopNesting;

GenNew($90);               // nop   ; the remaining numbers of iterations of all nested FOR loops will be removed from stack here 
GenNew($90);               // nop
GenNew($90);               // nop
GenNew($90);               // nop
GenNew($90);               // nop
GenNew($90);               // nop

GenerateForwardReference;
end;




procedure GenerateGotoEpilog;
var
  CodePos: LongInt;
  i: Integer;
begin
for i := 1 to NumGotos do
  begin
  CodePos := Gotos[i].Pos;
  DiscardStackTopAt(CodePos, Gotos[i].ForLoopNesting - Ident[Gotos[i].LabelIndex].ForLoopNesting); // Remove the remaining numbers of iterations of all nested FOR loops
  GenerateForwardResolutionToDestination(CodePos + 6, Ident[Gotos[i].LabelIndex].Address);
  end;
end;




procedure GenerateShortCircuitProlog(op: TTokenKind);
begin
GenPopReg(EAX);                                                    // pop eax
GenNew($85); Gen($C0);                                             // test eax, eax  
case op of
  ANDTOK: GenNew($75);                                             // jne ...
  ORTOK:  GenNew($74);                                             // je  ...
end;
Gen($05);                                                          // ... +5

GenerateIfProlog; 
end;  




procedure GenerateShortCircuitEpilog;
begin
GenPopReg(EAX);                                                    // pop eax
GenerateIfElseEpilog;
GenPushReg(EAX);                                                   // push eax
end;




procedure GenerateNestedProcsProlog;
begin
GenerateIfProlog;
end;




procedure GenerateNestedProcsEpilog;
begin
GenerateIfElseEpilog;
end;




procedure GenerateFPUInit;
begin
GenNew($DB); Gen($E3);                                           // fninit
end;




procedure GenerateStackFrameProlog(PreserveRegs: Boolean);
begin
GenPushReg(EBP);                                                 // push ebp
GenNew($8B); Gen($EC);                                           // mov ebp, esp

SaveCodePos;

GenNew($90);                                                     // nop   ; actual stack storage size will be inserted here 
GenNew($90);                                                     // nop
GenNew($90);                                                     // nop
GenNew($90);                                                     // nop
GenNew($90);                                                     // nop
GenNew($90);                                                     // nop

if PreserveRegs then
  begin
  GenPushReg(ESI);                                               // push esi
  GenPushReg(EDI);                                               // push edi
  end;
end;




procedure GenerateStackFrameEpilog(TotalStackStorageSize: LongInt; PreserveRegs: Boolean);
var
  CodePos: Integer;
begin
CodePos := RestoreCodePos;
GenAt(CodePos, $81); GenAt(CodePos + 1, $EC); GenDWordAt(CodePos + 2, TotalStackStorageSize);     // sub esp, TotalStackStorageSize

if PreserveRegs then
  begin
  GenPopReg(EDI);                                                                                 // pop edi
  GenPopReg(ESI);                                                                                 // pop esi
  end;

GenNew($8B); Gen($E5);                                                                            // mov esp, ebp
GenPopReg(EBP);                                                                                   // pop ebp
end;




procedure GenerateBreakProlog(LoopNesting: Integer);
begin
BreakCall[LoopNesting].NumCalls := 0;
end;




procedure GenerateBreakCall(LoopNesting: Integer);
begin
Inc(BreakCall[LoopNesting].NumCalls);
BreakCall[LoopNesting].Pos[BreakCall[LoopNesting].NumCalls] := GetCodeSize;

GenerateForwardReference;
end;




procedure GenerateBreakEpilog(LoopNesting: Integer);
var
  i: Integer;
begin
for i := 1 to BreakCall[LoopNesting].NumCalls do
  GenerateForwardResolution(BreakCall[LoopNesting].Pos[i]);
end;




procedure GenerateContinueProlog(LoopNesting: Integer);
begin
ContinueCall[LoopNesting].NumCalls := 0;
end;




procedure GenerateContinueCall(LoopNesting: Integer);
begin
Inc(ContinueCall[LoopNesting].NumCalls);
ContinueCall[LoopNesting].Pos[ContinueCall[LoopNesting].NumCalls] := GetCodeSize;

GenerateForwardReference;
end;




procedure GenerateContinueEpilog(LoopNesting: Integer);
var
  i: Integer;
begin
for i := 1 to ContinueCall[LoopNesting].NumCalls do
  GenerateForwardResolution(ContinueCall[LoopNesting].Pos[i]);
end;




procedure GenerateExitProlog;
begin
ExitCall.NumCalls := 0;
end;




procedure GenerateExitCall;
begin
DiscardStackTop(ForLoopNesting);      // Remove the remaining numbers of iterations of all nested FOR loops

Inc(ExitCall.NumCalls);
ExitCall.Pos[ExitCall.NumCalls] := GetCodeSize;

GenerateForwardReference;
end;




procedure GenerateExitEpilog;
var
  i: Integer;
begin
for i := 1 to ExitCall.NumCalls do
  GenerateForwardResolution(ExitCall.Pos[i]);
end;


end.
