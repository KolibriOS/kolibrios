program Donkey3D_TinyGL;

uses
  KolibriOS, TinyGL;

const
  MaxObstacles = 10;
  RoadLength   = 100;
  GrassStep    = 5;
  CarLODZ      = -55.0;
  CullZ        = -116.0;
  FogStart     = 40.0;
  FogEnd       = 118.0;
  ComboTime    = 150;
  NearMissDist = 2.7;
  BrakeMult    = 0.45;
  SIGN: array[0..1] of GLFloat = (-1.0, 1.0);
  QB: array[0..71] of ShortInt = (
    -1,-1,1, 1,-1,1, 1,1,1, -1,1,1,  1,-1,-1, -1,-1,-1, -1,1,-1, 1,1,-1,
    -1,1,1, 1,1,1, 1,1,-1, -1,1,-1,  -1,-1,-1, 1,-1,-1, 1,-1,1, -1,-1,1,
    1,-1,1, 1,-1,-1, 1,1,-1, 1,1,1,  -1,-1,-1, -1,-1,1, -1,1,1, -1,1,-1);
  SEG_MASK: array[0..9] of Byte = ($77, $12, $5D, $5B, $3A, $6B, $6F, $52, $7F, $7B);
  SEG_DEF: array[0..6, 0..3] of GLFloat = (
    (0, 1, 2, 0.2), (-1, 0.5, 0.2, 1), (1, 0.5, 0.2, 1),
    (0, 0, 2, 0.2), (-1, -0.5, 0.2, 1), (1, -0.5, 0.2, 1), (0, -1, 2, 0.2));
  RAIN_POS: array[0..39, 0..2] of GLFloat = (
    (-3.1, 0.1, -4.5), (2.3, 0.8, -6.1), (-1.2, 0.3, -3.2), (3.5, 0.9, -7.8),
    (0.5, 0.4, -5.5), (-2.7, 0.6, -8.2), (1.1, 0.2, -4.9), (-3.2, 0.7, -3.8),
    (2.5, 0.5, -6.5), (-1.8, 0.0, -7.1), (3.4, 0.2, -5.0), (-0.9, 0.5, -4.1),
    (-3.8, 0.9, -8.5), (2.9, 0.3, -3.9), (-1.7, 0.8, -6.8), (3.1, 0.1, -7.5),
    (0.8, 0.6, -4.2), (-2.1, 0.4, -5.8), (1.6, 0.7, -8.1), (-3.9, 0.2, -3.5),
    (2.2, 0.0, -6.9), (-1.3, 0.5, -4.6), (3.8, 0.9, -7.2), (-0.4, 0.3, -5.3),
    (-3.5, 0.7, -3.1), (2.6, 0.1, -8.8), (-1.9, 0.6, -6.4), (2.8, 0.4, -4.7),
    (0.2, 0.8, -7.9), (-2.5, 0.2, -5.1), (1.9, 0.5, -3.6), (-3.6, 0.0, -8.4),
    (2.9, 0.3, -6.2), (-1.1, 0.9, -4.0), (3.1, 0.7, -7.6), (-0.7, 0.1, -5.9),
    (-3.2, 0.4, -8.7), (2.1, 0.8, -3.4), (-1.4, 0.2, -6.7), (3.4, 0.6, -5.4)
  );

type
  TCar = record
    x, z, r, g, b, TargetX, Tilt: GLFloat;
    IsTruck, IsSports, HasSiren, IsCoin, IsDonkey, IsVan, IsPickup, Passed: Boolean;
  end;
  TSkid = record
    x, z, alpha: GLFloat;
  end;
  TBeepProc = procedure(NoteBuf: Pointer) stdcall;
  TNoteBuf = packed record
    Duration: Byte;  
    Divider: Word;   
  end;

var
  GameState: record
    PlayerX, PlayerTilt, PlayerPitch, PlayerZ, DistanceTravelled, FloatScore, Speed, BaseSpeed, BoostMult, GlobalCurve: GLFloat;
    Score, FrameCount, CrashTimer, DustTimer, NextDonkeyScore: Integer;
    NextRainScore, RainEndScore, Combo, ComboTimer: Integer;
    GameActive, GameOver, Paused: Boolean;
  end;
  Obstacles: array[1..MaxObstacles] of TCar;
  Skids: array[0..39] of TSkid;
  SkidHead, BeepTimer: Integer;
  GlobalBeepBuf: array[0..1] of TNoteBuf; 
  CTX: TKOSGLContext;
  WndLeft, WndTop, WndWidth, WndHeight: LongInt;
  KeyLeft, KeyRight, KeyUp, KeyDown: Boolean;
  SkyR, SkyG, SkyB, FogAmt, NightAmt, WetAmt, DriftX: GLFloat;

// Детерминированный псевдослучай от номера тайла: объект не мигает при движении,
// потому что номер тайла в мире не меняется.
function Hash01(n: Integer): GLFloat;
var
  h: Integer;
begin
  h := n * 374761393 + 668265263;
  h := (h xor (h shr 13)) * 1274126177;
  h := h and $7FFFFFFF;
  Hash01 := (h mod 4096) / 4096.0;
end;

// Туман: 0 — цвет объекта как есть, 1 — объект полностью растворён в небе.
// Ставится один раз перед отрисовкой объекта, применяется внутри DrawBlockGL/DrawQuadGL.
procedure SetFog(z: GLFloat);
begin
  FogAmt := (-z - FogStart) / (FogEnd - FogStart);
  if FogAmt < 0.0 then FogAmt := 0.0
  else if FogAmt > 1.0 then FogAmt := 1.0;
end;

procedure FogColor3f(r, g, b: GLFloat);
begin
  if FogAmt > 0.0 then
    glColor3f(r + (SkyR - r) * FogAmt, g + (SkyG - g) * FogAmt, b + (SkyB - b) * FogAmt)
  else
    glColor3f(r, g, b);
end;

function IfThenF(Cond: Boolean; TrueVal, FalseVal: GLFloat): GLFloat;
begin
  if Cond then IfThenF := TrueVal else IfThenF := FalseVal;
end;

function Frac(Val: GLFloat): GLFloat;
begin
  Frac := Val - Trunc(Val);
end;

function GetRoadXOffset(z: GLFloat): GLFloat;
var
  dz: GLFloat;
begin
  dz := GameState.PlayerZ - z;
  if dz < 0.0 then dz := 0.0;
  GetRoadXOffset := dz * dz * GameState.GlobalCurve * 0.00015;
end;

procedure Beep(Freq, TimerTicks: Integer);
const
  _Beep: array[0..19] of Byte = (
    $56, $B8, $37, $00, $00, $00, $53, $89, $C3, $8B, $74, $24, $0C, $CD, $40, $5B, $5E, $C2, $04, $00
  );
var
  BeepProc: TBeepProc;
begin
  BeepTimer := TimerTicks; 

  if Freq <= 0 then begin
    GlobalBeepBuf[0].Duration := 0; 
    GlobalBeepBuf[0].Divider := 0;
  end else begin
    GlobalBeepBuf[0].Duration := 15; 
    GlobalBeepBuf[0].Divider := 1193180 div Freq; 
  end;
  
  GlobalBeepBuf[1].Duration := 0; 
  GlobalBeepBuf[1].Divider := 0;
  
  BeepProc := TBeepProc(@_Beep);
  BeepProc(@GlobalBeepBuf);
end;

procedure Perspective(fovY, Aspect, zNear, zFar: GLDouble);
var
  fW, fH, fovYPI360: GLDouble;
  M: array[0..15] of GLFloat;
begin
  FillChar(M, SizeOf(M), #0);
  fovYPI360 := fovY * PI / 360;
  fH := Sin(fovYPI360) * zNear / Cos(fovYPI360);
  fW := fH * Aspect;
  M[0] := zNear / fW;  M[5] := zNear / fH;
  M[10] := -(zFar + zNear) / (zFar - zNear);  M[11] := -1;
  M[14] := -2 * zFar * zNear / (zFar - zNear);
  glLoadMatrixf(@M[0]);
end;

procedure DrawBlockGL(w, h, l, r, g, b: GLFloat);
var
  i: Integer;
begin
  FogColor3f(r, g, b);
  glBegin(GL_QUADS);
  for i := 0 to 23 do
    glVertex3f(w * 0.5 * QB[i*3], h * 0.5 * QB[i*3+1], l * 0.5 * QB[i*3+2]);
  glEnd();
end;

// Плоская горизонтальная площадка: 4 вершины вместо 24 у DrawBlockGL.
// Порядок вершин совпадает с верхней гранью куба (QB), чтобы работал GL_CULL_FACE.
procedure DrawQuadGL(w, l, r, g, b: GLFloat);
var
  xw, zl: GLFloat;
begin
  xw := w * 0.5; zl := l * 0.5;
  FogColor3f(r, g, b);
  glBegin(GL_QUADS);
    glVertex3f(-xw, 0.0, zl); glVertex3f(xw, 0.0, zl);
    glVertex3f(xw, 0.0, -zl); glVertex3f(-xw, 0.0, -zl);
  glEnd();
end;

// Плоская вертикальная площадка в плоскости XY (обход граней как у передней грани куба).
procedure DrawVQuadGL(w, h, r, g, b: GLFloat);
var
  xw, yh: GLFloat;
begin
  xw := w * 0.5; yh := h * 0.5;
  FogColor3f(r, g, b);
  glBegin(GL_QUADS);
    glVertex3f(-xw, -yh, 0.0); glVertex3f(xw, -yh, 0.0);
    glVertex3f(xw, yh, 0.0);   glVertex3f(-xw, yh, 0.0);
  glEnd();
end;

procedure DrawDonkeyGL(x, y, z: GLFloat);
begin
  glPushMatrix();
  glTranslatef(x, y, z);
  
  glPushMatrix();
  glTranslatef(0.0, -y - 1.18, 0.0);
  DrawQuadGL(0.8, 1.4, 0.15, 0.15, 0.15);
  glPopMatrix();

  glPushMatrix(); glTranslatef(-0.25, -0.05, -0.4); DrawBlockGL(0.15, 0.4, 0.15, 0.2, 0.2, 0.2); glPopMatrix();
  glPushMatrix(); glTranslatef(0.25, -0.05, -0.4); DrawBlockGL(0.15, 0.4, 0.15, 0.2, 0.2, 0.2); glPopMatrix();
  glPushMatrix(); glTranslatef(-0.25, -0.05, 0.4); DrawBlockGL(0.15, 0.4, 0.15, 0.2, 0.2, 0.2); glPopMatrix();
  glPushMatrix(); glTranslatef(0.25, -0.05, 0.4); DrawBlockGL(0.15, 0.4, 0.15, 0.2, 0.2, 0.2); glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.25, 0.0);
  DrawBlockGL(0.7, 0.5, 1.2, 0.25, 0.25, 0.25);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.55, -0.45);
  glRotatef(-20.0, 1.0, 0.0, 0.0);
  DrawBlockGL(0.3, 0.5, 0.3, 0.3, 0.3, 0.3);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.8, -0.65);
  DrawBlockGL(0.25, 0.3, 0.5, 0.35, 0.35, 0.35);
  
  glTranslatef(-0.15, 0.2, 0.15);
  DrawBlockGL(0.1, 0.3, 0.1, 0.2, 0.2, 0.2);
  glTranslatef(0.3, 0.0, 0.0);
  DrawBlockGL(0.1, 0.3, 0.1, 0.2, 0.2, 0.2);
  glPopMatrix();

  glPushMatrix();
  glTranslatef(0.0, 0.3, 0.6);
  glRotatef(20.0, 1.0, 0.0, 0.0);
  DrawBlockGL(0.05, 0.4, 0.05, 0.15, 0.15, 0.15);
  glPopMatrix();

  glPopMatrix();
end;

procedure DrawCabinGL(w, h, l, r, g, b: GLFloat);
var
  xw, yh, zl, ind: GLFloat;
begin
  xw := w * 0.5; yh := h * 0.5; zl := l * 0.5; ind := 0.2;
  glBegin(GL_QUADS);
    glColor3f(0.62, 0.84, 0.93);
    glVertex3f(-xw, -yh, zl); glVertex3f(xw, -yh, zl); glVertex3f(xw-ind, yh, zl-ind); glVertex3f(-xw+ind, yh, zl-ind);
    glVertex3f(xw-ind, yh, -zl+ind); glVertex3f(-xw+ind, yh, -zl+ind); glVertex3f(-xw, -yh, -zl); glVertex3f(xw, -yh, -zl);
    glColor3f(r, g, b);
    glVertex3f(-xw+ind, yh, zl-ind); glVertex3f(xw-ind, yh, zl-ind); glVertex3f(xw-ind, yh, -zl+ind); glVertex3f(-xw+ind, yh, -zl+ind);
  glEnd();
  glBegin(GL_TRIANGLES);
    glColor3f(0.50, 0.78, 0.87);
    glVertex3f(-xw, -yh, -zl); glVertex3f(-xw, -yh, zl); glVertex3f(-xw+ind, yh, zl-ind);
    glVertex3f(-xw, -yh, -zl); glVertex3f(-xw+ind, yh, zl-ind); glVertex3f(-xw+ind, yh, -zl+ind);
    glVertex3f(xw, -yh, zl); glVertex3f(xw, -yh, -zl); glVertex3f(xw-ind, yh, -zl+ind);
    glVertex3f(xw, -yh, zl); glVertex3f(xw-ind, yh, -zl+ind); glVertex3f(-xw+ind, yh, zl-ind);
  glEnd();
end;

procedure DrawDetailedWheel(xOffset, yOffset, zOffset: GLFloat);
begin
  glPushMatrix();
  glTranslatef(xOffset, yOffset, zOffset);
  DrawBlockGL(0.16, 0.32, 0.52, 0.08, 0.08, 0.08);
  if xOffset > 0 then glTranslatef(0.015, 0.0, 0.0) else glTranslatef(-0.015, 0.0, 0.0);
  DrawBlockGL(0.14, 0.16, 0.26, 0.75, 0.75, 0.75);
  glPopMatrix();
end;

procedure DrawCarGL(x, y, z, r, g, b, tilt, pitch: GLFloat; IsPlayer, IsTruck, IsSports, HasSiren, IsCoin, IsVan, IsPickup: Boolean);
var
  i, j: Integer;
  lr, lg, lb, pProgress, fc: GLFloat;
  s30, c30, s35, s40, c40, s45, s50, s60, s70: GLFloat;
  FarLOD: Boolean;
begin
  FarLOD := z < CarLODZ;
  glPushMatrix();
  glTranslatef(x, y, z);

  if IsCoin then begin
    glPushMatrix();
    glTranslatef(0.0, 0.6 + Sin(GameState.FrameCount * 0.1) * 0.2, 0.0);
    glRotatef(GameState.FrameCount * 6.0, 0.0, 1.0, 0.0);
    DrawBlockGL(0.8, 0.8, 0.15, r, g, b); 
    glTranslatef(0.0, 0.0, 0.05);
    DrawBlockGL(0.4, 0.4, 0.2, 1.0, 1.0, 0.7); 
    glPopMatrix();
    glPopMatrix();
    Exit;
  end;

  glPushMatrix();
  glTranslatef(0.0, -y - 1.18, 0.0);
  DrawQuadGL(1.8, 2.7, 0.15, 0.15, 0.15);
  glPopMatrix();

  glRotatef(tilt, 0.0, 0.0, -1.0);
  glRotatef(tilt * 0.4, 0.0, -1.0, 0.0);
  glRotatef(pitch, 1.0, 0.0, 0.0);

  glPushMatrix();
  glTranslatef(0.0, -0.05, 0.0);
  DrawBlockGL(1.76, 0.15, 2.625, 0.2, 0.2, 0.2);
  glPopMatrix();

  glPushMatrix();
  if IsSports then begin
    glTranslatef(0.0, 0.02, 0.0);
    DrawBlockGL(1.72, 0.25, 2.5, r, g, b);
  end else begin
    glTranslatef(0.0, 0.1, 0.0);
    DrawBlockGL(1.68, 0.4, 2.5, r, g, b);
  end;
  glPopMatrix();

  // Дальние машины: без колёс, молдингов и зеркал — 12 блоков экономии на каждую.
  if not FarLOD then
  for i := 0 to 1 do begin
    glPushMatrix();
    glTranslatef(SIGN[i] * 0.842, IfThenF(IsSports, 0.08, 0.15), 0.0);
    DrawBlockGL(0.01, IfThenF(IsSports, 0.18, 0.25), 1.0, r * 0.85, g * 0.85, b * 0.85);
    glTranslatef(-SIGN[i] * 0.005, 0.02, 0.375);
    DrawBlockGL(0.012, 0.03, 0.08, 0.85, 0.85, 0.85);
    glPopMatrix();

    glPushMatrix();
    if IsPlayer then glTranslatef(0.0, 0.22, -0.625) else glTranslatef(0.0, 0.22, 0.625);
    glTranslatef(SIGN[i] * 0.896, IfThenF(IsSports, -0.08, 0.0), 0.0);
    DrawBlockGL(0.08, 0.06, 0.06, 0.15, 0.15, 0.15);
    glPopMatrix();

    for j := 0 to 1 do DrawDetailedWheel(SIGN[i] * 0.875, -0.1, SIGN[j] * 0.8333);
  end;

  if IsPlayer and (GameState.DustTimer > 0) then begin
    pProgress := (20.0 - GameState.DustTimer) / 20.0;
    for i := 0 to 1 do begin
      for j := 0 to 2 do begin
        glPushMatrix();
        glTranslatef(
          SIGN[i] * 0.4 + Sin(GameState.FrameCount * 0.6 + j) * 0.05,
          -0.1 + pProgress * 0.5 + j * 0.1,
          1.3 + pProgress * 1.5 + j * 0.2
        );
        DrawBlockGL(
          0.15 + pProgress * 0.15, 0.15 + pProgress * 0.15, 0.15 + pProgress * 0.15,
          0.35, 0.35, 0.35
        );
        glPopMatrix();
      end;
    end;
  end;

  glPushMatrix();
  if IsTruck then begin
    glTranslatef(0.0, 0.55, 0.5);
    DrawCabinGL(1.568, 0.9, 1.25, r, g, b);
    glTranslatef(0.0, -0.15, -1.5);
    DrawBlockGL(1.52, 0.7, 1.5, 0.4, 0.25, 0.15);
  end else if IsSports then begin
    glTranslatef(0.0, 0.22, -0.2);
    DrawCabinGL(1.4, 0.35, 1.2, r, g, b);
    glTranslatef(0.0, 0.15, -1.0);
    DrawBlockGL(1.6, 0.08, 0.25, 0.1, 0.1, 0.1);
  end else if IsVan then begin
    glTranslatef(0.0, 0.45, 0.0);
    DrawCabinGL(1.52, 0.7, 2.3, r, g, b);
  end else if IsPickup then begin
    glTranslatef(0.0, 0.35, 0.4);
    DrawCabinGL(1.52, 0.5, 1.0, r, g, b);
    glTranslatef(0.0, -0.15, -1.2);
    DrawBlockGL(1.52, 0.2, 1.2, r * 0.8, g * 0.8, b * 0.8);
  end else begin
    glTranslatef(0.0, 0.35, 0.0);
    DrawCabinGL(1.52, 0.5, 1.5, r, g, b);
  end;
  glPopMatrix();

  if not FarLOD then begin
    glPushMatrix();
    if IsPlayer then glTranslatef(0.0, 0.12, -1.26) else glTranslatef(0.0, 0.12, 1.26);
    DrawBlockGL(0.96, 0.15, 0.01, 0.05, 0.05, 0.05);
    glPopMatrix();
  end;

  if IsPlayer and (GameState.CrashTimer > 0) then begin
    // Тригонометрия считается один раз на кадр, а не 9 раз внутри отрисовки.
    fc := GameState.FrameCount;
    s30 := Sin(fc * 0.3);   c30 := Cos(fc * 0.3);
    s35 := Sin(fc * 0.35);
    s40 := Sin(fc * 0.4);   c40 := Cos(fc * 0.4);
    s45 := Sin(fc * 0.45);
    s50 := Sin(fc * 0.5);
    s60 := Sin(fc * 0.6);
    s70 := Sin(fc * 0.7);

    glPushMatrix();
    glTranslatef(0.0, 0.3, -0.8);
    glPushMatrix();
    glTranslatef(0.0, 0.4 + s40 * 0.1, 0.0);
    DrawBlockGL(0.7, 0.8 + s50 * 0.2, 0.7, 1.0, 0.95, 0.2);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.15, 0.6 + c30 * 0.25, -0.1);
    DrawBlockGL(0.85, 1.2 + s35 * 0.3, 0.85, 1.0, 0.4, 0.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(-0.2, 0.5 + s45 * 0.2, 0.15);
    DrawBlockGL(0.75, 1.0 + c40 * 0.3, 0.75, 0.9, 0.2, 0.0);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(s60 * 0.1, 1.2 + s50 * 0.3, 0.0);
    DrawBlockGL(0.5, 0.7 + s70 * 0.2, 0.5, 1.0, 0.7, 0.0);
    glPopMatrix();

    for i := 0 to 3 do begin
      glPushMatrix();
      glTranslatef(Sin(GameState.FrameCount * 0.3 + i * 1.5) * 0.4,
        1.2 + Frac(GameState.FrameCount * 0.08 + i * 0.25) * 1.8,
        Cos(GameState.FrameCount * 0.3 + i * 2.0) * 0.3);
      DrawBlockGL(0.12, 0.15, 0.12, 1.0, 0.85, 0.1);
      glPopMatrix();
    end;
    for i := 0 to 2 do begin
      glPushMatrix();
      glTranslatef(Sin(GameState.FrameCount * 0.15 + i * 2.1) * 0.3,
        1.8 + Frac(GameState.FrameCount * 0.04 + i * 0.33) * 2.2,
        Cos(GameState.FrameCount * 0.15 + i * 1.7) * 0.3);
      DrawBlockGL(0.7 + i * 0.2, 0.6 + i * 0.2, 0.7 + i * 0.2, 0.25, 0.25, 0.25);
      glPopMatrix();
    end;
    glPopMatrix();
  end;

  if (not IsPlayer) and HasSiren then begin
    glPushMatrix();
    if IsTruck then glTranslatef(0.0, 1.05, 0.5)
    else if IsSports then glTranslatef(0.0, 0.45, -0.2)
    else if IsVan then glTranslatef(0.0, 0.85, 0.0)
    else if IsPickup then glTranslatef(0.0, 0.65, 0.4)
    else glTranslatef(0.0, 0.62, 0.0);
    
    if ((GameState.FrameCount div 8) mod 2) = 0 then begin
      DrawBlockGL(0.15, 0.08, 0.1, 1.0, 0.0, 0.0);
      glTranslatef(0.16, 0.0, 0.0);
      DrawBlockGL(0.15, 0.08, 0.1, 0.0, 0.0, 1.0);
    end else begin
      DrawBlockGL(0.15, 0.08, 0.1, 0.0, 0.0, 1.0);
      glTranslatef(0.16, 0.0, 0.0);
      DrawBlockGL(0.15, 0.08, 0.1, 1.0, 0.0, 0.0);
    end;
    glPopMatrix();
  end;

  for j := 0 to 1 do begin
    if (IsPlayer = (j = 1)) then begin
       if IsPlayer and (pitch > 2.0) then begin 
          lr := 1.0; lg := 0.1; lb := 0.1; 
       end else begin
          lr := 0.6; lg := 0.0; lb := 0.0; 
       end;
    end else begin 
       lr := 1.0; lg := 1.0; lb := 0.2; 
    end;
    for i := 0 to 1 do begin
      glPushMatrix();
      glTranslatef(SIGN[i] * 0.5333, 0.12, SIGN[j] * 1.26);
      if IsPlayer and (pitch > 2.0) and (j = 1) then
         DrawBlockGL(0.24, 0.14, 0.04, lr, lg, lb)
      else
         DrawBlockGL(0.2, 0.1, 0.02, lr, lg, lb);
      glPopMatrix();
    end;
  end;
  glPopMatrix();
end;

procedure DrawCelestialBodies(cycleDist: Integer);
var
  sunY, moonY, p: GLFloat;
const
  HIGH_Y = 55.0;
  LOW_Y = -20.0;
begin
  if (cycleDist < 350) or (cycleDist >= 950) then begin
    sunY := HIGH_Y;
    moonY := LOW_Y;
  end
  else if (cycleDist >= 350) and (cycleDist < 400) then begin
    p := (cycleDist - 350) / 50.0;
    sunY := HIGH_Y + p * (LOW_Y - HIGH_Y);
    moonY := LOW_Y + p * (HIGH_Y - LOW_Y);
  end
  else if (cycleDist >= 400) and (cycleDist < 900) then begin
    sunY := LOW_Y;
    moonY := HIGH_Y;
  end
  else begin 
    p := (cycleDist - 900) / 50.0;
    sunY := LOW_Y + p * (HIGH_Y - LOW_Y);
    moonY := HIGH_Y + p * (LOW_Y - HIGH_Y);
  end;

  glDisable(GL_DEPTH_TEST);
  FogAmt := 0.0;
  glPushMatrix();

  if sunY > LOW_Y then begin
    glPushMatrix();
    glTranslatef(12.0, sunY, -80.0);
    DrawBlockGL(4.0, 4.0, 0.1, 1.0, 0.9, 0.1); 
    glPopMatrix();
  end;

  if moonY > LOW_Y then begin
    glPushMatrix();
    glTranslatef(-12.0, moonY, -80.0);
    DrawBlockGL(3.5, 3.5, 0.1, 0.9, 0.9, 0.85); 
    glPopMatrix();
  end;

  glPopMatrix();
  glEnable(GL_DEPTH_TEST);
end;

procedure DrawDigit(digit: Integer; x, y, z, scale: GLFloat);
var
  i: Integer;
  sx, sy: GLFloat;
begin
  if (digit < 0) or (digit > 9) then Exit;
  sx := scale * 0.3; sy := scale * 0.5;
  for i := 0 to 6 do
    if (SEG_MASK[digit] and (1 shl (6 - i))) <> 0 then begin
      glPushMatrix();
      glTranslatef(x + sx * SEG_DEF[i,0], y + sy * SEG_DEF[i,1], z);
      DrawBlockGL(sx * SEG_DEF[i,2], sy * SEG_DEF[i,3], 0.05, 1.0, 0.85, 0.1);
      glPopMatrix();
    end;
end;

procedure DrawNumber(num: Integer; x, y, z, scale: GLFloat);
var
  d: array[0..9] of Integer;
  c, i: Integer;
begin
  if num = 0 then begin
    DrawDigit(0, x, y, z, scale);
    Exit;
  end;
  c := 0;
  while num > 0 do begin
    d[c] := num mod 10;
    num := num div 10;
    Inc(c);
  end;
  x := x - (c * scale * 0.95) * 0.5;
  for i := c - 1 downto 0 do begin
    DrawDigit(d[i], x, y, z, scale);
    x := x + scale * 0.95;
  end;
end;

// Звёзды дальше гряды (-150 против -128) и рисуются с включённым тестом глубины,
// поэтому силуэт города их закрывает, а не наоборот.
procedure DrawStars;
var
  i: Integer;
begin
  glColor3f(1.0, 1.0, 0.9);
  glBegin(GL_POINTS);
  for i := 0 to 39 do
    glVertex3f((Hash01(i * 17 + 2) - 0.5) * 260.0, 2.0 + Hash01(i * 23 + 9) * 28.0, -150.0);
  glEnd();
end;

// Силуэт гряды на горизонте, за концом полотна. Не скроллится по Z вообще —
// только смещается по X вместе с дальним концом дороги, что и даёт параллакс.
procedure DrawBackdrop;
var
  i: Integer;
  hgt, wid, xs, shift, currentZ: GLFloat;
begin
  if NightAmt > 0.5 then DrawStars;

  // Дальний грунт. Полосы травы вдоль дороги узкие (до |x| = 33), а на расстоянии
  // 120 в кадр попадает больше 60 единиц в каждую сторону — по бокам открывалась
  // дыра до самого неба. Пять широких полос закрывают её, у каждой свой туман,
  // поэтому переход к настоящей траве не виден.
  for i := 0 to 4 do begin
    currentZ := -50.0 - i * 20.0;
    SetFog(currentZ);
    glPushMatrix();
    glTranslatef(GetRoadXOffset(currentZ), -1.23, currentZ);
    DrawQuadGL(340.0, 21.0, 0.2, 0.5, 0.2);
    glPopMatrix();
  end;

  shift := GetRoadXOffset(-128.0);
  FogAmt := 0.62;
  for i := 0 to 21 do begin
    hgt := 6.0 + Hash01(i * 13 + 5) * 16.0;
    wid := 12.0 + Hash01(i * 29 + 3) * 14.0;
    xs := (i - 10.5) * 9.0 + Hash01(i * 7 + 1) * 4.0;
    glPushMatrix();
    glTranslatef(xs + shift, -1.2 + hgt * 0.5, -128.0);
    DrawVQuadGL(wid, hgt, 0.30, 0.34, 0.42);
    glPopMatrix();
  end;
end;

// Крупные ориентиры за лесом. Попадаются редко, поэтому детализация минимальная.
// Матрица уже сдвинута в основание объекта на уровне земли.
procedure DrawLandmark(kind: Integer);
var
  i, j: Integer;
begin
  if kind = 0 then begin              // ветряк
    glPushMatrix();
    glTranslatef(0.0, 4.0, 0.0);
    DrawBlockGL(0.7, 8.0, 0.7, 0.85, 0.85, 0.8);
    glPopMatrix();
    for i := 0 to 2 do begin
      glPushMatrix();
      glTranslatef(0.0, 8.0, 0.6);
      glRotatef(GameState.FrameCount * 1.4 + i * 120.0, 0.0, 0.0, 1.0);
      glTranslatef(0.0, 1.9, 0.0);
      DrawBlockGL(0.3, 3.8, 0.12, 0.95, 0.95, 0.9);
      glPopMatrix();
    end;
  end else if kind = 1 then begin     // вышка ЛЭП
    for i := 0 to 1 do begin
      glPushMatrix();
      glTranslatef(SIGN[i] * 0.9, 3.6, 0.0);
      DrawBlockGL(0.25, 7.2, 0.25, 0.45, 0.45, 0.5);
      glPopMatrix();
    end;
    glPushMatrix();
    glTranslatef(0.0, 5.3, 0.0);
    DrawBlockGL(3.8, 0.22, 0.22, 0.45, 0.45, 0.5);
    glTranslatef(0.0, 1.3, 0.0);
    DrawBlockGL(2.8, 0.22, 0.22, 0.45, 0.45, 0.5);
    glPopMatrix();
  end else if kind = 2 then begin     // амбар
    glPushMatrix();
    glTranslatef(0.0, 1.6, 0.0);
    DrawBlockGL(6.0, 3.2, 4.5, 0.55, 0.18, 0.14);
    glTranslatef(0.0, 2.0, 0.0);
    DrawBlockGL(6.4, 0.9, 4.8, 0.3, 0.3, 0.32);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(0.0, 0.9, 2.3);
    // Ночью в окне горит свет.
    if NightAmt > 0.5 then DrawBlockGL(1.5, 1.7, 0.08, 1.0, 0.9, 0.45)
    else DrawBlockGL(1.5, 1.7, 0.08, 0.35, 0.2, 0.12);
    glPopMatrix();
  end else begin                      // водонапорная башня
    for i := 0 to 1 do
      for j := 0 to 1 do begin
        glPushMatrix();
        glTranslatef(SIGN[i] * 1.1, 2.1, SIGN[j] * 1.1);
        DrawBlockGL(0.28, 4.2, 0.28, 0.5, 0.52, 0.5);
        glPopMatrix();
      end;
    glPushMatrix();
    glTranslatef(0.0, 5.3, 0.0);
    DrawBlockGL(3.4, 2.2, 3.4, 0.6, 0.62, 0.6);
    glTranslatef(0.0, 1.5, 0.0);
    DrawBlockGL(3.0, 0.8, 3.0, 0.45, 0.5, 0.5);
    glPopMatrix();
  end;
end;

procedure DrawRoadGL;
var
  z, i, tileBase, t: Integer;
  currentZ, actualZ, curveX, blockSize, tileOffset, dOverB: GLFloat;
  th, tg, tdx, road, mark: GLFloat;
  IsEven: Boolean;
begin
  blockSize := 1.2;
  // Инвариант цикла: раньше это деление с Trunc считалось 4 раза на каждый тайл.
  dOverB := GameState.DistanceTravelled / blockSize;
  tileBase := Trunc(dOverB);
  tileOffset := (dOverB - tileBase) * blockSize;
  glPushMatrix();
  glTranslatef(0.0, 0.0, tileOffset);

  // Трава рисуется длинными полосами по GrassStep тайлов, а не потайлово.
  // Полоса на 2 единицы шире и на 0.01 ниже бордюра, чтобы стык не расходился
  // на кривой и не было z-fighting.
  for z := 0 to (RoadLength div GrassStep) - 1 do begin
    currentZ := -(z * GrassStep + (GrassStep - 1) * 0.5) * blockSize;
    curveX := GetRoadXOffset(currentZ + tileOffset);
    SetFog(currentZ);
    for i := 0 to 1 do begin
      glPushMatrix();
      glTranslatef(curveX + SIGN[i] * 20.0, -1.21, currentZ);
      DrawQuadGL(26.0, blockSize * GrassStep, 0.2, 0.5, 0.2);
      glPopMatrix();
      // Отбойник: сплошной поручень такими же длинными кусками, как трава.
      glPushMatrix();
      glTranslatef(curveX + SIGN[i] * 8.6, -0.72, currentZ);
      DrawBlockGL(0.1, 0.34, blockSize * GrassStep, 0.72, 0.72, 0.75);
      glPopMatrix();
    end;
  end;

  for z := 0 to RoadLength - 1 do begin
    currentZ := -z * blockSize;
    actualZ := currentZ + tileOffset;
    curveX := GetRoadXOffset(actualZ);
    SetFog(currentZ);
    t := z + tileBase;
    IsEven := (t div 3) mod 2 = 0;

    for i := 0 to 1 do begin
      glPushMatrix();
      glTranslatef(curveX + SIGN[i] * 7.3, -1.2, currentZ);
      DrawQuadGL(1.4, blockSize, 0.4, 0.4, 0.4);
      glTranslatef(-SIGN[i] * 0.9, 0.0, 0.0);
      DrawQuadGL(0.9, 1.2, 0.925, 0.925, 0.925);
      glPopMatrix();
    end;

    // Столбики отбойника — реже поручня, чтобы не утяжелять кадр.
    if (t mod 4) = 0 then
      for i := 0 to 1 do begin
        glPushMatrix();
        glTranslatef(curveX + SIGN[i] * 8.6, -1.0, currentZ);
        DrawBlockGL(0.14, 0.8, 0.14, 0.5, 0.5, 0.53);
        glPopMatrix();
      end;

    // Деревья: нерегулярный шаг и разные высота, оттенок и отступ от дороги —
    // всё из Hash01 от номера тайла, поэтому лес не «дышит» при движении.
    if (t mod 3) = 0 then
      for i := 0 to 1 do
        if Hash01(t * 7 + i * 131) > 0.45 then begin
          th := 0.7 + Hash01(t * 11 + i * 17) * 0.8;
          tg := 0.45 + Hash01(t * 13 + i * 41) * 0.28;
          tdx := Hash01(t * 5 + i * 23) * 4.5;
          glPushMatrix();
          glTranslatef(curveX + SIGN[i] * (13.5 + tdx), -1.2 + 0.8 * th, currentZ);
          DrawBlockGL(0.4, 1.6 * th, 0.4, 0.4, 0.25, 0.15);
          glTranslatef(0.0, 0.9 * th, 0.0);
          DrawBlockGL(1.8 * th, 1.0 * th, 1.8 * th, tg * 0.22, tg, tg * 0.36);
          glTranslatef(0.0, 0.7 * th, 0.0);
          DrawBlockGL(1.2 * th, 0.9 * th, 1.2 * th, tg * 0.3, tg * 1.18, tg * 0.42);
          glPopMatrix();
        end;

    // Крупные ориентиры далеко за лесом.
    if (t mod 40) = 0 then begin
      i := Trunc(Hash01(t * 3 + 71) * 1.99);
      glPushMatrix();
      glTranslatef(curveX + SIGN[i] * (30.0 + Hash01(t * 19) * 8.0), -1.2, currentZ);
      DrawLandmark(Trunc(Hash01(t * 29 + 13) * 3.99));
      glPopMatrix();
    end;

    // Рекламный щит.
    if (t mod 78) = 9 then begin
      i := Trunc(Hash01(t * 37 + 5) * 1.99);
      glPushMatrix();
      glTranslatef(curveX + SIGN[i] * 11.0, -1.2, currentZ);
      glTranslatef(0.0, 1.1, 0.0);
      DrawBlockGL(0.22, 2.2, 0.22, 0.35, 0.3, 0.25);
      glTranslatef(0.0, 1.6, 0.0);
      DrawBlockGL(3.0, 1.8, 0.12, 0.15, 0.35, 0.6);
      glPopMatrix();
    end;

    // Фонарные столбы.
    if (t mod 18) = 4 then
      for i := 0 to 1 do begin
        glPushMatrix();
        glTranslatef(curveX + SIGN[i] * 11.5, -1.2, currentZ);
        glTranslatef(0.0, 2.5, 0.0);
        DrawBlockGL(0.2, 5.0, 0.2, 0.4, 0.4, 0.42);
        glTranslatef(-SIGN[i] * 1.2, 2.4, 0.0);
        DrawBlockGL(2.4, 0.2, 0.2, 0.4, 0.4, 0.42);
        glTranslatef(-SIGN[i] * 1.0, -0.15, 0.0);
        DrawBlockGL(0.6, 0.15, 0.4, 1.0, 0.95, 0.6);
        glPopMatrix();
      end;

    // Мокрый асфальт темнее сухого.
    road := IfThenF(IsEven, 0.45, 0.65) * (1.0 - WetAmt * 0.32);
    mark := 0.9 * (1.0 - WetAmt * 0.18);

    glPushMatrix();
    glTranslatef(curveX, -1.2, currentZ);
    DrawQuadGL(11.8, blockSize, road, road, road * 1.04);
    glPopMatrix();

    if IsEven then begin
      glPushMatrix();
      glTranslatef(curveX, -1.19, currentZ);
      DrawQuadGL(0.12, blockSize, mark, mark, mark);
      glPopMatrix();
    end;

  end;
  glPopMatrix();

  for i := 0 to 39 do begin
    if Skids[i].alpha > 0.0 then begin
      curveX := GetRoadXOffset(Skids[i].z);
      SetFog(Skids[i].z);
      glPushMatrix();
      glTranslatef(curveX + Skids[i].x, -1.18, Skids[i].z);
      DrawQuadGL(0.8, 0.5, 0.35, 0.35, 0.35);
      glPopMatrix();
      if not GameState.Paused then begin
        Skids[i].z := Skids[i].z + GameState.Speed;
        Skids[i].alpha := Skids[i].alpha - 0.02;
      end;
    end;
  end;
end;

procedure SpawnObstacle(var C: TCar; ZPos: GLFloat);
var
  rType: GLFloat;
begin
  C.x := (Random * 9.0) - 4.5;
  C.TargetX := C.x;
  C.Tilt := 0.0;
  C.z := ZPos;
  C.IsDonkey := False;
  C.IsCoin := False;
  C.Passed := False;

  if GameState.Score >= GameState.NextDonkeyScore then begin
    C.IsDonkey := True;
    GameState.NextDonkeyScore := GameState.NextDonkeyScore + 1500;
  end else begin
    rType := Random;
    C.IsCoin := rType > 0.90; 
    if C.IsCoin then begin
      C.r := 1.0; C.g := 0.85; C.b := 0.1;
    end else begin
      C.IsTruck := False; C.IsSports := False; C.IsVan := False; C.IsPickup := False;
      
      if rType > 0.75 then C.IsTruck := True
      else if rType > 0.60 then C.IsSports := True
      else if rType > 0.45 then C.IsVan := True
      else if rType > 0.30 then C.IsPickup := True;

      C.HasSiren := Random < 0.15;
      
      if C.HasSiren then begin
        C.r := 0.1; C.g := 0.3; C.b := 1.0; 
      end else begin
        C.r := (Random * 0.7) + 0.3; C.g := (Random * 0.7) + 0.3; C.b := (Random * 0.7) + 0.3;
      end;
    end;
  end;
end;

procedure InitObstacles;
var
  i: Integer;
begin
  for i := 1 to MaxObstacles do SpawnObstacle(Obstacles[i], -180.0 + (i * 20.0));
  for i := 0 to 39 do Skids[i].alpha := 0.0;
end;

// UIScale = 1 / (2 * tan(45/2)) — прежний вид интерфейса при FOV 45 и z = -2.
procedure DrawGameUI(Aspect: GLFloat);
const
  UIScale = 1.2071;
  BarW    = 0.42;
var
  i, c, n: Integer;
  fill, br, bg, bb, leftX, margin: GLFloat;
begin
  // Интерфейс рисуется в ортографии: раньше проекция оставалась перспективной,
  // поэтому размер цифр менялся вместе с FOV при бусте.
  glMatrixMode(GL_PROJECTION); glPushMatrix(); glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);  glPushMatrix();
  glLoadIdentity();
  glScalef(UIScale / Aspect, UIScale, 1.0);
  glDisable(GL_DEPTH_TEST);
  FogAmt := 0.0;

  // Левый отступ делается равным верхнему: 1/UIScale — это верхний край экрана
  // в наших единицах, Aspect/UIScale — правый, так что отступ считается один раз
  // и не зависит от пропорций окна.
  margin := 1.0 / UIScale - 0.785;
  leftX := -Aspect / UIScale + margin;

  // Счёт прижимается тем же краем. DrawNumber центрирует число, поэтому центр
  // приходится считать от количества цифр: полуширина блока плюс вылет крайнего
  // сегмента (0.33 от кегля).
  c := 1; n := GameState.Score;
  while n >= 10 do begin n := n div 10; Inc(c); end;
  DrawNumber(GameState.Score, leftX + c * 0.12 * 0.475 + 0.12 * 0.33, 0.55, -0.5, 0.12);

  // Полоса скорости в левом верхнем углу: зелёная на бусте, красная на тормозе.
  fill := (GameState.BoostMult - BrakeMult) / (3.5 - BrakeMult);
  if fill < 0.0 then fill := 0.0 else if fill > 1.0 then fill := 1.0;
  glPushMatrix();
  glTranslatef(leftX + (BarW + 0.03) * 0.5, 0.74, -0.5);
  DrawBlockGL(BarW + 0.03, 0.09, 0.01, 0.1, 0.1, 0.12);
  glPopMatrix();
  if GameState.BoostMult > 1.01 then begin
    br := 0.2; bg := 1.0; bb := 0.3;
  end else if GameState.BoostMult < 0.99 then begin
    br := 1.0; bg := 0.25; bb := 0.15;
  end else begin
    br := 0.95; bg := 0.85; bb := 0.2;
  end;
  if fill > 0.01 then begin
    glPushMatrix();
    glTranslatef(leftX + 0.015 + BarW * fill * 0.5, 0.74, -0.51);
    DrawBlockGL(BarW * fill, 0.06, 0.01, br, bg, bb);
    glPopMatrix();
  end;

  // Множитель комбо появляется правее счёта, пока идёт серия монет.
  if GameState.Combo > 1 then
    DrawNumber(GameState.Combo, leftX + c * 0.114 + 0.16, 0.55, -0.5, 0.09);

  if GameState.Paused then
    for i := 0 to 1 do begin
      glPushMatrix();
      glTranslatef(SIGN[i] * 0.09, 0.0, -0.6);
      DrawBlockGL(0.09, 0.34, 0.01, 0.95, 0.95, 0.95);
      glPopMatrix();
    end;

  if GameState.GameOver then begin
    DrawBlockGL(1.4, 0.4, 0, 0.8, 0.0, 0.0);
    // z отрицательный: в ортографии число должно быть ближе к камере, чем плашка.
    DrawNumber(GameState.Score, 0.0, 0.0, -0.4, 0.25);
  end;

  glEnable(GL_DEPTH_TEST);
  glPopMatrix(); glMatrixMode(GL_PROJECTION); glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
end;

procedure SetKeyboardInputMode(Mode: TKeyboardInputMode);
type
  TSetModeProc = procedure(ModeVal: Integer) stdcall;
const
  _SetMode: array[0..20] of Byte = (
    $53,                     
    $B8, $42, $00, $00, $00, 
    $BB, $01, $00, $00, $00, 
    $8B, $4C, $24, $08,      
    $CD, $40,                
    $5B,                     
    $C2, $04, $00            
  );
var
  SetModeProc: TSetModeProc;
begin
  SetModeProc := TSetModeProc(@_SetMode);
  SetModeProc(Ord(Mode));
end;

procedure GLDraw;
var
  ThreadInfo: TThreadInfo;
  i, cycleDist: Integer;
  rSky, gSky, bSky, rainY, rainX, rainR, rainG, rainB, curveX, Aspect, shake: GLFloat;
  Upd: Boolean;
begin
  GetThreadInfo($FFFFFFFF, ThreadInfo);
  if ThreadInfo.Client.Height <= 3 then Exit;

  if BeepTimer > 0 then begin
    Dec(BeepTimer);
    if BeepTimer <= 0 then Beep(0, 0);
  end;

  kosglMakeCurrent(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height, CTX);
  glViewPort(0, 0, ThreadInfo.Client.Width, ThreadInfo.Client.Height);

  cycleDist := Trunc(GameState.DistanceTravelled) mod 1000;

  if GameState.Score >= GameState.NextRainScore then begin
    GameState.RainEndScore := GameState.Score + 1000;
    GameState.NextRainScore := GameState.RainEndScore + 1200 + Trunc(Random * 2000.0);
  end;
  
  if (cycleDist >= 350) and (cycleDist < 400) then begin
    NightAmt := (cycleDist - 350) / 50.0;
    rainR := 0.5; rainG := 0.55; rainB := 0.65;
  end else if (cycleDist >= 400) and (cycleDist < 900) then begin
    NightAmt := 1.0;
    rainR := 0.2; rainG := 0.25; rainB := 0.35;
  end else if (cycleDist >= 900) and (cycleDist < 950) then begin
    NightAmt := 1.0 - (cycleDist - 900) / 50.0;
    rainR := 0.5; rainG := 0.55; rainB := 0.65;
  end else begin
    NightAmt := 0.0;
    rainR := 0.7; rainG := 0.8; rainB := 0.9;
  end;
  rSky := 0.53 + (0.08 - 0.53) * NightAmt;
  gSky := 0.81 + (0.13 - 0.81) * NightAmt;
  bSky := 0.92 + (0.18 - 0.92) * NightAmt;

  // Мокрый асфальт держится ещё 400 очков после конца ливня и плавно сохнет.
  if GameState.Score < GameState.RainEndScore then WetAmt := 1.0
  else if GameState.Score < GameState.RainEndScore + 400 then
    WetAmt := (GameState.RainEndScore + 400 - GameState.Score) / 400.0
  else WetAmt := 0.0;
  glClearColor(rSky, gSky, bSky, 0.0);
  // Цвет неба — цель растворения для тумана.
  SkyR := rSky; SkyG := gSky; SkyB := bSky;

  glClear(GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);

  Aspect := ThreadInfo.Client.Width / ThreadInfo.Client.Height;
  glMatrixMode(GL_PROJECTION); glLoadIdentity();
  Perspective(45.0 + (GameState.BoostMult - 1.0) * 8.0, Aspect, 0.1, 300.0);

  glMatrixMode(GL_MODELVIEW); glLoadIdentity();
  
  DrawCelestialBodies(cycleDist);

  if GameState.Score < GameState.RainEndScore then begin
    // Капля — четырёхугольник из 4 вершин вместо куба из 24, и один glBegin на весь
    // дождь вместо 40 пар glBegin/glEnd и 160 матричных операций.
    // GL_LINES тут не годится: в этой сборке TinyGL отрезки не растеризуются.
    // Скос верхних вершин по X заменяет прежний наклон капли на 10 градусов.
    glDisable(GL_DEPTH_TEST);
    glColor3f(rainR, rainG, rainB);
    glBegin(GL_QUADS);
    for i := 0 to 39 do begin
      rainY := 3.0 - Frac(RAIN_POS[i, 1] + GameState.FrameCount * 0.12) * 6.0;
      rainX := RAIN_POS[i, 0];
      glVertex3f(rainX - 0.018, rainY - 0.12, RAIN_POS[i, 2]);
      glVertex3f(rainX + 0.018, rainY - 0.12, RAIN_POS[i, 2]);
      glVertex3f(rainX + 0.053, rainY + 0.12, RAIN_POS[i, 2]);
      glVertex3f(rainX + 0.017, rainY + 0.12, RAIN_POS[i, 2]);
    end;
    glEnd();
    glEnable(GL_DEPTH_TEST);
  end;

  glTranslatef(0.0, -2.5, -1.0);
  glRotatef(15.0, 1.0, 0.0, 0.0);

  // Тряска камеры при аварии: только первая половина CrashTimer, пик тот же.
  if GameState.CrashTimer > 25 then begin
    shake := (GameState.CrashTimer - 25) * 0.014;
    glTranslatef(Sin(GameState.FrameCount * 2.1) * shake,
                 Cos(GameState.FrameCount * 1.7) * shake, 0.0);
  end;

  Upd := GameState.GameActive and (not GameState.Paused);

  if not GameState.Paused then GameState.FrameCount := GameState.FrameCount + 1;
  DrawBackdrop;
  DrawRoadGL;

  if GameState.ComboTimer > 0 then begin
    if not GameState.Paused then Dec(GameState.ComboTimer);
    if GameState.ComboTimer = 0 then GameState.Combo := 0;
  end;

  if GameState.Paused then begin
    // Замороженное состояние: никакой физики, только отрисовка.
  end else if KeyDown then begin
    // Тормоз: тот же множитель BoostMult, только вниз от единицы.
    GameState.BoostMult := GameState.BoostMult - 0.25;
    if GameState.BoostMult < BrakeMult then GameState.BoostMult := BrakeMult;
    // Клевок носом вперёд заодно зажигает стоп-сигналы (ветка pitch > 2.0).
    GameState.PlayerPitch := GameState.PlayerPitch + (6.0 - GameState.PlayerPitch) * 0.3;
  end else if KeyUp then begin
    if GameState.BoostMult < 1.0 then GameState.BoostMult := 1.0;
    if GameState.BoostMult < 3.5 then GameState.BoostMult := GameState.BoostMult + 0.2;
    GameState.PlayerPitch := GameState.PlayerPitch + (-3.0 - GameState.PlayerPitch) * 0.3;
  end else begin
    if GameState.BoostMult < 1.0 then begin
      GameState.BoostMult := GameState.BoostMult + 0.1;
      if GameState.BoostMult > 1.0 then GameState.BoostMult := 1.0;
      GameState.PlayerPitch := GameState.PlayerPitch + (0.0 - GameState.PlayerPitch) * 0.2;
    end else
    if GameState.BoostMult > 1.0 then begin
       GameState.BoostMult := GameState.BoostMult - 0.2; 
       if GameState.BoostMult < 1.0 then GameState.BoostMult := 1.0; 
       GameState.PlayerPitch := GameState.PlayerPitch + (4.0 - GameState.PlayerPitch) * 0.4;
    end else begin
       GameState.BoostMult := 1.0;
       GameState.PlayerPitch := GameState.PlayerPitch + (0.0 - GameState.PlayerPitch) * 0.2; 
    end;
  end;

  if Upd and (KeyLeft or KeyRight) then begin
    for i := 0 to 1 do begin
      Skids[SkidHead].x := GameState.PlayerX + SIGN[i] * 0.875;
      Skids[SkidHead].z := GameState.PlayerZ;
      Skids[SkidHead].alpha := 0.8;
      SkidHead := (SkidHead + 1) mod 40;
    end;
  end;

  GameState.PlayerZ := -6.0;

  if (GameState.DustTimer > 0) and (not GameState.Paused) then Dec(GameState.DustTimer);

  if Upd then begin
    GameState.Speed := (GameState.BaseSpeed + ((GameState.Score mod 1500) / 2000.0)) * GameState.BoostMult;
    
    GameState.GlobalCurve := Sin(GameState.DistanceTravelled * 0.002) * 2.0;

    GameState.PlayerX := GameState.PlayerX - (GameState.Speed * GameState.GlobalCurve * 0.04);
    if GameState.PlayerX < -5.0 then GameState.PlayerX := -5.0;
    if GameState.PlayerX > 5.0 then GameState.PlayerX := 5.0;
  end;

  for i := 1 to MaxObstacles do begin
    if Upd then begin
      Obstacles[i].z := Obstacles[i].z + (GameState.Speed + IfThenF(Obstacles[i].IsCoin or Obstacles[i].IsDonkey, 0.0, 0.15));

      if (not Obstacles[i].IsCoin) and (not Obstacles[i].IsDonkey) and (Random < 0.01) then
        Obstacles[i].TargetX := (Random * 9.0) - 4.5;
        
      if Abs(Obstacles[i].x - Obstacles[i].TargetX) > 0.1 then begin
        if Obstacles[i].x < Obstacles[i].TargetX then begin
          Obstacles[i].x := Obstacles[i].x + 0.05;
          Obstacles[i].Tilt := -8.0;
        end else begin
          Obstacles[i].x := Obstacles[i].x - 0.05;
          Obstacles[i].Tilt := 8.0;
        end;
      end else
        Obstacles[i].Tilt := Obstacles[i].Tilt * 0.8;
    end;

    if Obstacles[i].z > 2.0 then SpawnObstacle(Obstacles[i], -180.0);

    // Полотно дороги кончается на RoadLength*1.2 = -120, дальше рисовать нечего.
    if Obstacles[i].z >= CullZ then begin
    curveX := GetRoadXOffset(Obstacles[i].z);
    SetFog(Obstacles[i].z);

    if Obstacles[i].IsDonkey then
      DrawDonkeyGL(curveX + Obstacles[i].x, -0.9, Obstacles[i].z)
    else
      DrawCarGL(curveX + Obstacles[i].x, -0.9, Obstacles[i].z, Obstacles[i].r, Obstacles[i].g, Obstacles[i].b, Obstacles[i].Tilt, 0.0, False, Obstacles[i].IsTruck, Obstacles[i].IsSports, Obstacles[i].HasSiren, Obstacles[i].IsCoin, Obstacles[i].IsVan, Obstacles[i].IsPickup);
    end;

    if Upd and (Abs(Obstacles[i].z - GameState.PlayerZ) < 2.05) and (Abs(Obstacles[i].x - GameState.PlayerX) < 1.4) then begin
      if Obstacles[i].IsCoin then begin
        // Комбо: каждая следующая монета в серии дороже, серия живёт ComboTime кадров.
        if GameState.Combo < 8 then Inc(GameState.Combo);
        GameState.ComboTimer := ComboTime;
        GameState.FloatScore := GameState.FloatScore + 500.0 + (GameState.Combo - 1) * 250.0;
        SpawnObstacle(Obstacles[i], -180.0);
        Beep(1000 + GameState.Combo * 150, 2);
      end else begin
        GameState.GameActive := False;
        GameState.GameOver := True;
        GameState.CrashTimer := 50;
        GameState.DustTimer := 0;
        GameState.Combo := 0;
        GameState.ComboTimer := 0;
        Beep(100, 10);
      end;
    end;

    // Бонус за близкий разъезд: начисляется один раз, когда машина ушла за спину.
    if Upd and (not Obstacles[i].Passed) and (Obstacles[i].z > GameState.PlayerZ + 2.05) then begin
      Obstacles[i].Passed := True;
      if (not Obstacles[i].IsCoin) and (not Obstacles[i].IsDonkey) and
         (Abs(Obstacles[i].x - GameState.PlayerX) < NearMissDist) then begin
        GameState.FloatScore := GameState.FloatScore + 150.0;
      end;
    end;
  end;

  FogAmt := 0.0;
  DrawCarGL(GameState.PlayerX, -0.9, GameState.PlayerZ, 1.0, 0.84, 0.0, GameState.PlayerTilt, GameState.PlayerPitch, True, False, False, False, False, False, False);

  if Upd then begin
    GameState.DistanceTravelled := GameState.DistanceTravelled + GameState.Speed;
    GameState.FloatScore := GameState.FloatScore + GameState.Speed * 5.0;
    GameState.Score := Trunc(GameState.FloatScore);
  end else if GameState.GameOver then begin
    if GameState.CrashTimer > 0 then Dec(GameState.CrashTimer);
  end;

  if GameState.GameOver and (GameState.CrashTimer <= 0) then begin
    DrawGameUI(Aspect);
    kosglSwapBuffers();
    Sleep(267);
    with GameState do begin
      PlayerX := 0; PlayerZ := -6.0; PlayerTilt := 0; PlayerPitch := 0; 
      DistanceTravelled := 0; FloatScore := 0; Score := 0; GlobalCurve := 0.0;
      Speed := BaseSpeed; BoostMult := 1.0; GameActive := True; GameOver := False;
      CrashTimer := 0; DustTimer := 0; NextDonkeyScore := 1500;
      NextRainScore := 500 + Trunc(Random * 900.0); RainEndScore := 0;
      Combo := 0; ComboTimer := 0; Paused := False;
    end;
    DriftX := 0.0;
    InitObstacles;
    Exit;
  end;

  if Upd then begin
    if KeyLeft and (GameState.PlayerX > -5.0) then begin
      GameState.PlayerX := GameState.PlayerX - 0.3;
      GameState.PlayerTilt := -14.0;
      DriftX := DriftX - 0.05 * WetAmt;
    end else if KeyRight and (GameState.PlayerX < 5.0) then begin
      GameState.PlayerX := GameState.PlayerX + 0.3;
      GameState.PlayerTilt := 14.0;
      DriftX := DriftX + 0.05 * WetAmt;
    end else
      GameState.PlayerTilt := GameState.PlayerTilt * 0.6;

    // На мокром машину сносит: руление копит боковую скорость, которая гаснет не сразу.
    GameState.PlayerX := GameState.PlayerX + DriftX;
    DriftX := DriftX * 0.88;
    if GameState.PlayerX < -5.0 then begin GameState.PlayerX := -5.0; DriftX := 0.0; end;
    if GameState.PlayerX > 5.0 then begin GameState.PlayerX := 5.0; DriftX := 0.0; end;
  end;

  DrawGameUI(Aspect);
  kosglSwapBuffers();
end;

begin
  TinyGL_initialization;
  Randomize;
  SetKeyboardInputMode(kmScan);
  with GameState do begin
    PlayerX := 0.0; PlayerZ := -6.0; PlayerTilt := 0.0; PlayerPitch := 0.0; 
    DistanceTravelled := 0.0; FloatScore := 0.0; BaseSpeed := 0.4; GlobalCurve := 0.0;
    Speed := 0.14; BoostMult := 1.0; Score := 0; FrameCount := 0; CrashTimer := 0; DustTimer := 0;
    NextDonkeyScore := 1500; GameActive := True; GameOver := False;
    NextRainScore := 500 + Trunc(Random * 900.0); RainEndScore := 0;
    Combo := 0; ComboTimer := 0; Paused := False;
  end;
  SkyR := 0.53; SkyG := 0.81; SkyB := 0.92;
  FogAmt := 0.0; NightAmt := 0.0; WetAmt := 0.0; DriftX := 0.0;
  KeyLeft := False; KeyRight := False; KeyUp := False; KeyDown := False;
  SkidHead := 0;
  BeepTimer := 0;
  InitObstacles;

  with GetScreenSize do begin
    WndWidth := Width - (Width div 3); WndHeight := Height - (Height div 3);
    WndLeft := (Width - WndWidth) div 2; WndTop := (Height - WndHeight) div 2;
  end;

  while True do
    case WaitEventByTime(1) of
      REDRAW_EVENT: begin
        BeginDraw;
        DrawWindow(WndLeft, WndTop, WndWidth, WndHeight, 'XDRASH', $00FFFFFF,
          WS_SKINNED_SIZABLE + WS_CLIENT_COORDS + WS_CAPTION + WS_TRANSPARENT_FILL, CAPTION_MOVABLE);
        GLDraw;
        EndDraw;
      end;
      KEY_EVENT:
        case GetKey.Code of
          #075: begin 
              KeyLeft := True;   
              Beep(30, 10);
          end; 
          #077: begin 
              KeyRight := True;  
              Beep(30, 10);
          end; 
          #072: begin
            // Только на само нажатие, иначе автоповтор клавиши строчит писком.
            if not KeyUp then begin
              GameState.DustTimer := 20;
              Beep(80, 10);
            end;
            KeyUp := True;
          end;
          #200: KeyUp := False;
          #080: KeyDown := True;
          #208: KeyDown := False;
          #025: GameState.Paused := not GameState.Paused;   // P — пауза
          #203: KeyLeft := False;
          #205: KeyRight := False;
        end;
      BUTTON_EVENT:
        if GetButton.ID = 1 then ExitThread;
    else
      GLDraw;
    end;
end.