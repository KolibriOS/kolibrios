program Pyramid_TinyGL;

uses
  KolibriOS, TinyGL;

var
  CTX: TKOSGLContext;
  Angle: GLFloat = 0;
  Speed: GLFloat = 3.5;

  WndLeft, WndTop, WndWidth, WndHeight: Integer;

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

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit; // otherwise app crashes

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort (0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(0.11, 0.22, 0.66, 0);
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);

  glLoadIdentity();
  glTranslatef(0, 0, -5);

  glRotatef(Angle, 0.25, 0.75, 0.75);

  glBegin(GL_TRIANGLE_FAN);
    glColor3f(1, 0, 0); glVertex3f(0, 0.75, 0);
    glColor3f(1, 1, 0); glVertex3f(-0.75, -0.75, 0.75);
    glColor3f(1, 1, 1); glVertex3f(0.75, -0.75, 0.75);
    glColor3f(0, 1, 1); glVertex3f(0.75, -0.75, -0.75);
    glColor3f(0, 0, 1); glVertex3f(-0.75, -0.75, -0.75);
    glColor3f(0, 1, 0); glVertex3f(-0.75, -0.75, 0.75);
  glEnd();

  Angle := Angle + Speed;
  if Angle > 360 then Angle := 0;

  kosglSwapBuffers();
 end;

begin
  TinyGL_initialization;

  with GetScreenSize do
  begin
    WndWidth  := Width div 2;
    WndHeight := Height div 3 * 2;
    WndLeft   := (Width - WndWidth) div 2;
    WndTop    := (Height - WndHeight) div 2;
  end;

  while True do
    case WaitEventByTime(1) of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'Pyramid TinyGL', $00FFFFFF,
            WS_SKINNED_SIZABLE + WS_TRANSPARENT_FILL + WS_CLIENT_COORDS + WS_CAPTION, CAPTION_MOVABLE);
          GLDraw;
          EndDraw;
        end;
      KEY_EVENT:
        GetKey;
      BUTTON_EVENT:
        if GetButton.ID = 1 then
          Exit;
    else
      GLDraw;
    end;

end.