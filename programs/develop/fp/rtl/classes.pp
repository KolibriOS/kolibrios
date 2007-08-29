{$mode objfpc}

unit Classes;

{$i _defines.inc}

interface

uses
  RTLConsts, SysUtils, Types, TypInfo;

{$i classesh.inc}

implementation

uses
  SysConst;

{ OS - independent class implementations are in /inc directory. }
{$i classes.inc}

initialization
  CommonInit;
finalization
  CommonCleanup;
end.
