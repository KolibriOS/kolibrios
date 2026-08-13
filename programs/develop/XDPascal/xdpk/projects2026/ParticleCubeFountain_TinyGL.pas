program ParticleCubeFountain;

uses
  KolibriOS, TinyGL;

const
  MAX_PARTICLES = 100; // максимальное количество кубиков

  // НАСТРОЙКА ФИЗИКИ
  GRAVITY       = 0.0066; // Сила тяжести: чем меньше, тем выше взлетает фонтан
  BOUNCE_FACTOR = 0.70;   // Коэффициент отскока: 0.70 означает сохранение 70% высоты прыжка
  FLOOR_Y       = -3.0;   // Уровень пола, от которого отскакивают кубики

type
  TParticle = record
    X, Y, Z:    GLFloat;
    VX, VY, VZ: GLFloat;
    R, G, B:    GLFloat;
    Life:       GLFloat;
  end;

var
  Particles: array[1..MAX_PARTICLES] of TParticle;
  Rotation:  GLFloat = 0;
  Speed:     GLFloat = 0.8;
  ZTr:       GLFloat = -15.0;
  CTX:       TKOSGLContext;

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

procedure ResetParticle(Index: Integer);
begin
  Particles[Index].X := 0.0;
  Particles[Index].Y := -1.5;
  Particles[Index].Z := 0.0;

  Particles[Index].VX := ((Index mod 11) - 5) * 0.012;
  Particles[Index].VY := 0.10 + (Index mod 7) * 0.012; // Начальная скорость вверх
  Particles[Index].VZ := ((Index mod 13) - 6) * 0.012;

  // Насыщенные цвета (смесь фиолетового, синего и бирюзового для разнообразия)
  Particles[Index].R := 0.2 + (Index mod 4) * 0.2;
  Particles[Index].G := 0.4 + (Index mod 6) * 0.1;
  Particles[Index].B := 0.8 + (Index mod 3) * 0.1;

  Particles[Index].Life := 1.0;
end;

procedure InitParticles;
var
  I: Integer;
begin
  for I := 1 to MAX_PARTICLES do
    ResetParticle(I);
end;

procedure UpdatePhysics;
var
  I: Integer;
begin
  for I := 1 to MAX_PARTICLES do
  begin
    Particles[I].X := Particles[I].X + Particles[I].VX;
    Particles[I].Y := Particles[I].Y + Particles[I].VY;
    Particles[I].Z := Particles[I].Z + Particles[I].VZ;

    // Применяем настраиваемую гравитацию
    Particles[I].VY := Particles[I].VY - GRAVITY;

    // Отскок от пола с учетом настраиваемых параметров
    if Particles[I].Y < FLOOR_Y then
    begin
      Particles[I].Y := FLOOR_Y;
      Particles[I].VY := -Particles[I].VY * BOUNCE_FACTOR;
    end;

    Particles[I].Life := Particles[I].Life - 0.006;

    if Particles[I].Life <= 0.0 then
      ResetParticle(I);
  end;
end;

// процедура отрисовки 3D-куба с гранями
procedure DrawCube(R, G, B: GLFloat);
begin
  glBegin(GL_QUADS);
    // Передняя грань
    glColor3f(R, G, B);
    glVertex3f(-1.0, -1.0,  1.0);  glVertex3f( 1.0, -1.0,  1.0);
    glVertex3f( 1.0,  1.0,  1.0);  glVertex3f(-1.0,  1.0,  1.0);
    // Задняя грань
    glColor3f(R * 0.8, G * 0.8, B * 0.8); // Чуть темнее для объема
    glVertex3f(-1.0, -1.0, -1.0);  glVertex3f(-1.0,  1.0, -1.0);
    glVertex3f( 1.0,  1.0, -1.0);  glVertex3f( 1.0, -1.0, -1.0);
    // Верхняя грань
    glColor3f(R * 0.9, G * 0.9, B * 0.9);
    glVertex3f(-1.0,  1.0, -1.0);  glVertex3f(-1.0,  1.0,  1.0);
    glVertex3f( 1.0,  1.0,  1.0);  glVertex3f( 1.0,  1.0, -1.0);
    // Нижняя грань
    glColor3f(R * 0.6, G * 0.6, B * 0.6);
    glVertex3f(-1.0, -1.0, -1.0);  glVertex3f( 1.0, -1.0, -1.0);
    glVertex3f( 1.0, -1.0,  1.0);  glVertex3f(-1.0, -1.0,  1.0);
    // Правая грань
    glColor3f(R * 0.85, G * 0.85, B * 0.85);
    glVertex3f( 1.0, -1.0, -1.0);  glVertex3f( 1.0,  1.0, -1.0);
    glVertex3f( 1.0,  1.0,  1.0);  glVertex3f( 1.0, -1.0,  1.0);
    // Левая грань
    glColor3f(R * 0.75, G * 0.75, B * 0.75);
    glVertex3f(-1.0, -1.0, -1.0);  glVertex3f(-1.0, -1.0,  1.0);
    glVertex3f(-1.0,  1.0,  1.0);  glVertex3f(-1.0,  1.0, -1.0);
  glEnd();
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  I: Integer;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(0.05, 0.05, 0.1, 0.0); // Глубокий полуночный фон
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.0, 0.0, ZTr);
  glRotatef(Rotation, 0.0, 1.0, 0.0); // Вращение сцены по оси Y

  // Отрисовка плоскости пола
  glBegin(GL_QUADS);
    glColor3f(0.25, 0.25, 0.45);
    glVertex3f(-6.0, FLOOR_Y,  6.0);
    glVertex3f( 6.0, FLOOR_Y,  6.0);
    glVertex3f( 6.0, FLOOR_Y, -6.0);
    glVertex3f(-6.0, FLOOR_Y, -6.0);
  glEnd();

  // Цикл отрисовки 3D-кубиков
  for I := 1 to MAX_PARTICLES do
  begin
    glPushMatrix();
    glTranslatef(Particles[I].X, Particles[I].Y, Particles[I].Z);
    // Вращаем каждый кубик вокруг своей оси для динамики
    glRotatef(Rotation * 2.5 + I, 0.4, 0.8, 0.2);
    // Масштабируем куб пропорционально его времени жизни
    glScalef(Particles[I].Life * 0.25, Particles[I].Life * 0.25, Particles[I].Life * 0.25);
    // Вызов кастомной отрисовки геометрии
    DrawCube(Particles[I].R, Particles[I].G, Particles[I].B);
    glPopMatrix();
  end;

  kosglSwapBuffers();

  UpdatePhysics;
  Rotation := Rotation + Speed;
end;

begin
  TinyGL_initialization;

  InitParticles;

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
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'TinyGL 3D Cube Fountain', $00FFFFFF,
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