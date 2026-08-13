// XD Pascal - a 32-bit compiler for Windows
// Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov

{$I-}
{$H-}

unit Common;


interface


const
  VERSION                   = '0.12';
  
  NUMKEYWORDS               = 43;          
  MAXSTRLENGTH              = 255;
  MAXSETELEMENTS            = 256;
  MAXENUMELEMENTS           = 256;
  MAXIDENTS                 = 2000;
  MAXTYPES                  = 2000;
  MAXUNITS                  = 100;
  MAXFOLDERS                = 10;
  MAXBLOCKNESTING           = 10;
  MAXPARAMS                 = 30;
  MAXFIELDS                 = 100;
  MAXWITHNESTING            = 20;

  MAXINITIALIZEDDATASIZE    =    1 * 1024 * 1024;
  MAXUNINITIALIZEDDATASIZE  = 1024 * 1024 * 1024;
  MAXSTACKSIZE              =   16 * 1024 * 1024;  



type
  TCharacter     = Char;
  PCharacter     = PChar;
  TString        = string;  
  TShortString   = string;
  TGenericString = string;
  
  PLongInt = ^LongInt;
  
  TInFile  = file;  
  TOutFile = file;  
  
  TTokenKind =
    (
    EMPTYTOK,
    
    // Delimiters        
    OPARTOK,
    CPARTOK,
    MULTOK,
    PLUSTOK,
    COMMATOK,
    MINUSTOK,
    PERIODTOK,
    RANGETOK,
    DIVTOK,
    COLONTOK,
    ASSIGNTOK,
    SEMICOLONTOK,
    LTTOK,
    LETOK,
    NETOK,
    EQTOK,
    GTTOK,
    GETOK,
    ADDRESSTOK,
    OBRACKETTOK,
    CBRACKETTOK,
    DEREFERENCETOK,

    // Keywords
    ANDTOK,
    ARRAYTOK,
    BEGINTOK,
    CASETOK,
    CONSTTOK,
    IDIVTOK,
    DOTOK,
    DOWNTOTOK,
    ELSETOK,
    ENDTOK,
    FILETOK,
    FORTOK,
    FUNCTIONTOK,
    GOTOTOK,
    IFTOK,
    IMPLEMENTATIONTOK,
    INTOK,
    INTERFACETOK,
    LABELTOK,
    MODTOK,
    NILTOK,
    NOTTOK,
    OFTOK,
    ORTOK,
    PACKEDTOK,
    PROCEDURETOK,
    PROGRAMTOK,
    RECORDTOK,
    REPEATTOK,
    SETTOK,
    SHLTOK,
    SHRTOK,
    STRINGTOK,
    THENTOK,
    TOTOK,
    TYPETOK,
    UNITTOK,
    UNTILTOK,
    USESTOK,
    VARTOK,
    WHILETOK,
    WITHTOK,
    XORTOK,

    // User tokens
    IDENTTOK,
    INTNUMBERTOK,
    REALNUMBERTOK,
    CHARLITERALTOK,
    STRINGLITERALTOK    
    );
    
  TToken = record
    Name: TString;
  case Kind: TTokenKind of
    IDENTTOK:         (NonUppercaseName: TShortString);  
    INTNUMBERTOK:     (OrdValue: LongInt);                   // For all ordinal types
    REALNUMBERTOK:    (RealValue: Double);
    STRINGLITERALTOK: (StrAddress: Integer;
                       StrLength: Integer);
  end;
  
  TTypeKind = (EMPTYTYPE, ANYTYPE, INTEGERTYPE, SMALLINTTYPE, SHORTINTTYPE, WORDTYPE, BYTETYPE, CHARTYPE, BOOLEANTYPE, REALTYPE, SINGLETYPE,
               POINTERTYPE, FILETYPE, ARRAYTYPE, RECORDTYPE, INTERFACETYPE, SETTYPE, ENUMERATEDTYPE, SUBRANGETYPE, 
               PROCEDURALTYPE, METHODTYPE, FORWARDTYPE);  



const
  // Predefined type indices
  ANYTYPEINDEX          = 1;      // Untyped parameter, or base type for untyped pointers and files
  INTEGERTYPEINDEX      = 2;
  SMALLINTTYPEINDEX     = 3;
  SHORTINTTYPEINDEX     = 4;
  WORDTYPEINDEX         = 5;
  BYTETYPEINDEX         = 6;  
  CHARTYPEINDEX         = 7;
  BOOLEANTYPEINDEX      = 8;
  REALTYPEINDEX         = 9;      // Basic real type: 64-bit double (all temporary real results are of this type)
  SINGLETYPEINDEX       = 10;
  POINTERTYPEINDEX      = 11;     // Untyped pointer, compatible with any other pointers
  FILETYPEINDEX         = 12;     // Untyped file, compatible with text files
  STRINGTYPEINDEX       = 13;     // String of maximum allowed length



type  
  TByteSet = set of Byte;

  TConst = packed record
  case Kind: TTypeKind of
    INTEGERTYPE: (OrdValue: LongInt);         // For all ordinal types 
    REALTYPE:    (RealValue: Double);
    SINGLETYPE:  (SingleValue: Single);
    ARRAYTYPE:   (StrValue: TShortString);
    SETTYPE:     (SetValue: TByteSet);        // For all set types    
  end;   
  
  TPassMethod = (EMPTYPASSING, VALPASSING, CONSTPASSING, VARPASSING); 

  TParam = record
    Name: TString;
    DataType: Integer;
    PassMethod: TPassMethod;
    Default: TConst;
  end;

  PParam = ^TParam;
  
  PParams = array [1..MAXPARAMS] of PParam;   
    
  TIdentKind = (EMPTYIDENT, GOTOLABEL, CONSTANT, USERTYPE, VARIABLE, PROC, FUNC);
  
  TScope = (EMPTYSCOPE, GLOBAL, LOCAL);
  
  TRelocType = (EMPTYRELOC, CODERELOC, INITDATARELOC, UNINITDATARELOC, IMPORTRELOC);
  
  TCallConv = (DEFAULTCONV, STDCALLCONV, CDECLCONV);
  
  TPredefProc = 
    (
    EMPTYPROC,
    
    // Procedures     
    INCPROC, 
    DECPROC, 
    READPROC, 
    WRITEPROC, 
    READLNPROC, 
    WRITELNPROC, 
    NEWPROC, 
    DISPOSEPROC, 
    BREAKPROC, 
    CONTINUEPROC,
    EXITPROC,
    HALTPROC,

    // Functions
    SIZEOFFUNC,
    ORDFUNC,
    CHRFUNC,
    LOWFUNC,
    HIGHFUNC,
    PREDFUNC,
    SUCCFUNC,
    ROUNDFUNC,
    TRUNCFUNC,
    ABSFUNC,
    SQRFUNC,
    SINFUNC,
    COSFUNC,
    ARCTANFUNC,
    EXPFUNC,
    LNFUNC,
    SQRTFUNC
    );
    
  TSignature = record
    NumParams: Integer;
    NumDefaultParams: Integer;
    Param: PParams;
    ResultType: Integer;
    CallConv: TCallConv;
  end;      

  TIdentifier = record
    Kind: TIdentKind;
    Name: TString;
    DataType: Integer;
    Address: LongInt;
    ConstVal: TConst;
    UnitIndex: Integer;
    Block: Integer;                             // Index of a block in which the identifier is defined
    NestingLevel: Byte;
    ReceiverName: TString;                      // Receiver variable name for a method    
    ReceiverType: Integer;                      // Receiver type for a method
    Scope: TScope;
    RelocType: TRelocType;
    PassMethod: TPassMethod;                    // Value, CONST or VAR parameter status
    Signature: TSignature;
    ResultIdentIndex: Integer;
    ProcAsBlock: Integer;
    PredefProc: TPredefProc;
    IsUsed: Boolean;
    IsUnresolvedForward: Boolean;
    IsExported: Boolean;
    IsTypedConst: Boolean;
    IsInCStack: Boolean;
    ForLoopNesting: Integer;                    // Number of nested FOR loops where the label is defined
  end;

  TField = record
    Name: TString;
    DataType: Integer;
    Offset: Integer;
  end;

  PField = ^TField;    

  TType = record    
    Block: Integer;
    BaseType: Integer;
    AliasType: Integer;
    
  case Kind: TTypeKind of     
    SUBRANGETYPE:              (Low, High: Integer);
  
    ARRAYTYPE:                 (IndexType: Integer;
                                IsOpenArray: Boolean);
                      
    RECORDTYPE, INTERFACETYPE: (NumFields: Integer;
                                Field: array [1..MAXFIELDS] of PField);
                      
    PROCEDURALTYPE:            (Signature: TSignature;
                                SelfPointerOffset: LongInt);  // For interface method variables as temporary results
    
    METHODTYPE:                (MethodIdentIndex: Integer);   // For static methods as temporary results
  
    FORWARDTYPE:               (TypeIdentName: TShortString);   
  end;
  
  TBlock = record
    Index: Integer;
    LocalDataSize, ParamDataSize, TempDataSize: LongInt;
  end;
  
  TUnit = record
    Name: TString;
  end;

  TUnitStatus = record
    Index: Integer;
    UsedUnits: set of Byte;
  end;      
  
  TWithDesignator = record
    TempPointer: Integer;
    DataType: Integer;
    IsConst: Boolean;
  end;
  
  TWriteProc = procedure (ClassInstance: Pointer; const Msg: TString);
  
  
  
const    
  // Operator sets  
  MultiplicativeOperators = [MULTOK, DIVTOK, IDIVTOK, MODTOK, SHLTOK, SHRTOK, ANDTOK];
  AdditiveOperators       = [PLUSTOK, MINUSTOK, ORTOK, XORTOK];
  UnaryOperators          = [PLUSTOK, MINUSTOK];
  RelationOperators       = [EQTOK, NETOK, LTTOK, LETOK, GTTOK, GETOK];

  OperatorsForIntegers    = MultiplicativeOperators - [DIVTOK] + AdditiveOperators + RelationOperators + [NOTTOK];
  OperatorsForReals       = [MULTOK, DIVTOK, PLUSTOK, MINUSTOK] + RelationOperators;
  OperatorsForBooleans    = [ANDTOK, ORTOK, XORTOK, NOTTOK] + RelationOperators;

  // Type sets
  IntegerTypes     = [INTEGERTYPE, SMALLINTTYPE, SHORTINTTYPE, WORDTYPE, BYTETYPE];
  OrdinalTypes     = IntegerTypes + [CHARTYPE, BOOLEANTYPE, SUBRANGETYPE, ENUMERATEDTYPE];
  UnsignedTypes    = [WORDTYPE, BYTETYPE, CHARTYPE];
  NumericTypes     = IntegerTypes + [REALTYPE];
  StructuredTypes  = [ARRAYTYPE, RECORDTYPE, INTERFACETYPE, SETTYPE, FILETYPE];
  CastableTypes    = OrdinalTypes + [POINTERTYPE, PROCEDURALTYPE];     
  
  

var
  Ident: array [1..MAXIDENTS] of TIdentifier;
  Types: array [1..MAXTYPES] of TType;
  InitializedGlobalData: array [0..MAXINITIALIZEDDATASIZE - 1] of Byte;
  Units: array [1..MAXUNITS] of TUnit;  
  Folders: array [1..MAXFOLDERS] of TString;
  BlockStack: array [1..MAXBLOCKNESTING] of TBlock;
  WithStack: array [1..MAXWITHNESTING] of TWithDesignator;

  NumIdent, NumTypes, NumUnits, NumFolders, NumBlocks, BlockStackTop, ForLoopNesting, WithNesting,
  InitializedGlobalDataSize, UninitializedGlobalDataSize: Integer;
  
  IsConsoleProgram: Boolean;
  


procedure InitializeCommon;
procedure FinalizeCommon;
procedure CopyParams(var LeftSignature, RightSignature: TSignature);
procedure DisposeParams(var Signature: TSignature);
procedure DisposeFields(var DataType: TType);
function GetTokSpelling(TokKind: TTokenKind): TString;
function GetTypeSpelling(DataType: Integer): TString;
function Align(Size, Alignment: Integer): Integer;
procedure SetWriteProcs(ClassInstance: Pointer; NewNoticeProc, NewWarningProc, NewErrorProc: TWriteProc);
procedure Notice(const Msg: TString);
procedure Warning(const Msg: TString);
procedure Error(const Msg: TString);
procedure DefineStaticString(const StrValue: TString; var Addr: LongInt; FixedAddr: LongInt = -1);
procedure DefineStaticSet(const SetValue: TByteSet; var Addr: LongInt; FixedAddr: LongInt = -1);
function IsString(DataType: Integer): Boolean;
function LowBound(DataType: Integer): Integer;
function HighBound(DataType: Integer): Integer;
function TypeSize(DataType: Integer): Integer;
function GetTotalParamSize(const Signature: TSignature; IsMethod, AlwaysTreatStructuresAsReferences: Boolean): Integer;
function GetCompatibleType(LeftType, RightType: Integer): Integer;
function GetCompatibleRefType(LeftType, RightType: Integer): Integer;
procedure CheckOperator(const Tok: TToken; DataType: Integer);
procedure CheckSignatures(var Signature1, Signature2: TSignature; const Name: TString; CheckParamNames: Boolean = TRUE); 
procedure SetUnitStatus(var NewUnitStatus: TUnitStatus);
function GetUnitUnsafe(const UnitName: TString): Integer;
function GetUnit(const UnitName: TString): Integer;
function GetKeyword(const KeywordName: TString): TTokenKind;
function GetIdentUnsafe(const IdentName: TString; AllowForwardReference: Boolean = FALSE; RecType: Integer = 0): Integer;
function GetIdent(const IdentName: TString; AllowForwardReference: Boolean = FALSE; RecType: Integer = 0): Integer;
function GetFieldUnsafe(RecType: Integer; const FieldName: TString): Integer;
function GetField(RecType: Integer; const FieldName: TString): Integer;
function GetFieldInsideWith(var RecPointer: Integer; var RecType: Integer; var IsConst: Boolean; const FieldName: TString): Integer;
function GetMethodUnsafe(RecType: Integer; const MethodName: TString): Integer;
function GetMethod(RecType: Integer; const MethodName: TString): Integer;
function GetMethodInsideWith(var RecPointer: Integer; var RecType: Integer; var IsConst: Boolean; const MethodName: TString): Integer;
function FieldOrMethodInsideWithFound(const Name: TString): Boolean;



implementation


const
  Keyword: array [1..NUMKEYWORDS] of TString = 
    (
    'AND',
    'ARRAY',
    'BEGIN',
    'CASE',
    'CONST',
    'DIV',
    'DO',
    'DOWNTO',
    'ELSE',
    'END',
    'FILE',
    'FOR',
    'FUNCTION',
    'GOTO',
    'IF',
    'IMPLEMENTATION',
    'IN',
    'INTERFACE',
    'LABEL',
    'MOD',
    'NIL',
    'NOT',
    'OF',
    'OR',
    'PACKED',
    'PROCEDURE',
    'PROGRAM',
    'RECORD',
    'REPEAT',
    'SET',
    'SHL',
    'SHR',
    'STRING',
    'THEN',
    'TO',
    'TYPE',
    'UNIT',
    'UNTIL',
    'USES',
    'VAR',
    'WHILE',
    'WITH',
    'XOR'
    );
 


var
  NoticeProc, WarningProc, ErrorProc: TWriteProc;
  WriteProcsClassInstance: Pointer;
  UnitStatus: TUnitStatus;
  


procedure InitializeCommon;
begin
FillChar(Ident, SizeOf(Ident), #0);
FillChar(Types, SizeOf(Types), #0);
FillChar(Units, SizeOf(Units), #0);
FillChar(InitializedGlobalData, SizeOf(InitializedGlobalData), #0);

NumIdent                    := 0; 
NumTypes                    := 0;
NumUnits                    := 0;
NumFolders                  := 0; 
NumBlocks                   := 0; 
BlockStackTop               := 0; 
ForLoopNesting              := 0;
WithNesting                 := 0;
InitializedGlobalDataSize   := 0;
UninitializedGlobalDataSize := 0;

IsConsoleProgram            := TRUE;  // Console program by default
end;




procedure FinalizeCommon;
var
  i: Integer;
  
begin
// Dispose of dynamically allocated parameter data
for i := 1 to NumIdent do
  if (Ident[i].Kind = PROC) or (Ident[i].Kind = FUNC) then
    DisposeParams(Ident[i].Signature);

// Dispose of dynamically allocated parameter and field data
for i := 1 to NumTypes do
  begin
  if Types[i].Kind = PROCEDURALTYPE then
    DisposeParams(Types[i].Signature) 
  else if Types[i].Kind in [RECORDTYPE, INTERFACETYPE] then
    DisposeFields(Types[i]);
  end;
end;




procedure CopyParams(var LeftSignature, RightSignature: TSignature);
var
  i: Integer;
begin
for i := 1 to RightSignature.NumParams do
  begin
  New(LeftSignature.Param[i]);
  LeftSignature.Param[i]^ := RightSignature.Param[i]^;
  end;
end;




procedure DisposeParams(var Signature: TSignature);
var
  i: Integer;
begin
for i := 1 to Signature.NumParams do
  Dispose(Signature.Param[i]);
end; 




procedure DisposeFields(var DataType: TType);
var
  i: Integer;
begin
for i := 1 to DataType.NumFields do
  Dispose(DataType.Field[i]);
end; 




function GetTokSpelling(TokKind: TTokenKind): TString;
begin
case TokKind of
  EMPTYTOK:                          Result := 'no token';
  OPARTOK:                           Result := '(';
  CPARTOK:                           Result := ')';
  MULTOK:                            Result := '*';
  PLUSTOK:                           Result := '+';
  COMMATOK:                          Result := ',';
  MINUSTOK:                          Result := '-';
  PERIODTOK:                         Result := '.';
  RANGETOK:                          Result := '..';
  DIVTOK:                            Result := '/';
  COLONTOK:                          Result := ':';
  ASSIGNTOK:                         Result := ':=';
  SEMICOLONTOK:                      Result := ';';
  LTTOK:                             Result := '<';
  LETOK:                             Result := '<=';
  NETOK:                             Result := '<>';
  EQTOK:                             Result := '=';
  GTTOK:                             Result := '>';
  GETOK:                             Result := '>=';
  ADDRESSTOK:                        Result := '@';
  OBRACKETTOK:                       Result := '[';
  CBRACKETTOK:                       Result := ']';
  DEREFERENCETOK:                    Result := '^';
  ANDTOK..XORTOK:                    Result := Keyword[Ord(TokKind) - Ord(ANDTOK) + 1];
  IDENTTOK:                          Result := 'identifier';
  INTNUMBERTOK, REALNUMBERTOK:       Result := 'number';
  CHARLITERALTOK:                    Result := 'character literal';
  STRINGLITERALTOK:                  Result := 'string literal'
else
  Result := 'unknown token';
end; //case
end;




function GetTypeSpelling(DataType: Integer): TString;
begin
case Types[DataType].Kind of
  EMPTYTYPE:      Result := 'no type';
  ANYTYPE:        Result := 'any type';
  INTEGERTYPE:    Result := 'integer';
  SMALLINTTYPE:   Result := 'small integer';
  SHORTINTTYPE:   Result := 'short integer';
  WORDTYPE:       Result := 'word';
  BYTETYPE:       Result := 'byte';
  CHARTYPE:       Result := 'character';
  BOOLEANTYPE:    Result := 'Boolean';
  REALTYPE:       Result := 'real';
  SINGLETYPE:     Result := 'single-precision real';
  POINTERTYPE:    begin
                  Result := 'pointer';
                  if Types[Types[DataType].BaseType].Kind <> ANYTYPE then
                    Result := Result + ' to ' + GetTypeSpelling(Types[DataType].BaseType);
                  end;  
  FILETYPE:       begin
                  Result := 'file';
                  if Types[Types[DataType].BaseType].Kind <> ANYTYPE then
                    Result := Result + ' of ' + GetTypeSpelling(Types[DataType].BaseType);
                  end;  
  ARRAYTYPE:      Result := 'array of ' + GetTypeSpelling(Types[DataType].BaseType); 
  RECORDTYPE:     Result := 'record';
  INTERFACETYPE:  Result := 'interface';
  SETTYPE:        Result := 'set of ' + GetTypeSpelling(Types[DataType].BaseType);
  ENUMERATEDTYPE: Result := 'enumeration';
  SUBRANGETYPE:   Result := 'subrange of ' + GetTypeSpelling(Types[DataType].BaseType); 
  PROCEDURALTYPE: Result := 'procedural type';
else
  Result := 'unknown type';
end; //case
end;




function Align(Size, Alignment: Integer): Integer;
begin
Result := ((Size + (Alignment - 1)) div Alignment) * Alignment;
end;




procedure SetWriteProcs(ClassInstance: Pointer; NewNoticeProc, NewWarningProc, NewErrorProc: TWriteProc);
begin
WriteProcsClassInstance := ClassInstance;

NoticeProc  := NewNoticeProc;
WarningProc := NewWarningProc;
ErrorProc   := NewErrorProc;
end;




procedure Notice(const Msg: TString);
begin
NoticeProc(WriteProcsClassInstance, Msg);
end;




procedure Warning(const Msg: TString);
begin
WarningProc(WriteProcsClassInstance, Msg);
end;



  
procedure Error(const Msg: TString);
begin
ErrorProc(WriteProcsClassInstance, Msg);
end;




procedure DefineStaticString(const StrValue: TString; var Addr: LongInt; FixedAddr: LongInt = -1);
var
  Len: Integer;  
begin
Len := Length(StrValue);

if FixedAddr <> -1 then
  Addr := FixedAddr
else  
  begin
  if Len + 1 > MAXINITIALIZEDDATASIZE - InitializedGlobalDataSize then
    Error('Not enough memory for static string');

  Addr := InitializedGlobalDataSize;  // Relocatable
  InitializedGlobalDataSize := InitializedGlobalDataSize + Len + 1;
  end;
  
Move(StrValue[1], InitializedGlobalData[Addr], Len);
InitializedGlobalData[Addr + Len] := 0;      // Add string termination character
end;




procedure DefineStaticSet(const SetValue: TByteSet; var Addr: LongInt; FixedAddr: LongInt = -1);
var
  i: Integer;
  ElementPtr: ^Byte;  
begin
if FixedAddr <> -1 then
  Addr := FixedAddr
else  
  begin
  if MAXSETELEMENTS div 8 > MAXINITIALIZEDDATASIZE - InitializedGlobalDataSize then
    Error('Not enough memory for static set');
  
  Addr := InitializedGlobalDataSize;
  InitializedGlobalDataSize := InitializedGlobalDataSize + MAXSETELEMENTS div 8;
  end;   

for i := 0 to MAXSETELEMENTS - 1 do
  if i in SetValue then
    begin
    ElementPtr := @InitializedGlobalData[Addr + i shr 3];
    ElementPtr^ := ElementPtr^ or (1 shl (i and 7));
    end;
end;




function IsString(DataType: Integer): Boolean;
begin
Result := (Types[DataType].Kind = ARRAYTYPE) and (Types[Types[DataType].BaseType].Kind = CHARTYPE);
end;




function LowBound(DataType: Integer): Integer;
begin
Result := 0;
case Types[DataType].Kind of
  INTEGERTYPE:    Result := -2147483647 - 1;
  SMALLINTTYPE:   Result := -32768;
  SHORTINTTYPE:   Result := -128;
  WORDTYPE:       Result :=  0;
  BYTETYPE:       Result :=  0;
  CHARTYPE:       Result :=  0;
  BOOLEANTYPE:    Result :=  0;
  SUBRANGETYPE:   Result :=  Types[DataType].Low;
  ENUMERATEDTYPE: Result :=  Types[DataType].Low
else
  Error('Ordinal type expected')
end;// case
end;
                        



function HighBound(DataType: Integer): Integer;
begin
Result := 0;
case Types[DataType].Kind of
  INTEGERTYPE:    Result := 2147483647;
  SMALLINTTYPE:   Result := 32767;
  SHORTINTTYPE:   Result := 127;
  WORDTYPE:       Result := 65535;
  BYTETYPE:       Result := 255;  
  CHARTYPE:       Result := 255;
  BOOLEANTYPE:    Result := 1;
  SUBRANGETYPE:   Result := Types[DataType].High;
  ENUMERATEDTYPE: Result := Types[DataType].High
else
  Error('Ordinal type expected')
end;// case
end;




function TypeSize(DataType: Integer): Integer;
var
  CurSize, BaseTypeSize, FieldTypeSize: Integer;
  NumElements, FieldOffset, i: Integer;
begin
Result := 0;
case Types[DataType].Kind of
  INTEGERTYPE:               Result := SizeOf(Integer);
  SMALLINTTYPE:              Result := SizeOf(SmallInt);
  SHORTINTTYPE:              Result := SizeOf(ShortInt);
  WORDTYPE:                  Result := SizeOf(Word);
  BYTETYPE:                  Result := SizeOf(Byte);  
  CHARTYPE:                  Result := SizeOf(TCharacter);
  BOOLEANTYPE:               Result := SizeOf(Boolean);
  REALTYPE:                  Result := SizeOf(Double);
  SINGLETYPE:                Result := SizeOf(Single);
  POINTERTYPE:               Result := SizeOf(Pointer);
  FILETYPE:                  Result := SizeOf(TString) + SizeOf(Integer);  // Name + Handle
  SUBRANGETYPE:              Result := TypeSize(Types[DataType].BaseType);
  
  ARRAYTYPE:                 begin
                             if Types[DataType].IsOpenArray then
                               Error('Illegal type');
                             
                             NumElements := HighBound(Types[DataType].IndexType) - LowBound(Types[DataType].IndexType) + 1;
                             BaseTypeSize := TypeSize(Types[DataType].BaseType);
                             
                             if (NumElements > 0) and (BaseTypeSize > HighBound(INTEGERTYPEINDEX) div NumElements) then
                               Error('Type size is too large');
                               
                             Result := NumElements * BaseTypeSize;
                             end;
                             
  RECORDTYPE, INTERFACETYPE: for i := 1 to Types[DataType].NumFields do
                               begin
                               FieldOffset := Types[DataType].Field[i]^.Offset;
                               FieldTypeSize := TypeSize(Types[DataType].Field[i]^.DataType);
                               
                               if FieldTypeSize > HighBound(INTEGERTYPEINDEX) - FieldOffset then
                                 Error('Type size is too large');
                               
                               CurSize := FieldOffset + FieldTypeSize;
                               if CurSize > Result then Result := CurSize;
                               end;
  
  SETTYPE:                   Result := MAXSETELEMENTS div 8;
  ENUMERATEDTYPE:            Result := SizeOf(Byte);                
  PROCEDURALTYPE:            Result := SizeOf(Pointer)               
else
  Error('Illegal type')
end;// case
end; 




function GetTotalParamSize(const Signature: TSignature; IsMethod, AlwaysTreatStructuresAsReferences: Boolean): Integer;
var
  i: Integer;
begin
if (Signature.CallConv <> DEFAULTCONV) and IsMethod then
  Error('Internal fault: Methods cannot be STDCALL/CDECL');
  
Result := 0;
  
// For a method, Self is a first (hidden) VAR parameter
if IsMethod then
  Result := Result + SizeOf(LongInt);

// Allocate space for structured Result as a hidden VAR parameter (except STDCALL/CDECL functions returning small structures in EDX:EAX)
with Signature do
  if (ResultType <> 0) and (Types[ResultType].Kind in StructuredTypes) and ((CallConv = DEFAULTCONV) or (TypeSize(ResultType) > 2 * SizeOf(LongInt))) then
    Result := Result + SizeOf(LongInt);
  
// Any parameter occupies 4 bytes (except structures in the C stack)
if (Signature.CallConv <> DEFAULTCONV) and not AlwaysTreatStructuresAsReferences then
  for i := 1 to Signature.NumParams do
    if Signature.Param[i]^.PassMethod = VALPASSING then
      Result := Result + Align(TypeSize(Signature.Param[i]^.DataType), SizeOf(LongInt))
    else  
      Result := Result + SizeOf(LongInt)
else
  for i := 1 to Signature.NumParams do
    if (Signature.Param[i]^.PassMethod = VALPASSING) and (Types[Signature.Param[i]^.DataType].Kind = REALTYPE) then
      Result := Result + SizeOf(Double)
    else  
      Result := Result + SizeOf(LongInt);
       
end; // GetTotalParamSize   




function GetCompatibleType(LeftType, RightType: Integer): Integer;
begin
Result := 0;

// General rule
if LeftType = RightType then                 
  Result := LeftType

// Special cases
// All types are compatible with their aliases
else if Types[LeftType].AliasType <> 0 then
  Result := GetCompatibleType(Types[LeftType].AliasType, RightType)
else if Types[RightType].AliasType <> 0 then
  Result := GetCompatibleType(LeftType, Types[RightType].AliasType)

// Sets are compatible with other sets having a compatible base type, or with an empty set constructor
else if (Types[LeftType].Kind = SETTYPE) and (Types[RightType].Kind = SETTYPE) then
  begin
  if Types[RightType].BaseType = ANYTYPEINDEX then
    Result := LeftType
  else if Types[LeftType].BaseType = ANYTYPEINDEX then
    Result := RightType
  else
    begin  
    GetCompatibleType(Types[LeftType].BaseType, Types[RightType].BaseType);
    Result := LeftType;
    end;
  end
  
// Strings are compatible with any other strings
else if IsString(LeftType) and IsString(RightType) then
  Result := LeftType

// Untyped pointers are compatible with any pointers or procedural types
else if (Types[LeftType].Kind = POINTERTYPE) and (Types[LeftType].BaseType = ANYTYPEINDEX) and
        (Types[RightType].Kind in [POINTERTYPE, PROCEDURALTYPE]) then
  Result := LeftType
else if (Types[RightType].Kind = POINTERTYPE) and (Types[RightType].BaseType = ANYTYPEINDEX) and
        (Types[LeftType].Kind in [POINTERTYPE, PROCEDURALTYPE]) then
  Result := RightType   
  
// Typed pointers are compatible with any pointers to a reference-compatible type
else if (Types[LeftType].Kind = POINTERTYPE) and (Types[RightType].Kind = POINTERTYPE) then
  Result := GetCompatibleRefType(Types[LeftType].BaseType, Types[RightType].BaseType)
    
// Procedural types are compatible if their Self pointer offsets are equal and their signatures are compatible
else if (Types[LeftType].Kind = PROCEDURALTYPE) and (Types[RightType].Kind = PROCEDURALTYPE) and
        (Types[LeftType].SelfPointerOffset = Types[RightType].SelfPointerOffset) then
  begin
  CheckSignatures(Types[LeftType].Signature, Types[RightType].Signature, 'procedural variable', FALSE);
  Result := LeftType; 
  end
  
// Subranges are compatible with their host types
else if Types[LeftType].Kind = SUBRANGETYPE then
  Result := GetCompatibleType(Types[LeftType].BaseType, RightType)
else if Types[RightType].Kind = SUBRANGETYPE then
  Result := GetCompatibleType(LeftType, Types[RightType].BaseType)

// Integers
else if (Types[LeftType].Kind in IntegerTypes) and (Types[RightType].Kind in IntegerTypes) then
  Result := LeftType

// Booleans
else if (Types[LeftType].Kind = BOOLEANTYPE) and (Types[RightType].Kind = BOOLEANTYPE) then
  Result := LeftType

// Characters
else if (Types[LeftType].Kind = CHARTYPE) and  (Types[RightType].Kind = CHARTYPE) then
  Result := LeftType;

if Result = 0 then
  Error('Incompatible types: ' + GetTypeSpelling(LeftType) + ' and ' + GetTypeSpelling(RightType));  
end;




function GetCompatibleRefType(LeftType, RightType: Integer): Integer;
begin
// This function is asymmetric and implies Variable(LeftType) := Variable(RightType)
Result := 0;

// General rule
if LeftType = RightType then                 
  Result := RightType
  
// Special cases
// All types are compatible with their aliases  
else if Types[LeftType].AliasType <> 0 then
  Result := GetCompatibleRefType(Types[LeftType].AliasType, RightType)
else if Types[RightType].AliasType <> 0 then
  Result := GetCompatibleRefType(LeftType, Types[RightType].AliasType)

// Open arrays are compatible with any other arrays of the same base type
else if (Types[LeftType].Kind = ARRAYTYPE) and (Types[RightType].Kind = ARRAYTYPE) and 
         Types[LeftType].IsOpenArray and (Types[LeftType].BaseType = Types[RightType].BaseType) then       
  Result := RightType

// Untyped pointers are compatible with any other pointers 
else if (Types[LeftType].Kind = POINTERTYPE) and (Types[RightType].Kind = POINTERTYPE) and
       ((Types[LeftType].BaseType = Types[RightType].BaseType) or (Types[LeftType].BaseType = ANYTYPEINDEX)) then  
  Result := RightType
  
// Untyped files are compatible with any other files 
else if (Types[LeftType].Kind = FILETYPE) and (Types[RightType].Kind = FILETYPE) and
        (Types[LeftType].BaseType = ANYTYPEINDEX) then  
  Result := RightType   
  
// Untyped parameters are compatible with any type
else if Types[LeftType].Kind = ANYTYPE then
  Result := RightType;

if Result = 0 then
  Error('Incompatible types: ' + GetTypeSpelling(LeftType) + ' and ' + GetTypeSpelling(RightType));  
end;




procedure CheckOperator(const Tok: TToken; DataType: Integer); 
begin
with Types[DataType] do
  if Kind = SUBRANGETYPE then
    CheckOperator(Tok, BaseType)
  else 
    begin
    if not (Kind in OrdinalTypes) and (Kind <> REALTYPE) and (Kind <> POINTERTYPE) and (Kind <> PROCEDURALTYPE) then
      Error('Operator ' + GetTokSpelling(Tok.Kind) + ' is not applicable to ' + GetTypeSpelling(DataType));
     
    if ((Kind in IntegerTypes)  and not (Tok.Kind in OperatorsForIntegers)) or
       ((Kind = REALTYPE)       and not (Tok.Kind in OperatorsForReals)) or   
       ((Kind = CHARTYPE)       and not (Tok.Kind in RelationOperators)) or
       ((Kind = BOOLEANTYPE)    and not (Tok.Kind in OperatorsForBooleans)) or
       ((Kind = POINTERTYPE)    and not (Tok.Kind in RelationOperators)) or
       ((Kind = ENUMERATEDTYPE) and not (Tok.Kind in RelationOperators)) or
       ((Kind = PROCEDURALTYPE) and not (Tok.Kind in RelationOperators)) 
    then
      Error('Operator ' + GetTokSpelling(Tok.Kind) + ' is not applicable to ' + GetTypeSpelling(DataType));
    end;  
end;




procedure CheckSignatures(var Signature1, Signature2: TSignature; const Name: TString; CheckParamNames: Boolean = TRUE);
var
  i: Integer;
begin
if Signature1.NumParams <> Signature2.NumParams then
  Error('Incompatible number of parameters in ' + Name);
  
if Signature1.NumDefaultParams <> Signature2.NumDefaultParams then
  Error('Incompatible number of default parameters in ' + Name);
  
for i := 1 to Signature1.NumParams do
  begin
  if (Signature1.Param[i]^.Name <> Signature2.Param[i]^.Name) and CheckParamNames then
    Error('Incompatible parameter names in ' + Name);
  
  if Signature1.Param[i]^.DataType <> Signature2.Param[i]^.DataType then
    if not Types[Signature1.Param[i]^.DataType].IsOpenArray or not Types[Signature2.Param[i]^.DataType].IsOpenArray or
       (Types[Signature1.Param[i]^.DataType].BaseType <> Types[Signature2.Param[i]^.DataType].BaseType) 
    then 
      Error('Incompatible parameter types in ' + Name + ': ' + GetTypeSpelling(Signature1.Param[i]^.DataType) + ' and ' + GetTypeSpelling(Signature2.Param[i]^.DataType));
    
  if Signature1.Param[i]^.PassMethod <> Signature2.Param[i]^.PassMethod then
    Error('Incompatible CONST/VAR modifiers in ' + Name);

  if Signature1.Param[i]^.Default.OrdValue <> Signature2.Param[i]^.Default.OrdValue then
    Error('Incompatible default values in ' + Name);   
  end; // if

if Signature1.ResultType <> Signature2.ResultType then
  Error('Incompatible result types in ' + Name + ': ' + GetTypeSpelling(Signature1.ResultType) + ' and ' + GetTypeSpelling(Signature2.ResultType));
  
if Signature1.CallConv <> Signature2.CallConv then
  Error('Incompatible calling convention in ' + Name);

end;




procedure SetUnitStatus(var NewUnitStatus: TUnitStatus);
begin 
UnitStatus := NewUnitStatus;
end;




function GetUnitUnsafe(const UnitName: TString): Integer;
var
  UnitIndex: Integer;
begin
for UnitIndex := 1 to NumUnits do
  if Units[UnitIndex].Name = UnitName then 
    begin
    Result := UnitIndex;
    Exit;
    end;
      
Result := 0;
end;




function GetUnit(const UnitName: TString): Integer;
begin
Result := GetUnitUnsafe(UnitName);
if Result = 0 then
  Error('Unknown unit ' + UnitName);
end;




function GetKeyword(const KeywordName: TString): TTokenKind;
var
  Max, Mid, Min: Integer;
  Found: Boolean;
begin
Result := EMPTYTOK;

// Binary search
Min := 1;
Max := NUMKEYWORDS;

repeat
  Mid := (Min + Max) div 2;
  if KeywordName > Keyword[Mid] then
    Min := Mid + 1
  else
    Max := Mid - 1;
  Found := KeywordName = Keyword[Mid];
until Found or (Min > Max);

if Found then Result := TTokenKind(Ord(ANDTOK) - 1 + Mid);
end;




function GetIdentUnsafe(const IdentName: TString; AllowForwardReference: Boolean = FALSE; RecType: Integer = 0): Integer;
var
  IdentIndex: Integer;
begin
for IdentIndex := NumIdent downto 1 do
  with Ident[IdentIndex] do
    if ((UnitIndex = UnitStatus.Index) or (IsExported and (UnitIndex in UnitStatus.UsedUnits))) and       
       (AllowForwardReference or (Kind <> USERTYPE) or (Types[DataType].Kind <> FORWARDTYPE)) and
       (ReceiverType = RecType) and  // Receiver type for methods, 0 otherwise
       (Name = IdentName)
    then 
      begin
      Result := IdentIndex;
      Exit;
      end;          
     
Result := 0;
end;




function GetIdent(const IdentName: TString; AllowForwardReference: Boolean = FALSE; RecType: Integer = 0): Integer;
begin
Result := GetIdentUnsafe(IdentName, AllowForwardReference, RecType);
if Result = 0 then
  Error('Unknown identifier ' + IdentName);
end;




function GetFieldUnsafe(RecType: Integer; const FieldName: TString): Integer;
var
  FieldIndex: Integer;
begin
for FieldIndex := 1 to Types[RecType].NumFields do
  if Types[RecType].Field[FieldIndex]^.Name = FieldName then
    begin
    Result := FieldIndex;
    Exit;
    end;

Result := 0;
end;




function GetField(RecType: Integer; const FieldName: TString): Integer;
begin
Result := GetFieldUnsafe(RecType, FieldName);
if Result = 0 then
  Error('Unknown field ' + FieldName);
end;




function GetFieldInsideWith(var RecPointer: Integer; var RecType: Integer; var IsConst: Boolean; const FieldName: TString): Integer;
var
  FieldIndex, WithIndex: Integer;
begin 
for WithIndex := WithNesting downto 1 do
  begin
  RecType := WithStack[WithIndex].DataType;
  FieldIndex := GetFieldUnsafe(RecType, FieldName);
  
  if FieldIndex <> 0 then
    begin
    RecPointer := WithStack[WithIndex].TempPointer;  
    IsConst := WithStack[WithIndex].IsConst;
    Result := FieldIndex;
    Exit;
    end;
  end;

Result := 0;  
end;




function GetMethodUnsafe(RecType: Integer; const MethodName: TString): Integer;
begin
Result := GetIdentUnsafe(MethodName, FALSE, RecType);
end;




function GetMethod(RecType: Integer; const MethodName: TString): Integer;
begin
Result := GetIdent(MethodName, FALSE, RecType);
if (Ident[Result].Kind <> PROC) and (Ident[Result].Kind <> FUNC) then
  Error('Method expected');
end;




function GetMethodInsideWith(var RecPointer: Integer; var RecType: Integer; var IsConst: Boolean; const MethodName: TString): Integer;
var
  MethodIndex, WithIndex: Integer;
begin 
for WithIndex := WithNesting downto 1 do
  begin
  RecType := WithStack[WithIndex].DataType;
  MethodIndex := GetMethodUnsafe(RecType, MethodName);
  
  if MethodIndex <> 0 then
    begin
    RecPointer := WithStack[WithIndex].TempPointer;
    IsConst := WithStack[WithIndex].IsConst;
    Result := MethodIndex;
    Exit;
    end;
  end;

Result := 0;  
end;




function FieldOrMethodInsideWithFound(const Name: TString): Boolean;
var
  RecPointer: Integer; 
  RecType: Integer;
  IsConst: Boolean;        
begin
Result := (GetFieldInsideWith(RecPointer, RecType, IsConst, Name) <> 0) or (GetMethodInsideWith(RecPointer, RecType, IsConst, Name) <> 0);
end;



end.
