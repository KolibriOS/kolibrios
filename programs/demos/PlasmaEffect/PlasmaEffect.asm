Comment *---------------------------+
|                                   |
|           Plasma Effect           |
|                                   |
+-----------------------------------*
.686
.Model Flat, StdCall
Option CaseMap: None

RepArg Macro Arg
  Local NewStr
    Quot SubStr <Arg>, 1, 1
  IfIdn Quot, <">
    .Data
      NewStr db Arg,0
    .Code
    ExitM <ADDR NewStr>
  Else
    ExitM <Arg>
  EndIf
EndM

@ Macro Function:Req, Args:VarArg
  Arg equ <Invoke Function>
  For Var, <Args>
    Arg CatStr Arg, <, RepArg(Var)>
  EndM
  Arg
  ExitM <eax>
EndM

Public @Main

; Event Constants
REDRAW_EVENT      = 1
KEY_EVENT         = 2

; Event Mask Constants
EM_REDRAW         = 1
EM_KEY            = 2

; Window styles
WS_FILL_TRANSPARENT  = 40000000H

TSize Struct
  SizeY Word ?
  SizeX Word ?
TSize EndS

TRGBQuad Struct
  Blue     Byte ?
  Green    Byte ?
  Red      Byte ?
  reserved Byte ?
TRGBQuad EndS

SetEventMask    Proto EventMask:Dword
CheckEvent      Proto
BeginDraw       Proto
EndDraw         Proto
GetScreenSize   Proto
DrawWindow      Proto Left:SDword, Top:SDword, Right:SDword, Bottom:SDword, Caption:Ptr, BackColor:Dword, Style:Dword, CapStyle:Dword
DrawImageEx     Proto Image:Ptr, X:SDword, Y:SDword, XSize:Dword, YSize:Dword, BPP:Dword, Palette:Ptr, Padding:Dword
HeapCreate      Proto
HeapAllocate    Proto Bytes:Dword
GetTickCount    Proto
Sleep           Proto Time:Dword
ThreadTerminate Proto

.Const
Float_50 Real4 50.0 ; to keep the FrameRate
Float_40 Real4 40.0 ; around 40-50 FPS
; The table below can be calculated approximately as: SinTab[i] = Sin(2 * PI * i / 255) * 128 + 128
SinTab Label Byte
Byte 128, 131, 134, 137, 141, 144, 147, 150, 153, 156, 159, 162, 165, 168, 171, 174
Byte 177, 180, 183, 186, 189, 191, 194, 197, 199, 202, 205, 207, 209, 212, 214, 217
Byte 219, 221, 223, 225, 227, 229, 231, 233, 235, 236, 238, 240, 241, 243, 244, 245
Byte 246, 248, 249, 250, 251, 252, 252, 253, 254, 254, 255, 255, 255, 255, 255, 255
Byte 255, 255, 255, 255, 255, 255, 254, 254, 253, 253, 252, 251, 250, 249, 248, 247
Byte 246, 245, 243, 242, 240, 239, 237, 236, 234, 232, 230, 228, 226, 224, 222, 220
Byte 218, 215, 213, 211, 208, 206, 203, 201, 198, 195, 193, 190, 187, 184, 181, 179
Byte 176, 173, 170, 167, 164, 161, 158, 155, 152, 148, 145, 142, 139, 136, 133, 130
Byte 126, 123, 120, 117, 114, 111, 108, 104, 101,  98,  95,  92,  89,  86,  83,  80
Byte  77,  75,  72,  69,  66,  63,  61,  58,  55,  53,  50,  48,  45,  43,  41,  38
Byte  36,  34,  32,  30,  28,  26,  24,  22,  20,  19,  17,  16,  14,  13,  11,  10
Byte   9,   8,   7,   6,   5,   4,   3,   3,   2,   2,   1,   1,   0,   0,   0,   0
Byte   0,   0,   0,   1,   1,   1,   2,   2,   3,   4,   4,   5,   6,   7,   8,  10
Byte  11,  12,  13,  15,  16,  18,  20,  21,  23,  25,  27,  29,  31,  33,  35,  37
Byte  39,  42,  44,  47,  49,  51,  54,  57,  59,  62,  65,  67,  70,  73,  76,  79
Byte  82,  85,  88,  91,  94,  97, 100, 103, 106, 109, 112, 115, 119, 122, 125, 128

.Data
i1 Dword 50
j1 Dword 90
Freq Dword 100 ; // GetTickCount return count of 1/100s of second
Instant Real4 0.0
SleepTime Dword 0

.Data?
Screen TSize <>
i2 Dword ?
j2 Dword ?
Palette TRGBQuad 256 Dup (<>)
Image Dword ?
ImageWidth Dword ?
ImageHeight Dword ?
FrameStart Dword ?

.Code
CreatePalette Proc Uses ebx
Local i:Dword
Local Red:Dword, Green:Dword, Blue:Dword
  mov i, 0
  .Repeat
    mov eax, i
    .If eax <= 63
      ; do nothing
    .ElseIf eax <= 127
      neg eax
      add eax, 127
    .ElseIf eax <= 189
      sub eax, 128
    .ElseIf eax <= 255
      neg eax
      add eax, 255
    .EndIf

    mov Red, eax
    mov Green, eax
    mov Blue, eax

    .If i <= 127
      shl Green, 1
      shl Blue, 2
    .ElseIf i <= 255
      shl Red, 2
      shl Blue, 1
    .EndIf

    mov eax, Red   ;
    cmp eax, 255   ;
    setna bl       ; IF Red > 255 THEN bl = 255
    dec bl         ;              ELSE bl = Red
    or bl, al      ;

    mov eax, Green ;
    cmp eax, 255   ;
    setna cl       ; IF Green > 255 THEN cl = 255
    dec cl         ;                ELSE cl = Green
    or cl, al      ;

    mov eax, Blue  ;
    cmp eax, 255   ;
    setna dl       ; IF Blue > 255 THEN dl = 255
    dec dl         ;               ELSE dl = Blue
    or dl, al      ;

    mov eax, i
    mov Palette.Red[eax * SizeOf(Type(Palette))], bl
    mov Palette.Green[eax * SizeOf(Type(Palette))], cl
    mov Palette.Blue[eax * SizeOf(Type(Palette))], dl

    inc i
  .Until i > 255
  ret
CreatePalette EndP
;*******************************************************************************
Render Proc
Local X:Dword, Y:Dword
Local Row:Ptr
  dec i1
  add j1, 2

  mov Y, 0
  Align 4
  .Repeat
    mov eax, Y
    add eax, i1
    and eax, 255
    movzx eax, SinTab[eax]
    mov i2, eax

    mov eax, j1
    and eax, 255
    movzx eax, SinTab[eax]
    mov j2, eax

    mov eax, Y
    mul ImageWidth
    add eax, Image
    mov Row, eax

    mov X, 0
    Align 4
    .Repeat
      mov eax, X
      add eax, i2
      and eax, 255
      movzx ecx, SinTab[eax]

      mov eax, Y
      add eax, j2
      and eax, 255
      movzx eax, SinTab[eax]
      add ecx, eax

      mov eax, Row
      add eax, X
      mov [eax], cl

      inc X
      mov eax, ImageWidth
    .Until eax == X
    inc Y
    mov eax, ImageHeight
  .Until eax == Y
  ret
Render EndP
;*******************************************************************************
QueryPerf Proc
Local Diff:Dword
  Invoke GetTickCount
  sub eax, FrameStart
  mov Diff, eax
  fild Diff
  fild Freq
  fdivrp
  fstp Instant
  mov FrameStart, @(GetTickCount)
  ret
QueryPerf EndP
;*******************************************************************************
Waiting Proc
; Keep the FrameRate around 40-50 FPS
  fld Float_50
  fld Instant
  fcomip st(0), st(1)
  fstp st(0)
  .If !CARRY? && !ZERO?
    inc SleepTime
  .Else
    fld Float_40
    fld Instant
    fcomip st(0), st(1)
    fstp st(0)
    .If CARRY? && (SleepTime != 0)
      dec SleepTime
    .EndIf
  .EndIf
  Invoke Sleep, SleepTime
  ret
Waiting EndP
;*******************************************************************************
@Main:
  Invoke HeapCreate
  mov Screen, @(GetScreenSize)
  Invoke CreatePalette
  movzx eax, Screen.SizeX
  movzx ecx, Screen.SizeY
  mov ImageWidth, eax
  mov ImageHeight, ecx
  mul ecx
  mov Image, @(HeapAllocate, eax)
  mov FrameStart, @(GetTickCount)
  Invoke SetEventMask, EM_REDRAW + EM_KEY
  .Repeat
    Invoke CheckEvent
    .If eax == REDRAW_EVENT
      Invoke BeginDraw
      Invoke DrawWindow, 0, 0, ImageWidth, ImageHeight, 0, 0, WS_FILL_TRANSPARENT, 0
      Invoke EndDraw
    .ElseIf eax == KEY_EVENT
      Invoke ThreadTerminate
    .Else
      Invoke Render
      Invoke QueryPerf
      Invoke DrawImageEx, Image, 0, 0, ImageWidth, ImageHeight, 8, Offset Palette, 0
      Invoke Waiting
    .EndIf
  .Until 0
END