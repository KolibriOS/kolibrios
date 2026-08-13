program DNAHelix3D;

uses
  KolibriOS, TinyGL;

const
  MAX_ELEMENTS = 40;   { Количество ступеней в цепочке ДНК }
  RADIUS       = 1.1;  { Радиус закручивания спирали }
  STEP_HEIGHT  = 0.12; { Расстояние между ступенями по вертикали }

type
  TBaseNode = record
    X1, Y1, Z1: GLFloat;
    X2, Y2, Z2: GLFloat;
    R, G, B:    GLFloat;
  end;

var
  Nodes:    array[1..MAX_ELEMENTS] of TBaseNode;
  Rotation: GLFloat = 0;
  Speed:    GLFloat = 0.5;
  ZTr:      GLFloat = -7.5;
  TimeVal:  GLFloat = 0;
  CTX:      TKOSGLContext;

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

{ Математический расчет волнового скручивания и деформации ДНК }
procedure UpdateDNAPhysics;
var
  I: Integer;
  Angle, VertPos, Wave: GLFloat;
begin
  for I := 1 to MAX_ELEMENTS do
  begin
    { Базовый угол закручивания + динамическая волна времени }
    Angle := (I * 0.4) + Sin(TimeVal + I * 0.15) * 0.25;
    VertPos := (I - (MAX_ELEMENTS div 2)) * STEP_HEIGHT;

    { Тепловой волновой шум }
    Wave := Cos(TimeVal * 2.0 + I * 0.1) * 0.08;

    { Нить А спирали }
    Nodes[I].X1 := Cos(Angle) * (RADIUS + Wave);
    Nodes[I].Y1 := VertPos;
    Nodes[I].Z1 := Sin(Angle) * (RADIUS + Wave);

    { Нить Б спирали (противофаза 180 градусов / PI) }
    Nodes[I].X2 := Cos(Angle + 3.14159) * (RADIUS + Wave);
    Nodes[I].Y2 := VertPos;
    Nodes[I].Z2 := Sin(Angle + 3.14159) * (RADIUS + Wave);

    { Цветовое кодирование нуклеотидов в зависимости от высоты }
    Nodes[I].R := 0.4 + Sin(I * 0.2) * 0.4;
    Nodes[I].G := 0.2 + Cos(I * 0.1) * 0.4;
    Nodes[I].B := 0.8 - Sin(I * 0.05) * 0.2;
  end;
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  I: Integer;
  S: GLFloat;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(0.15, 0.15, 0.3, 0.0); { Глубокий био-кибернетический фон }
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.0, 0.0, ZTr);
  glRotatef(Rotation, 0.3, 1.0, 0.1); { Плавный облет камеры }

  S := 0.06; { Толщина узлов }

  for I := 1 to MAX_ELEMENTS do
  begin
    { 1. Рисуем перемычки между нитями ДНК }
    glBegin(GL_QUADS);
      glColor3f(Nodes[I].R * 0.6, Nodes[I].G * 0.6, Nodes[I].B * 0.6);
      glVertex3f(Nodes[I].X1, Nodes[I].Y1 - 0.02, Nodes[I].Z1);
      glVertex3f(Nodes[I].X2, Nodes[I].Y2 - 0.02, Nodes[I].Z2);
      glVertex3f(Nodes[I].X2, Nodes[I].Y2 + 0.02, Nodes[I].Z2);
      glVertex3f(Nodes[I].X1, Nodes[I].Y1 + 0.02, Nodes[I].Z1);
    glEnd();

    { 2. Рисуем сферу-кубик нити А }
    glBegin(GL_QUADS);
      glColor3f(Nodes[I].R, Nodes[I].G, Nodes[I].B);
      glVertex3f(Nodes[I].X1 - S, Nodes[I].Y1 - S, Nodes[I].Z1 + S);
      glVertex3f(Nodes[I].X1 + S, Nodes[I].Y1 - S, Nodes[I].Z1 + S);
      glVertex3f(Nodes[I].X1 + S, Nodes[I].Y1 + S, Nodes[I].Z1 + S);
      glVertex3f(Nodes[I].X1 - S, Nodes[I].Y1 + S, Nodes[I].Z1 + S);
    glEnd();

    { 3. Рисуем сферу-кубик нити Б }
    glBegin(GL_QUADS);
      glColor3f(Nodes[I].B, Nodes[I].R, Nodes[I].G); { Инвертированный цвет для пары }
      glVertex3f(Nodes[I].X2 - S, Nodes[I].Y2 - S, Nodes[I].Z2 + S);
      glVertex3f(Nodes[I].X2 + S, Nodes[I].Y2 - S, Nodes[I].Z2 + S);
      glVertex3f(Nodes[I].X2 + S, Nodes[I].Y2 + S, Nodes[I].Z2 + S);
      glVertex3f(Nodes[I].X2 - S, Nodes[I].Y2 + S, Nodes[I].Z2 + S);
    glEnd();
  end;

  kosglSwapBuffers();

  UpdateDNAPhysics;
  Rotation := Rotation + Speed;
  TimeVal  := TimeVal + 0.03;
end;

begin
  TinyGL_initialization;

  with GetScreenSize do
  begin
    WndWidth  := Width div 3 * 2;
    WndHeight := Height div 3 * 2;
    WndLeft   := (Width - WndWidth) div 2;
    WndTop    := (Height - WndHeight) div 2;
  end;

  while True do
    case WaitEventByTime(1) of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, '3D DNA Helix Wave', $00FFFFFF,
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