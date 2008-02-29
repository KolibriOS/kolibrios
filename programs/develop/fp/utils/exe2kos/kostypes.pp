unit KOSTypes;

interface

type
  TKosSign = array[0..7] of Byte;
  TKosHeader = packed record
    sign   : TKOSSign;
    version: DWord;
    start  : DWord;
    size   : DWord;
    memory : DWord;
    stack  : DWord;
    args   : DWord;
    path   : DWord;
  end;

const
  KOS_SIGN: TKOSSign = ($4D, $45, $4E, $55, $45, $54, $30, $31);

implementation

end.
