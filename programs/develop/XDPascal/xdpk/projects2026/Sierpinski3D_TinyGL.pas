program SierpinskiTinyGL;

uses
  KolibriOS, TinyGL;


type
  Point3D = record
    X, Y, Z: GLFloat;
  end;
  
const
  { Координаты базового тетраэдра }
  TopV:   Point3D = (X:  0.0; Y: -1.0; Z:  0.0);
  LeftV:  Point3D = (X: -1.0; Y:  0.7; Z: -0.5);
  RightV: Point3D = (X:  1.0; Y:  0.7; Z: -0.5);
  BackV:  Point3D = (X:  0.0; Y:  0.7; Z:  1.0);

var
  AngleX:    GLFloat = 0.5;
  AngleY:    GLFloat = 0.5;
  TimeColor: GLFloat = 0.0;
  TimeDepth: GLFloat = 0.0;
  CTX:       TKOSGLContext;

  WndLeft, WndTop, WndWidth, WndHeight: LongInt;

  IsLine: Boolean;
  
{ Настройка перспективы камеры }
procedure Perspective(fovY, Aspect, zNear, zFar: GLDouble);
var
  fW, fH, fovYPI360: GLDouble;
  M: array[0..15] of GLFloat;
begin
  FillChar(M, SizeOf(M), #0);
  fovYPI360 := fovY * PI / 360;
  fH := Sin(fovYPI360) * zNear / Cos(fovYPI360);
  fW := fH * Aspect;
  M[0] := zNear / fW;
  M[5] := zNear / fH;
  M[10] := -(zFar + zNear) / (zFar - zNear);
  M[11] := -1;
  M[14] := -2 * zFar * zNear / (zFar - zNear);
  glLoadMatrixf(@M[0]);
end;

function Midpoint(P1, P2: Point3D): Point3D;
begin
  Result.X := (P1.X + P2.X) / 2;
  Result.Y := (P1.Y + P2.Y) / 2;
  Result.Z := (P1.Z + P2.Z) / 2;
end;

{ вычисляет жидкий градиентный цвет по координатам точки
  и передаёт вершину в TinyGL }
procedure GlColorVertex(P: Point3D);
var
  R, G, B: GLFloat;
begin
  { Жидкий градиент}
  R := sin(P.X * 2 + TimeColor) * 0.5 + 0.5;
  G := sin(P.Y * 2 + TimeColor * 1.5) * 0.5 + 0.5;
  B := cos(P.Z * 2 + TimeColor) * 0.5 + 0.5;
  
  if not IsLine then
    glColor3f(R, G, B)
  else
    glColor3f(1 - R * 0.25, 1 - G * 0.25, 1 - B * 0.25);  
    
  glVertex3f(P.X, P.Y, P.Z);
end;

procedure DrawTetrahedron(P1, P2, P3, P4: Point3D);
begin
  GlColorVertex(P1); GlColorVertex(P2); GlColorVertex(P3);
  GlColorVertex(P1); GlColorVertex(P3); GlColorVertex(P4);
  GlColorVertex(P1); GlColorVertex(P4); GlColorVertex(P2);
  GlColorVertex(P2); GlColorVertex(P3); GlColorVertex(P4);
end;

procedure Sierpinski3D(P1, P2, P3, P4: Point3D; Depth: Integer);
var
  M12, M23, M31, M14, M24, M34: Point3D;
begin
  if Depth = 0 then
    DrawTetrahedron(P1, P2, P3, P4)
  else
  begin
    M12 := Midpoint(P1, P2);
    M23 := Midpoint(P2, P3);
    M31 := Midpoint(P3, P1);
    M14 := Midpoint(P1, P4);
    M24 := Midpoint(P2, P4);
    M34 := Midpoint(P3, P4);
    
    Sierpinski3D(P1,  M12, M31, M14, Depth - 1);
    Sierpinski3D(M12, P2,  M23, M24, Depth - 1);
    Sierpinski3D(M31, M23, P3,  M34, Depth - 1);
    Sierpinski3D(M14, M24, M34, P4,  Depth - 1);
  end;
end;

procedure GLDraw;
var
  ThreadInfo:   TThreadInfo;
  CurrentDepth: Integer;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  { Камера }
  glTranslatef(0.0, 0.0, -3.5);

  { Вращение }
  glRotatef(AngleX * (180 / PI), 1.0, 0.0, 0.0);
  glRotatef(AngleY * (180 / PI), 0.0, 1.0, 0.0);

  { Изменение глубины фрактала }
  CurrentDepth := Trunc(2.5 + Sin(TimeDepth) * 1.6);
  if CurrentDepth < 1 then CurrentDepth := 1;
  if CurrentDepth > 4 then CurrentDepth := 4;

  { рисуем с заливкой }
  IsLine := False;  
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);    
  glBegin(GL_TRIANGLES);
  Sierpinski3D(TopV, LeftV, RightV, BackV, CurrentDepth);
  glEnd();

  { рисуем контурные линии }
  IsLine := True;
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  
  glBegin(GL_TRIANGLES);
  Sierpinski3D(TopV, LeftV, RightV, BackV, CurrentDepth);
  glEnd();
  
  kosglSwapBuffers();

  { Обновление углов }
  AngleX    := AngleX    + 0.003;
  AngleY    := AngleY    + 0.007;
  
  TimeColor := TimeColor + 0.02;
  TimeDepth := TimeDepth + 0.008;
end;

begin
  TinyGL_initialization;

  with GetScreenSize do
  begin
    WndWidth  := Width div 3 * 2;
    WndHeight := Height div 3 * 2;
    WndLeft   := (Width  - WndWidth)  div 2;
    WndTop    := (Height - WndHeight) div 2;
  end;

  while True do
    case WaitEventByTime(1) of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight,
            'Sierpinski 3D', $00000000,
            WS_SKINNED_SIZABLE + WS_CLIENT_COORDS + WS_CAPTION,
            CAPTION_MOVABLE);
          GLDraw;
          EndDraw;
        end;
      KEY_EVENT:
        GetKey;
      BUTTON_EVENT:
        if GetButton.ID = 1 then
          ExitThread;
    else
      GLDraw;
    end;
end.