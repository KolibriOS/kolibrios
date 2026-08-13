program FractalStormForest;

uses
  KolibriOS, TinyGL;

const
  NUM_TREES   = 7; // количество деревьев
  MAX_DEPTH   = 5; // глубина фрактала

var
  Rotation:   GLFloat = 0;
  Speed:      GLFloat = 0.5;
  ZTr:        GLFloat = -11.0;
  TimeVal:    GLFloat = 0;
  CTX:        TKOSGLContext;

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

procedure DrawBranch(Length: GLFloat; Depth: Integer; WindForce: GLFloat);
var
  R, G, B: GLFloat;
begin
  if Depth > MAX_DEPTH then Exit;

  // ОСЕННЯЯ ПАЛИТРА ЛИСТВЫ
  // Ствол темно-бордовый }
  // Крона ярко-золотая / оранжевая }
  R := 0.3 + (Depth * 0.18);
  G := 0.05 + (Depth * 0.18);
  B := 0.1;

  glBegin(GL_QUADS);
    glColor3f(R, G, B);
    glVertex3f(-0.1 * Length, 0.0,  0.1 * Length);
    glVertex3f( 0.1 * Length, 0.0,  0.1 * Length);
    glVertex3f( 0.05 * Length, Length,  0.05 * Length);
    glVertex3f(-0.05 * Length, Length,  0.05 * Length);

    glColor3f(R * 0.8, G * 0.8, B * 0.8);
    glVertex3f( 0.1 * Length, 0.0,  0.1 * Length);
    glVertex3f( 0.1 * Length, 0.0, -0.1 * Length);
    glVertex3f( 0.05 * Length, Length, -0.05 * Length);
    glVertex3f( 0.05 * Length, Length,  0.05 * Length);

    glColor3f(R * 0.6, G * 0.6, B * 0.6);
    glVertex3f( 0.1 * Length, 0.0, -0.1 * Length);
    glVertex3f(-0.1 * Length, 0.0, -0.1 * Length);
    glVertex3f(-0.05 * Length, Length, -0.05 * Length);
    glVertex3f( 0.05 * Length, Length, -0.05 * Length);

    glColor3f(R * 0.7, G * 0.7, B * 0.7);
    glVertex3f(-0.1 * Length, 0.0, -0.1 * Length);
    glVertex3f(-0.1 * Length, 0.0,  0.1 * Length);
    glVertex3f(-0.05 * Length, Length,  0.05 * Length);
    glVertex3f(-0.05 * Length, Length, -0.05 * Length);
  glEnd();

  glTranslatef(0.0, Length, 0.0);

  glPushMatrix();
  glRotatef(22.0 + WindForce, 1.0, 0.0, 0.0);
  glRotatef(20.0, 0.0, 0.0, 1.0);
  glScalef(0.75, 0.75, 0.75);
  DrawBranch(Length, Depth + 1, WindForce * 1.25);
  glPopMatrix();

  glPushMatrix();
  glRotatef(-22.0 + WindForce, 1.0, 0.0, 0.0);
  glRotatef(-20.0, 0.0, 0.0, 1.0);
  glScalef(0.75, 0.75, 0.75);
  DrawBranch(Length, Depth + 1, WindForce * 1.25);
  glPopMatrix();

  glPushMatrix();
  glRotatef(30.0, 0.0, 1.0, 0.0);
  glRotatef(15.0 + WindForce, 0.0, 0.0, 1.0);
  glScalef(0.7, 0.7, 0.7);
  DrawBranch(Length, Depth + 1, WindForce * 1.15);
  glPopMatrix();
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  I: Integer;
  TreeAngle, Wind: GLFloat;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(0.04, 0.04, 0.08, 0.0); // Иссиня-черное небо
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glTranslatef(0.0, -1.2, ZTr);
  glRotatef(Rotation, 0.0, 1.0, 0.0);

  glBegin(GL_QUADS);
    glColor3f(0.35, 0.30, 0.20);
    glVertex3f(-4.0, -1.0,  4.0);  glVertex3f( 4.0, -1.0,  4.0);
    glVertex3f( 4.0, -1.0, -4.0);  glVertex3f(-4.0, -1.0, -4.0);
  glEnd();

  // ПОРЫВИСТЫЙ ВЕТЕР
  Wind := (Sin(TimeVal * 0.8) * 6.0) + (Sin(TimeVal * 2.5) * 3.5) + (Sin(TimeVal * 6.0) * 1.5);

  for I := 0 to NUM_TREES - 1 do
  begin
    glPushMatrix();
    TreeAngle := (I * 360.0) / NUM_TREES; 
    glRotatef(TreeAngle, 0.0, 1.0, 0.0);
    glTranslatef(0.0, -1.0, 2.2);

    DrawBranch(2.0, 1, Wind);
    glPopMatrix();
  end;

  kosglSwapBuffers();

  Rotation := Rotation + Speed;
  TimeVal  := TimeVal + 0.04;
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
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'TinyGL Autumn Storm', $00FFFFFF,
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