program Perfect3DBalls;

uses
  KolibriOS, TinyGL;

const
  MAX_BALLS = 72;
  GRAVITY   = 0.015;
  AIR       = 0.99;
  BOUNCE    = -0.85;

type
  TBall = record
    Active: Boolean;
    IsShard: Boolean;
    X, Y, Z: Single;
    VX, VY, VZ: Single;
    Radius: Single;
    Hue: Integer;
    Darkness: Single;
    Life: Single;
  end;

var
  WndLeft, WndTop, WndWidth, WndHeight: LongInt;
  CTX: TKOSGLContext;
  Balls: array[0..MAX_BALLS - 1] of TBall;
  Angle: Single = 0.0;

  BOX_X, BOX_Y, BOX_Z: Single;
  i: Integer;

function Max(A, B: Extended): Extended; begin if A > B then Result := A else Result := B; end;
function Min(A, B: Extended): Extended; begin if A < B then Result := A else Result := B; end;

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

function Frac(N: Single): Single;
begin
  Result := N - Trunc(N) ;
end;

procedure HSLtoRGB(H: Integer; S, L: Single; var R, G, B: Single);
var
  C, X, M, R1, G1, B1: Single;
  HPrime: Single;
begin
  C := (1.0 - Abs(2.0 * L - 1.0)) * S;
  HPrime := H / 60.0;
  X := C * (1.0 - Abs(Frac(HPrime / 2.0) * 2.0 - 1.0));
  M := L - C / 2.0;
  if (HPrime >= 0) and (HPrime < 1) then      begin R1 := C; G1 := X; B1 := 0; end
  else if (HPrime >= 1) and (HPrime < 2) then begin R1 := X; G1 := C; B1 := 0; end
  else if (HPrime >= 2) and (HPrime < 3) then begin R1 := 0; G1 := C; B1 := X; end
  else if (HPrime >= 3) and (HPrime < 4) then begin R1 := 0; G1 := X; B1 := C; end
  else if (HPrime >= 4) and (HPrime < 5) then begin R1 := X; G1 := 0; B1 := C; end
  else                                        begin R1 := C; G1 := 0; B1 := X; end;
  R := R1 + M; G := G1 + M; B := B1 + M;
end;

procedure DrawSphere(Radius: Single);
var
  l_i, l_j: Integer;
  t_Segments: Integer;
  lat0, z0, r0, lat1, z1, r1: Single;
  lng, x, y: Single;
begin
  t_Segments := 18;

  for l_i := 0 to t_Segments - 1 do
  begin
    lat0 := PI * (-0.5 + l_i / t_Segments);
    z0   := Sin(lat0);
    r0   := Cos(lat0);

    lat1 := PI * (-0.5 + (l_i + 1) / t_Segments);
    z1   := Sin(lat1);
    r1   := Cos(lat1);

    glBegin(GL_QUAD_STRIP);
    for l_j := 0 to t_Segments do
    begin
      lng := 2.0 * PI * l_j / t_Segments;
      x := Cos(lng);
      y := Sin(lng);

      glNormal3f(x * r0, y * r0, z0);
      glVertex3f(x * r0 * Radius, y * r0 * Radius, z0 * Radius);

      glNormal3f(x * r1, y * r1, z1);
      glVertex3f(x * r1 * Radius, y * r1 * Radius, z1 * Radius);
    end;
    glEnd();
  end;
end;

procedure ResizeTopology(W, H: Integer);
begin
  BOX_X := 5.0 * (W / H);
  BOX_Y := 5.0;
  BOX_Z := 5.0;
end;

function FindFreeBallIndex: Integer;
var
  K: Integer;
begin
  Result := -1;
  for K := 0 to MAX_BALLS - 1 do
  begin
    if not Balls[K].Active then
    begin
      Result := K;
      Exit;
    end;
  end;
end;

procedure SpawnBall(Index: Integer; IsShard: Boolean; ParentIndex: Integer);
begin
  Balls[Index].Active := True;
  Balls[Index].IsShard := IsShard;
  Balls[Index].Darkness := 1.0;

  if not IsShard then
  begin
    // Появляются в самом верху (BOX_Y) и падают вниз
    Balls[Index].X := (Random * 100 / 100.0 - 0.5) * BOX_X * 1.6;
    Balls[Index].Y := BOX_Y - 0.5 - (Random * 100 / 50.0);
    Balls[Index].Z := (Random * 100 / 100.0 - 0.5) * BOX_Z * 1.6;
    Balls[Index].VX := (Random * 100 / 100.0 - 0.5) * 0.4;
    Balls[Index].VY := -(Random * 100 / 100.0) * 0.1; // Направление вектора вниз
    Balls[Index].VZ := (Random * 100 / 100.0 - 0.5) * 0.4;
    Balls[Index].Radius := 0.4 + (Random * 100 / 250.0);
    Balls[Index].Hue := Trunc(Random * 360);
    Balls[Index].Life := 1.0;
  end
  else
  begin
    Balls[Index].X := Balls[ParentIndex].X;
    Balls[Index].Y := Balls[ParentIndex].Y;
    Balls[Index].Z := Balls[ParentIndex].Z;
    Balls[Index].Radius := Balls[ParentIndex].Radius * 0.35;
    Balls[Index].Hue := Balls[ParentIndex].Hue;
    Balls[Index].Life := 1.0;
  end;
end;

procedure UpdatePhysics;
var
  Idx, J, FreeIdx: Integer;
  DX, DY, DZ, Dist, MinDist: Single;
  NX, NY, NZ, Overlap: Single;
  KX, KY, KZ, PFactor: Single;
  XS, YS, ZS: Single;
begin
  for Idx := 0 to MAX_BALLS - 1 do
  begin
    if not Balls[Idx].Active then Continue;
    for J := Idx + 1 to MAX_BALLS - 1 do
    begin
      if not Balls[J].Active then Continue;
      DX := Balls[J].X - Balls[Idx].X;
      DY := Balls[J].Y - Balls[Idx].Y;
      DZ := Balls[J].Z - Balls[Idx].Z;
      Dist := Sqrt(DX * DX + DY * DY + DZ * DZ);
      MinDist := Balls[Idx].Radius + Balls[J].Radius;

      if Dist < MinDist then
      begin
        if Dist > 0.001 then
        begin
          NX := DX / Dist; NY := DY / Dist; NZ := DZ / Dist;
        end
        else
        begin
          NX := 0.1; NY := 0.0; NZ := 0.0;
        end;
        Overlap := MinDist - Dist;
        Balls[Idx].X := Balls[Idx].X - NX * Overlap * 0.5;
        Balls[Idx].Y := Balls[Idx].Y - NY * Overlap * 0.5;
        Balls[Idx].Z := Balls[Idx].Z - NZ * Overlap * 0.5;
        Balls[J].X := Balls[J].X + NX * Overlap * 0.5;
        Balls[J].Y := Balls[J].Y + NY * Overlap * 0.5;
        Balls[J].Z := Balls[J].Z + NZ * Overlap * 0.5;
        KX := Balls[Idx].VX - Balls[J].VX;
        KY := Balls[Idx].VY - Balls[J].VY;
        KZ := Balls[Idx].VZ - Balls[J].VZ;
        PFactor := 2.0 * (NX * KX + NY * KY + NZ * KZ) / 2.0 + 0.01;
        Balls[Idx].VX := Balls[Idx].VX - PFactor * NX;
        Balls[Idx].VY := Balls[Idx].VY - PFactor * NY;
        Balls[Idx].VZ := Balls[Idx].VZ - PFactor * NZ;
        Balls[J].VX := Balls[J].VX + PFactor * NX;
        Balls[J].VY := Balls[J].VY + PFactor * NY;
        Balls[J].VZ := Balls[J].VZ + PFactor * NZ;
      end;
    end;
  end;

  for Idx := 0 to MAX_BALLS - 1 do
  begin
    if not Balls[Idx].Active then Continue;

    Balls[Idx].VY := Balls[Idx].VY - GRAVITY;
    Balls[Idx].VX := Balls[Idx].VX * AIR;
    Balls[Idx].VY := Balls[Idx].VY * AIR;
    Balls[Idx].VZ := Balls[Idx].VZ * AIR;
    Balls[Idx].X := Balls[Idx].X + Balls[Idx].VX;
    Balls[Idx].Y := Balls[Idx].Y + Balls[Idx].VY;
    Balls[Idx].Z := Balls[Idx].Z + Balls[Idx].VZ;

    if Balls[Idx].IsShard then
    begin
      Balls[Idx].Life := Balls[Idx].Life - 0.015;
      Balls[Idx].Darkness := Balls[Idx].Life;

      if Balls[Idx].Y - Balls[Idx].Radius < -BOX_Y then
      begin
        Balls[Idx].Y := -BOX_Y + Balls[Idx].Radius; Balls[Idx].VY := Balls[Idx].VY * BOUNCE;
      end;
      if Balls[Idx].Y + Balls[Idx].Radius > BOX_Y then
      begin
        Balls[Idx].Y := BOX_Y - Balls[Idx].Radius; Balls[Idx].VY := Balls[Idx].VY * BOUNCE;
      end;
      if Balls[Idx].X - Balls[Idx].Radius < -BOX_X then
      begin
        Balls[Idx].X := -BOX_X + Balls[Idx].Radius; Balls[Idx].VX := Balls[Idx].VX * BOUNCE;
      end;
      if Balls[Idx].X + Balls[Idx].Radius > BOX_X then
      begin
        Balls[Idx].X := BOX_X - Balls[Idx].Radius; Balls[Idx].VX := Balls[Idx].VX * BOUNCE;
      end;
      if Balls[Idx].Z - Balls[Idx].Radius < -BOX_Z then
      begin
        Balls[Idx].Z := -BOX_Z + Balls[Idx].Radius; Balls[Idx].VZ := Balls[Idx].VZ * BOUNCE;
      end;
      if Balls[Idx].Z + Balls[Idx].Radius > BOX_Z then
      begin
        Balls[Idx].Z := BOX_Z - Balls[Idx].Radius; Balls[Idx].VZ := Balls[Idx].VZ * BOUNCE;
      end;
      if Balls[Idx].Life <= 0.0 then Balls[Idx].Active := False;
    end
    else
    begin
      // Разрушение больших шаров при контакте с любой из 3D-границ окна
      if (Balls[Idx].Y - Balls[Idx].Radius < -BOX_Y) or
         (Balls[Idx].Y + Balls[Idx].Radius > BOX_Y) or
         (Abs(Balls[Idx].X) + Balls[Idx].Radius > BOX_X) or
         (Abs(Balls[Idx].Z) + Balls[Idx].Radius > BOX_Z) then
      begin
        XS := -0.25;
        while XS <= 0.25 do
        begin
          YS := -0.25;
          while YS <= 0.25 do
          begin
            ZS := -0.25;
            while ZS <= 0.25 do
            begin
              FreeIdx := FindFreeBallIndex;
              if FreeIdx <> -1 then
              begin
                SpawnBall(FreeIdx, True, Idx);
                Balls[FreeIdx].VX := Balls[Idx].VX * 0.4 + XS * (1.0 + Random * 50 / 100.0);
                Balls[FreeIdx].VY := Balls[Idx].VY * BOUNCE + YS * (1.0 + Random * 50 / 100.0) + 0.15;
                Balls[FreeIdx].VZ := Balls[Idx].VZ * 0.4 + ZS * (1.0 + Random * 50 / 100.0);
              end;
              ZS := ZS + 0.5;
            end;
            YS := YS + 0.5;
          end;
          XS := XS + 0.5;
        end;
        SpawnBall(Idx, False, 0);
      end;
    end;
  end;
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  CosA, SinA: Single;
  LightPos: array[0..3] of GLfloat;
  LightAmbient: array[0..3] of GLfloat;
  LightDiffuse: array[0..3] of GLfloat;
  MatAmbient: array[0..3] of GLfloat;
  MatDiffuse: array[0..3] of GLfloat;
  MatSpecular: array[0..3] of GLfloat;
  R, G, B: Single;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if not (ThreadInfo.Client.Height > 3) then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(5/255, 6/255, 11/255, 0.0);
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  UpdatePhysics;
  ResizeTopology(ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  Angle := Angle + 0.003;
  CosA := Cos(Angle); SinA := Sin(Angle);

  glTranslatef(0.0, 0.0, -13.5);

  // Настройка позиции света (смещен чуть выше для красивого блика)
  LightPos[0] := -8.0 * CosA - (-6.0) * SinA;
  LightPos[1] := 15.0; // Направлен сверху вниз
  LightPos[2] := -8.0 * SinA + (-6.0) * CosA;
  LightPos[3] := 0.5;

  LightAmbient[0] := 0.35; LightAmbient[1] := 0.35; LightAmbient[2] := 0.38; LightAmbient[3] := 0.5;
  LightDiffuse[0] := 0.95; LightDiffuse[1] := 0.95; LightDiffuse[2] := 0.95; LightDiffuse[3] := 0.5;

  glLightfv(GL_LIGHT0, GL_POSITION, @LightPos[0]);
  glLightfv(GL_LIGHT0, GL_AMBIENT, @LightAmbient[0]);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, @LightDiffuse[0]);

  MatSpecular[0] := 0.9; MatSpecular[1] := 0.9; MatSpecular[2] := 0.9; MatSpecular[3] := 0.5;
  glMaterialfv(GL_FRONT, GL_SPECULAR, @MatSpecular[0]);
  glMaterialf(GL_FRONT, GL_SHININESS, 64.0);

  for i := 0 to MAX_BALLS - 1 do
  begin
    if not Balls[i].Active then Continue;

    HSLtoRGB(Balls[i].Hue, 0.95, 0.55 * Balls[i].Darkness, R, G, B);

    MatAmbient[0] := R * 0.6; MatAmbient[1] := G * 0.6; MatAmbient[2] := B * 0.6; MatAmbient[3] := 1.0;
    MatDiffuse[0] := R;       MatDiffuse[1] := G;       MatDiffuse[2] := B;       MatDiffuse[3] := 1.0;

    glMaterialfv(GL_FRONT, GL_AMBIENT, @MatAmbient[0]);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, @MatDiffuse[0]);

    glPushMatrix();
      glTranslatef(Balls[i].X, Balls[i].Y, Balls[i].Z);
      DrawSphere(Balls[i].Radius);
    glPopMatrix();
  end;

  kosglSwapBuffers();
end;

begin
  TinyGL_initialization;

  Randomize;
  with GetScreenSize do
  begin
    WndWidth  := Width div 3 * 2;
    WndHeight := Height div 3 * 2;
    WndLeft   := (Width - WndWidth) div 2;
    WndTop    := (Height - WndHeight) div 2;
  end;

  ResizeTopology(WndWidth, WndHeight);

  FillChar(Balls, SizeOf(Balls), #0);
  for i := 0 to 24 do
  begin
    SpawnBall(i, False, 0);
    // Распределяем равномерно по всей высоте коробки (от -BOX_Y до BOX_Y)
    Balls[i].Y := -BOX_Y + 1.0 + (Random * 100 / 100.0) * (BOX_Y * 2.0 - 2.0);
  end;

  while True do
    case WaitEventByTime(1) of
      REDRAW_EVENT:
        begin
          BeginDraw;
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'TinyGL Light 3D Balls', $00FFFFFF,
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