program RubikRotate3D;

uses
  KolibriOS, TinyGL;

const
  CUBE_DIST = 0.72; // Расстояние между центрами кубиков

type
  TVertex3D = record
    X, Y, Z: GLFloat;
  end;

var
  Rotation:   GLFloat = 0;
  CamSpeed:   GLFloat = 0.5;
  ZTr:        GLFloat = -6.5;
  TimeVal:    GLFloat = 0;
  
  // Параметры анимации слоев
  LayerAngle:  GLFloat = 0;
  ActiveAxis:  Integer = 0; // 0 = X, 1 = Y, 2 = Z 
  ActiveLayer: Integer = -1; // -1 = левый, 0 = центральный, 1 = правый 
  
  CTX:         TKOSGLContext;
  WndLeft, WndTop, WndWidth, WndHeight: LongInt;

// Настройка перспективы камеры
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

// вращение одной 3D точки по осям X, Y или Z
procedure RotatePoint(var Px, Py, Pz: GLFloat; Axis: Integer; AngleDegrees: GLFloat);
var
  Rad, CosA, SinA, Tx, Ty, Tz: GLFloat;
begin
  Rad  := AngleDegrees * PI / 180.0;
  CosA := Cos(Rad);
  SinA := Sin(Rad);
  Tx   := Px; Ty := Py; Tz := Pz;

  if Axis = 0 then { Вокруг X }
  begin
    Py := Ty * CosA - Tz * SinA;
    Pz := Ty * SinA + Tz * CosA;
  end
  else if Axis = 1 then { Вокруг Y }
  begin
    Px := Tx * CosA + Tz * SinA;
    Pz := -Tx * SinA + Tz * CosA;
  end
  else if Axis = 2 then { Вокруг Z }
  begin
    Px := Tx * CosA - Ty * SinA;
    Py := Tx * SinA + Ty * CosA;
  end;
end;

// Отрисовка куба с явным расчетом 8 вершин
procedure DrawSubCube(S: GLFloat; GridX, GridY, GridZ: Integer; R, G, B: GLFloat);
var
  Pts: array[0..7] of TVertex3D;
  I: Integer;
  IsLayerActive: Boolean;
begin
  IsLayerActive := False;
  if (ActiveAxis = 0) and (GridX = ActiveLayer) then IsLayerActive := True;
  if (ActiveAxis = 1) and (GridY = ActiveLayer) then IsLayerActive := True;
  if (ActiveAxis = 2) and (GridZ = ActiveLayer) then IsLayerActive := True;

  // Задаем локальные координаты куба
  Pts[0].X := -S; Pts[0].Y := -S; Pts[0].Z :=  S;
  Pts[1].X :=  S; Pts[1].Y := -S; Pts[1].Z :=  S;
  Pts[2].X :=  S; Pts[2].Y :=  S; Pts[2].Z :=  S;
  Pts[3].X := -S; Pts[3].Y :=  S; Pts[3].Z :=  S;
  Pts[4].X := -S; Pts[4].Y := -S; Pts[4].Z := -S;
  Pts[5].X := -S; Pts[5].Y :=  S; Pts[5].Z := -S;
  Pts[6].X :=  S; Pts[6].Y :=  S; Pts[6].Z := -S;
  Pts[7].X :=  S; Pts[7].Y := -S; Pts[7].Z := -S;

  for I := 0 to 7 do
  begin
    // 1. Сначала сдвигаем элементы на радиус орбиты
    Pts[I].X := Pts[I].X + (GridX * CUBE_DIST);
    Pts[I].Y := Pts[I].Y + (GridY * CUBE_DIST);
    Pts[I].Z := Pts[I].Z + (GridZ * CUBE_DIST);

    // 2. Если кубик в активном слое, вращаем его строго в локальных осях
    if IsLayerActive then
      RotatePoint(Pts[I].X, Pts[I].Y, Pts[I].Z, ActiveAxis, LayerAngle);
  end;

  { Выводим 6 граней }
  glBegin(GL_QUADS);
    { Передняя }
    glColor3f(R, G, B);
    glVertex3f(Pts[0].X, Pts[0].Y, Pts[0].Z); glVertex3f(Pts[1].X, Pts[1].Y, Pts[1].Z);
    glVertex3f(Pts[2].X, Pts[2].Y, Pts[2].Z); glVertex3f(Pts[3].X, Pts[3].Y, Pts[3].Z);
    { Задняя }
    glColor3f(R * 0.5, G * 0.5, B * 0.5);
    glVertex3f(Pts[4].X, Pts[4].Y, Pts[4].Z); glVertex3f(Pts[5].X, Pts[5].Y, Pts[5].Z);
    glVertex3f(Pts[6].X, Pts[6].Y, Pts[6].Z); glVertex3f(Pts[7].X, Pts[7].Y, Pts[7].Z);
    { Верхняя }
    glColor3f(R * 0.9, G * 0.9, B * 0.9);
    glVertex3f(Pts[5].X, Pts[5].Y, Pts[5].Z); glVertex3f(Pts[3].X, Pts[3].Y, Pts[3].Z);
    glVertex3f(Pts[2].X, Pts[2].Y, Pts[2].Z); glVertex3f(Pts[6].X, Pts[6].Y, Pts[6].Z);
    { Нижняя }
    glColor3f(R * 0.3, G * 0.3, B * 0.3);
    glVertex3f(Pts[4].X, Pts[4].Y, Pts[4].Z); glVertex3f(Pts[7].X, Pts[7].Y, Pts[7].Z);
    glVertex3f(Pts[1].X, Pts[1].Y, Pts[1].Z); glVertex3f(Pts[0].X, Pts[0].Y, Pts[0].Z);
    { Правая }
    glColor3f(R * 0.8, G * 0.8, B * 0.8);
    glVertex3f(Pts[7].X, Pts[7].Y, Pts[7].Z); glVertex3f(Pts[6].X, Pts[6].Y, Pts[6].Z);
    glVertex3f(Pts[2].X, Pts[2].Y, Pts[2].Z); glVertex3f(Pts[1].X, Pts[1].Y, Pts[1].Z);
    { Левая }
    glColor3f(R * 0.6, G * 0.6, B * 0.6);
    glVertex3f(Pts[4].X, Pts[4].Y, Pts[4].Z); glVertex3f(Pts[0].X, Pts[0].Y, Pts[0].Z);
    glVertex3f(Pts[3].X, Pts[3].Y, Pts[3].Z); glVertex3f(Pts[5].X, Pts[5].Y, Pts[5].Z);
  glEnd();
end;

procedure UpdateRubikLogic;
begin
  LayerAngle := LayerAngle + 3.0;
  if LayerAngle >= 90.0 then
  begin
    LayerAngle := 0;
    Inc(ActiveLayer);
    if ActiveLayer > 1 then
    begin
      ActiveLayer := -1;
      ActiveAxis := (ActiveAxis + 1) mod 3;
    end;
  end;
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  X, Y, Z: Integer;
  R_Clr, G_Clr, B_Clr: GLFloat;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(0.03, 0.03, 0.06, 0.0);
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // МИРОВЫЕ ТРАНСФОРМАЦИИ (Вращение камеры вокруг всей сцены)
  glTranslatef(0.0, 0.0, ZTr);
  glRotatef(25.0 + Rotation * 0.4, 1.0, 0.0, 0.0);
  glRotatef(35.0 + Rotation, 0.0, 1.0, 0.0);

  // СБОРКА И ОТРИСОВКА СЕТКИ
  for X := -1 to 1 do
    for Y := -1 to 1 do
      for Z := -1 to 1 do
      begin
        // палитра на синусах от времени
        R_Clr := 0.7 + Sin(TimeVal + X * 0.5) * 0.3;
        G_Clr := 0.7 + Cos(TimeVal * 0.8 + Y * 0.5) * 0.3;
        B_Clr := 0.7 + Sin(TimeVal * 1.2 + Z * 0.5) * 0.3;

        DrawSubCube(0.26, X, Y, Z, R_Clr, G_Clr, B_Clr);
      end;

  kosglSwapBuffers();

  UpdateRubikLogic;
  Rotation := Rotation + CamSpeed;
  TimeVal  := TimeVal + 0.03;
end;

begin
  TinyGL_initialization;

  with GetScreenSize do
  begin
    WndWidth  := Width div 3 * 2; 
    WndHeight := Height div 3 * 2;
    WndLeft   := (Width - WndWidth) div 2; 
    WndTop := (Height - WndHeight) div 2;
  end;

  while True do
    case WaitEventByTime(1) of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, '3D Rubik Rotate', $00FFFFFF,
            WS_SKINNED_SIZABLE + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);
          GLDraw;
          EndDraw;
        end;
      KEY_EVENT: GetKey;
      BUTTON_EVENT: if GetButton.ID = 1 then ExitThread;
    else
      GLDraw;
    end;
end.