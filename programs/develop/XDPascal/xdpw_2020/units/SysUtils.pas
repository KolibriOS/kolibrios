// XD Pascal - a 32-bit compiler for Windows
// Copyright (c) 2009-2010, 2019-2020, Vasiliy Tereshkov

unit SysUtils;


interface


type
  TFloatFormat = (ffGeneral, ffFixed);

  AnsiChar = Char;
  PAnsiChar = PChar;
  
  WideChar = Word;
  PWideChar = ^WideChar;
  WideString = array [1..MaxStrLength + 1] of WideChar;


function IntToStr(n: Integer): string;
function StrToInt(const s: string): Integer;
function FloatToStr(x: Real): string;
function FloatToStrF(x: Real; Format: TFloatFormat; Precision, Digits: Integer): string;
function StrToFloat(const s: string): Real; 
function StrToPWideChar(const s: string): PWideChar;
function PWideCharToStr(p: PWideChar): string; 



implementation


var
  WideStringBuf: WideString;
  


function IntToStr(n: Integer): string;
begin
IStr(n, Result);
end;




function StrToInt(const s: string): Integer;
var
  Code: Integer;
begin
IVal(s, Result, Code);
if Code <> 0 then Halt(1);
end;




function FloatToStr(x: Real): string;
begin
if abs(ln(abs(x)) / ln(10)) > 9 then
  Str(x, Result)
else
  Str(x, Result, 0, 16);  
end;




function FloatToStrF(x: Real; Format: TFloatFormat; Precision, Digits: Integer): string;
begin
case Format of
  ffGeneral: 
    Result := FloatToStr(x);
    
  ffFixed:       
    if Digits > Precision then
      Str(x, Result)
    else  
      Str(x, Result, 0, Digits);
end;               
end;




function StrToFloat(const s: string): Real;
var
  Code: Integer;
begin
Val(s, Result, Code);
if Code <> 0 then Halt(1);
end;


  
  
function StrToPWideChar(const s: string): PWideChar;
var
  i: Integer;  
begin
i := 0;
repeat
  Inc(i);
  WideStringBuf[i] := Ord(s[i]);
until s[i] = #0;
Result := @WideStringBuf[1];  
end;




function PWideCharToStr(p: PWideChar): string;
var
  i: Integer;
begin
i := 0;
repeat
  Inc(i);
  Result[i] := Char(p^);
  p := PWideChar(Integer(p) + SizeOf(WideChar));
until Result[i] = #0;
end;


end.
  