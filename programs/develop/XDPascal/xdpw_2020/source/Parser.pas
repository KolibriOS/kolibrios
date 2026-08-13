// XD Pascal - a 32-bit compiler for Windows
// Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov

{$I-}
{$H-}

unit Parser;


interface


uses SysUtils, Common, Scanner, CodeGen, Linker;


function CompileProgramOrUnit(const Name: TString): Integer;



implementation



type
  TParserState = record
    IsUnit, IsInterfaceSection: Boolean;
    UnitStatus: TUnitStatus;
  end;  



var
  ParserState: TParserState;



procedure CompileConstExpression(var ConstVal: TConst; var ConstValType: Integer); forward;
function CompileDesignator(var ValType: Integer; AllowConst: Boolean = TRUE): Boolean; forward;
procedure CompileExpression(var ValType: Integer); forward;
procedure CompileStatement(LoopNesting: Integer); forward;
procedure CompileType(var DataType: Integer); forward;




procedure DeclareIdent(const IdentName: TString; IdentKind: TIdentKind; TotalParamDataSize: Integer; IdentIsInCStack: Boolean; IdentDataType: Integer; IdentPassMethod: TPassMethod; 
                       IdentOrdConstValue: LongInt; IdentRealConstValue: Double; const IdentStrConstValue: TString; const IdentSetConstValue: TByteSet;
                       IdentPredefProc: TPredefProc; const IdentReceiverName: TString; IdentReceiverType: Integer);
var
  i, AdditionalStackItems, IdentTypeSize: Integer;
  IdentScope: TScope;
  
begin
if BlockStack[BlockStackTop].Index = 1 then IdentScope := GLOBAL else IdentScope := LOCAL;

i := GetIdentUnsafe(IdentName, FALSE, IdentReceiverType);

if (i > 0) and (Ident[i].UnitIndex = ParserState.UnitStatus.Index) and (Ident[i].Block = BlockStack[BlockStackTop].Index) then
  Error('Duplicate identifier ' + IdentName);

Inc(NumIdent);
if NumIdent > MAXIDENTS then
  Error('Maximum number of identifiers exceeded');
  
with Ident[NumIdent] do
  begin  
  Kind                := IdentKind;
  Name                := IdentName;
  Address             := 0;  
  Scope               := IdentScope;
  RelocType           := UNINITDATARELOC;
  DataType            := IdentDataType;
  UnitIndex           := ParserState.UnitStatus.Index;
  Block               := BlockStack[BlockStackTop].Index;
  NestingLevel        := BlockStackTop;
  ReceiverName        := IdentReceiverName;
  ReceiverType        := IdentReceiverType;
  Signature.NumParams := 0;
  Signature.CallConv  := DEFAULTCONV;
  PassMethod          := IdentPassMethod;
  IsUsed              := FALSE;
  IsUnresolvedForward := FALSE;
  IsExported          := ParserState.IsInterfaceSection and (IdentScope = GLOBAL);
  IsTypedConst        := FALSE;
  IsInCStack          := IdentIsInCStack;
  ForLoopNesting      := 0;
  end;

case IdentKind of
  PROC, FUNC:
    begin
    Ident[NumIdent].Signature.ResultType := IdentDataType;
    if IdentPredefProc = EMPTYPROC then
      begin
      Ident[NumIdent].Address := GetCodeSize;                            // Routine entry point address
      Ident[NumIdent].PredefProc := EMPTYPROC;
      end
    else
      begin
      Ident[NumIdent].Address := 0;
      Ident[NumIdent].PredefProc := IdentPredefProc;                     // Predefined routine index
      end;
    end;  

  VARIABLE:
    case IdentScope of
     GLOBAL:
       begin
       IdentTypeSize := TypeSize(IdentDataType);
       if IdentTypeSize > MAXUNINITIALIZEDDATASIZE - UninitializedGlobalDataSize then
         Error('Not enough memory for global variable');
         
       Ident[NumIdent].Address := UninitializedGlobalDataSize;                                 // Variable address (relocatable)
       UninitializedGlobalDataSize := UninitializedGlobalDataSize + IdentTypeSize;
       end;// else

     LOCAL:
       if TotalParamDataSize > 0 then               // Declare parameter (always 4 bytes, except structures in the C stack and doubles)
         begin          
         if Ident[NumIdent].NestingLevel = 2 then                                            // Inside a non-nested routine
           AdditionalStackItems := 1                                                         // Return address
         else                                                                                // Inside a nested routine
           AdditionalStackItems := 2;                                                        // Return address, static link (hidden parameter)  

         with BlockStack[BlockStackTop] do
           begin
           if (IdentIsInCStack or (Types[IdentDataType].Kind = REALTYPE)) and (IdentPassMethod = VALPASSING) then           
             IdentTypeSize := Align(TypeSize(IdentDataType), SizeOf(LongInt))
           else
             IdentTypeSize := SizeOf(LongInt);
  
           if IdentTypeSize > MAXSTACKSIZE - ParamDataSize then
             Error('Not enough memory for parameter');

           Ident[NumIdent].Address := AdditionalStackItems * SizeOf(LongInt) + TotalParamDataSize - ParamDataSize - (IdentTypeSize - SizeOf(LongInt));  // Parameter offset from EBP (>0)
           ParamDataSize := ParamDataSize + IdentTypeSize;
           end
         end
       else
         with BlockStack[BlockStackTop] do          // Declare local variable
           begin
           IdentTypeSize := TypeSize(IdentDataType);
           if IdentTypeSize > MAXSTACKSIZE - LocalDataSize then
             Error('Not enough memory for local variable');
           
           Ident[NumIdent].Address := -LocalDataSize - IdentTypeSize;                          // Local variable offset from EBP (<0)
           LocalDataSize := LocalDataSize + IdentTypeSize;
           end;
    end; // case


  CONSTANT:
    if IdentPassMethod = EMPTYPASSING then                              // Untyped constant
      case Types[IdentDataType].Kind of
        SETTYPE:    begin
                    Ident[NumIdent].ConstVal.SetValue := IdentSetConstValue;
                    DefineStaticSet(Ident[NumIdent].ConstVal.SetValue, Ident[NumIdent].Address);                      
                    end;
                    
        ARRAYTYPE:  begin
                    Ident[NumIdent].ConstVal.StrValue := IdentStrConstValue;
                    DefineStaticString(Ident[NumIdent].ConstVal.StrValue, Ident[NumIdent].Address);
                    end;
                    
        REALTYPE:   Ident[NumIdent].ConstVal.RealValue := IdentRealConstValue;       // Real constant value        
        else        Ident[NumIdent].ConstVal.OrdValue := IdentOrdConstValue;         // Ordinal constant value
      end    
    else                                                                // Typed constant (actually an initialized global variable)    
      begin
      with Ident[NumIdent] do
        begin
        Kind         := VARIABLE;
        Scope        := GLOBAL;
        RelocType    := INITDATARELOC;
        PassMethod   := EMPTYPASSING;
        IsTypedConst := TRUE; 
        end;
      
      IdentTypeSize := TypeSize(IdentDataType);
      if IdentTypeSize > MAXINITIALIZEDDATASIZE - InitializedGlobalDataSize then
         Error('Not enough memory for initialized global variable');

      Ident[NumIdent].Address := InitializedGlobalDataSize;               // Typed constant address (relocatable)
      InitializedGlobalDataSize := InitializedGlobalDataSize + IdentTypeSize;      
      end;
      
  GOTOLABEL:
    Ident[NumIdent].IsUnresolvedForward := TRUE;

end;// case

end; // DeclareIdent




procedure DeclareType(TypeKind: TTypeKind);
begin
Inc(NumTypes);
if NumTypes > MAXTYPES then
  Error('Maximum number of types exceeded');

with Types[NumTypes] do
  begin
  Kind := TypeKind;
  Block := BlockStack[BlockStackTop].Index;
  end;
end; // DeclareType  




procedure DeclarePredefinedIdents;
begin
// Constants
DeclareIdent('TRUE',  CONSTANT, 0, FALSE, BOOLEANTYPEINDEX, EMPTYPASSING, 1, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('FALSE', CONSTANT, 0, FALSE, BOOLEANTYPEINDEX, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);

// Types
DeclareIdent('INTEGER',  USERTYPE, 0, FALSE, INTEGERTYPEINDEX,  EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('SMALLINT', USERTYPE, 0, FALSE, SMALLINTTYPEINDEX, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('SHORTINT', USERTYPE, 0, FALSE, SHORTINTTYPEINDEX, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('WORD',     USERTYPE, 0, FALSE, WORDTYPEINDEX,     EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('BYTE',     USERTYPE, 0, FALSE, BYTETYPEINDEX,     EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);  
DeclareIdent('CHAR',     USERTYPE, 0, FALSE, CHARTYPEINDEX,     EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('BOOLEAN',  USERTYPE, 0, FALSE, BOOLEANTYPEINDEX,  EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('REAL',     USERTYPE, 0, FALSE, REALTYPEINDEX,     EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('SINGLE',   USERTYPE, 0, FALSE, SINGLETYPEINDEX,   EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
DeclareIdent('POINTER',  USERTYPE, 0, FALSE, POINTERTYPEINDEX,  EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);

// Procedures
DeclareIdent('INC',      PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], INCPROC,      '', 0);
DeclareIdent('DEC',      PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], DECPROC,      '', 0);
DeclareIdent('READ',     PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], READPROC,     '', 0);
DeclareIdent('WRITE',    PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], WRITEPROC,    '', 0);
DeclareIdent('READLN',   PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], READLNPROC,   '', 0);
DeclareIdent('WRITELN',  PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], WRITELNPROC,  '', 0);
DeclareIdent('NEW',      PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], NEWPROC,      '', 0);
DeclareIdent('DISPOSE',  PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], DISPOSEPROC,  '', 0);
DeclareIdent('BREAK',    PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], BREAKPROC,    '', 0);
DeclareIdent('CONTINUE', PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], CONTINUEPROC, '', 0);  
DeclareIdent('EXIT',     PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], EXITPROC,     '', 0);
DeclareIdent('HALT',     PROC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], HALTPROC,     '', 0);

// Functions
DeclareIdent('SIZEOF', FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], SIZEOFFUNC, '', 0);
DeclareIdent('ORD',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], ORDFUNC,    '', 0);
DeclareIdent('CHR',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], CHRFUNC,    '', 0);
DeclareIdent('LOW',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], LOWFUNC,    '', 0);
DeclareIdent('HIGH',   FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], HIGHFUNC,   '', 0);
DeclareIdent('PRED',   FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], PREDFUNC,   '', 0);
DeclareIdent('SUCC',   FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], SUCCFUNC,   '', 0);
DeclareIdent('ROUND',  FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], ROUNDFUNC,  '', 0);
DeclareIdent('TRUNC',  FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], TRUNCFUNC,  '', 0);
DeclareIdent('ABS',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], ABSFUNC,    '', 0);
DeclareIdent('SQR',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], SQRFUNC,    '', 0);
DeclareIdent('SIN',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], SINFUNC,    '', 0);
DeclareIdent('COS',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], COSFUNC,    '', 0);
DeclareIdent('ARCTAN', FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], ARCTANFUNC, '', 0);
DeclareIdent('EXP',    FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], EXPFUNC,    '', 0);
DeclareIdent('LN',     FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], LNFUNC,     '', 0);
DeclareIdent('SQRT',   FUNC, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], SQRTFUNC,   '', 0);
end;// DeclarePredefinedIdents




procedure DeclarePredefinedTypes;
begin
NumTypes := STRINGTYPEINDEX;

Types[ANYTYPEINDEX].Kind      := ANYTYPE;
Types[INTEGERTYPEINDEX].Kind  := INTEGERTYPE;
Types[SMALLINTTYPEINDEX].Kind := SMALLINTTYPE;
Types[SHORTINTTYPEINDEX].Kind := SHORTINTTYPE;
Types[WORDTYPEINDEX].Kind     := WORDTYPE;  
Types[BYTETYPEINDEX].Kind     := BYTETYPE;  
Types[CHARTYPEINDEX].Kind     := CHARTYPE;
Types[BOOLEANTYPEINDEX].Kind  := BOOLEANTYPE;
Types[REALTYPEINDEX].Kind     := REALTYPE;
Types[SINGLETYPEINDEX].Kind   := SINGLETYPE;
Types[POINTERTYPEINDEX].Kind  := POINTERTYPE;
Types[FILETYPEINDEX].Kind     := FILETYPE;
Types[STRINGTYPEINDEX].Kind   := ARRAYTYPE;

Types[POINTERTYPEINDEX].BaseType := ANYTYPEINDEX;
Types[FILETYPEINDEX].BaseType    := ANYTYPEINDEX;

// Add new anonymous type: 1 .. MAXSTRLENGTH + 1
DeclareType(SUBRANGETYPE);

Types[NumTypes].BaseType := INTEGERTYPEINDEX;
Types[NumTypes].Low      := 1;
Types[NumTypes].High     := MAXSTRLENGTH + 1;

Types[STRINGTYPEINDEX].BaseType    := CHARTYPEINDEX;
Types[STRINGTYPEINDEX].IndexType   := NumTypes;
Types[STRINGTYPEINDEX].IsOpenArray := FALSE;
end;// DeclarePredefinedTypes




function AllocateTempStorage(Size: Integer): Integer;
begin
with BlockStack[BlockStackTop] do
  begin
  TempDataSize := TempDataSize + Size;    
  Result := -LocalDataSize - TempDataSize;
  end;
end; // AllocateTempStorage




procedure PushTempStoragePtr(Addr: Integer);
begin
PushVarPtr(Addr, LOCAL, 0, UNINITDATARELOC);
end; // PushTempStoragePtr




procedure PushVarIdentPtr(IdentIndex: Integer);
begin
PushVarPtr(Ident[IdentIndex].Address, Ident[IdentIndex].Scope, BlockStackTop - Ident[IdentIndex].NestingLevel, Ident[IdentIndex].RelocType);
Ident[IdentIndex].IsUsed := TRUE;
end; // PushVarIdentPtr




procedure ConvertConstIntegerToReal(DestType: Integer; var SrcType: Integer; var ConstVal: TConst);
begin
// Try to convert an integer (right-hand side) into a real
if (Types[DestType].Kind in [REALTYPE, SINGLETYPE]) and
   ((Types[SrcType].Kind in IntegerTypes) or 
   ((Types[SrcType].Kind = SUBRANGETYPE) and (Types[Types[SrcType].BaseType].Kind in IntegerTypes)))
then
  begin
  ConstVal.RealValue := ConstVal.OrdValue;
  SrcType := REALTYPEINDEX;
  end;   
end; // ConvertConstIntegerToReal




procedure ConvertIntegerToReal(DestType: Integer; var SrcType: Integer; Depth: Integer);
begin
// Try to convert an integer (right-hand side) into a real
if (Types[DestType].Kind in [REALTYPE, SINGLETYPE]) and
   ((Types[SrcType].Kind in IntegerTypes) or 
   ((Types[SrcType].Kind = SUBRANGETYPE) and (Types[Types[SrcType].BaseType].Kind in IntegerTypes)))
then
  begin
  GenerateDoubleFromInteger(Depth);
  SrcType := REALTYPEINDEX;
  end;   
end; // ConvertIntegerToReal




procedure ConvertConstRealToReal(DestType: Integer; var SrcType: Integer; var ConstVal: TConst);
begin
// Try to convert a single (right-hand side) into a double or vice versa
if (Types[DestType].Kind = REALTYPE) and (Types[SrcType].Kind = SINGLETYPE) then
  begin
  ConstVal.RealValue := ConstVal.SingleValue;
  SrcType := REALTYPEINDEX;
  end
else if (Types[DestType].Kind = SINGLETYPE) and (Types[SrcType].Kind = REALTYPE) then
  begin
  ConstVal.SingleValue := ConstVal.RealValue;
  SrcType := SINGLETYPEINDEX;
  end  
end; // ConvertConstRealToReal




procedure ConvertRealToReal(DestType: Integer; var SrcType: Integer);
begin
// Try to convert a single (right-hand side) into a double or vice versa
if (Types[DestType].Kind = REALTYPE) and (Types[SrcType].Kind = SINGLETYPE) then
  begin
  GenerateDoubleFromSingle;
  SrcType := REALTYPEINDEX;
  end
else if (Types[DestType].Kind = SINGLETYPE) and (Types[SrcType].Kind = REALTYPE) then
  begin
  GenerateSingleFromDouble;
  SrcType := SINGLETYPEINDEX;
  end  
end; // ConvertRealToReal




procedure ConvertConstCharToString(DestType: Integer; var SrcType: Integer; var ConstVal: TConst);
var
  ch: TCharacter;
begin
if IsString(DestType) and 
   ((Types[SrcType].Kind = CHARTYPE) or 
   ((Types[SrcType].Kind = SUBRANGETYPE) and (Types[Types[SrcType].BaseType].Kind = CHARTYPE))) 
then
  begin
  ch := Char(ConstVal.OrdValue);
  ConstVal.StrValue := ch;
  SrcType := STRINGTYPEINDEX;
  end;   
end; // ConvertConstCharToString




procedure ConvertCharToString(DestType: Integer; var SrcType: Integer; Depth: Integer);
var
  TempStorageAddr: LongInt;
begin
// Try to convert a character (right-hand side) into a 2-character temporary string
if IsString(DestType) and 
   ((Types[SrcType].Kind = CHARTYPE) or 
   ((Types[SrcType].Kind = SUBRANGETYPE) and (Types[Types[SrcType].BaseType].Kind = CHARTYPE))) 
then
  begin
  TempStorageAddr := AllocateTempStorage(2 * SizeOf(TCharacter));    
  PushTempStoragePtr(TempStorageAddr);  
  GetCharAsTempString(Depth);    
  SrcType := STRINGTYPEINDEX;
  end;
end; // ConvertCharToString




procedure ConvertStringToPChar(DestType: Integer; var SrcType: Integer);
begin
// Try to convert a string (right-hand side) into a pointer to character
if (Types[DestType].Kind = POINTERTYPE) and (Types[Types[DestType].BaseType].Kind = CHARTYPE) and IsString(SrcType) then    
  SrcType := DestType;
end; // ConvertStringToPChar




procedure ConvertToInterface(DestType: Integer; var SrcType: Integer);
var
  SrcField, DestField: PField;
  TempStorageAddr: LongInt;
  FieldIndex, MethodIndex: Integer;
begin
// Try to convert a concrete or interface type to an interface type
if (Types[DestType].Kind = INTERFACETYPE) and (DestType <> SrcType) then
  begin
  // Allocate new interface variable
  TempStorageAddr := AllocateTempStorage(TypeSize(DestType));

  // Set interface's Self pointer (offset 0) to the concrete/interface data
  if Types[SrcType].Kind = INTERFACETYPE then
    begin
    DuplicateStackTop;
    DerefPtr(POINTERTYPEINDEX);
    GenerateInterfaceFieldAssignment(TempStorageAddr, TRUE, 0, UNINITDATARELOC);
    DiscardStackTop(1);
    end
  else
    GenerateInterfaceFieldAssignment(TempStorageAddr, TRUE, 0, UNINITDATARELOC);  

  // Set interface's procedure pointers to the concrete/interface methods
  for FieldIndex := 2 to Types[DestType].NumFields do
    begin
    DestField := Types[DestType].Field[FieldIndex];
    
    if Types[SrcType].Kind = INTERFACETYPE then       // Interface to interface 
      begin
      SrcField := Types[SrcType].Field[GetField(SrcType, DestField^.Name)];
      CheckSignatures(Types[SrcField^.DataType].Signature, Types[DestField^.DataType].Signature, SrcField^.Name, FALSE);
      DuplicateStackTop;
      GetFieldPtr(SrcField^.Offset);
      DerefPtr(POINTERTYPEINDEX);
      GenerateInterfaceFieldAssignment(TempStorageAddr + (FieldIndex - 1) * SizeOf(Pointer), TRUE, 0, CODERELOC);
      DiscardStackTop(1);
      end
    else                                              // Concrete to interface
      begin  
      MethodIndex := GetMethod(SrcType, DestField^.Name);
      CheckSignatures(Ident[MethodIndex].Signature, Types[DestField^.DataType].Signature, Ident[MethodIndex].Name, FALSE);
      GenerateInterfaceFieldAssignment(TempStorageAddr + (FieldIndex - 1) * SizeOf(Pointer), FALSE, Ident[MethodIndex].Address, CODERELOC);
      end;
    end; // for  
  
  DiscardStackTop(1);                                       // Remove source pointer
  PushTempStoragePtr(TempStorageAddr);   // Push destination pointer
  SrcType := DestType;
  end;
end; // ConvertToInterface




procedure CompileConstPredefinedFunc(func: TPredefProc; var ConstVal: TConst; var ConstValType: Integer);
var
  IdentIndex: Integer;

begin
NextTok;
EatTok(OPARTOK);

case func of

  SIZEOFFUNC:
    begin
    AssertIdent;
    IdentIndex := GetIdentUnsafe(Tok.Name);
    if (IdentIndex <> 0) and (Ident[IdentIndex].Kind = USERTYPE) then   // Type name
      begin
      NextTok;
      ConstVal.OrdValue := TypeSize(Ident[IdentIndex].DataType);
      end
    else                                                                // Variable name
      Error('Type name expected');
    ConstValType := INTEGERTYPEINDEX;
    end;
    

  ROUNDFUNC, TRUNCFUNC:
    begin
    CompileConstExpression(ConstVal, ConstValType);

    if not (Types[ConstValType].Kind in IntegerTypes) then
      begin     
      GetCompatibleType(ConstValType, REALTYPEINDEX);
      if func = TRUNCFUNC then
        ConstVal.OrdValue := Trunc(ConstVal.RealValue)
      else
        ConstVal.OrdValue := Round(ConstVal.RealValue);
      end;
    ConstValType := INTEGERTYPEINDEX;
    end;
    

  ORDFUNC:
    begin
    CompileConstExpression(ConstVal, ConstValType);
    if not (Types[ConstValType].Kind in OrdinalTypes) then
      Error('Ordinal type expected');
    ConstValType := INTEGERTYPEINDEX;
    end;
    

  CHRFUNC:
    begin
    CompileConstExpression(ConstVal, ConstValType);
    GetCompatibleType(ConstValType, INTEGERTYPEINDEX);
    ConstValType := CHARTYPEINDEX;
    end;    


  LOWFUNC, HIGHFUNC:
    begin
    AssertIdent;
    IdentIndex := GetIdentUnsafe(Tok.Name);
    if (IdentIndex <> 0) and (Ident[IdentIndex].Kind = USERTYPE) then   // Type name
      begin
      NextTok;
      ConstValType := Ident[IdentIndex].DataType;
      end
    else                                                                // Variable name
      Error('Type name expected');
          
    if (Types[ConstValType].Kind = ARRAYTYPE) and not Types[ConstValType].IsOpenArray then
      ConstValType := Types[ConstValType].IndexType;
    if func = HIGHFUNC then  
      ConstVal.OrdValue := HighBound(ConstValType)
    else
      ConstVal.OrdValue := LowBound(ConstValType); 
    end;


  PREDFUNC, SUCCFUNC:
    begin
    CompileConstExpression(ConstVal, ConstValType);
    if not (Types[ConstValType].Kind in OrdinalTypes) then
      Error('Ordinal type expected');
    if func = SUCCFUNC then
      Inc(ConstVal.OrdValue)
    else
      Dec(ConstVal.OrdValue);
    end;
    

  ABSFUNC, SQRFUNC, SINFUNC, COSFUNC, ARCTANFUNC, EXPFUNC, LNFUNC, SQRTFUNC:
    begin
    CompileConstExpression(ConstVal, ConstValType);
    if (func = ABSFUNC) or (func = SQRFUNC) then                          // Abs and Sqr accept real or integer parameters
      begin
      if not ((Types[ConstValType].Kind in NumericTypes) or
             ((Types[ConstValType].Kind = SUBRANGETYPE) and (Types[Types[ConstValType].BaseType].Kind in NumericTypes))) then
        Error('Numeric type expected');

      if Types[ConstValType].Kind = REALTYPE then
        if func = ABSFUNC then
          ConstVal.RealValue := abs(ConstVal.RealValue)
        else
          ConstVal.RealValue := sqr(ConstVal.RealValue)
      else
        if func = ABSFUNC then
          ConstVal.OrdValue := abs(ConstVal.OrdValue)
        else
          ConstVal.OrdValue := sqr(ConstVal.OrdValue);
      end
    else
      Error('Function is not allowed in constant expressions');   
    end;
    
end;// case

EatTok(CPARTOK);
end;// CompileConstPredefinedFunc




procedure CompileConstSetConstructor(var ConstVal: TConst; var ConstValType: Integer);
var
  ElementVal, ElementVal2: TConst;
  ElementValType: Integer;
  ElementIndex: Integer;
  
    
begin
ConstVal.SetValue := [];

// Add new anonymous type
DeclareType(SETTYPE);
Types[NumTypes].BaseType := ANYTYPEINDEX;
ConstValType := NumTypes;

// Compile constructor
EatTok(OBRACKETTOK);

if Tok.Kind <> CBRACKETTOK then
  repeat      
    CompileConstExpression(ElementVal, ElementValType);

    if Types[ConstValType].BaseType = ANYTYPEINDEX then
      begin
      if not (Types[ElementValType].Kind in OrdinalTypes) then
        Error('Ordinal type expected');        
      Types[ConstValType].BaseType := ElementValType;
      end  
    else  
      GetCompatibleType(ElementValType, Types[ConstValType].BaseType);

    if Tok.Kind = RANGETOK then
      begin
      NextTok;
      CompileConstExpression(ElementVal2, ElementValType);    
      GetCompatibleType(ElementValType, Types[ConstValType].BaseType);
      end
    else
      ElementVal2 := ElementVal;
      
    if (ElementVal.OrdValue < 0) or (ElementVal.OrdValue >= MAXSETELEMENTS) or
       (ElementVal2.OrdValue < 0) or (ElementVal2.OrdValue >= MAXSETELEMENTS)
    then
      Error('Set elements must be between 0 and ' + IntToStr(MAXSETELEMENTS - 1));  
      
    for ElementIndex := ElementVal.OrdValue to ElementVal2.OrdValue do
      ConstVal.SetValue := ConstVal.SetValue + [ElementIndex];

    if Tok.Kind <> COMMATOK then Break;
    NextTok;
  until FALSE;

EatTok(CBRACKETTOK);
end; // CompileConstSetConstructor




procedure CompileConstFactor(var ConstVal: TConst; var ConstValType: Integer);
var
  NotOpTok: TToken;
  IdentIndex: Integer;
  
begin
case Tok.Kind of
  IDENTTOK:
    begin
    IdentIndex := GetIdent(Tok.Name);

    case Ident[IdentIndex].Kind of
    
      GOTOLABEL:
        Error('Constant expression expected but label ' + Ident[IdentIndex].Name + ' found');
    
      PROC:
        Error('Constant expression expected but procedure ' + Ident[IdentIndex].Name + ' found');
        
      FUNC:
        if Ident[IdentIndex].PredefProc <> EMPTYPROC then
          CompileConstPredefinedFunc(Ident[IdentIndex].PredefProc, ConstVal, ConstValType)
        else
          Error('Function ' + Ident[IdentIndex].Name + ' is not allowed in constant expressions');

      VARIABLE:
        Error('Constant expression expected but variable ' + Ident[IdentIndex].Name + ' found');     
    
      CONSTANT:
        begin
        ConstValType := Ident[IdentIndex].DataType;
        
        case Types[ConstValType].Kind of
          SETTYPE:   ConstVal.SetValue  := Ident[IdentIndex].ConstVal.SetValue;
          ARRAYTYPE: ConstVal.StrValue  := Ident[IdentIndex].ConstVal.StrValue; 
          REALTYPE:  ConstVal.RealValue := Ident[IdentIndex].ConstVal.RealValue;
          else       ConstVal.OrdValue  := Ident[IdentIndex].ConstVal.OrdValue;
        end;
        
        NextTok;
        end;
        
      USERTYPE:
        Error('Constant expression expected but type ' + Ident[IdentIndex].Name + ' found');

      else
        Error('Internal fault: Illegal identifier');  
      end; // case Ident[IdentIndex].Kind           
    end;


  INTNUMBERTOK:
    begin
    ConstVal.OrdValue := Tok.OrdValue;
    ConstValType := INTEGERTYPEINDEX;
    NextTok;
    end;


  REALNUMBERTOK:
    begin
    ConstVal.RealValue := Tok.RealValue;
    ConstValType := REALTYPEINDEX;
    NextTok;
    end;


  CHARLITERALTOK:
    begin
    ConstVal.OrdValue := Tok.OrdValue;
    ConstValType := CHARTYPEINDEX;
    NextTok;
    end;
    
    
  STRINGLITERALTOK:
    begin
    ConstVal.StrValue := Tok.Name;
    ConstValType := STRINGTYPEINDEX;
    NextTok;
    end;    


  OPARTOK:
    begin
    NextTok;
    CompileConstExpression(ConstVal, ConstValType);
    EatTok(CPARTOK);
    end;


  NOTTOK:
    begin
    NotOpTok := Tok;
    NextTok;
    CompileConstFactor(ConstVal, ConstValType);
    CheckOperator(NotOpTok, ConstValType);        
    ConstVal.OrdValue := not ConstVal.OrdValue;
    
    if Types[ConstValType].Kind = BOOLEANTYPE then
      ConstVal.OrdValue := ConstVal.OrdValue and 1;
    end;


  OBRACKETTOK:  
    CompileConstSetConstructor(ConstVal, ConstValType); 

else
  Error('Expression expected but ' + GetTokSpelling(Tok.Kind) + ' found');
end;// case

end;// CompileConstFactor




procedure CompileConstTerm(var ConstVal: TConst; var ConstValType: Integer);
var
  OpTok: TToken;
  RightConstVal: TConst;
  RightConstValType: Integer;

begin
CompileConstFactor(ConstVal, ConstValType);

while Tok.Kind in MultiplicativeOperators do
  begin
  OpTok := Tok;
  NextTok;
  CompileConstFactor(RightConstVal, RightConstValType);

  // Try to convert integer to real
  ConvertConstIntegerToReal(RightConstValType, ConstValType, ConstVal);
  ConvertConstIntegerToReal(ConstValType, RightConstValType, RightConstVal);
  
  // Special case: real division of two integers
  if OpTok.Kind = DIVTOK then
    begin
    ConvertConstIntegerToReal(REALTYPEINDEX, ConstValType, ConstVal);
    ConvertConstIntegerToReal(REALTYPEINDEX, RightConstValType, RightConstVal);
    end;
    
  ConstValType := GetCompatibleType(ConstValType, RightConstValType);  
    
  // Special case: set intersection  
  if (OpTok.Kind = MULTOK) and (Types[ConstValType].Kind = SETTYPE) then  
    ConstVal.SetValue := ConstVal.SetValue * RightConstVal.SetValue
  // General rule  
  else
    begin    
    CheckOperator(OpTok, ConstValType);

    if Types[ConstValType].Kind = REALTYPE then        // Real constants
      case OpTok.Kind of
        MULTOK:  ConstVal.RealValue := ConstVal.RealValue * RightConstVal.RealValue;
        DIVTOK:  if RightConstVal.RealValue <> 0 then
                   ConstVal.RealValue := ConstVal.RealValue / RightConstVal.RealValue
                 else
                   Error('Constant division by zero')
      end
    else                                               // Integer constants
      begin
      case OpTok.Kind of             
        MULTOK:  ConstVal.OrdValue := ConstVal.OrdValue * RightConstVal.OrdValue;
        IDIVTOK: if RightConstVal.OrdValue <> 0 then
                   ConstVal.OrdValue := ConstVal.OrdValue div RightConstVal.OrdValue
                 else
                   Error('Constant division by zero');  
        MODTOK:  if RightConstVal.OrdValue <> 0 then
                   ConstVal.OrdValue := ConstVal.OrdValue mod RightConstVal.OrdValue
                 else
                   Error('Constant division by zero');
        SHLTOK:  ConstVal.OrdValue := ConstVal.OrdValue shl RightConstVal.OrdValue;
        SHRTOK:  ConstVal.OrdValue := ConstVal.OrdValue shr RightConstVal.OrdValue;
        ANDTOK:  ConstVal.OrdValue := ConstVal.OrdValue and RightConstVal.OrdValue;
      end;
      
      if Types[ConstValType].Kind = BOOLEANTYPE then
        ConstVal.OrdValue := ConstVal.OrdValue and 1;
      end // else  
    end; // else
  end;// while

end;// CompileConstTerm



procedure CompileSimpleConstExpression(var ConstVal: TConst; var ConstValType: Integer);
var
  UnaryOpTok, OpTok: TToken;
  RightConstVal: TConst;
  RightConstValType: Integer;

begin
UnaryOpTok := Tok;
if UnaryOpTok.Kind in UnaryOperators then
  NextTok;

CompileConstTerm(ConstVal, ConstValType);

if UnaryOpTok.Kind in UnaryOperators then
  CheckOperator(UnaryOpTok, ConstValType);

if UnaryOpTok.Kind = MINUSTOK then      // Unary minus
  if Types[ConstValType].Kind = REALTYPE then
    ConstVal.RealValue := -ConstVal.RealValue
  else
    ConstVal.OrdValue := -ConstVal.OrdValue;

while Tok.Kind in AdditiveOperators do
  begin
  OpTok := Tok;
  NextTok;
  CompileConstTerm(RightConstVal, RightConstValType);

  // Try to convert integer to real
  ConvertConstIntegerToReal(RightConstValType, ConstValType, ConstVal);
  ConvertConstIntegerToReal(ConstValType, RightConstValType, RightConstVal);
  
  // Try to convert character to string
  ConvertConstCharToString(RightConstValType, ConstValType, ConstVal);
  ConvertConstCharToString(ConstValType, RightConstValType, RightConstVal);

  ConstValType := GetCompatibleType(ConstValType, RightConstValType); 
      
  // Special case: string concatenation
  if (OpTok.Kind = PLUSTOK) and IsString(ConstValType) and IsString(RightConstValType) then
    ConstVal.StrValue := ConstVal.StrValue + RightConstVal.StrValue
  // Special case: set union or difference  
  else if (OpTok.Kind in [PLUSTOK, MINUSTOK]) and (Types[ConstValType].Kind = SETTYPE) then
    ConstVal.SetValue := ConstVal.SetValue + RightConstVal.SetValue  
  // General rule
  else
    begin  
    CheckOperator(OpTok, ConstValType);

    if Types[ConstValType].Kind = REALTYPE then       // Real constants
      case OpTok.Kind of
        PLUSTOK:  ConstVal.RealValue := ConstVal.RealValue + RightConstVal.RealValue;
        MINUSTOK: ConstVal.RealValue := ConstVal.RealValue - RightConstVal.RealValue;
      end
    else                                                  // Integer constants
      begin
      case OpTok.Kind of
        PLUSTOK:  ConstVal.OrdValue := ConstVal.OrdValue  +  RightConstVal.OrdValue;
        MINUSTOK: ConstVal.OrdValue := ConstVal.OrdValue  -  RightConstVal.OrdValue;
        ORTOK:    ConstVal.OrdValue := ConstVal.OrdValue  or RightConstVal.OrdValue;
        XORTOK:   ConstVal.OrdValue := ConstVal.OrdValue xor RightConstVal.OrdValue;
      end;
      
      if Types[ConstValType].Kind = BOOLEANTYPE then
        ConstVal.OrdValue := ConstVal.OrdValue and 1;
      end;  
    end;

  end;// while

end;// CompileSimpleConstExpression



procedure CompileConstExpression(var ConstVal: TConst; var ConstValType: Integer);
var
  OpTok: TToken;
  RightConstVal: TConst;
  RightConstValType: Integer;
  Yes: Boolean;

begin
Yes := FALSE;
CompileSimpleConstExpression(ConstVal, ConstValType);

if Tok.Kind in RelationOperators then
  begin
  OpTok := Tok;
  NextTok;
  CompileSimpleConstExpression(RightConstVal, RightConstValType);

  // Try to convert integer to real
  ConvertConstIntegerToReal(RightConstValType, ConstValType, ConstVal);
  ConvertConstIntegerToReal(ConstValType, RightConstValType, RightConstVal);
  
  // Try to convert character to string
  ConvertConstCharToString(RightConstValType, ConstValType, ConstVal);
  ConvertConstCharToString(ConstValType, RightConstValType, RightConstVal); 

  GetCompatibleType(ConstValType, RightConstValType);    
    
  // Special case: string comparison
  if IsString(ConstValType) and IsString(RightConstValType) then
    case OpTok.Kind of 
      EQTOK: Yes := ConstVal.StrValue =  RightConstVal.StrValue;
      NETOK: Yes := ConstVal.StrValue <> RightConstVal.StrValue;
      LTTOK: Yes := ConstVal.StrValue <  RightConstVal.StrValue;
      LETOK: Yes := ConstVal.StrValue <= RightConstVal.StrValue;
      GTTOK: Yes := ConstVal.StrValue >  RightConstVal.StrValue;
      GETOK: Yes := ConstVal.StrValue >= RightConstVal.StrValue;    
    end
  // Special case: set comparison
  else if (OpTok.Kind in [EQTOK, NETOK, GETOK, LETOK]) and (Types[ConstValType].Kind = SETTYPE) then
    case OpTok.Kind of 
      EQTOK: Yes := ConstVal.SetValue =  RightConstVal.SetValue;
      NETOK: Yes := ConstVal.SetValue <> RightConstVal.SetValue;
      LETOK: Yes := ConstVal.SetValue <= RightConstVal.SetValue;
      GETOK: Yes := ConstVal.SetValue >= RightConstVal.SetValue;    
    end 
  // General rule  
  else
    begin
    CheckOperator(OpTok, ConstValType);

    if Types[ConstValType].Kind = REALTYPE then
      case OpTok.Kind of
        EQTOK: Yes := ConstVal.RealValue =  RightConstVal.RealValue;
        NETOK: Yes := ConstVal.RealValue <> RightConstVal.RealValue;
        LTTOK: Yes := ConstVal.RealValue <  RightConstVal.RealValue;
        LETOK: Yes := ConstVal.RealValue <= RightConstVal.RealValue;
        GTTOK: Yes := ConstVal.RealValue >  RightConstVal.RealValue;
        GETOK: Yes := ConstVal.RealValue >= RightConstVal.RealValue;
      end
    else
      case OpTok.Kind of
        EQTOK: Yes := ConstVal.OrdValue =  RightConstVal.OrdValue;
        NETOK: Yes := ConstVal.OrdValue <> RightConstVal.OrdValue;
        LTTOK: Yes := ConstVal.OrdValue <  RightConstVal.OrdValue;
        LETOK: Yes := ConstVal.OrdValue <= RightConstVal.OrdValue;
        GTTOK: Yes := ConstVal.OrdValue >  RightConstVal.OrdValue;
        GETOK: Yes := ConstVal.OrdValue >= RightConstVal.OrdValue;
      end;
    end;
    
  if Yes then ConstVal.OrdValue := 1 else ConstVal.OrdValue := 0;    
  ConstValType := BOOLEANTYPEINDEX;      
  end;  

end;// CompileConstExpression




procedure CompilePredefinedProc(proc: TPredefProc; LoopNesting: Integer);


  function GetReadProcIdent(DataType: Integer): Integer;
  begin
  Result := 0;
  
  with Types[DataType] do
    if (Kind = INTEGERTYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = INTEGERTYPE)) then
      Result := GetIdent('READINT')                 // Integer argument
          
    else if (Kind = SMALLINTTYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = SMALLINTTYPE)) then
      Result := GetIdent('READSMALLINT')            // Small integer argument
          
    else if (Kind = SHORTINTTYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = SHORTINTTYPE)) then
      Result := GetIdent('READSHORTINT')            // Short integer argument
          
    else if (Kind = WORDTYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = WORDTYPE)) then
      Result := GetIdent('READWORD')                // Word argument

    else if (Kind = BYTETYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = BYTETYPE)) then
      Result := GetIdent('READBYTE')                // Byte argument
         
    else if (Kind = BOOLEANTYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = BOOLEANTYPE)) then
      Result := GetIdent('READBOOLEAN')             // Boolean argument
    
    else if (Kind = CHARTYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = CHARTYPE)) then
      Result := GetIdent('READCH')                  // Character argument
          
    else if Kind = REALTYPE then
      Result := GetIdent('READREAL')                // Real argument
      
    else if Kind = SINGLETYPE then
      Result := GetIdent('READSINGLE')              // Single argument      
          
    else if (Kind = ARRAYTYPE) and (BaseType = CHARTYPEINDEX) then
      Result := GetIdent('READSTRING')              // String argument
          
    else
      Error('Cannot read ' + GetTypeSpelling(DataType));
 
  end; // GetReadProcIdent
  
  
  
  function GetWriteProcIdent(DataType: Integer): Integer;
  begin
  Result := 0;
  
  with Types[DataType] do
    if (Kind in IntegerTypes) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind in IntegerTypes)) then
      Result := GetIdent('WRITEINTF')                 // Integer argument
          
    else if (Kind = BOOLEANTYPE) or ((Kind = SUBRANGETYPE) and (Types[BaseType].Kind = BOOLEANTYPE)) then
      Result := GetIdent('WRITEBOOLEANF')             // Boolean argument
          
    else if Kind = REALTYPE then
      Result := GetIdent('WRITEREALF')                // Real argument
          
    else if Kind = POINTERTYPE then
      Result := GetIdent('WRITEPOINTERF')             // Pointer argument
          
    else if (Kind = ARRAYTYPE) and (BaseType = CHARTYPEINDEX) then
      Result := GetIdent('WRITESTRINGF')              // String argument
          
    else
      Error('Cannot write ' + GetTypeSpelling(DataType));
  
  end; // GetWriteProcIdentIndex
  
 

var
  DesignatorType, FileVarType, ExpressionType, FormatterType: Integer;
  LibProcIdentIndex, ConsoleIndex: Integer;
  IsFirstParam: Boolean;
  
  
begin // CompilePredefinedProc
NextTok;

case proc of
  INCPROC, DECPROC:
    begin
    EatTok(OPARTOK);
    CompileDesignator(DesignatorType, FALSE);
    
    if (Types[DesignatorType].Kind = POINTERTYPE) and (Types[DesignatorType].BaseType <> ANYTYPEINDEX) then     // Special case: typed pointer
      GenerateIncDec(proc, TypeSize(DesignatorType), TypeSize(Types[DesignatorType].BaseType))
    else                                                                                                        // General rule
      begin  
      GetCompatibleType(DesignatorType, INTEGERTYPEINDEX);
      GenerateIncDec(proc, TypeSize(DesignatorType));
      end;
      
    EatTok(CPARTOK);
    end;


  READPROC, READLNPROC:
    begin
    ConsoleIndex := GetIdent('STDINPUTFILE');
    FileVarType := ANYTYPEINDEX;
    IsFirstParam := TRUE;

    if Tok.Kind = OPARTOK then
      begin
      NextTok;
      repeat
        // 1st argument - file handle
        if FileVarType <> ANYTYPEINDEX then
          DuplicateStackTop
        else
          PushVarIdentPtr(ConsoleIndex);

        // 2nd argument - stream handle
        PushConst(0);
        
        // 3rd argument - designator
        CompileDesignator(DesignatorType, FALSE);

        if Types[DesignatorType].Kind = FILETYPE then               // File handle
          begin
          if not IsFirstParam or ((proc = READLNPROC) and (Types[DesignatorType].BaseType <> ANYTYPEINDEX)) then
            Error('Cannot read ' + GetTypeSpelling(DesignatorType));            
          FileVarType := DesignatorType;          
          end
        else                                                        // Any input variable
          begin
          // Select input subroutine
          if (Types[FileVarType].Kind = FILETYPE) and (Types[FileVarType].BaseType <> ANYTYPEINDEX) then      // Read from typed file
            begin            
            GetCompatibleRefType(Types[FileVarType].BaseType, DesignatorType);
            
            // 4th argument - record length 
            PushConst(TypeSize(Types[FileVarType].BaseType));
 
            LibProcIdentIndex := GetIdent('READREC');
            end
          else                                                                                                // Read from text file 
            LibProcIdentIndex := GetReadProcIdent(DesignatorType);  
            
          // Call selected input subroutine. Interface: FileHandle; StreamHandle; var Designator [; Length]  
          GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
          end; // else

        IsFirstParam := FALSE;

        if Tok.Kind <> COMMATOK then Break;
        NextTok;
      until FALSE;
      
      EatTok(CPARTOK);
      end; // if OPARTOR
      
      
    // Add CR+LF, if necessary
    if proc = READLNPROC then
      begin
      // 1st argument - file handle
      if FileVarType <> ANYTYPEINDEX then
        DuplicateStackTop
      else
        PushVarIdentPtr(ConsoleIndex);
        
      // 2nd argument - stream handle
      PushConst(0);  
      
      LibProcIdentIndex := GetIdent('READNEWLINE');      
      GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
      end;
      
    // Remove first 3 arguments if they correspond to a file variable 
    if FileVarType <> ANYTYPEINDEX then
      DiscardStackTop(3);

    end;// READPROC, READLNPROC


  WRITEPROC, WRITELNPROC:
    begin
    ConsoleIndex := GetIdent('STDOUTPUTFILE');
    FileVarType := ANYTYPEINDEX;
    IsFirstParam := TRUE;

    if Tok.Kind = OPARTOK then
      begin
      NextTok;
      repeat
        // 1st argument - file handle
        if FileVarType <> ANYTYPEINDEX then
          DuplicateStackTop
        else
          PushVarIdentPtr(ConsoleIndex);

        // 2nd argument - stream handle
        PushConst(0);
        
        // 3rd argument - expression (for untyped/text files) or designator (for typed files)
        if (Types[FileVarType].Kind = FILETYPE) and (Types[FileVarType].BaseType <> ANYTYPEINDEX) then
          CompileDesignator(ExpressionType)
        else
          begin
          CompileExpression(ExpressionType);
          
          // Try to convert single to double
          ConvertRealToReal(REALTYPEINDEX, ExpressionType);
          
          // Try to convert character to string
          ConvertCharToString(STRINGTYPEINDEX, ExpressionType, 0);
          end;
        
        if Types[ExpressionType].Kind = FILETYPE then           // File handle
          begin
          if not IsFirstParam or ((proc = WRITELNPROC) and (Types[ExpressionType].BaseType <> ANYTYPEINDEX)) then
            Error('Cannot write ' + GetTypeSpelling(ExpressionType));
          FileVarType := ExpressionType;
          end
        else                                                    // Any output expression
          begin
          // 4th argument - minimum width
          if Tok.Kind = COLONTOK then
            begin
            if (Types[FileVarType].Kind = FILETYPE) and (Types[FileVarType].BaseType <> ANYTYPEINDEX) then
              Error('Format specifiers are not allowed for typed files');
              
            NextTok;
            CompileExpression(FormatterType);
            GetCompatibleType(FormatterType, INTEGERTYPEINDEX);
            
            // 5th argument - number of decimal places
            if (Tok.Kind = COLONTOK) and (Types[ExpressionType].Kind = REALTYPE) then
              begin
              NextTok;
              CompileExpression(FormatterType);
              GetCompatibleType(FormatterType, INTEGERTYPEINDEX);
              end
            else
              PushConst(0);
 
            end            
          else
            begin
            PushConst(0);
            PushConst(0);
            end;            
          
          // Select output subroutine
          if (Types[FileVarType].Kind = FILETYPE) and (Types[FileVarType].BaseType <> ANYTYPEINDEX) then      // Write to typed file                                                     
            begin           
            GetCompatibleRefType(Types[FileVarType].BaseType, ExpressionType);
            
            // Discard 4th and 5th arguments - format specifiers
            DiscardStackTop(2); 
  
            // 4th argument - record length 
            PushConst(TypeSize(Types[FileVarType].BaseType));
            
            LibProcIdentIndex := GetIdent('WRITEREC');
            end 
          else                                                                                                // Write to text file
            LibProcIdentIndex := GetWriteProcIdent(ExpressionType);
            
          // Call selected output subroutine. Interface: FileHandle; StreamHandle; (Designator | Expression); (Length; | MinWidth; DecPlaces)
          GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
          end; // else

        IsFirstParam := FALSE;

        if Tok.Kind <> COMMATOK then Break;
        NextTok;
      until FALSE;
      
      EatTok(CPARTOK);
      end; // if OPARTOR
      
      
    // Add CR+LF, if necessary
    if proc = WRITELNPROC then
      begin
      LibProcIdentIndex := GetIdent('WRITENEWLINE');
      
      // 1st argument - file handle
      if FileVarType <> ANYTYPEINDEX then
        DuplicateStackTop
      else
        PushVarIdentPtr(ConsoleIndex);
        
      // 2nd argument - stream handle
      PushConst(0);         

      GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
      end;

    // Remove first 3 arguments if they correspond to a file variable 
    if FileVarType <> ANYTYPEINDEX then
      DiscardStackTop(3);
    
    end;// WRITEPROC, WRITELNPROC
    

  NEWPROC, DISPOSEPROC:
    begin
    EatTok(OPARTOK);
    CompileDesignator(DesignatorType, FALSE);
    GetCompatibleType(DesignatorType, POINTERTYPEINDEX);
    
    if proc = NEWPROC then
      begin
      PushConst(TypeSize(Types[DesignatorType].BaseType));
      LibProcIdentIndex := GetIdent('GETMEM');
      end
    else
      LibProcIdentIndex := GetIdent('FREEMEM');
 
    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
    
    EatTok(CPARTOK);
    end;
    
    
  BREAKPROC:
    begin
    if LoopNesting < 1 then
      Error('BREAK outside of loop is not allowed');
    GenerateBreakCall(LoopNesting);
    end;
  

  CONTINUEPROC:
    begin
    if LoopNesting < 1 then
      Error('CONTINUE outside of loop is not allowed');
    GenerateContinueCall(LoopNesting);
    end;
    
    
  EXITPROC:
    GenerateExitCall;
  

  HALTPROC:
    begin
    if Tok.Kind = OPARTOK then
      begin
      NextTok;
      CompileExpression(ExpressionType);
      GetCompatibleType(ExpressionType, INTEGERTYPEINDEX);
      EatTok(CPARTOK);
      end
    else
      PushConst(0);
      
    LibProcIdentIndex := GetIdent('EXITPROCESS');
    GenerateCall(Ident[LibProcIdentIndex].Address, 1, 1);
    end;

end;// case

end;// CompilePredefinedProc




procedure CompilePredefinedFunc(func: TPredefProc; var ValType: Integer);
var
  IdentIndex: Integer;

begin
NextTok;
EatTok(OPARTOK);

case func of

  SIZEOFFUNC:
    begin
    AssertIdent;
    IdentIndex := GetIdentUnsafe(Tok.Name);
    if (IdentIndex <> 0) and (Ident[IdentIndex].Kind = USERTYPE) then   // Type name
      begin
      NextTok;
      PushConst(TypeSize(Ident[IdentIndex].DataType));
      end
    else                                                                // Variable name
      begin
      CompileDesignator(ValType);
      DiscardStackTop(1);
      PushConst(TypeSize(ValType));
      end;
    ValType := INTEGERTYPEINDEX;
    end;
    

  ROUNDFUNC, TRUNCFUNC:
    begin
    CompileExpression(ValType);

    // Try to convert integer to real
    ConvertIntegerToReal(REALTYPEINDEX, ValType, 0);
    
    GetCompatibleType(ValType, REALTYPEINDEX);
    GenerateRound(func = TRUNCFUNC);
    ValType := INTEGERTYPEINDEX;
    end;
    

  ORDFUNC:
    begin
    CompileExpression(ValType);
    if not (Types[ValType].Kind in OrdinalTypes) then
      Error('Ordinal type expected');
    ValType := INTEGERTYPEINDEX;
    end;
    

  CHRFUNC:
    begin
    CompileExpression(ValType);
    GetCompatibleType(ValType, INTEGERTYPEINDEX);
    ValType := CHARTYPEINDEX;
    end;    


  LOWFUNC, HIGHFUNC:
    begin
    AssertIdent;
    IdentIndex := GetIdentUnsafe(Tok.Name);
    if (IdentIndex <> 0) and (Ident[IdentIndex].Kind = USERTYPE) then   // Type name
      begin
      NextTok;
      ValType := Ident[IdentIndex].DataType;
      end
    else                                                                // Variable name
      begin
      CompileDesignator(ValType);
      DiscardStackTop(1);
      end;
          
    if (Types[ValType].Kind = ARRAYTYPE) and not Types[ValType].IsOpenArray then
      ValType := Types[ValType].IndexType;
    if func = HIGHFUNC then  
      PushConst(HighBound(ValType))
    else
      PushConst(LowBound(ValType)); 
    end;


  PREDFUNC, SUCCFUNC:
    begin
    CompileExpression(ValType);
    if not (Types[ValType].Kind in OrdinalTypes) then
      Error('Ordinal type expected');
    if func = SUCCFUNC then
      PushConst(1)
    else
      PushConst(-1);
    GenerateBinaryOperator(PLUSTOK, INTEGERTYPEINDEX);
    end;
    

  ABSFUNC, SQRFUNC, SINFUNC, COSFUNC, ARCTANFUNC, EXPFUNC, LNFUNC, SQRTFUNC:
    begin
    CompileExpression(ValType);
    if (func = ABSFUNC) or (func = SQRFUNC) then                          // Abs and Sqr accept real or integer parameters
      begin
      if not ((Types[ValType].Kind in NumericTypes) or
             ((Types[ValType].Kind = SUBRANGETYPE) and (Types[Types[ValType].BaseType].Kind in NumericTypes))) then
        Error('Numeric type expected')
      end
    else
      begin
      // Try to convert integer to real
      ConvertIntegerToReal(REALTYPEINDEX, ValType, 0);
      GetCompatibleType(ValType, REALTYPEINDEX);
      end;

    GenerateMathFunction(func, ValType);
    end;
    
end;// case

EatTok(CPARTOK);
end;// CompilePredefinedFunc




procedure CompileTypeIdent(var DataType: Integer; AllowForwardReference: Boolean);
var
  IdentIndex: Integer;
begin
// STRING, FILE or type name allowed
case Tok.Kind of
  STRINGTOK:
    DataType := STRINGTYPEINDEX;
  FILETOK:
    DataType := FILETYPEINDEX
else
  AssertIdent;
  
  if AllowForwardReference then
    IdentIndex := GetIdentUnsafe(Tok.Name, AllowForwardReference)
  else
    IdentIndex := GetIdent(Tok.Name, AllowForwardReference);                         
  
  if AllowForwardReference and ((IdentIndex = 0) or (Ident[IdentIndex].Block <> BlockStack[BlockStackTop].Index)) then
    begin
    // Add new forward-referenced type
    DeclareType(FORWARDTYPE);
    Types[NumTypes].TypeIdentName := Tok.Name;
    DataType := NumTypes;
    end
  else
    begin
    // Use existing type
    if Ident[IdentIndex].Kind <> USERTYPE then
      Error('Type name expected');
  
    DataType := Ident[IdentIndex].DataType;
    end;
end; // case

NextTok;
end; // CompileTypeIdent
  



procedure CompileFormalParametersAndResult(IsFunction: Boolean; var Signature: TSignature);
var
  IdentInListName: array [1..MAXPARAMS] of TString;
  NumIdentInList, IdentInListIndex: Integer;  
  ParamType, DefaultValueType: Integer;    
  ListPassMethod: TPassMethod;
  IsOpenArrayList, StringByValFound: Boolean;
  Default: TConst;
  
begin
Signature.NumParams := 0;
Signature.NumDefaultParams := 0;

StringByValFound := FALSE;
  
if Tok.Kind = OPARTOK then
  begin
  NextTok;
  repeat
    NumIdentInList := 0;
    ListPassMethod := VALPASSING;

    if Tok.Kind = CONSTTOK then
      begin
      ListPassMethod := CONSTPASSING;
      NextTok;
      end
    else if Tok.Kind = VARTOK then
      begin
      ListPassMethod := VARPASSING;
      NextTok;
      end;

    repeat
      AssertIdent;

      Inc(NumIdentInList);
      IdentInListName[NumIdentInList] := Tok.Name;

      NextTok;

      if Tok.Kind <> COMMATOK then Break;
      NextTok;
    until FALSE;

    
    // Formal parameter list type
    if Tok.Kind = COLONTOK then                       // Typed parameters 
      begin
      NextTok;
    
      // Special case: open array parameters
      if Tok.Kind = ARRAYTOK then
        begin
        NextTok;
        EatTok(OFTOK);
        IsOpenArrayList := TRUE;
        end
      else
        IsOpenArrayList := FALSE;
 
      // Type itself
      CompileTypeIdent(ParamType, FALSE);          
                 
      // Special case: open array parameters
      if IsOpenArrayList then
        begin
        // Add new anonymous type 0..0 for array index
        DeclareType(SUBRANGETYPE);
        
        Types[NumTypes].BaseType    := INTEGERTYPEINDEX;
        Types[NumTypes].Low         := 0;
        Types[NumTypes].High        := 0;
        
        // Add new anonymous type for array itself
        DeclareType(ARRAYTYPE);
        
        Types[NumTypes].BaseType    := ParamType;
        Types[NumTypes].IndexType   := NumTypes - 1;
        Types[NumTypes].IsOpenArray := TRUE;
        
        ParamType := NumTypes;
        end;  
      end
    else                                              // Untyped parameters (CONST or VAR only) 
      ParamType := ANYTYPEINDEX;

    
    if (ListPassMethod <> VARPASSING) and (ParamType = ANYTYPEINDEX) then
      Error('Untyped parameters require VAR');
      
    if (ListPassMethod = VALPASSING) and IsString(ParamType) then
      StringByValFound := TRUE;
      

    // Default parameter value
    if (Tok.Kind = EQTOK) or (Signature.NumDefaultParams > 0) then
      begin
      EatTok(EQTOK);
      
      if not (Types[ParamType].Kind in OrdinalTypes + [REALTYPE]) then
        Error('Ordinal or real type expected for default parameter');
        
      if ListPassMethod <> VALPASSING then
        Error('Default parameters cannot be passed by reference');

      CompileConstExpression(Default, DefaultValueType);
      GetCompatibleType(ParamType, DefaultValueType);
        
      Inc(Signature.NumDefaultParams);
      end;
      

    for IdentInListIndex := 1 to NumIdentInList do
      begin
      Inc(Signature.NumParams);

      if Signature.NumParams > MAXPARAMS then
        Error('Too many formal parameters');

      New(Signature.Param[Signature.NumParams]);

      with Signature, Param[NumParams]^ do
        begin
        Name             := IdentInListName[IdentInListIndex];
        DataType         := ParamType;
        PassMethod       := ListPassMethod;
        Default.OrdValue := 0;
        end;
      
      // Default parameter value
      if (Signature.NumDefaultParams > 0) and (IdentInListIndex = 1) then
        begin
        if NumIdentInList > 1 then
          Error('Default parameters cannot be grouped');          
        Signature.Param[Signature.NumParams]^.Default := Default; 
        end;
        
      end;// for
      

    if Tok.Kind <> SEMICOLONTOK then Break;
    NextTok;
  until FALSE;

  EatTok(CPARTOK);
  end;// if Tok.Kind = OPARTOK


// Function result type
Signature.ResultType := 0;

if IsFunction then
  begin
  EatTok(COLONTOK);  
  CompileTypeIdent(Signature.ResultType, FALSE);
  end;
  
  
// Call modifier
if (Tok.Kind = IDENTTOK) and (Tok.Name = 'STDCALL') then
  begin    
  Signature.CallConv := STDCALLCONV;
  NextTok;
  end
else if (Tok.Kind = IDENTTOK) and (Tok.Name = 'CDECL') then
  begin    
  Signature.CallConv := CDECLCONV;
  NextTok;
  end  
else  
  Signature.CallConv := DEFAULTCONV;
  
if (Signature.CallConv <> DEFAULTCONV) and StringByValFound then
  Error('Strings cannot be passed by value to STDCALL/CDECL procedures');  
  
end; // CompileFormalParametersAndResult




procedure CompileActualParameters(const Signature: TSignature; var StructuredResultAddr: LongInt);


  procedure CompileExpressionCopy(var ValType: Integer; CallConv: TCallConv);
  var
    TempStorageAddr: Integer;
    LibProcIdentIndex: Integer;
    
  begin
  CompileExpression(ValType);
  
  // Copy structured parameter passed by value (for STDCALL/CDECL functions there is no need to do it here since it will be done in MakeCStack)
  if (Types[ValType].Kind in StructuredTypes) and (CallConv = DEFAULTCONV) then
    begin
    SaveStackTopToEAX; 
    TempStorageAddr := AllocateTempStorage(TypeSize(ValType));
    PushTempStoragePtr(TempStorageAddr);
    RestoreStackTopFromEAX;
    
    if IsString(ValType) then
      begin 
      LibProcIdentIndex := GetIdent('ASSIGNSTR');    
      GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);    
      end
    else
      GenerateStructuredAssignment(ValType);

    PushTempStoragePtr(TempStorageAddr);
    end;
    
  end; // CompileExpressionCopy
    

var
  NumActualParams: Integer;
  ActualParamType: Integer;
  DefaultParamIndex: Integer;
  CurParam: PParam;
  
begin
// Allocate space for structured Result as a hidden VAR parameter (except STDCALL/CDECL functions returning small structures in EDX:EAX)
with Signature do
  if (ResultType <> 0) and (Types[ResultType].Kind in StructuredTypes) and ((CallConv = DEFAULTCONV) or (TypeSize(ResultType) > 2 * SizeOf(LongInt))) then
    begin
    StructuredResultAddr := AllocateTempStorage(TypeSize(ResultType));
    PushTempStoragePtr(StructuredResultAddr);
    end
  else
    StructuredResultAddr := 0;  

NumActualParams := 0;

if Tok.Kind = OPARTOK then                            // Actual parameter list found
  begin
  NextTok;
  
  if Tok.Kind <> CPARTOK then
    repeat
      if NumActualParams + 1 > Signature.NumParams then
        Error('Too many actual parameters');

      CurParam := Signature.Param[NumActualParams + 1];

      case CurParam^.PassMethod of
        VALPASSING:   CompileExpressionCopy(ActualParamType, Signature.CallConv);
        CONSTPASSING: CompileExpression(ActualParamType);
        VARPASSING:   CompileDesignator(ActualParamType, CurParam^.DataType = ANYTYPEINDEX);
      else
        Error('Internal fault: Illegal parameter passing method');        
      end;

      Inc(NumActualParams);

      // Try to convert integer to double, single to double or double to single 
      if CurParam^.PassMethod <> VARPASSING then
        begin
        ConvertIntegerToReal(CurParam^.DataType, ActualParamType, 0);
        ConvertRealToReal(CurParam^.DataType, ActualParamType);
        end;        
       
      // Try to convert character to string
      ConvertCharToString(CurParam^.DataType, ActualParamType, 0);

      // Try to convert string to pointer to character
      ConvertStringToPChar(CurParam^.DataType, ActualParamType);      
        
      // Try to convert a concrete type to an interface type
      ConvertToInterface(CurParam^.DataType, ActualParamType);
      
      if CurParam^.PassMethod = VARPASSING then  
        GetCompatibleRefType(CurParam^.DataType, ActualParamType)  // Strict type checking for parameters passed by reference, except for open array parameters and untyped parameters
      else      
        GetCompatibleType(CurParam^.DataType, ActualParamType);    // Relaxed type checking for parameters passed by value
         
      if Tok.Kind <> COMMATOK then Break;
      NextTok;
    until FALSE;

  EatTok(CPARTOK);
  end;// if Tok.Kind = OPARTOK
  

if NumActualParams < Signature.NumParams - Signature.NumDefaultParams then
  Error('Too few actual parameters');
  
  
// Push default parameters
for DefaultParamIndex := NumActualParams + 1 to Signature.NumParams do
  begin
  CurParam := Signature.Param[DefaultParamIndex];
  PushConst(CurParam^.Default.OrdValue);
  end; // for
  
end;// CompileActualParameters




procedure MakeCStack(const Signature: TSignature);
var
  ParamIndex: Integer;
  SourceStackDepth: Integer;
begin
InitializeCStack;

// Push explicit parameters
SourceStackDepth := 0;
for ParamIndex := Signature.NumParams downto 1 do
  with Signature.Param[ParamIndex]^ do
    begin
    PushToCStack(SourceStackDepth, DataType, PassMethod = VALPASSING);
    
    if (Types[DataType].Kind = REALTYPE) and (PassMethod = VALPASSING) then           
      SourceStackDepth := SourceStackDepth + Align(TypeSize(DataType), SizeOf(LongInt))
    else
      SourceStackDepth := SourceStackDepth + SizeOf(LongInt);
    end;

// Push structured Result onto the C stack, except STDCALL/CDECL functions returning small structures in EDX:EAX
with Signature do 
  if (ResultType <> 0) and (Types[ResultType].Kind in StructuredTypes) and ((CallConv = DEFAULTCONV) or (TypeSize(ResultType) > 2 * SizeOf(LongInt))) then
    PushToCStack(NumParams * SizeOf(LongInt), ResultType, FALSE);
end; // MakeCStack




procedure ConvertResultFromCToPascal(const Signature: TSignature; StructuredResultAddr: LongInt);
begin
with Signature do
  if (ResultType <> 0) and (CallConv <> DEFAULTCONV) then
    if Types[ResultType].Kind in StructuredTypes then
      if TypeSize(ResultType) <= 2 * SizeOf(LongInt) then
        begin     
        // For small structures returned by STDCALL/CDECL functions, allocate structured result pointer and load structure from EDX:EAX
        StructuredResultAddr := AllocateTempStorage(TypeSize(ResultType));
        ConvertSmallStructureToPointer(StructuredResultAddr, TypeSize(ResultType));
        end
      else  
        begin
        // Save structured result pointer to EAX (not all external functions do it themselves)
        PushTempStoragePtr(StructuredResultAddr);
        SaveStackTopToEAX;
        end
    else if Types[ResultType].Kind in [REALTYPE, SINGLETYPE] then
      // STDCALL/CDECL functions generally return real result in ST(0), but we do it in EAX or EDX:EAX 
      MoveFunctionResultFromFPUToEDXEAX(ResultType);
end; // ConvertResultFromCToPascal




procedure ConvertResultFromPascalToC(const Signature: TSignature);
begin
with Signature do
  if (ResultType <> 0) and (CallConv <> DEFAULTCONV) then
    if (Types[ResultType].Kind in StructuredTypes) and (TypeSize(ResultType) <= 2 * SizeOf(LongInt)) then
      // In STDCALL/CDECL functions, return small structure in EDX:EAX
      ConvertPointerToSmallStructure(TypeSize(ResultType))
    else if Types[ResultType].Kind in [REALTYPE, SINGLETYPE] then
      // STDCALL/CDECL functions generally return real result in ST(0), but we do it in EAX or EDX:EAX 
      MoveFunctionResultFromEDXEAXToFPU(ResultType);          
end; // ConvertResultFromCPascalToC




procedure CompileCall(IdentIndex: Integer);
var
  TotalPascalParamSize, TotalCParamSize: Integer;
  StructuredResultAddr: LongInt;  
begin
TotalPascalParamSize := GetTotalParamSize(Ident[IdentIndex].Signature, FALSE, TRUE); 
TotalCParamSize      := GetTotalParamSize(Ident[IdentIndex].Signature, FALSE, FALSE);

CompileActualParameters(Ident[IdentIndex].Signature, StructuredResultAddr);

// Convert stack to C format
if (Ident[IdentIndex].Signature.CallConv <> DEFAULTCONV) and (TotalPascalParamSize > 0) then
  MakeCStack(Ident[IdentIndex].Signature);
  
GenerateCall(Ident[IdentIndex].Address, BlockStackTop - 1, Ident[IdentIndex].NestingLevel);

// Free C stack for a CDECL function
if (Ident[IdentIndex].Signature.CallConv = CDECLCONV) and (TotalPascalParamSize > 0) then
  DiscardStackTop(TotalCParamSize div SizeOf(LongInt));

// Free original stack
if (Ident[IdentIndex].Signature.CallConv <> DEFAULTCONV) and (TotalPascalParamSize > 0) then
  DiscardStackTop(TotalPascalParamSize div SizeOf(LongInt));

// Treat special cases in STDCALL/CDECL functions
ConvertResultFromCToPascal(Ident[IdentIndex].Signature, StructuredResultAddr);     
end; // CompileCall




procedure CompileMethodCall(ProcVarType: Integer);
var
  MethodIndex: Integer;
  StructuredResultAddr: LongInt;
  
begin
MethodIndex := Types[ProcVarType].MethodIdentIndex;  
 
// Self pointer has already been passed as the first (hidden) argument
CompileActualParameters(Ident[MethodIndex].Signature, StructuredResultAddr);

GenerateCall(Ident[MethodIndex].Address, BlockStackTop - 1, Ident[MethodIndex].NestingLevel);
end; // CompileMethodCall




procedure CompileIndirectCall(ProcVarType: Integer);
var
  TotalPascalParamSize, TotalCParamSize, CallAddrDepth: Integer;
  StructuredResultAddr: LongInt;
  
begin
TotalPascalParamSize := GetTotalParamSize(Types[ProcVarType].Signature, Types[ProcVarType].SelfPointerOffset <> 0, TRUE);
TotalCParamSize      := GetTotalParamSize(Types[ProcVarType].Signature, Types[ProcVarType].SelfPointerOffset <> 0, FALSE);

if Types[ProcVarType].SelfPointerOffset <> 0 then   // Interface method found
  begin
  if Types[ProcVarType].Signature.CallConv <> DEFAULTCONV then
    Error('STDCALL/CDECL is not allowed for methods');
  
  // Push Self pointer as a first (hidden) VAR parameter
  DuplicateStackTop;
  GetFieldPtr(Types[ProcVarType].SelfPointerOffset);
  DerefPtr(POINTERTYPEINDEX);
  end;
  
CompileActualParameters(Types[ProcVarType].Signature, StructuredResultAddr);

// Convert stack to C format
if (Types[ProcVarType].Signature.CallConv <> DEFAULTCONV) and (TotalPascalParamSize > 0) then
  begin
  MakeCStack(Types[ProcVarType].Signature); 
  CallAddrDepth := TotalPascalParamSize + TotalCParamSize;
  end
else
  CallAddrDepth := TotalPascalParamSize;  

GenerateIndirectCall(CallAddrDepth);

// Free C stack for a CDECL function
if (Types[ProcVarType].Signature.CallConv = CDECLCONV) and (TotalPascalParamSize > 0) then
  DiscardStackTop(TotalCParamSize div SizeOf(LongInt));

// Free original stack
if (Types[ProcVarType].Signature.CallConv <> DEFAULTCONV) and (TotalPascalParamSize > 0) then
  DiscardStackTop(TotalPascalParamSize div SizeOf(LongInt));
  
// Remove call address
DiscardStackTop(1);  

// Treat special cases in STDCALL/CDECL functions
ConvertResultFromCToPascal(Types[ProcVarType].Signature, StructuredResultAddr);      
end; // CompileIndirectCall




function CompileMethodOrProceduralVariableCall(var ValType: Integer; FunctionOnly, DesignatorOnly: Boolean): Boolean;
var
  ResultType: Integer;
  
begin
if Types[ValType].Kind = METHODTYPE then
  ResultType := Ident[Types[ValType].MethodIdentIndex].Signature.ResultType
else if Types[ValType].Kind = PROCEDURALTYPE then
  ResultType := Types[ValType].Signature.ResultType
else
  begin
  ResultType := 0;
  Error('Procedure or function expected'); 
  end; 

Result := FALSE; 
   
if not DesignatorOnly or ((ResultType <> 0) and (Types[ResultType].Kind in StructuredTypes)) then 
  begin
  if FunctionOnly and (ResultType = 0) then
    Error('Function expected');
  
  if Types[ValType].Kind = METHODTYPE then  
    CompileMethodCall(ValType)
  else
    CompileIndirectCall(ValType);
  
  ValType := ResultType;
  Result := TRUE; 
  end; 
end; // CompileMethodOrProceduralVariableCall




procedure CompileFieldOrMethodInsideWith(var ValType: Integer; var IsConst: Boolean);
var
  FieldIndex, MethodIndex: Integer;
  RecType: Integer;
  TempStorageAddr: Integer;
  
begin
AssertIdent; 
FieldIndex := GetFieldInsideWith(TempStorageAddr, RecType, IsConst, Tok.Name);
  
if FieldIndex <> 0 then
  begin
  PushTempStoragePtr(TempStorageAddr);
  DerefPtr(POINTERTYPEINDEX);
  
  GetFieldPtr(Types[RecType].Field[FieldIndex]^.Offset);
  ValType := Types[RecType].Field[FieldIndex]^.DataType;    
  
  Exit;
  end;
  
MethodIndex := GetMethodInsideWith(TempStorageAddr, RecType, IsConst, Tok.Name);
  
if MethodIndex <> 0 then
  begin
  PushTempStoragePtr(TempStorageAddr);
  DerefPtr(POINTERTYPEINDEX);
  
  // Add new anonymous 'method' type
  DeclareType(METHODTYPE);
  Types[NumTypes].MethodIdentIndex := MethodIndex; 
  ValType := NumTypes;

  Exit;   
  end;  

ValType := 0;  
end; // CompileFieldOrMethodInsideWith




procedure CompileSetConstructor(var ValType: Integer);
var
  ElementType: Integer;
  LibProcIdentIndex: Integer;
  TempStorageAddr: Integer;
  
begin
// Add new anonymous type
DeclareType(SETTYPE);
Types[NumTypes].BaseType := ANYTYPEINDEX;
ValType := NumTypes;

// Allocate temporary storage
TempStorageAddr := AllocateTempStorage(TypeSize(ValType));
PushTempStoragePtr(TempStorageAddr);

// Initialize set
LibProcIdentIndex := GetIdent('INITSET');
GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel); 

// Compile constructor
LibProcIdentIndex := GetIdent('ADDTOSET');
NextTok;

if Tok.Kind <> CBRACKETTOK then
  repeat
    PushTempStoragePtr(TempStorageAddr);
    
    CompileExpression(ElementType);
    
    if Types[ValType].BaseType = ANYTYPEINDEX then
      begin
      if not (Types[ElementType].Kind in OrdinalTypes) then
        Error('Ordinal type expected');        
      Types[ValType].BaseType := ElementType;
      end  
    else  
      GetCompatibleType(ElementType, Types[ValType].BaseType);

    if Tok.Kind = RANGETOK then
      begin
      NextTok;
      CompileExpression(ElementType);    
      GetCompatibleType(ElementType, Types[ValType].BaseType);
      end
    else
      PushConst(-1);
      
    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);        
      
    if Tok.Kind <> COMMATOK then Break;
    NextTok;
  until FALSE;
  
EatTok(CBRACKETTOK);

PushTempStoragePtr(TempStorageAddr);
end; // CompileSetConstructor




function DereferencePointerAsDesignator(var ValType: Integer; MustDereferenceAnyPointer: Boolean): Boolean;
begin
// If a pointer-type result is immediately followed by dereferencing, treat it as a designator that has the pointer's base type
Result := FALSE;

if Types[ValType].Kind = POINTERTYPE then                      
  if Types[ValType].BaseType <> ANYTYPEINDEX then
    begin
    if Tok.Kind = DEREFERENCETOK then
      begin
      ValType := Types[ValType].BaseType;
      Result := TRUE;
      NextTok;
      end
    else if MustDereferenceAnyPointer then 
      CheckTok(DEREFERENCETOK)
    end    
  else if MustDereferenceAnyPointer then
    Error('Typed pointer expected');
      
end; // DereferencePointerAsDesignator




function CompileSelectors(var ValType: Integer; BasicDesignatorIsStatement: Boolean = FALSE): Boolean;
var
  FieldIndex, MethodIndex: Integer;
  ArrayIndexType: Integer;
  Field: PField;   

begin
// A selector is only applicable to a memory location
// A function call can be part of a selector only if it returns an address (i.e. a structured result), not an immediate value
// All other calls are part of a factor or a statement

// Returns TRUE if constitutes a statement, i.e., ends with a function call
Result := BasicDesignatorIsStatement;

while Tok.Kind in [DEREFERENCETOK, OBRACKETTOK, PERIODTOK, OPARTOK] do
  begin
  Result := FALSE;
  
  case Tok.Kind of
  
    DEREFERENCETOK:                                   // Pointer dereferencing
      begin
      if (Types[ValType].Kind <> POINTERTYPE) or (Types[ValType].BaseType = ANYTYPEINDEX) then
        Error('Typed pointer expected');
      DerefPtr(ValType);
      ValType := Types[ValType].BaseType;
      NextTok;
      end;
      
    
    OBRACKETTOK:                                      // Array element access
      begin
      repeat
        // Convert a pointer to an array if needed (C style)
        if (Types[ValType].Kind = POINTERTYPE) and (Types[ValType].BaseType <> ANYTYPEINDEX) then
          begin
          DerefPtr(ValType);
          
          // Add new anonymous type 0..0 for array index
          DeclareType(SUBRANGETYPE);
          
          Types[NumTypes].BaseType    := INTEGERTYPEINDEX;
          Types[NumTypes].Low         := 0;
          Types[NumTypes].High        := 0;
          
          // Add new anonymous type for array itself
          DeclareType(ARRAYTYPE);
          
          Types[NumTypes].BaseType    := Types[ValType].BaseType;
          Types[NumTypes].IndexType   := NumTypes - 1;

          ValType := NumTypes;   
          end;
          
        if Types[ValType].Kind <> ARRAYTYPE then
          Error('Array expected');
          
        NextTok;
        CompileExpression(ArrayIndexType);            // Array index
        GetCompatibleType(ArrayIndexType, Types[ValType].IndexType);
        GetArrayElementPtr(ValType);
        ValType := Types[ValType].BaseType;
      until Tok.Kind <> COMMATOK;
      
      EatTok(CBRACKETTOK);
      end;
      
      
    PERIODTOK:                                        // Method or record field access
      begin
      NextTok;
      AssertIdent;
      
      // First search for a method
      MethodIndex := GetMethodUnsafe(ValType, Tok.Name); 
      if MethodIndex <> 0 then
        begin
        // Add new anonymous 'method' type
        DeclareType(METHODTYPE);
        Types[NumTypes].MethodIdentIndex := MethodIndex;       
        ValType := NumTypes;
        end  
      // If unsuccessful, search for a record field  
      else
        begin          
        if not (Types[ValType].Kind in [RECORDTYPE, INTERFACETYPE]) then
          Error('Record or interface expected');
        
        FieldIndex := GetField(ValType, Tok.Name);
        Field := Types[ValType].Field[FieldIndex];        
        GetFieldPtr(Field^.Offset);
        ValType := Field^.DataType;                
        end;
        
      NextTok;   
      end;
      
    
    OPARTOK: 
      begin
      if not CompileMethodOrProceduralVariableCall(ValType, TRUE, TRUE) then Break;  // Not a designator 
      PushFunctionResult(ValType);
      Result := TRUE; 
      end;
      
  end; // case
  
  end; // while
 
end; // CompileSelectors




function CompileBasicDesignator(var ValType: Integer; var IsConst: Boolean): Boolean;
var
  ResultType: Integer;
  IdentIndex: Integer;  
  
begin
// A designator always designates a memory location
// A function call can be part of a designator only if it returns an address (i.e. a structured result or a pointer), not an immediate value
// All other calls are part of a factor or a statement

// Returns TRUE if constitutes a statement, i.e., ends with a function call
Result := FALSE;
IsConst := FALSE;
   
AssertIdent;

// First search among records in WITH blocks
CompileFieldOrMethodInsideWith(ValType, IsConst);

// If unsuccessful, search among ordinary identifiers
if ValType = 0 then
  begin
  IdentIndex := GetIdent(Tok.Name);
  
  case Ident[IdentIndex].Kind of
  
    FUNC:
      begin
      ResultType := Ident[IdentIndex].Signature.ResultType;

      if not (Types[ResultType].Kind in StructuredTypes + [POINTERTYPE]) then   // Only allow a function that returns a designator
        Error('Function must return pointer or structured result');

      NextTok;
      CompileCall(IdentIndex);
      PushFunctionResult(ResultType);
      Result := TRUE;        
      ValType := ResultType;
      
      if DereferencePointerAsDesignator(ValType, TRUE) then
        Result := FALSE;
      end;  
             
    VARIABLE:                                  
      begin   
      PushVarIdentPtr(IdentIndex);      
      ValType := Ident[IdentIndex].DataType;
      IsConst := Ident[IdentIndex].IsTypedConst or (Ident[IdentIndex].PassMethod = CONSTPASSING);         
      
      // Structured CONST parameters are passed by reference, scalar CONST parameters are passed by value
      if (Ident[IdentIndex].PassMethod = VARPASSING) or
        ((Ident[IdentIndex].PassMethod <> VALPASSING) and (Types[ValType].Kind in StructuredTypes) and Ident[IdentIndex].IsInCStack) or
        ((Ident[IdentIndex].PassMethod <> EMPTYPASSING) and (Types[ValType].Kind in StructuredTypes) and not Ident[IdentIndex].IsInCStack)
      then
        DerefPtr(POINTERTYPEINDEX);
     
      NextTok;
      end;
      
    USERTYPE:                                                                       // Type cast
      begin                                                                      
      NextTok;
      
      EatTok(OPARTOK);
      CompileExpression(ValType);
      EatTok(CPARTOK);

      if not (Types[Ident[IdentIndex].DataType].Kind in StructuredTypes + [POINTERTYPE]) then   // Only allow a typecast that returns a designator
        Error('Typecast must return pointer or structured result');      
      
      if (Ident[IdentIndex].DataType <> ValType) and 
        not ((Types[Ident[IdentIndex].DataType].Kind in CastableTypes) and (Types[ValType].Kind in CastableTypes)) 
      then
        Error('Invalid typecast');            
      
      ValType := Ident[IdentIndex].DataType;
      
      DereferencePointerAsDesignator(ValType, TRUE);      
      end  
    
  else
    Error('Variable or function expected but ' + GetTokSpelling(Tok.Kind) + ' found');
  end; // case
    
  end
else
  NextTok;  
end; // CompileBasicDesignator




function CompileDesignator(var ValType: Integer; AllowConst: Boolean = TRUE): Boolean;
var
  IsConst: Boolean;
begin
// Returns TRUE if constitutes a statement, i.e., ends with a function call
Result := CompileBasicDesignator(ValType, IsConst);

if IsConst and not AllowConst then
  Error('Constant value cannot be modified');
  
Result := CompileSelectors(ValType, Result);
end; // CompileDesignator




procedure CompileFactor(var ValType: Integer);


  procedure CompileDereferenceOrCall(var ValType: Integer);
  begin
  if Tok.Kind = OPARTOK then   // Method or procedural variable call (parentheses are required even for empty parameter list)                                     
    begin
    CompileMethodOrProceduralVariableCall(ValType, TRUE, FALSE);
    PushFunctionResult(ValType);
    end       
  else                         // Usual variable
    if not (Types[ValType].Kind in StructuredTypes) then  // Structured expressions are stored as pointers to them
      DerefPtr(ValType);
      
  ConvertRealToReal(REALTYPEINDEX, ValType);              // Real expressions must be double, not single            
  end; // CompileDereferenceOrCall
  
  
var
  IdentIndex: Integer;
  NotOpTok: TToken;

  
begin // CompileFactor
case Tok.Kind of

  IDENTTOK:
    if FieldOrMethodInsideWithFound(Tok.Name) then                                      // Record field or method inside a WITH block
      begin
      CompileDesignator(ValType);
      CompileDereferenceOrCall(ValType);
      end      
    else                                                                                // Ordinary identifier
      begin
      IdentIndex := GetIdent(Tok.Name);

      case Ident[IdentIndex].Kind of
      
        GOTOLABEL:
          Error('Expression expected but label ' + Ident[IdentIndex].Name + ' found');
      
        PROC:
          Error('Expression expected but procedure ' + Ident[IdentIndex].Name + ' found');
          
        FUNC:                                                                           // Function call
          if Ident[IdentIndex].PredefProc <> EMPTYPROC then                             // Predefined function call
            CompilePredefinedFunc(Ident[IdentIndex].PredefProc, ValType)
          else                                                                          // User-defined function call
            begin
            NextTok;
            ValType := Ident[IdentIndex].Signature.ResultType;
            
            CompileCall(IdentIndex);
            PushFunctionResult(ValType);
            ConvertRealToReal(REALTYPEINDEX, ValType);              // Real expressions must be double, not single            

            if (Types[ValType].Kind in StructuredTypes) or DereferencePointerAsDesignator(ValType, FALSE) then
              begin
              CompileSelectors(ValType);
              CompileDereferenceOrCall(ValType);
              end; 
            end;
            
        VARIABLE:                                                                       // Variable
          begin
          CompileDesignator(ValType);
          CompileDereferenceOrCall(ValType);   
          end;
          
        CONSTANT:                                                                       // Constant
          begin
          if Types[Ident[IdentIndex].DataType].Kind in StructuredTypes then
            PushVarPtr(Ident[IdentIndex].Address, GLOBAL, 0, INITDATARELOC)
          else if Types[Ident[IdentIndex].DataType].Kind = REALTYPE then
            PushRealConst(Ident[IdentIndex].ConstVal.RealValue)
          else            
            PushConst(Ident[IdentIndex].ConstVal.OrdValue);
            
          ValType := Ident[IdentIndex].DataType;
          NextTok;
          
          if Types[Ident[IdentIndex].DataType].Kind in StructuredTypes then
            begin
            CompileSelectors(ValType);
            CompileDereferenceOrCall(ValType);
            end;            
          end;
          
        USERTYPE:                                                                       // Type cast
          begin                                                                      
          NextTok;
          
          EatTok(OPARTOK);
          CompileExpression(ValType);
          EatTok(CPARTOK);

          if (Ident[IdentIndex].DataType <> ValType) and 
             not ((Types[Ident[IdentIndex].DataType].Kind in CastableTypes) and (Types[ValType].Kind in CastableTypes)) 
          then
            Error('Invalid typecast');            
                     
          ValType := Ident[IdentIndex].DataType;
          
          if (Types[ValType].Kind = POINTERTYPE) and (Types[Types[ValType].BaseType].Kind in StructuredTypes) then
            begin
            if DereferencePointerAsDesignator(ValType, FALSE) then
              begin
              CompileSelectors(ValType);
              CompileDereferenceOrCall(ValType);
              end
            end             
          else  
            CompileSelectors(ValType); 
          end
          
      else
        Error('Internal fault: Illegal identifier');  
      end; // case Ident[IdentIndex].Kind
      end; // else  


  ADDRESSTOK:
    begin
    NextTok;
    
    if FieldOrMethodInsideWithFound(Tok.Name) then         // Record field inside a WITH block
      begin
      CompileDesignator(ValType);
      DeclareType(POINTERTYPE);
      Types[NumTypes].BaseType := ValType;
      end      
    else                                                    // Ordinary identifier
      begin  
      IdentIndex := GetIdent(Tok.Name);
      
      if Ident[IdentIndex].Kind in [PROC, FUNC] then
        begin
        if (Ident[IdentIndex].PredefProc <> EMPTYPROC) or (Ident[IdentIndex].Block <> 1) then
          Error('Procedure or function cannot be predefined or nested');
          
        PushRelocConst(Ident[IdentIndex].Address, CODERELOC); // To be resolved later when the code section origin is known        
        NextTok;
        
        DeclareType(PROCEDURALTYPE);
        Types[NumTypes].Signature := Ident[IdentIndex].Signature;
        CopyParams(Types[NumTypes].Signature, Ident[IdentIndex].Signature);
        end
      else
        begin  
        CompileDesignator(ValType);
        DeclareType(POINTERTYPE);
        Types[NumTypes].BaseType := ValType;
        end;
      end;  
    
    ValType := NumTypes;  
    end;


  INTNUMBERTOK:
    begin
    PushConst(Tok.OrdValue);
    ValType := INTEGERTYPEINDEX;
    NextTok;
    end;


  REALNUMBERTOK:
    begin
    PushRealConst(Tok.RealValue);
    ValType := REALTYPEINDEX;
    NextTok;
    end;


  CHARLITERALTOK:
    begin
    PushConst(Tok.OrdValue);
    ValType := CHARTYPEINDEX;
    NextTok;
    end;


  STRINGLITERALTOK:
    begin
    PushVarPtr(Tok.StrAddress, GLOBAL, 0, INITDATARELOC);
    ValType := STRINGTYPEINDEX;
    NextTok;
    end;


  OPARTOK:
    begin
    NextTok;
    CompileExpression(ValType);
    EatTok(CPARTOK);
    end;


  NOTTOK:
    begin
    NotOpTok := Tok;
    NextTok;
    CompileFactor(ValType);
    CheckOperator(NotOpTok, ValType);
    GenerateUnaryOperator(NOTTOK, ValType);
    end;
    
    
  OBRACKETTOK:  
    CompileSetConstructor(ValType);
    

  NILTOK:
    begin
    PushConst(0);
    ValType := POINTERTYPEINDEX;
    NextTok;
    end

else
  Error('Expression expected but ' + GetTokSpelling(Tok.Kind) + ' found');
end;// case

end;// CompileFactor




procedure CompileTerm(var ValType: Integer);
var
  OpTok: TToken;
  RightValType: Integer;
  LibProcIdentIndex: Integer;
  TempStorageAddr: Integer;
  UseShortCircuit: Boolean; 
  
begin
CompileFactor(ValType);

while Tok.Kind in MultiplicativeOperators do
  begin
  OpTok := Tok;
  NextTok;
  
  UseShortCircuit := (OpTok.Kind = ANDTOK) and (Types[ValType].Kind = BOOLEANTYPE);
  if UseShortCircuit then 
    GenerateShortCircuitProlog(OpTok.Kind);  
  
  CompileFactor(RightValType);

  // Try to convert integer to real
  ConvertIntegerToReal(ValType, RightValType, 0);
  ConvertIntegerToReal(RightValType, ValType, SizeOf(Double));
  
  // Special case: real division of two integers
  if OpTok.Kind = DIVTOK then
    begin
    ConvertIntegerToReal(REALTYPEINDEX, RightValType, 0);
    ConvertIntegerToReal(REALTYPEINDEX, ValType, SizeOf(Double));
    end;
    
  // Special case: set intersection  
  if (OpTok.Kind = MULTOK) and (Types[ValType].Kind = SETTYPE) then  
    begin
    ValType := GetCompatibleType(ValType, RightValType);
    
    LibProcIdentIndex := GetIdent('SETINTERSECTION');
      
    TempStorageAddr := AllocateTempStorage(TypeSize(ValType));    
    PushTempStoragePtr(TempStorageAddr);

    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);    
    PushTempStoragePtr(TempStorageAddr);
    end
  // General rule  
  else
    begin
    ValType := GetCompatibleType(ValType, RightValType);
    CheckOperator(OpTok, ValType);

    if UseShortCircuit then 
      GenerateShortCircuitEpilog
    else 
      GenerateBinaryOperator(OpTok.Kind, ValType);
    end;
    
  end;// while

end;// CompileTerm




procedure CompileSimpleExpression(var ValType: Integer);
var
  UnaryOpTok, OpTok: TToken;
  RightValType: Integer;
  LibProcIdentIndex: Integer;
  TempStorageAddr: Integer;
  UseShortCircuit: Boolean; 
  
begin
UnaryOpTok := Tok;
if UnaryOpTok.Kind in UnaryOperators then
  NextTok;

CompileTerm(ValType);

if UnaryOpTok.Kind in UnaryOperators then
  CheckOperator(UnaryOpTok, ValType);

if UnaryOpTok.Kind = MINUSTOK then GenerateUnaryOperator(MINUSTOK, ValType);     // Unary minus

while Tok.Kind in AdditiveOperators do
  begin
  OpTok := Tok;
  NextTok;
  
  UseShortCircuit := (OpTok.Kind = ORTOK) and (Types[ValType].Kind = BOOLEANTYPE);
  if UseShortCircuit then 
    GenerateShortCircuitProlog(OpTok.Kind);
  
  CompileTerm(RightValType); 

  // Try to convert integer to real
  ConvertIntegerToReal(ValType, RightValType, 0);
  ConvertIntegerToReal(RightValType, ValType, SizeOf(Double));
    
  // Try to convert character to string
  ConvertCharToString(ValType, RightValType, 0);
  ConvertCharToString(RightValType, ValType, SizeOf(LongInt));  
      
  // Special case: string concatenation
  if (OpTok.Kind = PLUSTOK) and IsString(ValType) and IsString(RightValType) then
    begin 
    LibProcIdentIndex := GetIdent('CONCATSTR');   

    TempStorageAddr := AllocateTempStorage(TypeSize(STRINGTYPEINDEX));    
    PushTempStoragePtr(TempStorageAddr);
    
    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);    
    PushTempStoragePtr(TempStorageAddr);
    ValType := STRINGTYPEINDEX;
    end
  // Special case: set union or difference  
  else if (OpTok.Kind in [PLUSTOK, MINUSTOK]) and (Types[ValType].Kind = SETTYPE) then  
    begin
    ValType := GetCompatibleType(ValType, RightValType);
    
    if OpTok.Kind = PLUSTOK then
      LibProcIdentIndex := GetIdent('SETUNION')
    else
      LibProcIdentIndex := GetIdent('SETDIFFERENCE');
      
    TempStorageAddr := AllocateTempStorage(TypeSize(ValType));    
    PushTempStoragePtr(TempStorageAddr);

    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);    
    PushTempStoragePtr(TempStorageAddr);
    end  
  // General rule
  else
    begin
    ValType := GetCompatibleType(ValType, RightValType);
    CheckOperator(OpTok, ValType);
 
    if UseShortCircuit then 
      GenerateShortCircuitEpilog
    else 
      GenerateBinaryOperator(OpTok.Kind, ValType);
    end;
  
  end;// while

end;// CompileSimpleExpression




procedure CompileExpression(var ValType: Integer);
var
  OpTok: TToken;
  RightValType: Integer;
  LibProcIdentIndex: Integer;

  
begin // CompileExpression
CompileSimpleExpression(ValType);

if Tok.Kind in RelationOperators then
  begin
  OpTok := Tok;
  NextTok;
  CompileSimpleExpression(RightValType);

  // Try to convert integer to real
  ConvertIntegerToReal(ValType, RightValType, 0);
  ConvertIntegerToReal(RightValType, ValType, SizeOf(Double));
    
  // Try to convert character to string
  ConvertCharToString(ValType, RightValType, 0);
  ConvertCharToString(RightValType, ValType, SizeOf(LongInt));     
    
  // Special case: string comparison
  if IsString(ValType) and IsString(RightValType) then
    begin 
    LibProcIdentIndex := GetIdent('COMPARESTR');
    
    ValType := Ident[LibProcIdentIndex].Signature.ResultType;
    RightValType := INTEGERTYPEINDEX;
    
    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
    PushFunctionResult(ValType); 
    PushConst(0);    
    end;

  // Special case: set comparison
  if (OpTok.Kind in [EQTOK, NETOK, GETOK, LETOK]) and (Types[ValType].Kind = SETTYPE) then
    begin
    GetCompatibleType(ValType, RightValType); 
    
    case OpTok.Kind of
      GETOK: LibProcIdentIndex := GetIdent('TESTSUPERSET');    // Returns  1 if Val >= RightVal, -1 otherwise 
      LETOK: LibProcIdentIndex := GetIdent('TESTSUBSET');      // Returns -1 if Val <= RightVal,  1 otherwise 
      else   LibProcIdentIndex := GetIdent('COMPARESETS');     // Returns  0 if Val  = RightVal,  1 otherwise  
    end;
 
    ValType := Ident[LibProcIdentIndex].Signature.ResultType;
    RightValType := INTEGERTYPEINDEX;
    
    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
    PushFunctionResult(ValType);
    PushConst(0); 
    end;  

  GetCompatibleType(ValType, RightValType);
  CheckOperator(OpTok, ValType);
  ValType := BOOLEANTYPEINDEX;
  GenerateRelation(OpTok.Kind, RightValType);
  end
else if Tok.Kind = INTOK then
  begin
  NextTok;
  CompileSimpleExpression(RightValType);
  
  if Types[RightValType].Kind <> SETTYPE then
    Error('Set expected');
  
  if Types[RightValType].BaseType <> ANYTYPEINDEX then
    GetCompatibleType(ValType, Types[RightValType].BaseType)
  else if not (Types[ValType].Kind in OrdinalTypes) then
    Error('Ordinal type expected');   

  LibProcIdentIndex := GetIdent('INSET');
  ValType := Ident[LibProcIdentIndex].Signature.ResultType;
  
  GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);
  PushFunctionResult(ValType);    
  end;  

end;// CompileExpression




procedure CompileStatementList(LoopNesting: Integer);
begin
CompileStatement(LoopNesting);
while Tok.Kind = SEMICOLONTOK do
  begin
  NextTok;
  CompileStatement(LoopNesting);
  end;
end; // CompileStatementList 




procedure CompileCompoundStatement(LoopNesting: Integer);
begin
EatTok(BEGINTOK);
CompileStatementList(LoopNesting);
EatTok(ENDTOK);
end; // CompileCompoundStatement




procedure CompileStatement(LoopNesting: Integer);



  procedure CompileLabel;
  var
    LabelIndex: Integer;
  begin
  if Tok.Kind = IDENTTOK then
    begin
    LabelIndex := GetIdentUnsafe(Tok.Name);
    
    if LabelIndex <> 0 then
      if Ident[LabelIndex].Kind = GOTOLABEL then
        begin
        if Ident[LabelIndex].Block <> BlockStack[BlockStackTop].Index then
          Error('Label is not declared in current procedure');
        
        Ident[LabelIndex].Address := GetCodeSize;        
        Ident[LabelIndex].IsUnresolvedForward := FALSE;
        Ident[LabelIndex].ForLoopNesting := ForLoopNesting;
        
        NextTok;
        EatTok(COLONTOK);
        end;      
    end;
  end; // CompileLabel
  
  
  
  
  procedure CompileAssignment(DesignatorType: Integer);
  var
    ExpressionType: Integer;
    LibProcIdentIndex: Integer;
  begin
  NextTok;

  CompileExpression(ExpressionType);
  
  // Try to convert integer to double, single to double or double to single
  ConvertIntegerToReal(DesignatorType, ExpressionType, 0);
  ConvertRealToReal(DesignatorType, ExpressionType);
    
  // Try to convert character to string
  ConvertCharToString(DesignatorType, ExpressionType, 0);

  // Try to convert string to pointer to character
  ConvertStringToPChar(DesignatorType, ExpressionType);     
    
  // Try to convert a concrete type to an interface type
  ConvertToInterface(DesignatorType, ExpressionType);       

  GetCompatibleType(DesignatorType, ExpressionType);

  if IsString(DesignatorType) then
    begin 
    LibProcIdentIndex := GetIdent('ASSIGNSTR');    
    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);    
    end
  else if Types[DesignatorType].Kind in StructuredTypes then
    GenerateStructuredAssignment(DesignatorType)
  else
    GenerateAssignment(DesignatorType);
  
  end; // CompileAssignment
  
  
  
  
  procedure CompileAssignmentOrCall(DesignatorType: Integer; DesignatorIsStatement: Boolean);
  begin
  if Tok.Kind = OPARTOK then            // Method or procedural variable call (parentheses are required even for empty parameter list)                                     
    CompileMethodOrProceduralVariableCall(DesignatorType, FALSE, FALSE)
  else if DesignatorIsStatement then    // Optional assignment if designator already ends with a function call   
    if Tok.Kind = ASSIGNTOK then     
      CompileAssignment(DesignatorType)
    else
      DiscardStackTop(1)                // Nothing to do - remove designator
  else                                  // Mandatory assignment
    begin  
    CheckTok(ASSIGNTOK);                               
    CompileAssignment(DesignatorType)
    end; 
  end; // CompileAssignmentOrCall
  

  
  
  procedure CompileIfStatement(LoopNesting: Integer);
  var
    ExpressionType: Integer;
    
  begin
  NextTok;
  
  CompileExpression(ExpressionType);
  GetCompatibleType(ExpressionType, BOOLEANTYPEINDEX);
  
  EatTok(THENTOK);

  GenerateIfCondition;              // Satisfied if expression is not zero
  GenerateIfProlog;
  CompileStatement(LoopNesting);

  if Tok.Kind = ELSETOK then
    begin
    NextTok;
    GenerateElseProlog;                 
    CompileStatement(LoopNesting);
    end;

  GenerateIfElseEpilog;
  end; // CompileIfStatement  



  
  procedure CompileCaseStatement(LoopNesting: Integer);
  var
    SelectorType, ConstValType: Integer;
    NumCaseStatements: Integer;
    ConstVal, ConstVal2: TConst;
    
  begin
  NextTok;
  
  CompileExpression(SelectorType);
  if not (Types[SelectorType].Kind in OrdinalTypes) then
    Error('Ordinal variable expected as CASE selector');
  
  EatTok(OFTOK);

  GenerateCaseProlog;  

  NumCaseStatements := 0;

  repeat       // Loop over all cases

    repeat     // Loop over all constants for the current case
      CompileConstExpression(ConstVal, ConstValType);
      GetCompatibleType(ConstValType, SelectorType);

      if Tok.Kind = RANGETOK then                                      // Range check
        begin
        NextTok;
        CompileConstExpression(ConstVal2, ConstValType);
        GetCompatibleType(ConstValType, SelectorType);
        GenerateCaseRangeCheck(ConstVal.OrdValue, ConstVal2.OrdValue);
        end
      else
        GenerateCaseEqualityCheck(ConstVal.OrdValue);                     // Equality check

      if Tok.Kind <> COMMATOK then Break;
      NextTok;
    until FALSE;

    EatTok(COLONTOK);

    GenerateCaseStatementProlog;
    CompileStatement(LoopNesting);
    GenerateCaseStatementEpilog;

    Inc(NumCaseStatements);
    
    if (Tok.Kind = ELSETOK) or (Tok.Kind = ENDTOK) then Break;
    
    EatTok(SEMICOLONTOK);
  until (Tok.Kind = ELSETOK) or (Tok.Kind = ENDTOK);  
  
  // Default statements
  if Tok.Kind = ELSETOK then              
    begin
    NextTok;
    CompileStatementList(LoopNesting);
    end;          

  EatTok(ENDTOK);

  GenerateCaseEpilog(NumCaseStatements);
  end; // CompileCaseStatement
  
  
  
  
  procedure CompileWhileStatement(LoopNesting: Integer);
  var
    ExpressionType: Integer;
    
  begin
  SaveCodePos;      // Save return address used by GenerateWhileEpilog

  NextTok;
  CompileExpression(ExpressionType);
  GetCompatibleType(ExpressionType, BOOLEANTYPEINDEX);
  
  EatTok(DOTOK);

  GenerateBreakProlog(LoopNesting);
  GenerateContinueProlog(LoopNesting);
  GenerateWhileCondition;                         // Satisfied if expression is not zero
  GenerateWhileProlog;
  
  CompileStatement(LoopNesting);
  
  GenerateContinueEpilog(LoopNesting);
  GenerateWhileEpilog;
  GenerateBreakEpilog(LoopNesting);
  end; // CompileWhileStatement
  
  
  
  
  procedure CompileRepeatStatement(LoopNesting: Integer);
  var
    ExpressionType: Integer;
    
  begin
  GenerateBreakProlog(LoopNesting);
  GenerateContinueProlog(LoopNesting);
  GenerateRepeatProlog;

  NextTok;
  CompileStatementList(LoopNesting);

  EatTok(UNTILTOK);
  
  GenerateContinueEpilog(LoopNesting);

  CompileExpression(ExpressionType);
  GetCompatibleType(ExpressionType, BOOLEANTYPEINDEX);
  
  GenerateRepeatCondition;
  GenerateRepeatEpilog;
  GenerateBreakEpilog(LoopNesting);
  end; // CompileRepeatStatement
  
  
  
  
  procedure CompileForStatement(LoopNesting: Integer);
  var
    CounterIndex: Integer;
    ExpressionType: Integer;
    Down: Boolean;
      
  begin
  NextTok;
  
  AssertIdent;
  CounterIndex := GetIdent(Tok.Name);

  if (Ident[CounterIndex].Kind <> VARIABLE) or
    ((Ident[CounterIndex].NestingLevel <> 1) and (Ident[CounterIndex].NestingLevel <> BlockStackTop)) or
     (Ident[CounterIndex].PassMethod <> EMPTYPASSING) then
    Error('Simple local variable expected as FOR loop counter');

  if not (Types[Ident[CounterIndex].DataType].Kind in OrdinalTypes) then
    Error('Ordinal variable expected as FOR loop counter');
    
  PushVarIdentPtr(CounterIndex);
  
  NextTok;
  EatTok(ASSIGNTOK);
  
  // Initial counter value
  CompileExpression(ExpressionType);
  GetCompatibleType(ExpressionType, Ident[CounterIndex].DataType);  

  if not (Tok.Kind in [TOTOK, DOWNTOTOK]) then
    CheckTok(TOTOK);

  Down := Tok.Kind = DOWNTOTOK;
  NextTok;
  
  // Final counter value
  CompileExpression(ExpressionType);
  GetCompatibleType(ExpressionType, Ident[CounterIndex].DataType);
  
  // Assign initial value to the counter, compute and save the total number of iterations
  GenerateForAssignmentAndNumberOfIterations(Ident[CounterIndex].DataType, Down);
  
  // Save return address used by GenerateForEpilog
  SaveCodePos;
  
  // Check the remaining number of iterations
  GenerateForCondition;

  EatTok(DOTOK);
  
  GenerateBreakProlog(LoopNesting);
  GenerateContinueProlog(LoopNesting);
  GenerateForProlog;
  
  CompileStatement(LoopNesting);    
  
  GenerateContinueEpilog(LoopNesting);
  
  PushVarIdentPtr(CounterIndex);         
  GenerateForEpilog(Ident[CounterIndex].DataType, Down);
  
  GenerateBreakEpilog(LoopNesting);
  
  // Pop and discard the remaining number of iterations (i.e. zero)
  DiscardStackTop(1);                                          
  end; // CompileForStatement

  
  
  
  procedure CompileGotoStatement(LoopNesting: Integer);
  var
    LabelIndex: Integer;
    
  begin
  NextTok;
  
  AssertIdent;
  LabelIndex := GetIdent(Tok.Name);
  
  if Ident[LabelIndex].Kind <> GOTOLABEL then
    Error('Label expected');
    
  if Ident[LabelIndex].Block <> BlockStack[BlockStackTop].Index then
    Error('Label is not declared in current procedure');
    
  GenerateGoto(LabelIndex);

  NextTok;
  end; // CompileGotoStatement



  
  procedure CompileWithStatement(LoopNesting: Integer);
  var
    DesignatorType: Integer;
    DeltaWithNesting: Integer;
    TempStorageAddr: Integer;
    IsConst: Boolean;
    
  begin
  NextTok;  
  DeltaWithNesting := 0; 

  repeat   
    // Save designator pointer to temporary storage
    TempStorageAddr := AllocateTempStorage(TypeSize(POINTERTYPEINDEX));    
    PushTempStoragePtr(TempStorageAddr);
    
    CompileBasicDesignator(DesignatorType, IsConst);
    CompileSelectors(DesignatorType);
    if not (Types[DesignatorType].Kind in [RECORDTYPE, INTERFACETYPE]) then
      Error('Record or interface expected');
      
    GenerateAssignment(POINTERTYPEINDEX);

    // Save designator info
    Inc(DeltaWithNesting);
    Inc(WithNesting);
    
    if WithNesting > MAXWITHNESTING then
      Error('Maximum WITH block nesting exceeded');
    
    WithStack[WithNesting].TempPointer := TempStorageAddr;
    WithStack[WithNesting].DataType := DesignatorType;
    WithStack[WithNesting].IsConst := IsConst;    
    
    if Tok.Kind <> COMMATOK then Break;
    NextTok;
  until FALSE;
  
  EatTok(DOTOK);
  
  CompileStatement(LoopNesting);
  
  WithNesting := WithNesting - DeltaWithNesting;
  end; // CompileWithStatement
  
  
  
  
  function IsCurrentOrOuterFunc(FuncIdentIndex: Integer): Boolean;
  var
    BlockStackIndex: Integer;
  begin
  if Ident[FuncIdentIndex].Kind = FUNC then
    for BlockStackIndex := BlockStackTop downto 1 do
      if BlockStack[BlockStackIndex].Index = Ident[FuncIdentIndex].ProcAsBlock then
          begin
          Result := TRUE;
          Exit;
          end;        
  Result := FALSE;
  end; // IsCurrentOrOuterFunc



  
var
  IdentIndex: Integer;
  DesignatorType: Integer;
  DesignatorIsStatement: Boolean;
  
  
begin // CompileStatement
CompileLabel;

case Tok.Kind of

  IDENTTOK:
    begin   
    if FieldOrMethodInsideWithFound(Tok.Name) then                      // Record field or method inside a WITH block
      begin
      DesignatorIsStatement := CompileDesignator(DesignatorType, FALSE);
      CompileAssignmentOrCall(DesignatorType, DesignatorIsStatement);
      end 
    else                                                                // Ordinary identifier                                                                                
      begin  
      IdentIndex := GetIdent(Tok.Name);
      
      case Ident[IdentIndex].Kind of
      
        VARIABLE, USERTYPE:                                             // Assignment or procedural variable call
          begin
          DesignatorIsStatement := CompileDesignator(DesignatorType, FALSE);
          CompileAssignmentOrCall(DesignatorType, DesignatorIsStatement); 
          end;

        PROC, FUNC:                                                     // Procedure or function call (returned result discarded)
          if Ident[IdentIndex].PredefProc <> EMPTYPROC then             // Predefined procedure
            begin
            if Ident[IdentIndex].Kind <> PROC then
              Error('Procedure expected but predefined function ' + Ident[IdentIndex].Name + ' found');            
            CompilePredefinedProc(Ident[IdentIndex].PredefProc, LoopNesting)
            end
          else                                                          // User-defined procedure or function
            begin
            NextTok;
            
            if Tok.Kind = ASSIGNTOK then                                // Special case: assignment to a function name
              begin
              if not IsCurrentOrOuterFunc(IdentIndex) then
                Error('Function name expected but ' + Ident[IdentIndex].Name + ' found');

              // Push pointer to Result
              PushVarIdentPtr(Ident[IdentIndex].ResultIdentIndex);              
              DesignatorType := Ident[Ident[IdentIndex].ResultIdentIndex].DataType;
              
              if Ident[Ident[IdentIndex].ResultIdentIndex].PassMethod = VARPASSING then
                DerefPtr(POINTERTYPEINDEX);                        

              CompileAssignment(DesignatorType);
              end
            else                                                        // General rule: procedure or function call
              begin  
              CompileCall(IdentIndex);
              
              DesignatorType := Ident[IdentIndex].Signature.ResultType;
              
              if (Ident[IdentIndex].Kind = FUNC) and (Tok.Kind in [DEREFERENCETOK, OBRACKETTOK, PERIODTOK, OPARTOK]) and
                 ((Types[DesignatorType].Kind in StructuredTypes) or DereferencePointerAsDesignator(DesignatorType, FALSE))                                   
              then
                begin
                PushFunctionResult(DesignatorType);
                DesignatorIsStatement := CompileSelectors(DesignatorType, TRUE);
                CompileAssignmentOrCall(DesignatorType, DesignatorIsStatement); 
                end;
              end;              
              
            end;  
                 
      else
        Error('Statement expected but ' + Ident[IdentIndex].Name + ' found');
      end // case Ident[IdentIndex].Kind
      end; // else
    end;    

  BEGINTOK:
    CompileCompoundStatement(LoopNesting);    

  IFTOK:
    CompileIfStatement(LoopNesting);    

  CASETOK:
    CompileCaseStatement(LoopNesting);  

  WHILETOK:
    CompileWhileStatement(LoopNesting + 1);

  REPEATTOK:
    CompileRepeatStatement(LoopNesting + 1);    

  FORTOK:
    CompileForStatement(LoopNesting + 1);
    
  GOTOTOK:
    CompileGotoStatement(LoopNesting);

  WITHTOK:
    CompileWithStatement(LoopNesting);

end;// case

end;// CompileStatement




procedure CompileType(var DataType: Integer);


  procedure CompileEnumeratedType(var DataType: Integer);
  var
    ConstIndex: Integer;
  begin
  // Add new anonymous type
  DeclareType(ENUMERATEDTYPE);
  DataType := NumTypes;

  // Compile enumeration constants
  ConstIndex := 0;
  NextTok;
  
  repeat
    AssertIdent;
    DeclareIdent(Tok.Name, CONSTANT, 0, FALSE, DataType, EMPTYPASSING, ConstIndex, 0.0, '', [], EMPTYPROC, '', 0);
    
    Inc(ConstIndex);
    if ConstIndex > MAXENUMELEMENTS - 1 then
      Error('Too many enumeration elements');
      
    NextTok;
    
    if Tok.Kind <> COMMATOK then Break;
    NextTok;
  until FALSE;
  
  EatTok(CPARTOK);
  
  Types[DataType].Low  := 0;
  Types[DataType].High := ConstIndex - 1;
  end; // CompileEnumeratedType




  procedure CompileTypedPointerType(var DataType: Integer);
  var
    NestedDataType: Integer;
  begin
  // Add new anonymous type
  DeclareType(POINTERTYPE);
  DataType := NumTypes;

  // Compile pointer base type
  NextTok;
  CompileTypeIdent(NestedDataType, TRUE);
    
  Types[DataType].BaseType := NestedDataType;
  end; // CompileTypedPointerType
  
  
  
  
  procedure CompileArrayType(var DataType: Integer);
  var
    ArrType, IndexType, NestedDataType: Integer;
  begin
  NextTok;
  EatTok(OBRACKETTOK);

  DataType := NumTypes + 1;

  repeat
    // Add new anonymous type
    DeclareType(ARRAYTYPE);
    Types[NumTypes].IsOpenArray := FALSE;
    ArrType := NumTypes;

    CompileType(IndexType);
    if not (Types[IndexType].Kind in OrdinalTypes) then
      Error('Ordinal type expected');
    Types[ArrType].IndexType := IndexType;

    if Tok.Kind <> COMMATOK then Break;
    
    Types[ArrType].BaseType := NumTypes + 1;
    NextTok;
  until FALSE;

  EatTok(CBRACKETTOK);
  EatTok(OFTOK);

  CompileType(NestedDataType);
  Types[ArrType].BaseType := NestedDataType;  
  end; // CompileArrayType
  
  
  
  
  procedure CompileRecordOrInterfaceType(var DataType: Integer; IsInterfaceType: Boolean);
  

    procedure DeclareField(const FieldName: TString; RecType, FieldType: Integer; var NextFieldOffset: Integer);
    var
      i, FieldTypeSize: Integer;      
    begin
    for i := 1 to Types[RecType].NumFields do
      if Types[RecType].Field[i]^.Name = FieldName then
        Error('Duplicate field ' + FieldName);

    // Add new field
    Inc(Types[RecType].NumFields);
    if Types[RecType].NumFields > MAXFIELDS then
      Error('Too many fields');
      
    New(Types[RecType].Field[Types[RecType].NumFields]);
    
    with Types[RecType].Field[Types[RecType].NumFields]^ do
      begin
      Name     := FieldName;
      DataType := FieldType;
      Offset   := NextFieldOffset;            
      end;
      
    // For interfaces, save Self pointer offset from the procedural field
    if Types[RecType].Kind = INTERFACETYPE then
      Types[FieldType].SelfPointerOffset := -NextFieldOffset;      
    
    FieldTypeSize := TypeSize(FieldType);
    if FieldTypeSize > HighBound(INTEGERTYPEINDEX) - NextFieldOffset then
      Error('Type size is too large');

    NextFieldOffset := NextFieldOffset + FieldTypeSize;
    end; // DeclareField
    
    
    
    procedure CompileFixedFields(RecType: Integer; var NextFieldOffset: Integer);
    var
      FieldInListName: array [1..MAXFIELDS] of TString;
      NumFieldsInList, FieldInListIndex: Integer;
      FieldType: Integer;
      
    begin      
    while not (Tok.Kind in [CASETOK, ENDTOK, CPARTOK]) do
      begin
      NumFieldsInList := 0;
      
      repeat
        AssertIdent;

        Inc(NumFieldsInList);
        if NumFieldsInList > MAXFIELDS then
          Error('Too many fields');
          
        FieldInListName[NumFieldsInList] := Tok.Name;

        NextTok;
        if (Tok.Kind <> COMMATOK) or IsInterfaceType then Break;
        NextTok;
      until FALSE;

      EatTok(COLONTOK);

      CompileType(FieldType);
      
      if IsInterfaceType and (Types[FieldType].Kind <> PROCEDURALTYPE) then
        Error('Non-procedural fields are not allowed in interfaces');      

      for FieldInListIndex := 1 to NumFieldsInList do
        DeclareField(FieldInListName[FieldInListIndex], DataType, FieldType, NextFieldOffset);

      if Tok.Kind <> SEMICOLONTOK then Break; 
      NextTok;
      end; // while
    
    end; // CompileFixedFields
    
    
        
    procedure CompileFields(RecType: Integer; var NextFieldOffset: Integer);    
    var
      TagName: TString;
      TagVal: TConst;
      TagType, TagValType: Integer;
      TagTypeIdentIndex: Integer;
      VariantStartOffset: Integer;
    
    begin
    // Fixed fields
    CompileFixedFields(DataType, NextFieldOffset);
    
    // Variant fields
    if (Tok.Kind = CASETOK) and not IsInterfaceType then
      begin   
      NextTok;
      
      // Tag field
      AssertIdent;
      TagTypeIdentIndex := GetIdentUnsafe(Tok.Name);
      
      if (TagTypeIdentIndex <> 0) and (Ident[TagTypeIdentIndex].Kind = USERTYPE) then      // Type name found
        begin
        TagType := Ident[TagTypeIdentIndex].DataType;
        NextTok;
        end
      else                                                                                 // Field name found  
        begin
        TagName := Tok.Name;
        NextTok;
        EatTok(COLONTOK);      
        CompileType(TagType);           
        DeclareField(TagName, DataType, TagType, NextFieldOffset);
        end;
        
      if not (Types[TagType].Kind in OrdinalTypes) then
        Error('Ordinal type expected');
    
      VariantStartOffset := NextFieldOffset;    
      EatTok(OFTOK);
      
      // Variants
      repeat
        repeat
          CompileConstExpression(TagVal, TagValType);
          GetCompatibleType(TagType, TagValType);
          if Tok.Kind <> COMMATOK then Break;
          NextTok; 
        until FALSE;
    
        EatTok(COLONTOK);
        EatTok(OPARTOK);
        
        NextFieldOffset := VariantStartOffset;
        CompileFields(DataType, NextFieldOffset);
        
        EatTok(CPARTOK);      
        if (Tok.Kind = CPARTOK) or (Tok.Kind = ENDTOK) then Break;
 
        EatTok(SEMICOLONTOK);      
      until (Tok.Kind = CPARTOK) or (Tok.Kind = ENDTOK);
      
      end; // if    
    end; // CompileFields    
    

  
  var
    NextFieldOffset: Integer;
    
  
  begin // CompileRecordOrInterfaceType
  NextFieldOffset := 0;
  
  // Add new anonymous type  
  if IsInterfaceType then
    DeclareType(INTERFACETYPE)
  else
    DeclareType(RECORDTYPE);
  
  Types[NumTypes].NumFields := 0;
  DataType := NumTypes;
  
  // Declare hidden Self pointer for interfaces
  if IsInterfaceType then
    DeclareField('SELF', DataType, POINTERTYPEINDEX, NextFieldOffset);  

  NextTok;
  CompileFields(DataType, NextFieldOffset);    
  EatTok(ENDTOK);
  end; // CompileRecordOrInterfaceType




  procedure CompileSetType(var DataType: Integer);
  var
    NestedDataType: Integer;
  begin
  // Add new anonymous type
  DeclareType(SETTYPE);
  DataType := NumTypes;
  
  NextTok;
  EatTok(OFTOK);

  CompileType(NestedDataType);
  
  if (LowBound(NestedDataType) < 0) or (HighBound(NestedDataType) > MAXSETELEMENTS - 1) then
    Error('Too many set elements');
  
  Types[DataType].BaseType := NestedDataType; 
  end; // CompileSetType
  
  
  

  procedure CompileStringType(var DataType: Integer);
  var
    LenConstVal: TConst;
    LenType, IndexType: Integer;    
  begin
  NextTok;    
  
  if Tok.Kind = OBRACKETTOK then
    begin
    NextTok;
    CompileConstExpression(LenConstVal, LenType);
    
    if not (Types[LenType].Kind in IntegerTypes) then
      Error('Integer type expected'); 
      
    if (LenConstVal.OrdValue <= 0) or (LenConstVal.OrdValue > MAXSTRLENGTH) then
      Error('Illegal string length');  
    
    // Add new anonymous type: 1..Len + 1
    DeclareType(SUBRANGETYPE);
    IndexType := NumTypes;

    Types[IndexType].BaseType := LenType;
    Types[IndexType].Low      := 1;
    Types[IndexType].High     := LenConstVal.OrdValue + 1;
    
    // Add new anonymous type: array [1..Len + 1] of Char
    DeclareType(ARRAYTYPE);
    DataType := NumTypes;

    Types[DataType].BaseType    := CHARTYPEINDEX;
    Types[DataType].IndexType   := IndexType;
    Types[DataType].IsOpenArray := FALSE;
    
    EatTok(CBRACKETTOK);
    end
  else
    DataType := STRINGTYPEINDEX;  

  end; // CompileStringType



  
  procedure CompileFileType(var DataType: Integer);
  var
    NestedDataType: Integer;
  begin
  NextTok;
  
  if Tok.Kind = OFTOK then          // Typed file
    begin
    NextTok;
    CompileType(NestedDataType);
    
    if Types[NestedDataType].Kind = FILETYPE then
      Error('File of files is not allowed'); 
   
    // Add new anonymous type
    DeclareType(FILETYPE);    
    Types[NumTypes].BaseType := NestedDataType;
    
    DataType := NumTypes;
    end
  else                              // Untyped/text file
    DataType := FILETYPEINDEX; 
 
  end; // CompileFileType
  



  procedure CompileSubrangeType(var DataType: Integer);
  var
    ConstVal: TConst;
    LowBoundType, HighBoundType: Integer;
  begin
  // Add new anonymous type
  DeclareType(SUBRANGETYPE);
  DataType := NumTypes;

  CompileConstExpression(ConstVal, LowBoundType);                               // Subrange lower bound
  if not (Types[LowBoundType].Kind in OrdinalTypes + [SUBRANGETYPE]) then
    Error('Ordinal type expected');
  Types[DataType].Low := ConstVal.OrdValue;

  EatTok(RANGETOK);

  CompileConstExpression(ConstVal, HighBoundType);                              // Subrange upper bound
  if not (Types[HighBoundType].Kind in OrdinalTypes + [SUBRANGETYPE]) then
    Error('Ordinal type expected');
  Types[DataType].High := ConstVal.OrdValue;

  GetCompatibleType(LowBoundType, HighBoundType);

  if Types[DataType].High < Types[DataType].Low then
    Error('Illegal subrange bounds');

  Types[DataType].BaseType := LowBoundType;  
  end; // CompileSubrangeType
  
  
  
  
  procedure CompileProceduralType(var DataType: Integer; IsFunction: Boolean);
  begin
  DeclareType(PROCEDURALTYPE);
  Types[NumTypes].MethodIdentIndex := 0;
  DataType := NumTypes;
  
  NextTok;
  
  CompileFormalParametersAndResult(IsFunction, Types[DataType].Signature);  
  end; // CompileProceduralType
  
  

var
  IdentIndex: LongInt;
  TypeNameGiven: Boolean;   


begin // CompileType

if Tok.Kind = PACKEDTOK then        // PACKED has no effect
  begin
  NextTok;
  if not (Tok.Kind in [ARRAYTOK, RECORDTOK, INTERFACETOK, SETTOK, FILETOK]) then
    Error('PACKED is not allowed here');
  end;
 
case Tok.Kind of

  OPARTOK:
    CompileEnumeratedType(DataType);
    
  DEREFERENCETOK: 
    CompileTypedPointerType(DataType);
  
  ARRAYTOK:       
    CompileArrayType(DataType); 
 
  RECORDTOK, INTERFACETOK:      
    CompileRecordOrInterfaceType(DataType, Tok.Kind = INTERFACETOK);
    
  SETTOK:      
    CompileSetType(DataType); 
   
  STRINGTOK:
    CompileStringType(DataType);
    
  FILETOK:
    CompileFileType(DataType);
    
  PROCEDURETOK, FUNCTIONTOK:
    CompileProceduralType(DataType, Tok.Kind = FUNCTIONTOK)
     
else                                                                              // Subrange or type name
  TypeNameGiven := FALSE;
  IdentIndex := 0;
  if Tok.Kind = IDENTTOK then      
    begin
    IdentIndex := GetIdent(Tok.Name);
    if Ident[IdentIndex].Kind = USERTYPE then TypeNameGiven := TRUE;
    end;

  if TypeNameGiven then                                                           // Type name
    begin
    DataType := Ident[IdentIndex].DataType;
    NextTok;
    end
  else                                                                            // Subrange
    CompileSubrangeType(DataType);
    
end; // case  

end;// CompileType




procedure CompileBlock(BlockIdentIndex: Integer);



  procedure ResolveForwardReferences;
  var
    TypeIndex, TypeIdentIndex, FieldIndex: Integer;
    DataType: Integer;    
  begin 
  for TypeIndex := 1 to NumTypes do
    if Types[TypeIndex].Kind = FORWARDTYPE then
      begin
      TypeIdentIndex := GetIdent(Types[TypeIndex].TypeIdentName);     
      
      if Ident[TypeIdentIndex].Kind <> USERTYPE then
        Error('Type name expected');
        
      // Forward reference resolution
      DataType := Ident[TypeIdentIndex].DataType;
      
      Types[TypeIndex] := Types[DataType];
      Types[TypeIndex].AliasType := DataType;
      
      if Types[DataType].Kind in [RECORDTYPE, INTERFACETYPE] then
        for FieldIndex := 1 to Types[DataType].NumFields do
          begin
          New(Types[TypeIndex].Field[FieldIndex]);
          Types[TypeIndex].Field[FieldIndex]^ := Types[DataType].Field[FieldIndex]^;
          end;
      end; // if    
  end; // ResolveForwardReferences




  procedure CompileInitializer(InitializedDataOffset: LongInt; ConstType: Integer);
  var
    ConstVal: TConst;
    ConstValType: Integer;
    NumElements, ElementIndex, FieldIndex: Integer;
    
  begin
  // Numbers
  if Types[ConstType].Kind in OrdinalTypes + [REALTYPE, SINGLETYPE] then
    begin
    CompileConstExpression(ConstVal, ConstValType);

    // Try to convert integer to double or double to single
    ConvertConstIntegerToReal(ConstType, ConstValType, ConstVal);
    ConvertConstRealToReal(ConstType, ConstValType, ConstVal);          
    GetCompatibleType(ConstType, ConstValType); 
      
    if Types[ConstType].Kind = REALTYPE then
      Move(ConstVal.RealValue, InitializedGlobalData[InitializedDataOffset], TypeSize(ConstType))
    else if Types[ConstType].Kind = SINGLETYPE then 
      Move(ConstVal.SingleValue, InitializedGlobalData[InitializedDataOffset], TypeSize(ConstType)) 
    else
      Move(ConstVal.OrdValue, InitializedGlobalData[InitializedDataOffset], TypeSize(ConstType));
    end
    
  // Arrays
  else if Types[ConstType].Kind = ARRAYTYPE then
    begin
    
    if IsString(ConstType) and (Tok.Kind <> OPARTOK) then         // Special case: strings
      begin
      CompileConstExpression(ConstVal, ConstValType);
      ConvertConstCharToString(ConstType, ConstValType, ConstVal);
      GetCompatibleType(ConstType, ConstValType);
      
      if Length(ConstVal.StrValue) > TypeSize(ConstType) - 1 then
        Error('String is too long');
        
      DefineStaticString(ConstVal.StrValue, InitializedDataOffset, InitializedDataOffset);
      end
    else                                                          // General rule
      begin
      EatTok(OPARTOK);
      
      NumElements := HighBound(Types[ConstType].IndexType) - LowBound(Types[ConstType].IndexType) + 1;
      for ElementIndex := 1 to NumElements do
        begin
        CompileInitializer(InitializedDataOffset, Types[ConstType].BaseType);
        InitializedDataOffset := InitializedDataOffset + TypeSize(Types[ConstType].BaseType);
        
        if ElementIndex < NumElements then 
          EatTok(COMMATOK)
        else
          EatTok(CPARTOK);  
        end; // for
      end; // else

    end
    
  // Records
  else if Types[ConstType].Kind = RECORDTYPE then
    begin
    EatTok(OPARTOK);
    
    repeat
      AssertIdent;
      FieldIndex := GetField(ConstType, Tok.Name);
      
      NextTok;
      EatTok(COLONTOK);
      
      CompileInitializer(InitializedDataOffset + Types[ConstType].Field[FieldIndex]^.Offset, Types[ConstType].Field[FieldIndex]^.DataType);          
      
      if Tok.Kind <> SEMICOLONTOK then Break;
      NextTok; 
    until FALSE;
    
    EatTok(CPARTOK);
    end
    
  // Sets
  else if Types[ConstType].Kind = SETTYPE then
    begin
    CompileConstExpression(ConstVal, ConstValType);
    GetCompatibleType(ConstType, ConstValType);
    DefineStaticSet(ConstVal.SetValue, InitializedDataOffset, InitializedDataOffset);
    end        
 
  else
    Error('Illegal type');         

  end; // CompileInitializer    



 
  procedure CompileLabelDeclarations;
  begin
  repeat
    AssertIdent;
    
    DeclareIdent(Tok.Name, GOTOLABEL, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
    
    NextTok;
    if Tok.Kind <> COMMATOK then Break;
    NextTok;
  until FALSE;
  
  EatTok(SEMICOLONTOK);
  end; // CompileLabelDeclarations


 
  
  procedure CompileConstDeclarations;
  
  
    procedure CompileUntypedConstDeclaration(var NameTok: TToken);
    var
      ConstVal: TConst;
      ConstValType: Integer;    
    begin
    EatTok(EQTOK);    
    CompileConstExpression(ConstVal, ConstValType);
    DeclareIdent(NameTok.Name, CONSTANT, 0, FALSE, ConstValType, EMPTYPASSING, ConstVal.OrdValue, ConstVal.RealValue, ConstVal.StrValue, ConstVal.SetValue, EMPTYPROC, '', 0);
    end; // CompileUntypedConstDeclaration;
   
    
    procedure CompileTypedConstDeclaration(var NameTok: TToken);
    var
      ConstType: Integer;    
    begin
    EatTok(COLONTOK);    
    CompileType(ConstType);    
    DeclareIdent(NameTok.Name, CONSTANT, 0, FALSE, ConstType, VARPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);    
    EatTok(EQTOK);    
    CompileInitializer(Ident[NumIdent].Address, ConstType);   
    end; // CompileTypedConstDeclaration    


  var
    NameTok: TToken; 
   
  begin // CompileConstDeclarations
  repeat
    AssertIdent;

    NameTok := Tok;
    NextTok;
    
    if Tok.Kind = EQTOK then
      CompileUntypedConstDeclaration(NameTok)
    else
      CompileTypedConstDeclaration(NameTok);

    EatTok(SEMICOLONTOK);
  until Tok.Kind <> IDENTTOK;
  end; // CompileConstDeclarations
  
  

  
  procedure CompileTypeDeclarations;
  var
    NameTok: TToken;
    VarType: Integer;
  begin
  repeat
    AssertIdent;

    NameTok := Tok;
    NextTok;
    EatTok(EQTOK);

    CompileType(VarType);
    DeclareIdent(NameTok.Name, USERTYPE, 0, FALSE, VarType, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);   
    
    EatTok(SEMICOLONTOK);
  until Tok.Kind <> IDENTTOK;

  ResolveForwardReferences;
  end; // CompileTypeDeclarations
  
  
  
  
  procedure CompileVarDeclarations;
  var
    IdentInListName: array [1..MAXPARAMS] of TString;
    NumIdentInList, IdentInListIndex: Integer;
    VarType: Integer;
  begin
  repeat
    NumIdentInList := 0;
    repeat
      AssertIdent;

      Inc(NumIdentInList);
      if NumIdentInList > MAXPARAMS then
        Error('Too many variables in one list');
      
      IdentInListName[NumIdentInList] := Tok.Name;

      NextTok;

      if Tok.Kind <> COMMATOK then Break;
      NextTok;
    until FALSE;

    EatTok(COLONTOK);

    CompileType(VarType);

    if Tok.Kind = EQTOK then                                     // Initialized variable (equivalent to a typed constant, but mutable)
      begin
      if BlockStack[BlockStackTop].Index <> 1 then
        Error('Local variables cannot be initialized');
        
      if NumIdentInList <> 1 then
        Error('Multiple variables cannot be initialized');
        
      NextTok;
      DeclareIdent(IdentInListName[1], CONSTANT, 0, FALSE, VarType, VARPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
      Ident[NumIdent].IsTypedConst := FALSE;  // Allow mutability
      CompileInitializer(Ident[NumIdent].Address, VarType);      
      end
    else                                                         // Uninitialized variables   
      for IdentInListIndex := 1 to NumIdentInList do
        DeclareIdent(IdentInListName[IdentInListIndex], VARIABLE, 0, FALSE, VarType, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);  
      
    EatTok(SEMICOLONTOK);
  until Tok.Kind <> IDENTTOK;

  ResolveForwardReferences;
  end; // CompileVarDeclarations




  procedure CompileProcFuncDeclarations(IsFunction: Boolean);

    
    function CompileDirective(const ImportFuncName: TString): Boolean;
    var
      ImportLibNameConst: TConst;
      ImportLibNameConstValType: Integer;
      
    begin
    Result := FALSE;
    
    if Tok.Kind = IDENTTOK then
      if Tok.Name = 'EXTERNAL' then      // External (Windows API) declaration  
        begin
        if BlockStackTop <> 1 then
          Error('External declaration must be global');
          
        // Read import library name
        NextTok;      
        CompileConstExpression(ImportLibNameConst, ImportLibNameConstValType);
        if not IsString(ImportLibNameConstValType) then
          Error('Library name expected');      
        
        // Register import function
        GenerateImportFuncStub(AddImportFunc(ImportLibNameConst.StrValue, ImportFuncName));
        
        EatTok(SEMICOLONTOK);
        Result := TRUE;
        end
      else if Tok.Name = 'FORWARD' then  // Forward declaration
        begin
        Inc(NumBlocks);         
        Ident[NumIdent].ProcAsBlock := NumBlocks;
        Ident[NumIdent].IsUnresolvedForward := TRUE;
        GenerateForwardReference;
        
        NextTok;
        EatTok(SEMICOLONTOK);
        Result := TRUE;
        end
      else
        Error('Unknown directive ' + Tok.Name);  
    end; // CompileDirective
    
    


    function CompileInterface: Boolean;
    begin
    Result := FALSE;
    
    // Procedure interface in the interface section of a unit is an implicit forward declaration
    if ParserState.IsInterfaceSection and (BlockStack[BlockStackTop].Index = 1) then
      begin 
      Inc(NumBlocks);      
      Ident[NumIdent].ProcAsBlock := NumBlocks;
      Ident[NumIdent].IsUnresolvedForward := TRUE;
      GenerateForwardReference;

      Result := TRUE;
      end;
      
    end; // CompileInterface

    
  
  var
    ForwardIdentIndex, FieldIndex: Integer;
    ReceiverType: Integer;
    ProcOrFunc: TIdentKind;
    ProcName, NonUppercaseProcName, ReceiverName: TString;
    ForwardResolutionSignature: TSignature;
    
    
  begin // CompileProcFuncDeclarations   
  AssertIdent;
  ProcName := Tok.Name;
  NonUppercaseProcName := Tok.NonUppercaseName;
  NextTok;
  
  // Check for method declaration
  ReceiverName := '';
  ReceiverType := 0;
  
  if Tok.Kind = FORTOK then
    begin
    NextTok;
    AssertIdent;
    ReceiverName := Tok.Name;

    NextTok;
    EatTok(COLONTOK);
    CompileTypeIdent(ReceiverType, FALSE);
    
    if not (Types[ReceiverType].Kind in StructuredTypes) then
      Error('Structured type expected');
    
    if BlockStack[BlockStackTop].Index <> 1 then
      Error('Methods cannot be nested');

    if Types[ReceiverType].Kind in [RECORDTYPE, INTERFACETYPE] then
      begin
      FieldIndex := GetFieldUnsafe(ReceiverType, ProcName);
      if FieldIndex <> 0 then
        Error('Duplicate field');
      end;    
    end;  

  // Check for forward declaration resolution
  if ReceiverType <> 0 then
    ForwardIdentIndex := GetMethodUnsafe(ReceiverType, ProcName)
  else
    ForwardIdentIndex := GetIdentUnsafe(ProcName);
  
  // Possibly found an identifier of another kind or scope, or it is already resolved
  if ForwardIdentIndex <> 0 then
    if not Ident[ForwardIdentIndex].IsUnresolvedForward or
       (Ident[ForwardIdentIndex].Block <> BlockStack[BlockStackTop].Index) or
       ((Ident[ForwardIdentIndex].Kind <> PROC) and not IsFunction) or
       ((Ident[ForwardIdentIndex].Kind <> FUNC) and IsFunction) then
     ForwardIdentIndex := 0;

  // Procedure/function signature
  if ForwardIdentIndex <> 0 then                                      // Forward declaration resolution
    begin    
    CompileFormalParametersAndResult(IsFunction, ForwardResolutionSignature);
    CheckSignatures(Ident[ForwardIdentIndex].Signature, ForwardResolutionSignature, ProcName);
    DisposeParams(ForwardResolutionSignature);
    end
  else                                                                // Conventional declaration
    begin
    if IsFunction then ProcOrFunc := FUNC else ProcOrFunc := PROC;
    
    DeclareIdent(ProcName, ProcOrFunc, 0, FALSE, 0, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, ReceiverName, ReceiverType);
    CompileFormalParametersAndResult(IsFunction, Ident[NumIdent].Signature);

    if (ReceiverType <> 0) and (Ident[NumIdent].Signature.CallConv <> DEFAULTCONV) then
      Error('STDCALL/CDECL is not allowed for methods'); 
    end;           

  EatTok(SEMICOLONTOK);  
  
  // Procedure/function body, if any
  if ForwardIdentIndex <> 0 then                                                    // Forward declaration resolution
    begin
    if (ReceiverType <> 0) and (ReceiverName <> Ident[ForwardIdentIndex].ReceiverName) then
      Error('Incompatible receiver name');
   
    GenerateForwardResolution(Ident[ForwardIdentIndex].Address);
    
    CompileBlock(ForwardIdentIndex);
    EatTok(SEMICOLONTOK); 
    
    Ident[ForwardIdentIndex].IsUnresolvedForward := FALSE; 
    end  
  else if not CompileDirective(NonUppercaseProcName) and not CompileInterface then  // Declaration in the interface part, external or forward declaration                                                              
    begin
    Inc(NumBlocks);                                                                 // Conventional declaration   
    Ident[NumIdent].ProcAsBlock := NumBlocks;
    
    CompileBlock(NumIdent);    
    EatTok(SEMICOLONTOK);
    end;                                                               
   
  end; // CompileProcFuncDeclarations
  



  procedure CompileDeclarations;
  var
    DeclTok: TToken;
    ParamIndex, StackParamIndex: Integer;
    TotalParamSize: Integer;
    NestedProcsFound: Boolean;
    
    
    procedure DeclareResult;
    begin
    with Ident[BlockIdentIndex].Signature do
      if (Types[ResultType].Kind in StructuredTypes) and ((CallConv = DEFAULTCONV) or (TypeSize(ResultType) > 2 * SizeOf(LongInt))) then    // For functions returning structured variables, Result is a hidden VAR parameter 
        DeclareIdent('RESULT', VARIABLE, TotalParamSize, FALSE, ResultType, VARPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0)
      else                                                                                  // Otherwise, Result is a hidden local variable
        DeclareIdent('RESULT', VARIABLE, 0, FALSE, ResultType, EMPTYPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
      
    Ident[BlockIdentIndex].ResultIdentIndex := NumIdent;
    end; // DeclareResult

   
  begin  
  NestedProcsFound := FALSE; 
 
  // For procedures and functions, declare parameters and the Result variable
  
  // Default calling convention: ([var Self,] [var Result,] Parameter1, ... ParameterN [, StaticLink]), result returned in EAX
  // STDCALL calling convention: (ParameterN, ... Parameter1, [, var Result]), result returned in EAX, small structures in EDX:EAX, reals in ST(0)
  // CDECL calling convention:   equivalent to STDCALL, except that caller clears the stack
  
  if BlockStack[BlockStackTop].Index <> 1 then             
    begin
    TotalParamSize := GetTotalParamSize(Ident[BlockIdentIndex].Signature, Ident[BlockIdentIndex].ReceiverType <> 0, FALSE);
    
    // Declare Self
    if Ident[BlockIdentIndex].ReceiverType <> 0 then
      DeclareIdent(Ident[BlockIdentIndex].ReceiverName, VARIABLE, TotalParamSize, FALSE, Ident[BlockIdentIndex].ReceiverType, VARPASSING, 0, 0.0, '', [], EMPTYPROC, '', 0);
             
    // Declare Result (default calling convention)
    if (Ident[BlockIdentIndex].Kind = FUNC) and (Ident[BlockIdentIndex].Signature.CallConv = DEFAULTCONV) then
      DeclareResult;              
    
    // Allocate and declare other parameters
    for ParamIndex := 1 to Ident[BlockIdentIndex].Signature.NumParams do
      begin
      if Ident[BlockIdentIndex].Signature.CallConv = DEFAULTCONV then
        StackParamIndex := ParamIndex
      else  
        StackParamIndex := Ident[BlockIdentIndex].Signature.NumParams - ParamIndex + 1;    // Inverse parameter stack for STDCALL/CDECL procedures     
  
      DeclareIdent(Ident[BlockIdentIndex].Signature.Param[StackParamIndex]^.Name,
                   VARIABLE,
                   TotalParamSize,
                   Ident[BlockIdentIndex].Signature.CallConv <> DEFAULTCONV,
                   Ident[BlockIdentIndex].Signature.Param[StackParamIndex]^.DataType,
                   Ident[BlockIdentIndex].Signature.Param[StackParamIndex]^.PassMethod,
                   0,
                   0.0,
                   '',
                   [],
                   EMPTYPROC,
                   '', 
                   0);
      end;

    // Declare Result (STDCALL/CDECL calling convention)
    if (Ident[BlockIdentIndex].Kind = FUNC) and (Ident[BlockIdentIndex].Signature.CallConv <> DEFAULTCONV) then
      DeclareResult;
              
    end; // if

  
  // Loop over interface/implementation sections
  repeat
  
    // Local declarations
    while Tok.Kind in [LABELTOK, CONSTTOK, TYPETOK, VARTOK, PROCEDURETOK, FUNCTIONTOK] do
      begin
      DeclTok := Tok;
      NextTok;
      
      case DeclTok.Kind of
        LABELTOK:
          CompileLabelDeclarations;
          
        CONSTTOK:     
          CompileConstDeclarations;
          
        TYPETOK:      
          CompileTypeDeclarations;
          
        VARTOK:       
          CompileVarDeclarations;
          
        PROCEDURETOK, FUNCTIONTOK:
          begin
          if (BlockStack[BlockStackTop].Index <> 1) and not NestedProcsFound then
            begin
            NestedProcsFound := TRUE;
            GenerateNestedProcsProlog;
            end;
    
          CompileProcFuncDeclarations(DeclTok.Kind = FUNCTIONTOK);
          end;
      end; // case

      end;// while
      
      
    if ParserState.IsUnit and ParserState.IsInterfaceSection and (BlockStack[BlockStackTop].Index = 1) then
      begin
      EatTok(IMPLEMENTATIONTOK);
      ParserState.IsInterfaceSection := FALSE;
      end
    else
      Break;    
    
  until FALSE;      

    
  // Jump to entry point
  if NestedProcsFound then
    GenerateNestedProcsEpilog;
    
  end; // CompileDeclarations
  
  
  
  
  procedure CheckUnresolvedDeclarations;
  var
    IdentIndex: Integer;
  begin
  IdentIndex := NumIdent;
  
  while (IdentIndex > 0) and (Ident[IdentIndex].Block = BlockStack[BlockStackTop].Index) do
    begin
    if (Ident[IdentIndex].Kind in [GOTOLABEL, PROC, FUNC]) and Ident[IdentIndex].IsUnresolvedForward then
      Error('Unresolved declaration of ' + Ident[IdentIndex].Name);
    Dec(IdentIndex);
    end;
  end; // CheckUnresolvedDeclarations  
  
  


  procedure DeleteDeclarations;  
  begin  
  // Delete local identifiers
  while (NumIdent > 0) and (Ident[NumIdent].Block = BlockStack[BlockStackTop].Index) do
    begin
    // Warn if not used
    if not Ident[NumIdent].IsUsed and not Ident[NumIdent].IsExported and (Ident[NumIdent].Kind = VARIABLE) and (Ident[NumIdent].PassMethod = EMPTYPASSING) then
      Warning('Variable ' + Ident[NumIdent].Name + ' is not used');
  
    // If procedure or function, delete parameters first
    if Ident[NumIdent].Kind in [PROC, FUNC] then
      DisposeParams(Ident[NumIdent].Signature);

    // Delete identifier itself
    Dec(NumIdent);
    end;     
    
  // Delete local types
  while (NumTypes > 0) and (Types[NumTypes].Block = BlockStack[BlockStackTop].Index) do
    begin
    // If procedural type, delete parameters first
    if Types[NumTypes].Kind = PROCEDURALTYPE then
      DisposeParams(Types[NumTypes].Signature) 
    
    // If record or interface, delete fields first
    else if Types[NumTypes].Kind in [RECORDTYPE, INTERFACETYPE] then
      DisposeFields(Types[NumTypes]);    

    // Delete type itself
    Dec(NumTypes);
    end;
      
  end; // DeleteDeclarations




var
  LibProcIdentIndex: Integer;
  TotalParamSize: Integer;


begin // CompileBlock
Inc(BlockStackTop);

with BlockStack[BlockStackTop] do
  begin
  if BlockIdentIndex = 0 then Index := 1 else Index := Ident[BlockIdentIndex].ProcAsBlock;    
  LocalDataSize := 0;
  ParamDataSize := 0; 
  TempDataSize := 0;
  end;
  
if (ParserState.UnitStatus.Index = 1) and (BlockStack[BlockStackTop].Index = 1) then
  begin
  DeclarePredefinedTypes;
  DeclarePredefinedIdents;
  end; 
  
CompileDeclarations;

if ParserState.IsUnit and (BlockStack[BlockStackTop].Index = 1) then
  begin
  // Main block of a unit (may contain the implementation part, but not statements) 
    
  CheckUnresolvedDeclarations;    
  EatTok(ENDTOK);  
  end
else
  begin
  // Main block of a program, or a procedure as part of a program/unit  
  
  if BlockStack[BlockStackTop].Index = 1 then
    SetProgramEntryPoint;

  GenerateStackFrameProlog(Ident[BlockIdentIndex].Signature.CallConv <> DEFAULTCONV);

  if BlockStack[BlockStackTop].Index = 1 then          // Main program
    begin
    GenerateFPUInit;
    
    // Initialize heap and console I/O
    LibProcIdentIndex := GetIdent('INITSYSTEM');
    GenerateCall(Ident[LibProcIdentIndex].Address, BlockStackTop - 1, Ident[LibProcIdentIndex].NestingLevel);  
    end;


  // Block body
  GenerateGotoProlog;
  GenerateExitProlog;

  CompileCompoundStatement(0);

  CheckUnresolvedDeclarations;

  GenerateExitEpilog;                            // Direct all Exit procedure calls here
  GenerateGotoEpilog;

  if ForLoopNesting <> 0 then
    Error('Internal fault: Illegal FOR loop nesting');

  // If function, return Result value (via the EAX register, except some special cases in STDCALL/CDECL functions)
  if (BlockStack[BlockStackTop].Index <> 1) and (Ident[BlockIdentIndex].Kind = FUNC) then
    begin
    PushVarIdentPtr(Ident[BlockIdentIndex].ResultIdentIndex);
    if Types[Ident[BlockIdentIndex].Signature.ResultType].Kind in StructuredTypes then
      begin
      if Ident[Ident[BlockIdentIndex].ResultIdentIndex].PassMethod = VARPASSING then
        DerefPtr(POINTERTYPEINDEX);
      end
    else  
      DerefPtr(Ident[BlockIdentIndex].Signature.ResultType);
      
    SaveStackTopToEAX;
    
    // Return Double in EDX:EAX
    if Types[Ident[BlockIdentIndex].Signature.ResultType].Kind = REALTYPE then
      SaveStackTopToEDX;
    
    // Treat special cases in STDCALL/CDECL functions
    ConvertResultFromPascalToC(Ident[BlockIdentIndex].Signature);    
    end;

  if BlockStack[BlockStackTop].Index = 1 then          // Main program
    begin
    LibProcIdentIndex := GetIdent('EXITPROCESS');  
    PushConst(0);
    GenerateCall(Ident[LibProcIdentIndex].Address, 1, 1);
    end;

  GenerateStackFrameEpilog(Align(BlockStack[BlockStackTop].LocalDataSize + BlockStack[BlockStackTop].TempDataSize, SizeOf(LongInt)), 
                           Ident[BlockIdentIndex].Signature.CallConv <> DEFAULTCONV);

  if BlockStack[BlockStackTop].Index <> 1 then         
    begin
    if Ident[BlockIdentIndex].Signature.CallConv = CDECLCONV then
      TotalParamSize := 0                                           // CDECL implies that the stack is cleared by the caller - no need to do it here
    else  
      TotalParamSize := GetTotalParamSize(Ident[BlockIdentIndex].Signature, Ident[BlockIdentIndex].ReceiverType <> 0, FALSE);
      
    GenerateReturn(TotalParamSize, Ident[BlockIdentIndex].NestingLevel);
    end;
    
  DeleteDeclarations;
  end; // else    
  
Dec(BlockStackTop);
end;// CompileBlock




procedure CompileUsesClause;
var
  SavedParserState: TParserState;
  UnitIndex: Integer;
begin
NextTok;  

repeat 
  AssertIdent;
  
  UnitIndex := GetUnitUnsafe(Tok.Name);
  
  // If unit is not found, compile it now
  if UnitIndex = 0 then
    begin
    SavedParserState := ParserState;    
    if not SaveScanner then
      Error('Unit nesting is too deep');
 
    UnitIndex := CompileProgramOrUnit(Tok.Name + '.pas');

    ParserState := SavedParserState;    
    if not RestoreScanner then
      Error('Internal fault: Scanner state cannot be restored'); 
    end;
    
  ParserState.UnitStatus.UsedUnits := ParserState.UnitStatus.UsedUnits + [UnitIndex]; 
  SetUnitStatus(ParserState.UnitStatus);
  
  NextTok;
  
  if Tok.Kind <> COMMATOK then Break;
  NextTok;
until FALSE;

EatTok(SEMICOLONTOK);  
   
end; // CompileUsesClause  




function CompileProgramOrUnit(const Name: TString): Integer; 
begin
InitializeScanner(Name);

Inc(NumUnits);
if NumUnits > MAXUNITS then
  Error('Maximum number of units exceeded');
ParserState.UnitStatus.Index := NumUnits;

NextTok;

ParserState.IsUnit := FALSE;
if Tok.Kind in [PROGRAMTOK, UNITTOK] then
  begin
  ParserState.IsUnit := Tok.Kind = UNITTOK;
  
  NextTok; 
  AssertIdent;
  Units[ParserState.UnitStatus.Index].Name := Tok.Name;

  NextTok;
  EatTok(SEMICOLONTOK);
  end
else
  Units[ParserState.UnitStatus.Index].Name := 'MAIN';  

ParserState.IsInterfaceSection := FALSE;
if ParserState.IsUnit then
  begin
  EatTok(INTERFACETOK);
  ParserState.IsInterfaceSection := TRUE;
  end;
  
// Always use System unit, except when compiling System unit itself
if NumUnits > 1 then    
  ParserState.UnitStatus.UsedUnits := [1]
else  
  ParserState.UnitStatus.UsedUnits := []; 
  
SetUnitStatus(ParserState.UnitStatus);  

if Tok.Kind = USESTOK then
  CompileUsesClause;

Notice('Compiling ' + Name);

NumBlocks := 1;  
CompileBlock(0);

CheckTok(PERIODTOK);

Result := ParserState.UnitStatus.Index;
FinalizeScanner;
end;// CompileProgram


end.

