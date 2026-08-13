program CircleParticlesTinyGL;

uses
  KolibriOS, TinyGL;

const
  // Максимальное количество частиц
  MAX_PARTICLES = 3000;

  // Количество новых частиц за кадр
  SPAWN_PER_FRAME = 18;

  // Размер палитры
  PALETTE_SIZE = 64;

  // Количество сегментов круга
  CIRCLE_SEGMENTS = 12;

  // Угол обзора перспективы
  FIELD_OF_VIEW = 45.0;

type
  // Частица
  TParticle = record
    Life: LongInt;
    X, Y: GLFloat;
    VX, VY: GLFloat;
    Radius: GLFloat;
  end;

  // Цвет палитры
  TPaletteColor = record
    R, G, B: GLFloat;
  end;

var
  // Контекст TinyGL
  CTX: TKOSGLContext;

  // Пул частиц
  Particles: array[0..MAX_PARTICLES - 1] of TParticle;

  // Радужная палитра
  RainbowPalette: array[0..PALETTE_SIZE - 1] of TPaletteColor;

  // Курсор поиска свободной частицы
  PoolCursor: LongInt = 0;

  // Счётчик кадров
  FrameCounter: LongInt = 0;

  // Размеры и положение окна
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

// Случайное Float число
function RandomFloat(MinValue, MaxValue: GLFloat): GLFloat;
begin
  RandomFloat :=
    MinValue +
    Random * (MaxValue - MinValue);
end;

// HSV -> RGB
procedure HSVtoRGB(H, S, V: GLFloat; var R, G, B: GLFloat);
var
  I: LongInt;
  F, P, Q, T: GLFloat;
begin
  H := H * 6.0;
  I := Trunc(H);
  F := H - I;
  P := V * (1.0 - S);
  Q := V * (1.0 - S * F);
  T := V * (1.0 - S * (1.0 - F));
  case I mod 6 of
    0: begin R := V; G := T; B := P; end;
    1: begin R := Q; G := V; B := P; end;
    2: begin R := P; G := V; B := T; end;
    3: begin R := P; G := Q; B := V; end;
    4: begin R := T; G := P; B := V; end;
  else
    begin R := V; G := P; B := Q; end;
  end;
end;

// Радужная палитра
procedure BuildRainbowPalette;
var
  I: LongInt;
  H: GLFloat;
begin
  for I := 0 to PALETTE_SIZE - 1 do
  begin
    H := 1 - I / (PALETTE_SIZE - 1);
    HSVtoRGB(
      H,
      1.0,
      1.0,
      RainbowPalette[I].R,
      RainbowPalette[I].G,
      RainbowPalette[I].B
    );
  end;
end;

// Рисование круглой частицы
procedure DrawParticleCircle(X, Y, Radius: GLFloat);
var
  I: LongInt;
  Angle, PX, PY: GLFloat;
begin
  glBegin(GL_TRIANGLE_FAN);

    // Центр круга
    glVertex3f(X, Y, 0.0);

    // Окружность
    for I := 0 to CIRCLE_SEGMENTS do
    begin
      Angle := (PI * 2.0 * I) / CIRCLE_SEGMENTS;

      PX := X + Cos(Angle) * Radius;
      PY := Y + Sin(Angle) * Radius;

      glVertex3f(PX, PY, 0.0);
    end;

  glEnd();
end;

// Создание частицы
procedure SpawnParticle(EmitX, EmitY: GLFloat);
var
  Attempts: LongInt;
  P: ^TParticle;
  Angle,
  Speed: GLFloat;
begin
  Attempts := MAX_PARTICLES;

  while Attempts > 0 do
  begin
    P := @Particles[PoolCursor];

    PoolCursor :=
      (PoolCursor + 1) mod MAX_PARTICLES;

    if P^.Life <= 0 then
    begin
      Angle := RandomFloat(0.0, PI * 2.0);
      Speed := RandomFloat(5.0, 13.0);

      P^.X := EmitX;
      P^.Y := EmitY;

      P^.VX := Cos(Angle) * Speed;
      P^.VY := Sin(Angle) * Speed - 7.0;

      P^.Life := Round(40 + Random * 24);

      P^.Radius := 1 + Random * 3;

      Exit;
    end;

    Dec(Attempts);
  end;
end;

// Основной рендер
procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  Width,
  Height: LongInt;
  CameraZ, EmitX,
  EmitY: GLFloat;
  I: LongInt;
  P: ^TParticle;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);

  Width  := ThreadInfo.Client.Width;
  Height := ThreadInfo.Client.Height;

  if Height <= 3 then Exit;

  kosglMakeCurrent(0, 0, Width, Height, CTX);
  glViewPort( 0, 0, Width, Height);

  // Первый кадр полностью очищает экран
  if FrameCounter = 0 then
  begin
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
  end;

  // Настройка перспективы
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(
    FIELD_OF_VIEW,
    Width / Height,
    0.1,
    5000.0
  );

  // Динамическая камера
  // Размер сцены стабилен при любом размере окна
  CameraZ :=
    Height /
    (Sin(FIELD_OF_VIEW * PI / 360.0) /
    Cos(FIELD_OF_VIEW * PI / 360.0));
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glTranslatef(
    -Width * 0.5,
    -Height * 0.5,
    -CameraZ * 0.5
  );

  // Положение эмиттера
  // Отступы стабильны при любом размере окна
  EmitX :=
    (Width * 0.5) +
    Sin(FrameCounter * 0.03) *
    (Width * 0.46);

  EmitY :=
    (Height * 0.5) +
    Cos(FrameCounter * 0.02) *
    (Height * 0.44);

  // Создание новых частиц
  for I := 0 to SPAWN_PER_FRAME - 1 do
    SpawnParticle(EmitX, EmitY);

  // Обновление частиц
  for I := 0 to MAX_PARTICLES - 1 do
  begin
    P := @Particles[I];

    if P^.Life <= 0 then
      Continue;

    // Движение
    P^.X := P^.X + P^.VX;
    P^.Y := P^.Y + P^.VY;

    // Гравитация
    P^.VY := P^.VY + 0.18;

    // Отскок от стенок
    if P^.X > Width - P^.Radius then
    begin
      P^.X := Width - P^.Radius;
      P^.VX := P^.VX * (-0.80);
    end;

    if P^.X < P^.Radius then
    begin
      P^.X := P^.Radius;
      P^.VX := P^.VX * (-0.80);
    end;

    // Отскок от потолка и пола
    if P^.Y > Height - P^.Radius then
    begin
      P^.Y := Height - P^.Radius;
      P^.VY := P^.VY * (-0.70);
      P^.VX := P^.VX * 0.94;
    end;

    if P^.Y < P^.Radius then
    begin
      P^.Y := P^.Radius;
      P^.VY := P^.VY * (-0.70);
      P^.VX := P^.VX * 0.94;
    end;

    // Уменьшение жизни
    Dec(P^.Life);

    if P^.Life > 0 then
    begin
      glColor3f(
        RainbowPalette[P^.Life].R,
        RainbowPalette[P^.Life].G,
        RainbowPalette[P^.Life].B
      );

      DrawParticleCircle(
        P^.X,
        P^.Y,
        P^.Radius
      );
    end;
  end;

  // Вывод кадра
  kosglSwapBuffers();

  Inc(FrameCounter);
end;

begin
  TinyGL_initialization;

  Randomize;
  BuildRainbowPalette;

  with GetScreenSize do
  begin
    WndWidth  := Width div 2;
    WndHeight := Height div 2;
    WndLeft := (Width - WndWidth) div 2;
    WndTop := (Height - WndHeight) div 2;
  end;

  while True do
  begin
    case WaitEventByTime(1) of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(
            WndLeft,
            WndTop,
            WndWidth,
            WndHeight,
            'Circle Particles TinyGL',
            $00000000,
            WS_SKINNED_SIZABLE +
            WS_CLIENT_COORDS +
            WS_CAPTION,
            CAPTION_MOVABLE
          );
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
  end;
end.