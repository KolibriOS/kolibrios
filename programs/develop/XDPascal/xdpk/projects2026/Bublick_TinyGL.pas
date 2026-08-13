program TorusDemo;

uses
  KolibriOS, TinyGL;

const
  MAX_VERTICES = 20000;
  MAX_FACES    = 20000;

type
  TVertex = record
    x, y, z: Single;
    r, g, b: Byte;
  end;

  TFace = record
    v1, v2, v3, v4: TVertex;
    avgZ: Single;
    colorR, colorG, colorB: Byte;
  end;

var
  WndLeft, WndTop, WndWidth, WndHeight: LongInt;
  CTX: TKOSGLContext;

  MainRadius, BaseTubeRadius: Single;
  TorusSegments, TubeSegments, WaveCount: Integer;

  ProjectedVertices: array[0..MAX_VERTICES - 1] of TVertex;
  Faces: array[0..MAX_FACES - 1] of TFace;
  TotalVerticesCount, TotalFacesCount: Integer;

  AngleX: Single = 0.6;
  AngleY: Single = 0.5;
  TimeColor: Single = 0.0;
  TimeScale: Single = 0.0;

// процедура перспективной матрицы
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

function Min(A, B: Extended): Extended;
begin
  if A < B then Result := A else Result := B;
end;

function Max(A, B: Extended): Extended;
begin
  if A > B then Result := A else Result := B;
end;

function Power(Base, Exponent: Extended): Extended;
begin
  Result := Exp(Exponent * Ln(Base));
end;

procedure ResizeTopology(W, H: Integer);
begin
  MainRadius := H / 4.85;
  BaseTubeRadius := MainRadius / 2.45;

  TorusSegments := Round(H / 10);
  TubeSegments := Round(H / 15);
  WaveCount := Round(H / 100);

  if TorusSegments < 16 then TorusSegments := 16;
  if TubeSegments < 8 then TubeSegments := 8;
  if WaveCount < 3 then WaveCount := 3;

  TotalVerticesCount := (TorusSegments + 1) * (TubeSegments + 1);
  TotalFacesCount := TorusSegments * TubeSegments;

  // Защита от переполнения статического буфера
  if TotalVerticesCount > MAX_VERTICES then TotalVerticesCount := MAX_VERTICES;
  if TotalFacesCount > MAX_FACES then TotalFacesCount := MAX_FACES;
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  I, J, Idx, Stride: Integer;
  U, V, Wave1, Wave2, CurrentTubeRadius, SpineWave: Single;
  CosU, SinU, CosV, SinV, CosX, SinX, CosY, SinY: Single;
  X, Y, Z, X1, Z1, Y2, Z2, Factor, Fov: Single;
  CamDist, CenterX, CenterY: Single;
  Idx1, Idx2, Idx3, Idx4: Integer;
  P1, P2, P3, P4: TVertex;
  AX, AY, AZ, BX, BY, BZ, NX, NY, NZ, Len: Single;
  RFinal, GFinal, BFinal: Single;
  ViewDot, Rim, CenterDarkness: Single;
  NormX, NormY, NormZ: Single;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if not (ThreadInfo.Client.Height > 3) then Exit;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glClearColor(10/255, 10/255, 10/255, 0.0);
  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);

  // Считаем шаги сетки
  ResizeTopology(ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  Perspective(45.0, ThreadInfo.Client.Width / ThreadInfo.Client.Height, 0.1, 100.0);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  CamDist := 500.0;
  CenterX := ThreadInfo.Client.Width / 2.0;
  CenterY := ThreadInfo.Client.Height / 2.0;
  Fov := Min(ThreadInfo.Client.Width, ThreadInfo.Client.Height) * 1.1;

  CosY := Cos(AngleY); SinY := Sin(AngleY);
  CosX := Cos(AngleX); SinX := Sin(AngleX);

  // === Шаг 1: Математический расчет волнового тора ===
  Idx := 0;
  for I := 0 to TorusSegments do
  begin
    U := (I / TorusSegments) * PI * 2.0;
    Wave1 := Sin(U * WaveCount + TimeScale) * (BaseTubeRadius * 0.44);
    Wave2 := Cos(U * (WaveCount * 2) - TimeScale * 1.5) * (BaseTubeRadius * 0.17);
    CurrentTubeRadius := BaseTubeRadius + Wave1 + Wave2;
    SpineWave := Sin(U * 3.0 - TimeScale * 1.2) * (MainRadius * 0.25);
    CosU := Cos(U); SinU := Sin(U);

    for J := 0 to TubeSegments do
    begin
      if Idx >= MAX_VERTICES then Break;

      V := (J / TubeSegments) * PI * 2.0;
      CosV := Cos(V); SinV := Sin(V);

      X := (MainRadius + CurrentTubeRadius * CosV) * CosU;
      Y := CurrentTubeRadius * SinV + SpineWave;
      Z := (MainRadius + CurrentTubeRadius * CosV) * SinU;

      X1 := X * CosY - Z * SinY;
      Z1 := X * SinY + Z * CosY;
      Y2 := Y * CosX - Z1 * SinX;
      Z2 := Y * SinX + Z1 * CosX;

      Factor := Fov / (Z2 + CamDist);

      ProjectedVertices[Idx].x := CenterX + X1 * Factor;
      ProjectedVertices[Idx].y := CenterY + Y2 * Factor;
      ProjectedVertices[Idx].z := Z2;

      ProjectedVertices[Idx].r := Round((Sin(X * 0.015 + TimeColor) * 0.5 + 0.5) * 255.0);
      ProjectedVertices[Idx].g := Round((Sin(Y * 0.015 + TimeColor * 1.4) * 0.5 + 0.5) * 255.0);
      ProjectedVertices[Idx].b := Round((Cos(Z * 0.015 + TimeColor) * 0.5 + 0.5) * 255.0);

      Inc(Idx);
    end;
  end;

  // === Шаг 2: Сборка четырехугольных граней и Rim Lighting ===
  Stride := TubeSegments + 1;
  Idx := 0;
  for I := 0 to TorusSegments - 1 do
  begin
    for J := 0 to TubeSegments - 1 do
    begin
      if Idx >= MAX_FACES then Break;

      Idx1 := I * Stride + J;
      Idx2 := (I + 1) * Stride + J;
      Idx3 := (I + 1) * Stride + (J + 1);
      Idx4 := I * Stride + (J + 1);

      if (Idx1 >= MAX_VERTICES) or (Idx2 >= MAX_VERTICES) or
         (Idx3 >= MAX_VERTICES) or (Idx4 >= MAX_VERTICES) then Continue;

      P1 := ProjectedVertices[Idx1];
      P2 := ProjectedVertices[Idx2];
      P3 := ProjectedVertices[Idx3];
      P4 := ProjectedVertices[Idx4];

      AX := P2.x - P1.x; AY := P2.y - P1.y; AZ := P2.z - P1.z;
      BX := P4.x - P1.x; BY := P4.y - P1.y; BZ := P4.z - P1.z;
      NX := AY * BZ - AZ * BY;
      NY := AZ * BX - ax * BZ;
      NZ := AX * BY - AY * BX;
      Len := Sqrt(NX * NX + NY * NY + NZ * NZ);

      RFinal := P1.r; GFinal := P1.g; BFinal := P1.b;

      if Len > 0.0 then
      begin
        NZ := NZ / Len;
        ViewDot := Abs(NZ);
        Rim := Power(1.0 - ViewDot, 3) * 1.5;
        CenterDarkness := Max(0.15, ViewDot * 0.5);

        RFinal := (P1.r * CenterDarkness) + (P1.r * Rim);
        GFinal := (P1.g * CenterDarkness) + (P1.g * Rim);
        BFinal := (P1.b * CenterDarkness) + (P1.b * Rim);

        if RFinal > 255.0 then RFinal := 255.0;
        if GFinal > 255.0 then GFinal := 255.0;
        if BFinal > 255.0 then BFinal := 255.0;
      end;

      Faces[Idx].v1 := P1;
      Faces[Idx].v2 := P2;
      Faces[Idx].v3 := P3;
      Faces[Idx].v4 := P4;
      Faces[Idx].avgZ := (P1.z + P2.z + P3.z + P4.z) * 0.25;
      Faces[Idx].colorR := Round(RFinal);
      Faces[Idx].colorG := Round(GFinal);
      Faces[Idx].colorB := Round(BFinal);

      Inc(Idx);
    end;
  end;

  // Отрисовка в стандартном перспективном 3D-пространстве
  // Сдвигаем сцену назад по оси Z, чтобы геометрия полностью вошла в пирамиду Frustum
  glTranslatef(0.0, 0.0, -4.7);

  // === Шаг 4: Визуализация полигонов средствами TinyGL ===
  for I := 0 to TotalFacesCount - 1 do
  begin
    glBegin(GL_QUADS);
      glColor3f(Faces[I].colorR / 255.0, Faces[I].colorG / 255.0, Faces[I].colorB / 255.0);

      // переводим пиксели в диапазон (-1.0..1.0)
      // В glVertex3f передается глубина NormZ
      NormX := (Faces[I].v1.x / ThreadInfo.Client.Width) * 2.0 - 1.0;
      NormY := 1.0 - (Faces[I].v1.y / ThreadInfo.Client.Height) * 2.0;
      NormZ := -((Faces[I].v1.z + 250.0) / 500.0); // Масштабируем Z в канонический диапазон видимости
      glVertex3f(NormX * 1.95, NormY * 1.45, NormZ);

      NormX := (Faces[I].v2.x / ThreadInfo.Client.Width) * 2.0 - 1.0;
      NormY := 1.0 - (Faces[I].v2.y / ThreadInfo.Client.Height) * 2.0;
      NormZ := -((Faces[I].v2.z + 250.0) / 500.0);
      glVertex3f(NormX * 1.95, NormY * 1.45, NormZ);

      NormX := (Faces[I].v3.x / ThreadInfo.Client.Width) * 2.0 - 1.0;
      NormY := 1.0 - (Faces[I].v3.y / ThreadInfo.Client.Height) * 2.0;
      NormZ := -((Faces[I].v3.z + 250.0) / 500.0);
      glVertex3f(NormX * 1.95, NormY * 1.45, NormZ);

      NormX := (Faces[I].v4.x / ThreadInfo.Client.Width) * 2.0 - 1.0;
      NormY := 1.0 - (Faces[I].v4.y / ThreadInfo.Client.Height) * 2.0;
      NormZ := -((Faces[I].v4.z + 250.0) / 500.0);
      glVertex3f(NormX * 1.95, NormY * 1.45, NormZ);
    glEnd();
  end;

  kosglSwapBuffers();

  // === Шаг 5: Инкремент шагов анимации и динамических фазовых сдвигов ===
  AngleX := AngleX + 0.01;
  AngleY := AngleY + 0.02;
  TimeColor := TimeColor + 0.02;
  TimeScale := TimeScale + 0.1;
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
          DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'TinyGL Torus Demo', $00FFFFFF,
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