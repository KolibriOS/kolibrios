
CntItems = 0

ScanItems = 1 ;must be >0, temporary equal 0

EleSize = 6 ;xxh.Size

virtual at 0
Vals::
rb 64*1024
end virtual

macro AddElement Source
{
local A,B,C,Index,Found
C = 0
Index = 0
Found = 0
 if ScanItems
 repeat CntItems
   Found = 0
   load B fword from Vals:+(%-1)*EleSize+C
   load A fword from Source+C
   if A=B
     Found = %
     Break
   else
     Found = 0
   end if
 end repeat
 end if
 if ~Found
   load A fword from Source+C
   store fword A at Vals:+CntItems*EleSize+C
   dw (CntItems)*EleSize
   CntItems = CntItems+1
 else
   dw (Found-1)*EleSize
 end if
}

macro CpyElements Dst
{
local A
Dst rb CntItems*EleSize
  repeat CntItems
    load A fword from Vals:(%-1)*EleSize
    store fword A at Dst+(%-1)*EleSize
  end repeat
}
