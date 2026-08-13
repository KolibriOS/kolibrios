program WaveMesh;

uses
  KolibriOS, TinyGL;

const
  // --- СЕКЦИЯ КОНСТАНТ ---
  GRID_SIZE    = 45;
  TOTAL_POINTS = 2025; // 45 * 45 вершин
  SPACING      = 11;   // Шаг сетки
  PHI          = 1.6180339887; // Золотое сечение

var
  CTX: TKOSGLContext;
  WndLeft, WndTop, WndWidth, WndHeight: LongInt;

  // --- СЕКЦИЯ ГЛОБАЛЬНЫХ МАССИВОВ ---
  Points_GridX: array[0..TOTAL_POINTS - 1] of GLint;
  Points_GridZ: array[0..TOTAL_POINTS - 1] of GLint;
  Points_LocalX: array[0..TOTAL_POINTS - 1] of GLFloat;
  Points_LocalZ: array[0..TOTAL_POINTS - 1] of GLFloat;

  // Храним трехмерные координаты для передачи в конвейер TinyGL
  Render_X: array[0..TOTAL_POINTS - 1] of GLFloat;
  Render_Y: array[0..TOTAL_POINTS - 1] of GLFloat;
  Render_Z: array[0..TOTAL_POINTS - 1] of GLFloat;
  Render_ColorIdx: array[0..TOTAL_POINTS - 1] of GLint;

  LookupTable: array[0..(GRID_SIZE * GRID_SIZE) - 1] of GLint;

  // Преобразованная палитра для TinyGL: хранит R, G, B компоненты от 0.0 до 1.0
  MeshPaletteR: array[0..255] of GLfloat;
  MeshPaletteG: array[0..255] of GLfloat;
  MeshPaletteB: array[0..255] of GLfloat;

  // --- СЕКЦИЯ ПРОСТЫХ ПЕРЕМЕННЫХ ---
  Frame: GLint = 0;
  Out_X: GLFloat = 0.0;
  Out_Y: GLFloat = 0.0;
  Out_Z: GLFloat = 0.0;

  // Глобальные переменные для предрасчитанной тригонометрии кадра
  CosY: GLFloat = 0.0; SinY: GLFloat = 0.0;
  CosX: GLFloat = 0.0; SinX: GLFloat = 0.0;

{ Настройка перспективы камеры }
procedure Perspective(fovY, Aspect, zNear, zFar: GLFloat);
var
  fW, fH, fovYPI360: GLFloat;
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

// --- НАСТРОЙКА РАДУЖНОЙ ПАЛИТРЫ НА 256 ЦВЕТОВ ---
procedure Setup256Palette;
var
  C, Sector, Step: Integer;
  R, G, B: Byte;
begin
  for C := 0 to 255 do
  begin
    Sector := Trunc((C / 256.0) * 6.0);
    Step := Trunc(((C / 256.0) * 6.0 - Sector) * 255.0);

    if Sector = 0 then begin R := 255; G := Step; B := 0; end
    else if Sector = 1 then begin R := 255 - Step; G := 255; B := 0; end
    else if Sector = 2 then begin R := 0; G := 255; B := Step; end
    else if Sector = 3 then begin R := 0; G := 255 - Step; B := 255; end
    else if Sector = 4 then begin R := Step; G := 0; B := 255; end
    else begin R := 255; G := 0; B := 255 - Step; end;

    // Переводим цвета в диапазон 0.0 - 1.0 для TinyGL
    MeshPaletteR[C] := R / 255.0;
    MeshPaletteG[C] := G / 255.0;
    MeshPaletteB[C] := B / 255.0;
  end;
end;

// --- НАЧАЛЬНАЯ ИНИЦИАЛИЗАЦИЯ СЕТКИ ---
procedure InitializeGrid;
var
  X, Z, Idx: Integer;
  StartOffset: GLFloat;
begin
  StartOffset := -((GRID_SIZE - 1) * SPACING) / 2.0;
  Idx := 0;
  for X := 0 to GRID_SIZE - 1 do
  begin
    for Z := 0 to GRID_SIZE - 1 do
    begin
      Points_GridX[Idx] := X;
      Points_GridZ[Idx] := Z;
      Points_LocalX[Idx] := StartOffset + X * SPACING;
      Points_LocalZ[Idx] := StartOffset + Z * SPACING;
      LookupTable[Z * GRID_SIZE + X] := Idx;
      Inc(Idx);
    end;
  end;
end;

// Вращение по оси Y
procedure FastRotateY(X, Z: GLFloat);
begin
  Out_X := X * CosY - Z * SinY;
  Out_Z := X * SinY + Z * CosY;
end;

// Вращение по оси X
procedure FastRotateX(Y, Z: GLFloat);
begin
  Out_Y := Y * CosX - Z * SinX;
  Out_Z := Y * SinX + Z * CosX;
end;

// --- ОТРИСОВКА ЛИНИИ В ТРЕХМЕРНОМ ПРОСТРАНСТВЕ ---
procedure DrawLine3D(X1, Y1, Z1, X2, Y2, Z2: GLFloat; ColorIdx: GLint);
begin
  glColor3f(MeshPaletteR[ColorIdx], MeshPaletteG[ColorIdx], MeshPaletteB[ColorIdx]);
  glBegin(GL_LINES);
    glVertex3f(X1, Y1, Z1);
    glVertex3f(X2, Y2, Z2);
  glEnd();
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  I, X, Z, CurrentIdx, NextXIdx, NextZIdx, ColorIdx, AvgColor: GLint;
  LX, LZ, TrueDistance, LogDist, Wave1, Wave2, TrueHeightY: GLFloat;
  RotY_X, RotY_Z, RotX_Y, RotX_Z: GLFloat;
  WorldRotation, CameraPitch: GLFloat;
  X1, Y1, Z1: GLFloat;
  C1, C2, C3: GLint;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  // Инициализируем контекст под текущие размеры окна
  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  // Очищаем экран черным цветом и сбрасываем буфер глубины
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST); // Включаем тест глубины для корректного перекрытия линий в 3D

  // Настраиваем перспективную проекцию
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 1.0, 2000.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Inc(Frame);

  // Вычисляем углы вращения математической сетки
  WorldRotation := Frame * 0.005;
  CameraPitch := 0.6 + Sin(Frame * 0.01) * 0.15;

  // Предрасчет тригонометрии
  CosY := Cos(WorldRotation); SinY := Sin(WorldRotation);
  CosX := Cos(CameraPitch);   SinX := Sin(CameraPitch);

  // ШАГ 1: Математический расчет волн и трехмерных координат вершин
  for I := 0 to TOTAL_POINTS - 1 do
  begin
    LX := Points_LocalX[I];
    LZ := Points_LocalZ[I];

    // Вычисляем радиальное расстояние от центра сетки
    TrueDistance := Sqrt(LX * LX + LZ * LZ);

    // Расчет высоты волны по формуле Золотого сечения (PHI)
    LogDist := TrueDistance * 0.035;
    Wave1 := Sin(LogDist - Frame * 0.04) * 22.0;
    Wave2 := Sin((LogDist * PHI) - (Frame * 0.04 / PHI)) * (22.0 / PHI);
    TrueHeightY := Wave1 + Wave2;

    // Вращение по оси Y
    FastRotateY(LX, LZ);
    RotY_X := Out_X;
    RotY_Z := Out_Z;

    // Вращение по оси X (наклон)
    FastRotateX(TrueHeightY, RotY_Z);
    RotX_Y := Out_Y;
    RotX_Z := Out_Z;

    // Сохраняем вычисленные 3D-координаты.
    // Сдвигаем сцену по оси Z на -600.0 единиц вглубь экрана, чтобы она попала во фрустум камеры.
    Render_X[I] := RotY_X;
    Render_Y[I] := RotX_Y;
    Render_Z[I] := RotX_Z - 600.0;

    // Нормализуем высоту в индекс цвета палитры от 0 до 255
    ColorIdx := Trunc(((TrueHeightY + 35.0) / 70.0) * 255.0);
    if ColorIdx < 0 then ColorIdx := 0;
    if ColorIdx > 255 then ColorIdx := 255;
    Render_ColorIdx[I] := ColorIdx;
  end;

  // ШАГ 2: Отрисовка ребер сетки в 3D пространстве
  for X := 0 to GRID_SIZE - 1 do
  begin
    for Z := 0 to GRID_SIZE - 1 do
    begin
      CurrentIdx := LookupTable[Z * GRID_SIZE + X];
      X1 := Render_X[CurrentIdx];
      Y1 := Render_Y[CurrentIdx];
      Z1 := Render_Z[CurrentIdx];
      C1 := Render_ColorIdx[CurrentIdx];

      // Горизонтальное ребро сетки (к правому соседу)
      if X < GRID_SIZE - 1 then
      begin
        NextXIdx := LookupTable[Z * GRID_SIZE + (X + 1)];
        C2 := Render_ColorIdx[NextXIdx];
        AvgColor := (C1 + C2) div 2;
        DrawLine3D(X1, Y1, Z1, Render_X[NextXIdx], Render_Y[NextXIdx], Render_Z[NextXIdx], AvgColor);
      end;

      // Вертикальное ребро сетки (к нижнему соседу)
      if Z < GRID_SIZE - 1 then
      begin
        NextZIdx := LookupTable[(Z + 1) * GRID_SIZE + X];
        C3 := Render_ColorIdx[NextZIdx];
        AvgColor := (C1 + C3) div 2;
        DrawLine3D(X1, Y1, Z1, Render_X[NextZIdx], Render_Y[NextZIdx], Render_Z[NextZIdx], AvgColor);
      end;
    end;
  end;

  // Выводим готовый кадр на экран
  kosglSwapBuffers();
end;

begin
  TinyGL_initialization;

  // Предварительный расчет палитры цветов и координат сетки
  Setup256Palette;
  InitializeGrid;

  with GetScreenSize do
  begin
    WndWidth  := Width div 2;
    WndHeight := Height div 2;
    WndLeft   := (Width - WndWidth) div 2;
    WndTop    := (Height - WndHeight) div 2;
  end;

  while True do
    case WaitEventByTime(1) of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'Wave Mesh (TinyGL 3D)', $00000000,
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
