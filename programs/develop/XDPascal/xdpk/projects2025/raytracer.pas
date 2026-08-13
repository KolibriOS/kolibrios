// Raytracer demo - demonstrates XD Pascal methods and interfaces

{$APPTYPE CONSOLE}

program Raytracer;


type 
  TVec = array [1..3] of Real;
  

function add for u: TVec (var v: TVec): TVec; 
var i: Integer;
begin 
for i := 1 to 3 do Result[i] := u[i] + v[i]; 
end;


function sub for u: TVec (var v: TVec): TVec; 
var i: Integer;
begin 
for i := 1 to 3 do Result[i] := u[i] - v[i]; 
end;


function mul for v: TVec (a: Real): TVec; 
var i: Integer;
begin 
for i := 1 to 3 do Result[i] := a * v[i]; 
end;


function dot for u: TVec (var v: TVec): Real; 
var i: Integer;
begin
Result := 0; 
for i := 1 to 3 do Result := Result + u[i] * v[i]; 
end;


function elementwise for u: TVec (var v: TVec): TVec; 
var i: Integer;
begin 
for i := 1 to 3 do Result[i] := u[i] * v[i]; 
end;


function norm(var v: TVec): Real;
begin 
Result := sqrt(v.dot(v));
end;


function normalize(var v: TVec): TVec;
begin 
Result := v.mul(1.0 / norm(v));
end;


function rand: TVec;
var i: Integer;
begin 
for i := 1 to 3 do Result[i] := Random - 0.5; 
end;


type
  TColor = TVec;

  TRay = record
    Origin, Dir: TVec;
  end;
  
  TGenericBody = record
    Center: TVec;
    Color: TColor;
    Diffuseness: Real;
    IsLamp: Boolean;
  end; 

  PGenericBody = ^TGenericBody; 


function LambertFactor for b: TGenericBody (Lambert: Real): Real;
begin
Result := 1.0 - (1.0 - Lambert) * b.Diffuseness;
end;


type 
  TBox = record
    GenericBody: TGenericBody;
    HalfSize: TVec;
  end;


function GetGenericBody for b: TBox: PGenericBody; 
begin 
Result := @b.GenericBody; 
end;


function Intersect for b: TBox (var Ray: TRay; var Point, Normal: TVec): Boolean; 
  
  function IntersectFace(i, j, k: Integer): Boolean;
  
    function Within(x, y, xmin, ymin, xmax, ymax: Real): Boolean;
    begin
    Result := (x > xmin) and (x < xmax) and (y > ymin) and (y < ymax);
    end;
  
  var
    Side, Factor: Real;  
    
  begin // IntersectFace
  Result := FALSE;
  
  if abs(Ray.Dir[k]) > 1e-9 then
    begin 
    Side := 1.0;
    if Ray.Dir[k] > 0.0 then Side := -1.0;
    
    Factor := (b.GenericBody.Center[k] + Side * b.HalfSize[k] - Ray.Origin[k]) / Ray.Dir[k];  
    if Factor > 0.1 then
      begin
      Point := Ray.Origin.add(Ray.Dir.mul(Factor));
      
      if Within(Point[i], Point[j],
                b.GenericBody.Center[i] - b.HalfSize[i], b.GenericBody.Center[j] - b.HalfSize[j],
                b.GenericBody.Center[i] + b.HalfSize[i], b.GenericBody.Center[j] + b.HalfSize[j])
      then
        begin
        Normal[i] := 0; Normal[j] := 0; Normal[k] := Side;
        Result := TRUE;
        end;
      end;
    end;
  end;
    
begin // Intersect
Result := IntersectFace(1, 2, 3) or IntersectFace(3, 1, 2) or IntersectFace(2, 3, 1);
end;          


type 
  TSphere = record
    GenericBody: TGenericBody;
    Radius: Real;
  end;
  
  
function GetGenericBody for s: TSphere: PGenericBody; 
begin 
Result := @s.GenericBody; 
end;  
  

function Intersect for s: TSphere (var Ray: TRay; var Point, Normal: TVec): Boolean;
var
  Displacement: TVec;
  Proj, Discr, Factor: Real;
begin
Displacement := s.GenericBody.Center.sub(Ray.Origin);
Proj := Displacement.dot(Ray.Dir);
Discr := sqr(s.Radius) + sqr(Proj) - Displacement.dot(Displacement);

if Discr > 0 then
  begin
  Factor := Proj - sqrt(Discr);
  if Factor > 0.1 then
    begin
    Point := Ray.Origin.add(Ray.Dir.mul(Factor));
    Normal := Point.sub(s.GenericBody.Center).mul(1.0 / s.Radius);
    Result := TRUE;
    Exit;
    end;
  end;

Result := FALSE;
end;


type
  IBody = interface
    GetGenericBody: function: PGenericBody;
    Intersect: function (var Ray: TRay; var Point, Normal: TVec): Boolean;
  end;


const
  MAXBODIES = 10;
  

type 
  TScene = record
    AmbientColor: TColor;
    Body: array [1..MAXBODIES] of IBody;
    NumBodies: Integer;
  end;
  

function Trace for sc: TScene (var Ray: TRay; Depth: Integer): TColor;
var
  BestBody: PGenericBody;
  Point, Normal, BestPoint, BestNormal, SpecularDir, DiffuseDir: TVec;
  DiffuseRay: TRay;
  Dist, BestDist, Lambert: Real;
  BestIndex, i: Integer;
  
begin
if Depth > 3 then
  begin
  Result := sc.AmbientColor;
  Exit;
  end;

// Find nearest intersection
BestDist := 1e9;
BestIndex := 0;

for i := 1 to sc.NumBodies do
  begin  
  if sc.Body[i].Intersect(Ray, Point, Normal) then
    begin
    Dist := norm(Point.sub(Ray.Origin));
    if Dist < BestDist then
      begin
      BestDist := Dist;
      BestIndex := i;
      BestPoint := Point;
      BestNormal := Normal;
      end;
    end;
  end;

// Reflect rays
if BestIndex > 0 then
  begin
  BestBody := sc.Body[BestIndex].GetGenericBody();

  if BestBody^.IsLamp then
    begin
    Result := BestBody^.Color;
    Exit;
    end;

  SpecularDir := Ray.Dir.sub(BestNormal.mul(2.0 * (Ray.Dir.dot(BestNormal))));
  DiffuseDir := normalize(SpecularDir.add(rand.mul(2.0 * BestBody^.Diffuseness)));

  Lambert := DiffuseDir.dot(BestNormal);
  if Lambert < 0 then
    begin
    DiffuseDir := DiffuseDir.sub(BestNormal.mul(2.0 * Lambert));
    Lambert := -Lambert;
    end;

  DiffuseRay.Origin := BestPoint;
  DiffuseRay.Dir := DiffuseDir;

  with BestBody^ do
    Result := sc.Trace(DiffuseRay, Depth + 1).mul(LambertFactor(Lambert)).elementwise(Color);
    
  Exit;
  end;

Result := sc.AmbientColor;
end;


// Main program

const  
  // Define scene
  Box1: TBox = 
    (
    GenericBody: 
      (
      Center: (500, -100, 1200);
      Color: (0.4, 0.7, 1.0);
      Diffuseness: 0.1;
      IsLamp: FALSE
      );      
    HalfSize: (400 / 2, 600 / 2, 300 / 2)
    );

  Box2: TBox = 
    (
    GenericBody:
      (
      Center: (550, 210, 1100);
      Color: (0.9, 1.0, 0.6);
      Diffuseness: 0.3;
      IsLamp: FALSE
      );      
    HalfSize: (1000 / 2, 20 / 2, 1000 / 2)
    );

  Sphere1: TSphere = 
    (
    GenericBody:
      (
      Center: (600, 0, 700);
      Color: (1.0, 0.4, 0.6);
      Diffuseness: 0.2;
      IsLamp: FALSE
      );      
    Radius: 200
    );

  Sphere2: TSphere = 
    (
    GenericBody:
      (
      Center: (330, 150, 700);
      Color: (1.0, 1.0, 0.3);
      Diffuseness: 0.15;
      IsLamp: FALSE
      );      
    Radius: 50
    );

  // Define light
  Lamp1: TSphere = 
    (
    GenericBody:
      (
      Center: (500, -1000, -700);
      Color: (1.0, 1.0, 1.0);
      Diffuseness: 1.0;
      IsLamp: TRUE
      );
    Radius: 800
    );
    
  AmbientLightColor: TColor = (0.2, 0.2, 0.2);  


  // Define eye
  Pos: TVec = (0, 0, 0);
  Azimuth = 30.0 * pi / 180.0;
  Width = 640;
  Height = 480;
  Focal = 500;
  Antialiasing = 1.0;
  
  
  // Output file
  FileName = 'scene.ppm';
  
  
var
  Scene: TScene;
  Dir, RotDir, RandomDir: TVec;
  Color: TColor;
  Ray: TRay;
  sinAz, cosAz: Real;
  F: Text;
  Rays, i, j, r: Integer;
  StartTime, StopTime: LongInt; 
  
  Buf: array [0..Width * Height * 3 - 1] of Byte;
  BufPos: LongInt = 0;
  

begin
WriteLn('Raytracer demo');
WriteLn;
Write('Rays per pixel (recommended 1 to 100): '); ReadLn(Rays);
WriteLn;

Randomize;

with Scene do
  begin 
  AmbientColor := AmbientLightColor;
  Body[1] := Box1;
  Body[2] := Box2;
  Body[3] := Sphere1;
  Body[4] := Sphere2;
  Body[5] := Lamp1;
  NumBodies := 5;
  end;
  
sinAz := sin(Azimuth);  cosAz := cos(Azimuth);
  
Assign(F, FileName);
Rewrite(F);
Write(F, 'P6 ');

Write(F, Width, ' ', Height, ' ');
Write(F, 255, ' ');

StartTime := GetTickCount;

for i := 0 to Height - 1 do
  begin
  for j := 0 to Width - 1 do
    begin
    Color[1] := 0; 
    Color[2] := 0; 
    Color[3] := 0; 

    Dir[1] := j - Width / 2;  
    Dir[2] := i - Height / 2;  
    Dir[3] := Focal; 

    RotDir[1] :=  Dir[1] * cosAz + Dir[3] * sinAz;
    RotDir[2] :=  Dir[2];
    RotDir[3] := -Dir[1] * sinAz + Dir[3] * cosAz;
  
    for r := 1 to Rays do
      begin
      RandomDir := RotDir.add(rand.mul(Antialiasing));
      Ray.Origin := Pos;
      Ray.Dir := normalize(RandomDir);
      Color := Color.add(Scene.Trace(Ray, 0));
      end;
      
    Color := Color.mul(255.0 / Rays);
    
    Buf[BufPos] := Round(Color[1]);
    Buf[BufPos + 1] := Round(Color[2]);
    Buf[BufPos + 2] := Round(Color[3]);
    BufPos := BufPos + 3;
    end;
    
  WriteLn(i + 1, '/', Height);
  end;
  
  BlockWrite(F, Buf, SizeOf(Buf));
  
StopTime := GetTickCount;
  
Close(F);

WriteLn;
WriteLn('Rendering time: ', (StopTime - StartTime) / 1000 :5:1, ' s');
WriteLn('Done. See ' + FileName);
ReadLn;  
end.
