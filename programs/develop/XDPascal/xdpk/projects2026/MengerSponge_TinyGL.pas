program MengerSpongeFractal;

uses
  KolibriOS, TinyGL;

const
  MAX_DEPTH = 2; { Глубина фрактала. Внимание: значение 3 затормозит старый процессор }

var
  Rotation: GLFloat = 0;
  Speed:    GLFloat = 0.835;
  ZTr:      GLFloat = -5.0;
  TimeVal:  GLFloat = 0;
  CTX:      TKOSGLContext;

  WndLeft, WndTop, WndWidth, WndHeight: LongInt;

  IsLine: Boolean;

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

// Отрисовка базового куба фрактала с псевдо-освещением граней
procedure DrawSolidCube(Size: GLFloat);
var
  S: GLFloat;
begin
  S := Size * 0.5;
  glBegin(GL_QUADS);
    // Передняя грань - Яркая бирюзовая
    if not IsLine then
      glColor3f(0.1, 0.7, 0.8)
    else
      glColor3f(0.5, 0.5, 0.5);
    glVertex3f(-S, -S,  S); glVertex3f( S, -S,  S);
    glVertex3f( S,  S,  S); glVertex3f(-S,  S,  S);
    // Задняя грань - Синяя
    if not IsLine then
      glColor3f(0.05, 0.4, 0.6)
    else
      glColor3f(0.5, 0.5, 0.5);
    glVertex3f(-S, -S, -S); glVertex3f(-S,  S, -S);
    glVertex3f( S,  S, -S); glVertex3f( S, -S, -S);
    // Верхняя грань - Светло-голубая
    if not IsLine then
      glColor3f(0.2, 0.8, 1.0)
    else
      glColor3f(0.5, 0.5, 0.5);
    glVertex3f(-S,  S, -S); glVertex3f(-S,  S,  S);
    glVertex3f( S,  S,  S); glVertex3f( S,  S, -S);
    // Нижня грань - Темно-синяя
    if not IsLine then
      glColor3f(0.02, 0.2, 0.4)
    else
      glColor3f(0.5, 0.5, 0.5);
    glVertex3f(-S, -S, -S); glVertex3f( S, -S, -S);
    glVertex3f( S, -S,  S); glVertex3f(-S, -S,  S);
    // Правая грань }
    if not IsLine then
      glColor3f(0.08, 0.5, 0.7)
    else
      glColor3f(0.5, 0.5, 0.5);
    glVertex3f( S, -S, -S); glVertex3f( S,  S, -S);
    glVertex3f( S,  S,  S); glVertex3f( S, -S,  S);
    // Левая грань }
    if not IsLine then
      glColor3f(0.06, 0.45, 0.65)
    else
      glColor3f(0.5, 0.5, 0.5);
    glVertex3f(-S, -S, -S); glVertex3f(-S, -S,  S);
    glVertex3f(-S,  S,  S); glVertex3f(-S,  S, -S);
  glEnd();
end;

// Рекурсивный алгоритм генерации Губки Менгера
procedure BuildSponge(X, Y, Z, Size: GLFloat; Depth: Integer);
var
  NewSize: GLFloat;
  dx, dy, dz: Integer;
  Sum: Integer;
begin
  // Если дошли до предела рекурсии — рисуем сплошной кубик в этих координатах
  if Depth = MAX_DEPTH then
  begin
    glPushMatrix();
    glTranslatef(X, Y, Z);
    DrawSolidCube(Size);
    glPopMatrix();    
  end
  else
  begin
    NewSize := Size / 3.0;    
    // Перебираем сетку 3x3x3
    for dx := -1 to 1 do
      for dy := -1 to 1 do
        for dz := -1 to 1 do
        begin
          // Вычисляем математическое условие пустоты для Губки Менгера
          Sum := 0;
          if dx = 0 then Inc(Sum);
          if dy = 0 then Inc(Sum);
          if dz = 0 then Inc(Sum);
          // Если более чем одна координата равна 0, это сквозное отверстие — пропускаем его
          if Sum < 2 then
          begin
            BuildSponge(
              X + dx * NewSize,
              Y + dy * NewSize,
              Z + dz * NewSize,
              NewSize,
              Depth + 1
            );
          end;
        end;
  end;      
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  Pulse: GLFloat;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(0.03, 0.04, 0.06, 0.0); // Глубокий космический фон
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.0, 0.0, ZTr);

  // Вращаем фрактал по двум осям для полноценного 3D обзора
  glRotatef(Rotation, 0.5, 1.0, 0.2);

  // Вычисляем физическую пульсацию размера от времени
  Pulse := 1.8 + Sin(TimeVal) * 0.25;

  // Запуск генерации фрактала из центра (0,0,0) с динамическим размером
  IsLine := False;
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // рисуем с заливкой 
  BuildSponge(0.0, 0.0, 0.0, Pulse, 0);
  IsLine := True;
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // рисуем контурные линии
  BuildSponge(0.0, 0.0, 0.0, Pulse, 0);

  kosglSwapBuffers();

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
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'Menger Sponge 3D Fractal', $00FFFFFF,
            WS_SKINNED_SIZABLE + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);
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