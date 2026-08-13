// Sorting demo

{$APPTYPE CONSOLE}

program Sort;



type
  TValue = Integer;
  TCompareFunc = function(x, y: TValue): Boolean;
    


procedure Swap(var x, y: TValue);
var
  buf: TValue;
begin
buf := x;
x := y;
y := buf;
end;



procedure QuickSort(var data: array of TValue; len: Integer; Ordered: TCompareFunc);


  function Partition(var data: array of TValue; low, high: Integer; Ordered: TCompareFunc): Integer;
  var
    pivot: TValue;
    pivotIndex, i: Integer;
  begin
  pivot := data[high];
  pivotIndex := low;

  for i := low to high - 1 do
    if not Ordered(data[i], pivot) then
      begin
      Swap(data[pivotIndex], data[i]);
      Inc(pivotIndex);
      end; // if

  Swap(data[high], data[pivotIndex]);

  Result := pivotIndex;
  end; 
  
  
  procedure Sort(var data: array of TValue; low, high: Integer; Ordered: TCompareFunc);
  var
    pivotIndex: Integer;
  begin
  if high > low then
    begin
    pivotIndex := Partition(data, low, high, Ordered);
    
    Sort(data, low, pivotIndex - 1, Ordered);
    Sort(data, pivotIndex + 1, high, Ordered);
    end; // if
  end;  
    
  
begin
Sort(data, 0, len - 1, Ordered);
end;



procedure BubbleSort(var data: array of TValue; len: Integer; Ordered: TCompareFunc);
var
  changed: Boolean;
  i: Integer;
begin
repeat
  changed := FALSE;

  for i := 0 to len - 2 do
    if not Ordered(data[i + 1], data[i]) then
      begin
      Swap(data[i + 1], data[i]);
      changed := TRUE;
      end;

until not changed; 
end;



procedure SelectionSort(var data: array of TValue; len: Integer; Ordered: TCompareFunc);
var
  i, j, extrIndex: Integer;
  extr: TValue;
begin
for i := 0 to len - 1 do
  begin
  extr := data[i];
  extrIndex := i;

  for j := i + 1 to len - 1 do
    if not Ordered(data[j], extr) then
      begin
      extr := data[j];
      extrIndex := j;
      end;

  Swap(data[i], data[extrIndex]);
  end; // for
end;



function Sorted(var data: array of TValue; len: Integer; Ordered: TCompareFunc): Boolean;
var
  i: Integer;
begin
Result := TRUE;
for i := 0 to len - 2 do
  if not Ordered(data[i + 1], data[i]) then
    begin
    Result := FALSE;
    Break;
    end;
end;



function OrderedAscending(y, x: TValue): Boolean;
begin
Result := y >= x;
end;



function OrderedDescending(y, x: TValue): Boolean;
begin
Result := y <= x;
end;   



const
  DataLength = 600;



var
  RandomData: array [1..DataLength] of TValue;
  i: Integer;
  Ordered: TCompareFunc;
  Order, Method: Char;



begin
WriteLn;
WriteLn('Sorting demo');
WriteLn;
WriteLn('Initial array: ');
WriteLn;

Randomize;

for i := 1 to DataLength do
  begin
  RandomData[i] := Round((Random - 0.5) * 1000000);
  Write(RandomData[i]: 8);
  if i mod 6 = 0 then WriteLn;
  end;

WriteLn;
WriteLn;
Write('Select order (A - ascending, D - descending): '); ReadLn(Order);
WriteLn;

case Order of
  'A', 'a': 
    begin
    WriteLn('Ascending order');
    Ordered := @OrderedAscending;
    end;
  'D', 'd':
    begin
    WriteLn('Descending order');
    Ordered := @OrderedDescending;
    end
else
  WriteLn('Order is not selected.');
  Ordered := nil;
  ReadLn;
  Halt;     
end;

WriteLn;
Write('Select method (Q - quick, B - bubble, S - selection): '); ReadLn(Method);
WriteLn;

case Method of
  'Q', 'q': 
    begin
    WriteLn('Quick sorting');
    QuickSort(RandomData, DataLength, Ordered);
    end;
  'B', 'b':
    begin
    WriteLn('Bubble sorting');
    BubbleSort(RandomData, DataLength, Ordered);
    end;
  'S', 's': 
    begin
    WriteLn('Selection sorting');
    SelectionSort(RandomData, DataLength, Ordered);
    end
else
  WriteLn('Sorting method is not selected.');
  ReadLn;
  Halt;     
end;

WriteLn;
WriteLn('Sorted array: ');
WriteLn;

for i := 1 to DataLength do
  begin
  Write(RandomData[i]: 8); 
  if i mod 6 = 0 then WriteLn;
  end;
WriteLn;

WriteLn('Sorted: ', Sorted(RandomData, DataLength, Ordered));
WriteLn('Done.');

ReadLn;
end.








